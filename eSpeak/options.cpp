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

Options opts = {0};

static int UserInfoInitialize(WPARAM wParam, LPARAM lParam);

static BOOL CALLBACK UserInfoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SystemDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK TypesDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

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
	{ &opts.enable_only_idle,		CONTROL_CHECKBOX,		IDC_ONLY_IDLE,		"EnableOnlyIfIdle", FALSE },
	{ &opts.truncate,				CONTROL_CHECKBOX,		IDC_TRUNCATE_L,		"Truncate", FALSE },
	{ &opts.truncate_len,			CONTROL_SPIN,			IDC_TRUNCATE,		"TruncateLen", (WORD) 128, IDC_TRUNCATE_SPIN, (WORD) 1, (WORD) 1024 },
	{ &opts.use_flags,				CONTROL_CHECKBOX,		IDC_USE_FLAGS,		"UseFlags", TRUE },
	{ &opts.respect_sndvol_mute,	CONTROL_CHECKBOX,		IDC_SNDVOL,			"RespectSndVolMute", TRUE },
	{ &opts.select_variant_per_genre,CONTROL_CHECKBOX,		IDC_PER_GENRE,		"SelectVariantPerGenre", TRUE },
};


// Functions //////////////////////////////////////////////////////////////////////////////////////


BOOL GetSettingBool(SPEAK_TYPE *type, char *setting, BOOL def)
{
	return GetSettingBool(type, -1, setting, def);
}

BOOL GetSettingBool(SPEAK_TYPE *type, int templ, char *setting, BOOL def)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%s_%d_%s", type->name, templ, setting);
	return DBGetContactSettingByte(NULL, type->module == NULL ? MODULE_NAME : type->module, tmp, def) != 0;
}

void WriteSettingBool(SPEAK_TYPE *type, char *setting, BOOL val)
{
	WriteSettingBool(type, -1, setting, val);
}

void WriteSettingBool(SPEAK_TYPE *type, int templ, char *setting, BOOL val)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%s_%d_%s", type->name, templ, setting);
	DBWriteContactSettingByte(NULL, type->module == NULL ? MODULE_NAME : type->module, tmp, val ? 1 : 0);
}

void WriteSettingTString(SPEAK_TYPE *type, int templ, char *setting, TCHAR *str)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%s_%d_%s", type->name, templ, setting);
	DBWriteContactSettingTString(NULL, type->module == NULL ? MODULE_NAME : type->module, tmp, str);
}


