/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-07  George Hazan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL$
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

*/

#include "jabber.h"
#include "jabber_list.h"
#include "resource.h"

#include <m_contacts.h>


/////////////////////////////////////////////////////////////////////////////////////////
// module data

HANDLE hMenuRequestAuth = NULL;
HANDLE hMenuGrantAuth = NULL;
HANDLE hMenuRevokeAuth = NULL;
HANDLE hMenuJoinLeave = NULL;
HANDLE hMenuConvert = NULL;
HANDLE hMenuRosterAdd = NULL;
HANDLE hMenuLogin = NULL;
HANDLE hMenuRefresh = NULL;
HANDLE hMenuCommands = NULL;
HANDLE hMenuAddBookmark = NULL;

HANDLE hMenuVisitGMail = NULL;

extern HANDLE hMenuBookmarks;

static void sttEnableMenuItem( HANDLE hMenuItem, BOOL bEnable )
{
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof( CLISTMENUITEM );
	clmi.flags = CMIM_FLAGS;
	if ( !bEnable )
		clmi.flags |= CMIF_HIDDEN;

	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuItem, ( LPARAM )&clmi );
}

int JabberMenuPrebuildContactMenu( WPARAM wParam, LPARAM lParam )
{
	sttEnableMenuItem( hMenuRequestAuth, FALSE );
	sttEnableMenuItem( hMenuGrantAuth, FALSE );
	sttEnableMenuItem( hMenuRevokeAuth, FALSE );
	sttEnableMenuItem( hMenuJoinLeave, FALSE );
	sttEnableMenuItem( hMenuCommands, FALSE );
	sttEnableMenuItem( hMenuConvert, FALSE );
	sttEnableMenuItem( hMenuRosterAdd, FALSE );
	sttEnableMenuItem( hMenuLogin, FALSE );
	sttEnableMenuItem( hMenuVisitGMail, FALSE );
	sttEnableMenuItem( hMenuRefresh, FALSE );
	sttEnableMenuItem( hMenuAddBookmark, FALSE );

	HANDLE hContact;
	if (( hContact=( HANDLE )wParam ) == NULL )
		return 0;
	else {
		DBVARIANT dbv;
		if ( !JGetStringT( hContact, "FakeContact", &dbv )) {
			if (!_tcsicmp( dbv.ptszVal, _T("GMAIL"))){
				sttEnableMenuItem( hMenuVisitGMail, TRUE );
				return 0;
			}
			JFreeVariant( &dbv );
	}	}

	BYTE bIsChatRoom  = (BYTE)JGetByte( hContact, "ChatRoom", 0 );
	BYTE bIsTransport = (BYTE)JGetByte( hContact, "IsTransport", 0 );

	if ((bIsChatRoom == GCW_CHATROOM) || bIsChatRoom == 0 ) {
		DBVARIANT dbv;
		if ( !JGetStringT( hContact, bIsChatRoom?(char*)"ChatRoomID":(char*)"jid", &dbv )) {
			JFreeVariant( &dbv );
			CLISTMENUITEM clmi = { 0 };
			sttEnableMenuItem( hMenuConvert, TRUE );
			clmi.cbSize = sizeof( clmi );
			clmi.pszName = (char *)(bIsChatRoom ? "&Convert to Contact" : "&Convert to Chat Room");
			clmi.flags = CMIM_NAME | CMIM_FLAGS;
			JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuConvert, ( LPARAM )&clmi );
	}	}

	if (!jabberOnline) 
		return 0;

	if ( bIsChatRoom ) {
		DBVARIANT dbv;
		if ( !JGetStringT( hContact, "ChatRoomID", &dbv )) {
			if ( JabberListGetItemPtr( LIST_ROSTER, dbv.ptszVal ) == NULL )
				sttEnableMenuItem( hMenuRosterAdd, TRUE );

			if ( JabberListGetItemPtr( LIST_BOOKMARK, dbv.ptszVal ) == NULL )
				if ( jabberThreadInfo && jabberThreadInfo->caps & CAPS_BOOKMARK )
					sttEnableMenuItem( hMenuAddBookmark, TRUE );

			JFreeVariant( &dbv );
	}	}

	if ( bIsChatRoom == GCW_CHATROOM ) {
		CLISTMENUITEM clmi = { 0 };
		clmi.cbSize = sizeof( clmi );
		clmi.pszName = (char *)(( JGetWord( hContact, "Status", 0 ) == ID_STATUS_ONLINE ) ? "&Leave" : "&Join");
		clmi.flags = CMIM_NAME | CMIM_FLAGS;
		JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuJoinLeave, ( LPARAM )&clmi );
		return 0;
	}

	if ( bIsTransport ) {
		sttEnableMenuItem( hMenuLogin, TRUE );
		sttEnableMenuItem( hMenuRefresh, TRUE );
	}

	DBVARIANT dbv;
	if ( !JGetStringT( hContact, "jid", &dbv )) {
		JABBER_LIST_ITEM * item;
		item=JabberListGetItemPtr( LIST_ROSTER, dbv.ptszVal );
		if ( !bIsTransport ||
			 (( item != NULL ) && (item->cap & AGENT_CAP_ADHOC) ) ) {
				sttEnableMenuItem( hMenuCommands, TRUE );				
		}
		else 
			sttEnableMenuItem( hMenuCommands, FALSE );
		JFreeVariant( &dbv );
	}

	if ( !JGetStringT( hContact, "jid", &dbv )) {
		JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, dbv.ptszVal );
		JFreeVariant( &dbv );
		if ( item != NULL ) {
			BOOL bCtrlPressed = ( GetKeyState( VK_CONTROL)&0x8000 ) != 0;
			sttEnableMenuItem( hMenuRequestAuth, item->subscription == SUB_FROM || item->subscription == SUB_NONE || bCtrlPressed );
			sttEnableMenuItem( hMenuGrantAuth, item->subscription == SUB_TO || item->subscription == SUB_NONE || bCtrlPressed );
			sttEnableMenuItem( hMenuRevokeAuth, item->subscription == SUB_FROM || item->subscription == SUB_BOTH || bCtrlPressed );
			sttEnableMenuItem( hMenuCommands, (!bIsChatRoom && (!bIsTransport || (item->cap & AGENT_CAP_ADHOC))) );
			return 0;
	}	}

	return 0;
}

