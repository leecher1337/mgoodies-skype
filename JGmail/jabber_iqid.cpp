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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_iqid.cpp,v $
Revision       : $Revision: 1.37 $
Last change on : $Date: 2006/01/22 20:05:00 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "resource.h"
#include "jabber_list.h"
#include "jabber_iq.h"
#include "sha1.h"

extern char* streamId;
extern char* jabberVcardPhotoFileName;
extern char* jabberVcardPhotoType;

/* this is not needed anylonger - new Auth procedure
void JabberIqResultGetAuth( XmlNode *iqNode, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	XmlNode *queryNode;
	char* type;
	char* str;
	char text[256];

	// RECVED: result of the request for authentication method
	// ACTION: send account authentication information to log in
	JabberLog( "<iq/> iqIdGetAuth" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;

	if ( !strcmp( type, "result" )) {
		if ( JabberXmlGetChild( queryNode, "digest" )!=NULL && streamId ) {
			if (( str=JabberUtf8Encode( info->password )) != NULL ) {
				wsprintfA( text, "%s%s", streamId, str );
				free( str );
				if (( str=JabberSha1( text )) != NULL ) {
					mir_snprintf( text, sizeof( text ), "<digest>%s</digest>", str );
					free( str );
			}	}
		}
		else if ( JabberXmlGetChild( queryNode, "password" ) != NULL ) {
			mir_snprintf( text, sizeof( text ), "<password>%s</password>", TXT( info->password ));
		}
		else {
			JabberLog( "No known authentication mechanism accepted by the server." );
			JabberSend( info->s, "</stream:stream>" );
			return;
		}
		if ( JabberXmlGetChild( queryNode, "resource" ) != NULL ) {
			if (( str=JabberTextEncode( info->resource )) != NULL ) {
				mir_snprintf( text+strlen( text ), sizeof( text )-strlen( text ), "<resource>%s</resource>", str );
				free( str );
		}	}

		if (( str=JabberTextEncode( info->username )) != NULL ) {
			int iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultSetAuth );
			JabberSend( info->s, "<iq type='set' id='"JABBER_IQID"%d'><query xmlns='jabber:iq:auth'><username>%s</username>%s</query></iq>", iqId, str, text );
			free( str );
		}
	}
	else if ( !strcmp( type, "error" )) {
		JabberSend( info->s, "</stream:stream>" );
}	}

void JabberIqResultSetAuth( XmlNode *iqNode, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	char* type;
	int iqId;

	// RECVED: authentication result
	// ACTION: if successfully logged in, continue by requesting roster list and set my initial status
	JabberLog( "<iq/> iqIdSetAuth" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;

	if ( !strcmp( type, "result" )) {
		DBVARIANT dbv;

		if ( DBGetContactSetting( NULL, jabberProtoName, "Nick", &dbv ))
			JSetString( NULL, "Nick", info->username );
		else
			JFreeVariant( &dbv );
		iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetRoster );
		JabberSend( info->s, "<iq type='get' id='"JABBER_IQID"%d'><query xmlns='jabber:iq:roster'/></iq>", iqId );
		if ( hwndJabberAgents ) {
			// Retrieve agent information
			iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_GETAGENTS, JabberIqResultGetAgents );
			JabberSend( info->s, "<iq type='get' id='"JABBER_IQID"%d'><query xmlns='jabber:iq:agents'/></iq>", iqId );
		}
	}
	// What to do if password error? etc...
	else if ( !strcmp( type, "error" )) {
		char text[128];

		JabberSend( info->s, "</stream:stream>" );
		mir_snprintf( text, sizeof( text ), "%s %s@%s.", JTranslate( "Authentication failed for" ), info->username, info->server );
		MessageBoxA( NULL, text, JTranslate( "Jabber Authentication" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
		JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD );
		jabberThreadInfo = NULL;	// To disallow auto reconnect
}	}
*/
void JabberIqResultBind( XmlNode *iqNode, void *userdata )
{
//	JabberXmlDumpNode( iqNode );
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	int iqId;
	XmlNode* queryNode = JabberXmlGetChild( iqNode, "bind" );
	if (queryNode){
//		JabberLog("Has query node");
		if (queryNode=JabberXmlGetChild( queryNode, "jid" )){
//			JabberLog("Has query jid");
			if (queryNode->text) {
//				JabberLog("JID has text");
//				JabberLog("text: %s",queryNode->text);
				if (!strncmp(info->fullJID,queryNode->text,sizeof (info->fullJID))){
					JabberLog( "Result Bind: %s %s %s",info->fullJID,"confirmed.",NULL);
				} else {
					JabberLog( "Result Bind: %s %s %s",info->fullJID,"changed to",queryNode->text);
					strncpy(info->fullJID,queryNode->text,sizeof (info->fullJID));
			}	}
		} else if (queryNode=JabberXmlGetChild( queryNode, "error" )){
			//rfc3920 page 39
			char errorMessage [256];
			int pos=0;
			pos = mir_snprintf(errorMessage,256,Translate("Resource "));
			XmlNode *tempNode;
			if (tempNode = JabberXmlGetChild( queryNode, "resource" )) pos += mir_snprintf(errorMessage,256-pos,"\"%s\" ",tempNode->text);
			pos += mir_snprintf(errorMessage,256-pos,Translate("refused by server\n%s: %s"),Translate("Type"),Translate(JabberXmlGetAttrValue( queryNode, "type" )));
			if (queryNode->numChild) pos += mir_snprintf(errorMessage+pos,256-pos,"\n%s: %s\n",Translate("Reason"),Translate(queryNode->child[0]->name));
			mir_snprintf( errorMessage,256-pos, "%s %s@%s.", JTranslate( "Authentication failed for" ), info->username, info->server );
			MessageBoxA( NULL, errorMessage, JTranslate( "Jabber Protocol" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
			JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPROTOCOL );
			JabberSend( info->s, "</stream:stream>" );
			jabberThreadInfo = NULL;	// To disallow auto reconnect
		}
	}
	{
		int i = JGetByte(NULL,"EnableGMail",1);
		if (i & 1) {
			JabberEnableNotifications(info);
			if ((i & 2) == 0) JabberRequestMailBox(info->s);
		}
	}
	iqId = JabberSerialNext();
	JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetRoster );
	JabberSend( info->s, "<iq type='get' id='"JABBER_IQID"%d'><query xmlns='jabber:iq:roster'/></iq>", iqId );
	if ( hwndJabberAgents ) {
		// Retrieve agent information
		iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_GETAGENTS, JabberIqResultGetAgents );
		JabberSend( info->s, "<iq type='get' id='"JABBER_IQID"%d'><query xmlns='jabber:iq:agents'/></iq>", iqId );
	}
}

