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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_misc.cpp,v $
Revision       : $Revision: 1.26 $
Last change on : $Date: 2006/03/21 13:07:30 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_list.h"

///////////////////////////////////////////////////////////////////////////////
// JabberAddContactToRoster() - adds a contact to the roster

void JabberAddContactToRoster( const char* jid, const char* nick, const char* grpName )
{
	char* szJid  = JabberUrlEncode( jid );
	char* szNick = JabberUrlEncode( nick );

	if ( grpName != NULL ) {
		char* szGroup = JabberUrlEncode( grpName );
		JabberSend( jabberThreadInfo->s,
            "<iq type=\"set\"><query xmlns=\"jabber:iq:roster\"><item name=\"%s\" jid=\"%s\"><group>%s</group></item></query></iq>",
			szNick, szJid, szGroup );
		free( szGroup );
	}
	else
		JabberSend( jabberThreadInfo->s,
            "<iq type=\"set\"><query xmlns=\"jabber:iq:roster\"><item name=\"%s\" jid=\"%s\"/></query></iq>",
			szNick, szJid );

	free( szNick );
	free( szJid );
}

///////////////////////////////////////////////////////////////////////////////
// JabberChatDllError() - missing CHAT.DLL

void JabberChatDllError()
{
	MessageBox( NULL,
		TranslateT( "CHAT plugin is required for conferences. Install it before chatting" ),
		TranslateT( "Jabber Error Message" ), MB_OK|MB_SETFOREGROUND );
}

///////////////////////////////////////////////////////////////////////////////
// JabberCompareJids

int JabberCompareJids( const char* jid1, const char* jid2 )
{
	if ( !JabberUtfCompareI( jid1, jid2 ))
		return 0;

	// match only node@domain part
	char szTempJid1[ JABBER_MAX_JID_LEN ], szTempJid2[ JABBER_MAX_JID_LEN ];
	return JabberUtfCompareI(
		JabberStripJid( jid1, szTempJid1, sizeof szTempJid1 ),
		JabberStripJid( jid2, szTempJid2, sizeof szTempJid2 ));
}

///////////////////////////////////////////////////////////////////////////////
// JabberContactListCreateGroup()

static void JabberContactListCreateClistGroup( char* groupName )
{
	char str[33], newName[128];
	int i;
	DBVARIANT dbv;
	char* name;

	for ( i=0;;i++ ) {
		itoa( i, str, 10 );
		if ( DBGetContactSettingStringUtf( NULL, "CListGroups", str, &dbv ))
			break;
		name = dbv.pszVal;
		if ( name[0]!='\0' && !strcmp( name+1, groupName )) {
			// Already exists, no need to create
			JFreeVariant( &dbv );
			return;
		}
		JFreeVariant( &dbv );
	}

	// Create new group with id = i ( str is the text representation of i )
	newName[0] = 1 | GROUPF_EXPANDED;
	strncpy( newName+1, groupName, sizeof( newName )-1 );
	newName[sizeof( newName )-1] = '\0';
	DBWriteContactSettingStringUtf( NULL, "CListGroups", str, newName );
	JCallService( MS_CLUI_GROUPADDED, i+1, 0 );
}

