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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_misc.cpp,v $
Revision       : $Revision: 1.29 $
Last change on : $Date: 2006/05/14 13:19:26 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_list.h"
#ifdef __GNUC__
	#define __try
	#define __except(x) if (0) /* don't execute handler */
	#define __finally
	#define _try __try
	#define _except __except
	#define _finally __finally 
#endif

///////////////////////////////////////////////////////////////////////////////
// JabberAddContactToRoster() - adds a contact to the roster

void JabberAddContactToRoster( const TCHAR* jid, const TCHAR* nick, const TCHAR* grpName, JABBER_SUBSCRIPTION subscription )
{
	XmlNodeIq iq( "set" );
	XmlNode* query = iq.addQuery( "jabber:iq:roster" );
	XmlNode* item = query->addChild( "item" ); item->addAttr( "name", nick ); item->addAttr( "jid", jid );
	switch( subscription ) {
		case SUB_BOTH: item->addAttr( "subscription", "both" ); break;
		case SUB_TO:   item->addAttr( "subscription", "to" );   break;
		case SUB_FROM: item->addAttr( "subscription", "from" ); break;
		default:       item->addAttr( "subscription", "none" ); break;
	}

	if ( grpName != NULL )
		item->addChild( "group", grpName );
	JabberSend( jabberThreadInfo->s, iq );
}

///////////////////////////////////////////////////////////////////////////////
// JabberChatDllError() - missing CHAT.DLL

void JabberChatDllError()
{
	MessagePopup( NULL,
		TranslateT( "CHAT plugin is required for conferences. Install it before chatting" ),
		TranslateT( "Jabber Error Message" ), MB_ICONASTERISK|MB_OK|MB_SETFOREGROUND );
}

///////////////////////////////////////////////////////////////////////////////
// JabberCompareJids

int JabberCompareJids( const TCHAR* jid1, const TCHAR* jid2 )
{
	if ( !lstrcmpi( jid1, jid2 ))
		return 0;

	// match only node@domain part
	TCHAR szTempJid1[ JABBER_MAX_JID_LEN ], szTempJid2[ JABBER_MAX_JID_LEN ];
	return lstrcmpi(
		JabberStripJid( jid1, szTempJid1, sizeof szTempJid1 ),
		JabberStripJid( jid2, szTempJid2, sizeof szTempJid2 ));
}

///////////////////////////////////////////////////////////////////////////////
// JabberContactListCreateGroup()

static void JabberContactListCreateClistGroup( TCHAR* groupName )
{
	char str[33];
	int i;
	DBVARIANT dbv;

	for ( i=0;;i++ ) {
		itoa( i, str, 10 );
		if ( DBGetContactSettingTString( NULL, "CListGroups", str, &dbv ))
			break;
		TCHAR* name = dbv.ptszVal;
		if ( name[0]!='\0' && !_tcscmp( name+1, groupName )) {
			// Already exists, no need to create
			JFreeVariant( &dbv );
			return;
		}
		JFreeVariant( &dbv );
	}

	// Create new group with id = i ( str is the text representation of i )
	TCHAR newName[128];
	newName[0] = 1 | GROUPF_EXPANDED;
	_tcsncpy( newName+1, groupName, SIZEOF( newName )-1 );
	newName[ SIZEOF( newName )-1] = '\0';
	DBWriteContactSettingTString( NULL, "CListGroups", str, newName );
	JCallService( MS_CLUI_GROUPADDED, i+1, 0 );
}

