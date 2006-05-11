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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_svc.cpp,v $
Revision       : $Revision: 1.43 $
Last change on : $Date: 2006/02/11 20:33:56 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "resource.h"
#include "jabber_list.h"
#include "jabber_iq.h"

////////////////////////////////////////////////////////////////////////////////////////
// JabberAddToList - adds a contact to the contact list

static HANDLE AddToListByJID( const char* newJid, DWORD flags )
{
	HANDLE hContact;
	char* jid, *nick;

	JabberLog( "AddToListByJID jid=%s", newJid );

	if (( hContact=JabberHContactFromJID( newJid )) == NULL ) {
		// not already there: add
		jid = _strdup( newJid );
		JabberLog( "Add new jid to contact jid=%s", jid );
		hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_ADD, 0, 0 );
		JCallService( MS_PROTO_ADDTOCONTACT, ( WPARAM ) hContact, ( LPARAM )jabberProtoName );
		JSetStringUtf( hContact, "jid", jid );
		if (( nick=JabberNickFromJID( newJid )) == NULL )
			nick = _strdup( newJid );
		JSetStringUtf( hContact, "Nick", nick );
		free( nick );
		free( jid );

		// Note that by removing or disable the "NotOnList" will trigger
		// the plugin to add a particular contact to the roster list.
		// See DBSettingChanged hook at the bottom part of this source file.
		// But the add module will delete "NotOnList". So we will not do it here.
		// Also because we need "MyHandle" and "Group" info, which are set after
		// PS_ADDTOLIST is called but before the add dialog issue deletion of
		// "NotOnList".
		// If temporary add, "NotOnList" won't be deleted, and that's expected.
		DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
		if ( flags & PALF_TEMPORARY )
			DBWriteContactSettingByte( hContact, "CList", "Hidden", 1 );
	}
	else {
		// already exist
		// Set up a dummy "NotOnList" when adding permanently only
		if ( !( flags&PALF_TEMPORARY ))
			DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
	}

	return hContact;
}

int JabberAddToList( WPARAM wParam, LPARAM lParam )
{
	JABBER_SEARCH_RESULT* jsr = ( JABBER_SEARCH_RESULT * ) lParam;
	if ( jsr->hdr.cbSize != sizeof( JABBER_SEARCH_RESULT ))
		return ( int )NULL;

	return ( int )AddToListByJID( jsr->jid, wParam );	// wParam is flag e.g. PALF_TEMPORARY
}

int JabberAddToListByEvent( WPARAM wParam, LPARAM lParam )
{
	DBEVENTINFO dbei;
	HANDLE hContact;
	char* nick, *firstName, *lastName, *jid;

	JabberLog( "AddToListByEvent" );
	ZeroMemory( &dbei, sizeof( dbei ));
	dbei.cbSize = sizeof( dbei );
	if (( dbei.cbBlob=JCallService( MS_DB_EVENT_GETBLOBSIZE, lParam, 0 )) == ( DWORD )( -1 ))
		return ( int )( HANDLE ) NULL;
	if (( dbei.pBlob=( PBYTE ) malloc( dbei.cbBlob )) == NULL )
		return ( int )( HANDLE ) NULL;
	if ( JCallService( MS_DB_EVENT_GET, lParam, ( LPARAM )&dbei )) {
		free( dbei.pBlob );
		return ( int )( HANDLE ) NULL;
	}
	if ( strcmp( dbei.szModule, jabberProtoName )) {
		free( dbei.pBlob );
		return ( int )( HANDLE ) NULL;
	}

/*
	// EVENTTYPE_CONTACTS is when adding from when we receive contact list ( not used in Jabber )
	// EVENTTYPE_ADDED is when adding from when we receive "You are added" ( also not used in Jabber )
	// Jabber will only handle the case of EVENTTYPE_AUTHREQUEST
	// EVENTTYPE_AUTHREQUEST is when adding from the authorization request dialog
*/

	if ( dbei.eventType != EVENTTYPE_AUTHREQUEST ) {
		free( dbei.pBlob );
		return ( int )( HANDLE ) NULL;
	}

	nick = ( char* )( dbei.pBlob + sizeof( DWORD )+ sizeof( HANDLE ));
	firstName = nick + strlen( nick ) + 1;
	lastName = firstName + strlen( firstName ) + 1;
	jid = lastName + strlen( lastName ) + 1;

	hContact = ( HANDLE ) AddToListByJID( jid, wParam );
	free( dbei.pBlob );
	return ( int ) hContact;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAuthAllow - processes the successful authorization

int JabberAuthAllow( WPARAM wParam, LPARAM lParam )
{
	DBEVENTINFO dbei;
	char* nick, *firstName, *lastName, *jid;

	if ( !jabberOnline )
		return 1;

	memset( &dbei, sizeof( dbei ), 0 );
	dbei.cbSize = sizeof( dbei );
	if (( dbei.cbBlob=JCallService( MS_DB_EVENT_GETBLOBSIZE, wParam, 0 )) == ( DWORD )( -1 ))
		return 1;
	if (( dbei.pBlob=( PBYTE ) malloc( dbei.cbBlob )) == NULL )
		return 1;
	if ( JCallService( MS_DB_EVENT_GET, wParam, ( LPARAM )&dbei )) {
		free( dbei.pBlob );
		return 1;
	}
	if ( dbei.eventType != EVENTTYPE_AUTHREQUEST ) {
		free( dbei.pBlob );
		return 1;
	}
	if ( strcmp( dbei.szModule, jabberProtoName )) {
		free( dbei.pBlob );
		return 1;
	}

	nick = ( char* )( dbei.pBlob + sizeof( DWORD )+ sizeof( HANDLE ));
	firstName = nick + strlen( nick ) + 1;
	lastName = firstName + strlen( firstName ) + 1;
	jid = lastName + strlen( lastName ) + 1;

	JabberLog( "Send 'authorization allowed' to %s", jid );
    JabberSend( jabberThreadInfo->s, "<presence to=\"%s\" type=\"subscribed\"/>", jid );

	// Automatically add this user to my roster if option is enabled
	if ( JGetByte( "AutoAdd", TRUE ) == TRUE ) {
		HANDLE hContact;
		JABBER_LIST_ITEM *item;

		if (( item = JabberListGetItemPtr( LIST_ROSTER, jid )) == NULL || ( item->subscription != SUB_BOTH && item->subscription != SUB_TO )) {
			JabberLog( "Try adding contact automatically jid=%s", jid );
			if (( hContact=AddToListByJID( jid, 0 )) != NULL ) {
				// Trigger actual add by removing the "NotOnList" added by AddToListByJID()
				// See AddToListByJID() and JabberDbSettingChanged().
				DBDeleteContactSetting( hContact, "CList", "NotOnList" );
	}	}	}

	free( dbei.pBlob );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAuthDeny - handles the unsuccessful authorization

int JabberAuthDeny( WPARAM wParam, LPARAM lParam )
{
	DBEVENTINFO dbei;
	char* nick, *firstName, *lastName, *jid;

	if ( !jabberOnline )
		return 1;

	JabberLog( "Entering AuthDeny" );
	memset( &dbei, sizeof( dbei ), 0 );
	dbei.cbSize = sizeof( dbei );
	if (( dbei.cbBlob=JCallService( MS_DB_EVENT_GETBLOBSIZE, wParam, 0 )) == ( DWORD )( -1 ))
		return 1;
	if (( dbei.pBlob=( PBYTE ) malloc( dbei.cbBlob )) == NULL )
		return 1;
	if ( JCallService( MS_DB_EVENT_GET, wParam, ( LPARAM )&dbei )) {
		free( dbei.pBlob );
		return 1;
	}
	if ( dbei.eventType != EVENTTYPE_AUTHREQUEST ) {
		free( dbei.pBlob );
		return 1;
	}
	if ( strcmp( dbei.szModule, jabberProtoName )) {
		free( dbei.pBlob );
		return 1;
	}

	nick = ( char* )( dbei.pBlob + sizeof( DWORD )+ sizeof( HANDLE ));
	firstName = nick + strlen( nick ) + 1;
	lastName = firstName + strlen( firstName ) + 1;
	jid = lastName + strlen( lastName ) + 1;

	JabberLog( "Send 'authorization denied' to %s", jid );
    JabberSend( jabberThreadInfo->s, "<presence to=\"%s\" type=\"unsubscribed\"/>", jid );

	free( dbei.pBlob );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberBasicSearch - searches the contact by JID

struct JABBER_SEARCH_BASIC
{	int hSearch;
	char jid[128];
};

static void __cdecl JabberBasicSearchThread( JABBER_SEARCH_BASIC *jsb )
{
	SleepEx( 100, TRUE );

	JABBER_SEARCH_RESULT jsr = { 0 };
	jsr.hdr.cbSize = sizeof( JABBER_SEARCH_RESULT );
	jsr.hdr.nick = "";
	jsr.hdr.firstName = "";
	jsr.hdr.lastName = "";
	jsr.hdr.email = jsb->jid;
	strncpy( jsr.jid, jsb->jid, sizeof( jsr.jid ));
	jsr.jid[sizeof( jsr.jid )-1] = '\0';
	JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, ( HANDLE ) jsb->hSearch, ( LPARAM )&jsr );
	JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) jsb->hSearch, 0 );
	free( jsb );
}

