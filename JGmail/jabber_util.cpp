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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_util.cpp,v $
Revision       : $Revision: 1.28 $
Last change on : $Date: 2006/02/04 23:53:48 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_ssl.h"
#include "jabber_list.h"
#include "sha1.h"

extern CRITICAL_SECTION mutex;
extern UINT jabberCodePage;

static CRITICAL_SECTION serialMutex;
static unsigned int serial;

void __stdcall JabberSerialInit( void )
{
	InitializeCriticalSection( &serialMutex );
	serial = 0;
}

void __stdcall JabberSerialUninit( void )
{
	DeleteCriticalSection( &serialMutex );
}

unsigned int __stdcall JabberSerialNext( void )
{
	unsigned int ret;

	EnterCriticalSection( &serialMutex );
	ret = serial;
	serial++;
	LeaveCriticalSection( &serialMutex );
	return ret;
}

void __stdcall JabberLog( const char* fmt, ... )
{
	va_list vararg;
	va_start( vararg, fmt );
	char* str = ( char* )alloca( 32000 );
	mir_vsnprintf( str, 32000, fmt, vararg );
	va_end( vararg );

	JCallService( MS_NETLIB_LOG, ( WPARAM )hNetlibUser, ( LPARAM )str );
}

// Caution: DO NOT use JabberSend() to send binary ( non-string ) data
int __stdcall JabberSend( HANDLE hConn, const char* fmt, ... )
{
	char* str;
	int size;
	va_list vararg;
	int result;
	PVOID ssl;
	char* szLogBuffer;

	EnterCriticalSection( &mutex );

	va_start( vararg,fmt );
	size = 512;
	str = ( char* )malloc( size );
	while ( _vsnprintf( str, size, fmt, vararg ) == -1 ) {
		size += 512;
		str = ( char* )realloc( str, size );
	}
	va_end( vararg );

	size = strlen( str );
	if (( ssl=JabberSslHandleToSsl( hConn )) != NULL ) {
		if ( DBGetContactSettingByte( NULL, "Netlib", "DumpSent", TRUE ) == TRUE ) {
			if (( szLogBuffer=( char* )malloc( size+32 )) != NULL ) {
				strcpy( szLogBuffer, "( SSL ) Data sent\n" );
				memcpy( szLogBuffer+strlen( szLogBuffer ), str, size+1 /* also copy \0 */ );
				Netlib_Logf( hNetlibUser, "%s", szLogBuffer );	// %s to protect against when fmt tokens are in szLogBuffer causing crash
				free( szLogBuffer );
		}	}

		result = pfn_SSL_write( ssl, str, size );
	}
	else
		result = JabberWsSend( hConn, str, size );
	LeaveCriticalSection( &mutex );

	free( str );
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// JabberHContactFromJID - looks for the HCONTACT with required JID

HANDLE __stdcall JabberHContactFromJID( const char* jid )
{
	if ( jid == NULL )
		return ( HANDLE )NULL;

	HANDLE hContactMatched = NULL;
	HANDLE hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL ) {
		char* szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
		if ( szProto != NULL && !strcmp( jabberProtoName, szProto )) {
			DBVARIANT dbv;
			int result = JGetStringUtf( hContact, "jid", &dbv );
			if ( result )
				result = JGetStringUtf( hContact, "ChatRoomID", &dbv );

			if ( !result ) {
				int result;
				if ( JGetByte( hContact, "ChatRoom", 0 ))
					result = JabberUtfCompareI( jid, dbv.pszVal );
				else
					result = JabberCompareJids( jid, dbv.pszVal );
				JFreeVariant( &dbv );
				if ( !result ) {
					hContactMatched = hContact;
					break;
		}	}	}

		hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM ) hContact, 0 );
	}

	return hContactMatched;
}

char* __stdcall JabberNickFromJID( const char* jid )
{
	const char* p;
	char* nick;

	if (( p=strchr( jid, '@' )) == NULL )
		p = strchr( jid, '/' );

	if ( p != NULL ) {
		if (( nick=( char* )malloc(( p-jid )+1 )) != NULL ) {
			strncpy( nick, jid, p-jid );
			nick[p-jid] = '\0';
		}
	}
	else nick = _strdup( jid );

	return nick;
}
char* __stdcall JabberUrlDecode( char* str )
{
	char* p, *q;

	if ( str == NULL )
		return NULL;

	for ( p=q=str; *p!='\0'; p++,q++ ) {
		if ( *p == '&' ) {
			if ( !strncmp( p, "&amp;", 5 )) {	*q = '&'; p += 4; }
			else if ( !strncmp( p, "&apos;", 6 )) { *q = '\''; p += 5; }
			else if ( !strncmp( p, "&gt;", 4 )) { *q = '>'; p += 3; }
			else if ( !strncmp( p, "&lt;", 4 )) { *q = '<'; p += 3; }
			else if ( !strncmp( p, "&quot;", 6 )) { *q = '"'; p += 5; }
			else { *q = *p;	}
		}
		else {
			*q = *p;
		}
	}
	*q = '\0';
	return str;
}

