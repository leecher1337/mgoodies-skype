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


static LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


static UINT hTimer = NULL;


GenericPlayer *singleton = NULL;



GenericPlayer::GenericPlayer()
{
	enabled = TRUE;
	received[0] = L'\0';
	singleton = this;

	WNDCLASS wc = {0};
	wc.lpfnWndProc		= ReceiverWndProc;
	wc.hInstance		= hInst;
	wc.lpszClassName	= MIRANDA_WINDOWCLASS;

	RegisterClass(&wc);

	hWnd = CreateWindow(MIRANDA_WINDOWCLASS, _T("Miranda ListeningTo receiver"), 
						0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
}



GenericPlayer::~GenericPlayer()
{
	if (hTimer != NULL)
		KillTimer(NULL, hTimer);

	UnregisterClass(MIRANDA_WINDOWCLASS, hInst);
	singleton = NULL;
}



void GenericPlayer::ProcessReceived()
{
	EnterCriticalSection(&cs);

	// Do the processing
	// L"<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0\\0"

	WCHAR *p1 = wcsstr(received, L"\\0");

	if (received[0] == L'\0' || p1 == NULL)
	{
		// Ignore
		LeaveCriticalSection(&cs);
		return;
	}

	// Process string
	WCHAR *parts[11] = {0};
	int pCount = 0;
	WCHAR *p = received;
	do {
		*p1 = L'\0';
		parts[pCount] = p;
		pCount ++;
		p = p1 + 2;
		p1 = wcsstr(p, L"\\0");
	} while( p1 != NULL && pCount < 10 );
	parts[pCount] = p;

	if (pCount < 5)
	{
		// Ignore
		LeaveCriticalSection(&cs);
		return;
	}

	// See if player is enabled
	int i;
	for (i = FIRST_PLAYER; i < NUM_PLAYERS; i++)
	{
#ifdef UNICODE
		WCHAR *player_name = players[i]->name;
#else
		WCHAR player_name[128];
		MultiByteToWideChar(CP_ACP, 0, players[i]->name, -1, player_name, MAX_REGS(player_name));
#endif
		if (_wcsicmp(parts[1], player_name) == 0)
			break;
	}

	if ((i == NUM_PLAYERS && !opts.enable_other_players) 
		|| (i != NUM_PLAYERS && !players[i]->enabled))
	{
		// Ignore
		LeaveCriticalSection(&cs);
		return;
	}

	FreeData();

	changed = TRUE;

	if (wcscmp(L"1", parts[0]) != 0 || parts[1][0] == L'\0' || (parts[3][0] == L'\0' && parts[4][0] == L'\0'))
	{
		// Stoped playing or not enought info
		LeaveCriticalSection(&cs);
		NotifyInfoChanged();
		return;
	}

	listening_info.cbSize = sizeof(listening_info);
	listening_info.dwFlags = LTI_TCHAR;

	listening_info.ptszType = mir_dupTW(parts[2][0] == L'\0' ? L"Music" : parts[2]);
	listening_info.ptszArtist = mir_dupTW(parts[4]);
	listening_info.ptszAlbum = mir_dupTW(parts[5]);
	listening_info.ptszTitle = mir_dupTW(parts[3]);
	listening_info.ptszTrack = mir_dupTW(parts[6]);
	listening_info.ptszYear = mir_dupTW(parts[7]);
	listening_info.ptszGenre = mir_dupTW(parts[8]);

	if (i == NUM_PLAYERS)
		listening_info.ptszPlayer = mir_dupTW(parts[1]);
	else
		listening_info.ptszPlayer = mir_tstrdup(players[i]->name);

	if (parts[9] != NULL)
	{
		long length = _wtoi(parts[9]);
		if (length > 0)
		{
			listening_info.ptszLength = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));

			int s = length % 60;
			int m = (length / 60) % 60;
			int h = (length / 60) / 60;

			if (h > 0)
				mir_sntprintf(listening_info.ptszLength, 9, _T("%d:%02d:%02d"), h, m, s);
			else
				mir_sntprintf(listening_info.ptszLength, 9, _T("%d:%02d"), m, s);
		}
	}

	// Put back the '\\'s
	for(i = 1; i <= pCount; i++)
		*(parts[i] - 2) = L'\\';

	wcscpy(last_received, received);

	LeaveCriticalSection(&cs);

	NotifyInfoChanged();
}


static VOID CALLBACK SendTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(NULL, hTimer);
	hTimer = NULL;

	if (singleton != NULL)
		singleton->ProcessReceived();
}


void GenericPlayer::NewData(WCHAR *data, size_t len)
{
	if (data[0] == L'\0')
		return;

	EnterCriticalSection(&cs);

	if (wcsncmp(received, data, min(len, 1024)) != 0)
	{
		wcsncpy(received, data, min(len, 1024));

		if (hTimer)
			KillTimer(NULL, hTimer);
		hTimer = SetTimer(NULL, NULL, 300, SendTimerProc); // Do the processing after we return true
	}

	LeaveCriticalSection(&cs);
}


static LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COPYDATA:
		{
			COPYDATASTRUCT* pData = (PCOPYDATASTRUCT) lParam;
			if (pData == NULL || pData->dwData != MIRANDA_DW_PROTECTION 
					|| pData->cbData == 0 || pData->lpData == NULL)
				return FALSE;

			singleton->NewData((WCHAR *) pData->lpData, pData->cbData / 2);

			return TRUE;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default :
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
