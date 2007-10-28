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
HANDLE hUserInfoInitHook = NULL;

Options opts;

static int UserInfoInitialize(WPARAM wParam, LPARAM lParam);

static BOOL CALLBACK UserInfoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static Language *GetLanguage(HWND hwndDlg);
static Voice *GetVoice(HWND hwndDlg);
static Variant *GetVariant(HWND hwndDlg);


static OptPageControl optionsControls[] = { 
	{ &opts.disable_offline,		CONTROL_CHECKBOX,		ID_OFFLINE,			"DisableOffline", FALSE },
	{ &opts.disable_online,			CONTROL_CHECKBOX,		ID_ONLINE,			"DisableOnline", FALSE },
	{ &opts.disable_away,			CONTROL_CHECKBOX,		ID_AWAY,			"DisableAway", FALSE },
	{ &opts.disable_dnd,			CONTROL_CHECKBOX,		ID_DND,				"DisableDND", FALSE },
	{ &opts.disable_na,				CONTROL_CHECKBOX,		ID_NA,				"DisableNA", FALSE },
	{ &opts.disable_occupied,		CONTROL_CHECKBOX,		ID_OCCUPIED,		"DisableOccupied", FALSE },
	{ &opts.disable_freechat,		CONTROL_CHECKBOX,		ID_FREECHAT,		"DisableFreeChat", FALSE },
	{ &opts.disable_invisible,		CONTROL_CHECKBOX,		ID_INVISIBLE,		"DisableInvisible", FALSE },
	{ &opts.disable_onthephone,		CONTROL_CHECKBOX,		ID_ONTHEPHONE,		"DisableOnThePhone", FALSE },
	{ &opts.disable_outtolunch,		CONTROL_CHECKBOX,		ID_OUTTOLUNCH,		"DisableOutToLunch", FALSE },
	{ &opts.use_flags,				CONTROL_CHECKBOX,		IDC_USE_FLAGS,		"UseFlags", TRUE },
};

static UINT optionsExpertControls[] = { 
	IDC_STATUS, ID_OFFLINE, ID_ONLINE, ID_AWAY, ID_DND, ID_NA, ID_OCCUPIED, ID_FREECHAT, 
	ID_INVISIBLE, ID_ONTHEPHONE, ID_OUTTOLUNCH, 
	IDC_ADVANCED, IDC_USE_FLAGS
};


// Functions //////////////////////////////////////////////////////////////////////////////////////


int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=0;
	odp.hInstance=hInst;
//	odp.ptszGroup = TranslateT("Speak");
	odp.ptszTitle = TranslateT("Speak");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
    odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.nIDBottomSimpleControl = IDC_SYSTEM;
	odp.expertOnlyControls = optionsExpertControls;
	odp.nExpertOnlyControls = MAX_REGS(optionsExpertControls);
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


void InitOptions()
{
	LoadOptions();
	
	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
	hUserInfoInitHook = HookEvent(ME_USERINFO_INITIALISE, UserInfoInitialize);
}


void DeInitOptions()
{
	UnhookEvent(hOptHook);
	UnhookEvent(hUserInfoInitHook);
}


void LoadOptions()
{
	LoadOpts(optionsControls, MAX_REGS(optionsControls), MODULE_NAME);
	
	if (languages.getCount() <= 0)
	{
		opts.default_language = NULL;
		opts.default_voice = NULL;
		opts.default_variant = NULL;
		return;
	}

	opts.default_language = GetContactLanguage(NULL);
	opts.default_voice = GetContactVoice(NULL, opts.default_language);
	opts.default_variant = GetContactVariant(NULL);
}


void FillLanguagesCombo(HWND hwndDlg, HANDLE hContact)
{
	Language *def_lang;
	if (hContact == NULL)
		def_lang = opts.default_language;
	else
		def_lang = GetContactLanguage(hContact);

	for (int i = 0; i < languages.getCount(); i++)
	{
		int pos = SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_ADDSTRING, 0, (LPARAM) languages[i]->full_name);
		if (pos >= 0)
			SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_SETITEMDATA, pos, (LPARAM) languages[i]);
	}
	
	if (SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_SELECTSTRING, -1, (WPARAM) def_lang->full_name) < 0)
	{
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_SETCURSEL, 0, 0);
	}
}


