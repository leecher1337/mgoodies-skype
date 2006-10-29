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
	PLUGIN_MAKE_VERSION(0,0,0,1),
	"Spell Checker",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2006 Ricardo Pescuma Domenecci",
	"http://miranda-im.org/",
	0,	//not transient
	0	//doesn't replace anything built-in
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hEnableMenu = NULL; 
HANDLE hDisableMenu = NULL; 
HANDLE hModulesLoaded = NULL;
HANDLE hPreBuildCMenu = NULL;
HANDLE hSettingChanged = NULL;

char *metacontacts_proto = NULL;
BOOL loaded = FALSE;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreBuildContactMenu(WPARAM wParam,LPARAM lParam);
int SettingChanged(WPARAM wParam,LPARAM lParam);
int MsgWindowEvent(WPARAM wParam, LPARAM lParam);

int EnableHistory(WPARAM wParam,LPARAM lParam);
int DisableHistory(WPARAM wParam,LPARAM lParam);
int HistoryEnabled(WPARAM wParam, LPARAM lParam);

BOOL ContactEnabled(HANDLE hContact);
BOOL ProtocolEnabled(const char *protocol);


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
	hSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);

	InitOptions();
	InitPopups();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	DeInitPopups();
	DeInitOptions();

	UnhookEvent(hModulesLoaded);
	UnhookEvent(hPreBuildCMenu);
	UnhookEvent(hSettingChanged);
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

		upd.szBetaVersionURL = "http://eth0.dk/files/pescuma/smh_version.txt";
		upd.szBetaChangelogURL = "http://eth0.dk/files/pescuma/smh_changelog.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"Status Message History ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://eth0.dk/files/pescuma/smhW.zip";
#else
		upd.szBetaUpdateURL = "http://eth0.dk/files/pescuma/smh.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}


	HookEvent(ME_MSG_WINDOWEVENT,&MsgWindowEvent);

	loaded = TRUE;

	return 0;
}


int PreBuildContactMenu(WPARAM wParam,LPARAM lParam) 
{
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof(clmi);

	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (!ProtocolEnabled(proto))
	{
		clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hDisableMenu, (LPARAM) &clmi);

		clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hEnableMenu, (LPARAM) &clmi);
	}
	else if (HistoryEnabled(wParam, 0))
	{
		clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hEnableMenu, (LPARAM) &clmi);

		clmi.flags = CMIM_FLAGS | CMIM_ICON;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hDisableMenu, (LPARAM) &clmi);
	}
	else
	{
		clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hDisableMenu, (LPARAM) &clmi);

		clmi.flags = CMIM_FLAGS | CMIM_ICON;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hEnableMenu, (LPARAM) &clmi);
	}

	return 0;
}


int EnableHistory(WPARAM wParam,LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;

	if (hContact != NULL)
		DBWriteContactSettingByte(hContact, MODULE_NAME, "Enabled", TRUE);

	return 0;
}


int DisableHistory(WPARAM wParam,LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;

	if (hContact != NULL)
		DBWriteContactSettingByte(hContact, MODULE_NAME, "Enabled", FALSE);

	return 0;
}


int HistoryEnabled(WPARAM wParam, LPARAM lParam) 
{
	return ContactEnabled((HANDLE) wParam);
}


BOOL AllowProtocol(const char *proto)
{	
	if ((CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGRECV) == 0)
		return FALSE;

	return TRUE;
}


BOOL ProtocolEnabled(const char *proto)
{
	if (proto == NULL)
		return FALSE;
		
	if (!AllowProtocol(proto))
		return FALSE;

	char setting[256];
	mir_snprintf(setting, sizeof(setting), "%sEnabled", proto);
	return (BOOL) DBGetContactSettingByte(NULL, MODULE_NAME, setting, TRUE);
}


BOOL ContactEnabled(HANDLE hContact) 
{
	if (hContact == NULL)
		return FALSE;

	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (!ProtocolEnabled(proto))
		return FALSE;

	BYTE def = TRUE;

	// Is a subcontact?
	if (ServiceExists(MS_MC_GETMETACONTACT)) 
	{
		HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

		if (hMetaContact != NULL)
			def = ContactEnabled(hMetaContact);
	}

	return DBGetContactSettingByte(hContact, MODULE_NAME, "Enabled", def);
}


