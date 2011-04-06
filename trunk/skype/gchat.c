#include "skype.h"
#include "skypeapi.h"
#include "gchat.h"
#include "contacts.h"
#include "debug.h"
#include "utf8.h"
#include "../../include/m_langpack.h"
#include "../../include/m_userinfo.h"
#include "../../include/m_history.h"
#include "../../include/m_contacts.h"

#ifndef DWLP_USER
#define DWLP_USER DWL_USER
#endif

#ifdef _UNICODE
#define STR "%S"
#else
#define STR "%s"
#endif

extern HANDLE hInitChat;
extern HINSTANCE hInst;
extern char protocol;

static gchat_contacts *chats=NULL;
static int chatcount=0;
static CRITICAL_SECTION m_GCMutex;

// TODO: Disable groupchat for Protocol verisons <5

/****************************************************************************/
/*                  Chat management helper functions                        */
/****************************************************************************/

/* Get the gchat_contacts entry for the chat with the id szChatId
   If the chat doesn't already exist in the list, it is added.

   Parameters: szChatId - String with the chat ID of the chat to be found
   Returns:    Pointer to the gchat_contacts entry for the given id.
			   NULL on failure (not enough memory)
*/
gchat_contacts *GetChat(TCHAR *szChatId) {
	int i;

	for (i=0;i<chatcount;i++)
		if (!_tcscmp(chats[i].szChatName, szChatId)) return &chats[i];
	if (chats = (gchat_contacts *)realloc(chats, sizeof(gchat_contacts)*(++chatcount))) {
		memset(&chats[chatcount-1], 0, sizeof(gchat_contacts));
		chats[chatcount-1].szChatName=_tcsdup(szChatId);
		return &chats[chatcount-1];
	}
	return NULL;
}

/* Removes the gchat_contacts entry for the chat with the id szChatId,
   if it exists.
  
   Parameters: szChatId - String with the chat ID to be removed from list
 */
static void RemChat(TCHAR *szChatId) {
	int i;

	for (i=0;i<chatcount;i++)
		if (!_tcscmp(chats[i].szChatName, szChatId)) {
			if (chats[i].szChatName) free(chats[i].szChatName);
			if (chats[i].mJoinedContacts) free(chats[i].mJoinedContacts);
			if (i<--chatcount) memmove(&chats[i], &chats[i+1], (chatcount-i)*sizeof(gchat_contacts));
			chats = (gchat_contacts *)realloc(chats, sizeof(gchat_contacts)*chatcount);
			return;
		}
}

/* Checks, if the contact with the handle hContact exists in the groupchat
   given in gc

  Parameters: gc       - gchat_contacts entry for the chat session to be searched
			  hContact - Handle to the contact to be found.
  Returns:    -1  = Not found
              >=0 = Number of found item
 */
static int ExistsChatContact(gchat_contacts *gc, HANDLE hContact) {
	int i;

	for (i=0;i<gc->mJoinedCount;i++)
		if (gc->mJoinedContacts[i]==hContact) return i;
	return -1;
}

/* Adds contact with the name who to the groupchat given in gc

  Parameters: gc   -
  Returns:    -1  = Contact not found
			  -2  = On failure
			  >=0 = Number of added item
 */
