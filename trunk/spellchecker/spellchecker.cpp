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
	PLUGIN_MAKE_VERSION(0,0,1,4),
	"Spell Checker",
	"Ricardo Pescuma Domenecci",
	"",
	"� 2006 Ricardo Pescuma Domenecci",
	"http://miranda-im.org/",
	0,	//not transient
	0	//doesn't replace anything built-in
};


#define TIMER_ID 17982


HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hModulesLoaded = NULL;
HANDLE hPreShutdownHook = NULL;
HANDLE hMsgWindowEvent = NULL;
HANDLE hMsgWindowPopup = NULL;

HANDLE hDictionariesFolder = NULL;
TCHAR dictionariesFolder[1024];

HANDLE hCustomDictionariesFolder = NULL;
TCHAR customDictionariesFolder[1024];

char *metacontacts_proto = NULL;
BOOL loaded = FALSE;

Dictionaries languages = {0};

std::map<HWND, Dialog *> dialogs;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);
int MsgWindowEvent(WPARAM wParam, LPARAM lParam);
int MsgWindowPopup(WPARAM wParam, LPARAM lParam);

int AddContactTextBox(HANDLE hContact, HWND hwnd, char *name);
int RemoveContactTextBox(HWND hwnd);
int ShowPopupMenu(HWND hwnd, HMENU hMenu, POINT pt, HWND hwndOwner);

int AddContactTextBoxService(WPARAM wParam, LPARAM lParam);
int RemoveContactTextBoxService(WPARAM wParam, LPARAM lParam);
int ShowPopupMenuService(WPARAM wParam, LPARAM lParam);

BOOL GetWordCharRange(Dialog *dlg, CHARRANGE &sel, TCHAR *text, size_t text_len, int &first_char);
TCHAR *GetWordUnderPoint(Dialog *dlg, POINT pt, CHARRANGE &sel);

typedef void (*FoundWrongWordCallback)(TCHAR *word, CHARRANGE pos, void *param);


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
	hPreShutdownHook = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	DestroyServiceFunction(MS_SPELLCHECKER_ADD_RICHEDIT);
	DestroyServiceFunction(MS_SPELLCHECKER_REMOVE_RICHEDIT);
	DestroyServiceFunction(MS_SPELLCHECKER_SHOW_POPUP_MENU);

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
		lstrcat(dictionariesFolder, _T("\\Dictionaries"));

		lstrcpy(customDictionariesFolder, dictionariesFolder);
	}

	languages = GetAvaibleDictionaries(dictionariesFolder, customDictionariesFolder);

	InitOptions();

	if (opts.default_language[0] != _T('\0'))
	{
		for(int i = 0; i < languages.count; i++)
		{
			if (lstrcmp(languages.dicts[i]->language, opts.default_language) == 0)
			{
				languages.dicts[i]->load();
				break;
			}
		}
	}

	hMsgWindowEvent = HookEvent(ME_MSG_WINDOWEVENT,&MsgWindowEvent);
	hMsgWindowPopup = HookEvent(ME_MSG_WINDOWPOPUP,&MsgWindowPopup);

	CreateServiceFunction(MS_SPELLCHECKER_ADD_RICHEDIT, AddContactTextBoxService);
	CreateServiceFunction(MS_SPELLCHECKER_REMOVE_RICHEDIT, RemoveContactTextBoxService);
	CreateServiceFunction(MS_SPELLCHECKER_SHOW_POPUP_MENU, ShowPopupMenuService);

	loaded = TRUE;

	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	DeInitOptions();

	UnhookEvent(hMsgWindowPopup);
	UnhookEvent(hMsgWindowEvent);
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hPreShutdownHook);

	return 0;
}