// Returns true if the unicode buffer only contains 7-bit characters.
BOOL IsUnicodeAscii(const WCHAR * pBuffer, int nSize)
{
	BOOL bResult = TRUE;
	int nIndex;

	for (nIndex = 0; nIndex < nSize; nIndex++) {
		if (pBuffer[nIndex] > 0x7F) {
			bResult = FALSE;
			break;
		}
	}
	return bResult;
}


HANDLE HistoryLog(HANDLE hContact, TCHAR *log_text)
{
	if (log_text != NULL)
	{
		DBEVENTINFO event = { 0 };
		BYTE *tmp = NULL;

		event.cbSize = sizeof(event);

#ifdef UNICODE

		size_t needed = WideCharToMultiByte(CP_ACP, 0, log_text, -1, NULL, 0, NULL, NULL);
		size_t len = lstrlen(log_text);
		size_t size;

		if (opts.history_only_ansi_if_possible && IsUnicodeAscii(log_text, len))
			size = needed;
		else
			size = needed + (len + 1) * sizeof(WCHAR);

		tmp = (BYTE *) mir_alloc0(size);

		WideCharToMultiByte(CP_ACP, 0, log_text, -1, (char *) tmp, needed, NULL, NULL);

		if (size > needed)
			lstrcpyn((WCHAR *) &tmp[needed], log_text, len + 1);

		event.pBlob = tmp;
		event.cbBlob = size;

#else

		event.pBlob = (PBYTE) log_text;
		event.cbBlob = strlen(log_text) + 1;

#endif

		event.eventType = EVENTTYPE_STATUSMESSAGE_CHANGE;
		event.flags = DBEF_READ;
		event.timestamp = (DWORD) time(NULL);

		event.szModule = MODULE_NAME;
		
		// Is a subcontact?
		if (ServiceExists(MS_MC_GETMETACONTACT)) 
		{
			HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

			if (hMetaContact != NULL && ContactEnabled(hMetaContact))
				CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&event);
		}

		HANDLE ret = (HANDLE) CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&event);

		mir_free(tmp);

		return ret;
	}
	else
	{
		return NULL;
	}
}

void ReplaceChars(TCHAR *text) 
{
	TCHAR *p;
	while(p = _tcsstr(text, _T("\\n")))
	{
		p[0] = _T('\r');
		p[1] = _T('\n');
	}
}

void Notify(HANDLE hContact, TCHAR *text)
{
	if (text != NULL && text[0] == _T('\0'))
		text = NULL;

	if (!opts.track_changes && text != NULL)
		return;

	if (!opts.track_removes && text == NULL)
		return;

	// Replace template with status_message
	TCHAR templ[1024];
	lstrcpyn(templ, text == NULL ? opts.template_removed : opts.template_changed, MAX_REGS(templ));
	ReplaceChars(templ);

	TCHAR log[1024];
	mir_sntprintf(log, sizeof(log), templ, 
		text == NULL ? TranslateT("<no status message>") : text);

	if (opts.history_enable)
		HistoryLog(hContact, log);

	if (opts.popup_enable)
		ShowPopup(hContact, NULL, log);
}

int inline CheckStr(char *str, int not_empty, int empty)
{
	if (str == NULL || str[0] == '\0')
		return empty;
	else
		return not_empty;
}

#ifdef UNICODE

int inline CheckStr(TCHAR *str, int not_empty, int empty)
{
	if (str == NULL || str[0] == L'\0')
		return empty;
	else
		return not_empty;
}

#endif

