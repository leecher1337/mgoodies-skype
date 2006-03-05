// ========= HISTORY STUFF ==========
// taken from last seen plugin

#include "main.h"

char* BuildSetting(historyLast) {
	static char setting[15];
	static char sztemp[15];
	*setting = '\0';
	strcat(setting, "History_");
	strcat(setting, itoa(historyLast, sztemp, 10));
	return setting;
}

static void LoadHistoryList(HANDLE hContact, HWND hwnd, int nList) {
	short historyFirst, historyLast, historyMax;
	short i;
	TCHAR *str, *newstr, tempstr[2048];
	DBVARIANT dbv;
	str = (TCHAR*)malloc(2 * sizeof(TCHAR));
	lstrcpy(str, "");
//	SendDlgItemMessage(hwnd, nList, LB_RESETCONTENT, 0, 0);
	historyMax = (int)options.dHistMax;
	if (historyMax < 0) historyMax = 0; 
	else if (historyMax > 99) historyMax = 99;
	if (historyMax == 0) return;
	historyFirst = DBGetContactSettingWord(hContact, MODULE, "HistoryFirst",0);
	if (historyFirst >=  historyMax) historyFirst = 0;
	historyLast = DBGetContactSettingWord(hContact, MODULE, "HistoryLast",0);
	if (historyLast >= historyMax) historyLast = historyMax - 1;
	// reading history
	i = historyLast;
	while (i != historyFirst) {
		i = (i - 1 + historyMax) % historyMax;
		dbv.type = 0;
		lstrcpy(tempstr, !DBGetContactSetting(hContact, MODULE, BuildSetting(i), &dbv)?dbv.pszVal:"");
		newstr = (TCHAR*)malloc((lstrlen(str) + lstrlen(tempstr) + 50) * sizeof(TCHAR));
		lstrcpy(newstr, str);
		if (lstrlen(str) > 0) lstrcat(newstr, "\r\n");
		lstrcat(newstr, tempstr);
		free(str);
		str = newstr;
//		SendDlgItemMessage(hwnd, nList, LB_ADDSTRING, 0, (LPARAM)(!DBGetContactSetting(hContact,MODULE,BuildSetting(i),&dbv)?dbv.pszVal:""));
		if (dbv.type)
			DBFreeVariant(&dbv);
	}
	SetDlgItemText(hwnd, nList, str);
	free(str);
}

static void ClearHistory(HANDLE hContact) {
	int i;
	BOOL bSettingExists = TRUE;
	for (i = 0; i < (int)options.dHistMax && bSettingExists; i++) {
		bSettingExists = !DBDeleteContactSetting(hContact, MODULE, BuildSetting(i));
	}
	DBDeleteContactSetting(hContact, MODULE, "HistoryFirst");
	DBDeleteContactSetting(hContact, MODULE, "HistoryLast");
}

void ClearAllHistory() {
	HANDLE hContact;
	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) {
		ClearHistory(hContact);
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}

static HDWP MyResizeWindow (HDWP hDwp, HWND hwndDlg, HWND hwndControl, int nHorizontalOffset, int nVerticalOffset, int nWidthOffset, int nHeightOffset) {
	POINT pt;
	RECT rcinit;
	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);
	// get current top left point
	pt.x = rcinit.left;
	pt.y = rcinit.top;
	ScreenToClient(hwndDlg, &pt);
	return DeferWindowPos(hDwp, hwndControl, NULL,
			pt.x + nHorizontalOffset,
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left + nWidthOffset,
			rcinit.bottom - rcinit.top + nHeightOffset,
			SWP_NOZORDER);
}

static HDWP MyHorizCenterWindow (HDWP hDwp, HWND hwndDlg, HWND hwndControl, int nClientWidth, int nVerticalOffset, int nHeightOffset) {
	POINT pt;
	RECT rcinit;
	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);
	// get current top left point
	pt.x = rcinit.left;
	pt.y = rcinit.top;
	ScreenToClient(hwndDlg, &pt);
	return DeferWindowPos(hDwp, hwndControl, NULL,
			(int) ((nClientWidth - (rcinit.right - rcinit.left))/2),
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left,
			rcinit.bottom - rcinit.top + nHeightOffset,
			SWP_NOZORDER);
}

static void MyResizeGetOffset (HWND hwndDlg, HWND hwndControl, int nWidth, int nHeight, int* nDx, int* nDy) {
	RECT rcinit;
	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);
	// calculate offsets
	*nDx = nWidth - (rcinit.right - rcinit.left);
	*nDy = nHeight - (rcinit.bottom - rcinit.top);
}

