/*
SRMM

Copyright 2000-2005 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "commonheaders.h"
#include "richutil.h"

/*
	To initialize this library, call:
		RichUtil_Load();
	Before the application exits, call:
		RichUtil_Unload();

	Then to use the library (it draws the xp border around it), you need
	to make sure you control has the WS_EX_CLIENTEDGE flag.  Then you just
	subclass it with:
		RichUtil_SubClass(hwndEdit);

	If no xptheme is present, the window isn't subclassed the SubClass function
	just returns.  And if WS_EX_CLIENTEDGE isn't present, the subclass does nothing.
	Otherwise it removes the border and draws it by itself.
*/

static struct LIST_INTERFACE li;
static SortedList sListInt;

static int RichUtil_CmpVal(void *p1, void *p2) {
	TRichUtil *tp1 = (TRichUtil*)p1;
	TRichUtil *tp2 = (TRichUtil*)p2;
	if (tp1->hwnd==tp2->hwnd)
		return 0;
	return (int)((int)tp1->hwnd-(int)tp2->hwnd);
}

// UxTheme Stuff
static HMODULE mTheme = 0;
static HANDLE  (WINAPI *MyOpenThemeData)(HWND,LPCWSTR) = 0;
static HRESULT (WINAPI *MyCloseThemeData)(HANDLE) = 0;
static BOOL    (WINAPI *MyIsThemeActive)() = 0;
static HRESULT (WINAPI *MyDrawThemeBackground)(HANDLE,HDC,int,int,const RECT*,const RECT *) = 0;
static HRESULT (WINAPI *MyGetThemeBackgroundContentRect)(HANDLE,HDC,int,int,const RECT *,RECT *) = 0;
static HRESULT (WINAPI *MyDrawThemeParentBackground)(HWND,HDC,RECT*) = 0;
static BOOL    (WINAPI *MyIsThemeBackgroundPartiallyTransparent)(HANDLE,int,int) = 0;

static CRITICAL_SECTION csRich;

static LRESULT CALLBACK RichUtil_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void RichUtil_ClearUglyBorder(TRichUtil *ru);

void RichUtil_Load() {
	li.cbSize = sizeof(li);
	CallService(MS_SYSTEM_GET_LI,0,(LPARAM)&li);
	ZeroMemory(&sListInt, sizeof(sListInt));
	sListInt.increment = 10;
	sListInt.sortFunc = RichUtil_CmpVal;
	mTheme = RIsWinVerXPPlus()?LoadLibraryA("uxtheme.dll"):0;
	InitializeCriticalSection(&csRich);
	if (!mTheme) return;
	MyOpenThemeData = (HANDLE (WINAPI *)(HWND, LPCWSTR))GetProcAddress(mTheme, "OpenThemeData");
	MyCloseThemeData = (HRESULT (WINAPI *)(HANDLE))GetProcAddress(mTheme, "CloseThemeData");
	MyIsThemeActive = (BOOL (WINAPI *)())GetProcAddress(mTheme, "IsThemeActive");
	MyDrawThemeBackground = (HRESULT (WINAPI *)(HANDLE, HDC, int, int, const RECT*, const RECT *))GetProcAddress(mTheme, "DrawThemeBackground");
	MyGetThemeBackgroundContentRect = (HRESULT (WINAPI *)(HANDLE, HDC, int, int,  const RECT *, RECT *))GetProcAddress(mTheme, "GetThemeBackgroundContentRect");
	MyDrawThemeParentBackground = (HRESULT (WINAPI *)(HWND, HDC, RECT*))GetProcAddress(mTheme, "DrawThemeParentBackground");
	MyIsThemeBackgroundPartiallyTransparent = (BOOL (WINAPI *)(HANDLE, int, int))GetProcAddress(mTheme, "IsThemeBackgroundPartiallyTransparent");
	if (!MyOpenThemeData||
			!MyCloseThemeData||
			!MyIsThemeActive||
			!MyDrawThemeBackground||
			!MyGetThemeBackgroundContentRect||
			!MyDrawThemeParentBackground||
			!MyIsThemeBackgroundPartiallyTransparent) {
		FreeLibrary(mTheme);
		mTheme=NULL;
	}
}

void RichUtil_Unload() {
	li.List_Destroy(&sListInt);
	DeleteCriticalSection(&csRich);
	if (mTheme) {
		FreeLibrary(mTheme);
	}
}

int RichUtil_SubClass(HWND hwndEdit) {
	if (IsWindow(hwndEdit)) {
		int idx;

		TRichUtil *ru = (TRichUtil*)mir_alloc(sizeof(TRichUtil));

		ZeroMemory(ru, sizeof(TRichUtil));
		ru->hwnd = hwndEdit;
		ru->hasUglyBorder = 0;
		EnterCriticalSection(&csRich);
		if (!li.List_GetIndex(&sListInt, ru, &idx))
			li.List_Insert(&sListInt, ru, idx);
		LeaveCriticalSection(&csRich);
		SetWindowLong(ru->hwnd, GWL_USERDATA, (LONG)ru); // Ugly hack
		ru->origProc = (WNDPROC)SetWindowLong(ru->hwnd, GWL_WNDPROC, (LONG)&RichUtil_Proc);
		RichUtil_ClearUglyBorder(ru);
		return 1;
	}
	return 0;
}

