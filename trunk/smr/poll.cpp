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
#define XSTATUS_CHANGE 16


UINT_PTR hTimer = 0;

void QueueAdd(HANDLE hContact, DWORD check_time, int type);
void QueueRemove(HANDLE hContact);
int QueueSortItems(void *i1, void *i2);


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

	return delay;
}

int ErrorDelay()
{
	int delay = max(50, DBGetContactSettingWord(NULL, MODULE_NAME, "ErrorDelay", ERROR_DELAY));

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
		DBGetContactSettingByte(hContact,"CList","ApparentMode",0) != ID_STATUS_OFFLINE &&
		DBGetContactSettingByte(hContact, MODULE_NAME, OPT_CONTACT_GETMSG, TRUE);
}


// Add a contact to the poll when the status of the contact has changed
void PollStatusChangeAddContact(HANDLE hContact)
{
	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);

	if (proto != NULL && PollCheckProtocol(proto))
	{
		// Delete some messages now (don't add to list...)
		Check what = ProtocolStatusCheckMsg(hContact, proto);

		if (what == Retrieve)
		{
			if (opts.poll_clear_on_status_change)
				ClearStatusMessage(hContact);

			if (opts.poll_check_on_status_change)
				QueueAdd(hContact, GetTickCount(), STATUS_CHANGE);

			if (opts.poll_check_on_status_change_timer)
				QueueAdd(hContact, GetTickCount() + opts.poll_timer_status * 1000, STATUS_CHANGE_TIMER);
		}
		else if (what == UseXStatus || what == ClearXStatus)
		{
			PollXStatusChangeAddContact(hContact);
		}
		else
		{
			ProcessCheckNotToServer(what, hContact, proto);
		}
	}
}

// Check after some time, to allow proto to set all data about XStatus
void PollXStatusChangeAddContact(HANDLE hContact)
{
	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);

	if (proto != NULL && PollCheckProtocol(proto) && opts.when_xstatus != Normal)
	{
		QueueAdd(hContact, GetTickCount() + 200, XSTATUS_CHANGE);
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
	// Has to check?
	if (protocol != NULL && !PollCheckProtocol(protocol))
		return;

	// Make list for next timer ...
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		char *proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

		if (proto != NULL && (protocol == NULL || strcmp(proto, protocol) == 0) 
			&& (protocol != NULL || PollCheckProtocol(proto)))
		{
			if (type == PROTOCOL_ONLINE)
			{
				Check what = ProtocolStatusCheckMsg(hContact, proto);
				ProcessCheckNotToServer(what, hContact, proto);
			}

			QueueAdd(hContact, GetTickCount() + timer, type);
		}

		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}

VOID CALLBACK PollTimerAddContacts(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	if (hTimer != NULL) 
	{
		KillTimer(NULL, hTimer);
		hTimer = NULL;
	}

	// Adds with half the timer because the real timer is the calling of this function. 
	// The timer passed here is the amout of time that it will wait to see if the user do not 
	// request the message by itself
	PollAddAllContactsTimer(opts.poll_timer_check * 60000 / 3);

	PollSetTimer();
}


