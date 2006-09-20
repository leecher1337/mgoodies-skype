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
	{ &opts.override_contact_template,	CONTROL_CHECKBOX,	IDC_OVERRIDE_CONTACTS_TEMPLATE,	"OverrideContactsTemplate", FALSE}
};

static UINT optionsExpertControls[] = { 
	IDC_FORMAT_G, IDC_TEMPLATE_L, IDC_TEMPLATE, IDC_OVERRIDE_CONTACTS_TEMPLATE,
	IDC_VARS_L, IDC_ARTIST_L, IDC_ALBUM_L, IDC_TITLE_L, IDC_TRACK_L, IDC_YEAR_L, IDC_GENRE_L, IDC_LENGTH_L,
	IDC_PLAYER_L, IDC_TYPE_L
};

static OptPageControl playersControls[] = { 
	{ &opts.get_info_from_watrack,	CONTROL_CHECKBOX,	IDC_WATRACK,	"GetInfoFromWATrack", TRUE },
	{ &opts.time_to_pool,			CONTROL_SPIN,		IDC_POLL_TIMER,	"TimeToPool", (WORD) 5, IDC_POLL_TIMER_SPIN, (WORD) 1, (WORD) 255 },
	{ &players[WINAMP]->enabled,	CONTROL_CHECKBOX,	IDC_WINAMP,		"EnableWinamp", TRUE },
	{ &players[ITUNES]->enabled,	CONTROL_CHECKBOX,	IDC_ITUNES,		"EnableITunes", TRUE },
	{ &players[WMP]->enabled,		CONTROL_CHECKBOX,	IDC_WMP,		"EnableWMP", TRUE }
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

	if (!ServiceExists(MM_GETMUSICINFO))
		opts.get_info_from_watrack = FALSE;
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
	EnableWindow(GetDlgItem(hwndDlg, IDC_WATRACK), ServiceExists(MM_GETMUSICINFO));

	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_WATRACK);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PLAYERS_L), enabled);

	if (enabled)
	{
		BOOL needPoll = FALSE;
		for (int i = 0; i < MAX_REGS(playerDlgs); i += 2)
		{
			EnableWindow(GetDlgItem(hwndDlg, playerDlgs[i+1]), enabled);
			if (players[playerDlgs[i]]->needPoll && IsDlgButtonChecked(hwndDlg, playerDlgs[i+1]))
			{
				needPoll = TRUE;
				break;
			}
		}

		EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_L), needPoll);
		EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER), needPoll);
		EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_S_L), needPoll);
	}

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
				StartTimer();
			}

			break;
		}
	}

	return ret;
}

