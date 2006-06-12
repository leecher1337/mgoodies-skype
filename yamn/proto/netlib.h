#ifndef __NETLIB_H
#define __NETLIB_H

#include "netclient.h"

#pragma warning( disable : 4290 )

class CNLClient: public CNetClient
{
public:
	CNLClient(): hConnection(NULL) {}
	void Connect(const char* servername,const int port) throw(DWORD);
	void Send(const char *query) throw(DWORD);
	char* Recv(char *buf=NULL,int buflen=65536) throw(DWORD);
	void Disconnect();
	void SSLify()throw(DWORD);
	
	inline BOOL Connected() {return hConnection!=NULL;}

protected:
	HANDLE hConnection;
	BOOL isTLSed;
	PVOID ssl;
	int LocalNetlib_Send(HANDLE hConn,const char *buf,int len,int flags);
	int LocalNetlib_Recv(HANDLE hConn,char *buf,int len,int flags);
};

enum
{
	ENL_WINSOCKINIT=1,	//error initializing socket	//only wsock
	ENL_GETHOSTBYNAME,	//DNS error			//only wsock
	ENL_CREATESOCKET,	//error creating socket		//only wsock
	ENL_CONNECT,		//cannot connect to server
	ENL_SEND,			//cannot send data
	ENL_RECV,			//cannot receive data
	ENL_RECVALLOC,		//cannot allocate memory for received data
	ENL_TIMEOUT,		//timed out during recv
};

#endif