int JabberBasicSearch( WPARAM wParam, LPARAM lParam )
{
	char* szJid = ( char* )lParam;
	JabberLog( "JabberBasicSearch called with lParam = %s", szJid );

	JABBER_SEARCH_BASIC *jsb;
	if ( !jabberOnline || ( jsb=( JABBER_SEARCH_BASIC * ) malloc( sizeof( JABBER_SEARCH_BASIC )) )==NULL )
		return 0;

	jsb->hSearch = JabberSerialNext();
	if ( strchr( szJid, '@' ) == NULL ) {
		char szServer[ 100 ];
		if ( JGetStaticString( "LoginServer", NULL, szServer, sizeof szServer ))
			strcpy( szServer, "gmail.com" );

		mir_snprintf( jsb->jid, sizeof jsb->jid, "%s@%s", szJid, szServer );
	}
	else strncpy( jsb->jid, szJid, sizeof jsb->jid );

	JabberForkThread(( JABBER_THREAD_FUNC )JabberBasicSearchThread, 0, jsb );
	return jsb->hSearch;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberContactDeleted - processes a contact deletion

int JabberContactDeleted( WPARAM wParam, LPARAM lParam )
{
	char* szProto;
	DBVARIANT dbv;

	if( !jabberOnline )	// should never happen
		return 0;
	szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
	if ( szProto==NULL || strcmp( szProto, jabberProtoName ))
		return 0;
	if ( !JGetStringUtf(( HANDLE ) wParam, "jid", &dbv )) {
		char* jid, *p, *q;

		jid = dbv.pszVal;
		if (( p=strchr( jid, '@' )) != NULL ) {
			if (( q=strchr( p, '/' )) != NULL )
				*q = '\0';
		}
		if ( JabberListExist( LIST_ROSTER, jid )) {
			// Remove from roster, server also handles the presence unsubscription process.
            JabberSend( jabberThreadInfo->s, "<iq type=\"set\"><query xmlns=\"jabber:iq:roster\"><item jid=\"%s\" subscription=\"remove\"/></query></iq>", jid );
		}

		JFreeVariant( &dbv );
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberDbSettingChanged - process database changes

void sttRenameGroup( DBCONTACTWRITESETTING* cws, HANDLE hContact )
{
	DBVARIANT jid, dbv;
	if ( JGetStringUtf( hContact, "jid", &jid ))
		return;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, jid.pszVal );
	JFreeVariant( &jid );
	if ( item == NULL )
		return;

	char* nick;
	if ( !DBGetContactSettingStringUtf( hContact, "CList", "MyHandle", &dbv )) {
		nick = strdup( dbv.pszVal );
		JFreeVariant( &dbv );
	}
	else if ( !JGetStringUtf( hContact, "Nick", &dbv )) {
		nick = strdup( dbv.pszVal );
		JFreeVariant( &dbv );
	}
	else nick = JabberNickFromJID( item->jid );
	if ( nick == NULL )
		return;

	if ( cws->value.type == DBVT_DELETED ) {
		if ( item->group != NULL ) {
			JabberLog( "Group set to nothing" );
			JabberAddContactToRoster( item->jid, nick, NULL );
		}
	}
	else if ( cws->value.pszVal != NULL && lstrcmpA( cws->value.pszVal, item->group )) {
		JabberLog( "Group set to %s", cws->value.pszVal );
		switch( cws->value.type ) {
			case DBVT_ASCIIZ:	JabberAddContactToRoster( item->jid, nick, UTF8( cws->value.pszVal ));  break;
			case DBVT_UTF8:   JabberAddContactToRoster( item->jid, nick, cws->value.pszVal );        break;
	}	}

	free( nick );
}

void sttRenameContact( DBCONTACTWRITESETTING* cws, HANDLE hContact )
{
	DBVARIANT jid;
	if ( JGetStringUtf( hContact, "jid", &jid ))
		return;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, jid.pszVal );
	JFreeVariant( &jid );
	if ( item == NULL )
		return;

	if ( cws->value.type == DBVT_DELETED ) {
		char* nick = ( char* )JCallService( MS_CLIST_GETCONTACTDISPLAYNAME, ( WPARAM )hContact, GCDNF_NOMYHANDLE );
		JabberAddContactToRoster( item->jid, UTF8(nick), item->group );
		mir_free(nick);
		return;
	}

	char* newNick; bool bNeedsFree = false;
	switch( cws->value.type ) {
		case DBVT_ASCIIZ: newNick = JabberTextEncode( cws->value.pszVal ); bNeedsFree = true; break;
		case DBVT_UTF8:   newNick = cws->value.pszVal;  break;
		default: return;
	}

	if ( cws->value.pszVal != NULL && lstrcmpA( item->nick, newNick )) {
		JabberLog( "Renaming contact %s: %s -> %s", item->jid, item->nick, newNick );
		JabberAddContactToRoster( item->jid, newNick, item->group );
	}

	if ( bNeedsFree )
		free( newNick );
}

void sttAddContactForever( DBCONTACTWRITESETTING* cws, HANDLE hContact )
{
	if ( cws->value.type != DBVT_DELETED && !( cws->value.type==DBVT_BYTE && cws->value.bVal==0 ))
		return;

	DBVARIANT jid, dbv;
	if ( JGetStringUtf( hContact, "jid", &jid ))
		return;

	char *nick;
	JabberLog( "Add %s permanently to list", jid.pszVal );
	if ( !DBGetContactSettingStringUtf( hContact, "CList", "MyHandle", &dbv )) {
		nick = strdup( dbv.pszVal );
		JFreeVariant( &dbv );
	}
	else if ( !JGetStringUtf( hContact, "Nick", &dbv )) {
		nick = strdup( dbv.pszVal );
		JFreeVariant( &dbv );
	}
	else nick = JabberNickFromJID( jid.pszVal );
	if ( nick == NULL ) {
		JFreeVariant( &jid );
		return;
	}

	if ( !DBGetContactSettingStringUtf( hContact, "CList", "Group", &dbv )) {
		JabberAddContactToRoster( jid.pszVal, nick, dbv.pszVal );
		JFreeVariant( &dbv );
	}
	else JabberAddContactToRoster( jid.pszVal, nick, NULL );
    JabberSend( jabberThreadInfo->s, "<presence to=\"%s\" type=\"subscribe\"/>", jid.pszVal );

	free( nick );
	DBDeleteContactSetting( hContact, "CList", "Hidden" );
	JFreeVariant( &jid );
}

void sttStatusChanged( DBCONTACTWRITESETTING* cws, HANDLE hContact ){
	
	DBVARIANT jid;
	if ( JGetStringUtf( hContact, "jid", &jid ))
		return;
	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, jid.pszVal );
	//if ( item == NULL )
	//	return;
	//(item->defaultResource<0)?NULL:item->resource[item->defaultResource].resourceName;
	char *courRes = JabberListGetBestClientResourceNamePtr(jid.pszVal);
	JabberLog("Status Changed: %s, Best Resource: %s",jid.pszVal,
		(courRes)?courRes:"none");
	if (courRes) {
		for (int i=0;i<item->resourceCount;i++){
			if (!strcmp(courRes,item->resource[i].resourceName)){
				putResUserSett(hContact,&item->resource[i]);
				JabberLog("Software: %s (%s)",item->resource[i].software,item->resource[i].version);
				break;
			}
		}
	}
	//else JSetStringUtf(hContact,"Resource","");
	JFreeVariant( &jid );
	
}

