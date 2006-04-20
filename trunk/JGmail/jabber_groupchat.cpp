/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005     George Hazan

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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_groupchat.cpp,v $
Revision       : $Revision: 1.40 $
Last change on : $Date: 2006/02/27 21:56:07 $
Last change by : $Author: ghazan $

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
            JabberSend( jabberThreadInfo->s, "<iq type=\"get\" id=\""JABBER_IQID"%d\" to=\"%s\"><query xmlns=\"http://jabber.org/protocol/disco#items\"/></iq>", iqId, ( char* )lParam );
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
	char* localJID1, *localJID2;
	int res;

	res = 0;
	item1 = JabberListGetItemPtr( LIST_ROOM, ( char* )lParam1 );
	item2 = JabberListGetItemPtr( LIST_ROOM, ( char* )lParam2 );
	if ( item1!=NULL && item2!=NULL ) {
		switch ( lParamSort ) {
		case 0:	// sort by JID column
			{
				localJID1 = JabberTextDecode( item1->jid );
				localJID2 = JabberTextDecode( item2->jid );
				if ( localJID1!=NULL && localJID2!=NULL )
					res = strcmp( localJID1, localJID2 );
				free( localJID1 );
				free( localJID2 );
			}
			break;
		case 1: // sort by Name column
			if ( item1->name!=NULL && item2->name!=NULL )
				res = strcmp( item1->name, item2->name );
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
	char text[128];
	char* p;

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
			if (( char* )lParam != NULL ) {
				SetDlgItemTextA( hwndDlg, IDC_SERVER, ( char* )lParam );
				int iqId = JabberSerialNext();
				JabberIqAdd( iqId, IQ_PROC_DISCOROOMSERVER, JabberIqResultDiscoRoomItems );
                JabberSend( jabberThreadInfo->s, "<iq type=\"get\" id=\""JABBER_IQID"%d\" to=\"%s\"><query xmlns=\"http://jabber.org/protocol/disco#items\"/></iq>", iqId, ( char* )lParam );
			}
			else {
				DBVARIANT dbv;
				int i;

				for ( i=0; i<GC_SERVER_LIST_SIZE; i++ ) {
					mir_snprintf( text, sizeof( text ), "GcServerLast%d", i );
					if ( !DBGetContactSetting( NULL, jabberProtoName, text, &dbv )) {
						SendDlgItemMessageA( hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, ( LPARAM )dbv.pszVal );
						JFreeVariant( &dbv );
			}	}	}
		}
		else EnableWindow( GetDlgItem( hwndDlg, IDC_JOIN ), FALSE );
		return TRUE;
	case WM_JABBER_ACTIVATE:
		// lParam = server from which agent information is obtained
		if ( lParam )
			SetDlgItemTextA( hwndDlg, IDC_SERVER, ( char* )lParam );
		ListView_DeleteAllItems( GetDlgItem( hwndDlg, IDC_ROOM ));
		EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), FALSE );
		return TRUE;
	case WM_JABBER_REFRESH:
		// lParam = server from which agent information is obtained
		{
			int i;
			char szBuffer[256];

			if ( lParam ){
				DBVARIANT dbv;

				strncpy( szBuffer, ( char* )lParam, sizeof( szBuffer ));
				for ( i=0; i<GC_SERVER_LIST_SIZE; i++ ) {
					mir_snprintf( text, sizeof( text ), "GcServerLast%d", i );
					if ( !DBGetContactSetting( NULL, jabberProtoName, text, &dbv )) {
						JSetString( NULL, text, szBuffer );
						if ( !stricmp( dbv.pszVal, ( char* )lParam )) {
							JFreeVariant( &dbv );
							break;
						}
						strncpy( szBuffer, dbv.pszVal, sizeof( szBuffer ));
						JFreeVariant( &dbv );
					}
					else {
						JSetString( NULL, text, szBuffer );
						break;
					}
				}
				SendDlgItemMessage( hwndDlg, IDC_SERVER, CB_RESETCONTENT, 0, 0 );
				for ( i=0; i<GC_SERVER_LIST_SIZE; i++ ) {
					mir_snprintf( text, sizeof( text ), "GcServerLast%d", i );
					if ( !DBGetContactSetting( NULL, jabberProtoName, text, &dbv )) {
						SendDlgItemMessageA( hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, ( LPARAM )dbv.pszVal );
						JFreeVariant( &dbv );
					}
				}
				SetDlgItemTextA( hwndDlg, IDC_SERVER, ( char* )lParam );
			}
			i = 0;
			lv = GetDlgItem( hwndDlg, IDC_ROOM );
			ListView_DeleteAllItems( lv );
			LVITEMA lvItem;
			lvItem.iItem = 0;
			while (( i=JabberListFindNext( LIST_ROOM, i )) >= 0 ) {
				if (( item=JabberListGetItemPtrFromIndex( i )) != NULL ) {
					lvItem.mask = LVIF_PARAM | LVIF_TEXT;
					lvItem.iSubItem = 0;
					strncpy( szBuffer, item->jid, sizeof(szBuffer));
					szBuffer[ sizeof(szBuffer)-1 ] = 0;
					JabberUtf8Decode( szBuffer, NULL );
					lvItem.lParam = ( LPARAM )item->jid;
					lvItem.pszText = szBuffer;
					SendMessageA( lv, LVM_INSERTITEMA, 0, (LPARAM)&lvItem );

					lvItem.mask = LVIF_TEXT;
					lvItem.iSubItem = 1;
					lvItem.pszText = item->name;
					SendMessageA( lv, LVM_SETITEMA, 0, (LPARAM)&lvItem );
					lvItem.iSubItem = 2;
					lvItem.pszText = item->type;
					SendMessageA( lv, LVM_SETITEMA, 0, (LPARAM)&lvItem );
					lvItem.iItem++;
				}
				i++;
			}
			EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), TRUE );
		}
		return TRUE;
	case WM_JABBER_CHECK_ONLINE:
		if ( jabberOnline ) {
			EnableWindow( GetDlgItem( hwndDlg, IDC_JOIN ), TRUE );
			GetDlgItemTextA( hwndDlg, IDC_SERVER, text, sizeof( text ));
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
					GetDlgItemTextA( hwndDlg, IDC_SERVER, text, sizeof( text ));
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
				char* jid = ( char* )lvItem.lParam;
				{	GCSESSION gcw = {0};
					gcw.cbSize = sizeof(GCSESSION);
					gcw.iType = GCW_CHATROOM;
					gcw.pszID = jid;
					gcw.pszModule = jabberProtoName;
					gcw.pszName = strcpy(( char* )alloca( strlen(jid)+1 ), jid );
					if (( p = (char*)strchr( gcw.pszName, '@' )) != NULL )
						*p = 0;
					CallService( MS_GC_NEWSESSION, 0, ( LPARAM )&gcw );
				}

                JabberSend( jabberThreadInfo->s, "<iq type=\"set\"><query xmlns=\"jabber:iq:roster\"><item jid=\"%s\"/></query></iq>", jid );
                JabberSend( jabberThreadInfo->s, "<presence to=\"%s\" type=\"subscribe\"/>", jid );
			}
			break;
		case IDC_SERVER:
			GetDlgItemTextA( hwndDlg, IDC_SERVER, text, sizeof( text ));
			if ( jabberOnline && ( text[0] || HIWORD( wParam )==CBN_SELCHANGE ))
				EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), TRUE );
			break;
		case IDC_BROWSE:
			GetDlgItemTextA( hwndDlg, IDC_SERVER, text, sizeof( text ));
			if ( jabberOnline && text[0] ) {
				EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), FALSE );
				ListView_DeleteAllItems( GetDlgItem( hwndDlg, IDC_ROOM ));
				GetDlgItemTextA( hwndDlg, IDC_SERVER, text, sizeof( text ));
				if (( p=JabberTextEncode( text )) != NULL ) {
					int iqId = JabberSerialNext();
					JabberIqAdd( iqId, IQ_PROC_DISCOROOMSERVER, JabberIqResultDiscoRoomItems );
                    JabberSend( jabberThreadInfo->s, "<iq type=\"get\" id=\""JABBER_IQID"%d\" to=\"%s\"><query xmlns=\"http://jabber.org/protocol/disco#items\"/></iq>", iqId, p );
					free( p );
				}
			}
			return TRUE;
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