static int AddChatContact(gchat_contacts *gc, char *who) {
	int i = -2;
	HANDLE hContact;
	GCDEST gcd = {0};
	GCEVENT gce = {0};
    CONTACTINFO ci = {0};
	TCHAR *twho;

	if (!(hContact=find_contact(who))) return -1;
	if ((i=ExistsChatContact(gc, hContact))>=0) return i;


#ifdef _UNICODE
	if (!(twho = make_unicode_string(who)))
		return -2;
#else
	twho = who;
#endif
	gcd.pszModule = SKYPE_PROTONAME;
	gcd.ptszID = gc->szChatName;
	gcd.iType = GC_EVENT_JOIN;

	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.ptszStatus = _T("USER");		// TODO: Add role support, query role via CHAT MEMBEROBJECTS / ROLE in protocol >=7
	gce.time = time(NULL);
	gce.dwFlags = GCEF_ADDTOLOG | GC_TCHAR;

	ci.cbSize = sizeof(ci);
	ci.szProto = SKYPE_PROTONAME;
	ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
	ci.hContact = hContact;

	if (!CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) gce.ptszNick=ci.pszVal; 
	else gce.ptszNick=twho;
        
	gce.ptszUID=twho;
	if (!CallService(MS_GC_EVENT, 0, (LPARAM)&gce)) {
		if ((gc->mJoinedContacts=(void **)realloc(gc->mJoinedContacts, (gc->mJoinedCount+1)*sizeof(HANDLE))))
		{
			gc->mJoinedContacts[i=gc->mJoinedCount]=hContact;
			gc->mJoinedCount++;
		}
	}
    if (ci.pszVal) miranda_sys_free (ci.pszVal);

#ifdef _UNICODE
	free (twho);
#endif
	return i;
}

void RemChatContact(gchat_contacts *gc, HANDLE hContact) {
	int i;

	for (i=0;i<gc->mJoinedCount;i++)
		if (gc->mJoinedContacts[i]==hContact) {
			if (i<--gc->mJoinedCount) 
				memmove(&gc->mJoinedContacts[i], &gc->mJoinedContacts[i+1], (gc->mJoinedCount-i)*sizeof(HANDLE));
			if (gc->mJoinedCount) gc->mJoinedContacts = (void **) realloc(gc->mJoinedContacts, sizeof(HANDLE)*gc->mJoinedCount);
			else {free (gc->mJoinedContacts); gc->mJoinedContacts = NULL; }
			return;
		}
}

HANDLE find_chat(TCHAR *chatname) {
	char *szProto;
	int tCompareResult;
	HANDLE hContact;
	DBVARIANT dbv;

	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME) &&
			DBGetContactSettingByte(hContact, SKYPE_PROTONAME, "ChatRoom", 0)==1)
		{
			if (DBGetContactSettingTString(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv)) continue;
            tCompareResult = _tcscmp(dbv.ptszVal, chatname);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}

#ifdef _UNICODE
HANDLE find_chatA(char *chatname) {
	char *szProto;
	int tCompareResult;
	HANDLE hContact;
	DBVARIANT dbv;

	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME) &&
			DBGetContactSettingByte(hContact, SKYPE_PROTONAME, "ChatRoom", 0)==1)
		{
			if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv)) continue;
            tCompareResult = strcmp(dbv.pszVal, chatname);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}
#endif



