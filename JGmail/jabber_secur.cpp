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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_proxy.cpp,v $
Revision       : $Revision: 2866 $
Last change on : $Date: 2006-05-16 20:39:40 +0400 (Вт, 16 май 2006) $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_secur.h"
#include "md5.h"

/////////////////////////////////////////////////////////////////////////////////////////
// ntlm auth - LanServer based authorization

TNtlmAuth::TNtlmAuth( ThreadData* info ) :
	TJabberAuth( info )
{
	szName = "NTLM";
	switch ((int)(hProvider = Netlib_InitSecurityProvider( "NTLM" ))){
	case CALLSERVICE_NOTFOUND: hProvider = NULL;
	case NULL: bIsValid = false;
	}
}

TNtlmAuth::~TNtlmAuth()
{
	if ( hProvider != NULL )
		Netlib_DestroySecurityProvider( "NTLM", hProvider );
}

char* TNtlmAuth::getInitialRequest()
{
	if ( !hProvider )
		return NULL;

	return Netlib_NtlmCreateResponse( hProvider, "" );
}

char* TNtlmAuth::getChallenge( const TCHAR* challenge )
{
	if ( !hProvider )
		return NULL;

	char* text = t2a( challenge );
	char* result = Netlib_NtlmCreateResponse( hProvider, text );
	mir_free( text );
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// md5 auth - digest-based authorization

TMD5Auth::TMD5Auth( ThreadData* info ) :
	TJabberAuth( info ),
	iCallCount( 0 )
{
	szName = "DIGEST-MD5";
}

TMD5Auth::~TMD5Auth()
{
}

char* TMD5Auth::getChallenge( const TCHAR* challenge )
{
	if ( iCallCount > 0 )
		return NULL;

	iCallCount++;

	int resultLen;
	char* text = JabberBase64Decode( challenge, &resultLen );
	JabberLog( "MD5 challenge = <%s>", text );

	TStringPairs pairs( text );
	const char *realm = pairs["realm"], *nonce = pairs["nonce"];

	char randomNumber[40], cnonce[40], tmpBuf[40];
	srand( time(0));
	itoa( rand(), randomNumber, 10 );

	DWORD digest[4], hash1[4], hash2[4];
	MD5_CTX ctx;
	MD5Init( &ctx );
	MD5Update( &ctx, (BYTE*)randomNumber, strlen(randomNumber));
	MD5Final(( BYTE* )digest, &ctx );
	sprintf( cnonce, "%08x%08x%08x%08x", htonl(digest[0]), htonl(digest[1]), htonl(digest[2]), htonl(digest[3]));

	char *uname = mir_utf8encodeT( info->username ), 
		  *passw = mir_utf8encode( info->password ), 
		  *serv  = mir_utf8encode( info->server );

	MD5Init( &ctx );
	MD5Update( &ctx, ( BYTE* )uname,  strlen( uname ));
	MD5Update( &ctx, ( BYTE* )":",    1 );
	MD5Update( &ctx, ( BYTE* )realm,  strlen( realm ));
	MD5Update( &ctx, ( BYTE* )":",    1 );
	MD5Update( &ctx, ( BYTE* )passw,  strlen( passw ));
	MD5Final(( BYTE* )hash1, &ctx );

	MD5Init( &ctx );
	MD5Update( &ctx, ( BYTE* )hash1,  16 );
	MD5Update( &ctx, ( BYTE* )":",    1 );
	MD5Update( &ctx, ( BYTE* )nonce,  strlen( nonce ));
	MD5Update( &ctx, ( BYTE* )":",    1 );
	MD5Update( &ctx, ( BYTE* )cnonce, strlen( cnonce ));
	MD5Final(( BYTE* )hash1, &ctx );
	
	MD5Init( &ctx );
	MD5Update( &ctx, ( BYTE* )"AUTHENTICATE:xmpp/", 18 );
	MD5Update( &ctx, ( BYTE* )serv,   strlen( serv ));
	MD5Final(( BYTE* )hash2, &ctx );

	MD5Init( &ctx );
	sprintf( tmpBuf, "%08x%08x%08x%08x", htonl(hash1[0]), htonl(hash1[1]), htonl(hash1[2]), htonl(hash1[3]));
	MD5Update( &ctx, ( BYTE* )tmpBuf, strlen( tmpBuf ));
	MD5Update( &ctx, ( BYTE* )":",    1 );
	MD5Update( &ctx, ( BYTE* )nonce,  strlen( nonce ));
	sprintf( tmpBuf, ":%08d:", iCallCount );
	MD5Update( &ctx, ( BYTE* )tmpBuf, strlen( tmpBuf ));
	MD5Update( &ctx, ( BYTE* )cnonce, strlen( cnonce ));
	MD5Update( &ctx, ( BYTE* )":auth:", 6 );
	sprintf( tmpBuf, "%08x%08x%08x%08x", htonl(hash2[0]), htonl(hash2[1]), htonl(hash2[2]), htonl(hash2[3]));
	MD5Update( &ctx, ( BYTE* )tmpBuf, strlen( tmpBuf ));
	MD5Final(( BYTE* )digest, &ctx );

	char* buf = (char*)alloca(8000);
	int cbLen = mir_snprintf( buf, 8000, 
		"username=\"%s\",realm=\"%s\",nonce=\"%s\",cnonce=\"%s\",nc=%08d,"
		"qop=auth,digest-uri=\"xmpp/%s\",charset=utf-8,response=%08x%08x%08x%08x",
		uname, realm, nonce, cnonce, iCallCount, serv,
		htonl(digest[0]), htonl(digest[1]), htonl(digest[2]), htonl(digest[3]));

	mir_free( uname );
	mir_free( passw );
	mir_free( serv );

   return JabberBase64Encode( buf, cbLen );
}

/////////////////////////////////////////////////////////////////////////////////////////
// plain auth - the most simple one

TPlainAuth::TPlainAuth( ThreadData* info ) :
	TJabberAuth( info )
{
	szName = "PLAIN";
}

TPlainAuth::~TPlainAuth()
{
}

char* TPlainAuth::getInitialRequest()
{
	char *temp = t2a(info->username);
	int size = strlen(temp)*2+strlen(info->server)+strlen(info->password)+3;
	char *toEncode = ( char* )alloca( size+1 );
	mir_snprintf( toEncode, size+1, "%s@%s%c%s%c%s", temp, info->server, 0, temp, 0, info->password );
	char* result = JabberBase64Encode( toEncode, size );
	mir_free(temp);
	JabberLog( "Never publish the hash below" );
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// basic type

TJabberAuth::TJabberAuth( ThreadData* pInfo ) :
	bIsValid( true ),
	szName( NULL ),
	info( pInfo )
{
}

TJabberAuth::~TJabberAuth()
{
}

char* TJabberAuth::getInitialRequest()
{
	return NULL;
}

char* TJabberAuth::getChallenge( const TCHAR* challenge )
{
	return NULL;
}
