/* Module:  imo_request.c
   Purpose: Posts XMLHHTP-Requests to imo.im server
   Author:  leecher
   Date:    30.08.2009
*/
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fifo.h"
#include "imo_request.h"
#include "io_layer.h"

#define SSID_LENGTH 16

struct _tagIMORQ
{
	IOLAYER *hIO;
	char szSessId[SSID_LENGTH+2];
    unsigned long send_ack;   // Some Sending ACK number
    unsigned long send_seq;   // Within one ACK there seems to be a SEQ-Number?
};

// Forward declaration of private functions
static size_t add_data(char *data, size_t size, size_t nmemb, void *ctx);
static IMORQ *Init(void);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

IMORQ *ImoRq_Init(void)
{

	IMORQ *hRq;

	if (hRq = Init())
	{
		/* Create session ID */
		ImoRq_CreateID (hRq->szSessId, SSID_LENGTH+1);

		/* Fetch start page to get cookies */
		if (IoLayer_Get (hRq->hIO, "https://o.imo.im/"))

		/* Get new session ID from system */
		{
			char *pszRPC = ImoRq_ResetRPC (hRq);
			if (pszRPC)
			{
				if (pszRPC = strstr(pszRPC, "ssid\":\""))
					strcpy (hRq->szSessId, strtok (pszRPC+7, "\""));
			}
		} else {
			ImoRq_Exit(hRq);
			hRq = NULL;
		}
	}

	return hRq;
}

// -----------------------------------------------------------------------------

IMORQ *ImoRq_Clone (IMORQ *hRq)
{
	IMORQ *hDup;

	if (!(hDup = Init())) return NULL;
	strcpy (hDup->szSessId, hRq->szSessId);
	return hDup;
}

// -----------------------------------------------------------------------------

void ImoRq_Exit (IMORQ *hRq)
{
	if (hRq->hIO) IoLayer_Exit(hRq->hIO);
	free (hRq);
}
// -----------------------------------------------------------------------------

char *ImoRq_PostImo(IMORQ *hRq, char *pszMethod, cJSON *data)
{
	TYP_FIFO *hPostString;
	char *pszData, *pszEscData;
	unsigned int uiCount = -1;

	if (!(pszData = cJSON_Print(data))) return NULL;
//printf ("-> %s\n", pszData);
#ifdef _WIN32
OutputDebugString (pszData);
OutputDebugString ("\n");
#endif
	pszEscData = IoLayer_EscapeString(hRq->hIO, pszData);
	free (pszData);
	if (!pszEscData || !(hPostString = Fifo_Init(strlen(pszEscData)+32)))
	{
		if (pszEscData) IoLayer_FreeEscapeString (pszEscData);
		return NULL;
	}
	Fifo_AddString (hPostString, "method=");
	Fifo_AddString (hPostString, pszMethod);
	Fifo_AddString (hPostString, "&data=");
	Fifo_AddString (hPostString, pszEscData);
	IoLayer_FreeEscapeString (pszEscData);
	pszEscData =  Fifo_Get(hPostString, &uiCount);
	pszData = IoLayer_Post (hRq->hIO, "https://o.imo.im/imo", pszEscData,
		uiCount-1);
	Fifo_Exit(hPostString);
printf ("<- %s\n", pszData);
	return pszData;
}

// -----------------------------------------------------------------------------

char *ImoRq_PostSystem(IMORQ *hRq, char *pszMethod, char *pszSysTo, char *pszSysFrom, cJSON *data, int bFreeData)
{
    cJSON *root, *msgs, *msg, *to, *from;
	char *pszRet;

    if (!(root=cJSON_CreateObject())) return NULL;
    cJSON_AddNumberToObject (root, "ack", hRq->send_ack);
    if (*hRq->szSessId) cJSON_AddStringToObject (root, "ssid", hRq->szSessId);
	else cJSON_AddNumberToObject (root, "ssid", 0);
    cJSON_AddItemToObject (root, "messages", (msgs = cJSON_CreateArray()));
	if (data)
	{
	    msg=cJSON_CreateObject();
	    cJSON_AddItemToObject(msg, "data", data);
	    to = cJSON_CreateObject();
	    cJSON_AddStringToObject (to, "system", pszSysTo);
	    cJSON_AddItemToObject(msg, "to", to);
	    from = cJSON_CreateObject();
	    cJSON_AddStringToObject (from, "system", pszSysFrom);
		if (*hRq->szSessId) cJSON_AddStringToObject (from, "ssid", hRq->szSessId);
		else cJSON_AddNumberToObject (from, "ssid", 0);
	    cJSON_AddItemToObject(msg, "from", from);
	    cJSON_AddNumberToObject (msg, "seq", hRq->send_seq++);
	    cJSON_AddItemToArray (msgs, msg);
	}
    pszRet = ImoRq_PostImo (hRq, pszMethod, root);
	if (data && !bFreeData)
	{
		msg->child = data->next;
		data->next = NULL;
	}
	cJSON_Delete (root);
	return pszRet;
}