void PollSetTimer(void)
{
	if (hTimer != NULL) 
	{
		KillTimer(NULL, hTimer);
		hTimer = NULL;
	}

	if (opts.poll_check_on_timer)
		hTimer = SetTimer(NULL, 0, opts.poll_timer_check * 60000, PollTimerAddContacts);
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
		DWORD now = GetTickCount() + 5000;	// 5secs error allowed

		for (int i = statusQueue.queue->realCount - 1 ; i >= 0 ; i-- )
		{
			QueueItem *item = (QueueItem*) statusQueue.queue->items[i];
			
			if (item->hContact == hContact)
			{
				// Remove old items and status changes
				if ((item->type & ~STATUS_CHANGE_TIMER) || item->check_time <= now)
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
		case XSTATUS_CHANGE:
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
			{
				add = FALSE;
			}

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
	QueueItem *item = (QueueItem *) mir_alloc0(sizeof(QueueItem));

	if (item == NULL)
		return;

	item->hContact = hContact;
	item->check_time = check_time;
	item->type = type;

	EnterCriticalSection(&update_cs);

	if (QueueTestAdd(item))
	{
		List_InsertOrdered(statusQueue.queue, item);
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

// Returns if has to check that status
Check ProtocolStatusCheckMsg(HANDLE hContact, const char *protocol)
{
	BOOL check = PollCheckContact(hContact);
	int status = CallProtoService(protocol, PS_GETSTATUS, 0, 0);

	// Exclude offline, connecting and invisible
	if (status >= ID_STATUS_ONLINE && status <= ID_STATUS_OUTTOLUNCH && status != ID_STATUS_INVISIBLE) 
	{
		// Status
		int contact_status = DBGetContactSettingWord(hContact, protocol, "Status", 0);
		int contact_xstatus = DBGetContactSettingByte(hContact, protocol, "XStatusId", 0);

		// Check
		if (opts.when_xstatus != Normal && contact_xstatus != 0)
		{
			// Use XStatus

			bool has_xstatus_name = false;
			bool has_xstatus_message = false;
			TCHAR name[256] = {0};
			TCHAR msg[256] = {0};

			DBVARIANT dbv;
			if (!DBGetContactSettingTString(hContact, protocol, "XStatusName", &dbv))
			{
				if (dbv.ptszVal != NULL && dbv.ptszVal[0] != _T('\0'))
				{
					lstrcpyn(name, dbv.ptszVal, sizeof(name));
					has_xstatus_name = true;
				}
				DBFreeVariant(&dbv);
			}
			if (!DBGetContactSettingTString(hContact, protocol, "XStatusMsg", &dbv))
			{
				if (dbv.ptszVal != NULL && dbv.ptszVal[0] != _T('\0'))
				{
					lstrcpyn(msg, dbv.ptszVal, sizeof(msg));
					has_xstatus_message = true;
				}
				DBFreeVariant(&dbv);
			}

			if (opts.when_xstatus == Clear 
				|| (opts.when_xstatus == ClearOnMessage && has_xstatus_message) )
			{
				return (check || opts.always_clear) ? ClearMessage : DoNothing;
			}
			else if (opts.when_xstatus == SetToXStatusName 
					 || opts.when_xstatus == SetToXStatusMessage 
					 || opts.when_xstatus == SetToXStatusNameXStatusMessage)
			{
				return check ? UseXStatus : DoNothing;
			}
		}

		// If get until this point, Use normal status message
		DWORD protoStatusFlags = CallProtoService(protocol, PS_GETCAPS, PFLAGNUM_3, 0);
		if (protoStatusFlags & Proto_Status2Flag(contact_status))
		{
			return check ? Retrieve : DoNothing;// here you know the proto named protocol supports status i
		}
		else
		{
			return (check || opts.always_clear) ? ClearMessage : DoNothing;
		}
	}
	else // Protocol in a status that do not have to check msgs
	{
		return (check || opts.always_clear) ? ClearMessage : DoNothing;
	}
}

// This should be called from outside the critical session
void ProcessCheckNotToServer(Check what, HANDLE hContact, const char *protocol)
{
	switch(what)
	{
		case ClearMessage:
		{
			ClearStatusMessage(hContact);
			PollReceivedContactMessage(hContact, FALSE);
			break;
		}
		case UseXStatus:
		{
			bool has_xstatus_name = false;
			bool has_xstatus_message = false;
			TCHAR name[256] = {0};
			TCHAR msg[256] = {0};

			DBVARIANT dbv;
			if (!DBGetContactSettingTString(hContact, protocol, "XStatusName", &dbv))
			{
				if (dbv.ptszVal != NULL && dbv.ptszVal[0] != _T('\0'))
				{
					lstrcpyn(name, dbv.ptszVal, sizeof(name));
					has_xstatus_name = true;
				}
				DBFreeVariant(&dbv);
			}
			if (!DBGetContactSettingTString(hContact, protocol, "XStatusMsg", &dbv))
			{
				if (dbv.ptszVal != NULL && dbv.ptszVal[0] != _T('\0'))
				{
					lstrcpyn(msg, dbv.ptszVal, sizeof(msg));
					has_xstatus_message = true;
				}
				DBFreeVariant(&dbv);
			}

			// SetToXStatusName
			if (opts.when_xstatus == SetToXStatusName)
			{
				SetStatusMessage(hContact, name);
				PollReceivedContactMessage(hContact, FALSE);
			}

			// SetToXStatusMessage
			else if (opts.when_xstatus == SetToXStatusMessage)
			{
				SetStatusMessage(hContact, msg);
				PollReceivedContactMessage(hContact, FALSE);
			}

			// SetToXStatusNameXStatusValue
			else if (opts.when_xstatus == SetToXStatusNameXStatusMessage)
			{
				if (has_xstatus_name && has_xstatus_message)
				{
					TCHAR message[512];
					mir_sntprintf(message, sizeof(message), _T("%s: %s"), name, msg);
					SetStatusMessage(hContact, message);
				}
				else if (has_xstatus_name)
				{
					SetStatusMessage(hContact, name);
				}
				else if (has_xstatus_message)
				{
					SetStatusMessage(hContact, msg);
				}
				PollReceivedContactMessage(hContact, FALSE);
			}
			break;
		}
	}
}


DWORD WINAPI UpdateThread(LPVOID vParam)
{
	// Initial timer
	Sleep(INITIAL_TIMER);

	while (statusQueue.bThreadRunning)
	{
		// First remove all that are due and do not have to be pooled
		EnterCriticalSection(&update_cs);

		if (!List_HasItens(statusQueue.queue))
		{
			LeaveCriticalSection(&update_cs);
		}
		else
		{
			// Create a copy of the queue
			SortedList *tmpQueue;
			tmpQueue = List_Create(0, statusQueue.queue->realCount);
			tmpQueue->sortFunc = QueueSortItems;

			for (int i = statusQueue.queue->realCount - 1; i >= 0; i--)
			{
				QueueItem *qi = (QueueItem *) List_Pop(statusQueue.queue);

				if (qi->check_time > GetTickCount()) 
				{
					List_Push(statusQueue.queue, qi);
					break;
				}
				else
				{
					List_InsertOrdered(tmpQueue, qi);
				}
			}

			LeaveCriticalSection(&update_cs);

			if (!statusQueue.bThreadRunning)
				break;

			// See the itens that was copied
			if (List_HasItens(tmpQueue))
			{
				for (int i = tmpQueue->realCount - 1; i >= 0; i--)
				{
					if (!statusQueue.bThreadRunning)
						break;

					QueueItem *qi = (QueueItem *) tmpQueue->items[i];

					char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)qi->hContact, 0);

					if (proto == NULL || !PollCheckProtocol(proto))
					{
						List_Remove(tmpQueue, i);
						mir_free(qi);
					}
					else
					{
						Check what = ProtocolStatusCheckMsg(qi->hContact, proto);

						if (what != DoNothing && what != Retrieve)
						{
							ProcessCheckNotToServer(what, qi->hContact, proto);

							List_Remove(tmpQueue, i);
							mir_free(qi);
						}
						else if (what == DoNothing)
						{
							List_Remove(tmpQueue, i);
							mir_free(qi);
						}
					}
				}

				if (!statusQueue.bThreadRunning)
					break;

				// See if has itens to be copied back
				if (List_HasItens(tmpQueue))
				{
					EnterCriticalSection(&update_cs);
					for (int i = tmpQueue->realCount - 1; i >= 0; i--)
					{
						QueueItem *qi = (QueueItem *) List_Pop(tmpQueue);
						List_InsertOrdered(statusQueue.queue, qi);
					}
					LeaveCriticalSection(&update_cs);
				}
			}
		}

		if (!statusQueue.bThreadRunning)
			break;

		// Was to run yet?
		DWORD test_timer = GetNextRequestAt();
		if (test_timer > GetTickCount())
		{
			if (statusQueue.bThreadRunning)
				Sleep(POOL_DELAY);
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

					if (statusQueue.bThreadRunning)
						Sleep(POOL_DELAY);
				}
				else
				{
					mir_free(requested_item);
					requested_item = NULL;

					LeaveCriticalSection(&update_cs);
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
						Sleep(POOL_DELAY);
				}
				else
				{
					qi = (QueueItem *) List_Pop(statusQueue.queue);

					LeaveCriticalSection(&update_cs);

					if (!statusQueue.bThreadRunning)
						break;

					char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)qi->hContact, 0);

					if (proto == NULL || !PollCheckProtocol(proto))
					{
						mir_free(qi);
					}
					else
					{
						Check what = ProtocolStatusCheckMsg(qi->hContact, proto);

						if (what == DoNothing)
						{
							mir_free(qi);
						}
						else if (what != Retrieve)
						{
							ProcessCheckNotToServer(what, qi->hContact, proto);
							mir_free(qi);
						}
						else // if (what == Retrieve)
						{
							if (!statusQueue.bThreadRunning)
								break;

							int ret = CallContactService(qi->hContact,PSS_GETAWAYMSG,0,0);

							if (ret != 0)
							{
								EnterCriticalSection(&update_cs);

								requested_item = qi;
								requested_item->check_time = GetTickCount();

								LeaveCriticalSection(&update_cs);

								if (statusQueue.bThreadRunning)
									Sleep(POOL_DELAY);
							}
							else
							{
								EnterCriticalSection(&update_cs);

								// Error, pause for a while
								List_Push(statusQueue.queue, qi);

								LeaveCriticalSection(&update_cs);

								int delay = ErrorDelay();
								SetNextRequestAt(GetTickCount() + delay);
								if (statusQueue.bThreadRunning)
									Sleep(delay);
							}
						}
					}
				}
			}
		}
	}

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