void SetUnderline(HWND hRichEdit, int pos_start, int pos_end, BOOL all = FALSE, BOOL disable = FALSE)
{
	if (!all)
	{
		// Select this range
		CHARRANGE sel = { pos_start, pos_end };
		SendMessage(hRichEdit, EM_EXSETSEL, 0, (LPARAM) &sel);
	}

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_UNDERLINE | CFM_UNDERLINETYPE;
	cf.dwEffects = disable ? 0 : CFE_UNDERLINE;
	cf.bUnderlineType = disable ? 0 : ((opts.underline_type + CFU_UNDERLINEDOUBLE) | 0x50);
	SendMessage(hRichEdit, EM_SETCHARFORMAT, (WPARAM) all ? SCF_ALL : SCF_SELECTION, (LPARAM)&cf);
}


void SetNoUnderline(HWND hRichEdit, int pos_start, int pos_end)
{
	SetUnderline(hRichEdit, pos_start, pos_end, FALSE, TRUE);
}

void SetNoUnderline(HWND hRichEdit)
{
	SetUnderline(hRichEdit, 0, 0, TRUE, TRUE);
}


inline void GetLineOfText(Dialog *dlg, int line, int &first_char, TCHAR *text, size_t text_len)
{
	first_char = SendMessage(dlg->hwnd, EM_LINEINDEX, (WPARAM) line, 0);

	*((WORD*)text) = text_len;
	SendMessage(dlg->hwnd, EM_GETLINE, (WPARAM) line, (LPARAM) text);
	text[text_len-1] = _T('\0');
}

// Helper to avoid copy and pastle
inline void DealWord(Dialog *dlg, TCHAR *text, int &first_char, int &last_pos, int &pos, 
					 CHARRANGE &old_sel, BOOL auto_correct, 
					 FoundWrongWordCallback callback, void *param)
{
	// Is wrong?
	text[last_pos + 1] = _T('\0');
	if (!dlg->lang->spell(&text[pos + 1]))
	{
		CHARRANGE sel = { first_char + pos + 1, first_char + last_pos + 1 };
		BOOL mark = TRUE;

		// Has to correct?
		if (auto_correct)
		{
			TCHAR *word = dlg->lang->autoReplace(&text[pos + 1]);
			if (word == NULL)
				word = dlg->lang->autoSuggestOne(&text[pos + 1]);

			if (word != NULL)
			{
				mark = FALSE;

				// Replace in rich edit
				SendMessage(dlg->hwnd, EM_EXSETSEL, 0, (LPARAM) &sel);
				SendMessage(dlg->hwnd, EM_REPLACESEL, TRUE, (LPARAM) word);

				// Fix old sel
				int dif = lstrlen(word) - sel.cpMax + sel.cpMin;
				if (old_sel.cpMin >= sel.cpMax)
					old_sel.cpMin += dif;
				if (old_sel.cpMax >= sel.cpMax)
					old_sel.cpMax += dif;

				free(word);
			}
		}
		
		// Mark
		if (mark)
		{
			SetUnderline(dlg->hwnd, sel.cpMin, sel.cpMax);
				
			if (callback != NULL)
				callback(&text[pos + 1], sel, param);
		}
	}
}

