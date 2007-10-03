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

#include "ContactAsyncQueue.h"



struct QueueItem
{
	DWORD check_time;
	HANDLE hContact;
	void *param;
};


// Itens with higher time at end
static int QueueSortItems(void *i1, void *i2)
{
	return ((QueueItem*)i2)->check_time - ((QueueItem*)i1)->check_time;
}

// Itens with higher time at end
static void ContactAsyncQueueThread(void *obj)
{
	((ContactAsyncQueue *)obj)->Thread();
}

ContactAsyncQueue::ContactAsyncQueue(pfContactAsyncQueueCallback fContactAsyncQueueCallback, int initialSize)
{
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	finished = 0;
	callback = fContactAsyncQueueCallback;

	queue = List_Create(0, initialSize);
	queue->sortFunc = QueueSortItems;
	InitializeCriticalSection(&cs);

	mir_forkthread(ContactAsyncQueueThread, this);
}

ContactAsyncQueue::~ContactAsyncQueue()
{
	if (finished == 0)
		finished = 1;
	SetEvent(hEvent);
	int count = 0;
	while(finished != 2 && ++count < 50)
		Sleep(10);

	List_DestroyFreeContents(queue);
	DeleteCriticalSection(&cs);
}


void ContactAsyncQueue::RemoveAll(HANDLE hContact)
{
	EnterCriticalSection(&cs);

	if (queue->items != NULL)
	{
		for (int i = queue->realCount - 1 ; i >= 0 ; i-- )
		{
			QueueItem *item = (QueueItem*) queue->items[i];
			
			if (item->hContact == hContact)
			{
				List_Remove(queue, i);
				mir_free(item);
			}
		}
	}

	LeaveCriticalSection(&cs);
}


void ContactAsyncQueue::RemoveAllConsiderParam(HANDLE hContact, void *param)
{
	EnterCriticalSection(&cs);

	if (queue->items != NULL)
	{
		for (int i = queue->realCount - 1 ; i >= 0 ; i-- )
		{
			QueueItem *item = (QueueItem*) queue->items[i];
			
			if (item->hContact == hContact && item->param == param)
			{
				List_Remove(queue, i);
				mir_free(item);
			}
		}
	}

	LeaveCriticalSection(&cs);
}


void ContactAsyncQueue::Add(int waitTime, HANDLE hContact, void *param)
{
	EnterCriticalSection(&cs);

	InternalAdd(waitTime, hContact, param);
	
	LeaveCriticalSection(&cs);
}

void ContactAsyncQueue::AddIfDontHave(int waitTime, HANDLE hContact, void *param)
{
	EnterCriticalSection(&cs);

	int i;
	for (i = queue->realCount - 1; i >= 0; i--)
		if (((QueueItem *) queue->items[i])->hContact == hContact)
			break;

	if (i < 0)
		InternalAdd(waitTime, hContact, param);

	LeaveCriticalSection(&cs);
}

void ContactAsyncQueue::AddAndRemovePrevious(int waitTime, HANDLE hContact, void *param)
{
	EnterCriticalSection(&cs);

	RemoveAll(hContact);
	InternalAdd(waitTime, hContact, param);

	LeaveCriticalSection(&cs);
}

void ContactAsyncQueue::AddAndRemovePreviousConsiderParam(int waitTime, HANDLE hContact, void *param)
{
	EnterCriticalSection(&cs);

	RemoveAllConsiderParam(hContact, param);
	InternalAdd(waitTime, hContact, param);

	LeaveCriticalSection(&cs);
}

void ContactAsyncQueue::InternalAdd(int waitTime, HANDLE hContact, void *param)
{
	QueueItem *item = (QueueItem *) mir_alloc(sizeof(QueueItem));
	item->hContact = hContact;
	item->check_time = GetTickCount() + waitTime;
	item->param = param;

	List_InsertOrdered(queue, item);

	SetEvent(hEvent);
}

void ContactAsyncQueue::Thread()
{
	while (!finished)
	{
		EnterCriticalSection(&cs);

		if (!List_HasItens(queue))
		{
			// No items, so supend thread
			LeaveCriticalSection(&cs);

			wait(INFINITE);
		}
		else
		{
			// Take a look at first item
			QueueItem *qi = (QueueItem *) List_Peek(queue);

			int dt = qi->check_time - GetTickCount();
			if (dt > 0) 
			{
				// Not time to request yet, wait...
				LeaveCriticalSection(&cs);

				wait(dt);
			}
			else
			{
				// Will request this item
				qi = (QueueItem *) List_Pop(queue);

				LeaveCriticalSection(&cs);

				callback(qi->hContact, qi->param);

				mir_free(qi);
			}
		}
	}

	finished = 2;
}

void ContactAsyncQueue::wait(int time)
{
	if (!finished)
		WaitForSingleObject(hEvent, time);
}






