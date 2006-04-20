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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_thread.cpp,v $
Revision       : $Revision: 1.48 $
Last change on : $Date: 2006/03/16 19:08:24 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"

#include <io.h>
#include <WinDNS.h>   // requires Windows Platform SDK

#include "jabber_ssl.h"
#include "jabber_list.h"
#include "jabber_iq.h"
#include "resource.h"

// <iq/> identification number for various actions
// for JABBER_REGISTER thread
unsigned int iqIdRegGetReg;
unsigned int iqIdRegSetReg;

static void __cdecl JabberKeepAliveThread( JABBER_SOCKET s );
static void JabberProcessStreamOpening( XmlNode *node, void *userdata );
static void JabberProcessStreamClosing( XmlNode *node, void *userdata );
static void JabberProcessProtocol( XmlNode *node, void *userdata );
static void JabberProcessMessage( XmlNode *node, void *userdata );
static void JabberProcessPresence( XmlNode *node, void *userdata );
static void JabberProcessIq( XmlNode *node, void *userdata );
static void JabberProcessProceed( XmlNode *node, void *userdata );
static void JabberProcessRegIq( XmlNode *node, void *userdata );

static VOID CALLBACK JabberDummyApcFunc( DWORD param )
{
	return;
}

static char onlinePassword[128];
static HANDLE hEventPasswdDlg;

static BOOL CALLBACK JabberPasswordDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	char text[128];

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		wsprintfA( text, "%s %s", JTranslate( "Enter password for" ), ( char* )lParam );
		SetDlgItemTextA( hwndDlg, IDC_JID, text );
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
			GetDlgItemTextA( hwndDlg, IDC_PASSWORD, onlinePassword, sizeof( onlinePassword ));
			JabberLog( "Password is %s", onlinePassword );
			//EndDialog( hwndDlg, ( int ) onlinePassword );
			//return TRUE;
			// Fall through
		case IDCANCEL:
			//EndDialog( hwndDlg, 0 );
			SetEvent( hEventPasswdDlg );
			DestroyWindow( hwndDlg );
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static VOID CALLBACK JabberPasswordCreateDialogApcProc( DWORD param )
{
	CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_PASSWORD ), NULL, JabberPasswordDlgProc, ( LPARAM )param );
}

static VOID CALLBACK JabberOfflineChatWindows( DWORD )
{
	GCDEST gcd = { jabberProtoName, NULL, GC_EVENT_CONTROL };
	GCEVENT gce = { 0 };
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	CallService( MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce );
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef DNS_STATUS (WINAPI *DNSQUERYA)(IN PCSTR pszName, IN WORD wType, IN DWORD Options, IN PIP4_ARRAY aipServers OPTIONAL, IN OUT PDNS_RECORD *ppQueryResults OPTIONAL, IN OUT PVOID *pReserved OPTIONAL);
typedef void (WINAPI *DNSFREELIST)(IN OUT PDNS_RECORD pRecordList, IN DNS_FREE_TYPE FreeType);

static int xmpp_client_query( char* domain )
{
	HINSTANCE hDnsapi = LoadLibraryA( "dnsapi.dll" );
	if ( hDnsapi == NULL )
		return 0;

	DNSQUERYA pDnsQuery_A = (DNSQUERYA)GetProcAddress(hDnsapi, "DnsQuery_A");
	DNSFREELIST pDnsRecordListFree = (DNSFREELIST)GetProcAddress(hDnsapi, "DnsRecordListFree");
	if ( pDnsQuery_A == NULL || pDnsQuery_A == NULL ) {
		//dnsapi.dll is not the needed dnsapi ;)
		FreeLibrary( hDnsapi );
		return 0;
	}

   char temp[256];
	mir_snprintf( temp, sizeof temp, "_xmpp-client._tcp.%s", domain );

	DNS_RECORD *results = NULL;
	DNS_STATUS status = pDnsQuery_A(temp, DNS_TYPE_SRV, DNS_QUERY_STANDARD, NULL, &results, NULL);
	if (FAILED(status)||!results || results[0].Data.Srv.pNameTarget == 0||results[0].wType != DNS_TYPE_SRV) {
		FreeLibrary(hDnsapi);
		return NULL;
	}

	strncpy(domain, (char*)results[0].Data.Srv.pNameTarget, 127);
	int port = results[0].Data.Srv.wPort;
	pDnsRecordListFree(results, DnsFreeRecordList);
	FreeLibrary(hDnsapi);
	return port;
}

void __cdecl JabberServerThread( struct ThreadData *info )
{
	DBVARIANT dbv;
	char* buffer;
	int datalen;
	int oldStatus;
	PVOID ssl;

	JabberLog( "Thread started: type=%d", info->type );

	if ( info->type == JABBER_SESSION_NORMAL ) {

		// Normal server connection, we will fetch all connection parameters
		// e.g. username, password, etc. from the database.

		if ( jabberThreadInfo != NULL ) {
			// Will not start another connection thread if a thread is already running.
			// Make APC call to the main thread. This will immediately wake the thread up
			// in case it is asleep in the reconnect loop so that it will immediately
			// reconnect.
			QueueUserAPC( JabberDummyApcFunc, jabberThreadInfo->hThread, 0 );
			JabberLog( "Thread ended, another normal thread is running" );
			free( info );
			return;
		}

		jabberThreadInfo = info;
		if ( streamId ) free( streamId );
		streamId = NULL;

		if ( !DBGetContactSetting( NULL, jabberProtoName, "LoginName", &dbv )) {
			strncpy( info->username, dbv.pszVal, sizeof( info->username )-1 );
			JFreeVariant( &dbv );
		}
		else {
			JabberLog( "Thread ended, login name is not configured" );
			JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID );
LBL_FatalError:
			jabberThreadInfo = NULL;
			oldStatus = jabberStatus;
			jabberStatus = ID_STATUS_OFFLINE;
			JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );
LBL_Exit:
			free( info );
			return;
		}

		if ( !DBGetContactSetting( NULL, jabberProtoName, "LoginServer", &dbv )) {
			strncpy( info->server, dbv.pszVal, sizeof( info->server )-1 );
			JFreeVariant( &dbv );
		}
		else {
			JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK );
			JabberLog( "Thread ended, login server is not configured" );
			goto LBL_FatalError;
		}

		if ( !DBGetContactSetting( NULL, jabberProtoName, "Resource", &dbv )) {
			strncpy( info->resource, dbv.pszVal, sizeof( info->resource )-1 );
			JFreeVariant( &dbv );
		}
		else strcpy( info->resource, "Miranda" );

		char jidStr[128];
		mir_snprintf( jidStr, sizeof( jidStr ), "%s@%s/%s", info->username, info->server, info->resource );
		strncpy( info->fullJID, TXT(jidStr), sizeof( info->fullJID )-1 );

		if ( JGetByte( "SavePassword", TRUE ) == FALSE ) {
			mir_snprintf( jidStr, sizeof( jidStr ), "%s@%s", info->username, info->server );
			// Ugly hack: continue logging on only the return value is &( onlinePassword[0] )
			// because if WM_QUIT while dialog box is still visible, p is returned with some
			// exit code which may not be NULL.
			// Should be better with modeless.
			onlinePassword[0] = ( char ) -1;
			hEventPasswdDlg = CreateEvent( NULL, FALSE, FALSE, NULL );
			QueueUserAPC( JabberPasswordCreateDialogApcProc, hMainThread, ( DWORD )jidStr );
			WaitForSingleObject( hEventPasswdDlg, INFINITE );
			CloseHandle( hEventPasswdDlg );
			//if (( p=( char* )DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_PASSWORD ), NULL, JabberPasswordDlgProc, ( LPARAM )jidStr )) != onlinePassword ) {
			if ( onlinePassword[0] == ( char ) -1 ) {
				JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID );
				JabberLog( "Thread ended, password request dialog was canceled" );
				goto LBL_FatalError;
			}
			strncpy( info->password, onlinePassword, sizeof( info->password ));
			info->password[sizeof( info->password )-1] = '\0';
		}
		else {
			if ( DBGetContactSetting( NULL, jabberProtoName, "Password", &dbv )) {
				JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID );
				JabberLog( "Thread ended, password is not configured" );
				goto LBL_FatalError;
			}
			JCallService( MS_DB_CRYPT_DECODESTRING, strlen( dbv.pszVal )+1, ( LPARAM )dbv.pszVal );
			strncpy( info->password, dbv.pszVal, sizeof( info->password ));
			info->password[sizeof( info->password )-1] = '\0';
			JFreeVariant( &dbv );
		}

		if ( JGetByte( "ManualConnect", FALSE ) == TRUE ) {
			if ( !DBGetContactSetting( NULL, jabberProtoName, "ManualHost", &dbv )) {
				strncpy( info->manualHost, dbv.pszVal, sizeof( info->manualHost ));
				info->manualHost[sizeof( info->manualHost )-1] = '\0';
				JFreeVariant( &dbv );
			}
			info->port = JGetWord( NULL, "ManualPort", JABBER_DEFAULT_PORT );
		}
		else info->port = JGetWord( NULL, "Port", JABBER_DEFAULT_PORT );

		info->useSSL = JGetByte( "UseSSL", FALSE );
	}

	else if ( info->type == JABBER_SESSION_REGISTER ) {
		// Register new user connection, all connection parameters are already filled-in.
		// Multiple thread allowed, although not possible : )
		// thinking again.. multiple thread should not be allowed
		info->reg_done = FALSE;
		SendMessage( info->reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 25, ( LPARAM )JTranslate( "Connecting..." ));
		iqIdRegGetReg = -1;
		iqIdRegSetReg = -1;
	}
	else {
		JabberLog( "Thread ended, invalid session type" );
		goto LBL_Exit;
	}

	char connectHost[128];
	if ( info->manualHost[0] == 0 ) {
		int port_temp;
		strncpy( connectHost, info->server, 128 );
		if ( port_temp = xmpp_client_query( connectHost )) { // port_temp will be > 0 if resolution is successful
			JabberLog("%s%s resolved to %s:%d","_xmpp-client._tcp.",info->server,connectHost,port_temp);
			if (info->port==0 || info->port==5222)
				info->port = port_temp;
		}
		else JabberLog("%s%s not resolved", "_xmpp-client._tcp.", connectHost);
	}
	else strncpy(connectHost,info->manualHost,128); // do not resolve if manual host is selected

	JabberLog( "Thread type=%d server='%s' port='%d'", info->type, connectHost, info->port );

	int jabberNetworkBufferSize = 2048;
	if (( buffer=( char* )malloc( jabberNetworkBufferSize+1 )) == NULL ) {	// +1 is for '\0' when debug logging this buffer
		JabberLog( "Cannot allocate network buffer, thread ended" );
		if ( info->type == JABBER_SESSION_NORMAL ) {
			oldStatus = jabberStatus;
			jabberStatus = ID_STATUS_OFFLINE;
			JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK );
			JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );
			jabberThreadInfo = NULL;
		}
		else if ( info->type == JABBER_SESSION_REGISTER ) {
			SendMessage( info->reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, ( LPARAM )JTranslate( "Error: Not enough memory" ));
		}
		JabberLog( "Thread ended, network buffer cannot be allocated" );
		goto LBL_Exit;
	}

	info->s = JabberWsConnect( connectHost, info->port );
	if ( info->s == NULL ) {
		JabberLog( "Connection failed ( %d )", WSAGetLastError());
		if ( info->type == JABBER_SESSION_NORMAL ) {
			if ( jabberThreadInfo == info ) {
				oldStatus = jabberStatus;
				jabberStatus = ID_STATUS_OFFLINE;
				JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK );
				JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );
				jabberThreadInfo = NULL;
		}	}
		else if ( info->type == JABBER_SESSION_REGISTER )
			SendMessage( info->reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, ( LPARAM )JTranslate( "Error: Cannot connect to the server" ));

		JabberLog( "Thread ended, connection failed" );
		free( buffer );
		goto LBL_Exit;
	}

	// Determine local IP
	int socket = JCallService( MS_NETLIB_GETSOCKET, ( WPARAM ) info->s, 0 );
	if ( info->type==JABBER_SESSION_NORMAL && socket!=INVALID_SOCKET ) {
		struct sockaddr_in saddr;
		int len;

		len = sizeof( saddr );
		getsockname( socket, ( struct sockaddr * ) &saddr, &len );
		jabberLocalIP = saddr.sin_addr.S_un.S_addr;
		JabberLog( "Local IP = %s", inet_ntoa( saddr.sin_addr ));
	}

	BOOL sslMode = FALSE;
	if ( info->useSSL ) {
		JabberLog( "Intializing SSL connection" );
		if ( 
#ifndef STATICSSL
			hLibSSL!=NULL && 
#endif
			socket!=INVALID_SOCKET ) {
			JabberLog( "SSL using socket = %d", socket );
			if (( ssl=pfn_SSL_new( jabberSslCtx )) != NULL ) {
				JabberLog( "SSL create context ok" );
				if ( pfn_SSL_set_fd( ssl, socket ) > 0 ) {
					JabberLog( "SSL set fd ok" );
					if ( pfn_SSL_connect( ssl ) > 0 ) {
						JabberLog( "SSL negotiation ok" );
						JabberSslAddHandle( info->s, ssl );	// This make all communication on this handle use SSL
						sslMode = TRUE;		// Used in the receive loop below
						JabberLog( "SSL enabled for handle = %d", info->s );
					}
					else {
						JabberLog( "SSL negotiation failed" );
						pfn_SSL_free( ssl );
				}	}
				else {
					JabberLog( "SSL set fd failed" );
					pfn_SSL_free( ssl );
		}	}	}

		if ( !sslMode ) {
			if ( info->type == JABBER_SESSION_NORMAL ) {
				oldStatus = jabberStatus;
				jabberStatus = ID_STATUS_OFFLINE;
				JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );
				JSendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK );
				if ( jabberThreadInfo == info )
					jabberThreadInfo = NULL;
			}
			else if ( info->type == JABBER_SESSION_REGISTER ) {
				SendMessage( info->reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, ( LPARAM )JTranslate( "Error: Cannot connect to the server" ));
			}
			free( buffer );