void CALLBACK sttCreateRoom( ULONG dwParam )
{
	char* jid = ( char* )dwParam, *p;

	GCSESSION gcw = {0};
	gcw.cbSize = sizeof(GCSESSION);
	gcw.iType = GCW_CHATROOM;
	gcw.pszID = jid;
	gcw.pszModule = jabberProtoName;
	gcw.pszName = strcpy(( char* )alloca( strlen(jid)+1 ), jid );
	if (( p = (char*)strchr( gcw.pszName, '@' )) != NULL )
		*p = 0;
	CallService( MS_GC_NEWSESSION, 0, ( LPARAM )&gcw );
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetRoster - populates LIST_ROSTER and creates contact for any new rosters

void JabberIqResultGetRoster( XmlNode* iqNode, void* )
{
	JabberLog( "<iq/> iqIdGetRoster" );
	char* type = JabberXmlGetAttrValue( iqNode, "type" );
	if ( lstrcmpA( type, "result" ))
		return;

	XmlNode* queryNode = JabberXmlGetChild( iqNode, "query" );
   if ( queryNode == NULL )
		return;

	if ( lstrcmpA( JabberXmlGetAttrValue( queryNode, "xmlns" ), "jabber:iq:roster" ))
		return;

	char* name, *nick;
	int i;

	for ( i=0; i < queryNode->numChild; i++ ) {
		XmlNode* itemNode = queryNode->child[i];
		if ( strcmp( itemNode->name, "item" ))
			continue;

		char* str = JabberXmlGetAttrValue( itemNode, "subscription" );

		JABBER_SUBSCRIPTION sub;
		if ( str == NULL ) sub = SUB_NONE;
		else if ( !strcmp( str, "both" )) sub = SUB_BOTH;
		else if ( !strcmp( str, "to" )) sub = SUB_TO;
		else if ( !strcmp( str, "from" )) sub = SUB_FROM;
		else sub = SUB_NONE;

		char* jid = JabberXmlGetAttrValue( itemNode, "jid" );
		if ( jid == NULL )
			continue;

		JabberUrlDecode( jid );
		if (( name = JabberXmlGetAttrValue( itemNode, "name" )) != NULL )
			nick = strdup( JabberUrlDecode( name ));
		else
			nick = JabberNickFromJID( jid );

		if ( nick == NULL )
			continue;

		JABBER_LIST_ITEM* item = JabberListAdd( LIST_ROSTER, jid );
		item->subscription = sub;

		if ( item->nick ) free( item->nick );
		item->nick = nick;

		if ( item->group ) free( item->group );
		XmlNode* groupNode = JabberXmlGetChild( itemNode, "group" );
		if ( groupNode != NULL && groupNode->text != NULL )
			item->group = strdup( JabberUrlDecode( groupNode->text ));
		else
			item->group = NULL;

		HANDLE hContact = JabberHContactFromJID( jid );
		if ( hContact == NULL ) {
			// Received roster has a new JID.
			// Add the jid ( with empty resource ) to Miranda contact list.
			hContact = JabberDBCreateContact( jid, nick, FALSE, TRUE );
		}

		if ( JGetByte( hContact, "ChatRoom", 0 ))
			QueueUserAPC( sttCreateRoom, hMainThread, ( unsigned long )jid );

      DBVARIANT dbNick;
		if ( !JGetStringUtf( hContact, "Nick", &dbNick )) {
			if ( strcmp( nick, dbNick.pszVal ) != 0 )
				DBWriteContactSettingStringUtf( hContact, "CList", "MyHandle", nick );
			else
				DBDeleteContactSetting( hContact, "CList", "MyHandle" );
			JFreeVariant( &dbNick );
		}
		else DBWriteContactSettingStringUtf( hContact, "CList", "MyHandle", nick );

		if ( JGetByte( hContact, "ChatRoom", 0 ))
			DBDeleteContactSetting( hContact, "CList", "Hidden" );

		if ( item->group != NULL ) {
			JabberContactListCreateGroup( item->group );

			// Don't set group again if already correct, or Miranda may show wrong group count in some case
			DBVARIANT dbv;
			if ( !DBGetContactSettingStringUtf( hContact, "CList", "Group", &dbv )) {
				if ( strcmp( dbv.pszVal, item->group ))
					DBWriteContactSettingStringUtf( hContact, "CList", "Group", item->group );
				JFreeVariant( &dbv );
			}
			else DBWriteContactSettingStringUtf( hContact, "CList", "Group", item->group );
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
				if ( !JGetStringUtf( hContact, "jid", &dbv )) {
					if ( !JabberListExist( LIST_ROSTER, dbv.pszVal )) {
						JabberLog( "Syncing roster: preparing to delete %s ( hContact=0x%x )", dbv.pszVal, hContact );
						if ( listSize >= listAllocSize ) {
							listAllocSize = listSize + 100;
							if (( list=( HANDLE * ) realloc( list, listAllocSize * sizeof( HANDLE ))) == NULL ) {
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
			free( list );
	}

	jabberOnline = TRUE;
	JabberEnableMenuItems( TRUE );

	if ( hwndJabberGroupchat )
		SendMessage( hwndJabberGroupchat, WM_JABBER_CHECK_ONLINE, 0, 0 );
	if ( hwndJabberJoinGroupchat )
		SendMessage( hwndJabberJoinGroupchat, WM_JABBER_CHECK_ONLINE, 0, 0 );

	JabberLog( "Status changed via THREADSTART" );
	modeMsgStatusChangePending = FALSE;
	JabberSetServerStatus( jabberDesiredStatus );

	if ( hwndJabberAgents )
		SendMessage( hwndJabberAgents, WM_JABBER_TRANSPORT_REFRESH, 0, 0 );
	if ( hwndJabberVcard )
		SendMessage( hwndJabberVcard, WM_JABBER_CHECK_ONLINE, 0, 0 );
}

void JabberIqResultGetAgents( XmlNode *iqNode, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	XmlNode *queryNode;
	char* type, *jid;
	char* str;

	// RECVED: agent list
	// ACTION: refresh agent list dialog
	JabberLog( "<iq/> iqIdGetAgents" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;

	if ( !strcmp( type, "result" )) {
		str = JabberXmlGetAttrValue( queryNode, "xmlns" );
		if ( str!=NULL && !strcmp( str, "jabber:iq:agents" )) {
			XmlNode *agentNode, *n;
			JABBER_LIST_ITEM *item;
			int i;

			JabberListRemoveList( LIST_AGENT );
			for ( i=0; i<queryNode->numChild; i++ ) {
				agentNode = queryNode->child[i];
				if ( !strcmp( agentNode->name, "agent" )) {
					if (( jid=JabberXmlGetAttrValue( agentNode, "jid" )) != NULL ) {
						item = JabberListAdd( LIST_AGENT, JabberUrlDecode( jid ));
						if ( JabberXmlGetChild( agentNode, "register" ) != NULL )
							item->cap |= AGENT_CAP_REGISTER;
						if ( JabberXmlGetChild( agentNode, "search" ) != NULL )
							item->cap |= AGENT_CAP_SEARCH;
						if ( JabberXmlGetChild( agentNode, "groupchat" ) != NULL )
							item->cap |= AGENT_CAP_GROUPCHAT;
						// set service also???
						// most chatroom servers don't announce <grouchat/> so we will
						// also treat <service>public</service> as groupchat services
						if (( n=JabberXmlGetChild( agentNode, "service" ))!=NULL && n->text!=NULL && !strcmp( n->text, "public" ))
							item->cap |= AGENT_CAP_GROUPCHAT;
						if (( n=JabberXmlGetChild( agentNode, "name" ))!=NULL && n->text!=NULL )
							item->name = JabberTextDecode( n->text );
		}	}	}	}

		if ( hwndJabberAgents != NULL ) {
			if (( jid = JabberXmlGetAttrValue( iqNode, "from" )) != NULL )
				SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )jid );
			else
				SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )info->server );
}	}	}

void JabberIqResultGetRegister( XmlNode *iqNode, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	XmlNode *queryNode, *errorNode, *n;
	char* type;
	char* str;

	// RECVED: result of the request for ( agent ) registration mechanism
	// ACTION: activate ( agent ) registration input dialog
	JabberLog( "<iq/> iqIdGetRegister" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;

	if ( !strcmp( type, "result" )) {
		if ( hwndAgentRegInput )
			if (( n = JabberXmlCopyNode( iqNode )) != NULL )
				SendMessage( hwndAgentRegInput, WM_JABBER_REGINPUT_ACTIVATE, 1 /*success*/, ( LPARAM )n );
	}
	else if ( !strcmp( type, "error" )) {
		if ( hwndAgentRegInput ) {
			errorNode = JabberXmlGetChild( iqNode, "error" );
			str = JabberErrorMsg( errorNode );
			SendMessage( hwndAgentRegInput, WM_JABBER_REGINPUT_ACTIVATE, 0 /*error*/, ( LPARAM )str );
			free( str );
}	}	}

void JabberIqResultSetRegister( XmlNode *iqNode, void *userdata )
{
	XmlNode *errorNode;
	char* type;
	char* str;

	// RECVED: result of registration process
	// ACTION: notify of successful agent registration
	JabberLog( "<iq/> iqIdSetRegister" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;

	if ( !strcmp( type, "result" )) {
		if ( hwndRegProgress )
			SendMessage( hwndRegProgress, WM_JABBER_REGDLG_UPDATE, 100, ( LPARAM )JTranslate( "Registration successful" ));
	}
	else if ( !strcmp( type, "error" )) {
		if ( hwndRegProgress ) {
			errorNode = JabberXmlGetChild( iqNode, "error" );
			str = JabberErrorMsg( errorNode );
			SendMessage( hwndRegProgress, WM_JABBER_REGDLG_UPDATE, 100, ( LPARAM )str );
			free( str );
}	}	}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultGetVcard - processes the server-side v-card

static void JabberIqResultGetVcardPhoto( const char* jid, XmlNode* n, HANDLE hContact, BOOL& hasPhoto )
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
		if ( strcmp( m->text, "image/jpeg" ) && strcmp( m->text, "image/png" ) && strcmp( m->text, "image/gif" ) && strcmp( m->text, "image/bmp" ))
			goto LBL_NoTypeSpecified;

		replaceStr( jabberVcardPhotoType, m->text );
	}

	DWORD nWritten;
	char szTempPath[MAX_PATH], szTempFileName[MAX_PATH];
	JABBER_LIST_ITEM *item;
	DBVARIANT dbv;

	if ( GetTempPathA( sizeof( szTempPath ), szTempPath ) <= 0 )
		lstrcpyA( szTempPath, ".\\" );
	if ( !GetTempFileNameA( szTempPath, "jab", 0, szTempFileName )) {
LBL_Ret:	
		free( buffer );
		return;
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
			free( jabberVcardPhotoFileName );
		}
		replaceStr( jabberVcardPhotoFileName, szTempFileName );
		JabberLog( "My picture saved to %s", szTempFileName );
	}
	else if ( !JGetStringUtf( hContact, "jid", &dbv )) {
		if (( item = JabberListGetItemPtr( LIST_ROSTER, jid )) != NULL ) {
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

void JabberIqResultGetVcard( XmlNode *iqNode, void *userdata )
{
	XmlNode *vCardNode, *m, *n, *o;
	char* type, *jid;
	HANDLE hContact;
	char text[128];
	int len;
	DBVARIANT dbv;

	JabberLog( "<iq/> iqIdGetVcard" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( jid=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;

	JabberUrlDecode( jid );
	len = strlen( jabberJID );
	if ( !strnicmp( jid, jabberJID, len ) && ( jid[len]=='/' || jid[len]=='\0' )) {
		hContact = NULL;
		JabberLog( "Vcard for myself" );
	}
	else {
		if (( hContact = JabberHContactFromJID( jid )) == NULL )
			return;
		JabberLog( "Other user's vcard" );
	}

	if ( !strcmp( type, "result" )) {
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
						JSetStringUtf( hContact, "FullName", JabberUrlDecode( n->text ));
					}
				}
				else if ( !strcmp( n->name, "NICKNAME" )) {
					if ( n->text != NULL ) {
						hasNick = TRUE;
						JSetStringUtf( hContact, "Nick", JabberUrlDecode( n->text ));
					}
				}
				else if ( !strcmp( n->name, "N" )) {
					// First/Last name
					if ( !hasGiven && !hasFamily && !hasMiddle ) {
						if (( m=JabberXmlGetChild( n, "GIVEN" )) != NULL && m->text!=NULL ) {
							hasGiven = TRUE;
							JSetStringUtf( hContact, "FirstName", JabberUrlDecode( m->text ));
						}
						if (( m=JabberXmlGetChild( n, "FAMILY" )) != NULL && m->text!=NULL ) {
							hasFamily = TRUE;
							JSetStringUtf( hContact, "LastName", JabberUrlDecode( m->text ));
						}
						if (( m=JabberXmlGetChild( n, "MIDDLE" )) != NULL && m->text != NULL ) {
							hasMiddle = TRUE;
							JSetStringUtf( hContact, "MiddleName", JabberUrlDecode( m->text ));
					}	}
				}
				else if ( !strcmp( n->name, "EMAIL" )) {
					// E-mail address( es )
					if (( m=JabberXmlGetChild( n, "USERID" )) == NULL )	// Some bad client put e-mail directly in <EMAIL/> instead of <USERID/>
						m = n;
					if ( m->text != NULL ) {
						if ( hContact != NULL ) {
							if ( nEmail == 0 )
								strcpy( text, "e-mail" );
							else
								sprintf( text, "e-mail%d", nEmail-1 );
						}
						else sprintf( text, "e-mail%d", nEmail );
						JSetStringUtf( hContact, text, JabberUrlDecode( m->text ));

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
							if ( sscanf( n->text, "%d-%d-%d", &nYear, &nMonth, &nDay ) == 3 ) {
								hasBday = TRUE;
								JSetWord( hContact, "BirthYear", ( WORD )nYear );
								JSetByte( hContact, "BirthMonth", ( BYTE ) nMonth );
								JSetByte( hContact, "BirthDay", ( BYTE ) nDay );
							}
						}
						else {
							hasBday = TRUE;
							JSetStringUtf( NULL, "BirthDate", JabberUrlDecode( n->text ));
					}	}
				}
				else if ( !strcmp( n->name, "GENDER" )) {
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
							JSetStringUtf( NULL, "GenderString", JabberUrlDecode( n->text ));
					}	}
				}
				else if ( !strcmp( n->name, "ADR" )) {
					if ( !hasHome && JabberXmlGetChild( n, "HOME" )!=NULL ) {
						// Home address
						hasHome = TRUE;
						if (( m=JabberXmlGetChild( n, "STREET" )) != NULL && m->text != NULL ) {
							hasHomeStreet = TRUE;
							JabberUrlDecode( m->text );
							if ( hContact != NULL ) {
								if (( o=JabberXmlGetChild( n, "EXTADR" )) != NULL && o->text != NULL )
									mir_snprintf( text, sizeof( text ), "%s\r\n%s", m->text, JabberUrlDecode( o->text ));
								else if (( o=JabberXmlGetChild( n, "EXTADD" ))!=NULL && o->text!=NULL )
									mir_snprintf( text, sizeof( text ), "%s\r\n%s", m->text, JabberUrlDecode( o->text ));
								else
									strncpy( text, m->text, sizeof( text ));
								text[sizeof( text )-1] = '\0';
								JSetStringUtf( hContact, "Street", text );
							}
							else {
								JSetStringUtf( hContact, "Street", m->text );
								if (( m=JabberXmlGetChild( n, "EXTADR" )) == NULL )
									m = JabberXmlGetChild( n, "EXTADD" );
								if ( m!=NULL && m->text!=NULL ) {
									hasHomeStreet2 = TRUE;
									JSetStringUtf( hContact, "Street2", JabberUrlDecode( m->text ));
						}	}	}

						if (( m=JabberXmlGetChild( n, "LOCALITY" ))!=NULL && m->text!=NULL ) {
							hasHomeLocality = TRUE;
							JSetStringUtf( hContact, "City", JabberUrlDecode( m->text ));
						}
						if (( m=JabberXmlGetChild( n, "REGION" ))!=NULL && m->text!=NULL ) {
							hasHomeRegion = TRUE;
							JSetStringUtf( hContact, "State", JabberUrlDecode( m->text ));
						}
						if (( m=JabberXmlGetChild( n, "PCODE" ))!=NULL && m->text!=NULL ) {
							hasHomePcode = TRUE;
							JSetStringUtf( hContact, "ZIP", JabberUrlDecode( m->text ));
						}
						if (( m=JabberXmlGetChild( n, "CTRY" ))==NULL || m->text==NULL )	// Some bad client use <COUNTRY/> instead of <CTRY/>
							m = JabberXmlGetChild( n, "COUNTRY" );
						if ( m!=NULL && m->text!=NULL ) {
							hasHomeCtry = TRUE;
							JabberUrlDecode( m->text );
							if ( hContact != NULL )
								JSetWord( hContact, "Country", ( WORD )JabberCountryNameToId( m->text ));
							else
								JSetStringUtf( hContact, "CountryName", m->text );
					}	}

					if ( !hasWork && JabberXmlGetChild( n, "WORK" )!=NULL ) {
						// Work address
						hasWork = TRUE;
						if (( m=JabberXmlGetChild( n, "STREET" ))!=NULL && m->text!=NULL ) {
							hasWorkStreet = TRUE;
							JabberUrlDecode( m->text );
							if ( hContact != NULL ) {
								if (( o=JabberXmlGetChild( n, "EXTADR" ))!=NULL && o->text!=NULL )
									mir_snprintf( text, sizeof( text ), "%s\r\n%s", m->text, JabberUrlDecode( o->text ));
								else if (( o=JabberXmlGetChild( n, "EXTADD" ))!=NULL && o->text!=NULL )
									mir_snprintf( text, sizeof( text ), "%s\r\n%s", m->text, JabberUrlDecode( o->text ));
								else
									strncpy( text, m->text, sizeof( text ));
								text[sizeof( text )-1] = '\0';
								JSetStringUtf( hContact, "CompanyStreet", text );
							}
							else {
								JSetStringUtf( hContact, "CompanyStreet", m->text );
								if (( m=JabberXmlGetChild( n, "EXTADR" )) == NULL )
									m = JabberXmlGetChild( n, "EXTADD" );
								if ( m!=NULL && m->text!=NULL ) {
									hasWorkStreet2 = TRUE;
									JSetStringUtf( hContact, "CompanyStreet2", JabberUrlDecode( m->text ));
						}	}	}

						if (( m=JabberXmlGetChild( n, "LOCALITY" ))!=NULL && m->text!=NULL ) {
							hasWorkLocality = TRUE;
							JSetStringUtf( hContact, "CompanyCity", JabberUrlDecode( m->text ));
						}
						if (( m=JabberXmlGetChild( n, "REGION" ))!=NULL && m->text!=NULL ) {
							hasWorkRegion = TRUE;
							JSetStringUtf( hContact, "CompanyState", JabberUrlDecode( m->text ));
						}
						if (( m=JabberXmlGetChild( n, "PCODE" ))!=NULL && m->text!=NULL ) {
							hasWorkPcode = TRUE;
							JSetStringUtf( hContact, "CompanyZIP", JabberUrlDecode( m->text ));
						}
						if (( m=JabberXmlGetChild( n, "CTRY" ))==NULL || m->text==NULL )	// Some bad client use <COUNTRY/> instead of <CTRY/>
							m = JabberXmlGetChild( n, "COUNTRY" );
						if ( m!=NULL && m->text!=NULL ) {
							hasWorkCtry = TRUE;
							JabberUrlDecode( m->text );
							if ( hContact != NULL )
								JSetWord( hContact, "CompanyCountry", ( WORD )JabberCountryNameToId( m->text ));
							else
								JSetStringUtf( hContact, "CompanyCountryName", m->text );
					}	}
				}
				else if ( !strcmp( n->name, "TEL" )) {
					// Telephone/Fax/Cellular
					if (( m=JabberXmlGetChild( n, "NUMBER" ))!=NULL && m->text!=NULL ) {
						JabberUrlDecode( m->text );
						if ( hContact != NULL ) {
							if ( !hasFax && JabberXmlGetChild( n, "FAX" )!=NULL ) {
								hasFax = TRUE;
								JSetStringUtf( hContact, "Fax", m->text );
							}
							if ( !hasCell && JabberXmlGetChild( n, "CELL" )!=NULL ) {
								hasCell = TRUE;
								JSetStringUtf( hContact, "Cellular", m->text );
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
								JSetString( hContact, "Phone", m->text );
							}
						}
						else {
							sprintf( text, "Phone%d", nPhone );

							JSetString( NULL, text, m->text );
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
						JSetStringUtf( hContact, "Homepage", JabberUrlDecode( n->text ));
					}
				}
				else if ( !strcmp( n->name, "ORG" )) {
					if ( !hasOrgname && !hasOrgunit ) {
						if (( m=JabberXmlGetChild( n, "ORGNAME" ))!=NULL && m->text!=NULL ) {
							hasOrgname = TRUE;
							JSetStringUtf( hContact, "Company", JabberUrlDecode( m->text ));
						}
						if (( m=JabberXmlGetChild( n, "ORGUNIT" ))!=NULL && m->text!=NULL ) {	// The real vCard can have multiple <ORGUNIT/> but we will only display the first one
							hasOrgunit = TRUE;
							JSetStringUtf( hContact, "CompanyDepartment", JabberUrlDecode( m->text ));
					}	}
				}
				else if ( !strcmp( n->name, "ROLE" )) {
					if ( !hasRole && n->text!=NULL ) {
						hasRole = TRUE;
						JSetStringUtf( hContact, "Role", JabberUrlDecode( n->text ));
					}
				}
				else if ( !strcmp( n->name, "TITLE" )) {
					if ( !hasTitle && n->text!=NULL ) {
						hasTitle = TRUE;
						JSetStringUtf( hContact, "CompanyPosition", JabberUrlDecode( n->text ));
					}
				}
				else if ( !strcmp( n->name, "DESC" )) {
					if ( !hasDesc && n->text!=NULL ) {
						hasDesc = TRUE;
						char* szMemo = JabberUnixToDos( n->text );
						JabberUrlDecode( szMemo );
						JSetStringUtf( hContact, "About", szMemo );
						free( szMemo );
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

		if ( hContact != NULL )
			JSendBroadcast( hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, ( HANDLE ) 1, 0 );
		else if ( hwndJabberVcard )
			SendMessage( hwndJabberVcard, WM_JABBER_REFRESH, 0, 0 );
	}
	else if ( !strcmp( type, "error" )) {
		if ( hContact != NULL )
			JSendBroadcast( hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, ( HANDLE ) 1, 0 );
}	}

void JabberIqResultSetVcard( XmlNode *iqNode, void *userdata )
{
	JabberLog( "<iq/> iqIdSetVcard" );
	char* type = JabberXmlGetAttrValue( iqNode, "type" );
	if ( type == NULL )
		return;

	if ( hwndJabberVcard )
		SendMessage( hwndJabberVcard, WM_JABBER_REFRESH, 0, 0 );
}

void JabberIqResultSetSearch( XmlNode *iqNode, void *userdata )
{
	XmlNode *queryNode, *itemNode, *n;
	char* type, *jid, *str;
	int id, i;
	JABBER_SEARCH_RESULT jsr;

	JabberLog( "<iq/> iqIdGetSearch" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( str=JabberXmlGetAttrValue( iqNode, "id" )) == NULL ) return;
	id = atoi( str+strlen( JABBER_IQID ));

	if ( !strcmp( type, "result" )) {
		if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;
		jsr.hdr.cbSize = sizeof( JABBER_SEARCH_RESULT );
		for ( i=0; i<queryNode->numChild; i++ ) {
			itemNode = queryNode->child[i];
			if ( !strcmp( itemNode->name, "item" )) {
				if (( jid=JabberXmlGetAttrValue( itemNode, "jid" )) != NULL ) {
					strncpy( jsr.jid, JabberUrlDecode( jid ), sizeof( jsr.jid ));
					jsr.jid[sizeof( jsr.jid )-1] = '\0';
					JabberLog( "Result jid=%s", jid );
					if (( n=JabberXmlGetChild( itemNode, "nick" ))!=NULL && n->text!=NULL )
						jsr.hdr.nick = JabberTextDecode( n->text );
					else
						jsr.hdr.nick = _strdup( "" );
					if (( n=JabberXmlGetChild( itemNode, "first" ))!=NULL && n->text!=NULL )
						jsr.hdr.firstName = JabberTextDecode( n->text );
					else
						jsr.hdr.firstName = _strdup( "" );
					if (( n=JabberXmlGetChild( itemNode, "last" ))!=NULL && n->text!=NULL )
						jsr.hdr.lastName = JabberTextDecode( n->text );
					else
						jsr.hdr.lastName = _strdup( "" );
					if (( n=JabberXmlGetChild( itemNode, "email" ))!=NULL && n->text!=NULL )
						jsr.hdr.email = JabberTextDecode( n->text );
					else
						jsr.hdr.email = _strdup( "" );
					JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, ( HANDLE ) id, ( LPARAM )&jsr );
					free( jsr.hdr.nick );
					free( jsr.hdr.firstName );
					free( jsr.hdr.lastName );
					free( jsr.hdr.email );
		}	}	}

		JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) id, 0 );
	}
	else if ( !strcmp( type, "error" ))
		JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) id, 0 );
}

