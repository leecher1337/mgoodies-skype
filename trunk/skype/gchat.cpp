#include "skype.h"
#include "skypeapi.h"
#include "gchat.h"
#include "contacts.h"
#include "debug.h"
#include "../../include/m_langpack.h"
#include "../../include/m_userinfo.h"
#include "../../include/m_history.h"

extern char pszSkypeProtoName[MAX_PATH+30];
extern HANDLE hInitChat;
extern HINSTANCE hInst;

gchat_contacts *chats=NULL;
int chatcount=0;

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
		chats[chatcount-1].szChatName=strdup(szChatId);
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

	if (!(hContact=find_contact(who))) return -1;
	if ((i=ExistsChatContact(gc, hContact))>=0) return i;

	gcd.pszModule = pszSkypeProtoName;
	gcd.pszID = gc->szChatName;
	gcd.iType = GC_EVENT_JOIN;

	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.pszStatus = Translate("Others");
	gce.time = time(NULL);
	gce.bAddToLog = TRUE;

	if (!DBGetContactSetting(hContact, pszSkypeProtoName, "Nick", &dbv)) 
		gce.pszNick=dbv.pszVal; else gce.pszNick=who;
	gce.pszUID=who;
	if (CallService(MS_GC_EVENT, 0, (LPARAM)&gce)) {
		DBFreeVariant(&dbv);
		return -2;
	}
	DBFreeVariant(&dbv);

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
		if (szProto!=NULL && !strcmp(szProto, pszSkypeProtoName) &&
			DBGetContactSettingByte(hContact, pszSkypeProtoName, "ChatRoom", 0)==1)
		{
			if (DBGetContactSetting(hContact, pszSkypeProtoName, "ChatRoomID", &dbv)) continue;
            tCompareResult = strcmp(dbv.pszVal, chatname);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}


