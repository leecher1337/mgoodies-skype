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
#include "statusicon.h"
#include "chat/chat.h"

/*
#ifdef _MSC_VER
#define STRING2(x) #x
#define STRING(x) STRING2(x)
#pragma message ("_MSC_VER: "STRING(_MSC_VER))
#endif
*/
#ifndef __MINGW32__
#if (_MSC_VER < 1300)
#include "multimon.h"
#endif
#endif

extern HINSTANCE g_hInst;
extern HCURSOR hDragCursor;

#define SB_CHAR_WIDTH		 40
#define SB_SENDING_WIDTH 	 25
#define SB_UNICODE_WIDTH 	 18

#define TIMERID_FLASHWND     1
#define TIMEOUT_FLASHWND     900

static WNDPROC OldTabCtrlProc;
static void DrawTab(HWND hwnd, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern TCHAR *GetNickname(HANDLE hContact, const char* szProto);


void SubclassTabCtrl(HWND hwnd) {
	OldTabCtrlProc = (WNDPROC) SetWindowLong(hwnd, GWL_WNDPROC, (LONG) TabCtrlProc);
	SendMessage(hwnd, EM_SUBCLASSED, 0, 0);
}

void UnsubclassTabCtrl(HWND hwnd) {
	SendMessage(hwnd, EM_UNSUBCLASSED, 0, 0);
	SetWindowLong(hwnd, GWL_WNDPROC, (LONG) OldTabCtrlProc);
}

TCHAR* GetWindowTitle(HANDLE *hContact, const char *szProto)
{
	DBVARIANT dbv;
	int isTemplate;
	int len, contactNameLen = 0, statusLen = 0, statusMsgLen = 0, protocolLen = 0;
	TCHAR *p, *tmplt, *szContactName = NULL, *szStatus = NULL, *szStatusMsg = NULL, *szProtocol = NULL, *title;
	TCHAR *pszNewTitleEnd = mir_tstrdup(TranslateT("Message Session"));
	isTemplate = 0;
	if (hContact && szProto) {
		szContactName = GetNickname(hContact, szProto);
		contactNameLen = lstrlen(szContactName);
		szStatus = a2t((char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, szProto == NULL ? ID_STATUS_OFFLINE : DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE), 0));
		statusLen = lstrlen(szStatus);
		if (!DBGetContactSetting(hContact, "CList", "StatusMsg",&dbv)) {
			if (strlen(dbv.pszVal) > 0) {
				int i, j;
       			szStatusMsg = a2t(dbv.pszVal);
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
			tmplt = a2t(dbv.pszVal);
			DBFreeVariant(&dbv);
		} else {
			if (g_dat->flags & SMF_STATUSICON) {
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
	title = (TCHAR *)mir_alloc(sizeof(TCHAR) * (len + 1));
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
		mir_free(tmplt);
	}
	mir_free(szContactName);
	mir_free(szStatus);
	mir_free(pszNewTitleEnd);
	if (szStatusMsg)
		mir_free(szStatusMsg);
	return title;
}

TCHAR* GetTabName(HANDLE *hContact)
{
	if (hContact) {
		return GetNickname(hContact, NULL);
	}
	return NULL;
}

static int GetChildCount(ParentWindowData *dat) {
	return TabCtrl_GetItemCount(dat->hwndTabs);
}

static void GetChildWindowRect(ParentWindowData *dat, RECT *rcChild)
{
	RECT rc, rcStatus, rcTabs;
	GetClientRect(dat->hwnd, &rc);
	GetClientRect(dat->hwndTabs, &rcTabs);
	TabCtrl_AdjustRect(dat->hwndTabs, FALSE, &rcTabs);
	rcStatus.top = rcStatus.bottom = 0;
	if (dat->flags2 & SMF2_SHOWSTATUSBAR) {
		GetWindowRect(dat->hwndStatus, &rcStatus);
	}
	rcChild->left = 0;
	rcChild->right = rc.right;
	if (dat->flags2 & SMF2_TABSATBOTTOM) {
		rcChild->top = 2;
		if ((dat->flags2 & SMF2_USETABS && !(dat->flags2 & SMF2_HIDEONETAB)) || (dat->childrenCount > 1)) {
			rcChild->bottom = rcTabs.bottom + 4;
		} else {
			rcChild->bottom = rc.bottom - rc.top - (rcStatus.bottom - rcStatus.top);
		}
	} else {
		if ((dat->flags2 & SMF2_USETABS && !(dat->flags2 & SMF2_HIDEONETAB)) || (dat->childrenCount > 1)) {
			rcChild->top = rcTabs.top;
		} else {
			rcChild->top = 2;//rcTabs.top - 2;
		}
		rcChild->bottom = rc.bottom - rc.top - (rcStatus.bottom - rcStatus.top);
	}
}

static int GetTabFromHWND(ParentWindowData *dat, HWND child)
{
	MessageWindowTabData * mwtd;
	TCITEM tci;
	int l, i;
	l = TabCtrl_GetItemCount(dat->hwndTabs);
	for (i = 0; i < l; i++) {
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(dat->hwndTabs, i, &tci);
		mwtd = (MessageWindowTabData *) tci.lParam;
		if (mwtd->hwnd == child) {
			return i;
		}
	}
	return -1;

}

static MessageWindowTabData * GetChildFromTab(HWND hwndTabs, int tabId)
{
	TCITEM tci;
	tci.mask = TCIF_PARAM;
	TabCtrl_GetItem(hwndTabs, tabId, &tci);
	return (MessageWindowTabData *) tci.lParam;
}

static MessageWindowTabData * GetChildFromHWND(ParentWindowData *dat, HWND hwnd)
{
	MessageWindowTabData * mwtd;
	TCITEM tci;
	int l, i;
	l = TabCtrl_GetItemCount(dat->hwndTabs);
	for (i = 0; i < l; i++) {
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(dat->hwndTabs, i, &tci);
		mwtd = (MessageWindowTabData *) tci.lParam;
		if (mwtd->hwnd == hwnd) {
			return mwtd;
		}
	}
	return NULL;
}

static void GetMinimunWindowSize(ParentWindowData *dat, SIZE *size)
{
	MINMAXINFO mmi;
	RECT rc, rcWindow;
	int i, minW = 216, minH = 80;
	GetWindowRect(dat->hwnd, &rcWindow);
	GetChildWindowRect(dat, &rc);
	for (i=0;i<dat->childrenCount;i++) {
		MessageWindowTabData * mwtd = GetChildFromTab(dat->hwndTabs, i);
		SendMessage(mwtd->hwnd, WM_GETMINMAXINFO, 0, (LPARAM) &mmi);
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

static void SetupStatusBar(ParentWindowData *dat)
{
	int statusIconNum = GetStatusIconsCount(dat->hContact);
	int statwidths[4];
	RECT rc;
	GetClientRect(dat->hwnd, &rc);
	statwidths[0] = rc.right - rc.left - SB_CHAR_WIDTH - SB_UNICODE_WIDTH - 2 * (statusIconNum > 0) - statusIconNum * (GetSystemMetrics(SM_CXSMICON) + 2);
	statwidths[1] = rc.right - rc.left - SB_UNICODE_WIDTH - 2 * (statusIconNum > 0) - statusIconNum * (GetSystemMetrics(SM_CXSMICON) + 2);
	statwidths[2] = rc.right - rc.left - SB_UNICODE_WIDTH;
	statwidths[3] = -1;
	SendMessage(dat->hwndStatus, SB_SETPARTS, 4, (LPARAM) statwidths);
	SendMessage(dat->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, (LPARAM)0);
	SendMessage(dat->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_NOBORDERS) | 3, (LPARAM)0);
}

static void ActivateChild(ParentWindowData *dat, HWND child) {
	int i;
	RECT rcChild;
	MessageWindowTabData *mwtd;
	GetChildWindowRect(dat, &rcChild);
	SetWindowPos(child, HWND_TOP, rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, SWP_NOSIZE);
	i = GetTabFromHWND(dat, child);
	mwtd = GetChildFromTab(dat->hwndTabs, i);
	dat->hContact = mwtd->hContact;
	if(child != dat->hwndActive) {
		HWND prev = dat->hwndActive;
		dat->hwndActive = child;
		SetupStatusBar(dat);
		SendMessage(dat->hwndActive, DM_UPDATESTATUSBAR, 0, 0);
		SendMessage(dat->hwndActive, DM_UPDATETITLEBAR, 0, 0);
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
		ShowWindow(dat->hwndActive, SW_SHOWNOACTIVATE);
		SendMessage(dat->hwndActive, DM_SCROLLLOGTOBOTTOM, 0, 0);
		if (prev!=NULL) ShowWindow(prev, SW_HIDE);
	} else {
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
	}
	TabCtrl_SetCurSel(dat->hwndTabs, i);
	SendMessage(dat->hwndActive, DM_ACTIVATE, WA_ACTIVE, 0);
}

static void AddChild(ParentWindowData *dat, HWND hwnd, HANDLE hContact)
{
	TCITEM tci;
	int tabId;
	MessageWindowTabData *mwtd = (MessageWindowTabData *) mir_alloc(sizeof(MessageWindowTabData));
	mwtd->hwnd = hwnd;
	mwtd->hContact = hContact;
	mwtd->szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	mwtd->parent = dat;
	dat->childrenCount++;
	tci.mask = TCIF_PARAM;
	tci.lParam = (LPARAM) mwtd;
	tabId = TabCtrl_InsertItem(dat->hwndTabs, dat->childrenCount-1, &tci);
//	ActivateChild(dat, mdat->hwnd);
	SetWindowPos(mwtd->hwnd, HWND_TOP, dat->childRect.left, dat->childRect.top, dat->childRect.right-dat->childRect.left, dat->childRect.bottom - dat->childRect.top, SWP_HIDEWINDOW);
	SendMessage(dat->hwnd, WM_SIZE, 0, 0);
}

static void RemoveChild(ParentWindowData *dat, HWND child)
{
	int tab = GetTabFromHWND(dat, child);
	if (tab >= 0) {
		TCITEM tci;
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(dat->hwndTabs, tab, &tci);
		TabCtrl_DeleteItem(dat->hwndTabs, tab);
		mir_free((MessageWindowTabData *) tci.lParam);
		dat->childrenCount--;
		if (child == dat->hwndActive) {
			if (tab == TabCtrl_GetItemCount(dat->hwndTabs)) tab--;
			if (tab >=0 ) {
				ActivateChild(dat, GetChildFromTab(dat->hwndTabs, tab)->hwnd);
			}
		}
	}
}

static void CloseOtherChilden(ParentWindowData *dat, HWND child)
{
	int i;
	ActivateChild(dat, child);
	for (i=dat->childrenCount-1;i>=0;i--) {
		MessageWindowTabData *mwtd = GetChildFromTab(dat->hwndTabs, i);
		if (mwtd != NULL && mwtd->hwnd != child) {
			SendMessage(mwtd->hwnd, WM_CLOSE, 0, 0);
		}
	}
	ActivateChild(dat, child);
}

static void ActivateNextChild(ParentWindowData *dat, HWND child)
{
	int i = GetTabFromHWND(dat, child);
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	i = (i+1) % l;
	ActivateChild(dat, GetChildFromTab(dat->hwndTabs, i)->hwnd);
}

static void ActivatePrevChild(ParentWindowData *dat, HWND child)
{
	int i = GetTabFromHWND(dat, child);
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	i = (i+l-1) % l;
	ActivateChild(dat, GetChildFromTab(dat->hwndTabs, i)->hwnd);
}


static void SetContainerWindowStyle(ParentWindowData *dat)
{
	DWORD ws;
	RECT rc;
	if (!(dat->flags2 & SMF2_SHOWSTATUSBAR)) {
		ShowWindow(dat->hwndStatus, SW_HIDE);
	} else {
		ShowWindow(dat->hwndStatus, SW_SHOW);
	}
	ws = GetWindowLong(dat->hwnd, GWL_STYLE) & ~(WS_CAPTION);
	if (dat->flags2 & SMF2_SHOWTITLEBAR) {
		ws |= WS_CAPTION;
	}
	SetWindowLong(dat->hwnd, GWL_STYLE, ws);

	ws = GetWindowLong(dat->hwnd, GWL_EXSTYLE)& ~WS_EX_LAYERED;
	ws |= dat->flags2 & SMF2_USETRANSPARENCY ? WS_EX_LAYERED : 0;
	SetWindowLong(dat->hwnd , GWL_EXSTYLE , ws);
	if (dat->flags2 & SMF2_USETRANSPARENCY) {
		pSetLayeredWindowAttributes(dat->hwnd, RGB(255,255,255), (BYTE)(255-g_dat->inactiveAlpha), LWA_ALPHA);
	}

	ws = GetWindowLong(dat->hwndTabs, GWL_STYLE) & ~(TCS_BOTTOM | 0x2000);
	if (dat->flags2 & SMF2_TABSATBOTTOM) {
		ws |= TCS_BOTTOM;
	}
	if (dat->flags2 & SMF2_TABCLOSEBUTTON) {
		ws |= 0x2000; //TCS_OWNERDRAWFIXED
		TabCtrl_SetPadding(dat->hwndTabs, GetSystemMetrics(SM_CXEDGE) + 12, GetSystemMetrics(SM_CYEDGE) + 1);
	} else {
		TabCtrl_SetPadding(dat->hwndTabs, GetSystemMetrics(SM_CXEDGE) + 4, GetSystemMetrics(SM_CYEDGE) + 1);
	}
	SetWindowLong(dat->hwndTabs, GWL_STYLE, ws);
	GetWindowRect(dat->hwnd, &rc);
	SetWindowPos(dat->hwnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
}

BOOL CALLBACK DlgProcParentWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DWORD ws;
	ParentWindowData *dat;
	dat = (ParentWindowData *) GetWindowLong(hwndDlg, GWL_USERDATA);
	if (!dat && msg!=WM_INITDIALOG) return FALSE;
	switch (msg) {
	case WM_INITDIALOG:
		{
			HMENU hMenu;
			HANDLE hSContact;
			int savePerContact = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT);
			NewMessageWindowLParam *newData = (NewMessageWindowLParam *) lParam;
			dat = (ParentWindowData *) mir_alloc(sizeof(ParentWindowData));
			dat->foregroundWindow = GetForegroundWindow();
			dat->hContact = newData->hContact;
			dat->nFlash = 0;
			dat->nFlashMax = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT);
			dat->childrenCount = 0;
			dat->hwnd = hwndDlg;
			dat->mouseLBDown = 0;
			dat->windowWasCascaded = 0;
			dat->bMinimized = 0;
			dat->bVMaximized = 0;
			dat->flags2 = g_dat->flags2;
			dat->hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hwndDlg, NULL, g_hInst, NULL);
			dat->isChat = newData->isChat;
			SendMessage(dat->hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);
			//SetupStatusBar(dat);
			dat->hwndTabs = GetDlgItem(hwndDlg, IDC_TABS);
			dat->hwndActive = NULL;
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) dat);
			if (g_dat->hTabIconList != NULL) {
				TabCtrl_SetImageList(dat->hwndTabs, g_dat->hTabIconList);
			}
			dat->next = NULL;
			if (!newData->isChat) {
				dat->prev = g_dat->lastParent;
				g_dat->lastParent = dat;
			} else {
				dat->prev = g_dat->lastChatParent;
				g_dat->lastChatParent = dat;
			}
			if (dat->prev != NULL) {
				dat->prev->next = dat;
			}
			WindowList_Add(g_dat->hParentWindowList, hwndDlg, hwndDlg);
			SubclassTabCtrl(dat->hwndTabs);

			SetContainerWindowStyle(dat);

//			hSContact = !(dat->flags2 & SMF2_USETABS) && savePerContact ? dat->hContact : NULL;
			hSContact = savePerContact ? dat->hContact : NULL;
			dat->bTopmost = DBGetContactSettingByte(hSContact, SRMMMOD, SRMSGSET_TOPMOST, SRMSGDEFSET_TOPMOST);
			if (ScriverRestoreWindowPosition(hwndDlg, hSContact, SRMMMOD, (newData->isChat && !savePerContact) ? "chat" : "", 0, SW_HIDE)) {
				if (ScriverRestoreWindowPosition(hwndDlg, hSContact, SRMMMOD, (newData->isChat && !savePerContact) ? "chat" : "", RWPF_NOSIZE, SW_HIDE)) {
					SetWindowPos(GetParent(hwndDlg), 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_HIDEWINDOW);
				} else {
					SetWindowPos(hwndDlg, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_HIDEWINDOW);
				}
			}
//			if (!(dat->flags2 & SMF2_USETABS)) {
				if (!savePerContact && DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_CASCADE, SRMSGDEFSET_CASCADE))
					WindowList_Broadcast(g_dat->hParentWindowList, DM_CASCADENEWWINDOW, (WPARAM) hwndDlg, (LPARAM) &dat->windowWasCascaded);
	//		}
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
		if (dat->bVMaximized) {
			MONITORINFO mi;
			HMONITOR hMonitor;
			WINDOWPLACEMENT wp;
			RECT rcDesktop;
			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			hMonitor = MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTONEAREST);
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);
			rcDesktop = mi.rcWork;
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
			RECT rc, rcStatus, rcChild, rcWindow;
			SIZE size;
			dat->bMinimized = 0;
			GetClientRect(hwndDlg, &rc);
			GetWindowRect(hwndDlg, &rcWindow);
			rcStatus.top = rcStatus.bottom = 0;
			if (dat->flags2 & SMF2_SHOWSTATUSBAR) {
				GetWindowRect(dat->hwndStatus, &rcStatus);
				SetupStatusBar(dat);
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
			MoveWindow(dat->hwndActive, rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, TRUE);
			RedrawWindow(GetDlgItem(dat->hwndActive, IDC_LOG), NULL, NULL, RDW_INVALIDATE);
			if (dat->flags2 & SMF2_SHOWSTATUSBAR) {
				SendMessage(dat->hwndStatus, WM_SIZE, 0, 0);
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
	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;
			if (dat && dat->hwndActive && dis->hwndItem == dat->hwndStatus) {
				MessageWindowTabData *mwtd = GetChildFromHWND(dat, dat->hwndActive);
				if (mwtd != NULL) {
					DrawStatusIcons(mwtd->hContact, dis->hDC, dis->rcItem, 2);
				}
				return TRUE;
			} else if (dis->hwndItem == dat->hwndTabs) {
				DrawTab(dat->hwndTabs, wParam, lParam);
				return TRUE;
			}
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		}
	case WM_COMMAND:
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM) dat->hContact)) {
			break;
		}
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			//DestroyWindow(hwndDlg);
			return TRUE;
		}
		break;
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
							MessageWindowTabData * mwtd = (MessageWindowTabData *) tci.lParam;
							ActivateChild(dat, mwtd->hwnd);
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
							HMENU hMenu, hSubMenu, hUserMenu;
							BOOL menuResult;
							MessageWindowTabData * mwtd = GetChildFromTab(dat->hwndTabs, tabId);
							hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
							hSubMenu = GetSubMenu(hMenu, 3);
							hUserMenu = (HMENU) SendMessage(mwtd->hwnd, DM_GETCONTEXTMENU, 0, 0);
							if (hUserMenu != NULL) {
								InsertMenu(hSubMenu, 0, MF_POPUP | MF_BYPOSITION, (UINT)hUserMenu, TranslateT("User Menu"));
								InsertMenu(hSubMenu, 1, MF_SEPARATOR | MF_BYPOSITION, 0, 0);
							}
							menuResult = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, x, y, 0, hwndDlg, NULL);
							switch (menuResult) {
							case IDM_CLOSETAB:
								SendMessage(mwtd->hwnd, WM_CLOSE, 0, 0);
								break;
							case IDM_CLOSEOTHERTABS:
								CloseOtherChilden(dat, mwtd->hwnd);
								break;
							default:
								CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(menuResult), MPCF_CONTACTMENU), (LPARAM) mwtd->hContact);
							}
							if (hUserMenu != NULL) {
								DestroyMenu(hUserMenu);
							}
							DestroyMenu(hMenu);
						}
					}
					break;
				}
			} else if (pNMHDR->hwndFrom == dat->hwndStatus)  {
				switch (pNMHDR->code) {
				case NM_CLICK:
//				case NM_RCLICK:
					{
						NMMOUSE *nm=(NMMOUSE*)lParam;
						RECT rc;
						char str[1024];
						SendMessage(dat->hwndStatus, SB_GETRECT, SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 2, (LPARAM)&rc);
						if (nm->pt.x >= rc.left) {
							MessageWindowTabData *mwtd = GetChildFromHWND(dat, dat->hwndActive);
							if (mwtd != NULL) {
								CheckStatusIconClick(mwtd->hContact, dat->hwndStatus, nm->pt, rc, 2, (pNMHDR->code == NM_RCLICK ? MBCF_RIGHTBUTTON : 0));
							}
						}
 						return TRUE;
					}
				}
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
		if (dat->hwndStatus && dat->hwndStatus == (HWND) wParam) {
			RECT rc;
			POINT pt, pt2;
			GetCursorPos(&pt);
			pt2.x = pt.x;
			pt2.y = pt.y;
			ScreenToClient(dat->hwndStatus, &pt);

			SendMessage(dat->hwndStatus, SB_GETRECT, SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 2, (LPARAM)&rc);
			if(pt.x >= rc.left) {
				MessageWindowTabData *mwtd = GetChildFromHWND(dat, dat->hwndActive);
				if (mwtd != NULL) {
					CheckStatusIconClick(mwtd->hContact, dat->hwndStatus, pt, rc, 2, MBCF_RIGHTBUTTON);
				}
				break;
			} else
				SendMessage(dat->hwndActive, WM_CONTEXTMENU, (WPARAM)hwndDlg, 0);
		}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			ws = GetWindowLong(hwndDlg, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			ws |= dat->flags2 & SMF2_USETRANSPARENCY ? WS_EX_LAYERED : 0;
			SetWindowLong(hwndDlg , GWL_EXSTYLE , ws);
			if (dat->flags2 & SMF2_USETRANSPARENCY) {
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
		ws |= dat->flags2 & SMF2_USETRANSPARENCY ? WS_EX_LAYERED : 0;
		SetWindowLong(hwndDlg , GWL_EXSTYLE , ws);
		if (dat->flags2 & SMF2_USETRANSPARENCY) {
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
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
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
		break;
	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_MAXIMIZE) {
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				dat->bVMaximized = 1;
			} else {
				dat->bVMaximized = 0;
			}
		}
		else if ((wParam & 0xFFF0) == SC_MOVE) {
			RECT  rc;
			GetWindowRect(hwndDlg, &rc);
			dat->mouseLBDownPos.x = ((LONG) lParam << 16 >> 16) - rc.left;
			dat->mouseLBDownPos.y = ((LONG) lParam >> 16) - rc.top;
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
			int i;
			char szSettingName[64];
			char *szNamePrefix;
			int savePerContact = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT);
			for (i=dat->childrenCount;--i>=0;) {
				TCITEM tci;
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(dat->hwndTabs, i, &tci);
				TabCtrl_DeleteItem(dat->hwndTabs, i);
				mir_free((MessageWindowTabData *) tci.lParam);
			}
			SetWindowLong(hwndDlg, GWL_USERDATA, 0);
			WindowList_Remove(g_dat->hParentWindowList, hwndDlg);
			if (savePerContact)
//			if (!(dat->flags2 & SMF2_USETABS) && DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT))
				hContact = dat->hContact;
			else
				hContact = NULL;
			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			szNamePrefix = (!savePerContact && dat->isChat) ? "chat" : "";
			if (!dat->windowWasCascaded) {
				wsprintfA(szSettingName,"%sx",szNamePrefix);
				DBWriteContactSettingDword(hContact, SRMMMOD, szSettingName, wp.rcNormalPosition.left);
				wsprintfA(szSettingName,"%sy",szNamePrefix);
				DBWriteContactSettingDword(hContact, SRMMMOD, szSettingName, wp.rcNormalPosition.top);
			}
			wsprintfA(szSettingName,"%swidth",szNamePrefix);
			DBWriteContactSettingDword(hContact, SRMMMOD, szSettingName, wp.rcNormalPosition.right - wp.rcNormalPosition.left);
			wsprintfA(szSettingName,"%sheight",szNamePrefix);
			DBWriteContactSettingDword(hContact, SRMMMOD, szSettingName, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
			DBWriteContactSettingByte(hContact, SRMMMOD, SRMSGSET_TOPMOST, (BYTE)dat->bTopmost);
			if (g_dat->lastParent == dat) {
				g_dat->lastParent = dat->prev;
			}
			if (g_dat->lastChatParent == dat) {
				g_dat->lastChatParent = dat->prev;
			}
			if (dat->prev != NULL) {
				dat->prev->next = dat->next;
			}
			if (dat->next != NULL) {
				dat->next->prev = dat->prev;
			}
			UnsubclassTabCtrl(dat->hwndTabs);
			mir_free(dat);
		}
		break;
	case DM_DEACTIVATE:
		SetForegroundWindow(dat->foregroundWindow);
		break;
	case DM_ERRORDECIDED:
		break;
	case CM_STARTFLASHING:
		if ((GetActiveWindow() != hwndDlg || GetForegroundWindow() != hwndDlg)) {// && !(g_dat->flags2 & SMF2_STAYMINIMIZED)) {
			dat->nFlash = 0;
			SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
		}
		break;
	case CM_REMOVECHILD:
		{
			RemoveChild(dat, (HWND) lParam);
			if (dat->childrenCount != 0) {
				SetFocus(dat->hwndActive);
			} else {
				DestroyWindow(hwndDlg);
			}
		}
		return TRUE;
	case CM_ADDCHILD:
		{
			AddChild(dat, (HWND)wParam, (HANDLE)lParam);
		}
		return TRUE;
	case CM_ACTIVATECHILD:
//		if((HWND) lParam != dat->hwndActive) {
			ActivateChild(dat, (HWND) lParam);
//		}
		return TRUE;
	case CM_ACTIVATEPREV:
		ActivatePrevChild(dat, (HWND) lParam);
		SetFocus(dat->hwndActive);
		return TRUE;
	case CM_ACTIVATENEXT:
		ActivateNextChild(dat, (HWND) lParam);
		SetFocus(dat->hwndActive);
		return TRUE;
	case CM_GETCHILDCOUNT:
		SetWindowLong(hwndDlg, DWL_MSGRESULT, (LONG)GetChildCount(dat));
		return TRUE;
	case DM_SENDMESSAGE:
		{
			int i;
			for (i=0;i<dat->childrenCount;i++) {
				MessageWindowTabData * mwtd = GetChildFromTab(dat->hwndTabs, i);
				SendMessage(mwtd->hwnd, DM_SENDMESSAGE, wParam, lParam);
			}
		}
		break;
	case DM_OPTIONSAPPLIED:
		{
			dat->flags2 = g_dat->flags2;
			SetContainerWindowStyle(dat);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			//RedrawWindow(hwndDlg, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
			break;
		}
	case CM_UPDATETITLEBAR:
		{
			HWND hwnd = (HWND) lParam;
			TitleBarData *tbd = (TitleBarData *) wParam;
			if (tbd != NULL) {
				if ((tbd->iFlags & TBDF_TEXT) && dat->hwndActive == hwnd) {
					TCHAR oldtitle[256];
					GetWindowText(hwndDlg, oldtitle, sizeof(oldtitle));
					if (lstrcmp(tbd->pszText, oldtitle)) { //swt() flickers even if the title hasn't actually changed
						SetWindowText(hwndDlg, tbd->pszText);
						//SendMessage(hwndDlg, WM_SIZE, 0, 0);
					}
				}
				if ((tbd->iFlags & TBDF_ICON) &&  hwnd == dat->hwndActive) {
					SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) tbd->hIcon);
				}
			}
			break;
		}
	case CM_UPDATESTATUSBAR:
		{
			HWND hwnd = (HWND) lParam;
			StatusBarData *sbd = (StatusBarData *) wParam;
			int iItem = sbd->iItem;
			if (sbd != NULL) {
				if ((sbd->iFlags & SBDF_TEXT) && dat->hwndActive == hwnd) {
					SendMessage(dat->hwndStatus, SB_SETTEXT, iItem, (LPARAM) sbd->pszText);
				}
				if ((sbd->iFlags & SBDF_ICON) && dat->hwndActive == hwnd) {
					SendMessage(dat->hwndStatus, SB_SETICON, iItem, (LPARAM) sbd->hIcon);
				}
			}
			RedrawWindow(dat->hwndStatus, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			break;
		}
	case DM_STATUSICONCHANGE:
		SendMessage(dat->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, (LPARAM)0);
		return 0;
	case CM_UPDATETABCONTROL:
		{
			TCHAR *ptszTemp = NULL;
			TabControlData *tcd = (TabControlData *) wParam;
			int tabId = GetTabFromHWND(dat, (HWND) lParam);
			if (tabId >= 0 && tcd != NULL) {
				TCITEM tci;
				tci.mask = 0;
				if (tcd->iFlags & TCDF_TEXT) {
					tci.mask |= TCIF_TEXT;
					tci.pszText = tcd->pszText;
					if (g_dat->flags2 & SMF2_LIMITNAMES) {
						int len = lstrlen(tcd->pszText);
						if (len > g_dat->limitNamesLength ) {
							ptszTemp = mir_alloc(sizeof(TCHAR) * (len + 4));
							_tcsncpy(ptszTemp, tcd->pszText, g_dat->limitNamesLength + 1);
							_tcsncpy(ptszTemp + g_dat->limitNamesLength, _T("..."), 4);
							tci.pszText = ptszTemp;
						}
					}
				}
				if (tcd->iFlags & TCDF_ICON) {
					tci.mask |= TCIF_IMAGE;
					tci.iImage = tcd->iconIdx;
				}
				TabCtrl_SetItem(dat->hwndTabs, tabId, &tci);
			}
			mir_free(ptszTemp);
			break;
		}
	case DM_SWITCHSTATUSBAR:
		dat->flags2 ^= SMF2_SHOWSTATUSBAR;
		if (!(dat->flags2 & SMF2_SHOWSTATUSBAR)) {
			ShowWindow(dat->hwndStatus, SW_HIDE);
		} else {
			ShowWindow(dat->hwndStatus, SW_SHOW);
		}
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;
	case DM_SWITCHTOOLBAR:
		{
			int i;
			dat->flags2 ^= SMF2_SHOWTOOLBAR;
			for (i=0;i<dat->childrenCount;i++) {
				MessageWindowTabData * mwtd = GetChildFromTab(dat->hwndTabs, i);
				SendMessage(mwtd->hwnd, DM_SWITCHTOOLBAR, 0, 0);
			}
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
		}
		break;
	case DM_SWITCHTITLEBAR:
		{
			RECT rc;
			dat->flags2 ^= SMF2_SHOWTITLEBAR;
			ws = GetWindowLong(hwndDlg, GWL_STYLE) & ~(WS_CAPTION);
			if (dat->flags2 & SMF2_SHOWTITLEBAR) {
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

static void DrawTab(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	TCITEM tci;
	LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
	int	iTabIndex = lpDIS->itemID;
	if (iTabIndex >= 0) {
		TCHAR szLabel[1024];
		tci.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_STATE;
		tci.pszText = szLabel;
		tci.cchTextMax = SIZEOF(szLabel);
		tci.dwStateMask = TCIS_HIGHLIGHTED;
		if (TabCtrl_GetItem(hwnd, iTabIndex, &tci)) {
			IMAGEINFO info;
			RECT rIcon = lpDIS->rcItem;
			RECT rect = lpDIS->rcItem;
			int bSelected = lpDIS->itemState & ODS_SELECTED;
			int	iOldBkMode = SetBkMode(lpDIS->hDC, TRANSPARENT);
			int atTop = (GetWindowLong(hwnd, GWL_STYLE) & TCS_BOTTOM) == 0;
	//		COLORREF crOldColor = SetTextColor(lpDIS->hDC, (tci.dwState & TCIS_HIGHLIGHTED) ? RGB(196, 0, 0) : GetSysColor(COLOR_BTNTEXT));
			UINT dwFormat;
			if (pfnIsAppThemed && !pfnIsAppThemed()) {
				FillRect(lpDIS->hDC, &rect, GetSysColorBrush(COLOR_BTNFACE));
			}
			if (bSelected) {
			}
			if (atTop) {
				dwFormat = DT_SINGLELINE|DT_TOP|DT_CENTER|DT_NOPREFIX|DT_NOCLIP;
				rIcon.top = rect.top + GetSystemMetrics(SM_CYEDGE);
				if (tci.iImage >= 0) {
					rIcon.left = rect.left + GetSystemMetrics(SM_CXEDGE) + (bSelected ? 6 : 2);
					ImageList_GetImageInfo(g_dat->hTabIconList, tci.iImage, &info);
					ImageList_DrawEx(g_dat->hTabIconList, tci.iImage, lpDIS->hDC, rIcon.left, rIcon.top, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
					rect.left = rIcon.left + (info.rcImage.right - info.rcImage.left);
				}
				ImageList_GetImageInfo(g_dat->hButtonIconList, 0, &info);
				rIcon.left = rect.right - GetSystemMetrics(SM_CXEDGE) - (bSelected ? 6 : 2) - (info.rcImage.right - info.rcImage.left);
				ImageList_DrawEx(g_dat->hButtonIconList, 0, lpDIS->hDC, rIcon.left, rIcon.top, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
				rect.right = rIcon.left - 1;
				rect.top += GetSystemMetrics(SM_CYEDGE) + 2;
			} else {
				dwFormat = DT_SINGLELINE|DT_BOTTOM|DT_CENTER|DT_NOPREFIX|DT_NOCLIP;
				rIcon.left = rect.left + GetSystemMetrics(SM_CXEDGE) + (bSelected ? 6 : 2);
				if (tci.iImage >= 0) {
					ImageList_GetImageInfo(g_dat->hTabIconList, tci.iImage, &info);
					rIcon.top = rect.bottom - (info.rcImage.bottom - info.rcImage.top) - 2;
					ImageList_DrawEx(g_dat->hTabIconList, tci.iImage, lpDIS->hDC, rIcon.left, rIcon.top, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
					rect.left = rIcon.left + (info.rcImage.right - info.rcImage.left);
				}
				ImageList_GetImageInfo(g_dat->hButtonIconList, 0, &info);
				rIcon.top = rect.bottom - (info.rcImage.bottom - info.rcImage.top) - GetSystemMetrics(SM_CYEDGE);
				rIcon.left = rect.right - GetSystemMetrics(SM_CXEDGE) - (bSelected ? 6 : 2) - (info.rcImage.right - info.rcImage.left);
				ImageList_DrawEx(g_dat->hButtonIconList, 0, lpDIS->hDC, rIcon.left, rIcon.top, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
				rect.right = rIcon.left - 1;
				rect.bottom -= GetSystemMetrics(SM_CYEDGE) + 2;
			}
			DrawText(lpDIS->hDC, szLabel, -1, &rect, dwFormat);
			//SetTextColor(lpDIS->hDC, crOldColor);
			SetBkMode(lpDIS->hDC, iOldBkMode);
		}
	}
}

BOOL CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TabCtrlData *dat;
	dat = (TabCtrlData *) GetWindowLong(hwnd, GWL_USERDATA);
    switch(msg) {
    	case EM_SUBCLASSED:
			dat = (TabCtrlData *) mir_alloc(sizeof(TabCtrlData));
			SetWindowLong(hwnd, GWL_USERDATA, (LONG) dat);
			dat->bDragging = FALSE;
			dat->bDragged = FALSE;
			dat->lastClickTab = -1;
	        return 0;
        case WM_MBUTTONDOWN:
		{
			TCITEM tci;
			int tabId;
			MessageWindowTabData *mwtd;
			TCHITTESTINFO thinfo;
			thinfo.pt.x = (lParam<<16)>>16;
			thinfo.pt.y = lParam>>16;
			tabId = TabCtrl_HitTest(hwnd, &thinfo);
			if (tabId >= 0) {
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwnd, tabId, &tci);
				mwtd = (MessageWindowTabData *) tci.lParam;
				if (mwtd != NULL) {
					SendMessage(mwtd->hwnd, WM_CLOSE, 0, 0);
    			}
			}
	        return 0;
        }
		case WM_LBUTTONDBLCLK:
		{
			TCHITTESTINFO thinfo;
			int tabId;
			thinfo.pt.x = (lParam<<16)>>16;
			thinfo.pt.y = lParam>>16;
			tabId = TabCtrl_HitTest(hwnd, &thinfo);
			if (tabId >=0 ) {
				HWND clickChild = GetChildFromTab(hwnd, tabId)->hwnd;
				if (tabId == dat->lastClickTab) {
					SendMessage(clickChild, WM_CLOSE, 0, 0);
				}
			}
			dat->lastClickTab = -1;//Child = NULL;
		}
		break;
		case WM_LBUTTONDOWN:
			if (!dat->bDragging) {
				TCHITTESTINFO thinfo;
				int clickedTab;
				FILETIME ft;
				thinfo.pt.x = (lParam<<16)>>16;
				thinfo.pt.y = lParam>>16;
				clickedTab = TabCtrl_HitTest(hwnd, &thinfo);
				GetSystemTimeAsFileTime(&ft);
				dat->lastClickTab = dat->srcTab = dat->destTab = clickedTab;
				if (dat->srcTab >=0 ) {
					dat->bDragging = TRUE;
					dat->bDragged = FALSE;
					dat->clickLParam = lParam;
					dat->clickWParam = wParam;
					dat->lastClickTime = ft.dwLowDateTime;
					dat->mouseLBDownPos.x = thinfo.pt.x;
					dat->mouseLBDownPos.y = thinfo.pt.y;
					SetCapture(hwnd);
				}
				return 0;
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
					} else if (GetKeyState(VK_CONTROL) & 0x8000 && g_dat->flags2 & SMF2_USETABS) {
						MessageWindowTabData *mwtd;
						TCITEM tci;
						POINT pt;
						NewMessageWindowLParam newData = { 0 };
						tci.mask = TCIF_PARAM;
						TabCtrl_GetItem(hwnd, dat->srcTab, &tci);
						mwtd = (MessageWindowTabData *) tci.lParam;
						if (mwtd != NULL) {
							HWND hChild = mwtd->hwnd;
							HANDLE hContact = mwtd->hContact;
							HWND hParent;
							GetCursorPos(&pt);
							hParent = WindowFromPoint(pt);
							while (GetParent(hParent) != NULL) {
								hParent = GetParent(hParent);
							}
							hParent = WindowList_Find(g_dat->hParentWindowList, hParent);
							if ((hParent != NULL && hParent != GetParent(hwnd)) || (hParent == NULL && mwtd->parent->childrenCount > 1)) {
								if (hParent == NULL) {
									MONITORINFO mi;
									HMONITOR hMonitor;
									RECT rc, rcDesktop;
									newData.hContact = hContact;
									hParent = (HWND)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGWIN), NULL, DlgProcParentWindow, (LPARAM) & newData);
									GetWindowRect(hParent, &rc);
									rc.right = (rc.right - rc.left);
									rc.bottom = (rc.bottom - rc.top);
									rc.left = pt.x - rc.right / 2;
									rc.top = pt.y - rc.bottom / 2;
									hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
									mi.cbSize = sizeof(mi);
									GetMonitorInfo(hMonitor, &mi);
									rcDesktop = mi.rcWork;
									if (rc.left < rcDesktop.left) {
										rc.left = rcDesktop.left;
									}
									if (rc.top < rcDesktop.top) {
										rc.top = rcDesktop.top;
									}
									MoveWindow(hParent, rc.left, rc.top, rc.right, rc.bottom, FALSE);

								}
								SetParent(hChild, hParent);
								SendMessage(GetParent(hwnd), CM_REMOVECHILD, 0, (LPARAM) hChild);
								SendMessage(hChild, DM_SETPARENT, 0, (LPARAM) hParent);
								SendMessage(hParent, CM_ADDCHILD, (WPARAM)hChild, (LPARAM) hContact);
								SendMessage(hParent, CM_ACTIVATECHILD, 0, (LPARAM) hChild);
								NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_CLOSING);
								NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_CLOSE);
								NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_OPENING);
								NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_OPEN);
								ShowWindow(hParent, SW_SHOWNA);
							}
						}
					}
				} else if (dat->lastClickTab >= 0 && g_dat->flags2 & SMF2_TABCLOSEBUTTON) {
					IMAGEINFO info;
					POINT pt;
					RECT rect;
					int atTop = (GetWindowLong(hwnd, GWL_STYLE) & TCS_BOTTOM) == 0;
					TabCtrl_GetItemRect(hwnd, dat->lastClickTab, &rect);
					pt.x = (lParam<<16)>>16;
					pt.y = lParam>>16;
					ImageList_GetImageInfo(g_dat->hButtonIconList, 0, &info);
					rect.left = rect.right - (info.rcImage.right - info.rcImage.left) - 6;
					if (!atTop) {
						rect.top = rect.bottom - (info.rcImage.bottom - info.rcImage.top);
					}
					if (pt.x >= rect.left && pt.x < rect.left + (info.rcImage.right - info.rcImage.left) && pt.y >= rect.top && pt.y < rect.top + (info.rcImage.bottom - info.rcImage.top)) {
						HBITMAP hOldBitmap, hBmp;
						COLORREF color1, color2;
						HDC hdc = GetDC(NULL);
						HDC hdcMem = CreateCompatibleDC(hdc);
						pt.x -= rect.left;
						pt.y -= rect.top;
						hBmp = CreateCompatibleBitmap(hdc, info.rcImage.right - info.rcImage.left + 1, info.rcImage.bottom - info.rcImage.top + 1);
						hOldBitmap = SelectObject(hdcMem, hBmp);
						SetPixel(hdcMem, pt.x, pt.y, 0x000000);
						ImageList_DrawEx(g_dat->hButtonIconList, 0, hdcMem, 0, 0, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
						color1 = GetPixel(hdcMem, pt.x, pt.y);
						SetPixel(hdcMem, pt.x, pt.y, 0xFFFFFF);
						ImageList_DrawEx(g_dat->hButtonIconList, 0, hdcMem, 0, 0, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
						color2 = GetPixel(hdcMem, pt.x, pt.y);
						SelectObject(hdcMem, hOldBitmap);
						DeleteDC(hdcMem);
						DeleteObject(hBmp);
						ReleaseDC(NULL, hdc);
						if (color1 != 0x000000 || color2 != 0xFFFFFF) {
							SendMessage(GetChildFromTab(hwnd, dat->lastClickTab)->hwnd, WM_CLOSE, 0, 0);
						}
					} else {
						SendMessage(hwnd, WM_LBUTTONDOWN, dat->clickWParam, dat->clickLParam);
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
			if (wParam & MK_LBUTTON) {
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
					return 0;
				}
			}
			break;
       	case EM_UNSUBCLASSED:
			mir_free(dat);
			return 0;
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
	} else {
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

HWND GetParentWindow(HANDLE hContact, BOOL bChat) {
	NewMessageWindowLParam newData = { 0 };
	newData.hContact = hContact;
	newData.isChat = bChat;
	if (g_dat->flags2 & SMF2_USETABS) {
		if (!bChat || !(g_dat->flags2 & SMF2_SEPARATECHATSCONTAINERS)) {
			if (g_dat->lastParent != NULL) {
				DWORD tabsNum = (DWORD) SendMessage(g_dat->lastParent->hwnd, CM_GETCHILDCOUNT, 0, 0);
				if (!(g_dat->flags2 & SMF2_LIMITTABS) || tabsNum < g_dat->limitTabsNum) {
					return g_dat->lastParent->hwnd;
				}
			}
		} else {
			if (g_dat->lastChatParent != NULL) {
				DWORD tabsNum = (DWORD) SendMessage(g_dat->lastChatParent->hwnd, CM_GETCHILDCOUNT, 0, 0);
				if (!(g_dat->flags2 & SMF2_LIMITCHATSTABS) || tabsNum < g_dat->limitChatsTabsNum) {
					return g_dat->lastChatParent->hwnd;
				}
			}
		}
	}
	if (!(g_dat->flags2 & SMF2_SEPARATECHATSCONTAINERS)) {
		newData.isChat =FALSE;
	}
	return CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGWIN), NULL, DlgProcParentWindow, (LPARAM) & newData);
}
