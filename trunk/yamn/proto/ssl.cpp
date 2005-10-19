/*
 * This code implements communication based on OpenSSL library
 *
 * (c) majvan 2002,2004
 */

#include <windows.h>
#include <stdio.h>
#include "../../../../SDK/headers_c/newpluginapi.h"	//CallService,UnHookEvent
#include "../../../../SDK/headers_c/m_netlib.h"		//socket thorugh proxy functions
#include "../../../../SDK/headers_c/m_langpack.h"	//langpack for "connection" and other words
#include "../debug.h"
#ifdef SSLTHRUNETLIB
	#include "netlib.h"					//yes, we want to use netlib connection
#endif
#include "ssl.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

PFN_SSL_int_void		SSL_library_init;		// int SSL_library_init()
PFN_SSL_pvoid_void		SSLv23_client_method;		// SSL_METHOD *SSLv23_client_method()
PFN_SSL_pvoid_pvoid		SSL_CTX_new;			// SSL_CTX *SSL_CTX_new(SSL_METHOD *method)
PFN_SSL_void_pvoid		SSL_CTX_free;			// void SSL_CTX_free(SSL_CTX *ctx);
PFN_SSL_pvoid_pvoid		SSL_new;			// SSL *SSL_new(SSL_CTX *ctx)
PFN_SSL_void_pvoid		SSL_free;			// void SSL_free(SSL *ssl);
PFN_SSL_int_pvoid_int		SSL_set_fd;			// int SSL_set_fd(SSL *ssl, int fd);
PFN_SSL_int_pvoid		SSL_connect;			// int SSL_connect(SSL *ssl);
PFN_SSL_int_pvoid_pvoid_int	SSL_read;			// int SSL_read(SSL *ssl, void *buffer, int bufsize)
PFN_SSL_int_pvoid_pvoid_int	SSL_write;			// int SSL_write(SSL *ssl, void *buffer, int bufsize)
PFN_SSL_int_pvoid_int	SSL_get_error;			// int SSL_write(SSL *ssl, int ret)


PVOID CSSLClient::SSLCtx=NULL;
BOOL CSSLClient::SSLLoaded=FALSE;
HINSTANCE CSSLClient::hSSLLibrary=(HINSTANCE)NULL;
extern HANDLE hNetlibUser;

int RegisterSSL()
{
#ifdef DEBUG_COMM
	DebugLog(CommFile,"<Register SSL support>");
#endif
	if(NULL==(CSSLClient::hSSLLibrary=LoadLibrary("ssleay32.dll")))
		if(NULL==(CSSLClient::hSSLLibrary=LoadLibrary("libssl32.dll")))		//try to load library using the old OpenSSL filename
		{
#ifdef DEBUG_COMM
			DebugLog(CommFile,"<error, status:library not found></Register SSL support>\n");
#endif
			return 0;
		}

	if(NULL!=(SSL_library_init=(PFN_SSL_int_void)GetProcAddress(CSSLClient::hSSLLibrary,"SSL_library_init")))
		if(NULL!=(SSLv23_client_method=(PFN_SSL_pvoid_void)GetProcAddress(CSSLClient::hSSLLibrary,"SSLv23_client_method")))
			if(NULL!=(SSL_CTX_new=(PFN_SSL_pvoid_pvoid)GetProcAddress(CSSLClient::hSSLLibrary,"SSL_CTX_new")))
				if(NULL!=(SSL_CTX_free=(PFN_SSL_void_pvoid)GetProcAddress(CSSLClient::hSSLLibrary,"SSL_CTX_free")))
					if(NULL!=(SSL_new=(PFN_SSL_pvoid_pvoid)GetProcAddress(CSSLClient::hSSLLibrary,"SSL_new")))
						if(NULL!=(SSL_free=(PFN_SSL_void_pvoid)GetProcAddress(CSSLClient::hSSLLibrary,"SSL_free")))
							if(NULL!=(SSL_set_fd=(PFN_SSL_int_pvoid_int)GetProcAddress(CSSLClient::hSSLLibrary,"SSL_set_fd")))
								if(NULL!=(SSL_connect=(PFN_SSL_int_pvoid)GetProcAddress(CSSLClient::hSSLLibrary,"SSL_connect")))
									if(NULL!=(SSL_read=(PFN_SSL_int_pvoid_pvoid_int)GetProcAddress(CSSLClient::hSSLLibrary,"SSL_read")))
										if(NULL!=(SSL_write=(PFN_SSL_int_pvoid_pvoid_int)GetProcAddress(CSSLClient::hSSLLibrary,"SSL_write")))
											if(NULL!=(SSL_get_error=(PFN_SSL_int_pvoid_int)GetProcAddress(CSSLClient::hSSLLibrary,"SSL_get_error")))
											{
												SSL_library_init();
												CSSLClient::SSLCtx=SSL_CTX_new(SSLv23_client_method());	//SSL2,3 & TLS1 used
#ifdef DEBUG_COMM
												DebugLog(CommFile,"</Register SSL support>\n");
#endif
												CSSLClient::SSLLoaded=TRUE;
												return 1;
											}
	FreeLibrary(CSSLClient::hSSLLibrary);
	CSSLClient::hSSLLibrary=(HINSTANCE)NULL;
#ifdef DEBUG_COMM
	DebugLog(CommFile,"<error, status:library not compatible></Register SSL support>\n");
#endif
	return 0;
}

