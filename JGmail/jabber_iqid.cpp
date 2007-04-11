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
#include "resource.h"
#include "jabber_list.h"
#include "jabber_iq.h"

extern char* jabberVcardPhotoFileName;
extern char* jabberVcardPhotoType;

static void JabberOnLoggedIn( ThreadData* info )
{
	jabberOnline = TRUE;
	jabberLoggedInTime = time(0);
		
	int enableGmailSetting = JGetByte(NULL,"EnableGMail",1);
	if (enableGmailSetting & 1) JabberEnableNotifications(info);
	int iqId = JabberSerialNext();
	JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetRoster );
		
	XmlNode iq( "iq" ); iq.addAttr( "type", "get" ); iq.addAttrID( iqId );
	XmlNode* query = iq.addChild( "query" ); query->addAttr( "xmlns", "jabber:iq:roster" );
	JabberSend( info->s, iq );

	iqId = JabberSerialNext();
	JabberIqAdd( iqId, IQ_PROC_DISCOBOOKMARKS, JabberIqResultDiscoBookmarks);
	XmlNodeIq biq( "get", iqId);
	XmlNode* bquery = biq.addQuery( "jabber:iq:private" );
	XmlNode* storage = bquery->addChild("storage");
	storage->addAttr("xmlns","storage:bookmarks");

	JabberSend( jabberThreadInfo->s, biq );
	
	char szServerName[ sizeof(info->server) ];
	if ( JGetStaticString( "LastLoggedServer", NULL, szServerName, sizeof(szServerName)))
		JabberSendGetVcard( jabberJID );
	else if ( strcmp( info->server, szServerName ))
		JabberSendGetVcard( jabberJID );
	JSetString( NULL, "LastLoggedServer", info->server );

	if ((enableGmailSetting & 3) == 1) JabberRequestMailBox(info->s);
}

void JabberIqResultGetAuth( XmlNode *iqNode, void *userdata )
{
	// RECVED: result of the request for authentication method
	// ACTION: send account authentication information to log in
	JabberLog( "<iq/> iqIdGetAuth" );

	ThreadData* info = ( ThreadData* ) userdata;
	XmlNode *queryNode;
	TCHAR* type;
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result"))) {
		int iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultSetAuth );

		XmlNodeIq iq( "set", iqId );
		XmlNode* query = iq.addQuery( "jabber:iq:auth" );
		query->addChild( "username", info->username );
		if ( JabberXmlGetChild( queryNode, "digest" )!=NULL && streamId ) {
			char* str = mir_utf8encode( info->password );
			char text[200];
			mir_snprintf( text, SIZEOF(text), "%s%s", streamId, str );
			mir_free( str );
         if (( str=JabberSha1( text )) != NULL ) {
				query->addChild( "digest", str );
				mir_free( str );
			}
		}
		else if ( JabberXmlGetChild( queryNode, "password" ) != NULL )
			query->addChild( "password", info->password );
		else {
			JabberLog( "No known authentication mechanism accepted by the server." );

			JabberSend( info->s, "</stream:stream>" );
			return;
		}

		if ( JabberXmlGetChild( queryNode, "resource" ) != NULL )
			query->addChild( "resource", info->resource );

		JabberSend( info->s, iq );
	}
	else if ( !lstrcmp( type, _T("error"))) {
 		JabberSend( info->s, "</stream:stream>" );

		TCHAR text[128];
		mir_sntprintf( text, SIZEOF( text ), _T("%s %s."), TranslateT( "Authentication failed for" ), info->username );
		MessagePopup( NULL, text, TranslateT( "Jabber Authentication" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
		JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD );
		jabberThreadInfo = NULL;	// To disallow auto reconnect
}	}

void JabberIqResultSetAuth( XmlNode *iqNode, void *userdata )
{
	ThreadData* info = ( ThreadData* ) userdata;
	TCHAR* type;
	int iqId;

	// RECVED: authentication result
	// ACTION: if successfully logged in, continue by requesting roster list and set my initial status
	JabberLog( "<iq/> iqIdSetAuth" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result"))) {
		DBVARIANT dbv;
		if ( JGetStringT( NULL, "Nick", &dbv ))
			JSetStringT( NULL, "Nick", info->username );
		else
			JFreeVariant( &dbv );

		JabberOnLoggedIn( info );

		if ( hwndJabberAgents ) {
			// Retrieve agent information
			iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_GETAGENTS, JabberIqResultGetAgents );

			XmlNodeIq iq( "get", iqId ); 
			XmlNode* query = iq.addQuery( "jabber:iq:agents" );
			JabberSend( info->s, iq );
		}
	}
	// What to do if password error? etc...
	else if ( !lstrcmp( type, _T("error"))) {
		TCHAR text[128];

		JabberSend( info->s, "</stream:stream>" );
		mir_sntprintf( text, SIZEOF( text ), _T("%s %s."), TranslateT( "Authentication failed for" ), info->username );
		MessagePopup( NULL, text, TranslateT( "Jabber Authentication" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
		JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD );
		jabberThreadInfo = NULL;	// To disallow auto reconnect
}	}

void JabberIqResultBind( XmlNode *iqNode, void *userdata )
{
	ThreadData* info = ( ThreadData* ) userdata;
	XmlNode* n = JabberXmlGetChild( iqNode, "bind" );
	if ( n != NULL ) {
		if ( n = JabberXmlGetChild( n, "jid" )) {
			if ( n->text ) {
				if ( !_tcsncmp( info->fullJID, n->text, SIZEOF( info->fullJID )))
					JabberLog( "Result Bind: "TCHAR_STR_PARAM" %s "TCHAR_STR_PARAM, info->fullJID, "confirmed.", NULL );
				else {
					JabberLog( "Result Bind: "TCHAR_STR_PARAM" %s "TCHAR_STR_PARAM, info->fullJID, "changed to", n->text);
					_tcsncpy( info->fullJID, n->text, SIZEOF( info->fullJID ));
		}	}	}

		if ( info->bIsSessionAvailable ) {
			int iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultSession );

			XmlNodeIq iq( "set" ); iq.addAttrID( iqId );
			iq.addChild( "session" )->addAttr( "xmlns", "urn:ietf:params:xml:ns:xmpp-session" );
			JabberSend( info->s, iq );
		}
		else JabberOnLoggedIn( info );
	}
   else if ( n = JabberXmlGetChild( n, "error" )) {
		//rfc3920 page 39
		TCHAR errorMessage [256];
		int pos=0;
		pos = mir_sntprintf( errorMessage, SIZEOF(errorMessage), TranslateT("Resource "));
		XmlNode *tempNode;
		if (tempNode = JabberXmlGetChild( n, "resource" ))
			pos += mir_sntprintf(errorMessage,256-pos,_T("\"%s\" "),tempNode->text);
		pos += mir_sntprintf( errorMessage+pos,256-pos,TranslateT("refused by server\n%s: %s"),TranslateT("Type"),Translate(JabberXmlGetAttrValue( n, "type" )));
		if ( n->numChild )
			pos += mir_sntprintf( errorMessage+pos,256-pos,_T("\n%s: ")_T(TCHAR_STR_PARAM)_T("\n"),TranslateT("Reason"),JTranslate( n->child[0]->name));
		mir_sntprintf( errorMessage,256-pos, _T("%s @")_T(TCHAR_STR_PARAM)_T("."), TranslateT( "Authentication failed for" ), info->username, info->server );
		MessageBox( NULL, errorMessage, TranslateT( "Jabber Protocol" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
		JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPROTOCOL );
		JabberSend( info->s, "</stream:stream>" );
		jabberThreadInfo = NULL;	// To disallow auto reconnect
}	}

