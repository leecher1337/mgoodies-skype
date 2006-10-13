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
#include "jabber_iq.h"
#include "resource.h"

extern HANDLE hInitChat;

/////////////////////////////////////////////////////////////////////////////////////////
// One string entry dialog

struct JabberEnterStringParams
{	TCHAR* result;
	size_t resultLen;
};

BOOL CALLBACK JabberEnterStringDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault( hwndDlg );
		JabberEnterStringParams* params = ( JabberEnterStringParams* )lParam;
		SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG )params );
		SetWindowText( hwndDlg, params->result );
		return TRUE;
	}

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
		{	JabberEnterStringParams* params = ( JabberEnterStringParams* )GetWindowLong( hwndDlg, GWL_USERDATA );
			GetDlgItemText( hwndDlg, IDC_TOPIC, params->result, params->resultLen );
			params->result[ params->resultLen-1 ] = 0;
			EndDialog( hwndDlg, 1 );
			break;
		}
		case IDCANCEL:
			EndDialog( hwndDlg, 0 );
			break;
	}	}

	return FALSE;
}

BOOL JabberEnterString( TCHAR* result, size_t resultLen )
{
	JabberEnterStringParams params = { result, resultLen };
	return DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_GROUPCHAT_INPUT ), NULL, JabberEnterStringDlgProc, LPARAM( &params ));
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGcInit - initializes the new chat

static TCHAR* sttRoles[] = { _T("Other"), _T("Visitors"), _T("Participants"), _T("Moderators") };

