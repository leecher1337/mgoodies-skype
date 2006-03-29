/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#include "poll.h"


// Prototypes ///////////////////////////////////////////////////////////////////////////

static CRITICAL_SECTION update_cs;
static CRITICAL_SECTION pause_cs;

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
	int type;
};

// Types
#define STATUS_CHANGE 1
#define STATUS_CHANGE_TIMER 2
#define PROTOCOL_ONLINE 4
#define POOL 8


UINT_PTR hTimer = 0;

void QueueAdd(HANDLE hContact, DWORD check_time, int type);
void QueueRemove(HANDLE hContact);
int QueueSortItems(void *i1, void *i2);


BOOL ProtocolStatusAllowMsgs(HANDLE hContact, const char *protocol);
VOID CALLBACK PollTimerAddContacts(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
void PollAddAllContacts(int timer, const char *protocol, int type);

DWORD WINAPI UpdateThread(LPVOID vParam);


DWORD next_request_at = 0;
void SetNextRequestAt(DWORD t);
DWORD GetNextRequestAt(void);

QueueItem *requested_item;


// Functions ////////////////////////////////////////////////////////////////////////////


void InitPoll() 
{
	int queuesize = CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);

	// Init queue
	ZeroMemory(&statusQueue, sizeof(statusQueue));
	statusQueue.queue = List_Create(0, queuesize + 10);
	statusQueue.queue->sortFunc = QueueSortItems;
	statusQueue.bThreadRunning = TRUE;
	statusQueue.hThread = CreateThread(NULL, 16000, UpdateThread, NULL, 0, &statusQueue.dwThreadID);

	InitializeCriticalSection(&update_cs);
	InitializeCriticalSection(&pause_cs);

	requested_item = NULL;
}


void FreePoll()
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
	DeleteCriticalSection(&pause_cs);

	// Free lists
	List_DestroyFreeContents(statusQueue.queue);
	mir_free(statusQueue.queue);
}

int UpdateDelay()
{
	int delay = max(50, DBGetContactSettingWord(NULL, MODULE_NAME, "UpdateDelay", UPDATE_DELAY));

log(MODULE_NAME, "UpdateDelay", "Waiting %d ms", delay);

	return delay;
}

int ErrorDelay()
{
	int delay = max(50, DBGetContactSettingWord(NULL, MODULE_NAME, "ErrorDelay", ERROR_DELAY));

log(MODULE_NAME, "ErrorDelay", "Waiting %d ms", delay);

	return delay;
}

int WaitTime()
{
	int delay = max(50, DBGetContactSettingWord(NULL, MODULE_NAME, "WaitTime", WAIT_TIME));

	return delay;
}


// Return true if this protocol has to be checked
BOOL PollCheckProtocol(const char *protocol)
{
	char setting[256];

	if (protocol == NULL)
		return FALSE;

	mir_snprintf(setting, sizeof(setting), OPT_PROTOCOL_GETMSG, protocol);

	return (BOOL) DBGetContactSettingByte(NULL, MODULE_NAME, setting, FALSE);
}


// Return true if this contact has to be checked
BOOL PollCheckContact(HANDLE hContact)
{
	return !DBGetContactSettingByte(hContact,"CList","Hidden",0) && 
		!DBGetContactSettingByte(hContact,"CList","NotOnList",0) &&
		DBGetContactSettingByte(hContact, MODULE_NAME, OPT_CONTACT_GETMSG, TRUE);
}


// Add a contact to the poll when the status of the contact has changed
void PollStatusChangeAddContact(HANDLE hContact)
{
	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);

logC(MODULE_NAME, "PollStatusChangeAddContact", hContact, "Status changed");

	if (proto != NULL && PollCheckProtocol(proto))
	{
		BOOL check = PollCheckContact(hContact);

		// Delete some messages now (don't add to list...)
		if (ProtocolStatusAllowMsgs(hContact, proto))
		{
			if (check)
			{
				if (opts.poll_clear_on_status_change)
					ClearStatusMessage(hContact);

				if (opts.poll_check_on_status_change)
					QueueAdd(hContact, GetTickCount(), STATUS_CHANGE);

				if (opts.poll_check_on_status_change_timer)
					QueueAdd(hContact, GetTickCount() + opts.poll_timer_status * 1000, STATUS_CHANGE_TIMER);
			}
		}
		else
		{
			if (check || opts.always_clear)
			{
				PollReceivedContactMessage(hContact, FALSE);
				ClearStatusMessage(hContact);
			}
		}
	}
	else
	{
logC(MODULE_NAME, "PollStatusChangeAddContact", hContact, "Contact should not be checked");
	}
}

void PollAddAllContactsTimer(int timer)
{
	PollAddAllContacts(timer, NULL, POOL);
}

void PollAddAllContactsProtoOnline(int timer, const char *protocol)
{
	PollAddAllContacts(timer, protocol, PROTOCOL_ONLINE);
}

