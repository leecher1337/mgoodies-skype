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
	"Status Message History (Unicode)",
#else
	"Status Message History",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,3),
	"Log status message changes to history",
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

	CreateServiceFunction(MS_SMH_DISABLE, DisableHistory);
	CreateServiceFunction(MS_SMH_ENABLE, EnableHistory);
	CreateServiceFunction(MS_SMH_ENABLED, HistoryEnabled);

	// Add menu item to enable/disable status message check
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_NOTOFFLIST;
	mi.position = 1000100010;

	mi.pszName = Translate("Don't log Status Message changes");
	mi.pszService = MS_SMH_DISABLE;
	hDisableMenu = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	
	mi.pszName = Translate("Log Status Message changes");
	mi.pszService = MS_SMH_ENABLE;
	hEnableMenu = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	
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

		upd.szBetaVersionURL = "http://br.geocities.com/ricardo_pescuma/smh_version.txt";
		upd.szBetaChangelogURL = "http://br.geocities.com/ricardo_pescuma/smh_changelog.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"Status Message History ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
		upd.szBetaUpdateURL = "http://br.geocities.com/ricardo_pescuma/smh.zip";

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

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


void Notify(HANDLE hContact, TCHAR *text)
{
	if (text != NULL && text[0] == _T('\0'))
		text = NULL;

	if (!opts.track_changes && text != NULL)
		return;

	if (!opts.track_removes && text == NULL)
		return;

	// Replace template with status_message
	TCHAR log[1024];
	mir_sntprintf(log, sizeof(log), 
		text == NULL ? opts.template_removed : opts.template_changed, 
		text == NULL ? TranslateT("<no status message>") : text);

	if (opts.history_enable)
		HistoryLog(hContact, log);

	if (opts.popup_enable)
		ShowPopup(hContact, NULL, log);
}


// Return TRUE if changed
BOOL TrackChange(HANDLE hContact, DBCONTACTWRITESETTING *cws)
{
	char old_setting[256];
	mir_snprintf(old_setting, MAX_REGS(old_setting), "%sOld", cws->szSetting);

	BOOL ret = FALSE;

	DBVARIANT dbv = {0};
	if (DBGetContactSetting(hContact, cws->szModule, old_setting, &dbv))
	{
		// Old value does not exist

		if (cws->value.type == DBVT_DELETED)
		{
			ret = FALSE;
		}
		else if (cws->value.type == DBVT_ASCIIZ)
		{
			ret = (cws->value.pszVal[0] != '\0');
		}
#ifdef UNICODE
		else if (cws->value.type == DBVT_UTF8)
		{
			ret = (cws->value.pszVal[0] != '\0');
		}
		else if (cws->value.type == DBVT_WCHAR)
		{
			ret = (cws->value.pwszVal[0] != L'\0');
		}
#endif
		else
		{
			ret = TRUE;
		}
	}
	else
	{
		// Old value exist

		if (dbv.type != cws->value.type)
		{
			if (cws->value.type == DBVT_DELETED && dbv.type == DBVT_ASCIIZ)
			{
				ret = (cws->value.pszVal[0] != '\0');
			}
#ifdef UNICODE
			else if (cws->value.type == DBVT_DELETED && dbv.type == DBVT_UTF8)
			{
				ret = (cws->value.pszVal[0] != '\0');
			}
			else if (cws->value.type == DBVT_DELETED && dbv.type == DBVT_WCHAR)
			{
				ret = (cws->value.pwszVal[0] != L'\0');
			}
			else if ( (cws->value.type == DBVT_UTF8 || cws->value.type == DBVT_ASCIIZ || cws->value.type == DBVT_WCHAR)
				&& (dbv.type == DBVT_UTF8 || dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_WCHAR))
			{
				WCHAR tmp_cws[1024] = L"";
				if (cws->value.type == DBVT_ASCIIZ)
					MultiByteToWideChar(CP_ACP, 0, cws->value.pszVal, -1, tmp_cws, MAX_REGS(tmp_cws));
				else if (cws->value.type == DBVT_UTF8)
					MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, tmp_cws, MAX_REGS(tmp_cws));
				else if (cws->value.type == DBVT_WCHAR)
					lstrcpyn(tmp_cws, cws->value.pwszVal, MAX_REGS(tmp_cws));

				WCHAR tmp_dbv[1024] = L"";
				if (dbv.type == DBVT_ASCIIZ)
					MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, tmp_dbv, MAX_REGS(tmp_dbv));
				else if (dbv.type == DBVT_UTF8)
					MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, tmp_dbv, MAX_REGS(tmp_dbv));
				else if (dbv.type == DBVT_WCHAR)
					lstrcpyn(tmp_dbv, dbv.pwszVal, MAX_REGS(tmp_dbv));

				ret = lstrcmpW(tmp_cws, tmp_dbv);
			}
#endif
			else
			{
				ret = TRUE;
			}
		}
		else if (dbv.type == DBVT_BYTE)
		{
			ret = (cws->value.bVal != dbv.bVal);
		}
		else if (dbv.type == DBVT_WORD)
		{
			ret = (cws->value.wVal != dbv.wVal);
		}
		else if (dbv.type == DBVT_DWORD)
		{
			ret = (cws->value.dVal != dbv.dVal);
		}
		else if (dbv.type == DBVT_ASCIIZ)
		{
			ret = strcmp(cws->value.pszVal, dbv.pszVal);
		}
#ifdef UNICODE
		else if (dbv.type == DBVT_UTF8)
		{
			ret = strcmp(cws->value.pszVal, dbv.pszVal);
		}
		else if (dbv.type == DBVT_WCHAR)
		{
			ret = lstrcmp(cws->value.pwszVal, dbv.pwszVal);
		}
#endif

		DBFreeVariant(&dbv);
	}

	if (ret)
	{
		if (cws->value.type == DBVT_DELETED)
		{
			DBDeleteContactSetting(hContact, cws->szModule, old_setting);
		}
		else
		{
			DBCONTACTWRITESETTING cws_old;
			memmove(&cws_old, cws, sizeof(cws_old));
			cws_old.szSetting = old_setting;
			CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws_old);
		}
	}

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

		if (!TrackChange(hContact, cws))
			return 0;

#ifdef UNICODE
		if (cws->value.type == DBVT_ASCIIZ)
		{
			if (cws->value.pszVal != NULL)
			{
				WCHAR tmp[1024] = L"";
				MultiByteToWideChar(CP_ACP, 0, cws->value.pszVal, -1, tmp, MAX_REGS(tmp));
				Notify(hContact, tmp);
			}
			else
			{
				Notify(hContact, NULL);
			}
		}
		else if (cws->value.type == DBVT_UTF8)
		{
			if (cws->value.pszVal != NULL)
			{
				WCHAR tmp[1024] = L"";
				MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, tmp, MAX_REGS(tmp));
				Notify(hContact, tmp);
			}
			else
			{
				Notify(hContact, NULL);
			}
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
		else if (cws->value.type == DBVT_DELETED)
		{
			Notify(hContact, NULL);
		}
	}

	return 0;
}

