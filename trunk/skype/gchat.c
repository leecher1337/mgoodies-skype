#include "skype.h"
#include "skypeapi.h"
#include "gchat.h"
#include "contacts.h"
#include "debug.h"
#include "../../include/m_langpack.h"
#include "../../include/m_userinfo.h"
#include "../../include/m_history.h"
#include "../../include/m_contacts.h"

extern HANDLE hInitChat;
extern HINSTANCE hInst;

gchat_contacts *chats=NULL;
int chatcount=0;

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
gchat_contacts *GetChat(char *szChatId) {
	int i;

	for (i=0;i<chatcount;i++)
		if (!strcmp(chats[i].szChatName, szChatId)) return &chats[i];
	if (chats = (gchat_contacts *)realloc(chats, sizeof(gchat_contacts)*(++chatcount))) {
		memset(&chats[chatcount-1], 0, sizeof(gchat_contacts));
		chats[chatcount-1].szChatName=_strdup(szChatId);
		return &chats[chatcount-1];
	}
	return NULL;
}

/* Removes the gchat_contacts entry for the chat with the id szChatId,
   if it exists.
  
   Parameters: szChatId - String with the chat ID to be removed from list
 */
void RemChat(char *szChatId) {
	int i;

	for (i=0;i<chatcount;i++)
		if (!strcmp(chats[i].szChatName, szChatId)) {
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
int ExistsChatContact(gchat_contacts *gc, HANDLE hContact) {
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
int AddChatContact(gchat_contacts *gc, char *who) {
	int i;
	DBVARIANT dbv;
	HANDLE hContact;
	GCDEST gcd = {0};
	GCEVENT gce = {0};
    CONTACTINFO ci = {0};

	if (!(hContact=find_contact(who))) return -1;
	if ((i=ExistsChatContact(gc, hContact))>=0) return i;

	gcd.pszModule = SKYPE_PROTONAME;
	gcd.pszID = gc->szChatName;
	gcd.iType = GC_EVENT_JOIN;

	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.pszStatus = Translate("Others");
	gce.time = time(NULL);
	gce.bAddToLog = TRUE;

	ci.cbSize = sizeof(ci);
	ci.szProto = SKYPE_PROTONAME;
	ci.dwFlag = CNF_DISPLAY;
	ci.hContact = hContact;
	if (!CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) gce.pszNick=ci.pszVal; 
	else gce.pszNick=who;
        
	gce.pszUID=who;
	if (CallService(MS_GC_EVENT, 0, (LPARAM)&gce)) {
		DBFreeVariant(&dbv);
        if (ci.pszVal) miranda_sys_free (ci.pszVal);
		return -2;
	}
	DBFreeVariant(&dbv);
    if (ci.pszVal) miranda_sys_free (ci.pszVal);

	if (!(gc->mJoinedContacts=(void **)realloc(gc->mJoinedContacts, ++gc->mJoinedCount*sizeof(HANDLE)))) return -2;
	gc->mJoinedContacts[i=(gc->mJoinedCount-1)]=hContact;
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

HANDLE find_chat(char *chatname) {
	char *szProto;
	int tCompareResult;
	HANDLE hContact;
	DBVARIANT dbv;

	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME) &&
			DBGetContactSettingByte(hContact, SKYPE_PROTONAME, "ChatRoom", 0)==1)
		{
			if (DBGetContactSetting(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv)) continue;
            tCompareResult = strcmp(dbv.pszVal, chatname);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}


int  __cdecl AddMembers(char *szSkypeMsg) {
	BYTE *contactmask=NULL;
	DBVARIANT dbv, dbv2;
	CONTACTINFO ci={0};
	char *ptr, *who, *szChatId;
	int i;
	gchat_contacts *gc;

	LOG(("AddMembers STARTED")); 
	if (!(ptr=strstr(szSkypeMsg, " MEMBERS"))) return -1;
	ptr[0]=0;
    szChatId = szSkypeMsg+5;
	ptr+=9;
	if (!find_chat(szChatId) || !(gc=GetChat(szChatId))) return -1;

	who=strtok(ptr, " ");
	if (DBGetContactSetting(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv2)) return -1;

	// Add new contacts
	while (who) {
		if (strcmp(who, dbv2.pszVal)) {
			i=AddChatContact(gc, who);
			if (i!=-1 && !contactmask && !(contactmask= (unsigned char *)calloc(gc->mJoinedCount, 1))) i=-1;
			if (i==-1 || !(contactmask= (unsigned char *) realloc(contactmask, gc->mJoinedCount))) {
				if (contactmask) free(contactmask);
				DBFreeVariant(&dbv2);
				return -1;
			}
			contactmask[i]=TRUE;
		}
		who=strtok(NULL, " ");
	}
	// Quit contacts which are no longer there
	if (contactmask) {
		GCDEST gcd = {0};
		GCEVENT gce = {0};

		gcd.pszModule = SKYPE_PROTONAME;
		gcd.pszID = szChatId;
		gcd.iType = GC_EVENT_QUIT;

		gce.cbSize = sizeof(GCEVENT);
		gce.pDest = &gcd;
		gce.time = time(NULL);
		gce.bAddToLog = TRUE;
        
        ci.cbSize = sizeof(ci);
		ci.szProto = SKYPE_PROTONAME;
		ci.dwFlag = CNF_DISPLAY;

		for (i=0;i<gc->mJoinedCount;i++)
		if (!contactmask[i] &&
			!DBGetContactSetting(gc->mJoinedContacts[i], SKYPE_PROTONAME, SKYPE_NAME, &dbv)) 
		{
			ci.hContact = gc->mJoinedContacts[i];
			if (!CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) gce.pszNick=ci.pszVal; 
			else gce.pszNick=dbv.pszVal;
			RemChatContact(gc, gc->mJoinedContacts[i]);
			gce.pszUID = dbv.pszVal;
			CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
			DBFreeVariant(&dbv);
            if (ci.pszVal) {
				miranda_sys_free (ci.pszVal);
				ci.pszVal=NULL;
			}
		}
		free(contactmask);
// We don't do this, because the dialog group-chat may have been started intentionally
/*
		if (gc->mJoinedCount == 1) {
			// switch back to normal session
			KillChatSession(&gcd);
		}
*/
	}
	DBFreeVariant(&dbv2);
	LOG(("AddMembers DONE"));
	return 0;
}

/****************************************************************************/
/*                           Window procedures                              */
/****************************************************************************/
BOOL CALLBACK InputBoxDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static char *szText;

	switch (msg) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);
		szText = (char*)lParam;
		SetDlgItemText (hwndDlg, IDC_TEXT, szText);
		return TRUE;
	}

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
		{	
			GetDlgItemText(hwndDlg, IDC_TEXT, szText, MAX_BUF-1);
			szText[MAX_BUF] = 0;
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
   Parameters:  wParam = (char *)Name of new chat session
				lParam = 0
*/
int __cdecl  ChatInit(WPARAM wParam, LPARAM lParam) {
	GCWINDOW gcw = {0};
	GCEVENT gce = {0};
	GCDEST gcd = {0};
	DBVARIANT dbv, dbv2;
	char *szTopic;

	if (!wParam) return -1;
	gcw.cbSize = sizeof(GCWINDOW);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = SKYPE_PROTONAME;
	gcw.pszID = (char *)wParam;

	if (!(szTopic = SkypeGet ("CHAT", (char *)gcw.pszID, "TOPIC"))) return -1;
	if (!szTopic[0]) gcw.pszName=Translate("No Topic"); else gcw.pszName=szTopic;

	gcw.pszStatusbarText = NULL;
	gcw.bDisableNickList = FALSE;
	if (CallService(MS_GC_NEWCHAT, 0, (LPARAM)&gcw)) {
		free (szTopic);
		return -1;
	}
    free (szTopic);

	gce.cbSize = sizeof(GCEVENT);
	gcd.pszModule = SKYPE_PROTONAME;
	gcd.pszID = (char *) gcw.pszID;
	gcd.iType = GC_EVENT_ADDGROUP;
	gce.pDest = &gcd;
	gce.pszStatus = Translate("Me");
	// BUG: Groupchat returns nonzero on success here in earlier versions, so we don't check
	// it here
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_JOIN;
	gce.pszStatus = Translate("Me");
	if (DBGetContactSetting(NULL, SKYPE_PROTONAME, "Nick", &dbv)) return -1;
	if (DBGetContactSetting(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv2)) {
		DBFreeVariant(&dbv);
		return -1;
	}
	gce.pszNick = dbv.pszVal;
	gce.pszUID = dbv2.pszVal;
	gce.time = 0;
	gce.bIsMe = TRUE;
	gce.bAddToLog = FALSE;
	if (CallService(MS_GC_EVENT, 0, (LPARAM)&gce)) {
        LOG (("ChatInit: Joining 'me' failed."));
		DBFreeVariant(&dbv);
		DBFreeVariant(&dbv2);
		return -1;
	}
	DBFreeVariant(&dbv);
	DBFreeVariant(&dbv2);

	gcd.iType = GC_EVENT_ADDGROUP;
	gce.pDest = &gcd;
	gce.pszStatus = Translate("Others");
	// BUG: Groupchat returns nonzero on success here in earlier versions, so we don't check
	// it here
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

    SkypeSend ("GET CHAT %s MEMBERS", (char *)wParam);
	gce.cbSize = sizeof(GCEVENT);
	gcd.iType = GC_EVENT_CONTROL;
	gce.pDest = &gcd;
	CallService(MS_GC_EVENT, WINDOW_INITDONE, (LPARAM)&gce);
	CallService(MS_GC_EVENT, WINDOW_ONLINE, (LPARAM)&gce);
	CallService(MS_GC_EVENT, WINDOW_VISIBLE, (LPARAM)&gce);
	return 0;
}

/* Open new Groupchat

   Parameters:  szChatId = (char *)Name of new chat session
*/
int  __cdecl ChatStart(char *szChatId) {
	LOG(("ChatStart: New groupchat started"));
	if (!szChatId || NotifyEventHooks(hInitChat, (WPARAM)szChatId, 0)) return -1;
	return AddMembers(szChatId);
}


void KillChatSession(GCDEST *gcd) {
	GCEVENT gce = {0};

	LOG(("KillChatSession: Groupchatsession terminated."));
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = gcd;
	gcd->iType = GC_EVENT_CONTROL;
	CallService(MS_GC_EVENT, WINDOW_OFFLINE, (LPARAM)&gce);
	CallService(MS_GC_EVENT, WINDOW_TERMINATE, (LPARAM)&gce);
}

void InviteUser(char *szChatId) {
	HMENU tMenu = CreatePopupMenu();
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0), hInvitedUser;
	DBVARIANT dbv;
	HWND tWindow;
	POINT pt;
	CONTACTINFO ci = {0};
	gchat_contacts *gc;
	int j;

	if (!(gc=GetChat(szChatId))) return;

	// add the heading
	AppendMenu(tMenu, MF_STRING|MF_GRAYED|MF_DISABLED, (UINT_PTR)0, Translate("&Invite user..."));
	AppendMenu(tMenu, MF_SEPARATOR, (UINT_PTR)1, NULL);
    
    ci.cbSize = sizeof(ci);
	ci.szProto = SKYPE_PROTONAME;
	ci.dwFlag = CNF_DISPLAY;

	// generate a list of contact
	while (hContact) {
		if (!lstrcmp(SKYPE_PROTONAME, (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact,0 )) &&
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
            if (!alreadyInSession) {
				ci.hContact = hContact;
				if (!CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) {
					AppendMenu(tMenu, MF_STRING, (UINT_PTR)hContact, ci.pszVal);				
					miranda_sys_free (ci.pszVal);
                }
            }
        }
		hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact, 0);
	}

	tWindow = CreateWindow("EDIT","",0,1,1,1,1,NULL,NULL,hInst,NULL);

	GetCursorPos (&pt);
	hInvitedUser = (HANDLE)TrackPopupMenu(tMenu, TPM_NONOTIFY | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, tWindow, NULL);
	DestroyMenu(tMenu);
	DestroyWindow(tWindow);

	if (!hInvitedUser || DBGetContactSetting(hInvitedUser, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) 
		return;
	SkypeSend ("ALTER CHAT %s ADDMEMBERS %s", szChatId, dbv.pszVal);
	DBFreeVariant(&dbv);

}

void SetChatTopic (char *szChatId, char *szTopic)
{
	GCDEST gcd = {0};
	GCEVENT gce = {0};
	HANDLE hContact = find_chat (szChatId);

	gce.cbSize = sizeof(GCEVENT);
	gcd.pszModule = SKYPE_PROTONAME;
	gcd.pszID = szChatId;
	gcd.iType = GC_EVENT_TOPIC;
	gce.pDest = &gcd;
	gce.pszText = szTopic;
	gce.bAddToLog = TRUE;
	gce.time = time (NULL);
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
	gcd.iType = GC_EVENT_SETSBTEXT;
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

	SkypeSend ("ALTER CHAT %s SETTOPIC %s", szChatId, szTopic);
	testfor ("ALTER CHAT SETTOPIC", INFINITE);

	if (hContact)
		DBWriteContactSettingString(hContact, SKYPE_PROTONAME, "Nick", szTopic);
}


int GCEventHook(WPARAM wParam,LPARAM lParam) {
	GCHOOK *gch = (GCHOOK*) lParam;
	gchat_contacts *gc = GetChat(gch->pDest->pszID);

	if(gch) {
		if (!lstrcmpi(gch->pDest->pszModule, SKYPE_PROTONAME)) {

			switch (gch->pDest->iType) {
			case GC_USER_TERMINATE: {
				if (gc->mJoinedCount == 1) {
					// switch back to normal session
                    // I don't know if this behaviour isn't a bit annoying, therefore, we
					// don't do this now, until a user requests this feature :)
                    
					// open up srmm dialog when quit while 1 person left
//					CallService(MS_MSG_SENDMESSAGE, (WPARAM)gc->mJoinedContacts[0], 0);

					RemChatContact(gc, gc->mJoinedContacts[0]);
				}
				RemChat(gc->szChatName);
				break;
			}
			case GC_USER_MESSAGE:
				if(gch && gch->pszText && lstrlen(gch->pszText) > 0) {
					DBVARIANT dbv, dbv2;
					CCSDATA ccs = {0};
					GCDEST gcd = {0};
					GCEVENT gce = {0};

					// remove the ending linebreak
					while (gch->pszText[lstrlen(gch->pszText)-1] == '\r' || gch->pszText[lstrlen(gch->pszText)-1] == '\n') {
						gch->pszText[lstrlen(gch->pszText)-1] = '\0';
					}
                    // Send message to the chat-contact    
					if (ccs.hContact = find_chat(gch->pDest->pszID)) {
						ccs.lParam = (LPARAM)gch->pszText;
						CallProtoService(SKYPE_PROTONAME, PSS_MESSAGE, (WPARAM)0, (LPARAM)&ccs);
					}

					// Add our line to the chatlog	
					gcd.pszModule = gch->pDest->pszModule;
					gcd.pszID = gch->pDest->pszID;
					gcd.iType = GC_EVENT_MESSAGE;

					gce.cbSize = sizeof(GCEVENT);
					gce.pDest = &gcd;
					if (DBGetContactSetting(NULL, SKYPE_PROTONAME, "Nick", &dbv)) gce.pszNick=Translate("Me");
					else gce.pszNick = dbv.pszVal;
					DBGetContactSetting(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv2);
					gce.pszUID = dbv2.pszVal;
					gce.time = time(NULL);
					gce.pszText = gch->pszText;
					gce.bAddToLog = TRUE;
					gce.bIsMe = TRUE;
					CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
					if (dbv.pszVal) DBFreeVariant(&dbv);
					if (dbv2.pszVal) DBFreeVariant(&dbv2);
				}
				break;
			case GC_USER_CHANMGR:
				InviteUser(gch->pDest->pszID);
				break;
			case GC_USER_PRIVMESS: {
				HANDLE hContact = find_contact(gch->pszUID);
				if (hContact) CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
				break;

			}
			case GC_USER_LOGMENU:
				switch(gch->dwData) {
				case 10: InviteUser(gch->pDest->pszID); break;
				case 20: KillChatSession(gch->pDest); break;
                case 30: 
					{
						char *ptr, buf[MAX_BUF];

						ptr = SkypeGet ("CHAT", gch->pDest->pszID, "TOPIC");
						strcpy (buf, ptr);
						free(ptr);
						if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INPUTBOX), NULL, InputBoxDlgProc, (LPARAM)&buf))
							SetChatTopic (gch->pDest->pszID, buf);
						break;
					}
				}
				break;
			case GC_USER_NICKLISTMENU: {
				HANDLE hContact = find_contact(gch->pszUID);

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
	char* szInvite  = Translate("&Invite user...");
	char* szLeave   = Translate("&Leave chat session");
		char* szTopic   = Translate("Set &Topic...");
	char* szDetails = Translate("User &details");
	char* szHistory = Translate("User &history");

	static struct gc_item Item_log[] = {
		{NULL, 10, MENU_ITEM, FALSE},
		{NULL, 30, MENU_ITEM, FALSE},
		{NULL, 20, MENU_ITEM, FALSE}
	};
	static struct gc_item Item_nicklist_me[] = {
		{NULL, 20, MENU_ITEM, FALSE},
		{"", 100, MENU_SEPARATOR, FALSE},
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
		if (!lstrcmpi(gcmi->pszModule, SKYPE_PROTONAME)) {
			if(gcmi->Type == MENU_ON_LOG) {
				gcmi->nItems = sizeof(Item_log)/sizeof(Item_log[0]);
				gcmi->Item = &Item_log[0];
                LOG (("GCMenuHook: Items in log window: %d", gcmi->nItems));
			}
			if(gcmi->Type == MENU_ON_NICKLIST) {
				if (DBGetContactSetting(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) return -1;
				if (!lstrcmp(dbv.pszVal, (char *)gcmi->pszUID)) {
					gcmi->nItems = sizeof(Item_nicklist_me)/sizeof(Item_nicklist_me[0]);
					gcmi->Item = &Item_nicklist_me[0];
				} else {
					gcmi->nItems = sizeof(Item_nicklist)/sizeof(Item_nicklist[0]);
					gcmi->Item = &Item_nicklist[0];
				}
				DBFreeVariant(&dbv);
			}
        } else {LOG (("GCMenuHook: ERROR: Not our protocol."));}
	} else {LOG (("GCMenuHook: ERROR: No gcmi"));}
	LOG (("GCMenuHook: terminated."));
	return 0;
}