void JabberIqResultSession( XmlNode *iqNode, void *userdata )
{
	ThreadData* info = ( ThreadData* )userdata;

	TCHAR* type;
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result")))
		JabberOnLoggedIn( info );
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetRoster - populates LIST_ROSTER and creates contact for any new rosters

void sttGroupchatJoinByHContact( HANDLE hContact )
{
	DBVARIANT dbv;
	if( JGetStringT( hContact, "ChatRoomID", &dbv ))
		return;
	if( dbv.type != DBVT_ASCIIZ && dbv.type != DBVT_WCHAR )
		return;

	TCHAR* roomjid = mir_tstrdup( dbv.ptszVal );
	JFreeVariant( &dbv );
	if( !roomjid ) return;

	TCHAR* room = roomjid;
	TCHAR* server = _tcschr( roomjid, '@' );
	if( !server )
		return;
	server[0] = '\0'; server++;

	TCHAR nick[ 256 ];
	if ( JGetStringT( hContact, "MyNick", &dbv )) {
		TCHAR* jidnick = JabberNickFromJID( jabberJID );
		if( !jidnick ) {
			mir_free( jidnick );
			mir_free( roomjid );
			return;
		}
		_tcsncpy( nick, jidnick, SIZEOF( nick ));
		mir_free( jidnick );
	}
	else {
		_tcsncpy( nick, dbv.ptszVal, SIZEOF( nick ));
		JFreeVariant( &dbv );
	}

	JabberGroupchatJoinRoom( server, room, nick, _T(""));
	mir_free( roomjid );
}

void CALLBACK sttCreateRoom( ULONG dwParam )
{
	GCSESSION gcw = {0};
	gcw.cbSize = sizeof(GCSESSION);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = jabberProtoName;
	gcw.dwFlags = GC_TCHAR;
	gcw.ptszID = ( TCHAR* )dwParam;
	gcw.ptszName = NEWTSTR_ALLOCA(( TCHAR* )dwParam );

	TCHAR* p = (TCHAR*)_tcschr( gcw.ptszName, '@' );
	if ( p ) *p = 0;

	CallService( MS_GC_NEWSESSION, 0, ( LPARAM )&gcw );
}

void JabberIqResultGetRoster( XmlNode* iqNode, void* )
{
	JabberLog( "<iq/> iqIdGetRoster" );
	TCHAR* type = JabberXmlGetAttrValue( iqNode, "type" );
	if ( lstrcmp( type, _T("result")))
		return;

	XmlNode* queryNode = JabberXmlGetChild( iqNode, "query" );
   if ( queryNode == NULL )
		return;

	if ( lstrcmp( JabberXmlGetAttrValue( queryNode, "xmlns" ), _T("jabber:iq:roster")))
		return;

	TCHAR* name, *nick;
	int i;
	SortedList chatRooms = {0};
	chatRooms.increment = 10;

	for ( i=0; i < queryNode->numChild; i++ ) {
		XmlNode* itemNode = queryNode->child[i];
		if ( strcmp( itemNode->name, "item" ))
			continue;

		TCHAR* str = JabberXmlGetAttrValue( itemNode, "subscription" );

		JABBER_SUBSCRIPTION sub;
		if ( str == NULL ) sub = SUB_NONE;
		else if ( !_tcscmp( str, _T("both"))) sub = SUB_BOTH;
		else if ( !_tcscmp( str, _T("to"))) sub = SUB_TO;
		else if ( !_tcscmp( str, _T("from"))) sub = SUB_FROM;
		else sub = SUB_NONE;

		TCHAR* jid = JabberXmlGetAttrValue( itemNode, "jid" );
		if ( jid == NULL )
			continue;

		if (( name = JabberXmlGetAttrValue( itemNode, "name" )) != NULL )
			nick = mir_tstrdup( name );
		else
			nick = JabberNickFromJID( jid );

		if ( nick == NULL )
			continue;

		JABBER_LIST_ITEM* item = JabberListAdd( LIST_ROSTER, jid );
		item->subscription = sub;

		mir_free( item->nick ); item->nick = nick;

		XmlNode* groupNode = JabberXmlGetChild( itemNode, "group" );
		replaceStr( item->group, ( groupNode ) ? groupNode->text : NULL );

		HANDLE hContact = JabberHContactFromJID( jid );
		if ( hContact == NULL ) {
			// Received roster has a new JID.
			// Add the jid ( with empty resource ) to Miranda contact list.
			hContact = JabberDBCreateContact( jid, nick, FALSE, TRUE );
		}

		if ( name != NULL ) {
			DBVARIANT dbNick;
			if ( !JGetStringT( hContact, "Nick", &dbNick )) {
				if ( lstrcmp( nick, dbNick.ptszVal ) != 0 )
					DBWriteContactSettingTString( hContact, "CList", "MyHandle", nick );
				else
					DBDeleteContactSetting( hContact, "CList", "MyHandle" );

				JFreeVariant( &dbNick );
			}
			else DBWriteContactSettingTString( hContact, "CList", "MyHandle", nick );
		}
		else DBDeleteContactSetting( hContact, "CList", "MyHandle" );

		if ( JGetByte( hContact, "ChatRoom", 0 )) {
			QueueUserAPC( sttCreateRoom, hMainThread, ( unsigned long )jid );
			DBDeleteContactSetting( hContact, "CList", "Hidden" );
			li.List_Insert( &chatRooms, hContact, chatRooms.realCount );
		}

		if ( item->group != NULL ) {
			JabberContactListCreateGroup( item->group );

			// Don't set group again if already correct, or Miranda may show wrong group count in some case
			DBVARIANT dbv;
			if ( !DBGetContactSettingTString( hContact, "CList", "Group", &dbv )) {
				if ( lstrcmp( dbv.ptszVal, item->group ))
					DBWriteContactSettingTString( hContact, "CList", "Group", item->group );
				JFreeVariant( &dbv );
			}
			else DBWriteContactSettingTString( hContact, "CList", "Group", item->group );
		}
		else DBDeleteContactSetting( hContact, "CList", "Group" );
	}

	// Delete orphaned contacts ( if roster sync is enabled )
	if ( JGetByte( "RosterSync", FALSE ) == TRUE ) {
		int listSize = 0, listAllocSize = 0;
		HANDLE* list = NULL;
		HANDLE hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		while ( hContact != NULL ) {
			char* str = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
			if ( str != NULL && !strcmp( str, jabberProtoName )) {
				DBVARIANT dbv;
				if ( !JGetStringT( hContact, "jid", &dbv )) {
					if ( !JabberListExist( LIST_ROSTER, dbv.ptszVal )) {
						JabberLog( "Syncing roster: preparing to delete " TCHAR_STR_PARAM " ( hContact=0x%x )", dbv.ptszVal, hContact );
						if ( listSize >= listAllocSize ) {
							listAllocSize = listSize + 100;
							if (( list=( HANDLE * ) mir_realloc( list, listAllocSize * sizeof( HANDLE ))) == NULL ) {
								listSize = 0;
								break;
						}	}

						list[listSize++] = hContact;
					}
					JFreeVariant( &dbv );
			}	}

			hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM ) hContact, 0 );
		}

		for ( i=0; i < listSize; i++ ) {
			JabberLog( "Syncing roster: deleting 0x%x", list[i] );
			JCallService( MS_DB_CONTACT_DELETE, ( WPARAM ) list[i], 0 );
		}
		if ( list != NULL )
			mir_free( list );
	}

	JabberEnableMenuItems( TRUE );

	if ( hwndJabberGroupchat )
		SendMessage( hwndJabberGroupchat, WM_JABBER_CHECK_ONLINE, 0, 0 );
	if ( hwndJabberJoinGroupchat )
		SendMessage( hwndJabberJoinGroupchat, WM_JABBER_CHECK_ONLINE, 0, 0 );
	if ( hwndJabberBookmarks )
		SendMessage( hwndJabberBookmarks, WM_JABBER_CHECK_ONLINE, 0, 0 );
	if ( hwndJabberAddBookmark)
		SendMessage( hwndJabberAddBookmark, WM_JABBER_CHECK_ONLINE, 0, 0 );



	JabberLog( "Status changed via THREADSTART" );
	modeMsgStatusChangePending = FALSE;
	JabberSetServerStatus( jabberDesiredStatus );

	if ( JGetByte( "AutoJoinConferences", 0 )) {
		for ( int i=0; i < chatRooms.realCount; i++ )
			sttGroupchatJoinByHContact(( HANDLE )chatRooms.items[i] );
	}
	li.List_Destroy( &chatRooms );

	if ( hwndJabberAgents )
		SendMessage( hwndJabberAgents, WM_JABBER_TRANSPORT_REFRESH, 0, 0 );
	if ( hwndJabberVcard )
		SendMessage( hwndJabberVcard, WM_JABBER_CHECK_ONLINE, 0, 0 );
}

void JabberIqResultGetAgents( XmlNode *iqNode, void *userdata )
{
	ThreadData* info = ( ThreadData* ) userdata;
	XmlNode *queryNode;
	TCHAR* type, *jid, *str;

	// RECVED: agent list
	// ACTION: refresh agent list dialog
	JabberLog( "<iq/> iqIdGetAgents" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result"))) {
		str = JabberXmlGetAttrValue( queryNode, "xmlns" );
		if ( str!=NULL && !lstrcmp( str, _T("jabber:iq:agents"))) {
			XmlNode *agentNode, *n;
			JABBER_LIST_ITEM *item;
			int i;

			JabberListRemoveList( LIST_AGENT );
			for ( i=0; i<queryNode->numChild; i++ ) {
				agentNode = queryNode->child[i];
				if ( !lstrcmpA( agentNode->name, "agent" )) {
					if (( jid=JabberXmlGetAttrValue( agentNode, "jid" )) != NULL ) {
						item = JabberListAdd( LIST_AGENT, jid );
						if ( JabberXmlGetChild( agentNode, "register" ) != NULL )
							item->cap |= AGENT_CAP_REGISTER;
						if ( JabberXmlGetChild( agentNode, "search" ) != NULL )
							item->cap |= AGENT_CAP_SEARCH;
						if ( JabberXmlGetChild( agentNode, "groupchat" ) != NULL )
							item->cap |= AGENT_CAP_GROUPCHAT;
						// set service also???
						// most chatroom servers don't announce <grouchat/> so we will
						// also treat <service>public</service> as groupchat services
						if (( n=JabberXmlGetChild( agentNode, "service" ))!=NULL && n->text!=NULL && !_tcscmp( n->text, _T("public")))
							item->cap |= AGENT_CAP_GROUPCHAT;
						if (( n=JabberXmlGetChild( agentNode, "name" ))!=NULL && n->text!=NULL )
							item->name = mir_tstrdup( n->text );
		}	}	}	}

		if ( hwndJabberAgents != NULL ) {
			if (( jid = JabberXmlGetAttrValue( iqNode, "from" )) != NULL )
				SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )jid );
			else
				SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )info->server );
}	}	}