void JabberContactListCreateGroup( TCHAR* groupName )
{
	TCHAR name[128], *p;

	if ( groupName==NULL || groupName[0]=='\0' || groupName[0]=='\\' ) return;

	_tcsncpy( name, groupName, SIZEOF( name ));
	name[ SIZEOF( name )-1] = '\0';
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

void JabberDBAddAuthRequest( TCHAR* jid, TCHAR* nick )
{
	HANDLE hContact = JabberDBCreateContact( jid, NULL, FALSE, TRUE );
	JDeleteSetting( hContact, "Hidden" );
	JSetStringT( hContact, "Nick", nick );

	#if defined( _UNICODE )
		char* szJid = u2a( jid );
		char* szNick = u2a( nick );
	#else
		char* szJid = jid;
		char* szNick = nick;
	#endif

	//blob is: uin( DWORD ), hContact( HANDLE ), nick( ASCIIZ ), first( ASCIIZ ), last( ASCIIZ ), email( ASCIIZ ), reason( ASCIIZ )
	//blob is: 0( DWORD ), hContact( HANDLE ), nick( ASCIIZ ), ""( ASCIIZ ), ""( ASCIIZ ), email( ASCIIZ ), ""( ASCIIZ )
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof( DBEVENTINFO );
	dbei.szModule = jabberProtoName;
	dbei.timestamp = ( DWORD )time( NULL );
	dbei.flags = 0;
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = sizeof( DWORD )+ sizeof( HANDLE ) + strlen( szNick ) + strlen( szJid ) + 5;
	PBYTE pCurBlob = dbei.pBlob = ( PBYTE ) mir_alloc( dbei.cbBlob );
	*(( PDWORD ) pCurBlob ) = 0; pCurBlob += sizeof( DWORD );
	*(( PHANDLE ) pCurBlob ) = hContact; pCurBlob += sizeof( HANDLE );
	strcpy(( char* )pCurBlob, szNick ); pCurBlob += strlen( szNick )+1;
	*pCurBlob = '\0'; pCurBlob++;		//firstName
	*pCurBlob = '\0'; pCurBlob++;		//lastName
	strcpy(( char* )pCurBlob, szJid ); pCurBlob += strlen( szJid )+1;
	*pCurBlob = '\0';					//reason

	JCallService( MS_DB_EVENT_ADD, ( WPARAM ) ( HANDLE ) NULL, ( LPARAM )&dbei );
	JabberLog( "Setup DBAUTHREQUEST with nick='" TCHAR_STR_PARAM "' jid='" TCHAR_STR_PARAM "'", szNick, szJid );

	#if defined( _UNICODE )
		mir_free( szJid );
		mir_free( szNick );
	#endif
}

///////////////////////////////////////////////////////////////////////////////
// JabberDBCreateContact()
// jid & nick are passed in TXT

HANDLE JabberDBCreateContact( TCHAR* jid, TCHAR* nick, BOOL temporary, BOOL stripResource )
{
	TCHAR* s, *p, *q;
	int len;
	char* szProto;

	if ( jid==NULL || jid[0]=='\0' )
		return NULL;

	s = mir_tstrdup( jid );
	q = NULL;
	// strip resource if present
	if (( p = _tcschr( s, '@' )) != NULL )
		if (( q = _tcschr( p, '/' )) != NULL )
			*q = '\0';

	if ( !stripResource && q!=NULL )	// so that resource is not stripped
		*q = '/';
	len = _tcslen( s );

	// We can't use JabberHContactFromJID() here because of the stripResource option
	HANDLE hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL ) {
		szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
		if ( szProto!=NULL && !strcmp( jabberProtoName, szProto )) {
			DBVARIANT dbv;
			if ( !JGetStringT( hContact, "jid", &dbv )) {
				p = dbv.ptszVal;
				if ( p && ( int )_tcslen( p )>=len && ( p[len]=='\0'||p[len]=='/' ) && !_tcsnicmp( p, s, len )) {
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
		JSetStringT( hContact, "jid", s );
		if ( nick != NULL && *nick != '\0' )
			JSetStringT( hContact, "Nick", nick );
		if ( temporary )
			DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
		JabberLog( "Create Jabber contact jid=" TCHAR_STR_PARAM ", nick=" TCHAR_STR_PARAM, s, nick );
	}

	mir_free( s );
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
		mir_free( hash );
	}
	else if ( jabberThreadInfo != NULL ) {
		mir_snprintf( pszDest + tPathLen, MAX_PATH - tPathLen, TCHAR_STR_PARAM"@%s avatar.%s", jabberThreadInfo->username, jabberThreadInfo->server, szFileType );
	}
	else {
		DBVARIANT dbv1, dbv2;
		BOOL res1 = DBGetContactSetting( NULL, jabberProtoName, "LoginName", &dbv1 );
		BOOL res2 = DBGetContactSetting( NULL, jabberProtoName, "LoginServer", &dbv2 );
		mir_snprintf( pszDest + tPathLen, MAX_PATH - tPathLen, "%s@%s avatar.%s", 
			res1 ? "noname" : dbv1.pszVal, 
			res2 ? jabberProtoName : dbv2.pszVal,
			szFileType );
		if (!res1) JFreeVariant( &dbv1 );
		if (!res2) JFreeVariant( &dbv2 );
	}
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
	JabberLog( "Thread started: %08X %d", callercode, GetCurrentThreadId());
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
		return mir_strdup( pszText );

	char* pszNewText = (char*)mir_alloc( strlen( pszText ) + 1 + nChars ), *s, *d;
	if ( pszNewText == NULL )
		return mir_strdup( pszText );

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
