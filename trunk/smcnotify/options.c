
#include "main.h"
void ClearAllHistory();

void OptionsRead() {
	DBVARIANT dbv;
	// popups
	options.bDisablePopUps = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_DISPOPUPS, FALSE);
	options.bShowOnConnect = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_SHOWONC, FALSE);
	options.bIgnoreEmptyPopup = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_IGNOREPOP, FALSE);
	options.bIgnoreEmptyAll = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_IGNOREALL, FALSE);
	options.bUseOSD = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_USEOSD, FALSE);
	options.bDefaultColor = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_COLDEFAULT, FALSE);
	options.colBack = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLBACK, DEFAULT_COLBACK);
	options.colText = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLTEXT, DEFAULT_COLTEXT);
	options.dSec = (DWORD)DBGetContactSettingDword(NULL, MODULE, OPT_DSEC, 0);
	options.bInfiniteDelay = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_DINFINITE, FALSE);
	options.LeftClickAction = DBGetContactSettingDword(NULL,MODULE, OPT_LCLKACT, IDM_M1);
	options.RightClickAction = DBGetContactSettingDword(NULL,MODULE, OPT_RCLKACT, IDM_M1);
	// general
	options.bHideSettingsMenu = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_HIDEMENU, FALSE);
	options.bLogToFile = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_LOGTOFILE, TRUE);
	options.dHistMax = (DWORD)DBGetContactSettingDword(NULL, MODULE, OPT_HISTMAX, 20);
	options.bShowMsgChanges = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_SHOWCH, FALSE);
	options.bUseBgImage = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_USEBGIMG, FALSE);
	options.colListBack = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLLISTBACK, DEFAULT_COLLISTBACK);
	options.colListText = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLLISTTEXT, DEFAULT_COLLISTTEXT);
	// icq
	options.bCheckIcqStatusMsgs = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_CHECKPROTOCOLMESSAGES, TRUE);
	options.bCheckIcqStatusMsgsOnChange = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_CHECKPROTOCOLMESSAGESONCHANGE, TRUE);
	options.dCheckIcqTimer = (DWORD)DBGetContactSettingDword(NULL, MODULE, OPT_CHECKPROTOCOLMESSAGESTMR, DEFAULT_ICQTIMER);
	// strings
	// popup text
	if (DBGetContactSetting(NULL, MODULE, OPT_POPTXT, &dbv)) {
		TCHAR str[512];
		_stprintf(str, "%s%s", Translate("changes his/her status message to:"), "\r\n%n");
		lstrcpy(options.popuptext, str);
	}
	else {
		lstrcpyn(options.popuptext, dbv.pszVal, min(lstrlen(dbv.pszVal)+1, MAXPOPUPLEN));
		DBFreeVariant(&dbv);
	}
	// log file
	if (DBGetContactSetting(NULL, MODULE, OPT_LOGFILE, &dbv))
		lstrcpy(options.logfile, "");
	else {
		if (lstrcmp(dbv.pszVal, ""))
			CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)options.logfile);
		else lstrcpy(options.logfile, "");
		DBFreeVariant(&dbv);
	}
	// log to file format
	if (DBGetContactSetting(NULL, MODULE, OPT_LOG, &dbv))
		lstrcpy(options.log, "[%Y/%M/%D %h:%m %a | %c] %n");
	else {
		lstrcpyn(options.log, dbv.pszVal, min(lstrlen(dbv.pszVal)+1, MAXSTRLEN));
		DBFreeVariant(&dbv);
	}
	// log to history format
	if (DBGetContactSetting(NULL, MODULE, OPT_HISTORY, &dbv))
		lstrcpy(options.his, "[%Y/%M/%D %h:%m %a] %n");
	else {
		lstrcpyn(options.his, dbv.pszVal, min(lstrlen(dbv.pszVal)+1, MAXSTRLEN));
		DBFreeVariant(&dbv);
	}
	// status message cleared notification
	if (DBGetContactSetting(NULL, MODULE, OPT_MSGCLRNTF, &dbv)) {
		lstrcpy(options.msgcleared, Translate("cleares his/her status message"));
	}
	else {
		lstrcpyn(options.msgcleared, dbv.pszVal, min(lstrlen(dbv.pszVal)+1, MAXSTRLEN));
		DBFreeVariant(&dbv);
	}
	// status message changed notofication
	if (DBGetContactSetting(NULL, MODULE, OPT_MSGCHNNTF, &dbv)) {
		TCHAR str[512];
		_stprintf(str, "%s%s", Translate("changes his/her status message:"), "\\n[%n]");
		lstrcpy(options.msgchanged, str);
	}
	else {
		lstrcpyn(options.msgchanged, dbv.pszVal, min(lstrlen(dbv.pszVal)+1, MAXSTRLEN));
		DBFreeVariant(&dbv);
	}
	// list background image filepath
	if (DBGetContactSetting(NULL, MODULE, OPT_LISTBG, &dbv))
		lstrcpy(options.listbgimage, "");
	else {
		if (lstrcmp(dbv.pszVal, ""))
			CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)options.listbgimage);
		else lstrcpy(options.listbgimage, "");
		DBFreeVariant(&dbv);
	}
}