void FillVoicesCombo(HWND hwndDlg, HANDLE hContact)
{
	Language *lang = GetLanguage(hwndDlg);

	Voice *def_voice;
	if (hContact == NULL)
		def_voice = opts.default_voice;
	else
		def_voice = GetContactVoice(hContact, lang);

	SendDlgItemMessage(hwndDlg, IDC_VOICE, CB_RESETCONTENT, 0, 0);

	int sel = -1;
	for (int i = 0; i < lang->voices.getCount(); i++)
	{
		TCHAR name[NAME_SIZE];
		Voice *voice = lang->voices[i];
		if (voice->gender == GENDER_MALE)
		{
			mir_sntprintf(name, MAX_REGS(name), _T(TCHAR_STR_PARAM) _T(" (%s)"),
				voice->name, TranslateT("Male"));
		}
		else if (voice->gender == GENDER_FEMALE)
		{
			mir_sntprintf(name, MAX_REGS(name), _T(TCHAR_STR_PARAM) _T(" (%s)"),
				voice->name, TranslateT("Female"));
		}
		else
		{
			mir_sntprintf(name, MAX_REGS(name), _T(TCHAR_STR_PARAM),
				voice->name);
		}
		int pos = SendDlgItemMessage(hwndDlg, IDC_VOICE, CB_ADDSTRING, 0, (LPARAM) name);
		if (pos >= 0)
			SendDlgItemMessage(hwndDlg, IDC_VOICE, CB_SETITEMDATA, pos, (LPARAM) voice);

		if (def_voice == voice)
			sel = pos;
	}
	if (sel < 0)
	{
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		sel = 0;
	}

	SendDlgItemMessage(hwndDlg, IDC_VOICE, CB_SETCURSEL, sel, 0);
}


void FillVariantsCombo(HWND hwndDlg, HANDLE hContact)
{
	Variant *def_var;
	if (hContact == NULL)
		def_var = opts.default_variant;
	else
		def_var = GetContactVariant(hContact);

	int pos = SendDlgItemMessageA(hwndDlg, IDC_VARIANT, CB_ADDSTRING, 0, (LPARAM) "<None>");
	if (pos >= 0)
		SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_SETITEMDATA, pos, (LPARAM) NULL);

	for (int i = 0; i < variants.getCount(); i++)
	{
		int pos = SendDlgItemMessageA(hwndDlg, IDC_VARIANT, CB_ADDSTRING, 0, (LPARAM) variants[i]->name);
		if (pos >= 0)
			SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_SETITEMDATA, pos, (LPARAM) variants[i]);
	}

	if (SendDlgItemMessageA(hwndDlg, IDC_VARIANT, CB_SELECTSTRING, -1, (LPARAM) def_var->name) < 0)
	{
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_SETCURSEL, 0, 0);
	}
}


static Language *GetLanguage(HWND hwndDlg)
{
	int sel = SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_GETCURSEL, 0, 0);
	if (sel < 0)
		sel = 0;

	return (Language *) SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_GETITEMDATA, sel, 0);
}


static Voice *GetVoice(HWND hwndDlg)
{
	int sel = SendDlgItemMessage(hwndDlg, IDC_VOICE, CB_GETCURSEL, 0, 0);
	if (sel < 0)
		sel = 0;

	return (Voice *) SendDlgItemMessage(hwndDlg, IDC_VOICE, CB_GETITEMDATA, sel, 0);
}


static Variant *GetVariant(HWND hwndDlg)
{
	int sel = SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_GETCURSEL, 0, 0);
	if (sel < 0)
		sel = 0;

	return (Variant *) SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_GETITEMDATA, sel, 0);
}

