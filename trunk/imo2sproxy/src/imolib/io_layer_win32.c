/* Module:  io_layer_win32.c
   Purpose: IO Layer for Internet communication using WININET (Win32)
   Author:  leecher
   Date:    30.08.2009
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "fifo.h"
#include "io_layer.h"

#pragma comment(lib,"wininet.lib")

#define INET_FLAGS INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | \
		INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD  | INTERNET_FLAG_NO_UI | \
		INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_SECURE

typedef struct
{
	IOLAYER vtbl;
	HINTERNET hInet;
	HINTERNET hRequest;
	TYP_FIFO *hResult;
	LPVOID lpErrorBuf;
} IOLAYER_INST;

static void IoLayer_Exit (IOLAYER *hPIO);
static char *IoLayer_Post(IOLAYER *hPIO, char *pszURL, char *pszPostFields, unsigned int cbPostFields, unsigned int *pdwLength);
static char *IoLayer_Get(IOLAYER *hIO, char *pszURL, unsigned int *pdwLength);
static void IoLayer_Cancel(IOLAYER *hIO);
static char *IoLayer_GetLastError(IOLAYER *hIO);
static char *IoLayer_EscapeString(IOLAYER *hPIO, char *pszData);
static void IoLayer_FreeEscapeString(char *pszData);
static void FetchLastError (IOLAYER_INST *hIO);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

IOLAYER *IoLayerW32_Init(void)
{
	IOLAYER_INST *hIO;
	
	if (!(hIO = calloc(1, sizeof(IOLAYER_INST))))
		return NULL;
		
	// Init Inet
	if (!(hIO->hInet = InternetOpen ("Mozilla/5.0 (Windows; U; Windows NT 5.1; de; rv:1.9.2.13) Gecko/20101203 Firefox/3.6.13",
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, "<local>", 0)))
	{
		free (hIO);
		return NULL;
	}
	
	if (!(hIO->hResult = Fifo_Init(1024)))
	{
		IoLayer_Exit((IOLAYER*)hIO);
		return NULL;
	}

	//InternetSetCookie ("https://o.imo.im/", "proto", "prpl-skype");

	// Init Vtbl
	hIO->vtbl.Exit = IoLayer_Exit;
	hIO->vtbl.Post = IoLayer_Post;
	hIO->vtbl.Get = IoLayer_Get;
	hIO->vtbl.Cancel = IoLayer_Cancel;
	hIO->vtbl.GetLastError = IoLayer_GetLastError;
	hIO->vtbl.EscapeString = IoLayer_EscapeString;
	hIO->vtbl.FreeEscapeString = IoLayer_FreeEscapeString;

	return (IOLAYER*)hIO;
}
// -----------------------------------------------------------------------------

static void IoLayer_Exit (IOLAYER *hPIO)
{
	IOLAYER_INST *hIO = (IOLAYER_INST*)hPIO;

	if (hIO->hInet) InternetCloseHandle (hIO->hInet);
	if (hIO->lpErrorBuf) LocalFree(hIO->lpErrorBuf);
	if (hIO->hResult) Fifo_Exit(hIO->hResult);
	free (hIO);
}

// -----------------------------------------------------------------------------

static char *IoLayer_Post(IOLAYER *hPIO, char *pszURL, char *pszPostFields, unsigned int cbPostFields, unsigned int *pdwLength)
{
	IOLAYER_INST *hIO = (IOLAYER_INST*)hPIO;
	URL_COMPONENTS urlInfo = {0};
	HINTERNET hUrl;
	DWORD dwFlags = 0, cbFlags = sizeof(dwFlags), dwRemaining = 0;
	char szHostName[INTERNET_MAX_HOST_NAME_LENGTH],
		szURLPath[INTERNET_MAX_URL_LENGTH], *p;

//OutputDebugString(pszPostFields);
	urlInfo.dwStructSize = sizeof (URL_COMPONENTS);
	urlInfo.lpszHostName = szHostName;
	urlInfo.dwHostNameLength = sizeof(szHostName);
	urlInfo.lpszUrlPath = szURLPath;
	urlInfo.dwUrlPathLength = sizeof(szURLPath);
	if (!InternetCrackUrl(pszURL, strlen(pszURL), 0, &urlInfo)) return NULL;
	/*
	if (!pszPostFields)
	{
		if (pszPostFields=strchr (pszURL, '?'))
			cbPostFields = strlen(pszPostFields);
	}
	*/

	if (!(hUrl = InternetConnect (hIO->hInet, szHostName, 
		INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0)))
	{
		FetchLastError (hIO);
		return NULL;
	}
	
	hIO->hRequest = HttpOpenRequest (hUrl, pszPostFields?"POST":"GET", szURLPath, NULL, NULL, NULL, 
		INET_FLAGS, 0);
	if (!hIO->hRequest)
	{
		FetchLastError (hIO);
		InternetCloseHandle (hUrl);
		return NULL;
	}
	
	InternetQueryOption (hIO->hRequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &cbFlags); 
	dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
	InternetSetOption (hIO->hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof (dwFlags));

	/*
	{
		char szCookies[4096];
		DWORD cbCookies, dwIndex=0;

		OutputDebugString ("Sending headers:\n");
		do
		{
			cbCookies=sizeof(szCookies);
			HttpQueryInfo (hIO->hRequest, HTTP_QUERY_FLAG_REQUEST_HEADERS|HTTP_QUERY_RAW_HEADERS_CRLF, szCookies, &cbCookies, &dwIndex);
			OutputDebugString (szCookies);
		} while (GetLastError() == ERROR_SUCCESS);
	}
	*/

	if (!(HttpSendRequest (hIO->hRequest, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n"
		"X-Requested-With: XMLHttpRequest", -1,
		pszPostFields, cbPostFields)))
	{
		FetchLastError (hIO);
		InternetCloseHandle (hIO->hRequest);
		hIO->hRequest = NULL;
		InternetCloseHandle (hUrl);
		return NULL;
	}

	/*
	{
		char szCookies[4096];
		DWORD cbCookies, dwIndex=0;

		OutputDebugString ("Received headers:\n");
		do
		{
			cbCookies=sizeof(szCookies);
			HttpQueryInfo (hIO->hRequest, HTTP_QUERY_FLAG_REQUEST_HEADERS|HTTP_QUERY_RAW_HEADERS_CRLF, szCookies, &cbCookies, &dwIndex);
			OutputDebugString (szCookies);
		} while (GetLastError() == ERROR_SUCCESS);
	}
	*/


	while (InternetQueryDataAvailable (hIO->hRequest, &dwRemaining, 0, 0) && dwRemaining > 0)
	{
		if (p = Fifo_AllocBuffer (hIO->hResult, dwRemaining))
			InternetReadFile (hIO->hRequest, p, dwRemaining, &dwRemaining);
	}
	if (!pdwLength)
	{
		// Get string
		Fifo_Add (hIO->hResult, "", 1);
		p = Fifo_Get (hIO->hResult, NULL);
	}
	else
	{
		// Get binary, return size of buffer
		*pdwLength = (unsigned int)-1;
		p = Fifo_Get (hIO->hResult, pdwLength);
	}
	InternetCloseHandle (hIO->hRequest);
	hIO->hRequest = NULL;
	InternetCloseHandle (hUrl);