#ifndef STATICSSL
			if ( !hLibSSL )
#endif
				MessageBox( NULL, TranslateT( "The connection requires an OpenSSL library, which is not installed." ), TranslateT( "Jabber Connection Error" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
			JabberLog( "Thread ended, SSL connection failed" );
			goto LBL_Exit;
	}	}

	// User may change status to OFFLINE while we are connecting above
	if ( jabberDesiredStatus!=ID_STATUS_OFFLINE || info->type==JABBER_SESSION_REGISTER ) {

		if ( info->type == JABBER_SESSION_NORMAL ) {
			jabberConnected = TRUE;
			jabberJID = ( char* )malloc( strlen( info->username )+strlen( info->server )+2 );
			sprintf( jabberJID, "%s@%s", info->username, info->server );
			if ( JGetByte( "KeepAlive", 1 ))
				jabberSendKeepAlive = TRUE;
			else
				jabberSendKeepAlive = FALSE;
			JabberForkThread( JabberKeepAliveThread, 0, info->s );
		}

		XmlState xmlState;
		JabberXmlInitState( &xmlState );
		JabberXmlSetCallback( &xmlState, 1, ELEM_OPEN, JabberProcessStreamOpening, info );
		JabberXmlSetCallback( &xmlState, 1, ELEM_CLOSE, JabberProcessStreamClosing, info );
		JabberXmlSetCallback( &xmlState, 2, ELEM_CLOSE, JabberProcessProtocol, info );

		JabberSend( info->s, "<?xml version='1.0' encoding='UTF-8'?><stream:stream to='%s' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>", TXT(info->server) );

		JabberLog( "Entering main recv loop" );
		datalen = 0;

		for ( ;; ) {
			int recvResult, bytesParsed;

			if ( !sslMode ) if (info->useSSL){
				ssl = JabberSslHandleToSsl( info->s );
				sslMode = TRUE;
				JabberXmlDestroyState(&xmlState);
				JabberXmlInitState( &xmlState );
				JabberXmlSetCallback( &xmlState, 1, ELEM_OPEN, JabberProcessStreamOpening, info );
				JabberXmlSetCallback( &xmlState, 1, ELEM_CLOSE, JabberProcessStreamClosing, info );
				JabberXmlSetCallback( &xmlState, 2, ELEM_CLOSE, JabberProcessProtocol, info );
				JabberSend( info->s, "<?xml version='1.0' encoding='UTF-8'?><stream:stream to='%s' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>", TXT(info->server) );
			}

			if ( sslMode )
				recvResult = pfn_SSL_read( ssl, buffer+datalen, jabberNetworkBufferSize-datalen );
			else
				recvResult = JabberWsRecv( info->s, buffer+datalen, jabberNetworkBufferSize-datalen );

			JabberLog( "recvResult = %d", recvResult );
			if ( recvResult <= 0 )
				break;
			datalen += recvResult;

			buffer[datalen] = '\0';
			if ( sslMode && DBGetContactSettingByte( NULL, "Netlib", "DumpRecv", TRUE ) == TRUE ) {
				// Emulate netlib log feature for SSL connection
				char* szLogBuffer = ( char* )malloc( recvResult+128 );
				if ( szLogBuffer != NULL ) {
					strcpy( szLogBuffer, "( SSL ) Data received\n" );
					memcpy( szLogBuffer+strlen( szLogBuffer ), buffer+datalen-recvResult, recvResult+1 /* also copy \0 */ );
					Netlib_Logf( hNetlibUser, "%s", szLogBuffer );	// %s to protect against when fmt tokens are in szLogBuffer causing crash
					free( szLogBuffer );
			}	}

			bytesParsed = JabberXmlParse( &xmlState, buffer, datalen );
			JabberLog( "bytesParsed = %d", bytesParsed );
			if ( bytesParsed > 0 ) {
				if ( bytesParsed < datalen )
					memmove( buffer, buffer+bytesParsed, datalen-bytesParsed );
				datalen -= bytesParsed;
			}
			else if ( datalen == jabberNetworkBufferSize ) {
				jabberNetworkBufferSize += 2048;
				JabberLog( "Increasing network buffer size to %d", jabberNetworkBufferSize );
				if (( buffer=( char* )realloc( buffer, jabberNetworkBufferSize+1 )) == NULL ) {
					JabberLog( "Cannot reallocate more network buffer, go offline now" );
					break;
			}	}
			else JabberLog( "Unknown state: bytesParsed=%d, datalen=%d, jabberNetworkBufferSize=%d", bytesParsed, datalen, jabberNetworkBufferSize );
		}

		JabberXmlDestroyState( &xmlState );

		if ( info->type == JABBER_SESSION_NORMAL ) {
			jabberOnline = FALSE;
			jabberConnected = FALSE;
			JabberEnableMenuItems( FALSE );
			if ( hwndJabberChangePassword ) {
				//DestroyWindow( hwndJabberChangePassword );
				// Since this is a different thread, simulate the click on the cancel button instead
				SendMessage( hwndJabberChangePassword, WM_COMMAND, MAKEWORD( IDCANCEL, 0 ), 0 );
			}

			if ( jabberChatDllPresent )
				QueueUserAPC( JabberOfflineChatWindows, hMainThread, 0 );

			JabberListRemoveList( LIST_CHATROOM );
			if ( hwndJabberAgents )
				SendMessage( hwndJabberAgents, WM_JABBER_CHECK_ONLINE, 0, 0 );
			if ( hwndJabberGroupchat )
				SendMessage( hwndJabberGroupchat, WM_JABBER_CHECK_ONLINE, 0, 0 );
			if ( hwndJabberJoinGroupchat )
				SendMessage( hwndJabberJoinGroupchat, WM_JABBER_CHECK_ONLINE, 0, 0 );

			// Set status to offline
			oldStatus = jabberStatus;
			jabberStatus = ID_STATUS_OFFLINE;
			JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );

			// Set all contacts to offline
			HANDLE hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
			while ( hContact != NULL ) {
				if ( !lstrcmpA(( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 ), jabberProtoName ))
					if ( JGetWord( hContact, "Status", ID_STATUS_OFFLINE ) != ID_STATUS_OFFLINE )
						JSetWord( hContact, "Status", ID_STATUS_OFFLINE );

				hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM ) hContact, 0 );
			}

			free( jabberJID );
			jabberJID = NULL;
			JabberListWipe();
			if ( hwndJabberAgents ) {
				SendMessage( hwndJabberAgents, WM_JABBER_AGENT_REFRESH, 0, ( LPARAM )"" );
				SendMessage( hwndJabberAgents, WM_JABBER_TRANSPORT_REFRESH, 0, 0 );
			}
			if ( hwndJabberVcard )
				SendMessage( hwndJabberVcard, WM_JABBER_CHECK_ONLINE, 0, 0 );
		}
		else if ( info->type==JABBER_SESSION_REGISTER && !info->reg_done ) {
			SendMessage( info->reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, ( LPARAM )JTranslate( "Error: Connection lost" ));
	}	}
	else {
		if ( info->type == JABBER_SESSION_NORMAL ) {
			oldStatus = jabberStatus;
			jabberStatus = ID_STATUS_OFFLINE;
			JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );
	}	}

	Netlib_CloseHandle( info->s );

	if ( sslMode ) {
		pfn_SSL_free( ssl );
		JabberSslRemoveHandle( info->s );
	}

	JabberLog( "Thread ended: type=%d server='%s'", info->type, info->server );

	if ( info->type==JABBER_SESSION_NORMAL && jabberThreadInfo==info ) {
		if ( streamId ) free( streamId );
		streamId = NULL;
		jabberThreadInfo = NULL;
	}

	free( buffer );
	JabberLog( "Exiting ServerThread" );
	goto LBL_Exit;
}

