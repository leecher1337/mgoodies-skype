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
static BOOL CALLBACK PopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ &opts.default_language,	CONTROL_COMBO_TEXT,	IDC_DEF_LANG,			"DefaultLanguage", NULL, 0, 0, MAX_REGS(opts.default_language) },
	{ &opts.auto_correct,		CONTROL_CHECKBOX,	IDC_AUTOCORRECT,		"AutoCorrect", FALSE },
	{ &opts.auto_srmm_support,	CONTROL_CHECKBOX,	IDC_SIMULATE_SUPPORT,	"SimulateSRMMSupport", FALSE }
};

static UINT optionsExpertControls[] = { 
	IDC_SIMULATE_SUPPORT
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
	odp.expertOnlyControls = optionsExpertControls;
	odp.nExpertOnlyControls = MAX_REGS(optionsExpertControls);
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


void InitOptions()
{
	LoadOptions();
	
	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);

	if (num_laguages <= 0)
	{
		opts.default_language[0] = _T('\0');
		return;
	}

	for(int i = 0; i < num_laguages; i++)
		if (lstrcmp(languages[i].name, opts.default_language) == 0)
			break;

	if (i == num_laguages)
		lstrcpy(opts.default_language, languages[0].name);
}


void DeInitOptions()
{
	UnhookEvent(hOptHook);

	//FreeMirOptions();
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
			for(int i = 0; i < num_laguages; i++)
				SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_ADDSTRING, 0, (LONG) languages[i].name);

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