void JabberIqResultGetRegister( XmlNode *iqNode, void *userdata )
{
	// RECVED: result of the request for ( agent ) registration mechanism
	// ACTION: activate ( agent ) registration input dialog
	JabberLog( "<iq/> iqIdGetRegister" );

	ThreadData* info = ( ThreadData* ) userdata;
	XmlNode *queryNode, *n;
	TCHAR *type;
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result"))) {
		if ( hwndAgentRegInput )
			if (( n = JabberXmlCopyNode( iqNode )) != NULL )
				SendMessage( hwndAgentRegInput, WM_JABBER_REGINPUT_ACTIVATE, 1 /*success*/, ( LPARAM )n );
	}
	else if ( !lstrcmp( type, _T("error"))) {
		if ( hwndAgentRegInput ) {
			XmlNode *errorNode = JabberXmlGetChild( iqNode, "error" );
			TCHAR* str = JabberErrorMsg( errorNode );
			SendMessage( hwndAgentRegInput, WM_JABBER_REGINPUT_ACTIVATE, 0 /*error*/, ( LPARAM )str );
			mir_free( str );
}	}	}

void JabberIqResultSetRegister( XmlNode *iqNode, void *userdata )
{
	// RECVED: result of registration process
	// ACTION: notify of successful agent registration
	JabberLog( "<iq/> iqIdSetRegister" );

	TCHAR *type, *from;
	if (( type = JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( from = JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result"))) {
		HANDLE hContact = JabberHContactFromJID( from );
		if ( hContact != NULL )
			JSetByte( hContact, "IsTransport", TRUE );

		if ( hwndRegProgress )
			SendMessage( hwndRegProgress, WM_JABBER_REGDLG_UPDATE, 100, ( LPARAM )TranslateT( "Registration successful" ));
	}
	else if ( !lstrcmp( type, _T("error"))) {
		if ( hwndRegProgress ) {
			XmlNode *errorNode = JabberXmlGetChild( iqNode, "error" );
			TCHAR* str = JabberErrorMsg( errorNode );
			SendMessage( hwndRegProgress, WM_JABBER_REGDLG_UPDATE, 100, ( LPARAM )str );
			mir_free( str );
}	}	}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetVcard - processes the server-side v-card

static void JabberIqResultGetVcardPhoto( const TCHAR* jid, XmlNode* n, HANDLE hContact, BOOL& hasPhoto )
{
	if ( hasPhoto ) return;

	XmlNode* o = JabberXmlGetChild( n, "BINVAL" );
	if ( o == NULL || o->text == NULL ) return;

	int bufferLen;
	char* buffer = JabberBase64Decode( o->text, &bufferLen );
	if ( buffer == NULL ) return;

	XmlNode* m = JabberXmlGetChild( n, "TYPE" );
	if ( m == NULL || m->text == NULL ) {
LBL_NoTypeSpecified:
		char* szPicType;

		switch( JabberGetPictureType( buffer )) {
		case PA_FORMAT_GIF:	szPicType = "image/gif";	break;
		case PA_FORMAT_BMP:  szPicType = "image/bmp";	break;
		case PA_FORMAT_PNG:  szPicType = "image/png";	break;
		case PA_FORMAT_JPEG: szPicType = "image/jpeg";	break;
		default:
			goto LBL_Ret;
		}

		replaceStr( jabberVcardPhotoType, szPicType );
	}
	else {
		if ( _tcscmp( m->text, _T("image/jpeg")) && _tcscmp( m->text, _T("image/png")) && _tcscmp( m->text, _T("image/gif")) && _tcscmp( m->text, _T("image/bmp")))
			goto LBL_NoTypeSpecified;

		if ( jabberVcardPhotoType ) mir_free(jabberVcardPhotoType);
		jabberVcardPhotoType = t2a( m->text );
	}

	DWORD nWritten;
	char szTempPath[MAX_PATH], szTempFileName[MAX_PATH];
	JABBER_LIST_ITEM *item;
	DBVARIANT dbv;

	if ( GetTempPathA( sizeof( szTempPath ), szTempPath ) <= 0 )
		lstrcpyA( szTempPath, ".\\" );
	if ( !GetTempFileNameA( szTempPath, jabberProtoName, GetTickCount(), szTempFileName )) {
LBL_Ret:
		mir_free( buffer );
		return;
	}

	//put correct extension to make MS_UTILS_LOADBITMAP happy
	{	char* p = strrchr( szTempFileName, '.' );
		if ( p != NULL )
			lstrcpyA( p+1, strrchr(jabberVcardPhotoType,'/')+1 );
	}

	JabberLog( "Picture file name set to %s", szTempFileName );
	HANDLE hFile = CreateFileA( szTempFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		goto LBL_Ret;

	JabberLog( "Writing %d bytes", bufferLen );
	if ( !WriteFile( hFile, buffer, bufferLen, &nWritten, NULL ))
		goto LBL_Ret;

	JabberLog( "%d bytes written", nWritten );
	if ( hContact == NULL ) {
		hasPhoto = TRUE;
		if ( jabberVcardPhotoFileName ) {
			DeleteFileA( jabberVcardPhotoFileName );
			mir_free( jabberVcardPhotoFileName );
			jabberVcardPhotoFileName = NULL;
		}
		replaceStr( jabberVcardPhotoFileName, szTempFileName );
		JabberLog( "My picture saved to %s", szTempFileName );
	}
	else if ( !JGetStringT( hContact, "jid", &dbv )) {
		item = JabberListGetItemPtr( LIST_ROSTER, jid );
		if ( item == NULL )
			item = JabberListAdd( LIST_VCARD_TEMP, jid ); // adding to the temp list to store information about photo
		if (item != NULL ) {
			hasPhoto = TRUE;
			if ( item->photoFileName )
				DeleteFileA( item->photoFileName );
			replaceStr( item->photoFileName, szTempFileName );
			JabberLog( "Contact's picture saved to %s", szTempFileName );
		}
		JFreeVariant( &dbv );
	}

	CloseHandle( hFile );

	if ( !hasPhoto )
		DeleteFileA( szTempFileName );

	goto LBL_Ret;
}

static char* sttGetText( XmlNode* node, char* tag )
{
	XmlNode* n = JabberXmlGetChild( node, tag );
	if ( n == NULL )
		return NULL;

	return t2a( n->text );
}

void JabberIqResultGetVcard( XmlNode *iqNode, void *userdata )
{
	XmlNode *vCardNode, *m, *n, *o;
	TCHAR* type, *jid;
	HANDLE hContact;
	TCHAR text[128];
	DBVARIANT dbv;

	JabberLog( "<iq/> iqIdGetVcard" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( jid=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;
	int id = JabberGetPacketID( iqNode );

	if ( id == jabberSearchID ) {
		jabberSearchID = -1;

		if (( vCardNode = JabberXmlGetChild( iqNode, "vCard" )) != NULL ) {
			if ( !lstrcmp( type, _T("result"))) {
				JABBER_SEARCH_RESULT jsr = { 0 };
				jsr.hdr.cbSize = sizeof( JABBER_SEARCH_RESULT );
				jsr.hdr.nick = sttGetText( vCardNode, "NICKNAME" );
				jsr.hdr.firstName = sttGetText( vCardNode, "FN" );
				jsr.hdr.lastName = "";
				jsr.hdr.email = sttGetText( vCardNode, "EMAIL" );
				_tcsncpy( jsr.jid, jid, SIZEOF( jsr.jid ));
				jsr.jid[ SIZEOF( jsr.jid )-1 ] = '\0';
				JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, ( HANDLE )id, ( LPARAM )&jsr );
				JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE )id, 0 );
				mir_free( jsr.hdr.nick );
				mir_free( jsr.hdr.firstName );
				mir_free( jsr.hdr.email );
			}
			else if ( !lstrcmp( type, _T("error")))
				JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE )id, 0 );
		}
		else JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE )id, 0 );
		return;
	}

	int len = _tcslen( jabberJID );
	if ( !_tcsnicmp( jid, jabberJID, len ) && ( jid[len]=='/' || jid[len]=='\0' )) {
		hContact = NULL;
		JabberLog( "Vcard for myself" );
	}
	else {
		if (( hContact = JabberHContactFromJID( jid )) == NULL )
			return;
		JabberLog( "Other user's vcard" );
	}

	if ( !lstrcmp( type, _T("result"))) {
		BOOL hasFn, hasNick, hasGiven, hasFamily, hasMiddle, hasBday, hasGender;
		BOOL hasPhone, hasFax, hasCell, hasUrl;
		BOOL hasHome, hasHomeStreet, hasHomeStreet2, hasHomeLocality, hasHomeRegion, hasHomePcode, hasHomeCtry;
		BOOL hasWork, hasWorkStreet, hasWorkStreet2, hasWorkLocality, hasWorkRegion, hasWorkPcode, hasWorkCtry;
		BOOL hasOrgname, hasOrgunit, hasRole, hasTitle;
		BOOL hasDesc, hasPhoto;
		int nEmail, nPhone, nYear, nMonth, nDay;

		hasFn = hasNick = hasGiven = hasFamily = hasMiddle = hasBday = hasGender = FALSE;
		hasPhone = hasFax = hasCell = hasUrl = FALSE;
		hasHome = hasHomeStreet = hasHomeStreet2 = hasHomeLocality = hasHomeRegion = hasHomePcode = hasHomeCtry = FALSE;
		hasWork = hasWorkStreet = hasWorkStreet2 = hasWorkLocality = hasWorkRegion = hasWorkPcode = hasWorkCtry = FALSE;
		hasOrgname = hasOrgunit = hasRole = hasTitle = FALSE;
		hasDesc = hasPhoto = FALSE;
		nEmail = nPhone = 0;

		if (( vCardNode=JabberXmlGetChild( iqNode, "vCard" )) != NULL ) {
			for ( int i=0; i<vCardNode->numChild; i++ ) {
				n = vCardNode->child[i];
				if ( n==NULL || n->name==NULL ) continue;
				if ( !strcmp( n->name, "FN" )) {
					if ( n->text != NULL ) {
						hasFn = TRUE;
						JSetStringT( hContact, "FullName", n->text );
					}
				}
				else if ( !strcmp( n->name, "NICKNAME" )) {
					if ( n->text != NULL ) {
						hasNick = TRUE;
						JSetStringT( hContact, "Nick", n->text );
					}
				}
				else if ( !strcmp( n->name, "N" )) {
					// First/Last name
					if ( !hasGiven && !hasFamily && !hasMiddle ) {
						if (( m=JabberXmlGetChild( n, "GIVEN" )) != NULL && m->text!=NULL ) {
							hasGiven = TRUE;
							JSetStringT( hContact, "FirstName", m->text );
						}
						if (( m=JabberXmlGetChild( n, "FAMILY" )) != NULL && m->text!=NULL ) {
							hasFamily = TRUE;
							JSetStringT( hContact, "LastName", m->text );
						}
						if (( m=JabberXmlGetChild( n, "MIDDLE" )) != NULL && m->text != NULL ) {
							hasMiddle = TRUE;
							JSetStringT( hContact, "MiddleName", m->text );
					}	}
				}
				else if ( !strcmp( n->name, "EMAIL" )) {
					// E-mail address( es )
					if (( m=JabberXmlGetChild( n, "USERID" )) == NULL )	// Some bad client put e-mail directly in <EMAIL/> instead of <USERID/>
						m = n;
					if ( m->text != NULL ) {
						char text[100];
						if ( hContact != NULL ) {
							if ( nEmail == 0 )
								strcpy( text, "e-mail" );
							else
								sprintf( text, "e-mail%d", nEmail-1 );
						}
						else sprintf( text, "e-mail%d", nEmail );
						JSetStringT( hContact, text, m->text );

						if ( hContact == NULL ) {
							sprintf( text, "e-mailFlag%d", nEmail );
							int nFlag = 0;
							if ( JabberXmlGetChild( n, "HOME" ) != NULL ) nFlag |= JABBER_VCEMAIL_HOME;
							if ( JabberXmlGetChild( n, "WORK" ) != NULL ) nFlag |= JABBER_VCEMAIL_WORK;
							if ( JabberXmlGetChild( n, "INTERNET" ) != NULL ) nFlag |= JABBER_VCEMAIL_INTERNET;
							if ( JabberXmlGetChild( n, "X400" ) != NULL ) nFlag |= JABBER_VCEMAIL_X400;
							JSetWord( NULL, text, nFlag );
						}
						nEmail++;
					}
				}
				else if ( !strcmp( n->name, "BDAY" )) {
					// Birthday
					if ( !hasBday && n->text!=NULL ) {
						if ( hContact != NULL ) {
							if ( _stscanf( n->text, _T("%d-%d-%d"), &nYear, &nMonth, &nDay ) == 3 ) {
								hasBday = TRUE;
								JSetWord( hContact, "BirthYear", ( WORD )nYear );
								JSetByte( hContact, "BirthMonth", ( BYTE ) nMonth );
								JSetByte( hContact, "BirthDay", ( BYTE ) nDay );
							}
						}
						else {
							hasBday = TRUE;
							JSetStringT( NULL, "BirthDate", n->text );
					}	}
				}
				else if ( !lstrcmpA( n->name, "GENDER" )) {
					// Gender
					if ( !hasGender && n->text!=NULL ) {
						if ( hContact != NULL ) {
							if ( n->text[0] && strchr( "mMfF", n->text[0] )!=NULL ) {
								hasGender = TRUE;
								JSetByte( hContact, "Gender", ( BYTE ) toupper( n->text[0] ));
							}
						}
						else {
							hasGender = TRUE;
							JSetStringT( NULL, "GenderString", n->text );
					}	}
				}
				else if ( !strcmp( n->name, "ADR" )) {
					if ( !hasHome && JabberXmlGetChild( n, "HOME" )!=NULL ) {
						// Home address
						hasHome = TRUE;
						if (( m=JabberXmlGetChild( n, "STREET" )) != NULL && m->text != NULL ) {
							hasHomeStreet = TRUE;
							if ( hContact != NULL ) {
								if (( o=JabberXmlGetChild( n, "EXTADR" )) != NULL && o->text != NULL )
									mir_sntprintf( text, SIZEOF( text ), _T("%s\r\n%s"), m->text, o->text );
								else if (( o=JabberXmlGetChild( n, "EXTADD" ))!=NULL && o->text!=NULL )
									mir_sntprintf( text, SIZEOF( text ), _T("%s\r\n%s"), m->text, o->text );
								else
									_tcsncpy( text, m->text, SIZEOF( text ));
								text[sizeof( text )-1] = '\0';
								JSetStringT( hContact, "Street", text );
							}
							else {
								JSetStringT( hContact, "Street", m->text );
								if (( m=JabberXmlGetChild( n, "EXTADR" )) == NULL )
									m = JabberXmlGetChild( n, "EXTADD" );
								if ( m!=NULL && m->text!=NULL ) {
									hasHomeStreet2 = TRUE;
									JSetStringT( hContact, "Street2", m->text );
						}	}	}

						if (( m=JabberXmlGetChild( n, "LOCALITY" ))!=NULL && m->text!=NULL ) {
							hasHomeLocality = TRUE;
							JSetStringT( hContact, "City", m->text );
						}
						if (( m=JabberXmlGetChild( n, "REGION" ))!=NULL && m->text!=NULL ) {
							hasHomeRegion = TRUE;
							JSetStringT( hContact, "State", m->text );
						}
						if (( m=JabberXmlGetChild( n, "PCODE" ))!=NULL && m->text!=NULL ) {
							hasHomePcode = TRUE;
							JSetStringT( hContact, "ZIP", m->text );
						}
						if (( m=JabberXmlGetChild( n, "CTRY" ))==NULL || m->text==NULL )	// Some bad client use <COUNTRY/> instead of <CTRY/>
							m = JabberXmlGetChild( n, "COUNTRY" );
						if ( m!=NULL && m->text!=NULL ) {
							hasHomeCtry = TRUE;
							if ( hContact != NULL )
								JSetWord( hContact, "Country", ( WORD )JabberCountryNameToId( m->text ));
							else
								JSetStringT( hContact, "CountryName", m->text );
					}	}

					if ( !hasWork && JabberXmlGetChild( n, "WORK" )!=NULL ) {
						// Work address
						hasWork = TRUE;
						if (( m=JabberXmlGetChild( n, "STREET" ))!=NULL && m->text!=NULL ) {
							hasWorkStreet = TRUE;
							if ( hContact != NULL ) {
								if (( o=JabberXmlGetChild( n, "EXTADR" ))!=NULL && o->text!=NULL )
									mir_sntprintf( text, SIZEOF( text ), _T("%s\r\n%s"), m->text, o->text );
								else if (( o=JabberXmlGetChild( n, "EXTADD" ))!=NULL && o->text!=NULL )
									mir_sntprintf( text, SIZEOF( text ), _T("%s\r\n%s"), m->text, o->text );
								else
									_tcsncpy( text, m->text, SIZEOF( text ));
								text[sizeof( text )-1] = '\0';
								JSetStringT( hContact, "CompanyStreet", text );
							}
							else {
								JSetStringT( hContact, "CompanyStreet", m->text );
								if (( m=JabberXmlGetChild( n, "EXTADR" )) == NULL )
									m = JabberXmlGetChild( n, "EXTADD" );
								if ( m!=NULL && m->text!=NULL ) {
									hasWorkStreet2 = TRUE;
									JSetStringT( hContact, "CompanyStreet2", m->text );
						}	}	}

						if (( m=JabberXmlGetChild( n, "LOCALITY" ))!=NULL && m->text!=NULL ) {
							hasWorkLocality = TRUE;
							JSetStringT( hContact, "CompanyCity", m->text );
						}
						if (( m=JabberXmlGetChild( n, "REGION" ))!=NULL && m->text!=NULL ) {
							hasWorkRegion = TRUE;
							JSetStringT( hContact, "CompanyState", m->text );
						}
						if (( m=JabberXmlGetChild( n, "PCODE" ))!=NULL && m->text!=NULL ) {
							hasWorkPcode = TRUE;
							JSetStringT( hContact, "CompanyZIP", m->text );
						}
						if (( m=JabberXmlGetChild( n, "CTRY" ))==NULL || m->text==NULL )	// Some bad client use <COUNTRY/> instead of <CTRY/>
							m = JabberXmlGetChild( n, "COUNTRY" );
						if ( m!=NULL && m->text!=NULL ) {
							hasWorkCtry = TRUE;
							if ( hContact != NULL )
								JSetWord( hContact, "CompanyCountry", ( WORD )JabberCountryNameToId( m->text ));
							else
								JSetStringT( hContact, "CompanyCountryName", m->text );
					}	}
				}
				else if ( !strcmp( n->name, "TEL" )) {
					// Telephone/Fax/Cellular
					if (( m=JabberXmlGetChild( n, "NUMBER" ))!=NULL && m->text!=NULL ) {
						if ( hContact != NULL ) {
							if ( !hasFax && JabberXmlGetChild( n, "FAX" )!=NULL ) {
								hasFax = TRUE;
								JSetStringT( hContact, "Fax", m->text );
							}
							if ( !hasCell && JabberXmlGetChild( n, "CELL" )!=NULL ) {
								hasCell = TRUE;
								JSetStringT( hContact, "Cellular", m->text );
							}
							if ( !hasPhone &&
								( JabberXmlGetChild( n, "HOME" )!=NULL ||
								 JabberXmlGetChild( n, "WORK" )!=NULL ||
								 JabberXmlGetChild( n, "VOICE" )!=NULL ||
								 ( JabberXmlGetChild( n, "FAX" )==NULL &&
								  JabberXmlGetChild( n, "PAGER" )==NULL &&
								  JabberXmlGetChild( n, "MSG" )==NULL &&
								  JabberXmlGetChild( n, "CELL" )==NULL &&
								  JabberXmlGetChild( n, "VIDEO" )==NULL &&
								  JabberXmlGetChild( n, "BBS" )==NULL &&
								  JabberXmlGetChild( n, "MODEM" )==NULL &&
								  JabberXmlGetChild( n, "ISDN" )==NULL &&
								  JabberXmlGetChild( n, "PCS" )==NULL )) ) {
								hasPhone = TRUE;
								JSetStringT( hContact, "Phone", m->text );
							}
						}
						else {
							char text[ 100 ];
							sprintf( text, "Phone%d", nPhone );
							JSetStringT( NULL, text, m->text );

							sprintf( text, "PhoneFlag%d", nPhone );
							int nFlag = 0;
							if ( JabberXmlGetChild( n, "HOME" ) != NULL ) nFlag |= JABBER_VCTEL_HOME;
							if ( JabberXmlGetChild( n, "WORK" ) != NULL ) nFlag |= JABBER_VCTEL_WORK;
							if ( JabberXmlGetChild( n, "VOICE" ) != NULL ) nFlag |= JABBER_VCTEL_VOICE;
							if ( JabberXmlGetChild( n, "FAX" ) != NULL ) nFlag |= JABBER_VCTEL_FAX;
							if ( JabberXmlGetChild( n, "PAGER" ) != NULL ) nFlag |= JABBER_VCTEL_PAGER;
							if ( JabberXmlGetChild( n, "MSG" ) != NULL ) nFlag |= JABBER_VCTEL_MSG;
							if ( JabberXmlGetChild( n, "CELL" ) != NULL ) nFlag |= JABBER_VCTEL_CELL;
							if ( JabberXmlGetChild( n, "VIDEO" ) != NULL ) nFlag |= JABBER_VCTEL_VIDEO;
							if ( JabberXmlGetChild( n, "BBS" ) != NULL ) nFlag |= JABBER_VCTEL_BBS;
							if ( JabberXmlGetChild( n, "MODEM" ) != NULL ) nFlag |= JABBER_VCTEL_MODEM;
							if ( JabberXmlGetChild( n, "ISDN" ) != NULL ) nFlag |= JABBER_VCTEL_ISDN;
							if ( JabberXmlGetChild( n, "PCS" ) != NULL ) nFlag |= JABBER_VCTEL_PCS;
							JSetWord( NULL, text, nFlag );
							nPhone++;
					}	}
				}
				else if ( !strcmp( n->name, "URL" )) {
					// Homepage
					if ( !hasUrl && n->text!=NULL ) {
						hasUrl = TRUE;
						JSetStringT( hContact, "Homepage", n->text );
					}
				}
				else if ( !strcmp( n->name, "ORG" )) {
					if ( !hasOrgname && !hasOrgunit ) {
						if (( m=JabberXmlGetChild( n, "ORGNAME" ))!=NULL && m->text!=NULL ) {
							hasOrgname = TRUE;
							JSetStringT( hContact, "Company", m->text );
						}
						if (( m=JabberXmlGetChild( n, "ORGUNIT" ))!=NULL && m->text!=NULL ) {	// The real vCard can have multiple <ORGUNIT/> but we will only display the first one
							hasOrgunit = TRUE;
							JSetStringT( hContact, "CompanyDepartment", m->text );
					}	}
				}
				else if ( !strcmp( n->name, "ROLE" )) {
					if ( !hasRole && n->text!=NULL ) {
						hasRole = TRUE;
						JSetStringT( hContact, "Role", n->text );
					}
				}
				else if ( !strcmp( n->name, "TITLE" )) {
					if ( !hasTitle && n->text!=NULL ) {
						hasTitle = TRUE;
						JSetStringT( hContact, "CompanyPosition", n->text );
					}
				}
				else if ( !strcmp( n->name, "DESC" )) {
					if ( !hasDesc && n->text!=NULL ) {
						hasDesc = TRUE;
						TCHAR* szMemo = JabberUnixToDosT( n->text );
						JSetStringT( hContact, "About", szMemo );
						mir_free( szMemo );
					}
				}
				else if ( !strcmp( n->name, "PHOTO" ))
					JabberIqResultGetVcardPhoto( jid, n, hContact, hasPhoto );
		}	}

		if ( !hasFn )
			JDeleteSetting( hContact, "FullName" );
		// We are not deleting "Nick"
//		if ( !hasNick )
//			JDeleteSetting( hContact, "Nick" );
		if ( !hasGiven )
			JDeleteSetting( hContact, "FirstName" );
		if ( !hasFamily )
			JDeleteSetting( hContact, "LastName" );
		if ( !hasMiddle )
			JDeleteSetting( hContact, "MiddleName" );
		if ( hContact != NULL ) {
			while ( true ) {
				if ( nEmail <= 0 )
					JDeleteSetting( hContact, "e-mail" );
				else {
					char text[ 100 ];
					sprintf( text, "e-mail%d", nEmail-1 );
					if ( DBGetContactSetting( hContact, jabberProtoName, text, &dbv )) break;
					JFreeVariant( &dbv );
					JDeleteSetting( hContact, text );
				}
				nEmail++;
			}
		}
		else {
			while ( true ) {
				char text[ 100 ];
				sprintf( text, "e-mail%d", nEmail );
				if ( DBGetContactSetting( NULL, jabberProtoName, text, &dbv )) break;
				JFreeVariant( &dbv );
				JDeleteSetting( NULL, text );
				sprintf( text, "e-mailFlag%d", nEmail );
				JDeleteSetting( NULL, text );
				nEmail++;
		}	}

		if ( !hasBday ) {
			JDeleteSetting( hContact, "BirthYear" );
			JDeleteSetting( hContact, "BirthMonth" );
			JDeleteSetting( hContact, "BirthDay" );
			JDeleteSetting( hContact, "BirthDate" );
		}
		if ( !hasGender ) {
			if ( hContact != NULL )
				JDeleteSetting( hContact, "Gender" );
			else
				JDeleteSetting( NULL, "GenderString" );
		}
		if ( hContact != NULL ) {
			if ( !hasPhone )
				JDeleteSetting( hContact, "Phone" );
			if ( !hasFax )
				JDeleteSetting( hContact, "Fax" );
			if ( !hasCell )
				JDeleteSetting( hContact, "Cellular" );
		}
		else {
			while ( true ) {
				char text[ 100 ];
				sprintf( text, "Phone%d", nPhone );
				if ( DBGetContactSetting( NULL, jabberProtoName, text, &dbv )) break;
				JFreeVariant( &dbv );
				JDeleteSetting( NULL, text );
				sprintf( text, "PhoneFlag%d", nPhone );
				JDeleteSetting( NULL, text );
				nPhone++;
		}	}

		if ( !hasHomeStreet )
			JDeleteSetting( hContact, "Street" );
		if ( !hasHomeStreet2 && hContact==NULL )
			JDeleteSetting( hContact, "Street2" );
		if ( !hasHomeLocality )
			JDeleteSetting( hContact, "City" );
		if ( !hasHomeRegion )
			JDeleteSetting( hContact, "State" );
		if ( !hasHomePcode )
			JDeleteSetting( hContact, "ZIP" );
		if ( !hasHomeCtry ) {
			if ( hContact != NULL )
				JDeleteSetting( hContact, "Country" );
			else
				JDeleteSetting( hContact, "CountryName" );
		}
		if ( !hasWorkStreet )
			JDeleteSetting( hContact, "CompanyStreet" );
		if ( !hasWorkStreet2 && hContact==NULL )
			JDeleteSetting( hContact, "CompanyStreet2" );
		if ( !hasWorkLocality )
			JDeleteSetting( hContact, "CompanyCity" );
		if ( !hasWorkRegion )
			JDeleteSetting( hContact, "CompanyState" );
		if ( !hasWorkPcode )
			JDeleteSetting( hContact, "CompanyZIP" );
		if ( !hasWorkCtry ) {
			if ( hContact != NULL )
				JDeleteSetting( hContact, "CompanyCountry" );
			else
				JDeleteSetting( hContact, "CompanyCountryName" );
		}
		if ( !hasUrl )
			JDeleteSetting( hContact, "Homepage" );
		if ( !hasOrgname )
			JDeleteSetting( hContact, "Company" );
		if ( !hasOrgunit )
			JDeleteSetting( hContact, "CompanyDepartment" );
		if ( !hasRole )
			JDeleteSetting( hContact, "Role" );
		if ( !hasTitle )
			JDeleteSetting( hContact, "CompanyPosition" );
		if ( !hasDesc )
			JDeleteSetting( hContact, "About" );
		if ( !hasPhoto && jabberVcardPhotoFileName!=NULL ) {
			DeleteFileA( jabberVcardPhotoFileName );
			jabberVcardPhotoFileName = NULL;
		}

		if ( id == jabberThreadInfo->resolveID ) {
			TCHAR* p = _tcschr( jid, '@' );
			JabberResolveTransportNicks(( p != NULL ) ?  p+1 : jid );
		}
		else if ( hContact != NULL )
			JSendBroadcast( hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, ( HANDLE ) 1, 0 );
		else if ( hwndJabberVcard )
			SendMessage( hwndJabberVcard, WM_JABBER_REFRESH, 0, 0 );
	}
	else if ( !lstrcmp( type, _T("error"))) {
		if ( hContact != NULL )
			JSendBroadcast( hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, ( HANDLE ) 1, 0 );
}	}