int JabberDbSettingChanged( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact = ( HANDLE ) wParam;
	if ( hContact == NULL || !jabberConnected )
		return 0;

	DBCONTACTWRITESETTING* cws = ( DBCONTACTWRITESETTING* )lParam;
	if ( strcmp( cws->szModule, "CList" )){
		if (!strcmp( cws->szModule, jabberProtoName )){
			if ( !strcmp( cws->szSetting, "Status")) sttStatusChanged( cws, hContact );
		}
		return 0;
	}

	char* szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
	if ( szProto == NULL || strcmp( szProto, jabberProtoName ))
		return 0;

	if ( !strcmp( cws->szSetting, "Group" ))
		sttRenameGroup( cws, hContact );
	else if ( !strcmp( cws->szSetting, "MyHandle" ))
		sttRenameContact( cws, hContact );
	else if ( !strcmp( cws->szSetting, "NotOnList" )){
		if (!DBGetContactSettingByte((HANDLE) wParam, "CList", "Hidden", 0))  // skip hidden contacts!
			sttAddContactForever( cws, hContact );
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileAllow - starts a file transfer

int JabberFileAllow( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 0;

	CCSDATA *ccs = ( CCSDATA * ) lParam;
	filetransfer* ft = ( filetransfer* ) ccs->wParam;
	ft->std.workingDir = _strdup(( char* )ccs->lParam );
	int len = strlen( ft->std.workingDir )-1;
	if ( ft->std.workingDir[len] == '//' || ft->std.workingDir[len] == '\\' )
		ft->std.workingDir[len] = 0;

	switch ( ft->type ) {
	case FT_OOB:
		JabberForkThread(( JABBER_THREAD_FUNC )JabberFileReceiveThread, 0, ft );
		break;
	case FT_BYTESTREAM:
		JabberFtAcceptSiRequest( ft );
		break;
	}
	return ccs->wParam;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileCancel - cancels a file transfer

int JabberFileCancel( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA * ) lParam;
	filetransfer* ft = ( filetransfer* ) ccs->wParam;
	HANDLE hEvent;

	JabberLog( "Invoking FileCancel()" );
	if ( ft->type == FT_OOB ) {
		if ( ft->s ) {
			JabberLog( "FT canceled" );
			JabberLog( "Closing ft->s = %d", ft->s );
			ft->state = FT_ERROR;
			Netlib_CloseHandle( ft->s );
			ft->s = NULL;
			if ( ft->hFileEvent != NULL ) {
				hEvent = ft->hFileEvent;
				ft->hFileEvent = NULL;
				SetEvent( hEvent );
			}
			JabberLog( "ft->s is now NULL, ft->state is now FT_ERROR" );
		}
	}
	else JabberFtCancel( ft );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileDeny - denies a file transfer

int JabberFileDeny( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 1;

	CCSDATA *ccs = ( CCSDATA * ) lParam;
	filetransfer* ft = ( filetransfer* )ccs->wParam;
	char* szId = ft->iqId;
	switch ( ft->type ) {
	case FT_OOB:
        JabberSend( jabberThreadInfo->s, "<iq type=\"error\" to=\"%s\"%s%s%s><error code=\"406\">File transfer refused</error></iq>", ft->jid, ( szId )?" id=\"":"", ( szId )?szId:"", ( szId )?"\"":"" );
		break;
	case FT_BYTESTREAM:
        JabberSend( jabberThreadInfo->s, "<iq type=\"error\" to=\"%s\"%s%s%s><error code=\"403\" type=\"cancel\"><forbidden xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/><text xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\">File transfer refused</text></error></iq>", ft->jid, ( szId )?" id=\"":"", ( szId )?szId:"", ( szId )?"\"":"" );
		break;
	}
	delete ft;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileResume - processes file renaming etc

int JabberFileResume( WPARAM wParam, LPARAM lParam )
{
	filetransfer* ft = ( filetransfer* )wParam;
	if ( !jabberConnected || ft == NULL )
		return 1;

	PROTOFILERESUME *pfr = (PROTOFILERESUME*)lParam;
	if ( pfr->action == FILERESUME_RENAME ) {
		if ( ft->wszFileName != NULL ) {
			free( ft->wszFileName );
			ft->wszFileName = NULL;
		}

		replaceStr( ft->std.currentFile, pfr->szFilename );
	}

	SetEvent( ft->hWaitEvent );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarInfo - retrieves the avatar info

static int JabberGetAvatarInfo(WPARAM wParam,LPARAM lParam)
{
	if ( !JGetByte( "EnableAvatars", TRUE ))
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATION* AI = ( PROTO_AVATAR_INFORMATION* )lParam;

	char szHashValue[ MAX_PATH ];
	if ( JGetStaticString( "AvatarHash", AI->hContact, szHashValue, sizeof szHashValue )) {
		JabberLog( "No avatar" );
		return GAIR_NOAVATAR;
	}

	JabberGetAvatarFileName( AI->hContact, AI->filename, sizeof AI->filename );
	AI->format = ( AI->hContact == NULL ) ? PA_FORMAT_PNG : JGetByte( AI->hContact, "AvatarType", 0 );

	if ( ::access( AI->filename, 0 ) == 0 ) {
		char szSavedHash[ 256 ];
		if ( !JGetStaticString( "AvatarSaved", AI->hContact, szSavedHash, sizeof szSavedHash )) {
			if ( !strcmp( szSavedHash, szHashValue )) {
				JabberLog( "Avatar is Ok: %s == %s", szSavedHash, szHashValue );
				return GAIR_SUCCESS;
	}	}	}

	if (( wParam & GAIF_FORCE ) != 0 && AI->hContact != NULL && jabberOnline ) {
		DBVARIANT dbv;
		if ( !JGetStringUtf( AI->hContact, "jid", &dbv )) {
			JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, dbv.pszVal );
			if ( item != NULL ) {
				char szJid[ 512 ];
				if ( item->resourceCount != NULL )
					mir_snprintf( szJid, sizeof( szJid ), "%s/%s", dbv.pszVal, item->resource[0].resourceName );
				else
					lstrcpynA( szJid, dbv.pszVal, sizeof( szJid ));

				JabberLog( "Rereading avatar for %s", dbv.pszVal );

				int iqId = JabberSerialNext();
				JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetAvatar );
				JabberSend( jabberThreadInfo->s,
                    "<iq type=\"get\" to=\"%s\" id=\""JABBER_IQID"%d\"><query xmlns=\"jabber:iq:avatar\"/></iq>",
					szJid, iqId );
				JFreeVariant( &dbv );
				return GAIR_WAITFOR;
	}	}	}

	JabberLog( "No avatar" );
	return GAIR_NOAVATAR;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAwayMsg - returns a contact's away message

static void __cdecl JabberGetAwayMsgThread( HANDLE hContact )
{
	DBVARIANT dbv;
	JABBER_LIST_ITEM *item;
	JABBER_RESOURCE_STATUS *r;
	char* str;
	int i, len, msgCount;

	if ( !JGetStringUtf( hContact, "jid", &dbv )) {
		if (( item=JabberListGetItemPtr( LIST_ROSTER, dbv.pszVal )) != NULL ) {
			JFreeVariant( &dbv );
			if ( item->resourceCount > 0 ) {
				JabberLog( "resourceCount > 0" );
				r = item->resource;
				len = msgCount = 0;
				for ( i=0; i<item->resourceCount; i++ ) {
					if ( r[i].statusMessage ) {
						msgCount++;
						char *resNameTemp = JabberUrlDecodeNew(r[i].resourceName);
						len += ( strlen( resNameTemp ) + strlen( r[i].statusMessage ) + 8 );
						if (resNameTemp) free(resNameTemp);
					}
				}
				if (( str=( char* )malloc( len + 1 )) != NULL ) {
					str[0] = str[len] = '\0';
					for ( i=0; i<item->resourceCount; i++ ) {
						if ( r[i].statusMessage ) {
							if ( str[0] != '\0' ) strcat( str, "\r\n" );
							if ( msgCount > 1 ) {
								char *resNameTemp = JabberUrlDecodeNew(r[i].resourceName);
								int templen = strlen(resNameTemp)+1;
								char *temp = (char *)mir_alloc(templen);
								strcat( str, "( " );
								strncpy(temp,resNameTemp, templen);
								JabberUtf8Decode(temp,NULL);
								strcat( str, temp);
								strcat( str, " ): " );
								mir_free(temp);
								if (resNameTemp) free(resNameTemp);
							}
							strcat( str, r[i].statusMessage );
						}
					}
					JSendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE ) 1, ( LPARAM )str );
					free( str );
					return;
				}
			}
			else if ( item->statusMessage != NULL ) {
				JSendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE ) 1, ( LPARAM )item->statusMessage );
				return;
			}
		}
		else {
			JFreeVariant( &dbv );
		}
	}
	//JSendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_FAILED, ( HANDLE ) 1, ( LPARAM )0 );
	JSendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE ) 1, ( LPARAM )"" );
}

