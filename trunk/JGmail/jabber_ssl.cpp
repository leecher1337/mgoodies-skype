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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_ssl.cpp,v $
Revision       : $Revision: 1.7 $
Last change on : $Date: 2006/05/12 20:13:35 $
Last change by : $Author: ghazan $

*/

#define _JABBER_SSL_C_

#include "jabber.h"
#include "jabber_ssl.h"

PFN_SSL_int_void			pfn_SSL_library_init;		// int SSL_library_init()
PFN_SSL_pvoid_void			pfn_SSLv23_client_method;	// SSL_METHOD *SSLv23_client_method()
PFN_SSL_pvoid_pvoid			pfn_SSL_CTX_new;			// SSL_CTX *SSL_CTX_new( SSL_METHOD *method )
PFN_SSL_void_pvoid			pfn_SSL_CTX_free;			// void SSL_CTX_free( SSL_CTX *ctx );
PFN_SSL_pvoid_pvoid			pfn_SSL_new;				// SSL *SSL_new( SSL_CTX *ctx )
PFN_SSL_void_pvoid			pfn_SSL_free;				// void SSL_free( SSL *ssl );
PFN_SSL_int_pvoid_int		pfn_SSL_set_fd;				// int SSL_set_fd( SSL *ssl, int fd );
PFN_SSL_int_pvoid			pfn_SSL_connect;			// int SSL_connect( SSL *ssl );
PFN_SSL_int_pvoid_pvoid_int	pfn_SSL_read;				// int SSL_read( SSL *ssl, void *buffer, int bufsize )
PFN_SSL_int_pvoid_pvoid_int	pfn_SSL_write;				// int SSL_write( SSL *ssl, void *buffer, int bufsize )

static CRITICAL_SECTION sslHandleMutex;
static JABBER_SSL_MAPPING *sslHandleList = NULL;
static int sslHandleCount = 0;

#ifndef STATICSSL
BOOL JabberSslInit()
{
	BOOL error = FALSE;

	sslHandleList = NULL;
	sslHandleCount = 0;
	InitializeCriticalSection( &sslHandleMutex );

	hLibSSL = LoadLibraryA( "SSLEAY32.DLL" );

	if ( !hLibSSL )
		hLibSSL = LoadLibraryA( "LIBSSL32.DLL" );

	if ( hLibSSL ) {
		if (( pfn_SSL_library_init=( PFN_SSL_int_void )GetProcAddress( hLibSSL, "SSL_library_init" )) == NULL )
			error = TRUE;
		if (( pfn_SSLv23_client_method=( PFN_SSL_pvoid_void )GetProcAddress( hLibSSL, "SSLv23_client_method" )) == NULL )
			error = TRUE;
		if (( pfn_SSL_CTX_new=( PFN_SSL_pvoid_pvoid )GetProcAddress( hLibSSL, "SSL_CTX_new" )) == NULL )
			error = TRUE;
		if (( pfn_SSL_CTX_free=( PFN_SSL_void_pvoid )GetProcAddress( hLibSSL, "SSL_CTX_free" )) == NULL )
			error = TRUE;
		if (( pfn_SSL_new=( PFN_SSL_pvoid_pvoid )GetProcAddress( hLibSSL, "SSL_new" )) == NULL )
			error = TRUE;
		if (( pfn_SSL_free=( PFN_SSL_void_pvoid )GetProcAddress( hLibSSL, "SSL_free" )) == NULL )
			error = TRUE;
		if (( pfn_SSL_set_fd=( PFN_SSL_int_pvoid_int )GetProcAddress( hLibSSL, "SSL_set_fd" )) == NULL )
			error = TRUE;
		if (( pfn_SSL_connect=( PFN_SSL_int_pvoid )GetProcAddress( hLibSSL, "SSL_connect" )) == NULL )
			error = TRUE;
		if (( pfn_SSL_read=( PFN_SSL_int_pvoid_pvoid_int )GetProcAddress( hLibSSL, "SSL_read" )) == NULL )
			error = TRUE;
		if (( pfn_SSL_write=( PFN_SSL_int_pvoid_pvoid_int )GetProcAddress( hLibSSL, "SSL_write" )) == NULL )
			error = TRUE;

		if ( error == TRUE ) {
			FreeLibrary( hLibSSL );
			hLibSSL = NULL;
		}
	}


#ifdef _DEBUG
	if ( hLibSSL )
		JabberLog( "SSL library load successful" );
	else
		JabberLog( "SSL library cannot load" );
#endif

	if ( hLibSSL ) {
		pfn_SSL_library_init();
		jabberSslCtx = pfn_SSL_CTX_new( pfn_SSLv23_client_method());

		return TRUE;
	}
	else
		return FALSE;
}

