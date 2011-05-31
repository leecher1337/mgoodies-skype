/* 
ListeningTo plugin for Miranda IM
==========================================================================
Copyright	(C) 2005-2011 Ricardo Pescuma Domenecci
			(C) 2010-2011 Merlin_de

PRE-CONDITION to use this code under the GNU General Public License:
 1. you do not build another Miranda IM plugin with the code without written permission
    of the autor (peace for the project).
 2. you do not publish copies of the code in other Miranda IM-related code repositories.
    This project is already hosted in a SVN and you are welcome to become a contributing member.
 3. you do not create listeningTo-derivatives based on this code for the Miranda IM project.
    (feel free to do this for another project e.g. foobar)
 4. you do not distribute any kind of self-compiled binary of this plugin (we want continuity
    for the plugin users, who should know that they use the original) you can compile this plugin
    for your own needs, friends, but not for a whole branch of people (e.g. miranda plugin pack).
 5. This isn't free beer. If your jurisdiction (country) does not accept
    GNU General Public License, as a whole, you have no rights to the software
    until you sign a private contract with its author. !!!
 6. you always put these notes and copyright notice at the beginning of your code.
==========================================================================

in case you accept the pre-condition,
this is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "commons.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;

Options opts;

extern std::vector<ProtocolInfo> proto_itens;
extern HANDLE hExtraIcon;

BOOL ListeningToEnabled(char *proto, BOOL ignoreGlobal = FALSE);



static INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK PlayersDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK FormatDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ &opts.enable_sending,				CONTROL_CHECKBOX,	IDC_ENABLE_SEND,				"EnableSend", TRUE },
	{ &opts.enable_music,				CONTROL_CHECKBOX,	IDC_ENABLE_MUSIC,				"EnableMusic", TRUE },
	{ &opts.enable_radio,				CONTROL_CHECKBOX,	IDC_ENABLE_RADIO,				"EnableRadio", TRUE },
	{ &opts.enable_video,				CONTROL_CHECKBOX,	IDC_ENABLE_VIDEO,				"EnableVideo", TRUE },
	{ &opts.enable_others,				CONTROL_CHECKBOX,	IDC_ENABLE_OTHERS,				"EnableOthers", TRUE },
	{ &opts.xstatus_set,				CONTROL_RADIO,		IDC_SET_XSTATUS,				"XStatusSet", 0, SET_XSTATUS },
	{ &opts.xstatus_set,				CONTROL_RADIO,		IDC_CHECK_XSTATUS,				"XStatusSet", 0, CHECK_XSTATUS },
	{ &opts.xstatus_set,				CONTROL_RADIO,		IDC_CHECK_XSTATUS_MUSIC,		"XStatusSet", 0, CHECK_XSTATUS_MUSIC },
	{ &opts.xstatus_set,				CONTROL_RADIO,		IDC_IGNORE_XSTATUS,				"XStatusSet", 0, IGNORE_XSTATUS },
	{ &opts.override_contact_template,	CONTROL_CHECKBOX,	IDC_OVERRIDE_CONTACTS_TEMPLATE,	"OverrideContactsTemplate", FALSE},
	{ &opts.show_adv_icon,				CONTROL_CHECKBOX,	IDC_SHOW_ADV_ICON,				"ShowAdvancedIcon", FALSE},
	{ &opts.adv_icon_slot,				CONTROL_COMBO,		IDC_ADV_ICON,					"AdvancedIconSlot", 1}
};

static UINT optionsExpertControls[] = { 
	IDC_XSTATUS_G, IDC_XSTATUS_L, IDC_SET_XSTATUS, IDC_CHECK_XSTATUS, IDC_CHECK_XSTATUS_MUSIC, IDC_IGNORE_XSTATUS,
	IDC_CONTACTS_G, IDC_SHOW_ADV_ICON, IDC_ADV_ICON
};

static OptPageControl formatControls[] = { 
	{ &opts.templ,					CONTROL_TEXT,		IDC_TEMPLATE,			"Template", (ULONG_PTR) _T("%title% - %artist%") },
	{ &opts.unknown,				CONTROL_TEXT,		IDC_UNKNOWN,			"Unknown", (ULONG_PTR) _T("<Unknown>"), 0, 0, 128 },
	{ &opts.xstatus_name,			CONTROL_TEXT,		IDC_XSTATUS_NAME,		"XStatusName", (ULONG_PTR) _T("Listening to") },
	{ &opts.xstatus_message,		CONTROL_TEXT,		IDC_XSTATUS_MESSAGE,	"XStatusMessage", (ULONG_PTR) _T("%listening%") },
	{ &opts.nothing,				CONTROL_TEXT,		IDC_NOTHING,			"Nothing", (ULONG_PTR) _T("<Nothing>"), 0, 0, 128 }
};

static OptPageControl playersControls[] = {
	//first base class player 
	{ NULL,							CONTROL_CHECKBOX,	IDC_WATRACK,		"GetInfoFromWATrack", FALSE },
	{ NULL,							CONTROL_CHECKBOX,	IDC_WINAMP,			"EnableWinamp", TRUE },
	{ NULL,							CONTROL_CHECKBOX,	IDC_WMP,			"EnableWMP", TRUE },
	{ NULL,							CONTROL_CHECKBOX,	IDC_WLM,			"EnableWLM", TRUE },
	{ NULL,							CONTROL_CHECKBOX,	IDC_ITUNES,			"EnableITunes", TRUE },
	{ NULL,							CONTROL_CHECKBOX,	IDC_FOOBAR,			"EnableFoobar", TRUE },
	{ NULL/*&opts.enable_radio*/,	CONTROL_CHECKBOX,	IDC_MRADIO,			"EnableMRadio", TRUE },
