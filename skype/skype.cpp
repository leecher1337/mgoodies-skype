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

struct MM_INTERFACE   mmi; 

POPUPDATAT MessagePopup;

// Exported Globals
HWND hSkypeWnd=NULL, hWnd=NULL;
HANDLE SkypeReady, SkypeMsgReceived, hOptHook,hHookOnUserInfoInit, hInitChat=NULL, httbButton=NULL;
BOOL SkypeInitialized=FALSE, QueryMsgDirection=FALSE, MirandaShuttingDown=FALSE;
BOOL UseSockets=FALSE, bSkypeOut=FALSE;
char pszSkypeProtoName[MAX_PATH+30], skype_path[MAX_PATH], protocol=2;
int SkypeStatus=ID_STATUS_OFFLINE, hSearchThread=-1, receivers=1;
UINT ControlAPIAttach, ControlAPIDiscover;
LONG AttachStatus=-1;
HINSTANCE hInst;
HANDLE hProtocolAvatarsFolder;
char DefaultAvatarsFolder[MAX_PATH+1];


// Module Internal Globals
PLUGINLINK *pluginLink;
HANDLE hStatusHookContact=NULL, hHookModulesLoaded=NULL, MessagePumpReady;
HANDLE SkypeMsgFetched, hPrebuildCMenu=NULL, hChatEvent=NULL, hChatMenu=NULL;
HANDLE hEvInitChat=NULL, hBuddyAdded=NULL, hTTBModuleLoadedHook=NULL, hContactDeleted=NULL;
HANDLE hMenuAddSkypeContact=NULL, hHookOkToExit=NULL, hHookMirandaExit=NULL;

DWORD msgPumpThreadId = 0;
#ifdef SKYPEBUG_OFFLN
HANDLE GotUserstatus;
#endif

BOOL ImportingHistory=FALSE, bModulesLoaded=FALSE;
char *RequestedStatus=NULL;	// To fix Skype-API Statusmode-bug
char cmdMessage[12]="CHATMESSAGE", cmdPartner[8]="PARTNER";	// Compatibility commands


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
	PLUGIN_MAKE_VERSION(0,0,0,39),
	"Support for Skype network",
	"leecher - tweety",
	"leecher@dose.0wnz.at - tweety@user.berlios.de",
	"© 2004-2006 leecher - tweety",
	"http://dose.0wnz.at - http://developer.berlios.de/projects/mgoodies/",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
	{ 0xa71f8335, 0x7b87, 0x4432, { 0xb8, 0xa3, 0x81, 0x47, 0x94, 0x31, 0xc6, 0xf5 } } // {A71F8335-7B87-4432-B8A3-81479431C6F5}
};

#define MAPDND	1	// Map Occupied to DND status and say that you support it

/*                           P R O G R A M                                */

void RegisterToDbeditorpp(void)
{
    // known modules list
    if (ServiceExists("DBEditorpp/RegisterSingleModule"))
        CallService("DBEditorpp/RegisterSingleModule", (WPARAM)pszSkypeProtoName, 0);
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
		update.szUpdateURL = "http://addons.miranda-im.org/feed.php?dlfile=3200";
		update.szVersionURL = "http://addons.miranda-im.org/details.php?action=viewfile&id=3200";
		update.pbVersionPrefix = (BYTE *)"<span class=\"fileNameHeader\">Skype Protocol ";
	    update.szBetaUpdateURL = "http://www.miranda-fr.net/tweety/skype/skype.zip";
		update.szBetaVersionURL = "http://www.miranda-fr.net/tweety/skype/skype_beta.html";
		update.pbBetaVersionPrefix = (BYTE *)"SKYPE version ";

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
int ShowMessage(int iconID, char *lpzText, int mustShow) {



	if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "SuppressErrors", 0)) return -1;
	lpzText=Translate(lpzText);

	if (bModulesLoaded && ServiceExists(MS_POPUP_ADDPOPUPT) && DBGetContactSettingByte(NULL, pszSkypeProtoName, "UsePopup", 0) && !MirandaShuttingDown) {
		bool showPopup, popupWindowColor;
		unsigned int popupBackColor, popupTextColor;
		int popupTimeSec;

		popupTimeSec = DBGetContactSettingDword(NULL, pszSkypeProtoName, "popupTimeSecErr", 4);
		popupTextColor = DBGetContactSettingDword(NULL, pszSkypeProtoName, "popupTextColorErr", GetSysColor(COLOR_WINDOWTEXT));
		popupBackColor = DBGetContactSettingDword(NULL, pszSkypeProtoName, "popupBackColorErr", GetSysColor(COLOR_BTNFACE));
		popupWindowColor = ( 0 != DBGetContactSettingByte(NULL, pszSkypeProtoName, "popupWindowColorErr", TRUE));
		showPopup = ( 0 != DBGetContactSettingByte(NULL, pszSkypeProtoName, "showPopupErr", TRUE));

		MessagePopup.lchContact = NULL;
		MessagePopup.lchIcon = LoadIcon(hInst,MAKEINTRESOURCE(iconID));
		MessagePopup.colorBack = ! popupWindowColor ? popupBackColor : GetSysColor(COLOR_BTNFACE);
		MessagePopup.colorText = ! popupWindowColor ? popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
		MessagePopup.iSeconds = popupTimeSec;
		MessagePopup.PluginData = (void *)1;
		
		lstrcpy(MessagePopup.lpzText, TranslateT(lpzText));

		lstrcpy(MessagePopup.lptzContactName, TranslateT(pszSkypeProtoName));

		if(showPopup)
			CallService(MS_POPUP_ADDPOPUPT,(WPARAM)&MessagePopup,0);

		return 0;
	} 
	else {

		if (mustShow==1) MessageBox(NULL,lpzText,pluginInfo.shortName, MB_OK | MB_ICONWARNING);
			return 0;
	}
	return -1;


}


// processing Hooks

int HookContactAdded(WPARAM wParam, LPARAM lParam) {
	char *szProto;

	szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
	if (szProto!=NULL && !strcmp(szProto, pszSkypeProtoName))
		add_contextmenu((HANDLE)wParam);
	return 0;
}

int HookContactDeleted(WPARAM wParam, LPARAM lParam) {
	char *szProto;

	szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
	if (szProto!=NULL && !strcmp(szProto, pszSkypeProtoName)) {
		DBVARIANT dbv;
		int retval;

		if (DBGetContactSetting((HANDLE)wParam, pszSkypeProtoName, SKYPE_NAME, &dbv)) return 1;
		retval=SkypeSend("SET USER %s BUDDYSTATUS 1", dbv.pszVal);
		DBFreeVariant(&dbv);
		if (retval) return 1;
	}
	return 0;
}

