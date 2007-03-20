/*
Status Message Change Notify plugin for Miranda IM.

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

#include "commonheaders.h"


static void LoadHistory(HANDLE hContact, HWND hwnd, int nList) {
	WORD historyFirst, historyLast, historyMax;
	short i;
	//size_t size;
	TCHAR *str, *tempstr;
	DBVARIANT dbv;
	STATUSMSGINFO smi;

	historyMax = DBGetContactSettingWord(hContact, MODULE_NAME, "HistoryMax", opts.dHistoryMax);
	if (historyMax <= 0)
		return;
	else if (historyMax > 99)
		historyMax = 99;

	historyFirst = DBGetContactSettingWord(hContact, MODULE_NAME, "HistoryFirst",0);
	if (historyFirst >=  historyMax)
		historyFirst = 0;
	historyLast = DBGetContactSettingWord(hContact, MODULE_NAME, "HistoryLast",0);
	if (historyLast >= historyMax)
		historyLast = historyMax - 1;

	ZeroMemory(&smi, sizeof(smi));

	//str = (TCHAR*)mir_alloc(3 * sizeof(TCHAR));
	str = (TCHAR*)mir_alloc(historyMax * 1024 * sizeof(TCHAR));
	str[0] = _T('\0');
	tempstr = NULL;

	i = historyLast;
	while (i != historyFirst)
	{
		i = (i - 1 + historyMax) % historyMax;

		if (!DBGetContactSettingTString(hContact, MODULE_NAME, BuildSetting(i, NULL), &dbv))
		{
#ifdef UNICODE
			if (dbv.type == DBVT_ASCIIZ)
			{
				smi.newstatusmsg = mir_dupToUnicodeEx(dbv.pszVal, CP_ACP);
			}
			else if (dbv.type == DBVT_UTF8)
			{
				smi.newstatusmsg = mir_dupToUnicodeEx(dbv.pszVal, CP_UTF8);
			}
			else if (dbv.type == DBVT_WCHAR)
			{
				smi.newstatusmsg = dbv.pwszVal;
			}
#else
			if (dbv.type == DBVT_ASCIIZ)
			{
				smi.newstatusmsg = dbv.pszVal;
			}
#endif
			else
			{
				smi.newstatusmsg = NULL;
			}

			smi.dTimeStamp = DBGetContactSettingDword(hContact, MODULE_NAME, BuildSetting(i, "_ts"), 0);
			tempstr = GetStr(&smi, opts.history);
			mir_free(smi.newstatusmsg);
			DBFreeVariant(&dbv);
		}

		if ((tempstr != NULL) && (tempstr[0] != _T('\0')))
		{
			//size = (lstrlen(str) + lstrlen(tempstr) + 2) * sizeof(TCHAR);
			//str = (TCHAR*)mir_realloc(str, size);
			lstrcat(str, tempstr);
			lstrcat(str, _T("\r\n"));
		}
		mir_free(tempstr);
	}
	SetDlgItemText(hwnd, nList, str);

	mir_free(str);
	return;
}

static void ClearHistory(HANDLE hContact) {
	int i;
	BOOL bSettingExists = TRUE;
	for (i = 0; /*i < (int)options.dHistMax && */bSettingExists; i++)
	{
		bSettingExists = !DBDeleteContactSetting(hContact, MODULE_NAME, BuildSetting(i, NULL));
		DBDeleteContactSetting(hContact, MODULE_NAME, BuildSetting(i, "_ts"));
	}
	DBDeleteContactSetting(hContact, MODULE_NAME, "HistoryFirst");
	DBDeleteContactSetting(hContact, MODULE_NAME, "HistoryLast");
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

