/* Module:  msgqueue.c
   Purpose: Message queue for incoming messages
   Author:  leecher
   Date:    30.08.2009

   Fixme:   Sort on insert, do a binary search instead of iterating list.
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "msgqueue.h"
#include "buddylist.h"

static volatile unsigned int m_uMsgNr=0;
static void FreeEntry(void *pEntry);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

TYP_LIST *MsgQueue_Init(void)
{
	TYP_LIST *hList = List_Init(16);

	return hList;
}

// -----------------------------------------------------------------------------

void MsgQueue_Exit(TYP_LIST *hList)
{
	Queue_Exit(hList, FreeEntry);
}

// -----------------------------------------------------------------------------

MSGENTRY *MsgQueue_Insert(TYP_LIST *hList, cJSON *pNick)
{
	MSGENTRY *pEntry;

	if (!(pEntry = Queue_InsertEntry(hList, sizeof(MSGENTRY), ++m_uMsgNr, 
		FreeEntry))) return NULL;
	pEntry->pszUser = strdup(cJSON_GetObjectItem(pNick, "buid")->valuestring);
	pEntry->pszAlias = strdup(cJSON_GetObjectItem(pNick, "alias")->valuestring);
	pEntry->pszMessage = strdup(cJSON_GetObjectItem(pNick, "msg")->valuestring);
	pEntry->timestamp = cJSON_GetObjectItem(pNick, "timestamp")->valueint;
	strcpy (pEntry->szStatus, "RECEIVED");
	return pEntry;
}

// -----------------------------------------------------------------------------

MSGENTRY *MsgQueue_AddReflect(TYP_LIST *hList, cJSON *pNick, TYP_LIST *hBuddyList)
{
	MSGENTRY *pEntry;
	cJSON *pVal;

	if (!(pEntry = Queue_InsertEntry(hList, sizeof(MSGENTRY), ++m_uMsgNr,
		FreeEntry))) return NULL;
	pEntry->pszUser = strdup(cJSON_GetObjectItem(pNick, "buid")->valuestring);

	// Usually no alias in Reflection, so query buddy list, if available
	if (pVal = cJSON_GetObjectItem(pNick, "alias"))
		pEntry->pszAlias = strdup(pVal->valuestring);
	else
	{
		NICKENTRY *pBuddy = hBuddyList?BuddyList_Find (hBuddyList, pEntry->pszUser):NULL;
		pEntry->pszAlias = strdup (pBuddy?pBuddy->pszAlias:pEntry->pszUser);
	}

	pEntry->pszMessage = strdup(cJSON_GetObjectItem(pNick, "msg")->valuestring);
	pEntry->timestamp = cJSON_GetObjectItem(pNick, "timestamp")->valueint;
	strcpy (pEntry->szStatus, "SENT");
	return pEntry;
}

// -----------------------------------------------------------------------------

MSGENTRY *MsgQueue_AddSent(TYP_LIST *hList, char *pszUser, char *pszAlias, char *pszMessage, unsigned int *puMsgId)
{
	MSGENTRY *pEntry;

	if (!(pEntry = Queue_InsertEntry(hList, sizeof(MSGENTRY), ++m_uMsgNr,
		FreeEntry))) return NULL;
	pEntry->pszUser = strdup(pszUser);
	pEntry->pszAlias = strdup(pszAlias);
	pEntry->pszMessage = strdup(pszMessage);
	time (&pEntry->timestamp);
	strcpy (pEntry->szStatus, "SENDING");
	if (puMsgId) *puMsgId = pEntry->hdr.uMsgNr;
	return pEntry;
}

// -----------------------------------------------------------------------------

BOOL MsgQueue_Remove(TYP_LIST *hList, unsigned int uMsgNr)
{
	return Queue_Remove(hList, uMsgNr, FreeEntry);
}

// -----------------------------------------------------------------------------

MSGENTRY *MsgQueue_Find(TYP_LIST *hList, unsigned int uMsgNr)
{
	return (MSGENTRY*)Queue_Find(hList, uMsgNr);
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------

static void FreeEntry(void *pPEntry)
{
	MSGENTRY *pEntry = (MSGENTRY*)pPEntry;
	
	if (pEntry->pszAlias) free (pEntry->pszAlias);
	free (pEntry->pszUser);
	free (pEntry->pszMessage);
}