static void JabberIqProcessSearch( XmlNode *node, void *userdata )
{
}

static void JabberProcessStreamOpening( XmlNode *node, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	char* sid;

	if ( node->name==NULL || strcmp( node->name, "stream:stream" ))
		return;

	if ( !info->useSSL && 
#ifndef STATICSSL
		hLibSSL != NULL && 
#endif
		JGetByte( "UseTLS", TRUE )) {
		JabberLog( "Requesting TLS" );
		JabberSend( info->s, "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>" );
		return;
	}

	if ( info->type == JABBER_SESSION_NORMAL ) {
		if (( sid=JabberXmlGetAttrValue( node, "id" )) != NULL ) {
			if ( streamId ) free( streamId );
			streamId = _strdup( sid );
		}

		int iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetAuth );
		JabberSend( info->s, "<iq type='get' id='"JABBER_IQID"%d'><query xmlns='jabber:iq:auth'><username>%s</username></query></iq>", iqId, TXT(info->username));
	}
	else if ( info->type == JABBER_SESSION_REGISTER ) {
		iqIdRegGetReg = JabberSerialNext();
		JabberSend( info->s, "<iq type='get' id='"JABBER_IQID"%d' to='%s'><query xmlns='jabber:iq:register'/></iq>", iqIdRegGetReg, TXT(info->server));
		SendMessage( info->reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 50, ( LPARAM )JTranslate( "Requesting registration instruction..." ));
	}
	else JabberSend( info->s, "</stream:stream>" );
}

static void JabberProcessStreamClosing( XmlNode *node, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;

	Netlib_CloseHandle( info->s );
	if ( node->name && !strcmp( node->name, "stream:error" ) && node->text )
		MessageBoxA( NULL, JTranslate( node->text ), JTranslate( "Jabber Connection Error" ), MB_OK|MB_ICONERROR|MB_SETFOREGROUND );
}

static void JabberProcessProtocol( XmlNode *node, void *userdata )
{
	struct ThreadData *info;

	//JabberXmlDumpNode( node );

	info = ( struct ThreadData * ) userdata;

	if ( info->type == JABBER_SESSION_NORMAL ) {
		if ( !strcmp( node->name, "message" ))
			JabberProcessMessage( node, userdata );
		else if ( !strcmp( node->name, "presence" ))
			JabberProcessPresence( node, userdata );
		else if ( !strcmp( node->name, "iq" ))
			JabberProcessIq( node, userdata );
		else if ( !strcmp( node->name, "proceed" ))
			JabberProcessProceed( node, userdata );
		else
			JabberLog( "Invalid top-level tag ( only <message/> <presence/> and <iq/> allowed )" );
	}
	else if ( info->type == JABBER_SESSION_REGISTER ) {
		if ( !strcmp( node->name, "iq" ))
			JabberProcessRegIq( node, userdata );
		else if ( !strcmp( node->name, "proceed"))
			JabberProcessProceed( node, userdata );
		else
			JabberLog( "Invalid top-level tag ( only <iq/> allowed )" );
}	}