int JabberMenuConvertChatContact( WPARAM wParam, LPARAM lParam )
{
	BYTE bIsChatRoom = (BYTE)JGetByte( (HANDLE ) wParam, "ChatRoom", 0 );
	if ((bIsChatRoom == GCW_CHATROOM) || bIsChatRoom == 0 ) {
		DBVARIANT dbv;
		if ( !JGetStringT( (HANDLE ) wParam, (bIsChatRoom == GCW_CHATROOM)?(char*)"ChatRoomID":(char*)"jid", &dbv )) {
			JDeleteSetting( (HANDLE ) wParam, (bIsChatRoom == GCW_CHATROOM)?"ChatRoomID":"jid");
			JSetStringT( (HANDLE ) wParam, (bIsChatRoom != GCW_CHATROOM)?"ChatRoomID":"jid", dbv.ptszVal);
			JFreeVariant( &dbv );
			JSetByte((HANDLE ) wParam, "ChatRoom", (bIsChatRoom == GCW_CHATROOM)?0:GCW_CHATROOM);
	}	}
	return 0;
}

int JabberMenuRosterAdd( WPARAM wParam, LPARAM lParam )
{
	DBVARIANT dbv;
	if ( !wParam ) return 0; // we do not add ourself to the roster. (buggy situation - should not happen)
	if ( !JGetStringT( ( HANDLE ) wParam, "ChatRoomID", &dbv )) {
		TCHAR *roomID = mir_tstrdup(dbv.ptszVal);
		JFreeVariant( &dbv );
		if ( JabberListGetItemPtr( LIST_ROSTER, roomID ) == NULL ) {
			TCHAR *nick = 0;
			TCHAR *group = 0;
			if ( !DBGetContactSettingTString( ( HANDLE ) wParam, "CList", "Group", &dbv ) ) {
				group = mir_tstrdup(dbv.ptszVal);
				JFreeVariant( &dbv );
			}
			if ( !JGetStringT( ( HANDLE ) wParam, "Nick", &dbv ) ) {
				nick = mir_tstrdup(dbv.ptszVal);
				JFreeVariant( &dbv );
			}
			JabberAddContactToRoster(roomID, nick, group, SUB_NONE);
			if ( JGetByte( "AddRoster2Bookmarks", TRUE ) == TRUE ) {

				JABBER_LIST_ITEM* item = NULL;
				
				item = JabberListGetItemPtr(LIST_BOOKMARK, roomID);
				if (!item) {
					item = ( JABBER_LIST_ITEM* )mir_alloc( sizeof( JABBER_LIST_ITEM ));
					ZeroMemory( item, sizeof( JABBER_LIST_ITEM ));
					item->jid = mir_tstrdup(roomID);
					item->name = mir_tstrdup(nick);
					if ( !JGetStringT( ( HANDLE ) wParam, "MyNick", &dbv ) ) {
						item->nick = mir_tstrdup(dbv.ptszVal);
						JFreeVariant( &dbv );
					}
					JabberAddEditBookmark(NULL, (LPARAM) item);
					mir_free(item);
				}
			}
			if (nick) mir_free(nick);
			if (nick) mir_free(group);
		}
		mir_free(roomID);
	}
	return 0;
}

