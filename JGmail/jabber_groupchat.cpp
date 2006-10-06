/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-06  George Hazan

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
#include <commctrl.h>
#include "resource.h"
#include "jabber_iq.h"

#define GC_SERVER_LIST_SIZE 5

static BOOL CALLBACK JabberGroupchatDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );
static BOOL CALLBACK JabberGroupchatJoinDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );

int JabberMenuHandleGroupchat( WPARAM wParam, LPARAM lParam )
{
	int iqId;

	// lParam is the initial conference server to browse ( if any )
	if ( IsWindow( hwndJabberGroupchat )) {
		SetForegroundWindow( hwndJabberGroupchat );
		if ( lParam != 0 ) {
			SendMessage( hwndJabberGroupchat, WM_JABBER_ACTIVATE, 0, lParam );	// Just to update the IDC_SERVER and clear the list
			iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_DISCOROOMSERVER, JabberIqResultDiscoRoomItems );

			XmlNodeIq iq( "get", iqId, ( TCHAR* )lParam );
			XmlNode* query = iq.addQuery( "http://jabber.org/protocol/disco#items" );
			JabberSend( jabberThreadInfo->s, iq );
			// <iq/> result will send WM_JABBER_REFRESH to update the list with real data
		}
	}
	else hwndJabberGroupchat = CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_GROUPCHAT ), NULL, JabberGroupchatDlgProc, lParam );

	return 0;
}

static BOOL sortAscending;
static int sortColumn;

static int CALLBACK GroupchatCompare( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	JABBER_LIST_ITEM *item1, *item2;
	int res = 0;
	item1 = JabberListGetItemPtr( LIST_ROOM, ( TCHAR* )lParam1 );
	item2 = JabberListGetItemPtr( LIST_ROOM, ( TCHAR* )lParam2 );
	if ( item1!=NULL && item2!=NULL ) {
		switch ( lParamSort ) {
		case 0:	// sort by JID column
			res = lstrcmp( item1->jid, item2->jid );
			break;
		case 1: // sort by Name column
			res = lstrcmp( item1->name, item2->name );
			break;
	}	}

	if ( !sortAscending )
		res *= -1;

	return res;
}

