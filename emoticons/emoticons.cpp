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


PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
#ifdef UNICODE
	"Emoticons (Unicode)",
#else
	"Emoticons",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,7),
	"Emoticons",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2008 Ricardo Pescuma Domenecci",
	"http://pescuma.org/miranda/emoticons",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
#ifdef UNICODE
	{ 0x80ad2967, 0x2f29, 0x4550, { 0x87, 0x19, 0x23, 0x26, 0x41, 0xd4, 0xc8, 0x83 } } // {80AD2967-2F29-4550-8719-232641D4C883}
#else
	{ 0x8b47942a, 0xa294, 0x4b25, { 0x95, 0x1a, 0x20, 0x80, 0x44, 0xc9, 0x4f, 0x4d } } // {8B47942A-A294-4b25-951A-208044C94F4D}
#endif
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hHooks[3] = {0};
HANDLE hServices[3] = {0};

HANDLE hProtocolsFolder = NULL;
TCHAR protocolsFolder[1024];

HANDLE hEmoticonPacksFolder = NULL;
TCHAR emoticonPacksFolder[1024];

char *metacontacts_proto = NULL;
BOOL loaded = FALSE;

typedef map<HWND, Dialog *> DialogMapType;

DialogMapType dialogData;

LIST_INTERFACE li;

LIST<Module> modules(10);
LIST<EmoticonPack> packs(10);

BOOL LoadModule(Module *m);
void LoadModules();
BOOL LoadPack(EmoticonPack *p);
void LoadPacks();

void FillModuleImages(EmoticonPack *pack);

EmoticonPack *GetPack(TCHAR *name);
Module *GetModule(const char *name);


int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);
int MsgWindowEvent(WPARAM wParam, LPARAM lParam);

int ReplaceEmoticonsService(WPARAM wParam, LPARAM lParam);
int GetInfo2Service(WPARAM wParam, LPARAM lParam);
int ShowSelectionService(WPARAM wParam, LPARAM lParam);

TCHAR *GetText(RichEditCtrl &rec, int start, int end);


LRESULT CALLBACK MenuWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


#define DEFINE_GUIDXXX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        const GUID CDECL name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUIDXXX(IID_ITextDocument,0x8CC497C0,0xA1DF,0x11CE,0x80,0x98,
                0x00,0xAA,0x00,0x47,0xBE,0x5D);

#define SUSPEND_UNDO(rec)														\
	if (rec.textDocument != NULL)												\
		rec.textDocument->Undo(tomSuspend, NULL)

#define RESUME_UNDO(rec)														\
	if (rec.textDocument != NULL)												\
		rec.textDocument->Undo(tomResume, NULL)

#define	STOP_RICHEDIT(rec)														\
	SUSPEND_UNDO(rec);															\
	SendMessage(rec.hwnd, WM_SETREDRAW, FALSE, 0);								\
	POINT __old_scroll_pos;														\
	SendMessage(rec.hwnd, EM_GETSCROLLPOS, 0, (LPARAM) &__old_scroll_pos);		\
	CHARRANGE __old_sel;														\
	SendMessage(rec.hwnd, EM_EXGETSEL, 0, (LPARAM) &__old_sel);					\
	POINT __caretPos;															\
	GetCaretPos(&__caretPos);													\
    DWORD __old_mask = SendMessage(rec.hwnd, EM_GETEVENTMASK, 0, 0);			\
	SendMessage(rec.hwnd, EM_SETEVENTMASK, 0, __old_mask & ~ENM_CHANGE);		\
	BOOL __inverse = (__old_sel.cpMin >= LOWORD(SendMessage(rec.hwnd, EM_CHARFROMPOS, 0, (LPARAM) &__caretPos)))

#define START_RICHEDIT(rec)														\
	if (__inverse)																\
	{																			\
		LONG __tmp = __old_sel.cpMin;											\
		__old_sel.cpMin = __old_sel.cpMax;										\
		__old_sel.cpMax = __tmp;												\
	}																			\
	SendMessage(rec.hwnd, EM_SETEVENTMASK, 0, __old_mask);						\
	SendMessage(rec.hwnd, EM_EXSETSEL, 0, (LPARAM) &__old_sel);					\
	SendMessage(rec.hwnd, EM_SETSCROLLPOS, 0, (LPARAM) &__old_scroll_pos);		\
	SendMessage(rec.hwnd, WM_SETREDRAW, TRUE, 0);								\
	InvalidateRect(rec.hwnd, NULL, FALSE);										\
	RESUME_UNDO(rec)


// Functions ////////////////////////////////////////////////////////////////////////////


extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	hInst = hinstDLL;
	return TRUE;
}


extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) 
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*) &pluginInfo;
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFOEX);
	return &pluginInfo;
}


static const MUUID interfaces[] = { MIID_SMILEY, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link) 
{
	pluginLink = link;

	init_mir_malloc();
	mir_getLI(&li);

	// hooks
	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hHooks[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	return 0;
}


extern "C" int __declspec(dllexport) Unload(void) 
{
	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME, 0);

	TCHAR mirandaFolder[1024];
	GetModuleFileName(GetModuleHandle(NULL), mirandaFolder, MAX_REGS(mirandaFolder));
	TCHAR *p = _tcsrchr(mirandaFolder, _T('\\'));
	if (p != NULL)
		*p = _T('\0');

    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://pescuma.org/miranda/emoticons_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.org/miranda/emoticons#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"Emoticons ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/emoticonsW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/emoticons.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

    // Folders plugin support
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		hProtocolsFolder = (HANDLE) FoldersRegisterCustomPathT(Translate("Emoticons"), 
					Translate("Protocols Configuration"), 
					_T(MIRANDA_PATH) _T("\\Plugins\\Emoticons"));

		FoldersGetCustomPathT(hProtocolsFolder, protocolsFolder, MAX_REGS(protocolsFolder), _T("."));

		hEmoticonPacksFolder = (HANDLE) FoldersRegisterCustomPathT(Translate("Emoticons"), 
					Translate("Emoticon Packs"), 
					_T(MIRANDA_PATH) _T("\\Customize\\Emoticons"));

		FoldersGetCustomPathT(hEmoticonPacksFolder, emoticonPacksFolder, MAX_REGS(emoticonPacksFolder), _T("."));
	}
	else
	{
		mir_sntprintf(protocolsFolder, MAX_REGS(protocolsFolder), _T("%s\\Plugins\\Emoticons"), mirandaFolder);
		mir_sntprintf(emoticonPacksFolder, MAX_REGS(emoticonPacksFolder), _T("%s\\Customize\\Emoticons"), mirandaFolder);
	}

	InitOptions();
	
	LoadModules();
	LoadPacks();

	if (packs.getCount() > 0)
	{
		// Get default pack
		EmoticonPack *pack = GetPack(opts.pack);
		if (pack == NULL)
			pack = packs[0];
		FillModuleImages(pack);
	}


	hHooks[2] = HookEvent(ME_MSG_WINDOWEVENT, &MsgWindowEvent);

	hServices[0] = CreateServiceFunction(MS_SMILEYADD_REPLACESMILEYS, ReplaceEmoticonsService);
	hServices[1] = CreateServiceFunction(MS_SMILEYADD_GETINFO2, GetInfo2Service);
	hServices[2] = CreateServiceFunction(MS_SMILEYADD_SHOWSELECTION, ShowSelectionService);

	loaded = TRUE;

	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	int i;
	for(i = 0; i < MAX_REGS(hServices); i++)
		DestroyServiceFunction(hServices[i]);

	for(i = 0; i < MAX_REGS(hHooks); i++)
		UnhookEvent(hHooks[i]);

	DeInitOptions();

	return 0;
}


