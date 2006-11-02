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


// Prototypes ///////////////////////////////////////////////////////////////////////////


PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
#ifdef UNICODE
	"Spell Checker (Unicode)",
#else
	"Spell Checker",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,4),
	"Spell Checker",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2006 Ricardo Pescuma Domenecci",
	"http://miranda-im.org/",
	0,	//not transient
	0	//doesn't replace anything built-in
};


#define TIMER_ID 17982


HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hEnableMenu = NULL; 
HANDLE hDisableMenu = NULL; 
HANDLE hModulesLoaded = NULL;
HANDLE hPreBuildCMenu = NULL;
HANDLE hMsgWindowEvent = NULL;
HANDLE hMsgWindowPopup = NULL;

HANDLE hDictionariesFolder = NULL;
TCHAR dictionariesFolder[1024];

HANDLE hCustomDictionariesFolder = NULL;
TCHAR customDictionariesFolder[1024];

char *metacontacts_proto = NULL;
BOOL loaded = FALSE;

Language *languages;
int num_laguages = 0;

std::map<HWND, Dialog *> dialogs;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreBuildContactMenu(WPARAM wParam,LPARAM lParam);
int MsgWindowEvent(WPARAM wParam, LPARAM lParam);
int MsgWindowPopup(WPARAM wParam, LPARAM lParam);

void LoadLanguage(TCHAR *name);

int AddContactTextBox(HANDLE hContact, HWND hwnd, char *name);
int RemoveContactTextBox(HWND hwnd);
int ShowPopupMenu(HWND hwnd, HMENU hMenu, POINT pt);
void AddToCustomDict(Language *lang, char *word);
void LoadCustomDict(Language *lang);

int AddContactTextBoxService(WPARAM wParam, LPARAM lParam);
int RemoveContactTextBoxService(WPARAM wParam, LPARAM lParam);
int ShowPopupMenuService(WPARAM wParam, LPARAM lParam);


// Functions ////////////////////////////////////////////////////////////////////////////


extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	hInst = hinstDLL;
	return TRUE;
}


extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) 
{
	return &pluginInfo;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link) 
{
	pluginLink = link;

	init_mir_malloc();

	// hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hPreBuildCMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	DeInitOptions();

	UnhookEvent(hMsgWindowPopup);
	UnhookEvent(hMsgWindowEvent);
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hPreBuildCMenu);
	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME, 0);

    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://eth0.dk/files/pescuma/spellchecker_version.txt";
		upd.szBetaChangelogURL = "http://eth0.dk/files/pescuma/spellchecker_changelog.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"Spell Checker ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://eth0.dk/files/pescuma/spellcheckerW.zip";
