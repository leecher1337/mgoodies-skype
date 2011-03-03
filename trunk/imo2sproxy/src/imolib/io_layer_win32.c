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

struct _tagIOLAYER
{
	HINTERNET hInet;
	TYP_FIFO *hResult;
	LPVOID lpErrorBuf;
};

static void FetchLastError (IOLAYER *hIO);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

IOLAYER *IoLayer_Init(void)
{
	IOLAYER *hIO;
	
	if (!(hIO = calloc(1, sizeof(IOLAYER))))
		return NULL;
		
	if (!(hIO->hInet = InternetOpen ("Mozilla/5.0 (Windows; U; Windows NT 5.1; de; rv:1.9.2.13) Gecko/20101203 Firefox/3.6.13",
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, "<local>", 0)))
	{
		free (hIO);
		return NULL;
	}
	
	if (!(hIO->hResult = Fifo_Init(1024)))
	{
		IoLayer_Exit(hIO);
		return NULL;
	}

	//InternetSetCookie ("https://o.imo.im/", "proto", "prpl-skype");

	return hIO;
}
// -----------------------------------------------------------------------------

void IoLayer_Exit (IOLAYER *hIO)
{
	if (hIO->hInet) InternetCloseHandle (hIO->hInet);
	if (hIO->lpErrorBuf) LocalFree(hIO->lpErrorBuf);
	if (hIO->hResult) Fifo_Exit(hIO->hResult);
	free (hIO);
}

// -----------------------------------------------------------------------------

char *IoLayer_Post(IOLAYER *hIO, char *pszURL, char *pszPostFields, unsigned int cbPostFields)
{
	URL_COMPONENTS urlInfo = {0};
	HINTERNET hUrl;
	HINTERNET hRequest;
	DWORD dwFlags = 0, cbFlags = sizeof(dwFlags), dwLength = 512, dwRemaining = 0;
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
	
	hRequest = HttpOpenRequest (hUrl, pszPostFields?"POST":"GET", szURLPath, NULL, NULL, NULL, 
		INET_FLAGS, 0);
	if (!hRequest)
	{
		FetchLastError (hIO);
		InternetCloseHandle (hUrl);
		return NULL;
	}
	
	InternetQueryOption (hRequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &cbFlags); 
	dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
	InternetSetOption (hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof (dwFlags));

	/*
	{
		char szCookies[4096];
		DWORD cbCookies, dwIndex=0;

		OutputDebugString ("Sending headers:\n");
		do
		{
			cbCookies=sizeof(szCookies);
			HttpQueryInfo (hRequest, HTTP_QUERY_FLAG_REQUEST_HEADERS|HTTP_QUERY_RAW_HEADERS_CRLF, szCookies, &cbCookies, &dwIndex);
			OutputDebugString (szCookies);
		} while (GetLastError() == ERROR_SUCCESS);
	}
	*/

	if (!(HttpSendRequest (hRequest, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n"
		"X-Requested-With: XMLHttpRequest", -1,
		pszPostFields, cbPostFields)))
	{
		FetchLastError (hIO);
		InternetCloseHandle (hRequest);
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
			HttpQueryInfo (hRequest, HTTP_QUERY_FLAG_REQUEST_HEADERS|HTTP_QUERY_RAW_HEADERS_CRLF, szCookies, &cbCookies, &dwIndex);
			OutputDebugString (szCookies);
		} while (GetLastError() == ERROR_SUCCESS);
	}
	*/


	while (InternetQueryDataAvailable (hRequest, &dwRemaining, 0, 0) && dwRemaining > 0)
	{
		if (p = Fifo_AllocBuffer (hIO->hResult, dwRemaining))
			InternetReadFile (hRequest, p, dwRemaining, &dwRemaining);
	}
	Fifo_Add (hIO->hResult, "", 1);
	p = Fifo_Get (hIO->hResult, NULL);
	InternetCloseHandle (hRequest);
	InternetCloseHandle (hUrl);
OutputDebugString(p);
OutputDebugString("\n");
	return p;
}

// -----------------------------------------------------------------------------

char *IoLayer_Get(IOLAYER *hIO, char *pszURL)
{
	return IoLayer_Post (hIO, pszURL, NULL, 0);
}

// -----------------------------------------------------------------------------

char *IoLayer_GetLastError(IOLAYER *hIO)
{
	return (char*)hIO->lpErrorBuf;
}

// -----------------------------------------------------------------------------

char *IoLayer_EscapeString(IOLAYER *hIO, char *pszData)
{
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

void IoLayer_FreeEscapeString(char *pszData)
{
	free (pszData);
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------

static void FetchLastError (IOLAYER *hIO)
{
	if (hIO->lpErrorBuf) LocalFree(hIO->lpErrorBuf);
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
		GetLastError(), MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
		(LPTSTR)&hIO->lpErrorBuf, 0, NULL);
}
