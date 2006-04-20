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
#include "contacts.h"
#include "utf8.h"
#include "pthread.h"
#include "gchat.h"

#include "../../include/m_utils.h"
#include "../../include/m_options.h"
#include "../../include/m_langpack.h"
#include "../../include/m_userinfo.h"
#include "../../include/m_avatars.h"
#include "m_toptoolbar.h"

// Exported Globals
HWND hSkypeWnd=NULL, hWnd=NULL;
HANDLE SkypeReady, SkypeMsgReceived, hOptHook,hHookOnUserInfoInit, MessagePumpReady, hInitChat=NULL, httbButton=NULL;
BOOL SkypeInitialized=FALSE, QueryMsgDirection=FALSE, MirandaShuttingDown=FALSE;
BOOL UseSockets=FALSE, bSkypeOut=FALSE;
char pszSkypeProtoName[MAX_PATH+30], skype_path[MAX_PATH], protocol=2;
int SkypeStatus=ID_STATUS_OFFLINE, hSearchThread=-1, receivers=1;
UINT ControlAPIAttach, ControlAPIDiscover;
LONG AttachStatus=-1;
HINSTANCE hInst;


// Module Internal Globals
PLUGINLINK *pluginLink;
HANDLE hThread, hStatusHookContact, hHookModulesLoaded, hPingPong=NULL;
HANDLE SkypeMsgFetched, hHookOkToExit, hPrebuildCMenu, hChatEvent, hChatMenu;
HANDLE hEvInitChat=NULL, hBuddyAdded, hTTBModuleLoadedHook=NULL, hContactDeleted=NULL;
HANDLE hMenuAddSkypeContact=NULL;

#ifdef SKYPEBUG_OFFLN
HANDLE GotUserstatus;
#endif

BOOL ImportingHistory=FALSE, bModulesLoaded=FALSE;
char *RequestedStatus=NULL;	// To fix Skype-API Statusmode-bug
char cmdMessage[16]="MESSAGE", cmdPartner[8]="PARTNER";	// Compatibility commands

// Imported Globals
extern status_map status_codes[];

static HBITMAP hAvatar = NULL;

// Plugin Info
PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"Skype protocol",
	PLUGIN_MAKE_VERSION(0,0,0,19),
	"Support for Skype network",
	"leecher",
	"leecher@dose.0wnz.at",
	"© 2004-2005 leecher",
	"http://dose.0wnz.at",
	0,		//not transient
	0		//doesn't replace anything built-in
};

#define MAPDND	1	// Map Occupied to DND status and say that you support it

/*                           P R O G R A M                                */


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
    POPUPDATAEX pud={0};

	if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "SuppressErrors", 0)) return -1;
	lpzText=Translate(lpzText);
#ifdef USEPOPUP
	if (bModulesLoaded && ServiceExists(MS_POPUP_ADDPOPUP) &&
		DBGetContactSettingByte(NULL, pszSkypeProtoName, "UsePopup", 0)
	   ) {
		pud.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(iconID));
		strncpy(pud.lpzContactName, pluginInfo.shortName, MAX_CONTACTNAME);
		strncpy(pud.lpzText, lpzText, MAX_SECONDLINE);
		pud.iSeconds = mustShow==1?-1:0;
		pud.lpzClass = mustShow==1?POPUP_CLASS_WARNING:POPUP_CLASS_DEFAULT;
		if (PUAddPopUpEx(&pud)<0) {
			if (mustShow==1) MessageBox(NULL,lpzText,pluginInfo.shortName, MB_OK | MB_ICONWARNING);
			return -1;
		}
		if (mustShow==1) MessageBeep(-1);
		return 0;
	} else {
#endif
		if (mustShow==1) MessageBox(NULL,lpzText,pluginInfo.shortName, MB_OK | MB_ICONWARNING);
#ifdef USEPOPUP
		return 0;
	}
#else
	return -1;
#endif

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

void PingPong(void) {
	while (1) {
		Sleep(PING_INTERVAL);
		if (!hPingPong) return;
		if (SkypeSend("PING")==-1) {
			if (ConnectToSkypeAPI(NULL)!=-1) pthread_create(( pThreadFunc )SkypeSystemInit, NULL);
		}
		testfor("PONG", PING_INTERVAL);
		SkypeMsgCollectGarbage(MAX_MSG_AGE);
	}
}

void GetInfoThread(HANDLE hContact) {
	DBVARIANT dbv;
	int eol, i=0, len;
	char str[19+MAX_USERLEN], *ptr, buf[5], *nm, *utfdstr=NULL;
	struct CountryListEntry *countries;
	int countryCount;
	settings_map settings[]= {
		{"LANGUAGE", "Language1"},
		{"PROVINCE", "State"},
		{"CITY", "City"},
		{"PHONE_HOME", "Phone"},
		{"PHONE_OFFICE", "CompanyPhone"},
		{"PHONE_MOBILE", "Cellucar"},
		{"HOMEPAGE", "Homepage"},
		{"ABOUT", "About"},
		{NULL, NULL}
	};
    
	if (DBGetContactSetting(hContact, pszSkypeProtoName, SKYPE_NAME, &dbv) ||
		(len=strlen(dbv.pszVal))>MAX_USERLEN) return;

	eol=10+len;
	sprintf(str, "GET USER %s FULLNAME", dbv.pszVal);
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
				if (!stricmp(countries[i].szName, ptr+strlen(str+3))) 
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
			if (!stricmp(ptr+strlen(str+3), "MALE")) sex=0x4D;
			if (!stricmp(ptr+strlen(str+3), "FEMALE")) sex=0x46;
			if (sex) DBWriteContactSettingByte(hContact, pszSkypeProtoName, "Gender", sex);
		}
		free(ptr);
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
}