// Checks for errors in all text
void CheckText(Dialog *dlg, BOOL check_word_under_cursor, BOOL auto_correct, 
			   FoundWrongWordCallback callback = NULL, void *param = NULL)
{
	// Stop rich edit
	SendMessage(dlg->hwnd, WM_SETREDRAW, FALSE, 0);
	POINT old_scroll_pos;
	SendMessage(dlg->hwnd, EM_GETSCROLLPOS, 0, (LPARAM) &old_scroll_pos);
	CHARRANGE old_sel;
	SendMessage(dlg->hwnd, EM_EXGETSEL, 0, (LPARAM) &old_sel);
	POINT caretPos;
	GetCaretPos(&caretPos);
	BOOL inverse = (old_sel.cpMin >= LOWORD(SendMessage(dlg->hwnd, EM_CHARFROMPOS, 0, (LPARAM) &caretPos)));

	if (GetWindowTextLength(dlg->hwnd) > 0)
	{
		int lines = SendMessage(dlg->hwnd, EM_GETLINECOUNT, 0, 0);
		int line = 0;

		if (!check_word_under_cursor)
		{
			// Check only the current line, one up and one down
			int current_line = SendMessage(dlg->hwnd, EM_LINEFROMCHAR, (WPARAM) old_sel.cpMin, 0);
			line = max(line, current_line - 1);
			lines = min(lines, current_line + 2);
		}

		for(; line < lines; line++) 
		{
			TCHAR text[1024];
			int first_char;

			GetLineOfText(dlg, line, first_char, text, MAX_REGS(text));
			int len = lstrlen(text);

			SetNoUnderline(dlg->hwnd, first_char, first_char + len);

			// Now lets get the words
			int last_pos = -1;
			BOOL found_real_char = FALSE;
			for (int pos = len - 1; pos >= 0; pos--)
			{
				if (!dlg->lang->isWordChar(text[pos]))
				{
					if (last_pos != -1)
					{
						// We found a word

						// It has real chars?
						if (found_real_char)
						{
							// Is under cursor?
							if (check_word_under_cursor 
								|| !(first_char+pos+1 <= old_sel.cpMax && first_char+last_pos+1 >= old_sel.cpMin))
							{
								DealWord(dlg, text, first_char, last_pos, pos, old_sel, auto_correct, callback, param);
							}
						}

						last_pos = -1;
						found_real_char = FALSE;
					}
				}
				else 
				{
					if (last_pos == -1)
						last_pos = pos;

					if (text[pos] != _T('-'))
						found_real_char = TRUE;
				}
			}

			// Last word
			if (last_pos != -1)
			{
				// It has real chars?
				if (found_real_char)
				{
					// Is under cursor?
					if (check_word_under_cursor || !(pos+1 <= old_sel.cpMax && last_pos+1 >= old_sel.cpMin))
					{
						DealWord(dlg, text, first_char, last_pos, pos, old_sel, auto_correct, callback, param);
					}
				}
			}
		}
	}

	// Fix last char
	int len = GetWindowTextLength(dlg->hwnd);
	SetNoUnderline(dlg->hwnd, len, len);

	// Start rich edit
	if (inverse) 
	{
		LONG tmp = old_sel.cpMin;
		old_sel.cpMin = old_sel.cpMax;
		old_sel.cpMax = tmp;
	}
	SendMessage(dlg->hwnd, EM_EXSETSEL, 0, (LPARAM) &old_sel);
	SendMessage(dlg->hwnd, EM_SETSCROLLPOS, 0, (LPARAM) &old_scroll_pos);
	SendMessage(dlg->hwnd, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(dlg->hwnd, NULL, FALSE);
}


LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Dialog *dlg = dialogs[hwnd];
	if (dlg == NULL)
		return -1;

	LRESULT ret = CallWindowProc(dlg->old_edit_proc, hwnd, msg, wParam, lParam);

	switch(msg)
	{
		case WM_KEYDOWN:
		{
			if (wParam != VK_DELETE
				&& wParam != VK_UP
				&& wParam != VK_DOWN
				&& wParam != VK_HOME
				&& wParam != VK_END)
				break;
		}
		case WM_CHAR:
		{
			if (lParam & (1 << 28))	// ALT key
				break;

			if (GetKeyState(VK_CONTROL) & 0x8000)	// CTRL key
				break;

			// Need to do that to avoid changing the word while typing
			KillTimer(hwnd, TIMER_ID);
			SetTimer(hwnd, TIMER_ID, 1000, NULL);

			dlg->changed = TRUE;

			if ((lParam & 0xFF) > 1)	// Repeat rate
				break;

			if (!dlg->enabled || dlg->lang == NULL || !dlg->lang->isLoaded())
				break;

			// Don't check if field is read-only
			if (GetWindowLong(hwnd, GWL_STYLE) & ES_READONLY)
				break;

			TCHAR c = (TCHAR) wParam;
			if (!dlg->lang->isWordChar(c))
			{
				CheckText(dlg, FALSE, FALSE);
			}
			else
			{
				// Assert no selection
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				SendMessage(dlg->hwnd, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf);

				if ((cf.dwMask & CFM_UNDERLINETYPE) 
					&& (cf.dwEffects & 0x0F) >= CFU_UNDERLINEDOUBLE
					&& (cf.dwEffects & 0x0F) <= CFU_UNDERLINETHICK)
				{
					// Stop rich edit
					SendMessage(dlg->hwnd, WM_SETREDRAW, FALSE, 0);
					POINT old_scroll_pos;
					SendMessage(dlg->hwnd, EM_GETSCROLLPOS, 0, (LPARAM) &old_scroll_pos);
				    CHARRANGE old_sel;
				    SendMessage(dlg->hwnd, EM_EXGETSEL, 0, (LPARAM) &old_sel);
					POINT caretPos;
					GetCaretPos(&caretPos);
					BOOL inverse = (old_sel.cpMin >= LOWORD(SendMessage(dlg->hwnd, EM_CHARFROMPOS, 0, (LPARAM) &caretPos)));

					// Remove underline of current word
					TCHAR text[1024];
					int first_char;
					CHARRANGE sel;

					SendMessage(dlg->hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);

					GetWordCharRange(dlg, sel, text, MAX_REGS(text), first_char);

					SetNoUnderline(dlg->hwnd, sel.cpMin, sel.cpMax);

					// Start rich edit
					if (inverse) 
					{
						LONG tmp = old_sel.cpMin;
						old_sel.cpMin = old_sel.cpMax;
						old_sel.cpMax = tmp;
					}
					SendMessage(dlg->hwnd, EM_EXSETSEL, 0, (LPARAM) &old_sel);
					SendMessage(dlg->hwnd, EM_SETSCROLLPOS, 0, (LPARAM) &old_scroll_pos);
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

			if (!dlg->enabled || dlg->lang == NULL || !dlg->lang->isLoaded())
				break;

			// Don't check if field is read-only
			if (GetWindowLong(hwnd, GWL_STYLE) & ES_READONLY)
				break;

			// Parse all text
			CheckText(dlg, TRUE, FALSE);
			break;
		}

		case WM_TIMER:
		{
			if (wParam != TIMER_ID)
				break;

			if (!dlg->enabled || dlg->lang == NULL || !dlg->lang->isLoaded())
				break;

			// Don't check if field is read-only
			if (GetWindowLong(hwnd, GWL_STYLE) & ES_READONLY)
				break;

			int len = GetWindowTextLength(hwnd);
			if (len == dlg->old_text_len && !dlg->changed)
				break;

			dlg->old_text_len = len;
			dlg->changed = FALSE;

			CheckText(dlg, TRUE, opts.auto_correct);
			break;
		}
	}

	return ret;
}

int GetClosestLanguage(TCHAR *lang_name) 
{
	// Search the language by name
	for(int i = 0; i < languages.count; i++)
	{
		if (lstrcmpi(languages.dicts[i]->language, lang_name) == 0)
		{
			return i;
		}
	}

	// Try searching by the prefix only
	TCHAR *p = _tcschr(lang_name, _T('_'));
	if (p != NULL)
	{
		*p = _T('\0');
		for(int i = 0; i < languages.count; i++)
		{
			if (lstrcmpi(languages.dicts[i]->language, lang_name) == 0)
			{
				*p = '_';
				return i;
			}
		}
		*p = _T('_');
	}

	// Try any suffix, if one not provided
	if (p == NULL)
	{
		size_t len = lstrlen(lang_name);
		for(int i = 0; i < languages.count; i++)
		{
			if (_tcsnicmp(languages.dicts[i]->language, lang_name, len) == 0 
				&& languages.dicts[i]->language[len] == _T('_'))
			{
				return i;
			}
		}
	}

	return -1;
}

void GetUserProtoLanguageSetting(Dialog *dlg, HANDLE hContact, char *proto, char *setting)
{
	DBVARIANT dbv = {0};

	if (!DBGetContactSettingTString(hContact, proto, setting, &dbv))
	{
		for(int i = 0; i < languages.count; i++)
		{
			if (lstrcmpi(languages.dicts[i]->localized_name, dbv.ptszVal) == 0)
			{
				lstrcpyn(dlg->lang_name, languages.dicts[i]->language, MAX_REGS(dlg->lang_name));
				break;
			}
			if (lstrcmpi(languages.dicts[i]->english_name, dbv.ptszVal) == 0)
			{
				lstrcpyn(dlg->lang_name, languages.dicts[i]->language, MAX_REGS(dlg->lang_name));
				break;
			}
		}
		DBFreeVariant(&dbv);
	}
}

void GetUserLanguageSetting(Dialog *dlg, char *setting)
{
	DBVARIANT dbv = {0};

	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) dlg->hContact, 0);
	if (proto == NULL)
		return;

	GetUserProtoLanguageSetting(dlg, dlg->hContact, proto, setting);

	// If not found and is inside meta, try to get from the meta
	if (dlg->lang_name[0] != _T('\0'))
		return;
	
	// Is a subcontact?
	if (!ServiceExists(MS_MC_GETMETACONTACT)) 
		return;

	HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM) dlg->hContact, 0);
	if (hMetaContact == NULL)
		return;

	GetUserProtoLanguageSetting(dlg, hMetaContact, metacontacts_proto, setting);
}

