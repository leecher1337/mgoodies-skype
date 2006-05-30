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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_menu.cpp,v $
Revision       : $Revision: 1.17 $
Last change on : $Date: 2006/05/12 20:13:35 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_list.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////
// module data

HANDLE hMenuRequestAuth = NULL;
HANDLE hMenuGrantAuth = NULL;
HANDLE hMenuJoinLeave = NULL;
HANDLE hMenuConvert = NULL;
HANDLE hMenuRosterAdd = NULL;

HANDLE hMenuVisitGMail = NULL;

char* cidchar = "ChatRoomID";
char* jidchar = "jid";

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
	sttEnableMenuItem( hMenuConvert, FALSE );
	sttEnableMenuItem( hMenuRosterAdd, FALSE );
	sttEnableMenuItem( hMenuVisitGMail, FALSE );

	HANDLE hContact;
	if (( hContact=( HANDLE )wParam ) == NULL ){
		return 0;
	} else {
		DBVARIANT dbv;
		if ( !JGetStringT( hContact, "FakeContact", &dbv )) {
			if (!_tcsicmp( dbv.ptszVal, _T("GMAIL"))){
				sttEnableMenuItem( hMenuVisitGMail, TRUE );
				return 0;
			}		
			JFreeVariant( &dbv );
	}	}

	BYTE chatRoomType = (BYTE)JGetByte( hContact, "ChatRoom", 0 );

	if ((chatRoomType == GCW_CHATROOM) || chatRoomType == 0 ) {
		DBVARIANT dbv;
		if ( !JGetStringT( hContact, chatRoomType?cidchar:jidchar, &dbv )) { //cidchar and jidchar are defined above to make gcc happy
			JFreeVariant( &dbv );
			CLISTMENUITEM clmi = { 0 };
			sttEnableMenuItem( hMenuConvert, TRUE );
			clmi.cbSize = sizeof( clmi );
			clmi.pszName = JTranslate( chatRoomType ? "&Convert to Contact" : "&Convert to Chat Room" );
			clmi.flags = CMIM_NAME | CMIM_FLAGS;
			JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuConvert, ( LPARAM )&clmi );
	}	}

	if (!jabberOnline) 
		return 0;

	if ( chatRoomType ) {
		DBVARIANT dbv;
		if ( !JGetStringT( hContact, "ChatRoomID", &dbv )) {
			if ( JabberListGetItemPtr( LIST_ROSTER, dbv.ptszVal ) == NULL ) {
				sttEnableMenuItem( hMenuRosterAdd, TRUE );
			}
			JFreeVariant( &dbv );
	}	}

	if ( chatRoomType == GCW_CHATROOM ) {
		CLISTMENUITEM clmi = { 0 };
		clmi.cbSize = sizeof( clmi );
		clmi.pszName = JTranslate(( JGetWord( hContact, "Status", 0 ) == ID_STATUS_ONLINE ) ? "&Leave" : "&Join" );
		clmi.flags = CMIM_NAME | CMIM_FLAGS;
		JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuJoinLeave, ( LPARAM )&clmi );
		return 0;
	}

	DBVARIANT dbv;
	if ( !JGetStringT( hContact, "jid", &dbv )) {
		JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, dbv.ptszVal );
		JFreeVariant( &dbv );
		if ( item != NULL ) {
			sttEnableMenuItem( hMenuRequestAuth, item->subscription == SUB_FROM || item->subscription == SUB_NONE );
			sttEnableMenuItem( hMenuGrantAuth, item->subscription == SUB_TO || item->subscription == SUB_NONE );
			return 0;
	}	}

	return 0;
}

int JabberMenuConvertChatContact( WPARAM wParam, LPARAM lParam )
{
	BYTE chatRoomType = (BYTE)JGetByte( (HANDLE ) wParam, "ChatRoom", 0 );
	if ((chatRoomType == GCW_CHATROOM) || chatRoomType == 0 ) {
		DBVARIANT dbv;
		
		if ( !JGetStringT( (HANDLE ) wParam, (chatRoomType == GCW_CHATROOM)?cidchar:jidchar, &dbv )) {
			JDeleteSetting( (HANDLE ) wParam, (chatRoomType == GCW_CHATROOM)?cidchar:jidchar);
			JSetStringT( (HANDLE ) wParam, (chatRoomType != GCW_CHATROOM)?cidchar:jidchar, dbv.ptszVal);
			JFreeVariant( &dbv );
			JSetByte((HANDLE ) wParam, "ChatRoom", (chatRoomType == GCW_CHATROOM)?0:GCW_CHATROOM);
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

	// "Convert Chat/Contact"
	strcpy( tDest, "/ConvertChatContact" );
	CreateServiceFunction( text, JabberMenuConvertChatContact );
	mi.pszName = JTranslate( "Convert" );
	mi.position = -1999901003;
	mi.hIcon = iconList[16];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_USER2ROOM ));
	mi.pszService = text;
	mi.pszContactOwner = jabberProtoName;
	hMenuConvert = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// "Add to roster"
	strcpy( tDest, "/AddToRoster" );
	CreateServiceFunction( text, JabberMenuRosterAdd );
	mi.pszName = JTranslate( "Add to roster" );
	mi.position = -1999901004;
	mi.hIcon = iconList[15];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_ADDROSTER ));
	mi.pszService = text;
	mi.pszContactOwner = jabberProtoName;
	hMenuRosterAdd = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	// "visit GMail for the fake contact"
	strcpy( tDest, "/VisitGMail" );
	CreateServiceFunction( text, JabberMenuVisitGMail );
	mi.pszName = JTranslate( "Visit GMail" );
	mi.position = -2000100001;
	mi.hIcon = iconList[14]; // more icons are needed
	mi.pszService = text;
	mi.pszContactOwner = jabberProtoName;
	hMenuVisitGMail = ( HANDLE ) JCallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );
}