static BOOL CALLBACK JabberGroupchatDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HWND lv;
	LVCOLUMN lvCol;
	LVITEM lvItem;
	JABBER_LIST_ITEM *item;

	switch ( msg ) {
	case WM_INITDIALOG:
		// lParam is the initial conference server ( if any )
		SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )iconBigList[0] );
		TranslateDialogDefault( hwndDlg );
		sortColumn = -1;
		// Add columns
		lv = GetDlgItem( hwndDlg, IDC_ROOM );
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvCol.pszText = TranslateT( "JID" );
		lvCol.cx = 210;
		lvCol.iSubItem = 0;
		ListView_InsertColumn( lv, 0, &lvCol );
		lvCol.pszText = TranslateT( "Name" );
		lvCol.cx = 150;
		lvCol.iSubItem = 1;
		ListView_InsertColumn( lv, 1, &lvCol );
		lvCol.pszText = TranslateT( "Type" );
		lvCol.cx = 60;
		lvCol.iSubItem = 2;
		ListView_InsertColumn( lv, 2, &lvCol );
		if ( jabberOnline ) {
			if (( TCHAR* )lParam != NULL ) {
				SetDlgItemText( hwndDlg, IDC_SERVER, ( TCHAR* )lParam );
				int iqId = JabberSerialNext();
				JabberIqAdd( iqId, IQ_PROC_DISCOROOMSERVER, JabberIqResultDiscoRoomItems );

				XmlNodeIq iq( "get", iqId, ( TCHAR* )lParam );
				XmlNode* query = iq.addQuery( "http://jabber.org/protocol/disco#items" );
				JabberSend( jabberThreadInfo->s, iq );
			}
			else {
				for ( int i=0; i < GC_SERVER_LIST_SIZE; i++ ) {
					char text[100];
					mir_snprintf( text, sizeof( text ), "GcServerLast%d", i );
					DBVARIANT dbv;
					if ( !JGetStringT( NULL, text, &dbv )) {
						SendDlgItemMessage( hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, ( LPARAM )dbv.ptszVal );
						JFreeVariant( &dbv );
			}	}	}
		}
		else EnableWindow( GetDlgItem( hwndDlg, IDC_JOIN ), FALSE );
		return TRUE;

	case WM_JABBER_ACTIVATE:
		// lParam = server from which agent information is obtained
		if ( lParam )
			SetDlgItemText( hwndDlg, IDC_SERVER, ( TCHAR* )lParam );
		ListView_DeleteAllItems( GetDlgItem( hwndDlg, IDC_ROOM ));
		EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), FALSE );
		return TRUE;

	case WM_JABBER_REFRESH:
		// lParam = server from which agent information is obtained
		{
			int i;
			TCHAR szBuffer[256];
			char text[128];

			if ( lParam ){
				_tcsncpy( szBuffer, ( TCHAR* )lParam, SIZEOF( szBuffer ));
				for ( i=0; i<GC_SERVER_LIST_SIZE; i++ ) {
					mir_snprintf( text, SIZEOF( text ), "GcServerLast%d", i );
					DBVARIANT dbv;
					if ( !JGetStringT( NULL, text, &dbv )) {
						JSetStringT( NULL, text, szBuffer );
						if ( !_tcsicmp( dbv.ptszVal, ( TCHAR* )lParam )) {
							JFreeVariant( &dbv );
							break;
						}
						_tcsncpy( szBuffer, dbv.ptszVal, SIZEOF( szBuffer ));
						JFreeVariant( &dbv );
					}
					else {
						JSetStringT( NULL, text, szBuffer );
						break;
				}	}

				SendDlgItemMessage( hwndDlg, IDC_SERVER, CB_RESETCONTENT, 0, 0 );
				for ( i=0; i<GC_SERVER_LIST_SIZE; i++ ) {
					mir_snprintf( text, SIZEOF( text ), "GcServerLast%d", i );
					DBVARIANT dbv;
					if ( !JGetStringT( NULL, text, &dbv )) {
						SendDlgItemMessage( hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, ( LPARAM )dbv.ptszVal );
						JFreeVariant( &dbv );
					}
				}
				SetDlgItemText( hwndDlg, IDC_SERVER, ( TCHAR* )lParam );
			}
			i = 0;
			lv = GetDlgItem( hwndDlg, IDC_ROOM );
			ListView_DeleteAllItems( lv );
			LVITEM lvItem;
			lvItem.iItem = 0;
			while (( i=JabberListFindNext( LIST_ROOM, i )) >= 0 ) {
				if (( item=JabberListGetItemPtrFromIndex( i )) != NULL ) {
					lvItem.mask = LVIF_PARAM | LVIF_TEXT;
					lvItem.iSubItem = 0;
					_tcsncpy( szBuffer, item->jid, SIZEOF(szBuffer));
					szBuffer[ SIZEOF(szBuffer)-1 ] = 0;
					lvItem.lParam = ( LPARAM )item->jid;
					lvItem.pszText = szBuffer;
					ListView_InsertItem( lv, &lvItem );

					lvItem.mask = LVIF_TEXT;
					lvItem.iSubItem = 1;
					lvItem.pszText = item->name;
					ListView_SetItem( lv, &lvItem );

					lvItem.iSubItem = 2;
					lvItem.pszText = item->type;
					ListView_SetItem( lv, &lvItem );
					lvItem.iItem++;
				}
				i++;
			}
			EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), TRUE );
		}
		return TRUE;
	case WM_JABBER_CHECK_ONLINE:
	{
		TCHAR text[128];
		if ( jabberOnline ) {
			EnableWindow( GetDlgItem( hwndDlg, IDC_JOIN ), TRUE );
			GetDlgItemText( hwndDlg, IDC_SERVER, text, SIZEOF( text ));
			EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), ( text[0]!='\0' ));
		}
		else {
			EnableWindow( GetDlgItem( hwndDlg, IDC_JOIN ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), FALSE );
			SetDlgItemTextA( hwndDlg, IDC_SERVER, "" );
			lv = GetDlgItem( hwndDlg, IDC_ROOM );
			ListView_DeleteAllItems( lv );
		}
		break;
	}
	case WM_NOTIFY:
		switch ( wParam ) {
		case IDC_ROOM:
			switch (( ( LPNMHDR )lParam )->code ) {
			case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW pnmlv = ( LPNMLISTVIEW ) lParam;

					if ( pnmlv->iSubItem>=0 && pnmlv->iSubItem<=1 ) {
						if ( pnmlv->iSubItem == sortColumn )
							sortAscending = !sortAscending;
						else {
							sortAscending = TRUE;
							sortColumn = pnmlv->iSubItem;
						}
						ListView_SortItems( GetDlgItem( hwndDlg, IDC_ROOM ), GroupchatCompare, sortColumn );
					}
				}
				break;
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case WM_JABBER_JOIN:
			if ( jabberChatDllPresent ) {
				lv = GetDlgItem( hwndDlg, IDC_ROOM );
				if (( lvItem.iItem=ListView_GetNextItem( lv, -1, LVNI_SELECTED )) >= 0 ) {
					lvItem.iSubItem = 0;
					lvItem.mask = LVIF_PARAM;
					ListView_GetItem( lv, &lvItem );
					ListView_SetItemState( lv, lvItem.iItem, 0, LVIS_SELECTED ); // Unselect the item
					DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_GROUPCHAT_JOIN ), hwndDlg, JabberGroupchatJoinDlgProc, ( LPARAM )lvItem.lParam );
				}
				else {
					TCHAR text[128];
					GetDlgItemText( hwndDlg, IDC_SERVER, text, SIZEOF( text ));
					DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_GROUPCHAT_JOIN ), hwndDlg, JabberGroupchatJoinDlgProc, ( LPARAM )text );
			}	}
			else JabberChatDllError();
			return TRUE;

		case WM_JABBER_ADD_TO_ROSTER:
			lv = GetDlgItem( hwndDlg, IDC_ROOM );
			if (( lvItem.iItem=ListView_GetNextItem( lv, -1, LVNI_SELECTED )) >= 0 ) {
				lvItem.iSubItem = 0;
				lvItem.mask = LVIF_PARAM;
				ListView_GetItem( lv, &lvItem );
				TCHAR* jid = ( TCHAR* )lvItem.lParam;
				{	GCSESSION gcw = {0};
					gcw.cbSize = sizeof(GCSESSION);
					gcw.iType = GCW_CHATROOM;
					gcw.pszID = t2a(jid);
					gcw.pszModule = jabberProtoName;
					gcw.pszName = NEWSTR_ALLOCA(gcw.pszID);
					char* p = ( char* )strchr( gcw.pszName, '@' );
					if ( p != NULL )
						*p = 0;
					CallService( MS_GC_NEWSESSION, 0, ( LPARAM )&gcw );
					mir_free((void*)gcw.pszID);
				}
				{	XmlNodeIq iq( "set" );
					XmlNode* query = iq.addQuery( "jabber:iq:roster" );
					XmlNode* item = query->addChild( "item" ); item->addAttr( "jid", jid );
					JabberSend( jabberThreadInfo->s, iq );
				}
				{	XmlNode p( "presence" ); p.addAttr( "to", jid ); p.addAttr( "type", "subscribe" );
					JabberSend( jabberThreadInfo->s, p );
			}	}
			break;

		case IDC_SERVER:
		{	TCHAR text[ 128 ];
			GetDlgItemText( hwndDlg, IDC_SERVER, text, SIZEOF( text ));
			if ( jabberOnline && ( text[0] || HIWORD( wParam )==CBN_SELCHANGE ))
				EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), TRUE );
			break;
		}
		case IDC_BROWSE:
		{	TCHAR text[ 128 ];
			GetDlgItemText( hwndDlg, IDC_SERVER, text, SIZEOF( text ));
			if ( jabberOnline && text[0] ) {
				EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), FALSE );
				ListView_DeleteAllItems( GetDlgItem( hwndDlg, IDC_ROOM ));
				GetDlgItemText( hwndDlg, IDC_SERVER, text, SIZEOF( text ));

				int iqId = JabberSerialNext();
				JabberIqAdd( iqId, IQ_PROC_DISCOROOMSERVER, JabberIqResultDiscoRoomItems );

				XmlNodeIq iq( "get", iqId, text );
				XmlNode* query = iq.addQuery( "http://jabber.org/protocol/disco#items" );
				JabberSend( jabberThreadInfo->s, iq );
			}
			return TRUE;
		}
		case IDCLOSE:
			DestroyWindow( hwndDlg );
			return TRUE;
		}
		break;
	case WM_CONTEXTMENU:
		if (( HWND )wParam == GetDlgItem( hwndDlg, IDC_ROOM )) {
			HMENU hMenu = CreatePopupMenu();
			AppendMenu( hMenu, MF_STRING, WM_JABBER_JOIN, TranslateT( "Join" ));
			AppendMenu( hMenu, MF_STRING, WM_JABBER_ADD_TO_ROSTER, TranslateT( "Add to roster" ));
			TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_NONOTIFY, LOWORD(lParam), HIWORD(lParam), 0, hwndDlg, 0 );
			::DestroyMenu( hMenu );
			return TRUE;
		}
		break;
	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;
	case WM_DESTROY:
		hwndJabberGroupchat = NULL;
		break;
	}
	return FALSE;
}