// -----------------------------------------------------------------------------

char *ImoRq_ResetRPC(IMORQ *hRq)
{
	cJSON *root, *ssid;
	char *pszRet;

	if (!(root=cJSON_CreateObject())) return NULL;
	cJSON_AddStringToObject (root, "method", "get_ssid");
	ssid=cJSON_CreateObject();
	cJSON_AddStringToObject (ssid, "ssid", hRq->szSessId);
	cJSON_AddItemToObject(root, "data", ssid);
	*hRq->szSessId = 0;
    pszRet = ImoRq_PostSystem (hRq, "rest_rpc", "ssid", "client", root, 1);
	hRq->send_seq=0;
	return pszRet;
}

// -----------------------------------------------------------------------------

char *ImoRq_UserActivity(IMORQ *hRq)
{
	cJSON *ssid;

	ssid=cJSON_CreateObject();
	cJSON_AddStringToObject (ssid, "ssid", hRq->szSessId);
	return ImoRq_PostToSys (hRq, "observed_user_activity", "session", ssid, 1);
}

// -----------------------------------------------------------------------------

char *ImoRq_Echo(IMORQ *hRq)
{
	cJSON *data;
	time_t t;
	char szTime[16], *pszRet;

	if (!(data=cJSON_CreateObject())) return NULL;
	sprintf (szTime, "%ld", time(&t)*1000);
	cJSON_AddStringToObject (data, "t", szTime);
	pszRet = ImoRq_PostImo (hRq, "echo", data);
	cJSON_Delete (data);
	return pszRet;
}

// -----------------------------------------------------------------------------

char *ImoRq_Reui_Session(IMORQ *hRq)
{
	cJSON *ssid;

	ssid=cJSON_CreateObject();
	cJSON_AddStringToObject (ssid, "ssid", hRq->szSessId);
	return ImoRq_PostToSys (hRq, "reui_session", "session", ssid, 1);
}

// -----------------------------------------------------------------------------

char *ImoRq_PostToSys(IMORQ *hRq, char *pszMethod, char *pszSysTo, cJSON *data, int bFreeData)
{
	cJSON *root;
	char *pszRet;

	if (!(root=cJSON_CreateObject())) return NULL;
	cJSON_AddStringToObject (root, "method", pszMethod);
	cJSON_AddItemToObject(root, "data", data);
	pszRet = ImoRq_PostSystem (hRq, "forward_to_server", pszSysTo, "client", root, bFreeData);
	if (!bFreeData)
	{
		data->prev->next = data->next;
		if (data->next) data->next->prev = data->prev;
		data->prev = data->next = NULL;
		cJSON_Delete (root);
	}
	return pszRet;
}
// -----------------------------------------------------------------------------

char *ImoRq_PostAmy(IMORQ *hRq, char *pszMethod, cJSON *data)
{
	return ImoRq_PostToSys (hRq, pszMethod, "im", data, FALSE);
}

// -----------------------------------------------------------------------------

char *ImoRq_SessId(IMORQ *hRq)
{
	return hRq->szSessId;
}

// -----------------------------------------------------------------------------

char *ImoRq_GetLastError(IMORQ *hRq)
{
	return IoLayer_GetLastError (hRq->hIO);
}

// -----------------------------------------------------------------------------
void ImoRq_UpdateAck(IMORQ *hRq, unsigned long lAck)
{
	hRq->send_ack = lAck;
}
// -----------------------------------------------------------------------------
unsigned long ImoRq_GetSeq(IMORQ *hRq)
{
	return hRq->send_seq;
}
// -----------------------------------------------------------------------------

void ImoRq_CreateID(char *pszID, int cbID)
{
	int i, r;
	time_t curtime;

	srand(time(&curtime));
	for (i=0; i<cbID; i++)
	{
		r = rand()%62;
		if (r<26) pszID[i]='A'+r; else
		if (r<52) pszID[i]='a'+(r-26); else
		pszID[i]='0'+(r-52);
	}
	pszID[i]=0;
	return;
}

// -----------------------------------------------------------------------------

static IMORQ *Init(void)
{
	IMORQ *hRq = calloc(1, sizeof(IMORQ));

	/* Setup CURL */
	if (!hRq) return NULL;
	if (!(hRq->hIO = IoLayer_Init()))
	{
		ImoRq_Exit(hRq);
		return NULL;
	}
	return hRq;
}

