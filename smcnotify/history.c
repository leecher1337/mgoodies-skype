/*
StatusMessageChangeNotify plugin for Miranda IM.

Copyright © 2004-2005 NoName
Copyright © 2005-2006 Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci

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

#include "main.h"

static void LoadHistoryList(HANDLE hContact, HWND hwnd, int nList) {
	short historyFirst, historyLast, historyMax;
	short i;
	TCHAR *str, tempstr[2048];
	DBVARIANT dbv;
	STATUSMSGINFO smi;
	ZeroMemory(&smi, sizeof(smi));

	str = (TCHAR*)malloc(2 * sizeof(TCHAR));
	str[0] = _T('\0');

	historyMax = (short)DBGetContactSettingDword(hContact, MODULE, OPT_HISTMAX, options.dHistMax);
	if (historyMax < 0) historyMax = 0; 
	else if (historyMax > 99) historyMax = 99;
	if (historyMax == 0) return;
	historyFirst = DBGetContactSettingWord(hContact, MODULE, "HistoryFirst",0);
	if (historyFirst >=  historyMax) historyFirst = 0;
	historyLast = DBGetContactSettingWord(hContact, MODULE, "HistoryLast",0);
	if (historyLast >= historyMax) historyLast = historyMax - 1;

	// reading history
	i = historyLast;
	while (i != historyFirst)
	{
		i = (i - 1 + historyMax) % historyMax;
		tempstr[0] = _T('\0');
		dbv.type = 0;
		//reading old status message and its timestamp separately
		if (!DBGetContactSetting(hContact, MODULE, BuildSetting(i, FALSE), &dbv))
		{
			smi.newstatusmsg = malloc(strlen(dbv.pszVal) + 2);
			lstrcpy(smi.newstatusmsg, dbv.pszVal);
			smi.dTimeStamp = DBGetContactSettingDword(hContact, MODULE, BuildSetting(i, TRUE), 0);
			lstrcpyn(tempstr, GetStr(&smi, options.his), sizeof(tempstr));
			DBFreeVariant(&dbv);
		}

		if (tempstr[0] != _T('\0'))
		{
			str = realloc(str, (lstrlen(str) + lstrlen(tempstr) + 5) * sizeof(TCHAR));
			lstrcat(str, tempstr);
			lstrcat(str, _T("\r\n"));
		}
	}
	SetDlgItemText(hwnd, nList, str);
	free(str);
	str = NULL;
	//free STATUSMSGINFO memory
	if (smi.oldstatusmsg) free(smi.oldstatusmsg);
	if (smi.newstatusmsg) free(smi.newstatusmsg);
	if (smi.cust) free(smi.cust);
}

static void ClearHistory(HANDLE hContact) {
	int i;
	BOOL bSettingExists = TRUE;
	for (i = 0; /*i < (int)options.dHistMax && */bSettingExists; i++)
	{
		bSettingExists = !DBDeleteContactSetting(hContact, MODULE, BuildSetting(i, FALSE));
		DBDeleteContactSetting(hContact, MODULE, BuildSetting(i, TRUE));
	}
	DBDeleteContactSetting(hContact, MODULE, "HistoryFirst");
	DBDeleteContactSetting(hContact, MODULE, "HistoryLast");
}

void ClearAllHistory() {
	HANDLE hContact;
	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		ClearHistory(hContact);
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}