int JabberGcInit( WPARAM wParam, LPARAM lParam )
{
	JABBER_LIST_ITEM* item = ( JABBER_LIST_ITEM* )wParam;
	GCSESSION gcw = {0};
	GCEVENT gce = {0};

	TCHAR* szNick = JabberNickFromJID( item->jid );
	gcw.cbSize = sizeof(GCSESSION);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = jabberProtoName;
	gcw.ptszName = szNick;
	gcw.ptszID = item->jid;
	gcw.dwFlags = GC_TCHAR;
	JCallService( MS_GC_NEWSESSION, NULL, (LPARAM)&gcw );

	HANDLE hContact = JabberHContactFromJID( item->jid );
	if ( hContact != NULL ) {
		DBVARIANT dbv;
		if ( !DBGetContactSettingTString( hContact, jabberProtoName, "MyNick", &dbv )) {
			if ( !lstrcmp( dbv.ptszVal, szNick ))
				JDeleteSetting( hContact, "MyNick" );
			else
				JSetStringT( hContact, "MyNick", item->nick );
			JFreeVariant( &dbv );
		}
		else JSetStringT( hContact, "MyNick", item->nick );
	}
	mir_free( szNick );

	item->bChatActive = TRUE;

	GCDEST gcd = { jabberProtoName, NULL, GC_EVENT_ADDGROUP };
	gcd.ptszID = item->jid;
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.dwFlags = GC_TCHAR;
	for ( int i = SIZEOF(sttRoles)-1; i >= 0; i-- ) {
		gce.ptszStatus = TranslateTS( sttRoles[i] );
		JCallService(MS_GC_EVENT, NULL, ( LPARAM )&gce );
	}

	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gcd.iType = GC_EVENT_CONTROL;
	JCallService(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	JCallService(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
	return 0;
}

void JabberGcLogCreate( JABBER_LIST_ITEM* item )
{
	if ( item->bChatActive )
		return;

	NotifyEventHooks( hInitChat, (WPARAM)item, 0 );
}

void JabberGcLogUpdateMemberStatus( JABBER_LIST_ITEM* item, TCHAR* nick, int action, XmlNode* reason )
{
	int statusToSet = 0;
	TCHAR* szReason = NULL;
	if ( reason != NULL && reason->text != NULL )
		szReason = reason->text;

	TCHAR* myNick = (item->nick == NULL) ? NULL : mir_tstrdup( item->nick );
	if ( myNick == NULL )
		myNick = JabberNickFromJID( jabberJID );

	GCDEST gcd = { jabberProtoName, 0, 0 };
	gcd.ptszID = item->jid;
	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.ptszNick = nick;
	gce.ptszUID = nick;
	gce.ptszText = szReason;
	gce.dwFlags = GC_TCHAR;
	gce.pDest = &gcd;
 	if ( item->bChatActive == 2 ) {
		gce.dwFlags |= GCEF_ADDTOLOG;
		gce.time = time(0);
	}

	switch( gcd.iType = action ) {
	case GC_EVENT_PART:  break;
	case GC_EVENT_KICK:  gce.ptszStatus = TranslateT( "Moderator" );  break;
	default:
		for ( int i=0; i < item->resourceCount; i++ ) {
			JABBER_RESOURCE_STATUS& JS = item->resource[i];
			if ( !lstrcmp( nick, JS.resourceName )) {
				if ( action != GC_EVENT_JOIN ) {
					switch( action ) {
					case 0:
						gcd.iType = GC_EVENT_ADDSTATUS;
					case GC_EVENT_REMOVESTATUS:
						gce.dwFlags &= ~GCEF_ADDTOLOG;
					}
					gce.ptszText = TranslateT( "Moderator" );
				}
				gce.ptszStatus = TranslateTS( sttRoles[ JS.role ] );
				gce.bIsMe = ( lstrcmp( nick, myNick ) == 0 );
				statusToSet = JS.status;
				break;
	}	}	}

	JCallService( MS_GC_EVENT, NULL, ( LPARAM )&gce );

	if ( statusToSet != 0 ) {
		gce.ptszText = nick;
		if ( statusToSet == ID_STATUS_AWAY || statusToSet == ID_STATUS_NA || statusToSet == ID_STATUS_DND )
			gce.dwItemData = 3;
		else
			gce.dwItemData = 1;
		gcd.iType = GC_EVENT_SETSTATUSEX;
		JCallService( MS_GC_EVENT, NULL, ( LPARAM )&gce );
	}

	mir_free( myNick );
}

void JabberGcQuit( JABBER_LIST_ITEM* item, int code, XmlNode* reason )
{
	TCHAR* szReason = NULL;
	if ( reason != NULL && reason->text != NULL )
		szReason = reason->text;

	GCDEST gcd = { jabberProtoName, NULL, GC_EVENT_CONTROL };
	gcd.ptszID = item->jid;
	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.ptszUID = item->jid;
	gce.ptszText = szReason;
	gce.dwFlags = GC_TCHAR;
	gce.pDest = &gcd;

	if ( code != 307 ) {
		JCallService( MS_GC_EVENT, SESSION_TERMINATE, ( LPARAM )&gce );
		JCallService( MS_GC_EVENT, WINDOW_CLEARLOG, ( LPARAM )&gce );
	}
	else {
		TCHAR* myNick = JabberNickFromJID( jabberJID );
		JabberGcLogUpdateMemberStatus( item, myNick, GC_EVENT_KICK, reason );
		mir_free( myNick );
		JCallService( MS_GC_EVENT, SESSION_OFFLINE, ( LPARAM )&gce );
	}

	DBDeleteContactSetting( JabberHContactFromJID( item->jid ), "CList", "Hidden" );
	item->bChatActive = FALSE;

	if ( jabberOnline ) {
		TCHAR text[ 1024 ];
		mir_sntprintf( text, SIZEOF( text ), _T("%s/%s"), item->jid, item->nick );
		XmlNode p( "presence" ); p.addAttr( "to", text ); p.addAttr( "type", "unavailable" );
		JabberSend( jabberThreadInfo->s, p );
		JabberListRemove( LIST_CHATROOM, item->jid );
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu hooks

#define IDM_LEAVE       10
#define IDM_TOPIC       12

int JabberGcMenuHook( WPARAM wParam, LPARAM lParam )
{
	GCMENUITEMS* gcmi= ( GCMENUITEMS* )lParam;
	if ( gcmi == NULL )
		return 0;

	if ( lstrcmpiA( gcmi->pszModule, jabberProtoName ))
		return 0;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, gcmi->pszID );
	if ( item == NULL )
		return 0;

	JABBER_RESOURCE_STATUS *me = NULL, *him = NULL;
	for ( int i=0; i < item->resourceCount; i++ ) {
		JABBER_RESOURCE_STATUS& p = item->resource[i];
		if ( !lstrcmp( p.resourceName, item->nick   ))  me = &p;
		if ( !lstrcmp( p.resourceName, gcmi->pszUID ))  him = &p;
	}

	if ( gcmi->Type == MENU_ON_LOG ) {
		static struct gc_item sttLogListItems[] = {
			{ TranslateT( "&Leave chat session" ),    IDM_LEAVE,     MENU_ITEM, FALSE },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "&Voice List..." ),         IDM_VOICE,     MENU_ITEM, TRUE  },
			{ TranslateT( "&Ban List..." ),           IDM_BAN,       MENU_ITEM, TRUE  },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "&Member List..." ),        IDM_MEMBER,    MENU_ITEM, TRUE  },
			{ TranslateT( "Mo&derator List..." ),     IDM_MODERATOR, MENU_ITEM, TRUE  },
			{ TranslateT( "&Admin List..." ),         IDM_ADMIN,     MENU_ITEM, TRUE  },
			{ TranslateT( "&Owner List..." ),         IDM_OWNER,     MENU_ITEM, TRUE  },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "Change &Nickname..." ),    IDM_NICK,      MENU_ITEM, FALSE },
			{ TranslateT( "Set &Topic..." ),          IDM_TOPIC,     MENU_ITEM, FALSE },
			{ TranslateT( "&Invite a User..." ),      IDM_INVITE,    MENU_ITEM, FALSE },
			{ TranslateT( "Room Con&figuration..." ), IDM_CONFIG,    MENU_ITEM, TRUE  },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "Destroy Room..." ),        IDM_DESTROY,   MENU_ITEM, TRUE  }};

		gcmi->nItems = sizeof( sttLogListItems ) / sizeof( sttLogListItems[0] );
		gcmi->Item = sttLogListItems;

		if ( me != NULL ) {
			if ( me->role == ROLE_MODERATOR )
				sttLogListItems[2].bDisabled = FALSE;

			if ( me->affiliation == AFFILIATION_ADMIN )
				sttLogListItems[3].bDisabled = sttLogListItems[5].bDisabled = sttLogListItems[6].bDisabled = FALSE;
			else if ( me->affiliation == AFFILIATION_OWNER )
				sttLogListItems[3].bDisabled = sttLogListItems[5].bDisabled =
				sttLogListItems[6].bDisabled = sttLogListItems[7].bDisabled =
				sttLogListItems[8].bDisabled = sttLogListItems[13].bDisabled =
				sttLogListItems[15].bDisabled = FALSE;
		}
	}
	else if ( gcmi->Type == MENU_ON_NICKLIST ) {
		static struct gc_item sttListItems[] = {
			{ TranslateT( "&Leave chat session" ),    IDM_LEAVE,     MENU_ITEM, FALSE },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "Kick" ),                   IDM_KICK,      MENU_ITEM, TRUE },
			{ TranslateT( "Ban" ),                    IDM_BAN,       MENU_ITEM, TRUE },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "Toggle &Voice" ),          IDM_VOICE,     MENU_ITEM, TRUE },
			{ TranslateT( "Toggle Moderator" ),       IDM_MODERATOR, MENU_ITEM, TRUE },
			{ TranslateT( "Toggle Admin" ),           IDM_ADMIN,     MENU_ITEM, TRUE },
			{ TranslateT( "Toggle Owner" ),           IDM_OWNER,     MENU_ITEM, TRUE }};

		gcmi->nItems = sizeof( sttListItems )/sizeof( sttListItems[0] );
		gcmi->Item = sttListItems;

		if ( me != NULL && him != NULL ) {
			if ( me->role == ROLE_MODERATOR )
				if ( him->affiliation != AFFILIATION_ADMIN && him->affiliation != AFFILIATION_OWNER )
					sttListItems[2].bDisabled = sttListItems[3].bDisabled = FALSE;

			if ( me->affiliation == AFFILIATION_ADMIN ) {
				if ( him->affiliation != AFFILIATION_ADMIN && him->affiliation != AFFILIATION_OWNER )
					sttListItems[5].bDisabled = sttListItems[6].bDisabled = FALSE;
			}
			else if ( me->affiliation == AFFILIATION_OWNER )
				sttListItems[5].bDisabled = sttListItems[6].bDisabled =
				sttListItems[7].bDisabled = sttListItems[8].bDisabled = FALSE;
	}	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Conference invitation dialog

