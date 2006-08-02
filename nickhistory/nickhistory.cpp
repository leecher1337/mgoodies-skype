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
	"Nick History (Unicode)",
#else
	"Nick History",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,1),
	"Log nickname changes to history",
	"Ricardo Pescuma Domenecci",
	"",
	"",
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
int HistoryEnabled(HANDLE hContact);


#define DEFAULT_TEMPLATE "changed his/her nickname to %s"


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

	CreateServiceFunction(MS_NICKHISTORY_DISABLE, DisableHistory);
	CreateServiceFunction(MS_NICKHISTORY_ENABLE, EnableHistory);
	CreateServiceFunction(MS_NICKHISTORY_ENABLED, HistoryEnabled);

	// Add menu item to enable/disable status message check
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_NOTOFFLIST;
	mi.position = 1000100010;

	mi.pszName = Translate("Don't log Nickname changes");
	mi.pszService = MS_NICKHISTORY_DISABLE;
	hDisableMenu = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	
	mi.pszName = Translate("Log Nickname changes");
	mi.pszService = MS_NICKHISTORY_ENABLE;
	hEnableMenu = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	
	// hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hPreBuildCMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);
	hSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hPreBuildCMenu);
	UnhookEvent(hSettingChanged);
	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	init_mir_malloc();

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

		upd.szBetaVersionURL = "http://br.geocities.com/ricardo_pescuma/nickhistory_version.txt";
		upd.szBetaChangelogURL = "http://br.geocities.com/ricardo_pescuma/nickhistory_changelog.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"Nick History ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
		upd.szBetaUpdateURL = "http://br.geocities.com/ricardo_pescuma/nickhistory.zip";

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	loaded = TRUE;

	return 0;
}


int PreBuildContactMenu(WPARAM wParam,LPARAM lParam) 
{
	// See what to show

	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof(clmi);

	if (HistoryEnabled(wParam, lParam))
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

int HistoryEnabled(HANDLE hContact) 
{
	if (hContact != NULL)
		return DBGetContactSettingByte(hContact, MODULE_NAME, "Enabled", TRUE);
	else
		return FALSE;
}

int HistoryEnabled(WPARAM wParam, LPARAM lParam) 
{
	return HistoryEnabled((HANDLE) wParam);
}


HANDLE HistoryLog(HANDLE hContact, char *log_text)
{
	if (log_text != NULL)
	{
		DBEVENTINFO event = { 0 };;

		event.cbSize = sizeof(event);

		event.pBlob = (PBYTE) log_text;
		event.cbBlob = strlen(log_text) + 1;

		event.eventType = EVENTTYPE_NICKNAME_CHANGE;
		event.flags = DBEF_READ;
		event.timestamp = (DWORD) time(NULL);

		event.szModule = MODULE_NAME;
		
		// Is a subcontact?
		if (ServiceExists(MS_MC_GETMETACONTACT)) 
		{
			HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

			if (hMetaContact != NULL)
				CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&event);
		}

		return (HANDLE) CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&event);
	}
	else
	{
		return NULL;
	}
}

#ifdef UNICODE

