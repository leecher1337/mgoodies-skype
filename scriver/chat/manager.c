/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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
#include "chat.h"

extern char*	pszActiveWndID ;
extern char*	pszActiveWndModule ;
extern SESSION_INFO	g_TabSession;
extern HICON	hIcons[30];
extern HIMAGELIST hIconsList;
extern int eventMessageIcon;
extern int overlayIcon;
extern struct MM_INTERFACE		mmi ;

extern struct GlobalMessageData *g_dat;

#define WINDOWS_COMMANDS_MAX 30
#define	STATUSICONCOUNT 6

SESSION_INFO * m_WndList = 0;
TABLIST * g_TabList = 0;
MODULEINFO *m_ModList = 0;

void SetActiveSession(char * pszID, char * pszModule)
{
	SESSION_INFO * si = SM_FindSession(pszID, pszModule);
	if(si)
		SetActiveSessionEx(si);
}

void SetActiveSessionEx(SESSION_INFO * si)
{
	if(si)
	{
		pszActiveWndID = realloc(pszActiveWndID, lstrlenA(si->pszID)+1);
		lstrcpynA(pszActiveWndID, si->pszID, lstrlenA(si->pszID)+1);
		pszActiveWndModule = realloc(pszActiveWndModule, lstrlenA(si->pszModule)+1);
		lstrcpynA(pszActiveWndModule, si->pszModule, lstrlenA(si->pszModule)+1);
	}

}



SESSION_INFO * GetActiveSession(void)
{
	SESSION_INFO *  si = SM_FindSession(pszActiveWndID, pszActiveWndModule);
	if(si)
		return si;
	if (m_WndList)
		return m_WndList;
	return NULL;
}


//---------------------------------------------------
//		Session Manager functions
//
//		Keeps track of all sessions and its windows
//---------------------------------------------------




SESSION_INFO * SM_AddSession(char * pszID, char * pszModule)
{
	if(!pszID || !pszModule)
		return NULL;

	if (!SM_FindSession(pszID, pszModule))
	{
		SESSION_INFO *node = (SESSION_INFO*) malloc(sizeof(SESSION_INFO));
		ZeroMemory(node, sizeof(SESSION_INFO));
		node->pszID = (char*) malloc(lstrlenA(pszID) + 1);
		node->pszModule = (char*)malloc(lstrlenA(pszModule) + 1);
		lstrcpyA(node->pszModule, pszModule);
		lstrcpyA(node->pszID, pszID);

		if (m_WndList == NULL) // list is empty
		{
			m_WndList = node;
			node->next = NULL;
		}
		else
		{
			node->next = m_WndList;
			m_WndList = node;
		}
		return node;
	}
	return NULL;
}

int SM_RemoveSession(char * pszID, char * pszModule)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszModule)
		return FALSE;

	while (pTemp != NULL)
	{
		if ((!pszID && pTemp->iType != GCW_SERVER || !lstrcmpiA(pTemp->pszID,pszID)) && !lstrcmpiA(pTemp->pszModule,pszModule)) // match
		{
			COMMAND_INFO *pCurComm;
			DWORD dw = pTemp->dwItemData;

			if(!g_Settings.TabsEnable)
			{
				if(pTemp->hWnd )
					SendMessage(pTemp->hWnd, GC_EVENT_CONTROL+WM_USER+500, SESSION_TERMINATE, 0);
			}

			if(pTemp->hWnd)
				g_TabSession.nUsersInNicklist = 0;

			if (pLast == NULL)
				m_WndList = pTemp->next;
			else
				pLast->next = pTemp->next;

			DoEventHook(pTemp->pszID, pTemp->pszModule, GC_SESSION_TERMINATE, NULL, NULL, (DWORD)pTemp->dwItemData);

			UM_RemoveAll(&pTemp->pUsers);
			TM_RemoveAll(&pTemp->pStatuses);
			LM_RemoveAll(&pTemp->pLog, &pTemp->pLogEnd);
			pTemp->iStatusCount = 0;
			pTemp->nUsersInNicklist = 0;

			if(pTemp->hContact)
			{
				CList_SetOffline(pTemp->hContact, pTemp->iType == GCW_CHATROOM?TRUE:FALSE);
				if(pTemp->iType != GCW_SERVER)
				DBWriteContactSettingByte(pTemp->hContact, "CList", "Hidden", 1);
			}
			DBWriteContactSettingString(pTemp->hContact, pTemp->pszModule , "Topic", "");
			DBWriteContactSettingString(pTemp->hContact, pTemp->pszModule, "StatusBar", "");
			DBDeleteContactSetting(pTemp->hContact, "CList", "StatusMsg");

			free(pTemp->pszID);
			free(pTemp->pszModule);
			if(pTemp->pszName)
				free(pTemp->pszName);
			if(pTemp->pszStatusbarText)
				free(pTemp->pszStatusbarText);
			if(pTemp->pszTopic)
				free(pTemp->pszTopic);

			// delete commands
			pCurComm = pTemp->lpCommands;
			while (pCurComm != NULL)
			{
				COMMAND_INFO *pNext = pCurComm->next;
				free(pCurComm->lpCommand);
				free(pCurComm);
				pCurComm = pNext;
			}

			free(pTemp);
			if(pszID)
				return (int)dw;
			if(pLast)
				pTemp = pLast->next;
			else
				pTemp = m_WndList;
		}
		else
		{
			pLast = pTemp;
			pTemp = pTemp->next;
		}
	}
	return FALSE;
}