int JabberMenuHandleRequestAuth( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact;
	DBVARIANT dbv;

	if (( hContact=( HANDLE ) wParam )!=NULL && jabberOnline ) {
		if ( !JGetStringT( hContact, "jid", &dbv )) {
			XmlNode presence( "presence" ); presence.addAttr( "to", dbv.ptszVal ); presence.addAttr( "type", "subscribe" );
			JabberSend( jabberThreadInfo->s, presence );
			JFreeVariant( &dbv );
	}	}

	return 0;
}

int JabberMenuHandleGrantAuth( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact;
	DBVARIANT dbv;

	if (( hContact=( HANDLE ) wParam )!=NULL && jabberOnline ) {
		if ( !JGetStringT( hContact, "jid", &dbv )) {
			XmlNode presence( "presence" ); presence.addAttr( "to", dbv.ptszVal ); presence.addAttr( "type", "subscribed" );
			JabberSend( jabberThreadInfo->s, presence );
			JFreeVariant( &dbv );
	}	}

	return 0;
}

int JabberMenuRevokeAuth( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact;
	DBVARIANT dbv;

	if (( hContact=( HANDLE ) wParam )!=NULL && jabberOnline ) {
		if ( !JGetStringT( hContact, "jid", &dbv )) {
			XmlNode presence( "presence" ); presence.addAttr( "to", dbv.ptszVal ); presence.addAttr( "type", "unsubscribed" );
			JabberSend( jabberThreadInfo->s, presence );
			JFreeVariant( &dbv );
	}	}

	return 0;
}

int JabberMenuJoinLeave( WPARAM wParam, LPARAM lParam )
{
	DBVARIANT dbv, jid;
	if ( JGetStringT(( HANDLE )wParam, "ChatRoomID", &jid ))
		return 0;

	if ( JGetStringT(( HANDLE )wParam, "MyNick", &dbv ))
		if ( JGetStringT( NULL, "Nick", &dbv )) {
			JFreeVariant( &jid );
			return 0;
		}

	if ( JGetWord(( HANDLE )wParam, "Status", 0 ) != ID_STATUS_ONLINE ) {
		if ( !jabberChatDllPresent ) {
			JabberChatDllError();
			goto LBL_Return;
		}

		TCHAR* p = _tcschr( jid.ptszVal, '@' );
		if ( p == NULL )
			goto LBL_Return;

		*p++ = 0;
		JabberGroupchatJoinRoom( p, jid.ptszVal, dbv.ptszVal, _T(""));
	}
	else {
		JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, jid.ptszVal );
		if ( item != NULL )
			JabberGcQuit( item, 0, NULL );
	}