static void JabberProcessProceed( XmlNode *node, void *userdata )
{
	struct ThreadData *info;
	char* type;
	node = node;
	if (( info=( struct ThreadData * ) userdata ) == NULL ) return;
	if (( type = JabberXmlGetAttrValue( node, "xmlns" )) != NULL && !strcmp( type, "error" ))
		return;
	if ( !strcmp( type, "urn:ietf:params:xml:ns:xmpp-tls" )){
		JabberLog("Staring TLS...");
		int socket = JCallService( MS_NETLIB_GETSOCKET, ( WPARAM ) info->s, 0 );
		PVOID ssl;
		if (( ssl=pfn_SSL_new( jabberSslCtx )) != NULL ) {
			JabberLog( "SSL create context ok" );
			if ( pfn_SSL_set_fd( ssl, socket ) > 0 ) {
				JabberLog( "SSL set fd ok" );
				if ( pfn_SSL_connect( ssl ) > 0 ) {
					JabberLog( "SSL negotiation ok" );
					JabberSslAddHandle( info->s, ssl );	// This make all communication on this handle use SSL
					info->useSSL = true;
					JabberLog( "SSL enabled for handle = %d", info->s );
				}
				else {
					JabberLog( "SSL negotiation failed" );
					pfn_SSL_free( ssl );
			}	}
			else {
				JabberLog( "SSL set fd failed" );
				pfn_SSL_free( ssl );
}	}	}	}

static void JabberProcessMessage( XmlNode *node, void *userdata )
{
	struct ThreadData *info;
	XmlNode *bodyNode, *subjectNode, *xNode, *inviteNode, *idNode, *n;
	char* from, *type, *nick, *p, *idStr, *fromResource;
	int id;

	if ( !node->name || strcmp( node->name, "message" )) return;
	if (( info=( struct ThreadData * ) userdata ) == NULL ) return;

	if (( type = JabberXmlGetAttrValue( node, "type" )) != NULL && !strcmp( type, "error" ))
		return;
	if (( from = JabberXmlGetAttrValue( node, "from" )) == NULL )
		return;

	BOOL isChatRoomJid = JabberListExist( LIST_CHATROOM, JabberUrlDecode( from ));
	if ( isChatRoomJid && type != NULL && !strcmp( type, "groupchat" )) {
		JabberGroupchatProcessMessage( node, userdata );
		return;
	}

	// If message is from a stranger ( not in roster ), item is NULL
	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, from );
	if (( bodyNode = JabberXmlGetChild( node, "body" )) != NULL ) {
		if ( bodyNode->text == NULL )
			return;

		WCHAR* wszMessage;
		char*  szMessage;
		BOOL isRss = !strcmp( type, "headline" );

		if (( subjectNode=JabberXmlGetChild( node, "subject" ))!=NULL && subjectNode->text!=NULL && subjectNode->text[0]!='\0' && !isRss ) {
			p = ( char* )alloca( strlen( subjectNode->text ) + strlen( bodyNode->text ) + 12 );
			sprintf( p, "Subject: %s\r\n%s", subjectNode->text, bodyNode->text );
			szMessage = p;
		}
		else szMessage = bodyNode->text;

		time_t msgTime = 0, now;
		BOOL  isChatRoomInvitation = FALSE;
		char* inviteRoomJid = NULL;
		char* inviteFromJid = NULL;
		char* inviteReason = NULL;
		char* invitePassword = NULL;
		BOOL delivered = FALSE, composing = FALSE;

		for ( int i = 1; ( xNode = JabberXmlGetNthChild( node, "x", i )) != NULL; i++ ) {
			if (( p=JabberXmlGetAttrValue( xNode, "xmlns" )) != NULL ) {
				if ( !strcmp( p, "jabber:x:encrypted" ) ) {
					if ( xNode->text == NULL )
						return;
					char* prolog = JabberUtf8Encode("-----BEGIN PGP MESSAGE-----\r\n\r\n");
					char* epilog = JabberUtf8Encode("\r\n-----END PGP MESSAGE-----\r\n");
					char* tempstring = ( char* )alloca( strlen( prolog ) + strlen( xNode->text ) + strlen( epilog ) );
					strncpy( tempstring, prolog, strlen( prolog )+1 );
					strncpy(tempstring+strlen( prolog ), xNode->text, strlen( xNode->text )+1);
					strncpy(tempstring+strlen( prolog )+strlen(xNode->text ), epilog, strlen( epilog )+1);
					szMessage = tempstring;
            }
				else if ( !strcmp( p, "jabber:x:delay" ) && msgTime == 0 ) {
					if (( p=JabberXmlGetAttrValue( xNode, "stamp" )) != NULL )
						msgTime = JabberIsoToUnixTime( p );
				}
				else if ( !strcmp( p, "jabber:x:event" )) {
					// Check whether any event is requested
					if ( !delivered && ( n=JabberXmlGetChild( xNode, "delivered" ))!=NULL ) {
						delivered = TRUE;
						idStr = JabberXmlGetAttrValue( node, "id" );
						JabberSend( info->s, "<message to='%s'><x xmlns='jabber:x:event'><delivered/><id>%s</id></x></message>", from, ( idStr!=NULL )?idStr:"" );
					}
					if ( item!=NULL && JabberXmlGetChild( xNode, "composing" )!=NULL ) {
						composing = TRUE;
						if ( item->messageEventIdStr )
							free( item->messageEventIdStr );
						idStr = JabberXmlGetAttrValue( node, "id" );
						item->messageEventIdStr = ( idStr==NULL )?NULL:_strdup( idStr );
					}
				}
				else if ( !strcmp( p, "jabber:x:oob" ) && isRss) {
					XmlNode* rssUrlNode;
					if ( (rssUrlNode = JabberXmlGetNthChild( xNode, "url", 1 ))!=NULL) {
						p = ( char* )alloca( strlen( subjectNode->text ) + strlen( bodyNode->text ) + strlen( rssUrlNode->text ) + 14 );
						sprintf( p, "Subject: %s\r\n%s\r\n%s", subjectNode->text, rssUrlNode->text, bodyNode->text );
						szMessage = p;
					}
				}
				else if ( !strcmp( p, "http://jabber.org/protocol/muc#user" )) {
					if (( inviteNode=JabberXmlGetChild( xNode, "invite" )) != NULL ) {
						inviteFromJid = JabberXmlGetAttrValue( inviteNode, "from" );
						if (( n=JabberXmlGetChild( inviteNode, "reason" )) != NULL )
							inviteReason = n->text;
					}

					if (( n=JabberXmlGetChild( xNode, "password" )) != NULL )
						invitePassword = n->text;
				}
				else if ( !strcmp( p, "jabber:x:conference" )) {
					inviteRoomJid = JabberXmlGetAttrValue( xNode, "jid" );
					if ( inviteReason == NULL )
						inviteReason = xNode->text;
					isChatRoomInvitation = TRUE;
		}	}	}

		JabberUtf8Decode( szMessage, &wszMessage );
		JabberUrlDecode( szMessage );
		if (( szMessage = JabberUnixToDos( szMessage )) == NULL )
			szMessage = "";
		{
			JabberUrlDecodeW( wszMessage );
			WCHAR* p = JabberUnixToDosW( wszMessage );
			free( wszMessage );
			wszMessage = ( p == NULL ) ? (WCHAR*)L"" : p;
		}

		int cbAnsiLen = strlen( szMessage )+1, cbWideLen = wcslen( wszMessage )+1;
		char* buf = ( char* )alloca( cbAnsiLen + cbWideLen*sizeof( WCHAR ));
		memcpy( buf, szMessage, cbAnsiLen );
		memcpy( buf + cbAnsiLen, wszMessage, cbWideLen*sizeof( WCHAR ));

		if ( isChatRoomInvitation ) {
			if ( inviteRoomJid != NULL )
				JabberGroupchatProcessInvite( inviteRoomJid, inviteFromJid, inviteReason, invitePassword );
		}
		else {
			HANDLE hContact = JabberHContactFromJID( from );

			if ( item != NULL ) {
				item->wantComposingEvent = composing;
				if ( hContact != NULL )
					JCallService( MS_PROTO_CONTACTISTYPING, ( WPARAM ) hContact, PROTOTYPE_CONTACTTYPING_OFF );

				if ( item->resourceMode==RSMODE_LASTSEEN && ( fromResource=strchr( from, '/' ))!=NULL ) {
					fromResource++;
					if ( *fromResource != '\0' ) {
						for ( int i=0; i<item->resourceCount; i++ ) {
							if ( !strcmp( item->resource[i].resourceName, fromResource )) {
								item->defaultResource = i;
								break;
			}	}	}	}	}

			if ( hContact == NULL ) {
				// Create a temporary contact
				if ( isChatRoomJid ) {
					if (( p=strchr( from, '/' ))!=NULL && p[1]!='\0' )
						p++;
					else
						p = from;
					nick = JabberTextEncode( p );
					hContact = JabberDBCreateContact( from, nick, TRUE, FALSE );
				}
				else {
					nick = JabberNickFromJID( from );
					hContact = JabberDBCreateContact( from, nick, TRUE, TRUE );
				}
				free( nick );
			}

			now = time( NULL );
			if ( msgTime==0 || msgTime > now )
				msgTime = now;

			PROTORECVEVENT recv;
			recv.flags = PREF_UNICODE;
			recv.timestamp = ( DWORD )msgTime;
			recv.szMessage = buf;
			recv.lParam = 0;

			CCSDATA ccs;
			ccs.hContact = hContact;
			ccs.wParam = 0;
			ccs.szProtoService = PSR_MESSAGE;
			ccs.lParam = ( LPARAM )&recv;
			JCallService( MS_PROTO_CHAINRECV, 0, ( LPARAM )&ccs );
		}

		free( szMessage );
		free( wszMessage );
	}
	else {	// bodyNode==NULL - check for message event notification ( ack, composing )
		if (( xNode=JabberXmlGetChild( node, "x" )) != NULL ) {
			if (( p=JabberXmlGetAttrValue( xNode, "xmlns" ))!=NULL && !strcmp( p, "jabber:x:event" )) {
				idNode = JabberXmlGetChild( xNode, "id" );
				if ( JabberXmlGetChild( xNode, "delivered" )!=NULL ||
					JabberXmlGetChild( xNode, "offline" )!=NULL ) {

					id = -1;
					if ( idNode!=NULL && idNode->text!=NULL )
						if ( !strncmp( idNode->text, JABBER_IQID, strlen( JABBER_IQID )) )
							id = atoi(( idNode->text )+strlen( JABBER_IQID ));

					if ( id == item->idMsgAckPending )
						JSendBroadcast( JabberHContactFromJID( from ), ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, ( HANDLE ) 1, 0 );
				}

				HANDLE hContact;
				if ( JabberXmlGetChild( xNode, "composing" ) != NULL )
					if (( hContact = JabberHContactFromJID( from )) != NULL )
 						JCallService( MS_PROTO_CONTACTISTYPING, ( WPARAM ) hContact, 60 );

				if ( xNode->numChild==0 || ( xNode->numChild==1 && idNode!=NULL ))
					// Maybe a cancel to the previous composing
					if (( hContact = JabberHContactFromJID( from )) != NULL )
						JCallService( MS_PROTO_CONTACTISTYPING, ( WPARAM ) hContact, PROTOTYPE_CONTACTTYPING_OFF );
}	}	}	}