int  __cdecl AddMembers(char *szSkypeMsg) {
	BYTE *contactmask=NULL;
	DBVARIANT dbv, dbv2;
	CONTACTINFO ci={0};
	char *ptr, *who;
	TCHAR *szChatId;
	int i, iRet = 0;
	gchat_contacts *gc;

	LOG(("AddMembers STARTED")); 
	if (!(ptr=strstr(szSkypeMsg, " MEMBERS"))) return -1;
	EnterCriticalSection(&m_GCMutex);
	ptr[0]=0;
#ifdef _UNICODE
	szChatId = make_unicode_string(szSkypeMsg+5);
#else
	szChatId = szSkypeMsg+5;
#endif
	ptr+=9;
	if (find_chat(szChatId) && (gc=GetChat(szChatId)) && 
		!DBGetContactSettingString(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv2))
	{
		// Add new contacts
		for (who=strtok(ptr, " "); who; who=strtok(NULL, " ")) {
			if (strcmp(who, dbv2.pszVal)) {
				i=AddChatContact(gc, who);
				if (i>=0 && !contactmask && !(contactmask = (unsigned char*)calloc(gc->mJoinedCount, 1))) i=-2;
				if (i<0 || !(contactmask= (unsigned char *) realloc(contactmask, gc->mJoinedCount))) {
					iRet = -1;
					break;
				}
				contactmask[i]=TRUE;
			}
		}
		// Quit contacts which are no longer there
		if (iRet == 0 && contactmask) {
			GCDEST gcd = {0};
			GCEVENT gce = {0};

			gcd.pszModule = SKYPE_PROTONAME;
			gcd.ptszID = szChatId;
			gcd.iType = GC_EVENT_QUIT;

			gce.cbSize = sizeof(GCEVENT);
			gce.pDest = &gcd;
			gce.time = time(NULL);
			gce.dwFlags = GCEF_ADDTOLOG | GC_TCHAR;
        
			ci.cbSize = sizeof(ci);
			ci.szProto = SKYPE_PROTONAME;
			ci.dwFlag = CNF_DISPLAY;

			for (i=0;i<gc->mJoinedCount;i++)
			if (!contactmask[i] &&
				!DBGetContactSettingTString(gc->mJoinedContacts[i], SKYPE_PROTONAME, SKYPE_NAME, &dbv)) 
			{
				ci.hContact = gc->mJoinedContacts[i];
				ci.dwFlag = CNF_TCHAR;
				if (!CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) gce.ptszNick=ci.pszVal; 
				else gce.ptszNick=dbv.ptszVal;
				RemChatContact(gc, gc->mJoinedContacts[i]);
				gce.ptszUID = dbv.ptszVal;
				CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
				DBFreeVariant(&dbv);
				if (ci.pszVal) {
					miranda_sys_free (ci.pszVal);
					ci.pszVal=NULL;
				}
			}
	// We don't do this, because the dialog group-chat may have been started intentionally
	/*
			if (gc->mJoinedCount == 1) {
				// switch back to normal session
				KillChatSession(&gcd);
			}
	*/
		}
		if (contactmask) free(contactmask);
		DBFreeVariant(&dbv2);
	} else iRet = -1;
#ifdef _UNICODE
	free (szChatId);
#endif
	LeaveCriticalSection(&m_GCMutex);
	LOG(("AddMembers DONE"));
	return iRet;
}

/****************************************************************************/
/*                           Window procedures                              */
/****************************************************************************/
BOOL CALLBACK InputBoxDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);
		SetWindowLong (hwndDlg, DWLP_USER, lParam);
		SetDlgItemText (hwndDlg, IDC_TEXT, (TCHAR*)lParam);
		return TRUE;
	}

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
		{	
			GetDlgItemText(hwndDlg, IDC_TEXT, (TCHAR*)GetWindowLong(hwndDlg, DWLP_USER), MAX_BUF-1*sizeof(TCHAR));
			EndDialog(hwndDlg, 1);
			break;
		}
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
	}	}
	return FALSE;
}

/****************************************************************************/
/*                      Core Chat management functions                      */
/****************************************************************************/

