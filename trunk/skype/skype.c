/*

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


*/

#include "skype.h"
#include "debug.h"
#include "skypeapi.h"
#include "skypesvc.h"
#include "contacts.h"
#include "utf8.h"
#include "pthread.h"
#include "gchat.h"
#include "m_toptoolbar.h"
#include "time.h"
#include "voiceservice.h"
#include "msglist.h"
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif
#ifdef _WIN64
#pragma comment (lib, "bufferoverflowU.lib")
#endif


struct MM_INTERFACE   mmi; 

POPUPDATAT MessagePopup;

// Exported Globals
HWND hSkypeWnd=NULL, hWnd=NULL;
HANDLE SkypeReady, SkypeMsgReceived, hInitChat=NULL, httbButton=NULL, FetchMessageEvent=NULL;
BOOL SkypeInitialized=FALSE, QueryMsgDirection=FALSE, MirandaShuttingDown=FALSE;
BOOL UseSockets=FALSE, bSkypeOut=FALSE, bProtocolSet=FALSE;
char skype_path[MAX_PATH], protocol=2, *pszProxyCallout=NULL;
int SkypeStatus=ID_STATUS_OFFLINE, hSearchThread=-1, receivers=1;
UINT ControlAPIAttach, ControlAPIDiscover;
LONG AttachStatus=-1;
HINSTANCE hInst;
HANDLE hProtocolAvatarsFolder;
char DefaultAvatarsFolder[MAX_PATH+1];
DWORD mirandaVersion;

CRITICAL_SECTION RingAndEndcallMutex, QueryThreadMutex;

// Module Internal Globals
PLUGINLINK *pluginLink;
HANDLE MessagePumpReady;
HANDLE hChatEvent=NULL, hChatMenu=NULL;
HANDLE hEvInitChat=NULL, hBuddyAdded=NULL;
HANDLE hMenuAddSkypeContact=NULL;

DWORD msgPumpThreadId = 0;
#ifdef SKYPEBUG_OFFLN
HANDLE GotUserstatus;
#endif

BOOL ImportingHistory=FALSE, bModulesLoaded=FALSE;
char *RequestedStatus=NULL;	// To fix Skype-API Statusmode-bug
char cmdMessage[12]="MESSAGE", cmdPartner[8]="PARTNER";	// Compatibility commands

// Direct assignment of user properties to a DB-Setting
static const settings_map m_settings[]= {
		{"LANGUAGE", "Language1"},
		{"PROVINCE", "State"},
		{"CITY", "City"},
		{"PHONE_HOME", "Phone"},
		{"PHONE_OFFICE", "CompanyPhone"},
		{"PHONE_MOBILE", "Cellular"},
		{"HOMEPAGE", "Homepage"},
		{"ABOUT", "About"}
	};

// Imported Globals
extern status_map status_codes[];

BOOL (WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD) = 0;

HMODULE hUxTheme = 0;

// function pointers, use typedefs for casting to shut up the compiler when using GetProcAddress()

typedef BOOL (WINAPI *PITA)();
typedef HANDLE (WINAPI *POTD)(HWND, LPCWSTR);
typedef UINT (WINAPI *PDTB)(HANDLE, HDC, int, int, RECT *, RECT *);
typedef UINT (WINAPI *PCTD)(HANDLE);
typedef UINT (WINAPI *PDTT)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, RECT *);

PITA pfnIsThemeActive = 0;
POTD pfnOpenThemeData = 0;
PDTB pfnDrawThemeBackground = 0;
PCTD pfnCloseThemeData = 0;
PDTT pfnDrawThemeText = 0;

#define FIXED_TAB_SIZE 100                  // default value for fixed width tabs

typedef struct {
	char msgnum[16];
	BOOL getstatus;
	BOOL bIsRead;
	TYP_MSGLENTRY *pMsgEntry;
} fetchmsg_arg;

/*
 * visual styles support (XP+)
 * returns 0 on failure
 */

int InitVSApi()
{
    if((hUxTheme = LoadLibraryA("uxtheme.dll")) == 0)
        return 0;

    pfnIsThemeActive = (PITA)GetProcAddress(hUxTheme, "IsThemeActive");
    pfnOpenThemeData = (POTD)GetProcAddress(hUxTheme, "OpenThemeData");
    pfnDrawThemeBackground = (PDTB)GetProcAddress(hUxTheme, "DrawThemeBackground");
    pfnCloseThemeData = (PCTD)GetProcAddress(hUxTheme, "CloseThemeData");
    pfnDrawThemeText = (PDTT)GetProcAddress(hUxTheme, "DrawThemeText");
    
    MyEnableThemeDialogTexture = (BOOL (WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
    if(pfnIsThemeActive != 0 && pfnOpenThemeData != 0 && pfnDrawThemeBackground != 0 && pfnCloseThemeData != 0 && pfnDrawThemeText != 0) {
        return 1;
    }
    return 0;
}

/*
 * unload uxtheme.dll
 */

int FreeVSApi()
{
    if(hUxTheme != 0)
        FreeLibrary(hUxTheme);
    return 0;
}

// Plugin Info
PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Skype protocol",
	PLUGIN_MAKE_VERSION(0,0,0,49),
	"Support for Skype network",
	"leecher - tweety - jls17",
	"leecher@dose.0wnz.at - tweety@user.berlios.de",
	"© 2004-2011 leecher - tweety",
	"http://dose.0wnz.at - http://developer.berlios.de/projects/mgoodies/",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
	{ 0xa71f8335, 0x7b87, 0x4432, { 0xb8, 0xa3, 0x81, 0x47, 0x94, 0x31, 0xc6, 0xf5 } } // {A71F8335-7B87-4432-B8A3-81479431C6F5}
};

#define MAPDND	1	// Map Occupied to DND status and say that you support it
//#define MAPNA   1 // Map NA status to Away and say that you support it

/*                           P R O G R A M                                */

void RegisterToDbeditorpp(void)
{
    // known modules list
    if (ServiceExists("DBEditorpp/RegisterSingleModule"))
        CallService("DBEditorpp/RegisterSingleModule", (WPARAM)SKYPE_PROTONAME, 0);
}

void RegisterToUpdate(void)
{
	//Use for the Updater plugin
	if(ServiceExists(MS_UPDATE_REGISTER)) 
	{
		Update update = {0};
		char szVersion[16];

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO *)&pluginInfo, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);

#ifdef _WIN64
#ifdef _UNICODE
		update.szBetaUpdateURL = "http://dose.0wnz.at/miranda/Skype/Skype_protocol_unicode_x64.zip";
#else
		update.szBetaUpdateURL = "http://dose.0wnz.at/miranda/Skype/Skype_protocol_x64.zip";
#endif
		update.szBetaVersionURL = "http://dose.0wnz.at/miranda/Skype/";
		update.pbBetaVersionPrefix = (BYTE *)"SKYPE version ";
		update.szUpdateURL = update.szBetaUpdateURL;	// FIXME!!
		update.szVersionURL = update.szBetaVersionURL; // FIXME
		update.pbVersionPrefix = update.pbBetaVersionPrefix; //FIXME
#else /* _WIN64 */
#ifdef _UNICODE
		update.szBetaUpdateURL = "http://dose.0wnz.at/miranda/Skype/Skype_protocol_unicode.zip";
#else
	    update.szBetaUpdateURL = "http://dose.0wnz.at/miranda/Skype/Skype_protocol.zip";
#endif
		update.szBetaVersionURL = "http://dose.0wnz.at/miranda/Skype/";
		update.pbBetaVersionPrefix = (BYTE *)"SKYPE version ";
#ifdef _UNICODE
		update.szUpdateURL = update.szBetaUpdateURL;	// FIXME!!
		update.szVersionURL = update.szBetaVersionURL; // FIXME
		update.pbVersionPrefix = update.pbBetaVersionPrefix; //FIXME
#else
		update.szUpdateURL = "http://addons.miranda-im.org/feed.php?dlfile=3200";
		update.szVersionURL = "http://addons.miranda-im.org/details.php?action=viewfile&id=3200";
		update.pbVersionPrefix = (BYTE *)"<span class=\"fileNameHeader\">Skype Protocol ";
#endif
#endif

		update.cpbVersionPrefix = strlen((char *)update.pbVersionPrefix);
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);

	}
}

/*
 * ShowMessage
 *
 * Shows a popup, if the popup plugin is enabled.
 * mustShow: 1 -> If Popup-Plugin is not available/disabled, show Message
 *                in a Messagewindow
 *                If the Popup-Plugin is enabled, let the message stay on
 *                screen until someone clicks it away.
 *           0 -> If Popup-Plugin is not available/disabled, skip message
 * Returns 0 on success, -1 on failure
 *
 */
int ShowMessage(int iconID, TCHAR *lpzText, int mustShow) {



	if (DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "SuppressErrors", 0)) return -1;
	lpzText=TranslateTS(lpzText);

	if (bModulesLoaded && ServiceExists(MS_POPUP_ADDPOPUPT) && DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UsePopup", 0) && !MirandaShuttingDown) {
		BOOL showPopup, popupWindowColor;
		unsigned int popupBackColor, popupTextColor;
		int popupTimeSec;

		popupTimeSec = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupTimeSecErr", 4);
		popupTextColor = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupTextColorErr", GetSysColor(COLOR_WINDOWTEXT));
		popupBackColor = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupBackColorErr", GetSysColor(COLOR_BTNFACE));
		popupWindowColor = ( 0 != DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "popupWindowColorErr", TRUE));
		showPopup = ( 0 != DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "showPopupErr", TRUE));

		MessagePopup.lchContact = NULL;
		MessagePopup.lchIcon = LoadIcon(hInst,MAKEINTRESOURCE(iconID));
		MessagePopup.colorBack = ! popupWindowColor ? popupBackColor : GetSysColor(COLOR_BTNFACE);
		MessagePopup.colorText = ! popupWindowColor ? popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
		MessagePopup.iSeconds = popupTimeSec;
		MessagePopup.PluginData = (void *)1;
		
		lstrcpy(MessagePopup.lptzText, lpzText);

		lstrcpy(MessagePopup.lptzContactName, TranslateT(SKYPE_PROTONAME));

		if(showPopup)
			CallService(MS_POPUP_ADDPOPUPT,(WPARAM)&MessagePopup,0);

		return 0;
	} 
	else {

		if (mustShow==1) MessageBox(NULL,lpzText,_T("Skype protocol"), MB_OK | MB_ICONWARNING);
			return 0;
	}
	return -1;


}
#ifdef _UNICODE
int ShowMessageA(int iconID, char *lpzText, int mustShow) {
	WCHAR *lpwText;
	int iRet;
	size_t len = mbstowcs (NULL, lpzText, strlen(lpzText));
	if (len == -1 || !(lpwText = calloc(len+1,sizeof(WCHAR)))) return -1;
	iRet = ShowMessage(iconID, lpwText, mustShow);
	free (lpwText);
	return iRet;
}
#endif

// processing Hooks

int HookContactAdded(WPARAM wParam, LPARAM lParam) {
	char *szProto;

	szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
	if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME))
		add_contextmenu((HANDLE)wParam);
	return 0;
}

int HookContactDeleted(WPARAM wParam, LPARAM lParam) {
	char *szProto;

	szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
	if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME)) {
		DBVARIANT dbv;
		int retval;

		if (DBGetContactSettingString((HANDLE)wParam, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) return 1;
		retval=SkypeSend("SET USER %s BUDDYSTATUS 1", dbv.pszVal);
		DBFreeVariant(&dbv);
		if (retval) return 1;
	}
	return 0;
}

void GetInfoThread(HANDLE hContact) {
	DBVARIANT dbv, dbv2;
	int i;
	char *ptr;
	// All properties are already handled in the WndProc, so we just consume the 
	// messages here to do proper ERROR handling
	// If you add something here, be sure to handle it in WndProc, but let it
	// fall through there so that message gets added to the queue in order to be
	// consumed by SkypeGet
	char *pszProps[] = {
		"FULLNAME", "BIRTHDAY", "COUNTRY", "SEX", "MOOD_TEXT", "TIMEZONE", "IS_VIDEO_CAPABLE"};


	LOG (("GetInfoThread started."));
	if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) 
	{
		LOG (("GetInfoThread terminated, cannot find Skype Name for contact %08X.", hContact));
		return;
	}
	if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, "Nick", &dbv2))
		DBWriteContactSettingString(hContact, SKYPE_PROTONAME, "Nick", dbv.pszVal);
	else
		DBFreeVariant(&dbv2);

	EnterCriticalSection (&QueryThreadMutex);

	for (i=0; i<sizeof(pszProps)/sizeof(pszProps[0]); i++)
		if (ptr=SkypeGet ("USER", dbv.pszVal, pszProps[i])) free (ptr);

	if (protocol >= 7) {
		// Notify about the possibility of an avatar
		ACKDATA ack = {0};
		ack.cbSize = sizeof( ACKDATA );
		ack.szModule = SKYPE_PROTONAME;
		ack.hContact = hContact;
		ack.type = ACKTYPE_AVATAR;
		ack.result = ACKRESULT_STATUS;

		CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );
		if (ptr=SkypeGet ("USER", dbv.pszVal, "RICH_MOOD_TEXT")) free (ptr);
	}

	for (i=0; i<sizeof(m_settings)/sizeof(m_settings[0]); i++)
		if (ptr=SkypeGet ("USER", dbv.pszVal, m_settings[i].SkypeSetting)) free (ptr);

	ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
	LeaveCriticalSection(&QueryThreadMutex);
	DBFreeVariant(&dbv);
    LOG (("GetInfoThread terminated gracefully."));
}