static BOOL CALLBACK JabberGcLogInviteDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault( hwndDlg );
			SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )iconBigList[0]);
			SetDlgItemText( hwndDlg, IDC_ROOM, ( TCHAR* )lParam );
			HWND hwndComboBox = GetDlgItem( hwndDlg, IDC_USER );
			int index = 0;
			while (( index=JabberListFindNext( LIST_ROSTER, index )) >= 0 ) {
				JABBER_LIST_ITEM* item = JabberListGetItemPtrFromIndex( index );
				if ( item->status != ID_STATUS_OFFLINE ) {
					// Add every non-offline users to the combobox
					int n = SendMessage( hwndComboBox, CB_ADDSTRING, 0, ( LPARAM )item->jid );
					SendMessage( hwndComboBox, CB_SETITEMDATA, n, ( LPARAM )item->jid );
				}
				index++;
			}
			SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) mir_tstrdup(( TCHAR* )lParam ));
		}
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_INVITE:
			{
				TCHAR* room = ( TCHAR* )GetWindowLong( hwndDlg, GWL_USERDATA );
				if ( room != NULL ) {
					TCHAR text[256], user[256], *pUser;
					HWND hwndComboBox = GetDlgItem( hwndDlg, IDC_USER );
					int n = SendMessage( hwndComboBox, CB_GETCURSEL, 0, 0 );
					if ( n < 0 ) {
						GetWindowText( hwndComboBox, user, SIZEOF( user ));
						pUser = user;
					}
					else pUser = ( TCHAR* )SendMessage( hwndComboBox, CB_GETITEMDATA, n, 0 );

					if ( pUser != NULL ) {
						GetDlgItemText( hwndDlg, IDC_REASON, text, SIZEOF( text ));
						int iqId = JabberSerialNext();

						XmlNode m( "message" ); m.addAttr( "from", jabberJID ); m.addAttr( "to", room ); m.addAttrID( iqId );
						XmlNode* x = m.addChild( "x" ); x->addAttr( "xmlns", _T("http://jabber.org/protocol/muc#user"));
						XmlNode* i = x->addChild( "invite" ); i->addAttr( "to", pUser ); 
						if ( text[0] != 0 )
							i->addChild( "reason", text );
						JabberSend( jabberThreadInfo->s, m );
			}	}	}
			// Fall through
		case IDCANCEL:
		case IDCLOSE:
			DestroyWindow( hwndDlg );
			return TRUE;
		}
		break;

	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;

	case WM_DESTROY:
		mir_free(( TCHAR* )GetWindowLong( hwndDlg, GWL_USERDATA ));
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu processing