void JabberContactListCreateGroup( char* groupName )
{
	char name[128];
	char* p;

	if ( groupName==NULL || groupName[0]=='\0' || groupName[0]=='\\' ) return;

	strncpy( name, groupName, sizeof( name ));
	name[sizeof( name )-1] = '\0';
	for ( p=name; *p!='\0'; p++ ) {
		if ( *p == '\\' ) {
			*p = '\0';
			JabberContactListCreateClistGroup( name );
			*p = '\\';
		}
	}
	JabberContactListCreateClistGroup( name );
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBAddAuthRequest()

void JabberDBAddAuthRequest( char* jid, char* nick )
{
	HANDLE hContact = JabberDBCreateContact( jid, NULL, FALSE, TRUE );
	JDeleteSetting( hContact, "Hidden" );
	JSetString( hContact, "Nick", nick );

	//blob is: uin( DWORD ), hContact( HANDLE ), nick( ASCIIZ ), first( ASCIIZ ), last( ASCIIZ ), email( ASCIIZ ), reason( ASCIIZ )
	//blob is: 0( DWORD ), hContact( HANDLE ), nick( ASCIIZ ), ""( ASCIIZ ), ""( ASCIIZ ), email( ASCIIZ ), ""( ASCIIZ )
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof( DBEVENTINFO );
	dbei.szModule = jabberProtoName;
	dbei.timestamp = ( DWORD )time( NULL );
	dbei.flags = 0;
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = sizeof( DWORD )+ sizeof( HANDLE ) + strlen( nick ) + strlen( jid ) + 5;
	PBYTE pCurBlob = dbei.pBlob = ( PBYTE ) malloc( dbei.cbBlob );
	*(( PDWORD ) pCurBlob ) = 0; pCurBlob += sizeof( DWORD );
	*(( PHANDLE ) pCurBlob ) = hContact; pCurBlob += sizeof( HANDLE );
	strcpy(( char* )pCurBlob, nick ); pCurBlob += strlen( nick )+1;
	*pCurBlob = '\0'; pCurBlob++;		//firstName
	*pCurBlob = '\0'; pCurBlob++;		//lastName
	strcpy(( char* )pCurBlob, jid ); pCurBlob += strlen( jid )+1;
	*pCurBlob = '\0';					//reason

	JCallService( MS_DB_EVENT_ADD, ( WPARAM ) ( HANDLE ) NULL, ( LPARAM )&dbei );
	JabberLog( "Setup DBAUTHREQUEST with nick='%s' jid='%s'", nick, jid );
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBCreateContact()
// jid & nick are passed in TXT

HANDLE JabberDBCreateContact( char* jid, char* nick, BOOL temporary, BOOL stripResource )
{
	char* s, *p, *q;
	int len;
	char* szProto;

	if ( jid==NULL || jid[0]=='\0' )
		return NULL;

	s = _strdup( jid );
	q = NULL;
	// strip resource if present
	if (( p=strchr( s, '@' )) != NULL )
		if (( q=strchr( p, '/' )) != NULL )
			*q = '\0';

	if ( !stripResource && q!=NULL )	// so that resource is not stripped
		*q = '/';
	len = strlen( s );

	// We can't use JabberHContactFromJID() here because of the stripResource option
	HANDLE hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL ) {
		szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
		if ( szProto!=NULL && !strcmp( jabberProtoName, szProto )) {
			DBVARIANT dbv;
			if ( !JGetStringUtf( hContact, "jid", &dbv )) {
				p = dbv.pszVal;
				if ( p && ( int )strlen( p )>=len && ( p[len]=='\0'||p[len]=='/' ) && !_strnicmp( p, s, len )) {
					JFreeVariant( &dbv );
					break;
				}
				JFreeVariant( &dbv );
		}	}
		hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM ) hContact, 0 );
	}

	if ( hContact == NULL ) {
		hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_ADD, 0, 0 );
		JCallService( MS_PROTO_ADDTOCONTACT, ( WPARAM ) hContact, ( LPARAM )jabberProtoName );
		JSetStringUtf( hContact, "jid", s );
		if ( nick != NULL && *nick != '\0' )
			JSetStringUtf( hContact, "Nick", nick );
		if ( temporary )
			DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
		JabberLog( "Create Jabber contact jid=%s, nick=%s", s, nick );
	}

	free( s );
	return hContact;
}

///////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarFileName() - gets a file name for the avatar image