static void OptionsWrite() {
	char szPath[MAX_PATH];
	// popups
	DBWriteContactSettingByte(NULL, MODULE, OPT_DISPOPUPS, (BYTE)options.bDisablePopUps);
	DBWriteContactSettingByte(NULL, MODULE, OPT_SHOWONC, (BYTE)options.bShowOnConnect);
	DBWriteContactSettingByte(NULL, MODULE, OPT_IGNOREPOP, (BYTE)options.bIgnoreEmptyPopup);
	DBWriteContactSettingByte(NULL, MODULE, OPT_IGNOREALL, (BYTE)options.bIgnoreEmptyAll);
	DBWriteContactSettingByte(NULL, MODULE, OPT_USEOSD, (BYTE)options.bUseOSD);
	DBWriteContactSettingByte(NULL, MODULE, OPT_COLDEFAULT, (BYTE)options.bDefaultColor);
	DBWriteContactSettingDword(NULL, MODULE, OPT_COLBACK, (DWORD)options.colBack);
	DBWriteContactSettingDword(NULL, MODULE, OPT_COLTEXT, (DWORD)options.colText);
	DBWriteContactSettingDword(NULL, MODULE, OPT_DSEC, (DWORD)options.dSec);
	DBWriteContactSettingByte(NULL, MODULE, OPT_DINFINITE, (BYTE)options.bInfiniteDelay);
	DBWriteContactSettingDword(NULL, MODULE, OPT_LCLKACT, options.LeftClickAction);
	DBWriteContactSettingDword(NULL, MODULE, OPT_RCLKACT, options.RightClickAction);
	// general
	DBWriteContactSettingByte(NULL, MODULE, OPT_HIDEMENU, (BYTE)options.bHideSettingsMenu);
	DBWriteContactSettingByte(NULL, MODULE, OPT_LOGTOFILE, (BYTE)options.bLogToFile);
	DBWriteContactSettingDword(NULL, MODULE, OPT_HISTMAX, (DWORD)options.dHistMax);
	DBWriteContactSettingByte(NULL, MODULE, OPT_SHOWCH, (BYTE)options.bShowMsgChanges);
	DBWriteContactSettingByte(NULL, MODULE, OPT_USEBGIMG, (BYTE)options.bUseBgImage);
	DBWriteContactSettingDword(NULL, MODULE, OPT_COLLISTBACK, (DWORD)options.colListBack);
	DBWriteContactSettingDword(NULL, MODULE, OPT_COLLISTTEXT, (DWORD)options.colListText);
	// icq
	DBWriteContactSettingByte(NULL, MODULE, OPT_CHECKPROTOCOLMESSAGES, (BYTE)options.bCheckIcqStatusMsgs);
	DBWriteContactSettingByte(NULL, MODULE, OPT_CHECKPROTOCOLMESSAGESONCHANGE, (BYTE)options.bCheckIcqStatusMsgsOnChange);
	DBWriteContactSettingDword(NULL, MODULE, OPT_CHECKPROTOCOLMESSAGESTMR, (DWORD)options.dCheckIcqTimer);
	// strings
	DBWriteContactSettingString(NULL, MODULE, OPT_POPTXT, options.popuptext);
	if (strcmp(options.logfile, ""))
		CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)options.logfile, (LPARAM)szPath);
	else strcpy(szPath, "");
	DBWriteContactSettingString(NULL, MODULE, OPT_LOGFILE, szPath);
	DBWriteContactSettingString(NULL, MODULE, OPT_LOG, options.log);
	DBWriteContactSettingString(NULL, MODULE, OPT_HISTORY, options.his);
	DBWriteContactSettingString(NULL, MODULE, OPT_MSGCLRNTF, options.msgcleared);
	DBWriteContactSettingString(NULL, MODULE, OPT_MSGCHNNTF, options.msgchanged);
	if (strcmp(options.listbgimage, ""))
		CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)options.listbgimage, (LPARAM)szPath);
	else strcpy(szPath, "");
	DBWriteContactSettingString(NULL, MODULE, OPT_LISTBG, szPath);
}

