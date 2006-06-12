#ifndef __SSL_H
#define __SSL_H

#include "netclient.h"

#pragma warning( disable : 4290 )
#define SSLTHRUNETLIB		//performs netlib connection before normal winsock connection


typedef int (*PFN_SSL_int_void)(void);
typedef PVOID (*PFN_SSL_pvoid_void)(void);
typedef PVOID (*PFN_SSL_pvoid_pvoid)(PVOID);
typedef void (*PFN_SSL_void_pvoid)(PVOID);
typedef int (*PFN_SSL_int_pvoid_int)(PVOID, int);
typedef int (*PFN_SSL_int_pvoid)(PVOID);
typedef int (*PFN_SSL_int_pvoid_pvoid_int)(PVOID, PVOID, int);

class CSSLClient: public CNetClient
{
public:
	CSSLClient(): hConnection(NULL), sock(INVALID_SOCKET), ConEstablished(FALSE) {}
	void Connect(const char* servername,const int port) throw(DWORD);
	void Send(const char *query) throw(DWORD);
	char* Recv(char *buf=NULL,int buflen=65536) throw(DWORD);
	void Disconnect();
	void SSLify()throw(DWORD);

	inline BOOL Connected() {return ConEstablished;}

	//static BOOL SSLLoaded;
	//static HINSTANCE hSSLLibrary;
	//static PVOID SSLCtx;
protected:
	HANDLE hConnection;
#ifdef SSLTHRUNETLIB
	HANDLE hNLConn;
#endif

	int sock;
	struct hostent *server;
	struct sockaddr_in connection;

	BOOL ConEstablished;
};

enum
{
	ESSL_NOTLOADED=1,	//OpenSSL is not loaded
	ESSL_WINSOCKINIT,	//WinSock 2.0 init failed
	ESSL_GETHOSTBYNAME,	//DNS error
	ESSL_CREATESOCKET,	//error creating socket
	ESSL_SOCKETCONNECT,	//error connecting with socket
	ESSL_CREATESSL,		//error creating SSL session structure
	ESSL_SETSOCKET,		//error connect socket with SSL session for bidirect I/O space
	ESSL_CONNECT,		//cannot connect to server
	ESSL_SEND,		//cannot send data
	ESSL_RECV,		//cannot receive data
	ESSL_RECVALLOC,		//cannot allocate memory for received data
};

#endif
