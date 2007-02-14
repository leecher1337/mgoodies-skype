/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
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

static BOOL CALLBACK DlgProcGenOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_USER + 1:
		{
			HANDLE hContact = (HANDLE) wParam;
			DBCONTACTWRITESETTING *ws = (DBCONTACTWRITESETTING *) lParam;
			if (hContact == NULL && ws != NULL && ws->szModule != NULL && ws->szSetting != NULL
				&& lstrcmpiA(ws->szModule, "CList") == 0 && lstrcmpiA(ws->szSetting, "UseGroups") == 0 && IsWindowVisible(hwndDlg)) {
					CheckDlgButton(hwndDlg, IDC_DISABLEGROUPS, ws->value.bVal == 0);
				}
				break;
		}
	case WM_DESTROY:
		{
			UnhookEvent((HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA));
			break;
		}
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) HookEventMessage(ME_DB_CONTACT_SETTINGCHANGED, hwndDlg, WM_USER + 1));
		CheckDlgButton(hwndDlg, IDC_ONTOP, DBGetContactSettingByte(NULL, "CList", "OnTop", SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HIDEOFFLINE,
			DBGetContactSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HIDEEMPTYGROUPS,
			DBGetContactSettingByte(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DISABLEGROUPS,
			DBGetContactSettingByte(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_SORTBYNAME, !DBGetContactSettingByte(NULL, "CList", "SortByStatus", SETTING_SORTBYSTATUS_DEFAULT)
			&& !DBGetContactSettingByte(NULL, "CList", "SortByProto", SETTING_SORTBYPROTO_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SORTBYSTATUS,
			DBGetContactSettingByte(NULL, "CList", "SortByStatus", SETTING_SORTBYSTATUS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SORTBYPROTO,
			DBGetContactSettingByte(NULL, "CList", "SortByProto", SETTING_SORTBYPROTO_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CONFIRMDELETE,
			DBGetContactSettingByte(NULL, "CList", "ConfirmDelete", SETTING_CONFIRMDELETE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOHIDE,
			DBGetContactSettingByte(NULL, "CList", "AutoHide", SETTING_AUTOHIDE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIME), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		{
			DWORD caps = CallService(MS_CLUI_GETCAPS, CLUICAPS_FLAGS1, 0);
			if (!(caps & CLUIF_HIDEEMPTYGROUPS))
				ShowWindow(GetDlgItem(hwndDlg, IDC_HIDEEMPTYGROUPS), SW_HIDE);
			if (!(caps & CLUIF_DISABLEGROUPS))
				ShowWindow(GetDlgItem(hwndDlg, IDC_DISABLEGROUPS), SW_HIDE);
			if (caps & CLUIF_HASONTOPOPTION)
				ShowWindow(GetDlgItem(hwndDlg, IDC_ONTOP), SW_HIDE);
			if (caps & CLUIF_HASAUTOHIDEOPTION) {
				ShowWindow(GetDlgItem(hwndDlg, IDC_AUTOHIDE), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_HIDETIME), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_STAUTOHIDESECS), SW_HIDE);
			}
		}
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETRANGE, 0, MAKELONG(900, 1));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETPOS, 0,
			MAKELONG(DBGetContactSettingWord(NULL, "CList", "HideTime", SETTING_HIDETIME_DEFAULT), 0));
		CheckDlgButton(hwndDlg, IDC_ONECLK,
			DBGetContactSettingByte(NULL, "CList", "Tray1Click", SETTING_TRAY1CLICK_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALWAYSSTATUS,
			DBGetContactSettingByte(NULL, "CList", "AlwaysStatus", SETTING_ALWAYSSTATUS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALWAYSMULTI,
			!DBGetContactSettingByte(NULL, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DONTCYCLE,
			DBGetContactSettingByte(NULL, "CList", "TrayIcon",
			SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_SINGLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CYCLE,
			DBGetContactSettingByte(NULL, "CList", "TrayIcon",
			SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_CYCLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MULTITRAY,
			DBGetContactSettingByte(NULL, "CList", "TrayIcon",
			SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DISABLEBLINK,
			DBGetContactSettingByte(NULL, "CList", "DisableTrayFlash", 0) == 1 ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKTIME), !IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
		EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKSPIN), !IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
		EnableWindow(GetDlgItem(hwndDlg, IDC_STMSDELAY), !IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
		CheckDlgButton(hwndDlg, IDC_ICONBLINK, DBGetContactSettingByte(NULL, "CList", "NoIconBlink", 0) == 1 ? BST_CHECKED : BST_UNCHECKED);
		if (IsDlgButtonChecked(hwndDlg, IDC_DONTCYCLE)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIMESPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYSMULTI), FALSE);
		}
		if (IsDlgButtonChecked(hwndDlg, IDC_CYCLE)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYSMULTI), FALSE);
		}
		if (IsDlgButtonChecked(hwndDlg, IDC_MULTITRAY)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIMESPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS), FALSE);
		}
		SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_SETRANGE, 0, MAKELONG(120, 1));
		SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_SETPOS, 0,
			MAKELONG(DBGetContactSettingWord(NULL, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT), 0));
		{
			int i, count, item;
			PROTOCOLDESCRIPTOR **protos;
			char szName[64];
			DBVARIANT dbv = { DBVT_DELETED };
			DBGetContactSetting(NULL, "CList", "PrimaryStatus", &dbv);
			CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM) & count, (LPARAM) & protos);
			item = SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_ADDSTRING, 0, (LPARAM) TranslateT("Global"));
			SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_SETITEMDATA, item, (LPARAM) 0);
			for (i = 0; i < count; i++) {
				if (protos[i]->type != PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
					continue;
				CallProtoService(protos[i]->szName, PS_GETNAME, SIZEOF(szName), (LPARAM) szName);
				item = SendDlgItemMessageA(hwndDlg, IDC_PRIMARYSTATUS, CB_ADDSTRING, 0, (LPARAM) szName);
				SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_SETITEMDATA, item, (LPARAM) protos[i]);
				if (dbv.type == DBVT_ASCIIZ && !lstrcmpA(dbv.pszVal, protos[i]->szName))
					SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_SETCURSEL, item, 0);
			}
			DBFreeVariant(&dbv);
		}
		if (-1 == (int) SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_GETCURSEL, 0, 0))
			SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_SETCURSEL, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETBUDDY, (WPARAM) GetDlgItem(hwndDlg, IDC_BLINKTIME), 0);   // set buddy                    
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETRANGE, 0, MAKELONG(0x3FFF, 250));
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETPOS, 0, MAKELONG(DBGetContactSettingWord(NULL, "CList", "IconFlashTime", 550), 0));
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_AUTOHIDE) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIME), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		}
		if (LOWORD(wParam) == IDC_DONTCYCLE || LOWORD(wParam) == IDC_CYCLE || LOWORD(wParam) == IDC_MULTITRAY) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS), IsDlgButtonChecked(hwndDlg, IDC_DONTCYCLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIME), IsDlgButtonChecked(hwndDlg, IDC_CYCLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_CYCLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYSMULTI), IsDlgButtonChecked(hwndDlg, IDC_MULTITRAY));
		}
		if (LOWORD(wParam) == IDC_DISABLEBLINK) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKTIME), !IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
			EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKSPIN), !IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STMSDELAY), !IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
		}
		if ((LOWORD(wParam) == IDC_HIDETIME || LOWORD(wParam) == IDC_CYCLETIME) && HIWORD(wParam) != EN_CHANGE)
			break;
		if (LOWORD(wParam) == IDC_PRIMARYSTATUS && HIWORD(wParam) != CBN_SELCHANGE)
			break;
		if ((LOWORD(wParam) == IDC_HIDETIME || LOWORD(wParam) == IDC_CYCLETIME) && (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
			return 0;
		if (LOWORD(wParam) == IDC_BLINKTIME && HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus())
			return 0;       // dont make apply enabled during buddy set crap
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR) lParam)->code) {
			case PSN_APPLY:
				DBWriteContactSettingByte(NULL, "CList", "HideOffline", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_HIDEOFFLINE));
				{
					DWORD caps = CallService(MS_CLUI_GETCAPS, CLUICAPS_FLAGS1, 0);
					if (caps & CLUIF_HIDEEMPTYGROUPS)
						DBWriteContactSettingByte(NULL, "CList", "HideEmptyGroups",
						(BYTE) IsDlgButtonChecked(hwndDlg, IDC_HIDEEMPTYGROUPS));
					if (caps & CLUIF_DISABLEGROUPS)
						DBWriteContactSettingByte(NULL, "CList", "UseGroups", (BYTE) ! IsDlgButtonChecked(hwndDlg, IDC_DISABLEGROUPS));
					if (!(caps & CLUIF_HASONTOPOPTION)) {
						DBWriteContactSettingByte(NULL, "CList", "OnTop", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ONTOP));
						SetWindowPos( pcli->hwndContactList,
							IsDlgButtonChecked(hwndDlg, IDC_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
							SWP_NOMOVE | SWP_NOSIZE);
					}
					if (!(caps & CLUIF_HASAUTOHIDEOPTION)) {
						DBWriteContactSettingByte(NULL, "CList", "AutoHide", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
						DBWriteContactSettingWord(NULL, "CList", "HideTime",
							(WORD) SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_GETPOS, 0, 0));
					}
				}
				DBWriteContactSettingByte(NULL, "CList", "SortByStatus", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SORTBYSTATUS));
				DBWriteContactSettingByte(NULL, "CList", "SortByProto", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SORTBYPROTO));
				DBWriteContactSettingByte(NULL, "CList", "ConfirmDelete", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_CONFIRMDELETE));
				DBWriteContactSettingByte(NULL, "CList", "Tray1Click", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ONECLK));
				DBWriteContactSettingByte(NULL, "CList", "AlwaysStatus", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ALWAYSSTATUS));
				DBWriteContactSettingByte(NULL, "CList", "AlwaysMulti", (BYTE) ! IsDlgButtonChecked(hwndDlg, IDC_ALWAYSMULTI));
				DBWriteContactSettingByte(NULL, "CList", "TrayIcon",
					(BYTE) (IsDlgButtonChecked(hwndDlg, IDC_DONTCYCLE) ? SETTING_TRAYICON_SINGLE
					: (IsDlgButtonChecked(hwndDlg, IDC_CYCLE) ? SETTING_TRAYICON_CYCLE :
				SETTING_TRAYICON_MULTI)));
				DBWriteContactSettingWord(NULL, "CList", "CycleTime",
					(WORD) SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_GETPOS, 0, 0));
				DBWriteContactSettingWord(NULL, "CList", "IconFlashTime",
					(WORD) SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_GETPOS, 0, 0));
				DBWriteContactSettingByte(NULL, "CList", "DisableTrayFlash", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
				DBWriteContactSettingByte(NULL, "CList", "NoIconBlink", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ICONBLINK));
				if (!SendDlgItemMessage
					(hwndDlg, IDC_PRIMARYSTATUS, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_GETCURSEL, 0, 0), 0))
					DBDeleteContactSetting(NULL, "CList", "PrimaryStatus");
				else
					DBWriteContactSettingString(NULL, "CList", "PrimaryStatus",
					((PROTOCOLDESCRIPTOR *)
					SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_GETCURSEL, 0, 0),
					0))->szName);
				pcli->pfnTrayIconIconsChanged();
				pcli->pfnLoadContactTree();  /* this won't do job properly since it only really works when changes happen */
				pcli->pfnInvalidateDisplayNameCacheEntry( INVALID_HANDLE_VALUE );        /* force reshuffle */
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

