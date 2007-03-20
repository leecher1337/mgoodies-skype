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


BOOL AllowProtocol(const char *proto) {
	if ((CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGRECV) == 0)
		return FALSE;

	return TRUE;
}

static OptPageControl optionsControls[] = {
	{&opts.bIgnoreRemove,		CONTROL_CHECKBOX,	IDC_IGNOREREMOVE,	"IgnoreRemove", FALSE},
	{&opts.bListUseBkImage,		CONTROL_CHECKBOX,	IDC_USEBGIMG,		"ListUseBkImage", FALSE},
	{&opts.colListBack,			CONTROL_COLOR,		IDC_LISTBGCOLOR,	"ListBkColor", RGB(255,255,255)},
	{&opts.colListText,			CONTROL_COLOR,		IDC_LISTTEXTCOLOR,	"ListTextColor", RGB(0,0,0)},
	{NULL,						CONTROL_PROTOCOL_LIST,	IDC_PROTOCOLS,	"%sEnabled", TRUE, (int)AllowProtocol}
};

static OptPageControl advancedControls[] = {
	{&opts.bHistoryEnable,		CONTROL_CHECKBOX,	IDC_HISTORY,		"HistoryEnable", TRUE},
	{&opts.dHistoryMax,			CONTROL_SPIN,		IDC_HISTORYMAX,		"HistoryMax", 20, IDC_HISTORYMAX_SPIN, (WORD)1, (WORD)100},
	{&opts.history,				CONTROL_TEXT,		IDC_HISTORYTEXT,	"HistoryTemplate", (DWORD)_T(DEFAULT_TEMPLATE_HISTORY)},
	{&opts.bDBEnable,			CONTROL_CHECKBOX,	IDC_MESSAGEWND,		"DBEnable", TRUE},
//	{&opts.history_only_ansi_if_possible,	CONTROL_CHECKBOX,	IDC_ANSI,	"HistoryOnlyANSIIfPossible", TRUE},
	{&opts.msgchanged,			CONTROL_TEXT,		IDC_MSGCHANGED,		"TemplateChanged", (DWORD)_T(DEFAULT_TEMPLATE_CHANGED)},
	{&opts.msgremoved,			CONTROL_TEXT,		IDC_MSGREMOVED,		"TemplateRemoved", (DWORD)_T(DEFAULT_TEMPLATE_REMOVED)},
	{&opts.bLogEnable,			CONTROL_CHECKBOX,	IDC_LOG,			"LogEnable", FALSE},
#ifdef UNICODE
	{&opts.bLogAscii,			CONTROL_CHECKBOX,	IDC_LOGASCII,		"LogAscii", FALSE},
#endif
	{&opts.log,					CONTROL_TEXT,		IDC_LOGTEXT,		"LogTemplate", (DWORD)_T(DEFAULT_TEMPLATE_LOG)}
};