HANDLE HistoryLog(HANDLE hContact, wchar_t *log_text)
{
	if (log_text != NULL)
	{
		DBEVENTINFO event = { 0 };;

		event.cbSize = sizeof(event);

		size_t size = lstrlenW(log_text) + 1;
		BYTE *tmp = (BYTE *) mir_alloc0(size * 3);

		WideCharToMultiByte(CP_ACP, 0, log_text, -1, (char *) tmp, size, NULL, NULL);

		lstrcpynW((WCHAR *) &tmp[size], log_text, size);

		event.pBlob = tmp;
		event.cbBlob = size * 3;

		event.eventType = EVENTTYPE_NICKNAME_CHANGE;
		event.flags = DBEF_READ;
		event.timestamp = (DWORD) time(NULL);

		event.szModule = MODULE_NAME;
		
		// Is a subcontact?
		if (ServiceExists(MS_MC_GETMETACONTACT)) 
		{
			HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

			if (hMetaContact != NULL)
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

#endif


void AddToHistory(HANDLE hContact, char *nickname)
{
	char templ[1024] = "";

	// Get template
	DBVARIANT dbv;
	if (!DBGetContactSetting(hContact, MODULE_NAME, "HistoryTemplate", &dbv))
	{
		if (dbv.type == DBVT_ASCIIZ && dbv.pszVal != NULL && dbv.pszVal[0] != _T('\0'))
			strncpy(templ, dbv.pszVal, sizeof(templ));
		else
			strncpy(templ, DEFAULT_TEMPLATE, sizeof(templ));

		DBFreeVariant(&dbv);
	}
	else
	{
		strncpy(templ, DEFAULT_TEMPLATE, sizeof(templ));
	}

	// Replace template with nick
	char log[1024] = "";
	mir_snprintf(log, sizeof(log), templ, nickname);

	HistoryLog(hContact, log);
}

#ifdef UNICODE

void AddToHistory(HANDLE hContact, wchar_t *nickname)
{
	char templ[1024] = "";
	wchar_t wtempl[1024] = L"";

	// Get template
	DBVARIANT dbv;
	if (!DBGetContactSetting(hContact, MODULE_NAME, "HistoryTemplate", &dbv))
	{
		if (dbv.type == DBVT_ASCIIZ && dbv.pszVal != NULL && dbv.pszVal[0] != _T('\0'))
			strncpy(templ, dbv.pszVal, sizeof(templ));
		else
			strncpy(templ, DEFAULT_TEMPLATE, sizeof(templ));

		DBFreeVariant(&dbv);
	}
	else
	{
		strncpy(templ, DEFAULT_TEMPLATE, sizeof(templ));
	}

	MultiByteToWideChar(CP_ACP, 0, templ, -1, wtempl, MAX_REGS(wtempl));

	// Replace template with nick
	wchar_t log[1024] = L"";
	mir_sntprintf(log, sizeof(log), wtempl, nickname);

	HistoryLog(hContact, log);
}

void AddToHistoryConvert(HANDLE hContact, char *nickname)
{
	wchar_t nick[1024];

	MultiByteToWideChar(CP_UTF8, 0, nickname, -1, nick, MAX_REGS(nick));

	AddToHistory(hContact, nick);
}

#endif

// Return TRUE if changed
BOOL TrackChange(HANDLE hContact, DBCONTACTWRITESETTING *cws)
{
	char old_setting[256];
	mir_snprintf(old_setting, MAX_REGS(old_setting), "%sOld", cws->szSetting);

	BOOL ret = FALSE;

	DBVARIANT dbv = {0};
	if (DBGetContactSetting(hContact, cws->szModule, old_setting, &dbv))
	{
		ret = TRUE;
	}
	else
	{
		if (dbv.type != cws->value.type)
		{

#ifdef UNICODE

			if ( (cws->value.type == DBVT_UTF8 || cws->value.type == DBVT_ASCIIZ || cws->value.type == DBVT_WCHAR)
				&& (dbv.type == DBVT_UTF8 || dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_WCHAR))
			{
				wchar_t tmp_cws[1024];
				if (cws->value.type == DBVT_ASCIIZ)
					MultiByteToWideChar(CP_ACP, 0, cws->value.pszVal, -1, tmp_cws, MAX_REGS(tmp_cws));
				else if (cws->value.type == DBVT_UTF8)
					MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, tmp_cws, MAX_REGS(tmp_cws));
				else if (cws->value.type == DBVT_WCHAR)
					lstrcpynW(tmp_cws, cws->value.pwszVal, MAX_REGS(tmp_cws));

				wchar_t tmp_dbv[1024];
				if (dbv.type == DBVT_ASCIIZ)
					MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, tmp_dbv, MAX_REGS(tmp_dbv));
				else if (dbv.type == DBVT_UTF8)
					MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, tmp_dbv, MAX_REGS(tmp_dbv));
				else if (dbv.type == DBVT_WCHAR)
					lstrcpynW(tmp_dbv, dbv.pwszVal, MAX_REGS(tmp_dbv));

				ret = lstrcmpW(tmp_cws, tmp_dbv);
			}
			else

#endif

				ret = TRUE;
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

		else if (dbv.type == DBVT_WCHAR)
		{
			ret = lstrcmpW(cws->value.pwszVal, dbv.pwszVal);
		}

#endif

		DBFreeVariant(&dbv);
	}

	if (ret)
	{
		DBCONTACTWRITESETTING cws_old;
		memmove(&cws_old, cws, sizeof(cws_old));
		cws_old.szSetting = old_setting;
		CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws_old);
	}

	return ret;
}

int SettingChanged(WPARAM wParam,LPARAM lParam)
{
	if (!loaded)
		return 0;

	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (proto == NULL || (metacontacts_proto != NULL && !strcmp(proto, metacontacts_proto)))
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (!strcmp(cws->szModule, proto)  && !strcmp(cws->szSetting, "Nick"))
	{
		if (!HistoryEnabled(hContact))
			return 0;

		if (!TrackChange(hContact, cws))
			return 0;

		if (cws->value.type == DBVT_ASCIIZ)
		{
			AddToHistory(hContact, cws->value.pszVal);
		}
#ifdef UNICODE
		else if (cws->value.type == DBVT_UTF8)
		{
			AddToHistoryConvert(hContact, cws->value.pszVal);
		}
		else if (cws->value.type == DBVT_WCHAR)
		{
			AddToHistory(hContact, cws->value.pwszVal);
		}
#endif
	}

	return 0;
}

