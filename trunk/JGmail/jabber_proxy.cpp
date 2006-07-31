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
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

*/

#include "jabber.h"

int JabberHttpGatewayInit( HANDLE hConn, NETLIBOPENCONNECTION *nloc, NETLIBHTTPREQUEST *nlhr )
{
#ifdef NNNN
	WORD wLen, wVersion, wType;
	WORD wIpLen;
	DWORD dwSid1, dwSid2, dwSid3, dwSid4;
	BYTE response[300], *buf;
	int responseBytes, recvResult;
	char szSid[33], szHttpServer[256], szHttpGetUrl[300], szHttpPostUrl[300];
	NETLIBHTTPPROXYINFO nlhpi = {0};

	for( responseBytes = 0; ; ) {
		recvResult = Netlib_Recv( hConn, response + responseBytes, sizeof( response ) - responseBytes, MSG_DUMPPROXY );
		if( recvResult<=0 ) break;
		responseBytes += recvResult;
		if( responseBytes == sizeof( response ))
			break;
	}
	if( responseBytes < 31 )
	{
		SetLastError( ERROR_INVALID_DATA );
		return 0;
	}
	buf = response;
	unpackWord( &buf, &wLen );
	unpackWord( &buf, &wVersion );	  /* always 0x0443 */
	unpackWord( &buf, &wType );
	buf += 6;  /* dunno */
	unpackDWord( &buf, &dwSid1 );
	unpackDWord( &buf, &dwSid2 );
	unpackDWord( &buf, &dwSid3 );
	unpackDWord( &buf, &dwSid4 );
	sprintf( szSid, "%08x%08x%08x%08x", dwSid1, dwSid2, dwSid3, dwSid4 );
	unpackWord( &buf, &wIpLen );
	if( responseBytes < 30 + wIpLen || wIpLen == 0 || wIpLen > sizeof( szHttpServer ) - 1 )
	{
		SetLastError( ERROR_INVALID_DATA );
		return 0;
	}
	memcpy( szHttpServer, buf, wIpLen );
	szHttpServer[wIpLen] = '\0';

	nlhpi.cbSize = sizeof( nlhpi );
	nlhpi.flags = NLHPIF_USEPOSTSEQUENCE;
	nlhpi.szHttpGetUrl = szHttpGetUrl;
	nlhpi.szHttpPostUrl = szHttpPostUrl;
	nlhpi.firstPostSequence = 1;
	sprintf( szHttpGetUrl, "http://%s/monitor?sid=%s", szHttpServer, szSid );
	sprintf( szHttpPostUrl, "http://%s/data?sid=%s&seq=", szHttpServer, szSid );
	return JCallService( MS_NETLIB_SETHTTPPROXYINFO, ( WPARAM )hConn, ( LPARAM )&nlhpi );
#endif
	return 1;
}

int JabberHttpGatewayBegin( HANDLE hConn, NETLIBOPENCONNECTION *nloc )
{
	/*
	icq_packet packet;
	int serverNameLen;

	serverNameLen = strlen( nloc->szHost );

	packet.wLen = ( WORD )( serverNameLen + 4 );
	write_httphdr( &packet, HTTP_PACKETTYPE_LOGIN );
	packWord( &packet, ( WORD )serverNameLen );
	packString( &packet, nloc->szHost, ( WORD )serverNameLen );
	packWord( &packet, nloc->wPort );
	Netlib_Send( hConn, packet.pData, packet.wLen, MSG_DUMPPROXY|MSG_NOHTTPGATEWAYWRAP );
	mir_free( packet.pData );
	return 1;
	*/
	return 1;
}

#if 0
int icq_httpGatewayWrapSend( HANDLE hConn, PBYTE buf, int len, int flags, MIRANDASERVICE pfnNetlibSend )
{
	icq_packet packet;
	int sendResult;

	packet.wLen = len;
	write_httphdr( &packet, HTTP_PACKETTYPE_FLAP );
	packString( &packet, buf, ( WORD )len );
	sendResult = Netlib_Send( hConn, packet.pData, packet.wLen, flags );
	mir_free( packet.pData );
	if( sendResult <= 0 )
		return sendResult;
	if( sendResult < 14 )
		return 0;
	return sendResult - 14;
}

PBYTE icq_httpGatewayUnwrapRecv( NETLIBHTTPREQUEST *nlhr, PBYTE buf, int len, int *outBufLen, void *( *NetlibRealloc )( void *, size_t ))
{
	WORD wLen, wType;
	PBYTE tbuf;
	int i, copyBytes;

	tbuf = buf;
	for( i = 0;; )
	{
		if ( tbuf - buf + 2 > len ) break;
		unpackWord( &tbuf, &wLen );
		if ( wLen < 12 ) break;
		if ( tbuf - buf + wLen > len ) break;
		tbuf += 2;	  /* version */
		unpackWord( &tbuf, &wType );
		tbuf += 8;   /* flags & subtype */
		if ( wType == HTTP_PACKETTYPE_FLAP )
		{
			copyBytes = wLen - 12;
			if ( copyBytes > len - i )
			{
				/* invalid data - do our best to get something out of it */
				copyBytes = len - i;
			}
			memcpy( buf + i, tbuf, copyBytes );
			i += copyBytes;
		}
		tbuf += wLen - 12;
	}
	*outBufLen = i;
	return buf;
}
#endif