static BOOL CALLBACK BaseDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);
			FillLanguagesCombo(hwndDlg, hContact);
			FillVoicesCombo(hwndDlg, hContact);
			FillVariantsCombo(hwndDlg, hContact);
			break;
		}

		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDC_DEF_LANG && HIWORD(wParam) == CBN_SELCHANGE)
			{
				HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);
				FillVoicesCombo(hwndDlg, hContact);
			}
			else if (LOWORD(wParam) == IDC_SPEAK)
			{
				if (languages.getCount() <= 0)
					break;

				TCHAR text[1024];
				GetDlgItemText(hwndDlg, IDC_TEST, text, MAX_REGS(text));
				if (text[0] == _T('\0'))
					break;

				Voice *voice = GetVoice(hwndDlg);
				if (voice == NULL)
					break;

				Speak(voice, GetVariant(hwndDlg), text);
			}

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				if (languages.getCount() > 0)
				{
					HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);

					// Language
					Language *lang = GetLanguage(hwndDlg);

					BOOL remove;
					if (hContact == NULL)
					{
						TCHAR def[NAME_SIZE];
						GetLangPackLanguage(def, MAX_REGS(def));

						remove = (lstrcmpi(def, lang->language) == 0);
					}
					else
						remove = (lang == opts.default_language);

					if (remove)
						DBDeleteContactSetting(hContact, MODULE_NAME, "TalkLanguage");
					else
						DBWriteContactSettingTString(hContact, MODULE_NAME, "TalkLanguage", lang->language);

					if (hContact == NULL)
						opts.default_language = lang;

					// Voice
					Voice *voice = GetVoice(hwndDlg);
					if (voice == NULL)
						voice = lang->voices[0];

					if (hContact != NULL && lang == opts.default_language && voice == opts.default_voice)
						DBDeleteContactSetting(hContact, MODULE_NAME, "Voice");
					else
						DBWriteContactSettingString(hContact, MODULE_NAME, "Voice", voice->name);

					if (hContact == NULL)
						opts.default_voice = voice;

					// Variant
					Variant *var = GetVariant(hwndDlg);
					if (var == NULL || (hContact != NULL && lang == opts.default_language && voice == opts.default_voice && var == opts.default_variant))
						DBDeleteContactSetting(hContact, MODULE_NAME, "Variant");
					else
						DBWriteContactSettingString(hContact, MODULE_NAME, "Variant", var->name);

					if (hContact == NULL)
						opts.default_variant = var;
				}
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

			Language *lang = (Language *) lpdis->itemData;

			TEXTMETRIC tm;
			RECT rc;

			GetTextMetrics(lpdis->hDC, &tm);

			COLORREF clrfore = SetTextColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_DISABLED ? COLOR_GRAYTEXT : (lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)));
			COLORREF clrback = SetBkColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_DISABLED ? COLOR_BTNFACE : (lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW)));

			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(lpdis->itemState & ODS_DISABLED ? COLOR_BTNFACE : (lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW)));

			rc.left = lpdis->rcItem.left + 2;

			// Draw icon
			if (opts.use_flags)
			{
				HICON hFlag = LoadIconEx(lang);

				rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - ICON_SIZE) / 2;
				DrawIconEx(lpdis->hDC, rc.left, rc.top, hFlag, 16, 16, 0, NULL, DI_NORMAL);

				rc.left += ICON_SIZE + 4;
				
				ReleaseIconEx(hFlag);
			}

			// Draw text
			rc.right = lpdis->rcItem.right - 2;
			rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;
			rc.bottom = rc.top + tm.tmHeight;
			DrawText(lpdis->hDC, lang->full_name, lstrlen(lang->full_name), &rc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

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

	return FALSE;
}

static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) NULL);
			break;
		}

		case WM_COMMAND:
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				if ((HWND)lParam == GetFocus())
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}

			break;
		}
	}

	BOOL ret = BaseDlgProc(hwndDlg, msg, wParam, lParam);
	if (!ret)
		ret = SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
	return ret;
}



static int UserInfoInitialize(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) lParam;
	if (hContact == NULL)
		return 0;

	if (languages.getCount() < 0)
		return 0;

	// Contact dialog
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pfnDlgProc = UserInfoDlgProc;
	odp.position = 1000000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_CONTACT_LANG);
	odp.pszTitle = LPGEN("Speak");
	CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}


static void EnableDisableControls(HWND hwndDlg)
{
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_ENABLE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DEF_LANG_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DEF_LANG), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_VOICE_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_VOICE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_VARIANT_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_VARIANT), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEST_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEST), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SPEAK), enabled);
}


static BOOL CALLBACK UserInfoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			HANDLE hContact = (HANDLE) lParam;
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) hContact);

			TranslateDialogDefault(hwndDlg);

			CheckDlgButton(hwndDlg, IDC_ENABLE, DBGetContactSettingByte(hContact, MODULE_NAME, "Enabled", TRUE) ? BST_CHECKED : BST_UNCHECKED);

			EnableDisableControls(hwndDlg);

			break;
		}

		case WM_COMMAND:
		{
			if ((HIWORD(wParam) == CBN_SELCHANGE && (HWND)lParam == GetFocus()) 
					|| LOWORD(wParam) == IDC_ENABLE)
			{
				HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);

				DBWriteContactSettingByte(hContact, MODULE_NAME, "Enabled", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLE));

				NMHDR nmhdr;
				nmhdr.idFrom = 0;
				nmhdr.code = PSN_APPLY;
				BaseDlgProc(hwndDlg, WM_NOTIFY, 0, (LPARAM) &nmhdr);

				if (LOWORD(wParam) == IDC_ENABLE)
					EnableDisableControls(hwndDlg);
			}

			break;
		}
	}

	return BaseDlgProc(hwndDlg, msg, wParam, lParam);
}
