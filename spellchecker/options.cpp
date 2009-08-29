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


#include "commons.h"

#include "options.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;

Options opts;


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ &opts.auto_replace_dict,		CONTROL_CHECKBOX,		IDC_AUTO_DICT,				"AutoReplaceDict", FALSE },
	{ &opts.auto_replace_user,		CONTROL_CHECKBOX,		IDC_AUTO_USER,				"AutoReplaceUser", TRUE },
	{ &opts.ignore_uppercase,		CONTROL_CHECKBOX,		IDC_IGNORE_UPPERCASE,		"IgnoreUppercase", FALSE },
	{ &opts.underline_type,			CONTROL_COMBO,			IDC_UNDERLINE_TYPE,			"UnderlineType", CFU_UNDERLINEWAVE - CFU_UNDERLINEDOUBLE },
	{ &opts.cascade_corrections,	CONTROL_CHECKBOX,		IDC_CASCADE_CORRECTIONS,	"CascadeCorrections", FALSE },
	{ &opts.show_all_corrections,	CONTROL_CHECKBOX,		IDC_SHOW_ALL_CORRECTIONS,	"ShowAllCorrections", FALSE },
	{ &opts.show_wrong_word,		CONTROL_CHECKBOX,		IDC_SHOW_WRONG_WORD,		"ShowWrongWord", TRUE },
	{ &opts.use_flags,				CONTROL_CHECKBOX,		IDC_USE_FLAGS,				"UseFlags", TRUE },
	{ &opts.auto_locale,			CONTROL_CHECKBOX,		IDC_AUTO_LOCALE,			"AutoLocale", FALSE },
	{ &opts.use_other_apps_dicts,	CONTROL_CHECKBOX,		IDC_OTHER_PROGS,			"UseOtherAppsDicts", TRUE },
};

static UINT optionsExpertControls[] = { 
	IDC_ADVANCED, IDC_UNDERLINE_TYPE_L, IDC_UNDERLINE_TYPE, IDC_CASCADE_CORRECTIONS, IDC_SHOW_ALL_CORRECTIONS,
	IDC_USE_FLAGS
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
}


void DeInitOptions()
{
	UnhookEvent(hOptHook);
}


void LoadOptions()
{
	LoadOpts(optionsControls, MAX_REGS(optionsControls), MODULE_NAME);
	
	if (languages.getCount() <= 0)
	{
		opts.default_language[0] = _T('\0');
		return;
	}

	DBVARIANT dbv;
	if (!DBGetContactSettingTString(NULL, MODULE_NAME, "DefaultLanguage", &dbv))
	{
		lstrcpyn(opts.default_language, dbv.ptszVal, MAX_REGS(opts.default_language));
		DBFreeVariant(&dbv);
	}

	int i;
	for(i = 0; i < languages.getCount(); i++)
		if (lstrcmp(languages[i]->language, opts.default_language) == 0)
			break;

	if (i >= languages.getCount())
		lstrcpy(opts.default_language, languages[0]->language);
}


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			int i, sel = -1;
			for(i = 0; i < languages.getCount(); i++)
			{
				SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_ADDSTRING, 0, (LONG) languages[i]->full_name);
				SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_SETITEMDATA, i, (DWORD) languages[i]);

				if (lstrcmp(opts.default_language, languages[i]->language) == 0)
					sel = i;
			}
			SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_SETCURSEL, sel, 0);

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

			if (LOWORD(wParam) == IDC_DEF_LANG
					&& (HIWORD(wParam) == CBN_SELCHANGE && (HWND)lParam == GetFocus()))
			{
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				return 0;
			}

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY && languages.getCount() > 0)
			{
				int sel = SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_GETCURSEL, 0, 0);
				if (sel >= languages.getCount())
					sel = 0;
				DBWriteContactSettingTString(NULL, MODULE_NAME, "DefaultLanguage", 
					(TCHAR *) languages[sel]->language);
				lstrcpy(opts.default_language, languages[sel]->language);
			}
			
			break;
		}

		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
			if(lpdis->CtlID != IDC_DEF_LANG) 
				break;
			if(lpdis->itemID == -1) 
				break;

			Dictionary *dict = (Dictionary *) lpdis->itemData;

			TEXTMETRIC tm;
			RECT rc;

			GetTextMetrics(lpdis->hDC, &tm);

			COLORREF clrfore = SetTextColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_SELECTED?COLOR_HIGHLIGHTTEXT:COLOR_WINDOWTEXT));
			COLORREF clrback = SetBkColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_SELECTED?COLOR_HIGHLIGHT:COLOR_WINDOW));

			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));

			rc.left = lpdis->rcItem.left + 2;

			// Draw icon
			if (opts.use_flags)
			{
				HICON hFlag = IcoLib_LoadIcon(dict);

				rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - ICON_SIZE) / 2;
				DrawIconEx(lpdis->hDC, rc.left, rc.top, hFlag, 16, 16, 0, NULL, DI_NORMAL);

				rc.left += ICON_SIZE + 4;
				
				IcoLib_ReleaseIcon(hFlag);
			}

			// Draw text
			rc.right = lpdis->rcItem.right - 2;
			rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;
			rc.bottom = rc.top + tm.tmHeight;
			DrawText(lpdis->hDC, dict->full_name, lstrlen(dict->full_name), &rc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

			// Restore old colors
			SetTextColor(lpdis->hDC, clrfore);
			SetBkColor(lpdis->hDC, clrback);

			return TRUE;
		}

		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
			if(lpmis->CtlID != IDC_DEF_LANG) 
				break;

			TEXTMETRIC tm;
			GetTextMetrics(GetDC(hwndDlg), &tm);

			if (opts.use_flags)
				lpmis->itemHeight = max(ICON_SIZE, tm.tmHeight);
			else
				lpmis->itemHeight = tm.tmHeight;
				
			return TRUE;
		}
	}

	return SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}