void BasicSearchThread(char *nick) {
	PROTOSEARCHRESULT psr={0};
	char *cmd=NULL, *token=NULL, *ptr=NULL;
	time_t st;

    LOG (("BasicSearchThread started."));
	EnterCriticalSection (&QueryThreadMutex);
	time(&st);
	if (SkypeSend("SEARCH USERS %s", nick)==0 && (cmd=SkypeRcvTime("USERS", st, INFINITE))) {
		if (strncmp(cmd, "ERROR", 5)) {
			psr.cbSize=sizeof(psr);
			for (token=strtok(cmd+5, ", "); token; token=strtok(NULL, ", ")) {
				psr.nick=psr.id=token;
				psr.lastName=NULL;
				psr.firstName=NULL;
				psr.email=NULL;
				if (ptr=SkypeGet("USER", token, "FULLNAME")) {
					// We cannot use strtok() to seperate first & last name here,
					// because we already use it for parsing the user list
					// So we use our own function
					if (psr.lastName=strchr(ptr, ' ')) {
						*psr.lastName=0;
						psr.lastName++;
						LOG(("BasicSearchThread: lastName=%s", psr.lastName));
					}
					psr.firstName=ptr;
					LOG(("BasicSearchThread: firstName=%s", psr.firstName));
				}
				ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)hSearchThread, (LPARAM)(PROTOSEARCHRESULT*)&psr);
				if (ptr) free(ptr);
			}
		} else {
			OUT(cmd);
		}
		free(cmd);
	}
	ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)hSearchThread, 0);
	free(nick);
	LeaveCriticalSection(&QueryThreadMutex);
    LOG (("BasicSearchThread terminated gracefully."));
	return;
}

INT_PTR SkypeDBWriteContactSettingUTF8String(HANDLE hContact,const char *szModule,const char *szSetting,const char *val)
{
	DBCONTACTWRITESETTING cws;
	INT_PTR iRet;

	// Try to save it as UTF8 sting to DB. If this doesn't succeed (i.e. older Miranda version), we convert
	// accordingly and try to save again.

	cws.szModule=szModule;
	cws.szSetting=szSetting;
	cws.value.type=DBVT_UTF8;
	cws.value.pszVal=(char*)val;
	// DBVT_UTF8 support started with version 0.5.0.0, right...?
	if (mirandaVersion < 0x050000 || (iRet = CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws)))
	{
		// Failed, try to convert and then try again
		cws.value.type=DBVT_TCHAR;
#ifdef _UNICODE
		if (!(cws.value.ptszVal = make_unicode_string(val))) return -1;
#else
		if (utf8_decode(val, &cws.value.pszVal)==-1) return -1;
#endif
		iRet = CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
		free (cws.value.pszVal);
	}
	return iRet;
}


// added by TioDuke
void GetDisplaynameThread(char *dummy) {
	DBVARIANT dbv;
	char *ptr, *utfdstr=NULL;
    
	LOG(("GetDisplaynameThread started."));
	if (DBGetContactSettingString(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) {
		LOG(("GetDisplaynameThread terminated."));
		return;
	}
	EnterCriticalSection(&QueryThreadMutex);
    if ((ptr=SkypeGet("USER", dbv.pszVal, "FULLNAME"))) {
		if (*ptr) SkypeDBWriteContactSettingUTF8String(NULL, SKYPE_PROTONAME, "Nick", ptr);
		free(ptr);
	}
	DBFreeVariant(&dbv);
	LeaveCriticalSection(&QueryThreadMutex);
    LOG(("GetDisplaynameThread terminated gracefully."));
}


// Starts importing history from Skype
INT_PTR ImportHistory(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	
	if (DBGetContactSettingString((HANDLE)wParam, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) return 0;

	QueryMsgDirection=TRUE;
	ImportingHistory=TRUE;

	SkypeSend("SEARCH %sS %s", cmdMessage, dbv.pszVal);
	DBFreeVariant(&dbv);
	return 0;
}

int SearchFriends(void) {
	char *ptr, *token;
	int iRet = 0;
	time_t st;

	time(&st);
	if (SkypeSend("SEARCH FRIENDS")!=-1 && (ptr=SkypeRcvTime("USERS", st, INFINITE)))
	{
		if (strncmp(ptr, "ERROR", 5)) {
			if (ptr+5) {
				for (token=strtok(ptr+5, ", "); token; token=strtok(NULL, ", ")) {
					if (SkypeSend("GET USER %s ONLINESTATUS", token)==-1)
					{
						iRet = -1;
						break;
					}
				}
			}
		} else iRet=-1;
		free(ptr);
	} else iRet=-1;
	return iRet;
}

int SearchUsersWaitingMyAuthorization(void) {
	char *cmd, *token;

	if (SkypeSend("#UWA SEARCH USERSWAITINGMYAUTHORIZATION")) return -1;
	if (!(cmd=SkypeRcv("#UWA USERS", INFINITE))) return -1;
	if (!strncmp(cmd, "ERROR", 5)) {
		free(cmd);
		return -1;
	}

	token=strtok(cmd+10, ", ");
	while (token) {
		CCSDATA ccs={0};
		PROTORECVEVENT pre={0};
		HANDLE hContact;
		char *firstname=NULL, *lastname=NULL, *pCurBlob;
		
		LOG(("Awaiting auth: %s", token));
		ccs.szProtoService=PSR_AUTH;
		ccs.hContact=hContact=add_contact(token, PALF_TEMPORARY);
		ccs.wParam=0;
		ccs.lParam=(LPARAM)&pre;
		pre.flags=0;
		pre.timestamp=(DWORD)time(NULL);

		/* blob is: */
		//DWORD protocolSpecific HANDLE hContact
		//ASCIIZ nick, firstName, lastName, e-mail, requestReason
		if (firstname=SkypeGet("USER", token, "FULLNAME"))
			if (lastname=strchr(firstname, ' ')) {
				*lastname=0;
				lastname++;
			}
	
		pre.lParam=sizeof(DWORD)+sizeof(HANDLE)+strlen(token)+5;
		if (firstname) pre.lParam+=strlen(firstname);
		if (lastname) pre.lParam+=strlen(lastname);
		if (pre.szMessage  = pCurBlob = (char *)calloc(1, pre.lParam)) {
			pCurBlob+=sizeof(DWORD); // Not used
			memcpy(pCurBlob,&hContact,sizeof(HANDLE));	pCurBlob+=sizeof(HANDLE);
			strcpy((char *)pCurBlob,token);				pCurBlob+=strlen((char *)pCurBlob)+1;
			if (firstname) {
				strcpy((char *)pCurBlob,firstname); 
				if (lastname) {
					pCurBlob+=strlen((char *)pCurBlob)+1;
					strcpy((char *)pCurBlob,lastname);
				}
			}
			CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);
			free(pre.szMessage);
		}
		if (firstname) free(firstname);
		token=strtok(NULL, ", ");
	}
	free(cmd);
	return 0;
}

void SearchFriendsThread(char *dummy) {
	if (!SkypeInitialized) return;
    LOG(("SearchFriendsThread started."));
	EnterCriticalSection(&QueryThreadMutex);
	SkypeInitialized=FALSE;
	SearchFriends();
	SkypeInitialized=TRUE;
	LeaveCriticalSection(&QueryThreadMutex);
    LOG(("SearchFriendsThread terminated gracefully."));
}

void __cdecl SkypeSystemInit(char *dummy) {
	static BOOL Initializing=FALSE;

    LOG (("SkypeSystemInit thread started."));
	if (SkypeInitialized || Initializing) return;
	Initializing=TRUE;
// Do initial Skype-Tasks
	logoff_contacts();
// Add friends

	if (SkypeSend(SKYPE_PROTO)==-1 || !testfor("PROTOCOL", INFINITE) ||
		SkypeSend("GET PRIVILEGE SKYPEOUT")==-1) {
		Initializing=FALSE;
        LOG (("SkypeSystemInit thread stopped with failure."));
		return;	
	}

#ifdef SKYPEBUG_OFFLN
    if (!ResetEvent(GotUserstatus) || SkypeSend("GET USERSTATUS")==-1 || 
		WaitForSingleObject(GotUserstatus, INFINITE)==WAIT_FAILED) 
	{
        LOG (("SkypeSystemInit thread stopped with failure."));
		Initializing=FALSE;
		return;
	}
	if (SkypeStatus!=ID_STATUS_OFFLINE)
#endif
	if (SearchFriends()==-1) {
        LOG (("SkypeSystemInit thread stopped with failure."));
		Initializing=FALSE;
		return;	
	}
	if (protocol>=5) SearchUsersWaitingMyAuthorization();
	SkypeSend("SEARCH MISSED%sS", cmdMessage);
// Get my Nickname
	if (SkypeSend("GET CURRENTUSERHANDLE")==-1
#ifndef SKYPEBUG_OFFLN
		|| SkypeSend("GET USERSTATUS")==-1
#endif
		) 
	{
        LOG (("SkypeSystemInit thread stopped with failure."));
		Initializing=FALSE;
		return;
	}
	SetTimer (hWnd, 1, PING_INTERVAL, NULL);
	SkypeInitialized=TRUE;
	Initializing=FALSE;
	LOG (("SkypeSystemInit thread terminated gracefully."));
	return;
}

void FirstLaunch(char *dummy) {
	int counter=0;

	LOG (("FirstLaunch thread started."));
	if (!DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "StartSkype", 1) || ConnectToSkypeAPI(skype_path, FALSE)==-1) 
	{
		int oldstatus=SkypeStatus;

		LOG(("OnModulesLoaded starting offline.."));	
		InterlockedExchange((long *)&SkypeStatus, ID_STATUS_OFFLINE);
		ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldstatus, SkypeStatus);
	}
	if (AttachStatus==-1 || AttachStatus==SKYPECONTROLAPI_ATTACH_REFUSED || AttachStatus==SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE) {
		LOG (("FirstLaunch thread stopped because of invalid Attachstatus."));
		return;
	}
	
	// When you launch Skype and Attach is Successfull, it still takes some time
	// until it becomes available for receiving messages.
	// Let's probe this with PINGing
	LOG(("CheckIfApiIsResponding Entering test loop"));
	while (1) {
		LOG(("Test #%d", counter));
		if (SkypeSend("PING")==-1) counter ++; else break;
		if (counter>=20) {
			OUTPUT(_T("Cannot reach Skype API, plugin disfunct."));
			LOG (("FirstLaunch thread stopped: cannot reach Skype API."));
			return;
		}
		Sleep(500);
	}
	LOG(("CheckIfApiIsResponding: Testing for PONG"));
	testfor("PONG", 2000); // Flush PONG from MsgQueue

	pthread_create(( pThreadFunc )SkypeSystemInit, NULL);
	LOG (("FirstLaunch thread terminated gracefully."));
}

int CreateTopToolbarButton(WPARAM wParam, LPARAM lParam) {
	TTBButton ttb={0};
	
	ttb.cbSize = sizeof(ttb);
	ttb.dwFlags = TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP|TTBBF_DRAWBORDER;
	ttb.hbBitmapDown = ttb.hbBitmapUp = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_CALL));
	ttb.pszServiceDown = ttb.pszServiceUp = SKYPEOUT_CALL;
	ttb.name=Translate("Do a SkypeOut-call");
	if ((int)(httbButton=(HANDLE)CallService(MS_TTB_ADDBUTTON, (WPARAM)&ttb, 0))==-1) httbButton=0;
	return 0;
}


int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {
	bModulesLoaded=TRUE;

	logoff_contacts();

	HookEventsLoaded();
	RegisterToUpdate();
	RegisterToDbeditorpp();
	VoiceServiceModulesLoaded();
	GCInit();

	InitializeCriticalSection(&RingAndEndcallMutex);
	InitializeCriticalSection(&QueryThreadMutex);
	
	add_contextmenu(NULL);
	if ( ServiceExists( MS_GC_REGISTER )) 
	{
		GCREGISTER gcr = {0};
		static COLORREF crCols[1] = {0};

		gcr.cbSize = sizeof( GCREGISTER );
		gcr.dwFlags = GC_CHANMGR | GC_TCHAR; // |GC_ACKMSG; // TODO: Not implemented yet
        gcr.ptszModuleDispName = _T(SKYPE_PROTONAME);
		gcr.pszModule = SKYPE_PROTONAME;
		if (CallService(MS_GC_REGISTER, 0, (LPARAM)&gcr)) 
		{
			OUTPUT(_T("Unable to register with Groupchat module!"));
		}
		hInitChat = CreateHookableEvent(SKYPE_PROTONAME"\\ChatInit");
		hEvInitChat = HookEvent(SKYPE_PROTONAME"\\ChatInit", ChatInit);

		hChatEvent = HookEvent(ME_GC_EVENT, GCEventHook);
		hChatMenu = HookEvent(ME_GC_BUILDMENU, GCMenuHook);
        CreateServiceFunction (SKYPE_CHATNEW, SkypeChatCreate);
		CreateServiceFunction (SKYPE_PROTONAME PS_LEAVECHAT, GCOnLeaveChat);
		CreateServiceFunction (SKYPE_PROTONAME PS_JOINCHAT, GCOnJoinChat);
	}
	// Try folder service first
	hProtocolAvatarsFolder = NULL;
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		mir_snprintf(DefaultAvatarsFolder, sizeof(DefaultAvatarsFolder), "%s\\%s", PROFILE_PATH, SKYPE_PROTONAME);
		hProtocolAvatarsFolder = (HANDLE) FoldersRegisterCustomPath(SKYPE_PROTONAME, "Avatars Cache", DefaultAvatarsFolder);
	}
	
	if (hProtocolAvatarsFolder == NULL)
	{
		// Use defaults
		CallService(MS_DB_GETPROFILEPATH, (WPARAM) MAX_PATH, (LPARAM) DefaultAvatarsFolder);
		mir_snprintf(DefaultAvatarsFolder, sizeof(DefaultAvatarsFolder), "%s\\%s", DefaultAvatarsFolder, SKYPE_PROTONAME);
		CreateDirectoryA(DefaultAvatarsFolder, NULL);
	}

	pthread_create(( pThreadFunc )FirstLaunch, NULL);
	return 0;
}

