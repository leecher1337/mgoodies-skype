/*
Scriver

Copyright 2000-2003 Miranda ICQ/IM project, 
Copyright 2005 Piotr Piastucki

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

extern HINSTANCE g_hInst;
PSLWA pSetLayeredWindowAttributes;

#define SB_CHAR_WIDTH		 40
#define SB_SENDING_WIDTH 	 25
#define SB_TYPING_WIDTH 	 35

#define TIMERID_FLASHWND     1
#define TIMEOUT_FLASHWND     900

static WNDPROC OldTabCtrlProc;

BOOL CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static TCHAR* GetTabName(HANDLE *hContact)
{
	char *contactName;
	int len;
	TCHAR *result;
	contactName = Translate("Unknown");
	if (hContact) {
		contactName = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, 0);
	}
	len = strlen(contactName) + 1;
	result = (TCHAR *)malloc(len * sizeof(TCHAR));
#if defined ( _UNICODE )
	{
		MultiByteToWideChar(CP_ACP, 0, contactName, -1, result, len);
	}
#else
	memcpy(result, contactName, len);
#endif
	if (g_dat->flags & SMF_LIMITNAMES) {
		if (len > 20 ) {
			result[20] = '\0';
		}
	}
	return result;
}

static void GetChildWindowRect(struct ParentWindowData *dat, RECT *rcChild)
{
	RECT rc, rcStatus, rcTabs;
	GetClientRect(dat->hwnd, &rc);
	GetClientRect(dat->hwndTabs, &rcTabs);
	TabCtrl_AdjustRect(dat->hwndTabs, FALSE, &rcTabs);
	rcStatus.top = rcStatus.bottom = 0;
	if (dat->flags & SMF_SHOWSTATUSBAR) {
		GetWindowRect(dat->hwndStatus, &rcStatus);
	}
	rcChild->left = 0;
	rcChild->right = rc.right;
	if (dat->flags & SMF_TABSATBOTTOM) {
		rcChild->top = 2;
		if (dat->childrenCount > 1 || !(dat->flags & SMF_HIDEONETAB)) {
			rcChild->bottom = rcTabs.bottom + 4;
		} else {
			rcChild->bottom = rc.bottom - rc.top - (rcStatus.bottom - rcStatus.top);
		}
	} else {
		if (dat->childrenCount > 1 || !(dat->flags & SMF_HIDEONETAB)) {
			rcChild->top = rcTabs.top;
		} else {
			rcChild->top = 2;//rcTabs.top - 2;
		}
		rcChild->bottom = rc.bottom - rc.top - (rcStatus.bottom - rcStatus.top);
	}
}

static void GetMinimunWindowSize(struct ParentWindowData *dat, SIZE *size)
{
	MINMAXINFO mmi;
	RECT rc, rcWindow;
	int i, minW = 240, minH = 80;
	GetWindowRect(dat->hwnd, &rcWindow);
	GetChildWindowRect(dat, &rc);
	for (i=0;i<dat->childrenCount;i++) {
		SendMessage(dat->children[i], WM_GETMINMAXINFO, 0, (LPARAM) &mmi);
		if (i==0 || mmi.ptMinTrackSize.x > minW) minW = mmi.ptMinTrackSize.x;
		if (i==0 || mmi.ptMinTrackSize.y > minH) minH = mmi.ptMinTrackSize.y;
	}
	if (dat->bMinimized) {
		size->cx = minW;
		size->cy = minH;
	} else {
		size->cx = minW + (rcWindow.right - rcWindow.left) - (rc.right - rc.left);
		size->cy = minH + (rcWindow.bottom - rcWindow.top) - (rc.bottom - rc.top);
	}
}


static int GetChildTab(struct ParentWindowData *dat, HWND child) 
{
	struct MessageWindowData * mdat;
	TCITEM tci;
	int l, i;
	l = TabCtrl_GetItemCount(dat->hwndTabs);
	for (i = 0; i < l; i++) {
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(dat->hwndTabs, i, &tci);
		mdat = (struct MessageWindowData *) tci.lParam; 
		if (mdat->hwnd == child) {
			return i;
		}
	}
	return -1;

}

static void ActivateChild(struct ParentWindowData *dat, HWND child) {
	int i;
	RECT rcChild;

	GetChildWindowRect(dat, &rcChild);
	SetWindowPos(child, HWND_TOP, rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, SWP_NOSIZE);
	if(child != dat->hwndActive) {
		HWND prev = dat->hwndActive;
		dat->hwndActive = child;
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
		ShowWindow(dat->hwndActive, SW_SHOW);
		SendMessage(dat->hwndActive, DM_UPDATESTATUSBAR, 0, 0);
		SendMessage(dat->hwndActive, DM_UPDATETITLE, 0, 0);
		ShowWindow(prev, SW_HIDE);
	} else {
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
	}
	i = GetChildTab(dat, child);
	TabCtrl_SetCurSel(dat->hwndTabs, i);
	SendMessage(dat->hwndActive, WM_ACTIVATE, WA_ACTIVE, 0);
	SetFocus(dat->hwndActive);
}

static struct MessageWindowData * GetChildFromTab(struct ParentWindowData *dat, int tabId) 
{
	TCITEM tci;
	tci.mask = TCIF_PARAM;
	TabCtrl_GetItem(dat->hwndTabs, tabId, &tci);
	return (struct MessageWindowData *) tci.lParam; 
}

static struct MessageWindowData * GetChildFromHWND(struct ParentWindowData *dat, HWND hwnd) 
{
	struct MessageWindowData * mdat;
	TCITEM tci;
	int l, i;
	l = TabCtrl_GetItemCount(dat->hwndTabs);
	for (i = 0; i < l; i++) {
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(dat->hwndTabs, i, &tci);
		mdat = (struct MessageWindowData *) tci.lParam; 
		if (mdat->hwnd == hwnd) {
			return mdat;
		}
	}
	return NULL;
}

static void AddChild(struct ParentWindowData *dat, struct MessageWindowData * mdat) 
{
	TCHAR *contactName;
	TCITEM tci;
	int tabId;

	dat->children=(HWND*)realloc(dat->children, sizeof(HWND)*(dat->childrenCount+1));
	dat->children[dat->childrenCount++] = mdat->hwnd;
	contactName = GetTabName(mdat->hContact);
	tci.mask = TCIF_TEXT | TCIF_PARAM;
	tci.pszText = contactName;
	tci.lParam = (LPARAM) mdat;
	tabId = TabCtrl_InsertItem(dat->hwndTabs, dat->childrenCount-1, &tci);
	free(contactName);
	ActivateChild(dat, mdat->hwnd);
}

static void RemoveChild(struct ParentWindowData *dat, HWND child) 
{
	int i;
	for (i=0;i<dat->childrenCount;i++) {
		if (dat->children[i] == child) {
			MoveMemory(&dat->children[i], &dat->children[i+1], sizeof(HWND)*(dat->childrenCount-i-1));
			dat->childrenCount--;
			break;
		}
	}
	i = GetChildTab(dat, child);
	if (i >= 0) {
		TabCtrl_DeleteItem(dat->hwndTabs, i);
	}
	if (dat->childrenCount > 0) {
		if (i==dat->childrenCount) i--;
		ActivateChild(dat, dat->children[i]);
	}
}

static void ActivateNextChild(struct ParentWindowData *dat, HWND child) 
{
	int i;
	for (i=0;i<dat->childrenCount;i++) {
		if (dat->children[i] == child) {
			ActivateChild(dat, dat->children[(i+1)%dat->childrenCount]);
			break;
		}
	}
}

static void ActivatePrevChild(struct ParentWindowData *dat, HWND child) 
{
	int i;
	for (i=0;i<dat->childrenCount;i++) {
		if (dat->children[i] == child) {
			ActivateChild(dat, dat->children[(dat->childrenCount+i-1)%dat->childrenCount]);
			break;
		}
	}
}


BOOL CALLBACK DlgProcParentWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	DWORD ws;
	struct ParentWindowData *dat;
	dat = (struct ParentWindowData *) GetWindowLong(hwndDlg, GWL_USERDATA);
	if (!dat && msg!=WM_INITDIALOG) return FALSE;
	switch (msg) {
	case WM_INITDIALOG:
		{
			struct NewMessageWindowLParam *newData = (struct NewMessageWindowLParam *) lParam;
			dat = (struct ParentWindowData *) malloc(sizeof(struct ParentWindowData));
			dat->hContact = newData->hContact;
			dat->nFlash = 0;
			dat->nFlashMax = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT);
			dat->childrenCount = 0;
			dat->children = NULL;
			dat->hwnd = hwndDlg;
			dat->flags = g_dat->flags;// | SMF_SHOWTITLEBAR;
			dat->mouseLBDown = 0;
			dat->windowWasCascaded = 0;
			dat->hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hwndDlg, NULL, g_hInst, NULL);
			{
				int statwidths[4];
				RECT rc;
				SendMessage(dat->hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);
				GetWindowRect(dat->hwndStatus, &rc);
				/*
				statwidths[0] = rc.right - rc.left - SB_CHAR_WIDTH - SB_TYPING_WIDTH - SB_SENDING_WIDTH;
				statwidths[1] = rc.right - rc.left - SB_TYPING_WIDTH - SB_SENDING_WIDTH; //rc.right - rc.left - SB_CHAR_WIDTH;
				statwidths[2] = rc.right - rc.left - SB_TYPING_WIDTH; //rc.right - rc.left - SB_CHAR_WIDTH;
				statwidths[3] = -1;
				SendMessage(dat->hwndStatus, SB_SETPARTS, 4, (LPARAM) statwidths);
				*/
				statwidths[0] = rc.right - rc.left - SB_CHAR_WIDTH - SB_TYPING_WIDTH;
				statwidths[1] = rc.right - rc.left - SB_TYPING_WIDTH;
				statwidths[2] = -1;
				SendMessage(dat->hwndStatus, SB_SETPARTS, 3, (LPARAM) statwidths);
			}
			dat->hwndTabs = GetDlgItem(hwndDlg, IDC_TABS);
			dat->hwndActive = NULL;
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) dat);
			if (g_dat->hIconList != NULL) {
				TabCtrl_SetImageList(dat->hwndTabs, g_dat->hIconList);
			}
			WindowList_Add(g_dat->hParentWindowList, hwndDlg, 0);
			OldTabCtrlProc = (WNDPROC) SetWindowLong(dat->hwndTabs, GWL_WNDPROC, (LONG) TabCtrlProc);
			ws = GetWindowLong(dat->hwndTabs, GWL_STYLE) & ~(TCS_BOTTOM);
			if (dat->flags & SMF_TABSATBOTTOM) {
				ws |= TCS_BOTTOM;
			}
			SetWindowLong(dat->hwndTabs, GWL_STYLE, ws);
			ws = GetWindowLong(hwndDlg, GWL_STYLE) & ~(WS_CAPTION);
			if (dat->flags & SMF_SHOWTITLEBAR) {
				ws |= WS_CAPTION;
				SetWindowLong(hwndDlg, GWL_STYLE, ws);
			} else {
				RECT rc;
				SetWindowLong(hwndDlg, GWL_STYLE, ws);
				GetWindowRect(hwndDlg, &rc);
				SetWindowPos(hwndDlg, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
							 SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER  | SWP_FRAMECHANGED | SWP_NOSENDCHANGING); 
			}
			ws = GetWindowLong(hwndDlg, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			ws |= dat->flags & SMF_USETRANSPARENCY ? WS_EX_LAYERED : 0;
			SetWindowLong(hwndDlg , GWL_EXSTYLE , ws);
			if (dat->flags & SMF_USETRANSPARENCY) {
   				pSetLayeredWindowAttributes(hwndDlg, RGB(255,255,255), (BYTE)(255-g_dat->inactiveAlpha), LWA_ALPHA);
//				RedrawWindow(hwndDlg, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
			}

			//SetWindowPos(dat->hwndTabs, 0, 0, -10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			if (!(dat->flags & SMF_SHOWSTATUSBAR)) {
				ShowWindow(dat->hwndStatus, SW_HIDE);
			}
			if (dat->flags & SMF_USETABS) {
				if (ScriverRestoreWindowPosition(hwndDlg, NULL, SRMMMOD, "", 0, SW_HIDE)) {
					SetWindowPos(hwndDlg, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE  | SWP_HIDEWINDOW);
				}
			} else {
				int savePerContact = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT);
				if (ScriverRestoreWindowPosition(hwndDlg, savePerContact ? dat->hContact : NULL, SRMMMOD, "", 0, SW_HIDE)) {
				//if (Utils_RestoreWindowPosition(GetParent(hwndDlg), savePerContact ? dat->hContact : NULL, SRMMMOD, "")) {
					if (savePerContact) {
						if (ScriverRestoreWindowPosition(hwndDlg, NULL, SRMMMOD, "", RWPF_NOSIZE, SW_HIDE))
					//	if (Utils_RestoreWindowPositionNoMove(GetParent(hwndDlg), NULL, SRMMMOD, ""))
						SetWindowPos(GetParent(hwndDlg), 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE);
					}
					else
						SetWindowPos(hwndDlg, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE);
				}
				if (!savePerContact && DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_CASCADE, SRMSGDEFSET_CASCADE))
					WindowList_Broadcast(g_dat->hParentWindowList, DM_CASCADENEWWINDOW, (WPARAM) hwndDlg, (LPARAM) & dat->windowWasCascaded);
			}
		}
		return TRUE;
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO *mmi = (MINMAXINFO *) lParam;
		SIZE size;
		GetMinimunWindowSize(dat, &size);
		mmi->ptMinTrackSize.x = size.cx;
		mmi->ptMinTrackSize.y = size.cy;
		return FALSE;
	}

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) {
			dat->bMinimized = 1;

		} else if (!IsIconic(hwndDlg))	{
			int i;
			RECT rc, rcStatus, rcChild, rcWindow;
			SIZE size;
			dat->bMinimized = 0;
			GetClientRect(hwndDlg, &rc);
			rcStatus.top = rcStatus.bottom = 0;
			if (dat->flags & SMF_SHOWSTATUSBAR) {
				int statwidths[4];
				GetWindowRect(dat->hwndStatus, &rcStatus);
				statwidths[0] = rc.right - rc.left - SB_CHAR_WIDTH - SB_TYPING_WIDTH;
				statwidths[1] = rc.right - rc.left - SB_TYPING_WIDTH;
				statwidths[2] = -1;
				SendMessage(dat->hwndStatus, SB_SETPARTS, 3, (LPARAM) statwidths);
				SendMessage(dat->hwndStatus, WM_SIZE, 0, 0);
			}
			MoveWindow(dat->hwndTabs, 0, 2, (rc.right - rc.left), (rc.bottom - rc.top) - (rcStatus.bottom - rcStatus.top) - 2,	FALSE); 
			RedrawWindow(dat->hwndTabs, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_ERASE);
			GetMinimunWindowSize(dat, &size);
			GetWindowRect(hwndDlg, &rcWindow);
			if ((rcWindow.bottom-rcWindow.top) < size.cy || (rcWindow.right-rcWindow.left) < size.cx) {
				if ((rcWindow.bottom-rcWindow.top) < size.cy) {
					rcWindow.bottom = rcWindow.top + size.cy;
				} 
				if ((rcWindow.right-rcWindow.left) < size.cx) {
					rcWindow.right = rcWindow.left + size.cx;
				}
				MoveWindow(hwndDlg, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top,	TRUE); 
			}
			GetChildWindowRect(dat, &rcChild);
			for (i=0;i<dat->childrenCount;i++) {
				if (dat->children[i] == dat->hwndActive) {
					MoveWindow(dat->children[i], rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, TRUE);
					RedrawWindow(GetDlgItem(dat->children[i], IDC_LOG), NULL, NULL, RDW_INVALIDATE);
				} 
			}
			if (dat->flags & SMF_SHOWSTATUSBAR) {
				RedrawWindow(dat->hwndStatus, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		return FALSE;
	case WM_SETFOCUS:
		SetFocus(dat->hwndActive);
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			//DestroyWindow(hwndDlg);
			return TRUE;
		}
	case WM_NOTIFY: 
		{
			NMHDR* pNMHDR = (NMHDR*) lParam;
			if (pNMHDR->hwndFrom == dat->hwndTabs) {
				switch (pNMHDR->code) {
				case TCN_SELCHANGE:
					{
						TCITEM tci = {0};
						int iSel = TabCtrl_GetCurSel(dat->hwndTabs);
						tci.mask = TCIF_PARAM;
						if (TabCtrl_GetItem(dat->hwndTabs, iSel, &tci)) {
							struct MessageWindowData * mdat = (struct MessageWindowData *) tci.lParam;
							ActivateChild(dat, mdat->hwnd);
						}
					}
					break;
				case NM_CLICK: 
					{
						FILETIME ft;
						TCHITTESTINFO thinfo;
						int tabId;
						GetSystemTimeAsFileTime(&ft);
						GetCursorPos(&thinfo.pt);
						ScreenToClient(dat->hwndTabs, &thinfo.pt);
						tabId = TabCtrl_HitTest(dat->hwndTabs, &thinfo);
						if (tabId != -1 && tabId == dat->lastClickTab && 
							(ft.dwLowDateTime - dat->lastClickTime) < (GetDoubleClickTime() * 10000)) {
							SendMessage(GetChildFromTab(dat, tabId)->hwnd, WM_CLOSE, 0, 0);
							dat->lastClickTab = -1;
						} else {
							dat->lastClickTab = tabId;
						}
						dat->lastClickTime = ft.dwLowDateTime;
					}
					break;
				case NM_RCLICK:
					{
						TCHITTESTINFO thinfo;
						int tabId, x, y;
						GetCursorPos(&thinfo.pt);
						x = thinfo.pt.x; 
						y = thinfo.pt.y;
						ScreenToClient(dat->hwndTabs, &thinfo.pt);
						tabId = TabCtrl_HitTest(dat->hwndTabs, &thinfo);
						if (tabId != -1) {
							struct MessageWindowData * mwd = GetChildFromTab(dat, tabId);
							//CallService(MS_USERINFO_SHOWDIALOG, (WPARAM) mwd->hContact, 0);
							HMENU hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM) mwd->hContact, 0);
							TrackPopupMenu(hMenu, 0, x, y, 0, mwd->hwnd, NULL);
							DestroyMenu(hMenu);
						}
					}
					break;
				}
			} else if (pNMHDR->hwndFrom == dat->hwndStatus)  {
				switch (pNMHDR->code) {
				case NM_CLICK:
					{
						NMMOUSE *nm=(NMMOUSE*)lParam;
						RECT rc;
						SendMessage(dat->hwndStatus, SB_GETRECT, SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 1, (LPARAM)&rc);
						if (nm->pt.x >= rc.left) 
							SendMessage(dat->hwndActive, DM_SWITCHUNICODE, 0, 0);
					}
				}
				break;
			}
		}
		break;
	case WM_TIMER:
		if (wParam == TIMERID_FLASHWND) {
			if ((dat->nFlash > dat->nFlashMax) || (GetActiveWindow() == hwndDlg) || (GetForegroundWindow() == hwndDlg)) {
				KillTimer(hwndDlg, TIMERID_FLASHWND);
				FlashWindow(hwndDlg, FALSE);
			} else if (dat->nFlash < dat->nFlashMax) {
				FlashWindow(hwndDlg, TRUE);
				dat->nFlash++;
			}
		}
		break;
	case WM_CONTEXTMENU:
	{
		if (dat->hwndStatus && dat->hwndStatus == (HWND) wParam) {
			RECT rc;
			POINT pt, pt2;
			GetCursorPos(&pt);
			pt2.x = pt.x;
			pt2.y = pt.y;
			ScreenToClient(dat->hwndStatus, &pt);
			SendMessage(dat->hwndStatus, SB_GETRECT, SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 1, (LPARAM)&rc);
			if (pt.x >= rc.left && dat->hwndActive != NULL) {
				int codePage = (int) SendMessage(dat->hwndActive, DM_GETCODEPAGE, 0, 0);
				int i, iSel;
				for(i = 0; i < GetMenuItemCount(g_dat->hMenuANSIEncoding); i++) {
					CheckMenuItem (g_dat->hMenuANSIEncoding, i, MF_BYPOSITION | MF_UNCHECKED);
				}
				if(codePage == CP_ACP) {
					CheckMenuItem(g_dat->hMenuANSIEncoding, 0, MF_BYPOSITION | MF_CHECKED);
				} else {
					CheckMenuItem(g_dat->hMenuANSIEncoding, codePage, MF_BYCOMMAND | MF_CHECKED);
				}
				iSel = TrackPopupMenu(g_dat->hMenuANSIEncoding, TPM_RETURNCMD, pt2.x, pt2.y, 0, hwndDlg, NULL);
				if (iSel >= 500) {
					if (iSel == 500) iSel = CP_ACP;
					SendMessage(dat->hwndActive, DM_SETCODEPAGE, 0, iSel);
				}
			}
			else 
				SendMessage(dat->hwndActive, WM_CONTEXTMENU, (WPARAM)hwndDlg, 0);
		}
		break;
	}
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			ws = GetWindowLong(hwndDlg, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			ws |= dat->flags & SMF_USETRANSPARENCY ? WS_EX_LAYERED : 0;
			SetWindowLong(hwndDlg , GWL_EXSTYLE , ws);
			if (dat->flags & SMF_USETRANSPARENCY) {
   				pSetLayeredWindowAttributes(hwndDlg, RGB(255,255,255), (BYTE)(255-g_dat->inactiveAlpha), LWA_ALPHA);
//				RedrawWindow(hwndDlg, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
			}
		}
		if (LOWORD(wParam) != WA_ACTIVE)
			break;
	case WM_MOUSEACTIVATE:
		if (KillTimer(hwndDlg, TIMERID_FLASHWND)) {
			FlashWindow(hwndDlg, FALSE);
			dat->nFlash = 0;
		}
		ws = GetWindowLong(hwndDlg, GWL_EXSTYLE) & ~WS_EX_LAYERED;
		ws |= dat->flags & SMF_USETRANSPARENCY ? WS_EX_LAYERED : 0;
		SetWindowLong(hwndDlg , GWL_EXSTYLE , ws);
		if (dat->flags & SMF_USETRANSPARENCY) {
   			pSetLayeredWindowAttributes(hwndDlg, RGB(255,255,255), (BYTE)(255-g_dat->activeAlpha), LWA_ALPHA);
//				RedrawWindow(hwndDlg, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
		}
		SendMessage(dat->hwndActive, WM_ACTIVATE, WA_ACTIVE, 0);
		break;
	case WM_LBUTTONDOWN:
		dat->mouseLBDown = 1;
		GetCursorPos(&dat->mouseLBDownPos);
		SetCapture(hwndDlg);
		break;
	case WM_LBUTTONUP:
		dat->mouseLBDown = 0;
		ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		if (dat->mouseLBDown) { 
			POINT pt;
			RECT  rc;
			GetCursorPos(&pt);
			GetWindowRect(hwndDlg, &rc);
			SetWindowPos(hwndDlg, 0, rc.left - (dat->mouseLBDownPos.x - pt.x), rc.top - (dat->mouseLBDownPos.y - pt.y), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			dat->mouseLBDownPos = pt;
		}
		break;
	case WM_DESTROY:
		{
			g_dat->hParent = NULL;
			SetWindowLong(hwndDlg, GWL_USERDATA, 0);
			WindowList_Remove(g_dat->hParentWindowList, hwndDlg);
			if (dat->children!=NULL) free (dat->children);
			free(dat);
			if (dat->flags & SMF_USETABS) {
				WINDOWPLACEMENT wp = { 0 };
				wp.length = sizeof(wp);
				GetWindowPlacement(hwndDlg, &wp);
				DBWriteContactSettingDword(NULL, SRMMMOD, "x", wp.rcNormalPosition.left);
				DBWriteContactSettingDword(NULL, SRMMMOD, "y", wp.rcNormalPosition.top);
				DBWriteContactSettingDword(NULL, SRMMMOD, "width", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
				DBWriteContactSettingDword(NULL, SRMMMOD, "height", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
			} else {
				WINDOWPLACEMENT wp = { 0 };
				HANDLE hContact;
				if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT))
					hContact = dat->hContact;
				else
					hContact = NULL;
				wp.length = sizeof(wp);
				GetWindowPlacement(hwndDlg, &wp);
				if (!dat->windowWasCascaded) {
					DBWriteContactSettingDword(hContact, SRMMMOD, "x", wp.rcNormalPosition.left);
					DBWriteContactSettingDword(hContact, SRMMMOD, "y", wp.rcNormalPosition.top);
				}
				DBWriteContactSettingDword(hContact, SRMMMOD, "width", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
				DBWriteContactSettingDword(hContact, SRMMMOD, "height", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
			}

		}
		break;
	case DM_ERRORDECIDED:
		break;
	case DM_STARTFLASHING:
		if (GetActiveWindow() != hwndDlg && GetForegroundWindow() != hwndDlg) {
			dat->nFlash = 0;
			SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
		}
		break;
	case DM_REMOVECHILD:
		{
			RemoveChild(dat, (HWND) lParam);
			if (dat->childrenCount == 0) {
				DestroyWindow(hwndDlg);
			} else {
			}
		}
		return TRUE;
	case DM_ADDCHILD:
		{
			struct MessageWindowData * mdat = (struct MessageWindowData *) lParam;
			AddChild(dat, mdat);
		}
		return TRUE;
	case DM_ACTIVATECHILD:
		if((HWND) lParam != dat->hwndActive) {
			ActivateChild(dat, (HWND) lParam);
		}
		return TRUE;
	case DM_ACTIVATEPREV:
		ActivatePrevChild(dat, (HWND) lParam);
		return TRUE;
	case DM_ACTIVATENEXT:
		ActivateNextChild(dat, (HWND) lParam);
		return TRUE;
	case DM_SENDMESSAGE:
		{
			int i;
			for (i=0;i<dat->childrenCount;i++) {
				SendMessage(dat->children[i], DM_SENDMESSAGE, wParam, lParam);
			}
		}
		break;
	case DM_OPTIONSAPPLIED:
		{
			RECT rc;
			dat->flags = g_dat->flags;
			if (!(dat->flags & SMF_SHOWSTATUSBAR)) {
				ShowWindow(dat->hwndStatus, SW_HIDE);
			} else {
				ShowWindow(dat->hwndStatus, SW_SHOW);
			}
			ws = GetWindowLong(hwndDlg, GWL_STYLE) & ~(WS_CAPTION);
			if (dat->flags & SMF_SHOWTITLEBAR) {
				ws |= WS_CAPTION;
			} 
			SetWindowLong(hwndDlg, GWL_STYLE, ws);

			ws = GetWindowLong(hwndDlg, GWL_EXSTYLE)& ~WS_EX_LAYERED;
			ws |= dat->flags & SMF_USETRANSPARENCY ? WS_EX_LAYERED : 0;
			SetWindowLong(hwndDlg , GWL_EXSTYLE , ws);
			if (dat->flags & SMF_USETRANSPARENCY) {
   				pSetLayeredWindowAttributes(hwndDlg, RGB(255,255,255), (BYTE)(255-g_dat->inactiveAlpha), LWA_ALPHA);
//				RedrawWindow(hwndDlg, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
			}

			ws = GetWindowLong(dat->hwndTabs, GWL_STYLE) & ~(TCS_BOTTOM);
			if (dat->flags & SMF_TABSATBOTTOM) {
				ws |= TCS_BOTTOM;
			} 
			SetWindowLong(dat->hwndTabs, GWL_STYLE, ws);
			RedrawWindow(dat->hwndTabs, NULL, NULL, RDW_INVALIDATE);
			GetWindowRect(hwndDlg, &rc);
			SetWindowPos(hwndDlg, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top, 
                        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			//RedrawWindow(hwndDlg, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
			break;
		}
	case DM_UPDATETITLE:
		{
			struct MessageWindowData * mdat = (struct MessageWindowData *) lParam;
			TCITEM tci;
			int tabId;
			char newtitle[256], oldtitle[256];
			char *szStatus, *contactName, *pszNewTitleEnd;
			TCHAR *tContactName;
			if (mdat && mdat->hwnd == dat->hwndActive) {
				pszNewTitleEnd = "Message Session";
				if (mdat->hContact) {
					if (mdat->szProto) {
						CONTACTINFO ci;
						char buf[128];
						int statusIcon = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON);

						buf[0] = 0;
						mdat->wStatus = DBGetContactSettingWord(mdat->hContact, mdat->szProto, "Status", ID_STATUS_OFFLINE);
						contactName = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) mdat->hContact, 0);
						ZeroMemory(&ci, sizeof(ci));
						ci.cbSize = sizeof(ci);
						ci.hContact = mdat->hContact;
						ci.szProto = mdat->szProto;
						ci.dwFlag = CNF_UNIQUEID;
						if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
							switch (ci.type) {
							case CNFT_ASCIIZ:
								_snprintf(buf, sizeof(buf), "%s", ci.pszVal);
								miranda_sys_free(ci.pszVal);
								break;
							case CNFT_DWORD:
								_snprintf(buf, sizeof(buf), "%u", ci.dVal);
								break;
							}
						}
						szStatus = (char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, mdat->szProto == NULL ? ID_STATUS_OFFLINE : DBGetContactSettingWord(mdat->hContact, mdat->szProto, "Status", ID_STATUS_OFFLINE), 0);
						if (statusIcon)
							_snprintf(newtitle, sizeof(newtitle), "%s - %s", contactName, Translate(pszNewTitleEnd));
						else
							_snprintf(newtitle, sizeof(newtitle), "%s (%s): %s", contactName, szStatus, Translate(pszNewTitleEnd));

					}
				}
				else
					lstrcpynA(newtitle, pszNewTitleEnd, sizeof(newtitle));
				GetWindowTextA(hwndDlg, oldtitle, sizeof(oldtitle));
				if (lstrcmpA(newtitle, oldtitle)) { //swt() flickers even if the title hasn't actually changed
					SetWindowTextA(hwndDlg, newtitle);
					//SendMessage(hwndDlg, WM_SIZE, 0, 0);
				}
			}
			tabId = GetChildTab(dat, mdat->hwnd);
			tContactName = GetTabName(mdat->hContact);
			tci.mask = TCIF_TEXT;
			tci.pszText = tContactName;
			TabCtrl_SetItem(dat->hwndTabs, tabId, &tci);
			free(tContactName);
			break;
		}
	case DM_UPDATEWINICON:
		{
			struct MessageWindowData * mdat = (struct MessageWindowData *) lParam;
			if (mdat) {
				if (mdat->szProto) {
					int i, icoIdx = 0;
					WORD wStatus;
					wStatus = DBGetContactSettingWord(mdat->hContact, mdat->szProto, "Status", ID_STATUS_OFFLINE);
					if (mdat->hwnd == dat->hwndActive) {
						if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON)) {
							if (mdat->showTyping && (dat->flags&SMF_SHOWTYPINGWIN)) {
								SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) g_dat->hIcons[SMF_ICON_TYPING]);
							} else if (mdat->showUnread && GetActiveWindow() != hwndDlg && GetForegroundWindow() != hwndDlg) {
								SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));	
							} else {
								SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) LoadSkinnedProtoIcon(mdat->szProto, wStatus));
							}
						} else {
							SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
						}
					}
					icoIdx = 0;
					for (i = 0; i < g_dat->protoNum; i++) {
						if (!strcmp(g_dat->protoNames[i], mdat->szProto)) {
							icoIdx = wStatus - ID_STATUS_OFFLINE + (ID_STATUS_OUTTOLUNCH - ID_STATUS_OFFLINE + 1) * (i +1) + 2;
							break;
						}
					}
					if (mdat->hwnd != dat->hwndActive) {
						if (mdat->showTyping) {
							icoIdx = 1;
						} else if (mdat->showUnread & 1) {
							icoIdx = 0;
						}
					}
					i = GetChildTab(dat, mdat->hwnd);
					if (i>=0) {
						TCITEM tci;
						tci.mask = TCIF_IMAGE;
						tci.iImage = icoIdx;
						TabCtrl_SetItem(dat->hwndTabs, i, &tci);
					}
				} 
			} 
			break;
		}
	case DM_UPDATESTATUSBAR:
		break;
	case DM_SWITCHSTATUSBAR:
		dat->flags ^= SMF_SHOWSTATUSBAR;
		if (!(dat->flags & SMF_SHOWSTATUSBAR)) {
			ShowWindow(dat->hwndStatus, SW_HIDE);
		} else {
			ShowWindow(dat->hwndStatus, SW_SHOW);
		}
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;
	case DM_SWITCHTOOLBAR:
		{
			int i;
			dat->flags ^= SMF_SHOWBTNS;
			for (i=0;i<dat->childrenCount;i++) {
				SendMessage(dat->children[i], DM_SWITCHTOOLBAR, 0, 0);
			}
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
		}
		break;
	case DM_SWITCHTITLEBAR:
		{
			RECT rc;
			dat->flags ^= SMF_SHOWTITLEBAR;
			ws = GetWindowLong(hwndDlg, GWL_STYLE) & ~(WS_CAPTION);
			if (dat->flags & SMF_SHOWTITLEBAR) {
				ws |= WS_CAPTION;
			} 
			SetWindowLong(hwndDlg, GWL_STYLE, ws);
			GetWindowRect(hwndDlg, &rc);
			SetWindowPos(hwndDlg, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                         SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER  | SWP_FRAMECHANGED | SWP_NOSENDCHANGING); 
//			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			RedrawWindow(hwndDlg, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
		break;
	case DM_CASCADENEWWINDOW:
		if ((HWND) wParam == hwndDlg)
			break;
		{
			RECT rcThis, rcNew;
			GetWindowRect(hwndDlg, &rcThis);
			GetWindowRect((HWND) wParam, &rcNew);
			if (abs(rcThis.left - rcNew.left) < 3 && abs(rcThis.top - rcNew.top) < 3) {
				int offset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
				SetWindowPos((HWND) wParam, 0, rcNew.left + offset, rcNew.top + offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				*(int *) lParam = 1;
			}
		}
		break;
	//case DM_MESSAGESENDING:
	//	dat->messagesInProgress += wParam ? -1 : 1;
	//	if (dat->messagesInProgress < 0) dat->messagesInProgress = 0;
	//	break;
	}
	return FALSE;
}