LBL_Return:
	JFreeVariant( &dbv );
	JFreeVariant( &jid );
	return 0;
}

int JabberMenuTransportLogin( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact = ( HANDLE )wParam;
	if ( !JGetByte( hContact, "IsTransport", 0 ))
		return 0;

	DBVARIANT jid;
	if ( JGetStringT( hContact, "jid", &jid ))
		return 0;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, jid.ptszVal );
	if ( item != NULL ) {
		XmlNode p( "presence" ); p.addAttr( "to", item->jid );
		if ( item->status == ID_STATUS_ONLINE )
			p.addAttr( "type", "unavailable" );
		JabberSend( jabberThreadInfo->s, p );
	}

	JFreeVariant( &jid );
	return 0;
}

int JabberMenuTransportResolve( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact = ( HANDLE )wParam;
	if ( !JGetByte( hContact, "IsTransport", 0 ))
		return 0;

	DBVARIANT jid;
	if ( !JGetStringT( hContact, "jid", &jid )) {
		JabberResolveTransportNicks( jid.ptszVal );
		JFreeVariant( &jid );
	}
	return 0;
}

int JabberMenuBookmarkAdd( WPARAM wParam, LPARAM lParam )
{
	DBVARIANT dbv;
	if ( !wParam ) return 0; // we do not add ourself to the roster. (buggy situation - should not happen)
	if ( !JGetStringT( ( HANDLE ) wParam, "ChatRoomID", &dbv )) {
		TCHAR *roomID = mir_tstrdup(dbv.ptszVal);
		JFreeVariant( &dbv );
		if ( JabberListGetItemPtr( LIST_BOOKMARK, roomID ) == NULL ) {
			TCHAR *nick = 0;
			if ( !JGetStringT( ( HANDLE ) wParam, "Nick", &dbv ) ) {
				nick = mir_tstrdup(dbv.ptszVal);
				JFreeVariant( &dbv );
			}
			JABBER_LIST_ITEM* item = NULL;

			item = ( JABBER_LIST_ITEM* )mir_alloc( sizeof( JABBER_LIST_ITEM ));
			ZeroMemory( item, sizeof( JABBER_LIST_ITEM ));
			item->jid = mir_tstrdup(roomID);
			item->name = ( TCHAR* )JCallService( MS_CLIST_GETCONTACTDISPLAYNAME, wParam, GCDNF_TCHAR );
			item->type = _T("conference");
			if ( !JGetStringT(( HANDLE ) wParam, "MyNick", &dbv ) ) {
				item->nick = mir_tstrdup(dbv.ptszVal);
				JFreeVariant( &dbv );
			}
			JabberAddEditBookmark(NULL, (LPARAM) item);
			mir_free(item);
			
			if (nick) mir_free(nick);
		}
		mir_free(roomID);
	}
	return 0;

}


/////////////////////////////////////////////////////////////////////////////////////////
// contact menu initialization code
int JabberMenuVisitGMail( WPARAM wParam, LPARAM lParam );

