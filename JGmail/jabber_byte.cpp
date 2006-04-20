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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_byte.cpp,v $
Revision       : $Revision: 1.14 $
Last change on : $Date: 2006/01/29 16:56:22 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_iq.h"
#include "jabber_byte.h"

#define JABBER_NETWORK_BUFFER_SIZE 4096

///////////////// Bytestream sending /////////////////////////

static void JabberByteInitiateResult( XmlNode *iqNode, void *userdata );
static void JabberByteSendConnection( HANDLE hNewConnection, DWORD dwRemoteIP );
static int JabberByteSendParse( HANDLE hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen );

void JabberByteFreeJbt( JABBER_BYTE_TRANSFER *jbt )
{
	if ( !jbt ) return;
	if ( jbt->srcJID ) free( jbt->srcJID );
	if ( jbt->dstJID ) free( jbt->dstJID );
	if ( jbt->streamhostJID ) free( jbt->streamhostJID );
	if ( jbt->iqId ) free( jbt->iqId );
	if ( jbt->sid ) free( jbt->sid );
	if ( jbt->iqNode ) JabberXmlFreeNode( jbt->iqNode );
	free( jbt );
}

void __cdecl JabberByteSendThread( JABBER_BYTE_TRANSFER *jbt )
{
	BOOL bDirect, bProxy;
	char streamHost[256];
	char* localAddr;
	struct in_addr in;
	DBVARIANT dbv;
	NETLIBBIND nlb = {0};
	char szPort[8];
	int iqId;
	JABBER_LIST_ITEM *item;
	HANDLE hEvent;

	JabberLog( "Thread started: type=bytestream_send" );

	bDirect = JGetByte( "BsDirect", TRUE );
	bProxy = JGetByte( "BsProxy", FALSE );

	streamHost[0] = '\0';
	if ( bDirect ) {
		localAddr = NULL;
		if ( JGetByte( "BsDirectManual", FALSE ) == TRUE ) {
			if ( !DBGetContactSetting( NULL, jabberProtoName, "BsDirectAddr", &dbv )) {
				localAddr = _strdup( dbv.pszVal );
				JFreeVariant( &dbv );
			}
		}
		if ( localAddr == NULL ) {
			in.S_un.S_addr = jabberLocalIP;
			localAddr = _strdup( inet_ntoa( in ));
		}
		nlb.cbSize = sizeof( NETLIBBIND );
		nlb.pfnNewConnection = JabberByteSendConnection;
		nlb.wPort = 0;	// Use user-specified incoming port ranges, if available
		jbt->hConn = ( HANDLE ) JCallService( MS_NETLIB_BINDPORT, ( WPARAM ) hNetlibUser, ( LPARAM )&nlb );
		if ( jbt->hConn == NULL ) {
			JabberLog( "Cannot allocate port for bytestream_send thread, thread ended." );
			JabberByteFreeJbt( jbt );
			return;
		}
		mir_snprintf( szPort, sizeof( szPort ), "%d", nlb.wPort );
		item = JabberListAdd( LIST_BYTE, szPort );
		item->jbt = jbt;
		hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		jbt->hEvent = hEvent;
        mir_snprintf( streamHost, sizeof( streamHost ), "<streamhost jid=\"%s\" host=\"%s\" port=\"%d\"/>", jabberThreadInfo->fullJID, localAddr, nlb.wPort );
		free( localAddr );
	}

	iqId = JabberSerialNext();
	JabberIqAdd( iqId, IQ_PROC_NONE, JabberByteInitiateResult );
	JabberSend( jabberThreadInfo->s,
        "<iq type=\"set\" to=\"%s\" id=\""JABBER_IQID"%d\">"
            "<query xmlns=\"http://jabber.org/protocol/bytestreams\" sid=\"%s\">"
				"%s"
			"</query>"
		"</iq>",
		jbt->dstJID, iqId, jbt->sid, streamHost );

	if ( bDirect ) {
		WaitForSingleObject( hEvent, INFINITE );
		CloseHandle( hEvent );
		jbt->hEvent = NULL;
		jbt->pfnFinal(( jbt->state==JBT_DONE )?TRUE:FALSE, jbt->userdata );
		if ( jbt->hConn != NULL )
			Netlib_CloseHandle( jbt->hConn );
		JabberByteFreeJbt( jbt );
		JabberListRemove( LIST_BYTE, szPort );
	}

	JabberLog( "Thread ended: type=bytestream_send" );
}

static void JabberByteInitiateResult( XmlNode *iqNode, void *userdata )
{
	char* type;

	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if ( !strcmp( type, "result" )) {
	}
	else if ( !strcmp( type, "error" )) {
	}
}