OutputDebugString(p);
OutputDebugString("\n");
	return p;
}

// -----------------------------------------------------------------------------

static char *IoLayer_Get(IOLAYER *hIO, char *pszURL, unsigned int *pdwLength)
{
	return IoLayer_Post (hIO, pszURL, NULL, 0, pdwLength);
}

// -----------------------------------------------------------------------------

static void IoLayer_Cancel(IOLAYER *hPIO)
{
	IOLAYER_INST *hIO = (IOLAYER_INST*)hPIO;

	if (hIO->hRequest && InternetCloseHandle(hIO->hRequest))
		hIO->hRequest = NULL;
}

// -----------------------------------------------------------------------------

static char *IoLayer_GetLastError(IOLAYER *hIO)
{
	return (char*)((IOLAYER_INST*)hIO)->lpErrorBuf;
}

// -----------------------------------------------------------------------------

static char *IoLayer_EscapeString(IOLAYER *hPIO, char *pszData)
{
	IOLAYER_INST *hIO = (IOLAYER_INST*)hPIO;
	TYP_FIFO *hFifo;
	char szBuf[8], *pszRet;
	unsigned char *p;

	if (!(hFifo = Fifo_Init(strlen(pszData)))) return NULL;
	for (p=pszData; *p; p++)
	{
		if (isalnum(*p)) Fifo_Add (hFifo, p, 1);
		else {
			wsprintf (szBuf, "%%%02X", *p);
			Fifo_Add (hFifo, szBuf, 3);
		}
	}
	Fifo_Add (hFifo, "", 1);
	if (pszRet = Fifo_Get(hFifo, NULL))
		pszRet = strdup(pszRet);
	Fifo_Exit(hFifo);
	return pszRet;
}

// -----------------------------------------------------------------------------

static void IoLayer_FreeEscapeString(char *pszData)
{
	free (pszData);
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------

static void FetchLastError (IOLAYER_INST *hIO)
{
	if (hIO->lpErrorBuf) LocalFree(hIO->lpErrorBuf);
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
		GetLastError(), MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
		(LPTSTR)&hIO->lpErrorBuf, 0, NULL);
}