void JabberGroupchatJoinRoom( const TCHAR* server, const TCHAR* room, const TCHAR* nick, const TCHAR* password )
{
	TCHAR text[JABBER_MAX_JID_LEN];
	mir_sntprintf( text, SIZEOF(text), _T("%s@%s/%s"), room, server, nick );
	JABBER_LIST_ITEM* item = JabberListAdd( LIST_CHATROOM, text );
	replaceStr( item->nick, nick );

	int status = ( jabberStatus == ID_STATUS_INVISIBLE ) ? ID_STATUS_ONLINE : jabberStatus;
	XmlNode* x = new XmlNode( "x" ); x->addAttr( "xmlns", "http://jabber.org/protocol/muc" );
	if ( password && password[0]!='\0' )
		x->addChild( "password", password );
	JabberSendPresenceTo( status, text, x );
}

static BOOL CALLBACK JabberGroupchatJoinDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	TCHAR text[128];
	TCHAR* p;

	switch ( msg ) {
	case WM_INITDIALOG:
		{
			// lParam is the room JID ( room@server ) in UTF-8
			hwndJabberJoinGroupchat = hwndDlg;
			TranslateDialogDefault( hwndDlg );
			if ( lParam ){
				_tcsncpy( text, ( TCHAR* )lParam, SIZEOF( text ));
				if (( p = _tcschr( text, '@' )) != NULL ) {
					*p = '\0';
					// Need to save room name in UTF-8 in case the room codepage is different
					// from the local code page
					TCHAR* room = mir_tstrdup( text );
					SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) room );
					SetDlgItemText( hwndDlg, IDC_ROOM, room );
					SetDlgItemText( hwndDlg, IDC_SERVER, p+1 );
				}
				else SetDlgItemText( hwndDlg, IDC_SERVER, text );
			}

			DBVARIANT dbv;
			if ( !JGetStringT( NULL, "Nick", &dbv )) {
				SetDlgItemText( hwndDlg, IDC_NICK, dbv.ptszVal );
				JFreeVariant( &dbv );
			}
			else {
				TCHAR* nick = JabberNickFromJID( jabberJID );
				SetDlgItemText( hwndDlg, IDC_NICK, nick );
				mir_free( nick );
		}	}
		return TRUE;

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_ROOM:
			if (( HWND )lParam==GetFocus() && HIWORD( wParam )==EN_CHANGE ) {
				// Change in IDC_ROOM field is detected,
				// invalidate the saved UTF-8 room name if any
				char* str = ( char* )GetWindowLong( hwndDlg, GWL_USERDATA );
				if ( str != NULL ) {
					mir_free( str );
					SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) NULL );
			}	}
			break;
		case IDOK:
			{
				GetDlgItemText( hwndDlg, IDC_SERVER, text, SIZEOF( text ));
				TCHAR* server = NEWTSTR_ALLOCA( text ), *room;

				if (( room=( TCHAR* )GetWindowLong( hwndDlg, GWL_USERDATA )) != NULL )
					room = NEWTSTR_ALLOCA( room );
				else {
					GetDlgItemText( hwndDlg, IDC_ROOM, text, SIZEOF( text ));
					room = NEWTSTR_ALLOCA( text );
				}

				GetDlgItemText( hwndDlg, IDC_NICK, text, SIZEOF( text ));
				TCHAR* nick = NEWTSTR_ALLOCA( text );

				GetDlgItemText( hwndDlg, IDC_PASSWORD, text, SIZEOF( text ));
				TCHAR* password = NEWTSTR_ALLOCA( text );
				JabberGroupchatJoinRoom( server, room, nick, password );
			}
			// fall through
		case IDCANCEL:
			EndDialog( hwndDlg, 0 );
			break;
		}
		break;
	case WM_JABBER_CHECK_ONLINE:
		if ( !jabberOnline ) EndDialog( hwndDlg, 0 );
		break;
	case WM_DESTROY:
		{
			char* str = ( char* )GetWindowLong( hwndDlg, GWL_USERDATA );
			if ( str != NULL )
				mir_free( str );

			hwndJabberJoinGroupchat = NULL;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGroupchatProcessPresence - handles the group chat presence packet

static int sttGetStatusCode( XmlNode* node )
{
	XmlNode* statusNode = JabberXmlGetChild( node, "status" );
	if ( statusNode == NULL )
		return -1;

	TCHAR* statusCode = JabberXmlGetAttrValue( statusNode, "code" );
	if ( statusCode == NULL )
		return -1;

	return _ttol( statusCode );
}

void sttRenameParticipantNick( JABBER_LIST_ITEM* item, TCHAR* oldNick, XmlNode *itemNode )
{
	TCHAR* newNick = JabberXmlGetAttrValue( itemNode, "nick" );
	if ( newNick == NULL )
		return;

	for ( int i=0; i < item->resourceCount; i++ ) {
		JABBER_RESOURCE_STATUS& RS = item->resource[i];
		if ( !lstrcmp( RS.resourceName, oldNick )) {
			replaceStr( RS.resourceName, newNick );

			if ( !lstrcmp( item->nick, oldNick )) {
				replaceStr( item->nick, newNick );

				HANDLE hContact = JabberHContactFromJID( item->jid );
				if ( hContact != NULL )
					JSetStringT( hContact, "MyNick", newNick );
			}

			#if defined( _UNICODE )
				char* jid = u2a( item->jid );
				char* dispNick = u2a( newNick );
				char* dispOldNick = u2a( oldNick );
			#else
				char* jid = item->jid;
				char* dispNick = NEWSTR_ALLOCA( newNick );
				char* dispOldNick = NEWSTR_ALLOCA( oldNick );
			#endif

			GCDEST gcd = { jabberProtoName, jid, GC_EVENT_CHUID };
			GCEVENT gce = {0};
			gce.cbSize = sizeof(GCEVENT);
			gce.pDest = &gcd;
			gce.pszNick = dispOldNick;
			gce.pszText = dispNick;
			gce.time = time(0);
			JCallService( MS_GC_EVENT, NULL, ( LPARAM )&gce );

			gcd.iType = GC_EVENT_NICK;
			gce.pszNick = dispOldNick;
			gce.pszUID = dispNick;
			gce.pszText = dispNick;
			JCallService( MS_GC_EVENT, NULL, ( LPARAM )&gce );

			#if defined( _UNICODE )
				mir_free( jid );
				mir_free( dispNick );
				mir_free( dispOldNick );
			#endif
			break;
}	}	}

void JabberGroupchatProcessPresence( XmlNode *node, void *userdata )
{
	struct ThreadData *info;
	XmlNode *showNode, *statusNode, *errorNode, *itemNode, *n;
	TCHAR* from;
	int status, newRes;
	int i;
	BOOL roomCreated;

	if ( !node || !node->name || strcmp( node->name, "presence" )) return;
	if (( info=( struct ThreadData * ) userdata ) == NULL ) return;
	if (( from=JabberXmlGetAttrValue( node, "from" )) == NULL ) return;

	TCHAR* nick = _tcschr( from, '/' );
	if ( nick == NULL || nick[1] == '\0' )
		return;
	nick++;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, from );
	if ( item == NULL )
		return;

	XmlNode* xNode = JabberXmlGetChildWithGivenAttrValue( node, "x", "xmlns", _T("http://jabber.org/protocol/muc#user"));

	TCHAR* type = JabberXmlGetAttrValue( node, "type" );
	if ( type == NULL || !_tcscmp( type, _T("available"))) {
		TCHAR* room = JabberNickFromJID( from );
		if ( room == NULL )
			return;

		JabberGcLogCreate( item );

		// Update status of room participant
		status = ID_STATUS_ONLINE;
		if (( showNode=JabberXmlGetChild( node, "show" )) != NULL ) {
			if ( showNode->text != NULL ) {
				if ( !_tcscmp( showNode->text , _T("away"))) status = ID_STATUS_AWAY;
				else if ( !_tcscmp( showNode->text , _T("xa"))) status = ID_STATUS_NA;
				else if ( !_tcscmp( showNode->text , _T("dnd"))) status = ID_STATUS_DND;
				else if ( !_tcscmp( showNode->text , _T("chat"))) status = ID_STATUS_FREECHAT;
		}	}

		TCHAR* str;
		if (( statusNode=JabberXmlGetChild( node, "status" ))!=NULL && statusNode->text!=NULL )
			str = statusNode->text;
		else
			str = NULL;
		newRes = ( JabberListAddResource( LIST_CHATROOM, from, status, str ) == 0 ) ? 0 : GC_EVENT_JOIN;

		roomCreated = FALSE;

		// Check additional MUC info for this user
		if ( xNode != NULL ) {
			if (( itemNode=JabberXmlGetChild( xNode, "item" )) != NULL ) {
				JABBER_RESOURCE_STATUS* r = item->resource;
				for ( i=0; i<item->resourceCount && _tcscmp( r->resourceName, nick ); i++, r++ );
				if ( i < item->resourceCount ) {
					if (( str=JabberXmlGetAttrValue( itemNode, "affiliation" )) != NULL ) {
						if ( !_tcscmp( str, _T("owner")))        r->affiliation = AFFILIATION_OWNER;
						else if ( !_tcscmp( str, _T("admin")))   r->affiliation = AFFILIATION_ADMIN;
						else if ( !_tcscmp( str, _T("member")))  r->affiliation = AFFILIATION_MEMBER;
						else if ( !_tcscmp( str, _T("outcast"))) r->affiliation = AFFILIATION_OUTCAST;
					}
					if (( str=JabberXmlGetAttrValue( itemNode, "role" )) != NULL ) {
						JABBER_GC_ROLE newRole = r->role;

						if ( !_tcscmp( str, _T("moderator")))        newRole = ROLE_MODERATOR;
						else if ( !_tcscmp( str, _T("participant"))) newRole = ROLE_PARTICIPANT;
						else if ( !_tcscmp( str, _T("visitor")))     newRole = ROLE_VISITOR;
						else                                         newRole = ROLE_NONE;

						if ( newRole != r->role && r->role != ROLE_NONE ) {
							JabberGcLogUpdateMemberStatus( item, nick, GC_EVENT_REMOVESTATUS, NULL );
							newRes = GC_EVENT_ADDSTATUS;
						}
						r->role = newRole;
			}	}	}

			if ( sttGetStatusCode( xNode ) == 201 )
				roomCreated = TRUE;
		}

		// Update groupchat log window
		JabberGcLogUpdateMemberStatus( item, nick, newRes, NULL );

		HANDLE hContact = JabberHContactFromJID( from );
		if ( hContact != NULL )
			JSetWord( hContact, "Status", status );

		// Update room status
		//if ( item->status != ID_STATUS_ONLINE ) {
		//	item->status = ID_STATUS_ONLINE;
		//	JSetWord( hContact, "Status", ( WORD )ID_STATUS_ONLINE );
		//	JabberLog( "Room %s online", from );
		//}

		// Check <created/>
		if ( roomCreated ||
			(( n=JabberXmlGetChild( node, "created" ))!=NULL &&
				( str=JabberXmlGetAttrValue( n, "xmlns" ))!=NULL &&
				!_tcscmp( str, _T("http://jabber.org/protocol/muc#owner"))) ) {
			// A new room just created by me
			// Request room config
			int iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetMuc );

			XmlNodeIq iq( "get", iqId, item->jid );
			XmlNode* query = iq.addQuery( xmlnsOwner );
			JabberSend( jabberThreadInfo->s, iq );
		}

		mir_free( room );
	}
	else if ( !lstrcmp( type, _T("unavailable"))) {
		if ( xNode != NULL && item->nick != NULL ) {
			itemNode = JabberXmlGetChild( xNode, "item" );
			XmlNode* reasonNode = JabberXmlGetChild( itemNode, "reason" );

			if ( !lstrcmp( nick, item->nick )) {
				int iStatus = sttGetStatusCode( xNode );
				switch( iStatus ) {
				case 301:	case 307:
					JabberGcQuit( item, iStatus, reasonNode );
					break;

				case 303:
					sttRenameParticipantNick( item, nick, itemNode );
					return;
			}	}
			else {
				switch( sttGetStatusCode( xNode )) {
				case 303:
					sttRenameParticipantNick( item, nick, itemNode );
					return;

				case 307:
					JabberListRemoveResource( LIST_CHATROOM, from );
					JabberGcLogUpdateMemberStatus( item, nick, GC_EVENT_KICK, reasonNode );
					return;
		}	}	}

		JabberListRemoveResource( LIST_CHATROOM, from );
		JabberGcLogUpdateMemberStatus( item, nick, GC_EVENT_PART, NULL );
	}
	else if ( !lstrcmp( type, _T("error"))) {
		errorNode = JabberXmlGetChild( node, "error" );
		TCHAR* str = JabberErrorMsg( errorNode );
		MessagePopup( NULL, str, TranslateT( "Jabber Error Message" ), MB_OK|MB_SETFOREGROUND );
		//JabberListRemoveResource( LIST_CHATROOM, from );
		JabberListRemove( LIST_CHATROOM, from );
		mir_free( str );
}	}