// Return the size difference with the original text
int ReplaceEmoticonBackwards(HWND hwnd, TCHAR *text, int text_len, int last_pos, TCHAR next_char, Module *module)
{
	if (opts.only_replace_isolated && next_char != _T('\0') && /*!_istpunct(next_char) &&*/ !_istspace(next_char))
		return 0;

	// This are needed to allow 2 different emoticons that end the same way
	Emoticon *found = NULL;
	int foundLen = -1;
	TCHAR *foundText;

	for(int i = 0; i < module->emoticons.getCount(); i++)
	{
		Emoticon *e = module->emoticons[i];
		if (e->img == NULL)
			continue;

		for(int j = 0; j < e->texts.getCount(); j++)
		{
			TCHAR *txt = e->texts[j];
			int len = lstrlen(txt);
			if (last_pos < len || text_len < len)
				continue;

			if (len <= foundLen)
				continue;

			if (_tcsncmp(&text[text_len - len], txt, len) != 0)
				continue;

			if (opts.only_replace_isolated && text_len > len 
					/*&& !_istpunct(text[text_len - len - 1])*/
					&& !_istspace(text[text_len - len - 1]))
				continue;


			found = e;
			foundLen = len;
			foundText = txt;
		}
	}

	if (found != NULL)
	{
		// Found ya
		CHARRANGE sel = { last_pos - foundLen, last_pos };
		SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) &sel);

		CHARFORMAT2 cf;
		memset(&cf, 0, sizeof(CHARFORMAT2));
		cf.cbSize = sizeof(CHARFORMAT2);
		cf.dwMask = CFM_BACKCOLOR;
		SendMessage(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);

		if (cf.dwEffects & CFE_AUTOBACKCOLOR)
		{
			cf.crBackColor = SendMessage(hwnd, EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_WINDOW));
			SendMessage(hwnd, EM_SETBKGNDCOLOR, 0, cf.crBackColor);
		}

		if (InsertAnimatedSmiley(hwnd, found->img->path, cf.crBackColor, 0 , foundText))
		{
			return - foundLen + 1;
		}
	}
	return 0;
}

void FixSelection(LONG &sel, LONG end, int dif)
{
	if (sel >= end)
		sel += dif;
	else if (sel >= min(end, end + dif))
		sel = min(end, end + dif);
}


TCHAR *GetText(RichEditCtrl &rec, int start, int end)
{
	if (end <= start)
		end = GetWindowTextLength(rec.hwnd);

	ITextRange *range;
	if (rec.textDocument->Range(start, end, &range) != S_OK) 
		return FALSE;

	BSTR text = NULL;
	if (range->GetText(&text) != S_OK || text == NULL)
	{
		range->Release();
		return _T("");
	}

	TCHAR *ret = mir_u2t(text);

	SysFreeString(text);

	range->Release();

	return ret;
/*
	CHARRANGE sel = { start, end };
	if (sel.cpMax <= sel.cpMin)
		sel.cpMax = GetWindowTextLength(hwnd);
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) &sel);
	SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);

	int len = sel.cpMax - sel.cpMin;
	TCHAR *text = (TCHAR *) malloc((len + 1) * sizeof(TCHAR));

	GETTEXTEX ste = {0};
	ste.cb = (len + 1) * sizeof(TCHAR);
	ste.flags = ST_SELECTION;
#ifdef UNICODE
	ste.codepage = 1200; // UNICODE
	SendMessage(hwnd, EM_GETTEXTEX, (WPARAM) &ste, (LPARAM) text);
#else
	ste.codepage = CP_ACP;
	SendMessage(hwnd, EM_GETTEXTEX, (WPARAM) &ste, (LPARAM) text);
#endif

	return text;
*/
}


BOOL IsHidden(RichEditCtrl &rec, int start, int end)
{
	ITextRange *range;
	if (rec.textDocument->Range(start, end, &range) != S_OK) 
		return FALSE;

	ITextFont *font;
	if (range->GetFont(&font) != S_OK) 
	{
		range->Release();
		return FALSE;
	}

	long hidden;
	font->GetHidden(&hidden);
	BOOL ret = (hidden == tomTrue);

	font->Release();
	range->Release();

	return ret;

/*	CHARRANGE sel = { start, end };
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) &sel);
	SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);

	CHARFORMAT2 cf;
	memset(&cf, 0, sizeof(CHARFORMAT2));
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_HIDDEN;
	SendMessageA(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	return (cf.dwEffects & CFE_HIDDEN) != 0;
*/
}


void ReplaceAllEmoticonsBackwards(RichEditCtrl &rec, Module *module, TCHAR *text, int len, TCHAR next_char, int start, CHARRANGE &__old_sel)
{
	for(int i = len; i > 0; i--)
	{
		int dif = ReplaceEmoticonBackwards(rec.hwnd, text, i, start + i, i == len ? next_char : text[i], module);
		if (dif != 0)
		{
			FixSelection(__old_sel.cpMax, i, dif);
			FixSelection(__old_sel.cpMin, i, dif);

			i += dif;
		}
	}
}


void ReplaceAllEmoticonsBackwards(RichEditCtrl &rec, Module *module)
{
	STOP_RICHEDIT(rec);

	TCHAR *text = GetText(rec, 0, -1);
	int len = lstrlen(text);

	ReplaceAllEmoticonsBackwards(rec, module, text, len, _T('\0'), 0, __old_sel);

	mir_free(text);

	START_RICHEDIT(rec);
}


int matches(const TCHAR *tag, const TCHAR *text)
{
	int len = lstrlen(tag);
	if (_tcsncmp(tag, text, len) == 0)
		return len;
	else
		return 0;
}


static TCHAR *webs[] = { 
	_T("http://"), 
	_T("ftp://"), 
	_T("irc://"), 
	_T("gopher://"), 
	_T("file://"), 
	_T("www."), 
	_T("www2."),
	_T("ftp."),
	_T("irc."),
	_T("A:\\"),
	_T("B:\\"),
	_T("C:\\"),
	_T("D:\\"),
};


void ReplaceAllEmoticons(RichEditCtrl &rec, Module *module, int start, int end)
{
	STOP_RICHEDIT(rec);

	if (start < 0)
		start = 0;

	TCHAR *text = GetText(rec, start, end);
	int len = lstrlen(text);

	int last_start_pos = 0;
	BOOL replace = TRUE;
	HANDLE hContact = NULL;
	for(int i = 0; i <= len; i++)
	{
		int tl;
		if (replace)
		{
			if (i == 0 || !_istalnum(text[i - 1]))
			{
				for (int j = 0; j < MAX_REGS(webs); j++)
				{
					if (tl = matches(webs[j], &text[i]))
					{
						ReplaceAllEmoticonsBackwards(rec, module, &text[last_start_pos], i - last_start_pos, _T('\0'), start + last_start_pos, __old_sel);

						i += tl;
						
						for(;  (text[i] >= _T('a') && text[i] <= _T('z'))
							|| (text[i] >= _T('A') && text[i] <= _T('Z'))
							|| (text[i] >= _T('0') && text[i] <= _T('9'))
							|| text[i] == _T('.') || text[i] == _T('/')
							|| text[i] == _T('?') || text[i] == _T('_')
							|| text[i] == _T('=') || text[i] == _T('&')
							|| text[i] == _T('%') || text[i] == _T('-')
							; i++) ;

						last_start_pos = i;
					}
				}
			}
		}

		if (tl = matches(_T("<no-emoticon>"), &text[i]))
		{
			if (IsHidden(rec, start + i, start + i + tl))
			{
				ReplaceAllEmoticonsBackwards(rec, module, &text[last_start_pos], i - last_start_pos, _T('\0'), start + last_start_pos, __old_sel);

				replace = FALSE;
				i += tl - 1;
			}
			continue;
		}

		if (tl = matches(_T("</no-emoticon>"), &text[i]))
		{
			if (IsHidden(rec, start + i, start + i + tl))
			{
				replace = TRUE; 
				i += tl - 1;
				last_start_pos = i + 1;
			}
			continue;
		}

		if (tl = matches(_T("</emoticon-contact>"), &text[i]))
		{
			if (IsHidden(rec, start + i, start + i + tl))
			{
				ReplaceAllEmoticonsBackwards(rec, module, &text[last_start_pos], i - last_start_pos, _T('\0'), start + last_start_pos, __old_sel);

				hContact = NULL;
				i += tl - 1;
				last_start_pos = i + 1;
			}
			continue;
		}
		
		if (tl = matches(_T("<emoticon-contact "), &text[i]))
		{
			int len = tl;
			for(int j = 0; j < 10 && text[i + len] != '>'; j++, len++) 
				;

			if (text[i + len] != '>')
				continue;

			len++;

			if (IsHidden(rec, start + i, start + i + len))
			{
				ReplaceAllEmoticonsBackwards(rec, module, &text[last_start_pos], i - last_start_pos, _T('\0'), start + last_start_pos, __old_sel);

				hContact = (HANDLE) _ttoi(&text[i + tl]);
				i += len - 1;
				last_start_pos = i + 1;
			}
		}
	}

	if (replace)
		ReplaceAllEmoticonsBackwards(rec, module, &text[last_start_pos], len - last_start_pos, _T('\0'), start + last_start_pos, __old_sel);
	
	mir_free(text);

	START_RICHEDIT(rec);
}