void JabberIqResultSetVcard( XmlNode *iqNode, void *userdata )
{
	JabberLog( "<iq/> iqIdSetVcard" );
	TCHAR* type = JabberXmlGetAttrValue( iqNode, "type" );
	if ( type == NULL )
		return;

	if ( hwndJabberVcard )
		SendMessage( hwndJabberVcard, WM_JABBER_REFRESH, 0, 0 );
}

void JabberIqResultSetSearch( XmlNode *iqNode, void *userdata )
{
	XmlNode *queryNode, *itemNode, *n;
	TCHAR* type, *jid;
	int i, id;
	JABBER_SEARCH_RESULT jsr;

	JabberLog( "<iq/> iqIdGetSearch" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( id = JabberGetPacketID( iqNode )) == -1 ) return;

	if ( !lstrcmp( type, _T("result"))) {
		if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;
		jsr.hdr.cbSize = sizeof( JABBER_SEARCH_RESULT );
		for ( i=0; i<queryNode->numChild; i++ ) {
			itemNode = queryNode->child[i];
			if ( !lstrcmpA( itemNode->name, "item" )) {
				if (( jid=JabberXmlGetAttrValue( itemNode, "jid" )) != NULL ) {
					_tcsncpy( jsr.jid, jid, SIZEOF( jsr.jid ));
					jsr.jid[ SIZEOF( jsr.jid )-1] = '\0';
					JabberLog( "Result jid = " TCHAR_STR_PARAM, jid );
					if (( n=JabberXmlGetChild( itemNode, "nick" ))!=NULL && n->text!=NULL )
						jsr.hdr.nick = t2a( n->text );
					else
						jsr.hdr.nick = mir_strdup( "" );
					if (( n=JabberXmlGetChild( itemNode, "first" ))!=NULL && n->text!=NULL )
						jsr.hdr.firstName = t2a( n->text );
					else
						jsr.hdr.firstName = mir_strdup( "" );
					if (( n=JabberXmlGetChild( itemNode, "last" ))!=NULL && n->text!=NULL )
						jsr.hdr.lastName = t2a( n->text );
					else
						jsr.hdr.lastName = mir_strdup( "" );
					if (( n=JabberXmlGetChild( itemNode, "email" ))!=NULL && n->text!=NULL )
						jsr.hdr.email = t2a( n->text );
					else
						jsr.hdr.email = mir_strdup( "" );
					JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, ( HANDLE ) id, ( LPARAM )&jsr );
					mir_free( jsr.hdr.nick );
					mir_free( jsr.hdr.firstName );
					mir_free( jsr.hdr.lastName );
					mir_free( jsr.hdr.email );
		}	}	}

		JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) id, 0 );
	}
	else if ( !lstrcmp( type, _T("error")))
		JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) id, 0 );
}