void GetInfoThread(HANDLE hContact) {
	DBVARIANT dbv;
	int eol, i=0, len;
	char str[19+MAX_USERLEN], *ptr, buf[5], *nm, *utfdstr=NULL, usr[MAX_USERLEN];
	struct CountryListEntry *countries;
	int countryCount;
	settings_map settings[]= {
		{"LANGUAGE", "Language1"},
		{"PROVINCE", "State"},
		{"CITY", "City"},
		{"PHONE_HOME", "Phone"},
		{"PHONE_OFFICE", "CompanyPhone"},
		{"PHONE_MOBILE", "Cellular"},
		{"HOMEPAGE", "Homepage"},
		{"ABOUT", "About"},
		{NULL, NULL}
	};

	LOG ("GetInfoThread", "started.");
	if (DBGetContactSetting(hContact, pszSkypeProtoName, SKYPE_NAME, &dbv) ||
		(len=strlen(dbv.pszVal))>MAX_USERLEN) 
	{
		LOG ("GetInfoThread", "terminated.");
		return;
	}

	eol=10+len;
	sprintf(str, "GET USER %s FULLNAME", dbv.pszVal);

	strcpy(usr, dbv.pszVal);
	DBFreeVariant(&dbv);

	if (!SkypeSend(str) && (ptr=SkypeRcv(str+4, INFINITE))) {
		if (ptr[strlen(str+3)]) {
			if (utf8_decode(strtok(ptr+strlen(str+3), " "), &utfdstr)!=-1 && utfdstr) {
				DBWriteContactSettingString(hContact, pszSkypeProtoName, "FirstName", utfdstr);
				free(utfdstr);
				if (!(nm=strtok(NULL, ""))) DBDeleteContactSetting(hContact, pszSkypeProtoName, "LastName");
				else {
					if (utf8_decode(nm, &utfdstr)!=-1 && utfdstr) {
						DBWriteContactSettingString(hContact, pszSkypeProtoName, "LastName", utfdstr);
						free(utfdstr);
					}
				}
			}
		}
		free(ptr);
	}

	str[eol]=0;
	strcat(str, "BIRTHDAY");
	if (!SkypeSend(str) && (ptr=SkypeRcv(str+4, INFINITE))) {
		if (ptr[strlen(str+3)] && ptr[strlen(str+3)]!='0') {
			ZeroMemory(buf, 4);
			strncpy(buf, ptr+strlen(str+3), 4);
			DBWriteContactSettingWord(hContact, pszSkypeProtoName, "BirthYear", (WORD)atoi(buf));
			ZeroMemory(buf, 4);
			strncpy(buf, ptr+strlen(str+3)+4, 2);
			DBWriteContactSettingByte(hContact, pszSkypeProtoName, "BirthMonth", (BYTE)atoi(buf));
			ZeroMemory(buf, 4);
			strncpy(buf, ptr+strlen(str+3)+6, 2);
			DBWriteContactSettingByte(hContact, pszSkypeProtoName, "BirthDay", (BYTE)atoi(buf));
		} else {
			DBDeleteContactSetting(hContact, pszSkypeProtoName, "BirthYear");
			DBDeleteContactSetting(hContact, pszSkypeProtoName, "BirthMonth");
			DBDeleteContactSetting(hContact, pszSkypeProtoName, "BirthDay");
		}
		free(ptr);
	}


	str[eol]=0;
	strcat(str, "COUNTRY");
	if (!SkypeSend(str) && (ptr=SkypeRcv(str+4, INFINITE))) {
		DBDeleteContactSetting(hContact, pszSkypeProtoName, "Country");
		if (ptr[strlen(str+3)]) {
			CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&countryCount, (LPARAM)&countries);
			for (i=0; i<countryCount; i++) {
				if (countries[i].id == 0 || countries[i].id == 0xFFFF) continue;
				if (!_stricmp(countries[i].szName, ptr+strlen(str+3))) 
					DBWriteContactSettingWord(hContact, pszSkypeProtoName, "Country", (BYTE)countries[i].id);
			}
		}
		free(ptr);
	}

	str[eol]=0;
	strcat(str, "SEX");
	if (!SkypeSend(str) && (ptr=SkypeRcv(str+4, INFINITE))) {
		DBDeleteContactSetting(hContact, pszSkypeProtoName, "Gender");
		if (ptr[strlen(str+3)]) {
			BYTE sex=0;
			if (!_stricmp(ptr+strlen(str+3), "MALE")) sex=0x4D;
			if (!_stricmp(ptr+strlen(str+3), "FEMALE")) sex=0x46;
			if (sex) DBWriteContactSettingByte(hContact, pszSkypeProtoName, "Gender", sex);
		}
		free(ptr);
	}

	str[eol]=0;
	strcat(str, "MOOD_TEXT");
	if (!SkypeSend(str) && (ptr=SkypeRcv(str+4, INFINITE))) {
		if (ptr[strlen(str+3)]) {
			TCHAR *unicode = NULL;
			char *Mood = NULL;
						
			if(utf8_decode((ptr+strlen(str+3)), &Mood)!=-1)
			{
				if(DBWriteContactSettingTString(hContact, "CList", "StatusMsg", Mood)) 
				{
					#if defined( _UNICODE )
						char buff[TEXT_LEN];
						WideCharToMultiByte(code_page, 0, Mood, -1, buff, TEXT_LEN, 0, 0);
						buff[TEXT_LEN] = 0;
						DBWriteContactSettingString(hContact, "CList", "StatusMsg", buff);
					#endif
				}
			}
		}
		else
			DBDeleteContactSetting(hContact, "CList", "StatusMsg");

		free(ptr);
	}

	str[eol]=0;
	strcat(str, "TIMEZONE");
	if (!SkypeSend(str) && (ptr=SkypeRcv(str+4, INFINITE))) {
		if (ptr[strlen(str+3)]) {
			time_t temp;
			struct tm tms;
			long myTimeZone = timezone;
			int value = atoi(ptr+strlen(str+3));

			
			if ( value != 0) {
				temp = time(NULL);
				tms = *localtime(&temp);

				if (value >= 86400 ) myTimeZone=256-((2*(value - 86400))/3600);
				if (value < 86400 ) myTimeZone=((-2*(value - 86400))/3600); 
				if (tms.tm_isdst == 1 && DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseTimeZonePatch", 0)) 
				{
					LOG("WndProc", "Using the TimeZonePatch");
					DBWriteContactSettingByte(hContact, "UserInfo", "Timezone", (myTimeZone+2));
				}
				else
				{
					LOG("WndProc", "Not using the TimeZonePatch");
					DBWriteContactSettingByte(hContact, "UserInfo", "Timezone", (myTimeZone+0));
				}
			}
			else 
			{
				LOG("WndProc", "Deleting the TimeZone in UserInfo Section");
				DBDeleteContactSetting(hContact, "UserInfo", "Timezone");
			}
		}
		free(ptr);
	}


	str[eol]=0;
	strcat(str, "IS_VIDEO_CAPABLE");
	if (!SkypeSend(str) && (ptr=SkypeRcv(str+4, INFINITE))) {
		if (ptr[strlen(str+3)]) {
			if (!_stricmp(ptr+strlen(str+3), "True"))
				DBWriteContactSettingString(hContact, pszSkypeProtoName, "MirVer", "Skype 2.0");
			else
				DBWriteContactSettingString(hContact, pszSkypeProtoName, "MirVer", "Skype");
			
		}
		free(ptr);
	}

	if (protocol >= 7) {
		// Notify about the possibility of an avatar
		ACKDATA ack = {0};
		ack.cbSize = sizeof( ACKDATA );
		ack.szModule = pszSkypeProtoName;
		ack.hContact = hContact;
		ack.type = ACKTYPE_AVATAR;
		ack.result = ACKRESULT_STATUS;

		CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );
	}

	i=0;
	while (settings[i].SkypeSetting) {
		str[eol]=0;
		strcat(str, settings[i].SkypeSetting);
		if (!SkypeSend(str) && (ptr=SkypeRcv(str+4, INFINITE))) {
			if (ptr[strlen(str+3)] && utf8_decode(ptr+strlen(str+3), &utfdstr)!=-1) {
				DBWriteContactSettingString(hContact, pszSkypeProtoName, settings[i].MirandaSetting, utfdstr);
				free(utfdstr);
			}
			else
				DBDeleteContactSetting(hContact, pszSkypeProtoName, settings[i].MirandaSetting);
			free(ptr);
		}
		i++;
	}
	ProtoBroadcastAck(pszSkypeProtoName, hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
    LOG ("GetInfoThread", "terminated gracefully.");
}

void BasicSearchThread(char *nick) {
	PROTOSEARCHRESULT psr={0};
	char *cmd, *token, *ptr;

    LOG ("BasicSearchThread", "started.");
	if (SkypeSend("SEARCH USERS %s", nick) || !(cmd=SkypeRcv("USERS", INFINITE))) {
		ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)hSearchThread, 0);
        LOG ("BasicSearchThread", "terminated.");
		return;
	} 
	if (!strncmp(cmd, "ERROR", 5)) {
		OUT(cmd);
		free(cmd);
		ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)hSearchThread, 0);
        LOG ("BasicSearchThread", "terminated.");
		return;
	}
	token=strtok(cmd+5, ", ");
	psr.cbSize=sizeof(psr);
	while (token) {
		psr.nick=_strdup(token);
		psr.lastName=NULL;
		psr.firstName=NULL;
		psr.email=NULL;
		if (ptr=SkypeGet("USER", token, "FULLNAME")) {
			// We cannot use strtok() to seperate first & last name here,
			// because we already use it for parsing the user list
			// So we use our own function
			if (psr.lastName=strchr(ptr, ' ')) {
				*psr.lastName=0;
				psr.lastName=_strdup(psr.lastName+1);
				LOG("lastName", psr.lastName);
			}
			psr.firstName=_strdup(ptr);
			LOG("firstName", psr.firstName);
			free(ptr);
		}
		ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)hSearchThread, (LPARAM)(PROTOSEARCHRESULT*)&psr);
		free(psr.nick); psr.nick=NULL;
		free(psr.lastName);
		free(psr.firstName);
		token=strtok(NULL, ", ");
	}
	free(psr.nick);
	free(cmd);
	ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)hSearchThread, 0);
	free(nick);
    LOG ("BasicSearchThread", "terminated gracefully.");
	return;
}

// added by TioDuke
void GetDisplaynameThread(char *dummy) {
	DBVARIANT dbv;
	char *ptr, *utfdstr=NULL;
    
	LOG("GetDisplaynameThread", "started.");
	if (DBGetContactSetting(NULL, pszSkypeProtoName, SKYPE_NAME, &dbv)) {
		LOG("GetDisplaynameThread", "terminated.");
		return;
	}
    if ((ptr=SkypeGet("USER", dbv.pszVal, "FULLNAME"))) {
		if (utf8_decode(ptr, &utfdstr)!=-1 && utfdstr) {
			if (utfdstr[0]) DBWriteContactSettingString(NULL, pszSkypeProtoName, "Nick", utfdstr);
			free(utfdstr);
		}
		free(ptr);
	}
	DBFreeVariant(&dbv);
    LOG("GetDisplaynameThread", "terminated gracefully.");
}


// Starts importing history from Skype
int ImportHistory(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	
	if (DBGetContactSetting((HANDLE)wParam, pszSkypeProtoName, SKYPE_NAME, &dbv)) return 0;

	QueryMsgDirection=TRUE;
	ImportingHistory=TRUE;

	SkypeSend("SEARCH %sS %s", cmdMessage, dbv.pszVal);
	DBFreeVariant(&dbv);
	return 0;
}

int SearchFriends(void) {
	char *ptr, *token, buf[MAX_USERLEN+23];

	if (SkypeSend("SEARCH FRIENDS")==-1 || !(ptr=SkypeRcv("USERS", INFINITE))) return -1;
	if (!strncmp(ptr, "ERROR", 5)) {
		free(ptr);
		return -1;	
	}
	if (ptr+5) {
		token=strtok(ptr+5, ", ");
		while (token) {
			strcpy(buf, "GET USER ");
			strncat(buf, token, MAX_USERLEN);
			strcat(buf, " ONLINESTATUS");
			if (SkypeSend(buf)==-1) return -1;
			testfor(buf+4, INFINITE);
			token=strtok(NULL, ", ");
		}
	}
	free(ptr);
	return 0;
}

