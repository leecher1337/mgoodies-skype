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
//#include <multimon.h>

extern HINSTANCE g_hInst;
extern HCURSOR hDragCursor;
PSLWA pSetLayeredWindowAttributes;

#define SB_CHAR_WIDTH		 40
#define SB_SENDING_WIDTH 	 25
#define SB_TYPING_WIDTH 	 35

#define TIMERID_FLASHWND     1
#define TIMEOUT_FLASHWND     900

static WNDPROC OldTabCtrlProc;

BOOL CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern TCHAR *strToWcs(const char *text, int textlen, int cp);
extern TCHAR *GetNickname(HANDLE hContact, const char* szProto);

static TCHAR* GetWindowTitle(HANDLE *hContact, const char *szProto)
{
	DBVARIANT dbv;
	int isTemplate;
	int len, contactNameLen = 0, statusLen = 0, statusMsgLen = 0, protocolLen = 0;
	TCHAR *p, *tmplt, *szContactName = NULL, *szStatus = NULL, *szStatusMsg = NULL, *szProtocol = NULL, *title;
	TCHAR *pszNewTitleEnd = _tcsdup(TranslateT("Message Session"));
	isTemplate = 0;
	if (hContact && szProto) {
		szContactName = GetNickname(hContact, szProto);
		contactNameLen = lstrlen(szContactName);
		szStatus = strToWcs((char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, szProto == NULL ? ID_STATUS_OFFLINE : DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE), 0), -1, CP_ACP);
		statusLen = lstrlen(szStatus);
		if (!DBGetContactSetting(hContact, "CList", "StatusMsg",&dbv)) {
			if (strlen(dbv.pszVal) > 0) {
				int i, j;
       			szStatusMsg = strToWcs(dbv.pszVal, -1, CP_ACP);
				statusMsgLen = lstrlen(szStatusMsg);
				for (i = j = 0; i < statusMsgLen; i++) {
					if (szStatusMsg[i] == '\r') {
						continue;
					} else if (szStatusMsg[i] == '\n') {
						szStatusMsg[j++] = ' ';
					} else {
						szStatusMsg[j++] = szStatusMsg[i];
					}
				}
				szStatusMsg[j] = '\0';
				statusMsgLen = j;
			}
       		DBFreeVariant(&dbv);
		}

		if (!DBGetContactSetting(NULL, SRMMMOD, SRMSGSET_WINDOWTITLE, &dbv)) {
			isTemplate = 1;
			tmplt = strToWcs(dbv.pszVal, -1, CP_ACP);
			DBFreeVariant(&dbv);
		} else {
			int statusIcon = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON);
			if (statusIcon) {
				tmplt = _T("%name% - ");
			} else {
				tmplt = _T("%name% (%status%) : ");
			}
		}
	} else {
		tmplt = _T("");
	}
	for (len = 0, p = tmplt; *p; p++) {
		if (*p == '%') {
			if (!_tcsncmp(p, _T("%name%"), 6)) {
				len += contactNameLen;
				p += 5;
				continue;
			} else if (!_tcsncmp(p, _T("%status%"), 8)) {
				len += statusLen;
				p += 7;
				continue;
			} else if (!_tcsncmp(p, _T("%statusmsg%"), 11)) {
				len += statusMsgLen;
				p += 10;
				continue;
			}
		}
		len++;
	}
	if (!isTemplate) {
		len += lstrlen(pszNewTitleEnd);
	}
	title = (TCHAR *)malloc(sizeof(TCHAR) * (len + 1));
	for (len = 0, p = tmplt; *p; p++) {
		if (*p == '%') {
			if (!_tcsncmp(p, _T("%name%"), 6)) {
				memcpy(title+len, szContactName, sizeof(TCHAR) * contactNameLen);
				len += contactNameLen;
				p += 5;
				continue;
			} else if (!_tcsncmp(p, _T("%status%"), 8)) {
				memcpy(title+len, szStatus, sizeof(TCHAR) * statusLen);
				len += statusLen;
				p += 7;
				continue;
			} else if (!_tcsncmp(p, _T("%statusmsg%"), 11)) {
				memcpy(title+len, szStatusMsg, sizeof(TCHAR) * statusMsgLen);
				len += statusMsgLen;
				p += 10;
				continue;
			}
		}
		title[len++] = *p;
	}
	if (!isTemplate) {
		memcpy(title+len, pszNewTitleEnd, sizeof(TCHAR) * lstrlen(pszNewTitleEnd));
		len += lstrlen(pszNewTitleEnd);
	}
	title[len] = '\0';
	if (isTemplate) {
		free(tmplt);
	}
	free(szContactName);
	free(szStatus);
	free(pszNewTitleEnd);
	return title;
}