int RestoreInput(RichEditCtrl &rec, int start = 0, int end = -1)
{
	int ret = 0;

	int objectCount = rec.ole->GetObjectCount();
	for (int i = objectCount - 1; i >= 0; i--)
	{
		REOBJECT reObj = {0};
		reObj.cbStruct  = sizeof(REOBJECT);

		HRESULT hr = rec.ole->GetObject(i, &reObj, REO_GETOBJ_POLEOBJ);
		if (!SUCCEEDED(hr))
			continue;

		if (reObj.cp < start || (end >= start && reObj.cp >= end))
		{
			reObj.poleobj->Release();
			continue;
		}
		
		IGifSmileyCtrl *igsc = NULL;
		reObj.poleobj->QueryInterface(IID_IGifSmileyCtrl, (void**) &igsc);
		reObj.poleobj->Release();
		if (igsc == NULL)
			continue;

		BSTR hint = NULL;
		hr = igsc->raw_GetHint(&hint);
		if (SUCCEEDED(hr) && hint != NULL)
		{
			ITextRange *range;
			if (rec.textDocument->Range(reObj.cp, reObj.cp + 1, &range) == S_OK) 
			{
				if (range->SetText(hint) == S_OK)
					ret += wcslen(hint) - 1;

				range->Release();
			}

			SysFreeString(hint);
		}

		igsc->Release();
	}

	return ret;
}


LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DialogMapType::iterator dlgit = dialogData.find(hwnd);
	if (dlgit == dialogData.end())
		return -1;

	Dialog *dlg = dlgit->second;

	LRESULT ret = CallWindowProc(dlg->input.old_edit_proc, hwnd, msg, wParam, lParam);

	switch(msg)
	{
		case WM_KEYDOWN:
		{
			if (wParam != VK_DELETE && wParam != VK_BACK)
				break;
		}
		case WM_CHAR:
		{
			if (msg == WM_CHAR && wParam >= 0 && wParam <= 32 && (!_istspace(wParam) || !opts.only_replace_isolated))
				break;

			if (lParam & (1 << 28))	// ALT key
				break;

			if (wParam != _T('\n') && GetKeyState(VK_CONTROL) & 0x8000)	// CTRL key
				break;

			if ((lParam & 0xFF) > 2)	// Repeat rate
				break;

			STOP_RICHEDIT(dlg->input);

			CHARRANGE sel = {0};
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);

			int min = max(0, sel.cpMax - 10);

			int dif = RestoreInput(dlg->input, min, sel.cpMax);
			if (dif != 0)
			{
				FixSelection(__old_sel.cpMax, sel.cpMax, dif);
				FixSelection(__old_sel.cpMin, sel.cpMax, dif);
				sel.cpMax += dif;
			}

			TCHAR *text = GetText(dlg->input, min, sel.cpMax + 1);
			int len = lstrlen(text);
			TCHAR last;
			if (len == sel.cpMax + 1 - min) 
			{
				// Strip
				len--;
				last = text[len];
			}
			else
			{
				last = _T('\0');
			}

			if (dif == 0 && !opts.only_replace_isolated)
			{
				// Can replace just last text
				dif = ReplaceEmoticonBackwards(hwnd, text, len, sel.cpMax, last, dlg->module);
				if (dif != 0)
				{
					FixSelection(__old_sel.cpMax, sel.cpMax, dif);
					FixSelection(__old_sel.cpMin, sel.cpMax, dif);
				}
			}
			else
			{
				// Because we already changed the text, we need to replace all range
				ReplaceAllEmoticonsBackwards(dlg->input, dlg->module, text, len, last, min, __old_sel);
			}

			mir_free(text);

			START_RICHEDIT(dlg->input);

			break;
		}
		case EM_REPLACESEL:
		case WM_SETTEXT:
		case EM_SETTEXTEX:
			if (dlg->log.sending)
				break;
		case EM_PASTESPECIAL:
		case WM_PASTE:
		{
			ReplaceAllEmoticonsBackwards(dlg->input, dlg->module);

			break;
		}
	}

	return ret;
}

/*
LRESULT CALLBACK LogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DialogMapType::iterator dlgit = dialogData.find(hwnd);
	if (dlgit == dialogData.end())
		return -1;

	Dialog *dlg = dlgit->second;

	switch(msg)
	{
		case WM_SETREDRAW:
		{
			if (wParam == FALSE)
			{
				dlg->log.received_stream_in = FALSE;
			}
			else
			{
				if (dlg->log.received_stream_in)
				{
					RichEditCtrl &rec = dlg->log;
					SUSPEND_UNDO(rec);															\
					POINT __old_scroll_pos;														\
					SendMessage(rec.hwnd, EM_GETSCROLLPOS, 0, (LPARAM) &__old_scroll_pos);		\
					CHARRANGE __old_sel;														\
					SendMessage(rec.hwnd, EM_EXGETSEL, 0, (LPARAM) &__old_sel);					\
					POINT __caretPos;															\
					GetCaretPos(&__caretPos);													\
					DWORD __old_mask = SendMessage(rec.hwnd, EM_GETEVENTMASK, 0, 0);			\
					SendMessage(rec.hwnd, EM_SETEVENTMASK, 0, __old_mask & ~ENM_CHANGE);		\
					BOOL __inverse = (__old_sel.cpMin >= LOWORD(SendMessage(rec.hwnd, EM_CHARFROMPOS, 0, (LPARAM) &__caretPos)));

					CHARRANGE sel = { dlg->log.stream_in_pos, GetWindowTextLength(dlg->log.hwnd) };
					SendMessage(dlg->log.hwnd, EM_EXSETSEL, 0, (LPARAM) &sel);

					int len = sel.cpMax - sel.cpMin;
					TCHAR *text = (TCHAR *) malloc((len + 1) * sizeof(TCHAR));
					SendMessage(dlg->log.hwnd, EM_GETSELTEXT, 0, (LPARAM) text);

					for(int i = len; i > 0; i--)
					{
						int dif = ReplaceEmoticonBackwards(dlg->log.hwnd, text, i, sel.cpMin + i, dlg->module);

						FixSelection(__old_sel.cpMax, i, dif);
						FixSelection(__old_sel.cpMin, i, dif);
					}

					if (__inverse)																\
					{																			\
						LONG __tmp = __old_sel.cpMin;											\
						__old_sel.cpMin = __old_sel.cpMax;										\
						__old_sel.cpMax = __tmp;												\
					}																			\
					SendMessage(rec.hwnd, EM_SETEVENTMASK, 0, __old_mask);						\
					SendMessage(rec.hwnd, EM_EXSETSEL, 0, (LPARAM) &__old_sel);				\
					SendMessage(rec.hwnd, EM_SETSCROLLPOS, 0, (LPARAM) &__old_scroll_pos);		\
					InvalidateRect(rec.hwnd, NULL, FALSE);										\
					RESUME_UNDO(rec);
				}
			}
			break;
		}
		case EM_STREAMIN:
		{
			dlg->log.received_stream_in = TRUE;
			dlg->log.stream_in_pos = GetWindowTextLength(dlg->log.hwnd);
			break;
		}
	}

	LRESULT ret = CallWindowProc(dlg->log.old_edit_proc, hwnd, msg, wParam, lParam);

	return ret;
}
*/


LRESULT CALLBACK OwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DialogMapType::iterator dlgit = dialogData.find(hwnd);
	if (dlgit == dialogData.end())
		return -1;

	Dialog *dlg = dlgit->second;