void __stdcall JabberUrlDecodeW( WCHAR* str )
{
	if ( str == NULL )
		return;

	WCHAR* p, *q;
	for ( p=q=str; *p!='\0'; p++,q++ ) {
		if ( *p == '&' ) {
			if ( !wcsncmp( p, L"&amp;", 5 )) {	*q = '&'; p += 4; }
			else if ( !wcsncmp( p, L"&apos;", 6 )) { *q = '\''; p += 5; }
			else if ( !wcsncmp( p, L"&gt;", 4 )) { *q = '>'; p += 3; }
			else if ( !wcsncmp( p, L"&lt;", 4 )) { *q = '<'; p += 3; }
			else if ( !wcsncmp( p, L"&quot;", 6 )) { *q = '"'; p += 5; }
			else { *q = *p;	}
		}
		else {
			*q = *p;
		}
	}
	*q = '\0';
}

char* __stdcall JabberUrlEncode( const char* str )
{
	char* s, *p, *q;
	int c;

	if ( str == NULL )
		return NULL;

	for ( c=0,p=( char* )str; *p!='\0'; p++ ) {
		switch ( *p ) {
		case '&': c += 5; break;
		case '\'': c += 6; break;
		case '>': c += 4; break;
		case '<': c += 4; break;
		case '"': c += 6; break;
		default: c++; break;
		}
	}
	if (( s=( char* )malloc( c+1 )) != NULL ) {
		for ( p=( char* )str,q=s; *p!='\0'; p++ ) {
			switch ( *p ) {
			case '&': strcpy( q, "&amp;" ); q += 5; break;
			case '\'': strcpy( q, "&apos;" ); q += 6; break;
			case '>': strcpy( q, "&gt;" ); q += 4; break;
			case '<': strcpy( q, "&lt;" ); q += 4; break;
			case '"': strcpy( q, "&quot;" ); q += 6; break;
			default: *q = *p; q++; break;
			}
		}
		*q = '\0';
	}

	return s;
}

static void __stdcall sttUtf8Decode( const BYTE* str, wchar_t* tempBuf )
{
	wchar_t* d = tempBuf;
	BYTE* s = ( BYTE* )str;

	while( *s )
	{
		if (( *s & 0x80 ) == 0 ) {
			*d++ = *s++;
			continue;
		}

		if (( s[0] & 0xE0 ) == 0xE0 && ( s[1] & 0xC0 ) == 0x80 && ( s[2] & 0xC0 ) == 0x80 ) {
			*d++ = (( WORD )( s[0] & 0x0F ) << 12 ) + ( WORD )(( s[1] & 0x3F ) << 6 ) + ( WORD )( s[2] & 0x3F );
			s += 3;
			continue;
		}

		if (( s[0] & 0xE0 ) == 0xC0 && ( s[1] & 0xC0 ) == 0x80 ) {
			*d++ = ( WORD )(( s[0] & 0x1F ) << 6 ) + ( WORD )( s[1] & 0x3F );
			s += 2;
			continue;
		}

		*d++ = *s++;
	}

	*d = 0;
}

int __stdcall JabberUtfCompareI( const char* s1, const char* s2 )
{
	int l1 = strlen( s1 ), l2 = strlen( s2 );
	wchar_t* w1 = ( wchar_t* )alloca(( l1+1 )*sizeof( wchar_t ));
	wchar_t* w2 = ( wchar_t* )alloca(( l2+1 )*sizeof( wchar_t ));
	sttUtf8Decode(( BYTE* )s1, w1 );
	sttUtf8Decode(( BYTE* )s2, w2 );
	return _wcsicmp( w1, w2 );
}