static void UpdateOption(HWND hwndDlg) {
	options.bDisablePopUps = IsDlgButtonChecked(hwndDlg, IDC_CHKDISABLE);
	options.bShowOnConnect = IsDlgButtonChecked(hwndDlg, IDC_SHOWONC);
	options.bIgnoreEmptyPopup = IsDlgButtonChecked(hwndDlg, IDC_IGNOREPOP);
	options.bUseOSD = IsDlgButtonChecked(hwndDlg, IDC_USEOSD);
	options.bDefaultColor = IsDlgButtonChecked(hwndDlg, IDC_CHKDEFAULTCOL);
	options.colBack = SendDlgItemMessage(hwndDlg, IDC_COLBACK, CPM_GETCOLOUR, 0, 0);
	options.colText = SendDlgItemMessage(hwndDlg, IDC_COLTEXT, CPM_GETCOLOUR, 0, 0);
	options.dSec = (DWORD)GetDlgItemInt(hwndDlg, IDC_DELAY, NULL, FALSE);
	options.bInfiniteDelay = IsDlgButtonChecked(hwndDlg, IDC_DELAYINFINITE);
	GetDlgItemText(hwndDlg, IDC_POPUPTEXT, options.popuptext, sizeof(options.popuptext));
}

// used to select the menu item for popup action menu (mark selected menu item ??)
static void SelectMenuItem(HMENU hMenu, int Check) {
	int i;
	for (i = 0; i <= GetMenuItemCount(hMenu) - 1; i++)
		CheckMenuItem(hMenu, i, MF_BYPOSITION | ((int)GetMenuItemID(hMenu, i) == Check)*8);
}