SESSION_INFO * SM_FindSession(char *pszID, char * pszModule)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszID || !pszModule)
		return NULL;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszID,pszID) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			return pTemp;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return NULL;
}
BOOL SM_SetOffline(char *pszID, char * pszModule)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszModule)
		return FALSE;

	while (pTemp != NULL)
	{
		if ((!pszID || !lstrcmpiA(pTemp->pszID,pszID)) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			UM_RemoveAll(&pTemp->pUsers);
			pTemp->nUsersInNicklist = 0;
			if(pTemp->hWnd)
				g_TabSession.nUsersInNicklist = 0;
			if(pTemp->iType != GCW_SERVER)
				pTemp->bInitDone = FALSE;

			if(pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}
BOOL SM_SetStatusEx(char *pszID, char * pszModule, char * pszText, int flags )
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszModule)
		return FALSE;

	while (pTemp != NULL)
	{
		if ((!pszID || !lstrcmpiA(pTemp->pszID,pszID)) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			UM_SetStatusEx(pTemp->pUsers, pszText, flags);
			if(pTemp->hWnd)
				RedrawWindow(GetDlgItem(pTemp->hWnd, IDC_CHAT_LIST), NULL, NULL, RDW_INVALIDATE);
			if(pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}
HICON SM_GetStatusIcon(SESSION_INFO * si, USERINFO * ui)
{
	STATUSINFO * ti;
	if(!ui || !si)
		return NULL;

	ti = TM_FindStatus(si->pStatuses, TM_WordToString(si->pStatuses, ui->Status));
	if (ti)
	{
		if((int)ti->hIcon < STATUSICONCOUNT)
		{
			int id = si->iStatusCount - (int)ti->hIcon - 1;
			if(id == 0)
				return hIcons[ICON_STATUS0];
			if(id == 1)
				return hIcons[ICON_STATUS1];
			if(id == 2)
				return hIcons[ICON_STATUS2];
			if(id == 3)
				return hIcons[ICON_STATUS3];
			if(id == 4)
				return hIcons[ICON_STATUS4];
			if(id == 5)
				return hIcons[ICON_STATUS5];
		}
		else
			return ti->hIcon;
	}
	return hIcons[ICON_STATUS0];
}

BOOL SM_AddEventToAllMatchingUID(GCEVENT * gce)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;
	int bManyFix = 0;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszModule,gce->pDest->pszModule))
		{
			if(UM_FindUser(pTemp->pUsers, (char *)gce->pszUID))
			{
				if(pTemp->bInitDone)
				{
					if(SM_AddEvent(pTemp->pszID, pTemp->pszModule, gce, FALSE) && pTemp->hWnd && pTemp->bInitDone)
					{
						g_TabSession.pLog = pTemp->pLog;
						g_TabSession.pLogEnd = pTemp->pLogEnd;
						SendMessage(pTemp->hWnd, GC_ADDLOG, 0, 0);
					}
					else if(pTemp->hWnd && pTemp->bInitDone)
					{
						g_TabSession.pLog = pTemp->pLog;
						g_TabSession.pLogEnd = pTemp->pLogEnd;
						SendMessage(pTemp->hWnd, GC_REDRAWLOG2, 0, 0);
					}
					DoSoundsFlashPopupTrayStuff(pTemp, gce, FALSE, bManyFix);
					bManyFix ++;
					if(gce->bAddToLog && g_Settings.LoggingEnabled)
						LogToFile(pTemp, gce);
				}
			}
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return 0;

}
BOOL SM_AddEvent(char *pszID, char * pszModule, GCEVENT * gce, BOOL bIsHighlighted)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszID || !pszModule)
		return TRUE;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszID,pszID) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			LOGINFO * li = LM_AddEvent(&pTemp->pLog, &pTemp->pLogEnd);
			pTemp->iEventCount += 1;

			li->iType = gce->pDest->iType;
			if(gce->pszNick )
			{
				li->pszNick = (char*)malloc(lstrlenA(gce->pszNick) + 1);
				lstrcpyA(li->pszNick, gce->pszNick);
			}
			if(gce->pszText)
			{
				li->pszText = (char*)malloc(lstrlenA(gce->pszText) + 1);
				lstrcpyA(li->pszText, gce->pszText);
			}
			if(gce->pszStatus)
			{
				li->pszStatus = (char*)malloc(lstrlenA(gce->pszStatus) + 1);
				lstrcpyA(li->pszStatus, gce->pszStatus);
			}
			if(gce->pszUserInfo)
			{
				li->pszUserInfo = (char*)malloc(lstrlenA(gce->pszUserInfo) + 1);
				lstrcpyA(li->pszUserInfo, gce->pszUserInfo);
			}

			li->bIsMe = gce->bIsMe;
			li->time = gce->time;
			li->bIsHighlighted = bIsHighlighted;

			if (g_Settings.iEventLimit > 0 && pTemp->iEventCount > g_Settings.iEventLimit + 20)
			{
				LM_TrimLog(&pTemp->pLog, &pTemp->pLogEnd, pTemp->iEventCount - g_Settings.iEventLimit);
				pTemp->iEventCount = g_Settings.iEventLimit;
				return FALSE;
			}
			return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}

USERINFO * SM_AddUser(char *pszID, char * pszModule, char * pszUID, char * pszNick, WORD wStatus)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszID || !pszModule)
		return NULL;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszID,pszID) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			USERINFO * p = UM_AddUser(pTemp->pStatuses, &pTemp->pUsers, pszUID, pszNick, wStatus);
			pTemp->nUsersInNicklist++;
			if(pTemp->hWnd)
				g_TabSession.nUsersInNicklist ++;
			return p;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return 0;
}