void strdel( char* parBuffer, int len )
{
	char* p;
	for ( p = parBuffer+len; *p != 0; p++ )
		p[ -len ] = *p;

	p[ -len ] = '\0';
}

void JabberGroupchatProcessMessage( XmlNode *node, void *userdata )
{
	struct ThreadData *info;
	XmlNode *n, *xNode;
	TCHAR* from, *type, *p, *nick, *msgText;
	JABBER_LIST_ITEM *item;

	if ( !node->name || strcmp( node->name, "message" )) return;
	if (( info=( struct ThreadData * ) userdata ) == NULL ) return;
	if (( from = JabberXmlGetAttrValue( node, "from" )) == NULL ) return;
	if (( item = JabberListGetItemPtr( LIST_CHATROOM, from )) == NULL ) return;

	if (( type = JabberXmlGetAttrValue( node, "type" )) == NULL ) return;
	if ( !lstrcmp( type, _T("error")))
		return;

	GCDEST gcd = { jabberProtoName, NULL, 0 };
	gcd.ptszID = item->jid;

	if (( n = JabberXmlGetChild( node, "subject" )) != NULL ) {
		if ( n->text == NULL || n->text[0] == '\0' )
			return;

		msgText = n->text;

		gcd.iType = GC_EVENT_TOPIC;

		if ( from != NULL ) {
			nick = _tcschr( from, '/' );
			if ( nick == NULL || nick[1] == '\0' )
				nick = NULL;
			else
				nick++;
		}
		else nick = NULL;
	}
	else {
		if (( n = JabberXmlGetChild( node, "body" )) == NULL ) return;
		if ( n->text == NULL )
			return;

		nick = _tcschr( from, '/' );
		if ( nick == NULL || nick[1] == '\0' )
			return;
		nick++;

		msgText = n->text;

		if ( (_tcsncmp( msgText, _T("/me "), 4 ) == 0) && (_tcslen(msgText)>4) ){
			msgText += 4;
			gcd.iType = GC_EVENT_ACTION;
		}
		else gcd.iType = GC_EVENT_MESSAGE;
	}

	JabberGcLogCreate( item );

	time_t msgTime = 0;
	BOOL delivered = FALSE;
	for ( int i = 1; ( xNode=JabberXmlGetNthChild( node, "x", i )) != NULL; i++ )
		if (( p=JabberXmlGetAttrValue( xNode, "xmlns" )) != NULL )
			if ( !_tcscmp( p, _T("jabber:x:delay")) && msgTime==0 )
				if (( p=JabberXmlGetAttrValue( xNode, "stamp" )) != NULL )
					msgTime = JabberIsoToUnixTime( p );

	time_t now = time( NULL );
	if ( msgTime == 0 || msgTime > now )
		msgTime = now;

	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.ptszUID = nick;
	gce.ptszNick = nick;
	gce.bAddToLog = TRUE;
	gce.time = msgTime;
	gce.ptszText = EscapeChatTags( msgText );
	gce.bIsMe = lstrcmp( nick, item->nick ) == 0;
	gce.dwFlags = GC_TCHAR;
	JCallService(MS_GC_EVENT, NULL, (LPARAM)&gce);

	item->bChatActive = 2;

	if ( gcd.iType == GC_EVENT_TOPIC ) {
		gce.bAddToLog = FALSE;
		gcd.iType = GC_EVENT_SETSBTEXT;
		JCallService(MS_GC_EVENT, NULL, (LPARAM)&gce);
	}

	mir_free( (void*)gce.pszText ); // Since we processed msgText and created a new string
}