//	{ NULL,							CONTROL_CHECKBOX,	IDC_VIDEOLAN,		"EnableVideoLAN", TRUE },
	//other
	{ &opts.time_to_pool,			CONTROL_SPIN,		IDC_POLL_TIMER,		"TimeToPool", (WORD) 5, IDC_POLL_TIMER_SPIN, (WORD) 1, (WORD) 255 },
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
	odp.ptszGroup = LPGENT("Status");
	odp.ptszTitle = LPGENT("Listening info");

	odp.ptszTab = LPGENT("General");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.expertOnlyControls = optionsExpertControls;
	odp.nExpertOnlyControls = MAX_REGS(optionsExpertControls);
	odp.nIDBottomSimpleControl = IDC_LISTENING_G;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.ptszTab = LPGENT("Format");
	odp.pfnDlgProc = FormatDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_FORMAT);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_EXPERTONLY;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.ptszTab = LPGENT("Players");
	odp.pfnDlgProc = PlayersDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PLAYERS);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_EXPERTONLY;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


void InitOptions()
{
	playersControls[0].var = &players[WATRACK]->m_enabled;
	playersControls[1].var = &players[WINAMP]->m_enabled;
	playersControls[2].var = &players[WMP]->m_enabled;
	playersControls[3].var = &players[WLM]->m_enabled;
	playersControls[4].var = &players[ITUNES]->m_enabled;
	playersControls[5].var = &players[FOOBAR]->m_enabled;
	playersControls[6].var = &players[MRADIO]->m_enabled;
//	playersControls[7].var = &players[VIDEOLAN]->m_enabled;

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
	LoadOpts(formatControls, MAX_REGS(formatControls), MODULE_NAME);
	LoadOpts(playersControls, MAX_REGS(playersControls), MODULE_NAME);
}


BOOL IsTypeEnabled(LISTENINGTOINFO *lti)
{
	if (lti == NULL)
		return TRUE;

#ifdef UNICODE
	if (lti->dwFlags & LTI_UNICODE)
	{
		if (lstrcmpi(lti->ptszType, _T("Music")) == 0)
			return opts.enable_music;
		else if (lstrcmpi(lti->ptszType, _T("Radio")) == 0)
			return opts.enable_radio;
		else if (lstrcmpi(lti->ptszType, _T("Video")) == 0)
			return opts.enable_video;
		else 
			return opts.enable_others;
	}
	else
#endif
	{
		if (strcmpi(lti->pszType, "Music") == 0)
			return opts.enable_music;
		else if (strcmpi(lti->pszType, "Radio") == 0)
			return opts.enable_radio;
		else if (strcmpi(lti->pszType, "Video") == 0)
			return opts.enable_video;
		else 
			return opts.enable_others;
	}
}


