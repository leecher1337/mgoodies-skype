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



#define WINDOWCLASS _T("MsnMsgrUIManager")

LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


static UINT hTimer = 0;


WindowsMediaPlayer *singletron = NULL;



WindowsMediaPlayer::WindowsMediaPlayer(int anId) : CallbackPlayer(anId)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= ReceiverWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= WINDOWCLASS;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, WINDOWCLASS, _T("Miranda WILT receiver"), WS_POPUP | WS_MINIMIZE,
		0, 0, 100, 100, NULL, NULL, hInst, NULL);

	received[0] = L'\0';
	singletron = this;
}



WindowsMediaPlayer::~WindowsMediaPlayer()
{
	UnregisterClass(WINDOWCLASS, hInst);
	singletron = NULL;
}



void WindowsMediaPlayer::ProcessReceived()
{
	EnterCriticalSection(&cs);

	changed = TRUE;

	FreeData();

	// Do the processing
	// MSNMusicString = L"\\0Music\\0%d\\0%s\\0%s\\0%s\\0%s\\0%s\\0\\0"
	// MSNMusicString, msn->msncommand, strMSNFormat, msn->title, msn->artist, msn->album, msn->wmcontentid);

	if (received[0] == L'\0')
	{
		LeaveCriticalSection(&cs);
		return;
	}

	// Process string
	WCHAR *parts[8];
	int pCount = 0;
	WCHAR *p = received;
	WCHAR *p1 = wcsstr(p, L"\\0");

	if (p1 == NULL)
	{
		LeaveCriticalSection(&cs);
		return;
	}

	do {
		*p1 = L'\0';
		parts[pCount] = p;
		pCount ++;
		p = p1 + 2;
		p1 = wcsstr(p, L"\\0");
	} while( p1 != NULL && pCount < 7 );
	parts[pCount] = p;

	// Fill cache
	if (pCount > 4 && parts[1][0] != L'\0' && (parts[4][0] != L'\0' || parts[5][0] != L'\0'))
	{
		listening_info.szType = mir_dupTW(parts[1]);
		if (parts[4][0] != '\0') listening_info.szTitle = mir_dupTW(parts[4]);
		if (pCount > 5 && parts[5][0] != '\0') listening_info.szArtist = mir_dupTW(parts[5]);
		if (pCount > 6 && parts[6][0] != '\0') listening_info.szAlbum = mir_dupTW(parts[6]);

		listening_info.cbSize = sizeof(listening_info);
	}

	// Put back the '\\'s
	for(int i = 1; i <= pCount; i++)
		*(parts[i] - 2) = L'\\';

	LeaveCriticalSection(&cs);

	NotifyInfoChanged();
}



VOID CALLBACK SendTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(NULL, hTimer);
	hTimer = 0;

	if (singletron != NULL)
		singletron->ProcessReceived();
}



LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COPYDATA :
		{
			COPYDATASTRUCT* pData = (PCOPYDATASTRUCT) lParam;
			if (pData->dwData != 0x547 || pData->cbData == 0 || pData->lpData == NULL)
				return false;

			if (wcsncmp(singletron->received, (WCHAR*) pData->lpData, min(pData->cbData / 2, 1024)) != 0)
			{
				lstrcpynW(singletron->received, (WCHAR*) pData->lpData, min(pData->cbData / 2, 1024));

				if (hTimer)
					KillTimer(NULL, hTimer);
				hTimer = SetTimer(NULL, NULL, 5, SendTimerProc); // Do the processing after we return true
			}

			return TRUE;
			break;
		}
		case WM_DESTROY :
			PostQuitMessage(0);
			break;

		default :
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}