#else
		upd.szBetaUpdateURL = "http://eth0.dk/files/pescuma/spellchecker.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

    // Folders plugin support
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		hDictionariesFolder = (HANDLE) FoldersRegisterCustomPathT(Translate("Spell Checker"), 
					Translate("Dictionaries"), 
					_T(MIRANDA_PATH) _T("\\Dictionaries"));

		FoldersGetCustomPathT(hDictionariesFolder, dictionariesFolder, MAX_REGS(dictionariesFolder), _T("."));

		hCustomDictionariesFolder = (HANDLE) FoldersRegisterCustomPathT(Translate("Spell Checker"), 
					Translate("Custom Dictionaries"), 
					_T(PROFILE_PATH) _T("\\") _T(CURRENT_PROFILE) _T("\\Dictionaries"));

		FoldersGetCustomPathT(hCustomDictionariesFolder, customDictionariesFolder, MAX_REGS(customDictionariesFolder), _T("."));
	}
	else
	{
		GetModuleFileName(GetModuleHandle(NULL), dictionariesFolder, sizeof(dictionariesFolder));

		TCHAR *p = _tcsrchr(dictionariesFolder, _T('\\'));
		if (p != NULL)
			*p = _T('\0');
		lstrcat(dictionariesFolder, "\\Dictionaries");

		lstrcpy(customDictionariesFolder, dictionariesFolder);
	}

	// Load the language files and create an array with then
	TCHAR file[1024];
	mir_sntprintf(file, MAX_REGS(file), "%s\\*.dic", dictionariesFolder);

	// Lets count the files
	WIN32_FIND_DATA ffd = {0};
	HANDLE hFFD = FindFirstFile(file, &ffd);
	if (hFFD != INVALID_HANDLE_VALUE)
	{
		do
		{
			TCHAR tmp[1024];
			mir_sntprintf(tmp, MAX_REGS(tmp), "%s\\%s", dictionariesFolder, ffd.cFileName);

			// Check .dic
			DWORD attrib = GetFileAttributes(tmp);
			if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			// See if .aff exists too
			lstrcpy(&tmp[lstrlen(tmp) - 4], ".aff");
			attrib = GetFileAttributes(tmp);
			if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			num_laguages++;
		}
		while(FindNextFile(hFFD, &ffd));

		FindClose(hFFD);
	}

	if (num_laguages > 0)
	{
		// Oki, lets make our cache struct
		languages = (Language *) malloc(num_laguages * sizeof(Language));
		ZeroMemory(languages, num_laguages * sizeof(Language));

		int i = 0;
		hFFD = FindFirstFile(file, &ffd);
		if (hFFD != INVALID_HANDLE_VALUE)
		{
			do
			{
				TCHAR tmp[1024];
				mir_sntprintf(tmp, MAX_REGS(tmp), "%s\\%s", dictionariesFolder, ffd.cFileName);

				// Check .dic
				DWORD attrib = GetFileAttributes(tmp);
				if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
					continue;

				// See if .aff exists too
				lstrcpy(&tmp[lstrlen(tmp) - 4], ".aff");
				attrib = GetFileAttributes(tmp);
				if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
					continue;

				ffd.cFileName[lstrlen(ffd.cFileName)-4] = _T('\0');

				lstrcpy(languages[i].name, ffd.cFileName);
				languages[i].loaded = LANGUAGE_NOT_LOADED;
				languages[i].checker = NULL;

				i++;
			}
			while(i < num_laguages && FindNextFile(hFFD, &ffd));

			FindClose(hFFD);
		}
	}

	InitOptions();

	if (opts.default_language[0] != _T('\0'))
		LoadLanguage(opts.default_language);

	hMsgWindowEvent = HookEvent(ME_MSG_WINDOWEVENT,&MsgWindowEvent);
	hMsgWindowPopup = HookEvent(ME_MSG_WINDOWPOPUP,&MsgWindowPopup);

	CreateServiceFunction(MS_SPELLCHECKER_ADD_RICHEDIT, AddContactTextBoxService);
	CreateServiceFunction(MS_SPELLCHECKER_REMOVE_RICHEDIT, RemoveContactTextBoxService);
	CreateServiceFunction(MS_SPELLCHECKER_SHOW_POPUP_MENU, ShowPopupMenuService);

	loaded = TRUE;

	return 0;
}

int PreBuildContactMenu(WPARAM wParam,LPARAM lParam) 
{
	return 0;
}


void SetAttributes(HWND hRichEdit, int pos_start, int pos_end, DWORD dwMask, DWORD dwEffects, BYTE bUnderlineType, BOOL all = FALSE)
{
    CHARRANGE old_sel;
	if (!all)
	{
		// Get old selecton
	    SendMessage(hRichEdit, EM_EXGETSEL, 0, (LPARAM) &old_sel);

		// Select this range
		CHARRANGE sel = { pos_start, pos_end };
		SendMessage(hRichEdit, EM_EXSETSEL, 0, (LPARAM) &sel);
	}

	CHARFORMAT2 CharFormat;
	CharFormat.cbSize = sizeof(CHARFORMAT2);
	SendMessage(hRichEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&CharFormat);
	CharFormat.dwMask = dwMask;
	CharFormat.dwEffects = dwEffects;
	CharFormat.bUnderlineType = bUnderlineType;
	SendMessage(hRichEdit, EM_SETCHARFORMAT, (WPARAM) all ? SCF_ALL : SCF_SELECTION, (LPARAM)&CharFormat);

	if (!all)
	{
		// Back to old selection
		SendMessage(hRichEdit, EM_EXSETSEL, 0, (LPARAM) &old_sel);
	}
}


void SetAttributes(HWND hRichEdit, DWORD dwMask, DWORD dwEffects, BYTE bUnderlineType)
{
	SetAttributes(hRichEdit, 0, 0, dwMask, dwEffects, bUnderlineType, TRUE);
}



BOOL IsAlpha(Dialog *dlg, TCHAR c)
{
#ifdef UNICODE
	return iswalpha(c) 
		|| (c == L'-' && !dlg->lang->checker->get_forbidden_compound());
#else
	return (!_istcntrl(c) && !_istdigit(c) && !_istpunct(c) && !_istspace(c)) 
		|| (c == '-'  && !dlg->lang->checker->get_forbidden_compound());
#endif
}