void JabberMenuInit()
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof( CLISTMENUITEM );

	char text[ 200 ];
	strcpy( text, jabberProtoName );
	char* tDest = text + strlen( text );

	// "Request authorization"
	strcpy( tDest, "/RequestAuth" );
	CreateServiceFunction( text, JabberMenuHandleRequestAuth );
	mi.pszName = "Request authorization";
	mi.position = -2000001000;
	mi.hIcon = iconList[6];// IDI_REQUEST;
	mi.pszService = text;
	mi.pszContactOwner = jabberProtoName;
	hMenuRequestAuth = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// "Grant authorization"
	strcpy( tDest, "/GrantAuth" );
	CreateServiceFunction( text, JabberMenuHandleGrantAuth );
	mi.pszName = "Grant authorization";
	mi.position = -2000001001;
	mi.hIcon = iconList[7];// IDI_GRANT;
	hMenuGrantAuth = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// Revoke auth
	strcpy( tDest, "/RevokeAuth" );
	CreateServiceFunction( text, JabberMenuRevokeAuth );
	mi.pszName = "Revoke authorization";
	mi.position = -2000001002;
	mi.hIcon = iconList[8];//IDI_AUTHREVOKE;
	hMenuRevokeAuth = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// "Grant authorization"
	strcpy( tDest, "/JoinChat" );
	CreateServiceFunction( text, JabberMenuJoinLeave );
	mi.pszName = "Join chat";
	mi.position = -2000001003;
	mi.hIcon = iconBigList[0];
	hMenuJoinLeave = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// "Convert Chat/Contact"
	strcpy( tDest, "/ConvertChatContact" );
	CreateServiceFunction( text, JabberMenuConvertChatContact );
	mi.pszName = "Convert";
	mi.position = -1999901004;
	mi.hIcon = iconList[17];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_USER2ROOM ));
	hMenuConvert = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// "Add to roster"
	strcpy( tDest, "/AddToRoster" );
	CreateServiceFunction( text, JabberMenuRosterAdd );
	mi.pszName = "Add to roster";
	mi.position = -1999901005;
	mi.hIcon = iconList[16];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_ADDROSTER ));
	hMenuRosterAdd = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// "Add to Bookmarks"
	strcpy( tDest, "/AddToBookmarks" );
	CreateServiceFunction( text, JabberMenuBookmarkAdd );
	mi.pszName = "Add to Bookmarks";
	mi.position = -1999901006;
	mi.hIcon = iconList[20];//LoadIconEx( "bookmarks" ); GetIconHandle( IDI_BOOKMARKS);
	hMenuAddBookmark= ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// Login/logout
	strcpy( tDest, "/TransportLogin" );
	CreateServiceFunction( text, JabberMenuTransportLogin );
	mi.pszName = "Login/logout";
	mi.position = -1999901007;
	mi.hIcon = iconList[18];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_LOGIN ));
	hMenuLogin = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// "visit GMail for the fake contact"
	strcpy( tDest, "/VisitGMail" );
	CreateServiceFunction( text, JabberMenuVisitGMail );
	mi.pszName = "Visit GMail";
	mi.position = -2000100001;
	mi.hIcon = iconList[15];
	hMenuVisitGMail = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );


	// Retrieve nicks
	strcpy( tDest, "/TransportGetNicks" );
	CreateServiceFunction( text, JabberMenuTransportResolve );
	mi.pszName = "Resolve nicks";
	mi.position = -1999901007;
	mi.hIcon = iconList[19];//IDI_REFRESH;
	hMenuRefresh = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// Run Commands
	strcpy( tDest, "/RunCommands" );
	CreateServiceFunction( text, JabberContactMenuRunCommands );
	mi.pszName = "Commands";
	mi.position = -1999901009;
	mi.icolibItem = iconList[21];//GetIconHandle( IDI_COMMAND );
	hMenuCommands = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

}

//////////////////////////////////////////////////////////////////////////
// resource menu
static HANDLE hDialogsList = NULL;

void JabberMenuHideSrmmIcon(HANDLE hContact)
{
	StatusIconData sid = {0};
	sid.cbSize = sizeof(sid);
	sid.szModule = jabberProtoName;
	sid.flags = MBF_HIDDEN;
	CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
}