static BOOL CALLBACK HistoryDlgProc(HWND hwndDlg, UINT Message, WPARAM wParam, LPARAM lParam) {
	HANDLE hContact;
	switch(Message)
	{
		case WM_INITDIALOG: {
				DWORD ignore;
				hContact = (HANDLE)lParam;

				SetWindowLong(hwndDlg, GWL_USERDATA, lParam);

				// set buttons
				SendDlgItemMessage(hwndDlg,IDC_DI,BM_SETIMAGE,IMAGE_ICON, (LPARAM)ICO_HIST);
				SendDlgItemMessage(hwndDlg,IDC_DE,BM_SETIMAGE,IMAGE_ICON, (LPARAM)ICO_LOG);
				SendDlgItemMessage(hwndDlg,IDC_DP,BM_SETIMAGE,IMAGE_ICON, (LPARAM)ICO_POPUP_E);

				SendMessage(GetDlgItem(hwndDlg,IDC_DI), BUTTONSETASFLATBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DI), BUTTONSETASPUSHBTN, 0, 0);

				SendMessage(GetDlgItem(hwndDlg,IDC_DE), BUTTONSETASFLATBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DE), BUTTONSETASPUSHBTN, 0, 0);

				SendMessage(GetDlgItem(hwndDlg,IDC_DP), BUTTONSETASFLATBTN, 0, 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DP), BUTTONSETASPUSHBTN, 0, 0);

				ignore = DBGetContactSettingDword(hContact, IGNORE_MODULE, IGNORE_MASK, 0);
				CheckDlgButton(hwndDlg, IDC_DI, !(ignore & IGNORE_INT));
				CheckDlgButton(hwndDlg, IDC_DE, !(ignore & IGNORE_EXT));
				CheckDlgButton(hwndDlg, IDC_DP, !(ignore & IGNORE_POP));

				SendMessage(GetDlgItem(hwndDlg,IDC_DI), BUTTONADDTOOLTIP, (WPARAM)TranslateT("Enable/Disable internal logging for this contact"), 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DE), BUTTONADDTOOLTIP, (WPARAM)TranslateT("Enable/Disable external logging for this contact"), 0);
				SendMessage(GetDlgItem(hwndDlg,IDC_DP), BUTTONADDTOOLTIP, (WPARAM)TranslateT("Enable/Disable popups for this contact"), 0);

				//make dialog bigger if UserInfoEx in use [222x132 dlus/340x170 dlus]
/*				if (ServiceExists("SMR/MsgRetrievalEnabledForProtocol")) 
				{
					//SendMessage(hwndDlg, WM_SIZE, SIZE_RESTORED, MAKELPARAM(LOWORD(450), HIWORD(275)));
				}
*/
				TranslateDialogDefault(hwndDlg);
				LoadHistoryList(hContact, hwndDlg, IDC_HISTORYLIST);
			}
			break;
		case WM_COMMAND:
			hContact=(HANDLE)GetWindowLong(hwndDlg, GWL_USERDATA);
			switch(LOWORD(wParam))
			{
				case IDC_CLEARHIST:
					ClearHistory(hContact);
					SetDlgItemText(hwndDlg, IDC_HISTORYLIST, "");
					break;
				case IDC_DP:
				case IDC_DI:
				case IDC_DE: {
					DWORD ignore = IsDlgButtonChecked(hwndDlg, IDC_DP)?0:IGNORE_POP;
					ignore = IsDlgButtonChecked(hwndDlg, IDC_DI)?ignore:ignore | IGNORE_INT;
					ignore = IsDlgButtonChecked(hwndDlg, IDC_DE)?ignore:ignore | IGNORE_EXT;
					DBWriteContactSettingDword(hContact, IGNORE_MODULE, IGNORE_MASK, ignore);
					break;
				}
			}
			break;
//		case WM_CTLCOLORSTATIC:
//			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_HISTORYLIST))
//			{
//				SetBkMode((HDC)wParam, OPAQUE/*TRANSPARENT*/);
//				SetBkColor((HDC)wParam, 0x00000000/*GetSysColor(COLOR_WINDOW)*/);
//				SetTextColor((HDC)wParam, 0x00FFFFFF/*GetSysColor(COLOR_WINDOWTEXT)*/);
//				return (BOOL)GetSysColorBrush(COLOR_WINDOW);
//			}
//			return FALSE;
/*		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			WindowList_Remove(hWindowList, hwndDlg);
			break;
		case WM_DESTROY: {
				HFONT hFont;
				hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_USERMENU, WM_GETFONT, 0, 0);
				DeleteObject(hFont);
			}
			break;
*/		default:
			return FALSE;
	}
	return TRUE;
}

/*
Called when User Info dialog initilised
*/
int HookedUserInfo(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp;
	ZeroMemory(&odp, sizeof(odp));

	if ((HANDLE)lParam == NULL) return 0;

	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_HISTORY);
	odp.pszTitle = TranslateT("Status Message History");
	odp.pfnDlgProc = HistoryDlgProc;
	CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}

void ShowHistory(HANDLE hContact) {
	DBWriteContactSettingTString(NULL, "UserInfo", "LastTab", TranslateT("Status Message History"));
	CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)hContact, 0);
}