int SearchUsersWaitingMyAuthorization(void) {
	char *cmd, *token;

	if (SkypeSend("SEARCH USERSWAITINGMYAUTHORIZATION")) return -1;
	if (!(cmd=SkypeRcv("USERS", INFINITE))) return -1;
	if (!strncmp(cmd, "ERROR", 5)) {
		free(cmd);
		return -1;
	}

	token=strtok(cmd+5, ", ");
	while (token) {
		CCSDATA ccs={0};
		PROTORECVEVENT pre={0};
		HANDLE hContact;
		char *firstname=NULL, *lastname=NULL, *pCurBlob;
		
		LOG("Awaiting auth:", token);
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
    LOG("SearchFriendsThread", "started.");
	SkypeInitialized=FALSE;
	SearchFriends();
	SkypeInitialized=TRUE;
    LOG("SearchFriendsThread", "terminated gracefully.");
}

void __cdecl SkypeSystemInit(char *dummy) {
	char buf[48];
	static BOOL Initializing=FALSE;

    LOG ("SkypeSystemInit", "thread started.");
	if (SkypeInitialized || Initializing) return;
	Initializing=TRUE;
// Do initial Skype-Tasks
	logoff_contacts();
// Add friends

	if (SkypeSend(SKYPE_PROTO)==-1 || !testfor("PROTOCOL", INFINITE) ||
		SkypeSend("GET PRIVILEGE SKYPEOUT")==-1) {
		Initializing=FALSE;
        LOG ("SkypeSystemInit", "thread stopped with failure.");
		return;	
	}

#ifdef SKYPEBUG_OFFLN
    if (!ResetEvent(GotUserstatus) || SkypeSend("GET USERSTATUS")==-1 || 
		WaitForSingleObject(GotUserstatus, INFINITE)==WAIT_FAILED) 
	{
        LOG ("SkypeSystemInit", "thread stopped with failure.");
		Initializing=FALSE;
		return;
	}
	if (SkypeStatus!=ID_STATUS_OFFLINE)
#endif
	if (SearchFriends()==-1) {
        LOG ("SkypeSystemInit", "thread stopped with failure.");
		Initializing=FALSE;
		return;	
	}
	SearchUsersWaitingMyAuthorization();
	_snprintf(buf, sizeof(buf), "SEARCH MISSED%sS", cmdMessage);
	SkypeSend(buf);
// Get my Nickname
	if (SkypeSend("GET CURRENTUSERHANDLE")==-1
#ifndef SKYPEBUG_OFFLN
		|| SkypeSend("GET USERSTATUS")==-1
#endif
		) 
	{
        LOG ("SkypeSystemInit", "thread stopped with failure.");
		Initializing=FALSE;
		return;
	}
	SetTimer (hWnd, 1, PING_INTERVAL, NULL);
	SkypeInitialized=TRUE;
	Initializing=FALSE;
	LOG ("SkypeSystemInit", "thread terminated gracefully.");
	return;
}

void FirstLaunch(char *dummy) {
	int counter=0;

	LOG ("FirstLaunch", "thread started.");
	if (!DBGetContactSettingByte(NULL, pszSkypeProtoName, "StartSkype", 1) || ConnectToSkypeAPI(skype_path, false)==-1) 
	{
		int oldstatus=SkypeStatus;

		LOG("OnModulesLoaded", "starting offline..");	
		InterlockedExchange((long *)&SkypeStatus, ID_STATUS_OFFLINE);
		ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldstatus, SkypeStatus);
	}
	if (AttachStatus==-1 || AttachStatus==SKYPECONTROLAPI_ATTACH_REFUSED || AttachStatus==SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE) {
		LOG ("FirstLaunch", "thread stopped because of invalid Attachstatus.");
		return;
	}
	
	// When you launch Skype and Attach is Successfull, it still takes some time
	// until it becomes available for receiving messages.
	// Let's probe this with PINGing
	LOG("CheckIfApiIsResponding", "Entering test loop");
	while (1) {
		LOGL("Test #", counter);
		if (SkypeSend("PING")==-1) counter ++; else break;
		if (counter>=20) {
			OUTPUT("Cannot reach Skype API, plugin disfunct.");
			LOG ("FirstLaunch", "thread topped: cannot reach Skype API.");
			return;
		}
		Sleep(500);
	}
	LOG("CheckIfApiIsResponding", "Testing for PONG");
	testfor("PONG", 2000); // Flush PONG from MsgQueue

	pthread_create(( pThreadFunc )SkypeSystemInit, NULL);
	LOG ("FirstLaunch", "thread terminated gracefully.");
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

	RegisterToUpdate();
	RegisterToDbeditorpp();
	VoiceServiceModulesLoaded() ;
	
	add_contextmenu(NULL);
	if ( ServiceExists( MS_GC_REGISTER )) 
	{
		GCREGISTER gcr = {0};
		static COLORREF crCols[1] = {0};
		char *szEvent;

		gcr.cbSize = sizeof( GCREGISTER );
		gcr.dwFlags = GC_CHANMGR; // |GC_ACKMSG; // TODO: Not implemented yet
        gcr.pszModuleDispName = pszSkypeProtoName;
		gcr.pszModule = pszSkypeProtoName;
		if (CallService(MS_GC_REGISTER, 0, (LPARAM)&gcr)) 
		{
			OUTPUT("Unable to register with Groupchat module!");
		}
		if (szEvent=(char*)malloc(strlen(pszSkypeProtoName)+10)) 
		{
			_snprintf(szEvent, sizeof szEvent, "%s\\ChatInit", pszSkypeProtoName);
			hInitChat = CreateHookableEvent(szEvent);
			hEvInitChat = HookEvent(szEvent, ChatInit);
			free(szEvent);
		} 
		else { OUTPUT("Out of memory!"); }

		hChatEvent = HookEvent(ME_GC_EVENT, GCEventHook);
		hChatMenu = HookEvent(ME_GC_BUILDMENU, GCMenuHook);
        CreateServiceFunction (SKYPE_CHATNEW, SkypeChatCreate);
	}
	// We cannot check for the TTB-service before this event gets fired... :-/
	hTTBModuleLoadedHook = HookEvent(ME_TTB_MODULELOADED, CreateTopToolbarButton);
	hHookOnUserInfoInit = HookEvent( ME_USERINFO_INITIALISE, OnDetailsInit );

	// Try folder service first
	hProtocolAvatarsFolder = NULL;
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		mir_snprintf(DefaultAvatarsFolder, sizeof(DefaultAvatarsFolder), "%s\\%s", PROFILE_PATH, pszSkypeProtoName);
		hProtocolAvatarsFolder = (HANDLE) FoldersRegisterCustomPath(pszSkypeProtoName, "Avatars Cache", DefaultAvatarsFolder);
	}
	
	if (hProtocolAvatarsFolder == NULL)
	{
		// Use defaults
		CallService(MS_DB_GETPROFILEPATH, (WPARAM) MAX_PATH, (LPARAM) DefaultAvatarsFolder);
		mir_snprintf(DefaultAvatarsFolder, sizeof(DefaultAvatarsFolder), "%s\\%s", DefaultAvatarsFolder, pszSkypeProtoName);
		CreateDirectory(DefaultAvatarsFolder, NULL);
	}

	pthread_create(( pThreadFunc )FirstLaunch, NULL);
	return 0;
}