void FetchMessageThread(fetchmsg_arg *pargs) {
	char str[64], *ptr, *who, *msg, *msgptr, *pszProp, *pszEnd, *chat, *msg_emoted = NULL;
	int direction=0, msglen = 0;
	DWORD timestamp = 0, lwr=0;
    CCSDATA ccs={0};
    PROTORECVEVENT pre={0};
    HANDLE hContact = NULL, hDbEvent;
	DBEVENTINFO dbei={0};
	DBVARIANT dbv={0};
	fetchmsg_arg args;
	BOOL bEmoted=FALSE, isGroupChat=FALSE, bHasPartList=FALSE;
	BOOL bUseGroupChat = DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseGroupchat", 0);

	if (!pargs) return;
	args = *pargs;
	free (pargs);
	Sleep(200);
	
	pszProp = str + sprintf(str, "GET %s %s ", cmdMessage, args.msgnum);
	pre.lParam = strtoul(args.msgnum, NULL, 10);
	//if (args.bIsRead) pre.flags |= PREF_CREATEREAD;
	//pEvent = MsgList_FindMessage(pre.lParam);
	
	// Was it a message?
	strcpy(pszProp, "TYPE");
	LOG(("FetchMessageThread: Get the TYPE %s", str));
	if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
		return;
	}
	LOG(("FetchMessageThread: %s", ptr));
	if (strncmp(ptr, "ERROR", 5)) {	
		pszEnd = ptr+strlen(ptr);
		if( !strncmp(pszEnd-6, "EMOTED", 6) ) bEmoted = TRUE;
		if( !strncmp(pszEnd-16, "MULTI_SUBSCRIBED", 16) ) isGroupChat = TRUE;

		if (strncmp(pszEnd-4, "TEXT", 4) && strncmp(pszEnd-4, "SAID", 4) && !bEmoted) 
		{
			if (bUseGroupChat) 
			{
				if (!strncmp(pszEnd-10,"SAWMEMBERS", 10) || !strncmp(pszEnd-15, "CREATEDCHATWITH", 15) ||
					!strncmp(pszEnd-10,"ADDEDMEMBERS", 10)) 
				{
					char *ptr2, *pszID;

					// We have a new Groupchat
					LOG(("FetchMessageThread CHAT SAWMEMBERS"));
					free(ptr);
					strcpy(pszProp, "CHATNAME");
					if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) 
						return;
					if (ptr2=SkypeGet("CHAT", (pszID=ptr+strlen(str+4)+1), "STATUS"))
					{
						// IF not MULTI_SUBSCRIBED, we can ignore it
						if (!strcmp(ptr2, "MULTI_SUBSCRIBED"))
						{
							ChatStart(pszID);
						}
						free (ptr2);
					}
					free(ptr);
					return;
				}
				if (!strncmp(pszEnd-8,"SETTOPIC", 8)) 
				{
					GCDEST gcd = {0};
					GCEVENT gce = {0};
					char *ptr2;

					LOG(("FetchMessageThread CHAT SETTOPIC"));
					free(ptr);
					strcpy (pszProp, "CHATNAME");
					if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) 
						return;

					gce.cbSize = sizeof(GCEVENT);
					gcd.pszModule = SKYPE_PROTONAME;
					gcd.pszID = ptr+strlen(str+4)+1;
					gcd.iType = GC_EVENT_TOPIC;
					gce.pDest = &gcd;

					if (ptr2=SkypeGet("CHAT", gcd.pszID, "STATUS"))
					{
						// IF not MULTI_SUBSCRIBED, we can ignore it, but this shouldn't happen
						if (!strcmp(ptr2, "MULTI_SUBSCRIBED"))
						{
							free (ptr2);
							sprintf (pszProp, "%s_HANDLE", cmdPartner);
							if (SkypeSend(str)!=-1 && (who=SkypeRcv(str+4, INFINITE))) 
							{
								gce.pszUID = who+strlen(str+4)+1;
								strcpy (pszProp, "BODY");
								if (SkypeSend(str)!=-1 && (ptr2=SkypeRcv(str+4, INFINITE))) 
								{
									gce.pszText = ptr2+strlen(str+4)+1;
#ifdef _UNICODE
									gcd.ptszID = make_unicode_string(gcd.pszID);
									gce.ptszUID = make_unicode_string(gce.pszUID);
									gce.ptszText = make_unicode_string(gce.pszText);
									gce.dwFlags = GC_TCHAR;
#else
									utf8_decode (gce.pszText, (char**)&gce.ptszText);
#endif
									CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
									free(ptr2);
#ifdef _UNICODE
									free (gcd.ptszID);
									free ((void*)gce.ptszUID);
#endif
									free ((void*)gce.ptszText);

								}
								free(who);
							}
						} else free(ptr2);
					}
					free(ptr);
					return;
				}
				if (!strncmp(pszEnd-4,"LEFT", 4) || !strncmp(pszEnd-12,"ADDEDMEMBERS", 12)) 
				{

					LOG(("FetchMessageThread CHAT LEFT or ADDEDMEMBERS"));
					free(ptr);
					strcpy(pszProp, "CHATNAME");
					if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) 
						return;
					SkypeSend ("GET CHAT %s MEMBERS", ptr+strlen(str+4)+1);
					free(ptr);
					return;
				}
			}
			// Boo,no useful message, ignore it
			free(ptr);
			return;
		}
		free(ptr);
	}

	// Timestamp
	if (!args.pMsgEntry || !args.pMsgEntry->tEdited) {
		strcpy(pszProp, "TIMESTAMP");
		if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE)))
			return;
		if (strncmp(ptr, "ERROR", 5))
			timestamp=atol(ptr+strlen(str+4)+1);
		free(ptr);
	} else timestamp=args.pMsgEntry->tEdited;

	if (args.getstatus) {
		strcpy(pszProp, "STATUS");
		if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE)))
			return;
		if (strncmp(ptr, "ERROR", 5) &&
			!strncmp(ptr+strlen(str+4)+1, "SENT", 4)) direction=DBEF_SENT;
		free(ptr);
	}

	// Text which was sent (on edited msg, BODY may already be in queue, check)
	strcpy(pszProp, "BODY");
	if (!args.pMsgEntry || !args.pMsgEntry->tEdited || !(ptr=SkypeRcv(str+4, 1000)))
	{
		if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE)))
			return;
	}
	if (strncmp(ptr, "ERROR", 5)) {
		msgptr = ptr+strlen(str+4)+1;
		if (args.pMsgEntry && args.pMsgEntry->tEdited) {
			// Mark the message as edited
			if (!*msgptr && args.pMsgEntry->hEvent != INVALID_HANDLE_VALUE) {
				// Empty message and edited -> Delete event
				if ((int)(hContact = (HANDLE)CallService (MS_DB_EVENT_GETCONTACT, (WPARAM)args.pMsgEntry->hEvent, 0)) != -1) {
					CallService (MS_DB_EVENT_DELETE, (WPARAM)hContact, (LPARAM)args.pMsgEntry->hEvent);
					free (ptr);
					return;
				}
			} else {
				msgptr-=9;
				memcpy (msgptr, "[EDITED] ", 9);
			}
		}
		bHasPartList = strncmp(msgptr,"<partlist ",10)==0;

		if (mirandaVersion >= 0x070000 &&	// 0.7.0+ supports PREF_UTF flag, no need to decode UTF8
			!bUseGroupChat) {				// I guess Groupchat doesn't support UTF8?
			msg = ptr;
			pre.flags |= PREF_UTF;
		} else {	// Older version has to decode either UTF8->ANSI or UTF8->UNICODE
			// This could be replaced by mir_getUTFI - functions for Miranda 0.5+ builds, but we stay
			//´0.4 compatible for backwards compatibility. Unfortunately this requires us to link with utf8.c
			if (utf8_decode(msgptr, &msg)==-1) {
				free(ptr);
				return;
			}
#ifdef _UNICODE
			msglen = strlen(msg)+1;
			msg=realloc(msg, msglen+sizeof(WCHAR)*msglen);
			msgptr = (char*)make_unicode_string (msgptr);
			memcpy (msg+msglen, msgptr, msglen*sizeof(WCHAR));
			free(msgptr);
			pre.flags |= PREF_UNICODE;
#endif
			msgptr = msg;
			free (ptr);
		}
		msglen = strlen(msgptr)+1;
	} else {
		free (ptr);
		return;
	}

	// skype sends some xml statics after a call has finished. Check if thats the case and suppress it if necessary...
	if ((DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "SuppressCallSummaryMessage", 1) && 
		bHasPartList) || msgptr[0]==0) {
		free (msg);
		return;
	}

	// Who sent it?
	sprintf (pszProp, "%s_HANDLE", cmdPartner);
	if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE)))
		return;
	if (strncmp(ptr, "ERROR", 5)) {
		who=memmove (ptr, ptr+strlen(str+4)+1, strlen(ptr+strlen(str+4)));
	} else {
		OUTPUTA(ptr);
		free(ptr);
		free(msg);
		LOG(("FetchMessageThread terminated: Error - Cannot get Userhandle"));
		return;
	}


	// Aaaand add it..
	LOG(("FetchMessageThread Finding contact handle"));
	DBGetContactSettingString(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv);
	if (dbv.pszVal && !strcmp (who, dbv.pszVal))
	{
		char *pTok, *chat;

		// It's from me.. But to whom?
		free (who);
		// CHATMESSAGE .. USERS doesn't return anything, so we have to query the CHAT-Object
		strcpy(pszProp, "CHATNAME");
		if (SkypeSend(str)==-1 || !(chat=SkypeRcv(str+4, INFINITE))) {
			free (msg);
			DBFreeVariant (&dbv);
			return;
		}
		ptr=SkypeGet ("CHAT", chat+strlen(str+4)+1, "ACTIVEMEMBERS");
		free (chat);
		if (!ptr) {
			free(msg);
			DBFreeVariant (&dbv);
			return; // Dunno, skip that msg
		}

		for (pTok = strtok (ptr, " "); pTok; pTok=strtok(NULL, " ")) {
			if (strcmp (pTok, dbv.pszVal)) break; // Take the first dude in the list who is not me
		}

		if (!pTok) {
			free (ptr);
			free(msg);
			DBFreeVariant (&dbv);
			return; // We failed
		}
		who=memmove (ptr, pTok, strlen(pTok)+1);
		direction = DBEF_SENT;
	}
	DBFreeVariant (&dbv);

	if (!(hContact=find_contact(who))) {
		// Permanent adding of user obsolete, we use the BUDDYSTATUS now (bug #0000005)
		ResetEvent(hBuddyAdded);
		SkypeSend("GET USER %s BUDDYSTATUS", who);
		WaitForSingleObject(hBuddyAdded, INFINITE);
		if (!(hContact=find_contact(who))) {
			// Arrgh, the user has been deleted from contact list.
			// In this case, we add him temp. to receive the msg at least.
			hContact=add_contact(who, PALF_TEMPORARY);			
		}
	}

	LOG(("FetchMessageThread Check if message if a groupchat or not"));
	// Is it a groupchat message?
	strcpy (pszProp, "CHATNAME");
	LOG(("FetchMessageThread Request the CHATNAME"));
	if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
		free(who);
		free(msg);
		return;
	}
	if (strncmp(ptr, "ERROR", 5)) {
		GCDEST gcd = {0};
		HANDLE hChat;

		chat=memmove(ptr, ptr+strlen(str+4)+1, strlen(ptr+strlen(str+4)));
#ifdef _UNICODE
		gcd.ptszID = make_unicode_string(chat);
#else
		gcd.ptszID = chat;
#endif
		LOG(("FetchMessageThread Request the CHAT STATUS"));
		if (!(ptr=SkypeGet("CHAT", chat, "STATUS"))) {
			free(who);
			free(msg);
			return;
		}

		LOG(("FetchMessageThread Compare the STATUS (%s) to MULTI_SUBSCRIBED", ptr));
		if (!strcmp(ptr, "MULTI_SUBSCRIBED")) isGroupChat = TRUE;
		if (hChat = find_chat(gcd.ptszID)) isGroupChat = TRUE;
		free(ptr);

		if (bUseGroupChat) {

			LOG(("FetchMessageThread Using groupchat option is checked"));
			
			if (isGroupChat) {

				GCEVENT gce = {0};
				DBVARIANT dbv = {0};
				HANDLE hContact;
				CONTACTINFO ci = {0};

				if (!hChat) ChatStart(chat);
				LOG(("FetchMessageThread This is a group chat message"));
				gcd.pszModule = SKYPE_PROTONAME;
				gcd.iType = GC_EVENT_MESSAGE;
				gce.cbSize = sizeof(GCEVENT);
				gce.pDest = &gcd;
				if ((gce.bIsMe = (direction&DBEF_SENT)?TRUE:FALSE) &&
					DBGetContactSettingString(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv)==0)
				{
					free(who);
					who = strdup(dbv.pszVal);
					DBFreeVariant(&dbv);
				}
#ifdef _UNICODE
				gce.ptszUID = make_unicode_string(who);
#else
				gce.ptszUID = who;
#endif
				ci.cbSize = sizeof(ci);
				ci.szProto = SKYPE_PROTONAME;
				ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
				gce.ptszNick=gce.ptszUID;
				if (!gce.bIsMe && (hContact=find_contact(who)))
				{
					ci.hContact = hContact;
					if (!CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) gce.ptszNick=ci.pszVal; 
				}
				gce.time = timestamp>0?timestamp:time(NULL);
				

				gce.pszText = msgptr;
				if (pre.flags & PREF_UNICODE) gce.pszText += msglen;
				gce.dwFlags = GCEF_ADDTOLOG | GC_TCHAR;
				CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
				MsgList_Add (pre.lParam, INVALID_HANDLE_VALUE);	// Mark as groupchat
				if (dbv.pszVal) DBFreeVariant(&dbv);
				if (ci.pszVal) miranda_sys_free (ci.pszVal);
#ifdef _UNICODE
				free((void*)gce.ptszUID);
				free(gcd.ptszID);
#endif
				free(chat);
				free(who);
				free(msg);

				// Yes, we have successfully read the msg
				str[0]='S';	// SET, not GET
				strcpy (pszProp, "SEEN");
				SkypeSend(str);
				return;
			}
		}
		free(chat);
#ifdef _UNICODE
		free(gcd.ptszID);
#endif
	} else free(ptr);

	if (QueryMsgDirection || (direction&DBEF_SENT)) {
		// Check if the timestamp is valid
		LOG(("FetchMessageThread Checking timestamps"));
		dbei.cbSize=sizeof(dbei);
		dbei.cbBlob=0;
		if (hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDFIRST,(WPARAM)hContact,0)) {
			CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei);
			lwr=dbei.timestamp;
		}
		dbei.cbSize=sizeof(dbei);
		dbei.cbBlob=0;
		dbei.timestamp=0;
		if (hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDLAST,(WPARAM)hContact,0))
			CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei);
		if (timestamp<lwr || (direction&DBEF_SENT)) {
			LOG(("FetchMessageThread Adding event"));
			dbei.szModule=SKYPE_PROTONAME;
			dbei.cbBlob=msglen;
			if (pre.flags & PREF_UNICODE)
				dbei.cbBlob += sizeof(WCHAR)*( (DWORD)wcslen((WCHAR*)&msgptr[dbei.cbBlob])+1);
			dbei.pBlob=(PBYTE)msgptr;
			dbei.timestamp=timestamp>0?timestamp:time(NULL);
			dbei.flags=direction;
			if (pre.flags & PREF_CREATEREAD) dbei.flags|=DBEF_READ;
			if (pre.flags & PREF_UTF) dbei.flags|=DBEF_UTF;
			dbei.eventType=EVENTTYPE_MESSAGE;
			MsgList_Add (pre.lParam, (HANDLE)CallServiceSync(MS_DB_EVENT_ADD, (WPARAM)(HANDLE)hContact, (LPARAM)&dbei));
		}
	}


	if (!(direction&DBEF_SENT) && (!QueryMsgDirection || (QueryMsgDirection && timestamp>dbei.timestamp))) {
		LOG(("FetchMessageThread Normal message add..."));
		// Normal message received, process it
	    ccs.szProtoService = PSR_MESSAGE;
	    ccs.hContact = hContact;
		ccs.wParam = 0;
		ccs.lParam = (LPARAM)&pre;
		pre.flags |= direction;
	
		if(isGroupChat && DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "MarkGroupchatRead", 0))
			pre.flags |= PREF_CREATEREAD;

		pre.timestamp = timestamp>0?timestamp:time(NULL);
		if( bEmoted )
		{
			CONTACTINFO ci = {0};
			int newlen;
			char *pMsg;
			ci.cbSize = sizeof(ci);
			ci.szProto = SKYPE_PROTONAME;
			ci.dwFlag = CNF_DISPLAY;
			ci.hContact = hContact;
			CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci);
			newlen = msglen + (ci.pszVal?strlen((char*)ci.pszVal):0) + 8;
			if (pre.flags & PREF_UNICODE) newlen *= (sizeof(WCHAR)+1);
			msg_emoted = malloc(newlen);
			pMsg = msg_emoted + sprintf (msg_emoted, "** %s%s%s **",
				(ci.pszVal?(char*)ci.pszVal:""),(ci.pszVal?" ":""),(char*)msgptr) + 1;
			if (pre.flags & PREF_UNICODE) {
				ci.dwFlag |= CNF_UNICODE;
				CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci);
				swprintf ((WCHAR*)pMsg, L"** %s%s%s **",(ci.pszVal?(WCHAR*)ci.pszVal:L""),(ci.pszVal?L" ":L""),msgptr+msglen);
			}
			pre.szMessage = msg_emoted;
			if (ci.pszVal) miranda_sys_free (ci.pszVal);
		}
		else
		{
			pre.szMessage = msgptr;
		}
		CallServiceSync(MS_PROTO_CHAINRECV, 0, (LPARAM) &ccs);
		if (msg_emoted) free (msg_emoted);

		// Yes, we have successfully read the msg
		str[0]='S';	// SET, not GET
		strcpy (pszProp, "SEEN");
		SkypeSend(str);
		// MSG STATUS read is already handled and discarded by our MSG-handling routine,
		// so no need to check here
	}
	free(who);
	free(msg);
}