BOOL SM_MoveUser(char *pszID, char * pszModule, char * pszUID)
{
	SESSION_INFO *pTemp = m_WndList;

	if(!pszID || !pszModule || !pszUID)
		return FALSE;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszID,pszID) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			UM_SortUser(&pTemp->pUsers, pszUID);
			return TRUE;
		}
		pTemp = pTemp->next;
	}

	return FALSE;
}

BOOL SM_RemoveUser(char *pszID, char * pszModule, char * pszUID)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszModule || !pszUID)
		return FALSE;

	while (pTemp != NULL)
	{
		if ((!pszID || !lstrcmpiA(pTemp->pszID,pszID)) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			DWORD dw;
			USERINFO * ui = UM_FindUser(pTemp->pUsers, pszUID);
			if(ui)
			{
				pTemp->nUsersInNicklist--;
				if(pTemp->hWnd)
				{
					g_TabSession.pUsers = pTemp->pUsers;
					g_TabSession.nUsersInNicklist --;
				}

				dw = UM_RemoveUser(&pTemp->pUsers, pszUID);

				if(pTemp->hWnd)
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);

				if(pszID)
					return TRUE;
			}
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return 0;
}

USERINFO * SM_GetUserFromIndex(char *pszID, char * pszModule, int index)
{
	SESSION_INFO *pTemp = m_WndList;

	if(!pszModule)
		return FALSE;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszID,pszID) && !lstrcmpiA(pTemp->pszModule,pszModule))
			return UM_FindUserFromIndex(pTemp->pUsers, index);
		pTemp = pTemp->next;
	}

	return NULL;
}


STATUSINFO * SM_AddStatus(char *pszID, char * pszModule, char * pszStatus)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszID || !pszModule )
		return NULL;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszID,pszID) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			STATUSINFO * ti = TM_AddStatus(&pTemp->pStatuses, pszStatus, &pTemp->iStatusCount);
			if(ti)
				pTemp->iStatusCount++;
			return ti;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return 0;
}

