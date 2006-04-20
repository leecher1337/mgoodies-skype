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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_chat.cpp,v $
Revision       : $Revision: 1.31 $
Last change on : $Date: 2006/02/27 21:56:07 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_iq.h"
#include "resource.h"

extern HANDLE hInitChat;

/////////////////////////////////////////////////////////////////////////////////////////
// One string entry dialog

struct JabberEnterStringParams
{	char* result;
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
		SetWindowTextA( hwndDlg, params->result );
		return TRUE;
	}

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
		{	JabberEnterStringParams* params = ( JabberEnterStringParams* )GetWindowLong( hwndDlg, GWL_USERDATA );
			GetDlgItemTextA( hwndDlg, IDC_TOPIC, params->result, params->resultLen );
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

BOOL JabberEnterString( char* result, size_t resultLen )
{
	JabberEnterStringParams params = { result, resultLen };
	return DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_GROUPCHAT_INPUT ), NULL, JabberEnterStringDlgProc, LPARAM( &params ));
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGcInit - initializes the new chat

static char* sttRoles[] = { "Other", "Visitors", "Participants", "Moderators" };

int JabberGcInit( WPARAM wParam, LPARAM lParam )
{
	JABBER_LIST_ITEM* item = ( JABBER_LIST_ITEM* )wParam;
	GCSESSION gcw = {0};
	GCEVENT gce = {0};

	char* szNick = JabberNickFromJID( item->jid );
	gcw.cbSize = sizeof(GCSESSION);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = jabberProtoName;
	gcw.pszName = szNick;
	gcw.pszID = item->jid;
	gcw.pszStatusbarText = NULL;
	gcw.bDisableNickList = FALSE;
	JCallService(MS_GC_NEWSESSION, NULL, (LPARAM)&gcw);

	HANDLE hContact = JabberHContactFromJID( item->jid );
	if ( hContact != NULL ) {
		DBVARIANT dbv;
		if ( !JGetStringUtf( hContact, "MyNick", &dbv )) {
			if ( !strcmp( dbv.pszVal, szNick ))
				JDeleteSetting( hContact, "MyNick" );
			else
				JSetStringUtf( hContact, "MyNick", item->nick );
			JFreeVariant( &dbv );
		}
		else JSetStringUtf( hContact, "MyNick", item->nick );
	}
	free( szNick );

	item->bChatActive = TRUE;

	GCDEST gcd = { jabberProtoName, item->jid, GC_EVENT_ADDGROUP };
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	for ( int i=sizeof(sttRoles)/sizeof(char*)-1; i >= 0; i-- ) {
		gce.pszStatus = Translate( sttRoles[i] );
		JCallService(MS_GC_EVENT, NULL, ( LPARAM )&gce );
	}

	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gcd.iType = GC_EVENT_CONTROL;
	JCallService(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	JCallService(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
	JCallService(MS_GC_EVENT, WINDOW_VISIBLE, (LPARAM)&gce);
	return 0;
}

void JabberGcLogCreate( JABBER_LIST_ITEM* item )
{
	if ( item->bChatActive )
		return;

	NotifyEventHooks( hInitChat, (WPARAM)item, 0 );
}

void JabberGcLogUpdateMemberStatus( JABBER_LIST_ITEM* item, char* nick, int action, XmlNode* reason )
{
	char* dispNick = NEWSTR_ALLOCA( nick );
	JabberUtf8Decode( dispNick, NULL );

	char* szReason = NULL;
	if ( reason != NULL && reason->text != NULL ) {
		szReason = NEWSTR_ALLOCA( reason->text );
		JabberUtf8Decode( szReason, NULL );
	}

	char* myNick = (item->nick == NULL) ? NULL : _strdup( item->nick );
	if ( myNick == NULL )
		myNick = JabberNickFromJID( jabberJID );

	GCDEST gcd = { jabberProtoName, item->jid, 0 };
	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.pszNick = dispNick;
	gce.pszUID = nick;
	gce.pDest = &gcd;
	gce.pszText = szReason;
	if ( item->bChatActive == 2 ) {
		gce.bAddToLog = TRUE;
		gce.time = time(0);
	}

	switch( gcd.iType = action ) {
	case GC_EVENT_PART:  break;
	case GC_EVENT_KICK:  gce.pszStatus = Translate( "Moderator" );  break;
	default:
		for ( int i=0; i < item->resourceCount; i++ ) {
			JABBER_RESOURCE_STATUS& JS = item->resource[i];
			if ( !strcmp( nick, JS.resourceName )) {
				if ( action != GC_EVENT_JOIN ) {
					switch( action ) {
					case 0:
						gcd.iType = GC_EVENT_ADDSTATUS;
					case GC_EVENT_REMOVESTATUS:
						gce.bAddToLog = false;
					}
					gce.pszText = Translate( "Moderator" );
				}
				gce.pszStatus = JTranslate( sttRoles[ JS.role ] );
				gce.bIsMe = ( lstrcmpA( myNick, nick ) == 0 );
				break;
	}	}	}

	JCallService( MS_GC_EVENT, NULL, ( LPARAM )&gce );
	free( myNick );
}

void JabberGcQuit( JABBER_LIST_ITEM* item, int code, XmlNode* reason )
{
	GCDEST gcd = { jabberProtoName, item->jid, GC_EVENT_CONTROL };
	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.pszUID = item->jid;
	gce.pDest = &gcd;
	gce.pszText = ( reason != NULL ) ? reason->text : NULL;
	if ( code != 307 ) {
		JCallService( MS_GC_EVENT, SESSION_TERMINATE, ( LPARAM )&gce );
		JCallService( MS_GC_EVENT, WINDOW_CLEARLOG, ( LPARAM )&gce );
	}
	else {
		char* myNick = JabberNickFromJID( jabberJID );
		JabberGcLogUpdateMemberStatus( item, myNick, GC_EVENT_KICK, reason );
		free( myNick );
		JCallService( MS_GC_EVENT, SESSION_OFFLINE, ( LPARAM )&gce );
	}

	DBDeleteContactSetting( JabberHContactFromJID( item->jid ), "CList", "Hidden" );
	item->bChatActive = FALSE;

	if ( jabberOnline ) {
        JabberSend( jabberThreadInfo->s, "<presence to=\"%s\" type=\"unavailable\"/>", item->jid );
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
		if ( !lstrcmpA( p.resourceName, item->nick   ))  me = &p;
		if ( !lstrcmpA( p.resourceName, gcmi->pszUID ))  him = &p;
	}

	if ( gcmi->Type == MENU_ON_LOG ) {
		static struct gc_item sttLogListItems[] = {
			{ JTranslate( "&Leave chat session" ),    IDM_LEAVE,     MENU_ITEM, FALSE },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ JTranslate( "&Voice List..." ),         IDM_VOICE,     MENU_ITEM, TRUE  },
			{ JTranslate( "&Ban List..." ),           IDM_BAN,       MENU_ITEM, TRUE  },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ JTranslate( "&Member List..." ),        IDM_MEMBER,    MENU_ITEM, TRUE  },
			{ JTranslate( "Mo&derator List..." ),     IDM_MODERATOR, MENU_ITEM, TRUE  },
			{ JTranslate( "&Admin List..." ),         IDM_ADMIN,     MENU_ITEM, TRUE  },
			{ JTranslate( "&Owner List..." ),         IDM_OWNER,     MENU_ITEM, TRUE  },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ JTranslate( "Change &Nickname..." ),    IDM_NICK,      MENU_ITEM, FALSE },
			{ JTranslate( "Set &Topic..." ),          IDM_TOPIC,     MENU_ITEM, FALSE },
			{ JTranslate( "&Invite a User..." ),      IDM_INVITE,    MENU_ITEM, FALSE },
			{ JTranslate( "Room Con&figuration..." ), IDM_CONFIG,    MENU_ITEM, TRUE  },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ JTranslate( "Destroy Room..." ),        IDM_DESTROY,   MENU_ITEM, TRUE  }};

		gcmi->nItems = sizeof( sttLogListItems ) / sizeof( sttLogListItems[0] );
		gcmi->Item = sttLogListItems;

		if ( me != NULL ) {
			if ( me->role == ROLE_MODERATOR )
				sttLogListItems[2].bDisabled = FALSE;

			if ( me->affiliation == AFFILIATION_ADMIN )
				sttLogListItems[3].bDisabled = sttLogListItems[5].bDisabled = FALSE;
			else if ( me->affiliation == AFFILIATION_OWNER )
				sttLogListItems[3].bDisabled = sttLogListItems[5].bDisabled =
				sttLogListItems[6].bDisabled = sttLogListItems[7].bDisabled =
				sttLogListItems[8].bDisabled = sttLogListItems[13].bDisabled =
				sttLogListItems[15].bDisabled = FALSE;
		}
	}
	else if ( gcmi->Type == MENU_ON_NICKLIST ) {
		static struct gc_item sttListItems[] = {
			{ JTranslate( "&Leave chat session" ), IDM_LEAVE, MENU_ITEM, FALSE },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ JTranslate( "Kick" ), IDM_KICK, MENU_ITEM, TRUE },
			{ JTranslate( "Ban" ),  IDM_BAN,  MENU_ITEM, TRUE },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ JTranslate( "Toggle &Voice" ),     IDM_VOICE,      MENU_ITEM, TRUE },
			{ JTranslate( "Toggle Moderator" ),  IDM_MODERATOR,  MENU_ITEM, TRUE },
			{ JTranslate( "Toggle Admin" ),      IDM_ADMIN,      MENU_ITEM, TRUE },
			{ JTranslate( "Toggle Owner" ),      IDM_OWNER,      MENU_ITEM, TRUE }};

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
			SetDlgItemTextA( hwndDlg, IDC_ROOM, ( char* )lParam );
			HWND hwndComboBox = GetDlgItem( hwndDlg, IDC_USER );
			int index = 0;
			while (( index=JabberListFindNext( LIST_ROSTER, index )) >= 0 ) {
				JABBER_LIST_ITEM* item = JabberListGetItemPtrFromIndex( index );
				if ( item->status != ID_STATUS_OFFLINE ) {
					// Add every non-offline users to the combobox
					int n = SendMessageA( hwndComboBox, CB_ADDSTRING, 0, ( LPARAM )item->jid );
					SendMessage( hwndComboBox, CB_SETITEMDATA, n, ( LPARAM )item->jid );
				}
				index++;
			}
			SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) _strdup(( char* )lParam ));
		}
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_INVITE:
			{
				char text[256], user[256], *pUser;
				char* room;
				HWND hwndComboBox;
				int iqId, n;

				hwndComboBox = GetDlgItem( hwndDlg, IDC_USER );
				if (( room=( char* )GetWindowLong( hwndDlg, GWL_USERDATA )) != NULL ) {
					n = SendMessage( hwndComboBox, CB_GETCURSEL, 0, 0 );
					if ( n < 0 ) {
						GetWindowTextA( hwndComboBox, user, sizeof( user ));
						pUser = user;
					}
					else pUser = ( char* )SendMessage( hwndComboBox, CB_GETITEMDATA, n, 0 );

					if ( pUser != NULL ) {
						GetDlgItemTextA( hwndDlg, IDC_REASON, text, sizeof( text ));
						iqId = JabberSerialNext();
                        JabberSend( jabberThreadInfo->s, "<message id=\""JABBER_IQID"%d\" to=\"%s\"><x xmlns=\"http://jabber.org/protocol/muc#user\"><invite to=\"%s\"><reason>%s</reason></invite></x></message>",
							iqId, TXT(room), TXT(pUser), TXT(text));
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
		{
			char* str;

			if (( str=( char* )GetWindowLong( hwndDlg, GWL_USERDATA )) != NULL )
				free( str );
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu processing

static void JabberAdminSet( const char* to, const char* ns, const char* item, const char* itemVal, const char* var, const char* varVal )
{
    JabberSend( jabberThreadInfo->s, "<iq type=\"set\" to=\"%s\">%s<item %s=\"%s\" %s=\"%s\"/></query></iq>",
		to, ns, item, itemVal, var, varVal );
}

static void JabberAdminGet( const char* to, const char* ns, const char* var, const char* varVal, JABBER_IQ_PFUNC foo )
{
	int id = JabberSerialNext();
	JabberIqAdd( id, IQ_PROC_NONE, foo );
	JabberSend( jabberThreadInfo->s,
        "<iq type=\"get\" id=\""JABBER_IQID"%d\" to=\"%s\">%s<item %s=\"%s\"/></query></iq>",
		id, to, ns, var, varVal );
}

static void sttNickListHook( JABBER_LIST_ITEM* item, GCHOOK* gch )
{
	JABBER_RESOURCE_STATUS *me = NULL, *him = NULL;
	for ( int i=0; i < item->resourceCount; i++ ) {
		JABBER_RESOURCE_STATUS& p = item->resource[i];
		if ( !lstrcmpA( p.resourceName, item->nick  )) me = &p;
		if ( !lstrcmpA( p.resourceName, gch->pszUID )) him = &p;
	}

	if ( him == NULL || me == NULL )
		return;

	char   szBuffer[ 1024 ];
	char*  dispNick = NEWSTR_ALLOCA( him->resourceName );  JabberUtf8Decode( dispNick, NULL );

	switch( gch->dwData ) {
	case IDM_LEAVE:
		JabberGcQuit( item, 0, 0 );
		break;

	case IDM_KICK:
		mir_snprintf( szBuffer, sizeof szBuffer, "%s %s", JTranslate( "Reason to kick" ), dispNick );
		if ( JabberEnterString( szBuffer, sizeof szBuffer ))
            JabberSend( jabberThreadInfo->s, "<iq type=\"set\" to=\"%s\">%s<item nick=\"%s\" role=\"none\"><reason>%s</reason></item></query></iq>",
				item->jid, xmlnsAdmin, him->resourceName, TXT(szBuffer));
		break;

	case IDM_BAN:
		mir_snprintf( szBuffer, sizeof szBuffer, "%s %s", JTranslate( "Reason to ban" ), dispNick );
		if ( JabberEnterString( szBuffer, sizeof szBuffer ))
            JabberSend( jabberThreadInfo->s, "<iq type=\"set\" to=\"%s\">%s<item nick=\"%s\" affiliation=\"outcast\"><reason>%s</reason></item></query></iq>",
				item->jid, xmlnsAdmin, him->resourceName, TXT(szBuffer));
		break;

	case IDM_VOICE:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", TXT(him->resourceName),
			"role", ( him->role == ROLE_PARTICIPANT ) ? "visitor" : "participant" );
		break;

	case IDM_MODERATOR:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", TXT(him->resourceName),
			"role", ( him->role == ROLE_MODERATOR ) ? "participant" : "moderator" );
		break;

	case IDM_ADMIN:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", TXT(him->resourceName),
			"affiliation", ( him->affiliation==AFFILIATION_ADMIN )? "member" : "admin" );
		break;

	case IDM_OWNER:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", TXT(him->resourceName),
			"affiliation", ( him->affiliation==AFFILIATION_OWNER ) ? "admin" : "owner" );
		break;
}	}

