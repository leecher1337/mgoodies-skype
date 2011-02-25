#include "cJSON.h"
#include "memlist.h"

typedef struct
{
	char *pszUser;
	char *pszStatusText;
	char szStatus[16];
	char *pszAlias;
	int  iBuddyStatus;
} NICKENTRY;

TYP_LIST *BuddyList_Init(void);
void BuddyList_Exit(TYP_LIST *hList);

BOOL BuddyList_Insert(TYP_LIST *hList, cJSON *pNick);
BOOL BuddyList_AddTemporaryUser(TYP_LIST *hList, char *pszUser);
BOOL BuddyList_Remove(TYP_LIST *hList, NICKENTRY *pEntry);
NICKENTRY *BuddyList_Find(TYP_LIST *hList, char *pszUser);
BOOL BuddyList_SetStatus(TYP_LIST *hList, cJSON *pNick);
void BuddyList_FreeEntry(NICKENTRY *pEntry);
