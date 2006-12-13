/*
 * This code implements communication based on Miranda netlib library
 *
 * (c) majvan 2002-2004
 */

#include "../filter/simple/AggressiveOptimize.h"
#include <windows.h>
#include <stdio.h>
#include <newpluginapi.h>	//CallService,UnHookEvent
#include <m_netlib.h>		//socket thorugh proxy functions
#include <m_langpack.h>	//langpack for "connection" and other words
#include "../debug.h"
#include "netlib.h"
#include "ssl.h"

extern void __stdcall	SSL_DebugLog( const char *fmt, ... );
extern PFN_SSL_pvoid_pvoid		SSL_new;			// SSL *SSL_new(SSL_CTX *ctx)
extern PFN_SSL_void_pvoid		SSL_free;			// void SSL_free(SSL *ssl);
extern PFN_SSL_int_pvoid_int		SSL_set_fd;			// int SSL_set_fd(SSL *ssl, int fd);
extern PFN_SSL_int_pvoid		SSL_connect;			// int SSL_connect(SSL *ssl);
extern PFN_SSL_int_pvoid_pvoid_int	SSL_read;			// int SSL_read(SSL *ssl, void *buffer, int bufsize)
extern PFN_SSL_int_pvoid_pvoid_int	SSL_write;			// int SSL_write(SSL *ssl, void *buffer, int bufsize)

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

HANDLE hNetlibUser=NULL;

extern PVOID TLSCtx;
extern PVOID SSLCtx;

HANDLE RegisterNLClient(const char *name)
{
	static NETLIBUSER nlu={0};
	char desc[128];

	sprintf(desc, "%s %s",name,Translate("connection"));

#ifdef DEBUG_COMM
	DebugLog(CommFile,"<Register PROXY support>");
#endif
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING  | NUF_HTTPCONNS;
	nlu.szDescriptiveName=desc;
	nlu.szSettingsModule=(char *)name;
	hNetlibUser=(HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);

#ifdef DEBUG_COMM
	if(NULL==hNetlibUser)
		DebugLog(CommFile,"<error></Register PROXY support>\n");
	else
		DebugLog(CommFile,"</Register PROXY support>\n");
#endif
	return hNetlibUser;
}

//Move connection to SSL
void CNLClient::SSLify() throw(DWORD){
	SSL_DebugLog("Staring %s...",TLSCtx?"TLS":"SSL");
	int socket = CallService( MS_NETLIB_GETSOCKET, ( WPARAM ) hConnection, 0 );
	if ( (ssl=SSL_new( TLSCtx?TLSCtx:SSLCtx) ) != NULL ) 
	{
		SSL_DebugLog( "TLS(%08X) create layer %s",ssl,"ok" );
		if ( SSL_set_fd( ssl, socket ) > 0 ) 
		{
			SSL_DebugLog( "TLS(%08X) set fd %s",ssl,"ok" );
			if ( SSL_connect( ssl ) > 0 ) 
			{
				isTLSed = true;	// This make all communication on this handle use SSL
				SSL_DebugLog( "TLS(%08X) negotiation at (%08X:%d) %s", ssl, hConnection, socket, "ok" );
				return;
			} 
			else 
			{
				SSL_DebugLog( "TLS(%08X) negotiation at (%08X:%d) %s", ssl, hConnection, socket, "failed" );
		}	}
		else 
		{
			SSL_DebugLog( "TLS(%08X) set fd %s",ssl,"failed" );
		}
		SSL_free( ssl );
		ssl = NULL;  //::Disconnect should not try to SSL_free(ssl) again
//		SSL_CTX_free(localSSLCtx);
	} 
	else 
		SSL_DebugLog( "TLS(%08X) create layer %s",0,"failed" );
	throw NetworkError=(DWORD)ESSL_CREATESSL;
}

//Connects to the server through the sock
//if not success, exception is throwed
void CNLClient::Connect(const char* servername,const int port) throw(DWORD)
{
	NETLIBOPENCONNECTION nloc;

	NetworkError=SystemError=0;
	isTLSed = false;
	ssl = NULL;

#ifdef DEBUG_COMM
	DebugLog(CommFile,"<connect>\n");
#endif
	try
	{
		nloc.cbSize=sizeof(NETLIBOPENCONNECTION);
		nloc.szHost=servername;
		nloc.wPort=port;
		nloc.flags=0;
		if(NULL==(hConnection=(HANDLE)CallService(MS_NETLIB_OPENCONNECTION,(WPARAM)hNetlibUser,(LPARAM)&nloc)))
		{
			SystemError=WSAGetLastError();
			throw NetworkError=(DWORD)ENL_CONNECT;
		}
#ifdef DEBUG_COMM
		DebugLog(CommFile,"</connect>\n");
#endif
		return;
	}
	catch(...)
	{
#ifdef DEBUG_COMM
		DebugLog(CommFile,"<error></connect>\n");
#endif
		throw;
	}
}