static void JabberAdminSet( const TCHAR* to, const char* ns, const char* szItem, const TCHAR* itemVal, const char* var, const TCHAR* varVal )
{
	XmlNodeIq iq( "set", NOID, to );
	XmlNode* query = iq.addQuery( ns );
	XmlNode* item = query->addChild( "item" ); item->addAttr( szItem, itemVal ); item->addAttr( var, varVal );
	JabberSend( jabberThreadInfo->s, iq );
}

static void JabberAdminGet( const TCHAR* to, const char* ns, const char* var, const TCHAR* varVal, JABBER_IQ_PFUNC foo )
{
	int id = JabberSerialNext();
	JabberIqAdd( id, IQ_PROC_NONE, foo );

	XmlNodeIq iq( "get", id, to );
	XmlNode* query = iq.addQuery( ns );
	XmlNode* item = query->addChild( "item" ); item->addAttr( var, varVal );
	JabberSend( jabberThreadInfo->s, iq );
}

static void sttNickListHook( JABBER_LIST_ITEM* item, GCHOOK* gch )
{
	JABBER_RESOURCE_STATUS *me = NULL, *him = NULL;
	for ( int i=0; i < item->resourceCount; i++ ) {
		JABBER_RESOURCE_STATUS& p = item->resource[i];
		if ( !lstrcmp( p.resourceName, item->nick  )) me = &p;
		if ( !lstrcmp( p.resourceName, gch->ptszUID )) him = &p;
	}

	if ( him == NULL || me == NULL )
		return;

	TCHAR szBuffer[ 1024 ];

	switch( gch->dwData ) {
	case IDM_LEAVE:
		JabberGcQuit( item, 0, 0 );
		break;

	case IDM_KICK:
	{
		mir_sntprintf( szBuffer, SIZEOF(szBuffer), _T("%s %s"), TranslateT( "Reason to kick" ), him->resourceName );
		if ( JabberEnterString( szBuffer, SIZEOF(szBuffer))) {
			XmlNodeIq iq( "set", NOID, item->jid );
			XmlNode* query = iq.addQuery( xmlnsAdmin );
			XmlNode* item = query->addChild( "item" ); item->addAttr( "nick", him->resourceName ); item->addAttr( "role", "none" );
			item->addChild( "reason", szBuffer );
			JabberSend( jabberThreadInfo->s, iq );
		}
		break;
	}

	case IDM_BAN:
		mir_sntprintf( szBuffer, SIZEOF(szBuffer), _T("%s %s"), TranslateT( "Reason to ban" ), him->resourceName );
		if ( JabberEnterString( szBuffer, SIZEOF(szBuffer))) {
			XmlNodeIq iq( "set", NOID, item->jid );
			XmlNode* query = iq.addQuery( xmlnsAdmin );
			XmlNode* item = query->addChild( "item" ); item->addAttr( "nick", him->resourceName ); item->addAttr( "affiliation", "outcast" );
			item->addChild( "reason", szBuffer );
			JabberSend( jabberThreadInfo->s, iq );
		}
		break;

	case IDM_VOICE:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", him->resourceName,
			"role", ( him->role == ROLE_PARTICIPANT ) ? _T("visitor") : _T("participant"));
		break;

	case IDM_MODERATOR:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", him->resourceName,
			"role", ( him->role == ROLE_MODERATOR ) ? _T("participant") : _T("moderator"));
		break;

	case IDM_ADMIN:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", him->resourceName,
			"affiliation", ( him->affiliation==AFFILIATION_ADMIN )? _T("member") : _T("admin"));
		break;

	case IDM_OWNER:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", him->resourceName,
			"affiliation", ( him->affiliation==AFFILIATION_OWNER ) ? _T("admin") : _T("owner"));
		break;
}	}