static OptPageControl popupsControls[] = {
	{&puopts.bEnable,			CONTROL_CHECKBOX,	IDC_POPUPS,			"PopupsEnable", TRUE},
	{&puopts.bOnConnect,		CONTROL_CHECKBOX,	IDC_ONCONNECT,		"PopupsOnConnect", FALSE},
	{&puopts.bIfChanged,		CONTROL_CHECKBOX,	IDC_IFCHANGED,		"PopupsIfChanged", TRUE},
	{&puopts.bIgnoreRemove,		CONTROL_CHECKBOX,	IDC_PUIGNOREREMOVE,	"PopupsIgnoreRemove", FALSE},
#ifdef CUSTOMBUILD_OSDSUPPORT
	{&puopts.bUseOSD,			CONTROL_CHECKBOX,	IDC_USEOSD,			"PopupsUseOSD", TRUE},
#endif
	{&puopts.bColorType,		CONTROL_RADIO,		IDC_COLORFROMPU,	"PopupsColorType", POPUP_COLOR_DEFAULT, POPUP_COLOR_DEFAULT},
	{NULL,						CONTROL_RADIO,		IDC_COLORWINDOWS,	"PopupsColorType", POPUP_COLOR_DEFAULT, POPUP_COLOR_WINDOWS},
	{NULL,						CONTROL_RADIO,		IDC_COLORCUSTOM,	"PopupsColorType", POPUP_COLOR_DEFAULT, POPUP_COLOR_CUSTOM},
	{&puopts.colBack,			CONTROL_COLOR,		IDC_PUBGCOLOR,		"PopupsBkColor", RGB(201,125,234)},
	{&puopts.colText,			CONTROL_COLOR,		IDC_PUTEXTCOLOR,	"PopupsTextColor", RGB(0,0,0)},
	{&puopts.bDelayType,		CONTROL_RADIO,		IDC_DELAYFROMPU,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_DEFAULT},
	{NULL,						CONTROL_RADIO,		IDC_DELAYCUSTOM,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_CUSTOM},
	{NULL,						CONTROL_RADIO,		IDC_DELAYPERMANENT,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_PERMANENT},
	{&puopts.dDelay,			CONTROL_SPIN,		IDC_DELAY,			"PopupsDelay", 10, IDC_DELAY_SPIN, (WORD)1, (WORD)255},
	{&puopts.LeftClickAction,	CONTROL_COMBO,		IDC_LEFTACTION,		"PopupsLeftClick", POPUP_ACTION_INFO},
	{&puopts.RightClickAction,	CONTROL_COMBO,		IDC_RIGHTACTION,	"PopupsRightClick", POPUP_ACTION_CLOSE},
	{&puopts.text,				CONTROL_TEXT,		IDC_POPUPTEXT,		"PopupsTemplate", (DWORD)_T(DEFAULT_TEMPLATE_POPUP)}
};
/*
static UINT popupsExpertControls[] = {
	IDC_ONCONNECT, IDC_IFCHANGED, IDC_PUIGNOREREMOVE,
	IDC_EXPERT, IDC_PUBGCOLOR, IDC_PUTEXTCOLOR, IDC_COLORTYPE,
	IDC_DELAYFROMPU, IDC_DELAYCUSTOM, IDC_DELAYPERMANENT, IDC_DELAY, IDC_DELAY_SPIN,
	IDC_RIGHTACTION, IDC_LEFTACTION,
	IDC_POPUPTEXT, IDC_PREVIEW
};
*/
static OptPageControl fileControls[] = {
	{&opts.logfile,				CONTROL_TEXT,		IDC_LOGFILE,		"LogFile", (DWORD)_T(DEFAULT_LOG_FILENAME), 0, 0, MAX_PATH},
	{&opts.listbkimage,			CONTROL_TEXT,		IDC_BGIMGFILE,		"ListBkImage", (DWORD)_T(DEFAULT_BGIMAGE_FILENAME), 0, 0, MAX_PATH}
};

#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )

void LoadOptions() {
	TCHAR temp[MAX_PATH];

	LoadOpts(optionsControls, MAX_REGS(optionsControls), MODULE_NAME);
	LoadOpts(advancedControls, MAX_REGS(advancedControls), MODULE_NAME);
	LoadOpts(popupsControls, MAX_REGS(popupsControls), MODULE_NAME);

	LoadOpts(fileControls, MAX_REGS(fileControls), MODULE_NAME);
	if (opts.listbkimage != NULL && opts.listbkimage[0] != _T('\0'))
	{
		CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)opts.listbkimage, (LPARAM)temp);
		lstrcpyn(opts.listbkimage, temp, MAX_PATH);
	}
	if (opts.logfile != NULL && opts.logfile[0] != _T('\0'))
	{
		CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)opts.logfile, (LPARAM)temp);
		lstrcpyn(opts.logfile, temp, MAX_PATH);
	}
}

static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	BOOL ret = SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
	switch (msg)
	{
		case WM_INITDIALOG:
			SetDlgItemText(hwndDlg, IDC_BGIMGFILE, opts.listbkimage);
			SendMessage(hwndDlg, WM_USER + 10, 0, 0);
			break;
		case WM_USER + 10:
		{
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_USEBGIMG);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LISTBGCOLOR), !enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BGIMGFILE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BGIMGBROWSE), enabled);
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_USEBGIMG:
					SendMessage(hwndDlg, WM_USER + 10, 0, 0);
					break;
				case IDC_BGIMGBROWSE:
				{
					OPENFILENAME ofn;
					TCHAR filepath[MAX_PATH] = _T("");
					char filter[512] = "";
#ifdef UNICODE
					WCHAR filterW[512] = L"";
					int i;
#endif

					GetDlgItemText(hwndDlg, IDC_BGIMGFILE, filepath, sizeof(filepath));
					CallService(MS_UTILS_GETBITMAPFILTERSTRINGS, sizeof(filter), (LPARAM)filter);
#ifdef UNICODE
					for (i = 0; i < 512; i++)
						filterW[i] = filter[i];
#endif

					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);//OPENFILENAME_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