void FetchMessageThread(fetchmsg_arg *args) {
	char str[64], *ptr, *who, *msg, *msg_emoted;
	char *ptr3,strChat[64];
	int msgl, direction=0;
	DWORD timestamp, lwr=0;
    CCSDATA ccs;
    PROTORECVEVENT pre;
    HANDLE hContact, hDbEvent;
	DBEVENTINFO dbei={0};
	BOOL getstatus, bEmoted=false, isGroupChat=false;

	if (!args || !args->msgnum) return;
	Sleep(200);
	sprintf(str, "GET %s ", cmdMessage);
	strncat(str, args->msgnum, 32);
	msgl=6+strlen(cmdMessage)+strlen(args->msgnum);
	free(args->msgnum);
	getstatus=args->getstatus;
	free(args);
	
	// Was it a message?
	strcat(str, " TYPE");
	LOG("FetchMessageThread", "Get the TYPE");
	LOG("FetchMessageThread", str);
	if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
		SetEvent(SkypeMsgFetched);
		return;
	}
	ERRCHK
	str[msgl]=0;
	LOG("FetchMessageThread", ptr);
	if( !strncmp(ptr+strlen(ptr)-6, "EMOTED", 6) ) bEmoted = true;
	if( !strncmp(ptr+strlen(ptr)-16, "MULTI_SUBSCRIBED", 16) ) isGroupChat = true;
	if( !strncmp(ptr+strlen(ptr)-15, "CREATEDCHATWITH", 15) ) {
		SetEvent(SkypeMsgFetched);
		return;
	}
	if( !strncmp(ptr+strlen(ptr)-10, "SAWMEMBERS", 10) ) {
		SetEvent(SkypeMsgFetched);
		return;
	}

	if (strncmp(ptr+strlen(ptr)-4, "TEXT", 4) && strncmp(ptr+strlen(ptr)-4, "SAID", 4) && !bEmoted && isGroupChat) 
	{
		if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseGroupchat", 0)) 
		{
			if (!strncmp(ptr+strlen(ptr)-10,"SAWMEMBERS", 10)) 
			{
				// We have a new Groupchat
				LOG("FetchMessageThread", "CHAT SAWMEMBERS");
				free(ptr);
				strcat(str, "CHATNAME");
				if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) 
				{
					SetEvent(SkypeMsgFetched);
					return;
				}

				ChatStart(ptr+strlen(str+4)+1);
				free(ptr);
				SetEvent(SkypeMsgFetched);
				return;
			}
			if (!strncmp(ptr+strlen(ptr)-8,"SETTOPIC", 8)) 
			{
				LOG("FetchMessageThread", "CHAT SETTOPIC");
				GCDEST gcd = {0};
				GCEVENT gce = {0};
				char *ptr2;

				free(ptr);
				strcat(str, "CHATNAME");
				if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) 
				{
					SetEvent(SkypeMsgFetched);
					return;
				}
				str[msgl]=0;
				gce.cbSize = sizeof(GCEVENT);
				gcd.pszModule = pszSkypeProtoName;
				gcd.pszID = ptr+strlen(str+4)+1;
				gcd.iType = GC_EVENT_TOPIC;
				gce.pDest = &gcd;
				strcat(str, cmdPartner);
				strcat(str, "_HANDLE");
				if (SkypeSend(str)==-1 || !(who=SkypeRcv(str+4, INFINITE))) 
				{
					free(ptr);
					SetEvent(SkypeMsgFetched);
					return;
				}
				gce.pszUID = who+strlen(str+4)+1;
				sprintf(str, "CHAT %s TOPIC", gcd.pszID);
				if (ptr2=SkypeRcv(str, INFINITE)) 
				{
					gce.pszText = ptr2+strlen(str);
					CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
					free(ptr2);
				}
				free(ptr);
				free(who);
				SetEvent(SkypeMsgFetched);
				return;
			}
			if (!strncmp(ptr+strlen(ptr)-4,"LEFT", 4) || !strncmp(ptr+strlen(ptr)-12,"ADDEDMEMBERS", 12)) 
			{

				LOG("FetchMessageThread", "CHAT LEFT or ADDEDMEMBERS");
				free(ptr);
				strcat(str, "CHATNAME");
				if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) 
				{
					SetEvent(SkypeMsgFetched);
					return;
				}
				SkypeSend ("GET CHAT %s MEMBERS", ptr+strlen(str+4)+1);
				str[msgl]=0;

				free(ptr);
				SetEvent(SkypeMsgFetched);
				return;
			}
		}
		// Boo,no useful message, ignore it
		#ifdef _DEBUG
			OUTPUT(ptr);
			OUTPUT("NO TEXT! :(");
		#endif
		free(ptr);
		SetEvent(SkypeMsgFetched);
		return;
	}
	free(ptr);


	// Who sent it?
	strcat(str, cmdPartner);
	strcat(str, "_HANDLE");
	if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
		SetEvent(SkypeMsgFetched);
		return;
	}
	ERRCHK
	who=_strdup(ptr+strlen(str+4)+1);
	str[msgl]=0;
	free(ptr);

	// Timestamp
	strcat(str, "TIMESTAMP");
	if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
		SetEvent(SkypeMsgFetched);
		free(who);
		return;
	}
	ERRCHK
	timestamp=atol(ptr+strlen(str+4)+1);
	str[msgl]=0;
	free(ptr);

	if (QueryMsgDirection && getstatus) {
		strcat(str, "STATUS");
		if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
			free(who);
			SetEvent(SkypeMsgFetched);
			return;
		}
		ERRCHK
		if (!strncmp(ptr+strlen(str+4)+1, "SENT", 4)) direction=DBEF_SENT;
		str[msgl]=0;
		free(ptr);
	}

	// Text which was sent
	strcat(str, "BODY");
	if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
		free(who);
		SetEvent(SkypeMsgFetched);
		return;
	}
	ERRCHK

	wchar_t *unicode;
	unicode = make_unicode_string( (const unsigned char *)ptr+strlen(str+4)+1);
    if(unicode == NULL)
    {
		free(ptr);
		free(who);
		SetEvent(SkypeMsgFetched);
		return;
	}

	if (utf8_decode(ptr+strlen(str+4)+1, &msg)==-1) {
		free(ptr);
		free(who);
		SetEvent(SkypeMsgFetched);
		return;
	}
	free(ptr);
	str[msgl]=0;


	// Aaaand add it..
	LOG("FetchMessageThread", "Finding contact handle");
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

	if (QueryMsgDirection) {
		// Check if the timestamp is valid
		LOG("FetchMessageThread", "Checking timestamps");
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
		if (timestamp<lwr) {
			LOG("FetchMessageThread", "Adding event");
			dbei.szModule=pszSkypeProtoName;
			dbei.cbBlob=strlen(msg)+1;
			dbei.pBlob=(PBYTE)msg;
			dbei.timestamp=timestamp>0?timestamp:time(NULL);
			dbei.flags=direction | DBEF_READ;
			dbei.eventType=EVENTTYPE_MESSAGE;
			CallServiceSync(MS_DB_EVENT_ADD, (WPARAM)(HANDLE)hContact, (LPARAM)&dbei);
		}
	}
	
	LOG("FetchMessageThread", "Check if message if a groupchat or not");
	// Is it a groupchat message?
	strcpy(strChat,str);
	strcat(strChat, "CHATNAME");
	LOG("FetchMessageThread", "Request the CHATNAME");
	if (SkypeSend(strChat)==-1 || !(ptr=SkypeRcv(strChat+4, INFINITE))) {
		free(who);
		free(msg);
		SetEvent(SkypeMsgFetched);
		return;
	}

	LOG("FetchMessageThread", "Request the CHAT STATUS");
	if (!(ptr3=SkypeGet("CHAT", ptr+strlen(strChat+4)+1, "STATUS"))) {
		free(ptr);
		free(who);
		free(msg);
		SetEvent(SkypeMsgFetched);
		return;
	}

	LOG("FetchMessageThread", "Compare the STATUS to MULTI_SUBSCRIBED");
	LOG("FetchMessageThread",ptr3);
	if (!strcmp(ptr3, "MULTI_SUBSCRIBED"))
		isGroupChat = true;


	if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseGroupchat", 0)) {

		LOG("FetchMessageThread", "Using groupchat option is checked");
		

		/*strcat(str, "CHATNAME");
		if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
			free(who);
			free(msg);
			SetEvent(SkypeMsgFetched);
			return;
		}
		if (!(ptr3=SkypeGet("CHAT", ptr+strlen(str+4)+1, "STATUS"))) {
			free(ptr);
			free(who);
			free(msg);
			SetEvent(SkypeMsgFetched);
			return;
		}*/
		if (isGroupChat) {

			LOG("FetchMessageThread", "This is a group chat message");

			GCDEST gcd = {0};
			GCEVENT gce = {0};
			DBVARIANT dbv = {0};
			HANDLE hContact;
            CONTACTINFO ci = {0};

			gcd.pszModule = pszSkypeProtoName;
			gcd.pszID = ptr+strlen(str+4)+1;
			gcd.iType = GC_EVENT_MESSAGE;
			gce.cbSize = sizeof(GCEVENT);
			gce.pDest = &gcd;
			gce.pszUID = who;
			ci.cbSize = sizeof(ci);
			ci.szProto = pszSkypeProtoName;
			ci.dwFlag = CNF_DISPLAY;
			gce.pszNick=who;
			if ((hContact=find_contact(who)))
			{
				ci.hContact = hContact;
				if (!CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) gce.pszNick=ci.pszVal; 
			}
			gce.time = timestamp>0?timestamp:time(NULL);;
			gce.bIsMe = FALSE;

			gce.pszText = msg;
			gce.bAddToLog = TRUE;
			CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
			if (dbv.pszVal) DBFreeVariant(&dbv);
            if (ci.pszVal) miranda_sys_free (ci.pszVal);

			free(ptr);
			free(ptr3);
			free(who);
			free(msg);

			// Yes, we have successfully read the msg
			str[msgl]=0;
			str[0]='S';	// SET, not GET
			strcat(str, "SEEN");
			SkypeSend(str);
			SetEvent(SkypeMsgFetched);
			return;
		}
		str[msgl]=0;
	}

	LOG("FetchMessageThread", "free(ptr);");
	free(ptr);
	LOG("FetchMessageThread", "free(ptr3);");
	free(ptr3);

	if (!QueryMsgDirection || (QueryMsgDirection && timestamp>dbei.timestamp)) {
		LOG("FetchMessageThread", "Normal message add...");
		// Normal message received, process it
	    ccs.szProtoService = PSR_MESSAGE;
	    ccs.hContact = hContact;
		ccs.wParam = 0;
		ccs.lParam = (LPARAM)&pre;
		pre.flags = direction;
	
		if(isGroupChat && DBGetContactSettingByte(NULL, pszSkypeProtoName, "MarkGroupchatRead", 0))
			pre.flags = PREF_CREATEREAD;

		pre.timestamp = timestamp>0?timestamp:time(NULL);
		if( bEmoted )
		{
			CONTACTINFO ci = {0};
			ci.cbSize = sizeof(ci);
			ci.szProto = pszSkypeProtoName;
			ci.dwFlag = CNF_DISPLAY;
			if ((hContact=find_contact(who)))
			{
				ci.hContact = hContact;
				if (!CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci))
				{
					msg_emoted = new char[strlen(msg) + strlen(ci.pszVal) + 8];
					sprintf(msg_emoted,"** %s %s **",ci.pszVal,msg);
				}
				else
				{
					msg_emoted = new char[strlen(msg) + 6];
					sprintf(msg_emoted,"** %s **",msg);
				}
			}
			
			pre.szMessage = msg_emoted;

			if (ci.pszVal) miranda_sys_free (ci.pszVal);
		}
		else
		{
			pre.szMessage = msg;
		}
		pre.lParam = 0;
		CallServiceSync(MS_PROTO_CHAINRECV, 0, (LPARAM) &ccs);

		// Yes, we have successfully read the msg
		str[0]='S';	// SET, not GET
		strcat(str, "SEEN");
		SkypeSend(str);
		// MSG STATUS read is already handled and discarded by our MSG-handling routine,
		// so no need to check here
	}
	free(who);
	free(msg);
	
	SetEvent(SkypeMsgFetched);
}

void MessageListProcessingThread(char *str) {
	char *token;
	fetchmsg_arg *args;

	token=strtok(str, ",");
	while (token) {
		if (args=(fetchmsg_arg *)malloc(sizeof(*args))) {
			args->msgnum=_strdup(token+1);
			args->getstatus=TRUE;
			pthread_create(( pThreadFunc )FetchMessageThread, args);
			WaitForSingleObject(SkypeMsgFetched, INFINITE);
		}
		token=strtok(NULL, ",");
	}
	QueryMsgDirection=FALSE;
	if (ImportingHistory) {
		OUTPUT("History import complete!");
		ImportingHistory=FALSE;
	}
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
			if (DBGetContactSetting(hContact, pszSkypeProtoName, "SkypeOutNr", &dbv)) continue;
			tCompareResult = strcmp(dbv.pszVal, szHandle);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue; else break;
		}
	}
	free(szHandle);
	if (!hContact) {LOG("GetCallerContact", "Not found!");}
	return hContact;
}