static void JabberByteSendConnection( HANDLE hConn, DWORD dwRemoteIP )
{
	SOCKET s;
	SOCKADDR_IN saddr;
	int len;
	WORD localPort;
	char szPort[8];
	JABBER_BYTE_TRANSFER *jbt;
	int recvResult, bytesParsed;
	HANDLE hListen;
	JABBER_LIST_ITEM *item;
	char* buffer;
	int datalen;

	localPort = 0;
	if (( s=JCallService( MS_NETLIB_GETSOCKET, ( WPARAM ) hConn, 0 )) != INVALID_SOCKET ) {
		len = sizeof( saddr );
		if ( getsockname( s, ( SOCKADDR * ) &saddr, &len ) != SOCKET_ERROR )
			localPort = ntohs( saddr.sin_port );
	}
	if ( localPort == 0 ) {
		JabberLog( "bytestream_send_connection unable to determine the local port, connection closed." );
		Netlib_CloseHandle( hConn );
		return;
	}

	mir_snprintf( szPort, sizeof( szPort ), "%d", localPort );
	JabberLog( "bytestream_send_connection incoming connection accepted: local_port=%s", szPort );

	if (( item=JabberListGetItemPtr( LIST_BYTE, szPort )) == NULL ) {
		JabberLog( "No bytestream session is currently active, connection closed." );
		Netlib_CloseHandle( hConn );
		return;
	}

	jbt = item->jbt;

	if (( buffer=( char* )malloc( JABBER_NETWORK_BUFFER_SIZE )) == NULL ) {
		JabberLog( "bytestream_send cannot allocate network buffer, connection closed." );
		jbt->state = JBT_ERROR;
		Netlib_CloseHandle( hConn );
		if ( jbt->hEvent != NULL ) SetEvent( jbt->hEvent );
		return;
	}

	hListen = jbt->hConn;
	jbt->hConn = hConn;
	jbt->state = JBT_INIT;
	datalen = 0;
	while ( jbt->state!=JBT_DONE && jbt->state!=JBT_ERROR ) {
		recvResult = Netlib_Recv( hConn, buffer+datalen, JABBER_NETWORK_BUFFER_SIZE-datalen, 0 );
		if ( recvResult <= 0 ) break;
		datalen += recvResult;
		bytesParsed = JabberByteSendParse( hConn, jbt, buffer, datalen );
		if ( bytesParsed < datalen )
			memmove( buffer, buffer+bytesParsed, datalen-bytesParsed );
		datalen -= bytesParsed;
	}
	if ( jbt->hConn ) Netlib_CloseHandle( jbt->hConn );
	JabberLog( "bytestream_send_connection closing connection" );
	jbt->hConn = hListen;
	free( buffer );

	if ( jbt->hEvent != NULL ) SetEvent( jbt->hEvent );
}

static int JabberByteSendParse( HANDLE hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen )
{
	int nMethods;
	BYTE data[10];
	char text[256];
	int i;
	char* str;

	switch ( jbt->state ) {
	case JBT_INIT:
		// received:
		// 00-00 ver ( 0x05 )
		// 01-01 nmethods
		// 02-xx list of methods ( nmethods bytes )
		// send:
		// 00-00 ver ( 0x05 )
		// 01-01 select method ( 0=no auth required )
		if ( datalen>=2 && buffer[0]==5 && buffer[1]+2==datalen ) {
			nMethods = buffer[1];
			for ( i=0; i<nMethods && buffer[2+i]!=0; i++ );
			if ( i < nMethods ) {
				data[1] = 0;
				jbt->state = JBT_CONNECT;
			}
			else {
				data[1] = 0xff;
				jbt->state = JBT_ERROR;
			}
			data[0] = 5;
			Netlib_Send( hConn, ( char* )data, 2, 0 );
		}
		else
			jbt->state = JBT_ERROR;
		break;
	case JBT_CONNECT:
		// received:
		// 00-00 ver ( 0x05 )
		// 01-01 cmd ( 1=connect )
		// 02-02 reserved ( 0 )
		// 03-03 address type ( 3 )
		// 04-44 dst.addr ( 41 bytes: 1-byte length, 40-byte SHA1 hash of [sid,srcJID,dstJID] )
		// 45-46 dst.port ( 0 )
		// send:
		// 00-00 ver ( 0x05 )
		// 01-01 reply ( 0=success,2=not allowed )
		// 02-02 reserved ( 0 )
		// 03-03 address type ( 1=IPv4 address )
		// 04-07 bnd.addr server bound address
		// 08-09 bnd.port server bound port
		if ( datalen==47 && *(( DWORD* )buffer )==0x03000105 && buffer[4]==40 && *(( WORD* )( buffer+45 ))==0 ) {
			mir_snprintf( text, sizeof( text ), "%s%s%s", jbt->sid, jbt->srcJID, jbt->dstJID );
			JabberLog( "Auth: '%s'", text );
			if (( str=JabberSha1( text )) != NULL ) {
				for ( i=0; i<40 && buffer[i+5]==str[i]; i++ );
				free( str );
				ZeroMemory( data, 10 );
				data[1] = ( i>=20 )?0:2;
				data[0] = 5;
				data[3] = 1;
				Netlib_Send( hConn, ( char* )data, 10, 0 );
				if ( i>=20 && jbt->pfnSend( hConn, jbt->userdata )==TRUE )
					jbt->state = JBT_DONE;
				else
					jbt->state = JBT_ERROR;
			}
		}
		else
			jbt->state = JBT_ERROR;
		break;
	}

	return datalen;
}