#ifdef UNICODE
					ofn.lpstrFilter = filterW;
#else
					ofn.lpstrFilter = filter;
#endif
					ofn.lpstrFile = filepath;
					ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrDefExt = _T("bmp");

					if (GetOpenFileName(&ofn))
					{
						SetDlgItemText(hwndDlg, IDC_BGIMGFILE, filepath);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}

					break;
				}
				case IDC_BGIMGFILE:
					if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
						return 0;
//					else
//						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
			}
			break;
		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				TCHAR temp[MAX_PATH]; temp[0] = _T('\0');
				GetDlgItemText(hwndDlg, IDC_BGIMGFILE, opts.listbkimage, MAX_PATH);
				if (opts.listbkimage != NULL && opts.listbkimage[0] != _T('\0'))
					CallService(MS_UTILS_PATHTORELATIVET, (WPARAM)opts.listbkimage, (LPARAM)temp);
				DBWriteContactSettingTString(NULL, MODULE_NAME, "ListBkImage", temp);
			}
			break;
		}
	}
	return ret;
}

static BOOL CALLBACK AdvancedDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	BOOL ret = SaveOptsDlgProc(advancedControls, MAX_REGS(advancedControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TCHAR str[1024];str[0] = _T('\0');

			SetDlgItemText(hwndDlg, IDC_LOGFILE, opts.logfile);
#ifdef UNICODE
			ShowWindow(GetDlgItem(hwndDlg, IDC_LOGASCII), SW_HIDE);
#endif
			SendMessage(hwndDlg, WM_USER + 10, 0, 0);
			SendMessage(hwndDlg, WM_USER + 11, 0, 0);
			SendMessage(hwndDlg, WM_USER + 12, 0, 0);

			lstrcpy(str, _T("%n\tNew Status Message\r\n%o\tOld Status Message\r\n%c\tCustom Nickname\r\n\\n\tline break\r\n\\t\ttab stop"));
			SetDlgItemText(hwndDlg, IDC_VARS1, str);
			lstrcpy(str, _T("%D/%M/%Y\tDay/Month/Year\r\n%H:%m:%s\tTime (in 24h format)\r\n%h:%m:%s %a\t(in 12h format)"));
			SetDlgItemText(hwndDlg, IDC_VARS2, str);
			break;
		}
		case WM_USER + 10:
		{
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_MESSAGEWND);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MSGCHANGED), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MSGREMOVED), enabled);
			break;
		}
		case WM_USER + 11:
		{
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_HISTORY);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HISTORYMAX), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HISTORYMAX_SPIN), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HISTORYTEXT), enabled);
			break;
		}
		case WM_USER + 12:
		{
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_LOG);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGTEXT), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGFILE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGFILEBROWSE), enabled);
#ifdef UNICODE
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGASCII), enabled);
#endif
			break;
		}
		case WM_COMMAND:
			if ((LOWORD(wParam) == IDC_LOGFILE
				|| LOWORD(wParam) == IDC_VARS1
				|| LOWORD(wParam) == IDC_VARS2)
				&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
				return 0;
			switch (LOWORD(wParam))
			{
				case IDC_MESSAGEWND:
					SendMessage(hwndDlg, WM_USER + 10, 0, 0);
					break;
				case IDC_HISTORY:
					SendMessage(hwndDlg, WM_USER + 11, 0, 0);
					break;
				case IDC_LOG:
					SendMessage(hwndDlg, WM_USER + 12, 0, 0);
					break;
				case IDC_HISTORYCLEAR:
					ClearAllHistory();
					return 0;
				case IDC_LOGFILEBROWSE:
				{
					OPENFILENAME ofn;
					TCHAR filepath[MAX_PATH] = _T("");

					GetDlgItemText(hwndDlg, IDC_LOGFILE, filepath, sizeof(filepath));

					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);//OPENFILENAME_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.lpstrFilter = _T("Text Files (*.txt)\0*.txt\0Log Files (*.log)\0*.log\0All Files (*.*)\0*.*\0");
					ofn.lpstrFile = filepath;
					ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrDefExt = _T("txt");

					if (GetSaveFileName(&ofn))
					{
						SetDlgItemText(hwndDlg, IDC_LOGFILE, filepath);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}

					break;
				}
			}
			break;
		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				TCHAR temp[MAX_PATH]; temp[0] = _T('\0');
				GetDlgItemText(hwndDlg, IDC_LOGFILE, opts.logfile, MAX_PATH);
				if (opts.logfile != NULL && opts.logfile[0] != _T('\0'))
					CallService(MS_UTILS_PATHTORELATIVET, (WPARAM)opts.logfile, (LPARAM)temp);
				DBWriteContactSettingTString(NULL, MODULE_NAME, "LogFile", temp);
			}
			break;
		}
	}

	return ret;
}