char* __stdcall JabberUtf8Decode( char* str, WCHAR** ucs2 )
{
	if ( str == NULL )
		return NULL;

	int len = strlen( str );
	if ( len < 2 ) {
		if ( ucs2 != NULL ) {
			*ucs2 = ( wchar_t* )malloc(( len+1 )*sizeof( wchar_t ));
			MultiByteToWideChar( CP_ACP, 0, str, len, *ucs2, len );
			( *ucs2 )[ len ] = 0;
		}
		return str;
	}

	wchar_t* tempBuf = ( wchar_t* )alloca(( len+1 )*sizeof( wchar_t ));
	sttUtf8Decode(( BYTE* )str, tempBuf );

	if ( ucs2 != NULL ) {
		int fullLen = ( len+1 )*sizeof( wchar_t );
		*ucs2 = ( wchar_t* )malloc( fullLen );
		memcpy( *ucs2, tempBuf, fullLen );
	}

   WideCharToMultiByte( CP_ACP, 0, tempBuf, -1, str, len, NULL, NULL );
	return str;
}

char* __stdcall JabberUtf8EncodeW( const WCHAR* wstr )
{
	const WCHAR* w;

	// Convert unicode to utf8
	int len = 0;
	for ( w = wstr; *w; w++ ) {
		if ( *w < 0x0080 ) len++;
		else if ( *w < 0x0800 ) len += 2;
		else len += 3;
	}

	unsigned char* szOut = ( unsigned char* )malloc( len+1 );
	if ( szOut == NULL )
		return NULL;

	int i = 0;
	for ( w = wstr; *w; w++ ) {
		if ( *w < 0x0080 )
			szOut[i++] = ( unsigned char ) *w;
		else if ( *w < 0x0800 ) {
			szOut[i++] = 0xc0 | (( *w ) >> 6 );
			szOut[i++] = 0x80 | (( *w ) & 0x3f );
		}
		else {
			szOut[i++] = 0xe0 | (( *w ) >> 12 );
			szOut[i++] = 0x80 | (( ( *w ) >> 6 ) & 0x3f );
			szOut[i++] = 0x80 | (( *w ) & 0x3f );
	}	}

	szOut[ i ] = '\0';
	return ( char* )szOut;
}

char* __stdcall JabberUtf8Encode( const char* str )
{
	if ( str == NULL )
		return NULL;

	// Convert local codepage to unicode
	int len = strlen( str );
	WCHAR* wszTemp = ( WCHAR* )alloca( sizeof( WCHAR )*( len+1 ));
	MultiByteToWideChar( jabberCodePage, 0, str, -1, wszTemp, len+1 );

	return JabberUtf8EncodeW( wszTemp );
}

char* __stdcall JabberSha1( char* str )
{
	SHA1Context sha;
	uint8_t digest[20];
	char* result;
	int i;

	if ( str==NULL )
		return NULL;
	if ( SHA1Reset( &sha ))
		return NULL;
	if ( SHA1Input( &sha, ( const unsigned __int8* )str, strlen( str )) )
		return NULL;
	if ( SHA1Result( &sha, digest ))
		return NULL;
	if (( result=( char* )malloc( 41 )) == NULL )
		return NULL;
	for ( i=0; i<20; i++ )
		sprintf( result+( i<<1 ), "%02x", digest[i] );
	return result;
}

char* __stdcall JabberUnixToDos( const char* str )
{
	char* p, *q, *res;
	int extra;

	if ( str==NULL || str[0]=='\0' )
		return NULL;

	extra = 0;
	for ( p=( char* )str; *p!='\0'; p++ ) {
		if ( *p == '\n' )
			extra++;
	}
	if (( res=( char* )malloc( strlen( str )+extra+1 )) != NULL ) {
		for ( p=( char* )str,q=res; *p!='\0'; p++,q++ ) {
			if ( *p == '\n' ) {
				*q = '\r';
				q++;
			}
			*q = *p;
		}
		*q = '\0';
	}
	return res;
}

WCHAR* __stdcall JabberUnixToDosW( const WCHAR* str )
{
	if ( str==NULL || str[0]=='\0' )
		return NULL;

	const WCHAR* p;
	WCHAR* q, *res;
	int extra = 0;

	for ( p = str; *p!='\0'; p++ ) {
		if ( *p == '\n' )
			extra++;
	}
	if (( res = ( WCHAR* )malloc( sizeof( WCHAR )*( wcslen( str ) + extra + 1 )) ) != NULL ) {
		for ( p = str,q=res; *p!='\0'; p++,q++ ) {
			if ( *p == '\n' ) {
				*q = '\r';
				q++;
			}
			*q = *p;
		}
		*q = '\0';
	}
	return res;
}