void ReplaceWord(Dialog *dlg, CHARRANGE &sel, char *new_word)
{
	CHARRANGE old_sel;
	SendMessage(dlg->hwnd, EM_EXGETSEL, 0, (LPARAM) &old_sel);

	// Replace in rich edit
	SendMessage(dlg->hwnd, EM_EXSETSEL, 0, (LPARAM) &sel);
	SendMessage(dlg->hwnd, EM_REPLACESEL, TRUE, (LPARAM) new_word);

	// Fix old sel
	int dif = lstrlen(new_word) - sel.cpMax + sel.cpMin;
	if (old_sel.cpMin >= sel.cpMax)
		old_sel.cpMin += dif;
	if (old_sel.cpMax >= sel.cpMax)
		old_sel.cpMax += dif;

	SendMessage(dlg->hwnd, EM_EXSETSEL, 0, (LPARAM) &old_sel);
}


// The line of text must be read into dlg->text
// ini, end are global pos
// It breaks that string in end pos
void DealWord(Dialog *dlg, int ini, int end, BOOL auto_correct)
{
	if (ini >= end)
		return;

	// Fix values to be relative to text
	int line = SendMessage(dlg->hwnd, EM_LINEFROMCHAR, (WPARAM) ini, 0);
	int first_char = SendMessage(dlg->hwnd, EM_LINEINDEX, (WPARAM) line, 0);

	dlg->text[end - first_char] = _T('\0');

	// TODO: Convert to UTF8
	if (!dlg->lang->checker->spell(&dlg->text[ini - first_char]))
	{
		BOOL mark = TRUE;

		if (auto_correct)
		{
			int num_suggestions = 0;
			char ** suggestions;

			num_suggestions = dlg->lang->checker->suggest_auto(&suggestions, &dlg->text[ini - first_char]);
			if (num_suggestions > 0)
			{
				mark = FALSE;

				CHARRANGE sel = { ini, end };
				ReplaceWord(dlg, sel, suggestions[0]);
			}
		}
		
		if (mark)
			SetAttributes(dlg->hwnd, ini, end, 
						CFM_UNDERLINETYPE, 0, CFU_UNDERLINEWAVE | 0x50);
	}
}

void GetDlgTextLine(Dialog *dlg, int line)
{
	*((WORD*)dlg->text) = MAX_REGS(dlg->text);
	SendMessage(dlg->hwnd, EM_GETLINE, (WPARAM) line, (LPARAM) dlg->text);
	dlg->text[MAX_REGS(dlg->text)-1] = _T('\0');
}

