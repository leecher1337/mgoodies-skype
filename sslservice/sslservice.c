/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include <windows.h>
_CRTIMP int __cdecl _vsnprintf(char *, size_t, const char *, va_list);
#include <newpluginapi.h>
#include <m_netlib.h>
#include "m_sslservice.h"
#include "cyassl/include/openssl/ssl.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;
char authemail[] = "fscking@spammer.oip.info";//the correct e-mail shall be constructed in Load
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"SSL Service",
	PLUGIN_MAKE_VERSION(0,0,1,0),
	"Provides SSL support in Miranda.\nIncludes portions of cyassl implementation",
	"Y.B.",
	authemail, 
	"© 2007 yb",
	"http://saaplugin.no-ip.info/sslservice/",
	0,		//not transient
	0,		//doesn't replace anything built-in
	{ /* d950af49-0394-4f00-b0de-35ea653b358e */
		0xd950af49,
		0x0394,
		0x4f00,
		{0xb0, 0xde, 0x35, 0xea, 0x65, 0x3b, 0x35, 0x8e}
	}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	memcpy(pluginInfo.authorEmail,"y_b@saaplugin.no-",17);
	hInst=hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}
/* 5ba7c6f1-e708-40bb-af29-35e567a5e19e */
#define MIID_TEXTCONSOLE     { 0x5ba7c6f1, 0xe708, 0x40bb, {0xaf, 0x29, 0x35, 0xe5, 0x67, 0xa5, 0xe1, 0x9e}}
static const MUUID interfaces[] = {MIID_TEXTCONSOLE, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}
//#define proof_of_concept
#ifdef proof_of_concept
static HANDLE hOnModulesLoaded = NULL;
static int ProofOfConcept( WPARAM wParam, LPARAM lParam ){
	struct SSL_INTERFACE ssli;
	void *ctx=0;
	HANDLE hNetlibUser;
	NETLIBUSER nlu = {0};
	ssli.cbSize=sizeof(ssli);
	CallService(MS_SYSTEM_GET_SSLI,0,(LPARAM)&ssli);
	UnhookEvent(hOnModulesLoaded); hOnModulesLoaded=NULL;
	nlu.cbSize = sizeof( nlu );
	nlu.flags = NUF_OUTGOING;
	nlu.szDescriptiveName = "TestName";
	nlu.szSettingsModule = "SSLTest";

	hNetlibUser = ( HANDLE ) CallService( MS_NETLIB_REGISTERUSER, 0, ( LPARAM )&nlu );
	{
		NETLIBOPENCONNECTION tConn = { 0 };
		HANDLE h=0;
		tConn.cbSize = sizeof( tConn );
		tConn.szHost = "www.google.com";
		tConn.wPort = 443;
		h = ( HANDLE )CallService( MS_NETLIB_OPENCONNECTION, ( WPARAM )hNetlibUser, ( LPARAM )&tConn );
		if ( h == NULL )
			return 0;
		ctx = ssli.SSL_CTX_new(ssli.TLSv1_client_method());
		if (ctx){
			PVOID ssl = ssli.SSL_new(ctx);
			if (ssl){
				SOCKET s = CallService( MS_NETLIB_GETSOCKET, ( WPARAM )h, 0 );
				if ( s != INVALID_SOCKET ) {
					ssli.SSL_set_fd( ssl, s );
					if ( ssli.SSL_connect( ssl ) > 0 ) {
						char *buf = ( char* )malloc( 1024 );
						int nBytes = mir_snprintf( buf, 1024,
						"GET / HTTP/1.1\r\n"
						"Host: %s\r\n\r\n",tConn.szHost);
						CallService( MS_NETLIB_LOG,(WPARAM)hNetlibUser,(LPARAM)"SSL connection succeeded" );
						ssli.SSL_write( ssl, buf, nBytes);
						nBytes = ssli.SSL_read( ssl, buf, 1024-1 );
						CallService( MS_NETLIB_LOG,(WPARAM)hNetlibUser,(LPARAM)buf );
						free(buf);
					}
				}
				ssli.SSL_free( ssl );
			}
			ssli.SSL_CTX_free(ctx);
		}
		CallService( MS_NETLIB_CLOSEHANDLE, ( WPARAM )hNetlibUser, 0 );
	}

	return 0;
}
#endif //proof_of_concept

void* SSL_CTX_new_no_verify(void * method){
	void *ctx=SSL_CTX_new(method);
	if (ctx) SSL_CTX_set_verify((SSL_CTX *)ctx,SSL_VERIFY_NONE,0);
	return ctx;
}

int GetSslInterface(WPARAM wParam, LPARAM lParam)
{
	struct SSL_INTERFACE *ssli = (struct SSL_INTERFACE*) lParam;
	if ( ssli == NULL )
		return 1;
	if ( ssli->cbSize != sizeof( struct SSL_INTERFACE ))
		return 1;

	ssli->SSL_connect = SSL_connect;
	ssli->SSL_CTX_free = SSL_CTX_free;
	ssli->SSL_CTX_new = SSL_CTX_new_no_verify;
	ssli->SSL_free = SSL_free;
	ssli->SSL_new = SSL_new;
	ssli->SSL_read = SSL_read;
	ssli->SSL_set_fd = SSL_set_fd;
	ssli->SSL_write = SSL_write;
	ssli->SSLv3_client_method = SSLv3_client_method;
	ssli->TLSv1_client_method = TLSv1_client_method;
	return 0;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink=link;
	InitCyaSSL();
	CreateServiceFunction(MS_SYSTEM_GET_SSLI,GetSslInterface);
#ifdef proof_of_concept
	hOnModulesLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, ProofOfConcept );
#endif //proof_of_concept
	return 0;
}

int __declspec(dllexport) Unload(void)
{
	DestroyServiceFunction(MS_SYSTEM_GET_SSLI);
	FreeCyaSSL();
	return 0;
}