LRESULT CALLBACK InCallPopUpProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	switch(msg)
	{
		case WM_COMMAND:
			break;

		case WM_CONTEXTMENU:
			SendMessage(hWnd,UM_DESTROYPOPUP,0,0);
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
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

void RingThread(char *szSkypeMsg) {
	HANDLE hContact;
	DBEVENTINFO dbei={0};
	DBVARIANT dbv;
	char *ptr;

    LOG("RingThread", "started.");
	if (protocol >= 5) SkypeSend ("MINIMIZE");
	if (hContact=GetCallerContact(szSkypeMsg)) {
		// Make sure that an answering thread is not already in progress so that we don't get
		// the 'Incoming call' event twice
		if (!DBGetContactSetting(hContact, pszSkypeProtoName, "CallId", &dbv)) {
			DBFreeVariant(&dbv);
			free(szSkypeMsg);
            LOG("RingThread", "terminated.");
			return;
		}
		DBWriteContactSettingString(hContact, pszSkypeProtoName, "CallId", szSkypeMsg);
	}
	
	if (!(ptr=SkypeGet(szSkypeMsg, "TYPE", ""))) {
		free(szSkypeMsg);
        LOG("RingThread", "terminated.");
		return;
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
					free(ptr);
					free(szSkypeMsg);
					return;
				}
				DBDeleteContactSetting(hContact, "CList", "Hidden");
				DBWriteContactSettingWord(hContact, pszSkypeProtoName, "Status", (WORD)SkypeStatusToMiranda("SKYPEOUT"));
				DBWriteContactSettingString(hContact, pszSkypeProtoName, "SkypeOutNr", szHandle);
				free(szHandle);
			} else {
				free(ptr);
				free(szSkypeMsg);
				return;
			}
		}
	}

	if (HasVoiceService()) {
		// Voice service will handle it
		free(ptr);
		free(szSkypeMsg);
		return;
	}

	dbei.cbSize=sizeof(dbei);
	dbei.eventType=EVENTTYPE_CALL;
	dbei.szModule=pszSkypeProtoName;
	dbei.timestamp=time(NULL);
	dbei.pBlob=(unsigned char*)Translate("Phonecall");
	dbei.cbBlob=lstrlen((const char*)dbei.pBlob)+1;
	if (!strncmp(ptr, "INCOMING", 8)) 
	{
		if(ServiceExists(MS_POPUP_ADDPOPUPEX)) 
		{
			bool showPopup, popupWindowColor;
			unsigned int popupBackColor, popupTextColor;
			int popupTimeSec;
			POPUPDATAT InCallPopup;
			TCHAR * lpzContactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,GCDNF_TCHAR);

			popupTimeSec = DBGetContactSettingDword(NULL, pszSkypeProtoName, "popupTimeSec", 4);
			popupTextColor = DBGetContactSettingDword(NULL, pszSkypeProtoName, "popupTextColor", GetSysColor(COLOR_WINDOWTEXT));
			popupBackColor = DBGetContactSettingDword(NULL, pszSkypeProtoName, "popupBackColor", GetSysColor(COLOR_BTNFACE));
			popupWindowColor = (0 != DBGetContactSettingByte(NULL, pszSkypeProtoName, "popupWindowColor", TRUE));
			showPopup = (0 != DBGetContactSettingByte(NULL, pszSkypeProtoName, "showPopup", TRUE));

			InCallPopup.lchContact = hContact;
			InCallPopup.lchIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALL));
			InCallPopup.colorBack = ! popupWindowColor ? popupBackColor : GetSysColor(COLOR_BTNFACE);
			InCallPopup.colorText = ! popupWindowColor ? popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
			InCallPopup.iSeconds = popupTimeSec;
			InCallPopup.PluginWindowProc = (WNDPROC)InCallPopUpProc;
			InCallPopup.PluginData = (void *)1;
			
			lstrcpy(InCallPopup.lpzText, TranslateT("Incoming Skype Call"));

			lstrcpy(InCallPopup.lptzContactName, lpzContactName);

			if(showPopup)
				CallService(MS_POPUP_ADDPOPUPT,(WPARAM)&InCallPopup,0);

		}

		CLISTEVENT cle={0};
		char toolTip[256];

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

	free(ptr);
	free(szSkypeMsg);
}

void EndCallThread(char *szSkypeMsg) {
	HANDLE hContact=NULL, hDbEvent;
	DBEVENTINFO dbei={0};
	DBVARIANT dbv;
	int tCompareResult;

	if (szSkypeMsg) {
		for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
			if (DBGetContactSetting(hContact, pszSkypeProtoName, "CallId", &dbv)) continue;
			tCompareResult = strcmp(dbv.pszVal, szSkypeMsg);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue; else break;
		}
	}
	if (!hContact) {
		free(szSkypeMsg);
		return;
	}

	NofifyVoiceService(hContact, szSkypeMsg, VOICE_STATE_ENDED);

	DBDeleteContactSetting(hContact, pszSkypeProtoName, "CallId");

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

	if (!DBGetContactSetting(hContact, pszSkypeProtoName, "SkypeOutNr", &dbv)) {
		DBFreeVariant(&dbv);
		if (!strcmp((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0), pszSkypeProtoName) && 
			DBGetContactSettingByte(hContact, "CList", "NotOnList", 0)
		   )
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
	}
	free(szSkypeMsg);
}

void HoldCallThread(char *szSkypeMsg) {
	HANDLE hContact;

	LOG("HoldCallThread", "started");
    if (!szSkypeMsg) {
		LOG("HoldCallThread", "terminated.");
		return;
	}
	if (hContact=GetCallerContact(szSkypeMsg)) {
		DBWriteContactSettingByte(hContact, pszSkypeProtoName, "OnHold", 1);
		NofifyVoiceService(hContact, szSkypeMsg, VOICE_STATE_ON_HOLD);
	}
	free(szSkypeMsg);
	LOG("HoldCallThread", "terminated gracefully");
}

void ResumeCallThread(char *szSkypeMsg) {
	HANDLE hContact;

	LOG("ResumeCallThread", "started");
	if (!szSkypeMsg) {
		LOG("ResumeCallThread", "terminated.");
		return;
	}
	if (hContact=GetCallerContact(szSkypeMsg)) {
		DBDeleteContactSetting(hContact, pszSkypeProtoName, "OnHold");
		NofifyVoiceService(hContact, szSkypeMsg, VOICE_STATE_TALKING);
	}
	free(szSkypeMsg);
    LOG("ResumeCallThread", "terminated gracefully.");
}

int SetUserStatus(void) {
   char cStr[30];

   if (RequestedStatus && AttachStatus!=-1) {
	lstrcpy(cStr, "SET USERSTATUS ");
	lstrcat(cStr, RequestedStatus);
	if (SkypeSend(cStr)==-1) return 1;
   }
   return 0;
}

void LaunchSkypeAndSetStatusThread(void *newStatus) {

/*	   if (!DBGetContactSettingByte(NULL, pszSkypeProtoName, "UnloadOnOffline", 0)) {
		   logoff_contacts();
		   return 1;
	   }
*/
	LOG ("LaunchSkypeAndSetStatusThread", "started.");
	   
	if (ConnectToSkypeAPI(skype_path, true)!=-1) {
		int oldStatus=SkypeStatus;
		pthread_create(( pThreadFunc )SkypeSystemInit, NULL);
		InterlockedExchange((long *)&SkypeStatus, (int)newStatus);
		ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, SkypeStatus);
		SetUserStatus();
	}

	LOG ("LaunchSkypeAndSetStatusThread", "terminated gracefully.");
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
		 LOG("WM_COPYDATA", "start");
		 if(hSkypeWnd==(HWND)wParam) { 
			CopyData=(PCOPYDATASTRUCT)lParam;
			szSkypeMsg=_strdup((char*)CopyData->lpData);
			ReplyMessage(1);
			LOG("<", szSkypeMsg);

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
					ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldstatus, SkypeStatus);
					if (sstat!=ID_STATUS_OFFLINE) {
						if (sstat!=ID_STATUS_CONNECTING && (oldstatus==ID_STATUS_OFFLINE || oldstatus==ID_STATUS_CONNECTING)) {

							SkypeInitialized=FALSE;
							pthread_create(( pThreadFunc )SkypeSystemInit, NULL);
						}
						if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "KeepState", 0)) RestoreUserStatus=TRUE;
					}

