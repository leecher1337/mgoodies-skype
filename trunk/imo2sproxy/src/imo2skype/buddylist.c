/* Module:  buddylist.c
   Purpose: Manages your list of buddies in memory
   Author:  leecher
   Date:    30.08.2009
*/

#include <stdlib.h>
#include <string.h>
#include "memlist.h"
#include "buddylist.h"

static void SetEntry(NICKENTRY *pEntry, cJSON *pNick);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

TYP_LIST *BuddyList_Init(void)
{
	TYP_LIST *hList = List_Init(16);

	return hList;
}

// -----------------------------------------------------------------------------

void BuddyList_Exit(TYP_LIST *hList)
{
	NICKENTRY *pEntry;

	while (pEntry=(NICKENTRY*)List_Pop(hList))
	{
		BuddyList_FreeEntry(pEntry);
		free (pEntry);
	}
	List_Exit(hList);
}

// -----------------------------------------------------------------------------

BOOL BuddyList_Insert(TYP_LIST *hList, cJSON *pNick)
{
	NICKENTRY *pEntry;

	if (pEntry=BuddyList_Find (hList, cJSON_GetObjectItem(pNick, "buid")->valuestring))
		BuddyList_FreeEntry (pEntry);
	else
	{
		if (!(pEntry = calloc (1, sizeof(NICKENTRY)))) return FALSE;
		if (!List_Push(hList, pEntry)) return FALSE;
	}
	SetEntry(pEntry, pNick);
	pEntry->iBuddyStatus = 3;
	return TRUE;
}

// -----------------------------------------------------------------------------

BOOL BuddyList_AddTemporaryUser(TYP_LIST *hList, char *pszUser)
{
	NICKENTRY *pEntry;
	
	if (BuddyList_Find (hList, pszUser)) return TRUE;
	if (!(pEntry = calloc (1, sizeof(NICKENTRY)))) return FALSE;
	pEntry->pszUser = strdup(pszUser);
	pEntry->pszAlias = strdup(pszUser);
	strcpy (pEntry->szStatus, "OFFLINE");
	pEntry->iBuddyStatus = 2;
	return List_Push(hList, pEntry);
}

// -----------------------------------------------------------------------------

BOOL BuddyList_Remove(TYP_LIST *hList, NICKENTRY *pEntry)
{
	NICKENTRY *pListEntry;
	int i, nCount;

	for (i=0, nCount=List_Count(hList); i<nCount; i++)
	{
		pListEntry = List_ElementAt (hList, i);
		if (pListEntry == pEntry) break;
	}
	if (i<nCount)
	{
		BuddyList_FreeEntry (pEntry);
		List_RemoveElementAt(hList, i);
		free (pEntry);
		return TRUE;
	}
	return FALSE;
}

// -----------------------------------------------------------------------------

NICKENTRY *BuddyList_Find(TYP_LIST *hList, char *pszUser)
{
	int i, nCount;
	NICKENTRY *pEntry;

	for (i=0, nCount=List_Count(hList); i<nCount; i++)
	{
		pEntry = List_ElementAt (hList, i);
		if (strcmp(pEntry->pszUser, pszUser) == 0)
			return pEntry;
	}
	return NULL;
}

// -----------------------------------------------------------------------------

BOOL BuddyList_SetStatus(TYP_LIST *hList, cJSON *pNick)
{
	NICKENTRY *pEntry;

	if ((pEntry = BuddyList_Find(hList,  cJSON_GetObjectItem(pNick, "buid")->valuestring)))
	{
		BuddyList_FreeEntry(pEntry);
		SetEntry(pEntry, pNick);
		return TRUE;
	}
	return FALSE;
}

// -----------------------------------------------------------------------------

void BuddyList_FreeEntry(NICKENTRY *pEntry)
{
	if (pEntry->pszAlias) free (pEntry->pszAlias);
	if (pEntry->pszUser) free (pEntry->pszUser);
	if (pEntry->pszStatusText) free(pEntry->pszStatusText);
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------

static void SetEntry(NICKENTRY *pEntry, cJSON *pNick)
{
	pEntry->pszAlias = strdup(cJSON_GetObjectItem(pNick, "alias")->valuestring);
	pEntry->pszUser = strdup(cJSON_GetObjectItem(pNick, "buid")->valuestring);
	pEntry->pszStatusText = cJSON_GetObjectItem(pNick, "status")->valuestring;
	if (pEntry->pszStatusText) pEntry->pszStatusText = strdup(pEntry->pszStatusText);
	strcpy (pEntry->szStatus, cJSON_GetObjectItem(pNick, "primitive")->valuestring);
}