BOOL SM_GiveStatus(char *pszID, char * pszModule, char * pszUID,  char * pszStatus)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszID || !pszModule )
		return FALSE;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszID,pszID) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			USERINFO * ui = UM_GiveStatus(pTemp->pUsers, pszUID, TM_StringToWord(pTemp->pStatuses, pszStatus));
			if (ui) {
				SM_MoveUser(pTemp->pszID, pTemp->pszModule, ui->pszUID);
				if(pTemp->hWnd)
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);
			}
			return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return FALSE;
}
BOOL SM_TakeStatus(char *pszID, char * pszModule, char * pszUID,  char * pszStatus)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszID || !pszModule )
		return FALSE;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszID,pszID) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			USERINFO * ui = UM_TakeStatus(pTemp->pUsers, pszUID, TM_StringToWord(pTemp->pStatuses, pszStatus));
			SM_MoveUser(pTemp->pszID, pTemp->pszModule, ui->pszUID);
			if(pTemp->hWnd)
				SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);
			return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return FALSE;
}
LRESULT SM_SendMessage(char *pszID, char * pszModule, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	while (pTemp && pszModule)
	{
		if ((!pszID ||!lstrcmpiA(pTemp->pszID,pszID))  && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			if(pTemp->hWnd)
			{
				LRESULT i = SendMessage(pTemp->hWnd, msg, wParam, lParam);
				if (pszID)
					return i;
			}
			if(pszID)
				return 0;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

BOOL SM_PostMessage(char *pszID, char * pszModule, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszID || !pszModule)
		return 0;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszID,pszID) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			if(pTemp->hWnd)
				return PostMessage(pTemp->hWnd, msg, wParam, lParam);
			return FALSE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return FALSE;
}

BOOL SM_BroadcastMessage(char * pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	while (pTemp != NULL)
	{
		if(!pszModule || !lstrcmpiA(pTemp->pszModule, pszModule))
		{
			if(pTemp->hWnd)
			{
				if (bAsync)
					PostMessage(pTemp->hWnd, msg, wParam, lParam);
				else
					SendMessage(pTemp->hWnd, msg, wParam, lParam);
			}

		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}
BOOL SM_SetStatus(char *pszID, char * pszModule, int wStatus)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszModule)
		return FALSE;

	while (pTemp != NULL)
	{
		if ((!pszID || !lstrcmpiA(pTemp->pszID,pszID)) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			pTemp->wStatus = wStatus;

			if(pTemp->hContact)
			{
				if(pTemp->iType != GCW_SERVER)
				{
					if(wStatus != ID_STATUS_OFFLINE)
						DBDeleteContactSetting(pTemp->hContact, "CList", "Hidden");
				}
				DBWriteContactSettingWord(pTemp->hContact, pTemp->pszModule, "Status", (WORD)wStatus);

			}

			PostMessage(pTemp->hWnd, GC_FIXTABICONS, 0, 0);

			if(pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}
BOOL SM_SendUserMessage(char *pszID, char * pszModule, char * pszText)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszModule || !pszText)
		return FALSE;

	while (pTemp != NULL)
	{
		if ((!pszID || !lstrcmpiA(pTemp->pszID,pszID)) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			if(pTemp->iType == GCW_CHATROOM)
				DoEventHook(pTemp->pszID, pTemp->pszModule, GC_USER_MESSAGE, NULL, pszText, (LPARAM)NULL);
			if(pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}
SESSION_INFO * SM_GetPrevWindow(SESSION_INFO * si)
{
	BOOL bFound = FALSE;
	SESSION_INFO *pTemp = m_WndList;

	if(!si)
		return NULL;

	while (pTemp != NULL)
	{
		if (si == pTemp)
		{
			if(bFound)
				return NULL;
			else
				bFound = TRUE;
		}
		else if (bFound == TRUE && pTemp->hWnd)
			return pTemp;
		pTemp = pTemp->next;
		if(pTemp == NULL && bFound)
			pTemp = m_WndList;
	}
	return NULL;
}
SESSION_INFO * SM_GetNextWindow(SESSION_INFO * si)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!si)
		return NULL;

	while (pTemp != NULL)
	{
		if (si == pTemp)
		{
			if(pLast)
			{
				if(pLast != pTemp)
					return pLast;
				else
					return NULL;
			}
		}
		if (pTemp->hWnd)
			pLast = pTemp;
		pTemp = pTemp->next;
		if(pTemp == NULL)
			pTemp = m_WndList;
	}
	return NULL;
}
BOOL SM_ChangeUID(char *pszID, char * pszModule, char * pszUID, char * pszNewUID)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszModule)
		return FALSE;

	while (pTemp != NULL)
	{
		if ((!pszID || !lstrcmpiA(pTemp->pszID,pszID)) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			USERINFO * ui = UM_FindUser(pTemp->pUsers, pszUID);
			if(ui)
			{
				ui->pszUID = (char *)realloc(ui->pszUID, lstrlenA(pszNewUID) + 1);
				lstrcpynA(ui->pszUID, pszNewUID, lstrlenA(pszNewUID) + 1);
			}

			if(pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}


BOOL SM_SetTabbedWindowHwnd(SESSION_INFO * si, HWND hwnd)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	while (pTemp != NULL)
	{
		if (si && si == pTemp)
		{
			pTemp->hWnd = hwnd;
		}
		else
			pTemp->hWnd = NULL;
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}
BOOL SM_ChangeNick(char *pszID, char * pszModule, GCEVENT * gce)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if(!pszModule)
		return FALSE;

	while (pTemp != NULL)
	{
		if ((!pszID || !lstrcmpiA(pTemp->pszID,pszID)) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			USERINFO * ui = UM_FindUser(pTemp->pUsers, (char *)gce->pszUID);
			if(ui)
			{
				ui->pszNick = (char *)realloc(ui->pszNick, lstrlenA(gce->pszText) + 1);
				lstrcpynA(ui->pszNick, gce->pszText, lstrlenA(gce->pszText) + 1);
				SM_MoveUser(pTemp->pszID, pTemp->pszModule, ui->pszUID);
				if(pTemp->hWnd)
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);
			}

			if(pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}
BOOL SM_RemoveAll (void)
{
	while (m_WndList)
    {
		SESSION_INFO *pLast = m_WndList->next;

		if(m_WndList->hWnd)
			SendMessage(m_WndList->hWnd, GC_EVENT_CONTROL+WM_USER+500, SESSION_TERMINATE, 0);
		DoEventHook(m_WndList->pszID, m_WndList->pszModule, GC_SESSION_TERMINATE, NULL, NULL, (DWORD)m_WndList->dwItemData);
		if(m_WndList->hContact)
			CList_SetOffline(m_WndList->hContact, m_WndList->iType == GCW_CHATROOM?TRUE:FALSE);
		DBWriteContactSettingString(m_WndList->hContact, m_WndList->pszModule , "Topic", "");
		DBDeleteContactSetting(m_WndList->hContact, "CList", "StatusMsg");
		DBWriteContactSettingString(m_WndList->hContact, m_WndList->pszModule, "StatusBar", "");

		UM_RemoveAll(&m_WndList->pUsers);
		TM_RemoveAll(&m_WndList->pStatuses);
		LM_RemoveAll(&m_WndList->pLog, &m_WndList->pLogEnd);
		m_WndList->iStatusCount = 0;
		m_WndList->nUsersInNicklist = 0;

		free (m_WndList->pszID);
		free (m_WndList->pszModule);
		if(m_WndList->pszName)
			free(m_WndList->pszName);
		if(m_WndList->pszStatusbarText)
			free(m_WndList->pszStatusbarText);
		if(m_WndList->pszTopic)
			free(m_WndList->pszTopic);

		while (m_WndList->lpCommands != NULL)
		{
			COMMAND_INFO *pNext = m_WndList->lpCommands->next;
			free(m_WndList->lpCommands->lpCommand);
			free(m_WndList->lpCommands);
			m_WndList->lpCommands = pNext;
		}

		free (m_WndList);
		m_WndList = pLast;
	}
	m_WndList = NULL;
	return TRUE;
}



void SM_AddCommand(char *pszID, char * pszModule, const char *lpNewCommand)
{
	SESSION_INFO *pTemp = m_WndList;
	while (pTemp != NULL)
	{
		if (lstrcmpiA(pTemp->pszID,pszID) == 0 && lstrcmpiA(pTemp->pszModule,pszModule) == 0) // match
		{
			COMMAND_INFO *node = malloc(sizeof(COMMAND_INFO));
			node->lpCommand = malloc(lstrlenA(lpNewCommand) + 1);
			lstrcpyA(node->lpCommand,lpNewCommand);
			node->last = NULL; // always added at beginning!

			// new commands are added at start
			if (pTemp->lpCommands == NULL)
			{
				node->next = NULL;
				pTemp->lpCommands = node;
			}
			else
			{
				node->next = pTemp->lpCommands;
				pTemp->lpCommands->last = node; // hmm, weird
				pTemp->lpCommands = node;
			}
			pTemp->lpCurrentCommand = NULL; // current command
			pTemp->wCommandsNum++;

			if (pTemp->wCommandsNum > WINDOWS_COMMANDS_MAX)
			{
				COMMAND_INFO *pCurComm = pTemp->lpCommands;
				COMMAND_INFO *pLast;
				while (pCurComm->next != NULL) { pCurComm = pCurComm->next; }
				pLast = pCurComm->last;
				free(pCurComm->lpCommand);
				free(pCurComm);
				pLast->next = NULL;
				// done
				pTemp->wCommandsNum--;
			}
		}
		pTemp = pTemp->next;
	}
}


char *SM_GetPrevCommand(char *pszID, char * pszModule) // get previous command. returns NULL if previous command does not exist. current command remains as it was.
{
	SESSION_INFO *pTemp = m_WndList;
	while (pTemp != NULL)
	{
		if (lstrcmpiA(pTemp->pszID,pszID) == 0 && lstrcmpiA(pTemp->pszModule,pszModule) == 0) // match
		{
			COMMAND_INFO *pPrevCmd = NULL;
			if (pTemp->lpCurrentCommand != NULL)
			{
				if (pTemp->lpCurrentCommand->next != NULL) // not NULL
				{
					pPrevCmd = pTemp->lpCurrentCommand->next; // next command (newest at beginning)
				}
				else
				{
					pPrevCmd = pTemp->lpCurrentCommand;
				}
			}
			else
			{
				pPrevCmd = pTemp->lpCommands;
			}

			pTemp->lpCurrentCommand = pPrevCmd; // make it the new command

			return(((pPrevCmd) ? (pPrevCmd->lpCommand) : (NULL)));
		}
		pTemp = pTemp->next;
	}
	return(NULL);
}


char *SM_GetNextCommand(char *pszID, char * pszModule) // get next command. returns NULL if next command does not exist. current command becomes NULL (a prev command after this one will get you the last command)
{
	SESSION_INFO *pTemp = m_WndList;
	while (pTemp != NULL)
	{
		if (lstrcmpiA(pTemp->pszID,pszID) == 0 && lstrcmpiA(pTemp->pszModule,pszModule) == 0) // match
		{
			COMMAND_INFO *pNextCmd = NULL;
			if (pTemp->lpCurrentCommand != NULL)
			{
				pNextCmd = pTemp->lpCurrentCommand->last; // last command (newest at beginning)
			}

			pTemp->lpCurrentCommand = pNextCmd; // make it the new command

			return(((pNextCmd) ? (pNextCmd->lpCommand) : (NULL)));
		}
		pTemp = pTemp->next;
	}
	return(NULL);
}

int	SM_GetCount(char * pszModule)
{
	SESSION_INFO *pTemp = m_WndList;
	int count = 0;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pszModule, pTemp->pszModule))
			count++;

		pTemp = pTemp->next;
	}
	return count;
}
SESSION_INFO *	SM_FindSessionByIndex(char * pszModule, int iItem)
{
	SESSION_INFO *pTemp = m_WndList;
	int count = 0;
	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pszModule, pTemp->pszModule))
		{
			if(iItem ==count)
				return pTemp;
			else
				count++;
		}

		pTemp = pTemp->next;
	}
	return NULL;

}
char * SM_GetUsers(SESSION_INFO * si)
{
	SESSION_INFO *pTemp = m_WndList;
	int count = 0;

	while (pTemp != NULL)
	{
		if (si && si == pTemp)
		{
			USERINFO * utemp = pTemp->pUsers;
			if(utemp)
			{
				char * p = mmi.mmi_malloc(4096);
				int alloced = 4096;
				lstrcpyA(p, utemp->pszUID);
				lstrcatA(p, " ");
				utemp = utemp->next;
				while(utemp)
				{
					if(lstrlenA(p) + lstrlenA(utemp->pszUID) > alloced - 10)
					{
						char *p2 = mmi.mmi_malloc(alloced + 4096);
						lstrcpyA(p2, p);
						mmi.mmi_free(p);
						p = p2;
					}
					lstrcatA(p, utemp->pszUID);
					lstrcatA(p, " ");
					utemp = utemp->next;
				}
				return p;
			}
			return NULL;

		}
		pTemp = pTemp->next;
	}
	return NULL;
}