void PollAddAllContacts(int timer, const char *protocol, int type)
{
log(MODULE_NAME, "PollAddAllContacts", "[%s] Start", protocol == NULL ? "all" : protocol);

	EnterCriticalSection(&update_cs);

	// Make list for next timer ...
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		char *proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

		if (proto != NULL && (protocol == NULL || strcmp(proto, protocol) == 0))
		{
			if (type == PROTOCOL_ONLINE && PollCheckProtocol(proto) 
				&& (PollCheckContact(hContact) || opts.always_clear)
				&& !ProtocolStatusAllowMsgs(hContact, proto))
			{
				ClearStatusMessage(hContact);
			}

			QueueAdd(hContact, GetTickCount() + timer, type);
		}

		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}

	LeaveCriticalSection(&update_cs);

log(MODULE_NAME, "PollAddAllContacts", "End");

}

// Add a contact to the poll when the status of the contact has changed
VOID CALLBACK PollTimerAddContacts(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	if (hTimer != NULL) 
	{
		KillTimer(NULL, hTimer);
		hTimer = NULL;
	}

log(MODULE_NAME, "PollTimerAddContacts", "Loop Timer");

	PollAddAllContactsTimer(opts.poll_timer_check * 60000);

	PollSetTimer();
}


void PollSetTimer(void)
{
	if (hTimer != NULL)
		KillTimer(NULL, hTimer);

	if (opts.poll_check_on_timer)
		hTimer = SetTimer(NULL, 0, opts.poll_check_on_timer * 60000, PollTimerAddContacts);
}


// Remove a contact from the current poll
void PollReceivedContactMessage(HANDLE hContact, BOOL from_network)
{
	QueueRemove(hContact);

	if (from_network)
	{
		EnterCriticalSection(&update_cs);
	
		if (requested_item != NULL && requested_item->hContact == hContact)
		{
			mir_free(requested_item);
			requested_item = NULL;
		}

		LeaveCriticalSection(&update_cs);

		SetNextRequestAt(GetTickCount() + UpdateDelay());
	}
}


void QueueRemove(HANDLE hContact)
{
	EnterCriticalSection(&update_cs);

	if (statusQueue.queue->items != NULL)
	{
		DWORD now = GetTickCount();
		DWORD timePool = now + (DWORD) (opts.poll_timer_check / 3.0 * 60000);
		now += 5000;	// 5secs error allowed

		for (int i = statusQueue.queue->realCount - 1 ; i >= 0 ; i-- )
		{
			QueueItem *item = (QueueItem*) statusQueue.queue->items[i];
			
			if (item->hContact == hContact)
			{
				// Remove old items and status changes
				if ((item->type & ~(POOL | STATUS_CHANGE_TIMER)) || now <= item->check_time)
				{
					mir_free(item);
					List_Remove(statusQueue.queue, i);
				}
				// Remove pool if 1/3 has passed
				else if ((item->type & POOL) && timePool <= item->check_time)
				{
					mir_free(item);
					List_Remove(statusQueue.queue, i);
				}

			}
		}
	}

	LeaveCriticalSection(&update_cs);
}


// Test if has to add an item
BOOL QueueTestAdd(QueueItem *item)
{
	BOOL add = TRUE;

	switch(item->type)
	{
		case STATUS_CHANGE:
		{
			// Remove all, exept PROTOCOL_ONLINE
			int i;
			int test = ~PROTOCOL_ONLINE;
			for ( i = statusQueue.queue->realCount - 1 ; i >= 0 ; i-- )
			{
				QueueItem *tmp = (QueueItem *) statusQueue.queue->items[i];
				if (tmp->hContact == item->hContact)
				{
					if (tmp->type & test)
					{
						mir_free(tmp);
						List_Remove(statusQueue.queue, i);
					}
					else
					{
						add = FALSE;
					}
				}
			}

			if (requested_item != NULL && requested_item->hContact == item->hContact)
				add = FALSE;

			break;
		}
		case STATUS_CHANGE_TIMER:
		{
			// Remove STATUS_CHANGE_TIMER and POOL
			int i;
			int test = STATUS_CHANGE_TIMER | POOL;
			for ( i = statusQueue.queue->realCount - 1 ; i >= 0 ; i-- )
			{
				QueueItem *tmp = (QueueItem *) statusQueue.queue->items[i];
				if (tmp->hContact == item->hContact)
				{
					if (tmp->type & test)
					{
						mir_free(tmp);
						List_Remove(statusQueue.queue, i);
					}
					else if (tmp->type & PROTOCOL_ONLINE)
					{
						add = FALSE;
					}
				}
			}
			break;
		}
		case PROTOCOL_ONLINE:
		{
			// Remove ALL
			int i;
			for ( i = statusQueue.queue->realCount - 1 ; i >= 0 ; i-- )
			{
				QueueItem *tmp = (QueueItem *) statusQueue.queue->items[i];
				if (tmp->hContact == item->hContact)
				{
					mir_free(tmp);
					List_Remove(statusQueue.queue, i);
				}
			}
			break;
		}
		//case POOL:
		//{
		//	// Dont remove, allways add
		//	break;
		//}
	}

	return add;
}


