#include "commonheaders.h"

extern HINSTANCE g_hInst;

#define SB_CHAR_WIDTH		 45
#define SB_TYPING_WIDTH 	 35

#define TIMERID_FLASHWND     1
#define TIMEOUT_FLASHWND     900

static WNDPROC OldTabCtrlProc;
BOOL CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void GetChildWindowRect(struct ParentWindowData *dat, RECT *rcChild)
{
	RECT rc, rcStatus, rcTabs;
	GetClientRect(dat->hwnd, &rc);
	GetClientRect(dat->hwndTabs, &rcTabs);
	TabCtrl_AdjustRect(dat->hwndTabs, FALSE, &rcTabs);
	rcStatus.top = rcStatus.bottom = 0;
	if (g_dat->flags & SMF_SHOWSTATUSBAR) {
		GetWindowRect(dat->hwndStatus, &rcStatus);
	}
	rcChild->left = 0;
	rcChild->right = rc.right;
	if (g_dat->flags & SMF_TABSATBOTTOM) {
		rcChild->top = 2;
		if (dat->childrenCount > 1) {
			rcChild->bottom = rcTabs.bottom + 4;
		} else {
			rcChild->bottom = rc.bottom - rc.top - (rcStatus.bottom - rcStatus.top);
		}
	} else {
		if (dat->childrenCount > 1) {
			rcChild->top = rcTabs.top;
		} else {
			rcChild->top = 2;//rcTabs.top - 2;
		}
		rcChild->bottom = rc.bottom - rc.top - (rcStatus.bottom - rcStatus.top);
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

static void AddChild(struct ParentWindowData *dat, struct MessageWindowData * mdat) 
{
	char *contactName;
	TCITEM tci;
	int tabId;

	dat->children=(HWND*)realloc(dat->children, sizeof(HWND)*(dat->childrenCount+1));
	dat->children[dat->childrenCount++] = mdat->hwnd;
	contactName = Translate("Unknown");
	
	if (mdat->hContact) {
		contactName = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) mdat->hContact, 0);
	}
	tci.mask = TCIF_TEXT | TCIF_PARAM;
#if defined ( _UNICODE )
	{
		TCHAR wtext[256];
		MultiByteToWideChar(CP_ACP, 0, contactName, -1, wtext, 256);
		tci.pszText = wtext;
	}
#else
	tci.pszText = contactName;
#endif
	tci.lParam = (LPARAM) mdat;
	tabId = TabCtrl_InsertItem(dat->hwndTabs, dat->childrenCount-1, &tci);
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
				dat->nFlash = 0;
				dat->nFlashMax = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT);
				dat->childrenCount = 0;
				dat->children = NULL;
				dat->hwnd = hwndDlg;
				dat->hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hwndDlg, NULL, g_hInst, NULL);
				{
					int statwidths[3];
					RECT rc;
					SendMessage(dat->hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);
					GetWindowRect(dat->hwndStatus, &rc);
					statwidths[0] = rc.right - rc.left - SB_CHAR_WIDTH - SB_TYPING_WIDTH;
					statwidths[1] = rc.right - rc.left - SB_TYPING_WIDTH; //rc.right - rc.left - SB_CHAR_WIDTH;
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
				ws = GetWindowLong(dat->hwndTabs, GWL_STYLE);
				if (g_dat->flags & SMF_TABSATBOTTOM) {
					ws |= TCS_BOTTOM;
				}
				SetWindowLong(dat->hwndTabs, GWL_STYLE, ws);
				SetWindowPos(dat->hwndTabs, 0, 0, -10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				if (!(g_dat->flags & SMF_SHOWSTATUSBAR)) {
					ShowWindow(dat->hwndStatus, SW_HIDE);
				}
				ShowWindow(hwndDlg, SW_HIDE);
			}
			return TRUE;

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *) lParam;
			RECT rc, rcWindow;
			int i, minW = 240, minH = 80;
			GetWindowRect(hwndDlg, &rcWindow);
			GetChildWindowRect(dat, &rc);
			for (i=0;i<dat->childrenCount;i++) {
				SendMessage(dat->children[i], WM_GETMINMAXINFO, 0, lParam);
				if (i==0 || mmi->ptMinTrackSize.x > minW) minW = mmi->ptMinTrackSize.x;
				if (i==0 || mmi->ptMinTrackSize.y > minH) minH = mmi->ptMinTrackSize.y;
			}
			mmi->ptMinTrackSize.x = minW + (rcWindow.right - rcWindow.left) - (rc.right - rc.left);
			mmi->ptMinTrackSize.y = minH + (rcWindow.bottom - rcWindow.top) - (rc.bottom - rc.top);
			return FALSE;
		}

		case WM_SIZE:
			if (IsIconic(hwndDlg) || wParam == SIZE_MINIMIZED)
				break;
			{
				int i;
				RECT rc, rcStatus, rcChild, rcWindow;
				GetClientRect(hwndDlg, &rc);
				rcStatus.top = rcStatus.bottom = 0;
				if (g_dat->flags & SMF_SHOWSTATUSBAR) {
					int statwidths[3];
					GetWindowRect(dat->hwndStatus, &rcStatus);
					statwidths[0] = rc.right - rc.left - SB_CHAR_WIDTH - SB_TYPING_WIDTH;
					statwidths[1] = rc.right - rc.left - SB_TYPING_WIDTH; 
					statwidths[2] = -1;
					SendMessage(dat->hwndStatus, SB_SETPARTS, 3, (LPARAM) statwidths);
					SendMessage(dat->hwndStatus, WM_SIZE, 0, 0);
				}
				MoveWindow(dat->hwndTabs, 0, 2, (rc.right - rc.left), (rc.bottom - rc.top) - (rcStatus.bottom - rcStatus.top) - 2,	FALSE); 
				RedrawWindow(dat->hwndTabs, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_ERASE);
				GetChildWindowRect(dat, &rcChild);
				if ((rcChild.bottom-rcChild.top) < 81 || (rcChild.right-rcChild.left) < 240) {
					GetWindowRect(hwndDlg, &rcWindow);
					if ((rcChild.bottom-rcChild.top) < 81) {
						rcWindow.bottom = rcWindow.top + 81 + (rcWindow.bottom - rcWindow.top) - (rcChild.bottom - rcChild.top);
					} 
					if ((rcChild.right-rcChild.left) < 240) {
						rcWindow.right = rcWindow.left + 240;
					}
					MoveWindow(hwndDlg, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top,	TRUE); 
					GetChildWindowRect(dat, &rcChild);
				}
				for (i=0;i<dat->childrenCount;i++) {
					if (dat->children[i] == dat->hwndActive) {
						MoveWindow(dat->children[i], rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, TRUE);
						RedrawWindow(GetDlgItem(dat->children[i], IDC_LOG), NULL, NULL, RDW_INVALIDATE);
					} 
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
			
		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_ACTIVE)
				break;
		case WM_MOUSEACTIVATE:
			if (KillTimer(hwndDlg, TIMERID_FLASHWND)) {
				FlashWindow(hwndDlg, FALSE);
				dat->nFlash = 0;
			}
			SendMessage(dat->hwndActive, WM_ACTIVATE, WA_ACTIVE, 0);
			break;
		case WM_DESTROY:
			g_dat->hParent = NULL;
			SetWindowLong(hwndDlg, GWL_USERDATA, 0);
			WindowList_Remove(g_dat->hParentWindowList, hwndDlg);
			if (dat->children!=NULL) free (dat->children);
			free(dat);
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
		case DM_UPDATETITLE:
		{
			struct MessageWindowData * mdat = (struct MessageWindowData *) lParam;
			char newtitle[256], oldtitle[256];
			char *szStatus, *contactName, *pszNewTitleEnd;
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
							if (mdat->showTyping && (g_dat->flags&SMF_SHOWTYPINGWIN)) {
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
		case DM_OPTIONSAPPLIED:
		{
			if (!(g_dat->flags & SMF_SHOWSTATUSBAR)) {
				ShowWindow(dat->hwndStatus, SW_HIDE);
			} else {
				ShowWindow(dat->hwndStatus, SW_SHOW);
			}
			ws = GetWindowLong(dat->hwndTabs, GWL_STYLE) & ~(TCS_BOTTOM);
			if (g_dat->flags & SMF_TABSATBOTTOM) {
				ws |= TCS_BOTTOM;
			} 
			SetWindowLong(dat->hwndTabs, GWL_STYLE, ws);
			RedrawWindow(dat->hwndTabs, NULL, NULL, RDW_INVALIDATE);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			break;
		}


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