void GetContactLanguage(Dialog *dlg)
{
	DBVARIANT dbv = {0};

	dlg->lang_name[0] = _T('\0');

	if (dlg->hContact == NULL) 
	{
		if (!DBGetContactSettingTString(NULL, MODULE_NAME, dlg->name, &dbv))
		{
			lstrcpyn(dlg->lang_name, dbv.ptszVal, MAX_REGS(dlg->lang_name));
			DBFreeVariant(&dbv);
		}
	}
	else
	{
		if (!DBGetContactSettingTString(dlg->hContact, MODULE_NAME, "TalkLanguage", &dbv))
		{
			lstrcpyn(dlg->lang_name, dbv.ptszVal, MAX_REGS(dlg->lang_name));
			DBFreeVariant(&dbv);
		}

		// Try from metacontact
		if (dlg->lang_name[0] == _T('\0') && ServiceExists(MS_MC_GETMETACONTACT)) 
		{
			HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM) dlg->hContact, 0);
			if (hMetaContact != NULL)
			{
				if (!DBGetContactSettingTString(hMetaContact, MODULE_NAME, "TalkLanguage", &dbv))
				{
					lstrcpyn(dlg->lang_name, dbv.ptszVal, MAX_REGS(dlg->lang_name));
					DBFreeVariant(&dbv);
				}
			}
		}

		// Try to get from Language info
		if (dlg->lang_name[0] == _T('\0'))
			GetUserLanguageSetting(dlg, "Language");
		if (dlg->lang_name[0] == _T('\0'))
			GetUserLanguageSetting(dlg, "Language1");
		if (dlg->lang_name[0] == _T('\0'))
			GetUserLanguageSetting(dlg, "Language2");
		if (dlg->lang_name[0] == _T('\0'))
			GetUserLanguageSetting(dlg, "Language3");

		// Use default lang
		if (dlg->lang_name[0] == _T('\0'))
			lstrcpyn(dlg->lang_name, opts.default_language, MAX_REGS(dlg->lang_name));
	}

	int i = GetClosestLanguage(dlg->lang_name);
	if (i >= 0)
	{
		dlg->lang = languages.dicts[i];
		dlg->lang->load();
	}
	else 
	{
		dlg->lang = NULL;
	}
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
		// Fill dialog data
		Dialog *dlg = (Dialog *) malloc(sizeof(Dialog));
		ZeroMemory(dlg, sizeof(Dialog));

		dialogs[hwnd] = dlg;

		dlg->hContact = hContact;
		dlg->hwnd = hwnd;
		strncpy(dlg->name, name, sizeof(dlg->name));
		dlg->enabled = DBGetContactSettingByte(NULL, MODULE_NAME, dlg->name, 1);

		GetContactLanguage(dlg);

		dlg->old_edit_proc = (WNDPROC) SetWindowLong(hwnd, GWL_WNDPROC, (LONG) EditProc);

		SetTimer(hwnd, TIMER_ID, 500, NULL);
	}

	return 0;
}