char* __stdcall JabberHttpUrlEncode( const char* str )
{
	unsigned char* p, *q, *res;

	if ( str == NULL ) return NULL;
	res = ( BYTE* ) malloc( 3*strlen( str ) + 1 );
	for ( p=( BYTE* )str,q=res; *p!='\0'; p++,q++ ) {
		if (( *p>='A' && *p<='Z' ) || ( *p>='a' && *p<='z' ) || ( *p>='0' && *p<='9' ) || strchr( "$-_.+!*'(),", *p )!=NULL ) {
			*q = *p;
		}
		else {
			sprintf(( char* )q, "%%%02X", *p );
			q += 2;
		}
	}
	*q = '\0';
	return ( char* )res;
}

void __stdcall JabberHttpUrlDecode( char* str )
{
	unsigned char* p, *q;
	unsigned int code;

	if ( str == NULL ) return;
	for ( p=q=( BYTE* )str; *p!='\0'; p++,q++ ) {
		if ( *p=='%' && *( p+1 )!='\0' && isxdigit( *( p+1 )) && *( p+2 )!='\0' && isxdigit( *( p+2 )) ) {
			sscanf(( char* )p+1, "%2x", &code );
			*q = ( unsigned char ) code;
			p += 2;
		}
		else {
			*q = *p;
		}
	}
	*q = '\0';
}

int __stdcall JabberCombineStatus( int status1, int status2 )
{
	// Combine according to the following priority ( high to low )
	// ID_STATUS_FREECHAT
	// ID_STATUS_ONLINE
	// ID_STATUS_DND
	// ID_STATUS_AWAY
	// ID_STATUS_NA
	// ID_STATUS_INVISIBLE ( valid only for TLEN_PLUGIN )
	// ID_STATUS_OFFLINE
	// other ID_STATUS in random order ( actually return status1 )
	if ( status1==ID_STATUS_FREECHAT || status2==ID_STATUS_FREECHAT )
		return ID_STATUS_FREECHAT;
	if ( status1==ID_STATUS_ONLINE || status2==ID_STATUS_ONLINE )
		return ID_STATUS_ONLINE;
	if ( status1==ID_STATUS_DND || status2==ID_STATUS_DND )
		return ID_STATUS_DND;
	if ( status1==ID_STATUS_AWAY || status2==ID_STATUS_AWAY )
		return ID_STATUS_AWAY;
	if ( status1==ID_STATUS_NA || status2==ID_STATUS_NA )
		return ID_STATUS_NA;
	if ( status1==ID_STATUS_INVISIBLE || status2==ID_STATUS_INVISIBLE )
		return ID_STATUS_INVISIBLE;
	if ( status1==ID_STATUS_OFFLINE || status2==ID_STATUS_OFFLINE )
		return ID_STATUS_OFFLINE;
	return status1;
}

struct tagErrorCodeToStr {
	int code;
	char* str;
} JabberErrorCodeToStrMapping[] = {
	{JABBER_ERROR_REDIRECT,					"Redirect"},
	{JABBER_ERROR_BAD_REQUEST,				"Bad request"},
	{JABBER_ERROR_UNAUTHORIZED,				"Unauthorized"},
	{JABBER_ERROR_PAYMENT_REQUIRED,			"Payment required"},
	{JABBER_ERROR_FORBIDDEN,				"Forbidden"},
	{JABBER_ERROR_NOT_FOUND,				"Not found"},
	{JABBER_ERROR_NOT_ALLOWED,				"Not allowed"},
	{JABBER_ERROR_NOT_ACCEPTABLE,			"Not acceptable"},
	{JABBER_ERROR_REGISTRATION_REQUIRED,	"Registration required"},
	{JABBER_ERROR_REQUEST_TIMEOUT,			"Request timeout"},
	{JABBER_ERROR_CONFLICT,					"Conflict"},
	{JABBER_ERROR_INTERNAL_SERVER_ERROR,	"Internal server error"},
	{JABBER_ERROR_NOT_IMPLEMENTED,			"Not implemented"},
	{JABBER_ERROR_REMOTE_SERVER_ERROR,		"Remote server error"},
	{JABBER_ERROR_SERVICE_UNAVAILABLE,		"Service unavailable"},
	{JABBER_ERROR_REMOTE_SERVER_TIMEOUT,	"Remote server timeout"},
	{-1, "Unknown error"}
};

char* __stdcall JabberErrorStr( int errorCode )
{
	int i;

	for ( i=0; JabberErrorCodeToStrMapping[i].code!=-1 && JabberErrorCodeToStrMapping[i].code!=errorCode; i++ );
	return JabberErrorCodeToStrMapping[i].str;
}