void CheckText(Dialog *dlg, BOOL auto_correct)
{
	SendMessage(dlg->hwnd, WM_SETREDRAW, FALSE, 0);

	POINT old_scroll_pos;
	SendMessage(dlg->hwnd, EM_GETSCROLLPOS, 0, (LPARAM) &old_scroll_pos);

	SetAttributes(dlg->hwnd, CFM_UNDERLINE | CFM_UNDERLINETYPE, 0, 0);

	// Get text
	int lines = SendMessage(dlg->hwnd, EM_GETLINECOUNT, 0, 0);
	for(int line = 0; line < lines; line++) 
	{
		GetDlgTextLine(dlg, line);
		int len = lstrlen(dlg->text);
		int first_char = SendMessage(dlg->hwnd, EM_LINEINDEX, (WPARAM) line, 0);

		// Now lets get the words
		int last_pos = -1;
		for (int pos = len - 1; pos >= 0; pos--)
		{
			if (!IsAlpha(dlg, dlg->text[pos]))
			{
				if (last_pos != -1)
				{
					// We found a word
					DealWord(dlg, pos + 1 + first_char, last_pos + 1 + first_char, auto_correct);
					last_pos = -1;
				}
			}
			else 
			{
				if (last_pos == -1)
					last_pos = pos;
			}
		}

		if (last_pos != -1)
		{
			// Last word
			DealWord(dlg, pos + 1 + first_char, last_pos + 1 + first_char, auto_correct);
		}
	}

	int len = GetWindowTextLength(dlg->hwnd);
	SetAttributes(dlg->hwnd, len, len, CFM_UNDERLINE | CFM_UNDERLINETYPE, 0, 0);

	SendMessage(dlg->hwnd, EM_SETSCROLLPOS, 0, (LPARAM) &old_scroll_pos);

	SendMessage(dlg->hwnd, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(dlg->hwnd, NULL, FALSE);
}

void GetWordCharRange(Dialog *dlg, CHARRANGE &sel)
{
	sel.cpMax = sel.cpMin;
	int line = SendMessage(dlg->hwnd, EM_LINEFROMCHAR, (WPARAM) sel.cpMin, 0);
	int first_char = SendMessage(dlg->hwnd, EM_LINEINDEX, (WPARAM) line, 0);
	GetDlgTextLine(dlg, line);

	// Find the word
	sel.cpMin--;
	while (sel.cpMin >= first_char && IsAlpha(dlg, dlg->text[sel.cpMin - first_char]))
		sel.cpMin--;
	sel.cpMin++;

	while (dlg->text[sel.cpMax - first_char] != _T('\0') && IsAlpha(dlg, dlg->text[sel.cpMax - first_char]))
		sel.cpMax++;
}

LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Dialog *dlg = dialogs[hwnd];
	if (dlg == NULL)
		return -1;

	LRESULT ret = CallWindowProc(dlg->old_edit_proc, hwnd, msg, wParam, lParam);

	switch(msg)
	{
		case WM_CHAR:
		{
			// Need to do that to avoid changing the word while typing
			KillTimer(hwnd, TIMER_ID);
			SetTimer(hwnd, TIMER_ID, 1000, NULL);

			dlg->changed = TRUE;

			if (!dlg->enabled || dlg->lang->loaded != LANGUAGE_LOADED)
				break;

			int len = GetWindowTextLength(hwnd);
			if (len <= 0)
				break;

			// Handle the current word under cursor
			CHARRANGE sel;
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);
			int old_pos = sel.cpMin;

			SetAttributes(dlg->hwnd, old_pos > 0 ? old_pos - 1 : 0, old_pos, CFM_UNDERLINE | CFM_UNDERLINETYPE, 0, 0);

			// Next word
			if (old_pos < len - 1)
			{
				sel.cpMax = sel.cpMin = old_pos + 1;
				GetWordCharRange(dlg, sel);
				if (sel.cpMin < sel.cpMax)
				{
					SendMessage(dlg->hwnd, WM_SETREDRAW, FALSE, 0);
					SetAttributes(dlg->hwnd, sel.cpMin, sel.cpMax, CFM_UNDERLINE | CFM_UNDERLINETYPE, 0, 0);

					DealWord(dlg, sel.cpMin, sel.cpMax, FALSE);

					SetAttributes(dlg->hwnd, old_pos, old_pos, CFM_UNDERLINE | CFM_UNDERLINETYPE, 0, 0);
					SendMessage(dlg->hwnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(dlg->hwnd, NULL, FALSE);
				}
			}

			// Prev word
			if (old_pos > 0)
			{
				sel.cpMax = sel.cpMin = old_pos - 1;
				GetWordCharRange(dlg, sel);
				if (sel.cpMin < sel.cpMax)
				{
					SendMessage(dlg->hwnd, WM_SETREDRAW, FALSE, 0);
					SetAttributes(dlg->hwnd, sel.cpMin, sel.cpMax, CFM_UNDERLINE | CFM_UNDERLINETYPE, 0, 0);

					DealWord(dlg, sel.cpMin, sel.cpMax, FALSE);

					SetAttributes(dlg->hwnd, old_pos, old_pos, CFM_UNDERLINE | CFM_UNDERLINETYPE, 0, 0);
					SendMessage(dlg->hwnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(dlg->hwnd, NULL, FALSE);
				}
			}

			break;
		}
		case EM_PASTESPECIAL:
		case WM_PASTE:
		{
			// Need to do that to avoid changing the word while typing
			KillTimer(hwnd, TIMER_ID);
			SetTimer(hwnd, TIMER_ID, 1000, NULL);

			dlg->changed = TRUE;

			if (!dlg->enabled || dlg->lang->loaded != LANGUAGE_LOADED)
				break;

			int len = GetWindowTextLength(hwnd);
			if (len <= 0)
				break;

			// Parse all text
			CheckText(dlg, FALSE);
			break;
		}

		case WM_TIMER:
		{
			if (wParam != TIMER_ID)
				break;

			if (!dlg->enabled || dlg->lang->loaded != LANGUAGE_LOADED)
				break;

			int len = GetWindowTextLength(hwnd);
			if (len <= 0)
				break;

			if (len == dlg->old_text_len && !dlg->changed)
				break;

			dlg->old_text_len = len;
			dlg->changed = FALSE;

			CheckText(dlg, opts.auto_correct);
			break;
		}
	}

	return ret;
}


int GetContactLanguage(HANDLE hContact)
{
	if (num_laguages <= 0)
		return -1;

	TCHAR lang[64];
	DBVARIANT dbv = {0};

	if (hContact != NULL && !DBGetContactSettingTString(hContact, MODULE_NAME, "Language", &dbv)) 
	{
		lstrcpyn(lang, dbv.ptszVal, MAX_REGS(lang));
		DBFreeVariant(&dbv);
	}
	else
	{
		lstrcpyn(lang, opts.default_language, MAX_REGS(lang));
	}

	for(int i = 0; i < num_laguages; i++)
	{
		if (lstrcmp(languages[i].name, lang) == 0)
			return i;
	}

	return 0;
}


int AddContactTextBoxService(WPARAM wParam, LPARAM lParam)
{
	SPELLCHECKER_ITEM *sci = (SPELLCHECKER_ITEM *) wParam;
	if (sci == NULL || sci->cbSize != sizeof(SPELLCHECKER_ITEM))
		return -1;

	return AddContactTextBox(sci->hContact, sci->hwnd, sci->window_name);
}

int AddContactTextBox(HANDLE hContact, HWND hwnd, char *name) 
{
	if (dialogs[hwnd] == NULL)
	{
		int l = GetContactLanguage(hContact);
		if (l < 0)
			return -1;

		// Fill dialog data
		Dialog *dlg = (Dialog *) malloc(sizeof(Dialog));
		ZeroMemory(dlg, sizeof(Dialog));

		dialogs[hwnd] = dlg;

		dlg->hContact = hContact;
		dlg->hwnd = hwnd;
		dlg->lang = &languages[l];
		strncpy(dlg->name, name, sizeof(dlg->name));
		dlg->enabled = DBGetContactSettingByte(NULL, MODULE_NAME, dlg->name, 1);
		dlg->old_edit_proc = (WNDPROC) SetWindowLong(hwnd, GWL_WNDPROC, (LONG) EditProc);

		SetTimer(hwnd, TIMER_ID, 1000, NULL);

		LoadLanguage(dlg->lang->name);
	}

	return 0;
}


void FreePopupData(Dialog *dlg)
{
	if (dlg->word != NULL)
		free(dlg->word);

	if (dlg->num_suggestions > 0)
	{
		for (int i = dlg->num_suggestions - 1; i >= 0; i--) 
			free(dlg->suggestions[i]);
		free(dlg->suggestions);
	}

	dlg->num_suggestions = 0;
	dlg->suggestions = NULL;
	dlg->word = NULL;
}


int RemoveContactTextBoxService(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = (HWND) wParam;
	if (hwnd == NULL)
		return -1;

	return RemoveContactTextBox(hwnd);
}


int RemoveContactTextBox(HWND hwnd) 
{
	Dialog *dlg = dialogs[hwnd];

	if (dlg != NULL) 
	{
		KillTimer(hwnd, TIMER_ID);

		SetWindowLong(hwnd, GWL_WNDPROC, (LONG) dlg->old_edit_proc);

		dialogs.erase(hwnd);

		FreePopupData(dlg);
		free(dlg);
	}

	return 0;
}


CHARRANGE GetCharRangeUnderPoint(Dialog *dlg, POINT pt)
{
	CHARRANGE sel;
	sel.cpMin = sel.cpMax = LOWORD(SendMessage(dlg->hwnd, EM_CHARFROMPOS, 0, (LPARAM) &pt));

	// Find the word
	GetWordCharRange(dlg, sel);
	
	return sel;
}


TCHAR *GetWordUnderPoint(Dialog *dlg, POINT pt)
{
	// Get text
	if (GetWindowTextLength(dlg->hwnd) <= 0)
		return NULL;

	CHARRANGE sel = GetCharRangeUnderPoint(dlg, pt);

	int line = SendMessage(dlg->hwnd, EM_LINEFROMCHAR, (WPARAM) sel.cpMin, 0);
	int first_char = SendMessage(dlg->hwnd, EM_LINEINDEX, (WPARAM) line, 0);

	dlg->text[sel.cpMax - first_char] = _T('\0');

	TCHAR *ret;
	if (dlg->text[sel.cpMin - first_char] == _T('\0'))
		ret = NULL;
	else
		ret = _tcsdup(&dlg->text[sel.cpMin - first_char]);

	return ret;
}


void AddItemsToMenu(Dialog *dlg, HMENU hMenu, POINT pt)
{
	FreePopupData(dlg);

	BOOL wrong_word = FALSE;

	// Get text
	if (dlg->lang->loaded == LANGUAGE_LOADED)
	{
		dlg->word = GetWordUnderPoint(dlg, pt);
		if (dlg->word != NULL) 
		{
			wrong_word = !dlg->lang->checker->spell(dlg->word);
			if (wrong_word)
			{
				// Get suggestions
				dlg->num_suggestions = dlg->lang->checker->suggest(&dlg->suggestions, dlg->word);
			}
		}
	}

	// Make menu
	if (GetMenuItemCount(hMenu) > 0)
		InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

	InsertMenu(hMenu, 0, MF_BYPOSITION, dlg->num_suggestions + 3, TranslateT("Enable spell checking"));
	CheckMenuItem(hMenu, dlg->num_suggestions + 3, MF_BYCOMMAND | (dlg->enabled ? MF_CHECKED : MF_UNCHECKED));

	if (wrong_word) 
	{
		InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

		InsertMenu(hMenu, 0, MF_BYPOSITION, dlg->num_suggestions + 2, TranslateT("Ignore all"));
		InsertMenu(hMenu, 0, MF_BYPOSITION, dlg->num_suggestions + 1, TranslateT("Add to dictionary"));

		if (dlg->num_suggestions > 0)
		{
			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
			for (int i = dlg->num_suggestions - 1; i >= 0; i--) 
				InsertMenu(hMenu, 0, MF_BYPOSITION, i + 1, dlg->suggestions[i]);
		}

		InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

		TCHAR text[128];
		mir_sntprintf(text, MAX_REGS(text), TranslateT("Wrong word: %s"), dlg->word);
		InsertMenu(hMenu, 0, MF_BYPOSITION, 0, text);
	}
}


BOOL HandleMenuSelection(Dialog *dlg, POINT pt, int selection)
{
	BOOL ret = FALSE;

	if (selection > 0 && selection <= dlg->num_suggestions)
	{
		selection--;

		// Get text
		int len = GetWindowTextLength(dlg->hwnd);
		if (len > 0)
		{
			CHARRANGE sel = GetCharRangeUnderPoint(dlg, pt);

			SendMessage(dlg->hwnd, WM_SETREDRAW, FALSE, 0);

			ReplaceWord(dlg, sel, dlg->suggestions[selection]);
			
			SendMessage(dlg->hwnd, WM_SETREDRAW, TRUE, 0);
			InvalidateRect(dlg->hwnd, NULL, FALSE);
		}

		ret = TRUE;
	}
	else if (selection == dlg->num_suggestions + 1)
	{
		AddToCustomDict(dlg->lang, dlg->word);

		ret = TRUE;
	}
	else if (selection == dlg->num_suggestions + 2)
	{
		dlg->lang->checker->put_word(dlg->word);

		ret = TRUE;
	}
	else if (selection == dlg->num_suggestions + 3)
	{
		dlg->enabled = !dlg->enabled;
		DBWriteContactSettingByte(NULL, MODULE_NAME, dlg->name, dlg->enabled);

		if (!dlg->enabled)
			SetAttributes(dlg->hwnd, CFM_UNDERLINE | CFM_UNDERLINETYPE, 0, 0);

		ret = TRUE;
	}

	if (ret)
		dlg->changed = TRUE;

	FreePopupData(dlg);

	return ret;
}


int MsgWindowPopup(WPARAM wParam, LPARAM lParam)
{
	MessageWindowPopupData *mwpd = (MessageWindowPopupData *) lParam;
	if (mwpd == NULL || mwpd->cbSize < sizeof(MessageWindowPopupData)
			|| mwpd->uFlags != MSG_WINDOWPOPUP_INPUT)
		return 0;

	Dialog *dlg = dialogs[mwpd->hwnd];
	if (dlg == NULL) 
		return -1;

	POINT pt = mwpd->pt;
	ScreenToClient(dlg->hwnd, &pt);

	if (mwpd->uType == MSG_WINDOWPOPUP_SHOWING)
	{
		AddItemsToMenu(dlg, mwpd->hMenu, pt);
	}
	else if (mwpd->uType == MSG_WINDOWPOPUP_SELECTED)
	{
		HandleMenuSelection(dlg, pt, mwpd->selection);
	}
}


int ShowPopupMenuService(WPARAM wParam, LPARAM lParam)
{
	SPELLCHECKER_POPUPMENU *scp = (SPELLCHECKER_POPUPMENU *) wParam;
	if (scp == NULL || scp->cbSize != sizeof(SPELLCHECKER_POPUPMENU))
		return -1;

	return ShowPopupMenu(scp->hwnd, scp->hMenu, scp->pt);
}


int ShowPopupMenu(HWND hwnd, HMENU hMenu, POINT pt)
{
	Dialog *dlg = dialogs[hwnd];
	if (dlg == NULL) 
		return -1;

	if (pt.x == 0xFFFF && pt.y == 0xFFFF)
	{
		CHARRANGE sel;
		SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);

		// Get current cursor pos
		SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM) sel.cpMax);
	}
	else
	{
		ScreenToClient(hwnd, &pt);
	}

	BOOL create_menu = (hMenu == NULL);
	if (create_menu)
		hMenu = CreatePopupMenu();

	// Make menu
	AddItemsToMenu(dlg, hMenu, pt);

	// Show menu
	ClientToScreen(hwnd, &pt);
	int selection = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);

	// Do action
	if (HandleMenuSelection(dlg, pt, selection))
		selection = 0;

	if (create_menu)
		DestroyMenu(hMenu);

	return selection;
}