/*	int old_len;
	if (msg == DM_APPENDTOLOG)
	{
		old_len = GetWindowTextLength(dlg->log.hwnd);
	}
	else */ 
	if (msg == WM_COMMAND && LOWORD(wParam) == IDOK && dlg->input.old_edit_proc != NULL)
	{
		dlg->log.sending = TRUE;

		STOP_RICHEDIT(dlg->input);

		RestoreInput(dlg->input);

		START_RICHEDIT(dlg->input);
	}

	LRESULT ret = CallWindowProc(dlg->owner_old_edit_proc, hwnd, msg, wParam, lParam);

	switch(msg)
	{
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK)
			{
				if (!ret)
					// Add emoticons again
					ReplaceAllEmoticonsBackwards(dlg->input, dlg->module);

				dlg->log.sending = FALSE;
			}
			break;
		}
/*
		case DM_APPENDTOLOG:
		{
			break;

			STOP_RICHEDIT(dlg->log);

			TCHAR *text = GetText(dlg->log, old_len, -1);
			int len = lstrlen(text);

			for(int i = len; i > 0; i--)
			{
				int dif = ReplaceEmoticonBackwards(dlg->log.hwnd, text, i, old_len + i, dlg->module);

				FixSelection(__old_sel.cpMax, i, dif);
				FixSelection(__old_sel.cpMin, i, dif);
			}

			mir_free(text);

			START_RICHEDIT(dlg->log);
			
			break;
		}

		case DM_REMAKELOG:
		{
			break;

			STOP_RICHEDIT(dlg->log);

			TCHAR *text = GetText(dlg->log, 0, -1);
			int len = lstrlen(text);
			
			for(int i = len; i > 0; i--)
			{
				int dif = ReplaceEmoticonBackwards(dlg->log.hwnd, text, i, i, dlg->module);

				FixSelection(__old_sel.cpMax, i, dif);
				FixSelection(__old_sel.cpMin, i, dif);
			}

			mir_free(text);

			START_RICHEDIT(dlg->log);

			break;
		}
*/
	}

	return ret;
}


int LoadRichEdit(RichEditCtrl *rec, HWND hwnd) 
{
	rec->hwnd = hwnd;
	rec->ole = NULL;
	rec->textDocument = NULL;
	rec->old_edit_proc = NULL;
	rec->received_stream_in = FALSE;
	rec->sending = FALSE;

	SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&rec->ole);
	if (rec->ole == NULL)
		return 0;

	if (rec->ole->QueryInterface(IID_ITextDocument, (void**)&rec->textDocument) != S_OK)
		rec->textDocument = NULL;

	return 1;
}


void UnloadRichEdit(RichEditCtrl *rec) 
{
	if (rec->textDocument != NULL)
		rec->textDocument->Release();
	if (rec->ole != NULL)
		rec->ole->Release();
}


HANDLE GetRealContact(HANDLE hContact)
{
	if (!ServiceExists(MS_MC_GETMOSTONLINECONTACT))
		return hContact;

	HANDLE hReal = (HANDLE) CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM) hContact, 0);
	if (hReal == NULL)
		hReal = hContact;
	return hReal;
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
		HANDLE hReal = GetRealContact(event->hContact);

		char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hReal, 0);
		if (proto == NULL)
			return 0;

		Module *m = GetModule(proto);
		if (m == NULL)
			return 0;

		Dialog *dlg = (Dialog *) malloc(sizeof(Dialog));
		ZeroMemory(dlg, sizeof(Dialog));

		dlg->hContact = event->hContact;
		dlg->module = m;
		dlg->hwnd_owner = event->hwndWindow;

		LoadRichEdit(&dlg->input, event->hwndInput);
		LoadRichEdit(&dlg->log, event->hwndLog);

		if (opts.replace_in_input)
		{
			dlg->input.old_edit_proc = (WNDPROC) SetWindowLong(dlg->input.hwnd, GWL_WNDPROC, (LONG) EditProc);
			dialogData[dlg->input.hwnd] = dlg;
		}

		dlg->owner_old_edit_proc = (WNDPROC) SetWindowLong(dlg->hwnd_owner, GWL_WNDPROC, (LONG) OwnerProc);
		dialogData[dlg->hwnd_owner] = dlg;

//		dlg->log.old_edit_proc = (WNDPROC) SetWindowLong(dlg->log.hwnd, GWL_WNDPROC, (LONG) LogProc);
		dialogData[dlg->log.hwnd] = dlg;
	}
	else if (event->uType == MSG_WINDOW_EVT_CLOSING)
	{
		DialogMapType::iterator dlgit = dialogData.find(event->hwndWindow);
		if (dlgit != dialogData.end())
		{
			Dialog *dlg = dlgit->second;

			UnloadRichEdit(&dlg->input);
			UnloadRichEdit(&dlg->log);

			if (dlg->input.old_edit_proc != NULL)
				SetWindowLong(dlg->input.hwnd, GWL_WNDPROC, (LONG) dlg->input.old_edit_proc);
			SetWindowLong(dlg->hwnd_owner, GWL_WNDPROC, (LONG) dlg->owner_old_edit_proc);

			free(dlg);
		}

		dialogData.erase(event->hwndInput);
		dialogData.erase(event->hwndLog);
		dialogData.erase(event->hwndWindow);
	}

	return 0;
}


TCHAR *lstrtrim(TCHAR *str)
{
	int len = lstrlen(str);

	int i;
	for(i = len - 1; i >= 0 && (str[i] == _T(' ') || str[i] == _T('\t')); --i) ;
	if (i < len - 1)
	{
		++i;
		str[i] = _T('\0');
		len = i;
	}

	for(i = 0; i < len && (str[i] == _T(' ') || str[i] == _T('\t')); ++i) ;
	if (i > 0)
		memmove(str, &str[i], (len - i + 1) * sizeof(TCHAR));

	return str;
}


char *strtrim(char *str)
{
	int len = strlen(str);

	int i;
	for(i = len - 1; i >= 0 && (str[i] == ' ' || str[i] == '\t'); --i) ;
	if (i < len - 1)
	{
		++i;
		str[i] = '\0';
		len = i;
	}

	for(i = 0; i < len && (str[i] == ' ' || str[i] == '\t'); ++i) ;
	if (i > 0)
		memmove(str, &str[i], (len - i + 1) * sizeof(char));

	return str;
}


void LoadModules()
{
	// Load the language files and create an array with then
	TCHAR file[1024];
	mir_sntprintf(file, MAX_REGS(file), _T("%s\\*.emo"), protocolsFolder);

	WIN32_FIND_DATA ffd = {0};
	HANDLE hFFD = FindFirstFile(file, &ffd);
	if (hFFD != INVALID_HANDLE_VALUE)
	{
		do
		{
			TCHAR tmp[1024];
			mir_sntprintf(tmp, MAX_REGS(tmp), _T("%s\\%s"), protocolsFolder, ffd.cFileName);

			// Check .dic
			DWORD attrib = GetFileAttributes(tmp);
			if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			Module *m = new Module();
			m->name = mir_t2a(ffd.cFileName);
			m->name[strlen(m->name) - 4] = 0;
			m->path = mir_tstrdup(tmp);
			modules.insert(m);

			LoadModule(m);
		}
		while(FindNextFile(hFFD, &ffd));

		FindClose(hFFD);
	}
}

void HandleEmoLine(Module *m, char *tmp, int len)
{
	int state = 0;
	int pos;

	Emoticon *e = NULL;

	for(int i = 0; i < len; i++)
	{
		char c = tmp[i];
		if (c == ' ')
			continue;

		if ((state % 2) == 0)
		{
			if (c == '#')
				break;

			if (c != '"')
				continue;

			state ++;
			pos = i+1;
		}
		else
		{
			if (c == '\\')
			{
				i++;
				continue;
			}
			if (c != '"')
				continue;

			tmp[i] = 0;
			TCHAR * txt = mir_a2t(&tmp[pos]);

			for(int j = 0, orig = 0; j <= i - pos; j++)
			{
				if (txt[j] == '\\')
					j++;
				txt[orig] = txt[j];
				orig++;
			}

			// Found something
			switch(state)
			{
				case 1: 
					e = new Emoticon();
					e->name = mir_t2a(txt);
					mir_free(txt);
					break;
				case 3: 
					e->description = txt; 
					break;
				case 5: 
					e->texts.insert(txt); 
					break;
			}

			state++;
			if (state == 6)
				state = 4;
		}
	}

	if (e != NULL)
		m->emoticons.insert(e);
}


