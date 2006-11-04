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

#include "options.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;

Options opts;


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ &opts.default_language,	CONTROL_COMBO_ITEMDATA,	IDC_DEF_LANG,		"DefaultLanguage", NULL, 0, 0, MAX_REGS(opts.default_language) },
	{ &opts.auto_correct,		CONTROL_CHECKBOX,		IDC_AUTOCORRECT,	"AutoCorrect", FALSE },
	{ &opts.underline_type,		CONTROL_COMBO,			IDC_UNDERLINE_TYPE,	"UnderlineType", CFU_UNDERLINEWAVE - CFU_UNDERLINEDOUBLE }
};

static UINT optionsExpertControls[] = { 
	IDC_ADVANCED, IDC_UNDERLINE_TYPE_L, IDC_UNDERLINE_TYPE
};



// Functions //////////////////////////////////////////////////////////////////////////////////////


int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=0;
	odp.hInstance=hInst;
	odp.ptszGroup = TranslateT("Message Sessions");
	odp.ptszTitle = TranslateT("Spell Checker");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
    odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.nIDBottomSimpleControl = IDC_SPELL_CHECKER;
	odp.expertOnlyControls = optionsExpertControls;
	odp.nExpertOnlyControls = MAX_REGS(optionsExpertControls);
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


void InitOptions()
{
	LoadOptions();
	
	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);

	if (languages.count <= 0)
	{
		opts.default_language[0] = _T('\0');
		return;
	}

	for(int i = 0; i < languages.count; i++)
		if (lstrcmp(languages.dicts[i]->language, opts.default_language) == 0)
			break;

	if (i == languages.count)
		lstrcpy(opts.default_language, languages.dicts[0]->language);
}


void DeInitOptions()
{
	UnhookEvent(hOptHook);
}


void LoadOptions()
{
	LoadOpts(optionsControls, MAX_REGS(optionsControls), MODULE_NAME);
}


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			int i;
			for(i = 0; i < languages.count; i++)
			{
				if (languages.dicts[i]->localized_name[0] != _T('\0'))
				{
					TCHAR name[128];
					mir_sntprintf(name, MAX_REGS(name), _T("%s [%s]"), languages.dicts[i]->localized_name, languages.dicts[i]->language);
					SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_ADDSTRING, 0, (LONG) name);
				}
				else
				{
					SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_ADDSTRING, 0, (LONG) languages.dicts[i]->language);
				}

				SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_SETITEMDATA, i, (DWORD) languages.dicts[i]->language);
			}

			SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LONG) TranslateT("Line"));
			SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LONG) TranslateT("Dotted"));
			SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LONG) TranslateT("Dash"));
			SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LONG) TranslateT("Dash dot"));
			SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LONG) TranslateT("Dash dot dot"));
			SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LONG) TranslateT("Wave"));
			SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LONG) TranslateT("Thick"));

			break;
		}

		case WM_COMMAND:
		{
			if(LOWORD(wParam) == IDC_GETMORE)
				CallService(MS_UTILS_OPENURL, 1, (LPARAM) "http://wiki.services.openoffice.org/wiki/Dictionaries");

			break;
		}
	}

	return SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}