void JabberGroupchatJoinRoom( const char* server, const char* room, const char* nick, const char* password )
{
	char text[128];
	mir_snprintf( text, sizeof text, "%s@%s/%s", room, server, nick );

	JABBER_LIST_ITEM* item = JabberListAdd( LIST_CHATROOM, text );
	replaceStr( item->nick, nick );

	int status = ( jabberStatus == ID_STATUS_INVISIBLE ) ? ID_STATUS_ONLINE : jabberStatus;
	if ( password && password[0]!='\0' ) {
		char passwordText[256];
        mir_snprintf( passwordText, sizeof( passwordText ), "<x xmlns=\"http://jabber.org/protocol/muc\"><password>%s</password></x>", password );
		JabberSendPresenceTo( status, text, passwordText );
	}
    else JabberSendPresenceTo( status, text, "<x xmlns=\"http://jabber.org/protocol/muc\"/>" );
}

static BOOL CALLBACK JabberGroupchatJoinDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	char text[128];
	char* p;

	switch ( msg ) {
	case WM_INITDIALOG:
		{
			// lParam is the room JID ( room@server ) in UTF-8
			hwndJabberJoinGroupchat = hwndDlg;
			TranslateDialogDefault( hwndDlg );
			if ( lParam ){
				strncpy( text, ( char* )lParam, sizeof( text ));
				if (( p=strchr( text, '@' )) != NULL ) {
					*p = '\0';
					// Need to save room name in UTF-8 in case the room codepage is different
					// from the local code page
					char* room = strdup( text );
					SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) room );
					SetDlgItemTextA( hwndDlg, IDC_ROOM, room );
					SetDlgItemTextA( hwndDlg, IDC_SERVER, p+1 );
				}
				else SetDlgItemTextA( hwndDlg, IDC_SERVER, text );
			}

			char szNick[ 256 ];
			if ( !JGetStaticString( "Nick", NULL, szNick, sizeof szNick ))
				SetDlgItemTextA( hwndDlg, IDC_NICK, szNick );
			else {
				char* nick = JabberNickFromJID( jabberJID );
				SetDlgItemTextA( hwndDlg, IDC_NICK, nick );
				free( nick );
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
					free( str );
					SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) NULL );
			}	}
			break;
		case IDOK:
			{
				GetDlgItemTextA( hwndDlg, IDC_SERVER, text, sizeof( text ));
				char* server = NEWSTR_ALLOCA( text ), *room;

				if (( room=( char* )GetWindowLong( hwndDlg, GWL_USERDATA )) != NULL )
					room = NEWSTR_ALLOCA( room );
				else {
					GetDlgItemTextA( hwndDlg, IDC_ROOM, text, sizeof( text ));
					room = NEWSTR_ALLOCA( text );
				}

				GetDlgItemTextA( hwndDlg, IDC_NICK, text, sizeof( text ));
				char* nick = NEWSTR_ALLOCA( text );

				GetDlgItemTextA( hwndDlg, IDC_PASSWORD, text, sizeof( text ));
				char* password = NEWSTR_ALLOCA( text );
				JabberGroupchatJoinRoom( server, room, TXT(nick), password );
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
				free( str );

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

	char* statusCode = JabberXmlGetAttrValue( statusNode, "code" );
	if ( statusCode == NULL )
		return -1;

	return atol( statusCode );
}