//Performs a simple query
// query- command to send
int CNLClient::LocalNetlib_Send(HANDLE hConn,const char *buf,int len,int flags) {
	if (isTLSed) 
	{
		SSL_DebugLog("TLS(%08X) send: %s",ssl,buf);
		int res = SSL_write(ssl,(PVOID)buf,len);
//		SSL_DebugLog("TLS send result: %d",res);
		return res;
	} 
	else 
	{
		NETLIBBUFFER nlb={(char*)buf,len,flags};
		return CallService(MS_NETLIB_SEND,(WPARAM)hConn,(LPARAM)&nlb);
	}
}

void CNLClient::Send(const char *query) throw(DWORD)
{
	unsigned int Sent;

	if(NULL==query)
		return;
	if(hConnection==NULL)
		return;
#ifdef DEBUG_COMM
	DebugLog(CommFile,"<send>%s",query);
#endif
	try
	{
		if((SOCKET_ERROR==(Sent=LocalNetlib_Send(hConnection,query,strlen(query),MSG_DUMPASTEXT))) || Sent!=strlen(query))
		{
			SystemError=WSAGetLastError();
			throw NetworkError=(DWORD)ENL_SEND;
		}
#ifdef DEBUG_COMM
		DebugLog(CommFile,"</send>\n");
#endif
	}
	catch(...)
	{
#ifdef DEBUG_COMM
		DebugLog(CommFile,"<error></send>\n");
#endif
		throw;
	}
}

//Reads data from socket
// buf- buffer where to store max. buflen of received characters
//      if buf is NULL, creates buffer of buflen size
//      buf is NULL by default
//You need free() returned buffer, which can be allocated in this function
//if not success, exception is throwed

int CNLClient::LocalNetlib_Recv(HANDLE hConn,char *buf,int len,int flags) {
	if (isTLSed) 
	{
//		SSL_DebugLog("SSL(%08X) recving",ssl);
		if (ssl){
			int res = SSL_read(ssl,buf,len);
			SSL_DebugLog("TLS(%08X) recv: %s",ssl,buf);
			return res;
		} else {
			SSL_DebugLog("TLS(%08X) SSL connection is lost",ssl);
			return 0;
		}
	} 
	else 
	{
		NETLIBBUFFER nlb={buf,len,flags};
		return CallService(MS_NETLIB_RECV,(WPARAM)hConn,(LPARAM)&nlb);
	}
}

char* CNLClient::Recv(char *buf,int buflen) throw(DWORD)
{
#ifdef DEBUG_COMM
	DebugLog(CommFile,"<reading>");
#endif
	try
	{
		if(buf==NULL)
			buf=(char *)malloc(sizeof(char)*(buflen+1));
		if(buf==NULL)
			throw NetworkError=(DWORD)ENL_RECVALLOC;

		if (!isTLSed)
		{
			NETLIBSELECT nls;
			memset(&nls, 0, sizeof(NETLIBSELECT));
			nls.cbSize = sizeof(NETLIBSELECT);
			nls.dwTimeout = 60000;
			nls.hReadConns[0] = hConnection;
			switch (CallService(MS_NETLIB_SELECT, 0, (LPARAM) &nls)) 
			{
				case SOCKET_ERROR:
				free(buf);
				SystemError=WSAGetLastError();
				throw NetworkError = (DWORD) ENL_RECV;
				case 0: // time out!
				free(buf);
				throw NetworkError = (DWORD) ENL_TIMEOUT;
			}
 		}

		ZeroMemory(buf,buflen);
		if(SOCKET_ERROR==(Rcv=LocalNetlib_Recv(hConnection,buf,buflen,MSG_DUMPASTEXT)))
		{
			free(buf);
			SystemError=WSAGetLastError();
			throw NetworkError=(DWORD)ENL_RECV;
		}
		if(!Rcv)
		{
			free(buf);
			SystemError=WSAGetLastError();
			throw NetworkError=(DWORD)ENL_RECV;
		}
#ifdef DEBUG_COMM
		*(buf+Rcv)=0;				//end the buffer to write it to file
		DebugLog(CommFile,"%s",buf);
		DebugLog(CommFile,"</reading>\n");
#endif
		return(buf);
	}
	catch(...)
	{
#ifdef DEBUG_COMM
		DebugLog(CommFile,"<error></reading>\n");
#endif
		throw;
	}
}

//Closes netlib connection
void CNLClient::Disconnect()
{
	if (ssl) 
	{
		SSL_DebugLog("TLS(%08X) Unregistering",ssl);
		SSL_free( ssl );
		SSL_DebugLog("TLS(%08X) Done",ssl);
		ssl = NULL;
	}
	Netlib_CloseHandle(hConnection);
	hConnection=(HANDLE)NULL;
}

//Uninitializes netlib library
void UnregisterNLClient()
{
#ifdef DEBUG_COMM
	DebugLog(CommFile,"<Unregister PROXY support>");
#endif
	
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser=(HANDLE)NULL;
#ifdef DEBUG_COMM
	DebugLog(CommFile,"</Unregister PROXY support>\n");
#endif
}
