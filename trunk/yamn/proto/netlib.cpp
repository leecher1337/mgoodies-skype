/*
 * This code implements communication based on Miranda netlib library
 *
 * (c) majvan 2002-2004
 */

#include <windows.h>
#include <stdio.h>
#include "../../../../SDK/headers_c/newpluginapi.h"	//CallService,UnHookEvent
#include "../../../../SDK/headers_c/m_netlib.h"		//socket thorugh proxy functions
#include "../../../../SDK/headers_c/m_langpack.h"	//langpack for "connection" and other words
#include "../debug.h"
#include "netlib.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

HANDLE hNetlibUser=NULL;

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

//Connects to the server through the sock
//if not success, exception is throwed
void CNLClient::Connect(const char* servername,const int port) throw(DWORD)
{
	NETLIBOPENCONNECTION nloc;

	NetworkError=SystemError=0;
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
		if((SOCKET_ERROR==(Sent=Netlib_Send(hConnection,query,strlen(query),MSG_DUMPASTEXT))) || Sent!=strlen(query))
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
		ZeroMemory(buf,buflen);
		if(SOCKET_ERROR==(Rcv=Netlib_Recv(hConnection,buf,buflen,MSG_DUMPASTEXT)))
		{
			SystemError=WSAGetLastError();
			throw NetworkError=(DWORD)ENL_RECV;
		}
		if(!Rcv)
		{
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