BOOL LoadModule(Module *m)
{
	FILE *file = _tfopen(m->path, _T("rb"));
	if (file == NULL) 
		return FALSE;
	
	char tmp[1024];
	char c;
	int pos = 0;
	do
	{
		c = fgetc(file);

		if (c == '\n' || c == '\r' || c == EOF || pos >= MAX_REGS(tmp) - 1) 
		{
			tmp[pos] = 0;
			HandleEmoLine(m, tmp, pos);
			pos = 0;
		}
		else
		{
			tmp[pos] = c;
			pos ++;
		}
	}
	while(c != EOF);
	fclose(file);
	return TRUE;
}



void LoadPacks()
{
	// Load the language files and create an array with then
	TCHAR file[1024];
	mir_sntprintf(file, MAX_REGS(file), _T("%s\\*"), emoticonPacksFolder);

	WIN32_FIND_DATA ffd = {0};
	HANDLE hFFD = FindFirstFile(file, &ffd);
	if (hFFD != INVALID_HANDLE_VALUE)
	{
		do
		{
			TCHAR tmp[1024];
			mir_sntprintf(tmp, MAX_REGS(tmp), _T("%s\\%s"), emoticonPacksFolder, ffd.cFileName);

			if (lstrcmp(ffd.cFileName, _T(".")) == 0 || lstrcmp(ffd.cFileName, _T("..")) == 0)
				continue;

			// Check .dic
			DWORD attrib = GetFileAttributes(tmp);
			if (attrib == 0xFFFFFFFF || !(attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			EmoticonPack *p = new EmoticonPack();
			p->name = mir_tstrdup(ffd.cFileName);
			p->path = mir_tstrdup(tmp);
			packs.insert(p);

			LoadPack(p);
		}
		while(FindNextFile(hFFD, &ffd));

		FindClose(hFFD);
	}
}

BOOL LoadPack(EmoticonPack *p)
{
	// Load the language files and create an array with then
	TCHAR filename[1024];
	mir_sntprintf(filename, MAX_REGS(filename), _T("%s\\*.*"), p->path);

	WIN32_FIND_DATA ffd = {0};
	HANDLE hFFD = FindFirstFile(filename, &ffd);
	if (hFFD != INVALID_HANDLE_VALUE)
	{
		do
		{
			TCHAR tmp[1024];
			mir_sntprintf(tmp, MAX_REGS(tmp), _T("%s\\%s"), p->path, ffd.cFileName);

			// Check .dic
			DWORD attrib = GetFileAttributes(tmp);
			if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			int len = lstrlen(ffd.cFileName);
			if (len < 5)
				continue;
			if (lstrcmp(&ffd.cFileName[len-4], _T(".jpg")) != 0
					&& lstrcmp(&ffd.cFileName[len-4], _T(".gif")) != 0
					&& lstrcmp(&ffd.cFileName[len-4], _T(".png")))
				continue;

			EmoticonImage *img = new EmoticonImage();
			img->name = mir_t2a(ffd.cFileName);
			img->name[strlen(img->name) - 4] = 0;
			img->path = mir_tstrdup(tmp);
			p->images.insert(img);
		}
		while(FindNextFile(hFFD, &ffd));

		FindClose(hFFD);
	}

	// Load the mep file
	mir_sntprintf(filename, MAX_REGS(filename), _T("%s\\%s.mep"), p->path, p->name);
	FILE *file = _tfopen(filename, _T("rb"));
	if (file == NULL) 
		return TRUE;
	
	char tmp[1024];
	char c;
	int pos = 0;
	do
	{
		c = fgetc(file);

		if (c == '\n' || c == '\r' || c == EOF || pos >= MAX_REGS(tmp) - 1) 
		{
			tmp[pos] = 0;
			
			strtrim(tmp);
			if (strnicmp("Name:", tmp, 5) == 0)
			{
				char *name = strtrim(&tmp[5]);
				if (name[0] != '\0') 
				{
					mir_free(p->name);
					p->name = mir_a2t(name);
				}
			}
			else if (strnicmp("Creator:", tmp, 8) == 0)
			{
				char *creator = strtrim(&tmp[8]);
				if (creator[0] != '\0') 
					p->creator = mir_a2t(creator);
			}
			else if (strnicmp("Updater URL:", tmp, 12) == 0)
			{
				char *updater_URL = strtrim(&tmp[12]);
				if (updater_URL[0] != '\0') 
					p->updater_URL = mir_a2t(updater_URL);
			}

			pos = 0;
		}
		else
		{
			tmp[pos] = c;
			pos ++;
		}
	}
	while(c != EOF);
	fclose(file);
	return TRUE;
}


void FillModuleImages(EmoticonPack *pack)
{
	// Fill module data
	for(int j = 0; j < modules.getCount(); j++)
	{
		Module *m = modules[j];
		for(int k = 0; k < m->emoticons.getCount(); k++)
		{
			Emoticon *e = m->emoticons[k];
			for(int i = 0; i < pack->images.getCount(); i++)
			{
				EmoticonImage *img = pack->images[i];
				if (strcmp(img->name, e->name) == 0)
				{
					e->img = img;
					break;
				}
			}
			if (e->img == NULL)
			{
				TCHAR err[1024];
				mir_sntprintf(err, MAX_REGS(err), _T("  ***  The pack '%s' does not have the emoticon '") _T(TCHAR_STR_PARAM) _T("' needed by ") _T(TCHAR_STR_PARAM),
					pack->name, e->name, m->name);
				OutputDebugString(err);
			}
		}
	}
}


EmoticonPack *GetPack(TCHAR *name)
{
	EmoticonPack *ret = NULL;
	for(int i = 0; i < packs.getCount(); i++) 
	{
		if (lstrcmpi(packs[i]->name, name) == 0)
		{
			ret = packs[i];
			break;
		}
	}
	return ret;
}


Module *GetModuleByName(const char *name)
{
	Module *ret = NULL;
	for(int i = 0; i < modules.getCount(); i++) 
	{
		if (stricmp(modules[i]->name, name) == 0)
		{
			ret = modules[i];
			break;
		}
	}
	return ret;
}


Module *GetModule(const char *name)
{
	Module *ret = GetModuleByName(name);
	if (ret == NULL && opts.use_default_pack)
		ret = GetModuleByName("Default");
	return ret;
}


int ReplaceEmoticonsService(WPARAM wParam, LPARAM lParam)
{
	SMADD_RICHEDIT3 *sre = (SMADD_RICHEDIT3 *) lParam;
	if (sre == NULL || sre->cbSize < sizeof(SMADD_RICHEDIT3))
		return FALSE;
	if (sre->hwndRichEditControl == NULL ||
			(sre->Protocolname == NULL && sre->hContact == NULL))
		return FALSE;


	DialogMapType::iterator dlgit = dialogData.find(sre->hwndRichEditControl);
	if (dlgit != dialogData.end())
	{
		Dialog *dlg = dlgit->second;
		ReplaceAllEmoticons(dlg->log, dlg->module, sre->rangeToReplace == NULL ? 0 : sre->rangeToReplace->cpMin, 
			sre->rangeToReplace == NULL ? -1 : sre->rangeToReplace->cpMax);
	}
	else
	{
		const char *proto = NULL;
		if (sre->hContact != NULL)
		{
			HANDLE hReal = GetRealContact(sre->hContact);
			proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hReal, 0);
		}
		if (proto == NULL)
			proto = sre->Protocolname;
		if (proto == NULL)
			return FALSE;

		Module *m = GetModule(proto);
		if (m == NULL)
			return FALSE;

		RichEditCtrl rec = {0};
		LoadRichEdit(&rec, sre->hwndRichEditControl);
		ReplaceAllEmoticons(rec, m, sre->rangeToReplace == NULL ? 0 : sre->rangeToReplace->cpMin, 
			sre->rangeToReplace == NULL ? -1 : sre->rangeToReplace->cpMax);
	}

	return TRUE;
}


int GetInfo2Service(WPARAM wParam, LPARAM lParam)
{
	SMADD_INFO2 *si = (SMADD_INFO2 *) lParam;
	if (si == NULL || si->cbSize < sizeof(SMADD_INFO2))
		return FALSE;

	const char *proto = NULL;
	if (si->hContact != NULL)
	{
		HANDLE hReal = GetRealContact(si->hContact);
		proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hReal, 0);
	}
	if (proto == NULL)
		proto = si->Protocolname;
	if (proto == NULL)
		return FALSE;

	Module *m = GetModule(proto);
	if (m == NULL)
		return FALSE;

	si->NumberOfVisibleSmileys = si->NumberOfSmileys = m->emoticons.getCount();

	return TRUE;
}