void FetchMessageThreadSync(fetchmsg_arg *pargs) {
	// Secure this thread with a mutex.
	// This is needed to ensure that we get called after an old msg in the queue has
	// been added so that MsgList_FindEntry will find it.
	WaitForSingleObject (FetchMessageEvent, 30000);	// Wait max. 30 sec. for previous message fetch to complete
	if ((pargs->pMsgEntry = MsgList_FindMessage(strtoul(pargs->msgnum, NULL, 10))) && !pargs->pMsgEntry->tEdited) {
		// Better don't do this, as we set the msg as read and with this code, we would 
		// mark messages not opened by user as read which isn't that good
		/*
		if (pargs->bIsRead && pMsgEvent->hEvent != INVALID_HANDLE_VALUE)
		{
			HANDLE hContact;
			if ((int)(hContact = (HANDLE)CallService (MS_DB_EVENT_GETCONTACT, (WPARAM)pMsgEntry->hEvent, 0)) != -1)
				CallService (MS_DB_EVENT_MARKREAD, (WPARAM)hContact, (LPARAM)hDBEvent);
		}
		*/
		free (pargs);
	}
	else FetchMessageThread (pargs);
	SetEvent (FetchMessageEvent);
}

void MessageListProcessingThread(char *str) {
	char *token;
	fetchmsg_arg *args;

	for ((token=strtok(str, ",")); token; token=strtok(NULL, ",")) {
		if (args=calloc(1, sizeof(fetchmsg_arg)))
		{
			strncpy (args->msgnum, token+1, sizeof(args->msgnum));
			args->getstatus=TRUE;
			args->bIsRead=TRUE;
			FetchMessageThread (args);
		}
	}
	QueryMsgDirection=FALSE;
	if (ImportingHistory) {
		OUTPUT(_T("History import complete!"));
		ImportingHistory=FALSE;
	}
	free (str);
}

char *GetCallerHandle(char *szSkypeMsg) {
	return SkypeGet(szSkypeMsg, "PARTNER_HANDLE", "");
}


HANDLE GetCallerContact(char *szSkypeMsg) {
	char *szHandle;
	HANDLE hContact=NULL;

	if (!(szHandle=GetCallerHandle(szSkypeMsg))) return NULL;
	if (!(hContact=find_contact(szHandle))) {
		// If it's a SkypeOut-contact, PARTNER_HANDLE = SkypeOUT number
		DBVARIANT dbv;
		int tCompareResult;

		for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
			if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, "SkypeOutNr", &dbv)) continue;
			tCompareResult = strcmp(dbv.pszVal, szHandle);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue; else break;
		}
	}
	free(szHandle);
	if (!hContact) {LOG(("GetCallerContact Not found!"));}
	return hContact;
}

LRESULT CALLBACK InCallPopUpProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	switch(msg)
	{
		case WM_COMMAND:
			break;

		case WM_CONTEXTMENU:
			SendMessage(hwnd,UM_DESTROYPOPUP,0,0);
			break;			
		case UM_FREEPLUGINDATA:
			//Here we'd free our own data, if we had it.
			return FALSE;
		case UM_INITPOPUP:
			break;
		case UM_DESTROYPOPUP:
			break;
		case WM_NOTIFY:
		default:
			break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

void RingThread(char *szSkypeMsg) {
	HANDLE hContact;
	DBEVENTINFO dbei={0};
	DBVARIANT dbv;
	char *ptr = NULL;

	// We use a single critical section for the RingThread- and the EndCallThread-functions
	// so that only one function is running at the same time. This is needed, because when
	// a initated and unaccepted call (which is still ringing) is hangup/canceled, skype
	// sends two messages. First "CALL xxx STATUS RINGING" .. second "CALL xx STATUS CANCELED".
	// This starts two independend threads (first: RingThread; second: EndCallThread). Now 
	// the two message are processed in reverse order sometimes. This causes the EndCallThread to
	// delete the contacts "CallId" property and after that the RingThread saves the contacts 
	// "CallId" again. After that its not possible to call this contact, because the plugin
	// thinks that there is already a call going and the hangup-function isnt working, because 
	// skype doesnt accept status-changes for finished calls. The CriticalSection syncronizes
	// the threads and the messages are processed in correct order. 
	// Not the best solution, but it works.
	EnterCriticalSection (&RingAndEndcallMutex);

  LOG(("RingThread started."));
	if (protocol >= 5) SkypeSend ("MINIMIZE");
	if (hContact=GetCallerContact(szSkypeMsg)) {
		// Make sure that an answering thread is not already in progress so that we don't get
		// the 'Incoming call' event twice
		if (!DBGetContactSettingString(hContact, SKYPE_PROTONAME, "CallId", &dbv)) {
			DBFreeVariant(&dbv);
            LOG(("RingThread terminated."));
			goto l_exitRT;
		}
		DBWriteContactSettingString(hContact, SKYPE_PROTONAME, "CallId", szSkypeMsg);
	}
	
	if (!(ptr=SkypeGet(szSkypeMsg, "TYPE", ""))) {
        LOG(("RingThread terminated."));
		goto l_exitRT;;
	}

	if (!strncmp(ptr, "INCOMING", 8))
		NofifyVoiceService(hContact, szSkypeMsg, VOICE_STATE_RINGING);
	else
		NofifyVoiceService(hContact, szSkypeMsg, VOICE_STATE_CALLING);

	if (!strncmp(ptr, "INCOMING", 8)) {
		if (!hContact) {
			char *szHandle;
			
			if (szHandle=GetCallerHandle(szSkypeMsg)) {
				if (!(hContact=add_contact(szHandle, PALF_TEMPORARY))) {
					free(szHandle);
					goto l_exitRT;
				}
				DBDeleteContactSetting(hContact, "CList", "Hidden");
				DBWriteContactSettingWord(hContact, SKYPE_PROTONAME, "Status", (WORD)SkypeStatusToMiranda("SKYPEOUT"));
				DBWriteContactSettingString(hContact, SKYPE_PROTONAME, "SkypeOutNr", szHandle);
				free(szHandle);
			} else goto l_exitRT;
		}
	}

	if (HasVoiceService()) {
		// Voice service will handle it
		goto l_exitRT;
	}

	dbei.cbSize=sizeof(dbei);
	dbei.eventType=EVENTTYPE_CALL;
	dbei.szModule=SKYPE_PROTONAME;
	dbei.timestamp=time(NULL);
	dbei.pBlob=(unsigned char*)Translate("Phonecall");
	dbei.cbBlob=strlen((const char*)dbei.pBlob)+1;
	if (!strncmp(ptr, "INCOMING", 8)) 
	{
		CLISTEVENT cle={0};
		char toolTip[256];

		if(ServiceExists(MS_POPUP_ADDPOPUPEX)) 
		{
			BOOL showPopup, popupWindowColor;
			unsigned int popupBackColor, popupTextColor;
			int popupTimeSec;
			POPUPDATAT InCallPopup;
			TCHAR * lpzContactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,GCDNF_TCHAR);

			popupTimeSec = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupTimeSec", 4);
			popupTextColor = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupTextColor", GetSysColor(COLOR_WINDOWTEXT));
			popupBackColor = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupBackColor", GetSysColor(COLOR_BTNFACE));
			popupWindowColor = (0 != DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "popupWindowColor", TRUE));
			showPopup = (0 != DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "showPopup", TRUE));

			InCallPopup.lchContact = hContact;
			InCallPopup.lchIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALL));
			InCallPopup.colorBack = ! popupWindowColor ? popupBackColor : GetSysColor(COLOR_BTNFACE);
			InCallPopup.colorText = ! popupWindowColor ? popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
			InCallPopup.iSeconds = popupTimeSec;
			InCallPopup.PluginWindowProc = (WNDPROC)InCallPopUpProc;
			InCallPopup.PluginData = (void *)1;
			
			lstrcpy(InCallPopup.lptzText, TranslateT("Incoming Skype Call"));

			lstrcpy(InCallPopup.lptzContactName, lpzContactName);

			if(showPopup)
				CallService(MS_POPUP_ADDPOPUPT,(WPARAM)&InCallPopup,0);

		}
		cle.cbSize=sizeof(cle);
		cle.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALL));
		cle.pszService=SKYPE_ANSWERCALL;
		dbei.flags=DBEF_READ;
		cle.hContact=hContact;
		cle.hDbEvent=(HANDLE)CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&dbei);
		_snprintf(toolTip,sizeof(toolTip),Translate("Incoming call from %s"),(char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0));
		cle.pszTooltip=toolTip;
		CallServiceSync(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
	} 
	else 
	{
		dbei.flags=DBEF_SENT;
		CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&dbei);
	}

l_exitRT:
	if (ptr) free (ptr);
	free(szSkypeMsg);
	LeaveCriticalSection (&RingAndEndcallMutex);
}

void EndCallThread(char *szSkypeMsg) {
	HANDLE hContact=NULL, hDbEvent;
	DBEVENTINFO dbei={0};
	DBVARIANT dbv;
	int tCompareResult;

	// We use a single critical section for the RingThread- and the EndCallThread-functions
	// so that only one function is running at the same time. This is needed, because when
	// a initated and unaccepted call (which is still ringing) is hangup/canceled, skype
	// sends two messages. First "CALL xxx STATUS RINGING" .. second "CALL xx STATUS CANCELED".
	// This starts two independend threads (first: RingThread; second: EndCallThread). Now 
	// the two message are processed in reverse order sometimes. This causes the EndCallThread to
	// delete the contacts "CallId" property and after that the RingThread saves the contacts 
	// "CallId" again. After that its not possible to call this contact, because the plugin
	// thinks that there is already a call going and the hangup-function isnt working, because 
	// skype doesnt accept status-changes for finished calls. The CriticalSection syncronizes
	// the threads and the messages are processed in correct order. 
	// Not the best solution, but it works.
	EnterCriticalSection (&RingAndEndcallMutex);

  LOG(("EndCallThread started."));
	if (szSkypeMsg) {
		for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
			if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, "CallId", &dbv)) continue;
			tCompareResult = strcmp(dbv.pszVal, szSkypeMsg);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue; else break;
		}
	}
	if (hContact)
	{
		NofifyVoiceService(hContact, szSkypeMsg, VOICE_STATE_ENDED);

		DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "CallId");

		if (!HasVoiceService()) {
			dbei.cbSize=sizeof(dbei);
			hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDFIRSTUNREAD,(WPARAM)hContact,0);
			while(hDbEvent) {
				dbei.cbBlob=0;
				CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei);
					if (!(dbei.flags&(DBEF_SENT|DBEF_READ)) && dbei.eventType==EVENTTYPE_CALL) {
					CallService(MS_DB_EVENT_MARKREAD,(WPARAM)hContact,(LPARAM)hDbEvent);
					CallService(MS_CLIST_REMOVEEVENT,(WPARAM)hContact,(LPARAM)hDbEvent);
				}
				if (dbei.pBlob) free(dbei.pBlob);
				hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0);
			}
		}

		if (!DBGetContactSettingString(hContact, SKYPE_PROTONAME, "SkypeOutNr", &dbv)) {
			DBFreeVariant(&dbv);
			if (!strcmp((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0), SKYPE_PROTONAME) && 
				DBGetContactSettingByte(hContact, "CList", "NotOnList", 0)
			   )
					CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
		}
	}
	free(szSkypeMsg);
	LeaveCriticalSection (&RingAndEndcallMutex);
}

