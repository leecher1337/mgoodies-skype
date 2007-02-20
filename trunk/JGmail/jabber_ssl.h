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

File name      : $URL$
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

*/

#ifndef _JABBER_SSL_H_
#define _JABBER_SSL_H_

#ifdef STATICSSL
#include <openssl\ssl.h>
#endif


typedef int ( *PFN_SSL_int_void ) ( void );
typedef PVOID ( *PFN_SSL_pvoid_void ) ( void );
typedef PVOID ( *PFN_SSL_pvoid_pvoid ) ( PVOID );
typedef void ( *PFN_SSL_void_pvoid ) ( PVOID );
typedef int ( *PFN_SSL_int_pvoid_int ) ( PVOID, int );
typedef int ( *PFN_SSL_int_pvoid ) ( PVOID );
typedef int ( *PFN_SSL_int_pvoid_pvoid_int ) ( PVOID, PVOID, int );

#ifndef _JABBER_SSL_C_
extern PFN_SSL_int_void			pfn_SSL_library_init;		// int SSL_library_init()
extern PFN_SSL_pvoid_void			pfn_SSLv23_client_method;	// SSL_METHOD *SSLv23_client_method()
extern PFN_SSL_pvoid_pvoid			pfn_SSL_CTX_new;			// SSL_CTX *SSL_CTX_new( SSL_METHOD *method )
extern PFN_SSL_void_pvoid			pfn_SSL_CTX_free;			// void SSL_CTX_free( SSL_CTX *ctx );
extern PFN_SSL_pvoid_pvoid			pfn_SSL_new;				// SSL *SSL_new( SSL_CTX *ctx )
extern PFN_SSL_void_pvoid			pfn_SSL_free;				// void SSL_free( SSL *ssl );
extern PFN_SSL_int_pvoid_int		pfn_SSL_set_fd;				// int SSL_set_fd( SSL *ssl, int fd );
extern PFN_SSL_int_pvoid			pfn_SSL_connect;			// int SSL_connect( SSL *ssl );
extern PFN_SSL_int_pvoid_pvoid_int	pfn_SSL_read;				// int SSL_read( SSL *ssl, void *buffer, int bufsize )
extern PFN_SSL_int_pvoid_pvoid_int	pfn_SSL_write;				// int SSL_write( SSL *ssl, void *buffer, int bufsize )
#endif

BOOL JabberSslInit();
void JabberSslUninit();
char * getXGoogleToken(char * email, char * passwd);

#endif