#define DESTROY_MENY(_m_)	if (_m_ != NULL) { DestroyMenu(_m_); _m_ = NULL; }
#define FREE(_m_)			if (_m_ != NULL) { free(_m_); _m_ = NULL; }

void FreePopupData(Dialog *dlg)
{
	DESTROY_MENY(dlg->hLanguageSubMenu)
	DESTROY_MENY(dlg->hWrongWordsSubMenu)

	if (dlg->wrong_words != NULL)
	{
		for (int i = 0; i < dlg->wrong_words->size(); i++)
		{
			FREE((*dlg->wrong_words)[i].word)

			DESTROY_MENY((*dlg->wrong_words)[i].hMeSubMenu)
			DESTROY_MENY((*dlg->wrong_words)[i].hCorrectSubMenu)
			DESTROY_MENY((*dlg->wrong_words)[i].hReplaceSubMenu)

			FreeSuggestions((*dlg->wrong_words)[i].suggestions);
		}

		delete dlg->wrong_words;
		dlg->wrong_words = NULL;
	}
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


void ReplaceWord(Dialog *dlg, CHARRANGE &sel, TCHAR *new_word)
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


BOOL GetWordCharRange(Dialog *dlg, CHARRANGE &sel, TCHAR *text, size_t text_len, int &first_char)
{
	// Get line
	int line = SendMessage(dlg->hwnd, EM_LINEFROMCHAR, (WPARAM) sel.cpMin, 0);

	// Get text
	GetLineOfText(dlg, line, first_char, text, text_len);

	// Find the word
	sel.cpMin--;
	while (sel.cpMin >= first_char && dlg->lang->isWordChar(text[sel.cpMin - first_char]))
		sel.cpMin--;
	sel.cpMin++;

	while (text[sel.cpMax - first_char] != _T('\0') && dlg->lang->isWordChar(text[sel.cpMax - first_char]))
		sel.cpMax++;

	// Has a word?
	if (sel.cpMin >= sel.cpMax)
		return FALSE;

	// See if it has only '-'s
	BOOL has_valid_char = FALSE;
	for (int i = sel.cpMin; i < sel.cpMax && !has_valid_char; i++)
		has_valid_char = ( text[i - first_char] != _T('-') );

	if (!has_valid_char)
		return FALSE;

	return TRUE;
}

TCHAR *GetWordUnderPoint(Dialog *dlg, POINT pt, CHARRANGE &sel)
{
	// Get text
	if (GetWindowTextLength(dlg->hwnd) <= 0)
		return NULL;

	// Get pos
	sel.cpMin = sel.cpMax = LOWORD(SendMessage(dlg->hwnd, EM_CHARFROMPOS, 0, (LPARAM) &pt));

	// Get text
	TCHAR text[1024];
	int first_char;

	if (!GetWordCharRange(dlg, sel, text, MAX_REGS(text), first_char))
		return NULL;

	// copy the word
	text[sel.cpMax - first_char] = _T('\0');
	return _tcsdup(&text[sel.cpMin - first_char]);
}


void InsertSubmenu(HMENU hMenu, HMENU hSubMenu, TCHAR *name) 
{
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = MFT_STRING;
	mii.hSubMenu = hSubMenu;
	mii.dwTypeData = name;
	mii.cch = lstrlen(name);
	int ret = InsertMenuItem(hMenu, 0, TRUE, &mii);

}


#define LANGUAGE_MENU_ID_BASE 10
#define WORD_MENU_ID_BASE 100
#define AUTOREPLACE_MENU_ID_BASE 50

void AddMenuForWord(Dialog *dlg, TCHAR *word, CHARRANGE &pos, HMENU hMenu, BOOL in_submenu, UINT base)
{
	if (dlg->wrong_words == NULL)
		dlg->wrong_words = new vector<WrongWordPopupMenuData>(1);
	else
		dlg->wrong_words->resize(dlg->wrong_words->size() + 1);

	WrongWordPopupMenuData &data = (*dlg->wrong_words)[dlg->wrong_words->size() - 1];
	ZeroMemory(&data, sizeof(WrongWordPopupMenuData));

	// Get suggestions
	data.word = word;
	data.pos = pos;
	data.suggestions = dlg->lang->suggest(word);

	Suggestions &suggestions = data.suggestions;

	if (in_submenu)
	{
		data.hMeSubMenu = CreatePopupMenu();
		InsertSubmenu(hMenu, data.hMeSubMenu, word);
		hMenu = data.hMeSubMenu;
	}

	if (suggestions.count > 0)
	{
		data.hReplaceSubMenu = CreatePopupMenu();

		for (int i = suggestions.count - 1; i >= 0; i--) 
			InsertMenu(data.hReplaceSubMenu, 0, MF_BYPOSITION, 
					base + AUTOREPLACE_MENU_ID_BASE + i, suggestions.words[i]);

		InsertSubmenu(hMenu, data.hReplaceSubMenu, TranslateT("Always replace with"));
	}

	InsertMenu(hMenu, 0, MF_BYPOSITION, base + suggestions.count + 1, TranslateT("Ignore all"));
	InsertMenu(hMenu, 0, MF_BYPOSITION, base + suggestions.count, TranslateT("Add to dictionary"));

	if (suggestions.count > 0)
	{
		HMENU hSubMenu;
		if (opts.cascade_corrections)
		{
			hSubMenu = data.hCorrectSubMenu = CreatePopupMenu();
			InsertSubmenu(hMenu, hSubMenu, TranslateT("Corrections"));
		}
		else
		{
			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
			hSubMenu = hMenu;
		}

		for (int i = suggestions.count - 1; i >= 0; i--) 
			InsertMenu(hSubMenu, 0, MF_BYPOSITION, base + i, suggestions.words[i]);
	}

	if (!in_submenu)
	{
		InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

		TCHAR text[128];
		mir_sntprintf(text, MAX_REGS(text), TranslateT("Wrong word: %s"), word);
		InsertMenu(hMenu, 0, MF_BYPOSITION, 0, text);
	}
}


struct FoundWrongWordParam {
	Dialog *dlg;
	int count;
};

void FoundWrongWord(TCHAR *word, CHARRANGE pos, void *param)
{
	FoundWrongWordParam *p = (FoundWrongWordParam*) param;

	p->count ++;

	AddMenuForWord(p->dlg, _tcsdup(word), pos, p->dlg->hWrongWordsSubMenu, TRUE, WORD_MENU_ID_BASE * p->count);
}

void AddItemsToMenu(Dialog *dlg, HMENU hMenu, POINT pt)
{
	FreePopupData(dlg);

	BOOL wrong_word = FALSE;

	// Make menu
	if (GetMenuItemCount(hMenu) > 0)
		InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

	if (languages.count > 0 && dlg->enabled)
	{
		dlg->hLanguageSubMenu = CreatePopupMenu();

		// First add languages
		for(int i = 0; i < languages.count; i++)
		{
			if (languages.dicts[i]->localized_name[0] != _T('\0'))
			{
				TCHAR name[128];
				mir_sntprintf(name, MAX_REGS(name), _T("%s [%s]"), languages.dicts[i]->localized_name, languages.dicts[i]->language);
				AppendMenu(dlg->hLanguageSubMenu, MF_STRING | (languages.dicts[i] == dlg->lang ? MF_CHECKED : 0), 
					LANGUAGE_MENU_ID_BASE + i, name);
			}
			else
			{
				AppendMenu(dlg->hLanguageSubMenu, MF_STRING | (languages.dicts[i] == dlg->lang ? MF_CHECKED : 0), 
					LANGUAGE_MENU_ID_BASE + i, languages.dicts[i]->language);
			}

		}

		InsertSubmenu(hMenu, dlg->hLanguageSubMenu, TranslateT("Language"));
	}

	InsertMenu(hMenu, 0, MF_BYPOSITION, 1, TranslateT("Enable spell checking"));
	CheckMenuItem(hMenu, 1, MF_BYCOMMAND | (dlg->enabled ? MF_CHECKED : MF_UNCHECKED));

	// Get text
	if (dlg->lang != NULL && dlg->enabled)
	{
		if (opts.show_all_corrections)
		{
			dlg->hWrongWordsSubMenu = CreatePopupMenu(); 

			FoundWrongWordParam p = { dlg, 0 };
			CheckText(dlg, TRUE, opts.auto_correct, FoundWrongWord, &p);

			if (p.count > 0)
				InsertSubmenu(hMenu, dlg->hWrongWordsSubMenu, TranslateT("Wrong words"));
		}
		else
		{
			CHARRANGE sel;
			TCHAR *word = GetWordUnderPoint(dlg, pt, sel);
			if (word != NULL && !dlg->lang->spell(word))
			{
				InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

				AddMenuForWord(dlg, word, sel, hMenu, FALSE, WORD_MENU_ID_BASE);
			}
		}
	}
}


BOOL HandleMenuSelection(Dialog *dlg, POINT pt, int selection)
{
	BOOL ret = FALSE;

	if (selection == 1)
	{
		dlg->enabled = !dlg->enabled;
		DBWriteContactSettingByte(NULL, MODULE_NAME, dlg->name, dlg->enabled);

		if (!dlg->enabled)
			SetNoUnderline(dlg->hwnd);

		ret = TRUE;
	}
	else if (selection >= LANGUAGE_MENU_ID_BASE && selection < LANGUAGE_MENU_ID_BASE + languages.count)
	{
		if (dlg->hContact == NULL)
			DBWriteContactSettingTString(NULL, MODULE_NAME, dlg->name, 
					languages.dicts[selection - LANGUAGE_MENU_ID_BASE]->language);
		else
			DBWriteContactSettingTString(dlg->hContact, MODULE_NAME, "TalkLanguage", 
					languages.dicts[selection - LANGUAGE_MENU_ID_BASE]->language);
		GetContactLanguage(dlg);

		ret = TRUE;
	}
	else if (selection > 0 && dlg->wrong_words != NULL 
			 && selection >= WORD_MENU_ID_BASE
			 && selection < (dlg->wrong_words->size() + 1) * WORD_MENU_ID_BASE)
	{
		int pos = selection / WORD_MENU_ID_BASE;
		selection -=  pos * WORD_MENU_ID_BASE;
		pos--; // 0 based
		WrongWordPopupMenuData &data = (*dlg->wrong_words)[pos];

		if (selection < data.suggestions.count)
		{
			// TODO Assert that text hasn't changed
			ReplaceWord(dlg, data.pos, data.suggestions.words[selection]);

			ret = TRUE;
		}
		else if (selection == data.suggestions.count)
		{
			dlg->lang->addWord(data.word);

			ret = TRUE;
		}
		else if (selection == data.suggestions.count + 1)
		{
			dlg->lang->ignoreWord(data.word);

			ret = TRUE;
		}
		else if (selection >= AUTOREPLACE_MENU_ID_BASE 
				 && selection < AUTOREPLACE_MENU_ID_BASE + data.suggestions.count)
		{
			selection -= AUTOREPLACE_MENU_ID_BASE;
			
			// TODO Assert that text hasn't changed
			ReplaceWord(dlg, data.pos, data.suggestions.words[selection]);
			dlg->lang->addToAutoReplace(data.word, data.suggestions.words[selection]);

			ret = TRUE;
		}
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
	return 0;
}


int ShowPopupMenuService(WPARAM wParam, LPARAM lParam)
{
	SPELLCHECKER_POPUPMENU *scp = (SPELLCHECKER_POPUPMENU *) wParam;
	if (scp == NULL || scp->cbSize != sizeof(SPELLCHECKER_POPUPMENU))
		return -1;

	return ShowPopupMenu(scp->hwnd, scp->hMenu, scp->pt, scp->hwndOwner == NULL ? scp->hwnd : scp->hwndOwner);
}


int ShowPopupMenu(HWND hwnd, HMENU hMenu, POINT pt, HWND hwndOwner)
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
	POINT client = pt;
	ClientToScreen(hwnd, &pt);
	int selection = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndOwner, NULL);

	// Do action
	if (HandleMenuSelection(dlg, client, selection))
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