void BasicSearchThread(char *nick) {
	PROTOSEARCHRESULT psr={0};
	char *cmd, *token, *ptr;

	if (SkypeSend("SEARCH USERS %s", nick) || !(cmd=SkypeRcv("USERS", INFINITE))) {
		ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)hSearchThread, 0);
		return;
	} 
	if (!strncmp(cmd, "ERROR", 5)) {
		OUT(cmd);
		free(cmd);
		ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)hSearchThread, 0);
		return;
	}
	token=strtok(cmd+5, ", ");
	psr.cbSize=sizeof(psr);
	while (token) {
		psr.nick=strdup(token);
		psr.lastName=NULL;
		psr.firstName=NULL;
		psr.email=NULL;
		if (ptr=SkypeGet("USER", token, "FULLNAME")) {
			// We cannot use strtok() to seperate first & last name here,
			// because we already use it for parsing the user list
			// So we use our own function
			if (psr.lastName=strchr(ptr, ' ')) {
				*psr.lastName=0;
				psr.lastName=strdup(psr.lastName+1);
				LOG("lastName", psr.lastName);
			}
			psr.firstName=strdup(ptr);
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
	return;
}

// added by TioDuke
void GetDisplaynameThread(char *dummy) {
	DBVARIANT dbv;
	char *ptr, *utfdstr=NULL;
    
	if (DBGetContactSetting(NULL, pszSkypeProtoName, SKYPE_NAME, &dbv)) return;
    if ((ptr=SkypeGet("USER", dbv.pszVal, "FULLNAME"))) {
		if (utf8_decode(ptr, &utfdstr)!=-1 && utfdstr) {
			DBWriteContactSettingString(NULL, pszSkypeProtoName, "Nick", utfdstr);
			free(utfdstr);
		}
		free(ptr);
	}
	DBFreeVariant(&dbv);
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
	SkypeInitialized=FALSE;
	SearchFriends();
	SkypeInitialized=TRUE;
}

void __cdecl SkypeSystemInit(char *dummy) {
	char buf[48];
	DWORD ThreadID;
	static BOOL Initializing=FALSE;

	if (SkypeInitialized || Initializing) return;
	Initializing=TRUE;
// Do initial Skype-Tasks
	logoff_contacts();
// Add friends

	if (SkypeSend(SKYPE_PROTO)==-1 || !testfor("PROTOCOL", INFINITE) ||
		SkypeSend("GET PRIVILEGE SKYPEOUT")==-1) {
		Initializing=FALSE;
		return;	
	}

#ifdef SKYPEBUG_OFFLN
    if (!ResetEvent(GotUserstatus) || SkypeSend("GET USERSTATUS")==-1 || 
		WaitForSingleObject(GotUserstatus, INFINITE)==WAIT_FAILED) 
	{
		Initializing=FALSE;
		return;
	}
	if (SkypeStatus!=ID_STATUS_OFFLINE)
#endif
	if (SearchFriends()==-1) {
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
		Initializing=FALSE;
		return;
	}
	if (!hPingPong) hPingPong=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PingPong, NULL, 0, &ThreadID); 
	SkypeInitialized=TRUE;
	Initializing=FALSE;
	return;
}

void FirstLaunch(char *dummy) {
	int counter=0;

	if (!DBGetContactSettingByte(NULL, pszSkypeProtoName, "StartSkype", 1) || ConnectToSkypeAPI(skype_path)==-1) {
		int oldstatus=SkypeStatus;

		LOG("OnModulesLoaded", "starting offline..");	
		InterlockedExchange((long*)&SkypeStatus, ID_STATUS_OFFLINE);
		ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldstatus, SkypeStatus);
	}
	if (AttachStatus==-1) return;
	
	// When you launch Skype and Attach is Successfull, it still takes some time
	// until it becomes available for receiving messages.
	// Let's probe this with PINGing
	LOG("CheckIfApiIsResponding", "Entering test loop");
	while (1) {
		LOGL("Test #", counter);
		if (SkypeSend("PING")==-1) counter ++; else break;
		if (counter>=20) {
			OUTPUT("Cannot reach Skype API, plugin disfunct.");
			return;
		}
		Sleep(500);
	}
	LOG("CheckIfApiIsResponding", "Testing for PONG");
	testfor("PONG", 2000); // Flush PONG from MsgQueue

	pthread_create(( pThreadFunc )SkypeSystemInit, NULL);
}

static int CreateTopToolbarButton(WPARAM wParam, LPARAM lParam) {
	TTBButton ttb={0};
	
	ttb.cbSize = sizeof(ttb);
	ttb.dwFlags = TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP|TTBBF_DRAWBORDER;
	ttb.hbBitmapDown = ttb.hbBitmapUp = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_CALL));
	ttb.pszServiceDown = ttb.pszServiceUp = SKYPEOUT_CALL;
	ttb.name=Translate("Do a SkypeOut-call");
	if ((int)(httbButton=(HANDLE)CallService(MS_TTB_ADDBUTTON, (WPARAM)&ttb, 0))==-1) httbButton=0;
	return 0;
}


static int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {
	bModulesLoaded=TRUE;
	
	add_contextmenu(NULL);
	if ( ServiceExists( MS_GC_REGISTER )) {
		GCREGISTER gcr = {0};
		static COLORREF crCols[1] = {0};
		char *szEvent;

		gcr.cbSize = sizeof( GCREGISTER );
		gcr.dwFlags = GC_CHANMGR; // |GC_ACKMSG; // TODO: Not implemented yet
/*		gcr.iMaxText = 0;
		gcr.nColors = 0;
		gcr.pColors = &crCols[0];
*/		gcr.pszModuleDispName = pszSkypeProtoName;
		gcr.pszModule = pszSkypeProtoName;
		if (CallService(MS_GC_REGISTER, 0, (LPARAM)&gcr)) {
			OUTPUT("Unable to register with Groupchat module!");
		}
		if (szEvent=(char*)malloc(strlen(pszSkypeProtoName)+10)) {
			_snprintf(szEvent, sizeof szEvent, "%s\\ChatInit", pszSkypeProtoName);
			hInitChat = CreateHookableEvent(szEvent);
			hEvInitChat = HookEvent(szEvent, ChatInit);
			free(szEvent);
		} else { OUTPUT("Out of memory!"); }

		hChatEvent = HookEvent(ME_GC_EVENT, GCEventHook);
		hChatMenu = HookEvent(ME_GC_BUILDMENU, GCMenuHook);
	}
	// We cannot check for the TTB-service before this event gets fired... :-/
	hTTBModuleLoadedHook = HookEvent(ME_TTB_MODULELOADED, CreateTopToolbarButton);
	hHookOnUserInfoInit = HookEvent( ME_USERINFO_INITIALISE, OnDetailsInit );
	pthread_create(( pThreadFunc )FirstLaunch, NULL);
	return 0;
}