//					if (SkypeStatus==ID_STATUS_ONLINE) SkypeSend("SEARCH MISSEDMESSAGES");
				}
				break;
			}
			if (!strncmp(szSkypeMsg, "USERSTATUS", 10)) {
//				if ((sstat=SkypeStatusToMiranda(szSkypeMsg+11)) && SkypeStatus!=ID_STATUS_CONNECTING) {
				if ((sstat=SkypeStatusToMiranda(szSkypeMsg+11))) {				
						if (RestoreUserStatus && RequestedStatus) {
							SkypeSend ("SET USERSTATUS %s", RequestedStatus);

							RestoreUserStatus=FALSE;
						}
						oldstatus=SkypeStatus;
						InterlockedExchange((long*)&SkypeStatus, sstat);
						ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldstatus, sstat);
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
				if (!strcmp(ptr, "ONLINESTATUS")) {
					if (SkypeStatus==ID_STATUS_OFFLINE) break;
					if (!(hContact=find_contact(nick))) {

						SkypeSend("GET USER %s BUDDYSTATUS", nick);
//						break;
					} 
					else
					{
						DBWriteContactSettingWord(hContact, pszSkypeProtoName, "Status", (WORD)SkypeStatusToMiranda(ptr+13));
						if((WORD)SkypeStatusToMiranda(ptr+13) != ID_STATUS_OFFLINE)
						{
							LOG("WndProc", "Status is not offline so get user info");
							pthread_create(GetInfoThread, hContact);
						}
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
					break;
*/				
				}
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
				if (!strcmp(ptr, "MOOD_TEXT")){
					LOG("WndProc", "MOOD_TEXT");
					if (!(hContact=find_contact(nick)))
						SkypeSend("GET USER %s BUDDYSTATUS", nick);
					else
					{
						TCHAR *unicode = NULL;
						
						if(utf8_decode((ptr+10), &Mood)==-1) break;

						//DBWriteContactSettingString(hContact, "CList", "StatusMsg", Mood);
						if(DBWriteContactSettingTString(hContact, "CList", "StatusMsg", Mood)) 
						{
							#if defined( _UNICODE )
								char buff[TEXT_LEN];
								WideCharToMultiByte(code_page, 0, Mood, -1, buff, TEXT_LEN, 0, 0);
								buff[TEXT_LEN] = 0;
								DBWriteContactSettingString(hContact, "CList", "StatusMsg", buff);
							#endif
						}
												
						
					}
					free(buf);
					break;

				}

				if (!strcmp(ptr, "TIMEZONE")){
					time_t temp;
					struct tm tms;
					
					hContact=find_contact(nick);
					LOG("WndProc", "TIMEZONE");
					LOG("WndProc", nick);
					if (hContact==NULL)
					{
						LOG("WndProc", "No contact found");
						SkypeSend("GET USER %s BUDDYSTATUS", nick);
					}
					else
						if (atoi(ptr+9) != 0) {
							temp = time(NULL);
							tms = *localtime(&temp);
							//memcpy(&tms,localtime(&temp), sizeof(tm));
							//tms = localtime(&temp);
							if (atoi(ptr+9) >= 86400 ) timezone=256-((2*(atoi(ptr+9)-86400))/3600);
							if (atoi(ptr+9) < 86400 ) timezone=((-2*(atoi(ptr+9)-86400))/3600); 
							if (tms.tm_isdst == 1 && DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseTimeZonePatch", 0)) 
							{
								LOG("WndProc", "Using the TimeZonePatch");
								DBWriteContactSettingByte(hContact, "UserInfo", "Timezone", (timezone+2));
							}
							else
							{
								LOG("WndProc", "Not using the TimeZonePatch");
								DBWriteContactSettingByte(hContact, "UserInfo", "Timezone", (timezone+0));
							}
						}
						else 
						{
							LOG("WndProc", "Deleting the TimeZone in UserInfo Section");
							DBDeleteContactSetting(hContact, "UserInfo", "Timezone");
						}
  					free(buf);
					break;

				}
				if (!strcmp(ptr, "IS_VIDEO_CAPABLE")){
					if (!(hContact=find_contact(nick)))
						SkypeSend("GET USER %s BUDDYSTATUS", nick);
					else{
						if (!_stricmp(ptr + 17, "True"))
							DBWriteContactSettingString(hContact, pszSkypeProtoName, "MirVer", "Skype 2.0");
						else
							DBWriteContactSettingString(hContact, pszSkypeProtoName, "MirVer", "Skype");
					}
					free(buf);
					break;

				}

*/

				if (!strcmp(ptr, "DISPLAYNAME")) {
					// Skype Bug? -> If nickname isn't customised in the Skype-App, this won't return anything :-(
					if (ptr[12]) 
					{
						if(utf8_decode((ptr+12), &nick)==-1) break;
						DBWriteContactSettingString(find_contact(nick), pszSkypeProtoName, "Nick", nick);
					}
					free(buf);
					break;
				}
				if (!strcmp(ptr, "LASTONLINETIMESTAMP")) {
					free(buf);
					break; // Ignore
				}
				if (!strcmp(ptr, "BUDDYSTATUS")) {
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
				DBWriteContactSettingString(NULL, pszSkypeProtoName, SKYPE_NAME, szSkypeMsg+18);
				DBWriteContactSettingString(NULL, pszSkypeProtoName, "Nick", szSkypeMsg+18);
				pthread_create(( pThreadFunc )GetDisplaynameThread, NULL);
				break;
			}
			if ((strstr(szSkypeMsg, "STATUS READ") && !QueryMsgDirection) ||
				 strstr(szSkypeMsg, "AUTOAWAY") || !strncmp(szSkypeMsg, "OPEN ",5) ||
				 (SkypeInitialized && !strncmp (szSkypeMsg, "PONG", 4)) ||
				 !strncmp (szSkypeMsg, "MINIMIZE", 8)) 
			{
				// Currently we do not process these messages  
				break;
			}
			if (!strncmp(szSkypeMsg, "CHAT ", 5)) {
				// Currently we only process these notifications
				if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseGroupchat", 0)) 
				{
					// Throw away old unseen messages to reduce memory-usage
					if (ptr=strstr(szSkypeMsg, " TOPIC")) {
						ptr[6]=0;
						while (testfor(szSkypeMsg, 0));
						ptr[6]=' ';
					} else
					if (ptr=strstr(szSkypeMsg, " MEMBERS")) {					
						LOG("WndProc", "AddMembers");
						AddMembers (szSkypeMsg);
						break;
					}/*
					else
					if (ptr=strstr(szSkypeMsg, " STATUS")) {
						ptr[7]=0;
						while (testfor(szSkypeMsg, 0));
						ptr[7]=' ';
					} //else break;*/
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
						mi.pszContactOwner=pszSkypeProtoName;
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

			if (!strncmp(szSkypeMsg, cmdMessage, strlen(cmdMessage)) && (ptr=strstr(szSkypeMsg, " STATUS RECEIVED"))!=NULL) {
				// If new message is available, fetch it
				ptr[0]=0;
				if (!(args=(fetchmsg_arg *)malloc(sizeof(*args)))) break;
				args->msgnum=_strdup(strchr(szSkypeMsg, ' ')+1);
				args->getstatus=FALSE;
				pthread_create(( pThreadFunc )FetchMessageThread, args);
				break;
			}
			if (!strncmp(szSkypeMsg, "MESSAGES", 8) || !strncmp(szSkypeMsg, "CHATMESSAGES", 12)) {
				if (strlen(szSkypeMsg)<=(UINT)(strchr(szSkypeMsg, ' ')-szSkypeMsg+1)) 
				{
					LOGL( szSkypeMsg,(UINT)(strchr(szSkypeMsg, ' ')-szSkypeMsg+1));
					LOGL(_strdup(strchr(szSkypeMsg, ' ')), strlen(szSkypeMsg));
					break;
				}
				pthread_create(( pThreadFunc )MessageListProcessingThread, _strdup(strchr(szSkypeMsg, ' ')));
				break;
			}
			if (!strncmp(szSkypeMsg, "ERROR 68", 8)) {
				LOG("We got a sync problem :( -> ", "SendMessage() will try to recover...");
				break;
			}
			if (!strncmp(szSkypeMsg, "PROTOCOL ", 9)) {
				if ((protocol=atoi(szSkypeMsg+9))>=3) {
					strcpy(cmdMessage, "CHATMESSAGE");
					strcpy(cmdPartner, "FROM");
				}

				if (protocol<5 && !hMenuAddSkypeContact &&
					DBGetContactSettingByte(NULL, pszSkypeProtoName, "EnableMenu", 1)) 
				{
					hMenuAddSkypeContact = add_mainmenu();
				}
			}
			SkypeMsgAdd(szSkypeMsg);
			ReleaseSemaphore(SkypeMsgReceived, receivers, NULL);
		}  
        break; 

        case WM_TIMER:
			SkypeSend("PING");
			SkypeMsgCollectGarbage(MAX_MSG_AGE);
			if (receivers>1)
			{
				LOGL("Watchdog WARNING: there are still receivers waiting for MSGs: ", receivers);
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
					LOGL("Attaching: SkypeReady fired, Attachstatus is ", AttachStatus);
					SetEvent(SkypeReady);
				}
				break;
		 }
		 return (DefWindowProc(hWndDlg, message, wParam, lParam)); 
    }
	LOG("WM_COPYDATA", "exit");
	if (szSkypeMsg) free(szSkypeMsg);
	return 1;
} 

void TellError(DWORD err) {
	LPVOID lpMsgBuf;
	
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
        MessageBox( NULL, (char*)lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );
        LocalFree( lpMsgBuf );
		return;
}


// SERVICES //
int SkypeSetStatus(WPARAM wParam, LPARAM lParam)
{
	if (MirandaShuttingDown) return 0;

	int oldStatus;
	BOOL UseCustomCommand = DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseCustomCommand", 0);
	BOOL UnloadOnOffline = DBGetContactSettingByte(NULL, pszSkypeProtoName, "UnloadOnOffline", 0);

	//if (!SkypeInitialized && !DBGetContactSettingByte(NULL, pszSkypeProtoName, "UnloadOnOffline", 0)) return 0;

	// Workaround for Skype status-bug
    if ((int)wParam==ID_STATUS_OFFLINE) logoff_contacts();
	if (SkypeStatus==(int)wParam) return 0;
	oldStatus = SkypeStatus;

	if ((int)wParam==ID_STATUS_CONNECTING) return 0;
#ifdef MAPDND
	if ((int)wParam==ID_STATUS_OCCUPIED || (int)wParam==ID_STATUS_ONTHEPHONE) wParam=ID_STATUS_DND;
	if ((int)wParam==ID_STATUS_OUTTOLUNCH) wParam=ID_STATUS_NA;
#endif

   RequestedStatus=MirandaStatusToSkype((int)wParam);

   InterlockedExchange((long*)&SkypeStatus, (int)wParam);
   ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, SkypeStatus);
   
   if ((int)wParam==ID_STATUS_OFFLINE) 
   {
	   if (UnloadOnOffline) {
		   if (AttachStatus!=-1) 
		   {
			   if(UseCustomCommand)
			   {
				   DBVARIANT dbv;
					if(!DBGetContactSetting(NULL,pszSkypeProtoName,"CommandLine",&dbv)) 
					{
						_spawnl(_P_NOWAIT, dbv.pszVal, dbv.pszVal, "/SHUTDOWN", NULL);
						DBFreeVariant(&dbv);
					}
			   }
			   else
			   {
				   _spawnl(_P_NOWAIT, skype_path, skype_path, "/SHUTDOWN", NULL);
			   }
		   }
	 	   logoff_contacts();
		   SkypeInitialized=FALSE;
		   ResetEvent(SkypeReady);
		   AttachStatus=-1;
		   if (hWnd) KillTimer (hWnd, 1);
		   return 0;
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
	ack.szModule = pszSkypeProtoName;
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
	if ( !DBGetContactSetting( hContact, "CList", "StatusMsg", &dbv )) {
		SendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE )1, ( LPARAM )dbv.pszVal );
		DBFreeVariant( &dbv );
	}
	else SendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE )1, ( LPARAM )0 );
}

int SkypeGetAwayMessage(WPARAM wParam,LPARAM lParam)
{
	CCSDATA* ccs = ( CCSDATA* )lParam;
	pthread_create( SkypeGetAwayMessageThread, ccs->hContact );
	return 1;
}



/* RetrieveUserAvatar
 * 
 * Purpose: Get a user avatar from skype itself
 * Params : param=(void *)(HANDLE)hContact
 */
