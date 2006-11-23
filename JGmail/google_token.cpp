/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
Copyright (c) 2006 Y.B (Adapted for JGmail).
Copyright (c) 2003-5 George Hazan.
Copyright (c) 2002-3 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL$
Revision       : $Rev$
Last change on : $Date$
Last change by : $Author$

*/

#include "jabber.h"
#include "version.h"
#include "jabber_secur.h"

void __stdcall JabberLog( const char* fmt, ... );
#define respString "Step %d: Responce is %stive: %s"
#define SSL_BUF_SIZE 8192

/////////////////////////////////////////////////////////////////////////////////////////
// Basic SSL operation class
struct SSL_Base
{

	virtual	~SSL_Base() {}

	virtual  int init() = 0;
	virtual  char* getSslToken( char* data ) = 0;
	char * result;
};



/////////////////////////////////////////////////////////////////////////////////////////
// WinInet class
/////////////////////////////////////////////////////////////////////////////////////////

//all that junk out if static SSL is used
#ifndef STATICSSL

#define ERROR_FLAGS (FLAGS_ERROR_UI_FILTER_FOR_ERRORS | FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS )

#include "wininet.h"


typedef BOOL  ( WINAPI *ft_HttpQueryInfo )( HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD );
typedef BOOL  ( WINAPI *ft_HttpSendRequest )( HINTERNET, LPCSTR, DWORD, LPVOID, DWORD );
typedef BOOL  ( WINAPI *ft_InternetCloseHandle )( HINTERNET );
typedef DWORD ( WINAPI *ft_InternetErrorDlg )( HWND, HINTERNET, DWORD, DWORD, LPVOID* );
typedef BOOL  ( WINAPI *ft_InternetSetOption )( HINTERNET, DWORD, LPVOID, DWORD );
typedef BOOL  ( WINAPI *ft_InternetReadFile )( HINTERNET, LPVOID, DWORD, LPDWORD );

typedef HINTERNET ( WINAPI *ft_HttpOpenRequest )( HINTERNET, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR*, DWORD, DWORD );
typedef HINTERNET ( WINAPI *ft_InternetConnect )( HINTERNET, LPCSTR, INTERNET_PORT, LPCSTR, LPCSTR, DWORD, DWORD, DWORD );
typedef HINTERNET ( WINAPI *ft_InternetOpen )( LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD );

#define ERROR_INTERNET_SEC_CERT_ERRORS          (INTERNET_ERROR_BASE + 55)
#define ERROR_INTERNET_SEC_CERT_NO_REV          (INTERNET_ERROR_BASE + 56)
#define ERROR_INTERNET_SEC_CERT_REV_FAILED      (INTERNET_ERROR_BASE + 57)

struct SSL_WinInet : public SSL_Base
{
	virtual ~SSL_WinInet();

	virtual  char* getSslToken( char* data );
	virtual  int init();

	char* getSslResult(char * parUrl, char* data );

	void readInput( HINTERNET );
private: int stage;

	//-----------------------------------------------------------------------------------
	HMODULE m_dll;

	ft_InternetCloseHandle f_InternetCloseHandle;
	ft_InternetConnect     f_InternetConnect;
	ft_InternetErrorDlg    f_InternetErrorDlg;
	ft_InternetOpen        f_InternetOpen;
	ft_InternetReadFile    f_InternetReadFile;
	ft_InternetSetOption   f_InternetSetOption;
	ft_HttpOpenRequest     f_HttpOpenRequest;
	ft_HttpQueryInfo       f_HttpQueryInfo;
	ft_HttpSendRequest     f_HttpSendRequest;
};

/////////////////////////////////////////////////////////////////////////////////////////

int SSL_WinInet::init()
{
	if (( m_dll = LoadLibraryA( "WinInet.dll" )) == NULL )
		return 10;

	f_InternetCloseHandle = (ft_InternetCloseHandle)GetProcAddress( m_dll, "InternetCloseHandle" );
	f_InternetConnect = (ft_InternetConnect)GetProcAddress( m_dll, "InternetConnectA" );
	f_InternetErrorDlg = (ft_InternetErrorDlg)GetProcAddress( m_dll, "InternetErrorDlg" );
	f_InternetOpen = (ft_InternetOpen)GetProcAddress( m_dll, "InternetOpenA" );
	f_InternetReadFile = (ft_InternetReadFile)GetProcAddress( m_dll, "InternetReadFile" );
	f_InternetSetOption = (ft_InternetSetOption)GetProcAddress( m_dll, "InternetSetOptionA" );
	f_HttpOpenRequest = (ft_HttpOpenRequest)GetProcAddress( m_dll, "HttpOpenRequestA" );
	f_HttpQueryInfo = (ft_HttpQueryInfo)GetProcAddress( m_dll, "HttpQueryInfoA" );
	f_HttpSendRequest = (ft_HttpSendRequest)GetProcAddress( m_dll, "HttpSendRequestA" );

	stage = 0;
	result= 0;

	return 0;
}

