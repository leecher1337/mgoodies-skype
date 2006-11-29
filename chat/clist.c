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

extern HINSTANCE		g_hInst;

HANDLE CList_AddRoom(const char* pszModule, const TCHAR* pszRoom, const TCHAR* pszDisplayName, int iType)
{
	HANDLE hContact = CList_FindRoom(pszModule, pszRoom);
	DBVARIANT dbv;
	TCHAR pszGroup[50];

	*pszGroup = '\0';
	if ( !DBGetContactSettingTString( NULL, "Chat", "AddToGroup", &dbv )) {
		if ( lstrlen( dbv.ptszVal ) > 0 )
			lstrcpyn( pszGroup, dbv.ptszVal, 50);
		DBFreeVariant(&dbv);
	}
	else lstrcpyn( pszGroup, _T("Chat rooms"), 50);

	if ( pszGroup[0] )
		CList_CreateGroup(pszGroup);

	if ( hContact ) { //contact exist, make sure it is in the right group
		DBVARIANT dbv;
		DBVARIANT dbv2;
		char str[50];
		int i;

		if ( pszGroup[0] ) {
			for (i = 0;; i++) {
				itoa(i, str, 10);
				if ( DBGetContactSettingTString( NULL, "CListGroups", str, &dbv )) {
					DBWriteContactSettingTString(hContact, "CList", "Group", pszGroup);
					goto END_GROUPLOOP;
				}

				if ( !DBGetContactSettingTString( hContact, "CList", "Group", &dbv2 )) {
					if ( dbv.ptszVal[0] != '\0' && dbv2.ptszVal[0] != '\0' && !lstrcmpi( dbv.ptszVal + 1, dbv2.ptszVal )) {
							DBFreeVariant(&dbv);
							DBFreeVariant(&dbv2);
							goto END_GROUPLOOP;
						}
						DBFreeVariant(&dbv2);
					}
					DBFreeVariant(&dbv);
		}	}

END_GROUPLOOP:
		DBWriteContactSettingWord(hContact, pszModule, "Status", ID_STATUS_OFFLINE);
		DBWriteContactSettingTString(hContact, pszModule, "Nick", pszDisplayName );
		if(iType != GCW_SERVER)
			DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
		return hContact;
	}

	// here we create a new one since no one is to be found
	if (( hContact = (HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0)) == NULL )
		return NULL;

		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM) hContact, (LPARAM) pszModule);
	if ( pszGroup && lstrlen( pszGroup ) > 0 )
		DBWriteContactSettingTString(hContact, "CList", "Group", pszGroup );
		else
			DBDeleteContactSetting(hContact, "CList", "Group");
	DBWriteContactSettingTString( hContact, pszModule, "Nick", pszDisplayName );
	DBWriteContactSettingTString( hContact, pszModule, "ChatRoomID", pszRoom );
		DBWriteContactSettingByte(hContact, pszModule, "ChatRoom", (BYTE)iType);
		DBWriteContactSettingWord(hContact, pszModule, "Status", ID_STATUS_OFFLINE);
//		if(iType == GCW_SERVER)
			DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
		return hContact;
	}

BOOL CList_SetOffline(HANDLE hContact, BOOL bHide)
{
	if ( hContact ) {
		char * szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		int i = DBGetContactSettingByte(hContact, szProto, "ChatRoom", 0);
		DBWriteContactSettingWord(hContact, szProto,"ApparentMode",(LPARAM) 0);
		DBWriteContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		if (bHide && i != GCW_SERVER)
			DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
		return TRUE;
	}

	return FALSE;
}