void RetrieveUserAvatar(void *param)
{
	HANDLE hContact = (HANDLE) param;
	if (hContact == NULL)
		return;

	// Mount default ack
	ACKDATA ack = {0};
	ack.cbSize = sizeof( ACKDATA );
	ack.szModule = pszSkypeProtoName;
	ack.hContact = hContact;
	ack.type = ACKTYPE_AVATAR;

	// Get skype name
	DBVARIANT dbv;
	if (DBGetContactSetting(hContact, pszSkypeProtoName, SKYPE_NAME, &dbv)) 
	{
		// No skype name ??
		ack.result = ACKRESULT_FAILED;
		CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );
		return;
	}

	size_t len;
	if (dbv.pszVal == NULL || (len = strlen(dbv.pszVal)) > MAX_USERLEN)
	{
		// No skype name ??
		DBFreeVariant(&dbv);
		ack.result = ACKRESULT_FAILED;
		CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );
		return;
	}

	// Get filename
	char AvatarFile[MAX_PATH+1], AvatarTmpFile[MAX_PATH+1], command[500];

	FoldersGetCustomPath(hProtocolAvatarsFolder, AvatarFile, sizeof(AvatarFile), DefaultAvatarsFolder);
	mir_snprintf(AvatarTmpFile, sizeof(AvatarTmpFile), "%s\\%s_tmp.jpg", AvatarFile, dbv.pszVal);
	mir_snprintf(AvatarFile, sizeof(AvatarFile), "%s\\%s.jpg", AvatarFile, dbv.pszVal);

	mir_snprintf(command, sizeof(command), "GET USER %s AVATAR 1 %s", dbv.pszVal, AvatarTmpFile);

	DBFreeVariant(&dbv);

	// Just to be sure
	DeleteFile(AvatarTmpFile);
	HANDLE file = CreateFile(AvatarTmpFile, 0, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		// Error creating empty file
		ack.result = ACKRESULT_FAILED;
		CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );
		return;
	}
	CloseHandle(file);

	// Request avatar
	char *ptr;
	if (SkypeSend(command) || (ptr=SkypeRcv(command+4, INFINITE))== NULL) 
	{
		// Error receiving avatar
		ack.result = ACKRESULT_FAILED;
		CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );

		DeleteFile(AvatarTmpFile);
		return;
	}


	if (!strncmp(ptr, "ERROR", 5)) 
	{
		// Error receiving avatar
		ack.result = ACKRESULT_FAILED;
		CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );

		DeleteFile(AvatarTmpFile);
		free(ptr);
		return;
	}

	free(ptr);

	if (GetFileAttributesA(AvatarTmpFile) == INVALID_FILE_ATTRIBUTES)
	{
		// Error reading avatar
		ack.result = ACKRESULT_FAILED;
		CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );

		DeleteFile(AvatarTmpFile);
		return;
	}

	// Got it

	CopyFile(AvatarTmpFile, AvatarFile, FALSE);
	DeleteFile(AvatarTmpFile);

	PROTO_AVATAR_INFORMATION AI;
	AI.cbSize = sizeof( AI );
	AI.format = PA_FORMAT_JPEG;
	AI.hContact = hContact;
	strcpy(AI.filename, AvatarFile);

	ack.result = ACKRESULT_SUCCESS;
	ack.hProcess = HANDLE( &AI );
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
int SkypeGetAvatarInfo(WPARAM wParam,LPARAM lParam)
{

	DBVARIANT dbv;
	PROTO_AVATAR_INFORMATION* AI = ( PROTO_AVATAR_INFORMATION* )lParam;	
	if (AI->hContact == NULL) // User
	{
		if (!DBGetContactSetting(NULL,pszSkypeProtoName, "AvatarFile", &dbv))
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
		if (protocol < 7)
			return GAIR_NOAVATAR;

		if (wParam & GAIF_FORCE)
		{
			// Request anyway
			pthread_create(RetrieveUserAvatar, (void *) AI->hContact);
			return GAIR_WAITFOR;
		}

		DBVARIANT dbv;
		if (DBGetContactSetting(AI->hContact, pszSkypeProtoName, SKYPE_NAME, &dbv)) 
			// No skype name ??
			return GAIR_NOAVATAR;

		if (dbv.pszVal == NULL || strlen(dbv.pszVal) > MAX_USERLEN)
		{
			// No skype name ??
			DBFreeVariant(&dbv);
			return GAIR_NOAVATAR;
		}

		// Get filename
		char AvatarFile[MAX_PATH+1];
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


int SkypeGetStatus(WPARAM wParam, LPARAM lParam) {
	return SkypeStatus;
}

int SkypeGetInfo(WPARAM wParam,LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	
	pthread_create(GetInfoThread, ccs->hContact);
	return 0;
}

int SkypeAddToList(WPARAM wParam, LPARAM lParam) {
	PROTOSEARCHRESULT *psr=(PROTOSEARCHRESULT*)lParam;

	LOG("SkypeAddToList", "Adding API function called");
	if (psr->cbSize!=sizeof(PROTOSEARCHRESULT) || !psr->nick) return 0;
	LOG("SkypeAddToList", "OK");
    return (int)add_contact(psr->nick, wParam);
}

int SkypeBasicSearch(WPARAM wParam, LPARAM lParam) {

	LOG("SkypeBasicSearch", (char *)lParam);
	if (!SkypeInitialized) return 0;
	return (hSearchThread=pthread_create(( pThreadFunc )BasicSearchThread, _strdup((char *)lParam)));
}

void MessageSendWatchThread(HANDLE hContact) {
	char *str, *err;

	LOG("MessageSendWatchThread", "started.");
	if (!(str=SkypeRcv("\0MESSAGE\0STATUS SENT\0", DBGetContactSettingDword(NULL,"SRMsg","MessageTimeout",TIMEOUT_MSGSEND)+1000))) return;
	if (err=GetSkypeErrorMsg(str)) {
		ProtoBroadcastAck(pszSkypeProtoName, hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE) 1, (LPARAM)Translate(err));
		free(err);
		free(str);
		LOG("MessageSendWatchThread", "terminated.");
		return;
	}
	ProtoBroadcastAck(pszSkypeProtoName, hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
	free(str);
	LOG("MessageSendWatchThread", "terminated gracefully.");
}

int SkypeSendMessage(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	DBVARIANT dbv;
	BOOL sendok=TRUE;
    char *msg = (char *) ccs->lParam, *utfmsg=NULL, *mymsgcmd=cmdMessage;
    
	if (DBGetContactSetting(ccs->hContact, pszSkypeProtoName, SKYPE_NAME, &dbv)) {
      if (DBGetContactSetting(ccs->hContact, pszSkypeProtoName, "ChatRoomID", &dbv))
		  return 0;
	} else mymsgcmd += 4;
		
	if (utf8_encode(msg, &utfmsg)==-1 || !utfmsg || 
		SkypeSend("%s %s %s", mymsgcmd, dbv.pszVal, utfmsg)) sendok=FALSE;

	free(utfmsg);
	DBFreeVariant(&dbv);

	if (sendok) {
		pthread_create(MessageSendWatchThread, ccs->hContact);
		return 1;
	}
	ProtoBroadcastAck(pszSkypeProtoName, ccs->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE) 1, (LPARAM)"Connection to Skype lost");
	return 0;
}

int SkypeRecvMessage(WPARAM wParam, LPARAM lParam)
{
    DBEVENTINFO dbei;
    CCSDATA *ccs = (CCSDATA *) lParam;
    PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;

    DBDeleteContactSetting(ccs->hContact, "CList", "Hidden");
    ZeroMemory(&dbei, sizeof(dbei));
    dbei.cbSize = sizeof(dbei);
    dbei.szModule = pszSkypeProtoName;
    dbei.timestamp = pre->timestamp;
    dbei.flags = ((pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0);
    dbei.eventType = EVENTTYPE_MESSAGE;
    dbei.cbBlob = strlen(pre->szMessage) + 1;
    dbei.pBlob = (PBYTE) pre->szMessage;
    CallService(MS_DB_EVENT_ADD, (WPARAM)ccs->hContact, (LPARAM)&dbei);
    return 0;
}

int SkypeSendAuthRequest(WPARAM wParam, LPARAM lParam) {
	CCSDATA* ccs = (CCSDATA*)lParam;
	DBVARIANT dbv;
	int retval;

	if (!ccs->lParam || DBGetContactSetting(ccs->hContact, pszSkypeProtoName, SKYPE_NAME, &dbv))
		return 1;
	retval = SkypeSend("SET USER %s BUDDYSTATUS 2 %s", dbv.pszVal, (char *)ccs->lParam);
	DBFreeVariant(&dbv);
	if (retval) return 1; else return 0;
}

int SkypeRecvAuth(WPARAM wParam, LPARAM lParam) {
	DBEVENTINFO dbei = {0};
	CCSDATA* ccs = (CCSDATA*)lParam;
	PROTORECVEVENT* pre = (PROTORECVEVENT*)ccs->lParam;

	DBDeleteContactSetting(ccs->hContact, "CList", "Hidden");

	dbei.cbSize    = sizeof(dbei);
	dbei.szModule  = pszSkypeProtoName;
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
		strcmp(dbei.szModule, pszSkypeProtoName)) 
	{
		free(dbei.pBlob);
		return NULL;
	}
	return (char *)dbei.pBlob;
}

int SkypeAuthAllow(WPARAM wParam, LPARAM lParam) {
	char *pBlob;

	if (pBlob=__skypeauth(wParam))
	{ 
		int retval=SkypeSend("SET USER %s ISAUTHORIZED TRUE", pBlob+sizeof(DWORD)+sizeof(HANDLE));
		free(pBlob);
		if (!retval) return 0;
	}
	return 1;
}

int SkypeAuthDeny(WPARAM wParam, LPARAM lParam) {
	char *pBlob;

	if (pBlob=__skypeauth(wParam))
	{ 
		int retval=SkypeSend("SET USER %s ISAUTHORIZED FALSE", pBlob+sizeof(DWORD)+sizeof(HANDLE));
		free(pBlob);
		if (!retval) return 0;
	}
	return 1;
}


int SkypeAddToListByEvent(WPARAM wParam, LPARAM lParam) {
	char *pBlob;

	if (pBlob=__skypeauth(wParam))
	{ 
		HANDLE hContact=add_contact(pBlob+sizeof(DWORD)+sizeof(HANDLE), LOWORD(wParam));
		free(pBlob);
		if (hContact) return (int)hContact;
	}
	return 0;
}


void CleanupNicknames(char *dummy) {
	HANDLE hContact;
	char *szProto;
	DBVARIANT dbv, dbv2;

	LOG("CleanupNicknames", "Cleaning up...");
	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, pszSkypeProtoName) &&
			DBGetContactSettingByte(hContact, pszSkypeProtoName, "ChatRoom", 0)==0)	
		{
			if (DBGetContactSetting(hContact, pszSkypeProtoName, SKYPE_NAME, &dbv)) continue;
			if (DBGetContactSetting(hContact, pszSkypeProtoName, "Nick", &dbv2)) {
				DBFreeVariant(&dbv);
				continue;
			}
			if (!strcmp(dbv.pszVal, dbv2.pszVal)) {
				DBDeleteContactSetting(hContact, pszSkypeProtoName, "Nick");
				GetInfoThread(hContact);
			}
			DBFreeVariant(&dbv);
			DBFreeVariant(&dbv2);
		}
	}
	OUTPUT("Cleanup finished.");
}