void sttRenameParticipantNick( JABBER_LIST_ITEM* item, char* oldNick, XmlNode *itemNode )
{
	char* newNick = JabberXmlGetAttrValue( itemNode, "nick" );
	if ( newNick == NULL )
		return;

	for ( int i=0; i < item->resourceCount; i++ ) {
		JABBER_RESOURCE_STATUS& RS = item->resource[i];
		if ( !strcmp( RS.resourceName, oldNick )) {
			replaceStr( RS.resourceName, newNick );

			if ( !lstrcmpA( item->nick, oldNick )) {
				replaceStr( item->nick, newNick );

				HANDLE hContact = JabberHContactFromJID( item->jid );
				if ( hContact != NULL )
					JSetStringUtf( hContact, "MyNick", newNick );
			}

			char* dispNick = NEWSTR_ALLOCA( newNick );
			JabberUtf8Decode( dispNick, NULL );
			char* dispOldNick = NEWSTR_ALLOCA( oldNick );
			JabberUtf8Decode( dispOldNick, NULL );

			GCDEST gcd = { jabberProtoName, item->jid, GC_EVENT_CHUID };
			GCEVENT gce = {0};
			gce.cbSize = sizeof(GCEVENT);
			gce.pDest = &gcd;
			gce.pszNick = oldNick;
			gce.pszText = newNick;
			gce.time = time(0);
			JCallService( MS_GC_EVENT, NULL, ( LPARAM )&gce );

			gcd.iType = GC_EVENT_NICK;
			gce.pszNick = dispOldNick;
			gce.pszUID = newNick;
			gce.pszText = dispNick;
			JCallService( MS_GC_EVENT, NULL, ( LPARAM )&gce );
			break;
}	}	}

