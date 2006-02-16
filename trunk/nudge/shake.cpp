#include "headers.h"
#include "shake.h"

bool Shaking = false;
bool ShakingChat = false;
int nScaleClist = 5, nScaleChat = 2;
int nMoveClist = 15, nMoveChat = 15;

DWORD WINAPI ShakeChatWindow(LPVOID Param)
{
	if(!ShakingChat)
	{
		Shaking = true;
		HWND hWnd;
		hWnd = (HWND) Param;
		int i;
		RECT rect;
		GetWindowRect(hWnd, &rect);
		for(i = 0; i < nMoveChat; i++)
		{
			SetWindowPos(hWnd, 0, rect.left - nScaleChat, rect.top, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left, rect.top - nScaleChat, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left + nScaleChat, rect.top, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left, rect.top + nScaleChat, 0, 0, SWP_NOSIZE);
			Sleep(10);
		}
		SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOSIZE); //SWP_DRAWFRAME
		ShakingChat = false;
	}
	return 1;
}

DWORD WINAPI ShakeClistWindow(LPVOID Param)
{
	if(!Shaking)
	{
		Shaking = true;
		HWND hWnd;
		hWnd = (HWND) Param;
		int i;
		RECT rect;
		GetWindowRect(hWnd, &rect);
		for(i = 0; i < nMoveClist; i++)
		{
			SetWindowPos(hWnd, 0, rect.left - nScaleClist, rect.top, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left, rect.top - nScaleClist, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left + nScaleClist, rect.top, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left, rect.top + nScaleClist, 0, 0, SWP_NOSIZE);
			Sleep(10);
		}
		SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOSIZE);
		Shaking = false;
	}
	return 1;
}

int ShakeClist( WPARAM wParam, LPARAM lParam )
{
	DWORD tid;
	HWND hWnd;
	hWnd = (HWND) CallService( MS_CLUI_GETHWND, 0, 0 );
	CreateThread(NULL,0,ShakeClistWindow,hWnd,0,&tid);
	return 1;
}

int ShakeChat( WPARAM wParam, LPARAM lParam )
{
	DWORD tid;
	HWND hWnd;
	char srmmName[100];
	MessageWindowData mwd;
	MessageWindowInputData mwid;

	mwd.cbSize = sizeof(MessageWindowData);
	mwd.hContact = Nudge_GethContact((HANDLE) wParam);
	mwd.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	mwid.cbSize = sizeof(MessageWindowInputData);
	mwid.hContact = Nudge_GethContact((HANDLE) wParam);
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;



	CallService( MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd );
	CallService(MS_MSG_GETWINDOWCLASS,(WPARAM)srmmName,(LPARAM)100 );

	if ( !strnicmp( srmmName,"tabSRMM ", 7 ))
		hWnd = GetParent(GetParent(mwd.hwndWindow));
	
	if ( !strnicmp( srmmName,"SRMM ", 4))
		hWnd = mwd.hwndWindow;

	if ( !strnicmp( srmmName,"Scriver ", 7 ))
		hWnd = GetParent(mwd.hwndWindow);

	CreateThread(NULL,0,ShakeChatWindow,hWnd,0,&tid);
	return 1;
}

int TriggerShakeClist( WPARAM wParam, LPARAM lParam )
{
	DWORD tid;
	HWND hWnd;
	int flags;
	flags = (int)wParam;

	if (!flags&ACT_PERFORM)
		return 1;

	hWnd = (HWND) CallService( MS_CLUI_GETHWND, 0, 0 );
	CreateThread(NULL,0,ShakeClistWindow,hWnd,0,&tid);
	return 1;
}

int TriggerShakeChat( WPARAM wParam, LPARAM lParam )
{
	DWORD tid;
	HWND hWnd;
	char srmmName[100];
	MessageWindowData mwd;
	MessageWindowInputData mwid;
	int flags;
	flags = (int)wParam;

	if (!flags&ACT_PERFORM)
		return 1;

	SPECIFICACTIONINFO *sai;
	HANDLE hContact;

	
	sai = (SPECIFICACTIONINFO *) lParam;

	if ( (sai->td != NULL) && (sai->td->dFlags&DF_CONTACT) )
		hContact = sai->td->hContact;
	else
		return 0;

	mwd.cbSize = sizeof(MessageWindowData);
	mwd.hContact = Nudge_GethContact((HANDLE) hContact);
	mwd.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	mwid.cbSize = sizeof(MessageWindowInputData);
	mwid.hContact = Nudge_GethContact((HANDLE) hContact);
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	CallService( MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd );
	CallService(MS_MSG_GETWINDOWCLASS,(WPARAM)srmmName,(LPARAM)100 );

	if ( !strnicmp( srmmName,"tabSRMM ", 7 ))
		hWnd = GetParent(GetParent(mwd.hwndWindow));
	
	if ( !strnicmp( srmmName,"SRMM ", 4))
		hWnd = mwd.hwndWindow;

	if ( !strnicmp( srmmName,"Scriver ", 7 ))
		hWnd = GetParent(mwd.hwndWindow);

	CreateThread(NULL,0,ShakeChatWindow,hWnd,0,&tid);
	return 1;
}