void HoldCallThread(char *szSkypeMsg) {
	HANDLE hContact;

	LOG(("HoldCallThread started"));
    if (!szSkypeMsg) {
		LOG(("HoldCallThread terminated."));
		return;
	}
	if (hContact=GetCallerContact(szSkypeMsg)) {
		DBWriteContactSettingByte(hContact, SKYPE_PROTONAME, "OnHold", 1);
		NofifyVoiceService(hContact, szSkypeMsg, VOICE_STATE_ON_HOLD);
	}
	free(szSkypeMsg);
	LOG(("HoldCallThread terminated gracefully"));
}

void ResumeCallThread(char *szSkypeMsg) {
	HANDLE hContact;

	LOG(("ResumeCallThread started"));
	if (!szSkypeMsg) {
		LOG(("ResumeCallThread terminated."));
		return;
	}
	if (hContact=GetCallerContact(szSkypeMsg)) {
		DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "OnHold");
		NofifyVoiceService(hContact, szSkypeMsg, VOICE_STATE_TALKING);
	}
	free(szSkypeMsg);
    LOG(("ResumeCallThread terminated gracefully."));
}

int SetUserStatus(void) {
   if (RequestedStatus && AttachStatus!=-1) {
	if (SkypeSend("SET USERSTATUS %s", RequestedStatus)==-1) return 1;
   }
   return 0;
}

void LaunchSkypeAndSetStatusThread(void *newStatus) {

/*	   if (!DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UnloadOnOffline", 0)) {
		   logoff_contacts();
		   return 1;
	   }
*/
	int oldStatus=SkypeStatus;

	LOG (("LaunchSkypeAndSetStatusThread started."));
	InterlockedExchange((long *)&SkypeStatus, (int)ID_STATUS_CONNECTING);
	ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, SkypeStatus);
	   
	if (ConnectToSkypeAPI(skype_path, TRUE)!=-1) {
		pthread_create(( pThreadFunc )SkypeSystemInit, NULL);
		InterlockedExchange((long *)&SkypeStatus, (int)newStatus);
		ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, SkypeStatus);
		SetUserStatus();
	}

	LOG (("LaunchSkypeAndSetStatusThread terminated gracefully."));
}

LONG APIENTRY WndProc(HWND hWndDlg, UINT message, UINT wParam, LONG lParam) 
{ 
    PCOPYDATASTRUCT CopyData; 
	char *ptr, *szSkypeMsg=NULL, *nick, *buf;
	static char *onlinestatus=NULL;
	static BOOL RestoreUserStatus=FALSE;
	int sstat, oldstatus, flag;
	HANDLE hContact;
	fetchmsg_arg *args;

    switch (message) 
    { 
        case WM_COPYDATA: 
		 LOG(("WM_COPYDATA start"));
		 if(hSkypeWnd==(HWND)wParam) { 
			CopyData=(PCOPYDATASTRUCT)lParam;
			szSkypeMsg=_strdup((char*)CopyData->lpData);
			ReplyMessage(1);
			LOG(("< %s", szSkypeMsg));

 			if (!strncmp(szSkypeMsg, "CONNSTATUS", 10)) {
				if (!strncmp(szSkypeMsg+11, "LOGGEDOUT", 9)) {
					SkypeInitialized=FALSE;
					ResetEvent(SkypeReady);
					AttachStatus=-1;
					sstat=ID_STATUS_OFFLINE;
				    if (hWnd) KillTimer (hWnd, 1);
					logoff_contacts();
				} else 
					sstat=SkypeStatusToMiranda(szSkypeMsg+11);

				if (sstat) {
					oldstatus=SkypeStatus;
					InterlockedExchange((long*)&SkypeStatus, sstat);
					ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldstatus, SkypeStatus);
					if (sstat!=ID_STATUS_OFFLINE) {
						if (sstat!=ID_STATUS_CONNECTING && (oldstatus==ID_STATUS_OFFLINE || oldstatus==ID_STATUS_CONNECTING)) {

							SkypeInitialized=FALSE;
							pthread_create(( pThreadFunc )SkypeSystemInit, NULL);
						}
						if (DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "KeepState", 0)) RestoreUserStatus=TRUE;
					}

//					if (SkypeStatus==ID_STATUS_ONLINE) SkypeSend("SEARCH MISSEDMESSAGES");
				}
//				break;
			}
			if (!strncmp(szSkypeMsg, "USERSTATUS", 10)) {
//				if ((sstat=SkypeStatusToMiranda(szSkypeMsg+11)) && SkypeStatus!=ID_STATUS_CONNECTING) {
				if ((sstat=SkypeStatusToMiranda(szSkypeMsg+11))) {				
						if (RestoreUserStatus && RequestedStatus) {
							RestoreUserStatus=FALSE;
							SkypeSend ("SET USERSTATUS %s", RequestedStatus);
						}
						oldstatus=SkypeStatus;
						InterlockedExchange((long*)&SkypeStatus, sstat);
						ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldstatus, sstat);
#ifdef SKYPEBUG_OFFLN
						if ((oldstatus==ID_STATUS_OFFLINE || oldstatus==ID_STATUS_CONNECTING) && 
							SkypeStatus!=ID_STATUS_CONNECTING && SkypeStatus!=ID_STATUS_OFFLINE) 
							pthread_create(( pThreadFunc )SearchFriendsThread, NULL);
#endif
				}
#ifdef SKYPEBUG_OFFLN
				SetEvent(GotUserstatus);
#endif
				break;
			} 
			if (!strncmp(szSkypeMsg, "USER ", 5)) {
				buf=_strdup(szSkypeMsg+5);
				nick=strtok(buf, " ");
				ptr=strtok(NULL, " ");

				if (strcmp(ptr, "BUDDYSTATUS")) {
					if (!(hContact=find_contact(nick)) && strcmp(ptr, "FULLNAME")) {
						SkypeSend("GET USER %s BUDDYSTATUS", nick);
						free (buf);
						break;
					} 

					if (!strcmp(ptr, "ONLINESTATUS")) {
						if (SkypeStatus==ID_STATUS_OFFLINE)
						{
							free (buf);
							break;
						}
						DBWriteContactSettingWord(hContact, SKYPE_PROTONAME, "Status", (WORD)SkypeStatusToMiranda(ptr+13));
						if((WORD)SkypeStatusToMiranda(ptr+13) != ID_STATUS_OFFLINE)
						{
							LOG(("WndProc Status is not offline so get user info"));
							pthread_create(GetInfoThread, hContact);
						}

							
	/*						
							SkypeSend("GET USER %s TIMEZONE", nick);
							SkypeSend("GET USER %s MOOD_TEXT", nick);
							SkypeSend("GET USER %s IS_VIDEO_CAPABLE", nick);
							SkypeSend("GET USER %s AVATAR 1 e:\\skype_%s.jpg", nick, nick);
	*/
	/*						free(buf);
						if (SkypeInitialized==FALSE) { // Prevent flooding on startup
							SkypeMsgAdd(szSkypeMsg);
							ReleaseSemaphore(SkypeMsgReceived, receivers, NULL);
						}
	*/
						free (buf);
						break;
					}


					/* We handle the following properties right here in the wndProc, in case that
					 * Skype protocol broadcasts them to us.
					 *
					 * However, we still let them be added to the Message queue im memory, as they
					 * may get consumed by GetInfoThread.
					 * This is necessary to have a proper error handling in case the property is
					 * not supported (i.e. imo2sproxy).
					 *
					 * If one of the property GETs returns an error, the error-message has to be
					 * removed from the message queue, as the error is the answer to the query.
					 * If we don't remove the ERRORs from the list, another consumer may see the ERROR
					 * as a reply to his query and process it.
					 * In case the SKYPE Protocol really broadcasts one of these messages without being
					 * requested by GetInfoThread (i.e. MOOD_TEXT), the garbage collector will take 
					 * care of them and remove them after some time.
					 * This may not be the most efficient way, but ensures that we finally do proper
					 * error handling.
					 */
					if (!strcmp(ptr, "FULLNAME")) {
						char *utfdstr=NULL, *nm;

						if (nm = strtok(NULL, " "))
						{
							SkypeDBWriteContactSettingUTF8String(hContact, SKYPE_PROTONAME, "FirstName", nm);
							if (!(nm=strtok(NULL, ""))) DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "LastName");
							else 
								SkypeDBWriteContactSettingUTF8String(hContact, SKYPE_PROTONAME, "LastName", nm);
						}
					} else
					if (!strcmp(ptr, "BIRTHDAY")) {
						unsigned int y, m, d;
						if (sscanf(ptr+9, "%04d%02d%02d", &y, &m, &d)==3) {
							DBWriteContactSettingWord(hContact, SKYPE_PROTONAME, "BirthYear", (WORD)y);
							DBWriteContactSettingByte(hContact, SKYPE_PROTONAME, "BirthMonth", (BYTE)m);
							DBWriteContactSettingByte(hContact, SKYPE_PROTONAME, "BirthDay", (BYTE)d);
						} else {
							DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "BirthYear");
							DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "BirthMonth");
							DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "BirthDay");
						}
					} else
					if (!strcmp(ptr, "COUNTRY")) {
						if (ptr[8]) {
							struct CountryListEntry *countries;
							int countryCount, i;

							CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&countryCount, (LPARAM)&countries);
							for (i=0; i<countryCount; i++) {
								if (countries[i].id == 0 || countries[i].id == 0xFFFF) continue;
								if (!_stricmp(countries[i].szName, ptr+8)) 
								{
									DBWriteContactSettingWord(hContact, SKYPE_PROTONAME, "Country", (BYTE)countries[i].id);
									break;
								}
							}
						} else DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "Country");
					} else
					if (!strcmp(ptr, "SEX")) {
						if (ptr[4]) {
							BYTE sex=0;
							if (!_stricmp(ptr+4, "MALE")) sex=0x4D;
							if (!_stricmp(ptr+4, "FEMALE")) sex=0x46;
							if (sex) DBWriteContactSettingByte(hContact, SKYPE_PROTONAME, "Gender", sex);
						} else DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "Gender");
					} else
	/*				if (!strcmp(ptr, "AVATAR" )){
						LOG("WndProc", "AVATAR");
						if (!(hContact=find_contact(nick)))
							SkypeSend("GET USER %s BUDDYSTATUS", nick);
						else
						{
							TCHAR *unicode = NULL;
							
							if(utf8_decode((ptr+9), &Avatar)==-1) break;

							if( ServiceExists(MS_AV_SETAVATAR) )
							{
								CallService(MS_AV_SETAVATAR,(WPARAM) hContact,(LPARAM) Avatar);
							}
							else
							{

								if(DBWriteContactSettingTString(hContact, "ContactPhoto", "File", Avatar)) 
								{
									#if defined( _UNICODE )
										char buff[TEXT_LEN];
										WideCharToMultiByte(code_page, 0, Avatar, -1, buff, TEXT_LEN, 0, 0);
										buff[TEXT_LEN] = 0;
										DBWriteContactSettingString(hContact, "ContactPhoto", "File", buff);
									#endif
								}

							}
														
							
						}
						free(buf);
						break;
					}
					*/
					if (!strcmp(ptr, "MOOD_TEXT")){

						LOG(("WndProc MOOD_TEXT"));
						SkypeDBWriteContactSettingUTF8String (hContact, "CList", "StatusMsg", ptr+10);
					} else
					if (!strcmp(ptr, "TIMEZONE")){
						time_t temp;
						struct tm tms;
						int value=atoi(ptr+9);					

						LOG(("WndProc: TIMEZONE %s", nick));

						if (value && !DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "IgnoreTimeZones", 0)) {
							temp = time(NULL);
							tms = *localtime(&temp);
							//memcpy(&tms,localtime(&temp), sizeof(tm));
							//tms = localtime(&temp)
							timezone=(value >= 86400 )?(256-((2*(atoi(ptr+9)-86400))/3600)):((-2*(atoi(ptr+9)-86400))/3600);
							if (tms.tm_isdst == 1 && DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseTimeZonePatch", 0)) 
							{
								LOG(("WndProc: Using the TimeZonePatch"));
								DBWriteContactSettingByte(hContact, "UserInfo", "Timezone", (BYTE)(timezone+2));
							}
							else
							{
								LOG(("WndProc: Not using the TimeZonePatch"));
								DBWriteContactSettingByte(hContact, "UserInfo", "Timezone", (BYTE)(timezone+0));
							}
						} else 	{
							LOG(("WndProc: Deleting the TimeZone in UserInfo Section"));
							DBDeleteContactSetting(hContact, "UserInfo", "Timezone");
						}
					} else
					if (!strcmp(ptr, "IS_VIDEO_CAPABLE")){
						if (!_stricmp(ptr + 17, "True"))
							DBWriteContactSettingString(hContact, SKYPE_PROTONAME, "MirVer", "Skype 2.0");
						else
							DBWriteContactSettingString(hContact, SKYPE_PROTONAME, "MirVer", "Skype");
					} else
					if (!strcmp(ptr, "RICH_MOOD_TEXT")) {
						DBWriteContactSettingString(hContact, SKYPE_PROTONAME, "MirVer", "Skype 3.0");
					} else
					if (!strcmp(ptr, "DISPLAYNAME")) {
						// Skype Bug? -> If nickname isn't customised in the Skype-App, this won't return anything :-(
						if (ptr[12]) 
							SkypeDBWriteContactSettingUTF8String(hContact, SKYPE_PROTONAME, "Nick", ptr+12);
					} else	// Other proerties that can be directly assigned to a DB-Value
					{
						int i;
						char *pszProp;

						for (i=0; i<sizeof(m_settings)/sizeof(m_settings[0]); i++) {
							if (!strcmp(ptr, m_settings[i].SkypeSetting)) {
								pszProp = ptr+strlen(m_settings[i].SkypeSetting)+1;
								if (*pszProp)
									SkypeDBWriteContactSettingUTF8String(hContact, SKYPE_PROTONAME, m_settings[i].MirandaSetting, pszProp);
								else
									DBDeleteContactSetting(hContact, SKYPE_PROTONAME, m_settings[i].MirandaSetting);
							}
						}
					}
				} else { // BUDDYSTATUS:
					flag=0;
					switch(atoi(ptr+12)) {
						case 1: if (hContact=find_contact(nick)) CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0); break;
						case 0: break;
						case 2: flag=PALF_TEMPORARY;
						case 3: add_contact(nick, flag); 
								SkypeSend("GET USER %s ONLINESTATUS", nick);
								break;
					}
					free(buf);
					if (!SetEvent(hBuddyAdded)) TellError(GetLastError());
					break;
				}
				free(buf);
			}
			if (!strncmp(szSkypeMsg, "CURRENTUSERHANDLE", 17)) {	// My username
				DBWriteContactSettingString(NULL, SKYPE_PROTONAME, SKYPE_NAME, szSkypeMsg+18);
				DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "Nick", szSkypeMsg+18);
				pthread_create(( pThreadFunc )GetDisplaynameThread, NULL);
				break;
			}
			if (strstr(szSkypeMsg, "AUTOAWAY") || !strncmp(szSkypeMsg, "OPEN ",5) ||
				 (SkypeInitialized && !strncmp (szSkypeMsg, "PONG", 4)) ||
				 !strncmp (szSkypeMsg, "MINIMIZE", 8)) 
			{
				// Currently we do not process these messages  
				break;
			}
			if (!strncmp(szSkypeMsg, "CHAT ", 5)) {
				// Currently we only process these notifications
				if (DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseGroupchat", 0)) 
				{
					if (ptr=strstr(szSkypeMsg, " MEMBERS")) {
						LOG(("WndProc AddMembers"));
						AddMembers (szSkypeMsg);
					} else 
					if (ptr=strstr(szSkypeMsg, " FRIENDLYNAME ")) {
						// Chat session name
						HANDLE hContact;

						*ptr=0;
						if (hContact = find_chatA(szSkypeMsg+5))
						{
							GCDEST gcdest = {SKYPE_PROTONAME, szSkypeMsg+5, GC_EVENT_CHANGESESSIONAME};
							GCEVENT gcevent = {sizeof(GCEVENT), &gcdest};
							gcevent.pszText = ptr+14;
							CallService(MS_GC_EVENT, 0, (LPARAM)&gcevent);
							DBWriteContactSettingString (hContact, SKYPE_PROTONAME, "Nick", gcevent.pszText);
						}
						*ptr=' ';
					}
				}
			}
			if (!strncmp(szSkypeMsg, "CALL ",5)) {
				// incoming calls are already processed by Skype, so no need for us
				// to do this.
				// However we can give a user the possibility to hang up a call via Miranda's
				// context menu
				if (ptr=strstr(szSkypeMsg, " STATUS ")) {
					ptr[0]=0; ptr+=8;
					if (!strcmp(ptr, "RINGING") || !strcmp(ptr, "ROUTING")) pthread_create(( pThreadFunc )RingThread, _strdup(szSkypeMsg));
					if (!strcmp(ptr, "FAILED") || !strcmp(ptr, "FINISHED") ||
						!strcmp(ptr, "MISSED") || !strcmp(ptr, "REFUSED")  ||
						!strcmp(ptr, "BUSY")   || !strcmp(ptr, "CANCELLED"))
						pthread_create(( pThreadFunc )EndCallThread, _strdup(szSkypeMsg));
					if (!strcmp(ptr, "ONHOLD") || !strcmp(ptr, "LOCALHOLD") ||
						!strcmp(ptr, "REMOTEHOLD")) pthread_create(( pThreadFunc )HoldCallThread, _strdup(szSkypeMsg));
					if (!strcmp(ptr, "INPROGRESS")) pthread_create(( pThreadFunc )ResumeCallThread, _strdup(szSkypeMsg));
					break;
				} else if ((!strstr(szSkypeMsg, "PARTNER_HANDLE") && !strstr(szSkypeMsg, "FROM_HANDLE"))
							&& !strstr(szSkypeMsg, "TYPE")) break;
			}
			if (!strncmp(szSkypeMsg, "PRIVILEGE SKYPEOUT", 18)) {
				if (!strncmp(szSkypeMsg+19, "TRUE", 4)) {
					if (!bSkypeOut) {
						CLISTMENUITEM mi={0};

						bSkypeOut=TRUE; 
						mi.cbSize=sizeof(mi);
						mi.position=-2000005000;
						mi.flags=0;
						mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALLSKYPEOUT));
						mi.pszContactOwner=SKYPE_PROTONAME;
						mi.pszName=Translate("Do a SkypeOut-call");
						mi.pszService=SKYPEOUT_CALL;
						CallService(MS_CLIST_ADDMAINMENUITEM, (WPARAM)NULL,(LPARAM)&mi);
					}

				} else {
					bSkypeOut=FALSE;
					if (httbButton) {
						CallService(MS_TTB_REMOVEBUTTON, (WPARAM)httbButton, 0);
						httbButton=0;
					}
				}
				break;
			}

			if (!strncmp(szSkypeMsg, "MESSAGE", 7) || !strncmp(szSkypeMsg, "CHATMESSAGE", 11)) 
			{
				char *pMsgNum;
				TYP_MSGLENTRY *pEntry;

				if ((pMsgNum = strchr (szSkypeMsg, ' ')) && (ptr = strchr (++pMsgNum, ' ')))
				{
					BOOL bFetchMsg = FALSE;

					if (strncmp(ptr, " EDITED_TIMESTAMP", 17) == 0) {
						ptr[0]=0;
						if (pEntry = MsgList_FindMessage(strtoul(pMsgNum, NULL, 10))) {
							pEntry->tEdited = atol(ptr+18);
						}
						bFetchMsg = TRUE;
					} else bFetchMsg = strncmp(ptr, " STATUS RE", 10) == 0;

					if (bFetchMsg) {
						// If new message is available, fetch it
						ptr[0]=0;
						if (!(args=(fetchmsg_arg *)calloc(1, sizeof(*args)))) break;
						strncpy (args->msgnum, pMsgNum, sizeof(args->msgnum));
						args->getstatus=FALSE;
						args->bIsRead = strncmp(ptr+8, "READ", 4) == 0;
						pthread_create(( pThreadFunc )FetchMessageThreadSync, args);
						break;
					}
				}
			}
			if (!strncmp(szSkypeMsg, "MESSAGES", 8) || !strncmp(szSkypeMsg, "CHATMESSAGES", 12)) {
				if (strlen(szSkypeMsg)<=(UINT)(strchr(szSkypeMsg, ' ')-szSkypeMsg+1)) 
				{
					LOG(( "%s %d %s %d", szSkypeMsg,(UINT)(strchr(szSkypeMsg, ' ')-szSkypeMsg+1), 
						strchr(szSkypeMsg, ' '), strlen(szSkypeMsg)));
					break;
				}
				LOG(("MessageListProcessingThread launched"));
				pthread_create(( pThreadFunc )MessageListProcessingThread, _strdup(strchr(szSkypeMsg, ' ')));
				break;
			}
			if (!strncmp(szSkypeMsg, "ERROR 68", 8)) {
				LOG(("We got a sync problem :( ->  SendMessage() will try to recover..."));
				break;
			}
			if (!strncmp(szSkypeMsg, "PROTOCOL ", 9)) {
				if ((protocol=atoi(szSkypeMsg+9))>=3) {
					strcpy(cmdMessage, "CHATMESSAGE");
					strcpy(cmdPartner, "FROM");
				}
				bProtocolSet = TRUE;

				if (protocol<5 && !hMenuAddSkypeContact &&
					DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "EnableMenu", 1)) 
				{
					hMenuAddSkypeContact = add_mainmenu();
				}
			}
			LOG(("SkypeMsgAdd launched"));
			SkypeMsgAdd(szSkypeMsg);
			ReleaseSemaphore(SkypeMsgReceived, receivers, NULL);
		} else LOG (("WM_COPYDATA: Invalid handle, expected: %08X", (LONG)hSkypeWnd));
        break; 

        case WM_TIMER:
			SkypeSend("PING");
			SkypeMsgCollectGarbage(MAX_MSG_AGE);
			MsgList_CollectGarbage();
			if (receivers>1)
			{
				LOG(("Watchdog WARNING: there are still %d receivers waiting for MSGs", receivers));
			}
			break;

		case WM_CLOSE:
			PostQuitMessage (0);
			break;
		case WM_DESTROY:
			KillTimer (hWndDlg, 1);
			break; 

        default: 
		 if(message==ControlAPIAttach) {
				// Skype responds with Attach to the discover-message
				AttachStatus=lParam;
				if (AttachStatus==SKYPECONTROLAPI_ATTACH_SUCCESS) 
					hSkypeWnd=(HWND)wParam;	   // Skype gave us the communication window handle
				if (AttachStatus!=SKYPECONTROLAPI_ATTACH_API_AVAILABLE &&
					AttachStatus!=SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE) 
				{
					LOG(("Attaching: SkypeReady fired, Attachstatus is %d", AttachStatus));
					SetEvent(SkypeReady);
				}
				break;
		 }
		 return (DefWindowProc(hWndDlg, message, wParam, lParam)); 
    }
	LOG(("WM_COPYDATA exit (%08X)", message));
	if (szSkypeMsg) free(szSkypeMsg);
	return 1;
} 