// Add an contact to the poll queue
void QueueAdd(HANDLE hContact, DWORD check_time, int type)
{
	// Add this to thread...
	QueueItem *item = (QueueItem *) mir_alloc(sizeof(QueueItem));

	if (item == NULL)
		return;

	item->hContact = hContact;
	item->check_time = check_time;
	item->type = type;

	EnterCriticalSection(&update_cs);

	if (QueueTestAdd(item))
	{
		List_InsertOrdered(statusQueue.queue, item);
logC(MODULE_NAME, "QueueAdd", hContact, "Type = %d", type);
	}
	else
	{
		mir_free(item);
	}

	LeaveCriticalSection(&update_cs);

	ResumeThread(statusQueue.hThread);
}


// Itens with higher priority at end
int QueueSortItems(void *i1, void *i2)
{
	return ((QueueItem*)i2)->check_time - ((QueueItem*)i1)->check_time;
}


BOOL ProtocolStatusAllowMsgs(HANDLE hContact, const char *protocol)
{
	int status = CallProtoService(protocol, PS_GETSTATUS, 0, 0);

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


DWORD WINAPI UpdateThread(LPVOID vParam)
{
	// Initial timer
	Sleep(INITIAL_TIMER);

log(MODULE_NAME, "UpdateThread", "Start");

	while (statusQueue.bThreadRunning)
	{
		// Was to run yet?
		DWORD test_timer = GetNextRequestAt();
		if (test_timer > GetTickCount())
		{
			Sleep(test_timer - GetTickCount());
		}
		else
		{
			// Ok, lets check the message
			EnterCriticalSection(&update_cs);

			if (requested_item != NULL)
			{
				if (GetTickCount() < requested_item->check_time + WaitTime())
				{
					LeaveCriticalSection(&update_cs);

log(MODULE_NAME, "UpdateThread", "Last request not received yet. Waiting");

					if (statusQueue.bThreadRunning)
					{
						Sleep(POOL_DELAY);
					}
				}
				else
				{
					mir_free(requested_item);
					requested_item = NULL;

					LeaveCriticalSection(&update_cs);

log(MODULE_NAME, "UpdateThread", "Last request not received in time-out. Ignoring it");
				}
			}
			else if (!List_HasItens(statusQueue.queue))
			{
				LeaveCriticalSection(&update_cs);

				// Stop this one...
				if (statusQueue.bThreadRunning)
					SuspendThread(statusQueue.hThread);
			}
			else
			{
				// Get next job...

				/*
				 * the thread is awake, processing the update queue one by one entry with a given delay
				 * of UPDATE_DELAY seconds. The delay ensures that no protocol will kick us because of
				 * flood protection(s)
				 */
				QueueItem *qi = (QueueItem *) List_Peek(statusQueue.queue);

				if (qi->check_time > GetTickCount()) 
				{
					LeaveCriticalSection(&update_cs);

					if (statusQueue.bThreadRunning)
					{
						Sleep(POOL_DELAY);
					}
				}
				else
				{
					qi = (QueueItem *) List_Pop(statusQueue.queue);
					char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)qi->hContact, 0);

					if (proto && PollCheckProtocol(proto) && PollCheckContact(qi->hContact))
					{
logC(MODULE_NAME, "UpdateThread", qi->hContact, "Have to check");

						if (ProtocolStatusAllowMsgs(qi->hContact, proto))
						{
							int ret = CallContactService(qi->hContact,PSS_GETAWAYMSG,0,0);

							if (ret != 0)
							{
								requested_item = qi;
								requested_item->check_time = GetTickCount();

								LeaveCriticalSection(&update_cs);

logC(MODULE_NAME, "UpdateThread", qi->hContact, "Requested");

								if (statusQueue.bThreadRunning)
								{
									Sleep(POOL_DELAY);
								}
							}
							else
							{
								// Error, pause for a while
								List_Push(statusQueue.queue, qi);

								LeaveCriticalSection(&update_cs);

log(MODULE_NAME, "UpdateThread", "ERROR, pausing for a while");

								if (statusQueue.bThreadRunning)
								{
									int delay = ErrorDelay();
									SetNextRequestAt(GetTickCount() + delay);
									Sleep(delay);
								}
							}
						}
						else
						{
							ClearStatusMessage(qi->hContact);

							LeaveCriticalSection(&update_cs);

logC(MODULE_NAME, "UpdateThread", qi->hContact, "Set empty");

							mir_free(qi);
						}
					}
					else
					{
						mir_free(qi);

						LeaveCriticalSection(&update_cs);
					}
				}
			}
		}
	}

log(MODULE_NAME, "UpdateThread", "End");

	return 0;
}


void SetNextRequestAt(DWORD t)
{
	EnterCriticalSection(&pause_cs);
	next_request_at = t;
	LeaveCriticalSection(&pause_cs);
}

DWORD GetNextRequestAt(void)
{
	EnterCriticalSection(&pause_cs);
	DWORD ret = next_request_at;
	LeaveCriticalSection(&pause_cs);

	return ret;
}