void JabberIqResultExtSearch( XmlNode *iqNode, void *userdata )
{
	XmlNode *queryNode;
	TCHAR* type;
	int id;

	JabberLog( "<iq/> iqIdGetExtSearch" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( id = JabberGetPacketID( iqNode )) == -1 ) return;

	if ( !lstrcmp( type, _T("result"))) {
		if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;
		if (( queryNode=JabberXmlGetChild( queryNode, "x" )) == NULL ) return;
		for ( int i=0; i<queryNode->numChild; i++ ) {
			XmlNode* itemNode = queryNode->child[i];
			if ( strcmp( itemNode->name, "item" ))
				continue;

			JABBER_SEARCH_RESULT jsr = { 0 };
			jsr.hdr.cbSize = sizeof( JABBER_SEARCH_RESULT );
//			jsr.hdr.firstName = "";

			for ( int j=0; j < itemNode->numChild; j++ ) {
				XmlNode* fieldNode = itemNode->child[j];
				if ( strcmp( fieldNode->name, "field" ))
					continue;

				TCHAR* fieldName = JabberXmlGetAttrValue( fieldNode, "var" );
				if ( fieldName == NULL )
					continue;

				XmlNode* n = JabberXmlGetChild( fieldNode, "value" );
				if ( n == NULL )
					continue;

				if ( !lstrcmp( fieldName, _T("jid"))) {
					_tcsncpy( jsr.jid, n->text, SIZEOF( jsr.jid ));
					jsr.jid[sizeof( jsr.jid )-1] = '\0';
					JabberLog( "Result jid = " TCHAR_STR_PARAM, jsr.jid );
				}
				else if ( !lstrcmp( fieldName, _T("nickname")))
					jsr.hdr.nick = ( n->text != NULL ) ? t2a( n->text ) : mir_strdup( "" );
				else if ( !lstrcmp( fieldName, _T("fn"))) {
					mir_free( jsr.hdr.firstName );
					jsr.hdr.firstName = ( n->text != NULL ) ? t2a(n->text) : mir_strdup( "" );
				}
				else if ( !lstrcmp( fieldName, _T("given"))) {
					mir_free( jsr.hdr.firstName );
					jsr.hdr.firstName = ( n->text != NULL ) ? t2a(n->text) : mir_strdup( "" );
				}
				else if ( !lstrcmp( fieldName, _T("family")))
					jsr.hdr.lastName = ( n->text != NULL ) ? t2a(n->text) : mir_strdup( "" );
				else if ( !lstrcmp( fieldName, _T("email")))
					jsr.hdr.email = ( n->text != NULL ) ? t2a(n->text) : mir_strdup( "" );
			}

			JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, ( HANDLE ) id, ( LPARAM )&jsr );
			mir_free( jsr.hdr.nick );
			mir_free( jsr.hdr.firstName );
			mir_free( jsr.hdr.lastName );
			mir_free( jsr.hdr.email );
		}

		JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) id, 0 );
	}
	else if ( !lstrcmp( type, _T("error")))
		JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) id, 0 );
}