/////////////////////////////////////////////////////////////////////////////////////////
// EnterBitmapFileName - enters a bitmap filename

int __stdcall EnterBitmapFileName( char* szDest )
{
	*szDest = 0;

	char szFilter[ 512 ];
	CallService( MS_UTILS_GETBITMAPFILTERSTRINGS, sizeof szFilter, ( LPARAM )szFilter );

	char str[ MAX_PATH ]; str[0] = 0;
	OPENFILENAMEA ofn = {0};
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
	SetEvent (SkypeMsgFetched);
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
		if (DBGetContactSetting(hContact, "Protocol", "p", &dbv)) continue;
        tCompareResult = !strcmp(dbv.pszVal, pszSkypeProtoName);
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

	LOG("Updating old database settings if there are any...","");
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

			LOG("We're currently upgrading...","");
			for (i=0;i<pdi.dwCount;i++) {
				if (!DBGetContactSetting(hContact, OldName, pdi.szSettings[i], &dbv)) {
					cws.szModule=pszSkypeProtoName;
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
		if (hContact && !DBGetContactSetting(hContact, "Protocol", "p", &dbv)) {
			if (!strcmp(dbv.pszVal, OldName))
				DBWriteContactSettingString(hContact, "Protocol", "p", pszSkypeProtoName);
			DBFreeVariant(&dbv);
		}
		if (!hContact) break;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	} while (1);
	DBWriteContactSettingByte(NULL, pszSkypeProtoName, "UpgradeDone", (BYTE)1);
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
  WndClass.lpszClassName = "SkypeApiDispatchWindow"; 
  RegisterClass(&WndClass);
  // Do not check the retval of RegisterClass, because on non-unicode
  // win98 it will fail, as it is a stub that returns false() there
	
  // Create main window
  hWnd=CreateWindowEx( WS_EX_APPWINDOW|WS_EX_WINDOWEDGE,
		"SkypeApiDispatchWindow", "", WS_BORDER|WS_SYSMENU|WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 128, 128, NULL, 0, (HINSTANCE)WndClass.hInstance, 0);

  LOGL ("Created Dispatch window with handle", (long)hWnd);
  if (!hWnd) {
	OUTPUT("Cannot create window.");
	TellError(GetLastError());
	SetEvent(MessagePumpReady);
    return; 
  }
  ShowWindow(hWnd, 0); 
  UpdateWindow(hWnd); 
  msgPumpThreadId = GetCurrentThreadId();
  SetEvent(MessagePumpReady);

  LOG ("Messagepump", "started.");
  while (GetMessage (&msg, hWnd, 0, 0) > 0 && !Miranda_Terminated()) {
	  TranslateMessage (&msg);
	  DispatchMessage (&msg);
  }
  UnregisterClass (WndClass.lpszClassName, hInst);
  LOG ("Messagepump", "stopped.");
}

// DLL Stuff //

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
		pluginInfo.cbSize = sizeof(PLUGININFO);
		return (PLUGININFO*) &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MUUID_SKYPE_CALL, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}


extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}


int PreShutdown(WPARAM wParam, LPARAM lParam) {
	PostThreadMessage(msgPumpThreadId, WM_QUIT, 0, 0);
	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
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
	lstrcpyn(pszSkypeProtoName, protocolname, MAX_PATH);
*/	

#ifdef _DEBUG
	init_debug();
#endif

	LOG("Load", "Skype Plugin loading...");
	lstrcpyn(pszSkypeProtoName, "SKYPE", MAX_PATH);

	// We need to upgrade SKYPE_PROTOCOL internal name to Skype if not already done
	if (!DBGetContactSettingByte(NULL, pszSkypeProtoName, "UpgradeDone", 0))
		UpgradeName("SKYPE_PROTOCOL");

    // Initialisation of Skype MsgQueue must be done because of Cleanup in end and
	// Mutex is also initialized here.
	LOG("SkypeMsgInit", "initializing Skype MSG-queue");
	if (SkypeMsgInit()==-1) {
		OUTPUT("Memory allocation error on startup.");
		return 0;
	}

	// On first run on new profile, ask user, if he wants to enable the plugin in
	// this profile
	// --> Fixing Issue #0000006 from bugtracker.
	if (!DBGetContactSettingByte(NULL, pszSkypeProtoName, "FirstRun", 0)) {
		DBWriteContactSettingByte(NULL, pszSkypeProtoName, "FirstRun", 1);
		if (AnySkypeusers()==0) // First run, it seems :)
			if (MessageBox(NULL, Translate("This seems to be the first time that you're running the Skype protocol plugin. Do you want to enable the protocol for this Miranda-Profile? (If you chose NO, you can always enable it in the plugin options later."), "Welcome!", MB_ICONQUESTION|MB_YESNO)==IDNO) {
				char path[MAX_PATH], *filename;
				GetModuleFileName(hInst, path, sizeof(path));
				if (filename = strrchr(path,'\\')+1)
					DBWriteContactSettingByte(NULL,"PluginDisable",filename,1);
				return 0;
			}
	}


	// Check if Skype is installed
	SkypeInstalled=TRUE;
	UseCustomCommand = (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseCustomCommand", 0);
	UseSockets = (BOOL)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseSkype2Socket", 0);

	if (!UseSockets && !UseCustomCommand) 
	{
		if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Skype\\Phone", 0, KEY_READ, &MyKey)!=ERROR_SUCCESS)
		{
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Skype\\Phone", 0, KEY_READ, &MyKey)!=ERROR_SUCCESS)
			{
				SkypeInstalled=FALSE;
			}
		}
		
		Buffsize=sizeof(skype_path);
		
		if (SkypeInstalled==FALSE || RegQueryValueEx(MyKey, "SkypePath", NULL, NULL, (unsigned char *)skype_path,  &Buffsize)!=ERROR_SUCCESS) 
		{
			    OUTPUT("Skype was not found installed :( \nMaybe you are using portable skype.");
				RegCloseKey(MyKey);
				skype_path[0]=0;
				//return 0;
		}
		RegCloseKey(MyKey);
	}
	WSAStartup(MAKEWORD(2,2), &wsaData);

	// Start Skype connection 
	if (!(ControlAPIAttach=RegisterWindowMessage("SkypeControlAPIAttach")) || !(ControlAPIDiscover=RegisterWindowMessage("SkypeControlAPIDiscover"))) 
	{
			OUTPUT("Cannot register Window message.");
			return 0;
	}
	
	SkypeMsgReceived=CreateSemaphore(NULL, 0, MAX_MSGS, NULL);
	if (!(SkypeReady=CreateEvent(NULL, TRUE, FALSE, NULL)) ||
		!(SkypeMsgFetched=CreateEvent(NULL, FALSE, FALSE, NULL)) ||
		!(MessagePumpReady=CreateEvent(NULL, FALSE, FALSE, NULL)) ||
#ifdef SKYPEBUG_OFFLN
	    !(GotUserstatus=CreateEvent(NULL, TRUE, FALSE, NULL)) ||
#endif
		!(hBuddyAdded=CreateEvent(NULL, FALSE, FALSE, NULL))) {
		 OUTPUT("Unable to create Mutex!");
		return 0;
	}

	/* Register the module */
	ZeroMemory(&pd, sizeof(pd));
	pd.cbSize = sizeof(pd);
	pd.szName = pszSkypeProtoName;
	pd.type   = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	VoiceServiceInit();

	CreateServices();
	HookEvents();
	InitVSApi();

	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	// Startup Message-pump
    pthread_create (( pThreadFunc )MsgPump, NULL);
	WaitForSingleObject(MessagePumpReady, INFINITE);
	return 0;

}



extern "C" int __declspec( dllexport ) Unload(void) 
{
	BOOL UseCustomCommand = DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseCustomCommand", 0);
	BOOL Shutdown = DBGetContactSettingByte(NULL, pszSkypeProtoName, "Shutdown", 0);
	
	LOG ("Unload", "started");
	
	if ( Shutdown && ((skype_path && skype_path[0]) ||UseCustomCommand) ) {

		if(UseCustomCommand)
		{
			DBVARIANT dbv;
			if(!DBGetContactSetting(NULL,pszSkypeProtoName,"CommandLine",&dbv)) 
			{
				_spawnl(_P_NOWAIT, dbv.pszVal, dbv.pszVal, "/SHUTDOWN", NULL);
				LOG ("Unload Sent /shutdown to ", dbv.pszVal);
				DBFreeVariant(&dbv);
			}
		}
		else
		{
			_spawnl(_P_NOWAIT, skype_path, skype_path, "/SHUTDOWN", NULL);
			LOG ("Unload Sent /shutdown to ", skype_path);
		}
		
	}
	SkypeMsgCleanup();
	WSACleanup();
	FreeVSApi();
	if (hOptHook) UnhookEvent(hOptHook);
	if (hChatEvent) UnhookEvent(hChatEvent);
	if (hChatMenu) UnhookEvent (hChatMenu);
	if (hEvInitChat) UnhookEvent (hEvInitChat);
	if (hInitChat) DestroyHookableEvent(hInitChat);
	if (hTTBModuleLoadedHook) UnhookEvent(hTTBModuleLoadedHook);

	if (hStatusHookContact) {
		UnhookEvent(hStatusHookContact);
		UnhookEvent(hContactDeleted);
		UnhookEvent(hHookModulesLoaded);
		UnhookEvent(hPrebuildCMenu);
		UnhookEvent(hHookOkToExit);
		UnhookEvent(hHookMirandaExit);
		//UnhookEvent(ClistDblClick);
		CloseHandle(SkypeReady);
		CloseHandle(SkypeMsgReceived);
		CloseHandle(SkypeMsgFetched);
#ifdef SKYPEBUG_OFFLN
	    CloseHandle(GotUserstatus);
#endif
		CloseHandle(MessagePumpReady);
		CloseHandle(hBuddyAdded);
	}
	LOG ("Unload", "Shutdown complete");
#ifdef _DEBUG
	end_debug();
#endif
	return 0;
}

