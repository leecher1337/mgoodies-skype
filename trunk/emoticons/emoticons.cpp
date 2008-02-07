/* 
Copyright (C) 2008 Ricardo Pescuma Domenecci

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
	PLUGIN_MAKE_VERSION(0,0,0,9),
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
FI_INTERFACE *fei = NULL;

LIST<Module> modules(10);
LIST<EmoticonPack> packs(10);
LIST<Contact> contacts(10);
LIST<OleImage> downloading(10);

BOOL LoadModule(Module *m);
void LoadModules();
BOOL LoadPack(EmoticonPack *p);
void LoadPacks();

void FillModuleImages(EmoticonPack *pack);

EmoticonPack *GetPack(TCHAR *name);
Module *GetModule(const char *name);
Contact * GetContact(HANDLE hContact);
CustomEmoticon *GetCustomEmoticon(Contact *c, TCHAR *text);

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);
int MsgWindowEvent(WPARAM wParam, LPARAM lParam);
int CustomSmileyReceivedEvent(WPARAM wParam, LPARAM lParam);

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

	// TODO Assert results here
	init_mir_malloc();
	mir_getLI(&li);
	CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM) &fei);

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

	// Hook custom emoticons notification
	PROTOCOLDESCRIPTOR **protos;
	int count;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);
	for (int i = 0; i < count; i++)
	{
		if (protos[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0')
			continue;

		char evname[250];
		mir_snprintf(evname, MAX_REGS(evname), "%s%s", protos[i]->szName, ME_CUSTOMSMILEY_RECEIVED);
		HookEvent(evname, &CustomSmileyReceivedEvent);
	}

	loaded = TRUE;

	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	int i;

	for(i = downloading.getCount() - 1; i >= 0; i--)
	{
		OleImage *oimg = downloading[i];
		oimg->Release();

		downloading.remove(i);
	}

	for(i = 0; i < packs.getCount(); i++)
		for(int j = 0; j < packs[i]->images.getCount(); j++)
			ReleaseImage(packs[i]->images[j]);

	for(i = 0; i < MAX_REGS(hServices); i++)
		DestroyServiceFunction(hServices[i]);

	for(i = 0; i < MAX_REGS(hHooks); i++)
		UnhookEvent(hHooks[i]);

	DeInitOptions();

	return 0;
}


// Return the size difference with the original text
int ReplaceEmoticonBackwards(RichEditCtrl &rec, Contact *contact, Module *module, TCHAR *text, int text_len, int last_pos, TCHAR next_char)
{
	// This are needed to allow 2 different emoticons that end the same way
	TCHAR *found_path = NULL;
	int found_len = -1;
	TCHAR *found_text;
	BOOL down = FALSE;

	// Replace normal emoticons
	if (!opts.only_replace_isolated || next_char == _T('\0') || _istspace(next_char))
	{	
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

				if (len <= found_len)
					continue;

				if (_tcsncmp(&text[text_len - len], txt, len) != 0)
					continue;

				if (opts.only_replace_isolated && text_len > len 
						&& !_istspace(text[text_len - len - 1]))
					continue;

				found_path = e->img->path;
				found_len = len;
				found_text = txt;
			}
		}
	}

	// Replace custom smileys
	if (contact != NULL)
	{
		for(int i = 0; i < contact->emoticons.getCount(); i++)
		{
			CustomEmoticon *e = contact->emoticons[i];

			TCHAR *txt = e->text;
			int len = lstrlen(txt);
			if (last_pos < len || text_len < len)
				continue;

			if (len <= found_len)
				continue;

			if (_tcsncmp(&text[text_len - len], txt, len) != 0)
				continue;

			found_path = e->path;
			found_len = len;
			found_text = txt;
			down = e->downloading;
		}
	}

	int ret = 0;

	if (found_path != NULL)
	{
		// Found ya
		CHARRANGE sel = { last_pos - found_len, last_pos };
		SendMessage(rec.hwnd, EM_EXSETSEL, 0, (LPARAM) &sel);

		if (ServiceExists(MS_INSERTANISMILEY))
		{
			CHARFORMAT2 cf;
			memset(&cf, 0, sizeof(CHARFORMAT2));
			cf.cbSize = sizeof(CHARFORMAT2);
			cf.dwMask = CFM_BACKCOLOR;
			SendMessage(rec.hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);

			if (cf.dwEffects & CFE_AUTOBACKCOLOR)
			{
				cf.crBackColor = SendMessage(rec.hwnd, EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_WINDOW));
				SendMessage(rec.hwnd, EM_SETBKGNDCOLOR, 0, cf.crBackColor);
			}

			if (InsertAnimatedSmiley(rec.hwnd, found_path, cf.crBackColor, 0 , found_text))
			{
				ret = - found_len + 1;
			}
		}
		else
		{
			OleImage *img = new OleImage(found_path, found_text, found_text);
			if (!img->isValid())
			{
				if (down && img->ShowDownloadingIcon(TRUE))
				{
					img->AddRef();
					downloading.insert(img);
				}
				else
				{
					delete img;
					return 0;
				}
			}

			IOleClientSite *clientSite; 
			rec.ole->GetClientSite(&clientSite);

			REOBJECT reobject = {0};
			reobject.cbStruct = sizeof(REOBJECT);
			reobject.cp = REO_CP_SELECTION;
			reobject.dvaspect = DVASPECT_CONTENT;
			reobject.poleobj = img;
			reobject.polesite = clientSite;
			reobject.dwFlags = REO_BELOWBASELINE; // | REO_DYNAMICSIZE;

			if (rec.ole->InsertObject(&reobject) == S_OK)
			{
				img->SetClientSite(clientSite);
				ret = - found_len + 1;
			}

			clientSite->Release();
			img->Release();
		}
	}

	return ret;
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


void ReplaceAllEmoticonsBackwards(RichEditCtrl &rec, Contact *contact, Module *module, TCHAR *text, int len, TCHAR next_char, int start, CHARRANGE &__old_sel)
{
	for(int i = len; i > 0; i--)
	{
		int dif = ReplaceEmoticonBackwards(rec, contact, module, text, i, start + i, i == len ? next_char : text[i]);
		if (dif != 0)
		{
			FixSelection(__old_sel.cpMax, i, dif);
			FixSelection(__old_sel.cpMin, i, dif);

			i += dif;
		}
	}
}


void ReplaceAllEmoticonsBackwards(RichEditCtrl &rec, Contact *contact, Module *module)
{
	STOP_RICHEDIT(rec);

	TCHAR *text = GetText(rec, 0, -1);
	int len = lstrlen(text);

	ReplaceAllEmoticonsBackwards(rec, contact, module, text, len, _T('\0'), 0, __old_sel);

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


void ReplaceAllEmoticons(RichEditCtrl &rec, Contact *contact, Module *module, int start, int end)
{
	STOP_RICHEDIT(rec);

	if (start < 0)
		start = 0;

	TCHAR *text = GetText(rec, start, end);
	int len = lstrlen(text);

	int last_start_pos = 0;
	BOOL replace = TRUE;
	HANDLE hContact = (contact == NULL ? NULL : contact->hContact);
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
						ReplaceAllEmoticonsBackwards(rec, contact, module, &text[last_start_pos], i - last_start_pos, _T('\0'), start + last_start_pos, __old_sel);

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
				ReplaceAllEmoticonsBackwards(rec, contact, module, &text[last_start_pos], i - last_start_pos, _T('\0'), start + last_start_pos, __old_sel);

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
				ReplaceAllEmoticonsBackwards(rec, contact, module, &text[last_start_pos], i - last_start_pos, _T('\0'), start + last_start_pos, __old_sel);

				hContact = (contact == NULL ? NULL : contact->hContact);
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
				ReplaceAllEmoticonsBackwards(rec, contact, module, &text[last_start_pos], i - last_start_pos, _T('\0'), start + last_start_pos, __old_sel);

				hContact = (HANDLE) _ttoi(&text[i + tl]);
				i += len - 1;
				last_start_pos = i + 1;
			}
		}
	}

	if (replace)
		ReplaceAllEmoticonsBackwards(rec, contact, module, &text[last_start_pos], len - last_start_pos, _T('\0'), start + last_start_pos, __old_sel);
	
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

		ITooltipData *ttd = NULL;
		hr = reObj.poleobj->QueryInterface(__uuidof(ITooltipData), (void**) &ttd);
		reObj.poleobj->Release();
		if (SUCCEEDED(hr) && ttd == NULL)
			continue;

		BSTR hint = NULL;
		hr = ttd->GetTooltip(&hint);
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

		ttd->Release();
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
				dif = ReplaceEmoticonBackwards(dlg->input, NULL, dlg->module, text, len, sel.cpMax, last);
				if (dif != 0)
				{
					FixSelection(__old_sel.cpMax, sel.cpMax, dif);
					FixSelection(__old_sel.cpMin, sel.cpMax, dif);
				}
			}
			else
			{
				// Because we already changed the text, we need to replace all range
				ReplaceAllEmoticonsBackwards(dlg->input, NULL, dlg->module, text, len, last, min, __old_sel);
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
			ReplaceAllEmoticonsBackwards(dlg->input, NULL, dlg->module);

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
					ReplaceAllEmoticonsBackwards(dlg->input, NULL, dlg->module);

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

		dlg->contact = GetContact(event->hContact);
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
			e->img = NULL;
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
		ReplaceAllEmoticons(dlg->log, dlg->contact, dlg->module, sre->rangeToReplace == NULL ? 0 : sre->rangeToReplace->cpMin, 
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
		ReplaceAllEmoticons(rec, GetContact(sre->hContact), m, sre->rangeToReplace == NULL ? 0 : sre->rangeToReplace->cpMin, 
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


void LoadImage(EmoticonImage *img, int &max_height, int &max_width)
{
	if (img == NULL)
		return;

	if (img->img != NULL)
	{
		BITMAP bmp;
		GetObject(img->img, sizeof(bmp), &bmp);

		max_height = max(max_height, bmp.bmHeight);
		max_width = max(max_width, bmp.bmWidth);

		return;
	}

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


Contact * GetContact(HANDLE hContact)
{
	if (hContact == NULL)
		return NULL;

	// Check if already loaded
	for(int i = 0; i < contacts.getCount(); i++)
		if (contacts[i]->hContact == hContact)
			return contacts[i];

	Contact *c = new Contact(hContact);
	contacts.insert(c);

	// Get from db
	BOOL pack = FALSE;
	c->lastId = -1;
	char setting[256];
	while(TRUE) 
	{
		c->lastId++;

		mir_snprintf(setting, MAX_REGS(setting), "%d_Text", c->lastId);
		DBVARIANT dbv_text = {0};
		if (DBGetContactSettingTString(hContact, "CustomSmileys", setting, &dbv_text))
			break;

		mir_snprintf(setting, MAX_REGS(setting), "%d_Path", c->lastId);
		DBVARIANT dbv_path = {0};
		if (DBGetContactSettingTString(hContact, "CustomSmileys", setting, &dbv_path))
		{
			pack = TRUE;
			DBFreeVariant(&dbv_text);
			continue;
		}
		
		if (GetCustomEmoticon(c, dbv_text.ptszVal) != NULL)
		{
			pack = TRUE;
		}
		else
		{
			CustomEmoticon *ce = new CustomEmoticon();
			ce->text = mir_tstrdup(dbv_text.ptszVal);
			ce->path = mir_tstrdup(dbv_path.ptszVal);

			c->emoticons.insert(ce);
		}

		DBFreeVariant(&dbv_path);
		DBFreeVariant(&dbv_text);
	}

	c->lastId--;

	if (pack)
	{
		// Re-store then
		int i;
		for(i = 0; i < c->emoticons.getCount(); i++)
		{
			CustomEmoticon *ce = c->emoticons[i];

			mir_snprintf(setting, MAX_REGS(setting), "%d_Text", i);
			DBWriteContactSettingTString(c->hContact, "CustomSmileys", setting, ce->text);

			mir_snprintf(setting, MAX_REGS(setting), "%d_Path", i);
			DBWriteContactSettingTString(c->hContact, "CustomSmileys", setting, ce->path);
		}
		for(int j = i; j <= c->lastId; j++)
		{
			mir_snprintf(setting, MAX_REGS(setting), "%d_Text", j);
			DBDeleteContactSetting(c->hContact, "CustomSmileys", setting);

			mir_snprintf(setting, MAX_REGS(setting), "%d_Path", j);
			DBDeleteContactSetting(c->hContact, "CustomSmileys", setting);
		}

		c->lastId = i - 1;
	}

	return c;
}


CustomEmoticon *GetCustomEmoticon(Contact *c, TCHAR *text)
{
	for(int i = 0; i < c->emoticons.getCount(); i++)
	{
		if (lstrcmp(c->emoticons[i]->text, text) == 0)
			return c->emoticons[i];
	}
	return NULL;
}


void DownloadedCustomEmoticon(HANDLE hContact, TCHAR *path)
{
	// Mark that it was received

	Contact *c = GetContact(hContact);
	if (c != NULL)
	{
		for(int i = 0; i < c->emoticons.getCount(); i++)
		{
			CustomEmoticon *ce = c->emoticons[i];
			if (lstrcmpi(ce->path, path) == 0)
				ce->downloading = FALSE;
		}
	}

	for(int i = downloading.getCount() - 1; i >= 0; i--)
	{
		if (lstrcmpi(downloading[i]->GetFilename(), path) == 0)
		{
			OleImage *oimg = downloading[i];
			oimg->ShowDownloadingIcon(FALSE);
			oimg->Release();

			downloading.remove(i);
		}
	}
}

int CustomSmileyReceivedEvent(WPARAM wParam, LPARAM lParam)
{
	CUSTOMSMILEY *cs = (CUSTOMSMILEY *) lParam;
	if (cs == NULL || cs->cbSize < sizeof(CUSTOMSMILEY) || cs->pszFilename == NULL || cs->hContact == NULL)
		return 0;

	TCHAR *path = mir_a2t(cs->pszFilename);

	// Check if this is the second notification
	if (!(cs->flags & CUSTOMSMILEY_STATE_RECEIVED))
	{
		if (cs->flags & CUSTOMSMILEY_STATE_DOWNLOADED)
			DownloadedCustomEmoticon(cs->hContact, path);

		mir_free(path);
		return 0;
	}

	// This is the first one 
	if (cs->pszText == NULL)
	{
		mir_free(path);
		return 0;
	}

	// Get data
	Contact *c = GetContact(cs->hContact);

	TCHAR *text;
	if (cs->flags & CUSTOMSMILEY_UNICODE)
		text = mir_u2t(cs->pwszText);
	else
		text = mir_a2t(cs->pszText);

	// Create it
	CustomEmoticon *ce = GetCustomEmoticon(c, text);
	if (ce != NULL)
	{
		mir_free(ce->path);
		ce->path = path;

		mir_free(text);
	}
	else
	{
		ce = new CustomEmoticon();
		ce->text = text;
		ce->path = path;

		c->emoticons.insert(ce);
		c->lastId++;
	}

	// Check if need to download
	if (!(cs->flags & CUSTOMSMILEY_STATE_DOWNLOADED) && GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
	{
		// Request emoticon download
		cs->download = TRUE;
		ce->downloading = TRUE;
	}

	// Store in DB
	char setting[256];
	mir_snprintf(setting, MAX_REGS(setting), "%d_Text", c->lastId);
	DBWriteContactSettingTString(c->hContact, "CustomSmileys", setting, ce->text);

	mir_snprintf(setting, MAX_REGS(setting), "%d_Path", c->lastId);
	DBWriteContactSettingTString(c->hContact, "CustomSmileys", setting, ce->path);

	return 0;
}