static LRESULT CALLBACK RichUtil_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	TRichUtil *ru = 0, tru;
	int idx;

	EnterCriticalSection(&csRich);
	tru.hwnd = hwnd;
	if (li.List_GetIndex(&sListInt, &tru, &idx))
		ru = (TRichUtil*)sListInt.items[idx];
	LeaveCriticalSection(&csRich);
	switch(msg) {
		case WM_THEMECHANGED:
		case WM_STYLECHANGED:
		{
			RichUtil_ClearUglyBorder(ru);
			break;
		}
		case WM_NCPAINT:
		{
			LRESULT ret = CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);
			if (ru->hasUglyBorder&&MyIsThemeActive()) {
				HANDLE hTheme = MyOpenThemeData(ru->hwnd, L"EDIT");

				if (hTheme) {
					RECT rcBorder;
					RECT rcClient;
					int nState;
					HDC hdc = GetWindowDC(ru->hwnd);

					GetWindowRect(hwnd, &rcBorder);
					rcBorder.right -= rcBorder.left; rcBorder.bottom -= rcBorder.top;
					rcBorder.left = rcBorder.top = 0;
					CopyRect(&rcClient, &rcBorder);
					rcClient.left += ru->rect.left;
					rcClient.top += ru->rect.top;
					rcClient.right -= ru->rect.right;
					rcClient.bottom -= ru->rect.bottom;
					ExcludeClipRect(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
					if(MyIsThemeBackgroundPartiallyTransparent(hTheme, EP_EDITTEXT, ETS_NORMAL))
						MyDrawThemeParentBackground(hwnd, hdc, &rcBorder);
					if(!IsWindowEnabled(hwnd))
						nState = ETS_DISABLED;
					else if(SendMessage(hwnd, EM_GETOPTIONS, 0, 0) & ECO_READONLY)
						nState = ETS_READONLY;
					else nState = ETS_NORMAL;
					MyDrawThemeBackground(hTheme, hdc, EP_EDITTEXT, nState, &rcBorder, NULL);
					MyCloseThemeData(hTheme);
					ReleaseDC(hwnd, hdc);
					return 0;
				}
			}
			return ret;
		}
		case WM_NCCALCSIZE:
		{
			LRESULT ret = CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);
			NCCALCSIZE_PARAMS *ncsParam = (NCCALCSIZE_PARAMS*)lParam;

			if (ru->hasUglyBorder&&MyIsThemeActive()) {
				HANDLE hTheme = MyOpenThemeData(hwnd, L"EDIT");

				if (hTheme) {
					RECT rcClient;
					HDC hdc = GetDC(GetParent(hwnd));

					ZeroMemory(&rcClient, sizeof(RECT));
					if(MyGetThemeBackgroundContentRect(hTheme, hdc, EP_EDITTEXT, ETS_NORMAL, &ncsParam->rgrc[0], &rcClient) == S_OK) {
						ru->rect.left = rcClient.left-ncsParam->rgrc[0].left;
						ru->rect.top = rcClient.top-ncsParam->rgrc[0].top;
						ru->rect.right = ncsParam->rgrc[0].right-rcClient.right;
						ru->rect.bottom = ncsParam->rgrc[0].bottom-rcClient.bottom;
						CopyRect(&ncsParam->rgrc[0], &rcClient);
						MyCloseThemeData(hTheme);
						ReleaseDC(GetParent(hwnd), hdc);
						return WVR_REDRAW;
					}
					ReleaseDC(GetParent(hwnd), hdc);
					MyCloseThemeData(hTheme);
				}
			}
			return ret;
		}
		case WM_ENABLE:
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_UPDATENOW|RDW_FRAME);
			break;
		case WM_DESTROY:
		{
			LRESULT ret = CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);

			if(IsWindow(hwnd)) {
				if((WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC) == &RichUtil_Proc)
					SetWindowLong(hwnd, GWL_WNDPROC, (LONG)ru->origProc);
			}
			EnterCriticalSection(&csRich);
			li.List_Remove(&sListInt, idx);
			LeaveCriticalSection(&csRich);
			if (ru) mir_free(ru);
			return ret;
		}
	}
	return CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);
}

static void RichUtil_ClearUglyBorder(TRichUtil *ru) {
	if (mTheme&&MyIsThemeActive()&&GetWindowLong(ru->hwnd, GWL_EXSTYLE)&WS_EX_CLIENTEDGE) {
		ru->hasUglyBorder = 1;
		SetWindowLong(ru->hwnd, GWL_EXSTYLE, GetWindowLong(ru->hwnd, GWL_EXSTYLE)^WS_EX_CLIENTEDGE);
	}
	// Redraw window since the style may have changed
	SetWindowPos(ru->hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
	RedrawWindow(ru->hwnd, NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_UPDATENOW|RDW_FRAME);
}
