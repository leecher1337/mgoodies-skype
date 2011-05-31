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

extern Player *players[NUM_PLAYERS];
extern int activePlayer;

/////////////////////////////////////////////////////////////////////////////
// WLM Windows Live Messanger  forward declaration

#define WLM_WINDOWCLASS _T("MsnMsgrUIManager")

static LRESULT CALLBACK WLM_ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static VOID    CALLBACK WLM_SendTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

static UINT_PTR hSendTimer = NULL;
static WindowsLiveMessanger *singleton = NULL;

typedef struct{
  WCHAR*	pID;
  TCHAR*	pClass;
  BOOL*		off;
} wlmclass ;

static wlmclass wlmcs[] = {
	{L"WLM",	_T("WMPlayerApp"),		&players[WMP]->m_enabled},	//WMP latest
	{L"WLM",	_T("WMP Skin Host"),	&players[WMP]->m_enabled},	//WMP latest (Skin Host window)
	{L"WLM",	_T("Media Player 2"),	&players[WMP]->m_enabled},	//WMP old
	{L"iTunes",	_T("iTunes"),			&players[ITUNES]->m_enabled},	//
	{L"",		NULL,					NULL}	//
};

static TCHAR *wcs[] = {
		_T("WMPlayerApp"),		//WMP latest
		_T("WMP Skin Host"),	//WMP latest (Skin Host window)
		_T("Media Player 2")	//WMP old
};

/////////////////////////////////////////////////////////////////////////////
// common

WindowsLiveMessanger::WindowsLiveMessanger(int index)
: Player(index)
{
	m_name					= _T("WindowsMediaPlayer");
	m_window_classes		= wcs;
	m_window_classes_num	= MAX_REGS(wcs);

//	m_comAppH				= NULL;
//	m_comApp				= NULL;
//	m_comAppEventSink		= NULL;
//	m_comRet				= NULL;
//	m_vbServerState			= VARIANT_FALSE;
	m_state					= PL_OFFLINE;
//	m_pView					= NULL;
//	_Module.Init(NULL, hInst, &LIBID_ATLLib);
}

WindowsLiveMessanger::~WindowsLiveMessanger()
{
	switch(m_enabled){
		case 1:		//WLM
			WLM_Stop();
			break;
	}
	FreeData();
}

void 
WindowsLiveMessanger::EnableDisable()
{
	static BOOL old = FALSE;
	if(m_enabled == old) return;

	switch(old){				//stop ...
		case 0:					break;
		case 1:	WLM_Stop();		break;		//WLM
	}
	switch(m_enabled){			//start ...
		case 0:	{
			if(loaded)
				NotifyInfoChanged();
		}		break;
		case 1:	WLM_Start();	break;		//WLM
	}
	old = m_enabled;
}