// Return 0 if not changed, 1 if changed, 2 if removed
int TrackChange(HANDLE hContact, DBCONTACTWRITESETTING *cws_new, BOOL ignore_remove)
{
	char current_setting[256];
	mir_snprintf(current_setting, MAX_REGS(current_setting), "%sCurrent", cws_new->szSetting);

	int ret = 0;

	DBVARIANT dbv = {0};
#ifdef UNICODE
	BOOL found_current = (DBGetContactSettingW(hContact, cws_new->szModule, current_setting, &dbv) == 0);
#else
	BOOL found_current = (DBGetContactSetting(hContact, cws_new->szModule, current_setting, &dbv) == 0);
#endif
	if (!found_current)
	{
		// Current value does not exist

		if (cws_new->value.type == DBVT_DELETED)
		{
			ret = 0;
		}
		else if (cws_new->value.type == DBVT_ASCIIZ)
		{
			ret = CheckStr(cws_new->value.pszVal, 1, 0);
		}
#ifdef UNICODE
		else if (cws_new->value.type == DBVT_UTF8)
		{
			ret = CheckStr(cws_new->value.pszVal, 1, 0);
		}
		else if (cws_new->value.type == DBVT_WCHAR)
		{
			ret = CheckStr(cws_new->value.pwszVal, 1, 0);
		}
#endif
		else
		{
			ret = 1;
		}
	}
	else
	{
		// Current value exist

		if (cws_new->value.type == DBVT_DELETED)
		{
			if (dbv.type == DBVT_ASCIIZ)
			{
				ret = CheckStr(dbv.pszVal, 2, 0);
			}
#ifdef UNICODE
			else if (dbv.type == DBVT_UTF8)
			{
				ret = CheckStr(dbv.pszVal, 2, 0);
			}
			else if (dbv.type == DBVT_WCHAR)
			{
				ret = CheckStr(dbv.pwszVal, 2, 0);
			}
#endif
			else
			{
				ret = 2;
			}
		}
		else if (dbv.type != cws_new->value.type)
		{
#ifdef UNICODE
			if ( (cws_new->value.type == DBVT_UTF8 || cws_new->value.type == DBVT_ASCIIZ || cws_new->value.type == DBVT_WCHAR)
				&& (dbv.type == DBVT_UTF8 || dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_WCHAR))
			{
				WCHAR tmp_cws_new[1024] = L"";
				if (cws_new->value.type == DBVT_ASCIIZ)
					MultiByteToWideChar(CP_ACP, 0, cws_new->value.pszVal, -1, tmp_cws_new, MAX_REGS(tmp_cws_new));
				else if (cws_new->value.type == DBVT_UTF8)
					MultiByteToWideChar(CP_UTF8, 0, cws_new->value.pszVal, -1, tmp_cws_new, MAX_REGS(tmp_cws_new));
				else if (cws_new->value.type == DBVT_WCHAR)
					lstrcpyn(tmp_cws_new, cws_new->value.pwszVal, MAX_REGS(tmp_cws_new));

				WCHAR tmp_dbv[1024] = L"";
				if (dbv.type == DBVT_ASCIIZ)
					MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, tmp_dbv, MAX_REGS(tmp_dbv));
				else if (dbv.type == DBVT_UTF8)
					MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, tmp_dbv, MAX_REGS(tmp_dbv));
				else if (dbv.type == DBVT_WCHAR)
					lstrcpyn(tmp_dbv, dbv.pwszVal, MAX_REGS(tmp_dbv));

				ret = (lstrcmpW(tmp_cws_new, tmp_dbv) ? CheckStr(tmp_cws_new, 1, 2) : 0);
			}
			else
#endif
			{
				ret = 1;
			}
		}
		else if (dbv.type == DBVT_BYTE)
		{
			ret = (cws_new->value.bVal != dbv.bVal ? 1 : 0);
		}
		else if (dbv.type == DBVT_WORD)
		{
			ret = (cws_new->value.wVal != dbv.wVal ? 1 : 0);
		}
		else if (dbv.type == DBVT_DWORD)
		{
			ret = (cws_new->value.dVal != dbv.dVal ? 1 : 0);
		}
		else if (dbv.type == DBVT_ASCIIZ)
		{
			ret = (strcmp(cws_new->value.pszVal, dbv.pszVal) ? CheckStr(cws_new->value.pszVal, 1, 2) : 0);
		}
#ifdef UNICODE
		else if (dbv.type == DBVT_UTF8)
		{
			ret = (strcmp(cws_new->value.pszVal, dbv.pszVal) ? CheckStr(cws_new->value.pszVal, 1, 2) : 0);
		}
		else if (dbv.type == DBVT_WCHAR)
		{
			ret = (lstrcmp(cws_new->value.pwszVal, dbv.pwszVal) ? CheckStr(cws_new->value.pwszVal, 1, 2) : 0);
		}
#endif
	}

	if (ret == 1 || (ret == 2 && !ignore_remove))
	{
		// Copy current to old
		char old_setting[256];
		mir_snprintf(old_setting, MAX_REGS(old_setting), "%sOld", cws_new->szSetting);

		if (dbv.type == DBVT_DELETED)
		{
			DBDeleteContactSetting(hContact, cws_new->szModule, old_setting);
		}
		else
		{
			DBCONTACTWRITESETTING cws_old;
			cws_old.szModule = cws_new->szModule;
			cws_old.szSetting = old_setting;
			cws_old.value = dbv;
			CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws_old);
		}


		// Copy new to current
		if (cws_new->value.type == DBVT_DELETED)
		{
			DBDeleteContactSetting(hContact, cws_new->szModule, current_setting);
		}
		else
		{
			DBCONTACTWRITESETTING cws_old;
			cws_old.szModule = cws_new->szModule;
			cws_old.szSetting = current_setting;
			cws_old.value = cws_new->value;
			CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws_old);
		}
	}

	if (found_current)
		DBFreeVariant(&dbv);

	return ret;
}


