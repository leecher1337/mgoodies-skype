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



Winamp::Winamp()
{
	filename[0] = '\0';

	hwnd = NULL;
	process = NULL;

	_fi = NULL;
	_ret = NULL;
	_param = NULL;

	ret[0] = '\0';
}


void Winamp::FindWindow()
{
	hwnd = ::FindWindow(_T("Winamp v1.x"), NULL);
	
	if (hwnd == NULL)
		hwnd = ::FindWindow(_T("Winamp 3.x"), NULL);
}


BOOL Winamp::InitTempData()
{
	// Create vars there
	_fi = (extendedFileInfoStruct *) VirtualAllocEx(process, NULL, sizeof(extendedFileInfoStruct), MEM_COMMIT, PAGE_READWRITE);
	if (_fi == NULL)
		return FALSE;

	_param = (char *) VirtualAllocEx(process, NULL, 128, MEM_COMMIT, PAGE_READWRITE);
	if (_param == NULL)
		return FALSE;

	_ret = (char *) VirtualAllocEx(process, NULL, sizeof(ret), MEM_COMMIT, PAGE_READWRITE);
	if (_ret == NULL)
		return FALSE;

	// Init mine
	fi.ret = _ret;
	fi.retlen = sizeof(ret);
	fi.metadata = _param;

	// Copy to there
	return WriteProcessMemory(process, _fi, &fi, sizeof(fi), NULL);
}


void Winamp::FreeTempData()
{
	if (_fi != NULL) 
	{
		VirtualFreeEx(process, _fi, 0, MEM_RELEASE);
		_fi = NULL;
	}

	if (_ret != NULL)
	{
		VirtualFreeEx(process, _ret, 0, MEM_RELEASE);
		_ret = NULL;
	}

	if (_param != NULL)
	{
		VirtualFreeEx(process, _param, 0, MEM_RELEASE);
		_param = NULL;
	}

	if (process != NULL)
	{
		CloseHandle(process);
		process = NULL;
	}
}


void Winamp::FreeData()
{
	PollPlayer::FreeData();
	FreeTempData();
}


int Winamp::GetMetadata(char *metadata, TCHAR **data)
{
	if (!WriteProcessMemory(process, _param, metadata, strlen(metadata) + 1, NULL))
		return -1;

	if (SendMessage(hwnd, WM_WA_IPC, (WPARAM) _fi, IPC_GET_EXTENDED_FILE_INFO_HOOKABLE))
	{
		if (!ReadProcessMemory(process, _ret, ret, sizeof(ret), NULL))
			return -1;

		ret[sizeof(ret) - 1] = '\0';
		if (data != NULL && ret[0] != '\0')
			*data = mir_dupTA(ret);
		return 1;
	}
	else
	{
		return 0;
	}
}


// Init data and put filename playing in ret and fi.filename
BOOL Winamp::InitAndGetFilename()
{
	// Find window
	FindWindow();
	if (hwnd == NULL)
		return FALSE;

	if (SendMessage(hwnd, WM_WA_IPC, 0, IPC_ISPLAYING) != 1)
		return FALSE;

	// Get process
	unsigned long pid;
	GetWindowThreadProcessId(hwnd, &pid);
	process = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (process == NULL)
		return FALSE;

	// Get filename
	int pos = SendMessage(hwnd, WM_WA_IPC, 0, IPC_GETLISTPOS);
	char *_filename = (char *) SendMessage(hwnd, WM_WA_IPC, pos, IPC_GETPLAYLISTFILE);
	if (_filename == NULL)
		return FALSE;

	// Copy to here
	ReadProcessMemory(process, _filename, ret, sizeof(ret), NULL);
	ret[sizeof(ret) - 1] = '\0';

	fi.filename = _filename;

	return TRUE;
}


BOOL Winamp::FillCache()
{
	if (!InitTempData()) return FALSE;

	if (GetMetadata("ARTIST", &listening_info.szArtist) < 0) return FALSE;
	if (GetMetadata("ALBUM", &listening_info.szAlbum) < 0) return FALSE;
	if (GetMetadata("TITLE", &listening_info.szTitle) < 0) return FALSE;
	if (GetMetadata("YEAR", &listening_info.szYear) < 0) return FALSE;
	if (GetMetadata("TRACK", &listening_info.szTrack) < 0) return FALSE;
	if (GetMetadata("GENRE", &listening_info.szGenre) < 0) return FALSE;

	int err = GetMetadata("LENGTH", NULL);
	if (err < 0) 
	{
		return FALSE;
	}
	else if (err > 0)
	{
		listening_info.szLength = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));

		int length = atoi(ret) / 1000;
		int s = length % 60;
		int m = (length / 60) % 60;
		int h = (length / 60) / 60;

		if (h > 0)
			mir_sntprintf(listening_info.szLength, 9, _T("%d:%02d:%02d"), h, m, s);
		else
			mir_sntprintf(listening_info.szLength, 9, _T("%d:%02d"), m, s);
	}

	if (SendMessage(hwnd, WM_WA_IPC, 3, IPC_GETINFO))
		listening_info.szType = mir_dupT(_T("Video"));
	else
		listening_info.szType = mir_dupT(_T("Music"));

	if (listening_info.szTitle == NULL)
	{
		// Get from filename
		char *p = strrchr(filename, '\\');
		if (p != NULL)
			p++;
		else
			p = filename;
		
		listening_info.szTitle = mir_dupTA(p);

		TCHAR *pt = _tcsrchr(listening_info.szTitle, '.');
		if (pt != NULL)
			*p = _T('\0');
	}

	listening_info.szPlayer = mir_dupT(_T("Winamp"));

	listening_info.cbSize = sizeof(listening_info);

	return TRUE;
}


// < 0 removed
// 0 not changed
// > 0 changed
int Winamp::ChangedListeningInfo()
{
	if (!enabled || !InitAndGetFilename() || ret[0] == '\0')
	{
		FreeData();
		if (filename[0] != '\0')
		{
			filename[0] = '\0';
			return -1;
		}
		else
			return 0;
	}

	if (strcmpnull(filename, ret) == 0)
		return 0;

	// Fill the data cache
	FreeListeningInfo(&listening_info);

	strcpy(filename, ret);

	if (!FillCache())
	{
		FreeData();
		if (filename[0] != '\0')
		{
			filename[0] = '\0';
			return -1;
		}
		else
			return 0;
	}
	else
	{
		FreeTempData();
		return 1;
	}
}