///////////////// Bytestream receiving /////////////////////////

static int JabberByteReceiveParse( HANDLE hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen );

void __cdecl JabberByteReceiveThread( JABBER_BYTE_TRANSFER *jbt )
{
	XmlNode *iqNode, *queryNode, *n;
	char* from, *to, *sid, *szHost, *szPort, *szId, *str;
	int i;
	WORD port;
	HANDLE hConn;
	char data[3];
	char* buffer;
	int datalen, bytesParsed, recvResult;
	BOOL validStreamhost;

	if ( jbt == NULL ) return;
	if (( iqNode=jbt->iqNode )!=NULL &&
		( from=JabberXmlGetAttrValue( iqNode, "from" ))!=NULL &&
		( to=JabberXmlGetAttrValue( iqNode, "to" ))!=NULL &&
		( queryNode=JabberXmlGetChild( iqNode, "query" ))!=NULL &&
		( sid=JabberXmlGetAttrValue( queryNode, "sid" ))!=NULL &&
		( n=JabberXmlGetChild( queryNode, "streamhost" ))!=NULL ) {

		szId = JabberXmlGetAttrValue( iqNode, "id" );
		jbt->iqId = ( szId )?_strdup( szId ):NULL;
		jbt->srcJID = JabberUrlDecodeNew( from );
		jbt->dstJID = JabberUrlDecodeNew( to );
		jbt->sid = _strdup( sid );

		if (( buffer=( char* )malloc( JABBER_NETWORK_BUFFER_SIZE )) == NULL ) {
			JabberLog( "bytestream_send cannot allocate network buffer, connection closed." );
            JabberSend( jabberThreadInfo->s, "<iq type=\"error\" to=\"%s\" id=\"%s\"><error code=\"406\" type=\"auth\"><not-acceptable xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/></error></iq>", jbt->srcJID, jbt->iqId );
			JabberByteFreeJbt( jbt );
			return;
		}

		jbt->state = JBT_INIT;
		validStreamhost = FALSE;
		for ( i=1; ( n=JabberXmlGetNthChild( queryNode, "streamhost", i ))!=NULL; i++ ) {
			if (( szHost=JabberXmlGetAttrValue( n, "host" ))!=NULL &&
				( szPort=JabberXmlGetAttrValue( n, "port" ))!=NULL &&
				( str=JabberXmlGetAttrValue( n, "jid" ))!=NULL ) {

				port = ( WORD )atoi( szPort );
				if ( jbt->streamhostJID ) free( jbt->streamhostJID );
				jbt->streamhostJID =  JabberUrlDecodeNew( str );

				JabberLog( "bytestream_recv connecting to %s:%d", szHost, port );
				NETLIBOPENCONNECTION nloc = { 0 };
				nloc.cbSize = sizeof( nloc );
				nloc.szHost = szHost;
				nloc.wPort = port;
				hConn = ( HANDLE ) JCallService( MS_NETLIB_OPENCONNECTION, ( WPARAM ) hNetlibUser, ( LPARAM )&nloc );
				if ( hConn == NULL ) {
					JabberLog( "bytestream_recv_connection connection failed ( %d ), try next streamhost", WSAGetLastError());
					continue;
				}

				jbt->hConn = hConn;

				data[0] = 5;
				data[1] = 1;
				data[2] = 0;
				Netlib_Send( hConn, data, 3, 0 );

				jbt->state = JBT_INIT;
				datalen = 0;
				while ( jbt->state!=JBT_DONE && jbt->state!=JBT_ERROR && jbt->state!=JBT_SOCKSERR ) {
					recvResult = Netlib_Recv( hConn, buffer+datalen, JABBER_NETWORK_BUFFER_SIZE-datalen, 0 );
					if ( recvResult <= 0 ) break;
					datalen += recvResult;
					bytesParsed = JabberByteReceiveParse( hConn, jbt, buffer, datalen );
					if ( bytesParsed < datalen )
						memmove( buffer, buffer+bytesParsed, datalen-bytesParsed );
					datalen -= bytesParsed;
					if ( jbt->state == JBT_RECVING ) validStreamhost = TRUE;
				}
				Netlib_CloseHandle( hConn );
				JabberLog( "bytestream_recv_connection closing connection" );
			}
			if ( jbt->state==JBT_ERROR || validStreamhost==TRUE )
				break;
			JabberLog( "bytestream_recv_connection stream cannot be established, try next streamhost" );
		}
		free( buffer );
		jbt->pfnFinal(( jbt->state==JBT_DONE )?TRUE:FALSE, jbt->userdata );
		if ( !validStreamhost ) {
			JabberLog( "bytestream_recv_connection session not completed" );
            JabberSend( jabberThreadInfo->s, "<iq type=\"error\" to=\"%s\" id=\"%s\"><error code=\"404\" type=\"cancel\"><item-not-found xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/></error></iq>", jbt->srcJID, jbt->iqId );
		}
	}
	JabberByteFreeJbt( jbt );
	JabberLog( "Thread ended: type=bytestream_recv" );
}

