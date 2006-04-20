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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_menu.cpp,v $
Revision       : $Revision: 1.14 $
Last change on : $Date: 2005/10/23 18:55:58 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_list.h"
#include "resource.h"

void JabberGroupchatJoinRoom( const char* server, const char* room, const char* nick, const char* password );

/////////////////////////////////////////////////////////////////////////////////////////
// module data

HANDLE hMenuRequestAuth = NULL;
HANDLE hMenuGrantAuth = NULL;
HANDLE hMenuJoinLeave = NULL;

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
	sttEnableMenuItem( hMenuJoinLeave, FALSE );

	HANDLE hContact;
	if (( hContact=( HANDLE )wParam ) == NULL || !jabberOnline )
		return 0;

	if ( JGetByte( hContact, "ChatRoom", 0 ) == GCW_CHATROOM ) {
		CLISTMENUITEM clmi = { 0 };
		clmi.cbSize = sizeof( clmi );
		clmi.pszName = JTranslate(( JGetWord( hContact, "Status", 0 ) == ID_STATUS_ONLINE ) ? "&Leave" : "&Join" );
		clmi.flags = CMIM_NAME | CMIM_FLAGS;
		JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuJoinLeave, ( LPARAM )&clmi );
		return 0;
	}

	DBVARIANT dbv;
	if ( !JGetStringUtf( hContact, "jid", &dbv )) {
		JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, dbv.pszVal );
		JFreeVariant( &dbv );
		if ( item != NULL ) {
			sttEnableMenuItem( hMenuRequestAuth, item->subscription == SUB_FROM || item->subscription == SUB_NONE );
			sttEnableMenuItem( hMenuGrantAuth, item->subscription == SUB_TO || item->subscription == SUB_NONE );
			return 0;
	}	}

	return 0;
}

int JabberMenuHandleRequestAuth( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact;
	DBVARIANT dbv;

	if (( hContact=( HANDLE ) wParam )!=NULL && jabberOnline ) {
		if ( !JGetStringUtf( hContact, "jid", &dbv )) {
			JabberSend( jabberThreadInfo->s, "<presence to='%s' type='subscribe'/>", dbv.pszVal );
			JFreeVariant( &dbv );
	}	}

	return 0;
}

int JabberMenuHandleGrantAuth( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact;
	DBVARIANT dbv;

	if (( hContact=( HANDLE ) wParam )!=NULL && jabberOnline ) {
		if ( !JGetStringUtf( hContact, "jid", &dbv )) {
			JabberSend( jabberThreadInfo->s, "<presence to='%s' type='subscribed'/>", dbv.pszVal );
			JFreeVariant( &dbv );
	}	}

	return 0;
}

int JabberMenuJoinLeave( WPARAM wParam, LPARAM lParam )
{
	char szNick[ 256 ], szJid[ JABBER_MAX_JID_LEN ];
	if ( JGetStaticString( "ChatRoomID", ( HANDLE )wParam, szJid,  sizeof szJid  ))
		return 0;

	DBVARIANT dbv;
	if ( !JGetStringUtf(( HANDLE )wParam, "MyNick", &dbv )) {
		strncpy( szNick, dbv.pszVal, sizeof( szNick ));
		JFreeVariant( &dbv );
	}
	else if ( !JGetStringUtf( NULL, "Nick", &dbv )) {
		strncpy( szNick, dbv.pszVal, sizeof( szNick ));
		JFreeVariant( &dbv );
	}
	else return 0;

	if ( JGetWord(( HANDLE )wParam, "Status", 0 ) != ID_STATUS_ONLINE ) {
		if ( !jabberChatDllPresent ) {
			JabberChatDllError();
			return 0;
		}

		char* p = strchr( szJid, '@' );
		if ( p == NULL )
			return 0;

		*p++ = 0;
		JabberGroupchatJoinRoom( p, szJid, szNick, "" );
	}
	else {
		JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, szJid );
		if ( item != NULL )
			JabberGcQuit( item, 0, NULL );
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// contact menu initialization code

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
	mi.pszName = JTranslate( "Request authorization" );
	mi.position = -2000001001;
	mi.hIcon = iconList[6];// IDI_REQUEST;
	mi.pszService = text;
	mi.pszContactOwner = jabberProtoName;
	hMenuRequestAuth = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// "Grant authorization"
	strcpy( tDest, "/GrantAuth" );
	CreateServiceFunction( text, JabberMenuHandleGrantAuth );
	mi.pszName = JTranslate( "Grant authorization" );
	mi.position = -2000001000;
	mi.hIcon = iconList[7];// IDI_GRANT;
	mi.pszService = text;
	mi.pszContactOwner = jabberProtoName;
	hMenuGrantAuth = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// "Grant authorization"
	strcpy( tDest, "/JoinChat" );
	CreateServiceFunction( text, JabberMenuJoinLeave );
	mi.pszName = JTranslate( "Join chat" );
	mi.position = -2000001002;
	mi.hIcon = iconBigList[0];
	mi.pszService = text;
	mi.pszContactOwner = jabberProtoName;
	hMenuJoinLeave = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );
}