void FetchMessageThread(fetchmsg_arg *args) {
	char str[64], *ptr, *who, *msg;
	int msgl, direction=0;
	DWORD timestamp, lwr=0;
    CCSDATA ccs;
    PROTORECVEVENT pre;
    HANDLE hContact, hDbEvent;
	DBEVENTINFO dbei={0};
	BOOL getstatus;

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
	if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
		SetEvent(SkypeMsgFetched);
		return;
	}
	ERRCHK
	str[msgl]=0;
	if (strncmp(ptr+strlen(ptr)-4, "TEXT", 4) && strncmp(ptr+strlen(ptr)-4, "SAID", 4)) {
	  if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseGroupchat", 0)) {
		if (!strncmp(ptr+strlen(ptr)-10,"SAWMEMBERS", 10)) {
			// We have a new Groupchat
			free(ptr);
			strcat(str, "CHATNAME");
			if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
				SetEvent(SkypeMsgFetched);
				return;
			}

			ChatStart(ptr+strlen(str+4)+1);
			free(ptr);
			SetEvent(SkypeMsgFetched);
			return;
		}
		if (!strncmp(ptr+strlen(ptr)-8,"SETTOPIC", 8)) {
			GCDEST gcd = {0};
			GCEVENT gce = {0};
			char *ptr2;

			free(ptr);
			strcat(str, "CHATNAME");
			if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
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
			if (SkypeSend(str)==-1 || !(who=SkypeRcv(str+4, INFINITE))) {
				free(ptr);
				SetEvent(SkypeMsgFetched);
				return;
			}
			gce.pszUID = who+strlen(str+4)+1;
			sprintf(str, "CHAT %s TOPIC", gcd.pszID);
			if (ptr2=SkypeRcv(str, INFINITE)) {
				gce.pszText = ptr2+strlen(str);
				CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
				free(ptr2);
			}
			free(ptr);
			free(who);
			SetEvent(SkypeMsgFetched);
			return;
		}
		if (!strncmp(ptr+strlen(ptr)-4,"LEFT", 4) ||
			!strncmp(ptr+strlen(ptr)-12,"ADDEDMEMBERS", 12)) {

			free(ptr);
			strcat(str, "CHATNAME");
			if (SkypeSend(str)==-1 || !(ptr=SkypeRcv(str+4, INFINITE))) {
				SetEvent(SkypeMsgFetched);
				return;
			}
			AddMembers(ptr+strlen(str+4)+1);
			str[msgl]=0;

			free(ptr);
			SetEvent(SkypeMsgFetched);
			return;
		}
	  }
		// Boo,no useful message, ignore it
#ifdef _DEBUG
//		OUTPUT(ptr);
//		OUTPUT("NO TEXT! :(");
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
	who=strdup(ptr+strlen(str+4)+1);
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

	if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseGroupchat", 0)) {
		// Is it a groupchat message?
		char *ptr3;

		strcat(str, "CHATNAME");
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
		}
		if (!strcmp(ptr3, "MULTI_SUBSCRIBED")) {
			GCDEST gcd = {0};
			GCEVENT gce = {0};
			DBVARIANT dbv = {0};
			HANDLE hContact;

			gcd.pszModule = pszSkypeProtoName;
			gcd.pszID = ptr+strlen(str+4)+1;
			gcd.iType = GC_EVENT_MESSAGE;
			gce.cbSize = sizeof(GCEVENT);
			gce.pDest = &gcd;
			gce.pszUID = who;
			if ((hContact=find_contact(who)) && 
				!DBGetContactSetting(hContact, pszSkypeProtoName, "Nick", &dbv)
			   ) gce.pszNick = dbv.pszVal; else gce.pszNick=who;
			gce.time = timestamp>0?timestamp:time(NULL);;
			gce.bIsMe = FALSE;
			gce.pszText = msg;
			gce.bAddToLog = TRUE;
			CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
			if (dbv.pszVal) DBFreeVariant(&dbv);

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
		free(ptr);
		free(ptr3);
	}

	if (!QueryMsgDirection || (QueryMsgDirection && timestamp>dbei.timestamp)) {
		LOG("FetchMessageThread", "Normal message add...");
		// Normal message received, process it
	    ccs.szProtoService = PSR_MESSAGE;
	    ccs.hContact = hContact;
		ccs.wParam = 0;
		ccs.lParam = (LPARAM)&pre;
		pre.flags = direction;
		pre.timestamp = timestamp>0?timestamp:time(NULL);
		pre.szMessage = msg;
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
			args->msgnum=strdup(token+1);
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

void RingThread(char *szSkypeMsg) {
	HANDLE hContact;
	DBEVENTINFO dbei={0};
	DBVARIANT dbv;
	char *ptr;


	if (hContact=GetCallerContact(szSkypeMsg)) {
		// Make sure that an answering thread is not already in progress so that we don't get
		// the 'Incoming call' event twice
		if (!DBGetContactSetting(hContact, pszSkypeProtoName, "CallId", &dbv)) {
			DBFreeVariant(&dbv);
			free(szSkypeMsg);
			return;
		}
		DBWriteContactSettingString(hContact, pszSkypeProtoName, "CallId", szSkypeMsg);
	}
	
	if (!(ptr=SkypeGet(szSkypeMsg, "TYPE", ""))) {
		free(szSkypeMsg);
		return;
	}
	dbei.cbSize=sizeof(dbei);
	dbei.eventType=EVENTTYPE_CALL;
	dbei.szModule=pszSkypeProtoName;
	dbei.timestamp=time(NULL);
	dbei.pBlob=(unsigned char*)Translate("Phonecall");
	dbei.cbBlob=lstrlen((const char*)dbei.pBlob)+1;
	if (!strncmp(ptr, "INCOMING", 8)) {
		CLISTEVENT cle={0};
		char toolTip[256];

		cle.cbSize=sizeof(cle);
		cle.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALL));
		cle.pszService=SKYPE_ANSWERCALL;
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
		cle.hContact=hContact;
		cle.hDbEvent=(HANDLE)CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&dbei);
		_snprintf(toolTip,sizeof(toolTip),Translate("Incoming call from %s"),(char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0));
		cle.pszTooltip=toolTip;
		CallServiceSync(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
	} else {
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

	dbei.cbSize=sizeof(dbei);
	DBDeleteContactSetting(hContact, pszSkypeProtoName, "CallId");
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
	if (!szSkypeMsg) return;
	if (hContact=GetCallerContact(szSkypeMsg))
		DBWriteContactSettingByte(hContact, pszSkypeProtoName, "OnHold", 1);
	free(szSkypeMsg);
	LOG("HoldCallThread", "done");
}