static BOOL CALLBACK PopupOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			BOOL ret;

			SendDlgItemMessage(hwndDlg, IDC_RIGHTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Do nothing"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Close popup"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Open message window"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Open contact menu"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Open contact details"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("View status message history"));

			SendDlgItemMessage(hwndDlg, IDC_LEFTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Do nothing"));
			SendDlgItemMessage(hwndDlg, IDC_LEFTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Close popup"));
			SendDlgItemMessage(hwndDlg, IDC_LEFTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Open message window"));
			SendDlgItemMessage(hwndDlg, IDC_LEFTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Open contact menu"));
			SendDlgItemMessage(hwndDlg, IDC_LEFTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Open contact details"));
			SendDlgItemMessage(hwndDlg, IDC_LEFTACTION, CB_ADDSTRING, 0, (LONG) TranslateT("View status message history"));

			// Needs to be called here in this case
			ret = SaveOptsDlgProc(popupsControls, MAX_REGS(popupsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

			SendMessage(hwndDlg, WM_USER + 10, 0, 0);
			SendMessage(hwndDlg, WM_USER + 11, 0, 0);
			SendMessage(hwndDlg, WM_USER + 12, 0, 0);
#ifdef CUSTOMBUILD_OSDSUPPORT
			SendMessage(hwndDlg, WM_USER + 13, 0, 0);
#endif

			return ret;
		}
		case WM_USER + 10:
			EnableWindow(GetDlgItem(hwndDlg, IDC_PUBGCOLOR), IsDlgButtonChecked(hwndDlg, IDC_COLORCUSTOM));
			EnableWindow(GetDlgItem(hwndDlg, IDC_PUTEXTCOLOR), IsDlgButtonChecked(hwndDlg, IDC_COLORCUSTOM));
			break;
		case WM_USER + 11:
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM));
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SPIN), IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM));
			break;
		case WM_USER + 12:
			EnableWindow(GetDlgItem(hwndDlg, IDC_IFCHANGED), IsDlgButtonChecked(hwndDlg, IDC_ONCONNECT));
			break;
#ifdef CUSTOMBUILD_OSDSUPPORT
		case WM_USER + 13:
		{
			BOOL state = !IsDlgButtonChecked(hwndDlg, IDC_USEOSD);
			if (state) SendMessage(hwndDlg, WM_USER + 10, 0, 0);
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_PUBGCOLOR), state);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PUTEXTCOLOR), state);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLORTYPE), state);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LEFTCLICK), state);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTCLICK), state);
			break;
		}