int JabberGetAwayMsg( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA * ) lParam;

	JabberLog( "GetAwayMsg called, wParam=%d lParam=%d", wParam, lParam );
	JabberForkThread( JabberGetAwayMsgThread, 0, ( void * ) ccs->hContact );
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetCaps - return protocol capabilities bits

int JabberGetCaps( WPARAM wParam, LPARAM lParam )
{
	switch( wParam ) {
	case PFLAGNUM_1:
		return PF1_IM|PF1_AUTHREQ|PF1_SERVERCLIST|PF1_MODEMSG|PF1_BASICSEARCH|PF1_SEARCHBYEMAIL|PF1_SEARCHBYNAME|PF1_FILE|PF1_VISLIST|PF1_INVISLIST;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_NOCUSTOMAUTH | PF4_SUPPORTTYPING | PF4_AVATARS;
	case PFLAG_UNIQUEIDTEXT:
		return ( int ) JTranslate( "JID" );
	case PFLAG_UNIQUEIDSETTING:
		return ( int ) "jid";
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetInfo - retrieves a contact info

int JabberGetInfo( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline )
		return 1;

	CCSDATA *ccs = ( CCSDATA * ) lParam;
	int result = 1;
	DBVARIANT dbv;
	if ( !JGetStringUtf( ccs->hContact, "jid", &dbv )) {
		result = JabberSendGetVcard( dbv.pszVal );
		JFreeVariant( &dbv );
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetName - returns the protocol name

int JabberGetName( WPARAM wParam, LPARAM lParam )
{
	lstrcpynA(( char* )lParam, jabberModuleName, wParam );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetStatus - returns the protocol status

int JabberGetStatus( WPARAM wParam, LPARAM lParam )
{
	return jabberStatus;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberLoadIcon - loads an icon for the contact list

int JabberLoadIcon( WPARAM wParam, LPARAM lParam )
{
	if (( wParam&0xffff ) == PLI_PROTOCOL )
		return ( int ) LoadImage( hInst, MAKEINTRESOURCE( IDI_JABBER ), IMAGE_ICON, GetSystemMetrics( wParam&PLIF_SMALL?SM_CXSMICON:SM_CXICON ), GetSystemMetrics( wParam&PLIF_SMALL?SM_CYSMICON:SM_CYICON ), 0 );
	else
		return ( int ) ( HICON ) NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberRecvFile - receives a file

int JabberRecvFile( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA * ) lParam;
	PROTORECVEVENT *pre = ( PROTORECVEVENT * ) ccs->lParam;
	char* szFile = pre->szMessage + sizeof( DWORD );
	char* szDesc = szFile + strlen( szFile ) + 1;
	JabberLog( "Description = %s", szDesc );

	DBDeleteContactSetting( ccs->hContact, "CList", "Hidden" );

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof( dbei );
	dbei.szModule = jabberProtoName;
	dbei.timestamp = pre->timestamp;
	dbei.flags = pre->flags & ( PREF_CREATEREAD ? DBEF_READ : 0 );
	dbei.eventType = EVENTTYPE_FILE;
	dbei.cbBlob = sizeof( DWORD )+ strlen( szFile ) + strlen( szDesc ) + 2;
	dbei.pBlob = ( PBYTE ) pre->szMessage;
	JCallService( MS_DB_EVENT_ADD, ( WPARAM ) ccs->hContact, ( LPARAM )&dbei );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberRecvMessage - receives a message

int JabberRecvMessage( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA* )lParam;
	PROTORECVEVENT *pre = ( PROTORECVEVENT* )ccs->lParam;

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof( dbei );
	dbei.szModule = jabberProtoName;
	dbei.timestamp = pre->timestamp;
	dbei.flags = pre->flags&PREF_CREATEREAD?DBEF_READ:0;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = strlen( pre->szMessage ) + 1;
	if ( pre->flags & PREF_UNICODE )
		dbei.cbBlob *= ( sizeof( wchar_t )+1 );

	dbei.pBlob = ( PBYTE ) pre->szMessage;
	JCallService( MS_DB_EVENT_ADD, ( WPARAM ) ccs->hContact, ( LPARAM )&dbei );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSearchByEmail - searches the contact by its e-mail

int JabberSearchByEmail( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 0;
	if (( char* )lParam == NULL ) return 0;

	char szServerName[100];
	if ( JGetStaticString( "Jud", NULL, szServerName, sizeof szServerName ))
		strcpy( szServerName, "users.jabber.org" );

	int iqId = JabberSerialNext();
	JabberIqAdd( iqId, IQ_PROC_GETSEARCH, JabberIqResultSetSearch );
	JabberSend( jabberThreadInfo->s,
        "<iq type=\"set\" id=\""JABBER_IQID"%d\" to=\"%s\"><query xmlns=\"jabber:iq:search\"><email>%s</email></query></iq>",
		iqId, szServerName, TXT(( char* )lParam));
	return iqId;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSearchByName - searches the contact by its first or last name, or by a nickname

int JabberSearchByName( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 0;

	PROTOSEARCHBYNAME *psbn = ( PROTOSEARCHBYNAME * ) lParam;
	char*  text = ( char* )alloca( 1024 ); *text = 0;
	size_t len = 0;
	BOOL   bIsExtFormat = JGetByte( "ExtendedSearch", TRUE );

	if ( psbn->pszNick[0] != '\0' ) {
		if ( bIsExtFormat )
            len += mir_snprintf( text, 1024-len, "<field var=\"user\"><value>%s</value></field>", TXT(psbn->pszNick));
		else
			len += mir_snprintf( text, 1024-len, "<nick>%s</nick>", TXT(psbn->pszNick));
	}

	if ( psbn->pszFirstName[0] != '\0' ) {
		if ( bIsExtFormat )
            len += mir_snprintf( text+len, 1024-len, "<field var=\"fn\"><value>%s</value></field>", TXT(psbn->pszFirstName));
		else
			len += mir_snprintf( text+len, 1024-len, "<first>%s</first>", TXT(psbn->pszFirstName));
	}

	if ( psbn->pszLastName[0] != '\0' ) {
		if ( bIsExtFormat )
            len += mir_snprintf( text+len, 1024-len, "<field var=\"given\"><value>%s</value></field>", TXT(psbn->pszLastName));
		else
			len += mir_snprintf( text+len, 1024-len, "<last>%s</last>", TXT(psbn->pszLastName));
	}

	char szServerName[100];
	if ( JGetStaticString( "Jud", NULL, szServerName, sizeof szServerName ))
		strcpy( szServerName, "users.jabber.org" );

	int iqId = JabberSerialNext();
	if ( bIsExtFormat ) {
		JabberIqAdd( iqId, IQ_PROC_GETSEARCH, JabberIqResultExtSearch );
		JabberSend( jabberThreadInfo->s,
            "<iq type=\"set\" id=\""JABBER_IQID"%d\" to=\"%s\" xml:lang=\"en\">"
            "<query xmlns=\"jabber:iq:search\"><x xmlns=\"jabber:x:data\" type=\"submit\">%s</x></query></iq>",
			iqId, szServerName, text );
	}
	else {
		JabberIqAdd( iqId, IQ_PROC_GETSEARCH, JabberIqResultSetSearch );
		JabberSend( jabberThreadInfo->s,
            "<iq type=\"set\" id=\""JABBER_IQID"%d\" to=\"%s\"><query xmlns=\"jabber:iq:search\">%s</query></iq>",
			iqId, szServerName, text );
	}
	return iqId;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSendFile - sends a file

int JabberSendFile( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 0;

	CCSDATA *ccs = ( CCSDATA * ) lParam;
	if ( JGetWord( ccs->hContact, "Status", ID_STATUS_OFFLINE ) == ID_STATUS_OFFLINE )
		return 0;

	DBVARIANT dbv;
	if ( JGetStringUtf( ccs->hContact, "jid", &dbv ))
		return 0;

	char* *files = ( char* * ) ccs->lParam;
	int i, j;
	struct _stat statbuf;
	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, dbv.pszVal );
	if ( item == NULL )
		return 0;

	// Check if another file transfer session request is pending ( waiting for disco result )
	if ( item->ft != NULL ) return 0;

	filetransfer* ft = new filetransfer;
	ft->std.hContact = ccs->hContact;
	while( files[ ft->std.totalFiles ] != NULL )
		ft->std.totalFiles++;

	ft->std.files = ( char** ) malloc( sizeof( char* )* ft->std.totalFiles );
	ft->fileSize = ( long* ) malloc( sizeof( long ) * ft->std.totalFiles );
	for( i=j=0; i < ft->std.totalFiles; i++ ) {
		if ( _stat( files[i], &statbuf ))
			JabberLog( "'%s' is an invalid filename", files[i] );
		else {
			ft->std.files[j] = _strdup( files[i] );
			ft->fileSize[j] = statbuf.st_size;
			j++;
			ft->std.totalBytes += statbuf.st_size;
	}	}

	ft->std.currentFile = _strdup( files[0] );
	ft->szDescription = _strdup(( char* )ccs->wParam );
	ft->jid = _strdup( dbv.pszVal );
	JFreeVariant( &dbv );

	if ( item->cap == 0 ) {
		int iqId;
		char* rs;

		// Probe client capability
		if (( rs=JabberListGetBestClientResourceNamePtr( item->jid )) != NULL ) {
			item->ft = ft;
			iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultDiscoClientInfo );
            JabberSend( jabberThreadInfo->s, "<iq type=\"get\" id=\""JABBER_IQID"%d\" to=\"%s/%s\"><query xmlns=\"http://jabber.org/protocol/disco#info\"/></iq>", iqId, item->jid, rs );
		}
	}
	else if (( item->cap & CLIENT_CAP_FILE ) && ( item->cap & CLIENT_CAP_BYTESTREAM ))
		// Use the new standard file transfer
		JabberFtInitiate( item->jid, ft );
	else // Use the jabber:iq:oob file transfer
		JabberForkThread(( JABBER_THREAD_FUNC )JabberFileServerThread, 0, ft );

	return ( int )( HANDLE ) ft;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSendMessage - sends a message

static void __cdecl JabberSendMessageAckThread( HANDLE hContact )
{
	SleepEx( 10, TRUE );
	JabberLog( "Broadcast ACK" );
	JSendBroadcast( hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, ( HANDLE ) 1, 0 );
	JabberLog( "Returning from thread" );
}

static char PGP_PROLOG[] = "-----BEGIN PGP MESSAGE-----\r\n\r\n";
static char PGP_EPILOG[] = "\r\n-----END PGP MESSAGE-----\r\n";

int JabberSendMessage( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA * ) lParam;
	JABBER_LIST_ITEM *item;
	int id;

	DBVARIANT dbv;
	if ( !jabberOnline || JGetStringUtf( ccs->hContact, "jid", &dbv )) {
		JSendBroadcast( ccs->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, ( HANDLE ) 1, 0 );
		return 0;
	}

	char* pszSrc = ( char* )ccs->lParam, *msg;
	int  isEncrypted;

	char* pdest = strstr( pszSrc, PGP_PROLOG );//pdest-string+1 is index of first occurence
	if ( pdest != NULL ) {
		pdest = strstr( pszSrc, PGP_EPILOG );
		int result = ( pdest ) ? strlen( PGP_PROLOG ) : 0;

		char* tempstring = ( char* )alloca( strlen( pszSrc ));
		strncpy( tempstring, pszSrc+strlen(PGP_PROLOG), strlen(pszSrc)-strlen(PGP_EPILOG)-result );
		pszSrc = tempstring;
		isEncrypted = 1;
	}
	else isEncrypted = 0;

	if ( ccs->wParam & PREF_UNICODE )
		msg = JabberTextEncodeW(( wchar_t* )&pszSrc[ strlen( pszSrc )+1 ] );
	else
		msg = JabberTextEncode( pszSrc );

	if ( msg != NULL ) {
		char msgType[ 16 ];
		if ( JabberListExist( LIST_CHATROOM, dbv.pszVal ) && strchr( dbv.pszVal, '/' )==NULL )
			strcpy( msgType, "groupchat" );
		else
			strcpy( msgType, "chat" );

		if ( !strcmp( msgType, "groupchat" ) || JGetByte( "MsgAck", FALSE ) == FALSE ) {
			if ( !strcmp( msgType, "groupchat" )) {
				if ( !isEncrypted )
                    JabberSend( jabberThreadInfo->s, "<message to=\"%s\" type=\"%s\"><body>%s</body></message>", dbv.pszVal, msgType, msg );
				else
               JabberSend( jabberThreadInfo->s, "<message to=\"%s\" type=\"%s\"><body>[This message is encrypted.]</body><x xmlns=\"jabber:x:encrypted\">%s</x></message>",
						dbv.pszVal, msgType, msg );
			}
			else {
				id = JabberSerialNext();
				char szClientJid[ 256 ];
				JabberGetClientJID( dbv.pszVal, szClientJid, sizeof( szClientJid ));
				if ( !isEncrypted )
                    JabberSend( jabberThreadInfo->s, "<message to=\"%s\" type=\"%s\" id=\""JABBER_IQID"%d\"><body>%s</body><x xmlns=\"jabber:x:event\"><composing/></x></message>", szClientJid, msgType, id, msg );
				else
                    JabberSend( jabberThreadInfo->s, "<message to=\"%s\" type=\"%s\" id=\""JABBER_IQID"%d\"><body>[This message is encrypted.]</body><x xmlns=\"jabber:x:encrypted\">%s</x><x xmlns=\"jabber:x:event\"><composing/></x></message>",
						szClientJid, msgType, id, msg );
			}
			JabberForkThread( JabberSendMessageAckThread, 0, ( void* )ccs->hContact );
		}
		else {
			id = JabberSerialNext();
			if (( item=JabberListGetItemPtr( LIST_ROSTER, dbv.pszVal )) != NULL )
				item->idMsgAckPending = id;

			char szClientJid[ 256 ];
			JabberGetClientJID( dbv.pszVal, szClientJid, sizeof( szClientJid ));
			if ( !isEncrypted )
                JabberSend( jabberThreadInfo->s, "<message to=\"%s\" type=\"%s\" id=\""JABBER_IQID"%d\"><body>%s</body><x xmlns=\"jabber:x:event\"><offline/><delivered/><composing/></x></message>", szClientJid, msgType, id, msg );
			else
            JabberSend( jabberThreadInfo->s, "<message to=\"%s\" type=\"%s\" id=\""JABBER_IQID"%d\"><body>[This message is encrypted.]</body><x xmlns=\"jabber:x:encrypted\">%s</x><x xmlns=\"jabber:x:event\"><offline/><delivered/><composing/></x></message>",
					szClientJid, msgType, id, msg );
		}
		free( msg );
	}
	JFreeVariant( &dbv );
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetApparentMode - sets the visibility status

int JabberSetApparentMode( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA * ) lParam;
	DBVARIANT dbv;
	int oldMode;
	char* jid;

	if ( ccs->wParam!=0 && ccs->wParam!=ID_STATUS_ONLINE && ccs->wParam!=ID_STATUS_OFFLINE ) return 1;
	oldMode = JGetWord( ccs->hContact, "ApparentMode", 0 );
	if (( int ) ccs->wParam == oldMode ) return 1;
	JSetWord( ccs->hContact, "ApparentMode", ( WORD )ccs->wParam );

	if ( !jabberOnline ) return 0;

	if ( !JGetStringUtf( ccs->hContact, "jid", &dbv )) {
		jid = dbv.pszVal;
		switch ( ccs->wParam ) {
		case ID_STATUS_ONLINE:
			if ( jabberStatus == ID_STATUS_INVISIBLE || oldMode == ID_STATUS_OFFLINE )
                JabberSend( jabberThreadInfo->s, "<presence to=\"%s\"/>", jid );
			break;
		case ID_STATUS_OFFLINE:
			if ( jabberStatus != ID_STATUS_INVISIBLE || oldMode == ID_STATUS_ONLINE )
				JabberSendPresenceTo( ID_STATUS_INVISIBLE, jid, NULL );
			break;
		case 0:
			if ( oldMode == ID_STATUS_ONLINE && jabberStatus == ID_STATUS_INVISIBLE )
				JabberSendPresenceTo( ID_STATUS_INVISIBLE, jid, NULL );
			else if ( oldMode == ID_STATUS_OFFLINE && jabberStatus != ID_STATUS_INVISIBLE )
				JabberSendPresenceTo( jabberStatus, jid, NULL );
			break;
		}
		JFreeVariant( &dbv );
	}

	// TODO: update the zebra list ( jabber:iq:privacy )

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetAwayMsg - sets the away status message

int JabberSetAwayMsg( WPARAM wParam, LPARAM lParam )
{
	char* *szMsg;
	char* newModeMsg;
	int desiredStatus;

	JabberLog( "SetAwayMsg called, wParam=%d lParam=%s", wParam, ( char* )lParam );

	desiredStatus = wParam;
	newModeMsg = JabberTextEncode(( char* )lParam );

	EnterCriticalSection( &modeMsgMutex );

	switch ( desiredStatus ) {
	case ID_STATUS_ONLINE:
		szMsg = &modeMsgs.szOnline;
		break;
	case ID_STATUS_AWAY:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		szMsg = &modeMsgs.szAway;
		break;
	case ID_STATUS_NA:
		szMsg = &modeMsgs.szNa;
		break;
	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		szMsg = &modeMsgs.szDnd;
		break;
	case ID_STATUS_FREECHAT:
		szMsg = &modeMsgs.szFreechat;
		break;
	default:
		LeaveCriticalSection( &modeMsgMutex );
		return 1;
	}

	if (( *szMsg==NULL && newModeMsg==NULL ) ||
		( *szMsg!=NULL && newModeMsg!=NULL && !strcmp( *szMsg, newModeMsg )) ) {
		// Message is the same, no update needed
		if ( newModeMsg != NULL ) free( newModeMsg );
	}
	else {
		// Update with the new mode message
		if ( *szMsg != NULL ) free( *szMsg );
		*szMsg = newModeMsg;
		// Send a presence update if needed
		if ( desiredStatus == jabberStatus ) {
			JabberSendPresence( jabberStatus );
		}
	}

	LeaveCriticalSection( &modeMsgMutex );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetStatus - sets the protocol status

int JabberSetStatus( WPARAM wParam, LPARAM lParam )
{
	JabberLog( "PS_SETSTATUS( %d )", wParam );
	int desiredStatus = wParam;
	jabberDesiredStatus = desiredStatus;

 	if ( desiredStatus == ID_STATUS_OFFLINE ) {
		if ( jabberThreadInfo ) {
			HANDLE s = jabberThreadInfo->s;
			jabberThreadInfo = NULL;
			if ( jabberConnected ) {
				JabberSend( s, "</stream:stream>" );
				jabberConnected = jabberOnline = FALSE;
			}
			Netlib_CloseHandle(s); // New Line
		}

		int oldStatus = jabberStatus;
		jabberStatus = jabberDesiredStatus = ID_STATUS_OFFLINE;
		JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );
	}
	else if ( !jabberConnected && !( jabberStatus >= ID_STATUS_CONNECTING && jabberStatus < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES )) {
		if ( jabberConnected )
			return 0;

		ThreadData* thread = ( ThreadData* ) malloc( sizeof( struct ThreadData ));

		ZeroMemory( thread, sizeof( struct ThreadData ));
		thread->type = JABBER_SESSION_NORMAL;
		jabberDesiredStatus = desiredStatus;

		int oldStatus = jabberStatus;
		jabberStatus = ID_STATUS_CONNECTING;
		JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );
		thread->hThread = ( HANDLE ) JabberForkThread(( JABBER_THREAD_FUNC )JabberServerThread, 0, thread );
	}
	else JabberSetServerStatus( desiredStatus );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserIsTyping - sends a UTN notification

int JabberUserIsTyping( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact = ( HANDLE ) wParam;
	DBVARIANT dbv;
	JABBER_LIST_ITEM *item;

	if ( !jabberOnline ) return 0;
	if ( !JGetStringUtf( hContact, "jid", &dbv )) {
		if (( item=JabberListGetItemPtr( LIST_ROSTER, dbv.pszVal ))!=NULL && item->wantComposingEvent==TRUE ) {
			char szClientJid[ 256 ];
			JabberGetClientJID( dbv.pszVal, szClientJid, sizeof( szClientJid ));

			switch ( lParam ){
			case PROTOTYPE_SELFTYPING_OFF:
                JabberSend( jabberThreadInfo->s, "<message to=\"%s\"><x xmlns=\"jabber:x:event\"><id>%s</id></x></message>", szClientJid, ( item->messageEventIdStr==NULL )?"":item->messageEventIdStr );
				break;
			case PROTOTYPE_SELFTYPING_ON:
                JabberSend( jabberThreadInfo->s, "<message to=\"%s\"><x xmlns=\"jabber:x:event\"><composing/><id>%s</id></x></message>", szClientJid, ( item->messageEventIdStr==NULL )?"":item->messageEventIdStr );
				break;
		}	}

		JFreeVariant( &dbv );
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Service initialization code

static HANDLE hEventSettingChanged = NULL;
static HANDLE hEventContactDeleted = NULL;
static HANDLE hEventRebuildCMenu = NULL;

HANDLE hMenuAgent = NULL;
HANDLE hMenuChangePassword = NULL;
HANDLE hMenuGroupchat = NULL;
HANDLE hMenuVCard = NULL;

int JabberMenuHandleAgents( WPARAM wParam, LPARAM lParam );
int JabberMenuHandleChangePassword( WPARAM wParam, LPARAM lParam );
int JabberMenuHandleVcard( WPARAM wParam, LPARAM lParam );
int JabberMenuHandleRequestAuth( WPARAM wParam, LPARAM lParam );
int JabberMenuHandleGrantAuth( WPARAM wParam, LPARAM lParam );
int JabberMenuPrebuildContactMenu( WPARAM wParam, LPARAM lParam );

void JabberEnableMenuItems( BOOL bEnable )
{
	CLISTMENUITEM clmi = { 0 };
	clmi.cbSize = sizeof( CLISTMENUITEM );
	clmi.flags = CMIM_FLAGS;
	if ( !bEnable )
		clmi.flags += CMIF_GRAYED;

	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuAgent, ( LPARAM )&clmi );
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuChangePassword, ( LPARAM )&clmi );
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuGroupchat, ( LPARAM )&clmi );
}

int JabberSvcInit( void )
{
	hEventSettingChanged = HookEvent( ME_DB_CONTACT_SETTINGCHANGED, JabberDbSettingChanged );
	hEventContactDeleted = HookEvent( ME_DB_CONTACT_DELETED, JabberContactDeleted );
	hEventRebuildCMenu   = HookEvent( ME_CLIST_PREBUILDCONTACTMENU, JabberMenuPrebuildContactMenu );

	JCreateServiceFunction( PS_GETCAPS, JabberGetCaps );
	JCreateServiceFunction( PS_GETNAME, JabberGetName );
	JCreateServiceFunction( PS_LOADICON, JabberLoadIcon );
	JCreateServiceFunction( PS_BASICSEARCH, JabberBasicSearch );
	JCreateServiceFunction( PS_SEARCHBYEMAIL, JabberSearchByEmail );
	JCreateServiceFunction( PS_SEARCHBYNAME, JabberSearchByName );
	JCreateServiceFunction( PS_ADDTOLIST, JabberAddToList );
	JCreateServiceFunction( PS_ADDTOLISTBYEVENT, JabberAddToListByEvent );
	JCreateServiceFunction( PS_AUTHALLOW, JabberAuthAllow );
	JCreateServiceFunction( PS_AUTHDENY, JabberAuthDeny );
	JCreateServiceFunction( PS_SETSTATUS, JabberSetStatus );
	JCreateServiceFunction( PS_GETAVATARINFO, JabberGetAvatarInfo );
	JCreateServiceFunction( PS_GETSTATUS, JabberGetStatus );
	JCreateServiceFunction( PS_SETAWAYMSG, JabberSetAwayMsg );
	JCreateServiceFunction( PS_FILERESUME, JabberFileResume );

	JCreateServiceFunction( PSS_GETINFO, JabberGetInfo );
	JCreateServiceFunction( PSS_SETAPPARENTMODE, JabberSetApparentMode );
	JCreateServiceFunction( PSS_MESSAGE, JabberSendMessage );
	JCreateServiceFunction( PSS_GETAWAYMSG, JabberGetAwayMsg );
	JCreateServiceFunction( PSS_FILEALLOW, JabberFileAllow );
	JCreateServiceFunction( PSS_FILECANCEL, JabberFileCancel );
	JCreateServiceFunction( PSS_FILEDENY, JabberFileDeny );
	JCreateServiceFunction( PSS_FILE, JabberSendFile );
	JCreateServiceFunction( PSR_MESSAGE, JabberRecvMessage );
	JCreateServiceFunction( PSR_FILE, JabberRecvFile );
	JCreateServiceFunction( PSS_USERISTYPING, JabberUserIsTyping );

	CLISTMENUITEM mi, clmi;
	memset( &mi, 0, sizeof( CLISTMENUITEM ));
	mi.cbSize = sizeof( CLISTMENUITEM );
	memset( &clmi, 0, sizeof( CLISTMENUITEM ));
	clmi.cbSize = sizeof( CLISTMENUITEM );
	clmi.flags = CMIM_FLAGS | CMIF_GRAYED;

	// Add Jabber menu to the main menu
	char text[_MAX_PATH];
	strcpy( text, jabberProtoName );
	char* tDest = text + strlen( text );

	if ( !JGetByte( "DisableMainMenu", FALSE )) {
		// "Agents..."
		strcpy( tDest, "/Agents" );
		CreateServiceFunction( text, JabberMenuHandleAgents );

		mi.pszPopupName = jabberModuleName;
		mi.popupPosition = 500090000;
		mi.pszName = JTranslate( "Agents..." );
		mi.position = 2000050000;
		mi.hIcon = iconList[2];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_AGENTS ));
		mi.pszService = text;
		hMenuAgent = ( HANDLE ) JCallService( MS_CLIST_ADDMAINMENUITEM, 0, ( LPARAM )&mi );
		JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM ) hMenuAgent, ( LPARAM )&clmi );

		// "Change Password..."
		strcpy( tDest, "/ChangePassword" );
		CreateServiceFunction( text, JabberMenuHandleChangePassword );
		mi.pszName = JTranslate( "Change Password..." );
		mi.position = 2000050001;
		mi.hIcon = iconBigList[1];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_KEYS ));
		mi.pszService = text;
		hMenuChangePassword = ( HANDLE ) JCallService( MS_CLIST_ADDMAINMENUITEM, 0, ( LPARAM )&mi );
		JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM ) hMenuChangePassword, ( LPARAM )&clmi );

		// "Multi-User Conference..."
		strcpy( tDest, "/Groupchat" );
		CreateServiceFunction( text, JabberMenuHandleGroupchat );
		mi.pszName = JTranslate( "Multi-User Conference..." );
		mi.position = 2000050002;
		mi.hIcon = iconBigList[0];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_GROUP ));
		mi.pszService = text;
		hMenuGroupchat = ( HANDLE ) JCallService( MS_CLIST_ADDMAINMENUITEM, 0, ( LPARAM )&mi );
		JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM ) hMenuGroupchat, ( LPARAM )&clmi );

		// "Personal vCard..."
		strcpy( tDest,  "/Vcard" );
		CreateServiceFunction( text, JabberMenuHandleVcard );
		mi.pszName = JTranslate( "Personal vCard..." );
		mi.position = 2000050003;
		mi.hIcon = iconList[1];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_VCARD ));
		mi.pszService = text;
		hMenuVCard = ( HANDLE ) JCallService( MS_CLIST_ADDMAINMENUITEM, 0, ( LPARAM )&mi );
	}
	return 0;
}

int JabberSvcUninit()
{
	if ( hEventSettingChanged )   UnhookEvent( hEventSettingChanged );
	if ( hEventContactDeleted )   UnhookEvent( hEventContactDeleted );
	if ( hEventRebuildCMenu )     UnhookEvent( hEventRebuildCMenu );
	return 0;
}