// popup options page
static BOOL CALLBACK PopupsOptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	char str[255];
	int ID;
	HMENU hMenu;
	RECT pos;
	HWND button;
	switch (message) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			SetDlgItemInt(hwndDlg, IDC_DELAY, (int)options.dSec, FALSE);
			CheckDlgButton(hwndDlg, IDC_DELAYINFINITE, options.bInfiniteDelay?BST_CHECKED:BST_UNCHECKED);
			SendMessage(hwndDlg, WM_USER + 12, 0, 0);
			//make dialog represent the current options
			SendDlgItemMessage(hwndDlg, IDC_COLBACK, CPM_SETCOLOUR, 0, options.colBack);
			SendDlgItemMessage(hwndDlg, IDC_COLTEXT, CPM_SETCOLOUR, 0, options.colText);
			SetDlgItemText(hwndDlg, IDC_POPUPTEXT, options.popuptext);
			CheckDlgButton(hwndDlg, IDC_CHKDEFAULTCOL, options.bDefaultColor?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHKDISABLE, options.bDisablePopUps?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWONC, options.bShowOnConnect?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_IGNOREPOP, options.bIgnoreEmptyPopup?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_USEOSD, options.bUseOSD?BST_CHECKED:BST_UNCHECKED);
			//disable color picker when using default colors
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLBACK), !options.bDefaultColor);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLTEXT), !options.bDefaultColor);
			//disable non-OSD controls
			SendMessage(hwndDlg, WM_USER + 11, 0, 0);
			// click actions
			hMenu = GetSubMenu(LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU)), 0);
			CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)hMenu,0);
			GetMenuString(hMenu, options.LeftClickAction, str, sizeof(str), MF_BYCOMMAND);
			SetDlgItemText(hwndDlg, IDC_LeftClick, str);
			GetMenuString(hMenu, options.RightClickAction, str, sizeof(str), MF_BYCOMMAND);
			SetDlgItemText(hwndDlg, IDC_RightClick, str);
			// set buttons flat
			SendMessage(GetDlgItem(hwndDlg,IDC_LeftClick), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg,IDC_RightClick), BUTTONSETASFLATBTN, 0, 0);
			return TRUE;
		case WM_USER + 10:
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLBACK), !IsDlgButtonChecked(hwndDlg, IDC_CHKDEFAULTCOL));
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLTEXT), !IsDlgButtonChecked(hwndDlg, IDC_CHKDEFAULTCOL));
			break;
		case WM_USER + 11: {
			BOOL state = !IsDlgButtonChecked(hwndDlg, IDC_USEOSD);
			if (state) SendMessage(hwndDlg, WM_USER + 10, 0, 0);
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLBACK), state);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLTEXT), state);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKDEFAULTCOL), state);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LeftClick), state);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RightClick), state);
			break;
		}
		case WM_USER + 12:
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), !IsDlgButtonChecked(hwndDlg, IDC_DELAYINFINITE));
			break;
		case WM_COMMAND:
			if ((LOWORD(wParam) == IDC_POPUPTEXT || LOWORD(wParam) == IDC_DELAY)
				&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
				return 0;
			switch (LOWORD(wParam)) {
				case IDC_RightClick:
					// right click action selection menu
					button = GetDlgItem(hwndDlg, IDC_RightClick);
					GetWindowRect(button, &pos); 
					hMenu = GetSubMenu(LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU)), 0);
					CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)hMenu,0);
					SelectMenuItem(hMenu, options.RightClickAction);
					ID = TrackPopupMenu(hMenu, TPM_LEFTBUTTON|TPM_RETURNCMD, pos.left, pos.bottom, 0, hwndDlg, NULL);
					if (ID) options.RightClickAction = ID;
					GetMenuString(hMenu, options.RightClickAction, str, sizeof(str), MF_BYCOMMAND);
					SetDlgItemText(hwndDlg,IDC_RightClick, str);
					break;
				case IDC_LeftClick:
					// left click action selection menu
					button = GetDlgItem(hwndDlg, IDC_LeftClick);
					GetWindowRect(button, &pos);
					hMenu = GetSubMenu(LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU)), 0);
					CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)hMenu,0);
					SelectMenuItem(hMenu, options.LeftClickAction);
					ID = TrackPopupMenu(hMenu, TPM_LEFTBUTTON|TPM_RETURNCMD, pos.left, pos.bottom, 0, hwndDlg, NULL);
					if (ID) options.LeftClickAction = ID;
					GetMenuString(hMenu, options.LeftClickAction, str, sizeof(str), MF_BYCOMMAND);
					SetDlgItemText(hwndDlg,IDC_LeftClick, str);
					break;
				case IDC_PREVIEW: {
					STATUSMSGINFO n;
					n.hContact = NULL;
					n.cust = malloc(512);
					lstrcpy(n.cust, Translate("Contact"));
					n.oldstatusmsg = malloc(512);
					lstrcpy(n.oldstatusmsg, Translate("Old status message"));
					n.newstatusmsg = malloc(512);
					lstrcpy(n.newstatusmsg, Translate("New status message"));
					UpdateOption(hwndDlg);
					ShowPopup(n);
					OptionsRead();
					if (n.cust) free(n.cust);
					if (n.oldstatusmsg) free(n.oldstatusmsg);
					if (n.newstatusmsg) free(n.newstatusmsg);
					return 0;
				}
				case IDC_CHKDEFAULTCOL:
					SendMessage(hwndDlg, WM_USER + 10, 0, 0);
					break;
				case IDC_USEOSD:
					SendMessage(hwndDlg, WM_USER + 11, 0, 0);
					break;
				case IDC_DELAYINFINITE:
					SendMessage(hwndDlg, WM_USER + 12, 0, 0);
					break;
			}
			//enable "Apply" button
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code) {
				case PSN_APPLY:
					UpdateOption(hwndDlg);
					//send changes to menuitem
					UpdateMenu(options.bDisablePopUps);
					OptionsWrite();
				}
			break;
		default:
			break;
	}
	return FALSE;
}