static BOOL CALLBACK HistoryDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			DWORD ignore;
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);

			TranslateDialogDefault(hwndDlg);

			// set icons on buttons
			SendDlgItemMessage(hwndDlg,IDC_CPOPUP,BM_SETIMAGE,IMAGE_ICON, CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)ICO_POPUP_E));
			SendDlgItemMessage(hwndDlg,IDC_CHISTORY,BM_SETIMAGE,IMAGE_ICON, CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)ICO_HISTORY));
			SendDlgItemMessage(hwndDlg,IDC_CLOG,BM_SETIMAGE,IMAGE_ICON, CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)ICO_LOG));

			SendMessage(GetDlgItem(hwndDlg,IDC_CPOPUP), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg,IDC_CPOPUP), BUTTONSETASPUSHBTN, 0, 0);

			SendMessage(GetDlgItem(hwndDlg,IDC_CHISTORY), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg,IDC_CHISTORY), BUTTONSETASPUSHBTN, 0, 0);

			SendMessage(GetDlgItem(hwndDlg,IDC_CLOG), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg,IDC_CLOG), BUTTONSETASPUSHBTN, 0, 0);

			ignore = DBGetContactSettingDword((HANDLE)lParam, "Ignore", MODULE_NAME, 0);
			CheckDlgButton(hwndDlg, IDC_CPOPUP, !(ignore & SMII_POPUP));
			CheckDlgButton(hwndDlg, IDC_CHISTORY, !(ignore & SMII_HISTORY));
			CheckDlgButton(hwndDlg, IDC_CLOG, !(ignore & SMII_LOG));

			SendMessage(GetDlgItem(hwndDlg,IDC_CPOPUP), BUTTONADDTOOLTIP, (WPARAM)Translate("Enable/Disable popups for this contact"), 0);
			SendMessage(GetDlgItem(hwndDlg,IDC_CHISTORY), BUTTONADDTOOLTIP, (WPARAM)Translate("Enable/Disable history for this contact"), 0);
			SendMessage(GetDlgItem(hwndDlg,IDC_CLOG), BUTTONADDTOOLTIP, (WPARAM)Translate("Enable/Disable logging to file for this contact"), 0);

			//make dialog bigger if UserInfoEx in use [222x132 dlus/340x170 dlus]
			if (ServiceExists("UserInfo/Reminder/AggrassiveBackup"))
			{
				RECT rc, rc0, rcp;
				rc0.left = 2;rc0.top = 155;rc0.right = 298;rc0.bottom = 148;
				MapDialogRect(hwndDlg, &rc0);
				MoveWindow(GetDlgItem(hwndDlg, IDC_HISTORYLIST), rc0.left, 2 * rc0.left, rc0.right, rc0.bottom, TRUE);
				GetClientRect(GetDlgItem(hwndDlg, IDC_CHISTORYCLEAR), &rc);
				MoveWindow(GetDlgItem(hwndDlg, IDC_CHISTORYCLEAR), rc0.left, rc0.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
				rcp.left = 170;rcp.top = 188;rcp.right = 18;rcp.bottom = 148;MapDialogRect(hwndDlg, &rcp);
				GetClientRect(GetDlgItem(hwndDlg, IDC_CPOPUP), &rc);
				MoveWindow(GetDlgItem(hwndDlg, IDC_CPOPUP), rcp.left, rc0.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
				GetClientRect(GetDlgItem(hwndDlg, IDC_CHISTORY), &rc);
				MoveWindow(GetDlgItem(hwndDlg, IDC_CHISTORY), rcp.left + rcp.right, rc0.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
				GetClientRect(GetDlgItem(hwndDlg, IDC_CLOG), &rc);
				MoveWindow(GetDlgItem(hwndDlg, IDC_CLOG), rcp.left + rcp.right + rcp.right, rc0.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
			}

			LoadHistory((HANDLE)lParam, hwndDlg, IDC_HISTORYLIST);
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_CHISTORYCLEAR:
					ClearHistory((HANDLE)GetWindowLong(hwndDlg, GWL_USERDATA));
					SetDlgItemText(hwndDlg, IDC_HISTORYLIST, _T(""));
					break;
				case IDC_CPOPUP:
				case IDC_CLOG:
				case IDC_CHISTORY:
				{
					DWORD ignore = IsDlgButtonChecked(hwndDlg, IDC_CPOPUP)?0:SMII_POPUP;
					ignore = IsDlgButtonChecked(hwndDlg, IDC_CHISTORY)?ignore:ignore | SMII_HISTORY;
					ignore = IsDlgButtonChecked(hwndDlg, IDC_CLOG)?ignore:ignore | SMII_LOG;
					DBWriteContactSettingDword((HANDLE)GetWindowLong(hwndDlg, GWL_USERDATA), "Ignore", MODULE_NAME, ignore);
					break;
				}
			}
			break;
#ifdef CUSTOMBUILD_COLORHISTORY
		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_HISTORYLIST))
			{
				//SetBkMode((HDC)wParam, OPAQUE/*TRANSPARENT*/);
				SetBkColor((HDC)wParam, 0x00000000/*GetSysColor(COLOR_WINDOW)*/);
				SetTextColor((HDC)wParam, 0x00FFFFFF/*GetSysColor(COLOR_WINDOWTEXT)*/);
				//return (BOOL)GetSysColorBrush(COLOR_WINDOW);
				break;
			}
#endif
	}

	return 0;
}

extern int UserInfoInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));

	if ((HANDLE)lParam == NULL) return 0;

	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_HISTORY);
	odp.ptszTitle = TranslateT("Status Message History");
	odp.flags = ODPF_TCHAR;
	odp.pfnDlgProc = HistoryDlgProc;
	CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}