static int JabberByteReceiveParse( HANDLE hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen )
{
	BYTE data[47];
	char text[256];
	char* szHash;
	int bytesReceived;
	int num;

	num = datalen;
	switch ( jbt->state ) {
	case JBT_INIT:
		// received:
		// 00-00 ver ( 0x05 )
		// 01-01 selected method ( 0=no auth, 0xff=error )
		// send:
		// 00-00 ver ( 0x05 )
		// 01-01 cmd ( 1=connect )
		// 02-02 reserved ( 0 )
		// 03-03 address type ( 3 )
		// 04-44 dst.addr ( 41 bytes: 1-byte length, 40-byte SHA1 hash of [sid,srcJID,dstJID] )
		// 45-46 dst.port ( 0 )
		if ( datalen==2 && buffer[0]==5 && buffer[1]==0 ) {
			ZeroMemory( data, sizeof( data ));
			*(( DWORD* )data ) = 0x03000105;
			data[4] = 40;
			mir_snprintf( text, sizeof( text ), "%s%s%s", jbt->sid, jbt->srcJID, jbt->dstJID );
            JabberLog( "Auth: \"%s\"", text );
			szHash = JabberSha1( text );
			strncpy(( char* )( data+5 ), szHash, 40 );
			free( szHash );
			Netlib_Send( hConn, ( char* )data, 47, 0 );
			jbt->state = JBT_CONNECT;
		}
		else
			jbt->state = JBT_SOCKSERR;
		break;
	case JBT_CONNECT:
		// received:
		// 00-00 ver ( 0x05 )
		// 01-01 reply ( 0=success,2=not allowed )
		// 02-02 reserved ( 0 )
		// 03-03 address type ( 1=IPv4 address,3=host address )
		// 04-mm bnd.addr server bound address ( 4-byte IP if IPv4, 1-byte length + n-byte host address string if host address )
		// nn-nn+1 bnd.port server bound port
		if ( datalen>=5 && buffer[0]==5 && buffer[1]==0 && ( buffer[3]==1 || buffer[3]==3 )) {
			if ( buffer[3]==1 && datalen>=10 )
				num = 10;
			else if ( buffer[3]==3 && datalen>=buffer[4]+7 )
				num = buffer[4] + 7;
			else {
				jbt->state = JBT_SOCKSERR;
				break;
			}
			jbt->state = JBT_RECVING;
            JabberSend( jabberThreadInfo->s, "<iq type=\"result\" to=\"%s\" id=\"%s\"><query xmlns=\"http://jabber.org/protocol/bytestreams\"><streamhost-used jid=\"%s\"/></query></iq>", jbt->srcJID, jbt->iqId, TXT(jbt->streamhostJID));
		}
		else
			jbt->state = JBT_SOCKSERR;
		break;
	case JBT_RECVING:
		bytesReceived = jbt->pfnRecv( hConn, jbt->userdata, buffer, datalen );
		if ( bytesReceived < 0 )
			jbt->state = JBT_ERROR;
		else if ( bytesReceived == 0 )
			jbt->state = JBT_DONE;
		break;
	}

	return num;
}