/////////////////////////////////////////////////////////////////////////////////////////
// Accepting groupchat invitations

typedef struct {
	TCHAR* roomJid;
	TCHAR* from;
	TCHAR* reason;
	TCHAR* password;
}
	JABBER_GROUPCHAT_INVITE_INFO;

static void JabberAcceptGroupchatInvite( TCHAR* roomJid, TCHAR* reason, TCHAR* password )
{
	TCHAR room[256], *server, *p;
	_tcsncpy( room, roomJid, SIZEOF( room ));
	p = _tcstok( room, _T( "@" ));
	server = _tcstok( NULL, _T( "@" ));
	JabberGroupchatJoinRoom( server, p, reason, password );
}

static BOOL CALLBACK JabberGroupchatInviteAcceptDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		{
			JABBER_GROUPCHAT_INVITE_INFO *inviteInfo = ( JABBER_GROUPCHAT_INVITE_INFO * ) lParam;

			TranslateDialogDefault( hwndDlg );
			SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) inviteInfo );
			SetDlgItemText( hwndDlg, IDC_FROM, inviteInfo->from );
			SetDlgItemText( hwndDlg, IDC_ROOM, inviteInfo->roomJid );

			if ( inviteInfo->reason != NULL )
				SetDlgItemText( hwndDlg, IDC_REASON, inviteInfo->reason );

			TCHAR* myNick = JabberNickFromJID( jabberJID );
			SetDlgItemText( hwndDlg, IDC_NICK, myNick );
			mir_free( myNick );

			SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )iconBigList[0] );
		}
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_ACCEPT:
			{
				JABBER_GROUPCHAT_INVITE_INFO *inviteInfo = ( JABBER_GROUPCHAT_INVITE_INFO * ) GetWindowLong( hwndDlg, GWL_USERDATA );
				TCHAR text[128];
				GetDlgItemText( hwndDlg, IDC_NICK, text, SIZEOF( text ));
				JabberAcceptGroupchatInvite( inviteInfo->roomJid, text, inviteInfo->password );
			}
			// Fall through
		case IDCANCEL:
		case IDCLOSE:
			EndDialog( hwndDlg, 0 );
			return TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog( hwndDlg, 0 );
		break;
	}

	return FALSE;
}

