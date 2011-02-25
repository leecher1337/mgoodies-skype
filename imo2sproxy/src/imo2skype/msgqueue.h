#include "cJSON.h"
#include "queue.h"
#include <time.h>

typedef struct
{
	QUEUEHDR hdr;
	char *pszUser;
	char *pszAlias;
	char *pszMessage;
	time_t timestamp;
	char szStatus[16];
	char szFailure[256];
} MSGENTRY;

TYP_LIST *MsgQueue_Init(void);
void MsgQueue_Exit(TYP_LIST *hList);

MSGENTRY *MsgQueue_Insert(TYP_LIST *hList, cJSON *pNick);
MSGENTRY *MsgQueue_AddReflect(TYP_LIST *hList, cJSON *pNick, TYP_LIST *hBuddyList);
MSGENTRY *MsgQueue_AddSent(TYP_LIST *hList, char *pszUser, char *pszAlias, char *pszMessage, unsigned int *puMsgId);
BOOL MsgQueue_Remove(TYP_LIST *hList, unsigned int uMsgNr);
MSGENTRY *MsgQueue_Find(TYP_LIST *hList, unsigned int uMsgNr);
