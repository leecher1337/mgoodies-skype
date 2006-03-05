#include "main.h"

#define UPDATE_DELAY 500
#define STATUS_CHANGE_DELAY 500

#define INITIAL_TIMER TMR_ICQFIRSTCHECK

static BOOL statusThreadHasRan = FALSE;


typedef struct
{
	SortedListOld *queue;
	HANDLE hThread;
	DWORD dwThreadID;
	BOOL bThreadRunning;

} ThreadQueue;

ThreadQueue timerQueue;
ThreadQueue statusChangeQueue;


BOOL HasToGetStatusMsgForProtocol(const char *szProto)
{
	char setting[64];

	if (szProto == NULL)
		return FALSE;

	strncpy(setting, szProto, 55);
	setting[55] = '\0';
	strcat(setting, "CheckMsg");

	return (BOOL)DBGetContactSettingByte(NULL, MODULE, setting, FALSE);
}

BOOL HasToIgnoreContact(HANDLE hContact, const char* szProto)
{
	if (!DBGetContactSettingByte(hContact, MODULE, OPT_CONTACT_GETMSG, DEFAULT_ICQCHECK))
	{
		return TRUE;
	}

	return FALSE;

	/*
	// Has elapsed the initial timer?
	if (DBGetContactSettingDword(NULL, MODULE, szProto, 0) == 1)
	{
		return FALSE;
	}

	if ((GetTickCount() - DBGetContactSettingDword(NULL, MODULE, szProto, 0)) > TMR_ICQFIRSTCHECK)
	{
		DBWriteContactSettingDword(NULL, MODULE, szProto, 1);
		return FALSE;
	}

	// So don't...
	return TRUE;
	*/
}


BOOL HasToGetStatusMsgForContact(HANDLE hContact, const char *protocol)
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


static DWORD WINAPI TimerUpdateThread(LPVOID vParam)
{
	HANDLE hContact;
	char *szProto = NULL;

	// Initial timer
	Sleep(INITIAL_TIMER);

	if (options.bCheckIcqStatusMsgs)
	{
		EnterCriticalSection(&update_cs);

		// Make list for next timer ...
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact)
		{
			szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

			if (HasToGetStatusMsgForProtocol(szProto))
				List_Push(timerQueue.queue, hContact);

			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}

		LeaveCriticalSection(&update_cs);
	}

	Sleep(UPDATE_DELAY);

	while (timerQueue.bThreadRunning)
	{
		EnterCriticalSection(&update_cs);

		if (!List_HasItens(timerQueue.queue))
		{
			if (options.bCheckIcqStatusMsgs)
			{
				// Make list for next timer ...
				hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
				while (hContact)
				{
					szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

					if (HasToGetStatusMsgForProtocol(szProto))
						List_Push(timerQueue.queue, hContact);

					hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
				}
			}

			LeaveCriticalSection(&update_cs);

			if (options.bCheckIcqStatusMsgs)
			{
				SetTimer(hTimerWindow, 1, options.dCheckIcqTimer*60000, StatusMsgCheckTimerProc);
			}

			// Stop this one...
			if (timerQueue.bThreadRunning)
				SuspendThread(timerQueue.hThread);
		}
		else if (statusThreadHasRan)
		{
			statusThreadHasRan = FALSE;

			LeaveCriticalSection(&update_cs);

			if (timerQueue.bThreadRunning)
				Sleep(UPDATE_DELAY);
		}
		else
		{
			// Get next job...

			/*
			 * the thread is awake, processing the update queue one by one entry with a given delay
			 * of UPDATE_DELAY seconds. The delay ensures that no protocol will kick us because of
			 * flood protection(s)
			 */
			hContact = List_Pop(timerQueue.queue);

			szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

			if (hContact && szProto && HasToGetStatusMsgForProtocol(szProto) && !HasToIgnoreContact(hContact, szProto))
			{
				if (HasToGetStatusMsgForContact(hContact, szProto))
				{
					CallContactService(hContact,PSS_GETAWAYMSG,0,0);

					LeaveCriticalSection(&update_cs);

					if (timerQueue.bThreadRunning)
						Sleep(UPDATE_DELAY);
				}
				else
				{
					DBWriteContactSettingString(hContact, "CList", "StatusMsg", "");

					LeaveCriticalSection(&update_cs);
				}
			}
			else
			{
				LeaveCriticalSection(&update_cs);
			}
		}
	}

	return 0;
}


