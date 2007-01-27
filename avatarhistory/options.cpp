/*
Avatar History Plugin
 Copyright (C) 2006  Matthew Wild - Email: mwild1@gmail.com

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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "AvatarHistory.h"
#include <commctrl.h>
#include <prsht.h>
#include "../utils/mir_options.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

Options opts;


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ NULL,						CONTROL_CHECKBOX,		IDC_LOG_DISK,		"LogToDisk", AVH_DEF_LOGTODISK },
	{ &opts.log_old_style,		CONTROL_CHECKBOX,		IDC_OLD_STYLE,		"LogOldStyle", AVH_DEF_LOGOLDSTYLE },
	{ NULL,						CONTROL_CHECKBOX,		IDC_LOG_HISTORY,	"LogToHistory", AVH_DEF_LOGTOHISTORY },
	{ &opts.template_changed,	CONTROL_TEXT,			IDC_CHANGED,		"TemplateChanged", (DWORD) _T(DEFAULT_TEMPLATE_CHANGED) },
	{ &opts.track_removes,		CONTROL_CHECKBOX,		IDC_TRACK_REMOVE,	"TrackRemoves", TRUE },
	{ &opts.template_removed,	CONTROL_TEXT,			IDC_REMOVED,		"TemplateRemoved", (DWORD) _T(DEFAULT_TEMPLATE_REMOVED) },
	{ NULL,						CONTROL_PROTOCOL_LIST,	IDC_PROTOCOLS,		"%sEnabled", TRUE }
};


static OptPageControl popupsControls[] = { 
	{ NULL,									CONTROL_CHECKBOX,	IDC_POPUPS,			"AvatarPopups", AVH_DEF_AVPOPUPS },
	{ &opts.popup_bkg_color,				CONTROL_COLOR,		IDC_BGCOLOR,		"PopupsBgColor", AVH_DEF_POPUPBG },
	{ &opts.popup_text_color,				CONTROL_COLOR,		IDC_TEXTCOLOR,		"PopupsTextColor", AVH_DEF_POPUPFG },
	{ &opts.popup_use_win_colors,			CONTROL_CHECKBOX,	IDC_WINCOLORS,		"PopupsWinColors", FALSE },
	{ &opts.popup_use_default_colors,		CONTROL_CHECKBOX,	IDC_DEFAULTCOLORS,	"PopupsDefaultColors", AVH_DEF_DEFPOPUPS },
	{ &opts.popup_delay_type,				CONTROL_RADIO,		IDC_DELAYFROMPU,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_DEFAULT },
	{ NULL,									CONTROL_RADIO,		IDC_DELAYCUSTOM,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_CUSTOM },
	{ NULL,									CONTROL_RADIO,		IDC_DELAYPERMANENT,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_PERMANENT },
	{ &opts.popup_timeout,					CONTROL_SPIN,		IDC_DELAY,			"PopupsTimeout", 10, IDC_DELAY_SPIN, (WORD) 1, (WORD) 255 },
	{ &opts.popup_right_click_action,		CONTROL_COMBO,		IDC_RIGHT_ACTION,	"PopupsRightClick", POPUP_ACTION_CLOSEPOPUP },
	{ &opts.popup_left_click_action,		CONTROL_COMBO,		IDC_LEFT_ACTION,	"PopupsLeftClick", POPUP_ACTION_OPENAVATARHISTORY }
};

static UINT popupsExpertControls[] = { 
	IDC_COLOURS_G, IDC_BGCOLOR, IDC_BGCOLOR_L, IDC_TEXTCOLOR, IDC_TEXTCOLOR_L, IDC_WINCOLORS, IDC_DEFAULTCOLORS, 
	IDC_DELAY_G, IDC_DELAYFROMPU, IDC_DELAYCUSTOM, IDC_DELAYPERMANENT, IDC_DELAY, IDC_DELAY_SPIN,
	IDC_ACTIONS_G, IDC_RIGHT_ACTION_L, IDC_RIGHT_ACTION, IDC_LEFT_ACTION_L, IDC_LEFT_ACTION,
	IDC_PREV
};


// Functions //////////////////////////////////////////////////////////////////////////////////////


int OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=0;
	odp.hInstance=hInst;
	odp.ptszGroup = TranslateT("History"); // group to put your item under
	odp.ptszTitle = TranslateT("Avatar"); // name of the item
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
    odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_EXPERTONLY;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	if(ServiceExists(MS_POPUP_ADDPOPUPEX)
#ifdef UNICODE
		|| ServiceExists(MS_POPUP_ADDPOPUPW)
#endif
		)
	{
		ZeroMemory(&odp,sizeof(odp));
		odp.cbSize=sizeof(odp);
		odp.position=0;
		odp.hInstance=hInst;
		odp.ptszGroup = TranslateT("Popups");
		odp.ptszTitle = TranslateT("Avatar Change");
		odp.pfnDlgProc = PopupsDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPS);
		odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
		odp.expertOnlyControls = popupsExpertControls;
		odp.nExpertOnlyControls = MAX_REGS(popupsExpertControls);
		odp.nIDBottomSimpleControl = IDC_POPUPS;
		odp.nIDRightSimpleControl = IDC_POPUPS;
		CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	}

	return 0;
}


void LoadOptions()
{
	LoadOpts(optionsControls, MAX_REGS(optionsControls), MODULE_NAME);
	LoadOpts(popupsControls, MAX_REGS(popupsControls), MODULE_NAME);
}


static void OptionsEnableDisableCtrls(HWND hwndDlg)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVED_L), IsDlgButtonChecked(hwndDlg, IDC_TRACK_REMOVE));
	EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVED), IsDlgButtonChecked(hwndDlg, IDC_TRACK_REMOVE));

	EnableWindow(GetDlgItem(hwndDlg, IDC_OLD_STYLE), IsDlgButtonChecked(hwndDlg, IDC_LOG_DISK));
}


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	BOOL ret = SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			OptionsEnableDisableCtrls(hwndDlg);
			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDC_LOG_DISK:
				case IDC_TRACK_REMOVE:
				{
					if (HIWORD(wParam) == BN_CLICKED)
						OptionsEnableDisableCtrls(hwndDlg);

					break;
				}
			}
			break;
		}
	}

	return ret;
}


static void PopupsEnableDisableCtrls(HWND hwndDlg)
{
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_POPUPS);

	EnableWindow(GetDlgItem(hwndDlg, IDC_COLOURS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYFROMPU), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYCUSTOM), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYPERMANENT), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIONS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHT_ACTION_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHT_ACTION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LEFT_ACTION_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LEFT_ACTION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PREV), enabled);
	
	EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), enabled &&
			!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) && 
			!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR), enabled &&
			!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) && 
			!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULTCOLORS), enabled &&
			!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_WINCOLORS), enabled &&
			!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));

	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), enabled &&
			IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM));
}


static BOOL CALLBACK PopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Do nothing"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Close popup"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Show avatar history"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Show contact history"));

			SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Do nothing"));
			SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Close popup"));
			SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Show avatar history"));
			SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Show contact history"));

			// Needs to be called here in this case
			BOOL ret = SaveOptsDlgProc(popupsControls, MAX_REGS(popupsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

			PopupsEnableDisableCtrls(hwndDlg);

			return ret;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDC_POPUPS:
				case IDC_WINCOLORS:
				case IDC_DEFAULTCOLORS:
				case IDC_DELAYFROMPU:
				case IDC_DELAYPERMANENT:
				case IDC_DELAYCUSTOM:
				{
					if (HIWORD(wParam) == BN_CLICKED)
						PopupsEnableDisableCtrls(hwndDlg);

					break;
				}
				case IDC_PREV: 
				{
					Options op = opts;

					if (IsDlgButtonChecked(hwndDlg, IDC_DELAYFROMPU))
						op.popup_delay_type = POPUP_DELAY_DEFAULT;
					else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM))
						op.popup_delay_type = POPUP_DELAY_CUSTOM;
					else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYPERMANENT))
						op.popup_delay_type = POPUP_DELAY_PERMANENT;

					op.popup_timeout = GetDlgItemInt(hwndDlg,IDC_DELAY, NULL, FALSE);
					op.popup_bkg_color = SendDlgItemMessage(hwndDlg,IDC_BGCOLOR,CPM_GETCOLOUR,0,0);
					op.popup_text_color = SendDlgItemMessage(hwndDlg,IDC_TEXTCOLOR,CPM_GETCOLOUR,0,0);
					op.popup_use_win_colors = IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) != 0;
					op.popup_use_default_colors = IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS) != 0;

					ShowTestPopup(TranslateT("Test Contact"), TranslateT("Test description"), &op);

					break;
				}
			}
			break;
		}
	}

	return SaveOptsDlgProc(popupsControls, MAX_REGS(popupsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}