static BOOL CALLBACK StatusOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			SetDlgItemInt(hwndDlg, IDC_HISMAX, options.dHistMax, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COLLISTBACK, CPM_SETCOLOUR, 0, options.colListBack);
			SendDlgItemMessage(hwndDlg, IDC_COLLISTTEXT, CPM_SETCOLOUR, 0, options.colListText);
			SetDlgItemText(hwndDlg, IDC_IMGFILENAME, options.listbgimage);
			CheckDlgButton(hwndDlg, IDC_USEBGIMG, options.bUseBgImage?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOGTOFILE, options.bLogToFile?BST_CHECKED:BST_UNCHECKED);
			SetDlgItemText(hwndDlg, IDC_LOGTO, options.logfile);
			SetDlgItemText(hwndDlg, IDC_FHIS, options.his);
			SetDlgItemText(hwndDlg, IDC_FLOG, options.log);
			SetDlgItemText(hwndDlg, IDC_LOGMSGCL, options.msgcleared);
			SetDlgItemText(hwndDlg, IDC_LOGMSGCH, options.msgchanged);
			CheckDlgButton(hwndDlg, IDC_IGNOREALL, options.bIgnoreEmptyAll?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWCH, options.bShowMsgChanges?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_HIDECMENUITEMS, options.bHideSettingsMenu?BST_CHECKED:BST_UNCHECKED);
			SendMessage(hwndDlg, WM_USER + 10, 0, 0);
			SendMessage(hwndDlg, WM_USER + 11, 0, 0);
			return TRUE;
		case WM_USER + 10: {
			BOOL bState = IsDlgButtonChecked(hwndDlg, IDC_USEBGIMG);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLLISTBACK), !bState);
			EnableWindow(GetDlgItem(hwndDlg, IDC_IMGFILENAME), bState);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), bState);
			break;
		}
		case WM_USER + 11: {
			BOOL bState = IsDlgButtonChecked(hwndDlg, IDC_LOGTOFILE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGTO), bState);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FLOG), bState);
			break;
		}
		case WM_COMMAND:
			if ((LOWORD(wParam) == IDC_IMGFILENAME
				|| LOWORD(wParam) == IDC_LOGTO
				|| LOWORD(wParam) == IDC_FLOG
				|| LOWORD(wParam) == IDC_FHIS
				|| LOWORD(wParam) == IDC_HISMAX
				|| LOWORD(wParam) == IDC_LOGMSGCL
				|| LOWORD(wParam) == IDC_LOGMSGCH)
				&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
				return 0;
			switch (LOWORD(wParam))
			{
				case IDC_USEBGIMG:
					SendMessage(hwndDlg, WM_USER + 10, 0, 0);
					break;
				case IDC_LOGTOFILE:
					SendMessage(hwndDlg, WM_USER + 11, 0, 0);
					break;
				case IDC_CLEARALLHIS:
					ClearAllHistory();
					break;
				case IDC_BROWSE:
				{
					char str[MAX_PATH];
					OPENFILENAME ofn;
					char filter[512];
					GetDlgItemText(hwndDlg, IDC_IMGFILENAME, str, sizeof(str));
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);//OPENFILENAME_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					CallService(MS_UTILS_GETBITMAPFILTERSTRINGS, sizeof(filter), (LPARAM)filter);
					ofn.lpstrFilter = filter;
					ofn.lpstrFile = str;
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
					ofn.nMaxFile = sizeof(str);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "bmp";
					if (!GetOpenFileName(&ofn))
						break;
					SetDlgItemText(hwndDlg, IDC_IMGFILENAME, str);
				}
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR) lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR) lParam)->code) {
						case PSN_APPLY: {
							options.dHistMax = GetDlgItemInt(hwndDlg, IDC_HISMAX, NULL, FALSE);
							options.colListBack = SendDlgItemMessage(hwndDlg, IDC_COLLISTBACK, CPM_GETCOLOUR, 0, 0);
							options.colListText = SendDlgItemMessage(hwndDlg, IDC_COLLISTTEXT, CPM_GETCOLOUR, 0, 0);
							GetDlgItemText(hwndDlg, IDC_IMGFILENAME, options.listbgimage, sizeof(options.listbgimage));
							options.bUseBgImage = IsDlgButtonChecked(hwndDlg, IDC_USEBGIMG);
							GetDlgItemText(hwndDlg, IDC_LOGTO, options.logfile, sizeof(options.logfile));
							GetDlgItemText(hwndDlg, IDC_FHIS, options.his, sizeof(options.his));
							GetDlgItemText(hwndDlg, IDC_FLOG, options.log, sizeof(options.log));
							GetDlgItemText(hwndDlg, IDC_LOGMSGCL, options.msgcleared, sizeof(options.msgcleared));
							GetDlgItemText(hwndDlg, IDC_LOGMSGCH, options.msgchanged, sizeof(options.msgchanged));
							options.bIgnoreEmptyAll = IsDlgButtonChecked(hwndDlg, IDC_IGNOREALL);
							options.bShowMsgChanges = IsDlgButtonChecked(hwndDlg, IDC_SHOWCH);
							options.bHideSettingsMenu = IsDlgButtonChecked(hwndDlg, IDC_HIDECMENUITEMS);
							options.bLogToFile = IsDlgButtonChecked(hwndDlg, IDC_LOGTOFILE);
							OptionsWrite();
							return TRUE;
						}
					}
					break;
			}
			break;
	}
	return FALSE;
}