void TellError(DWORD err) {
	LPVOID lpMsgBuf;
	
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
        MessageBox( NULL, (TCHAR*)lpMsgBuf, _T("GetLastError"), MB_OK|MB_ICONINFORMATION );
        LocalFree( lpMsgBuf );
		return;
}


// SERVICES //
INT_PTR SkypeSetStatus(WPARAM wParam, LPARAM lParam)
{
	int oldStatus;
	BOOL UseCustomCommand, UnloadOnOffline;


	if (MirandaShuttingDown) return 0;
	UseCustomCommand = DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseCustomCommand", 0);
	UnloadOnOffline = DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UnloadOnOffline", 0);

	//if (!SkypeInitialized && !DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UnloadOnOffline", 0)) return 0;

	// Workaround for Skype status-bug
    if ((int)wParam==ID_STATUS_OFFLINE) logoff_contacts();
	if (SkypeStatus==(int)wParam) return 0;
	oldStatus = SkypeStatus;

	if ((int)wParam==ID_STATUS_CONNECTING) return 0;
#ifdef MAPDND
	if ((int)wParam==ID_STATUS_OCCUPIED || (int)wParam==ID_STATUS_ONTHEPHONE) wParam=ID_STATUS_DND;
	if ((int)wParam==ID_STATUS_OUTTOLUNCH) wParam=ID_STATUS_NA;
#endif
#ifdef MAPNA
	if ((int)wParam==ID_STATUS_NA) wParam = ID_STATUS_AWAY;
#endif

   RequestedStatus=MirandaStatusToSkype((int)wParam);

   if (SkypeStatus != ID_STATUS_OFFLINE)
   {
     InterlockedExchange((long*)&SkypeStatus, (int)wParam);
     ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, SkypeStatus);
   }
   
   if ((int)wParam==ID_STATUS_OFFLINE && UnloadOnOffline) 
   {
			char* path = NULL;

			if(UseCustomCommand)
			{
				DBVARIANT dbv;
				if(!DBGetContactSettingString(NULL,SKYPE_PROTONAME,"CommandLine",&dbv)) 
				{
					CloseSkypeAPI(dbv.pszVal);
					DBFreeVariant(&dbv);
				}
			}
			else
			{
				CloseSkypeAPI(skype_path);
			}

   } else if (AttachStatus==-1) 
   {
	   pthread_create(LaunchSkypeAndSetStatusThread, (void *)wParam);
	   return 0;
   }

   return SetUserStatus(); 
}

int __stdcall SendBroadcast( HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam )
{
	ACKDATA ack = {0};
	ack.cbSize = sizeof( ACKDATA );
	ack.szModule = SKYPE_PROTONAME;
	ack.hContact = hContact;
	ack.type = type;
	ack.result = result;
	ack.hProcess = hProcess;
	ack.lParam = lParam;
	return CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );
}

static void __cdecl SkypeGetAwayMessageThread( HANDLE hContact )
{
	DBVARIANT dbv;
	if ( !DBGetContactSettingString( hContact, "CList", "StatusMsg", &dbv )) {
		SendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE )1, ( LPARAM )dbv.pszVal );
		DBFreeVariant( &dbv );
	}
	else SendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE )1, ( LPARAM )0 );
}

INT_PTR SkypeGetAwayMessage(WPARAM wParam,LPARAM lParam)
{
	CCSDATA* ccs = ( CCSDATA* )lParam;
	pthread_create( SkypeGetAwayMessageThread, ccs->hContact );
	return 1;
}

#define POLYNOMIAL (0x488781ED) /* This is the CRC Poly */
#define TOPBIT (1 << (WIDTH - 1)) /* MSB */
#define WIDTH 32

static int GetFileHash(char* filename)
{
	HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	int remainder = 0, byte, bit;
	char data[1024];
	DWORD dwRead;

	if(hFile == INVALID_HANDLE_VALUE) return 0;

	do
	{
		// Read file chunk
		dwRead = 0;
		ReadFile(hFile, data, 1024, &dwRead, NULL);

		/* loop through each byte of data */
		for (byte = 0; byte < (int) dwRead; ++byte) {
			/* store the next byte into the remainder */
			remainder ^= (data[byte] << (WIDTH - 8));
			/* calculate for all 8 bits in the byte */
			for ( bit = 8; bit > 0; --bit) {
				/* check if MSB of remainder is a one */
				if (remainder & TOPBIT)
					remainder = (remainder << 1) ^ POLYNOMIAL;
				else
					remainder = (remainder << 1);
			}
		}
	} while(dwRead == 1024);

	CloseHandle(hFile);

	return remainder;
}