#endif
		case WM_COMMAND:
		{
			if ((LOWORD(wParam) == IDC_POPUPTEXT || LOWORD(wParam) == IDC_DELAY)
				&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
				return 0;
			switch (LOWORD(wParam)) //(HIWORD(wParam) == BN_CLICKED)
			{
#ifdef CUSTOMBUILD_OSDSUPPORT
				case IDC_USEOSD:
					SendMessage(hwndDlg, WM_USER + 13, 0, 0);
					break;
#endif
				case IDC_COLORFROMPU:
				case IDC_COLORWINDOWS:
				case IDC_COLORCUSTOM:
					SendMessage(hwndDlg, WM_USER + 10, 0, 0);
					break;
				case IDC_DELAYFROMPU:
				case IDC_DELAYPERMANENT:
				case IDC_DELAYCUSTOM:
					SendMessage(hwndDlg, WM_USER + 11, 0, 0);
					break;
				case IDC_ONCONNECT:
					SendMessage(hwndDlg, WM_USER + 12, 0, 0);
					break;
				case IDC_PREVIEW: 
				{
					STATUSMSGINFO temp_smi;
					SMCNOTIFY_PUOPTIONS temp_puo;

					ZeroMemory(&temp_smi, sizeof(temp_smi));
					temp_smi.hContact = NULL;
					temp_smi.cust = (TCHAR*)mir_alloc0(8 * sizeof(TCHAR));
					lstrcpy(temp_smi.cust, TranslateT("Contact"));
					temp_smi.oldstatusmsg = (TCHAR*)mir_alloc0(19 * sizeof(TCHAR));
					lstrcpy(temp_smi.oldstatusmsg, TranslateT("Old status message"));
					temp_smi.newstatusmsg = (TCHAR*)mir_alloc0(19 * sizeof(TCHAR));
					lstrcpy(temp_smi.newstatusmsg, TranslateT("New status message"));

					ZeroMemory(&temp_puo, sizeof(temp_puo));
					if (IsDlgButtonChecked(hwndDlg, IDC_DELAYFROMPU))
						temp_puo.bDelayType = POPUP_DELAY_DEFAULT;
					else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM))
					{
						temp_puo.bDelayType = POPUP_DELAY_CUSTOM;
						temp_puo.dDelay = GetDlgItemInt(hwndDlg,IDC_DELAY, NULL, FALSE);
					}
					else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYPERMANENT))
						temp_puo.bDelayType = POPUP_DELAY_PERMANENT;
					if (IsDlgButtonChecked(hwndDlg, IDC_COLORFROMPU))
						temp_puo.bColorType = POPUP_COLOR_DEFAULT;
					else if (IsDlgButtonChecked(hwndDlg, IDC_COLORWINDOWS))
						temp_puo.bColorType = POPUP_COLOR_WINDOWS;
					else if (IsDlgButtonChecked(hwndDlg, IDC_COLORCUSTOM))
					{
						temp_puo.bColorType = POPUP_COLOR_CUSTOM;
						temp_puo.colBack = SendDlgItemMessage(hwndDlg,IDC_PUBGCOLOR,CPM_GETCOLOUR,0,0);
						temp_puo.colText = SendDlgItemMessage(hwndDlg,IDC_PUTEXTCOLOR,CPM_GETCOLOUR,0,0);
					}
					GetDlgItemText(hwndDlg, IDC_POPUPTEXT, temp_puo.text, TEMPLATEMAXLEN);

					PopupNotify(&temp_smi, &temp_puo);
					MIR_FREE(temp_smi.cust);
					MIR_FREE(temp_smi.oldstatusmsg);
					MIR_FREE(temp_smi.newstatusmsg);
					return 0;
				}
			}
			break;
		}
	}

	return SaveOptsDlgProc(popupsControls, MAX_REGS(popupsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}

extern int OptionsInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;

	if(ServiceExists(MS_POPUP_ADDPOPUPEX)
#ifdef UNICODE
		|| ServiceExists(MS_POPUP_ADDPOPUPW)
#endif
		)
	{
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP);
		odp.ptszGroup = TranslateT("Popups");
		odp.ptszTitle = TranslateT(PLUGIN_NAME);
		odp.pfnDlgProc = PopupOptionsDlgProc;
//		odp.expertOnlyControls = popupsExpertControls;
//		odp.nExpertOnlyControls = sizeof(popupsExpertControls);
		CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	}

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.ptszGroup = TranslateT("History");
	odp.ptszTitle = TranslateT(PLUGIN_NAME);
	odp.ptszTab = TranslateT("General");
	odp.pfnDlgProc = OptionsDlgProc;
//	odp.expertOnlyControls = optionsExpertControls;
//	odp.nExpertOnlyControls = sizeof(optionsExpertControls);
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_EXPERTONLY;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_ADVANCED);
	odp.ptszTab = TranslateT("Advanced");
	odp.pfnDlgProc = AdvancedDlgProc;
//	odp.expertOnlyControls = optionsExpertControls;
//	odp.nExpertOnlyControls = sizeof(optionsExpertControls);
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_IGNORE);
	odp.ptszTab = TranslateT("Ignore");
	odp.pfnDlgProc = IgnoreDlgProc;
//	odp.expertOnlyControls = optionsExpertControls;
//	odp.nExpertOnlyControls = sizeof(optionsExpertControls);
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}