int SettingChanged(WPARAM wParam,LPARAM lParam)
{
	if (!loaded)
		return 0;

	if (!opts.history_enable && !opts.popup_enable)
		return 0;

	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (!strcmp(cws->szModule, "CList") && !strcmp(cws->szSetting, "StatusMsg"))
	{
		char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (proto == NULL || (metacontacts_proto != NULL && !strcmp(proto, metacontacts_proto)))
			return 0;
	
		if (opts.track_only_not_offline)
		{
			if (DBGetContactSettingWord(hContact, proto, "Status", 0) <= ID_STATUS_OFFLINE)
				return 0;
		}

		if (!ContactEnabled(hContact))
			return 0;

		int changed = TrackChange(hContact, cws, !opts.track_removes);
		if (changed == 0)
			return 0;

		if (changed == 2)
		{
			Notify(hContact, NULL);
		}
		else // changed == 1
#ifdef UNICODE
		if (cws->value.type == DBVT_ASCIIZ)
		{
			WCHAR tmp[1024] = L"";
			MultiByteToWideChar(CP_ACP, 0, cws->value.pszVal, -1, tmp, MAX_REGS(tmp));
			Notify(hContact, tmp);
		}
		else if (cws->value.type == DBVT_UTF8)
		{
			WCHAR tmp[1024] = L"";
			MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, tmp, MAX_REGS(tmp));
			Notify(hContact, tmp);
		}
		else if (cws->value.type == DBVT_WCHAR)
		{
			Notify(hContact, cws->value.pwszVal);
		}
#else
		if (cws->value.type == DBVT_ASCIIZ)
		{
			Notify(hContact, cws->value.pszVal);
		}
#endif
	}

	return 0;
}



#define TIMER_ID 17982
WNDPROC oldEditProc = NULL;
int txtlen = 0;
Hunspell *checker;

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
	SendMessage(hRichEdit,EM_GETCHARFORMAT,TRUE,(LPARAM)&CharFormat);
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


BOOL IsAlpha(TCHAR c)
{
#ifdef UNICODE
	return iswalpha(c) 
		|| (c == L'-' && !checker->get_forbidden_compound());
#else
	return (!_istcntrl(c) && !_istdigit(c) && !_istpunct(c) && !_istspace(c)) 
		|| (c == '-'  && !checker->get_forbidden_compound());
#endif
}

