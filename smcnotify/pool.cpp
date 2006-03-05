#include "pool.h"


// Prototypes ///////////////////////////////////////////////////////////////////////////

static CRITICAL_SECTION update_cs;

typedef struct
{
	SortedList *queue;
	HANDLE hThread;
	DWORD dwThreadID;
	BOOL bThreadRunning;

} ThreadQueue;

ThreadQueue statusQueue;


struct QueueItem
{
	HANDLE hContact;
	DWORD check_time;
};

void QueueAdd(HANDLE hContact, DWORD check_time, BOOL prio);
void QueueRemove(HANDLE hContact);
int QueueCompareItems(void *i1, void *i2);
int QueueSortItems(void *i1, void *i2);

BOOL ProtocolStatusAllowMsgs(HANDLE hContact, const char *protocol);


// Functions ////////////////////////////////////////////////////////////////////////////


void InitPool() 
{
	int queuesize = CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);

	// Init queue
	ZeroMemory(&statusQueue, sizeof(statusQueue));
	statusQueue.queue = List_Create(queuesize + 10, 10);
	statusQueue.queue->sortFunc = QueueSortItems;
	statusQueue.queue->compareFunc = QueueCompareItems;
	statusQueue.bThreadRunning = TRUE;
	statusQueue.hThread = CreateThread(NULL, 16000, UpdateThread, NULL, 0, &statusQueue.dwThreadID);

	InitializeCriticalSection(&update_cs);
}


void FreePool()
{
	DWORD dwExitcode;
	int steps;

	// Stop queue
	steps = 0;
	statusQueue.bThreadRunning = FALSE;
	ResumeThread(statusQueue.hThread);
	do {
		Sleep(100);
		GetExitCodeThread(statusQueue.hThread, &dwExitcode);
		steps++;
	} while ( dwExitcode == STILL_ACTIVE && steps < 20 );
	if (statusQueue.hThread)
		CloseHandle(statusQueue.hThread);

	// Delete cs
	DeleteCriticalSection(&update_cs);

	// Free lists
	List_DestroyFreeContents(statusQueue.queue);
	mir_free(statusQueue.queue);
}

// Return true if this protocol has to be checked
BOOL PoolCheckProtocol(const char *protocol)
{
	char setting[256];

	if (!options.pool_check_msgs || protocol == NULL)
		return FALSE;

	mir_snprintf(setting, sizeof(setting), "%sCheckMsg", protocol);

	return (BOOL) DBGetContactSettingByte(NULL, MODULE_NAME, setting, FALSE);
}


// Return true if this contact has to be checked
BOOL PoolCheckContact(HANDLE hContact)
{
	return (BOOL) DBGetContactSettingByte(hContact, MODULE_NAME, OPT_CONTACT_GETMSG, TRUE);
}


// Add a contact to the pool when the status of the contact has changed
void PoolStatusChangeAddContact(HANDLE hContact, const char* protocol)
{
	// Delete some messages now (don't add to list...)
	if (ProtocolStatusAllowMsgs(hContact, proto))
	{
		QueueAdd(hContact, GetTickCount() + options.pool_timer_status * 1000, TRUE);
	}
	else
	{
		DBWriteContactSettingString(hContact, "CList", "StatusMsg", "");
	}
}


// Add a contact to the pool when the status of the contact has changed
void PoolTimerAddContacts(void)
{
	EnterCriticalSection(&update_cs);

	// Make list for next timer ...
	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

		QueueAdd(hContact, GetTickCount() + options.pool_timer_check * 60 * 1000, FALSE);

		// Delete if possible
		if (!ProtocolStatusAllowMsgs(hContact, proto))
		{
			DBWriteContactSettingString(hContact, "CList", "StatusMsg", "");
		}

		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}

	LeaveCriticalSection(&update_cs);
}


// Remove a contact from the current pool
void PoolRemoveContact(HANDLE hContact)
{
	QueueRemove(hContact);
}


void QueueRemove(HANDLE hContact)
{
	// Add this to thread...
	QueueItem *item = (QueueItem *) mir_alloc(sizeof(QueueItem));

	if (item == NULL)
		return;

	EnterCriticalSection(&update_cs);

	item->hContact = hContact;

	List_RemoveByValueFreeContents(statusChangeQueue.queue, item);

	LeaveCriticalSection(&update_cs);

	mir_free(item);
}


// Add an contact to the pool queue
void QueueAdd(HANDLE hContact, DWORD check_time, BOOL prio)
{
	// Add this to thread...
	QueueItem *item = (QueueItem *) mir_alloc(sizeof(QueueItem));

	if (item == NULL)
		return;

	item->hContact = hContact;
	item->check_time = check_time;


	EnterCriticalSection(&update_cs);

	if (prio)
	{
		// Remove from timer
		List_RemoveByValueFreeContents(statusChangeQueue.queue, item);

		// Insert at start because they are get from the end
		List_Insert(statusChangeQueue.queue, hContact, 0);
	}
	else
	{
		if (List_IndexOf(statusChangeQueue.queue, item) != -1)
			// Insert at start because they are get from the end
			List_Insert(statusChangeQueue.queue, hContact, 0);
	}

	LeaveCriticalSection(&update_cs);

	ResumeThread(statusChangeQueue.hThread);
}

int QueueCompareItems(void *i1, void *i2)
{
	return ((QueueItem*)i1)->hContact - ((QueueItem*)i2)->hContact;
}

int QueueSortItems(void *i1, void *i2)
{
	return ((QueueItem*)i1)->check_time - ((QueueItem*)i2)->check_time;
}


BOOL ProtocolStatusAllowMsgs(HANDLE hContact, const char *protocol)
{
	int status = CallProtoService(szProto, PS_GETSTATUS, 0, 0);

	// Exclude offline, connecting and invisible
	if (status >= ID_STATUS_ONLINE && status <= ID_STATUS_OUTTOLUNCH && status != ID_STATUS_INVISIBLE) 
	{
		int contact_status = DBGetContactSettingWord(hContact, protocol, "Status", 0);

		DWORD protoStatusFlags = CallProtoService(protocol, PS_GETCAPS, PFLAGNUM_3, 0);
		if (protoStatusFlags & Proto_Status2Flag(contact_status))
		{
			return TRUE;// here you know the proto named protocol supports status i
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}