struct EmoticonSelectionData
{
	Module *module;
	COLORREF background;
	int max_height;
	int max_width;
	int lines;
	int cols;
	int selection;

    int xPosition;
    int yPosition;
    int Direction;
    HWND hwndTarget;
    UINT targetMessage;
    LPARAM targetWParam;

	void SetSelection(HWND hwnd, int sel)
	{
		if (sel < 0)
			sel = -1;
		if (sel >= module->emoticons.getCount())
			sel = -1;
		if (sel != selection)
			InvalidateRect(hwnd, NULL, FALSE);
		selection = sel;
	}
};


HBITMAP CreateBitmap32(int cx, int cy)
{
   BITMAPINFO RGB32BitsBITMAPINFO; 
    UINT * ptPixels;
    HBITMAP DirectBitmap;

    ZeroMemory(&RGB32BitsBITMAPINFO,sizeof(BITMAPINFO));
    RGB32BitsBITMAPINFO.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    RGB32BitsBITMAPINFO.bmiHeader.biWidth=cx;//bm.bmWidth;
    RGB32BitsBITMAPINFO.bmiHeader.biHeight=cy;//bm.bmHeight;
    RGB32BitsBITMAPINFO.bmiHeader.biPlanes=1;
    RGB32BitsBITMAPINFO.bmiHeader.biBitCount=32;

    DirectBitmap = CreateDIBSection(NULL, 
                                       (BITMAPINFO *)&RGB32BitsBITMAPINFO, 
                                       DIB_RGB_COLORS,
                                       (void **)&ptPixels, 
                                       NULL, 0);
    return DirectBitmap;
}


#define MIN_COLS 5
#define MAX_LINES 5
#define BORDER 5


void PreMultiply(HBITMAP hBitmap)
{
	BYTE *p = NULL;
	DWORD dwLen;
	int width, height, x, y;
	BITMAP bmp;
	BYTE alpha;
	BOOL transp = FALSE;

	GetObject(hBitmap, sizeof(bmp), &bmp);
	width = bmp.bmWidth;
	height = bmp.bmHeight;
	dwLen = width * height * 4;
	p = (BYTE *)malloc(dwLen);
	if (p != NULL)
	{
		GetBitmapBits(hBitmap, dwLen, p);

		for (y = 0; y < height; ++y)
		{
			BYTE *px = p + width * 4 * y;

			for (x = 0; x < width; ++x)
			{
				alpha = px[3];

				if (alpha < 255)
				{
					transp  = TRUE;

					px[0] = px[0] * alpha/255;
					px[1] = px[1] * alpha/255;
					px[2] = px[2] * alpha/255;
				}

				px += 4;
			}
		}

		if (transp)
			dwLen = SetBitmapBits(hBitmap, dwLen, p);
		free(p);
	}
}