static BOOL CALLBACK HistoryDlgProc(HWND hwndDlg, UINT Message, WPARAM wParam, LPARAM lParam) {
	HANDLE hContact;
	switch(Message) {
		case WM_INITDIALOG: {
				CONTACTINFO ci;
				DBVARIANT dbv;
				TCHAR sztemp[2048];
				char szProto[32];
				TranslateDialogDefault(hwndDlg);
				hContact = (HANDLE)lParam;
				lstrcpy(szProto, (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0));
				SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, STM_SETICON,
					(WPARAM)LoadSkinnedProtoIcon(szProto, DBGetContactSettingWord(hContact, szProto, "Status",0)), 0);
				ci.cbSize = sizeof(ci);
				ci.hContact = hContact;
				ci.dwFlag = CNF_UNIQUEID;
				ci.szProto = (char*)szProto;
				if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
					switch (ci.type) {
						case CNFT_ASCIIZ:
							SetDlgItemText(hwndDlg, IDC_STATUSMSG, (LPCTSTR)ci.pszVal);
							break;
						case CNFT_DWORD:
							ltoa(ci.dVal, sztemp, 10);
							SetDlgItemText(hwndDlg, IDC_STATUSMSG, (LPCTSTR)sztemp);
							break;
					}
				}
				else SetDlgItemText(hwndDlg, IDC_STATUSMSG, (LPCTSTR)(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0));
				SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
				lstrcpy(sztemp, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0));
				lstrcat(sztemp, ": ");
				lstrcat(sztemp, Translate("Status Message History"));
				SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)sztemp);
				SendMessage(hwndDlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_HIST)));
				SendMessage(hwndDlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_HIST)));
				// set buttons
				SendDlgItemMessage(hwndDlg, IDC_DETAILS,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_USERDETAILS),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0));
				// temporary work-around for message dialog icon
				if (DBGetContactSetting(NULL, "Icons", "100", &dbv))
					SendDlgItemMessage(hwndDlg,IDC_SENDMSG,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_RECVMSG),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0));
				else SendDlgItemMessage(hwndDlg,IDC_SENDMSG,BM_SETIMAGE,IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
				// end workaround
				SendDlgItemMessage(hwndDlg,IDC_HISTORY,BM_SETIMAGE,IMAGE_ICON, (LPARAM)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_HISTORY),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0));
				SendDlgItemMessage(hwndDlg,IDC_DI,BM_SETIMAGE,IMAGE_ICON, (LPARAM)ICO_INT);
				SendDlgItemMessage(hwndDlg,IDC_DE,BM_SETIMAGE,IMAGE_ICON, (LPARAM)ICO_EXT);
				SendDlgItemMessage(hwndDlg,IDC_DP,BM_SETIMAGE,IMAGE_ICON, (LPARAM)ICO_POPUP);
				SendDlgItemMessage(hwndDlg,IDC_USERMENU,BUTTONSETARROW,1,0);

				SendMessage(GetDlgItem(hwndDlg,IDC_DETAILS), BUTTONSETASFLATBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_SENDMSG), BUTTONSETASFLATBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_USERMENU), BUTTONSETASFLATBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_HISTORY), BUTTONSETASFLATBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DI), BUTTONSETASFLATBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DI), BUTTONSETASPUSHBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DE), BUTTONSETASFLATBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DE), BUTTONSETASPUSHBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DP), BUTTONSETASFLATBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DP), BUTTONSETASPUSHBTN, 0, 0);

				CheckDlgButton(hwndDlg, IDC_DI, DBGetContactSettingByte(hContact,MODULE,"Internal",TRUE));
				CheckDlgButton(hwndDlg, IDC_DE, DBGetContactSettingByte(hContact,MODULE,"External",TRUE));
				CheckDlgButton(hwndDlg, IDC_DP, DBGetContactSettingByte(hContact,MODULE,"Popup",TRUE));

				SendMessage(GetDlgItem(hwndDlg,IDC_USERMENU), BUTTONADDTOOLTIP, (WPARAM)Translate("User Menu"), 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DETAILS), BUTTONADDTOOLTIP, (WPARAM)Translate("View User's Details"), 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_HISTORY), BUTTONADDTOOLTIP, (WPARAM)Translate("View User's History"), 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_SENDMSG), BUTTONADDTOOLTIP, (WPARAM)Translate("Send Instant Message"), 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DI), BUTTONADDTOOLTIP, (WPARAM)Translate("Enable/Disable internal logging for this contact"), 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DE), BUTTONADDTOOLTIP, (WPARAM)Translate("Enable/Disable external logging for this contact"), 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DP), BUTTONADDTOOLTIP, (WPARAM)Translate("Enable/Disable popups for this contact"), 0);

				if (HasToGetStatusMsgForProtocol(szProto))
				{
					SendDlgItemMessage(hwndDlg,IDC_ICQCHECK,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadSkinnedProtoIcon(szProto,ID_STATUS_AWAY));
					SendMessage(GetDlgItem(hwndDlg,IDC_ICQCHECK), BUTTONSETASFLATBTN, 0, 0);
					SendMessage(GetDlgItem(hwndDlg,IDC_ICQCHECK), BUTTONSETASPUSHBTN, 0, 0);
					CheckDlgButton(hwndDlg, IDC_ICQCHECK, DBGetContactSettingByte(hContact,MODULE,OPT_CONTACT_GETMSG,DEFAULT_ICQCHECK));
					SendMessage(GetDlgItem(hwndDlg,IDC_ICQCHECK), BUTTONADDTOOLTIP, (WPARAM)Translate("Enable/Disable status message checking for this contact"), 0);
				}
				else ShowWindow(GetDlgItem(hwndDlg,IDC_ICQCHECK),SW_HIDE);

				Utils_RestoreWindowPositionNoMove(hwndDlg,NULL,MODULE,"History_");
				ShowWindow(hwndDlg, SW_SHOW);
			}
			break;
		case WM_MEASUREITEM:
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		case WM_DRAWITEM:
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam != GetDlgItem(hwndDlg, IDC_PROTOCOL)) {
				if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_HISTORYLIST)) {
					SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
					SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
					return (BOOL)GetSysColorBrush(COLOR_WINDOW);
				}
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (BOOL)GetStockObject(NULL_BRUSH);
			}
			return FALSE;
		case WM_COMMAND:
			hContact=(HANDLE)GetWindowLong(hwndDlg, GWL_USERDATA);
			if(CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)hContact))
				break;
			switch(LOWORD(wParam)) {
				case IDCANCEL:
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				case IDOK:
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				case IDC_CLEARHIST:
					ClearHistory(hContact);
					SetDlgItemText(hwndDlg, IDC_HISTORYLIST, "");
					break;
				case IDC_USERMENU: {
					RECT rc;
					HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)hContact, 0);
					GetWindowRect(GetDlgItem(hwndDlg, IDC_USERMENU), &rc);
					TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
					DestroyMenu(hMenu);
					break;
				}
				case IDC_DETAILS:
					CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)hContact, 0);
					break;
				case IDC_SENDMSG:
					CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
					break;
				case IDC_HISTORY:
					CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)hContact, 0);
					break;
				case IDC_DP:
					DBWriteContactSettingByte(hContact, MODULE, "Popup", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DP));
					break;
				case IDC_DI:
					DBWriteContactSettingByte(hContact, MODULE, "Internal", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DI));
					break;
				case IDC_DE:
					DBWriteContactSettingByte(hContact, MODULE, "External", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DE));
					break;
				case IDC_ICQCHECK:
					DBWriteContactSettingByte(hContact, MODULE, OPT_CONTACT_GETMSG, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ICQCHECK));
					break;
			}
			break;
		case WM_SIZE: {
				int dx, dy;
				HDWP hDwp;

				hDwp = BeginDeferWindowPos(12);
				MyResizeGetOffset(hwndDlg, GetDlgItem(hwndDlg, IDC_HISTORYLIST), LOWORD(lParam)-15, HIWORD(lParam)-64, &dx, &dy);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_USERMENU), dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_DETAILS), dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_HISTORY), dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_SENDMSG), dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_DE), dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_DI), dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_DP), dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_ICQCHECK), dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_HISTORYLIST), 0, 0, dx, dy);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_STATUSMSG), 0, 0, dx, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_CLEARHIST), 0, dy, 0, 0);
				hDwp = MyHorizCenterWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDOK), LOWORD(lParam), dy, 0);
				EndDeferWindowPos(hDwp);
			}
			break;
		case WM_GETMINMAXINFO: {
				MINMAXINFO mmi;
				CopyMemory (&mmi, (LPMINMAXINFO)lParam, sizeof(MINMAXINFO));
				/* The minimum width in points*/
				mmi.ptMinTrackSize.x = 400;
				/* The minimum height in points*/
				mmi.ptMinTrackSize.y = 190;
				CopyMemory ((LPMINMAXINFO)lParam, &mmi, sizeof(MINMAXINFO));
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			WindowList_Remove(hWindowList, hwndDlg);
			break;
		case WM_DESTROY: {
				HFONT hFont;
				hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_USERMENU, WM_GETFONT, 0, 0);
				DeleteObject(hFont);
				Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "History_");
			}
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

void ShowHistory(HANDLE hContact) {
	HWND hHistoryDlg;
	hHistoryDlg = WindowList_Find(hWindowList,hContact);
	if (hHistoryDlg == NULL) {
		hHistoryDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_HISTORY), NULL, HistoryDlgProc, (LPARAM)hContact);
		LoadHistoryList(hContact, hHistoryDlg, IDC_HISTORYLIST);
		WindowList_Add(hWindowList, hHistoryDlg, hContact);
	}
	else {
		SetForegroundWindow(hHistoryDlg);
		LoadHistoryList(hContact, hHistoryDlg, IDC_HISTORYLIST);
		SetFocus(hHistoryDlg);
	}
}
/*
void InitHistoryDialog(void) {
	hWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
}
*/