static void OptionsEnableDisableCtrls(HWND hwndDlg)
{
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_SEND);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_MUSIC), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_RADIO), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_VIDEO), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_OTHERS), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_MENU), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SET_XSTATUS), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_XSTATUS), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_XSTATUS_MUSIC), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_IGNORE_XSTATUS), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CONTACTS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_OVERRIDE_CONTACTS_TEMPLATE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_ADV_ICON), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ADV_ICON), enabled);
}


static INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	INT_PTR ret;
	if (msg != WM_INITDIALOG)
		ret = SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			if (hExtraIcon != NULL)
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_SHOW_ADV_ICON), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADV_ICON), SW_HIDE);
			}
			else
			{
				if (ServiceExists("CList/HideContactAvatar")!=0) /* check if Clist modern*/  
					{
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("<none>"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("E-Mail"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Protocol"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Phone/SMS"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #1"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #2"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Web page"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Client (fingerprint.dll is required)"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Visibility/Chat activity"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #3"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #4"));
					}
				else if (ServiceExists("CListFrame/SetSkinnedFrame")!=0) /*check if Clist Nicer*/  
					{
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Reserved, unused"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("E-Mail"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Reserved #1"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Telephone"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #1 (ICQ X-Status)"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #2"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Homepage"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Client (fingerprint required)"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Reserved #2"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #3"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #4"));
					}
				else if (ServiceExists("CLUI/GetConnectingIconForProtocol")!=0) /*check if Clist MW*/  
					{
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("<none>"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("E-Mail"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Protocol Type"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Cellular"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #1"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #2"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Homepage"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Client (fingerprint required)"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("<none>"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #3"));
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("Advanced #4"));
					}
				else
					{
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) TranslateT("<none>"));
					}
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
				RebuildMenu();
				if(!hTimer)		//check always if timer exist !!
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
	WLM, IDC_WLM, 
	ITUNES, IDC_ITUNES,
	FOOBAR, IDC_FOOBAR
};


static void PlayersEnableDisableCtrls(HWND hwndDlg)
{
	BOOL watrack_found = ServiceExists(MS_WAT_GETMUSICINFO);
	BOOL mradio_found = ServiceExists(MS_RADIO_EXPORT);		//mRadio 0.0.1.4

	EnableWindow(GetDlgItem(hwndDlg, IDC_WATRACK), watrack_found);

	BOOL enabled = !IsDlgButtonChecked(hwndDlg, IDC_WATRACK) || !watrack_found;
	EnableWindow(GetDlgItem(hwndDlg, IDC_PLAYERS_L), enabled);

	BOOL needPoll = FALSE;
	for (int i = 0; i < MAX_REGS(playerDlgs); i += 2)
	{
		EnableWindow(GetDlgItem(hwndDlg, playerDlgs[i+1]), enabled);
		if (players[playerDlgs[i]]->m_needPoll && IsDlgButtonChecked(hwndDlg, playerDlgs[i+1]))
			needPoll = TRUE;
	}
	EnableWindow(GetDlgItem(hwndDlg, IDC_MRADIO), enabled && mradio_found /*&& opts.enable_radio*/);
	EnableWindow(GetDlgItem(hwndDlg, IDC_OTHER), enabled);

	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_L), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_SPIN), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_S_L), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CODE_INJECTION), enabled);
}

static INT_PTR CALLBACK PlayersDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	INT_PTR ret = SaveOptsDlgProc(playersControls, MAX_REGS(playersControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

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
				if(!hTimer)		//check always if timer exist !!
					StartTimer();
			}

			break;
		}
	}

	return ret;
}

static INT_PTR CALLBACK FormatDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	return SaveOptsDlgProc(formatControls, MAX_REGS(formatControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}