//---------------------------------------------------
//		Module Manager functions
//
//		Necessary to keep track of all modules
//		that has registered with the plugin
//---------------------------------------------------


MODULEINFO * MM_AddModule(char * pszModule)
{
	if(!pszModule)
		return NULL;
	if (!MM_FindModule(pszModule))
	{
		MODULEINFO *node = (MODULEINFO*) malloc(sizeof(MODULEINFO));
		ZeroMemory(node, sizeof(MODULEINFO));

		node->pszModule = (char*)malloc(lstrlenA(pszModule) + 1);
		lstrcpyA(node->pszModule, pszModule);

		if (m_ModList == NULL) // list is empty
		{
			m_ModList = node;
			node->next = NULL;
		}
		else
		{
			node->next = m_ModList;
			m_ModList = node;
		}
		return node;
	}
	return FALSE;
}

void MM_IconsChanged(void)
{
	MODULEINFO *pTemp = m_ModList, *pLast = NULL;
	ImageList_ReplaceIcon(g_dat->hTabIconList, eventMessageIcon, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
	ImageList_ReplaceIcon(g_dat->hTabIconList, overlayIcon, LoadIconEx(IDI_OVERLAY, "overlay", 0, 0));
	while (pTemp != NULL)
	{
		pTemp->OnlineIconIndex = ImageList_ReplaceIcon(g_dat->hTabIconList, pTemp->OnlineIconIndex, LoadSkinnedProtoIcon(pTemp->pszModule, ID_STATUS_ONLINE));
		pTemp->OfflineIconIndex = ImageList_ReplaceIcon(g_dat->hTabIconList, pTemp->OfflineIconIndex, LoadSkinnedProtoIcon(pTemp->pszModule, ID_STATUS_OFFLINE));

		if(pTemp->hOfflineIcon)
			DestroyIcon(pTemp->hOfflineIcon);
		if(pTemp->hOnlineIcon)
			DestroyIcon(pTemp->hOnlineIcon);
		if(pTemp->hOnlineTalkIcon)
			DestroyIcon(pTemp->hOnlineTalkIcon);
		if(pTemp->hOfflineTalkIcon)
			DestroyIcon(pTemp->hOfflineTalkIcon);
		pTemp->hOfflineIcon = ImageList_GetIcon(g_dat->hTabIconList, pTemp->OfflineIconIndex, ILD_TRANSPARENT);
		pTemp->hOnlineIcon = ImageList_GetIcon(g_dat->hTabIconList, pTemp->OnlineIconIndex, ILD_TRANSPARENT);

		pTemp->hOnlineTalkIcon = ImageList_GetIcon(g_dat->hTabIconList, pTemp->OnlineIconIndex, ILD_TRANSPARENT|INDEXTOOVERLAYMASK(1));
		ImageList_ReplaceIcon(g_dat->hTabIconList, pTemp->OnlineIconIndex+1, pTemp->hOnlineTalkIcon);

		pTemp->hOfflineTalkIcon = ImageList_GetIcon(g_dat->hTabIconList, pTemp->OfflineIconIndex, ILD_TRANSPARENT|INDEXTOOVERLAYMASK(1));
		ImageList_ReplaceIcon(g_dat->hTabIconList, pTemp->OfflineIconIndex+1, pTemp->hOfflineTalkIcon);

		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return;
}
void MM_FontsChanged(void)
{
	MODULEINFO *pTemp = m_ModList;
	while (pTemp != NULL)
	{
		pTemp->pszHeader = Log_CreateRtfHeader(pTemp);
		pTemp = pTemp->next;
	}
	return;
}
MODULEINFO* MM_FindModule(char* pszModule)
{
	MODULEINFO *pTemp = m_ModList, *pLast = NULL;

	if(!pszModule)
		return NULL;

	while (pTemp != NULL)
	{
		if (lstrcmpiA(pTemp->pszModule,pszModule) == 0)
		{
			return pTemp;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

// stupid thing..
void MM_FixColors()
{
	MODULEINFO *pTemp = m_ModList;

	while (pTemp != NULL)
	{
		CheckColorsInModule(pTemp->pszModule);
		pTemp = pTemp->next;
	}
	return;
}

BOOL MM_RemoveAll (void)
{
	while (m_ModList != NULL)
    {
		MODULEINFO *pLast = m_ModList->next;
		free (m_ModList->pszModule);
		free(m_ModList->pszModDispName);
		if(m_ModList->pszHeader)
			free(m_ModList->pszHeader);
		if(m_ModList->crColors)
			free (m_ModList->crColors);
		if(m_ModList->hOfflineIcon)
			DestroyIcon(m_ModList->hOfflineIcon);
		if(m_ModList->hOnlineIcon)
			DestroyIcon(m_ModList->hOnlineIcon);
		if(m_ModList->hOnlineTalkIcon)
			DestroyIcon(m_ModList->hOnlineTalkIcon);
		if(m_ModList->hOfflineTalkIcon)
			DestroyIcon(m_ModList->hOfflineTalkIcon);
		free (m_ModList);
		m_ModList = pLast;
    }
	m_ModList = NULL;
	return TRUE;
}

//---------------------------------------------------
//		Status manager functions
//
//		Necessary to keep track of what user statuses
//		per window nicklist that is available
//---------------------------------------------------

STATUSINFO * TM_AddStatus(STATUSINFO** ppStatusList, char * pszStatus, int * iCount)
{
	if(!ppStatusList || !pszStatus)
		return NULL;

	if (!TM_FindStatus(*ppStatusList, pszStatus))
	{
		STATUSINFO *node = (STATUSINFO*) malloc(sizeof(STATUSINFO));
		ZeroMemory(node, sizeof(STATUSINFO));

		node->pszGroup = (char *) malloc(lstrlenA(pszStatus) + 1);
		lstrcpyA(node->pszGroup, pszStatus);

		node->hIcon = (HICON)(*iCount);
		while ((int)node->hIcon > STATUSICONCOUNT - 1)
			node->hIcon--;

		if (*ppStatusList == NULL) // list is empty
		{
			node->Status = 1;
			*ppStatusList = node;
			node->next = NULL;
		}
		else
		{
			node->Status = ppStatusList[0]->Status*2;
			node->next = *ppStatusList;
			*ppStatusList = node;
		}
		return node;

	}
	return FALSE;
}

STATUSINFO * TM_FindStatus(STATUSINFO* pStatusList, char* pszStatus)
{
	STATUSINFO *pTemp = pStatusList, *pLast = NULL;

	if(!pStatusList || !pszStatus)
	return NULL;

	while (pTemp != NULL)
	{
		if (lstrcmpiA(pTemp->pszGroup,pszStatus) == 0)
		{
			return pTemp;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

WORD TM_StringToWord(STATUSINFO* pStatusList, char* pszStatus)
{
	STATUSINFO *pTemp = pStatusList, *pLast = NULL;

	if(!pStatusList || !pszStatus)
	return 0;

	while (pTemp != NULL)
	{
		if (lstrcmpiA(pTemp->pszGroup,pszStatus) == 0)
		{
			return pTemp->Status;
		}
		if (pTemp->next == NULL)
			return pStatusList->Status;
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

char * TM_WordToString(STATUSINFO* pStatusList, WORD Status)
{
	STATUSINFO *pTemp = pStatusList, *pLast = NULL;

	if(!pStatusList)
		return NULL;

	while (pTemp != NULL)
	{
		if (pTemp->Status&Status)
		{
			Status -= pTemp->Status;
			if (Status == 0)
			{
				return pTemp->pszGroup;
			}
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

BOOL TM_RemoveAll (STATUSINFO** ppStatusList)
{

	if(!ppStatusList)
		return FALSE;

	while (*ppStatusList != NULL)
    {
		STATUSINFO *pLast = ppStatusList[0]->next;
		free (ppStatusList[0]->pszGroup);
		if((int)ppStatusList[0]->hIcon > 10)
			DestroyIcon(ppStatusList[0]->hIcon);
		free (*ppStatusList);
		*ppStatusList = pLast;
    }
	*ppStatusList = NULL;
	return TRUE;
}

//---------------------------------------------------
//		User manager functions
//
//		Necessary to keep track of the users
//		in a window nicklist
//---------------------------------------------------


static int UM_CompareItem(USERINFO * u1, char * pszNick, WORD wStatus)
{
	int i;

	WORD dw1 = u1->Status;
	WORD dw2 = wStatus;

	for (i=0; i<8; i++ )
	{
		if (( dw1 & 1 ) && !( dw2 & 1 ))
			return -1;
		if (( dw2 & 1 ) && !( dw1 & 1 ))
			return 1;
		if (( dw1 & 1 ) &&  ( dw2 & 1 ))
			return (int)lstrcmpA(u1->pszNick, pszNick);

		dw1 = dw1 >> 1;
		dw2 = dw2 >> 1;
	}
	return lstrcmpA(u1->pszNick, pszNick);

}
USERINFO * UM_SortUser(USERINFO** ppUserList, char * pszUID)
{
	USERINFO * pTemp = *ppUserList, *pLast = NULL;
	USERINFO * node = NULL;

	if(!pTemp || !pszUID)
		return NULL;

	while(pTemp && lstrcmpiA((char *)pTemp->pszUID, (char *)pszUID))
	{
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	if(pTemp)
	{
		node = pTemp;
		if(pLast)
			pLast->next = pTemp->next;
		else
			*ppUserList = pTemp->next;
		pTemp = *ppUserList;

		pLast = NULL;

		while(pTemp && UM_CompareItem(pTemp, node->pszNick, node->Status) <= 0)
		{
			pLast = pTemp;
			pTemp = pTemp->next;
		}

	//	if (!UM_FindUser(*ppUserList, pszUI, wStatus)
		{
			if (*ppUserList == NULL) // list is empty
			{
				*ppUserList = node;
				node->next = NULL;
			}
			else
			{
				if(pLast)
				{
					node->next = pTemp;
					pLast->next = node;

				}
				else
				{
					node->next = *ppUserList;
					*ppUserList = node;
				}
			}
			return node;

		}
	}
	return NULL;
}


USERINFO * UM_AddUser(STATUSINFO* pStatusList, USERINFO** ppUserList, char * pszUID, char * pszNick, WORD wStatus)
{
	USERINFO * pTemp = *ppUserList, *pLast = NULL;

	if(!pStatusList || !ppUserList || !ppUserList)
		return NULL;

	while(pTemp && UM_CompareItem(pTemp, pszNick, wStatus) <= 0)
	{
		pLast = pTemp;
		pTemp = pTemp->next;
	}

//	if (!UM_FindUser(*ppUserList, pszUI, wStatus)
	{
		USERINFO *node = (USERINFO*) malloc(sizeof(USERINFO));
		ZeroMemory(node, sizeof(USERINFO));

		node->pszUID = (char *) malloc(lstrlenA(pszUID) + 1);
		lstrcpyA(node->pszUID, pszUID);

		if (*ppUserList == NULL) // list is empty
		{
			*ppUserList = node;
			node->next = NULL;
		}
		else
		{
			if(pLast)
			{
				node->next = pTemp;
				pLast->next = node;

			}
			else
			{
				node->next = *ppUserList;
				*ppUserList = node;
			}
		}
		return node;

	}
	return NULL;
}

USERINFO* UM_FindUser(USERINFO* pUserList, char* pszUID)
{
	USERINFO *pTemp = pUserList, *pLast = NULL;

	if(!pUserList || !pszUID)
		return NULL;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszUID,pszUID))
		{
			return pTemp;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

USERINFO* UM_FindUserFromIndex(USERINFO* pUserList, int index)
{
	int i = 0;
	USERINFO *pTemp = pUserList;

	if(!pUserList)
		return NULL;

	while (pTemp != NULL)
	{
		if (i == index)
		{
			return pTemp;
		}
		pTemp = pTemp->next;
		i++;
	}
	return NULL;
}
USERINFO* UM_GiveStatus(USERINFO* pUserList, char* pszUID, WORD status)
{
	USERINFO *pTemp = pUserList, *pLast = NULL;

	if(!pUserList || !pszUID)
		return NULL;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszUID,pszUID))
		{
			pTemp->Status |= status;
			return pTemp;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}
BOOL UM_SetStatusEx(USERINFO* pUserList, char* pszText, int flags )
{
	USERINFO *pTemp = pUserList, *pLast = NULL;
	int bOnlyMe = ( flags & GC_SSE_ONLYLISTED ) != 0, bSetStatus = ( flags & GC_SSE_ONLINE ) != 0;
	char cDelimiter = ( flags & GC_SSE_TABDELIMITED ) ? '\t' : ' ';

	while (pTemp != NULL)
	{
		if ( !bOnlyMe )
			pTemp->iStatusEx = 0;

		if ( pszText != NULL ) {
			char* s = strstr(pszText, pTemp->pszUID);
			if ( s ) {
				pTemp->iStatusEx = 0;
				if ( s == pszText || s[-1] == cDelimiter ) {
					int len = lstrlenA(pTemp->pszUID);
					if ( s[len] == cDelimiter || s[len] == '\0' )
						pTemp->iStatusEx = ( !bOnlyMe || bSetStatus ) ? 1 : 0;
		}	}	}

		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}

USERINFO* UM_TakeStatus(USERINFO* pUserList, char* pszUID, WORD status)
{
	USERINFO *pTemp = pUserList, *pLast = NULL;

	if(!pUserList || !pszUID)
		return NULL;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszUID,pszUID))
		{
			pTemp->Status &= ~status;
			return pTemp;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}
char* UM_FindUserAutoComplete(USERINFO* pUserList, char * pszOriginal, char* pszCurrent)
{
	char * pszName = NULL;
	USERINFO *pTemp = pUserList, *pLast = NULL;

	if(!pUserList || !pszOriginal || !pszCurrent)
		return NULL;

	while (pTemp != NULL)
	{
		if (my_strstri(pTemp->pszNick,pszOriginal) == pTemp->pszNick)
		{
			if(lstrcmpiA(pTemp->pszNick, pszCurrent) > 0 && (!pszName || lstrcmpiA(pTemp->pszNick, pszName) < 0) )
				pszName =pTemp->pszNick;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return pszName;
}

BOOL UM_RemoveUser(USERINFO** ppUserList, char* pszUID)
{
	USERINFO *pTemp = *ppUserList, *pLast = NULL;

	if(!ppUserList || !pszUID)
		return FALSE;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pTemp->pszUID,pszUID) )
		{
			if (pLast == NULL)
				*ppUserList = pTemp->next;
			else
				pLast->next = pTemp->next;
			free(pTemp->pszNick);
			free(pTemp->pszUID);
			free(pTemp);
			return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return FALSE;
}

BOOL UM_RemoveAll (USERINFO** ppUserList)
{
	if(!ppUserList)
		return FALSE;

	while (*ppUserList != NULL)
    {
		USERINFO *pLast = ppUserList[0]->next;
		free (ppUserList[0]->pszUID);
		free (ppUserList[0]->pszNick);
		free (*ppUserList);
		*ppUserList = pLast;
    }
	*ppUserList = NULL;
	return TRUE;
}

//---------------------------------------------------
//		Log manager functions
//
//		Necessary to keep track of events
//		in a window log
//---------------------------------------------------

LOGINFO * LM_AddEvent(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd)
{

	LOGINFO *node = NULL;

	if(!ppLogListStart || !ppLogListEnd)
		return NULL;

	node = (LOGINFO*) malloc(sizeof(LOGINFO));
	ZeroMemory(node, sizeof(LOGINFO));


	if (*ppLogListStart == NULL) // list is empty
	{
		*ppLogListStart = node;
		*ppLogListEnd = node;
		node->next = NULL;
		node->prev = NULL;
	}
	else
	{
		ppLogListStart[0]->prev = node;
		node->next = *ppLogListStart;
		*ppLogListStart = node;
		ppLogListStart[0]->prev=NULL;
	}

	return node;
}

BOOL LM_TrimLog(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd, int iCount)
{
	LOGINFO *pTemp = *ppLogListEnd;
	while (pTemp != NULL && iCount > 0)
	{
		*ppLogListEnd = pTemp->prev;
		if (*ppLogListEnd == NULL)
			*ppLogListStart = NULL;

		if(pTemp->pszNick)
			free(pTemp->pszNick);
		if(pTemp->pszUserInfo)
			free(pTemp->pszUserInfo);
		if(pTemp->pszText)
			free(pTemp->pszText);
		if(pTemp->pszStatus)
			free(pTemp->pszStatus);
		if(pTemp)
			free(pTemp);
		pTemp = *ppLogListEnd;
		iCount--;
	}
	ppLogListEnd[0]->next = NULL;

	return TRUE;
}

BOOL LM_RemoveAll (LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd)
{
	while (*ppLogListStart != NULL)
    {
		LOGINFO *pLast = ppLogListStart[0]->next;
		if(ppLogListStart[0]->pszText)
			free (ppLogListStart[0]->pszText);
		if(ppLogListStart[0]->pszNick)
			free (ppLogListStart[0]->pszNick);
		if(ppLogListStart[0]->pszStatus)
			free (ppLogListStart[0]->pszStatus);
		if(ppLogListStart[0]->pszUserInfo)
			free (ppLogListStart[0]->pszUserInfo);
		if(*ppLogListStart)
			free (*ppLogListStart);
		*ppLogListStart = pLast;
    }
	*ppLogListStart = NULL;
	*ppLogListEnd = NULL;
	return TRUE;
}

