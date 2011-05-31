/* 
ListeningTo plugin for Miranda IM
==========================================================================
Copyright	(C) 2005-2011 Ricardo Pescuma Domenecci
			(C) 2010-2011 Merlin_de

PRE-CONDITION to use this code under the GNU General Public License:
 1. you do not build another Miranda IM plugin with the code without written permission
    of the autor (peace for the project).
 2. you do not publish copies of the code in other Miranda IM-related code repositories.
    This project is already hosted in a SVN and you are welcome to become a contributing member.
 3. you do not create listeningTo-derivatives based on this code for the Miranda IM project.
    (feel free to do this for another project e.g. foobar)
 4. you do not distribute any kind of self-compiled binary of this plugin (we want continuity
    for the plugin users, who should know that they use the original) you can compile this plugin
    for your own needs, friends, but not for a whole branch of people (e.g. miranda plugin pack).
 5. This isn't free beer. If your jurisdiction (country) does not accept
    GNU General Public License, as a whole, you have no rights to the software
    until you sign a private contract with its author. !!!
 6. you always put these notes and copyright notice at the beginning of your code.
==========================================================================

in case you accept the pre-condition,
this is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "..\commons.h"

static UINT_PTR			hSendTimer = NULL;
static GenericPlayer	*singleton = NULL;

static LRESULT	CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static VOID		CALLBACK SendTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

int 
m_log(const TCHAR *function, const TCHAR *fmt, ...)
{
#if 0
    va_list va;
    TCHAR text[1024];
	size_t len;

	mir_sntprintf(text, MAX_REGS(text) - 10, _T("[%08u - %08u] [%s] "), 
				 GetCurrentThreadId(), GetTickCount(), function);
	len = lstrlen(text);

    va_start(va, fmt);
    mir_vsntprintf(&text[len], MAX_REGS(text) - len, fmt, va);
    va_end(va);

	BOOL writeBOM = (GetFileAttributes(_T("c:\\miranda_listeningto.log.txt")) == INVALID_FILE_ATTRIBUTES);

	FILE *fp = _tfopen(_T("c:\\miranda_listeningto.log.txt"), _T("ab"));

	if (fp != NULL)
	{
#ifdef UNICODE
		if (writeBOM)
			fwprintf(fp, L"\xFEFF");
#endif

		_ftprintf(fp, _T("%s\r\n"), text);
		fclose(fp);
		return 0;
	}
	else
	{
		return -1;
	}
#else
	return 0;
#endif
}


/////////////////////////////////////////////////////////////////////////////
// main

GenericPlayer::GenericPlayer(int index)
: Player(index)
{
	m_name = _T("GenericPlayer");

	m_enabled = TRUE;
	received[0] = L'\0';
	singleton = this;

	WNDCLASS wc = {0};
	wc.lpfnWndProc		= ReceiverWndProc;
	wc.hInstance		= hInst;
	wc.lpszClassName	= MIRANDA_WINDOWCLASS;

	cWndclass = RegisterClass(&wc);
	DWORD err = GetLastError();
	if (!cWndclass) {
		TCHAR msg[1024];
		wsprintf(msg, TranslateT("Failed to register %s class."),wc.lpszClassName);
		MessageBox(NULL, msg, _T(MODULE_NAME), MB_ICONSTOP|MB_OK);
	}

	m_hwnd = CreateWindow(MIRANDA_WINDOWCLASS, _T("Miranda ListeningTo receiver"), 
						0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
}

GenericPlayer::~GenericPlayer()
{
	KILLTIMER(hSendTimer);

	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	//UnregisterClass(MIRANDA_WINDOWCLASS, hInst);
	UnregisterClass (MAKEINTATOM(cWndclass),hInst);
	cWndclass = 0;

	singleton = NULL;
}

void 
GenericPlayer::ProcessReceived()
{
	EnterCriticalSection(&cs);

	// Do the processing
	// L"<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0\\0"

	WCHAR *p1 = wcsstr(received, L"\\0");

	if (IsEmpty(received) || p1 == NULL)
	{
//		if (received[0] == L'\0')
//			m_log(_T("ProcessReceived"), _T("ERROR: Empty text"));
//		else
//			m_log(_T("ProcessReceived"), _T("ERROR: No \\0 found"));

		// Ignore
		LeaveCriticalSection(&cs);
		return;
	}

	// Process string
	int i;
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
	if (p1 != NULL)
		*p1 = L'\0';
	parts[pCount] = p;

	// select known player (default is generic = this)
	Player *player = m_enabled ? this : NULL;
	for (i = FIRST_PLAYER; i < NUM_PLAYERS; i++)
	{
		#ifdef UNICODE
			WCHAR *player_name = players[i]->m_name;
		#else
			WCHAR player_name[128];
			MultiByteToWideChar(CP_ACP, 0, players[i]->m_name, -1, player_name, MAX_REGS(player_name));
		#endif
		if (_wcsicmp(parts[1], player_name) == 0)
		{
			player = players[i];
			break;
		}
	}

	//is player enabled
	if(!player || !player->m_enabled) {
		LeaveCriticalSection(&cs);
		return;
	}

	//set player status
	SetActivePlayer(player->m_index, player->m_index);
	int status = IsEmpty(parts[0]) ? 0 : _wtoi(parts[0]);
	switch(status){
		case 0:
			player->m_state = player->GetStatus() ? PL_STOPPED : PL_OFFLINE;
			break;
		case 1:
			player->m_state = PL_PLAYING;
			break;
	}

	player->FreeData();
	if (pCount < 5 || wcscmp(L"1", parts[0]) != 0 || IsEmpty(parts[1]) || (IsEmpty(parts[3]) && IsEmpty(parts[4])))
	{
		// Stoped playing or not enought info
		player->m_state = PL_OFFLINE;
//		SetActivePlayer(player->m_index, -1);

//		if (wcscmp(L"1", parts[0]) != 0)
//			m_log(_T("ProcessReceived"), _T("END: Stoped playing"));
//		else
//			m_log(_T("ProcessReceived"), _T("ERROR: not enought info"));
	}
	else
	{
		SetActivePlayer(player->m_index, player->m_index);

		LISTENINGTOINFO *li = player->LockListeningInfo();

		li->cbSize		= sizeof(m_listening_info);
		li->dwFlags		= LTI_TCHAR;
		li->ptszType	= U2TD(parts[2], L"Music");
		li->ptszTitle	= U2T(parts[3]);
		li->ptszArtist	= U2T(parts[4]);
		li->ptszAlbum	= U2T(parts[5]);
		li->ptszTrack	= U2T(parts[6]);
		li->ptszYear	= U2T(parts[7]);
		li->ptszGenre	= U2T(parts[8]);

		if (player == this)
			li->ptszPlayer = mir_u2t(parts[1]);
		else
			li->ptszPlayer = mir_tstrdup(player->m_name);

		if (parts[9] != NULL)
		{
			long length = _wtoi(parts[9]);
			if (length > 0)
			{
				li->ptszLength = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));

				int s = length % 60;
				int m = (length / 60) % 60;
				int h = (length / 60) / 60;

				if (h > 0)
					mir_sntprintf(li->ptszLength, 9, _T("%d:%02d:%02d"), h, m, s);
				else
					mir_sntprintf(li->ptszLength, 9, _T("%d:%02d"), m, s);
			}
		}

		player->ReleaseListeningInfo();
	}

	// Put back the '\\'s
	for(i = 1; i <= pCount; i++)
		*(parts[i] - 2) = L'\\';
	if (p1 != NULL)
		*p1 = L'\\';

	wcscpy(last_received, received);

	LeaveCriticalSection(&cs);

	NotifyInfoChanged(player->m_index);

//	m_log(_T("ProcessReceived"), _T("END: Success"));
}

void 
GenericPlayer::NewData(const WCHAR *data, size_t len)
{
//	m_log(_T("NewData"), _T("Processing"));

	if (data[0] == L'\0')
	{
//		m_log(_T("NewData"), _T("ERROR: Text is empty"));
		return;
	}

	EnterCriticalSection(&cs);

	len = min(len, 1023);
	if (wcsncmp(received, data, len) != 0)
	{
//		m_log(_T("NewData"), _T("Got new text, scheduling update"));

		wcsncpy(received, data, len);
		received[len] = L'\0';

//#ifdef UNICODE
//		m_log(_T("NewData"), _T("Text: %s"), received);
//#else
//		m_log(_T("NewData"), _T("Text: %S"), received);
//#endif

		KILLTIMER(hSendTimer);
		hSendTimer = SetTimer(NULL, NULL, 300, (TIMERPROC)SendTimerProc); // Do the processing after we return true
	}
//	else
//	{
//		m_log(_T("NewData"), _T("END: Text is the same as last time"));
//	}

	LeaveCriticalSection(&cs);
}

static VOID 
CALLBACK SendTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (!loaded)
		return;
	KILLTIMER(hSendTimer);

//	m_log(_T("SendTimerProc"), _T("It's time to process"));

	if (singleton != NULL)
		singleton->ProcessReceived();
}

static LRESULT 
CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COPYDATA:
		{
			if (!loaded)
				return FALSE;

//			m_log(_T("ReceiverWndProc"), _T("START: Received message"));

			COPYDATASTRUCT* pData = (PCOPYDATASTRUCT) lParam;
			if (pData == NULL || pData->dwData != MIRANDA_DW_PROTECTION 
					|| pData->cbData == 0 || pData->lpData == NULL)
			{
/*				if (pData == NULL)
					m_log(_T("ReceiverWndProc"), _T("ERROR: COPYDATASTRUCT* is NULL"));
				else if (pData->dwData != MIRANDA_DW_PROTECTION)
					m_log(_T("ReceiverWndProc"), _T("ERROR: pData->dwData is incorrect"));
				else if (pData->cbData == 0)
					m_log(_T("ReceiverWndProc"), _T("ERROR: pData->cbData is 0"));
				else if (pData->lpData == NULL)
					m_log(_T("ReceiverWndProc"), _T("ERROR: pData->lpData is NULL"));
*/
				return FALSE;
			}

//			m_log(_T("ReceiverWndProc"), _T("Going to process"));
			if (singleton != NULL)
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