void JabberIqResultExtSearch( XmlNode *iqNode, void *userdata )
{
	XmlNode *queryNode;
	char* type, *str;

	JabberLog( "<iq/> iqIdGetExtSearch" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( str=JabberXmlGetAttrValue( iqNode, "id" )) == NULL ) return;
	int id = atoi( str+strlen( JABBER_IQID ));

	if ( !strcmp( type, "result" )) {
		if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;
		if (( queryNode=JabberXmlGetChild( queryNode, "x" )) == NULL ) return;
		for ( int i=0; i<queryNode->numChild; i++ ) {
			XmlNode* itemNode = queryNode->child[i];
			if ( strcmp( itemNode->name, "item" ))
				continue;

			JABBER_SEARCH_RESULT jsr = { 0 };
			jsr.hdr.cbSize = sizeof( JABBER_SEARCH_RESULT );
			jsr.hdr.firstName = "";

			for ( int j=0; j < itemNode->numChild; j++ ) {
				XmlNode* fieldNode = itemNode->child[j];
				if ( strcmp( fieldNode->name, "field" ))
					continue;

				char* fieldName = JabberXmlGetAttrValue( fieldNode, "var" );
				if ( fieldName == NULL )
					continue;

				XmlNode* n = JabberXmlGetChild( fieldNode, "value" );
				if ( n == NULL )
					continue;

				if ( !strcmp( fieldName, "jid" )) {
					strncpy( jsr.jid, JabberUrlDecode( n->text ), sizeof( jsr.jid ));
					jsr.jid[sizeof( jsr.jid )-1] = '\0';
					JabberLog( "Result jid=%s", jsr.jid );
				}
				else if ( !strcmp( fieldName, "nickname" ))
					jsr.hdr.nick = ( n->text != NULL ) ? JabberUtf8Decode( n->text, 0 ) : (char*)"";
				else if ( !strcmp( fieldName, "fn" )) {
					if ( n->text != NULL )
	               jsr.hdr.firstName = JabberUtf8Decode( n->text, 0 );
				}
				else if ( !strcmp( fieldName, "given" )) {
					if ( n->text != NULL )
	               jsr.hdr.firstName = JabberUtf8Decode( n->text, 0 );
				}
				else if ( !strcmp( fieldName, "family" ))
               jsr.hdr.lastName = ( n->text != NULL ) ? JabberUtf8Decode( n->text, 0 ) : (char*)"";
				else if ( !strcmp( fieldName, "email" ))
               jsr.hdr.email = ( n->text != NULL ) ? JabberUtf8Decode( n->text, 0 ) : (char*)"";
			}
			JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, ( HANDLE ) id, ( LPARAM )&jsr );
		}

		JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) id, 0 );
	}
	else if ( !strcmp( type, "error" ))
		JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) id, 0 );
}