static DWORD WINAPI StatusChangeUpdateThread(LPVOID vParam)
{
	HANDLE hContact;
	char *szProto = NULL;

	// Initial timer
	Sleep(INITIAL_TIMER);

	while (statusChangeQueue.bThreadRunning)
	{
		EnterCriticalSection(&update_cs);

		if (!List_HasItens(statusChangeQueue.queue))
		{
			// nothing to do...
			LeaveCriticalSection(&update_cs);

			if (statusChangeQueue.bThreadRunning)
				SuspendThread(statusChangeQueue.hThread);

			if (statusChangeQueue.bThreadRunning)
				// Timer here, so if other plugin get this status, we do not
				Sleep(STATUS_CHANGE_DELAY);
		}
		else
		{
			// Get next job...

			/*
			 * the thread is awake, processing the update queue one by one entry with a given delay
			 * of UPDATE_DELAY seconds. The delay ensures that no protocol will kick us because of
			 * flood protection(s)
			 */
			hContact = List_Pop(statusChangeQueue.queue);

			szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

			if (hContact && szProto && HasToGetStatusMsgForProtocol(szProto) && !HasToIgnoreContact(hContact, szProto))
			{
				if (HasToGetStatusMsgForContact(hContact, szProto))
				{
					CallContactService(hContact,PSS_GETAWAYMSG,0,0);

					statusThreadHasRan = TRUE;

					LeaveCriticalSection(&update_cs);

					if (statusChangeQueue.bThreadRunning)
						Sleep(UPDATE_DELAY);
				}
				else
				{
					DBWriteContactSettingString(hContact, "CList", "StatusMsg", "");

					LeaveCriticalSection(&update_cs);
				}
			}
			else
			{
				LeaveCriticalSection(&update_cs);
			}
		}
	}

	return 0;
}


void StatusChangeAddContact(HANDLE hContact, const char* proto)
{
	if (hContact && proto && HasToGetStatusMsgForProtocol(proto) && !HasToIgnoreContact(hContact, proto))
	{
		// Delete some messages now (don't add to list...)
		if (HasToGetStatusMsgForContact(hContact, proto))
		{
			// Add this to thread...
			EnterCriticalSection(&update_cs);

			// Remove from timer -> status change has priority
			List_RemoveByValue(timerQueue.queue, hContact);

			// Insert at start because they are get from the end
			List_Insert(statusChangeQueue.queue, hContact, 0);

			LeaveCriticalSection(&update_cs);

			ResumeThread(statusChangeQueue.hThread);
		}
		else
		{
			DBWriteContactSettingString(hContact, "CList", "StatusMsg", "");
		}
	}
}

void MessageGotForContact(HANDLE hContact)
{
	EnterCriticalSection(&update_cs);

	List_RemoveByValue(statusChangeQueue.queue, hContact);
	List_RemoveByValue(timerQueue.queue, hContact);

	LeaveCriticalSection(&update_cs);
}


VOID CALLBACK StatusMsgCheckTimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	if (uMsg != WM_TIMER || idEvent != TIMERID_STATUSCHECK)
	{
		return;
	}
	else
	{
		KillTimer(hWnd, idEvent);

		ResumeThread(timerQueue.hThread);
	}
}


void InitStatusUpdate()
{
	int queuesize = CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);

	// Init timer queue
	ZeroMemory(&timerQueue, sizeof(timerQueue));
	timerQueue.queue = List_Create(queuesize + 10, 10);
	timerQueue.bThreadRunning = TRUE;
	timerQueue.hThread = CreateThread(NULL, 16000, TimerUpdateThread, NULL, 0, &timerQueue.dwThreadID);

	// Init status change queue
	ZeroMemory(&statusChangeQueue, sizeof(statusChangeQueue));
	statusChangeQueue.queue = List_Create(10, 10);
	statusChangeQueue.bThreadRunning = TRUE;
	statusChangeQueue.hThread = CreateThread(NULL, 16000, StatusChangeUpdateThread, NULL, 0, &statusChangeQueue.dwThreadID);


	InitializeCriticalSection(&update_cs);
}

void EndStatusUpdate()
{
	DWORD dwExitcode;

	// Stop timer queue
	timerQueue.bThreadRunning = FALSE;
	ResumeThread(timerQueue.hThread);
	do {
		Sleep(100);
		GetExitCodeThread(timerQueue.hThread, &dwExitcode);
	} while ( dwExitcode == STILL_ACTIVE );
	if (timerQueue.hThread)
		CloseHandle(timerQueue.hThread);

	// Stop status change queue
	statusChangeQueue.bThreadRunning = FALSE;
	ResumeThread(statusChangeQueue.hThread);
	do {
		Sleep(100);
		GetExitCodeThread(statusChangeQueue.hThread, &dwExitcode);
	} while ( dwExitcode == STILL_ACTIVE );
	if (statusChangeQueue.hThread)
		CloseHandle(statusChangeQueue.hThread);

	// Delete cs
	DeleteCriticalSection(&update_cs);

	// Free lists
	List_Destroy(timerQueue.queue);
	List_Destroy(statusChangeQueue.queue);
}