void JabberGetAvatarFileName( HANDLE hContact, char* pszDest, int cbLen )
{
	JCallService( MS_DB_GETPROFILEPATH, cbLen, LPARAM( pszDest ));

	int tPathLen = strlen( pszDest );
	tPathLen += mir_snprintf( pszDest + tPathLen, MAX_PATH - tPathLen, "\\Jabber\\"  );
	CreateDirectoryA( pszDest, NULL );

	char* szFileType;
	switch( JGetByte( hContact, "AvatarType", PA_FORMAT_PNG )) {
		case PA_FORMAT_JPEG: szFileType = "jpg";   break;
		case PA_FORMAT_PNG:  szFileType = "png";   break;
		case PA_FORMAT_GIF:  szFileType = "gif";   break;
		case PA_FORMAT_BMP:  szFileType = "bmp";   break;
	}

	if ( hContact != NULL ) {
		char str[ 256 ];
		DBVARIANT dbv;
		if ( !JGetStringUtf( hContact, "jid", &dbv )) {
			strncpy( str, dbv.pszVal, sizeof str );
			str[ sizeof(str)-1 ] = 0;
			JFreeVariant( &dbv );
		}
		else ltoa(( long )hContact, str, 10 );

		char* hash = JabberSha1( str );
		mir_snprintf( pszDest + tPathLen, MAX_PATH - tPathLen, "%s.%s", hash, szFileType );
		free( hash );
	}
	else mir_snprintf( pszDest + tPathLen, MAX_PATH - tPathLen, "%s avatar.%s", jabberProtoName, szFileType );
}

///////////////////////////////////////////////////////////////////////////////
// JabberForkThread()

struct FORK_ARG {
	HANDLE hEvent;
	void ( __cdecl *threadcode )( void* );
	void *arg;
};

static void __cdecl forkthread_r( struct FORK_ARG *fa )
{
	void ( *callercode )( void* ) = fa->threadcode;
	void *arg = fa->arg;
	JCallService( MS_SYSTEM_THREAD_PUSH, 0, 0 );
	SetEvent( fa->hEvent );
	__try {
		callercode( arg );
	} __finally {
		JCallService( MS_SYSTEM_THREAD_POP, 0, 0 );
	}
	return;
}

ULONG JabberForkThread( void ( __cdecl *threadcode )( void* ), unsigned long stacksize, void *arg )
{
	struct FORK_ARG fa;
	fa.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	fa.threadcode = threadcode;
	fa.arg = arg;

	ULONG rc = _beginthread(( JABBER_THREAD_FUNC )forkthread_r, stacksize, &fa );
	if (( unsigned long ) -1L != rc )
		WaitForSingleObject( fa.hEvent, INFINITE );

	CloseHandle( fa.hEvent );
	return rc;
}

///////////////////////////////////////////////////////////////////////////////
// JabberSetServerStatus()

void JabberSetServerStatus( int iNewStatus )
{
	if ( !jabberConnected )
		return;

	// change status
	int oldStatus = jabberStatus;
	switch ( iNewStatus ) {
	case ID_STATUS_ONLINE:
	case ID_STATUS_NA:
	case ID_STATUS_FREECHAT:
	case ID_STATUS_INVISIBLE:
		jabberStatus = iNewStatus;
		break;
	case ID_STATUS_AWAY:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		jabberStatus = ID_STATUS_AWAY;
		break;
	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		jabberStatus = ID_STATUS_DND;
		break;
	default:
		return;
	}

	// send presence update
	JabberSendPresence( jabberStatus );
	JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );
}

// Process a string, and double all % characters, according to chat.dll's restrictions
// Returns a pointer to the new string (old one is not freed)
char* EscapeChatTags(char* pszText)
{
	int nChars = 0;
	for ( char* p = pszText; ( p = strchr( p, '%' )) != NULL; p++ )
		nChars++;

	if ( nChars == 0 )
		return _strdup( pszText );

	char* pszNewText = (char*)malloc( strlen( pszText ) + 1 + nChars ), *s, *d;
	if ( pszNewText == NULL )
		return _strdup( pszText );

	for ( s = pszText, d = pszNewText; *s; s++ ) {
		if ( *s == '%' )
			*d++ = '%';
		*d++ = *s;
	}
	*d = 0;
	return pszNewText;
}

char* UnEscapeChatTags(char* str_in)
{
	char* s = str_in, *d = str_in;
	while ( *s ) {
		if (( *s == '%' && s[1] == '%' ) || ( *s == '\n' && s[1] == '\n' ))
			s++;
		*d++ = *s++;
	}
	*d = 0;
	return str_in;
}