void JabberIqResultSetPassword( XmlNode *iqNode, void *userdata )
{
	JabberLog( "<iq/> iqIdSetPassword" );

	char* type = JabberXmlGetAttrValue( iqNode, "type" );
	if ( type == NULL )
		return;

	if ( !strcmp( type, "result" )) {
		strncpy( jabberThreadInfo->password, jabberThreadInfo->newPassword, sizeof( jabberThreadInfo->password ));
		MessageBox( NULL, TranslateT( "Password is successfully changed. Don't forget to update your password in the Jabber protocol option." ), TranslateT( "Change Password" ), MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND );
	}
	else if ( !strcmp( type, "error" ))
		MessageBox( NULL, TranslateT( "Password cannot be changed." ), TranslateT( "Change Password" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
}

void JabberIqResultDiscoAgentItems( XmlNode *iqNode, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	XmlNode *queryNode, *itemNode;
	char* type, *jid, *from;

	// RECVED: agent list
	// ACTION: refresh agent list dialog
	JabberLog( "<iq/> iqIdDiscoAgentItems" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( from=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;

	if ( !strcmp( type, "result" )) {
		if (( queryNode=JabberXmlGetChild( iqNode, "query" )) != NULL ) {
			char* str = JabberXmlGetAttrValue( queryNode, "xmlns" );
			if ( str!=NULL && !strcmp( str, "http://jabber.org/protocol/disco#items" )) {
				JabberListRemoveList( LIST_AGENT );
				for ( int i=0; i<queryNode->numChild; i++ ) {
					if (( itemNode=queryNode->child[i] )!=NULL && itemNode->name!=NULL && !strcmp( itemNode->name, "item" )) {
						if (( jid=JabberXmlGetAttrValue( itemNode, "jid" )) != NULL ) {
							JABBER_LIST_ITEM* item = JabberListAdd( LIST_AGENT, JabberUrlDecode( jid ));
							item->name = JabberTextDecode( JabberXmlGetAttrValue( itemNode, "name" ));
							item->cap = AGENT_CAP_REGISTER | AGENT_CAP_GROUPCHAT;	// default to all cap until specific info is later received
							int iqId = JabberSerialNext();
							JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultDiscoAgentInfo );
							JabberSend( jabberThreadInfo->s, "<iq type='get' id='"JABBER_IQID"%d' to='%s'><query xmlns='http://jabber.org/protocol/disco#info'/></iq>", iqId, jid );
		}	}	}	}	}

		if ( hwndJabberAgents != NULL ) {
			if (( jid=JabberXmlGetAttrValue( iqNode, "from" )) != NULL )
				SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )jid );
			else
				SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )info->server );
		}
	}
	else if ( !strcmp( type, "error" )) {
		// disco is not supported, try jabber:iq:agents
		int iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_GETAGENTS, JabberIqResultGetAgents );
		JabberSend( jabberThreadInfo->s, "<iq type='get' id='"JABBER_IQID"%d' to='%s'><query xmlns='jabber:iq:agents'/></iq>", iqId, from );
}	}