HWND CreateTooltip(HWND hwnd, RECT &rect, TCHAR *text)
{
          // struct specifying control classes to register
    INITCOMMONCONTROLSEX iccex; 
    HWND hwndTT;                 // handle to the ToolTip control
          // struct specifying info about tool in ToolTip control
    TOOLINFO ti;
    unsigned int uid = 0;       // for ti initialization

	// Load the ToolTip class from the DLL.
    iccex.dwSize = sizeof(iccex);
    iccex.dwICC  = ICC_BAR_CLASSES;

    if(!InitCommonControlsEx(&iccex))
       return NULL;

    /* CREATE A TOOLTIP WINDOW */
    hwndTT = CreateWindowEx(WS_EX_TOPMOST,
        TOOLTIPS_CLASS,
        NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hwnd,
        NULL,
        hInst,
        NULL
        );

	/* Gives problem with mToolTip
    SetWindowPos(hwndTT,
        HWND_TOPMOST,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	*/

    /* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hwnd;
    ti.hinst = hInst;
    ti.uId = uid;
    ti.lpszText = text;
        // ToolTip control will cover the whole window
    ti.rect.left = rect.left;    
    ti.rect.top = rect.top;
    ti.rect.right = rect.right;
    ti.rect.bottom = rect.bottom;

    /* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
    SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
	SendMessage(hwndTT, TTM_SETDELAYTIME, (WPARAM) (DWORD) TTDT_AUTOPOP, (LPARAM) MAKELONG(24 * 60 * 60 * 1000, 0));	

	return hwndTT;
} 


void AssertInsideScreen(RECT &rc)
{
	// Make sure it is inside screen
	if (IsWinVer98Plus()) {
		static BOOL loaded = FALSE;
		static HMONITOR (WINAPI *MyMonitorFromRect)(LPCRECT,DWORD) = NULL;
		static BOOL (WINAPI *MyGetMonitorInfo)(HMONITOR,LPMONITORINFO) = NULL;

		if (!loaded) {
			HMODULE hUser32 = GetModuleHandleA("user32");
			if (hUser32) {
				MyMonitorFromRect = (HMONITOR(WINAPI*)(LPCRECT,DWORD))GetProcAddress(hUser32,"MonitorFromRect");
				MyGetMonitorInfo = (BOOL(WINAPI*)(HMONITOR,LPMONITORINFO))GetProcAddress(hUser32,"GetMonitorInfoA");
				if (MyGetMonitorInfo == NULL)
					MyGetMonitorInfo = (BOOL(WINAPI*)(HMONITOR,LPMONITORINFO))GetProcAddress(hUser32,"GetMonitorInfo");
			}
			loaded = TRUE;
		}

		if (MyMonitorFromRect != NULL && MyGetMonitorInfo != NULL) {
			HMONITOR hMonitor;
			MONITORINFO mi;

			hMonitor = MyMonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
			mi.cbSize = sizeof(mi);
			MyGetMonitorInfo(hMonitor, &mi);

			if (rc.bottom > mi.rcWork.bottom)
				OffsetRect(&rc, 0, mi.rcWork.bottom - rc.bottom);
			if (rc.bottom < mi.rcWork.top)
				OffsetRect(&rc, 0, mi.rcWork.top - rc.top);
			if (rc.top > mi.rcWork.bottom)
				OffsetRect(&rc, 0, mi.rcWork.bottom - rc.bottom);
			if (rc.top < mi.rcWork.top)
				OffsetRect(&rc, 0, mi.rcWork.top - rc.top);
			if (rc.right > mi.rcWork.right)
				OffsetRect(&rc, mi.rcWork.right - rc.right, 0);
			if (rc.right < mi.rcWork.left)
				OffsetRect(&rc, mi.rcWork.left - rc.left, 0);
			if (rc.left > mi.rcWork.right)
				OffsetRect(&rc, mi.rcWork.right - rc.right, 0);
			if (rc.left < mi.rcWork.left)
				OffsetRect(&rc, mi.rcWork.left - rc.left, 0);
		}
	}
}

void LoadImage(EmoticonImage *img, int &max_height, int &max_width)
{
	if (img == NULL)
		return;

	DWORD transparent;

	char *path = mir_t2a(img->path);
	img->img = (HBITMAP) CallService(MS_AV_LOADBITMAP32, (WPARAM) &transparent, (LPARAM) path);
	mir_free(path);

	if (img->img == NULL)
		return;

	BITMAP bmp;
	if (!GetObject(img->img, sizeof(bmp), &bmp))
	{
		DeleteObject(img->img);
		img->img = NULL;
		return;
	}

	img->transparent = (bmp.bmBitsPixel == 32 && transparent);
	if (img->transparent)
		PreMultiply(img->img);

	max_height = max(max_height, bmp.bmHeight);
	max_width = max(max_width, bmp.bmWidth);
}


void ReleaseImage(EmoticonImage *img)
{
	if (img == NULL || img->img == NULL)
		return;

	DeleteObject(img->img);
	img->img = NULL;
}


INT_PTR CALLBACK EmoticonSeletionDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch(msg) 
	{
		case WM_INITDIALOG: 
		{
			EmoticonSelectionData *ssd = (EmoticonSelectionData *) lParam;
			SetWindowLong(hwnd, GWL_USERDATA, (LONG) ssd);

			ssd->selection = -1;

			// Load emoticons
			ssd->max_height = 4;
			ssd->max_width = 4;

			HDC hdc = GetDC(hwnd);

			int num_emotes = ssd->module->emoticons.getCount();
			int i;
			for(i = 0; i < num_emotes; i++)
			{
				Emoticon *e = ssd->module->emoticons[i];
				LoadImage(e->img, ssd->max_height, ssd->max_width);

				if (e->img == NULL || e->img->img == NULL)
				{
					HFONT hFont;
					if (ssd->hwndTarget != NULL)
					{
						CHARFORMAT2 cf;
						ZeroMemory(&cf, sizeof(cf));
						cf.cbSize = sizeof(cf);
						cf.dwMask = CFM_FACE | CFM_ITALIC | CFM_CHARSET | CFM_FACE | CFM_WEIGHT | CFM_SIZE;
						SendMessage(ssd->hwndTarget, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);

						LOGFONT lf = {0};
						lf.lfHeight = -MulDiv(cf.yHeight / 20, GetDeviceCaps(hdc, LOGPIXELSY), 72);
						lf.lfWeight = cf.wWeight;
						lf.lfItalic = (cf.dwEffects & CFE_ITALIC) == CFE_ITALIC;
						lf.lfCharSet = cf.bCharSet;
						lf.lfPitchAndFamily = cf.bPitchAndFamily;
						lstrcpyn(lf.lfFaceName, cf.szFaceName, MAX_REGS(lf.lfFaceName));

						hFont = CreateFontIndirect(&lf);
					}
					else
						hFont = (HFONT) SendMessage(hwnd, WM_GETFONT, 0, 0);

					if (hFont != NULL)
						SelectObject(hdc, hFont);

					RECT rc = { 0, 0, 0xFFFF, 0xFFFF };
					DrawText(hdc, e->texts[0], lstrlen(e->texts[0]), &rc, DT_CALCRECT | DT_NOPREFIX);

					ssd->max_height = max(ssd->max_height, rc.bottom - rc.top + 1);
					ssd->max_width = max(ssd->max_width, rc.right - rc.left + 1);

					if (ssd->hwndTarget != NULL)
						DeleteObject(hFont);
				}
			}

			ReleaseDC(hwnd, hdc);

			ssd->cols = num_emotes / MAX_LINES;
			if (num_emotes % MAX_LINES != 0)
				ssd->cols++;
			ssd->cols = max(ssd->cols, MIN_COLS);

			ssd->lines = num_emotes / ssd->cols;
			if (num_emotes % ssd->cols != 0)
				ssd->lines++;

			// Calc position
			int width = ssd->max_width * ssd->cols + (ssd->cols + 1) * BORDER + 1;
			int height = ssd->max_height * ssd->lines + (ssd->lines + 1) * BORDER + 1;

			int x = ssd->xPosition;
			int y = ssd->yPosition;
			switch (ssd->Direction) 
			{
				case 1: 
					x -= width;
					break;
				case 2:
					x -= width;
					y -= height;
					break;
				case 3:
					y -= height;
					break;
			}

			// Get background
			ssd->background = RGB(255, 255, 255);
			if (ssd->hwndTarget != NULL)
			{
				ssd->background = SendMessage(ssd->hwndTarget, EM_SETBKGNDCOLOR, 0, ssd->background);
				SendMessage(ssd->hwndTarget, EM_SETBKGNDCOLOR, 0, ssd->background);
			}

			RECT rc = { x, y, x + width, y + height };
			AssertInsideScreen(rc);
			SetWindowPos(hwnd, HWND_TOPMOST, rc.left, rc.top, width, height, 0);

			for(i = 0; i < ssd->lines; i++)
			{
				for(int j = 0; j < ssd->cols; j++)
				{
					int index = i * ssd->cols + j;
					if (index >= ssd->module->emoticons.getCount())
						break;
					
					Emoticon *e = ssd->module->emoticons[index];

					RECT fr;
					fr.left = BORDER + j * (ssd->max_width + BORDER) - 1;
					fr.right = fr.left + ssd->max_width + 2;
					fr.top = BORDER + i * (ssd->max_height + BORDER) - 1;
					fr.bottom = fr.top + ssd->max_height + 2;

					Buffer<TCHAR> tt;
					if (e->description[0] != _T('\0'))
					{
						tt += e->description;
						tt.translate();
						tt += _T(" ");
					}

					for(int k = 0; k < e->texts.getCount(); k++)
					{
						tt += _T(" ");
						tt += e->texts[k];
						tt += _T(" ");
					}
					tt.pack();

					e->tt = CreateTooltip(hwnd, fr, tt.str);
				}
			}

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER | TME_LEAVE;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);

			return TRUE;
		}

		case WM_PAINT:
		{
			RECT r;
			if (GetUpdateRect(hwnd, &r, FALSE)) 
			{
				EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

				PAINTSTRUCT ps;

				HDC hdc_orig = BeginPaint(hwnd, &ps);

				RECT rc;
				GetClientRect(hwnd, &rc);

				// Create double buffer
				HDC hdc = CreateCompatibleDC(hdc_orig);
				HBITMAP hBmp = CreateBitmap32(rc.right, rc.bottom);
				SelectObject(hdc, hBmp);

				SetBkMode(hdc, TRANSPARENT);

				// Erase background
				HBRUSH hB = CreateSolidBrush(ssd->background);
				FillRect(hdc, &rc, hB);
				DeleteObject(hB);

				// Draw emoticons
				for(int i = 0; i < ssd->lines; i++)
				{
					for(int j = 0; j < ssd->cols; j++)
					{
						int index = i * ssd->cols + j;
						if (index >= ssd->module->emoticons.getCount())
							break;
						
						Emoticon *e = ssd->module->emoticons[index];
						if (e->img == NULL || e->img->img == NULL)
						{
							HFONT hFont;
							if (ssd->hwndTarget != NULL)
							{
								CHARFORMAT2 cf;
								ZeroMemory(&cf, sizeof(cf));
								cf.cbSize = sizeof(cf);
								cf.dwMask = CFM_FACE | CFM_ITALIC | CFM_CHARSET | CFM_FACE | CFM_WEIGHT | CFM_SIZE | CFM_COLOR;
								SendMessage(ssd->hwndTarget, EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM) &cf);

								LOGFONT lf = {0};
								lf.lfHeight = -MulDiv(cf.yHeight / 20, GetDeviceCaps(hdc, LOGPIXELSY), 72);
								lf.lfWeight = cf.wWeight;
								lf.lfItalic = (cf.dwEffects & CFE_ITALIC) == CFE_ITALIC;
								lf.lfCharSet = cf.bCharSet;
								lf.lfPitchAndFamily = cf.bPitchAndFamily;
								lstrcpyn(lf.lfFaceName, cf.szFaceName, MAX_REGS(lf.lfFaceName));

								hFont = CreateFontIndirect(&lf);
								SetTextColor(hdc, cf.crTextColor);
							}
							else
								hFont = (HFONT) SendMessage(hwnd, WM_GETFONT, 0, 0);

							if (hFont != NULL)
								SelectObject(hdc, hFont);

							RECT rc = { 0, 0, 0xFFFF, 0xFFFF };
							DrawText(hdc, e->texts[0], lstrlen(e->texts[0]), &rc, DT_CALCRECT | DT_NOPREFIX);

							int height = rc.bottom - rc.top + 1;
							int width = rc.right - rc.left + 1;

							rc.left = BORDER + j * (ssd->max_width + BORDER) + (ssd->max_width - width) / 2;
							rc.top = BORDER + i * (ssd->max_height + BORDER) + (ssd->max_height - height) / 2;

							rc.right = rc.left + width;
							rc.bottom = rc.top + height;

							DrawText(hdc, e->texts[0], lstrlen(e->texts[0]), &rc, DT_NOPREFIX);

							if (ssd->hwndTarget != NULL)
								DeleteObject(hFont);
						}
						else
						{
							BITMAP bmp;
							GetObject(e->img->img, sizeof(bmp), &bmp);

							int x = BORDER + j * (ssd->max_width + BORDER) + (ssd->max_width - bmp.bmWidth) / 2;
							int y = BORDER + i * (ssd->max_height + BORDER) + (ssd->max_height - bmp.bmHeight) / 2;

							HDC hdc_img = CreateCompatibleDC(hdc);
							HBITMAP old_bmp = (HBITMAP) SelectObject(hdc_img, e->img->img);

							if (e->img->transparent)
							{
								BLENDFUNCTION bf = {0};
								bf.SourceConstantAlpha = 255;
								bf.AlphaFormat = AC_SRC_ALPHA;
								AlphaBlend(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hdc_img, 0, 0, bmp.bmWidth, bmp.bmHeight, bf);
							}
							else
							{
								BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hdc_img, 0, 0, SRCCOPY);
							}

							SelectObject(hdc_img, old_bmp);
							DeleteDC(hdc_img);

						}

						if (ssd->selection == index)
						{
							RECT fr;
							fr.left = BORDER + j * (ssd->max_width + BORDER) - 1;
							fr.right = fr.left + ssd->max_width + 2;
							fr.top = BORDER + i * (ssd->max_height + BORDER) - 1;
							fr.bottom = fr.top + ssd->max_height + 2;
							FrameRect(hdc, &fr, (HBRUSH) GetStockObject(GRAY_BRUSH));
						}
					}
				}

				// Copy buffer to screen
				BitBlt(hdc_orig, rc.left, rc.top, rc.right - rc.left, 
						rc.bottom - rc.top, hdc, rc.left, rc.top, SRCCOPY);
				DeleteDC(hdc);
				DeleteObject(hBmp);

				EndPaint(hwnd, &ps);
			}
			
			return TRUE;
		}

		case WM_MOUSELEAVE:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
		}
		case WM_NCMOUSEMOVE:
		{
			EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);
			ssd->SetSelection(hwnd, -1);
			break;
		}

		case WM_MOUSEHOVER:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
		}
		case WM_MOUSEMOVE:
		{
			EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

			POINT p;
			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 

			int col;
			if (p.x % (BORDER + ssd->max_width) < BORDER)
				col = -1;
			else
				col = p.x / (BORDER + ssd->max_width);

			int line;
			if (p.y % (BORDER + ssd->max_height) < BORDER)
				line = -1;
			else
				line = p.y / (BORDER + ssd->max_height);

			int index = line * ssd->cols + col;

			if (col >= 0 && line >= 0 && index < ssd->module->emoticons.getCount())
			{
				ssd->SetSelection(hwnd, index);
			}
			else
			{
				ssd->SetSelection(hwnd, -1);
			}

			break;
		}

		case WM_GETDLGCODE:
		{
			if (lParam != NULL)
			{
				static DWORD last_time = 0;

				MSG *msg = (MSG* ) lParam;
				if (msg->message == WM_KEYDOWN && msg->time != last_time)
				{
					last_time = msg->time;

					if (msg->wParam == VK_UP)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

						if (ssd->selection < 0)
						{
							ssd->SetSelection(hwnd, (ssd->lines - 1) * ssd->cols);
						}
						else
						{
							int index = (ssd->selection - ssd->cols) % ssd->module->emoticons.getCount();
							if (index < 0)
								index += ssd->module->emoticons.getCount();
							ssd->SetSelection(hwnd, index);
						}
					}
					else if (msg->wParam == VK_DOWN)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

						if (ssd->selection < 0)
						{
							ssd->SetSelection(hwnd, 0);
						}
						else
						{
							ssd->SetSelection(hwnd, (ssd->selection + ssd->cols) % ssd->module->emoticons.getCount());
						}
					}
					else if (msg->wParam == VK_LEFT)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

						if (ssd->selection < 0)
						{
							ssd->SetSelection(hwnd, ssd->cols - 1);
						}
						else
						{
							int index = (ssd->selection - 1) % ssd->module->emoticons.getCount();
							if (index < 0)
								index += ssd->module->emoticons.getCount();
							ssd->SetSelection(hwnd, index);
						}
					}
					else if (msg->wParam == VK_RIGHT)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

						if (ssd->selection < 0)
						{
							ssd->SetSelection(hwnd, 0);
						}
						else
						{
							ssd->SetSelection(hwnd, (ssd->selection + 1) % ssd->module->emoticons.getCount());
						}
					}
					else if (msg->wParam == VK_HOME)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);
						ssd->SetSelection(hwnd, 0);
					}
					else if (msg->wParam == VK_END)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);
						ssd->SetSelection(hwnd, ssd->module->emoticons.getCount() - 1);
					}
				}
			}

			return DLGC_WANTALLKEYS;
		}
		
	    case WM_ACTIVATE:
		{
			if (wParam == WA_INACTIVE) 
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDOK:
					PostMessage(hwnd, WM_LBUTTONUP, 0, 0);
					break;

				case IDCANCEL:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
			}
			break;
		}

		case WM_LBUTTONUP:
		{
			EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);
			if (ssd->selection >= 0 && ssd->hwndTarget != NULL)
			{
				if (opts.only_replace_isolated)
				{
					TCHAR tmp[16];
					mir_sntprintf(tmp, MAX_REGS(tmp), _T(" %s "), ssd->module->emoticons[ssd->selection]->texts[0]);
					SendMessage(ssd->hwndTarget, ssd->targetMessage, ssd->targetWParam, (LPARAM) tmp);
				}
				else
					SendMessage(ssd->hwndTarget, ssd->targetMessage, ssd->targetWParam, (LPARAM) ssd->module->emoticons[ssd->selection]->texts[0]);
			}

			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		}

		case WM_CLOSE:
		{
			EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);
			SetWindowLong(hwnd, GWL_USERDATA, NULL);

			for(int i = 0; i < ssd->module->emoticons.getCount(); i++)
			{
				Emoticon *e = ssd->module->emoticons[i];
				ReleaseImage(e->img);

				if (e->tt != NULL)
				{
					DestroyWindow(e->tt);
					e->tt = NULL;
				}
			}

			DestroyWindow(hwnd);

			SetFocus(ssd->hwndTarget);
			delete ssd;
			break;
		}
	}

	return FALSE;
}

int ShowSelectionService(WPARAM wParam, LPARAM lParam)
{
    SMADD_SHOWSEL3 *sss = (SMADD_SHOWSEL3 *)lParam;
	if (sss == NULL || sss->cbSize < sizeof(SMADD_SHOWSEL3)) 
		return FALSE;

	const char *proto = NULL;
	if (sss->hContact != NULL)
	{
		HANDLE hReal = GetRealContact(sss->hContact);
		proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hReal, 0);
	}
	if (proto == NULL)
		proto = sss->Protocolname;
	if (proto == NULL)
		return FALSE;

	Module *m = GetModule(proto);
	if (m == NULL)
		return FALSE;

	EmoticonSelectionData * ssd = new EmoticonSelectionData();
	ssd->module = m;

	ssd->xPosition = sss->xPosition;
	ssd->yPosition = sss->yPosition;
	ssd->Direction = sss->Direction;

	ssd->hwndTarget = sss->hwndTarget;
	ssd->targetMessage = sss->targetMessage;
	ssd->targetWParam = sss->targetWParam;

	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EMOTICON_SELECTION), sss->hwndParent, 
					  EmoticonSeletionDlgProc, (LPARAM) ssd);

    return TRUE;
}