BOOL CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) {
        case WM_MBUTTONDOWN: {
			TCITEM tci;
			int tabId;
			struct MessageWindowData *mwd;
			TCHITTESTINFO thinfo;
		    thinfo.pt.x = LOWORD(lParam);
		    thinfo.pt.y = HIWORD(lParam);
			tabId = TabCtrl_HitTest(hwnd, &thinfo);
			if (tabId >= 0) {
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwnd, tabId, &tci);
				mwd = (struct MessageWindowData *) tci.lParam; 
				if (mwd != NULL) {
					SendMessage(mwd->hwnd, WM_CLOSE, 0, 0);
    			}
			}
	        return TRUE;
        }
    }
	return CallWindowProc(OldTabCtrlProc, hwnd, msg, wParam, lParam); 
}


int ScriverRestoreWindowPosition(HWND hwnd,HANDLE hContact,const char *szModule,const char *szNamePrefix, int flags, int showCmd)
{
	WINDOWPLACEMENT wp;
	char szSettingName[64];
	int x,y;

	wp.length=sizeof(wp);
	GetWindowPlacement(hwnd,&wp);
	wsprintfA(szSettingName,"%sx",szNamePrefix);
	x=DBGetContactSettingDword(hContact,szModule,szSettingName,-1);
	wsprintfA(szSettingName,"%sy",szNamePrefix);
	y=(int)DBGetContactSettingDword(hContact,szModule,szSettingName,-1);
	if(x==-1) return 1;
	if(flags&RWPF_NOSIZE) {
		OffsetRect(&wp.rcNormalPosition,x-wp.rcNormalPosition.left,y-wp.rcNormalPosition.top);
	}
	else {
		wp.rcNormalPosition.left=x;
		wp.rcNormalPosition.top=y;
		wsprintfA(szSettingName,"%swidth",szNamePrefix);
		wp.rcNormalPosition.right=wp.rcNormalPosition.left+DBGetContactSettingDword(hContact,szModule,szSettingName,-1);
		wsprintfA(szSettingName,"%sheight",szNamePrefix);
		wp.rcNormalPosition.bottom=wp.rcNormalPosition.top+DBGetContactSettingDword(hContact,szModule,szSettingName,-1);
	}
	wp.flags=0;
	wp.showCmd = showCmd;
	SetWindowPlacement(hwnd,&wp);
	return 0;
}