void JabberIqResultDiscoAgentInfo( XmlNode *iqNode, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	XmlNode *queryNode, *itemNode, *identityNode;
	char* type, *from, *var;
	JABBER_LIST_ITEM *item;

	// RECVED: info for a specific agent
	// ACTION: refresh agent list dialog
	JabberLog( "<iq/> iqIdDiscoAgentInfo" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( from=JabberUrlDecode( JabberXmlGetAttrValue( iqNode, "from" ))) == NULL ) return;

	if ( !strcmp( type, "result" )) {
		if (( queryNode=JabberXmlGetChild( iqNode, "query" )) != NULL ) {
			char* str = JabberXmlGetAttrValue( queryNode, "xmlns" );
			if ( str!=NULL && !strcmp( str, "http://jabber.org/protocol/disco#info" )) {
				if (( item=JabberListGetItemPtr( LIST_AGENT, from )) != NULL ) {
					// Use the first <identity/> to set name
					if (( identityNode=JabberXmlGetChild( queryNode, "identity" )) != NULL ) {
						if (( str=JabberXmlGetAttrValue( identityNode, "name" )) != NULL ) {
							if ( item->name ) free( item->name );
							item->name = JabberTextDecode( str );
					}	}

					item->cap = 0;
					for ( int i=0; i<queryNode->numChild; i++ ) {
						if (( itemNode=queryNode->child[i] )!=NULL && itemNode->name!=NULL ) {
							if ( !strcmp( itemNode->name, "feature" )) {
								if (( var=JabberXmlGetAttrValue( itemNode, "var" )) != NULL ) {
									if ( !strcmp( var, "jabber:iq:register" ))
										item->cap |= AGENT_CAP_REGISTER;
									else if ( !strcmp( var, "http://jabber.org/protocol/muc" ))
										item->cap |= AGENT_CAP_GROUPCHAT;
		}	}	}	}	}	}	}

		if ( hwndJabberAgents != NULL )
			SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )NULL );
}	}