/* We have a new Groupchat

   This hook is called when a new chat is initialised.
   Parameters:  wParam = (char *)Name of new chat session [Has to be ASCIIZ/UTF8]
				lParam = 0
*/
int __cdecl  ChatInit(WPARAM wParam, LPARAM lParam) {
	GCSESSION gcw = {0};
	GCEVENT gce = {0};
	GCDEST gcd = {0};
	DBVARIANT dbv, dbv2;
	char *szChatName;
	int iRet = -1;

	if (!wParam) return -1;

	gcw.cbSize = sizeof(GCSESSION);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = SKYPE_PROTONAME;
	gcw.dwFlags = GC_TCHAR;

	if (!(szChatName = SkypeGet ("CHAT", (char *)wParam, "FRIENDLYNAME")) || !*szChatName)
		gcw.ptszName=TranslateT("Unknown"); else {
#ifdef _UNICODE
		gcw.ptszName=make_unicode_string(szChatName);
		free (szChatName);
		szChatName = (char*)gcw.ptszName;
#else
		gcw.ptszName=szChatName;
#endif
	}
#ifdef _UNICODE
	gcw.ptszID = make_unicode_string((char*)wParam);
#else
	gcw.ptszID = (char *)wParam;
#endif

	gcw.pszStatusbarText = NULL;
	EnterCriticalSection(&m_GCMutex);
	if (!CallService(MS_GC_NEWSESSION, 0, (LPARAM)&gcw)) {
		char *szChatRole;

		gce.cbSize = sizeof(GCEVENT);
		gcd.pszModule = SKYPE_PROTONAME;
		gcd.ptszID = (TCHAR*)gcw.ptszID;
		gcd.iType = GC_EVENT_ADDGROUP;
		gce.pDest = &gcd;
		gce.ptszStatus = _T("CREATOR");
		gce.dwFlags = GC_TCHAR;
		// BUG: Groupchat returns nonzero on success here in earlier versions, so we don't check
		// it here
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
		gce.ptszStatus = _T("MASTER");
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
		gce.ptszStatus = _T("HELPER");
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
		gce.ptszStatus = _T("USER");
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
		gce.ptszStatus = _T("LISTENER");
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
		gce.ptszStatus = _T("APPLICANT");
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		gcd.iType = GC_EVENT_JOIN;
		gce.ptszStatus = NULL;
		if (protocol >=7 && (szChatRole = SkypeGet ("CHAT", (char *)wParam, "MYROLE"))) {
			if (strncmp(szChatRole, "ERROR", 5))
			{
#ifdef _UNICODE
				gce.ptszStatus = make_unicode_string(szChatRole);
				free (szChatRole);
#else
				gce.ptszStatus = szChatRole;
#endif
			}
		}
		if (!gce.ptszStatus) gce.ptszStatus=_tcsdup(_T("CREATOR"));

		if (!DBGetContactSettingTString(NULL, SKYPE_PROTONAME, "Nick", &dbv)) {
			if (!DBGetContactSettingTString(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv2)) {
				gce.ptszNick = dbv.ptszVal;
				gce.ptszUID = dbv2.ptszVal;
				gce.time = 0;
				gce.bIsMe = TRUE;
				gce.dwFlags |= GCEF_ADDTOLOG;
				if (!CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce)) {
					SkypeSend ("GET CHAT %s MEMBERS", (char *)wParam);
					gce.cbSize = sizeof(GCEVENT);
					gcd.iType = GC_EVENT_CONTROL;
					gce.pDest = &gcd;
					CallService(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
					CallService(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
					CallService(MS_GC_EVENT, WINDOW_VISIBLE, (LPARAM)&gce);
					iRet = 0;
				} else {LOG (("ChatInit: Joining 'me' failed."));}
			}
			DBFreeVariant(&dbv2);
		}
		free ((void*)gce.ptszStatus);
		DBFreeVariant(&dbv);
	}
	free (szChatName);
#ifdef _UNICODE
	free ((void*)gcw.ptszID);
#endif
	LeaveCriticalSection(&m_GCMutex);
	return iRet;
}

/* Open new Groupchat

   Parameters:  szChatId = (char *)Name of new chat session
*/
int  __cdecl ChatStart(char *szChatId) {
	LOG(("ChatStart: New groupchat started"));
	if (!szChatId || NotifyEventHooks(hInitChat, (WPARAM)szChatId, 0)) return -1;
	return 0;
}


void KillChatSession(GCDEST *gcd) {
	GCEVENT gce = {0};

	LOG(("KillChatSession: Groupchatsession terminated."));
	gce.cbSize = sizeof(GCEVENT);
	gce.dwFlags = GC_TCHAR;
	gce.pDest = gcd;
	gcd->iType = GC_EVENT_CONTROL;
	if (SkypeSend ("ALTER CHAT "STR" LEAVE", gcd->ptszID) == 0)
	{
		CallService(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
		CallService(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
	}
}

void InviteUser(TCHAR *szChatId) {
	HMENU tMenu = CreatePopupMenu();
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0), hInvitedUser;
	DBVARIANT dbv;
	HWND tWindow;
	POINT pt;
	gchat_contacts *gc;
	int j;

	if (!(gc=GetChat(szChatId))) return;

	// add the heading
	AppendMenu(tMenu, MF_STRING|MF_GRAYED|MF_DISABLED, (UINT_PTR)0, TranslateT("&Invite user..."));
	AppendMenu(tMenu, MF_SEPARATOR, (UINT_PTR)1, NULL);
    
	// generate a list of contact
	while (hContact) {
		if (!strcmp(SKYPE_PROTONAME, (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact,0 )) &&
			!DBGetContactSettingByte(hContact, SKYPE_PROTONAME, "ChatRoom", 0) &&
			 DBGetContactSettingWord(hContact, SKYPE_PROTONAME, "Status", ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE) 
		{
			BOOL alreadyInSession = FALSE;
			for (j=0; j<gc->mJoinedCount; j++) {
				if (gc->mJoinedContacts[j]==hContact) {
					alreadyInSession = TRUE;
					break;
				}
			}
            if (!alreadyInSession)
				AppendMenu(tMenu, MF_STRING, (UINT_PTR)hContact, 
					(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR));
        }
		hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact, 0);
	}

	tWindow = CreateWindow(_T("EDIT"),_T(""),0,1,1,1,1,NULL,NULL,hInst,NULL);

	GetCursorPos (&pt);
	hInvitedUser = (HANDLE)TrackPopupMenu(tMenu, TPM_NONOTIFY | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, tWindow, NULL);
	DestroyMenu(tMenu);
	DestroyWindow(tWindow);

	if (!hInvitedUser || DBGetContactSettingString(hInvitedUser, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) 
		return;
	SkypeSend ("ALTER CHAT "STR" ADDMEMBERS %s", szChatId, dbv.pszVal);
	DBFreeVariant(&dbv);

}

void SetChatTopic (TCHAR *szChatId, TCHAR *szTopic)
{
	GCDEST gcd = {0};
	GCEVENT gce = {0};
	HANDLE hContact = find_chat (szChatId);
	char *szUTFTopic;

	gce.cbSize = sizeof(GCEVENT);
	gcd.pszModule = SKYPE_PROTONAME;
	gcd.ptszID = szChatId;
	gcd.iType = GC_EVENT_TOPIC;
	gce.pDest = &gcd;
	gce.ptszText = szTopic;
	gce.dwFlags = GCEF_ADDTOLOG | GC_TCHAR;
	gce.time = time (NULL);
	gce.dwFlags = GC_TCHAR;
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
	gcd.iType = GC_EVENT_SETSBTEXT;
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

#ifdef _UNICODE
	szUTFTopic=make_utf8_string(szTopic);
#else
	if (utf8_encode(szTopic, &szUTFTopic)==-1) szUTFTopic = NULL;
#endif
	if (szUTFTopic) {
		SkypeSend ("ALTER CHAT "STR" SETTOPIC %s", szChatId, szUTFTopic);
		free (szUTFTopic);
	}
	testfor ("ALTER CHAT SETTOPIC", INFINITE);

	if (hContact)
		DBWriteContactSettingTString(hContact, SKYPE_PROTONAME, "Nick", szTopic);
}


int GCEventHook(WPARAM wParam,LPARAM lParam) {
	GCHOOK *gch = (GCHOOK*) lParam;
	gchat_contacts *gc = GetChat(gch->pDest->ptszID);

	if(gch) {
		if (!stricmp(gch->pDest->pszModule, SKYPE_PROTONAME)) {

			switch (gch->pDest->iType) {
			case GC_SESSION_TERMINATE: {
				HANDLE hContact;
				if (gc->mJoinedCount == 1) {
					// switch back to normal session
                    // I don't know if this behaviour isn't a bit annoying, therefore, we
					// don't do this now, until a user requests this feature :)
                    
					// open up srmm dialog when quit while 1 person left
//					CallService(MS_MSG_SENDMESSAGE, (WPARAM)gc->mJoinedContacts[0], 0);

					RemChatContact(gc, gc->mJoinedContacts[0]);
				}
				// Delete Chatroom from Contact list, as we don't need it anymore...?
				if (hContact = find_chat(gc->szChatName))
					CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0); 
				RemChat(gc->szChatName);

				break;
			}
			case GC_USER_MESSAGE:
				if(gch && gch->ptszText && _tcslen(gch->ptszText) > 0) {
					DBVARIANT dbv, dbv2;
					CCSDATA ccs = {0};
					GCDEST gcd = {0};
					GCEVENT gce = {0};
					TCHAR *pEnd;
					char *utfmsg;

					// remove the ending linebreak
					for (pEnd = &gch->ptszText[_tcslen(gch->ptszText) - 1];
						 *pEnd==_T('\r') || *pEnd==_T('\n'); pEnd--) *pEnd=0;
                    // Send message to the chat-contact    
					/*
					if (ccs.hContact = find_chat(gch->pDest->ptszID)) {
						ccs.lParam = (LPARAM)gch->ptszText;
						ccs.wParam = PREF_TCHAR;
						CallService (SKYPE_PROTONAME PSS_MESSAGE, 0, (LPARAM)&ccs);
					}
					*/
					// Just send the stuff to Skype, no need to call PSS_MESSAGE
#ifdef _UNICODE
					if (!(utfmsg = make_utf8_string((WCHAR*)gch->ptszText))) break;
#else
					if (utf8_encode(gch->ptszText, &utfmsg)==-1) break;
#endif
					if (SkypeSend("CHATMESSAGE "STR" %s", gch->pDest->ptszID, utfmsg)) {
						free (utfmsg);
						break;
					}
					free (utfmsg);

					// Add our line to the chatlog	
					gcd.pszModule = gch->pDest->pszModule;
					gcd.ptszID = gch->pDest->ptszID;
					gcd.iType = GC_EVENT_MESSAGE;

					gce.cbSize = sizeof(GCEVENT);
					gce.pDest = &gcd;
					if (DBGetContactSettingTString(NULL, SKYPE_PROTONAME, "Nick", &dbv)) gce.ptszNick=TranslateT("Me");
					else gce.ptszNick = dbv.ptszVal;
					DBGetContactSettingTString(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv2);
					gce.ptszUID = dbv2.ptszVal;
					gce.time = time(NULL);
					gce.ptszText = gch->ptszText;
					gce.dwFlags = GCEF_ADDTOLOG | GC_TCHAR;
					gce.bIsMe = TRUE;
					CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
					if (dbv.pszVal) DBFreeVariant(&dbv);
					if (dbv2.pszVal) DBFreeVariant(&dbv2);
				}
				break;
			case GC_USER_CHANMGR:
				InviteUser(gch->pDest->ptszID);
				break;
			case GC_USER_PRIVMESS: {
				HANDLE hContact = find_contactT(gch->ptszUID);
				if (hContact) CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
				break;

			}
			case GC_USER_LOGMENU:
				switch(gch->dwData) {
				case 10: InviteUser(gch->pDest->ptszID); break;
				case 20: KillChatSession(gch->pDest); break;
                case 30: 
					{
						TCHAR *ptr, buf[MAX_BUF];

						ptr = SkypeGetT ("CHAT", gch->pDest->ptszID, "TOPIC");
						_tcscpy(buf, ptr);
						free(ptr);
						if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INPUTBOX), NULL, InputBoxDlgProc, (LPARAM)&buf))
							SetChatTopic (gch->pDest->ptszID, buf);
						break;
					}
				}
				break;
			case GC_USER_NICKLISTMENU: {
				HANDLE hContact = find_contactT(gch->ptszUID);

				switch(gch->dwData) {
				case 10:CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)hContact, 0); break;
				case 20:CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)hContact, 0); break;
				case 110: KillChatSession(gch->pDest); break;
				}
				break;
			}			
			default:
				break;
			}
		}

	}
	return 0;
}