int  __cdecl AddMembers(char *szChatId) {
	BYTE *contactmask=NULL;
	DBVARIANT dbv, dbv2;
	char *ptr2, *str, *who;
	int i;
	gchat_contacts *gc;

	LOG("AddMembers", "STARTED"); 
	if (!(str=(char*)malloc(strlen(szChatId)+18)) ||
		!find_chat(szChatId)) return -1;
	sprintf(str, "GET CHAT %s MEMBERS", szChatId);
	if (SkypeSend(str)==-1 || !(ptr2=SkypeRcv(str+4, INFINITE))) {
		free(str);
		return -1;
	}
	who=strtok(ptr2+strlen(str+4)+1, " ");
	free(str);
	if (DBGetContactSetting(NULL, pszSkypeProtoName, SKYPE_NAME, &dbv2)) {
		free(ptr2);
		return -1;
	}

	if (!(gc=GetChat(szChatId))) return -1;
	// Add new contacts
	while (who) {
		if (strcmp(who, dbv2.pszVal)) {
			i=AddChatContact(gc, who);
			if (i!=-1 && !contactmask && !(contactmask= (unsigned char *)calloc(gc->mJoinedCount, 1))) i=-1;
			if (i==-1 || !(contactmask= (unsigned char *) realloc(contactmask, gc->mJoinedCount))) {
				if (contactmask) free(contactmask);
				free(ptr2);
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

		gcd.pszModule = pszSkypeProtoName;
		gcd.pszID = szChatId;
		gcd.iType = GC_EVENT_QUIT;

		gce.cbSize = sizeof(GCEVENT);
		gce.pDest = &gcd;
		gce.time = time(NULL);
		gce.bAddToLog = TRUE;

		for (i=0;i<gc->mJoinedCount;i++)
		if (!contactmask[i] &&
			!DBGetContactSetting(gc->mJoinedContacts[i], pszSkypeProtoName, SKYPE_NAME, &dbv)) 
		{
			DBFreeVariant(&dbv2);
			if (!DBGetContactSetting(gc->mJoinedContacts[i], pszSkypeProtoName, "Nick", &dbv2)) gce.pszNick=dbv2.pszVal;
			RemChatContact(gc, gc->mJoinedContacts[i]);
			gce.pszUID = dbv.pszVal;
			CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
			DBFreeVariant(&dbv);
		}
		free(contactmask);
		if (gc->mJoinedCount == 1) {
			// switch back to normal session
			KillChatSession(&gcd);
		}
	}
	free(ptr2);
	if (dbv2.pszVal) DBFreeVariant(&dbv2);
	LOG("AddMembers", "DONE");
	return 0;
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
	char *str, *ptr2;

	if (!wParam) return -1;
	gcw.cbSize = sizeof(GCWINDOW);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = pszSkypeProtoName;
	gcw.pszID = (char *)wParam;

	if (!(str= (char *) malloc(strlen(gcw.pszID)+16))) return -1;
	sprintf(str, "GET CHAT %s TOPIC", gcw.pszID);
	if (SkypeSend(str)==-1 || !(ptr2=SkypeRcv(str+4, INFINITE))) {
		free(str);
		return -1;
	}

	gcw.pszName = ptr2+strlen(str+4)+1;
	free(str);
	if (!gcw.pszName[0]) gcw.pszName=Translate("No Topic");
	gcw.pszStatusbarText = NULL;
	gcw.bDisableNickList = FALSE;
	if (CallService(MS_GC_NEWCHAT, 0, (LPARAM)&gcw)) {
		free(ptr2);
		return -1;
	}
	free(ptr2);

	gce.cbSize = sizeof(GCEVENT);
	gcd.pszModule = pszSkypeProtoName;
	gcd.pszID = (char *) gcw.pszID;
	gcd.iType = GC_EVENT_ADDGROUP;
	gce.pDest = &gcd;
	gce.pszStatus = Translate("Me");
	// BUG: Groupchat returns nonzero on success here
	if (!CallService(MS_GC_EVENT, 0, (LPARAM)&gce)) return -1;

	gcd.iType = GC_EVENT_JOIN;
	gce.pszStatus = Translate("Me");
	if (DBGetContactSetting(NULL, pszSkypeProtoName, "Nick", &dbv)) return -1;
	if (DBGetContactSetting(NULL, pszSkypeProtoName, SKYPE_NAME, &dbv2)) {
		DBFreeVariant(&dbv);
		return -1;
	}
	gce.pszNick = dbv.pszVal;
	gce.pszUID = dbv2.pszVal;
	gce.time = 0;
	gce.bIsMe = TRUE;
	gce.bAddToLog = FALSE;
	if (CallService(MS_GC_EVENT, 0, (LPARAM)&gce)) {
		DBFreeVariant(&dbv);
		DBFreeVariant(&dbv2);
		return -1;
	}
	DBFreeVariant(&dbv);
	DBFreeVariant(&dbv2);

	gcd.iType = GC_EVENT_ADDGROUP;
	gce.pDest = &gcd;
	gce.pszStatus = Translate("Others");
	// BUG: Groupchat returns nonzero on success here
	if (!CallService(MS_GC_EVENT, 0, (LPARAM)&gce)) return -1;

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
	LOG("ChatStart", "New groupchat started");
	if (!szChatId || NotifyEventHooks(hInitChat, (WPARAM)szChatId, 0)) return -1;
	return AddMembers(szChatId);
}


void KillChatSession(GCDEST *gcd) {
	GCEVENT gce = {0};

	LOG("KillChatSession", "Groupchatsession terminated.");
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = gcd;
	gcd->iType = GC_EVENT_CONTROL;
	CallService(MS_GC_EVENT, WINDOW_OFFLINE, (LPARAM)&gce);
	CallService(MS_GC_EVENT, WINDOW_TERMINATE, (LPARAM)&gce);
}

void InviteUser(char *szChatId) {
	HMENU tMenu = CreatePopupMenu();
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0), hInvitedUser;
	HWND tWindow;
	POINT pt;
	gchat_contacts *gc;
	int j;

	if (!(gc=GetChat(szChatId))) return;

	// add the heading
	AppendMenu(tMenu, MF_STRING|MF_GRAYED|MF_DISABLED, (UINT_PTR)0, Translate("&Invite user..."));
	AppendMenu(tMenu, MF_SEPARATOR, (UINT_PTR)1, NULL);

	// generate a list of contact
	while (hContact) {
		if (!lstrcmp(pszSkypeProtoName, (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact,0 ))) {
			if (!DBGetContactSettingByte(hContact, pszSkypeProtoName, "ChatRoom", 0)) {
				if (DBGetContactSettingWord(hContact, pszSkypeProtoName, "Status", ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE) {
					BOOL alreadyInSession = FALSE;
					for (j=0; j<gc->mJoinedCount; j++) {
						if (gc->mJoinedContacts[j]==hContact) {
							alreadyInSession = TRUE;
							break;
						}
					}
					if (!alreadyInSession) {
						DBVARIANT dbv;
					
						if (!DBGetContactSetting(hContact, pszSkypeProtoName, "Nick", &dbv)) {
							AppendMenu(tMenu, MF_STRING, (UINT_PTR)hContact, dbv.pszVal);
							DBFreeVariant(&dbv);
						}
					}
				}
		}	}
		hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact, 0);
	}

	tWindow = CreateWindow("EDIT","",0,1,1,1,1,NULL,NULL,hInst,NULL);

	GetCursorPos (&pt);
	hInvitedUser = (HANDLE)TrackPopupMenu(tMenu, TPM_NONOTIFY | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, tWindow, NULL);
	DestroyMenu(tMenu);
	DestroyWindow(tWindow);

	if (!hInvitedUser) return;

	// TODO:
	// Insert code to invite hInvitedUser
}


int GCEventHook(WPARAM wParam,LPARAM lParam) {
	GCHOOK *gch = (GCHOOK*) lParam;
	gchat_contacts *gc = GetChat(gch->pDest->pszID);

	if(gch) {
		if (!lstrcmpi(gch->pDest->pszModule, pszSkypeProtoName)) {

			switch (gch->pDest->iType) {
			case GC_USER_TERMINATE: {
				// open up srmm dialog when quit while 1 person left
				if (gc->mJoinedCount == 1) {
					// switch back to normal session
					CallService(MS_MSG_SENDMESSAGE, (WPARAM)gc->mJoinedContacts[0], 0);
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
					int i;

					// remove the ending linebreak
					while (gch->pszText[lstrlen(gch->pszText)-1] == '\r' || gch->pszText[lstrlen(gch->pszText)-1] == '\n') {
						gch->pszText[lstrlen(gch->pszText)-1] = '\0';
					}
/*
					if (ccs.hContact = find_chat(gch->pDest->pszID)) {
						ccs.lParam = (LPARAM)gch->pszText;
						CallProtoService(pszSkypeProtoName, PSS_MESSAGE, (WPARAM)0, (LPARAM)&ccs);
					}
*/
					// Temporary hack: Send to all users who are participating in this groupchat
					ccs.lParam = (LPARAM)gch->pszText;
					for (i=0;i<gc->mJoinedCount;i++) {
						ccs.hContact = gc->mJoinedContacts[i];
						CallProtoService(pszSkypeProtoName, PSS_MESSAGE, (WPARAM)0, (LPARAM)&ccs);
					}

					// Add our line to the chatlog	
					gcd.pszModule = gch->pDest->pszModule;
					gcd.pszID = gch->pDest->pszID;
					gcd.iType = GC_EVENT_MESSAGE;

					gce.cbSize = sizeof(GCEVENT);
					gce.pDest = &gcd;
					if (DBGetContactSetting(NULL, pszSkypeProtoName, "Nick", &dbv)) gce.pszNick=Translate("Me");
					else gce.pszNick = dbv.pszVal;
					DBGetContactSetting(NULL, pszSkypeProtoName, SKYPE_NAME, &dbv2);
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
	char* szDetails = Translate("User &details");
	char* szHistory = Translate("User &history");

	struct gc_item Item_log[] = {
		{szInvite, 10, MENU_ITEM, FALSE},
		{szLeave, 20, MENU_ITEM, FALSE}
	};
	struct gc_item Item_nicklist_me[] = {
		{szHistory, 20, MENU_ITEM, FALSE},
		{"", 100, MENU_SEPARATOR, FALSE},
		{szLeave, 110, MENU_ITEM, FALSE}
	};
	struct gc_item Item_nicklist[] = {
		{szDetails, 10, MENU_ITEM, FALSE},
		{szHistory, 20, MENU_ITEM, FALSE}
	};

	if(gcmi) {
		if (!lstrcmpi(gcmi->pszModule, pszSkypeProtoName)) {
			if(gcmi->Type == MENU_ON_LOG) {
				gcmi->nItems = sizeof(Item_log)/sizeof(Item_log[0]);
				gcmi->Item = &Item_log[0];
			}
			if(gcmi->Type == MENU_ON_NICKLIST) {
				if (DBGetContactSetting(NULL, pszSkypeProtoName, SKYPE_NAME, &dbv)) return -1;
				if (!lstrcmp(dbv.pszVal, (char *)gcmi->pszUID)) {
					gcmi->nItems = sizeof(Item_nicklist_me)/sizeof(Item_nicklist_me[0]);
					gcmi->Item = &Item_nicklist_me[0];
				} else {
					gcmi->nItems = sizeof(Item_nicklist)/sizeof(Item_nicklist[0]);
					gcmi->Item = &Item_nicklist[0];
				}
				DBFreeVariant(&dbv);
			}
		}
	}
	return 0;
}