BOOL changed = TRUE;
LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_PASTE:
		case WM_CHAR:
		{
			changed = TRUE;
			break;
		}

		case WM_TIMER:
		{
			if (wParam != TIMER_ID)
				break;

			int len = GetWindowTextLength(hwnd);
			if (len <= 0)
				break;

			if (len == txtlen && !changed)
				break;

			txtlen = len;
			changed = FALSE;

			OutputDebugString(" **** HERE\n");

			SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

			SetAttributes(hwnd, CFM_UNDERLINE | CFM_UNDERLINETYPE, 0, 0);

			// Get text
			TCHAR *line_text = (TCHAR *) malloc((len + 1) * sizeof(TCHAR));
			GetWindowText(hwnd, line_text, len+1);

			// Now lets get the words
			int last_pos = -1;
			for (int pos = 0; pos < len; pos++)
			{
				if (!IsAlpha(line_text[pos]))
				{
					if (last_pos != -1)
					{
						// We found a word
						TCHAR old = line_text[pos];
						line_text[pos] = _T('\0');

						// TODO: Convert to UTF8
						BOOL right = checker->spell(&line_text[last_pos]);

						line_text[pos] = old;

						if (!right)
						{
							SetAttributes(hwnd, last_pos, pos, 
											CFM_UNDERLINETYPE, 0, CFU_UNDERLINEWAVE | 0x50);
						}

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
				// We found a word
				TCHAR old = line_text[pos];
				line_text[pos] = _T('\0');

				// TODO: Convert to UTF8
				BOOL right = checker->spell(&line_text[last_pos]);

				line_text[pos] = old;

				if (!right)
				{
					SetAttributes(hwnd, last_pos, pos, 
									CFM_UNDERLINETYPE, 0, CFU_UNDERLINEWAVE | 0x50);
				}

				last_pos = -1;
			}

			SetAttributes(hwnd, len, len, CFM_UNDERLINE | CFM_UNDERLINETYPE, 0, 0);

			free(line_text);

			SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
			InvalidateRect(hwnd, NULL, FALSE);

			break;
		}

		case WM_CONTEXTMENU:
		{
			int len = GetWindowTextLength(hwnd);
			if (len <= 0)
				break;

			// Get text
			TCHAR *line_text = (TCHAR *) malloc((len + 1) * sizeof(TCHAR));
			GetWindowText(hwnd, line_text, len+1);

			// Get cursor pos
			POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            ScreenToClient(hwnd, &pt);

            CHARRANGE sel;
			sel.cpMin = sel.cpMax = LOWORD(SendMessage(hwnd, EM_CHARFROMPOS, 0, (LPARAM) &pt));

			// Find the word
			while (sel.cpMin >= 0 && IsAlpha(line_text[sel.cpMin]))
				sel.cpMin--;
			sel.cpMin++;

			while (IsAlpha(line_text[sel.cpMax]))
				sel.cpMax++;
			line_text[sel.cpMax] = _T('\0');

			// Get suggestions
			char ** suggestions;
			int num_suggestions = checker->suggest(&suggestions, &line_text[sel.cpMin]);

			free(line_text);

			// Make menu
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT));
			HMENU hSubMenu = GetSubMenu(hMenu, 0);
            CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);

			if (num_suggestions > 0)
			{
				InsertMenu(hSubMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
				for (int i = num_suggestions - 1; i >= 0; i--) 
				{
					InsertMenu(hSubMenu, 0, MF_BYPOSITION, i + 1, suggestions[i]);
				}
			}

			// Show menu
			int opt = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
			if (opt > 0 && opt <= num_suggestions)
			{
				opt--;

				// Get old selecton
				CHARRANGE old_sel;
				SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &old_sel);

				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) &sel);
				SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM) suggestions[opt]);

				// Fix old sel
				int dif = lstrlen(suggestions[opt]) - sel.cpMax + sel.cpMin;
				if (old_sel.cpMin >= sel.cpMax)
					old_sel.cpMin += dif;
				if (old_sel.cpMax >= sel.cpMax)
					old_sel.cpMax += dif;

				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) &old_sel);

				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				InvalidateRect(hwnd, NULL, FALSE);
			}

			DestroyMenu(hMenu);

			if (num_suggestions > 0)
			{
				for (int i = num_suggestions - 1; i >= 0; i--) 
					free(suggestions[i]);
				free(suggestions);
			}

			break;
		}
	}

	return CallWindowProc(oldEditProc, hwnd, msg, wParam, lParam);
}


int AddContactTextBox(HANDLE hContact, HWND hwnd) 
{
	if (oldEditProc == NULL)
	{
		oldEditProc = (WNDPROC) SetWindowLong(hwnd, GWL_WNDPROC, (LONG) EditProc);

		checker = new Hunspell("C:/Programas2/VC/Miranda/bin/debug/diticonaries/pt_BR.aff", "C:/Programas2/VC/Miranda/bin/debug/diticonaries/pt_BR.dic");

		SetTimer(hwnd, TIMER_ID, 1000, NULL);
	}

	return 0;
}

int RemoveContactTextBox(HANDLE hContact, HWND hwnd) 
{
	if (oldEditProc != NULL) {
		SetWindowLong(hwnd, GWL_WNDPROC, (LONG) oldEditProc);
		oldEditProc = NULL;

		delete checker;

		KillTimer(hwnd, TIMER_ID);
	}

	return 0;
}

		
// TABSRMM
#define IDC_MESSAGE                     1002

int MsgWindowEvent(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;
	if (event == NULL)
		return 0;

	HWND hwnd = GetDlgItem(event->hwndWindow, IDC_MESSAGE);
	if (hwnd == NULL)
		return 0;

	if (event->uType == MSG_WINDOW_EVT_OPEN)
	{
		AddContactTextBox(event->hContact, hwnd);
	}
	else if (event->uType == MSG_WINDOW_EVT_CLOSING)
	{
		RemoveContactTextBox(event->hContact, hwnd);
	}

	return 0;
}