BOOL CList_SetAllOffline(BOOL bHide)
{
    HANDLE hContact;
	char * szProto;

    hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while ( hContact ) {
       szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      if ( MM_FindModule( szProto )) {
		   int i = DBGetContactSettingByte(hContact, szProto, "ChatRoom", 0);
			if ( i != 0 ) {
				DBWriteContactSettingWord(hContact, szProto,"ApparentMode",(LPARAM)(WORD) 0);
				DBWriteContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
				if (bHide && i == GCW_CHATROOM)
					DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
			}
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	return TRUE;
}

int	CList_RoomDoubleclicked(WPARAM wParam,LPARAM lParam)
{
    DBVARIANT dbv;
    char *szProto;

	HANDLE hContact = (HANDLE)wParam;
	if (!hContact)
		return 0;

	szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if ( MM_FindModule(szProto)) {
		if(DBGetContactSettingByte(hContact, szProto, "ChatRoom", 0) == 0)
			return 0;

		if ( !DBGetContactSettingTString( hContact, szProto, "ChatRoomID", &dbv )) {
			SESSION_INFO* si = SM_FindSession( dbv.ptszVal, szProto );
			if ( si ) {
				// is the "toggle visibility option set, so we need to close the window?
				if (si->hWnd != NULL
					&& DBGetContactSettingByte(NULL, "Chat", "ToggleVisibility", 0)==1
					&& !CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, 0)
					&& IsWindowVisible(si->hWnd)
					&& !IsIconic(si->hWnd))
			{
					PostMessage(si->hWnd, GC_CLOSEWINDOW, 0, 0);
					DBFreeVariant(&dbv);
					return 1;
				}
				ShowRoom(si, WINDOW_VISIBLE, TRUE);
			}
			DBFreeVariant(&dbv);
			return 1;
		}
	}
	return 0;
}

int	CList_EventDoubleclicked(WPARAM wParam,LPARAM lParam)
{
	return CList_RoomDoubleclicked((WPARAM) ((CLISTEVENT*)lParam)->hContact,(LPARAM) 0);
}

void CList_CreateGroup(TCHAR* group)
{
	int i;
    char str[50];
	TCHAR name[256];
    DBVARIANT dbv;

	if (!group)
		return;

	for (i = 0;; i++)
	{
        itoa(i, str, 10);
		if ( DBGetContactSettingTString( NULL, "CListGroups", str, &dbv ))
            break;

		if ( dbv.pszVal[0] != '\0' && !lstrcmpi(dbv.ptszVal + 1, group)) {
				DBFreeVariant(&dbv);
				return;
			}

	        DBFreeVariant(&dbv);
        }

 //	CallService(MS_CLIST_GROUPCREATE, (WPARAM)group, 0);
	name[0] = 1 | GROUPF_EXPANDED;
	_tcsncpy(name + 1, group, SIZEOF(name) - 1);
	name[ lstrlen(group) + 1] = '\0';
	DBWriteContactSettingTString(NULL, "CListGroups", str, name);
    CallService(MS_CLUI_GROUPADDED, i + 1, 0);
}

BOOL CList_AddEvent(HANDLE hContact, HICON Icon, HANDLE event, int type, TCHAR* fmt, ... )
{
	CLISTEVENT cle;
	va_list marker;
	TCHAR* szBuf = (TCHAR*)alloca(4096 * sizeof(TCHAR));

	if (!fmt || lstrlen(fmt) < 1 || lstrlen(fmt) > 2000)
		return FALSE;

	va_start(marker, fmt);
	_vstprintf(szBuf, fmt, marker);
	va_end(marker);

	cle.cbSize=sizeof(cle);
	cle.hContact=(HANDLE)hContact;
	cle.hDbEvent=(HANDLE)event;
	cle.flags = type + CLEF_TCHAR;
	cle.hIcon=Icon;
	cle.pszService= "GChat/DblClickEvent" ;
	cle.ptszTooltip = TranslateTS(szBuf);
	if ( type ) {
		if(!CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, (LPARAM)0))
			CallService(MS_CLIST_ADDEVENT,(WPARAM) hContact,(LPARAM) &cle);
	}
	else {
		if(CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, (LPARAM)0))
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)hContact, (LPARAM)"chaticon");
		CallService(MS_CLIST_ADDEVENT,(WPARAM) hContact,(LPARAM) &cle);
	}
	return TRUE;
}

HANDLE CList_FindRoom ( const char* pszModule, const TCHAR* pszRoom)
{
	HANDLE hContact = ( HANDLE )CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) {
		char* szProto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0 );
		if ( szProto && !lstrcmpiA( szProto, pszModule )) {
			if ( DBGetContactSettingByte( hContact, szProto, "ChatRoom", 0) != 0 ) {
	DBVARIANT dbv;
				if ( !DBGetContactSettingTString( hContact, szProto, "ChatRoomID", &dbv )) {
					if ( !lstrcmpi(dbv.ptszVal, pszRoom)) {
						DBFreeVariant(&dbv);
						return hContact;
					}
					DBFreeVariant(&dbv);
		}	}	}

		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	return 0;
}

int WCCmp(TCHAR* wild, TCHAR* string)
{
	TCHAR *cp, *mp;
	if ( wild == NULL || !lstrlen(wild) || string == NULL || !lstrlen(string))
		return 0;

	while ((*string) && (*wild != '*')) {
		if ((*wild != *string) && (*wild != '?'))
			return 0;

		wild++;
		string++;
	}

	while (*string) {
		if (*wild == '*') {
			if (!*++wild)
				return 1;

			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild == '?')) {
			wild++;
			string++;
		}
		else {
			wild = mp;
			string = cp++;
	}	}

	while (*wild == '*')
		wild++;

	return !*wild;
}