void ResumeCallThread(char *szSkypeMsg) {
	HANDLE hContact;

	if (!szSkypeMsg) return;
	if (hContact=GetCallerContact(szSkypeMsg))
		DBDeleteContactSetting(hContact, pszSkypeProtoName, "OnHold");
	free(szSkypeMsg);
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
	BYTE startSkype;

/*	   if (!DBGetContactSettingByte(NULL, pszSkypeProtoName, "UnloadOnOffline", 0)) {
		   logoff_contacts();
		   return 1;
	   }
*/
	InterlockedExchange((long*)&SkypeStatus, ID_STATUS_CONNECTING);
	ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) ID_STATUS_OFFLINE, SkypeStatus);
	   
	startSkype=DBGetContactSettingByte(NULL, pszSkypeProtoName, "StartSkype", 1);
	if (!startSkype) DBWriteContactSettingByte(NULL, pszSkypeProtoName, "StartSkype", 1);
	if (ConnectToSkypeAPI(skype_path)!=-1) {
		int oldStatus=SkypeStatus;
		pthread_create(( pThreadFunc )SkypeSystemInit, NULL);
		InterlockedExchange((long*)&SkypeStatus, (int)newStatus);
		ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, SkypeStatus);
	} else {
	   InterlockedExchange((long*)&SkypeStatus, ID_STATUS_OFFLINE);
	   ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) ID_STATUS_CONNECTING, SkypeStatus);
	}
	DBWriteContactSettingByte(NULL, pszSkypeProtoName, "StartSkype", startSkype);
	SetUserStatus();
}