static void JabberProcessPresence( XmlNode *node, void *userdata )
{
	struct ThreadData *info;
	HANDLE hContact;
	XmlNode *showNode, *statusNode;
	JABBER_LIST_ITEM *item;
	char* from, *nick, *show;
	int i;
	char* p;

	if ( !node || !node->name || strcmp( node->name, "presence" )) return;
	if (( info=( struct ThreadData * ) userdata ) == NULL ) return;
	if (( from = JabberXmlGetAttrValue( node, "from" )) == NULL ) return;

	JabberUrlDecode( from );
	if ( JabberListExist( LIST_CHATROOM, from )) {
		JabberGroupchatProcessPresence( node, userdata );
		return;
	}

	char* type = JabberXmlGetAttrValue( node, "type" );
	if ( type == NULL || !strcmp( type, "available" )) {
		if (( nick=JabberNickFromJID( from )) == NULL )
			return;
		if (( hContact = JabberHContactFromJID( from )) == NULL )
			hContact = JabberDBCreateContact( from, nick, FALSE, TRUE );
		if ( !JabberListExist( LIST_ROSTER, from )) {
			JabberLog( "Receive presence online from %s ( who is not in my roster )", from );
			JabberListAdd( LIST_ROSTER, from );
		}
		int status = ID_STATUS_ONLINE;
		if (( showNode = JabberXmlGetChild( node, "show" )) != NULL ) {
			if (( show = showNode->text ) != NULL ) {
				if ( !strcmp( show, "away" )) status = ID_STATUS_AWAY;
				else if ( !strcmp( show, "xa" )) status = ID_STATUS_NA;
				else if ( !strcmp( show, "dnd" )) status = ID_STATUS_DND;
				else if ( !strcmp( show, "chat" )) status = ID_STATUS_FREECHAT;
		}	}

		// Send version query if this is the new resource
		if (( p=strchr( from, '@' )) != NULL ) {
			if (( p=strchr( p, '/' ))!=NULL && p[1]!='\0' ) {
				p++;
				if (( item = JabberListGetItemPtr( LIST_ROSTER, from )) != NULL ) {
					JABBER_RESOURCE_STATUS *r = item->resource;
					for ( i=0; i < item->resourceCount && strcmp( r->resourceName, p ); i++, r++ );
					if ( i >= item->resourceCount || ( r->version == NULL && r->system == NULL && r->software == NULL ))
						JabberSend( info->s, "<iq type='get' to='%s'><query xmlns='jabber:iq:version'/></iq>", from );
		}	}	}

		if (( statusNode = JabberXmlGetChild( node, "status" )) != NULL && statusNode->text != NULL )
			p = JabberTextDecode( statusNode->text );
		else 
			p = NULL;
		JabberListAddResource( LIST_ROSTER, from, status, p );
		if ( p ) {
			DBWriteContactSettingString( hContact, "CList", "StatusMsg", p );
			free( p );
		}
		else DBDeleteContactSetting( hContact, "CList", "StatusMsg" );

		// Determine status to show for the contact
		if (( item=JabberListGetItemPtr( LIST_ROSTER, from )) != NULL ) {
			for ( i=0; i < item->resourceCount; i++ )
				status = JabberCombineStatus( status, item->resource[i].status );
			item->status = status;
		}

		if ( strchr( from, '@' )!=NULL || JGetByte( "ShowTransport", TRUE )==TRUE )
			if ( JGetWord( hContact, "Status", ID_STATUS_OFFLINE ) != status )
				JSetWord( hContact, "Status", ( WORD )status );

		if ( strchr( from, '@' )==NULL && hwndJabberAgents )
			SendMessage( hwndJabberAgents, WM_JABBER_TRANSPORT_REFRESH, 0, 0 );
		JabberLog( "%s ( %s ) online, set contact status to %d", nick, from, status );
		free( nick );

		XmlNode* xNode;
		for ( int i = 1; ( xNode=JabberXmlGetNthChild( node, "x", i )) != NULL; i++ ) {
         if ( !lstrcmpA( JabberXmlGetAttrValue( xNode, "xmlns" ), "jabber:x:avatar" )) {
				if (( xNode = JabberXmlGetChild( xNode, "hash" )) != NULL && xNode->text != NULL && JGetByte( "EnableAvatars", TRUE )) {
					JSetString( hContact, "AvatarHash", xNode->text );

					char szSavedHash[ 100 ];
					int result = JGetStaticString( "AvatarSaved", hContact, szSavedHash, sizeof szSavedHash );
					if ( result || strcmp( szSavedHash, xNode->text )) {
						JabberLog( "Avatar was changed" );
						JSendBroadcast( hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, NULL );
		}	}	}	}
		return;
	}

	if ( !strcmp( type, "unavailable" )) {
		if ( !JabberListExist( LIST_ROSTER, from )) {
			JabberLog( "Receive presence offline from %s ( who is not in my roster )", from );
			JabberListAdd( LIST_ROSTER, from );
		}
		else JabberListRemoveResource( LIST_ROSTER, from );

		int status = ID_STATUS_OFFLINE;
		if (( statusNode = JabberXmlGetChild( node, "status" )) != NULL ) {
			if ( JGetByte( "OfflineAsInvisible", FALSE ) == TRUE )
				status = ID_STATUS_INVISIBLE;

			p = JabberTextDecode( statusNode->text );
			if (( hContact = JabberHContactFromJID( from )) != NULL) {
				if ( p )
					DBWriteContactSettingString(hContact, "CList", "StatusMsg", p);
				else 
					DBDeleteContactSetting(hContact, "CList", "StatusMsg");
			}
			if (p) free(p);
		}
		if (( item=JabberListGetItemPtr( LIST_ROSTER, from )) != NULL ) {
			// Determine status to show for the contact based on the remaining resources
			status = ID_STATUS_OFFLINE;
			for ( i=0; i < item->resourceCount; i++ )
				status = JabberCombineStatus( status, item->resource[i].status );
			item->status = status;
		}
		if (( hContact=JabberHContactFromJID( from )) != NULL ) {
			if ( strchr( from, '@' )!=NULL || JGetByte( "ShowTransport", TRUE )==TRUE )
				if ( JGetWord( hContact, "Status", ID_STATUS_OFFLINE ) != status )
					JSetWord( hContact, "Status", ( WORD )status );

			JabberLog( "%s offline, set contact status to %d", from, status );
		}
		if ( strchr( from, '@' )==NULL && hwndJabberAgents )
			SendMessage( hwndJabberAgents, WM_JABBER_TRANSPORT_REFRESH, 0, 0 );
		return;
	}

	if ( !strcmp( type, "subscribe" )) {
		if ( strchr( from, '@' ) == NULL ) {
			// automatically send authorization allowed to agent/transport
			JabberSend( info->s, "<presence to='%s' type='subscribed'/>", from );
		}
		else if (( nick=JabberNickFromJID( from )) != NULL ) {
			JabberLog( "%s ( %s ) requests authorization", nick, from );
			JabberDBAddAuthRequest( from, nick );
			free( nick );
		}
		return;
	}

	if ( !strcmp( type, "subscribed" )) {
		if (( item=JabberListGetItemPtr( LIST_ROSTER, from )) != NULL ) {
			if ( item->subscription == SUB_FROM ) item->subscription = SUB_BOTH;
			else if ( item->subscription == SUB_NONE ) {
				item->subscription = SUB_TO;
				if ( hwndJabberAgents && strchr( from, '@' )==NULL )
					SendMessage( hwndJabberAgents, WM_JABBER_TRANSPORT_REFRESH, 0, 0 );
}	}	}	}