void JabberIqResultDiscoClientInfo( XmlNode *iqNode, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	XmlNode *queryNode, *itemNode;
	char* type, *from, *var;
	JABBER_LIST_ITEM *item;

	// RECVED: info for a specific client
	// ACTION: update client cap
	// ACTION: if item->ft is pending, initiate file transfer
	JabberLog( "<iq/> iqIdDiscoClientInfo" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( from=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;

	if ( strcmp( type, "result" ) != 0 )
		return;
	if (( item=JabberListGetItemPtr( LIST_ROSTER, JabberUrlDecode( from ))) == NULL )
		return;

	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) != NULL ) {
		char* str = JabberXmlGetAttrValue( queryNode, "xmlns" );
		if ( !lstrcmpA( str, "http://jabber.org/protocol/disco#info" )) {
			item->cap = CLIENT_CAP_READY;
			for ( int i=0; i<queryNode->numChild; i++ ) {
				if (( itemNode=queryNode->child[i] )!=NULL && itemNode->name!=NULL ) {
					if ( !strcmp( itemNode->name, "feature" )) {
						if (( var=JabberXmlGetAttrValue( itemNode, "var" )) != NULL ) {
							if ( !strcmp( var, "http://jabber.org/protocol/si" ))
								item->cap |= CLIENT_CAP_SI;
							else if ( !strcmp( var, "http://jabber.org/protocol/si/profile/file-transfer" ))
								item->cap |= CLIENT_CAP_SIFILE;
							else if ( !strcmp( var, "http://jabber.org/protocol/bytestreams" ))
								item->cap |= CLIENT_CAP_BYTESTREAM;
	}	}	}	}	}	}

	// Check for pending file transfer session request
	if ( item->ft != NULL ) {
		filetransfer* ft = item->ft;
		item->ft = NULL;
		if (( item->cap & CLIENT_CAP_FILE ) && ( item->cap & CLIENT_CAP_BYTESTREAM ))
			JabberFtInitiate( item->jid, ft );
		else
			JabberForkThread(( JABBER_THREAD_FUNC )JabberFileServerThread, 0, ft );
}	}