LONG APIENTRY WndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam) 
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
//		 LOG("WM_COPYDATA", "start");
		 if(hSkypeWnd==(HWND)wParam) { 
			CopyData=(PCOPYDATASTRUCT)lParam;
			szSkypeMsg=strdup((char*)CopyData->lpData);
			ReplyMessage(1);
			LOG("<", szSkypeMsg);

 			if (!strncmp(szSkypeMsg, "CONNSTATUS", 10)) {
				if (!strncmp(szSkypeMsg+11, "LOGGEDOUT", 9)) {
					SkypeInitialized=FALSE;
					ResetEvent(SkypeReady);
					AttachStatus=-1;
					sstat=ID_STATUS_OFFLINE;
				    CloseHandle(hPingPong);
					hPingPong=NULL;
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
							char cStr[30];

							// To fix Skype Statusmode-Bug
							lstrcpy(cStr, "SET USERSTATUS ");
							lstrcat(cStr, RequestedStatus);
							SkypeSend(cStr);

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
				buf=strdup(szSkypeMsg+5);
				nick=strtok(buf, " ");
				ptr=strtok(NULL, " ");
				if (!strcmp(ptr, "ONLINESTATUS")) {
					//if (SkypeStatus==ID_STATUS_OFFLINE) break;
					if (!(hContact=find_contact(nick))) {

						SkypeSend("GET USER %s BUDDYSTATUS", nick);
//						break;
					} else
						DBWriteContactSettingWord(hContact, pszSkypeProtoName, "Status", (WORD)SkypeStatusToMiranda(ptr+13));
/*						free(buf);
					if (SkypeInitialized==FALSE) { // Prevent flooding on startup
						SkypeMsgAdd(szSkypeMsg);
						ReleaseSemaphore(SkypeMsgReceived, receivers, NULL);
					}
					break;
*/				}
				if (!strcmp(ptr, "DISPLAYNAME")) {
					// Skype Bug? -> If nickname isn't customised in the Skype-App, this won't return anything :-(
					if (ptr[12]) DBWriteContactSettingString(find_contact(nick), pszSkypeProtoName, "Nick", ptr+12);
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
				 strstr(szSkypeMsg, "AUTOAWAY") || !strncmp(szSkypeMsg, "OPEN ",5)) {
				// Currently we do not process these messages 
				break;
			}
			if (!strncmp(szSkypeMsg, "CHAT ", 5)) {
				// Currently we only process these notifications
				if (!DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseGroupchat", 0)) break;
				// Throw away old unseen messages to reduce memory-usage
				if (ptr=strstr(szSkypeMsg, " TOPIC")) {
					ptr[6]=0;
					while (testfor(szSkypeMsg, 0));
					ptr[6]=' ';
				} else
				if (ptr=strstr(szSkypeMsg, " MEMBERS")) {
					ptr[8]=0;
					while (testfor(szSkypeMsg, 0));
					ptr[8]=' ';
				} else
				if (ptr=strstr(szSkypeMsg, " STATUS")) {
					ptr[7]=0;
					while (testfor(szSkypeMsg, 0));
					ptr[7]=' ';
				} else break;
			}
			if (!strncmp(szSkypeMsg, "CALL ",5)) {
				// incoming calls are already processed by Skype, so no need for us
				// to do this.
				// However we can give a user the possibility to hang up a call via Miranda's
				// context menu
				if (ptr=strstr(szSkypeMsg, " STATUS ")) {
					ptr[0]=0; ptr+=8;
					if (!strcmp(ptr, "RINGING") || !strcmp(ptr, "ROUTING")) pthread_create(( pThreadFunc )RingThread, strdup(szSkypeMsg));
					if (!strcmp(ptr, "FAILED") || !strcmp(ptr, "FINISHED") ||
						!strcmp(ptr, "MISSED") || !strcmp(ptr, "REFUSED")  ||
						!strcmp(ptr, "BUSY")   || !strcmp(ptr, "CANCELLED"))
							pthread_create(( pThreadFunc )EndCallThread, strdup(szSkypeMsg));
					if (!strcmp(ptr, "ONHOLD") || !strcmp(ptr, "LOCALHOLD") ||
						!strcmp(ptr, "REMOTEHOLD")) pthread_create(( pThreadFunc )HoldCallThread, strdup(szSkypeMsg));
					if (!strcmp(ptr, "INPROGRESS")) pthread_create(( pThreadFunc )ResumeCallThread, strdup(szSkypeMsg));
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

			if (!strncmp(szSkypeMsg, cmdMessage, strlen(cmdMessage))
				 && (ptr=strstr(szSkypeMsg, " STATUS RECEIVED"))!=NULL) {
			  // If new message is available, fetch it
			  ptr[0]=0;
			  if (!(args=(fetchmsg_arg *)malloc(sizeof(*args)))) break;
			  args->msgnum=strdup(strchr(szSkypeMsg, ' ')+1);
			  args->getstatus=FALSE;
			  pthread_create(( pThreadFunc )FetchMessageThread, args);
			  break;
			}
			if (!strncmp(szSkypeMsg, "MESSAGES", 8) || !strncmp(szSkypeMsg, "CHATMESSAGES", 12)) {
				if (strlen(szSkypeMsg)<=(UINT)(strchr(szSkypeMsg, ' ')-szSkypeMsg+1)) break;
				pthread_create(( pThreadFunc )MessageListProcessingThread, strdup(strchr(szSkypeMsg, ' ')));
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

        case WM_DESTROY: 
            PostQuitMessage(0); 
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
		 return (DefWindowProc(hWnd, message, wParam, lParam)); 
    }
//	LOG("WM_COPYDATA", "exit");
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

DWORD WINAPI ThreadFunc(VOID) 
{ 
    MSG      Message; 
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


    if (!hWnd) {
		OUTPUT("Cannot create window.");
		TellError(GetLastError());
		CloseHandle(WndClass.hInstance);
		SetEvent(MessagePumpReady);
        return FALSE; 
	}
    ShowWindow(hWnd, 0); 
    UpdateWindow(hWnd); 

	// Unnecessary code
/*	if (!UseSockets) {
		if(!SendMessageTimeout(HWND_BROADCAST, ControlAPIDiscover, (WPARAM)hWnd, 0, SMTO_ABORTIFHUNG, 3000, NULL)) {
			 OUTPUT("Cannot broadcast our window handle");
			CloseHandle(WndClass.hInstance);
			SetEvent(MessagePumpReady);
			return FALSE;
		}
	}
*/
	SetEvent(MessagePumpReady);
    while (GetMessage(&Message, NULL, 0, 0)) 
    { 
        TranslateMessage(&Message); 
        DispatchMessage(&Message);
    }
	LOG("ThreadFunc", "Messagepump stopped");
	CloseHandle(WndClass.hInstance);
    return (Message.wParam); 
}   



// SERVICES //

int SkypeGetCaps(WPARAM wParam, LPARAM lParam) {
    int ret = 0;
    switch (wParam) {        
        case PFLAGNUM_1:
			ret = PF1_BASICSEARCH | PF1_IM | PF1_MODEMSG; // | PF1_AUTHREQ;
			if (protocol>=5) ret |= PF1_ADDSEARCHRES;
            break;

        case PFLAGNUM_2:
            ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_INVISIBLE | PF2_HEAVYDND | PF2_FREECHAT; 
#ifdef MAPDND
	ret |= PF2_LIGHTDND;
#endif		
            break;

        case PFLAGNUM_3:
			ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_ONTHEPHONE;
            break;
            
        case PFLAGNUM_4:
            ret = PF4_FORCEAUTH | PF4_FORCEADDED | PF4_AVATARS;
            break;
        case PFLAG_UNIQUEIDTEXT:
            ret = (int) "NAME";
            break;
        case PFLAG_UNIQUEIDSETTING:
            ret = (int) SKYPE_NAME;
            break;
    }
    return ret;
		
}

int SkypeGetName(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
	{
		lstrcpyn((char *)lParam, pszSkypeProtoName, wParam);
		return 0; // Success
	}
	return 1; // Failure
}


int SkypeLoadIcon(WPARAM wParam,LPARAM lParam)
{
	UINT id;

	switch(wParam&0xFFFF) {
		case PLI_PROTOCOL: id=IDI_SKYPE; break; // IDI_MAIN is the main icon for the protocol
		default: return (int)(HICON)NULL;	
	}
	return (int)LoadImage(hInst,MAKEINTRESOURCE(id),IMAGE_ICON,GetSystemMetrics(wParam&PLIF_SMALL?SM_CXSMICON:SM_CXICON),GetSystemMetrics(wParam&PLIF_SMALL?SM_CYSMICON:SM_CYICON),0);
}

int SkypeSetStatus(WPARAM wParam, LPARAM lParam)
{
	int oldStatus;

	//if (!SkypeInitialized && !DBGetContactSettingByte(NULL, pszSkypeProtoName, "UnloadOnOffline", 0)) return 0;

	// Workaround for Skype status-bug
    if ((int)wParam==ID_STATUS_OFFLINE) logoff_contacts();
	if (SkypeStatus==(int)wParam) return 0;
	oldStatus = SkypeStatus;

	if ((int)wParam==ID_STATUS_CONNECTING) return 0;
#ifdef MAPDND
	if ((int)wParam==ID_STATUS_OCCUPIED || 
		(int)wParam==ID_STATUS_ONTHEPHONE) wParam=ID_STATUS_DND;
	if ((int)wParam==ID_STATUS_OUTTOLUNCH) wParam=ID_STATUS_NA;
#endif

   RequestedStatus=MirandaStatusToSkype((int)wParam);
   if (MirandaShuttingDown) return 0;
   if ((int)wParam==ID_STATUS_OFFLINE) {
	   if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "UnloadOnOffline", 0)) {
		   if (AttachStatus!=-1) _spawnl(_P_NOWAIT, skype_path, skype_path, "/SHUTDOWN", NULL);
		   InterlockedExchange((long*)&SkypeStatus, (int)wParam);
		   ProtoBroadcastAck(pszSkypeProtoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, SkypeStatus);
	 	   SkypeInitialized=FALSE;
		   ResetEvent(SkypeReady);
		   AttachStatus=-1;
		   CloseHandle(hPingPong);
		   hPingPong=NULL;
		   return 0;
	   }
   } else if (AttachStatus==-1) {
	   pthread_create(LaunchSkypeAndSetStatusThread, (void *)wParam);
	   return 0;
   }

   return SetUserStatus(); 
}

int SkypeGetAwayMessage(WPARAM wParam,LPARAM lParam)
{
	return 0;
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
	if (!DBGetContactSetting(NULL,pszSkypeProtoName, "AvatarFile", &dbv) && (AI->hContact == NULL)){
		lstrcpynA(AI->filename, dbv.pszVal, sizeof(AI->filename));
		DBFreeVariant(&dbv);
		return GAIR_SUCCESS;
	}
	else
		return GAIR_NOAVATAR;


	if (( wParam & GAIF_FORCE ) != 0 && AI->hContact != NULL ) {
		return GAIR_WAITFOR;
	}

	return GAIR_NOAVATAR;
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
	return (hSearchThread=pthread_create(( pThreadFunc )BasicSearchThread, strdup((char *)lParam)));
}

void MessageSendWatchThread(HANDLE hContact) {
	char *str, *err;

	if (!(str=SkypeRcv("\0STATUS SENT", DBGetContactSettingDword(NULL,"SRMsg","MessageTimeout",TIMEOUT_MSGSEND)+1000))) return;
	if (err=GetSkypeErrorMsg(str)) {
		ProtoBroadcastAck(pszSkypeProtoName, hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE) 1, (LPARAM)Translate(err));
		free(err);
		free(str);
		return;
	}
	ProtoBroadcastAck(pszSkypeProtoName, hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
	free(str);
}

int SkypeSendMessage(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	DBVARIANT dbv;
    char *msg = (char *) ccs->lParam, *utfmsg=NULL;
    
	if (!DBGetContactSetting(ccs->hContact, pszSkypeProtoName, SKYPE_NAME, &dbv) ||
		!DBGetContactSetting(ccs->hContact, pszSkypeProtoName, "ChatRoomID", &dbv)) {
		BOOL sendok=TRUE;

		if (utf8_encode(msg, &utfmsg)==-1 || !utfmsg || 
			SkypeSend("MESSAGE %s %s", dbv.pszVal, utfmsg)) sendok=FALSE;

		free(utfmsg);
		DBFreeVariant(&dbv);

		if (sendok) {
			pthread_create(MessageSendWatchThread, ccs->hContact);
			return 1;
		}
		ProtoBroadcastAck(pszSkypeProtoName, ccs->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE) 1, (LPARAM)"Connection to Skype lost");
		return 0;
	}
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
    dbei.flags = pre->flags & (PREF_CREATEREAD ? DBEF_READ : 0);
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
	dbei.flags     = pre->flags & (PREF_CREATEREAD?DBEF_READ:0);
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

/*AvatarDlgProc
*
* For setting the skype avatar
*
*/
BOOL CALLBACK AvatarDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static RECT r;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		hAvatar = NULL;
		char tBuffer[ MAX_PATH ];
		if(ServiceExists(MS_AV_GETMYAVATAR)){
			struct avatarCacheEntry *ace = (struct avatarCacheEntry *)CallService(MS_AV_GETMYAVATAR, 0,(LPARAM) pszSkypeProtoName);
			if (ace!=NULL) {
				hAvatar = ( HBITMAP )CallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )ace->szFilename);
				if ( hAvatar != NULL )
					SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)hAvatar );
			}
		}


		
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_SETAVATAR:
				char szFileName[ MAX_PATH ];
				if ( EnterBitmapFileName( szFileName ) != ERROR_SUCCESS )
					return false;

				hAvatar = ( HBITMAP )CallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )szFileName);
				if ( hAvatar != NULL ){
					SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)hAvatar );
					CallService(SKYPE_SETAVATAR, 0, ( LPARAM )szFileName);
				}
				break;

			case IDC_DELETEAVATAR:
				if ( hAvatar != NULL ) {
					DeleteObject( hAvatar );
					hAvatar = NULL;
					CallService(SKYPE_SETAVATAR, 0, NULL);
				}
				DBDeleteContactSetting( NULL, pszSkypeProtoName, "AvatarFile" );
				InvalidateRect( hwndDlg, NULL, TRUE );
				break;
		}	}
		break;

	case WM_DESTROY:
		if ( hAvatar != NULL )
			DeleteObject( hAvatar );
		break;
	}

	return 0;
}