static BOOL CALLBACK DlgProcHotkeyOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		DBVARIANT dbv;

		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_SHOWHIDE, DBGetContactSettingByte(NULL, "CList", "HKEnShowHide", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_READMSG, DBGetContactSettingByte(NULL, "CList", "HKEnReadMsg", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NETSEARCH, DBGetContactSettingByte(NULL, "CList", "HKEnNetSearch", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWOPTIONS, DBGetContactSettingByte(NULL, "CList", "HKEnShowOptions", 0) ? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_HKSHOWHIDE), IsDlgButtonChecked(hwndDlg, IDC_SHOWHIDE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HKREADMSG), IsDlgButtonChecked(hwndDlg, IDC_READMSG));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HKSEARCH), IsDlgButtonChecked(hwndDlg, IDC_NETSEARCH));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SEARCHURL), IsDlgButtonChecked(hwndDlg, IDC_NETSEARCH));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SEARCHNEWWND), IsDlgButtonChecked(hwndDlg, IDC_NETSEARCH));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HOTKEYURLSTR), IsDlgButtonChecked(hwndDlg, IDC_NETSEARCH));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HKSHOWOPTIONS), IsDlgButtonChecked(hwndDlg, IDC_SHOWOPTIONS));

		SendDlgItemMessage(hwndDlg, IDC_HKSHOWHIDE, HKM_SETHOTKEY,
			DBGetContactSettingWord(NULL, "CList", "HKShowHide", MAKEWORD('A', HOTKEYF_CONTROL | HOTKEYF_SHIFT)), 0);
		SendDlgItemMessage(hwndDlg, IDC_HKREADMSG, HKM_SETHOTKEY,
			DBGetContactSettingWord(NULL, "CList", "HKReadMsg", MAKEWORD('I', HOTKEYF_CONTROL | HOTKEYF_SHIFT)), 0);
		SendDlgItemMessage(hwndDlg, IDC_HKSEARCH, HKM_SETHOTKEY,
			DBGetContactSettingWord(NULL, "CList", "HKNetSearch", MAKEWORD('S', HOTKEYF_CONTROL | HOTKEYF_SHIFT)), 0);
		SendDlgItemMessage(hwndDlg, IDC_HKSHOWOPTIONS, HKM_SETHOTKEY,
			DBGetContactSettingWord(NULL, "CList", "HKShowOptions", MAKEWORD('O', HOTKEYF_CONTROL | HOTKEYF_SHIFT)), 0);
		if (!DBGetContactSetting(NULL, "CList", "SearchUrl", &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_SEARCHURL, dbv.pszVal);
			mir_free(dbv.pszVal);
		}
		else
			SetDlgItemTextA(hwndDlg, IDC_SEARCHURL, "http://www.google.com/");
		CheckDlgButton(hwndDlg, IDC_SEARCHNEWWND, DBGetContactSettingByte(NULL, "CList", "HKSearchNewWnd", 0) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SEARCHURL && (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		switch (LOWORD(wParam)) {
		case IDC_SHOWHIDE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_HKSHOWHIDE), IsDlgButtonChecked(hwndDlg, IDC_SHOWHIDE));
			break;
		case IDC_READMSG:
			EnableWindow(GetDlgItem(hwndDlg, IDC_HKREADMSG), IsDlgButtonChecked(hwndDlg, IDC_READMSG));
			break;
		case IDC_NETSEARCH:
			EnableWindow(GetDlgItem(hwndDlg, IDC_HKSEARCH), IsDlgButtonChecked(hwndDlg, IDC_NETSEARCH));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SEARCHURL), IsDlgButtonChecked(hwndDlg, IDC_NETSEARCH));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SEARCHNEWWND), IsDlgButtonChecked(hwndDlg, IDC_NETSEARCH));
			EnableWindow(GetDlgItem(hwndDlg, IDC_HOTKEYURLSTR), IsDlgButtonChecked(hwndDlg, IDC_NETSEARCH));
			break;
		case IDC_SHOWOPTIONS:
			EnableWindow(GetDlgItem(hwndDlg, IDC_HKSHOWOPTIONS), IsDlgButtonChecked(hwndDlg, IDC_SHOWOPTIONS));
			break;
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) {
		case PSN_APPLY:
			{
				char str[256];
				pcli->pfnHotKeysUnregister( pcli->hwndContactList );
				DBWriteContactSettingByte(NULL, "CList", "HKEnShowHide", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWHIDE));
				DBWriteContactSettingWord(NULL, "CList", "HKShowHide", (WORD) SendDlgItemMessage(hwndDlg, IDC_HKSHOWHIDE, HKM_GETHOTKEY, 0, 0));
				DBWriteContactSettingByte(NULL, "CList", "HKEnReadMsg", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_READMSG));
				DBWriteContactSettingWord(NULL, "CList", "HKReadMsg", (WORD) SendDlgItemMessage(hwndDlg, IDC_HKREADMSG, HKM_GETHOTKEY, 0, 0));
				DBWriteContactSettingByte(NULL, "CList", "HKEnNetSearch", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_NETSEARCH));
				DBWriteContactSettingWord(NULL, "CList", "HKNetSearch", (WORD) SendDlgItemMessage(hwndDlg, IDC_HKSEARCH, HKM_GETHOTKEY, 0, 0));
				GetDlgItemTextA(hwndDlg, IDC_SEARCHURL, str, SIZEOF(str));
				DBWriteContactSettingString(NULL, "CList", "SearchUrl", str);
				DBWriteContactSettingByte(NULL, "CList", "HKSearchNewWnd", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SEARCHNEWWND));
				DBWriteContactSettingByte(NULL, "CList", "HKEnShowOptions", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWOPTIONS));
				DBWriteContactSettingWord(NULL, "CList", "HKShowOptions", (WORD) SendDlgItemMessage(hwndDlg, IDC_HKSHOWOPTIONS, HKM_GETHOTKEY, 0, 0));
				pcli->pfnHotKeysRegister( pcli->hwndContactList );
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/****************************************************************************************/

static UINT expertOnlyControls[] = { IDC_ALWAYSSTATUS };

int CListOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = -1000000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLIST);
	odp.pszTitle = "Contact List";
	odp.pfnDlgProc = DlgProcGenOpts;
	odp.flags = ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl = IDC_STCLISTGROUP;
	odp.expertOnlyControls = expertOnlyControls;
	odp.nExpertOnlyControls = SIZEOF(expertOnlyControls);
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);

	odp.position = -900000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_HOTKEY);
	odp.pszTitle = "Hotkeys";
	odp.pszGroup = "Events";
	odp.pfnDlgProc = DlgProcHotkeyOpts;
	odp.nIDBottomSimpleControl = 0;
	odp.nExpertOnlyControls = 0;
	odp.expertOnlyControls = NULL;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);
	return 0;
}