void JabberIqResultGetAvatar( XmlNode *iqNode, void *userdata )
{
	if ( !JGetByte( "EnableAvatars", TRUE ))
		return;

	struct ThreadData *info = ( struct ThreadData * ) userdata;
	char* type;

	// RECVED: agent list
	// ACTION: refresh agent list dialog
	JabberLog( "<iq/> iqIdResultGetAvatar" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL )   return;
	if ( strcmp( type, "result" ))                                  return;

	char* from = JabberUrlDecode( JabberXmlGetAttrValue( iqNode, "from" ));
	if ( from == NULL )
		return;
	HANDLE hContact = JabberHContactFromJID( from );
	if ( hContact == NULL )
		return;

	XmlNode *queryNode = JabberXmlGetChild( iqNode, "query" );
	if ( queryNode == NULL )
		return;

	char* xmlns = JabberXmlGetAttrValue( queryNode, "xmlns" );
	if ( lstrcmpA( xmlns, "jabber:iq:avatar" ))
		return;

	XmlNode* n = JabberXmlGetChild( queryNode, "data" );
	if ( n == NULL )
		return;

	int resultLen = 0;
	char* body = JabberBase64Decode( n->text, &resultLen );

	int pictureType;
	char* mimeType = JabberXmlGetAttrValue( n, "mimetype" );
	if ( mimeType != NULL ) {
		if ( !strcmp( mimeType, "image/jpeg" ))     pictureType = PA_FORMAT_JPEG;
		else if ( !strcmp( mimeType, "image/png" )) pictureType = PA_FORMAT_PNG;
		else if ( !strcmp( mimeType, "image/gif" )) pictureType = PA_FORMAT_GIF;
		else if ( !strcmp( mimeType, "image/bmp" )) pictureType = PA_FORMAT_BMP;
		else {
LBL_ErrFormat:
			JabberLog( "Invalid mime type specified for picture: %s", mimeType );
			free( body );
			return;
	}	}
	else if (( pictureType = JabberGetPictureType( body )) == PA_FORMAT_UNKNOWN )
		goto LBL_ErrFormat;

	PROTO_AVATAR_INFORMATION AI;
	AI.cbSize = sizeof AI;
	AI.format = pictureType;
	AI.hContact = hContact;

	if ( JGetByte( hContact, "AvatarType", PA_FORMAT_UNKNOWN ) != pictureType ) {
		JabberGetAvatarFileName( hContact, AI.filename, sizeof AI.filename );
		DeleteFileA( AI.filename );
	}

	JSetByte( hContact, "AvatarType", pictureType );

	char buffer[ 41 ];
	uint8_t digest[20];
	SHA1Context sha;
	SHA1Reset( &sha );
	SHA1Input( &sha, ( const unsigned __int8* )body, resultLen );
	SHA1Result( &sha, digest );
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
	}
	else JSendBroadcast( hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, HANDLE( &AI ), NULL );

	free( body );
}