//Connects to the server through the sock
//if not success, exception is throwed
void CSSLClient::Connect(const char* servername,const int port) throw(DWORD)
{
	WSADATA wsaData;

	NetworkError=SystemError=0;

	if(!SSLLoaded)
		throw NetworkError=ESSL_NOTLOADED;
	try
	{
#ifdef SSLTHRUNETLIB
		NETLIBOPENCONNECTION nloc;

		nloc.cbSize=sizeof(NETLIBOPENCONNECTION);
		nloc.szHost=servername;
		nloc.wPort=port;
		nloc.flags=0;
	#ifdef DEBUG_COMM
		DebugLog(CommFile,"<open connection>\n");
	#endif
		if(NULL==(hNLConn=(HANDLE)CallService(MS_NETLIB_OPENCONNECTION,(WPARAM)hNetlibUser,(LPARAM)&nloc)))
		{			
	#ifdef DEBUG_COMM
			DebugLog(CommFile,"<error></open connection>\n");
	#endif
			sock=INVALID_SOCKET;
		}
		else
		{
	#ifdef DEBUG_COMM
			DebugLog(CommFile,"</open connection>\n");
	#endif
			sock=CallService(MS_NETLIB_GETSOCKET,(WPARAM)hNLConn,0);
		}
#endif

		if(sock==INVALID_SOCKET)
		{
			if(0!=WSAStartup(MAKEWORD(2,0),&wsaData))
			{
				SystemError=WSAGetLastError();
				throw NetworkError=(DWORD)ESSL_WINSOCKINIT;
			}
			ZeroMemory(&connection,sizeof(struct sockaddr_in));
#ifdef DEBUG_COMM
			DebugLog(CommFile,"<gethostbyname>\n");
#endif
			if(NULL==(server=gethostbyname(servername)))
			{
				SystemError=WSAGetLastError();
				throw NetworkError=(DWORD)ESSL_GETHOSTBYNAME;
			}
			memmove((char*)&(connection.sin_addr.s_addr),server->h_addr,server->h_length);
			connection.sin_family=AF_INET;
			connection.sin_port=htons((unsigned short int)port);	/* integral size mismatch in argument - htons(port)*/
			if(INVALID_SOCKET==(sock=socket(AF_INET,SOCK_STREAM,0)))
			{
				SystemError=WSAGetLastError();
				throw NetworkError=(DWORD)ESSL_CREATESOCKET;
			}
			if(-1==connect(sock,(struct sockaddr*)&connection,sizeof(connection)))
			{
				SystemError=WSAGetLastError();
				throw NetworkError=(DWORD)ESSL_SOCKETCONNECT;
			}
#ifdef DEBUG_COMM
			DebugLog(CommFile,"</gethostbyname>\n");
#endif
		}

#ifdef DEBUG_COMM
		DebugLog(CommFile,"<connect SSL>\n");
#endif
		if(NULL==(hConnection=SSL_new(SSLCtx)))
			throw NetworkError=(DWORD)ESSL_CREATESSL;
		if(SSL_set_fd(hConnection,sock)<1)
			throw NetworkError=(DWORD)ESSL_SETSOCKET;
		if(SSL_connect(hConnection)<1)
			throw NetworkError=(DWORD)ESSL_CONNECT;
		ConEstablished=TRUE;
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
void CSSLClient::Send(const char *query) throw(DWORD)
{
	unsigned int Sent;

	if(NULL==query)
		return;
#ifdef DEBUG_COMM
	DebugLog(CommFile,"<send SSL>%s",query);
#endif
	try
	{
		if(!ConEstablished)
			throw NetworkError=(DWORD)ESSL_SEND;
		Sent=SSL_write(hConnection,(PVOID)query,strlen(query));
		if(Sent!=strlen(query))
		{
			SystemError=SSL_get_error(hConnection,Sent);
			throw NetworkError=(DWORD)ESSL_SEND;
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

//Reads data from SSL socket
// buf- buffer where to store max. buflen of received characters
//      if buf is NULL, creates buffer of buflen size
//      buf is NULL by default
//You need free() returned buffer, which can be allocated in this function
//if not success, exception is throwed
char* CSSLClient::Recv(char *buf,int buflen) throw(DWORD)
{
#ifdef DEBUG_COMM
	DebugLog(CommFile,"<reading>");
#endif
	try
	{
		if(!ConEstablished)
			throw NetworkError=(DWORD)ESSL_RECV;
		if(buf==NULL)
			buf=(char *)malloc(sizeof(char)*(buflen+1));
		if(buf==NULL)
			throw NetworkError=(DWORD)ESSL_RECVALLOC;
		ZeroMemory(buf,buflen);
		Rcv=SSL_read(hConnection,buf,buflen);
		if(Rcv<1)
		{
			SystemError=SSL_get_error(hConnection,Rcv);
			throw NetworkError=(DWORD)ESSL_RECV;
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

//Closes SSL connection
void CSSLClient::Disconnect()
{
#ifdef SSLTHRUNETLIB
	if((HANDLE)NULL!=hNLConn)
		Netlib_CloseHandle(hNLConn);
	else
#endif
	if(INVALID_SOCKET!=sock)
		closesocket(sock);
	
	if(hConnection!=(HANDLE)NULL)
		SSL_free(hConnection);
	hConnection=(HANDLE)NULL;
	sock=INVALID_SOCKET;
	hNLConn=(HANDLE)NULL;
	ConEstablished=FALSE;
}

void UnregisterSSL()
{
	if(CSSLClient::SSLLoaded)
	{
#ifdef DEBUG_COMM
		DebugLog(CommFile,"<Unregister SSL support>");
#endif
		SSL_CTX_free(CSSLClient::SSLCtx);
		FreeLibrary(CSSLClient::hSSLLibrary);
		CSSLClient::hSSLLibrary=(HINSTANCE)NULL;
#ifdef DEBUG_COMM
		DebugLog(CommFile,"</Unregister SSL support>\n");
#endif
	}
}