void JabberMenuUpdateSrmmIcon(JABBER_LIST_ITEM *item)
{
	if ( item->list != LIST_ROSTER || !ServiceExists( MS_MSG_MODIFYICON ))
		return;

	HANDLE hContact = JabberHContactFromJID(item->jid);
	if (!hContact)
		return;

	StatusIconData sid = {0};
	sid.cbSize = sizeof(sid);
	sid.szModule = jabberProtoName;
	sid.flags = item->resourceCount ? 0 : MBF_HIDDEN;
	CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
}

int JabberMenuProcessSrmmEvent( WPARAM wParam, LPARAM lParam )
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

    if (event->uType == MSG_WINDOW_EVT_OPEN)
    {
		if (!hDialogsList)
			hDialogsList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
		WindowList_Add(hDialogsList, event->hwndWindow, event->hContact);
    } else
	if (event->uType == MSG_WINDOW_EVT_CLOSING)
    {
		if (hDialogsList)
			WindowList_Remove(hDialogsList, event->hwndWindow);
    }

	return 0;
}

#define MENUITEM_LASTSEEN	1
#define MENUITEM_SERVER		2
#define MENUITEM_RESOURCES	10
int JabberMenuProcessSrmmIconClick( WPARAM wParam, LPARAM lParam )
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (lstrcmpA(sicd->szModule, jabberProtoName))
		return 0;

	HANDLE hContact = (HANDLE)wParam;
	if (!hContact)
		return 0;

	DBVARIANT dbv;
	if (JGetStringT(hContact, "jid", &dbv))
		return 0;


	JABBER_LIST_ITEM *LI = JabberListGetItemPtr(LIST_ROSTER, dbv.ptszVal);
	JFreeVariant( &dbv );

	if (!LI)
		return 0;

	HMENU hMenu = CreatePopupMenu();
	TCHAR buf[256];

	mir_sntprintf(buf, SIZEOF(buf), _T("%s (%s)"), TranslateT("Last active"),
		((LI->lastSeenResource>=0) && (LI->lastSeenResource < LI->resourceCount)) ?
			LI->resource[LI->lastSeenResource].resourceName : TranslateT("No activity yet, use server's choice"));
	AppendMenu(hMenu, MF_STRING, MENUITEM_LASTSEEN, buf);

	AppendMenu(hMenu, MF_STRING, MENUITEM_SERVER, TranslateT("Highest priority (server's choice)"));

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	for (int i = 0; i < LI->resourceCount; ++i)
		AppendMenu(hMenu, MF_STRING, MENUITEM_RESOURCES+i, LI->resource[i].resourceName);
	if (LI->resourceMode == RSMODE_LASTSEEN)
		CheckMenuItem(hMenu, MENUITEM_LASTSEEN, MF_BYCOMMAND|MF_CHECKED);
	else if (LI->resourceMode == RSMODE_SERVER)
		CheckMenuItem(hMenu, MENUITEM_SERVER, MF_BYCOMMAND|MF_CHECKED);
	else
		CheckMenuItem(hMenu, MENUITEM_RESOURCES+LI->manualResource, MF_BYCOMMAND|MF_CHECKED);

	int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, sicd->clickLocation.x, sicd->clickLocation.y, 0, WindowList_Find(hDialogsList, hContact), NULL);
	if (res == MENUITEM_LASTSEEN)
	{
		LI->manualResource = -1;
		LI->resourceMode = RSMODE_LASTSEEN;
	} else
	if (res == MENUITEM_SERVER)
	{
		LI->manualResource = -1;
		LI->resourceMode = RSMODE_SERVER;
	} else
	if (res >= MENUITEM_RESOURCES)
	{
		LI->manualResource = res - MENUITEM_RESOURCES;
		LI->resourceMode = RSMODE_MANUAL;
	}

	JabberUpdateMirVer(LI);
	JabberMenuUpdateSrmmIcon(LI);

	return 0;
}