typedef struct tagProtocolData 
{
	char setting[64];
	BOOL show;
} ProtocolData;

static BOOL dialoginit = TRUE;

static BOOL CALLBACK ProtocolsOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG:
			dialoginit = TRUE;
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_CHECKICQ, options.bCheckIcqStatusMsgs?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHECKICQSTATUSCHANGE, options.bCheckIcqStatusMsgsOnChange?BST_CHECKED:BST_UNCHECKED);
			SendMessage(hwndDlg, WM_USER + 11, 0, 0);
			SetDlgItemInt(hwndDlg, IDC_CHECKICQTIMER, options.dCheckIcqTimer, FALSE);
			// Fill list view
			{
				HWND hwndProtocols = GetDlgItem(hwndDlg, IDC_PROTOCOLS);
				LVCOLUMN lvc;
				LVITEM lvi;
				PROTOCOLDESCRIPTOR **protos;
				int i,count;
				char szName[64];
				ProtocolData *pd;

				ListView_SetExtendedListViewStyle(hwndProtocols, LVS_EX_CHECKBOXES);

				ZeroMemory(&lvc, sizeof(lvc));
				lvc.mask = LVCF_FMT;
				lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
				ListView_InsertColumn(hwndProtocols, 0, &lvc);

				ZeroMemory(&lvi, sizeof(lvi));
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.iSubItem = 0;
				lvi.iItem = 1000;

				CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

				for (i = 0; i < count; i++)
				{
					if (protos[i]->type != PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
						continue;

					CallProtoService(protos[i]->szName, PS_GETNAME, sizeof(szName), (LPARAM)szName);

					pd = (ProtocolData*)malloc(sizeof(ProtocolData));
					strncpy(pd->setting, protos[i]->szName, 55);
					pd->setting[55] = '\0';
					strcat(pd->setting, "CheckMsg");
					pd->show = (BOOL)DBGetContactSettingByte(NULL, MODULE, pd->setting, FALSE);

					lvi.lParam = (LPARAM)pd;
					lvi.pszText = Translate(szName);
					lvi.iItem = ListView_InsertItem(hwndProtocols, &lvi);
					ListView_SetItemState(hwndProtocols, lvi.iItem, INDEXTOSTATEIMAGEMASK((pd->show)?2:1), LVIS_STATEIMAGEMASK);
				}

				ListView_SetColumnWidth(hwndProtocols, 0, LVSCW_AUTOSIZE);
				ListView_Arrange(hwndProtocols, LVA_ALIGNLEFT | LVA_ALIGNTOP);
			}
			dialoginit = FALSE;
			return TRUE;
		case WM_USER + 11:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHECKICQTIMER), IsDlgButtonChecked(hwndDlg, IDC_CHECKICQ));
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_CHECKICQ)
				SendMessage(hwndDlg, WM_USER + 11, 0, 0);
			if (LOWORD(wParam) == IDC_CHECKICQTIMER
				&& (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
				return 0;
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			if (dialoginit) break;
			switch (((LPNMHDR)lParam)->idFrom)
			{
				case IDC_PROTOCOLS:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case LVN_ITEMCHANGED:
							{
								NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;

								if(IsWindowVisible(GetDlgItem(hwndDlg, IDC_PROTOCOLS)) && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK))
									SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
							break;
					}
					break;
				}
				case 0:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							BOOL startedTimerNow = FALSE;

							{
								LVITEM lvi = {0};
								ProtocolData *pd;
								HWND hwndProtocols = GetDlgItem(hwndDlg, IDC_PROTOCOLS);
								int i;

								lvi.mask = (UINT) LVIF_PARAM;

								for (i = 0; i < ListView_GetItemCount(hwndProtocols); i++)
								{
									lvi.iItem = i;
									ListView_GetItem(hwndProtocols, &lvi);

									pd = (ProtocolData *)lvi.lParam;
									//DBWriteContactSettingByte(NULL, MODULE, pd->setting, (BYTE)pd->show);
									DBWriteContactSettingByte(NULL, MODULE, pd->setting, (BYTE)(BOOL)ListView_GetCheckState(hwndProtocols, i));
								}
							}


							if (options.bCheckIcqStatusMsgs != (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHECKICQ)) 
							{
								options.bCheckIcqStatusMsgs = IsDlgButtonChecked(hwndDlg, IDC_CHECKICQ);

								if (options.bCheckIcqStatusMsgs)
								{
									startedTimerNow = TRUE;

									SetTimer(hTimerWindow, 1, TMR_ICQFIRSTCHECK, StatusMsgCheckTimerProc);
								}
								else // if (!options.bCheckIcqStatusMsgs)
								{
									KillTimer(hTimerWindow, 1);
								}
							}

							options.bCheckIcqStatusMsgsOnChange = IsDlgButtonChecked(hwndDlg, IDC_CHECKICQSTATUSCHANGE);

							{
								DWORD timer = GetDlgItemInt(hwndDlg, IDC_CHECKICQTIMER, NULL, FALSE);
								if (options.bCheckIcqStatusMsgs && !startedTimerNow && options.dCheckIcqTimer > timer)
								{
									SetTimer(hTimerWindow, 1, timer*60000, StatusMsgCheckTimerProc);
								}
								options.dCheckIcqTimer = timer;
							}

							if (options.dCheckIcqTimer < 1) options.dCheckIcqTimer = 1;
							OptionsWrite();
							return TRUE;
						}
					}
					break;
				}
			}
			break;