/*
 * OptionsDlgProc
 *
 * This callback function is called, when the options dialog in Miranda is shown
 * The function contains all necessary stuff to process the options in the dialog
 * and store them in the database, when changed, and fill out the settings-dialog
 * correctly according to the current settings
 */
static int CALLBACK OptionsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int StartControls[]={IDC_NOSPLASH, IDC_MINIMIZED, IDC_NOTRAY};
	const int Skype2SocketControls[]={IDC_HOST, IDC_PORT, IDC_REQPASS, IDC_PASSWORD};
	static BOOL initDlg=FALSE;
	static int statusModes[]={ID_STATUS_OFFLINE,ID_STATUS_ONLINE,ID_STATUS_AWAY,ID_STATUS_NA,ID_STATUS_OCCUPIED,ID_STATUS_DND,ID_STATUS_FREECHAT,ID_STATUS_INVISIBLE,ID_STATUS_OUTTOLUNCH,ID_STATUS_ONTHEPHONE};
	DBVARIANT dbv;
	int i, j;
	
	switch (uMsg){
		case WM_INITDIALOG:	
			initDlg=TRUE;
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_STARTSKYPE, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "StartSkype", 1));
			CheckDlgButton(hwndDlg, IDC_NOSPLASH, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "nosplash", 1));
			CheckDlgButton(hwndDlg, IDC_MINIMIZED, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "minimized", 1));
			CheckDlgButton(hwndDlg, IDC_NOTRAY, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "notray", 0));
			CheckDlgButton(hwndDlg, IDC_SHUTDOWN, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "Shutdown", 0));
			CheckDlgButton(hwndDlg, IDC_ENABLEMENU, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "EnableMenu", 1));
			CheckDlgButton(hwndDlg, IDC_UNLOADOFFLINE, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UnloadOnOffline", 0));
			CheckDlgButton(hwndDlg, IDC_USES2S, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseSkype2Socket", 0));
			CheckDlgButton(hwndDlg, IDC_NOERRORS, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "SuppressErrors", 0));
			CheckDlgButton(hwndDlg, IDC_KEEPSTATE, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "KeepState", 0));
			SetDlgItemInt (hwndDlg, IDC_CONNATTEMPTS, DBGetContactSettingWord(NULL, pszSkypeProtoName, "ConnectionAttempts", 5), FALSE);
			if (ServiceExists(MS_GC_NEWCHAT) && atoi(SKYPE_PROTO+strlen(SKYPE_PROTO)-1)>=3)
				CheckDlgButton(hwndDlg, IDC_GROUPCHAT, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseGroupchat", 0));
			else
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT), FALSE);
#ifdef USEPOPUP
			if (ServiceExists(MS_POPUP_ADDPOPUP))
				CheckDlgButton(hwndDlg, IDC_USEPOPUP, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UsePopup", 0));
			else
#endif
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEPOPUP), FALSE);

			j=DBGetContactSettingDword(NULL, pszSkypeProtoName, "SkypeOutStatusMode", ID_STATUS_ONTHEPHONE);
			for(i=0;i<sizeof(statusModes)/sizeof(statusModes[0]);i++) {
				int k;

				k=SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_ADDSTRING,0,(LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,statusModes[i],0));
				SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_SETITEMDATA,k,statusModes[i]);
				if (statusModes[i]==j) SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_SETCURSEL,i,0);
			}

			if (!DBGetContactSetting(NULL, pszSkypeProtoName, "Host", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_HOST, dbv.pszVal);
				DBFreeVariant(&dbv);
			} else SetDlgItemText(hwndDlg, IDC_HOST, "localhost");
			SetDlgItemInt(hwndDlg, IDC_PORT, DBGetContactSettingWord(NULL, pszSkypeProtoName, "Port", 1401), FALSE);
			CheckDlgButton(hwndDlg, IDC_REQPASS, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "RequiresPassword", 0));
			if (!DBGetContactSetting(NULL, pszSkypeProtoName, "Password", &dbv)) {
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal)+1, (LPARAM)dbv.pszVal);
				SetDlgItemText(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			SendMessage(hwndDlg, WM_COMMAND, IDC_STARTSKYPE, 0);
			SendMessage(hwndDlg, WM_COMMAND, IDC_USES2S, 0);
			SendMessage(hwndDlg, WM_COMMAND, IDC_REQPASS, 0);
			initDlg=FALSE;
			return TRUE;
		case WM_NOTIFY: {
			NMHDR* nmhdr = (NMHDR*)lParam;
			char buf[1024];

			switch (nmhdr->code){
				case PSN_APPLY:
				case PSN_KILLACTIVE:
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "StartSkype", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_STARTSKYPE), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "nosplash", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOSPLASH), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "minimized", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_MINIMIZED), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "notray", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOTRAY), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "Shutdown", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_SHUTDOWN), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "EnableMenu", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_ENABLEMENU), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UnloadOnOffline", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_UNLOADOFFLINE), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UsePopup", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_USEPOPUP), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UseSkype2Socket", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_USES2S), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UseGroupchat", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_GROUPCHAT), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "SuppressErrors", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOERRORS), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "KeepState", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_KEEPSTATE), BM_GETCHECK,0,0)));
					DBWriteContactSettingWord (NULL, pszSkypeProtoName, "ConnectionAttempts", (unsigned short)GetDlgItemInt(hwndDlg, IDC_CONNATTEMPTS, NULL, FALSE));
					DBWriteContactSettingDword(NULL, pszSkypeProtoName, "SkypeOutStatusMode", SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_GETITEMDATA,SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_GETCURSEL,0,0),0));
					GetDlgItemText(hwndDlg, IDC_HOST, buf, sizeof(buf));
					DBWriteContactSettingString(NULL, pszSkypeProtoName, "Host", buf);
					DBWriteContactSettingWord(NULL, pszSkypeProtoName, "Port", (unsigned short)GetDlgItemInt(hwndDlg, IDC_PORT, NULL, FALSE));
					DBWriteContactSettingByte(NULL, pszSkypeProtoName, "RequiresPassword", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_REQPASS), BM_GETCHECK,0,0)));
					ZeroMemory(buf, sizeof(buf));
					GetDlgItemText(hwndDlg, IDC_PASSWORD, buf, sizeof(buf));
					CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(buf), (LPARAM)buf);
					DBWriteContactSettingString(NULL, pszSkypeProtoName, "Password", buf);
					return TRUE;
			}			
			break; 
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_STARTSKYPE:
					for (i=0; i<sizeof(StartControls)/sizeof(StartControls[0]); i++) EnableWindow(GetDlgItem(hwndDlg, StartControls[i]), SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0));
					break;
				case IDC_USES2S:
					for (i=0; i<sizeof(Skype2SocketControls)/sizeof(Skype2SocketControls[0]); i++) EnableWindow(GetDlgItem(hwndDlg, Skype2SocketControls[i]), SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0));
					if (SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0)) SendMessage(hwndDlg, WM_COMMAND, IDC_REQPASS, 0);
					break;
				case IDC_REQPASS:
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0));
					break;
				case IDC_CLEANUP:
					pthread_create(( pThreadFunc )CleanupNicknames, NULL);
					break;

			}
			if (!initDlg) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnDetailsInit - initializes user info dialog pages.

