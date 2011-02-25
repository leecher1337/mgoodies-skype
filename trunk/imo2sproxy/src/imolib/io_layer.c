/* Module:  io_layer.c
   Purpose: IO Layer for Internet communication using libcurl
   Author:  leecher
   Date:    30.08.2009
*/
#include <curl/curl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "fifo.h"
#include "io_layer.h"

struct _tagIOLAYER
{
	CURL *hCurl;
	TYP_FIFO *hResult;
	char szErrorBuf[CURL_ERROR_SIZE+1];
};

static size_t add_data(char *data, size_t size, size_t nmemb, void *ctx);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

IOLAYER *IoLayer_Init(void)
{
	IOLAYER *hIO;
	
	if (!(hIO = calloc(1, sizeof(IOLAYER))))
		return NULL;
		
	if (!(hIO->hCurl = curl_easy_init()))
	{
		free (hIO);
		return NULL;
	}
	
	if (!(hIO->hResult = Fifo_Init(1024)))
	{
		IoLayer_Exit(hIO);
		return NULL;
	}

	curl_easy_setopt(hIO->hCurl, CURLOPT_USERAGENT, "XMLHttpRequest/1.0");
	curl_easy_setopt(hIO->hCurl, CURLOPT_HEADER, 0);
	curl_easy_setopt(hIO->hCurl, CURLOPT_AUTOREFERER, 1);
//	curl_easy_setopt(hRq->hCurl, CURLOPT_RETURNTRANSFER, 1);
	curl_easy_setopt(hIO->hCurl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(hIO->hCurl, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(hIO->hCurl, CURLOPT_ERRORBUFFER, hIO->szErrorBuf);
	curl_easy_setopt(hIO->hCurl, CURLOPT_COOKIEFILE, "cookies.txt");
	curl_easy_setopt(hIO->hCurl, CURLOPT_COOKIEJAR, "cookies.txt");
	curl_easy_setopt(hIO->hCurl, CURLOPT_WRITEFUNCTION, add_data);
	curl_easy_setopt(hIO->hCurl, CURLOPT_FILE, hIO);
	
	return hIO;
}

// -----------------------------------------------------------------------------

void IoLayer_Exit (IOLAYER *hIO)
{
	if (hIO->hCurl) curl_easy_cleanup(hIO->hCurl);
	if (hIO->hResult) Fifo_Exit(hIO->hResult);
	free (hIO);
}

// -----------------------------------------------------------------------------

char *IoLayer_Post(IOLAYER *hIO, char *pszUrl, char *pszPostFields, unsigned int cbPostFields)
{
	curl_easy_setopt(hIO->hCurl, CURLOPT_POST, 1);
	curl_easy_setopt(hIO->hCurl, CURLOPT_URL, pszUrl);
	curl_easy_setopt(hIO->hCurl, CURLOPT_POSTFIELDS, pszPostFields);
	curl_easy_setopt(hIO->hCurl, CURLOPT_POSTFIELDSIZE, cbPostFields);
	if (curl_easy_perform(hIO->hCurl)) return NULL;
	Fifo_Add (hIO->hResult, "", 1);
	return Fifo_Get (hIO->hResult, NULL);
}

// -----------------------------------------------------------------------------

char *IoLayer_Get(IOLAYER *hIO, char *pszUrl)
{
	curl_easy_setopt(hIO->hCurl, CURLOPT_POST, 0);
	curl_easy_setopt(hIO->hCurl, CURLOPT_URL, pszUrl);
	if (curl_easy_perform(hIO->hCurl)) return NULL;
	Fifo_Add (hIO->hResult, "", 1);
	return Fifo_Get (hIO->hResult, NULL);
}

// -----------------------------------------------------------------------------

char *IoLayer_GetLastError(IOLAYER *hIO)
{
	return hIO->szErrorBuf;
}

// -----------------------------------------------------------------------------

char *IoLayer_EscapeString(IOLAYER *hIO, char *pszData)
{
	return curl_easy_escape(hIO->hCurl, pszData, 0);
}

// -----------------------------------------------------------------------------

void IoLayer_FreeEscapeString(char *pszData)
{
	curl_free(pszData);
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------

static size_t add_data(char *data, size_t size, size_t nmemb, void *ctx) 
{
	IOLAYER *hIO = (IOLAYER*)ctx;

	if (Fifo_Add(hIO->hResult, data, size * nmemb)) return size * nmemb;
	return 0;
}