char* __stdcall JabberErrorMsg( XmlNode *errorNode )
{
	char* errorStr, *str;
	int errorCode;

	errorStr = ( char* )malloc( 256 );
	if ( errorNode == NULL ) {
		mir_snprintf( errorStr, 256, "%s -1: %s", JTranslate( "Error" ), JTranslate( "Unknown error message" ));
		return errorStr;
	}

	errorCode = -1;
	if (( str=JabberXmlGetAttrValue( errorNode, "code" )) != NULL )
		errorCode = atoi( str );
	if (( str=errorNode->text ) != NULL )
		mir_snprintf( errorStr, 256, "%s %d: %s\r\n%s", JTranslate( "Error" ), errorCode, JTranslate( JabberErrorStr( errorCode )), str );
	else
		mir_snprintf( errorStr, 256, "%s %d: %s", JTranslate( "Error" ), errorCode, JTranslate( JabberErrorStr( errorCode )) );
	return errorStr;
}

void __stdcall JabberSendVisibleInvisiblePresence( BOOL invisible )
{
	if ( !jabberOnline ) return;

	for ( int i = 0; ( i=JabberListFindNext( LIST_ROSTER, i )) >= 0; i++ ) {
		JABBER_LIST_ITEM *item = JabberListGetItemPtrFromIndex( i );
		if ( item == NULL )
			continue;

		HANDLE hContact = JabberHContactFromJID( item->jid );
		if ( hContact == NULL )
			continue;

		WORD apparentMode = JGetWord( hContact, "ApparentMode", 0 );
		if ( invisible==TRUE && apparentMode==ID_STATUS_OFFLINE )
			JabberSend( jabberThreadInfo->s, "<presence to='%s' type='invisible'/>", item->jid );
		else if ( invisible==FALSE && apparentMode==ID_STATUS_ONLINE )
			JabberSendPresenceTo( jabberStatus, item->jid, NULL );
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberTextEncodeW - prepare a string for transmission

char* __stdcall JabberTextEncode( const char* str )
{
	if ( str == NULL )
		return NULL;

	char* s1 = JabberUrlEncode( str );
	if ( s1 == NULL )
		return NULL;

	// Convert invalid control characters to space
	if ( *s1 ) {
		char* p, *q;

		for ( p = s1; *p != '\0'; p++ )
			if ( *p > 0 && *p < 0x20 && *p != 0x09 && *p != 0x0a && *p != 0x0d )
				*p = ( char )0x20;

		for ( p = q = s1; *p!='\0'; p++ ) {
			if ( *p != '\r' ) {
				*q = *p;
				q++;
		}	}

		*q = '\0';
	}

	char* s2 = JabberUtf8Encode( s1 );
	free( s1 );
	return s2;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberTextEncodeW - prepare a string for transmission

char* __stdcall JabberTextEncodeW( const wchar_t* str )
{
	if ( str == NULL )
		return NULL;

	const wchar_t *s;
	int resLen = 1;

	for ( s = str; *s; s++ ) {
		switch( *s ) {
			case '\r':	continue;
			case '&':   resLen += 5;	break;
			case '\'':  resLen += 6;	break;
			case '>':
			case '<':	resLen += 6;	break;
			case '\"':  resLen += 6;	break;
			default:		resLen++;		break;
	}	}

	wchar_t* tmp = ( wchar_t* )alloca( resLen * sizeof( wchar_t )), *d;

	for ( s = str, d = tmp; *s; s++ ) {
		switch( *s ) {
		case '\r':	continue;
		case '&':   wcscpy( d, L"&amp;" );		d += 5;	break;
		case '\'':  wcscpy( d, L"&apos;" );	d += 6;	break;
		case '>':   wcscpy( d, L"&gt;" );		d += 4;	break;
		case '<':	wcscpy( d, L"&lt;" );		d += 4;	break;
		case '\"':  wcscpy( d, L"&quot;" );	d += 6;	break;
		default:
			if ( *s > 0 && *s < 0x20 && *s != 0x09 && *s != 0x0a && *s != 0x0d )
				*d++ = ' ';
			else
				*d++ = *s;
	}	}

	*d = 0;

	return JabberUtf8EncodeW( tmp );
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberTextDecode - retrieve a text from the encoded string

char* __stdcall JabberTextDecode( const char* str )
{
	if ( str == NULL )
		return NULL;

	char* s1 = ( char* )alloca( strlen( str )+1 ), *s2;
	strcpy( s1, str );

	JabberUtf8Decode( s1, NULL );
	JabberUrlDecode( s1 );
	if (( s2 = JabberUnixToDos( s1 )) == NULL )
		return NULL;

	return s2;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberBase64Encode

static char b64table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* __stdcall JabberBase64Encode( const char* buffer, int bufferLen )
{
	if ( buffer==NULL || bufferLen<=0 )
		return NULL;

	char* res = (char*)malloc(((( bufferLen+2 )*4 )/3 ) + 1);
	if ( res == NULL )
		return NULL;

	unsigned char igroup[3];
	int nGroups = 0;
	char *r = res;
	const char* peob = buffer + bufferLen;
	for ( const char* p = buffer; p < peob; ) {
		igroup[ 0 ] = igroup[ 1 ] = igroup[ 2 ] = 0;
		int n;
		for ( n=0; n<3; n++ ) {
			if ( p >= peob ) break;
			igroup[n] = ( unsigned char ) *p;
			p++;
		}

		if ( n > 0 ) {
			r[0] = b64table[ igroup[0]>>2 ];
			r[1] = b64table[ (( igroup[0]&3 )<<4 ) | ( igroup[1]>>4 ) ];
			r[2] = b64table[ (( igroup[1]&0xf )<<2 ) | ( igroup[2]>>6 ) ];
			r[3] = b64table[ igroup[2]&0x3f ];

			if ( n < 3 ) {
				r[3] = '=';
				if ( n < 2 )
					r[2] = '=';
			}
			r += 4;
	}	}

	*r = '\0';

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberBase64Decode

static unsigned char b64rtable[256];

char* __stdcall JabberBase64Decode( const char* str, int *resultLen )
{
	char* res;
	unsigned char* p, *r, igroup[4], a[4];
	int n, num, count;

	if ( str==NULL || resultLen==NULL ) return NULL;
	if (( res=( char* )malloc(( ( strlen( str )+3 )/4 )*3 )) == NULL ) return NULL;

	for ( n=0; n<256; n++ )
		b64rtable[n] = ( unsigned char ) 0x80;
	for ( n=0; n<26; n++ )
		b64rtable['A'+n] = n;
	for ( n=0; n<26; n++ )
		b64rtable['a'+n] = n + 26;
	for ( n=0; n<10; n++ )
		b64rtable['0'+n] = n + 52;
	b64rtable['+'] = 62;
	b64rtable['/'] = 63;
	b64rtable['='] = 0;
	count = 0;
	for ( p=( unsigned char* )str,r=( unsigned char* )res; *p!='\0'; ) {
		for ( n=0; n<4; n++ ) {
			if ( *p == '\r' || *p == '\n' ) {
				n--; p++;
				continue;
			}

			if ( *p=='\0' ) {
				if ( n == 0 )
					goto LBL_Exit;
				free( res );
				return NULL;
			}

			if ( b64rtable[*p]==0x80 ) {
				free( res );
				return NULL;
			}

			a[n] = *p;
			igroup[n] = b64rtable[*p];
			p++;
		}
		r[0] = igroup[0]<<2 | igroup[1]>>4;
		r[1] = igroup[1]<<4 | igroup[2]>>2;
		r[2] = igroup[2]<<6 | igroup[3];
		r += 3;
		num = ( a[2]=='='?1:( a[3]=='='?2:3 ));
		count += num;
		if ( num < 3 ) break;
	}
LBL_Exit:
	*resultLen = count;
	return res;
}

char* __stdcall JabberGetVersionText()
{
	char filename[MAX_PATH], *fileVersion, *res;
	DWORD unused;
	DWORD verInfoSize;
	UINT  blockSize;
	PVOID pVerInfo;

	GetModuleFileNameA( hInst, filename, sizeof( filename ));
	verInfoSize = GetFileVersionInfoSizeA( filename, &unused );
	if (( pVerInfo=malloc( verInfoSize )) != NULL ) {
		GetFileVersionInfoA( filename, 0, verInfoSize, pVerInfo );
		VerQueryValueA( pVerInfo, "\\StringFileInfo\\040904b0\\FileVersion", ( LPVOID* )&fileVersion, &blockSize );
		if ( strstr( fileVersion, "cvs" )) {
			res = ( char* )malloc( strlen( fileVersion ) + strlen( __DATE__ ) + 2 );
			sprintf( res, "%s %s", fileVersion, __DATE__ );
		}
		else {
			res = _strdup( fileVersion );
		}
		free( pVerInfo );
		return res;
	}
	return NULL;
}

time_t __stdcall JabberIsoToUnixTime( char* stamp )
{
	struct tm timestamp;
	char date[9];
	char* p;
	int i, y;
	time_t t;

	if ( stamp == NULL ) return ( time_t ) 0;

	p = stamp;

	// Get the date part
	for ( i=0; *p!='\0' && i<8 && isdigit( *p ); p++,i++ )
		date[i] = *p;

	// Parse year
	if ( i == 6 ) {
		// 2-digit year ( 1970-2069 )
		y = ( date[0]-'0' )*10 + ( date[1]-'0' );
		if ( y < 70 ) y += 100;
	}
	else if ( i == 8 ) {
		// 4-digit year
		y = ( date[0]-'0' )*1000 + ( date[1]-'0' )*100 + ( date[2]-'0' )*10 + date[3]-'0';
		y -= 1900;
	}
	else
		return ( time_t ) 0;
	timestamp.tm_year = y;
	// Parse month
	timestamp.tm_mon = ( date[i-4]-'0' )*10 + date[i-3]-'0' - 1;
	// Parse date
	timestamp.tm_mday = ( date[i-2]-'0' )*10 + date[i-1]-'0';

	// Skip any date/time delimiter
	for ( ; *p!='\0' && !isdigit( *p ); p++ );

	// Parse time
	if ( sscanf( p, "%d:%d:%d", &( timestamp.tm_hour ), &( timestamp.tm_min ), &( timestamp.tm_sec )) != 3 )
		return ( time_t ) 0;

	timestamp.tm_isdst = 0;	// DST is already present in _timezone below
	t = mktime( &timestamp );

	_tzset();
	t -= _timezone;

	if ( t >= 0 )
		return t;
	else
		return ( time_t ) 0;
}

int __stdcall JabberCountryNameToId( char* ctry )
{
	int ctryCount, i;
	struct CountryListEntry *ctryList;
	static struct CountryListEntry extraCtry[] = {
		{ 1,		"United States" },
		{ 1,		"United States of America" },
		{ 1,		"US" },
		{ 44,		"England" }
	};

	// Check for some common strings not present in the country list
	ctryCount = sizeof( extraCtry )/sizeof( extraCtry[0] );
	for ( i=0; i<ctryCount && stricmp( extraCtry[i].szName, ctry ); i++ );
	if ( i < ctryCount )
		return extraCtry[i].id;

	// Check Miranda country list
	JCallService( MS_UTILS_GETCOUNTRYLIST, ( WPARAM ) &ctryCount, ( LPARAM )&ctryList );
	for ( i=0; i<ctryCount && stricmp( ctryList[i].szName, ctry ); i++ );
	if ( i < ctryCount )
		return ctryList[i].id;
	else
		return 0xffff; // Unknown
}

void __stdcall JabberSendPresenceTo( int status, char* to, char* extra )
{
	if ( !jabberOnline ) return;

	// Send <presence/> update for status ( we won't handle ID_STATUS_OFFLINE here )
	// Note: jabberModeMsg is already encoded using JabberTextEncode()
	EnterCriticalSection( &modeMsgMutex );

	char priorityStr[ 200 ];
	int bytes = mir_snprintf( priorityStr, sizeof priorityStr, "<priority>%d</priority>", JGetWord( NULL, "Priority", 0 ));

	if ( JGetByte( "EnableAvatars", TRUE ))
	{
		char hashValue[ 50 ];
		if ( !JGetStaticString( "AvatarHash", NULL, hashValue, sizeof hashValue ))
			mir_snprintf( priorityStr+bytes, sizeof(priorityStr)-bytes, "<x xmlns='jabber:x:avatar'><hash>%s</hash></x>", hashValue );
	}

	char toStr[ 512 ];
	if ( to != NULL )
		mir_snprintf( toStr, sizeof toStr, " to='%s'", to );
	else
		toStr[0] = '\0';

	if ( extra == NULL )
		extra = "";

	switch ( status ) {
	case ID_STATUS_ONLINE:
		if ( modeMsgs.szOnline )
			JabberSend( jabberThreadInfo->s, "<presence%s><status>%s</status>%s%s</presence>", toStr, modeMsgs.szOnline, priorityStr, extra );
		else
			JabberSend( jabberThreadInfo->s, "<presence%s>%s%s</presence>", toStr, priorityStr, extra );
		break;
	case ID_STATUS_INVISIBLE:
		JabberSend( jabberThreadInfo->s, "<presence type='invisible'%s>%s%s</presence>", toStr, priorityStr, extra );
		break;
	case ID_STATUS_AWAY:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		if ( modeMsgs.szAway )
			JabberSend( jabberThreadInfo->s, "<presence%s><show>away</show><status>%s</status>%s%s</presence>", toStr, modeMsgs.szAway, priorityStr, extra );
		else
			JabberSend( jabberThreadInfo->s, "<presence%s><show>away</show>%s%s</presence>", toStr, priorityStr, extra );
		break;
	case ID_STATUS_NA:
		if ( modeMsgs.szNa )
			JabberSend( jabberThreadInfo->s, "<presence%s><show>xa</show><status>%s</status>%s%s</presence>", toStr, modeMsgs.szNa, priorityStr, extra );
		else
			JabberSend( jabberThreadInfo->s, "<presence%s><show>xa</show>%s%s</presence>", toStr, priorityStr, extra );
		break;
	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		if ( modeMsgs.szDnd )
			JabberSend( jabberThreadInfo->s, "<presence%s><show>dnd</show><status>%s</status>%s%s</presence>", toStr, modeMsgs.szDnd, priorityStr, extra );
		else
			JabberSend( jabberThreadInfo->s, "<presence%s><show>dnd</show>%s%s</presence>", toStr, priorityStr, extra );
		break;
	case ID_STATUS_FREECHAT:
		if ( modeMsgs.szFreechat )
			JabberSend( jabberThreadInfo->s, "<presence%s><show>chat</show><status>%s</status>%s%s</presence>", toStr, modeMsgs.szFreechat, priorityStr, extra );
		else
			JabberSend( jabberThreadInfo->s, "<presence%s><show>chat</show>%s%s</presence>", toStr, priorityStr, extra );
		break;
	default:
		// Should not reach here
		break;
	}
	LeaveCriticalSection( &modeMsgMutex );
}

void __stdcall JabberSendPresence( int status )
{
	JabberSendPresenceTo( status, NULL, NULL );
	JabberSendVisibleInvisiblePresence( status == ID_STATUS_INVISIBLE );

	// Also update status in all chatrooms
	for ( int i = 0; ( i=JabberListFindNext( LIST_CHATROOM, i )) >= 0; i++ ) {
		JABBER_LIST_ITEM *item = JabberListGetItemPtrFromIndex( i );
		if ( item != NULL )
			JabberSendPresenceTo( status, item->jid, NULL );
}	}

void __stdcall JabberStringAppend( char* *str, int *sizeAlloced, const char* fmt, ... )
{
	va_list vararg;
	char* p;
	int size, len;

	if ( str == NULL ) return;

	if ( *str==NULL || *sizeAlloced<=0 ) {
		*sizeAlloced = size = 2048;
		*str = ( char* )malloc( size );
		len = 0;
	}
	else {
		len = strlen( *str );
		size = *sizeAlloced - strlen( *str );
	}

	p = *str + len;
	va_start( vararg, fmt );
	while ( _vsnprintf( p, size, fmt, vararg ) == -1 ) {
		size += 2048;
		( *sizeAlloced ) += 2048;
		*str = ( char* )realloc( *str, *sizeAlloced );
		p = *str + len;
	}
	va_end( vararg );
}

///////////////////////////////////////////////////////////////////////////////
// JabberGetClientJID - adds a resource postfix to a JID

char* __stdcall JabberGetClientJID( const char* jid, char* dest, size_t destLen )
{
	if ( jid == NULL )
		return NULL;

	size_t len = strlen( jid );
	if ( len >= destLen )
		len = destLen-1;

	memcpy( dest, jid, len );
	dest[ len ] = '\0';

	char* p = strchr( dest, '/' );
	if ( p == NULL ) {
		char* resource = JabberListGetBestResourceNamePtr( jid );
		if ( resource != NULL )
			mir_snprintf( dest+len, destLen-len-1, "/%s", resource );
	}

	return dest;
}

///////////////////////////////////////////////////////////////////////////////
// JabberStripJid - strips a resource postfix from a JID

char* __stdcall JabberStripJid( const char* jid, char* dest, size_t destLen )
{
	if ( jid == NULL )
		*dest = 0;
	else {
		size_t len = strlen( jid );
		if ( len >= destLen )
			len = destLen-1;

		memcpy( dest, jid, len );
		dest[ len ] = 0;

		char* p = strchr( dest, '/' );
		if ( p != NULL )
			*p = 0;
	}

	return dest;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetPictureType - tries to autodetect the picture type from the buffer

int __stdcall JabberGetPictureType( const char* buf )
{
	if ( buf != NULL ) {
		if ( memcmp( buf, "GIF89", 5 ) == 0 )   return PA_FORMAT_GIF;
		if ( memcmp( buf, "\x89PNG", 4 ) == 0 ) return PA_FORMAT_PNG;
		if ( memcmp( buf, "BM", 2 ) == 0 )      return PA_FORMAT_BMP;
		if ( memcmp( buf+6, "JFIF", 4 ) == 0 )  return PA_FORMAT_JPEG;
	}

	return PA_FORMAT_UNKNOWN;
}
