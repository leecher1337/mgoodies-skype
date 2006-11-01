/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
Copyright ( c ) 2003-5 George Hazan.
Copyright ( c ) 2002-3 Richard Hughes ( original version ).

Miranda IM: the free icq client for MS Windows
Copyright ( C ) 2000-2002 Richard Hughes, Roland Rabien & Tristan Van de Vreede

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
#ifdef __GNUC__
	#define __try
	#define __except(x) if (0) /* don't execute handler */
	#define __finally
	#define _try __try
	#define _except __except
	#define _finally __finally
#endif

extern UINT jabberCodePage;

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


char* deprecatedUtf8Decode( char* str, WCHAR** ucs2 )
{
	if ( str == NULL )
		return NULL;

	int len = strlen( str );
	if ( len < 2 ) {
		if ( ucs2 != NULL ) {
			*ucs2 = ( wchar_t* )mir_alloc(( len+1 )*sizeof( wchar_t ));
			MultiByteToWideChar( CP_ACP, 0, str, len, *ucs2, len );
			( *ucs2 )[ len ] = 0;
		}
		return str;
	}

	wchar_t* tempBuf = ( wchar_t* )alloca(( len+1 )*sizeof( wchar_t ));
	sttUtf8Decode(( BYTE* )str, tempBuf );

	if ( ucs2 != NULL ) {
		int fullLen = ( len+1 )*sizeof( wchar_t );
		*ucs2 = ( wchar_t* )mir_alloc( fullLen );
		memcpy( *ucs2, tempBuf, fullLen );
	}

   WideCharToMultiByte( CP_ACP, 0, tempBuf, -1, str, len, NULL, NULL );
	return str;
}

char* deprecatedUtf8EncodeW( const WCHAR* wstr )
{
	const WCHAR* w;

	// Convert unicode to utf8
	int len = 0;
	for ( w = wstr; *w; w++ ) {
		if ( *w < 0x0080 ) len++;
		else if ( *w < 0x0800 ) len += 2;
		else len += 3;
	}

	unsigned char* szOut = ( unsigned char* )mir_alloc( len+1 );
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

char* deprecatedUtf8Encode( const char* str )
{
	if ( str == NULL )
		return NULL;

	// Convert local codepage to unicode
	int len = strlen( str );
	WCHAR* wszTemp = ( WCHAR* )alloca( sizeof( WCHAR )*( len+1 ));
	MultiByteToWideChar( jabberCodePage, 0, str, -1, wszTemp, len+1 );

	return deprecatedUtf8EncodeW( wszTemp );
}

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

BOOL hasForkThreadService =0;
ULONG deprecatedForkThread( void ( __cdecl *threadcode )( void* ), unsigned long stacksize, void *arg )
{
	struct FORK_ARG fa;
	fa.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	fa.threadcode = threadcode;
	fa.arg = arg;

	ULONG rc = _beginthread(( pThreadFunc )forkthread_r, 0, &fa );
	if (( unsigned long ) -1L != rc )
		WaitForSingleObject( fa.hEvent, INFINITE );

	CloseHandle( fa.hEvent );
	return rc;
}