static void sttLogListHook( JABBER_LIST_ITEM* item, GCHOOK* gch )
{
	char szBuffer[ 1024 ];

	switch( gch->dwData ) {
	case IDM_VOICE:
		JabberAdminGet( gch->pDest->pszID, xmlnsAdmin, "role", "participant", JabberIqResultMucGetVoiceList );
		break;

	case IDM_MEMBER:
		JabberAdminGet( gch->pDest->pszID, xmlnsAdmin, "affiliation", "member", JabberIqResultMucGetMemberList );
		break;

	case IDM_MODERATOR:
		JabberAdminGet( gch->pDest->pszID, xmlnsAdmin, "role", "moderator", JabberIqResultMucGetModeratorList );
		break;

	case IDM_BAN:
		JabberAdminGet( gch->pDest->pszID, xmlnsAdmin, "affiliation", "outcast", JabberIqResultMucGetBanList );
		break;

	case IDM_ADMIN:
		JabberAdminGet( gch->pDest->pszID, xmlnsOwner, "affiliation", "admin", JabberIqResultMucGetAdminList );
		break;

	case IDM_OWNER:
		JabberAdminGet( gch->pDest->pszID, xmlnsOwner, "affiliation", "owner", JabberIqResultMucGetOwnerList );
		break;

	case IDM_TOPIC:
		mir_snprintf( szBuffer, sizeof szBuffer, "%s %s", JTranslate( "Set topic for" ), gch->pDest->pszID );
		if ( JabberEnterString( szBuffer, sizeof szBuffer ))
            JabberSend( jabberThreadInfo->s, "<message to=\"%s\" type=\"groupchat\"><subject>%s</subject></message>",
				gch->pDest->pszID, TXT(szBuffer));
		break;

	case IDM_NICK:
		mir_snprintf( szBuffer, sizeof szBuffer, "%s %s", JTranslate( "Change nickname in" ), gch->pDest->pszID );
		if ( JabberEnterString( szBuffer, sizeof szBuffer )) {
			JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, gch->pDest->pszID );
			if ( item != NULL ) {
				char text[ 1024 ];
				mir_snprintf( text, sizeof( text ), "%s/%s", gch->pDest->pszID, TXT(szBuffer));
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
        JabberSend( jabberThreadInfo->s, "<iq type=\"get\" id=\""JABBER_IQID"%d\" to=\"%s\">%s</query></iq>",
			iqId, gch->pDest->pszID, xmlnsOwner );
		break;
	}
	case IDM_DESTROY:
		mir_snprintf( szBuffer, sizeof szBuffer, "%s %s", JTranslate( "Reason to destroy" ), gch->pDest->pszID );
		if ( !JabberEnterString( szBuffer, sizeof szBuffer ))
			break;

        JabberSend( jabberThreadInfo->s, "<iq type=\"set\" to=\"%s\">%s<destroy><reason>%s</reason></destroy></query></iq>",
			gch->pDest->pszID, xmlnsOwner, TXT(szBuffer));

	case IDM_LEAVE:
		JabberGcQuit( item, 0, 0 );
		break;
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// Sends a private message to a chat user

static void sttSendPrivateMessage( JABBER_LIST_ITEM* item, const char* nick )
{
	char szFullJid[ 256 ];
	mir_snprintf( szFullJid, sizeof szFullJid, "%s/%s", item->jid, nick );
	HANDLE hContact = JabberDBCreateContact( szFullJid, NULL, FALSE, FALSE );
	if ( hContact != NULL ) {
		DBWriteContactSettingByte( hContact, "CList", "Hidden", 1 );
		JSetStringUtf( hContact, "Nick", nick );
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

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, gch->pDest->pszID );
	if ( item == NULL )
		return 0;

	switch ( gch->pDest->iType ) {
	case GC_USER_MESSAGE:
		if ( gch->pszText && lstrlenA( gch->pszText) > 0 ) {
			rtrim( gch->pszText );

			if ( jabberOnline ) {
				char* str = JabberTextEncode( gch->pszText );
				if ( str != NULL ) {
					UnEscapeChatTags( str );
                    JabberSend( jabberThreadInfo->s, "<message to=\"%s\" type=\"groupchat\"><body>%s</body></message>", item->jid, str );
					free( str );
		}	}	}
		break;

	case GC_USER_PRIVMESS:
		sttSendPrivateMessage( item, gch->pszUID );
		break;

	case GC_USER_LOGMENU:
		sttLogListHook( item, gch );
		break;

	case GC_USER_NICKLISTMENU:
		sttNickListHook( item, gch );
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void JabberAddMucListItem( JABBER_MUC_JIDLIST_INFO* jidListInfo, char* str )
{
	const char* field = strchr(str,'@') ? "jid" : "nick";
	char* roomJid = jidListInfo->roomJid;

	switch (jidListInfo->type) {
	case MUC_VOICELIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "role", "participant" );
		JabberAdminGet( roomJid, xmlnsAdmin, "role", "participant", JabberIqResultMucGetVoiceList);
		break;
	case MUC_MEMBERLIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "affiliation", "member" );
		JabberAdminGet( roomJid, xmlnsAdmin, "affiliation", "member", JabberIqResultMucGetMemberList);
		break;
	case MUC_MODERATORLIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "role", "moderator" );
		JabberAdminGet( roomJid, xmlnsAdmin, "role", "moderator", JabberIqResultMucGetModeratorList);
		break;
	case MUC_BANLIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "affiliation", "outcast" );
		JabberAdminGet( roomJid, xmlnsAdmin, "affiliation", "outcast", JabberIqResultMucGetBanList);
		break;
	case MUC_ADMINLIST:
		JabberAdminSet( roomJid, xmlnsOwner, field, str, "affiliation", "admin" );
		JabberAdminGet( roomJid, xmlnsOwner, "affiliation", "admin", JabberIqResultMucGetAdminList);
		break;
	case MUC_OWNERLIST:
		JabberAdminSet( roomJid, xmlnsOwner, field, str, "affiliation", "owner" );
		JabberAdminGet( roomJid, xmlnsOwner, "affiliation", "owner", JabberIqResultMucGetOwnerList);
		break;
}	}

void JabberDeleteMucListItem( JABBER_MUC_JIDLIST_INFO* jidListInfo, char* jid )
{
	char* roomJid = jidListInfo->roomJid;

	switch ( jidListInfo->type ) {
	case MUC_VOICELIST:		// change role to visitor ( from participant )
		JabberAdminSet( roomJid, xmlnsAdmin, "jid", jid, "role", "visitor" );
		break;
	case MUC_BANLIST:		// change affiliation to none ( from outcast )
	case MUC_MEMBERLIST:	// change affiliation to none ( from member )
		JabberAdminSet( roomJid, xmlnsAdmin, "jid", jid, "affiliation", "none" );
		break;
	case MUC_MODERATORLIST:	// change role to participant ( from moderator )
		JabberAdminSet( roomJid, xmlnsAdmin, "jid", jid, "role", "participant" );
		break;
	case MUC_ADMINLIST:		// change affiliation to member ( from admin )
		JabberAdminSet( roomJid, xmlnsOwner, "jid", jid, "affiliation", "member" );
		break;
	case MUC_OWNERLIST:		// change affiliation to admin ( from owner )
		JabberAdminSet( roomJid, xmlnsOwner, "jid", jid, "affiliation", "admin" );
		break;
}	}