int OnDetailsInit( WPARAM wParam, LPARAM lParam )
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hIcon = NULL;
	odp.hInstance = hInst;

	HANDLE hContact = ( HANDLE )lParam;
	if ( hContact == NULL ) {
		
		char szTitle[256];
		mir_snprintf( szTitle, sizeof( szTitle ), "Skype %s", Translate( "Avatar" ));

		odp.pfnDlgProc = AvatarDlgProc;
		odp.position = 1900000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_SETAVATAR);
		odp.pszTitle = szTitle;
		CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);
	}
	return 0;
}
/*
 * RegisterOptions
 *
 * This function tells Miranda to add the configuration section of this plugin in
 * the Options-dialog.
 */
int RegisterOptions(WPARAM wParam, LPARAM lParam) {
   OPTIONSDIALOGPAGE odp;
   
   ZeroMemory(&odp, sizeof(odp));
   odp.cbSize = sizeof(odp);
   odp.hInstance = hInst;
   odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
   odp.pszGroup = Translate("Network");
   odp.pszTitle = pluginInfo.shortName;
   odp.pfnDlgProc = OptionsDlgProc;
   CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
   return 0;
}

int OkToExit(WPARAM wParam, LPARAM lParam) {
//	logoff_contacts();
	MirandaShuttingDown=TRUE;
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



// DLL Stuff //

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
		return &pluginInfo;
}



extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{

	PROTOCOLDESCRIPTOR pd;
	DWORD ThreadID, ExitCode, Buffsize;
	HKEY MyKey;
	BOOL SkypeInstalled;
	WSADATA wsaData;
//	char *path, *protocolname, *fend;

	pluginLink = link;

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

	hOptHook = HookEvent(ME_OPT_INITIALISE, RegisterOptions);

	// Check if Skype is installed
	SkypeInstalled=TRUE;

	UseSockets=(BOOL)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseSkype2Socket", 0);
	if (!UseSockets) {
		if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Skype\\Phone", 0, KEY_READ, &MyKey)!=ERROR_SUCCESS)
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Skype\\Phone", 0, KEY_READ, &MyKey)!=ERROR_SUCCESS)
				SkypeInstalled=FALSE;
		Buffsize=sizeof(skype_path);
		if (SkypeInstalled==FALSE || 
			RegQueryValueEx(MyKey, "SkypePath", NULL, NULL, (unsigned char *)skype_path,  &Buffsize)!=ERROR_SUCCESS) {
			    OUTPUT("Skype was not found installed :(");
				RegCloseKey(MyKey);
				skype_path[0]=0;
				return 0;
		}
		RegCloseKey(MyKey);
	}
	WSAStartup(MAKEWORD(2,2), &wsaData);

	// Start Skype connection 
	if (!(ControlAPIAttach=RegisterWindowMessage("SkypeControlAPIAttach")) ||
		!(ControlAPIDiscover=RegisterWindowMessage("SkypeControlAPIDiscover"))) {
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



	// Start up Messagepump
	//logoff_contacts();
	hThread=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc, NULL, 0, &ThreadID); 
	WaitForSingleObject(MessagePumpReady, INFINITE);
	GetExitCodeThread(hThread, &ExitCode);
	if (ExitCode==FALSE) {
		LOG("ThreadFunc", "returned failure, shutting down..");	
		return 0;
	}


	// Setup services
	{

		char pszServiceName[MAX_PATH+30];
		
		hPrebuildCMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

		//HookEvent(ME_CLIST_DOUBLECLICKED, ClistDblClick);
		CreateServiceFunction(SKYPE_CALL, SkypeCall);
		CreateServiceFunction(SKYPEOUT_CALL, SkypeOutCall);
		CreateServiceFunction(SKYPE_HOLDCALL, SkypeHoldCall);
		CreateServiceFunction(SKYPE_ADDUSER, SkypeAdduserDlg);
		CreateServiceFunction(SKYPE_IMPORTHISTORY, ImportHistory);
		CreateServiceFunction(SKYPE_ANSWERCALL, SkypeAnswerCall);
		CreateServiceFunction(SKYPE_SENDFILE, SkypeSendFile);
		CreateServiceFunction(SKYPE_SETAVATAR, SkypeSetAvatar);

		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETCAPS);
		CreateServiceFunction(pszServiceName , SkypeGetCaps);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETNAME);
		CreateServiceFunction(pszServiceName , SkypeGetName);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_LOADICON);
		CreateServiceFunction(pszServiceName , SkypeLoadIcon);

		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_SETSTATUS);
		CreateServiceFunction(pszServiceName , SkypeSetStatus);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETSTATUS);
		CreateServiceFunction(pszServiceName , SkypeGetStatus);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_ADDTOLIST);
		CreateServiceFunction(pszServiceName , SkypeAddToList);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_ADDTOLISTBYEVENT);
		CreateServiceFunction(pszServiceName , SkypeAddToListByEvent);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_BASICSEARCH);
		CreateServiceFunction(pszServiceName , SkypeBasicSearch);


		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSS_GETINFO);
		CreateServiceFunction(pszServiceName , SkypeGetInfo);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSS_MESSAGE);
		CreateServiceFunction(pszServiceName , SkypeSendMessage);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSR_MESSAGE);
		CreateServiceFunction(pszServiceName , SkypeRecvMessage);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSS_AUTHREQUEST);
		CreateServiceFunction(pszServiceName , SkypeSendAuthRequest);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSR_AUTH);
		CreateServiceFunction(pszServiceName , SkypeRecvAuth);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_AUTHALLOW);
		CreateServiceFunction(pszServiceName , SkypeAuthAllow);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_AUTHDENY);
		CreateServiceFunction(pszServiceName , SkypeAuthDeny);

		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETAVATARINFO);
		CreateServiceFunction(pszServiceName , SkypeGetAvatarInfo);

		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_SETAWAYMSG);
		CreateServiceFunction(pszServiceName , SkypeSetAwayMessage);
		strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETSTATUS);
		CreateServiceFunction(pszServiceName , SkypeGetAwayMessage);

	}
	hStatusHookContact = HookEvent(ME_DB_CONTACT_ADDED,HookContactAdded);
	hContactDeleted = HookEvent( ME_DB_CONTACT_DELETED, HookContactDeleted );

	/* Register the module */
	ZeroMemory(&pd, sizeof(pd));
	pd.cbSize = sizeof(pd);
	pd.szName = pszSkypeProtoName;
	pd.type   = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);
	hHookModulesLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hHookOkToExit = HookEvent(ME_SYSTEM_OKTOEXIT, OkToExit);
	
	
	return 0;

}



extern "C" int __declspec( dllexport ) Unload(void) 
{
	if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "Shutdown", 0) && skype_path[0])
		_spawnl(_P_NOWAIT, skype_path, skype_path, "/SHUTDOWN", NULL);
	SkypeMsgCleanup();
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
		UnhookEvent(hHookOkToExit);
		UnhookEvent(PrebuildContactMenu);
		UnhookEvent(hPrebuildCMenu);
		UnhookEvent(hHookOnUserInfoInit);
		//UnhookEvent(ClistDblClick);
		CloseHandle(hThread);
		CloseHandle(SkypeReady);
		CloseHandle(SkypeMsgReceived);
		CloseHandle(hPingPong);
		hPingPong=NULL;
		CloseHandle(SkypeMsgFetched);
		CloseHandle(MessagePumpReady);
	}
	return 0;
}