int __cdecl  GCMenuHook(WPARAM wParam,LPARAM lParam) {
	GCMENUITEMS *gcmi= (GCMENUITEMS*) lParam;
	DBVARIANT dbv;
	TCHAR* szInvite  = TranslateT("&Invite user...");
	TCHAR* szLeave   = TranslateT("&Leave chat session");
	TCHAR* szTopic   = TranslateT("Set &Topic...");
	TCHAR* szDetails = TranslateT("User &details");
	TCHAR* szHistory = TranslateT("User &history");

	static struct gc_item Item_log[] = {
		{NULL, 10, MENU_ITEM, FALSE},
		{NULL, 30, MENU_ITEM, FALSE},
		{NULL, 20, MENU_ITEM, FALSE}
	};
	static struct gc_item Item_nicklist_me[] = {
		{NULL, 20, MENU_ITEM, FALSE},
		{_T(""), 100, MENU_SEPARATOR, FALSE},
		{NULL, 110, MENU_ITEM, FALSE}
	};
	static struct gc_item Item_nicklist[] = {
		{NULL, 10, MENU_ITEM, FALSE},
		{NULL, 20, MENU_ITEM, FALSE}
	};

	Item_log[0].pszDesc  = szInvite;
	Item_log[1].pszDesc  = szTopic;
	Item_log[2].pszDesc  = szLeave;
	Item_nicklist_me[0].pszDesc  = szHistory;
	Item_nicklist_me[2].pszDesc  = szLeave;
	Item_nicklist[0].pszDesc  = szDetails;
	Item_nicklist[1].pszDesc  = szHistory;

	LOG (("GCMenuHook started."));
	if(gcmi) {
		if (!stricmp(gcmi->pszModule, SKYPE_PROTONAME)) {
			switch (gcmi->Type)
			{
			case MENU_ON_LOG:
				gcmi->nItems = sizeof(Item_log)/sizeof(Item_log[0]);
				gcmi->Item = &Item_log[0];
                LOG (("GCMenuHook: Items in log window: %d", gcmi->nItems));
				break;
			case MENU_ON_NICKLIST:
				if (DBGetContactSettingTString(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) return -1;
				if (!lstrcmp(dbv.ptszVal, gcmi->pszUID)) {
					gcmi->nItems = sizeof(Item_nicklist_me)/sizeof(Item_nicklist_me[0]);
					gcmi->Item = &Item_nicklist_me[0];
				} else {
					gcmi->nItems = sizeof(Item_nicklist)/sizeof(Item_nicklist[0]);
					gcmi->Item = &Item_nicklist[0];
				}
				DBFreeVariant(&dbv);
				break;
			}
        } else {LOG (("GCMenuHook: ERROR: Not our protocol."));}
	} else {LOG (("GCMenuHook: ERROR: No gcmi"));}
	LOG (("GCMenuHook: terminated."));
	return 0;
}

INT_PTR GCOnLeaveChat(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	DBVARIANT dbv;

	if (DBGetContactSettingTString(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv) == 0)
	{
		GCDEST gcd = {0};

		gcd.pszModule = SKYPE_PROTONAME;
		gcd.iType = GC_EVENT_CONTROL;
		gcd.ptszID = dbv.ptszVal;
		KillChatSession(&gcd);
		DBFreeVariant(&dbv);
	}
	return 0;
}
 
INT_PTR GCOnJoinChat(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	DBVARIANT dbv;

	if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv) == 0)
	{
		ChatStart (dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	return 0;
}

void GCInit(void)
{
	InitializeCriticalSection (&m_GCMutex);
}

void GCExit(void)
{
	int i;

	DeleteCriticalSection (&m_GCMutex);
	for (i=0;i<chatcount;i++) {
		if (chats[i].szChatName) free(chats[i].szChatName);
		if (chats[i].mJoinedContacts) free(chats[i].mJoinedContacts);
	}
	if (chats) free (chats);
	chats = NULL;
	chatcount = 0;
}