/////////////////////////////////////////////////////////////////////////////////////////
// Handles various <iq... requests

static void JabberProcessIqVersion( char* idStr, XmlNode* node )
{
	char* from, *resultId;
	if (( from=JabberXmlGetAttrValue( node, "from" )) == NULL )
		return;

	char* str = JabberGetVersionText();
	char* version = JabberTextEncode( str );
	char* os = NULL;

	OSVERSIONINFO osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	if ( GetVersionEx( &osvi )) {
		switch ( osvi.dwPlatformId ) {
		case VER_PLATFORM_WIN32_NT:
			if ( osvi.dwMajorVersion == 5 ) {
				if ( osvi.dwMinorVersion == 2 ) os = JabberTextEncode( JTranslate( "Windows Server 2003" ));
				else if ( osvi.dwMinorVersion == 1 ) os = JabberTextEncode( JTranslate( "Windows XP" ));
				else if ( osvi.dwMinorVersion == 0 ) os = JabberTextEncode( JTranslate( "Windows 2000" ));
			}
			else if ( osvi.dwMajorVersion <= 4 ) {
				os = JabberTextEncode( JTranslate( "Windows NT" ));
			}
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			if ( osvi.dwMajorVersion == 4 ) {
				if ( osvi.dwMinorVersion == 0 ) os = JabberTextEncode( JTranslate( "Windows 95" ));
				if ( osvi.dwMinorVersion == 10 ) os = JabberTextEncode( JTranslate( "Windows 98" ));
				if ( osvi.dwMinorVersion == 90 ) os = JabberTextEncode( JTranslate( "Windows ME" ));
			}
			break;
	}	}

	if ( os == NULL ) os = JabberTextEncode( JTranslate( "Windows" ));

	char mversion[64];
	JCallService( MS_SYSTEM_GETVERSIONTEXT, sizeof( mversion ), ( LPARAM )mversion );
	if (( resultId = JabberTextEncode( idStr )) != NULL ) {
		JabberSend( jabberThreadInfo->s, "<iq type='result' id='%s' to='%s'><query xmlns='jabber:iq:version'><name>Jabber Protocol Plugin ( Miranda IM %s )</name><version>%s</version><os>%s</os></query></iq>", resultId, from, mversion, version?version:"", os?os:"" );
		free( resultId );
	}
	else JabberSend( jabberThreadInfo->s, "<iq type='result' to='%s'><query xmlns='jabber:iq:version'><name>Jabber Protocol Plugin ( Miranda IM %s )</name><version>%s</version><os>%s</os></query></iq>", from, mversion, version?version:"", os?os:"" );

	if ( str ) free( str );
	if ( version ) free( version );
	if ( os ) free( os );
}

static void JabberProcessIqAvatar( char* idStr, XmlNode* node )
{
	if ( !JGetByte( "EnableAvatars", TRUE ))
		return;

	char* from;
	if (( from = JabberXmlGetAttrValue( node, "from" )) == NULL )
		return;

	int pictureType = JGetByte( "AvatarType", PA_FORMAT_UNKNOWN );
	if ( pictureType == PA_FORMAT_UNKNOWN )
		return;

	char* szMimeType;
	switch( pictureType ) {
		case PA_FORMAT_JPEG:	 szMimeType = "image/jpeg";   break;
		case PA_FORMAT_GIF:	 szMimeType = "image/gif";    break;
		case PA_FORMAT_PNG:	 szMimeType = "image/png";    break;
		case PA_FORMAT_BMP:	 szMimeType = "image/bmp";    break;
		default:	return;
	}

	char szFileName[ MAX_PATH ];
	JabberGetAvatarFileName( NULL, szFileName, MAX_PATH );

	FILE* in = fopen( szFileName, "rb" );
	if ( in == NULL )
		return;

	long bytes = filelength( fileno( in ));
	char* buffer = ( char* )malloc( bytes*4/3 + bytes + 1000 );
	if ( buffer == NULL ) {
		fclose( in );
		return;
	}

	fread( buffer, bytes, 1, in );
	fclose( in );

	char* str = JabberBase64Encode( buffer, bytes );
	char* resultId = JabberTextEncode( idStr );
	JabberSend( jabberThreadInfo->s,
		"<iq type='result' id='%s' to='%s'><query xmlns='jabber:iq:avatar'><data mimetype='%s'>%s</data></query></iq>",
		resultId, from, szMimeType, str );
	free( resultId );
	free( str );
	free( buffer );
}

