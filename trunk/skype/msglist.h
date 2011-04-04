#include <time.h>

typedef struct {
	DWORD uMsgNum;
	HANDLE hEvent;
	time_t t;
	time_t tEdited;
} TYP_MSGLENTRY;

void MsgList_Init(void);
void MsgList_Exit(void);
BOOL MsgList_Add(DWORD uMsgNum, HANDLE hEvent);
TYP_MSGLENTRY *MsgList_FindMessage(DWORD uMsgNum);
void MsgList_CollectGarbage(void);