#else // ndef STATICSSL
BOOL JabberSslInit()
{
	BOOL error = FALSE;

	sslHandleList = NULL;
	sslHandleCount = 0;
	InitializeCriticalSection( &sslHandleMutex );

		pfn_SSL_library_init=SSL_library_init;
		pfn_SSLv23_client_method=( PFN_SSL_pvoid_void )SSLv23_client_method;
		pfn_SSL_CTX_new=( PFN_SSL_pvoid_pvoid )SSL_CTX_new;
		pfn_SSL_CTX_free=( PFN_SSL_void_pvoid )SSL_CTX_free;
		pfn_SSL_new=( PFN_SSL_pvoid_pvoid )SSL_new;
		pfn_SSL_free=( PFN_SSL_void_pvoid )SSL_free;
		pfn_SSL_set_fd=( PFN_SSL_int_pvoid_int )SSL_set_fd;
		pfn_SSL_connect=( PFN_SSL_int_pvoid )SSL_connect;
		pfn_SSL_read=( PFN_SSL_int_pvoid_pvoid_int )SSL_read;
		pfn_SSL_write=( PFN_SSL_int_pvoid_pvoid_int )SSL_write;

		pfn_SSL_library_init();
		jabberSslCtx = pfn_SSL_CTX_new( pfn_SSLv23_client_method());

		return TRUE;
}
#endif // ndef STATICSSL

void JabberSslUninit()
{
#ifndef STATICSSL
	if ( hLibSSL ) {
#endif // ndef STATICSSL
		pfn_SSL_CTX_free( jabberSslCtx );
#ifndef STATICSSL
		JabberLog( "Free SSL library" );
		FreeLibrary( hLibSSL );
		hLibSSL = NULL;
	}
#endif // ndef STATICSSL
	if ( sslHandleList ) mir_free( sslHandleList );
	sslHandleCount = 0;
	DeleteCriticalSection( &sslHandleMutex );
}

int JabberSslFindHandle( HANDLE hConn )
{
	int i;

	EnterCriticalSection( &sslHandleMutex );
	for ( i=0; i<sslHandleCount; i++ ) {
		if ( sslHandleList[i].h == hConn ) {
			LeaveCriticalSection( &sslHandleMutex );
			return i;
		}
	}
	LeaveCriticalSection( &sslHandleMutex );
	return -1;
}

PVOID JabberSslHandleToSsl( HANDLE hConn )
{
	int i;

	EnterCriticalSection( &sslHandleMutex );
	for ( i=0; i<sslHandleCount; i++ ) {
		if ( sslHandleList[i].h == hConn ) {
			LeaveCriticalSection( &sslHandleMutex );
			return sslHandleList[i].ssl;
		}
	}
	LeaveCriticalSection( &sslHandleMutex );
	return NULL;
}

void JabberSslAddHandle( HANDLE hConn, PVOID ssl )
{
	EnterCriticalSection( &sslHandleMutex );
	if ( JabberSslFindHandle( hConn ) >= 0 ) {
		LeaveCriticalSection( &sslHandleMutex );
		return;
	}

	sslHandleList = ( JABBER_SSL_MAPPING * ) mir_realloc( sslHandleList, ( sslHandleCount+1 )*sizeof( JABBER_SSL_MAPPING ));
	sslHandleList[sslHandleCount].h = hConn;
	sslHandleList[sslHandleCount].ssl = ssl;
	sslHandleCount++;
	LeaveCriticalSection( &sslHandleMutex );
}

void JabberSslRemoveHandle( HANDLE hConn )
{
	int i;

	EnterCriticalSection( &sslHandleMutex );
	if (( i=JabberSslFindHandle( hConn )) < 0 ) {
		LeaveCriticalSection( &sslHandleMutex );
		return;
	}

	sslHandleCount--;
	memmove( sslHandleList+i, sslHandleList+i+1, ( sslHandleCount-i )*sizeof( JABBER_SSL_MAPPING ));
	sslHandleList = ( JABBER_SSL_MAPPING * ) mir_realloc( sslHandleList, sslHandleCount*sizeof( JABBER_SSL_MAPPING ));
	LeaveCriticalSection( &sslHandleMutex );
}