static void JabberProcessIq( XmlNode *node, void *userdata )
{
	struct ThreadData *info;
	HANDLE hContact;
	XmlNode *queryNode, *siNode, *n, *newMailNode;
	char* from, *type, *jid, *nick;
	char* xmlns, *profile;
	char* idStr, *str, *p, *q;
	char text[256];
	int id;
	int i;
	JABBER_IQ_PFUNC pfunc;

	if ( !node->name || strcmp( node->name, "iq" )) return;
	if (( info=( struct ThreadData * ) userdata ) == NULL ) return;
	if (( type=JabberXmlGetAttrValue( node, "type" )) == NULL ) return;

	id = -1;
	if (( idStr=JabberXmlGetAttrValue( node, "id" )) != NULL )
		if ( !strncmp( idStr, JABBER_IQID, strlen( JABBER_IQID )) )
			id = atoi( idStr+strlen( JABBER_IQID ));

	queryNode = JabberXmlGetChild( node, "query" );
	xmlns = JabberXmlGetAttrValue( queryNode, "xmlns" );

	/////////////////////////////////////////////////////////////////////////
	// MATCH BY ID
	/////////////////////////////////////////////////////////////////////////

	if (( pfunc=JabberIqFetchFunc( id )) != NULL ) {
		JabberLog( "Handling iq request for id=%d", id );
		pfunc( node, userdata );
	}

	/////////////////////////////////////////////////////////////////////////
	// MORE GENERAL ROUTINES, WHEN ID DOES NOT MATCH
	/////////////////////////////////////////////////////////////////////////

	else if (( pfunc=JabberIqFetchXmlnsFunc( xmlns )) != NULL ) {
		JabberLog( "Handling iq request for xmlns=%s", xmlns );
		pfunc( node, userdata );
	}

	// RECVED: <iq type='set'><query ...
	else if ( !strcmp( type, "set" ) && queryNode!=NULL && ( xmlns=JabberXmlGetAttrValue( queryNode, "xmlns" ))!=NULL ) {

		// RECVED: roster push
		// ACTION: similar to iqIdGetRoster above
		if ( !strcmp( xmlns, "jabber:iq:roster" )) {
			XmlNode *itemNode, *groupNode;
			JABBER_LIST_ITEM *item;
			char* name;

			JabberLog( "<iq/> Got roster push, query has %d children", queryNode->numChild );
			for ( i=0; i<queryNode->numChild; i++ ) {
				itemNode = queryNode->child[i];
				if ( strcmp( itemNode->name, "item" ) != 0 )
					continue;
				if (( jid = JabberXmlGetAttrValue( itemNode, "jid" )) == NULL )
					continue;
				if (( str = JabberXmlGetAttrValue( itemNode, "subscription" )) == NULL )
					continue;

				JabberUrlDecode( jid );

				// we will not add new account when subscription=remove
				if ( !strcmp( str, "to" ) || !strcmp( str, "both" ) || !strcmp( str, "from" ) || !strcmp( str, "none" )) {
					if (( name=JabberXmlGetAttrValue( itemNode, "name" )) != NULL )
						nick = strdup( JabberUrlDecode( name ));
					else
						nick = JabberNickFromJID( jid );

					if ( nick != NULL ) {
						if (( item=JabberListAdd( LIST_ROSTER, jid )) != NULL ) {
							if ( item->nick ) free( item->nick );
							item->nick = nick;

							if ( item->group ) free( item->group );
							if (( groupNode=JabberXmlGetChild( itemNode, "group" ))!=NULL && groupNode->text!=NULL )
								item->group = strdup( JabberUrlDecode( groupNode->text ));
							else
								item->group = NULL;

							if (( hContact=JabberHContactFromJID( jid )) == NULL ) {
								// Received roster has a new JID.
								// Add the jid ( with empty resource ) to Miranda contact list.
								hContact = JabberDBCreateContact( jid, nick, FALSE, TRUE );
							}
							else JSetStringUtf( hContact, "jid", jid );

                     DBVARIANT dbnick;
							if ( !JGetStringUtf( hContact, "Nick", &dbnick )) {
								if ( strcmp( nick, dbnick.pszVal ) != 0 )
									DBWriteContactSettingStringUtf( hContact, "CList", "MyHandle", nick );
								else
									DBDeleteContactSetting( hContact, "CList", "MyHandle" );
								JFreeVariant( &dbnick );
							}
							else DBWriteContactSettingStringUtf( hContact, "CList", "MyHandle", nick );

							if ( item->group != NULL ) {
								JabberContactListCreateGroup( item->group );
								DBWriteContactSettingStringUtf( hContact, "CList", "Group", item->group );
							}
							else DBDeleteContactSetting( hContact, "CList", "Group" );

							if ( !strcmp( str, "none" ) || ( !strcmp( str, "from" ) && strchr( jid, '@' )!=NULL ))
								if ( JGetWord( hContact, "Status", ID_STATUS_OFFLINE ) != ID_STATUS_OFFLINE )
									JSetWord( hContact, "Status", ID_STATUS_OFFLINE );
						}
						else free( nick );
				}	}

				if (( item=JabberListGetItemPtr( LIST_ROSTER, jid )) != NULL ) {
					if ( !strcmp( str, "both" )) item->subscription = SUB_BOTH;
					else if ( !strcmp( str, "to" )) item->subscription = SUB_TO;
					else if ( !strcmp( str, "from" )) item->subscription = SUB_FROM;
					else item->subscription = SUB_NONE;
					JabberLog( "Roster push for jid=%s, set subscription to %s", jid, str );
					// subscription = remove is to remove from roster list
					// but we will just set the contact to offline and not actually
					// remove, so that history will be retained.
					if ( !strcmp( str, "remove" )) {
						if (( hContact=JabberHContactFromJID( jid )) != NULL ) {
							if ( JGetWord( hContact, "Status", ID_STATUS_OFFLINE ) != ID_STATUS_OFFLINE )
								JSetWord( hContact, "Status", ID_STATUS_OFFLINE );
							JabberListRemove( LIST_ROSTER, jid );
					}	}
					else if ( JGetByte( hContact, "ChatRoom", 0 ))
						DBDeleteContactSetting( hContact, "CList", "Hidden" );
			}	}

			if ( hwndJabberAgents )
				SendMessage( hwndJabberAgents, WM_JABBER_TRANSPORT_REFRESH, 0, 0 );
		}

		// RECVED: file transfer request
		// ACTION: notify Miranda throuch CHAINRECV
		else if ( !strcmp( xmlns, "jabber:iq:oob" )) {
			if (( jid=JabberXmlGetAttrValue( node, "from" ))!=NULL && ( n=JabberXmlGetChild( queryNode, "url" ))!=NULL && n->text!=NULL ) {
				str = n->text;	// URL of the file to get
				filetransfer* ft = new filetransfer;
				ft->std.totalFiles = 1;
				ft->jid = _strdup( JabberUrlDecode( jid ));
				ft->std.hContact = JabberHContactFromJID( jid );
				ft->type = FT_OOB;
				ft->httpHostName = NULL;
				ft->httpPort = 80;
				ft->httpPath = NULL;
				// Parse the URL
				if ( !_strnicmp( str, "http://", 7 )) {
					p = str + 7;
					if (( q=strchr( p, '/' )) != NULL ) {
						if ( q-p < sizeof( text )) {
							strncpy( text, p, q-p );
							text[q-p] = '\0';
							if (( p=strchr( text, ':' )) != NULL ) {
								ft->httpPort = ( WORD )atoi( p+1 );
								*p = '\0';
							}
							ft->httpHostName = _strdup( text );
							q++;
							ft->httpPath = _strdup( q );
				}	}	}

				if (( str=JabberXmlGetAttrValue( node, "id" )) != NULL )
					ft->iqId = _strdup( str );

				if ( ft->httpHostName && ft->httpPath ) {
					CCSDATA ccs;
					PROTORECVEVENT pre;
					char* szBlob, *desc;

					JabberLog( "Host=%s Port=%d Path=%s", ft->httpHostName, ft->httpPort, ft->httpPath );
					if (( n=JabberXmlGetChild( queryNode, "desc" ))!=NULL && n->text!=NULL )
						desc = JabberTextDecode( n->text );
					else
						desc = _strdup( "" );

					if ( desc != NULL ) {
						JabberLog( "description = %s", desc );
						if (( str=strrchr( ft->httpPath, '/' )) != NULL )
							str++;
						else
							str = ft->httpPath;
						str = _strdup( str );
						JabberHttpUrlDecode( str );
						szBlob = ( char* )malloc( sizeof( DWORD )+ strlen( str ) + strlen( desc ) + 2 );
						*(( PDWORD ) szBlob ) = ( DWORD )ft;
						strcpy( szBlob + sizeof( DWORD ), str );
						strcpy( szBlob + sizeof( DWORD )+ strlen( str ) + 1, desc );
						pre.flags = 0;
						pre.timestamp = time( NULL );
						pre.szMessage = szBlob;
						pre.lParam = 0;
						ccs.szProtoService = PSR_FILE;
						ccs.hContact = ft->std.hContact;
						ccs.wParam = 0;
						ccs.lParam = ( LPARAM )&pre;
						JCallService( MS_PROTO_CHAINRECV, 0, ( LPARAM )&ccs );
						free( szBlob );
						free( str );
						free( desc );
					}
				}
				else {
					// reject
					if ( ft->iqId )
						JabberSend( jabberThreadInfo->s, "<iq type='error' to='%s' id='%s'><error code='406'>File transfer refused</error></iq>", ft->jid, ft->iqId );
					else
						JabberSend( jabberThreadInfo->s, "<iq type='error' to='%s'><error code='406'>File transfer refused</error></iq>", ft->jid );
					delete ft;
		}	}	}

		// RECVED: bytestream initiation request
		// ACTION: check for any stream negotiation that is pending ( now only file transfer is handled )
		else if ( !strcmp( xmlns, "http://jabber.org/protocol/bytestreams" ))
			JabberFtHandleBytestreamRequest( node );
	}
	// RECVED: <iq type='get'><query ...
	else if ( !strcmp( type, "get" ) && queryNode!=NULL && ( xmlns=JabberXmlGetAttrValue( queryNode, "xmlns" ))!=NULL ) {

		// RECVED: software version query
		// ACTION: return my software version
		if ( !strcmp( xmlns, "jabber:iq:version" ))
			JabberProcessIqVersion( idStr, node );
		else if ( !strcmp( xmlns, "jabber:iq:avatar" ))
			JabberProcessIqAvatar( idStr, node );
	}
	// RECVED: <iq type='result'><query ...
	else if ( !strcmp( type, "result" ) && queryNode!=NULL && ( xmlns=JabberXmlGetAttrValue( queryNode, "xmlns" ))!=NULL ) {

		// RECVED: software version result
		// ACTION: update version information for the specified jid/resource
		if ( !strcmp( xmlns, "jabber:iq:version" )) {
			char* from;
			JABBER_LIST_ITEM *item;
			JABBER_RESOURCE_STATUS *r;

			if (( from=JabberXmlGetAttrValue( node, "from" )) != NULL ) {
				if (( item=JabberListGetItemPtr( LIST_ROSTER, from ))!=NULL && ( r=item->resource )!=NULL ) {
					if (( p=strchr( from, '/' ))!=NULL && p[1]!='\0' ) {
						p++;
						for ( i=0; i<item->resourceCount && strcmp( r->resourceName, p ); i++, r++ );
						if ( i < item->resourceCount ) {
							if ( r->software ) free( r->software );
							if (( n=JabberXmlGetChild( queryNode, "name" ))!=NULL && n->text ) {
								if (( hContact=JabberHContactFromJID( item->jid )) != NULL ) {
									if (( p = strstr( n->text, "Miranda IM" )) != NULL )
										JSetStringUtf( hContact, "MirVer", p );
									else
										JSetStringUtf( hContact, "MirVer", n->text );
								}
								r->software = JabberTextDecode( n->text );
							}
							else r->software = NULL;
							if ( r->version ) free( r->version );
							if (( n=JabberXmlGetChild( queryNode, "version" ))!=NULL && n->text )
								r->version = JabberTextDecode( n->text );
							else
								r->version = NULL;
							if ( r->system ) free( r->system );
							if (( n=JabberXmlGetChild( queryNode, "os" ))!=NULL && n->text )
								r->system = JabberTextDecode( n->text );
							else
								r->system = NULL;
		}	}	}	}	}
	}
	// RECVED: <iq type='set'><si xmlns='http://jabber.org/protocol/si' ...
	else if ( !strcmp( type, "set" ) && ( siNode=JabberXmlGetChildWithGivenAttrValue( node, "si", "xmlns", "http://jabber.org/protocol/si" ))!=NULL && ( profile=JabberXmlGetAttrValue( siNode, "profile" ))!=NULL ) {

		// RECVED: file transfer request
		// ACTION: notify Miranda throuch CHAINRECV
		if ( !strcmp( profile, "http://jabber.org/protocol/si/profile/file-transfer" )) {
			JabberFtHandleSiRequest( node );
		}
		// RECVED: unknown profile
		// ACTION: reply with bad-profile
		else {
			if (( from=JabberXmlGetAttrValue( node, "from" )) != NULL ) {
				idStr = JabberXmlGetAttrValue( node, "id" );
				JabberSend( jabberThreadInfo->s, "<iq type='error' to='%s'%s%s%s><error code='400' type='cancel'><bad-request xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'/><bad-profile xmlns='http://jabber.org/protocol/si'/></error></iq>", from, ( idStr )?" id='":"", ( idStr )?idStr:"", ( idStr )?"'":"" );
		}	}
	}
	// RECVED: <iq type='set'><new-mail ' ...
	else if ( !strcmp( type, "set" ) && ( newMailNode=JabberXmlGetChild( node, "new-mail" ) ) ) {
		// RECVED: new-mail notify
		// ACTION: Reply & request 
		if (JGetByte(NULL,"EnableGMail",1) & 1) {
			idStr = JabberXmlGetAttrValue( node, "id" );
			JabberSend( jabberThreadInfo->s, "<iq type='result' id='%s'/>",idStr );
			JabberRequestMailBox( info->s );
	}	}
	// RECVED: <iq type='error'> ...
	else if ( !strcmp( type, "error" )) {

		JABBER_LIST_ITEM *item;

		JabberLog( "XXX on entry" );
		// Check for file transfer deny by comparing idStr with ft->iqId
		i = 0;
		while (( i=JabberListFindNext( LIST_FILE, i )) >= 0 ) {
			item = JabberListGetItemPtrFromIndex( i );
			if ( item->ft!=NULL && item->ft->state==FT_CONNECTING && !strcmp( idStr, item->ft->iqId )) {
				JabberLog( "Denying file sending request" );
				item->ft->state = FT_DENIED;
				if ( item->ft->hFileEvent != NULL )
					SetEvent( item->ft->hFileEvent );	// Simulate the termination of file server connection
			}
			i++;
}	}	}