int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=0;
	odp.hInstance=hInst;
	odp.ptszGroup = TranslateT("Events");
	odp.ptszTitle = TranslateT("Speak");
	odp.ptszTab = TranslateT("General");
	odp.pfnDlgProc = SystemDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
    odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.ptszTab = TranslateT("Advanced");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_ADVANCED);
    odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_EXPERTONLY;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	if (types.getCount() > 0) 
	{
		odp.ptszTab = TranslateT("Types");
		odp.pfnDlgProc = TypesDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_TYPES);
		odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_EXPERTONLY;
		CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	}

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
		return;

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
	
	if (def_lang != NULL && SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_SELECTSTRING, -1, (WPARAM) def_lang->full_name) < 0)
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
		
		TCHAR *gender = NULL;
		if (voice->gender == GENDER_MALE)
			gender = TranslateT("Male");
		else if (voice->gender == GENDER_FEMALE)
			gender = TranslateT("Female");

		TCHAR *age = NULL;
		if (voice->age[0] != 0)
			age = voice->age;

		if (gender != NULL && age != NULL)
			mir_sntprintf(name, MAX_REGS(name), _T("%s (%s, %s)"), voice->name, gender, age);
		
		else if (gender != NULL)
			mir_sntprintf(name, MAX_REGS(name), _T("%s (%s)"), voice->name, gender);
		
		else if (age != NULL)
			mir_sntprintf(name, MAX_REGS(name), _T("%s (%s)"), voice->name, age);
		
		else
			mir_sntprintf(name, MAX_REGS(name), _T("%s"), voice->name);

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
	SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_RESETCONTENT, 0, 0);

	int pos = SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_ADDSTRING, 0, (LPARAM) _T("<None>"));
	if (pos >= 0)
		SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_SETITEMDATA, pos, (LPARAM) NULL);

	if (GetVoice(hwndDlg)->engine == ENGINE_SAPI)
	{
		SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_SETCURSEL, 0, 0);
	}
	else
	{
		Variant *def_var;
		if (hContact == NULL)
			def_var = opts.default_variant;
		else
			def_var = GetContactVariant(hContact);

		for (int i = 0; i < variants.getCount(); i++)
		{
			int pos = SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_ADDSTRING, 0, (LPARAM) variants[i]->name);
			if (pos >= 0)
				SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_SETITEMDATA, pos, (LPARAM) variants[i]);
		}

		if (def_var == NULL)
		{
			SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_SETCURSEL, 0, 0);
		}
		else if (SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_SELECTSTRING, -1, (LPARAM) def_var->name) < 0)
		{
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_VARIANT, CB_SETCURSEL, 0, 0);
		}
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
			if (languages.getCount() > 0)
			{
				FillLanguagesCombo(hwndDlg, hContact);
				FillVoicesCombo(hwndDlg, hContact);
				FillVariantsCombo(hwndDlg, hContact);
			}

			HWND item = GetDlgItem(hwndDlg, IDC_PUNCT);
			if (item != NULL)
			{
				SendMessage(item, CB_ADDSTRING, 0, (LPARAM) TranslateT("None"));
				SendMessage(item, CB_ADDSTRING, 0, (LPARAM) TranslateT("All"));
				SendMessage(item, CB_ADDSTRING, 0, (LPARAM) TranslateT("Some"));
			}

			Voice *voice = GetVoice(hwndDlg);

			for (int i = 0; i < NUM_PARAMETERS; i++)
			{
				item = GetDlgItem(hwndDlg, PARAMETERS[i].ctrl);
				if (item == NULL)
					continue;

				RANGE *range;
				if (voice->engine == ENGINE_ESPEAK)
					range = &PARAMETERS[i].espeak;
				else if (voice->engine == ENGINE_SAPI)
					range = &PARAMETERS[i].sapi;
				else
					continue;

				BOOL enabled = (range->min < range->max);
				EnableWindow(item, enabled);
				EnableWindow(GetDlgItem(hwndDlg, PARAMETERS[i].label), enabled);

				if (enabled)
				{
					if (PARAMETERS[i].type == SCROLL)
					{
						SendMessage(item, TBM_SETRANGE, FALSE, MAKELONG(0, range->max - range->min));
						SendMessage(item, TBM_SETPOS, TRUE, GetContactParam(hContact, i) - range->min);
					}
					else
					{
						SendMessage(item, CB_SETCURSEL, GetContactParam(hContact, i), 0);
					}
				}
			}

			break;
		}

		case WM_HSCROLL:
		{
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}

		case WM_COMMAND:
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				if (languages.getCount() <= 0)
					break;

				if ((HWND)lParam == GetFocus())
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

				HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);

				if (LOWORD(wParam) == IDC_DEF_LANG)
				{
					FillVoicesCombo(hwndDlg, hContact);
					FillVariantsCombo(hwndDlg, hContact);
				}
				else if (LOWORD(wParam) == IDC_VOICE)
				{
					FillVariantsCombo(hwndDlg, hContact);
				}

				if (LOWORD(wParam) == IDC_DEF_LANG || LOWORD(wParam) == IDC_VOICE)
				{
					Voice *voice = GetVoice(hwndDlg);
					for (int i = 0; i < NUM_PARAMETERS; i++)
					{
						HWND item = GetDlgItem(hwndDlg, PARAMETERS[i].ctrl);
						if (item == NULL)
							continue;

						RANGE *range;
						if (voice->engine == ENGINE_ESPEAK)
							range = &PARAMETERS[i].espeak;
						else if (voice->engine == ENGINE_SAPI)
							range = &PARAMETERS[i].sapi;
						else
							continue;

						BOOL enabled = (range->min < range->max);
						EnableWindow(item, enabled);
						EnableWindow(GetDlgItem(hwndDlg, PARAMETERS[i].label), enabled);

						if (enabled)
						{
							if (PARAMETERS[i].type == SCROLL)
							{
								SendMessage(item, TBM_SETRANGE, FALSE, MAKELONG(0, range->max - range->min));

								int def;
								if (voice->engine == ENGINE_SAPI && PARAMETERS[i].eparam == espeakRATE)
									def = SAPI_GetDefaultRateFor(voice->id);
								else
									def = range->def;

								SendMessage(item, TBM_SETPOS, TRUE, def - range->min);
							}
							else
							{
								SendMessage(item, CB_SETCURSEL, range->def, 0);
							}
						}
					}
				}
			}
			else if (LOWORD(wParam) == IDC_SPEAK)
			{
				if (languages.getCount() <= 0)
					break;

				TCHAR text[1024];
				GetDlgItemText(hwndDlg, IDC_TEST, text, MAX_REGS(text));
				if (text[0] == _T('\0'))
					break;

				Language *lang = GetLanguage(hwndDlg);
				if (lang == NULL)
					break;

				Voice *voice = GetVoice(hwndDlg);
				if (voice == NULL)
					break;

				SpeakData *data = new SpeakData(lang, voice, GetVariant(hwndDlg), mir_tstrdup(text));
				for (int i = 0; i < NUM_PARAMETERS; i++)
				{
					HWND item = GetDlgItem(hwndDlg, PARAMETERS[i].ctrl);
					if (item == NULL)
					{
						data->setParameter(i, GetContactParam(NULL, i));
						break;
					}

					RANGE *range;
					if (voice->engine == ENGINE_ESPEAK)
						range = &PARAMETERS[i].espeak;
					else if (voice->engine == ENGINE_SAPI)
						range = &PARAMETERS[i].sapi;
					else
					{
						data->setParameter(i, GetContactParam(NULL, i));
						break;
					}

					if (PARAMETERS[i].type == SCROLL)
						data->setParameter(i, SendMessage(item, TBM_GETPOS, 0, 0) + range->min);
					else
						data->setParameter(i, SendMessage(item, CB_GETCURSEL, 0, 0));
				}
				queue->Add(0, (HANDLE) -1, data);
			}

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				if (languages.getCount() <= 0)
					break;

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
					remove = FALSE;

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

				DBWriteContactSettingTString(hContact, MODULE_NAME, "Voice", voice->name);

				if (hContact == NULL)
					opts.default_voice = voice;

				// Variant
				Variant *var = GetVariant(hwndDlg);
				DBWriteContactSettingTString(hContact, MODULE_NAME, "Variant", var->name);

				if (hContact == NULL)
					opts.default_variant = var;

				for (int i = 0; i < NUM_PARAMETERS; i++)
				{
					HWND item = GetDlgItem(hwndDlg, PARAMETERS[i].ctrl);
					if (item == NULL)
						continue;

					RANGE *range;
					if (voice->engine == ENGINE_ESPEAK)
						range = &PARAMETERS[i].espeak;
					else if (voice->engine == ENGINE_SAPI)
						range = &PARAMETERS[i].sapi;
					else
						continue;

					if (PARAMETERS[i].type == SCROLL)
						SetContactParam(hContact, i, SendMessage(item, TBM_GETPOS, 0, 0) + range->min);
					else
						SetContactParam(hContact, i, SendMessage(item, CB_GETCURSEL, 0, 0));
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
	return SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}


