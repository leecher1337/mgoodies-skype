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

#include "jabber.h"

BOOL JabberWsInit( void )
{
	NETLIBUSER nlu = {0};
	char name[128];

	sprintf( name, "%s %s", jabberModuleName, JTranslate( "connection" ));

	nlu.cbSize = sizeof( nlu );
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS;	// | NUF_HTTPGATEWAY;
	nlu.szDescriptiveName = name;
	nlu.szSettingsModule = jabberProtoName;
	//nlu.szHttpGatewayHello = "http://http.proxy.icq.com/hello";
	//nlu.szHttpGatewayUserAgent = "Mozilla/4.08 [en] ( WinNT; U ;Nav )";
	//nlu.pfnHttpGatewayInit = JabberHttpGatewayInit;
	//nlu.pfnHttpGatewayBegin = JabberHttpGatewayBegin;
	//nlu.pfnHttpGatewayWrapSend = JabberHttpGatewayWrapSend;
	//nlu.pfnHttpGatewayUnwrapRecv = JabberHttpGatewayUnwrapRecv;
	hNetlibUser = ( HANDLE ) JCallService( MS_NETLIB_REGISTERUSER, 0, ( LPARAM )&nlu );

	return ( hNetlibUser!=NULL )?TRUE:FALSE;
}

void JabberWsUninit( void )
{
	Netlib_CloseHandle( hNetlibUser );
	hNetlibUser = NULL;
}

JABBER_SOCKET JabberWsConnect( char* host, WORD port )
{
	NETLIBOPENCONNECTION nloc = { 0 };
	nloc.cbSize = sizeof( nloc );
	nloc.szHost = host;
	nloc.wPort = port;
	return ( HANDLE )JCallService( MS_NETLIB_OPENCONNECTION, ( WPARAM ) hNetlibUser, ( LPARAM )&nloc );
}

int JabberWsSend( JABBER_SOCKET hConn, char* data, int datalen )
{
	int len;

	if (( len=Netlib_Send( hConn, data, datalen, MSG_DUMPASTEXT ))==SOCKET_ERROR || len!=datalen ) {
		JabberLog( "Netlib_Send() failed, error=%d", WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}

int JabberWsRecv( JABBER_SOCKET hConn, char* data, long datalen )
{
	int ret;

	ret = Netlib_Recv( hConn, data, datalen, MSG_DUMPASTEXT );
	if( ret == SOCKET_ERROR ) {
		JabberLog( "Netlib_Recv() failed, error=%d", WSAGetLastError());
		return 0;
	}
	if( ret == 0 ) {
		JabberLog( "Connection closed gracefully" );
		return 0;
	}
	return ret;
}
