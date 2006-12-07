/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci

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


#include "commons.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;

Options opts;


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PlayersDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ &opts.enable_sending,				CONTROL_CHECKBOX,	IDC_ENABLE_SEND,				"EnableSend", TRUE },
	{ &opts.enable_menu_item,			CONTROL_CHECKBOX,	IDC_ENABLE_MENU,				"EnableMenu", TRUE },
	{ &opts.templ,						CONTROL_TEXT,		IDC_TEMPLATE,					"Template", (DWORD) _T("%title% - %artist%") },
	{ &opts.unknown,					CONTROL_TEXT,		IDC_UNKNOWN,					"Unknown", (DWORD) _T("<Unknown>"), 0, 0, 128 },
	{ &opts.override_contact_template,	CONTROL_CHECKBOX,	IDC_OVERRIDE_CONTACTS_TEMPLATE,	"OverrideContactsTemplate", FALSE},
	{ &opts.show_adv_icon,				CONTROL_CHECKBOX,	IDC_SHOW_ADV_ICON,				"ShowAdvancedIcon", FALSE},
	{ &opts.adv_icon_slot,				CONTROL_COMBO,		IDC_ADV_ICON,					"AdvancedIconSlot", 1}
};

static UINT optionsExpertControls[] = { 
	IDC_FORMAT_G, IDC_TEMPLATE_L, IDC_TEMPLATE, IDC_OVERRIDE_CONTACTS_TEMPLATE,
	IDC_VARS_L, IDC_ARTIST_L, IDC_ALBUM_L, IDC_TITLE_L, IDC_TRACK_L, IDC_YEAR_L, IDC_GENRE_L, IDC_LENGTH_L,
	IDC_PLAYER_L, IDC_TYPE_L, IDC_UNKNOWN_L, IDC_UNKNOWN, IDC_CONTACTS_G, IDC_SHOW_ADV_ICON, IDC_ADV_ICON
};

static OptPageControl playersControls[] = { 
	{ &players[WATRACK]->enabled,	CONTROL_CHECKBOX,	IDC_WATRACK,		"GetInfoFromWATrack", FALSE },
	{ &opts.time_to_pool,			CONTROL_SPIN,		IDC_POLL_TIMER,		"TimeToPool", (WORD) 5, IDC_POLL_TIMER_SPIN, (WORD) 1, (WORD) 255 },
	{ &players[WINAMP]->enabled,	CONTROL_CHECKBOX,	IDC_WINAMP,			"EnableWinamp", TRUE },
	{ &players[ITUNES]->enabled,	CONTROL_CHECKBOX,	IDC_ITUNES,			"EnableITunes", TRUE },
	{ &players[WMP]->enabled,		CONTROL_CHECKBOX,	IDC_WMP,			"EnableWMP", TRUE },
	{ &opts.enable_other_players,	CONTROL_CHECKBOX,	IDC_OTHER,			"EnableOtherPlayers", TRUE },
	{ &opts.enable_code_injection,	CONTROL_CHECKBOX,	IDC_CODE_INJECTION,	"EnableCodeInjection", TRUE }
};



// Functions //////////////////////////////////////////////////////////////////////////////////////


int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=0;
	odp.hInstance=hInst;
	odp.ptszGroup = TranslateT("Status");
	odp.ptszTitle = TranslateT("Listening info");
	odp.ptszTab = TranslateT("General");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
    odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.expertOnlyControls = optionsExpertControls;
	odp.nExpertOnlyControls = MAX_REGS(optionsExpertControls);
	odp.nIDBottomSimpleControl = IDC_LISTENING_G;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.ptszTab = TranslateT("Players");
	odp.pfnDlgProc = PlayersDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PLAYERS);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_EXPERTONLY;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


void InitOptions()
{
	LoadOptions();

	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}


void DeInitOptions()
{
	UnhookEvent(hOptHook);
}


void LoadOptions()
{
	LoadOpts(optionsControls, MAX_REGS(optionsControls), MODULE_NAME);
	LoadOpts(playersControls, MAX_REGS(playersControls), MODULE_NAME);
}


static void OptionsEnableDisableCtrls(HWND hwndDlg)
{
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_SEND);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_MENU), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_FORMAT_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATE_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_OVERRIDE_CONTACTS_TEMPLATE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_VARS_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ARTIST_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ALBUM_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TITLE_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TRACK_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_YEAR_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_GENRE_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LENGTH_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PLAYER_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TYPE_L), enabled);
}


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	BOOL ret;
	if (msg != WM_INITDIALOG)
		ret = SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			// Init combo
			int total = 0, first = 0;
			if (ServiceExists(MS_CLUI_GETCAPS))
			{
				total = CallService(MS_CLUI_GETCAPS, 0, CLUIF2_EXTRACOLUMNCOUNT);
				first = CallService(MS_CLUI_GETCAPS, 0, CLUIF2_USEREXTRASTART);
			}

			SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) _T("1"));
			SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) _T("2"));

			if (total > 0)
			{
				TCHAR tmp[10];
				for (int i = first; i <= total; i++)
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) _itot(i - first + 3, tmp, 10));
			}

			ret = SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
			OptionsEnableDisableCtrls(hwndDlg);

			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDC_ENABLE_SEND:
				{
					if (HIWORD(wParam) == BN_CLICKED)
						OptionsEnableDisableCtrls(hwndDlg);

					break;
				}
			}
			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				StartTimer();
			}

			break;
		}
	}

	return ret;
}


int playerDlgs[] = {
	WINAMP, IDC_WINAMP, 
	WMP, IDC_WMP, 
	ITUNES, IDC_ITUNES
};


static void PlayersEnableDisableCtrls(HWND hwndDlg)
{
	BOOL watrack_found = ServiceExists(MS_WAT_GETMUSICINFO);
	EnableWindow(GetDlgItem(hwndDlg, IDC_WATRACK), watrack_found);

	BOOL enabled = !IsDlgButtonChecked(hwndDlg, IDC_WATRACK) || !watrack_found;
	EnableWindow(GetDlgItem(hwndDlg, IDC_PLAYERS_L), enabled);

	BOOL needPoll = FALSE;
	for (int i = 0; i < MAX_REGS(playerDlgs); i += 2)
	{
		EnableWindow(GetDlgItem(hwndDlg, playerDlgs[i+1]), enabled);
		if (players[playerDlgs[i]]->needPoll && IsDlgButtonChecked(hwndDlg, playerDlgs[i+1]))
			needPoll = TRUE;
	}

	EnableWindow(GetDlgItem(hwndDlg, IDC_OTHER), enabled);

	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_L), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_SPIN), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_S_L), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CODE_INJECTION), enabled);
}

static BOOL CALLBACK PlayersDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	BOOL ret = SaveOptsDlgProc(playersControls, MAX_REGS(playersControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			PlayersEnableDisableCtrls(hwndDlg);

			break;
		}
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
				PlayersEnableDisableCtrls(hwndDlg);
			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				EnableDisablePlayers();
				StartTimer();
			}

			break;
		}
	}

	return ret;
}