/*		case WM_DESTROY:
		{
			LVITEM lvi;
			int i;
			HWND hwnd = GetDlgItem(hwndDlg,IDC_PROTOCOLS);

			ZeroMemory(&lvi, sizeof(lvi));
			lvi.mask = TVIF_PARAM;

			for (i = 0; i < ListView_GetItemCount(hwnd); i++)
			{
				lvi.iItem = i;
				ListView_GetItem(hwnd, &lvi);

				free((char *)lvi.lParam);
			}
			break;
		}
*/	}
	return FALSE;
}

void OptionsAdd(WPARAM addInfo) {
	OPTIONSDIALOGPAGE odp;
	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT);
	odp.pszTitle = Translate("Status Msg Change Notify");
	odp.pszGroup = Translate("PopUps");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = PopupsOptionsDlgProc;
	CallService(MS_OPT_ADDPAGE, addInfo, (LPARAM)&odp);

	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_NTF);
	odp.pszGroup = Translate("Status");
	odp.pfnDlgProc = StatusOptionsDlgProc;
	CallService(MS_OPT_ADDPAGE, addInfo, (LPARAM)&odp);

	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_PROTOCOLS);
	odp.pszTitle = Translate("Status Msg Retrieve");
	odp.pfnDlgProc = ProtocolsOptionsDlgProc;
	CallService(MS_OPT_ADDPAGE, addInfo, (LPARAM)&odp);
}