void JabberIqResultSetPassword( XmlNode *iqNode, void *userdata )
{
	JabberLog( "<iq/> iqIdSetPassword" );

	TCHAR* type = JabberXmlGetAttrValue( iqNode, "type" );
	if ( type == NULL )
		return;

	if ( !lstrcmp( type, _T("result"))) {
		strncpy( jabberThreadInfo->password, jabberThreadInfo->newPassword, SIZEOF( jabberThreadInfo->password ));
		MessagePopup( NULL, TranslateT( "Password is successfully changed. Don't forget to update your password in the Jabber protocol option." ), TranslateT( "Change Password" ), MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND );
	}
	else if ( !lstrcmp( type, _T("error")))
		MessagePopup( NULL, TranslateT( "Password cannot be changed." ), TranslateT( "Change Password" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
}

void JabberIqResultDiscoAgentItems( XmlNode *iqNode, void *userdata )
{
	ThreadData* info = ( ThreadData* ) userdata;
	XmlNode *queryNode, *itemNode;
	TCHAR* type, *jid, *from;

	// RECVED: agent list
	// ACTION: refresh agent list dialog
	JabberLog( "<iq/> iqIdDiscoAgentItems" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( from=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result"))) {
		if (( queryNode=JabberXmlGetChild( iqNode, "query" )) != NULL ) {
			TCHAR* str = JabberXmlGetAttrValue( queryNode, "xmlns" );
			if  ( !lstrcmp( str, _T("http://jabber.org/protocol/disco#items"))) {
				JabberListRemoveList( LIST_AGENT );
				for ( int i=0; i<queryNode->numChild; i++ ) {
					if (( itemNode=queryNode->child[i] )!=NULL && itemNode->name!=NULL && !lstrcmpA( itemNode->name, "item" )) {
						if (( jid=JabberXmlGetAttrValue( itemNode, "jid" )) != NULL ) {
							JABBER_LIST_ITEM* item = JabberListAdd( LIST_AGENT, jid );
							replaceStr( item->name, JabberXmlGetAttrValue( itemNode, "name" ));
							item->cap = AGENT_CAP_REGISTER | AGENT_CAP_GROUPCHAT;	// default to all cap until specific info is later received
							int iqId = JabberSerialNext();
							JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultDiscoAgentInfo );

							XmlNodeIq iq( "get", iqId, jid );
							XmlNode* query = iq.addQuery( "http://jabber.org/protocol/disco#info" );
							JabberSend( jabberThreadInfo->s, iq );
		}	}	}	}	}

		if ( hwndJabberAgents != NULL ) {
			if (( jid=JabberXmlGetAttrValue( iqNode, "from" )) != NULL )
				SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )jid );
			else
				SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )info->server );
		}
	}
	else if ( !lstrcmp( type, _T("error"))) {
		// disco is not supported, try jabber:iq:agents
		int iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_GETAGENTS, JabberIqResultGetAgents );

		XmlNodeIq iq( "get", iqId, from );
		XmlNode* query = iq.addQuery( "jabber:iq:agents" );
		JabberSend( jabberThreadInfo->s, iq );
}	}