static void JabberProcessRegIq( XmlNode *node, void *userdata )
{
	struct ThreadData *info;
	XmlNode *errorNode;
	char* type, *str;
	char text[256];
	char* p, *q;

	if ( !node->name || strcmp( node->name, "iq" )) return;
	if (( info=( struct ThreadData * ) userdata ) == NULL ) return;
	if (( type=JabberXmlGetAttrValue( node, "type" )) == NULL ) return;

	unsigned int id = -1;
	if (( str=JabberXmlGetAttrValue( node, "id" )) != NULL )
		if ( !strncmp( str, JABBER_IQID, strlen( JABBER_IQID )) )
			id = atoi( str+strlen( JABBER_IQID ));

	if ( !strcmp( type, "result" )) {

		// RECVED: result of the request for registration mechanism
		// ACTION: send account registration information
		if ( id == iqIdRegGetReg ) {
			if (( p=JabberTextEncode( info->username )) != NULL ) {
				if (( q=JabberTextEncode( info->password )) != NULL ) {
					wsprintfA( text, "<password>%s</password><username>%s</username>", q /*info->password*/, p /*info->username*/ );
					iqIdRegSetReg = JabberSerialNext();
					JabberSend( info->s, "<iq type='set' id='"JABBER_IQID"%d'><query xmlns='jabber:iq:register'>%s</query></iq>", iqIdRegSetReg, text );
					free( q );
				}
				free( p );
			}
			SendMessage( info->reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 75, ( LPARAM )JTranslate( "Sending registration information..." ));
		}
		// RECVED: result of the registration process
		// ACTION: account registration successful
		else if ( id == iqIdRegSetReg ) {
			JabberSend( info->s, "</stream:stream>" );
			SendMessage( info->reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, ( LPARAM )JTranslate( "Registration successful" ));
			info->reg_done = TRUE;
	}	}

	else if ( !strcmp( type, "error" )) {
		errorNode = JabberXmlGetChild( node, "error" );
		str = JabberErrorMsg( errorNode );
		SendMessage( info->reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, ( LPARAM )str );
		free( str );
		info->reg_done = TRUE;
		JabberSend( info->s, "</stream:stream>" );
}	}

static void __cdecl JabberKeepAliveThread( JABBER_SOCKET s )
{
	NETLIBSELECT nls = {0};

	nls.cbSize = sizeof( NETLIBSELECT );
	nls.dwTimeout = 60000;	// 60000 millisecond ( 1 minute )
	nls.hExceptConns[0] = s;
	for ( ;; ) {
		if ( JCallService( MS_NETLIB_SELECT, 0, ( LPARAM )&nls ) != 0 )
			break;
		if ( jabberSendKeepAlive )
			JabberSend( s, " \t " );
	}
	JabberLog( "Exiting KeepAliveThread" );
}