static TCHAR* GetTabName(HANDLE *hContact)
{
	int len;
	TCHAR *result = NULL;
	if (hContact) {
		result = GetNickname(hContact, NULL);
		len = lstrlen(result);
		if (g_dat->flags & SMF_LIMITNAMES) {
			if (len > 20 ) {
				result[20] = '\0';
			}
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

static int GetTabFromHWND(struct ParentWindowData *dat, HWND child)
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

static struct MessageWindowData * GetChildFromTab(HWND hwndTabs, int tabId)
{
	TCITEM tci;
	tci.mask = TCIF_PARAM;
	TabCtrl_GetItem(hwndTabs, tabId, &tci);
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

static void ActivateChild(struct ParentWindowData *dat, HWND child) {
	int i;
	RECT rcChild;

	GetChildWindowRect(dat, &rcChild);
	SetWindowPos(child, HWND_TOP, rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, SWP_NOSIZE);
	if(child != dat->hwndActive) {
		HWND prev = dat->hwndActive;
		dat->hwndActive = child;
		SendMessage(dat->hwndActive, DM_UPDATESTATUSBAR, 0, 0);
		SendMessage(dat->hwndActive, DM_UPDATETITLE, 0, 0);
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
		ShowWindow(dat->hwndActive, SW_SHOWNOACTIVATE);
		SendMessage(dat->hwndActive, DM_SCROLLLOGTOBOTTOM, 0, 0);
		if (prev!=NULL) ShowWindow(prev, SW_HIDE);
	} else {
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
	}
	i = GetTabFromHWND(dat, child);
	TabCtrl_SetCurSel(dat->hwndTabs, i);
	SendMessage(dat->hwndActive, DM_ACTIVATE, WA_ACTIVE, 0);
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
//	ActivateChild(dat, mdat->hwnd);
	SetWindowPos(mdat->hwnd, HWND_TOP, dat->childRect.left, dat->childRect.top, dat->childRect.right-dat->childRect.left, dat->childRect.bottom - dat->childRect.top, SWP_HIDEWINDOW);
	SendMessage(dat->hwnd, WM_SIZE, 0, 0);
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
	i = GetTabFromHWND(dat, child);
	if (i >= 0) {
		TabCtrl_DeleteItem(dat->hwndTabs, i);
	}
	if (dat->childrenCount > 0) {
		if (i == TabCtrl_GetItemCount(dat->hwndTabs)) i--;
		if (i >=0 ) {
			ActivateChild(dat, GetChildFromTab(dat->hwndTabs, i)->hwnd);
		}
	}
}

static void ActivateNextChild(struct ParentWindowData *dat, HWND child)
{
	int i = GetTabFromHWND(dat, child);
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	i = (i+1) % l;
	ActivateChild(dat, GetChildFromTab(dat->hwndTabs, i)->hwnd);
}

static void ActivatePrevChild(struct ParentWindowData *dat, HWND child)
{
	int i = GetTabFromHWND(dat, child);
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	i = (i+l-1) % l;
	ActivateChild(dat, GetChildFromTab(dat->hwndTabs, i)->hwnd);
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
			HMENU hMenu;
			HANDLE hSContact;
			int savePerContact = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT);
			struct NewMessageWindowLParam *newData = (struct NewMessageWindowLParam *) lParam;
			dat = (struct ParentWindowData *) malloc(sizeof(struct ParentWindowData));
			dat->foregroundWindow = GetForegroundWindow();
			dat->hContact = newData->hContact;
			dat->nFlash = 0;
			dat->nFlashMax = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT);
			dat->childrenCount = 0;
			dat->children = NULL;
			dat->hwnd = hwndDlg;
			dat->flags = g_dat->flags;// | SMF_SHOWTITLEBAR;
			dat->mouseLBDown = 0;
			dat->windowWasCascaded = 0;
			dat->bMinimized = 0;
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
			dat->tabCtrlDat = (struct TabCtrlData *) malloc(sizeof(struct TabCtrlData));
			dat->tabCtrlDat->bDragging = FALSE;
			SetWindowLong(dat->hwndTabs, GWL_USERDATA, (LONG) dat->tabCtrlDat);
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
			if (!(dat->flags & SMF_SHOWSTATUSBAR)) {
				ShowWindow(dat->hwndStatus, SW_HIDE);
			}
			hSContact = !(dat->flags & SMF_USETABS) && savePerContact ? dat->hContact : NULL;
			dat->bTopmost = DBGetContactSettingByte(hSContact, SRMMMOD, SRMSGSET_TOPMOST, SRMSGDEFSET_TOPMOST);
			if (ScriverRestoreWindowPosition(hwndDlg, hSContact, SRMMMOD, "", 0, SW_HIDE)) {
				if (ScriverRestoreWindowPosition(hwndDlg, hSContact, SRMMMOD, "", RWPF_NOSIZE, SW_HIDE)) {
					SetWindowPos(GetParent(hwndDlg), 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_HIDEWINDOW);
				} else {
					SetWindowPos(hwndDlg, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_HIDEWINDOW);
				}
			}
			if (!(dat->flags & SMF_USETABS)) {
				if (!savePerContact && DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_CASCADE, SRMSGDEFSET_CASCADE))
					WindowList_Broadcast(g_dat->hParentWindowList, DM_CASCADENEWWINDOW, (WPARAM) hwndDlg, (LPARAM) & dat->windowWasCascaded);
			}
			hMenu = GetSystemMenu( hwndDlg, FALSE );
			InsertMenu( hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			if (dat->bTopmost) {
				InsertMenu( hMenu, 0, MF_BYPOSITION | MF_ENABLED | MF_CHECKED | MF_STRING, IDM_TOPMOST, TranslateT("Always On Top"));
                SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			} else {
				InsertMenu( hMenu, 0, MF_BYPOSITION | MF_ENABLED | MF_UNCHECKED | MF_STRING, IDM_TOPMOST, TranslateT("Always On Top"));
			}
		}
		return TRUE;
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO *mmi = (MINMAXINFO *) lParam;
		SIZE size;
		if (GetKeyState(VK_CONTROL) & 0x8000) {
			WINDOWPLACEMENT wp;
			RECT rcDesktop;
			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
			mmi->ptMaxSize.x = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			mmi->ptMaxSize.y = rcDesktop.bottom - rcDesktop.top;
			mmi->ptMaxPosition.x = wp.rcNormalPosition.left;
			if(IsIconic(hwndDlg)) {
				mmi->ptMaxPosition.y = rcDesktop.top;
			} else {
				mmi->ptMaxPosition.y = 0;
			}
		}
		GetMinimunWindowSize(dat, &size);
		mmi->ptMinTrackSize.x = size.cx;
		mmi->ptMinTrackSize.y = size.cy;
		return FALSE;
	}

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) {
			dat->bMinimized = 1;

		}
		if (IsIconic(hwndDlg))	{
			MoveWindow(dat->hwndActive, dat->childRect.left, dat->childRect.top, dat->childRect.right-dat->childRect.left, dat->childRect.bottom - dat->childRect.top, TRUE);
		} else {
//		}
//		if (!IsIconic(hwndDlg)) {
			int i;
			RECT rc, rcStatus, rcChild, rcWindow;
			SIZE size;
			dat->bMinimized = 0;
			GetClientRect(hwndDlg, &rc);
			GetWindowRect(hwndDlg, &rcWindow);
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
			memcpy(&dat->childRect, &rcChild, sizeof(RECT));
			for (i=0;i<dat->childrenCount;i++) {
				if (dat->children[i] == dat->hwndActive) {
					MoveWindow(dat->children[i], rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, TRUE);
					RedrawWindow(GetDlgItem(dat->children[i], IDC_LOG), NULL, NULL, RDW_INVALIDATE);
				}
			}
			if (dat->flags & SMF_SHOWSTATUSBAR) {
				RedrawWindow(dat->hwndStatus, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			}
		}
		return FALSE;
	case WM_SETFOCUS:
		if (dat->hwndActive != NULL) {
			SetFocus(dat->hwndActive);
		}

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
							SetFocus(dat->hwndActive);
						}
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
							struct MessageWindowData * mwd = GetChildFromTab(dat->hwndTabs, tabId);
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
	case WM_DROPFILES:
		SendMessage(dat->hwndActive, WM_DROPFILES, wParam, lParam);
		break;
	case WM_TIMER:
		if (wParam == TIMERID_FLASHWND) {
			if ((dat->nFlash > dat->nFlashMax)) {// || ((GetActiveWindow() == hwndDlg) && (GetForegroundWindow() == hwndDlg))) {
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
		if (dat->hwndActive == NULL) { // do not set foreground window at all (always stay in the background !)
//			SendMessage(hwndDlg, DM_DEACTIVATE, 0, 0);
		} else {
			ActivateChild(dat, dat->hwndActive);
			PostMessage(dat->hwndActive, DM_SETFOCUS, 0, msg);
		}
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
		break;
	case WM_LBUTTONDOWN:
		if (!IsZoomed(hwndDlg)) {
			POINT pt;
			GetCursorPos(&pt);
		//	dat->mouseLBDown = 1;
		//	GetCursorPos(&dat->mouseLBDownPos);
			return SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
		//	SetCapture(hwndDlg);

		}
		break;
	case WM_LBUTTONUP:
		//if (dat->mouseLBDown) {
		//	dat->mouseLBDown = 0;
		//	ReleaseCapture();
		//}
		break;
	case WM_MOUSEMOVE:/*
		if (dat->mouseLBDown) {
			POINT pt;
			RECT  rc;
			GetCursorPos(&pt);
			GetWindowRect(hwndDlg, &rc);
			SetWindowPos(hwndDlg, 0, rc.left - (dat->mouseLBDownPos.x - pt.x), rc.top - (dat->mouseLBDownPos.y - pt.y), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			dat->mouseLBDownPos = pt;
		}*/
		break;
	case WM_MOVING:
		{
			int snapPixels = 10;
			RECT rcDesktop;
			RECT *pRect = (RECT *)lParam;
			POINT pt;
			MONITORINFO mi;
			HMONITOR hMonitor = MonitorFromRect(pRect, MONITOR_DEFAULTTONEAREST);
			SIZE szSize = {pRect->right-pRect->left,pRect->bottom-pRect->top};
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);			
			GetCursorPos(&pt);
//			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
			rcDesktop = mi.rcWork;
			pRect->left = pt.x-dat->mouseLBDownPos.x;
			pRect->top = pt.y-dat->mouseLBDownPos.y;
			pRect->right = pRect->left+szSize.cx;
			pRect->bottom = pRect->top+szSize.cy;
            if (!(GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
				if(pRect->top < rcDesktop.top+snapPixels && pRect->top > rcDesktop.top-snapPixels) {
					pRect->top = rcDesktop.top;
					pRect->bottom = rcDesktop.top + szSize.cy;
				}
				if(pRect->left < rcDesktop.left+snapPixels && pRect->left > rcDesktop.left-snapPixels) {
					pRect->left = rcDesktop.left;
					pRect->right = rcDesktop.left + szSize.cx;
				}
				if(pRect->right < rcDesktop.right+snapPixels && pRect->right > rcDesktop.right-snapPixels) {
					pRect->right = rcDesktop.right;
					pRect->left = rcDesktop.right - szSize.cx;
				}
				if(pRect->bottom < rcDesktop.bottom+snapPixels && pRect->bottom > rcDesktop.bottom-snapPixels) {
					pRect->bottom = rcDesktop.bottom;
					pRect->top = rcDesktop.bottom - szSize.cy;
				}
			}
		}
		break;
	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_MOVE) {
			RECT  rc;
			GetWindowRect(hwndDlg, &rc);
			dat->mouseLBDownPos.x = LOWORD(lParam) - rc.left;
			dat->mouseLBDownPos.y = HIWORD(lParam) - rc.top;
		} else if (wParam == IDM_TOPMOST) {
            HMENU hMenu = GetSystemMenu(hwndDlg, FALSE);
            if (dat->bTopmost)  {
                CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_UNCHECKED);
                SetWindowPos(hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                dat->bTopmost = FALSE;
            }
            else {
                CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_CHECKED);
                SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                dat->bTopmost = TRUE;
            }
        }
		break;
	case WM_DESTROY:
		{
			WINDOWPLACEMENT wp = { 0 };
			HANDLE hContact;
			g_dat->hParent = NULL;
			SetWindowLong(hwndDlg, GWL_USERDATA, 0);
			WindowList_Remove(g_dat->hParentWindowList, hwndDlg);
			if (dat->children!=NULL) free (dat->children);
			free(dat->tabCtrlDat);
			free(dat);
			if (!(dat->flags & SMF_USETABS) && DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT))
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
			DBWriteContactSettingByte(hContact, SRMMMOD, SRMSGSET_TOPMOST, (BYTE)dat->bTopmost);
		}
		break;
	case DM_DEACTIVATE:
		SetForegroundWindow(dat->foregroundWindow);
		break;
	case DM_ERRORDECIDED:
		break;
	case DM_STARTFLASHING:
		if (GetActiveWindow() != hwndDlg || GetForegroundWindow() != hwndDlg) {
			dat->nFlash = 0;
			SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
		}
		break;
	case DM_REMOVECHILD:
		{
			RemoveChild(dat, (HWND) lParam);
			if (dat->childrenCount != 0) {
				SetFocus(dat->hwndActive);
			} else {
				DestroyWindow(hwndDlg);
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
//		if((HWND) lParam != dat->hwndActive) {
			ActivateChild(dat, (HWND) lParam);
//		}
		return TRUE;
	case DM_ACTIVATEPREV:
		ActivatePrevChild(dat, (HWND) lParam);
		SetFocus(dat->hwndActive);
		return TRUE;
	case DM_ACTIVATENEXT:
		ActivateNextChild(dat, (HWND) lParam);
		SetFocus(dat->hwndActive);
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
			TCHAR *tContactName;
			if (mdat && mdat->hwnd == dat->hwndActive) {
				TCHAR *newtitle, oldtitle[256];
#if defined ( _UNICODE )
				newtitle = GetWindowTitle(mdat->hContact, mdat->szProto);
#else
				newtitle = GetWindowTitle(mdat->hContact, mdat->szProto);
#endif
				GetWindowText(hwndDlg, oldtitle, sizeof(oldtitle));
				if (lstrcmp(newtitle, oldtitle)) { //swt() flickers even if the title hasn't actually changed
					SetWindowText(hwndDlg, newtitle);
					//SendMessage(hwndDlg, WM_SIZE, 0, 0);
				}
				free(newtitle);
			}
			tabId = GetTabFromHWND(dat, mdat->hwnd);
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
					char *szProto = mdat->szProto;
					HANDLE hContact = mdat->hContact;
					if (strcmp(mdat->szProto, "MetaContacts") == 0 && DBGetContactSettingByte(NULL,"CLC","Meta",0) == 0) {
						hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT,(UINT)mdat->hContact, 0);
						if (hContact != NULL) {
							szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(UINT)hContact,0);
						} else {
							hContact = mdat->hContact;
						}
					}
					wStatus = DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
					mdat->wStatus = wStatus;
					if (mdat->hwnd == dat->hwndActive) {
						if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON)) {
							if (mdat->showTyping && (g_dat->flags&SMF_SHOWTYPINGWIN)) {
								SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) g_dat->hIcons[SMF_ICON_TYPING]);
							} else if (mdat->showUnread && (GetActiveWindow() != hwndDlg || GetForegroundWindow() != hwndDlg)) {
								SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
							} else {
								SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) LoadSkinnedProtoIcon(szProto, wStatus));
							}
						} else {
							SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
						}
					}
					SendDlgItemMessage(mdat->hwnd, IDC_USERMENU, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadSkinnedProtoIcon(szProto, wStatus));
					icoIdx = 0;
					for (i = 0; i < g_dat->protoNum; i++) {
						if (!strcmp(g_dat->protoNames[i], szProto)) {
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
					i = GetTabFromHWND(dat, mdat->hwnd);
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

#define INDICATOR_WIDTH 3
#define INDICATOR_COLOR 26

BOOL CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct TabCtrlData *dat;
	dat = (struct TabCtrlData *) GetWindowLong(hwnd, GWL_USERDATA);
    switch(msg) {
        case WM_MBUTTONDOWN:
		{
			TCITEM tci;
			int tabId;
			struct MessageWindowData *mwd;
			TCHITTESTINFO thinfo;
			thinfo.pt.x = (lParam<<16)>>16;
			thinfo.pt.y = lParam>>16;
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
		case WM_LBUTTONDBLCLK:
		{
			TCHITTESTINFO thinfo;
			int tabId;
			thinfo.pt.x = (lParam<<16)>>16;
			thinfo.pt.y = lParam>>16;
			tabId = TabCtrl_HitTest(hwnd, &thinfo);
			if (tabId >=0 ) {
				void * clickChild = GetChildFromTab(hwnd, tabId)->hwnd;
				if (clickChild == dat->lastClickChild) {
					SendMessage(clickChild, WM_CLOSE, 0, 0);
				}
			}
			dat->lastClickChild = NULL;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			if (!dat->bDragging) {
				FILETIME ft;
				TCHITTESTINFO thinfo;
				GetSystemTimeAsFileTime(&ft);
				thinfo.pt.x = (lParam<<16)>>16;
				thinfo.pt.y = lParam>>16;
				dat->srcTab = dat->destTab = TabCtrl_HitTest(hwnd, &thinfo);
				if (dat->srcTab >=0 ) {
					dat->lastClickChild = GetChildFromTab(hwnd, dat->srcTab)->hwnd;
				} else {
					dat->lastClickChild = NULL;
				}
				dat->bDragging = TRUE;
				dat->bDragged = FALSE;
				dat->clickLParam = lParam;
				dat->clickWParam = wParam;
				dat->lastClickTime = ft.dwLowDateTime;
				dat->mouseLBDownPos.x = thinfo.pt.x;
				dat->mouseLBDownPos.y = thinfo.pt.y;
				SetCapture(hwnd);
				return 0;
			}
		}
		break;
		case WM_CAPTURECHANGED:
		case WM_LBUTTONUP:
			if (dat->bDragging) {
				TCHITTESTINFO thinfo;
				thinfo.pt.x = (lParam<<16)>>16;
				thinfo.pt.y = lParam>>16;
				if (dat->bDragged) {
					ImageList_DragLeave(GetDesktopWindow());
					ImageList_EndDrag();
					ImageList_Destroy(dat->hDragImageList);
					SetCursor(LoadCursor(NULL, IDC_ARROW));
					dat->destTab = TabCtrl_HitTest(hwnd, &thinfo);
					if (thinfo.flags != TCHT_NOWHERE && dat->destTab != dat->srcTab)  {
						NMHDR nmh;
						TCHAR  sBuffer[501];
						TCITEM item;
						int curSel;
						curSel = TabCtrl_GetCurSel(hwnd);
						item.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT;
						item.pszText = sBuffer;
						item.cchTextMax = sizeof(sBuffer)/sizeof(TCHAR);
						TabCtrl_GetItem(hwnd, dat->srcTab, &item);
						sBuffer[sizeof(sBuffer)/sizeof(TCHAR)-1] = '\0';
						if (curSel == dat->srcTab) {
							curSel = dat->destTab;
						} else {
							if (curSel > dat->srcTab && curSel <= dat->destTab) {
								curSel--;
							} else if (curSel < dat->srcTab && curSel >= dat->destTab) {
								curSel++;
							}
						}
						TabCtrl_DeleteItem(hwnd, dat->srcTab);
						TabCtrl_InsertItem(hwnd, dat->destTab, &item );
						TabCtrl_SetCurSel(hwnd, curSel);
						nmh.hwndFrom = hwnd;
						nmh.idFrom = GetDlgCtrlID(hwnd);
						nmh.code = TCN_SELCHANGE;
						SendMessage(GetParent(hwnd), WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
						UpdateWindow(hwnd);
					}
				} else {
					SendMessage(hwnd, WM_LBUTTONDOWN, dat->clickWParam, dat->clickLParam);
				}
				dat->bDragged = FALSE;
				dat->bDragging = FALSE;
				ReleaseCapture();
			}
			break;
		case WM_MOUSEMOVE:
			if (!(wParam & MK_LBUTTON)) break;
			if (dat->bDragging) {
				FILETIME ft;
				TCHITTESTINFO thinfo;
				GetSystemTimeAsFileTime(&ft);
				thinfo.pt.x = (lParam<<16)>>16;
				thinfo.pt.y = lParam>>16;
				if (!dat->bDragged) {
					if ((abs(thinfo.pt.x-dat->mouseLBDownPos.x)<3 && abs(thinfo.pt.y-dat->mouseLBDownPos.y)<3)
						|| (ft.dwLowDateTime - dat->lastClickTime) < 10*1000*150)
						break;
				}
				if (!dat->bDragged) {
					POINT pt;
					RECT rect;
					RECT rect2;
					HDC hDC, hMemDC;
					HBITMAP hBitmap, hOldBitmap;
					HBRUSH hBrush = CreateSolidBrush(RGB(255,0,254));
					GetCursorPos(&pt);
					TabCtrl_GetItemRect(hwnd, dat->srcTab, &rect);
					rect.right -= rect.left-1;
					rect.bottom -= rect.top-1;
					rect2.left = 0; rect2.right = rect.right; rect2.top = 0; rect2.bottom = rect.bottom;
					dat->hDragImageList = ImageList_Create(rect.right, rect.bottom, ILC_COLOR | ILC_MASK, 0, 1);
					hDC = GetDC(hwnd);
					hMemDC = CreateCompatibleDC(hDC);
					hBitmap = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
					hOldBitmap = SelectObject(hMemDC, hBitmap);
					FillRect(hMemDC, &rect2, hBrush);
					SetWindowOrgEx (hMemDC, rect.left, rect.top, NULL);
					SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)hMemDC, PRF_CLIENT);
					SelectObject(hMemDC, hOldBitmap);
					ImageList_AddMasked(dat->hDragImageList, hBitmap, RGB(255,0,254));
					DeleteObject(hBitmap);
					DeleteObject(hBrush);
					ReleaseDC(hwnd, hDC);
					DeleteDC(hMemDC);
					ImageList_BeginDrag(dat->hDragImageList, 0, dat->mouseLBDownPos.x - rect.left, dat->mouseLBDownPos.y - rect.top);
					ImageList_DragEnter(GetDesktopWindow(), pt.x, pt.y);
					SetCursor(hDragCursor);
					dat->mouseLBDownPos.x = thinfo.pt.x;
					dat->mouseLBDownPos.y = thinfo.pt.y;
				} else {
					POINT pt;
					GetCursorPos(&pt);
					ImageList_DragMove(pt.x, pt.y);
				}
				dat->bDragged = TRUE;
				return TRUE;
			}
			break;
    }
	return CallWindowProc(OldTabCtrlProc, hwnd, msg, wParam, lParam);
}


int ScriverRestoreWindowPosition(HWND hwnd,HANDLE hContact,const char *szModule,const char *szNamePrefix, int flags, int showCmd)
{
	RECT rcDesktop;
	WINDOWPLACEMENT wp;
	char szSettingName[64];
	int x,y;
	MONITORINFO mi;
	HMONITOR hMonitor;
//	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
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

	hMonitor = MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTONEAREST);
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMonitor, &mi);			
	rcDesktop = mi.rcWork;
	if (wp.rcNormalPosition.left > rcDesktop.right || wp.rcNormalPosition.top > rcDesktop.bottom 
		|| wp.rcNormalPosition.right < rcDesktop.left || wp.rcNormalPosition.bottom < rcDesktop.top) return 1;
	SetWindowPlacement(hwnd,&wp);
	return 0;
}