void JabberIqResultDiscoAgentInfo( XmlNode *iqNode, void *userdata )
{
	ThreadData* info = ( ThreadData* ) userdata;
	XmlNode *queryNode, *itemNode, *identityNode;
	TCHAR* type, *from, *var;
	JABBER_LIST_ITEM *item;

	// RECVED: info for a specific agent
	// ACTION: refresh agent list dialog
	JabberLog( "<iq/> iqIdDiscoAgentInfo" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( from = JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result"))) {
		if (( queryNode=JabberXmlGetChild( iqNode, "query" )) != NULL ) {
			TCHAR* str = JabberXmlGetAttrValue( queryNode, "xmlns" );
			if ( !lstrcmp( str, _T("http://jabber.org/protocol/disco#info"))) {
				if (( item=JabberListGetItemPtr( LIST_AGENT, from )) != NULL ) {
					// Use the first <identity/> to set name
					if (( identityNode=JabberXmlGetChild( queryNode, "identity" )) != NULL ) {
						if (( str=JabberXmlGetAttrValue( identityNode, "name" )) != NULL )
							replaceStr( item->name, str );
					}

					item->cap = 0;
					for ( int i=0; i<queryNode->numChild; i++ ) {
						if (( itemNode=queryNode->child[i] )!=NULL && itemNode->name!=NULL ) {
							if ( !strcmp( itemNode->name, "feature" )) {
								if (( var=JabberXmlGetAttrValue( itemNode, "var" )) != NULL ) {
									if ( !lstrcmp( var, _T("jabber:iq:register")))
										item->cap |= AGENT_CAP_REGISTER;
									else if ( !lstrcmp( var, _T("http://jabber.org/protocol/muc")))
										item->cap |= AGENT_CAP_GROUPCHAT;
		}	}	}	}	}	}	}

		if ( hwndJabberAgents != NULL )
			SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )NULL );
}	}

void JabberIqResultDiscoClientInfo( XmlNode *iqNode, void *userdata )
{
	ThreadData* info = ( ThreadData* ) userdata;
	XmlNode *queryNode, *itemNode;
	TCHAR* type, *from, *var;
	JABBER_LIST_ITEM *item;

	// RECVED: info for a specific client
	// ACTION: update client cap
	// ACTION: if item->ft is pending, initiate file transfer
	JabberLog( "<iq/> iqIdDiscoClientInfo" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( from=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;

	if ( lstrcmp( type, _T("result")) != 0 )
		return;
	if (( item=JabberListGetItemPtr( LIST_ROSTER, from )) == NULL )
		return;

	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) != NULL ) {
		TCHAR* str = JabberXmlGetAttrValue( queryNode, "xmlns" );
		if ( !lstrcmp( str, _T("http://jabber.org/protocol/disco#info"))) {
			item->cap = CLIENT_CAP_READY;
			for ( int i=0; i<queryNode->numChild; i++ ) {
				if (( itemNode=queryNode->child[i] )!=NULL && itemNode->name!=NULL ) {
					if ( !strcmp( itemNode->name, "feature" )) {
						if (( var=JabberXmlGetAttrValue( itemNode, "var" )) != NULL ) {
							if ( !lstrcmp( var, _T("http://jabber.org/protocol/si")))
								item->cap |= CLIENT_CAP_SI;
							else if ( !lstrcmp( var, _T("http://jabber.org/protocol/si/profile/file-transfer")))
								item->cap |= CLIENT_CAP_SIFILE;
							else if ( !lstrcmp( var, _T("http://jabber.org/protocol/bytestreams")))
								item->cap |= CLIENT_CAP_BYTESTREAM;
	}	}	}	}	}	}

	// Check for pending file transfer session request
	if ( item->ft != NULL ) {
		filetransfer* ft = item->ft;
		item->ft = NULL;
		if (( item->cap & CLIENT_CAP_FILE ) && ( item->cap & CLIENT_CAP_BYTESTREAM ))
			JabberFtInitiate( item->jid, ft );
		else
			mir_forkthread(( pThreadFunc )JabberFileServerThread, ft );
}	}

void JabberIqResultGetAvatar( XmlNode *iqNode, void *userdata )
{
	if ( !JGetByte( "EnableAvatars", TRUE ))
		return;

	ThreadData* info = ( ThreadData* ) userdata;
	TCHAR* type;

	// RECVED: agent list
	// ACTION: refresh agent list dialog
	JabberLog( "<iq/> iqIdResultGetAvatar" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL )   return;
	if ( _tcscmp( type, _T("result")))                              return;

	TCHAR* from = JabberXmlGetAttrValue( iqNode, "from" );
	if ( from == NULL )
		return;
	HANDLE hContact = JabberHContactFromJID( from );
	if ( hContact == NULL )
		return;
	XmlNode* n = NULL;
	TCHAR* mimeType = NULL;
	if (JGetByte(hContact,"AvatarXVcard",0)){
		XmlNode *vCard = JabberXmlGetChild( iqNode, "vCard" );
		if (vCard == NULL) return;
		vCard = JabberXmlGetChild( vCard, "PHOTO" );
		if (vCard == NULL) return;
		XmlNode *typeNode = JabberXmlGetChild( vCard, "TYPE" );
		if (typeNode != NULL) mimeType = typeNode->text;
		n = JabberXmlGetChild( vCard, "BINVAL" );
	}else {
		XmlNode *queryNode = JabberXmlGetChild( iqNode, "query" );
		if ( queryNode == NULL )
			return;

		TCHAR* xmlns = JabberXmlGetAttrValue( queryNode, "xmlns" );
		if ( lstrcmp( xmlns, _T("jabber:iq:avatar")))
			return;

		mimeType = JabberXmlGetAttrValue( n, "mimetype" );

		n = JabberXmlGetChild( queryNode, "data" );
	}
	if ( n == NULL )
		return;
	
	int resultLen = 0;
	char* body = JabberBase64Decode( n->text, &resultLen );

	int pictureType;
	if ( mimeType != NULL ) {
		if ( !lstrcmp( mimeType, _T("image/jpeg")))     pictureType = PA_FORMAT_JPEG;
		else if ( !lstrcmp( mimeType, _T("image/png"))) pictureType = PA_FORMAT_PNG;
		else if ( !lstrcmp( mimeType, _T("image/gif"))) pictureType = PA_FORMAT_GIF;
		else if ( !lstrcmp( mimeType, _T("image/bmp"))) pictureType = PA_FORMAT_BMP;
		else {
LBL_ErrFormat:
			JabberLog( "Invalid mime type specified for picture: " TCHAR_STR_PARAM, mimeType );
			mir_free( body );
			return;
	}	}
	else if (( pictureType = JabberGetPictureType( body )) == PA_FORMAT_UNKNOWN )
		goto LBL_ErrFormat;

	PROTO_AVATAR_INFORMATION AI;
	AI.cbSize = sizeof AI;
	AI.format = pictureType;
	AI.hContact = hContact;

	if ( JGetByte( hContact, "AvatarType", PA_FORMAT_UNKNOWN ) != (unsigned char)pictureType ) {
		JabberGetAvatarFileName( hContact, AI.filename, sizeof AI.filename );
		DeleteFileA( AI.filename );
	}

	JSetByte( hContact, "AvatarType", pictureType );

	char buffer[ 41 ];
	mir_sha1_byte_t digest[20];
	mir_sha1_ctx sha;
	mir_sha1_init( &sha );
	mir_sha1_append( &sha, ( mir_sha1_byte_t* )body, resultLen );
	mir_sha1_finish( &sha, digest );
	for ( int i=0; i<20; i++ )
		sprintf( buffer+( i<<1 ), "%02x", digest[i] );
	JSetString( hContact, "AvatarSaved", buffer );

	JabberGetAvatarFileName( hContact, AI.filename, sizeof AI.filename );

	DBWriteContactSettingString( hContact, "ContactPhoto", "File", AI.filename );

	FILE* out = fopen( AI.filename, "wb" );
	if ( out != NULL ) {
		fwrite( body, resultLen, 1, out );
		fclose( out );
		JSendBroadcast( hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, HANDLE( &AI ), NULL );
		JabberLog("Broadcast new avatar: %s",AI.filename);
	}
	else JSendBroadcast( hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, HANDLE( &AI ), NULL );

	mir_free( body );
}