void JabberGroupchatProcessPresence( XmlNode *node, void *userdata )
{
	struct ThreadData *info;
	XmlNode *showNode, *statusNode, *errorNode, *itemNode, *n;
	char* from;
	int status, newRes;
	char* str;
	int i;
	BOOL roomCreated;

	if ( !node || !node->name || strcmp( node->name, "presence" )) return;
	if (( info=( struct ThreadData * ) userdata ) == NULL ) return;
	if (( from=JabberXmlGetAttrValue( node, "from" )) == NULL ) return;

	char* nick = strchr( from, '/' );
	if ( nick == NULL || nick[1] == '\0' )
		return;
	nick++;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, from );
	if ( item == NULL )
		return;

	XmlNode* xNode = JabberXmlGetChildWithGivenAttrValue( node, "x", "xmlns", "http://jabber.org/protocol/muc#user" );

	char* type = JabberXmlGetAttrValue( node, "type" );
	if ( type == NULL || !strcmp( type, "available" )) {
		char* room = JabberNickFromJID( from );
		if ( room == NULL )
			return;

		JabberGcLogCreate( item );

		// Update status of room participant
		status = ID_STATUS_ONLINE;
		if (( showNode=JabberXmlGetChild( node, "show" )) != NULL ) {
			if ( showNode->text != NULL ) {
				if ( !strcmp( showNode->text , "away" )) status = ID_STATUS_AWAY;
				else if ( !strcmp( showNode->text , "xa" )) status = ID_STATUS_NA;
				else if ( !strcmp( showNode->text , "dnd" )) status = ID_STATUS_DND;
				else if ( !strcmp( showNode->text , "chat" )) status = ID_STATUS_FREECHAT;
		}	}

		if (( statusNode=JabberXmlGetChild( node, "status" ))!=NULL && statusNode->text!=NULL )
			str = JabberTextDecode( statusNode->text );
		else
			str = NULL;
		newRes = ( JabberListAddResource( LIST_CHATROOM, from, status, str ) == 0 ) ? 0 : GC_EVENT_JOIN;
		if ( str ) free( str );

		roomCreated = FALSE;

		// Check additional MUC info for this user
		if ( xNode != NULL ) {
			if (( itemNode=JabberXmlGetChild( xNode, "item" )) != NULL ) {
				JABBER_RESOURCE_STATUS* r = item->resource;
				for ( i=0; i<item->resourceCount && strcmp( r->resourceName, nick ); i++, r++ );
				if ( i < item->resourceCount ) {
					if (( str=JabberXmlGetAttrValue( itemNode, "affiliation" )) != NULL ) {
						if ( !strcmp( str, "owner" )) r->affiliation = AFFILIATION_OWNER;
						else if ( !strcmp( str, "admin" )) r->affiliation = AFFILIATION_ADMIN;
						else if ( !strcmp( str, "member" )) r->affiliation = AFFILIATION_MEMBER;
						else if ( !strcmp( str, "outcast" )) r->affiliation = AFFILIATION_OUTCAST;
					}
					if (( str=JabberXmlGetAttrValue( itemNode, "role" )) != NULL ) {
						JABBER_GC_ROLE newRole = r->role;

						if ( !strcmp( str, "moderator" ))         newRole = ROLE_MODERATOR;
						else if ( !strcmp( str, "participant" ))  newRole = ROLE_PARTICIPANT;
						else if ( !strcmp( str, "visitor" ))      newRole = ROLE_VISITOR;
						else                                      newRole = ROLE_NONE;

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
				!strcmp( str, "http://jabber.org/protocol/muc#owner" )) ) {
			// A new room just created by me
			// Request room config
			int iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetMuc );
            JabberSend( jabberThreadInfo->s, "<iq type=\"get\" id=\""JABBER_IQID"%d\" to=\"%s\">%s</query></iq>",
				iqId, item->jid, xmlnsOwner );
		}

		free( room );
	}
	else if ( !strcmp( type, "unavailable" )) {
		if ( xNode != NULL && item->nick != NULL ) {
			itemNode = JabberXmlGetChild( xNode, "item" );
			XmlNode* reasonNode = JabberXmlGetChild( itemNode, "reason" );

			if ( !strcmp( nick, item->nick )) {
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
	else if ( !strcmp( type, "error" )) {
		errorNode = JabberXmlGetChild( node, "error" );
		str = JabberErrorMsg( errorNode );
		MessageBoxA( NULL, str, JTranslate( "Jabber Error Message" ), MB_OK|MB_SETFOREGROUND );
		//JabberListRemoveResource( LIST_CHATROOM, from );
		JabberListRemove( LIST_CHATROOM, from );
		free( str );
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
	char* from, *type, *p, *nick, *msgText;
	JABBER_LIST_ITEM *item;

	if ( !node->name || strcmp( node->name, "message" )) return;
	if (( info=( struct ThreadData * ) userdata ) == NULL ) return;
	if (( from = JabberXmlGetAttrValue( node, "from" )) == NULL ) return;
	if (( item = JabberListGetItemPtr( LIST_CHATROOM, from )) == NULL ) return;

	if (( type = JabberXmlGetAttrValue( node, "type" )) == NULL ) return;
	if ( !strcmp( type, "error" ))
		return;

	GCDEST gcd = { jabberProtoName, item->jid, 0 };

	if (( n = JabberXmlGetChild( node, "subject" )) != NULL ) {
		if ( n->text == NULL || n->text[0] == '\0' )
			return;

		msgText = JabberTextDecode( n->text );

		gcd.iType = GC_EVENT_TOPIC;

		if ( from != NULL ) {
			nick = strchr( from, '/' );
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

		nick = strchr( from, '/' );
		if ( nick == NULL || nick[1] == '\0' )
			return;
		nick++;

		msgText = JabberTextDecode( n->text );

		if ( memcmp( msgText, "/me", 3 ) == 0 ) {
			strdel( msgText, 4 );
			gcd.iType = GC_EVENT_ACTION;
		}
		else gcd.iType = GC_EVENT_MESSAGE;
	}

	JabberGcLogCreate( item );

	time_t msgTime = 0;
	BOOL delivered = FALSE;
	for ( int i = 1; ( xNode=JabberXmlGetNthChild( node, "x", i )) != NULL; i++ )
		if (( p=JabberXmlGetAttrValue( xNode, "xmlns" )) != NULL )
			if ( !strcmp( p, "jabber:x:delay" ) && msgTime==0 )
				if (( p=JabberXmlGetAttrValue( xNode, "stamp" )) != NULL )
					msgTime = JabberIsoToUnixTime( p );

	time_t now = time( NULL );
	if ( msgTime == 0 || msgTime > now )
		msgTime = now;

	char* dispNick = NEWSTR_ALLOCA( nick );
	JabberUtf8Decode( dispNick, NULL );

	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.pszUID = nick;
	gce.pszNick = dispNick;
	gce.bAddToLog = TRUE;
	gce.time = msgTime;
	gce.pszText = EscapeChatTags( msgText );
	gce.bIsMe = lstrcmpA( nick, item->nick ) == 0;
	JCallService(MS_GC_EVENT, NULL, (LPARAM)&gce);

	item->bChatActive = 2;

	if ( gcd.iType == GC_EVENT_TOPIC ) {
		gce.bAddToLog = FALSE;
		gcd.iType = GC_EVENT_SETSBTEXT;
		JCallService(MS_GC_EVENT, NULL, (LPARAM)&gce);
	}

	free( msgText );
	free( (void*)gce.pszText ); // Since we processed msgText and created a new string
}

typedef struct {
	char* roomJid;
	char* from;
	char* reason;
	char* password;
} JABBER_GROUPCHAT_INVITE_INFO;

static BOOL CALLBACK JabberGroupchatInviteAcceptDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		{
			JABBER_GROUPCHAT_INVITE_INFO *inviteInfo = ( JABBER_GROUPCHAT_INVITE_INFO * ) lParam;
			char* nick, *str;

			TranslateDialogDefault( hwndDlg );
			SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) inviteInfo );
			SetDlgItemTextA( hwndDlg, IDC_FROM, inviteInfo->from );
			SetDlgItemTextA( hwndDlg, IDC_ROOM, inviteInfo->roomJid );

			if (( str=JabberTextDecode( inviteInfo->reason )) != NULL ) {
				SetDlgItemTextA( hwndDlg, IDC_REASON, str );
				free( str );
			}
			nick = JabberNickFromJID( jabberJID );
			SetDlgItemTextA( hwndDlg, IDC_NICK, nick );
			free( nick );
			SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )iconBigList[0] );
		}
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_ACCEPT:
			{
				JABBER_GROUPCHAT_INVITE_INFO *inviteInfo;
				char roomJid[256], text[128];
				char* server, *room;

				inviteInfo = ( JABBER_GROUPCHAT_INVITE_INFO * ) GetWindowLong( hwndDlg, GWL_USERDATA );
				strncpy( roomJid, inviteInfo->roomJid, sizeof( roomJid ));
				room = strtok( roomJid, "@" );
				server = strtok( NULL, "@" );
				GetDlgItemTextA( hwndDlg, IDC_NICK, text, sizeof( text ));
				JabberGroupchatJoinRoom( server, room, TXT(text), inviteInfo->password );
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
	free( inviteInfo->roomJid );
	free( inviteInfo->from );
	free( inviteInfo->reason );
	free( inviteInfo->password );
	free( inviteInfo );
}

void JabberGroupchatProcessInvite( char* roomJid, char* from, char* reason, char* password )
{
	JABBER_GROUPCHAT_INVITE_INFO *inviteInfo;

	if ( roomJid == NULL )
		return;

	inviteInfo = ( JABBER_GROUPCHAT_INVITE_INFO * ) malloc( sizeof( JABBER_GROUPCHAT_INVITE_INFO ));
	inviteInfo->roomJid = _strdup( roomJid );
	inviteInfo->from = ( from!=NULL )?_strdup( from ):NULL;
	inviteInfo->reason = ( reason!=NULL )?_strdup( reason ):NULL;
	inviteInfo->password = ( password!=NULL )?_strdup( password ):NULL;
	JabberForkThread(( JABBER_THREAD_FUNC )JabberGroupchatInviteAcceptThread, 0, ( void * ) inviteInfo );
}