int MsgWindowEvent(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;
	if (event == NULL)
		return 0;

	if (event->cbSize < sizeof(MessageWindowEventData))
		return 0;

	if (event->uType == MSG_WINDOW_EVT_OPEN)
	{
		AddContactTextBox(event->hContact, event->hwndInput, "DefaultSRMM");
	}
	else if (event->uType == MSG_WINDOW_EVT_CLOSING)
	{
		RemoveContactTextBox(event->hwndInput);
	}

	return 0;
}


DWORD WINAPI LoadLanguageThread(LPVOID pos)
{
	TCHAR dic[1024];
	TCHAR aff[1024];

	int i = (int) pos;

	mir_sntprintf(dic, MAX_REGS(dic), "%s\\%s.dic", dictionariesFolder, languages[i].name);
	mir_sntprintf(aff, MAX_REGS(aff), "%s\\%s.aff", dictionariesFolder, languages[i].name);

	languages[i].checker = new Hunspell(aff, dic);
	LoadCustomDict(&languages[i]);
	languages[i].loaded = LANGUAGE_LOADED;

	return 0;
}


void LoadLanguage(TCHAR *name)
{
	DWORD thread_id;

	int i;
	for(i = 0; i < num_laguages; i++)
	{
		if (lstrcmp(name, languages[i].name) == 0)
		{
			if (languages[i].loaded == LANGUAGE_NOT_LOADED)
			{
				languages[i].loaded = LANGUAGE_LOADING;
				CreateThread(NULL, 0, LoadLanguageThread, (LPVOID) i, 0, &thread_id);
			}
			break;
		}
	}
}


void AppendToCustomDict(Language *lang, char *word)
{
	char filename[1024];
	mir_sntprintf(filename, MAX_REGS(filename), "%s\\%s.cdic", customDictionariesFolder, lang->name);

    FILE *file = fopen(filename,"a");
    if (file != NULL) 
	{
		fprintf(file, "%s\n", word);
	    fclose(file);
    }
}


void AddToCustomDict(Language *lang, char *word)
{
	lang->checker->put_word(word);
	AppendToCustomDict(lang, word);
}


void LoadCustomDict(Language *lang) 
{
	char filename[1024];
	mir_sntprintf(filename, MAX_REGS(filename), "%s\\%s.cdic", customDictionariesFolder, lang->name);

    FILE *file = fopen(filename,"r");
	if (file != NULL) 
	{
		char word[MAXLNLEN];
		while(fgets(word, MAXLNLEN, file)) 
		{
			size_t len = strlen(word);
			if (*(word + len - 1) == '\n') 
				*(word + len - 1) = '\0';

			lang->checker->put_word(word);
		}
		fclose(file);
	}
}