static int _GetFileSize(char* filename)
{
	HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	int size;

	if(hFile == INVALID_HANDLE_VALUE)
		return 0;
	size = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return size;
}

/* RetrieveUserAvatar
 * 
 * Purpose: Get a user avatar from skype itself
 * Params : param=(void *)(HANDLE)hContact
 */
void RetrieveUserAvatar(void *param)
{
	HANDLE hContact = (HANDLE) param, file;
	PROTO_AVATAR_INFORMATION AI={0};
	ACKDATA ack = {0};
	DBVARIANT dbv;
	char AvatarFile[MAX_PATH+1], AvatarTmpFile[MAX_PATH+10], *ptr, *pszTempFile;

	if (hContact == NULL)
		return;

	// Mount default ack
	ack.cbSize = sizeof( ACKDATA );
	ack.szModule = SKYPE_PROTONAME;
	ack.hContact = hContact;
	ack.type = ACKTYPE_AVATAR;
	ack.result = ACKRESULT_FAILED;
	
	AI.cbSize = sizeof( AI );
	AI.hContact = hContact;

	// Get skype name
	if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv) == 0) 
	{
		if (dbv.pszVal)
		{
			// Get filename
			FoldersGetCustomPath(hProtocolAvatarsFolder, AvatarFile, sizeof(AvatarFile), DefaultAvatarsFolder);
			mir_snprintf(AvatarTmpFile, sizeof(AvatarTmpFile), "AVATAR 1 %s\\%s_tmp.jpg", AvatarFile, dbv.pszVal);
			pszTempFile = AvatarTmpFile+9;
			mir_snprintf(AvatarFile, sizeof(AvatarFile), "%s\\%s.jpg", AvatarFile, dbv.pszVal);

			// Just to be sure
			DeleteFileA(pszTempFile);
			file = CreateFileA(pszTempFile, 0, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file != INVALID_HANDLE_VALUE)
			{
				CloseHandle(file);
				if (ptr=SkypeGet ("USER", dbv.pszVal, AvatarTmpFile))
				{
					if (strncmp(ptr, "ERROR", 5) && 
						GetFileAttributesA(pszTempFile) != INVALID_FILE_ATTRIBUTES) 
					{
						ack.result = ACKRESULT_SUCCESS;

						// Is no avatar image?
						if (!DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "ShowDefaultSkypeAvatar", 0) 
							&& GetFileHash(pszTempFile) == 0x8d34e05d && _GetFileSize(pszTempFile) == 3751)
						{
							// Has no avatar
							AI.format = PA_FORMAT_UNKNOWN;
							ack.hProcess = (HANDLE)&AI;
							DeleteFileA(AvatarFile);
						}
						else
						{
							// Got it
							MoveFileExA(pszTempFile, AvatarFile, MOVEFILE_REPLACE_EXISTING);
							AI.format = PA_FORMAT_JPEG;
							strcpy(AI.filename, AvatarFile);
							ack.hProcess = (HANDLE)&AI;
						}

					}
					free (ptr);
				}
				DeleteFileA(pszTempFile);
			}

		}
		DBFreeVariant(&dbv);
	}
	CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );
}


/* SkypeGetAvatarInfo
 * 
 * Purpose: Set user avatar in profile
 * Params : wParam=0
 *			lParam=(LPARAM)(const char*)filename
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeGetAvatarInfo(WPARAM wParam,LPARAM lParam)
{

	DBVARIANT dbv;
	PROTO_AVATAR_INFORMATION* AI = ( PROTO_AVATAR_INFORMATION* )lParam;	
	if (AI->hContact == NULL) // User
	{
		if (!DBGetContactSettingString(NULL,SKYPE_PROTONAME, "AvatarFile", &dbv))
		{
			lstrcpynA(AI->filename, dbv.pszVal, sizeof(AI->filename));
			DBFreeVariant(&dbv);
			return GAIR_SUCCESS;
		}
		else
			return GAIR_NOAVATAR;
	}
	else // Contact 
	{
		DBVARIANT dbv;
		char AvatarFile[MAX_PATH+1];

		if (protocol < 7)
			return GAIR_NOAVATAR;

		if (wParam & GAIF_FORCE)
		{
			// Request anyway
			pthread_create(RetrieveUserAvatar, (void *) AI->hContact);
			return GAIR_WAITFOR;
		}

		if (DBGetContactSettingString(AI->hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) 
			// No skype name ??
			return GAIR_NOAVATAR;

		if (dbv.pszVal == NULL)
		{
			// No skype name ??
			DBFreeVariant(&dbv);
			return GAIR_NOAVATAR;
		}

		// Get filename
		FoldersGetCustomPath(hProtocolAvatarsFolder, AvatarFile, sizeof(AvatarFile), DefaultAvatarsFolder);
		mir_snprintf(AvatarFile, sizeof(AvatarFile), "%s\\%s.jpg", AvatarFile, dbv.pszVal);
		DBFreeVariant(&dbv);

		// Check if the file exists
		if (GetFileAttributesA(AvatarFile) == INVALID_FILE_ATTRIBUTES)
			return GAIR_NOAVATAR;
		
		// Return the avatar
		AI->format = PA_FORMAT_JPEG;
		strcpy(AI->filename, AvatarFile);
		return GAIR_SUCCESS;
	}
}


/* SkypeGetAvatarCaps
 * 
 * Purpose: Query avatar caps for a protocol
 * Params : wParam=One of AF_*
 *			lParam=Depends on wParam
 * Returns: Depends on wParam
 */
INT_PTR SkypeGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
		case AF_MAXSIZE:
		{
			POINT *p = (POINT *) lParam;
			if (p == NULL)
				return -1;

			p->x = 96;
			p->y = 96;
			return 0;
		}
		case AF_PROPORTION:
		{
			return PIP_NONE;
		}
		case AF_FORMATSUPPORTED:
		{
			if (lParam == PA_FORMAT_PNG || lParam == PA_FORMAT_JPEG)
				return TRUE;
			else
				return FALSE;
		}
		case AF_ENABLED:
		{
			return TRUE;
		}
		case AF_DONTNEEDDELAYS:
		{
			return FALSE;
		}
	}
	return -1;
}


INT_PTR SkypeGetStatus(WPARAM wParam, LPARAM lParam) {
	return SkypeStatus;
}

INT_PTR SkypeGetInfo(WPARAM wParam,LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	
	pthread_create(GetInfoThread, ccs->hContact);
	return 0;
}

INT_PTR SkypeAddToList(WPARAM wParam, LPARAM lParam) {
	PROTOSEARCHRESULT *psr=(PROTOSEARCHRESULT*)lParam;

	LOG(("SkypeAddToList Adding API function called"));
	if (psr->cbSize!=sizeof(PROTOSEARCHRESULT) || !psr->nick) return 0;
	LOG(("SkypeAddToList OK"));
    return (INT_PTR)add_contact(psr->nick, wParam);
}

INT_PTR SkypeBasicSearch(WPARAM wParam, LPARAM lParam) {

	LOG(("SkypeBasicSearch %s", (char *)lParam));
	if (!SkypeInitialized) return 0;
	return (hSearchThread=pthread_create(( pThreadFunc )BasicSearchThread, _strdup((char *)lParam)));
}

void MessageSendWatchThread(HANDLE hContact) {
	char *str, *err;

	LOG(("MessageSendWatchThread started."));
	if (!(str=SkypeRcvMsg("\0MESSAGE\0STATUS SENT\0", time(NULL)-1, DBGetContactSettingDword(NULL,"SRMsg","MessageTimeout",TIMEOUT_MSGSEND)+1000))) return;
	if (err=GetSkypeErrorMsg(str)) {
		ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE) 1, (LPARAM)Translate(err));
		free(err);
		free(str);
		LOG(("MessageSendWatchThread terminated."));
		return;
	}
	ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
	free(str);
	LOG(("MessageSendWatchThread terminated gracefully."));
}

INT_PTR SkypeSendMessage(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	DBVARIANT dbv;
	BOOL sendok=TRUE;
    char *msg = (char *) ccs->lParam, *utfmsg=NULL, *mymsgcmd=cmdMessage;
    
	if (DBGetContactSettingString(ccs->hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) {
      if (DBGetContactSettingString(ccs->hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv))
		  return 0;
	} else mymsgcmd="MESSAGE";
		

	if (ccs->wParam & PREF_UTF) {
		utfmsg = msg;
	} else if (ccs->wParam & PREF_UNICODE) {
		utfmsg = make_utf8_string((WCHAR*)(msg+strlen(msg)+1));
	} else {
		if (utf8_encode(msg, &utfmsg)==-1) utfmsg=NULL;
	}
	if (!utfmsg || SkypeSend("%s %s %s", mymsgcmd, dbv.pszVal, utfmsg)) sendok=FALSE;
	if (utfmsg && utfmsg!=msg) free(utfmsg);
	DBFreeVariant(&dbv);

	if (sendok) {
		pthread_create(MessageSendWatchThread, ccs->hContact);
		return 1;
	}
	ProtoBroadcastAck(SKYPE_PROTONAME, ccs->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE) 1, (LPARAM)Translate("Connection to Skype lost"));
	return 0;
}

INT_PTR SkypeRecvMessage(WPARAM wParam, LPARAM lParam)
{
    DBEVENTINFO dbei={0};
    CCSDATA *ccs = (CCSDATA *) lParam;
    PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;

    DBDeleteContactSetting(ccs->hContact, "CList", "Hidden");
    dbei.cbSize = sizeof(dbei);
    dbei.szModule = SKYPE_PROTONAME;
    dbei.timestamp = pre->timestamp;
	if (pre->flags & PREF_CREATEREAD) dbei.flags|=DBEF_READ;
    if (pre->flags & PREF_UTF) dbei.flags|=DBEF_UTF;
    dbei.eventType = EVENTTYPE_MESSAGE;
    dbei.cbBlob = strlen(pre->szMessage) + 1;
	if (pre->flags & PREF_UNICODE)
		dbei.cbBlob += sizeof( wchar_t )*( (DWORD)wcslen(( wchar_t* )&pre->szMessage[dbei.cbBlob+1] )+1 );
    dbei.pBlob = (PBYTE) pre->szMessage;
    MsgList_Add (pre->lParam, (HANDLE)CallService(MS_DB_EVENT_ADD, (WPARAM)ccs->hContact, (LPARAM)&dbei));
    return 0;
}

INT_PTR SkypeSendAuthRequest(WPARAM wParam, LPARAM lParam) {
	CCSDATA* ccs = (CCSDATA*)lParam;
	DBVARIANT dbv;
	int retval;

	if (!ccs->lParam || DBGetContactSettingString(ccs->hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv))
		return 1;
	retval = SkypeSend("SET USER %s BUDDYSTATUS 2 %s", dbv.pszVal, (char *)ccs->lParam);
	DBFreeVariant(&dbv);
	if (retval) return 1; else return 0;
}

INT_PTR SkypeRecvAuth(WPARAM wParam, LPARAM lParam) {
	DBEVENTINFO dbei = {0};
	CCSDATA* ccs = (CCSDATA*)lParam;
	PROTORECVEVENT* pre = (PROTORECVEVENT*)ccs->lParam;

	DBDeleteContactSetting(ccs->hContact, "CList", "Hidden");

	dbei.cbSize    = sizeof(dbei);
	dbei.szModule  = SKYPE_PROTONAME;
	dbei.timestamp = pre->timestamp;
	dbei.flags     = ((pre->flags & PREF_CREATEREAD)?DBEF_READ:0);
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob	   = pre->lParam;
	dbei.pBlob     = (PBYTE)pre->szMessage;

	CallService(MS_DB_EVENT_ADD, (WPARAM)NULL, (LPARAM)&dbei);
	return 0;
}

char *__skypeauth(WPARAM wParam) {
	DBEVENTINFO dbei={0};

	if (!SkypeInitialized) return NULL;

	dbei.cbSize = sizeof(dbei);
	if ((dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, wParam, 0))==-1 ||
		!(dbei.pBlob = (unsigned char*)malloc(dbei.cbBlob))) 
	{	return NULL; }

	if (CallService(MS_DB_EVENT_GET, wParam, (LPARAM)&dbei) ||
		dbei.eventType != EVENTTYPE_AUTHREQUEST ||
		strcmp(dbei.szModule, SKYPE_PROTONAME)) 
	{
		free(dbei.pBlob);
		return NULL;
	}
	return (char *)dbei.pBlob;
}

INT_PTR SkypeAuthAllow(WPARAM wParam, LPARAM lParam) {
	char *pBlob;

	if (pBlob=__skypeauth(wParam))
	{ 
		int retval=SkypeSend("SET USER %s ISAUTHORIZED TRUE", pBlob+sizeof(DWORD)+sizeof(HANDLE));
		free(pBlob);
		if (!retval) return 0;
	}
	return 1;
}

INT_PTR SkypeAuthDeny(WPARAM wParam, LPARAM lParam) {
	char *pBlob;

	if (pBlob=__skypeauth(wParam))
	{ 
		int retval=SkypeSend("SET USER %s ISAUTHORIZED FALSE", pBlob+sizeof(DWORD)+sizeof(HANDLE));
		free(pBlob);
		if (!retval) return 0;
	}
	return 1;
}


INT_PTR SkypeAddToListByEvent(WPARAM wParam, LPARAM lParam) {
	char *pBlob;

	if (pBlob=__skypeauth(wParam))
	{ 
		HANDLE hContact=add_contact(pBlob+sizeof(DWORD)+sizeof(HANDLE), LOWORD(wParam));
		free(pBlob);
		if (hContact) return (int)hContact;
	}
	return 0;
}

INT_PTR SkypeRegisterProxy(WPARAM wParam, LPARAM lParam) {
	if (!lParam) {
		free (pszProxyCallout);
		pszProxyCallout = NULL;
	}
	pszProxyCallout = _strdup((char*)lParam);
	return 0;
}