static void sttLogListHook( JABBER_LIST_ITEM* item, GCHOOK* gch )
{
	TCHAR szBuffer[ 1024 ];

	switch( gch->dwData ) {
	case IDM_VOICE:
		JabberAdminGet( gch->pDest->ptszID, xmlnsAdmin, "role", _T("participant"), JabberIqResultMucGetVoiceList );
		break;

	case IDM_MEMBER:
		JabberAdminGet( gch->pDest->ptszID, xmlnsAdmin, "affiliation", _T("member"), JabberIqResultMucGetMemberList );
		break;

	case IDM_MODERATOR:
		JabberAdminGet( gch->pDest->ptszID, xmlnsAdmin, "role", _T("moderator"), JabberIqResultMucGetModeratorList );
		break;

	case IDM_BAN:
		JabberAdminGet( gch->pDest->ptszID, xmlnsAdmin, "affiliation", _T("outcast"), JabberIqResultMucGetBanList );
		break;

	case IDM_ADMIN:
		JabberAdminGet( gch->pDest->ptszID, xmlnsOwner, "affiliation", _T("admin"), JabberIqResultMucGetAdminList );
		break;

	case IDM_OWNER:
		JabberAdminGet( gch->pDest->ptszID, xmlnsOwner, "affiliation", _T("owner"), JabberIqResultMucGetOwnerList );
		break;

	case IDM_TOPIC:
		mir_sntprintf( szBuffer, SIZEOF(szBuffer), _T("%s %s"), TranslateT( "Set topic for" ), gch->pDest->ptszID );
		if ( JabberEnterString( szBuffer, SIZEOF(szBuffer))) {
			XmlNode msg( "message" ); msg.addAttr( "to", gch->pDest->ptszID ); msg.addAttr( "type", "groupchat" );
			msg.addChild( "subject", szBuffer );
			JabberSend( jabberThreadInfo->s, msg );
		}
		break;

	case IDM_NICK:
		mir_sntprintf( szBuffer, SIZEOF(szBuffer), _T("%s %s"), TranslateT( "Change nickname in" ), gch->pDest->ptszID );
		if ( JabberEnterString( szBuffer, SIZEOF(szBuffer))) {
			JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, gch->pDest->ptszID );
			if ( item != NULL ) {
				TCHAR text[ 1024 ];
				mir_sntprintf( text, SIZEOF( text ), _T("%s/%s"), gch->pDest->ptszID, szBuffer );
				JabberSendPresenceTo( jabberStatus, text, NULL );
		}	}
		break;

	case IDM_INVITE:
		CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_GROUPCHAT_INVITE ), NULL, JabberGcLogInviteDlgProc, ( LPARAM )gch->pDest->pszID );
		break;

	case IDM_CONFIG:
	{
		int iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetMuc );

		XmlNodeIq iq( "get", iqId, gch->pDest->ptszID );
		XmlNode* query = iq.addQuery( xmlnsOwner );
		JabberSend( jabberThreadInfo->s, iq );
		break;
	}
	case IDM_DESTROY:
		mir_sntprintf( szBuffer, SIZEOF(szBuffer), _T("%s %s"), TranslateT( "Reason to destroy" ), gch->pDest->ptszID );
		if ( !JabberEnterString( szBuffer, SIZEOF(szBuffer)))
			break;

		{	XmlNodeIq iq( "set", NOID, gch->pDest->ptszID );
			XmlNode* query = iq.addQuery( xmlnsOwner );
			query->addChild( "destroy" )->addChild( "reason", szBuffer );
			JabberSend( jabberThreadInfo->s, iq );
		}

	case IDM_LEAVE:
		JabberGcQuit( item, 0, 0 );
		break;
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// Sends a private message to a chat user