static BOOL CALLBACK SystemDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) NULL);
			break;
		}
	}

	return BaseDlgProc(hwndDlg, msg, wParam, lParam);
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
			if (LOWORD(wParam) == IDC_ENABLE)
			{
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				EnableDisableControls(hwndDlg);
			}

			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				HANDLE hContact = (HANDLE) GetWindowLong(hwndDlg, GWL_USERDATA);
				DBWriteContactSettingByte(hContact, MODULE_NAME, "Enabled", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLE));
			}

			break;
		}
	}

	return BaseDlgProc(hwndDlg, msg, wParam, lParam);
}


static BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	BOOL ret;

	POINT pt;

	pt.x = lpRect->left;
	pt.y = lpRect->top;

	ret = ScreenToClient(hWnd, &pt);

	if (!ret) return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;


	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}


static void GetTextMetric(HFONT hFont, TEXTMETRIC *tm)
{
	HDC hdc = GetDC(NULL);
	HFONT hOldFont = (HFONT) SelectObject(hdc, hFont);
	GetTextMetrics(hdc, tm);
	SelectObject(hdc, hOldFont);
	ReleaseDC(NULL, hdc);
}


static BOOL CALLBACK TypesDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	static int avaiable = 0;
	static int total = 0;
	static int current = 0;
	static int lineHeight = 0;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			RECT rc;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_EVENT_TYPES), &rc);

			POINT pt = { rc.left, rc.bottom + 5 };
			ScreenToClient(hwndDlg, &pt);
			int origY = pt.y;

			GetClientRect(hwndDlg, &rc);

			HFONT hFont = (HFONT) SendMessage(hwndDlg, WM_GETFONT, 0, 0);
			TEXTMETRIC font;
			GetTextMetric(hFont, &font);

			int height = max(font.tmHeight, 16) + 4;
			int width = rc.right - rc.left - 35;

			lineHeight = height;

			// Create all items
			int id = IDC_EVENT_TYPES + 1;
			for(int i = 0; i < types.getCount(); i++)
			{
				SPEAK_TYPE *type = types[i];

				int x = pt.x;

				// Event type

				HWND icon = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE, 
                        x, pt.y + (height - 16) / 2, 16, 16, hwndDlg, NULL, hInst, NULL);
				x += 20;

				SendMessage(icon, STM_SETICON, (WPARAM) LoadIconEx(type->icon, TRUE), 0);

				HWND tmp = CreateWindowA("STATIC", type->description, WS_CHILD | WS_VISIBLE, 
                        x, pt.y + (height - font.tmHeight) / 2, width - (x - pt.x), font.tmHeight, 
						hwndDlg, NULL, hInst, NULL);
				SendMessage(tmp, WM_SETFONT, (WPARAM) hFont, FALSE);

				if (type->numTemplates <= 0)
				{
					// No templates

					pt.y += height + 3;
					x = pt.x + 20;

					HWND chk = CreateWindow(_T("BUTTON"), TranslateT("Enable"), 
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX, 
							x, pt.y, width - (x - pt.x), height, hwndDlg, (HMENU) id, hInst, NULL);
					SendMessage(chk, BM_SETCHECK, GetSettingBool(type, TEMPLATE_ENABLED, FALSE) ? BST_CHECKED : BST_UNCHECKED, 0);
					SendMessage(chk, WM_SETFONT, (WPARAM) hFont, FALSE);

					pt.y += height + 3;
					x = pt.x + 20;

					chk = CreateWindow(_T("BUTTON"), TranslateT("Speak contact name before text"), 
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX, 
							x, pt.y, width - (x - pt.x), height, hwndDlg, (HMENU) id + 2, hInst, NULL);
					SendMessage(chk, BM_SETCHECK, GetSettingBool(type, SPEAK_NAME, TRUE) ? BST_CHECKED : BST_UNCHECKED, 0);
					SendMessage(chk, WM_SETFONT, (WPARAM) hFont, FALSE);
				}
				else
				{
					// Templates

					Buffer<char> name;
					Buffer<TCHAR> templ;
					for (int i = 0; i < type->numTemplates; i++)
					{
						pt.y += height + 3;
						x = pt.x + 20;

						name.clear();
						const char *end = strchr(type->templates[i], '\n');
						size_t len = (end == NULL ? strlen(type->templates[i]) : end - type->templates[i]);
						name.append(type->templates[i], len);
						name.translate();
						name.append(':');
						name.pack();

						HWND chk = CreateWindowA("BUTTON", name.str, 
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX, 
								x, pt.y, 120, height, hwndDlg, (HMENU) (id + 2 * i), hInst, NULL);
						SendMessage(chk, WM_SETFONT, (WPARAM) hFont, FALSE);
						SendMessage(chk, BM_SETCHECK, GetSettingBool(type, i, TEMPLATE_ENABLED, FALSE) ? BST_CHECKED : BST_UNCHECKED, 0);
						x += 120;

						templ.clear();
						GetTemplate(&templ, type, i);
						templ.pack();

						HWND edit = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), templ.str, 
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, 
								x, pt.y, width - (x - pt.x), height, hwndDlg, (HMENU) (id + 2 * i + 1), hInst, NULL);
						SendMessage(edit, WM_SETFONT, (WPARAM) hFont, FALSE);
					}
				}

				pt.y += height + 10;
				id += 60;
			}

			avaiable = rc.bottom - rc.top;
			total = pt.y - 7;
			current = 0;

			SCROLLINFO si; 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
			si.nMin   = 0; 
			si.nMax   = total; 
			si.nPage  = avaiable; 
			si.nPos   = current; 
			SetScrollInfo(hwndDlg, SB_VERT, &si, TRUE); 

			break;
		}

		case WM_VSCROLL: 
		{ 
			if (lParam != 0)
				break;

			int yDelta;     // yDelta = new_pos - current_pos 
			int yNewPos;    // new position 
 
			switch (LOWORD(wParam)) 
			{ 
				case SB_PAGEUP: 
					yNewPos = current - avaiable / 2; 
					break;  
				case SB_PAGEDOWN: 
					yNewPos = current + avaiable / 2; 
					break; 
				case SB_LINEUP: 
					yNewPos = current - lineHeight; 
					break; 
				case SB_LINEDOWN: 
					yNewPos = current + lineHeight; 
					break; 
				case SB_THUMBPOSITION: 
					yNewPos = HIWORD(wParam); 
					break; 
				case SB_THUMBTRACK:
					yNewPos = HIWORD(wParam); 
					break;
				default: 
					yNewPos = current; 
			} 

			yNewPos = min(total - avaiable, max(0, yNewPos)); 
 
			if (yNewPos == current) 
				break; 
 
			yDelta = yNewPos - current; 
			current = yNewPos; 
 
			// Scroll the window. (The system repaints most of the 
			// client area when ScrollWindowEx is called; however, it is 
			// necessary to call UpdateWindow in order to repaint the 
			// rectangle of pixels that were invalidated.) 
 
			ScrollWindowEx(hwndDlg, 0, -yDelta, (CONST RECT *) NULL, 
				(CONST RECT *) NULL, (HRGN) NULL, (LPRECT) NULL, 
				/* SW_ERASE | SW_INVALIDATE | */ SW_SCROLLCHILDREN); 
//			UpdateWindow(hwndDlg); 
			InvalidateRect(hwndDlg, NULL, TRUE);
 
			// Reset the scroll bar. 
 
			SCROLLINFO si; 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_POS; 
			si.nPos   = current; 
			SetScrollInfo(hwndDlg, SB_VERT, &si, TRUE); 

			break; 
		}

		case WM_COMMAND:
		{
			if ((HWND) lParam != GetFocus())
				break;

			int id = (LOWORD(wParam) - IDC_EVENT_TYPES - 1) % 2;
			if (id == 0)
			{
				// Checkboxes
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			else 
			{
				if (HIWORD(wParam) == EN_CHANGE)
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			if (lpnmhdr->idFrom != 0 || lpnmhdr->code != PSN_APPLY)
				break;

			int id = IDC_EVENT_TYPES + 1;
			for(int i = 0; i < types.getCount(); i++)
			{
				SPEAK_TYPE *type = types[i];

				if (type->numTemplates <= 0)
				{
					// No templates

					WriteSettingBool(type, TEMPLATE_ENABLED, IsDlgButtonChecked(hwndDlg, id));					
					WriteSettingBool(type, SPEAK_NAME, IsDlgButtonChecked(hwndDlg, id + 2));
				}
				else
				{
					// Templates

					for(int i = 0; i < type->numTemplates; i++)
					{
						WriteSettingBool(type, i, TEMPLATE_ENABLED, IsDlgButtonChecked(hwndDlg, id + 2 * i));

						TCHAR tmp[1024];
						GetDlgItemText(hwndDlg, id + 2 * i + 1, tmp, 1024);
						WriteSettingTString(type, i, TEMPLATE_TEXT, tmp);
					}
				}

				id += 60;
			}

			return TRUE;
		}

	}

	return 0;
}

