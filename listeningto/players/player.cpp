/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


#include "..\\commons.h"


extern void HasNewListeningInfo();


void CopyListeningInfo(LISTENINGTOINFO *dest, const LISTENINGTOINFO * const src)
{
	FreeListeningInfo(dest);

	dest->cbSize = src->cbSize;
	dest->szArtist = mir_dupT(src->szArtist);
	dest->szAlbum = mir_dupT(src->szAlbum);
	dest->szTitle = mir_dupT(src->szTitle);
	dest->szTrack = mir_dupT(src->szTrack);
	dest->szYear = mir_dupT(src->szYear);
	dest->szGenre = mir_dupT(src->szGenre);
	dest->szLength = mir_dupT(src->szLength);
	dest->szPlayer = mir_dupT(src->szPlayer);
	dest->szType = mir_dupT(src->szType);
}



Player::Player() 
{
	enabled = FALSE;
	needPoll = FALSE;
	ZeroMemory(&listening_info, sizeof(listening_info));
}

Player::~Player()
{
	FreeData();
}


void Player::NotifyInfoChanged()
{
	HasNewListeningInfo();
}


BOOL Player::GetListeningInfo(LISTENINGTOINFO *lti)
{
	if (!enabled)
	{
		FreeData();
		return FALSE;
	}

	if (listening_info.cbSize == 0)
		return FALSE;

	CopyListeningInfo(lti, &listening_info);
	return TRUE;
}

void Player::FreeData()
{
	FreeListeningInfo(&listening_info);
	listening_info.cbSize = 0;
}



PollPlayer::PollPlayer()
{
	needPoll = TRUE;
}



CallbackPlayer::CallbackPlayer()
{
	changed = FALSE;
	InitializeCriticalSection(&cs);
}

CallbackPlayer::~CallbackPlayer()
{
	DeleteCriticalSection(&cs);
}

void CallbackPlayer::FreeData()
{
	EnterCriticalSection(&cs);

	if (listening_info.cbSize != 0)
	{
		Player::FreeData();
		changed = TRUE;
	}

	LeaveCriticalSection(&cs);
}

int CallbackPlayer::ChangedListeningInfo()
{
	int ret;

	EnterCriticalSection(&cs);

	if (!enabled)
	{
		if (listening_info.cbSize == 0)
		{
			ret = 0;
		}
		else
		{
			FreeData();
			ret = -1;
		}
	}

	if (changed)
	{
		changed = FALSE;
		if (listening_info.cbSize == 0)
			ret = -1;
		else
			ret = 1;
	}
	else
	{
		ret = 0;
	}

	LeaveCriticalSection(&cs);

	return ret;
}


CodeInjectionPlayer::CodeInjectionPlayer()
{
	window_class = NULL;
	window_name = NULL;
	message_window_class = NULL;
	next_request_time = 0;
	found_window = FALSE;
}

CodeInjectionPlayer::~CodeInjectionPlayer()
{
}

int CodeInjectionPlayer::ChangedListeningInfo()
{
	if (!enabled || !opts.enable_code_injection || next_request_time > GetTickCount())
		return 0;

	// Window is opened?
	HWND hwnd = FindWindow(window_class, window_name);
	if (hwnd == NULL)
	{
		if (found_window)
		{
			found_window = FALSE;

			FreeData();
			NotifyInfoChanged();
		}
		return 0;
	}

	found_window = TRUE;

	// Msg Window is registered? (aka plugin is running?)
	HWND msgHwnd = FindWindow(message_window_class, NULL);
	if (msgHwnd != NULL)
		return 0;

	// Get the dll path
	char dll_path[1024] = {0};
	if (!GetModuleFileNameA(hInst, dll_path, MAX_REGS(dll_path)))
		return 0;

	char *p = strrchr(dll_path, '\\');
	if (p == NULL)
		return 0;

	p++;
	*p = '\0';

	size_t len = p - dll_path;

#ifdef UNICODE
	mir_snprintf(p, 1024 - len, "listeningto\\mlt_%S.dll", name);
#else
	mir_snprintf(p, 1024 - len, "listeningto\\mlt_%s.dll", name);
#endif

	len = strlen(dll_path);

	// File exists?
	DWORD attribs = GetFileAttributesA(dll_path);
	if (attribs == 0xFFFFFFFF || !(attribs & FILE_ATTRIBUTE_ARCHIVE))
		return 0;

	// Do the code injection
	unsigned long pid;
	GetWindowThreadProcessId(hwnd, &pid);
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION 
									| PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
	if (hProcess == NULL)
		return 0;

	char *_dll = (char *) VirtualAllocEx(hProcess, NULL, len+1, MEM_COMMIT, PAGE_READWRITE );
	if (_dll == NULL)
	{
		CloseHandle(hProcess);
		return 0;
	}
	WriteProcessMemory(hProcess, _dll, dll_path, len+1, NULL);

	HMODULE hKernel32 = GetModuleHandleA("kernel32");
	HANDLE hLoadLibraryA = GetProcAddress(hKernel32, "LoadLibraryA");
	DWORD threadId;
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) hLoadLibraryA, 
										_dll, 0, &threadId);
	if (hThread == NULL)
	{
		VirtualFreeEx(hProcess, _dll, len+1, MEM_RELEASE);
		CloseHandle(hProcess);
		return 0;
	}
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	VirtualFreeEx(hProcess, _dll, len+1, MEM_RELEASE);
	CloseHandle(hProcess);

	next_request_time = GetTickCount() + 11000;

	return 0;
}

BOOL CodeInjectionPlayer::GetListeningInfo(LISTENINGTOINFO *lti)
{
	return FALSE;
}