static void sttSendPrivateMessage( JABBER_LIST_ITEM* item, const TCHAR* nick )
{
	TCHAR szFullJid[ 256 ];
	mir_sntprintf( szFullJid, SIZEOF(szFullJid), _T("%s/%s"), item->jid, nick );
	HANDLE hContact = JabberDBCreateContact( szFullJid, NULL, TRUE, FALSE );
	if ( hContact != NULL ) {
		for ( int i=0; i < item->resourceCount; i++ ) {
			if ( _tcsicmp( item->resource[i].resourceName, nick ) == 0 ) {
				JSetWord( hContact, "Status", item->resource[i].status );
				break;
		}	}

		DBWriteContactSettingByte( hContact, "CList", "Hidden", 1 );
		JSetStringT( hContact, "Nick", nick );
		DBWriteContactSettingDword( hContact, "Ignore", "Mask1", 0 );
		JCallService( MS_MSG_SENDMESSAGE, ( WPARAM )hContact, 0 );
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// General chat event processing hook

int JabberGcEventHook(WPARAM wParam,LPARAM lParam)
{
	GCHOOK* gch = ( GCHOOK* )lParam;
	if ( gch == NULL )
		return 0;

	if ( lstrcmpiA( gch->pDest->pszModule, jabberProtoName ))
		return 0;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, gch->pDest->ptszID );
	if ( item == NULL )
		return 0;

	switch ( gch->pDest->iType ) {
	case GC_USER_MESSAGE:
		if ( gch->pszText && lstrlen( gch->ptszText) > 0 ) {
			rtrim( gch->ptszText );

			if ( jabberOnline ) {
				XmlNode m( "message" ); m.addAttr( "to", item->jid ); m.addAttr( "type", "groupchat" );
				XmlNode* b = m.addChild( "body", gch->ptszText );
				if ( b->sendText != NULL )
					UnEscapeChatTags( b->sendText );
				JabberSend( jabberThreadInfo->s, m );
		}	}
		break;

	case GC_USER_PRIVMESS:
		sttSendPrivateMessage( item, gch->ptszUID );
		break;

	case GC_USER_LOGMENU:
		sttLogListHook( item, gch );
		break;

	case GC_USER_NICKLISTMENU:
		sttNickListHook( item, gch );
		break;

	case GC_USER_CHANMGR:
		int iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetMuc );

		XmlNodeIq iq( "get", iqId, item->jid );
		XmlNode* query = iq.addQuery( xmlnsOwner );
		JabberSend( jabberThreadInfo->s, iq );
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void JabberAddMucListItem( JABBER_MUC_JIDLIST_INFO* jidListInfo, TCHAR* str )
{
	const char* field = _tcschr(str,'@') ? "jid" : "nick";
	TCHAR* roomJid = jidListInfo->roomJid;

	switch (jidListInfo->type) {
	case MUC_VOICELIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "role", _T("participant"));
		JabberAdminGet( roomJid, xmlnsAdmin, "role", _T("participant"), JabberIqResultMucGetVoiceList);
		break;
	case MUC_MEMBERLIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "affiliation", _T("member"));
		JabberAdminGet( roomJid, xmlnsAdmin, "affiliation", _T("member"), JabberIqResultMucGetMemberList);
		break;
	case MUC_MODERATORLIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "role", _T("moderator"));
		JabberAdminGet( roomJid, xmlnsAdmin, "role", _T("moderator"), JabberIqResultMucGetModeratorList);
		break;
	case MUC_BANLIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "affiliation", _T("outcast"));
		JabberAdminGet( roomJid, xmlnsAdmin, "affiliation", _T("outcast"), JabberIqResultMucGetBanList);
		break;
	case MUC_ADMINLIST:
		JabberAdminSet( roomJid, xmlnsOwner, field, str, "affiliation", _T("admin"));
		JabberAdminGet( roomJid, xmlnsOwner, "affiliation", _T("admin"), JabberIqResultMucGetAdminList);
		break;
	case MUC_OWNERLIST:
		JabberAdminSet( roomJid, xmlnsOwner, field, str, "affiliation", _T("owner"));
		JabberAdminGet( roomJid, xmlnsOwner, "affiliation", _T("owner"), JabberIqResultMucGetOwnerList);
		break;
}	}