static void __cdecl JabberGroupchatInviteAcceptThread( JABBER_GROUPCHAT_INVITE_INFO *inviteInfo )
{
	DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_GROUPCHAT_INVITE_ACCEPT ), NULL, JabberGroupchatInviteAcceptDlgProc, ( LPARAM )inviteInfo );
	mir_free( inviteInfo->roomJid );
	mir_free( inviteInfo->from );
	mir_free( inviteInfo->reason );
	mir_free( inviteInfo->password );
	mir_free( inviteInfo );
}

void JabberGroupchatProcessInvite( TCHAR* roomJid, TCHAR* from, TCHAR* reason, TCHAR* password )
{
	if ( roomJid == NULL )
		return;

	if ( JGetByte( "AutoAcceptMUC", FALSE ) == FALSE ) {
		JABBER_GROUPCHAT_INVITE_INFO* inviteInfo = ( JABBER_GROUPCHAT_INVITE_INFO * ) mir_alloc( sizeof( JABBER_GROUPCHAT_INVITE_INFO ));
		inviteInfo->roomJid  = mir_tstrdup( roomJid );
		inviteInfo->from     = mir_tstrdup( from );
		inviteInfo->reason   = mir_tstrdup( reason );
		inviteInfo->password = mir_tstrdup( password );
		JabberForkThread(( JABBER_THREAD_FUNC )JabberGroupchatInviteAcceptThread, 0, ( void* )inviteInfo );
	}
	else {
		TCHAR* myNick = JabberNickFromJID( jabberJID );
		JabberAcceptGroupchatInvite( roomJid, myNick, password );
		mir_free( myNick );
}	}