void CleanupNicknames(char *dummy) {
	HANDLE hContact;
	char *szProto;
	DBVARIANT dbv, dbv2;

	LOG(("CleanupNicknames Cleaning up..."));
	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME) &&
			DBGetContactSettingByte(hContact, SKYPE_PROTONAME, "ChatRoom", 0)==0)	
		{
			if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) continue;
			if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, "Nick", &dbv2)) {
				DBFreeVariant(&dbv);
				continue;
			}
			if (!strcmp(dbv.pszVal, dbv2.pszVal)) {
				DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "Nick");
				GetInfoThread(hContact);
			}
			DBFreeVariant(&dbv);
			DBFreeVariant(&dbv2);
		}
	}
	OUTPUT(_T("Cleanup finished."));
}

/////////////////////////////////////////////////////////////////////////////////////////
// EnterBitmapFileName - enters a bitmap filename

int __stdcall EnterBitmapFileName( char* szDest )
{
	char szFilter[ 512 ];
	char str[ MAX_PATH ] = {0};
	OPENFILENAMEA ofn = {0};
	*szDest = 0;

	CallService( MS_UTILS_GETBITMAPFILTERSTRINGS, sizeof szFilter, ( LPARAM )szFilter );
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szDest;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "bmp";
	if ( !GetOpenFileNameA( &ofn ))
		return 1;

	return ERROR_SUCCESS;
}

int MirandaExit(WPARAM wParam, LPARAM lParam) {
	MirandaShuttingDown=TRUE;
	return 0;
}

int OkToExit(WPARAM wParam, LPARAM lParam) {
//	logoff_contacts();
	MirandaShuttingDown=TRUE;

	// Trigger all semaphores and events just to be sure that there is no deadlock
	ReleaseSemaphore(SkypeMsgReceived, receivers, NULL);
    SetEvent (SkypeReady);
	SetEvent (MessagePumpReady);
#ifdef SKYPEBUG_OFFLN
	SetEvent(GotUserstatus);
#endif
	SetEvent (hBuddyAdded);

	SkypeFlush ();
	PostMessage (hWnd, WM_CLOSE, 0, 0);
	return 0;
}


struct PLUGINDI {
	char **szSettings;
	int dwCount;
};

// Taken from pluginopts.c and modified
int EnumOldPluginName(const char *szSetting,LPARAM lParam)
{
	struct PLUGINDI *pdi=(struct PLUGINDI*)lParam;
	if (pdi && lParam) {
			pdi->szSettings=(char**)realloc(pdi->szSettings,(pdi->dwCount+1)*sizeof(char*));
			pdi->szSettings[pdi->dwCount++]=_strdup(szSetting);
	} 
	return 0;
}

// Are there any Skype users on list? 
// 1 --> Yes
// 0 --> No
int AnySkypeusers(void) 
{
	HANDLE hContact;
	DBVARIANT dbv;
	int tCompareResult;

	// already on list?
	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		 hContact != NULL;
		 hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) 
	{
		// GETCONTACTBASEPROTO doesn't work on not loaded protocol, therefore get 
		// protocol from DB
		if (DBGetContactSettingString(hContact, "Protocol", "p", &dbv)) continue;
        tCompareResult = !strcmp(dbv.pszVal, SKYPE_PROTONAME);
		DBFreeVariant(&dbv);
		if (tCompareResult) return 1;
	}
	return 0;
}


void UpgradeName(char *OldName)
{	
	DBCONTACTENUMSETTINGS cns;
	DBCONTACTWRITESETTING cws;
	DBVARIANT dbv;
	HANDLE hContact=NULL;
	struct PLUGINDI pdi;

	LOG(("Updating old database settings if there are any..."));
	cns.pfnEnumProc=EnumOldPluginName;
	cns.lParam=(LPARAM)&pdi;
	cns.szModule=OldName;
	cns.ofsSettings=0;

	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);

	do {
		memset(&pdi,0,sizeof(pdi));
		CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)hContact,(LPARAM)&cns);
		// Upgrade Protocol settings to new string
		if (pdi.szSettings) {
			int i;

			LOG(("We're currently upgrading..."));
			for (i=0;i<pdi.dwCount;i++) {
				if (!DBGetContactSettingString(hContact, OldName, pdi.szSettings[i], &dbv)) {
					cws.szModule=SKYPE_PROTONAME;
					cws.szSetting=pdi.szSettings[i];
					cws.value=dbv;
					if (!CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws))
						DBDeleteContactSetting(hContact,OldName,pdi.szSettings[i]);
					DBFreeVariant(&dbv);
				}		
				free(pdi.szSettings[i]);
			}
			free(pdi.szSettings);
		} 
		// Upgrade Protocol assignment, if we are not main contact
		if (hContact && !DBGetContactSettingString(hContact, "Protocol", "p", &dbv)) {
			if (!strcmp(dbv.pszVal, OldName))
				DBWriteContactSettingString(hContact, "Protocol", "p", SKYPE_PROTONAME);
			DBFreeVariant(&dbv);
		}
		if (!hContact) break;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	} while (1);
	DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "UpgradeDone", (BYTE)1);
	return;
}

void __cdecl MsgPump (char *dummy)
{
  MSG msg;

  WNDCLASS WndClass; 

  // Create window class
  WndClass.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS; 
  WndClass.lpfnWndProc = (WNDPROC)WndProc; 
  WndClass.cbClsExtra = 0; 
  WndClass.cbWndExtra = 0; 
  WndClass.hInstance =  hInst;
  WndClass.hIcon = NULL; 
  WndClass.hCursor = NULL;
  WndClass.hbrBackground = NULL;
  WndClass.lpszMenuName = NULL; 
  WndClass.lpszClassName = _T("SkypeApiDispatchWindow"); 
  RegisterClass(&WndClass);
  // Do not check the retval of RegisterClass, because on non-unicode
  // win98 it will fail, as it is a stub that returns false() there
	
  // Create main window
  hWnd=CreateWindowEx( WS_EX_APPWINDOW|WS_EX_WINDOWEDGE,
		_T("SkypeApiDispatchWindow"), _T(""), WS_BORDER|WS_SYSMENU|WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 128, 128, NULL, 0, (HINSTANCE)WndClass.hInstance, 0);

  LOG (("Created Dispatch window with handle %08X", (long)hWnd));
  if (!hWnd) {
	OUTPUT(_T("Cannot create window."));
	TellError(GetLastError());
	SetEvent(MessagePumpReady);
    return; 
  }
  ShowWindow(hWnd, 0); 
  UpdateWindow(hWnd); 
  msgPumpThreadId = GetCurrentThreadId();
  SetEvent(MessagePumpReady);

  LOG (("Messagepump started."));
  while (GetMessage (&msg, hWnd, 0, 0) > 0 && !Miranda_Terminated()) {
	  TranslateMessage (&msg);
	  DispatchMessage (&msg);
  }
  UnregisterClass (WndClass.lpszClassName, hInst);
  LOG (("Messagepump stopped."));
}

// DLL Stuff //

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirVersion)
{
	mirandaVersion = mirVersion;

	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*) &pluginInfo;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirVersion)
{
	mirandaVersion = mirVersion;

	return &pluginInfo;
}

static const MUUID interfaces[] = {MUUID_SKYPE_CALL, MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}


int PreShutdown(WPARAM wParam, LPARAM lParam) {
	PostThreadMessage(msgPumpThreadId, WM_QUIT, 0, 0);
	return 0;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	PROTOCOLDESCRIPTOR pd;
	DWORD Buffsize;
	HKEY MyKey;
	BOOL SkypeInstalled;
	BOOL UseCustomCommand;
	WSADATA wsaData;
//	char *path, *protocolname, *fend;

	pluginLink = link;
	mir_getMMI( &mmi ); 

/*	GetModuleFileName( hInst, path, sizeof( path ));
	protocolname = strrchr(path,'\\');
	protocolname++;
	fend = strrchr(skype_path,'.');
	*fend = '\0';
	CharUpper( protocolname );
	lstrcpyn(SKYPE_PROTONAME, protocolname, MAX_PATH);
*/	

#ifdef _DEBUG
	init_debug();
#endif

	LOG(("Load: Skype Plugin loading..."));

	// We need to upgrade SKYPE_PROTOCOL internal name to Skype if not already done
	if (!DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UpgradeDone", 0))
		UpgradeName("SKYPE_PROTOCOL");

    // Initialisation of Skype MsgQueue must be done because of Cleanup in end and
	// Mutex is also initialized here.
	LOG(("SkypeMsgInit initializing Skype MSG-queue"));
	if (SkypeMsgInit()==-1) {
		OUTPUT(_T("Memory allocation error on startup."));
		return 0;
	}

	// On first run on new profile, ask user, if he wants to enable the plugin in
	// this profile
	// --> Fixing Issue #0000006 from bugtracker.
	if (!DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "FirstRun", 0)) {
		DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "FirstRun", 1);
		if (AnySkypeusers()==0) // First run, it seems :)
			if (MessageBox(NULL, TranslateT("This seems to be the first time that you're running the Skype protocol plugin. Do you want to enable the protocol for this Miranda-Profile? (If you chose NO, you can always enable it in the plugin options later."), _T("Welcome!"), MB_ICONQUESTION|MB_YESNO)==IDNO) {
				char path[MAX_PATH], *filename;
				GetModuleFileNameA(hInst, path, sizeof(path));
				if (filename = strrchr(path,'\\')+1)
					DBWriteContactSettingByte(NULL,"PluginDisable",filename,1);
				return 0;
			}
	}


	// Check if Skype is installed
	SkypeInstalled=TRUE;
	UseCustomCommand = (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseCustomCommand", 0);
	UseSockets = (BOOL)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseSkype2Socket", 0);

	if (!UseSockets && !UseCustomCommand) 
	{
		if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Skype\\Phone"), 0, KEY_READ, &MyKey)!=ERROR_SUCCESS)
		{
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Skype\\Phone"), 0, KEY_READ, &MyKey)!=ERROR_SUCCESS)
			{
				SkypeInstalled=FALSE;
			}
		}
		
		Buffsize=sizeof(skype_path);
		
		if (SkypeInstalled==FALSE || RegQueryValueExA(MyKey, "SkypePath", NULL, NULL, (unsigned char *)skype_path,  &Buffsize)!=ERROR_SUCCESS) 
		{
			    //OUTPUT("Skype was not found installed :( \nMaybe you are using portable skype.");
				RegCloseKey(MyKey);
				skype_path[0]=0;
				//return 0;
		}
		RegCloseKey(MyKey);
	}
	WSAStartup(MAKEWORD(2,2), &wsaData);

	// Start Skype connection 
	if (!(ControlAPIAttach=RegisterWindowMessage(_T("SkypeControlAPIAttach"))) || !(ControlAPIDiscover=RegisterWindowMessage(_T("SkypeControlAPIDiscover")))) 
	{
			OUTPUT(_T("Cannot register Window message."));
			return 0;
	}
	
	SkypeMsgReceived=CreateSemaphore(NULL, 0, MAX_MSGS, NULL);
	if (!(SkypeReady=CreateEvent(NULL, TRUE, FALSE, NULL)) ||
		!(MessagePumpReady=CreateEvent(NULL, FALSE, FALSE, NULL)) ||
#ifdef SKYPEBUG_OFFLN
	    !(GotUserstatus=CreateEvent(NULL, TRUE, FALSE, NULL)) ||
#endif
		!(hBuddyAdded=CreateEvent(NULL, FALSE, FALSE, NULL)) ||
		!(FetchMessageEvent=CreateEvent(NULL, FALSE, TRUE, NULL))) {
		 OUTPUT(_T("Unable to create Mutex!"));
		return 0;
	}

	/* Register the module */
	ZeroMemory(&pd, sizeof(pd));
	pd.cbSize = sizeof(pd);
	pd.szName = SKYPE_PROTONAME;
	pd.type   = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	VoiceServiceInit();

	CreateServices();
	HookEvents();
	InitVSApi();
	MsgList_Init();

	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	// Startup Message-pump
    pthread_create (( pThreadFunc )MsgPump, NULL);
	WaitForSingleObject(MessagePumpReady, INFINITE);
	return 0;
}



int __declspec( dllexport ) Unload(void) 
{
	BOOL UseCustomCommand = DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseCustomCommand", 0);
	BOOL Shutdown = DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "Shutdown", 0);
	
	LOG (("Unload started"));
	
	if ( Shutdown && ((skype_path && skype_path[0]) ||UseCustomCommand) ) {

		if(UseCustomCommand)
		{
			DBVARIANT dbv;
			if(!DBGetContactSettingString(NULL,SKYPE_PROTONAME,"CommandLine",&dbv)) 
			{
				_spawnl(_P_NOWAIT, dbv.pszVal, dbv.pszVal, "/SHUTDOWN", NULL);
				LOG (("Unload Sent /shutdown to %s", dbv.pszVal));
				DBFreeVariant(&dbv);
			}
		}
		else
		{
			_spawnl(_P_NOWAIT, skype_path, skype_path, "/SHUTDOWN", NULL);
			LOG (("Unload Sent /shutdown to %s", skype_path));
		}
		
	}
	SkypeMsgCleanup();
	WSACleanup();
	FreeVSApi();
	UnhookEvents();
	UnhookEvent(hChatEvent);
	UnhookEvent (hChatMenu);
	UnhookEvent (hEvInitChat);
	DestroyHookableEvent(hInitChat);
	VoiceServiceExit();
	GCExit();
	MsgList_Exit();

	CloseHandle(SkypeReady);
	CloseHandle(SkypeMsgReceived);
#ifdef SKYPEBUG_OFFLN
	CloseHandle(GotUserstatus);
#endif
	CloseHandle(MessagePumpReady);
	CloseHandle(hBuddyAdded);
	CloseHandle(FetchMessageEvent);

	DeleteCriticalSection(&RingAndEndcallMutex);
	DeleteCriticalSection(&QueryThreadMutex);

	SkypeRegisterProxy (0, 0);
	LOG (("Unload: Shutdown complete"));
#ifdef _DEBUG
	end_debug();
#endif
	return 0;
}

