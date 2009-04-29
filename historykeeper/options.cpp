/*	
Copyright (C) 2006-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

// XXX
// All this file is one BIG HACK

#include "commons.h"

#include <crtdbg.h>


// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;


static BOOL CALLBACK OptionsDlgProc(int type, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PopupsDlgProc(int type, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SpeakDlgProc(int type, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK NotificationsDlgProc(int type, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define HACK(i) \
		static BOOL CALLBACK OptionsDlgProc ## i (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) \
		{ \
			return OptionsDlgProc(i, hwndDlg, msg, wParam, lParam); \
		} \
		static BOOL CALLBACK PopupsDlgProc ## i (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) \
		{ \
			return PopupsDlgProc(i, hwndDlg, msg, wParam, lParam); \
		} \
		static BOOL CALLBACK SpeakDlgProc ## i (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) \
		{ \
			return SpeakDlgProc(i, hwndDlg, msg, wParam, lParam); \
		} \
		static BOOL CALLBACK NotificationsDlgProc ## i (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) \
		{ \
			return NotificationsDlgProc(i, hwndDlg, msg, wParam, lParam); \
		} \
		static BOOL AllowProtocol ## i(const char *proto) \
		{ \
			return AllowProtocol(i, proto); \
		}

HACK(0);
HACK(1);
HACK(2);
HACK(3);
HACK(4);
HACK(5);
HACK(6);
HACK(7);

DLGPROC NotificationsDlgProcArr[] = { NotificationsDlgProc0, NotificationsDlgProc1, NotificationsDlgProc2, NotificationsDlgProc3, NotificationsDlgProc4, NotificationsDlgProc5, NotificationsDlgProc6, NotificationsDlgProc7 };
DLGPROC OptionsDlgProcArr[] = { OptionsDlgProc0, OptionsDlgProc1, OptionsDlgProc2, OptionsDlgProc3, OptionsDlgProc4, OptionsDlgProc5, OptionsDlgProc6, OptionsDlgProc7 };
DLGPROC PopupsDlgProcArr[] = { PopupsDlgProc0, PopupsDlgProc1, PopupsDlgProc2, PopupsDlgProc3, PopupsDlgProc4, PopupsDlgProc5, PopupsDlgProc6, PopupsDlgProc7 };
DLGPROC SpeakDlgProcArr[] = { SpeakDlgProc0, SpeakDlgProc1, SpeakDlgProc2, SpeakDlgProc3, SpeakDlgProc4, SpeakDlgProc5, SpeakDlgProc6, SpeakDlgProc7 };
FPAllowProtocol AllowProtocolArr[] = { AllowProtocol0, AllowProtocol1, AllowProtocol2, AllowProtocol3, AllowProtocol4, AllowProtocol5, AllowProtocol6, AllowProtocol7 };



#define OPTIONS_CONTROLS_SIZE 9
static OptPageControl optionsControls[NUM_TYPES][OPTIONS_CONTROLS_SIZE] = {0};

#define POPUPS_CONTROLS_SIZE 14
static OptPageControl popupsControls[NUM_TYPES][POPUPS_CONTROLS_SIZE] = {0};

#define SPEAK_CONTROLS_SIZE 4
static OptPageControl speakControls[NUM_TYPES][SPEAK_CONTROLS_SIZE] = {0};

static UINT popupsExpertControls[] = { 
	IDC_COLOURS_G, IDC_BGCOLOR, IDC_BGCOLOR_L, IDC_TEXTCOLOR, IDC_TEXTCOLOR_L, IDC_WINCOLORS, IDC_DEFAULTCOLORS, 
	IDC_DELAY_G, IDC_DELAYFROMPU, IDC_DELAYCUSTOM, IDC_DELAYPERMANENT, IDC_DELAY, IDC_DELAY_SPIN,
	IDC_ACTIONS_G, IDC_RIGHT_ACTION_L, IDC_RIGHT_ACTION, IDC_LEFT_ACTION_L, IDC_LEFT_ACTION,
	IDC_PREV
};


Options opts[NUM_TYPES];


// Functions //////////////////////////////////////////////////////////////////////////////////////


BOOL HasPopups()
{
	return ServiceExists(MS_POPUP_ADDPOPUPEX)
#ifdef UNICODE
		|| ServiceExists(MS_POPUP_ADDPOPUPW)
#endif
	;
}


BOOL HasOldSpeak()
{
	return ServiceExists(MS_SPEAK_SAY_A) && !ServiceExists(MS_SPEAK_SAYEX);
}


int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszGroup = "History";
	odp.flags = ODPF_BOLDGROUPS | ODPF_EXPERTONLY;

	_ASSERT(MAX_REGS(OptionsDlgProcArr) == NUM_TYPES);
	for (int i = 0; i < NUM_TYPES; i++) 
	{
		odp.pszTitle = types[i].description;

		if (types[i].canBeRemoved)
			odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_REM);
		else
			odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_NOREM);
		odp.pszTab = "General";
		odp.pfnDlgProc = OptionsDlgProcArr[i];
		CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

		odp.pszTemplate = MAKEINTRESOURCEA(IDD_NOTIFICATIONS);
		odp.pszTab = "Notifications";
		odp.pfnDlgProc = NotificationsDlgProcArr[i];
		CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	}


	_ASSERT(MAX_REGS(PopupsDlgProcArr) == NUM_TYPES);
	if (HasPopups())
	{
		ZeroMemory(&odp,sizeof(odp));
		odp.cbSize = sizeof(odp);
		odp.hInstance = hInst;
		odp.pszGroup = Translate("Popups");
		odp.flags = ODPF_BOLDGROUPS;
		odp.expertOnlyControls = popupsExpertControls;
		odp.nExpertOnlyControls = MAX_REGS(popupsExpertControls);
		odp.nIDBottomSimpleControl = IDC_TRACK_G;

		for (int i = 0; i < NUM_TYPES; i++) 
		{
			if (types[i].canBeRemoved)
				odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPS_REM);
			else
				odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPS_NOREM);

			char tmp[128];
			mir_snprintf(tmp, MAX_REGS(tmp), Translate("%s Change"), types[i].description);

			odp.pszTitle = tmp;
			odp.pfnDlgProc = PopupsDlgProcArr[i];
			CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) &odp);
		}
	}

	_ASSERT(MAX_REGS(SpeakDlgProcArr) == NUM_TYPES);
	if (HasOldSpeak())
	{
		ZeroMemory(&odp,sizeof(odp));
		odp.cbSize = sizeof(odp);
		odp.hInstance = hInst;
		odp.pszGroup = Translate("Speak");
		odp.flags = ODPF_BOLDGROUPS;

		for (int i = 0; i < NUM_TYPES; i++) 
		{
			if (types[i].canBeRemoved)
				odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPEAK_REM);
			else
				odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPEAK_NOREM);

			char tmp[128];
			mir_snprintf(tmp, MAX_REGS(tmp), Translate("%s Change"), types[i].description);

			odp.pszTitle = tmp;
			odp.pfnDlgProc = SpeakDlgProcArr[i];
			CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) &odp);
		}
	}

	return 0;
}



void InitOptions()
{
	static TCHAR fileChangeTemplates[NUM_TYPES][128];
	static TCHAR fileRemoveTemplates[NUM_TYPES][128];
	static TCHAR changeTemplates[NUM_TYPES][128];
	static TCHAR removeTemplates[NUM_TYPES][128];
	static TCHAR speakChangeTemplates[NUM_TYPES][128];
	static TCHAR speakRemoveTemplates[NUM_TYPES][128];
	static char optSet[NUM_TYPES][OPTIONS_CONTROLS_SIZE][64];
	static char popSet[NUM_TYPES][POPUPS_CONTROLS_SIZE][64];
	static char speakSet[NUM_TYPES][POPUPS_CONTROLS_SIZE][64];

	for (int i = 0; i < NUM_TYPES; i++) 
	{
		TCHAR *tmp = mir_a2t(types[i].description);
		CharLower(tmp);

		// Options page
		if (types[i].defs.change_template != NULL)
			mir_sntprintf(&fileChangeTemplates[i][0], 128, _T("[%%date%%] %%contact%% ") _T(TCHAR_STR_PARAM), types[i].defs.change_template);
		else
			mir_sntprintf(&fileChangeTemplates[i][0], 128, _T("[%%date%%] %%contact%% changed his/her %s to %%new%% (was %%old%%)"), tmp);

		if (types[i].defs.remove_template != NULL)
			mir_sntprintf(&fileRemoveTemplates[i][0], 128, _T("[%%date%%] %%contact%% ") _T(TCHAR_STR_PARAM), types[i].defs.remove_template);
		else
			mir_sntprintf(&fileRemoveTemplates[i][0], 128, _T("[%%date%%] %%contact%% removed his/her %s (was %%old%%)"), tmp);

		OptPageControl opt[] = {
			{ &opts[i].ttw,						CONTROL_SPIN,			IDC_NWAIT,					"Delay", types[i].defs.ttw, IDC_NWAIT_SPIN, (WORD) 0, (WORD) 120 },
			{ &opts[i].track_only_not_offline,	CONTROL_CHECKBOX,		IDC_ONLY_NOT_OFFLINE,		"TrackOnlyWhenNotOffline", types[i].defs.track_only_not_offline },
			{ &opts[i].dont_notify_on_connect,	CONTROL_CHECKBOX,		IDC_DONT_NOTIFY_ON_CONNECT,	"DontNotifyOnConnect", TRUE },
			{ NULL,								CONTROL_PROTOCOL_LIST,	IDC_PROTOCOLS,				"%sEnabled", TRUE, (int) AllowProtocolArr[i] },
			{ &opts[i].file_name,				CONTROL_FILE,			IDC_FILENAME,				"FileName", (DWORD) _T("Log\\history_keeper.log") },
			{ &opts[i].file_track_changes,		CONTROL_CHECKBOX,		IDC_TRACK_CHANGE,			"FileTrackChanges", FALSE },
			{ &opts[i].file_template_changed,	CONTROL_TEXT,			IDC_CHANGED,				"FileTemplateChanged", (DWORD) &fileChangeTemplates[i][0] },
			{ &opts[i].file_track_removes,		CONTROL_CHECKBOX,		IDC_TRACK_REMOVE,			"FileTrackRemoves", FALSE },
			{ &opts[i].file_template_removed,	CONTROL_TEXT,			IDC_REMOVED,				"FileTemplateRemoved", (DWORD) &fileRemoveTemplates[i][0] },
		};

		_ASSERT(MAX_REGS(opt) == OPTIONS_CONTROLS_SIZE);
		int j;
		for(j = 0; j < OPTIONS_CONTROLS_SIZE; j++)
		{
			mir_snprintf(&optSet[i][j][0], 64, "%s_%s", types[i].name, opt[j].setting);
			opt[j].setting = &optSet[i][j][0];
		}
		memcpy(&optionsControls[i][0], &opt, sizeof(opt));

		// Popups page
		if (HasPopups())
		{
			if (types[i].defs.change_template_popup != NULL)
				mir_sntprintf(&changeTemplates[i][0], 128, types[i].defs.change_template_popup, tmp);
			else if (types[i].defs.change_template != NULL)
				mir_sntprintf(&changeTemplates[i][0], 128, _T(TCHAR_STR_PARAM), types[i].defs.change_template);
			else
				mir_sntprintf(&changeTemplates[i][0], 128, _T("changed his/her %s to %%new%% (was %%old%%)"), tmp);

			if (types[i].defs.remove_template_popup != NULL)
				mir_sntprintf(&removeTemplates[i][0], 128, types[i].defs.remove_template_popup, tmp);
			else if (types[i].defs.remove_template != NULL)
				mir_sntprintf(&removeTemplates[i][0], 128, _T(TCHAR_STR_PARAM), types[i].defs.remove_template);
			else
				mir_sntprintf(&removeTemplates[i][0], 128, _T("removed his/her %s (was %%old%%)"), tmp);

			OptPageControl pops[] = {
				{ &opts[i].popup_track_changes,			CONTROL_CHECKBOX,	IDC_TRACK_CHANGE,	"PopupsTrackChanges", TRUE },
				{ &opts[i].popup_template_changed,		CONTROL_TEXT,		IDC_CHANGED,		"PopupsTemplateChanged", (DWORD) &changeTemplates[i][0] },
				{ &opts[i].popup_track_removes,			CONTROL_CHECKBOX,	IDC_TRACK_REMOVE,	"PopupsTrackRemoves", TRUE },
				{ &opts[i].popup_template_removed,		CONTROL_TEXT,		IDC_REMOVED,		"PopupsTemplateRemoved", (DWORD) &removeTemplates[i][0] },
				{ &opts[i].popup_bkg_color,				CONTROL_COLOR,		IDC_BGCOLOR,		"PopupsBgColor", RGB(255,255,255) },
				{ &opts[i].popup_text_color,			CONTROL_COLOR,		IDC_TEXTCOLOR,		"PopupsTextColor", RGB(0,0,0) },
				{ &opts[i].popup_use_win_colors,		CONTROL_CHECKBOX,	IDC_WINCOLORS,		"PopupsWinColors", FALSE },
				{ &opts[i].popup_use_default_colors,	CONTROL_CHECKBOX,	IDC_DEFAULTCOLORS,	"PopupsDefaultColors", FALSE },
				{ &opts[i].popup_delay_type,			CONTROL_RADIO,		IDC_DELAYFROMPU,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_DEFAULT },
				{ NULL,									CONTROL_RADIO,		IDC_DELAYCUSTOM,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_CUSTOM },
				{ NULL,									CONTROL_RADIO,		IDC_DELAYPERMANENT,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_PERMANENT },
				{ &opts[i].popup_timeout,				CONTROL_SPIN,		IDC_DELAY,			"PopupsTimeout", 10, IDC_DELAY_SPIN, (WORD) 1, (WORD) 255 },
				{ &opts[i].popup_right_click_action,	CONTROL_COMBO,		IDC_RIGHT_ACTION,	"PopupsRightClick", POPUP_ACTION_CLOSEPOPUP },
				{ &opts[i].popup_left_click_action,		CONTROL_COMBO,		IDC_LEFT_ACTION,	"PopupsLeftClick", POPUP_ACTION_OPENHISTORY }
			};
			_ASSERT(MAX_REGS(pops) == POPUPS_CONTROLS_SIZE);
			for(j = 0; j < POPUPS_CONTROLS_SIZE; j++)
			{
				mir_snprintf(&popSet[i][j][0], 64, "%s_%s", types[i].name, pops[j].setting);
				pops[j].setting = &popSet[i][j][0];
			}
			memcpy(&popupsControls[i][0], &pops, sizeof(pops));
		}

	    if (HasOldSpeak())
		{
			// Speak pages
			if (types[i].defs.change_template != NULL)
				mir_sntprintf(&speakChangeTemplates[i][0], 128, _T("%%contact%% ") _T(TCHAR_STR_PARAM), types[i].defs.change_template);
			else
				mir_sntprintf(&speakChangeTemplates[i][0], 128, TranslateT("%%contact%% changed his/her %s to %%new%%"), tmp);

			if (types[i].defs.remove_template != NULL)
				mir_sntprintf(&speakRemoveTemplates[i][0], 128, _T("%%contact%% ") _T(TCHAR_STR_PARAM), types[i].defs.remove_template);
			else
				mir_sntprintf(&speakRemoveTemplates[i][0], 128, _T("%%contact%% removed his/her %s"), tmp);

			OptPageControl opt[] = {
				{ &opts[i].speak_track_changes,		CONTROL_CHECKBOX,		IDC_TRACK_CHANGE,		"SpeakTrackChanges", FALSE },
				{ &opts[i].speak_template_changed,	CONTROL_TEXT,			IDC_CHANGED,			"SpeakTemplateChanged", (DWORD) &speakChangeTemplates[i][0] },
				{ &opts[i].speak_track_removes,		CONTROL_CHECKBOX,		IDC_TRACK_REMOVE,		"SpeakTrackRemoves", FALSE },
				{ &opts[i].speak_template_removed,	CONTROL_TEXT,			IDC_REMOVED,			"SpeakTemplateRemoved", (DWORD) &speakRemoveTemplates[i][0] },
			};

			_ASSERT(MAX_REGS(opt) == SPEAK_CONTROLS_SIZE);
			int j;
			for(j = 0; j < SPEAK_CONTROLS_SIZE; j++)
			{
				mir_snprintf(&speakSet[i][j][0], 64, "%s_%s", types[i].name, opt[j].setting);
				opt[j].setting = &speakSet[i][j][0];
			}
			memcpy(&speakControls[i][0], &opt, sizeof(opt));
		}

		mir_free(tmp);
	}

	LoadOptions();

	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}


void DeInitOptions()
{
	UnhookEvent(hOptHook);
}


void LoadOptions()
{
	for (int i = 0; i < NUM_TYPES; i++) 
	{
		LoadOpts(&optionsControls[i][0], OPTIONS_CONTROLS_SIZE, MODULE_NAME);
		if (HasPopups())
			LoadOpts(&popupsControls[i][0], POPUPS_CONTROLS_SIZE, MODULE_NAME);
		if (HasOldSpeak())
			LoadOpts(&speakControls[i][0], SPEAK_CONTROLS_SIZE, MODULE_NAME);
	}
}


static BOOL CALLBACK OptionsDlgProc(int type, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			TCHAR *desc = mir_a2t(types[type].description);
			CharLower(desc);

			TCHAR tmp[256];
			mir_sntprintf(tmp, MAX_REGS(tmp), TranslateT("Log when contacts change their %s"), desc);
			SetDlgItemText(hwndDlg, IDC_TRACK_CHANGE, tmp);
			mir_sntprintf(tmp, MAX_REGS(tmp), TranslateT("Log when contacts remove their %s"), desc);
			SetDlgItemText(hwndDlg, IDC_TRACK_REMOVE, tmp);

			mir_free(desc);
			
			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDC_SELECT_FILE:
				{
					if (HIWORD(wParam) == BN_CLICKED)
					{
						TCHAR file[1024] = _T("");
						GetDlgItemText(hwndDlg, IDC_FILENAME, file, 1024);

				        OPENFILENAME ofn = {0};
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_NOREADONLYRETURN;
						ofn.hwndOwner = hwndDlg;
						ofn.lpstrFile = file;
						ofn.nMaxFile = MAX_REGS(file);
						ofn.lpstrFilter = _T("Log Files (*.log;*.txt)\0*.LOG;*.TXT\0All Files (*.*)\0*.*\0\0");
						ofn.lpstrDefExt = _T(".log");
						ofn.hInstance = hInst;

						if (GetOpenFileName(&ofn)) 
							SetDlgItemText(hwndDlg, IDC_FILENAME, file);
					}
					break;
				}
			}
			break;
		}
	}

	return SaveOptsDlgProc(&optionsControls[type][0], OPTIONS_CONTROLS_SIZE, MODULE_NAME, hwndDlg, msg, wParam, lParam);
}


static BOOL CALLBACK SpeakDlgProc(int type, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			TCHAR *desc = mir_a2t(types[type].description);
			CharLower(desc);

			TCHAR tmp[256];
			mir_sntprintf(tmp, MAX_REGS(tmp), TranslateT("Announce when contacts change their %s"), desc);
			SetDlgItemText(hwndDlg, IDC_TRACK_CHANGE, tmp);
			mir_sntprintf(tmp, MAX_REGS(tmp), TranslateT("Announce when contacts remove their %s"), desc);
			SetDlgItemText(hwndDlg, IDC_TRACK_REMOVE, tmp);

			mir_free(desc);
			
			break;
		}
	}
	return SaveOptsDlgProc(&speakControls[type][0], SPEAK_CONTROLS_SIZE, MODULE_NAME, hwndDlg, msg, wParam, lParam);
}


static void PopupsEnableDisableCtrls(HWND hwndDlg)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGED_L), IsDlgButtonChecked(hwndDlg, IDC_TRACK_CHANGE));
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGED), IsDlgButtonChecked(hwndDlg, IDC_TRACK_CHANGE));

	EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVED_L), IsDlgButtonChecked(hwndDlg, IDC_TRACK_REMOVE));
	EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVED), IsDlgButtonChecked(hwndDlg, IDC_TRACK_REMOVE));
	
	EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), 
			!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) && 
			!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR), 
			!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) && 
			!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULTCOLORS), 
			!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_WINCOLORS), 
			!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));

	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), 
			IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM));
}


static BOOL CALLBACK PopupsDlgProc(int type, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			TCHAR *desc = mir_a2t(types[type].description);
			CharLower(desc);

			TCHAR tmp[256];
			mir_sntprintf(tmp, MAX_REGS(tmp), TranslateT("Show when contacts change their %s"), desc);
			SetDlgItemText(hwndDlg, IDC_TRACK_CHANGE, tmp);
			mir_sntprintf(tmp, MAX_REGS(tmp), TranslateT("Show when contacts remove their %s"), desc);
			SetDlgItemText(hwndDlg, IDC_TRACK_REMOVE, tmp);

			mir_free(desc);

			SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Do nothing"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Close popup"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Show history"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Open message window"));

			SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Do nothing"));
			SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Close popup"));
			SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Show history"));
			SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Open message window"));

			// Needs to be called here in this case
			BOOL ret = SaveOptsDlgProc(&popupsControls[type][0], POPUPS_CONTROLS_SIZE, MODULE_NAME, hwndDlg, msg, wParam, lParam);

			PopupsEnableDisableCtrls(hwndDlg);

			return ret;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDC_TRACK_REMOVE:
				case IDC_TRACK_CHANGE:
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
					Options op;

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

					TCHAR tmp[1024];
					GetDlgItemText(hwndDlg, IDC_CHANGED, tmp, 1024);

					ShowTestPopup(type, TranslateT("Contact Name"), tmp, &op);

					break;
				}
			}
			break;
		}
	}

	return SaveOptsDlgProc(&popupsControls[type][0], POPUPS_CONTROLS_SIZE, MODULE_NAME, hwndDlg, msg, wParam, lParam);
}

static struct {
	int ico;
	int ctrl;
	int item;
} data[] = {
	{ IDI_HISTORY, IDC_HISTORY, LOG_HISTORY },
	{ IDI_FILE, IDC_FILE, LOG_FILE },
	{ IDI_POPUP, IDC_POPUP, NOTIFY_POPUP },
	{ IDI_SOUND, IDC_SOUND, NOTIFY_SOUND },
	{ IDI_SPEAK, IDC_SPEAK, NOTIFY_SPEAK }
};


static void SetAllContactIcons(int type, HWND hwndList)
{
	HANDLE hContact,hItem;

	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	do 
	{
		hItem=(HANDLE)SendMessage(hwndList,CLM_FINDCONTACT,(WPARAM)hContact,0);
		if(hItem) 
		{
			char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
			if (!ProtocolEnabled(type, proto))
			{
				SendMessage(hwndList,CLM_DELETEITEM,(WPARAM)hItem,0);
			}
			else
			{
				for (int i = 0; i < MAX_REGS(data); i++) {
					SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(i, 
						ItemEnabled(type, hContact, data[i].item) ? i+1 : 0));
				}
			}
		}
	} while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));
}


static void SetListGroupIcons(HWND hwndList,HANDLE hFirstItem,HANDLE hParentItem,int *groupChildCount)
{
	int typeOfFirst;
	int iconOn[5]={1,1,1,1,1};
	int childCount[5]={0,0,0,0,0},i;
	int iImage;
	HANDLE hItem,hChildItem;

	typeOfFirst=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);
	//check groups
	if(typeOfFirst==CLCIT_GROUP) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		hChildItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if(hChildItem) SetListGroupIcons(hwndList,hChildItem,hItem,childCount);
		for( i=0; i < MAX_REGS(iconOn); i++)
			if(iconOn[i] && SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,i)==0) iconOn[i]=0;
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}
	//check contacts
	if(typeOfFirst==CLCIT_CONTACT) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);
	while(hItem) {
		for ( i=0; i < MAX_REGS(iconOn); i++) {
			iImage=SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,i);
			if(iconOn[i] && iImage==0) iconOn[i]=0;
			if(iImage!=0xFF) childCount[i]++;
		}
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
	//set icons
	if (hParentItem != NULL) {
		for( i=0; i < MAX_REGS(iconOn); i++) {
			SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hParentItem,MAKELPARAM(i,childCount[i]?(iconOn[i]?i+1:0):0xFF));
			if(groupChildCount) groupChildCount[i]+=childCount[i];
		}
	}
}


static void SetAllChildIcons(HWND hwndList,HANDLE hFirstItem,int iColumn,int iImage)
{
	int typeOfFirst,iOldIcon;
	HANDLE hItem,hChildItem;

	typeOfFirst=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);
	//check groups
	if(typeOfFirst==CLCIT_GROUP) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		hChildItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if(hChildItem) SetAllChildIcons(hwndList,hChildItem,iColumn,iImage);
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}
	//check contacts
	if(typeOfFirst==CLCIT_CONTACT) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);
	while(hItem) {
		iOldIcon=SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,iColumn);
		if(iOldIcon!=0xFF && iOldIcon!=iImage) SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(iColumn,iImage));
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
}


static void ResetListOptions(HWND hwndList)
{
	int i;

	SendMessage(hwndList,CLM_SETBKBITMAP,0,(LPARAM)(HBITMAP)NULL);
	SendMessage(hwndList,CLM_SETBKCOLOR,GetSysColor(COLOR_WINDOW),0);
	SendMessage(hwndList,CLM_SETGREYOUTFLAGS,0,0);
	SendMessage(hwndList,CLM_SETLEFTMARGIN,4,0);
	SendMessage(hwndList,CLM_SETINDENT,10,0);
	SendMessage(hwndList,CLM_SETHIDEEMPTYGROUPS,1,0);
	for(i=0;i<=FONTID_MAX;i++)
		SendMessage(hwndList,CLM_SETTEXTCOLOR,i,GetSysColor(COLOR_WINDOWTEXT));
	SetWindowLong(hwndList,GWL_STYLE,GetWindowLong(hwndList,GWL_STYLE)|CLS_SHOWHIDDEN);
}


static int ImageList_AddIcon_NotShared(HIMAGELIST hIml, int ico) 
{   
    HICON hTempIcon = LoadIcon(hInst, MAKEINTRESOURCE(ico));
    int res = ImageList_AddIcon(hIml, hTempIcon);
    //DestroyIcon(hTempIcon); 
    return res;
}


static BOOL CALLBACK NotificationsDlgProc(int type, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hItemAll;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			{
				HIMAGELIST hIml;
				hIml=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),(IsWinVerXPPlus()?ILC_COLOR32:ILC_COLOR16)|ILC_MASK,3,3);

				ImageList_AddIcon_NotShared(hIml, IDI_SMALLDOT);
				int i;
				for (i = 0; i < MAX_REGS(data); i++)
				{
					ImageList_AddIcon_NotShared(hIml, data[i].ico);
				}

				SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_SETEXTRAIMAGELIST,0,(LPARAM)hIml);

				for (i = 0; i < MAX_REGS(data); i++)
				{
					HICON hIco = ImageList_GetIcon(hIml, i+1, ILD_NORMAL);
					SendDlgItemMessage(hwndDlg, data[i].ctrl, STM_SETICON, (WPARAM) hIco, 0);
				}
			}

			ResetListOptions(GetDlgItem(hwndDlg,IDC_LIST));
			SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_SETEXTRACOLUMNS, MAX_REGS(data), 0);

			CLCINFOITEM cii={0};
			cii.cbSize=sizeof(cii);
			cii.flags=CLCIIF_GROUPFONT;
			cii.pszText=TranslateT("** All contacts **");
			hItemAll=(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_ADDINFOITEM,0,(LPARAM)&cii);

			SetAllContactIcons(type, GetDlgItem(hwndDlg,IDC_LIST));
			SetListGroupIcons(GetDlgItem(hwndDlg,IDC_LIST),(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETNEXTITEM,CLGN_ROOT,0),hItemAll,NULL);
			return TRUE;
		}
		case WM_SETFOCUS:
			SetFocus(GetDlgItem(hwndDlg,IDC_LIST));
			break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case IDC_LIST:
					switch (((LPNMHDR)lParam)->code)
					{
						case CLN_NEWCONTACT:
						case CLN_LISTREBUILT:
							SetAllContactIcons(type, GetDlgItem(hwndDlg,IDC_LIST));
							//fall through
						case CLN_CONTACTMOVED:
							SetListGroupIcons(GetDlgItem(hwndDlg,IDC_LIST),(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETNEXTITEM,CLGN_ROOT,0),hItemAll,NULL);
							break;
						case CLN_OPTIONSCHANGED:
							ResetListOptions(GetDlgItem(hwndDlg,IDC_LIST));
							break;
						case NM_CLICK:
						{	HANDLE hItem;
							NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
							DWORD hitFlags;
							int iImage;
							int itemType;

							// Make sure we have an extra column
							if (nm->iColumn == -1)
								break;

							// Find clicked item
							hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x,nm->pt.y));
							// Nothing was clicked
							if (hItem == NULL) break; 
							// It was not a visbility icon
							if (!(hitFlags & CLCHT_ONITEMEXTRA)) break;

							// Get image in clicked column
							iImage = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
							if (iImage == 0)
								iImage = nm->iColumn + 1;
							else
								iImage = 0;

							// Get item type (contact, group, etc...)
							itemType = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETITEMTYPE, (WPARAM)hItem, 0);

							// Update list
							if (itemType == CLCIT_CONTACT) 
							{	// A contact
								SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, iImage));
							}
							else if (itemType == CLCIT_INFO) 
							{
								if (hItem == hItemAll) 
									SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn, iImage);
							}
							else if (itemType == CLCIT_GROUP) 
							{	// A group
								hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
								if (hItem) 
									SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn, iImage);
							}
							// Update the all/none icons
							SetListGroupIcons(GetDlgItem(hwndDlg, IDC_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);

							// Activate Apply button
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							break;
						}
					}
					break;
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{	HANDLE hContact,hItem;
							int iImage;

							hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
							do {
								hItem=(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_FINDCONTACT,(WPARAM)hContact,0);
								if(hItem) 
								{
									for(int i=0; i < MAX_REGS(data); i++) {
										iImage=SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(i,0));
										EnableItem(type, hContact, data[i].item, iImage != 0);
									}
								}
							} while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));
							return TRUE;
						}
					}
					break;
			}
			break;
		case WM_DESTROY:
			{
				HIMAGELIST hIml=(HIMAGELIST)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGELIST,0,0);
				ImageList_Destroy(hIml);
			}
			break;
	}
	return FALSE;

}