/////////////////////////////////////////////////////////////////////////////////////////
// Bookmarks

void JabberIqResultDiscoBookmarks( XmlNode *iqNode, void *userdata )
{
	ThreadData* info = ( ThreadData* ) userdata;
	XmlNode *queryNode, *itemNode, *storageNode, *nickNode, *passNode, *errorNode;
	TCHAR* type, *jid;

	// RECVED: list of bookmarks
	// ACTION: refresh bookmarks dialog
	JabberLog( "<iq/> iqIdGetBookmarks" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result"))) {
		if (( queryNode = JabberXmlGetChild( iqNode, "query" )) != NULL ) {
			if ((storageNode = JabberXmlGetChild( queryNode, "storage" )) != NULL) {
				JabberListRemoveList( LIST_BOOKMARK );
				for ( int i=0; i<storageNode->numChild; i++ ) {
					if (( itemNode = storageNode->child[i] ) != NULL && itemNode->name != NULL) {
						if ( !strcmp( itemNode->name, "conference" )) {
							if (( jid = JabberXmlGetAttrValue( itemNode, "jid" )) != NULL ) {
								JABBER_LIST_ITEM* item = JabberListAdd( LIST_BOOKMARK, jid );
								item->name = mir_tstrdup( JabberXmlGetAttrValue( itemNode, "name" ));
								item->type = mir_tstrdup( _T( "conference" ));
								if ( JabberXmlGetAttrValue( itemNode, "autojoin" ) != NULL ) {
									TCHAR* autoJ = JabberXmlGetAttrValue( itemNode, "autojoin" );
									item->bAutoJoin = ( !lstrcmp( autoJ, _T("true")) || !lstrcmp( autoJ, _T("1"))) ? true : false;
								}
								if (( nickNode = JabberXmlGetChild( itemNode, "nick" )) != NULL && nickNode->text != NULL )
									replaceStr( item->nick, nickNode->text );
								if (( passNode = JabberXmlGetChild( itemNode, "password" )) != NULL && passNode->text != NULL )
									replaceStr( item->password, passNode->text );
						}	}

						if ( !strcmp( itemNode->name, "url" )) {
							if (( jid = JabberXmlGetAttrValue( itemNode, "url" )) != NULL ) {
								JABBER_LIST_ITEM* item = JabberListAdd( LIST_BOOKMARK, jid );
								item->name = mir_tstrdup( JabberXmlGetAttrValue( itemNode, "name" ));
								item->type = _T("url");
			}	}	}	}	}

			if ( JGetByte( "AutoJoinBookmarks", FALSE ) == TRUE && !( info->caps & CAPS_BOOKMARKS_LOADED )) {
				if ( LIST_BOOKMARK != NULL ) {
					JABBER_LIST_ITEM* item;
					for ( int i=0; ( i = JabberListFindNext( LIST_BOOKMARK, i )) >= 0; i++ ) {
						if ((( item = JabberListGetItemPtrFromIndex( i )) != NULL ) && !lstrcmp( item->type, _T("conference") )) {
							if ( item->bAutoJoin && JabberListGetItemPtr( LIST_ROOM, item->jid ) == NULL ) {
								if ( jabberChatDllPresent ) {
									TCHAR room[256], *server, *p;
									TCHAR text[128];
									TCHAR* pass;
									_tcsncpy( text, item->jid, SIZEOF( text ));
									_tcsncpy( room, text, SIZEOF( room ));
									p = _tcstok( room, _T( "@" ));
									server = _tcstok( NULL, _T( "@" ));
									pass = mir_tstrdup(( item->password && item->password[0] != 0 ) ? item->password : _T(""));
									if ( item->nick && item->nick[0] != 0 )
										JabberGroupchatJoinRoom( server, p, item->nick, pass );
									else {
										TCHAR* nick = JabberNickFromJID( jabberJID );
										JabberGroupchatJoinRoom( server, p, nick, pass );
										mir_free( nick );
			}	}	}	}	}	}	}

			if ( hwndJabberBookmarks != NULL )
				SendMessage( hwndJabberBookmarks, WM_JABBER_REFRESH, 0, 0);
			info->caps |= CAPS_BOOKMARKS_LOADED;
		}
	}
	else if ( !lstrcmp( type, _T("error"))) {
		if ( info->caps & CAPS_BOOKMARK ) {
			info->caps &= ~CAPS_BOOKMARK;
			JabberEnableMenuItems( TRUE );
			return;
		}
		errorNode = JabberXmlGetChild( iqNode, "error" );
		TCHAR* str = JabberErrorMsg( errorNode );
		MessageBox( NULL, str, TranslateT( "Jabber Bookmarks Error" ), MB_OK|MB_SETFOREGROUND );
		mir_free( str );
		if ( hwndJabberBookmarks != NULL )
			SendMessage( hwndJabberBookmarks, WM_JABBER_ACTIVATE, 0, 0);
}	}

void JabberSetBookmarkRequest (XmlNodeIq& iq)
{
	XmlNode* query = iq.addQuery( "jabber:iq:private" );
	XmlNode* storage = query->addChild( "storage" );
	storage->addAttr( "xmlns", "storage:bookmarks" );

	for ( int i=0; ( i=JabberListFindNext( LIST_BOOKMARK, i )) >= 0; i++ ) {
		JABBER_LIST_ITEM* item = JabberListGetItemPtrFromIndex( i );
		if ( item == NULL )
			continue;

		if ( item->jid == NULL )
			continue;
		if (!lstrcmp( item->type, _T("conference") )) {
			XmlNode* itemNode = storage->addChild("conference");
			itemNode->addAttr( "jid", item->jid );
			if ( item->name )
				itemNode->addAttr( "name", item->name );
			if ( item->bAutoJoin )
				itemNode->addAttr( "autojoin", _T("1") );
			if ( item->nick )
				itemNode->addChild( "nick", item->nick );
			if ( item->password )
				itemNode->addChild( "password", item->password );
		}
		if (!lstrcmp( item->type, _T("url") )) {
			XmlNode* itemNode = storage->addChild("url");
			itemNode->addAttr( "url", item->jid );
			if ( item->name )
				itemNode->addAttr( "name", item->name );
		}
	}
}

void JabberIqResultSetBookmarks( XmlNode *iqNode, void *userdata )
{
	// RECVED: server's responce
	// ACTION: refresh bookmarks list dialog

	JabberLog( "<iq/> iqIdSetBookmarks" );

	TCHAR* type = JabberXmlGetAttrValue( iqNode, "type" );
	if ( type == NULL )
		return;

	if ( !lstrcmp( type, _T("result"))) {
		if ( hwndJabberBookmarks != NULL )
			SendMessage( hwndJabberBookmarks, WM_JABBER_REFRESH, 0, 0);
	}
	else if ( !lstrcmp( type, _T("error"))) {
		XmlNode* errorNode = JabberXmlGetChild( iqNode, "error" );
		TCHAR* str = JabberErrorMsg( errorNode );
		MessagePopup( NULL, str, TranslateT( "Jabber Bookmarks Error" ), MB_OK|MB_SETFOREGROUND );
		mir_free( str );
		if ( hwndJabberBookmarks != NULL )
			SendMessage( hwndJabberBookmarks, WM_JABBER_ACTIVATE, 0, 0);
}	}