void JabberDeleteMucListItem( JABBER_MUC_JIDLIST_INFO* jidListInfo, TCHAR* jid )
{
	TCHAR* roomJid = jidListInfo->roomJid;

	switch ( jidListInfo->type ) {
	case MUC_VOICELIST:		// change role to visitor ( from participant )
		JabberAdminSet( roomJid, xmlnsAdmin, "jid", jid, "role", _T("visitor"));
		break;
	case MUC_BANLIST:		// change affiliation to none ( from outcast )
	case MUC_MEMBERLIST:	// change affiliation to none ( from member )
		JabberAdminSet( roomJid, xmlnsAdmin, "jid", jid, "affiliation", _T("none"));
		break;
	case MUC_MODERATORLIST:	// change role to participant ( from moderator )
		JabberAdminSet( roomJid, xmlnsAdmin, "jid", jid, "role", _T("participant"));
		break;
	case MUC_ADMINLIST:		// change affiliation to member ( from admin )
		JabberAdminSet( roomJid, xmlnsOwner, "jid", jid, "affiliation", _T("member"));
		break;
	case MUC_OWNERLIST:		// change affiliation to admin ( from owner )
		JabberAdminSet( roomJid, xmlnsOwner, "jid", jid, "affiliation", _T("admin"));
		break;
}	}