BOOL 
WindowsLiveMessanger::GetListeningInfo(LISTENINGTOINFO *lti)
{
	switch(m_enabled){
		case 1:		//COM
//			FreeData();
//			return COM_infoCache() ? Player::GetListeningInfo(lti) : FALSE;
		case 2:		//WLM
			if(m_needPoll && FindWindow() == NULL)
				return FALSE;
			return Player::GetListeningInfo(lti);
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// WLM ...Windows Live Messanger

BOOL
WindowsLiveMessanger::WLM_Start()
{
	// Create windows class
	WNDCLASS wc				= {0};
	wc.lpfnWndProc			= WLM_ReceiverWndProc;
	wc.hInstance			= hInst;
	wc.lpszClassName		= WLM_WINDOWCLASS;

	cWndclass = RegisterClass(&wc);
	DWORD err = GetLastError();

	if (!cWndclass) {
		TCHAR msg[1024];
		wsprintf(msg, TranslateT("Failed to register %s class."),wc.lpszClassName);
		MessageBox(NULL, msg, _T(MODULE_NAME), MB_ICONSTOP|MB_OK);

		singleton = NULL;
		return FALSE;
	}
	// Create a window.
	m_received[0]			= L'\0';
	m_last_received[0]		= L'\0';
	singleton				= this;
	ZeroMemory(&m_window_class, sizeof m_window_class);
	m_hwndclass = CreateWindow(WLM_WINDOWCLASS, _T("Miranda ListeningTo WLM receiver"), 
								0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);

	return m_hwndclass == NULL ? FALSE : TRUE;
}

BOOL 
WindowsLiveMessanger::WLM_Stop()
{
	KILLTIMER(hSendTimer);
	m_state = PL_OFFLINE;
	DestroyWindow(m_hwndclass);
	m_hwndclass = NULL;
	UnregisterClass (MAKEINTATOM(cWndclass),hInst);
	cWndclass = 0;
	singleton = NULL;

	return TRUE;
}

HWND
WindowsLiveMessanger::FindWindow()
{
	HWND hwnd = NULL;
	for(int i = 0; i < m_window_classes_num; i++) {
		hwnd = ::FindWindow(m_window_classes[i], NULL);
		if (hwnd != NULL)
			break;
	}
	if (hwnd != m_hwnd) {
		m_hwnd = hwnd;
	}
	return m_hwnd;
}

BYTE 
WindowsLiveMessanger::GetStatus()
{
	if(FindWindow() == 0)
		m_state = PL_OFFLINE;
	return m_state;
}

void 
WindowsLiveMessanger::WLM_NewData(const WCHAR *data, size_t len)
{
	EnterCriticalSection(&cs);

	OutputDebugStringW(L"WLM_mess:\t");
	OutputDebugStringW(data);
	OutputDebugStringW(L"\n");

	len = min(len, 1023);
	if (wcsncmp(m_received, data, len) != 0)
	{
		wcsncpy(m_received, data, len);
		m_received[len] = L'\0';

/*#ifdef UNICODE
		m_log(_T("WLM_ReceiverWndProc"), _T("WMP : New data: [%d] %s"), len, received);
#else
		m_log(_T("WLM_ReceiverWndProc"), _T("WMP : New data: [%d] %S"), len, received);
#endif
*/
		KILLTIMER(hSendTimer);
		hSendTimer = SetTimer(NULL, NULL, 1000/*300*/, (TIMERPROC)WLM_SendTimerProc); // Do the processing after we return true
	}
/*	else
	{
		m_log(_T("WLM_NewData"), _T("END: Text is the same as last time"));
	}
*/
	LeaveCriticalSection(&cs);
}


void /*data received from LiveMessanger Status plugin*/ 
WindowsLiveMessanger::WLM_ProcessReceived()
{
	EnterCriticalSection(&cs);
	bool send = true;
	FreeData();

	/*/ Do the processing
	/* MSNActivityString = L"<PlayerID>\\0<ActivityType>\\0<bool>%d\\0<message>%s\\0<title>%s\\0<artist>%s\\0<album>%s\\0<WMContentID>%s\\<NULL>0"
		PlayerID:		'VLC'		WindowsMediaPlayer send 'WMP'
		ActivityType:	'Music'		show music-icon and link to msn online shop
						'Games'		show game-icon, no link
						'Office'	show office-icon
						'Empty'		show no icon
						'Video'		not supportet by original WLM but other Messengers use it (show Video icon)
						'Radio'		not supportet by original WLM but other Messengers use it (show Radio icon)
		bool:			0			activity off - use private status message (always send 0 if player stop !!!)
						1			activity on  - use activity message
		message:		activity message (use variable {.} or not ...
						'{0}'		<title>
						'{1}'		<artist>
						'{2}'		<album>
		WMContentID:	WindowsMedia-Content ID	- GUID identifying the content (GUID for mp3, wav etc)
						the WMContentID field doesn't appear to affect anything (though the iTunes code sends iTunes instead?
		NULL:			NULL termination

		eg:
		L"VLC\\0Music\\01\\0playing: {0} - {1} ({2})\\0Endless Quest\\0Enigma\\015 Years After\\0\\0"
			show message: 'playing: Endless Quest - Enigma (15 Years After)' with icon-music

		L"VLC\\0Radio\\01\\0playing: {0} - {1} ({2})\\0Endless Quest\\0Enigma\\http://www.rockantenne.de/webradio/rockantenne.wmx\\0\\0"
			show message: 'listening Radio: Endless Quest - Enigma (http://www.rockantenne.de/webradio/rockantenne.wmx)' with icon-radio(if supportet)

		L"VLC\\0Video\\01\\0watching: {0} - {1} ({2})\\0Rambo II\\0chapter 1\\0\\0\\0"
			show message: 'watching: Rambo II - chapter 1' with icon-Video (if supportet)

		L"VLC\0Music\00\0\0\0\0\0\0"
			disable activity message and show status message from messanger
	*/

	WCHAR *p1 = wcsstr(m_received, L"\\0");

	if (m_received[0] == L'\0' || p1 == NULL)
	{
		LeaveCriticalSection(&cs);
		NotifyInfoChanged();
		return;
	}

	// Process string
	WCHAR *parts[8] = {0};
	int pCount = 0;
	WCHAR *p = m_received;
	do {
		*p1 = L'\0';
		parts[pCount] = p;
		pCount ++;
		p = p1 + 2;
		p1 = wcsstr(p, L"\\0");
	} while( p1 != NULL && pCount < 7 );
	if (p1 != NULL)
		*p1 = L'\0';
	parts[pCount] = p;


	//identify the player
	HWND hwnd = NULL;
	if(m_window_class[0] == NULL){
		send = false;
		m_hwnd = NULL;
		for(int i = 0; i < SIZEOF(wlmcs); i++) {
			if(_wcsicmp(parts[0], wlmcs[i].pID) == 0) {
				if(wlmcs[i].off) {	//	wcsncmp
					break;
				}
				else if (NULL == (hwnd = ::FindWindow(wlmcs[i].pClass, WcharToTchar(parts[4])))) {
					continue;
				}
				send = true;
				m_hwnd = hwnd;
				_tcscpy(m_window_class ,wlmcs[i].pClass);
				m_window_classes[0] = m_window_class;
				m_window_classes_num = 1;
				break;
			}
		}
	}
	//set player status
	int status = IsEmpty(parts[2]) ? 0 : _wtoi(parts[2]);
	switch(status){
		case 0:
			m_state = GetStatus() ? PL_STOPPED : PL_OFFLINE;
			break;
		case 1:
			m_state = PL_PLAYING;
			break;
	}

	// Fill cache
	if (pCount > 4 && !IsEmpty(parts[1]) && (!IsEmpty(parts[4]) || !IsEmpty(parts[5])))
	{
		m_listening_info.cbSize		= sizeof(m_listening_info);
		m_listening_info.dwFlags	= LTI_TCHAR;

		m_listening_info.ptszType	= U2T(parts[1]);
		m_listening_info.ptszTitle	= U2T(parts[4]);
		m_listening_info.ptszArtist	= U2T(parts[5]);
		m_listening_info.ptszAlbum	= U2T(parts[6]);

		m_listening_info.ptszPlayer = mir_tstrdup(m_name);
	}


	// Put back the '\\'s
	for(int i = 1; i <= pCount; i++)
		*(parts[i] - 2) = L'\\';
	if (p1 != NULL)
		*p1 = L'\\';

	LeaveCriticalSection(&cs);

	NotifyInfoChanged();
}

VOID 
CALLBACK WLM_SendTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (!loaded)
		return;
	KILLTIMER(hSendTimer);

	if (singleton != NULL)
		singleton->WLM_ProcessReceived();
}



LRESULT 
CALLBACK WLM_ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	switch (message)
	{
		case WM_COPYDATA :
		{
			if (!loaded)
				return FALSE;

			if(	singleton == NULL || 
				!singleton->m_enabled ||
				(activePlayer!=singleton->m_index && activePlayer!=-1) )
				return FALSE;

			COPYDATASTRUCT* pData = (PCOPYDATASTRUCT) lParam;
			if (pData->dwData != 0x547 || pData->cbData == 0 || pData->lpData == NULL)
				return FALSE;

			if(wcsncmp((WCHAR *)pData->lpData, L"WMP", 3) == 0 && players[WMP]->m_enabled)
				return FALSE;


//			LPWSTR p  = (WCHAR *) pData->lpData;
//			LPWSTR p1 = wcsstr(p, L"\\0");

			if (singleton != NULL) {
				if( wParam &&
					GetClassName((HWND) wParam, singleton->m_window_class, SIZEOF(singleton->m_window_class)))
				{
					wParam, singleton->m_window_classes = (TCHAR **)&singleton->m_window_class;
					singleton->m_window_classes_num = 1;
				}
				bool fBlocked = ( InSendMessageEx(NULL) & (ISMEX_REPLIED|ISMEX_SEND) ) == ISMEX_SEND;
				if (fBlocked) {
					ReplyMessage(TRUE); 
				}
				singleton->WLM_NewData((WCHAR *) pData->lpData, pData->cbData / 2);
			}

			return TRUE;
		}
		case WM_DESTROY :
			SetActivePlayer(singleton->m_index, -1);
//			PostQuitMessage(0);
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