SSL_WinInet::~SSL_WinInet()
{
	#if defined( _UNICODE ) 
		FreeLibrary( m_dll );   // we free WININET.DLL only if we're under NT
	#endif
	if (result) mir_free(result);
	result = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

void SSL_WinInet::readInput( HINTERNET hRequest )
{
	DWORD dwSize;

	do {
		char tmpbuf[100];
		f_InternetReadFile( hRequest, tmpbuf, 50, &dwSize);
	}
		while (dwSize != 0);
}

char* SSL_WinInet::getSslResult(char * parUrl, char* data )
{
	stage++;
	DWORD tFlags =
		INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS |
		INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
		INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
		INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
      INTERNET_FLAG_KEEP_CONNECTION |
		INTERNET_FLAG_NO_AUTO_REDIRECT |
      INTERNET_FLAG_NO_CACHE_WRITE |
		INTERNET_FLAG_NO_COOKIES |
		INTERNET_FLAG_RELOAD |
		INTERNET_FLAG_SECURE;

	const DWORD tInternetFlags =
		INTERNET_FLAG_NO_COOKIES |
		INTERNET_FLAG_NO_UI |
		INTERNET_FLAG_PRAGMA_NOCACHE |
		INTERNET_FLAG_SECURE;

	char* urlStart = strstr( parUrl, "://" );
	if ( urlStart == NULL )
		urlStart = parUrl;
	else
		urlStart += 3;

	{	int tLen = strlen( urlStart )+1;
		parUrl = ( char* )alloca( tLen );
		memcpy( parUrl, urlStart, tLen );
	}

	char* tObjectName = ( char* )strchr( parUrl, '/' );
	if ( tObjectName != NULL ) {
		int tLen = strlen( tObjectName )+1;
		char* newBuf = ( char* )alloca( tLen );
		memcpy( newBuf, tObjectName, tLen );

		*tObjectName = 0;
		tObjectName = newBuf;
	}
	else tObjectName = "/";



	HINTERNET tNetHandle;
	char* tBuffer = ( char* )_alloca( SSL_BUF_SIZE );

	tNetHandle = f_InternetOpen( "Miranda IM (JGmail "__VERSION_STRING")", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, tInternetFlags );

	if ( tNetHandle == NULL ) {
		JabberLog( "InternetOpen() failed" );
		return NULL;
	}

//	JabberLog( "SSL request: '%s'", data );

	char* tSslAnswer = NULL;

	HINTERNET tUrlHandle = f_InternetConnect( tNetHandle, parUrl, INTERNET_DEFAULT_HTTPS_PORT, "", "", INTERNET_SERVICE_HTTP, INTERNET_FLAG_NO_AUTO_REDIRECT + INTERNET_FLAG_NO_COOKIES, 0 );
	if ( tUrlHandle != NULL ) {
		HINTERNET tRequest = f_HttpOpenRequest( tUrlHandle, "POST", tObjectName, NULL, "", NULL, tFlags, NULL );
		if ( tRequest != NULL ) {
			DWORD tBufSize;
			bool  bProxyParamsSubstituted = false;

LBL_Restart:
			JabberLog( "Step %d: Sending request %s%s...", stage, parUrl, tObjectName );
#define HTTP_ADD_HEADERS "Content-Type: application/x-www-form-urlencoded\r\nX-Opinion: WinInet is evil"
			DWORD tErrorCode = f_HttpSendRequest( tRequest, HTTP_ADD_HEADERS, strlen(HTTP_ADD_HEADERS), data, strlen(data) );
//			DWORD tErrorCode = f_HttpSendRequest( tRequest, NULL, 0, NULL, 0 );
			if ( tErrorCode == 0 ) {
				int lastError = GetLastError();
				JabberLog( "HttpSendRequest() failed with error %ld", lastError);
				tSslAnswer = strdup( "" );

				if ( lastError == 2 )
					JabberLog( "Internet Explorer is in the 'Offline' mode. Switch IE to the 'Online' mode and then try to relogin" );
			}
			else {
				DWORD dwCode;
				tBufSize = sizeof( dwCode );
				f_HttpQueryInfo( tRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwCode, &tBufSize, 0 );

				switch( dwCode ) {
				case HTTP_STATUS_REDIRECT:
					tBufSize = SSL_BUF_SIZE;
					f_HttpQueryInfo( tRequest, HTTP_QUERY_LOCATION, tBuffer, &tBufSize, NULL );
					JabberLog( "Redirected to '%s'", tBuffer );
					tSslAnswer = getSslResult( tBuffer, data );
					break;

				case HTTP_STATUS_DENIED:
				case HTTP_STATUS_PROXY_AUTH_REQ:
				case ERROR_INTERNET_HTTP_TO_HTTPS_ON_REDIR:
				case ERROR_INTERNET_INCORRECT_PASSWORD:
				case ERROR_INTERNET_INVALID_CA:
				case ERROR_INTERNET_POST_IS_NON_SECURE:
				case ERROR_INTERNET_SEC_CERT_CN_INVALID:
				case ERROR_INTERNET_SEC_CERT_DATE_INVALID:
				case ERROR_INTERNET_SEC_CERT_ERRORS:
				case ERROR_INTERNET_SEC_CERT_NO_REV:
				case ERROR_INTERNET_SEC_CERT_REV_FAILED:
					JabberLog( "HttpSendRequest returned error code %d", tErrorCode );
					if ( ERROR_INTERNET_FORCE_RETRY == f_InternetErrorDlg( GetDesktopWindow(), tRequest, tErrorCode, ERROR_FLAGS, NULL )) {
						readInput( tRequest );
						goto LBL_Restart;
					}

					// else fall into the general error handling routine

				case HTTP_STATUS_OK:
					JabberLog(respString,stage,"posi","OK");
					{
						unsigned long numRead = 0;
//						f_HttpQueryInfo( tRequest, HTTP_QUERY_RAW_HEADERS_CRLF, tBuffer, &tBufSize, NULL );
						if (f_InternetReadFile(tRequest, tBuffer, SSL_BUF_SIZE, &numRead)){
							tBuffer[numRead] = '\0';
							//JabberLog("numRead: %d; Data: %s\n",numRead,tBuffer);
							tSslAnswer = strdup( tBuffer );
						} else {
							tSslAnswer = strdup( "" );
						}
					}
					break;
				default:
					tBufSize = SSL_BUF_SIZE;
					if ( !f_HttpQueryInfo( tRequest, HTTP_QUERY_STATUS_TEXT, tBuffer, &tBufSize, NULL ))
						strcpy( tBuffer, "unknown error" );
					JabberLog(respString,stage,"nega",tBuffer);
					tSslAnswer = strdup( "" );
			}	}

			f_InternetCloseHandle( tRequest );
		}

		f_InternetCloseHandle( tUrlHandle );
	}
	else JabberLog( "InternetOpenUrl() failed" );

	f_InternetCloseHandle( tNetHandle );
	return tSslAnswer;
}

char* SSL_WinInet::getSslToken(char * data){
	char *sslResult = getSslResult("https://www.google.com/accounts/ClientAuth",data);
	int len = strlen(sslResult);
	char *temp;
	char * SID = NULL;
	char * LSID = NULL;
	temp = sslResult;
	while (temp < sslResult+len){
		if (!strncmp(temp,"SID=",4)) SID = temp;
		if (!strncmp(temp,"LSID=",5)) LSID = temp;
		temp = strchr(temp,'\n');
		temp[0] = '\0'; if (temp[-1] == '\r') temp[-1] = '\0'; // awful code!
		temp++;
	}
	if (SID && LSID){
		temp = (char *)mir_alloc(SSL_BUF_SIZE);
		mir_snprintf(temp,SSL_BUF_SIZE,"%s&%s&service=mail&Session=true\r\n",SID,LSID);
		free(sslResult);
		sslResult = getSslResult("https://www.google.com/accounts/IssueAuthToken",temp);
		len = strlen(sslResult);
		if (sslResult[len-1]=='\n') {len--; sslResult[len]='\0';}
		if (sslResult[len-1]=='\r') {len--; sslResult[len]='\0';}
		if (len){
			result = (char *)mir_alloc(len+1);
			strcpy(result,sslResult);
		}
	} else JabberLog("Server did not return SID and LSID");
	free(sslResult);
	return result;
}
#endif //#ifdef STATICSSL

/////////////////////////////////////////////////////////////////////////////////////////
// Performs the login via SSL3 using the OpenSSL library

#include "jabber_ssl.h"

struct SSL_OpenSsl : public SSL_Base
{
	virtual ~SSL_OpenSsl();
	virtual  char* getSslToken( char* data );
	virtual  int init();
};

/////////////////////////////////////////////////////////////////////////////////////////


int SSL_OpenSsl::init()
{
	result = NULL;
	return 0;
}

SSL_OpenSsl::~SSL_OpenSsl(){
	if (result) mir_free(result);
	result = NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////


char* SSL_OpenSsl::getSslToken( char* data )
{
	NETLIBOPENCONNECTION tConn = { 0 };
	tConn.cbSize = sizeof( tConn );
	tConn.szHost = "www.google.com";
	tConn.wPort = 443;
	HANDLE h = ( HANDLE )JCallService( MS_NETLIB_OPENCONNECTION, ( WPARAM )hNetlibUser, ( LPARAM )&tConn );
	if ( h == NULL )
		return NULL;

	PVOID ssl = pfn_SSL_new( jabberSslCtx  );
	if ( ssl != NULL ) {
		SOCKET s = JCallService( MS_NETLIB_GETSOCKET, ( WPARAM )h, 0 );
		if ( s != INVALID_SOCKET ) {
			pfn_SSL_set_fd( ssl, s );
			if ( pfn_SSL_connect( ssl ) > 0 ) {
				JabberLog( "SSL connection succeeded" );

				char *buf = ( char* )mir_alloc( SSL_BUF_SIZE );

				int nBytes = mir_snprintf( buf, SSL_BUF_SIZE,
					"POST /%s HTTP/1.1\r\n"
					"Host: %s\r\n"
					"User-Agent: Miranda IM (JGmail "__VERSION_STRING")\r\n"
					"Content-Length: %d\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n"
					"Connection: Keep-Alive\r\n\r\n%s", "accounts/ClientAuth", tConn.szHost, strlen(data),data );

				//JabberLog( "Sending SSL query:\n%s", buf );
				pfn_SSL_write( ssl, buf, nBytes);

				nBytes = pfn_SSL_read( ssl, buf, SSL_BUF_SIZE-1 );
				if ( nBytes > 0 ) {
					buf[nBytes] = '\0'; //just in case
					char * responce = strchr(buf,' ')+1;
					char * temp = strchr(buf,'\n');
					temp[0] = '\0'; if (temp[-1] == '\r') temp[-1] = '\0'; // awful code!
					if (!strncmp(responce,"200",3)){
						JabberLog(respString,1,"posi",responce);
						temp++;
						char * SID = NULL;
						char * LSID = NULL;
						while (temp < buf+nBytes){
							if (!strncmp(temp,"SID=",4)) SID = temp;
							if (!strncmp(temp,"LSID=",5)) LSID = temp;
							temp = strchr(temp,'\n');
							temp[0] = '\0'; if (temp[-1] == '\r') temp[-1] = '\0'; // awful code!
							temp++;
						}
						if (SID && LSID) {
							char *secondStage =  (char* )mir_alloc( SSL_BUF_SIZE );
							nBytes = mir_snprintf(secondStage,SSL_BUF_SIZE,"%s&%s&service=mail&Session=true\r\n",SID,LSID);
							nBytes = mir_snprintf( buf, SSL_BUF_SIZE,
								"POST /%s HTTP/1.1\r\n"
								"Host: %s\r\n"
								"User-Agent: Miranda IM (JGmail "__VERSION_STRING")\r\n"
								"Content-Length: %d\r\n"
								"Content-Type: application/x-www-form-urlencoded\r\n"
								"Connection: Keep-Alive\r\n\r\n%s", "accounts/IssueAuthToken", tConn.szHost, nBytes, secondStage);
							mir_free(secondStage);
							//JabberLog( "Sending SSL query:\n%s", buf );
							pfn_SSL_write( ssl, buf, nBytes);
							nBytes = pfn_SSL_read( ssl, buf, SSL_BUF_SIZE-1 );
							if ( nBytes > 0 ) {
								buf[nBytes] = '\0'; //just in case
								if (buf[nBytes-1] == '\n') {buf[nBytes-1] = '\0'; nBytes--;}
								if (buf[nBytes-1] == '\r') {buf[nBytes-1] = '\0'; nBytes--;}
								responce = strchr(buf,' ')+1;
								temp = strchr(buf,'\n');
								temp[0] = '\0'; if (temp[-1] == '\r') temp[-1] = '\0'; // awful code!
								if (!strncmp(responce,"200",3)){
									JabberLog(respString,2,"posi",responce);
									temp++;
									temp = strrchr(temp,'\n');
									temp++;
									//JabberLog( "SSL result:\n%s", temp );
									result = ( char* )mir_alloc( strlen(temp)+1 );
									strcpy(result,temp);
								} else JabberLog(respString,2,"nega",responce);
							} else JabberLog( "SSL read failed: %d",nBytes);
						} else JabberLog("Server did not return SID and LSID");
					} else JabberLog(respString,1,"nega",responce);
				} else JabberLog( "SSL read failed: %d",nBytes);
				mir_free(buf);
			} else JabberLog( "SSL connection failed" );
		}else JabberLog( "NetLib did not provide valid socket" );
		pfn_SSL_free( ssl );
	} else JabberLog( "pfn_SSL_new failed" );
	JCallService( MS_NETLIB_CLOSEHANDLE, ( WPARAM )h, 0 );
	return result;
}

char * getXGoogleToken(char * email, char * passwd){
	char *data = (char *)mir_alloc(SSL_BUF_SIZE);
	char *temp = JabberHttpUrlEncode(email);
	int p = mir_snprintf(data,SSL_BUF_SIZE,"Email=%s&",temp);
	mir_free(temp);
	temp = JabberHttpUrlEncode(passwd);
	p += mir_snprintf(data+p,SSL_BUF_SIZE-p,"Passwd=%s&PersistentCookie=false&source=JGmail",temp);
	mir_free(temp);
	SSL_Base *pAgent;
#ifndef STATICSSL
	if (!hLibSSL) pAgent = new SSL_WinInet();
	      else
#endif			  
			  pAgent = new SSL_OpenSsl();
	if (pAgent->init()){
		delete pAgent;
		return NULL;
	}
	if (pAgent->getSslToken(data)){
		int len = strlen(email)+strlen(pAgent->result)+2;
		char * toEncode = (char *)mir_alloc(len+1);
		mir_snprintf(toEncode,len+1,"%c%s%c%s",'\0',email,'\0',pAgent->result);
		char * base64Encoded = JabberBase64Encode( toEncode, len );
		mir_free(toEncode);
		strncpy(data,base64Encoded,SSL_BUF_SIZE);
		mir_free(base64Encoded);
	} else data = NULL;
	delete pAgent;
	return data;
}

/////////////////////////////////////////////////////////////////////////////////////////
// X-Google-Token auth class

TGoogleAuth::TGoogleAuth( ThreadData* info ) :
	TJabberAuth( info )
{
	szName = "X-GOOGLE-TOKEN";
	bWasGoogleTokenRequested = false;
}

TGoogleAuth::~TGoogleAuth()
{
}


//char * getXGoogleToken(char * email, char * passwd){
char* TGoogleAuth::getInitialRequest()
{
	DBVARIANT dbv;
	char *result=0;
	char *temp = t2a(info->username);
	int size = strlen(temp)+1+strlen(info->server);
	char *localJid = (char *)mir_alloc(size+1);
	mir_snprintf(localJid,size+1,"%s@%s",temp,info->server);
	int res = JGetStringT(NULL,"GoogleToken",&dbv);
	if (!res) {
		int decodedLen;
		char *tokenDecoded = JabberBase64Decode(dbv.ptszVal, &decodedLen);
		char *jidFromToken = tokenDecoded;
		jidFromToken++;// first char is '\0' - some day this may change
		int notequal = strncmp(jidFromToken,localJid,size);
		mir_free(tokenDecoded);
		if(!notequal){
			result = t2a(dbv.ptszVal);
			JabberLog("Re-using previous GoogleToken");
		}
		JFreeVariant(&dbv);
		if(notequal) goto LBL_RequestToken;
	} else {
LBL_RequestToken:
		bWasGoogleTokenRequested = true; // new token is being requested
		result = getXGoogleToken(localJid,info->password);
		if (result) {
			JSetString(NULL, "GoogleToken", result);
		} else {
			if (!res) JDeleteSetting(NULL,"GoogleToken"); // we came here from goto LBL_RequestToken
			//res = ""; //Later will show auth failed
		}
	}
	mir_free(localJid);
	mir_free(temp);
	return result;
}
