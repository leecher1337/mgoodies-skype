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
	"History Keeper (Unicode)",
#else
	"History Keeper",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,7),
	"Log various types of events to history",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2007 Ricardo Pescuma Domenecci",
	"http://pescuma.mirandaim.ru/miranda/historykeeper",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
#ifdef UNICODE
	{ 0xca52cf41, 0x12c2, 0x411b, { 0xb8, 0x0, 0xd2, 0xbd, 0x95, 0x9b, 0xe0, 0x99 } } // {CA52CF41-12C2-411b-B800-D2BD959BE099}
#else
	{ 0x5f33a404, 0x351f, 0x440b, { 0xa7, 0xdb, 0xb9, 0xb5, 0xd3, 0xeb, 0x2b, 0xbd } } // {5F33A404-351F-440b-A7DB-B9B5D3EB2BBD}
#endif
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hHooks[6] = {0};

HANDLE hEnableMenu[NUM_TYPES]; 
HANDLE hDisableMenu[NUM_TYPES]; 

//char *metacontacts_proto = NULL;
BOOL loaded = FALSE;
ContactAsyncQueue *queue;

BOOL PER_DEFAULT_LOG_SUBCONTACTS = FALSE;
BOOL PER_DEFAULT_NOTIFY_SUBCONTACTS = FALSE;
BOOL HAS_METACONTACTS = FALSE;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreBuildContactMenu(WPARAM wParam,LPARAM lParam);
int SettingChanged(WPARAM wParam,LPARAM lParam);
int ContactAdded(WPARAM wParam,LPARAM lParam);
int ProtoAckHook(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);

int EnableHistory(WPARAM wParam, LPARAM lParam, LPARAM type);
int DisableHistory(WPARAM wParam, LPARAM lParam, LPARAM type);
int HistoryEnabled(WPARAM wParam, LPARAM lParam, LPARAM type);

BOOL ContactEnabled(int type, HANDLE hContact);
BOOL ProtocolEnabled(int type, const char *protocol);

void Process(HANDLE hContact, void *param);

#define MODULE_PROTOCOL ((char *)-1)

char *item_names[NUM_ITEMS] = {
	"History",
	"File",
	"Popup",
	"Sound",
	"Speak",
};

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


static const MUUID interfaces[] = { MIID_STATUS_MESSAGE_CHANGE_LOGGER, MIID_STATUS_MESSAGE_CHANGE_NOTIFIER, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link) 
{
	pluginLink = link;

	init_mir_malloc();
	init_list_interface();

	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM) MODULE_NAME "/CreationTickCount");

	PROTOCOLDESCRIPTOR **protos;
	int i, count;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);
	char tmp[256];
	for (i = 0; i < count; i++)
	{
		if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0')
			continue;

		mir_snprintf(tmp, MAX_REGS(tmp), MODULE_NAME "/%s_OnOfflineTickCount", protos[i]->szName);
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM) tmp);

		mir_snprintf(tmp, MAX_REGS(tmp), MODULE_NAME "/%s_LastStatus", protos[i]->szName);
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM) tmp);

		for (int j = 0; j < NUM_TYPES; j++) 
		{
			HISTORY_TYPE &type = types[j];

			if (type.temporary && type.track.db.module == MODULE_PROTOCOL)
			{
				mir_snprintf(tmp, MAX_REGS(tmp), MODULE_NAME "/%s_%s_Current", protos[i]->szName, type.track.db.setting);
				CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM) tmp);
			}
		}
	}

	// Add menu item to enable/disable status message check
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.popupPosition = mi.position = 1000090020;

	mi.flags = CMIF_NOTOFFLIST|CMIF_ROOTPOPUP;
	mi.pszPopupName = (char *)-1;
	mi.pszName = "History Keeper";
	HANDLE hRootMenu = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	mi.pszPopupName = (char *) hRootMenu;
	mi.flags = CMIF_NOTOFFLIST|CMIF_CHILDPOPUP;
	char name[256];
	mi.pszName = name;
	char service[256];
	mi.pszService = service;

	for (i = 0; i < NUM_TYPES; i++) 
	{
		HISTORY_TYPE &type = types[i];

		// DB

		if (type.temporary && type.track.db.module != MODULE_PROTOCOL)
		{
			mir_snprintf(tmp, MAX_REGS(tmp), MODULE_NAME "/%s_%s_Current", type.track.db.module, type.track.db.setting);
			CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM) tmp);
		}


		// History

		char tmp[128];
		strncpy(tmp, type.description, MAX_REGS(tmp));
		CharLowerA(tmp);

		char change[256];
		mir_snprintf(change, MAX_REGS(change), "Change\nchanged his/her %s to %%new%%\n%%old%%\tOld value\n%%new%%\tNew value",
			tmp);

		if (type.canBeRemoved)
		{
			char remove[256];
			mir_snprintf(remove, MAX_REGS(remove), "Removal\nremoved his/her %s\n%%old%%\tOld value",
				tmp);

			char *templates[] = { change, remove };

			char desc[128];
			mir_snprintf(desc, MAX_REGS(desc), "%s Change", type.description);

			HICON hIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(type.icon), IMAGE_ICON, 16, 16, 0);
			HistoryEvents_RegisterMessageStyle(MODULE_NAME, type.name, desc, type.eventType, 
				hIcon, HISTORYEVENTS_FLAG_SHOW_IM_SRMM | HISTORYEVENTS_FLAG_EXPECT_CONTACT_NAME_BEFORE | type.historyFlags,
				templates, MAX_REGS(templates));
			DestroyIcon(hIcon);
		}
		else
		{
			char *templates[] = { change };

			char desc[128];
			mir_snprintf(desc, MAX_REGS(desc), "%s Change", type.description);

			HICON hIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(type.icon), IMAGE_ICON, 16, 16, 0);
			HistoryEvents_RegisterMessageStyle(MODULE_NAME, type.name, desc, type.eventType, 
				hIcon, HISTORYEVENTS_FLAG_SHOW_IM_SRMM | HISTORYEVENTS_FLAG_EXPECT_CONTACT_NAME_BEFORE | type.historyFlags,
				templates, MAX_REGS(templates));
			DestroyIcon(hIcon);
		}


		// Menus and services

		mi.position = i;

		mi.hIcon = HistoryEvents_GetIcon(type.eventType);

		mir_snprintf(service, MAX_REGS(service), "%s/Disable", type.name);
		CreateServiceFunctionParam(service, DisableHistory, i);

		mir_snprintf(name, MAX_REGS(name), "Ignore %s changes", type.description);
		hDisableMenu[i] = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

		mir_snprintf(service, MAX_REGS(service), "%s/Enable", type.name);
		CreateServiceFunctionParam(service, EnableHistory, i);
		
		mir_snprintf(name, MAX_REGS(name), "Log %s changes", type.description);
		hEnableMenu[i] = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

		mir_snprintf(service, MAX_REGS(service), "%s/Enabled", type.name);
		CreateServiceFunctionParam(service, HistoryEnabled, i);

		HistoryEvents_ReleaseIcon(mi.hIcon);

		// Sounds

		mir_snprintf(change, MAX_REGS(change), Translate("%s change"), type.description);
		SkinAddNewSoundEx(change, "Notifications", change);

		if (type.canBeRemoved)
		{
			mir_snprintf(change, MAX_REGS(change), Translate("%s removal"), type.description);
			SkinAddNewSoundEx(change, "Notifications", change);
		}
	}
	
	// hooks
	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hHooks[1] = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);
	hHooks[2] = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);
	hHooks[3] = HookEvent(ME_DB_CONTACT_ADDED, ContactAdded);
	hHooks[4] = HookEvent(ME_PROTO_ACK, ProtoAckHook);
	hHooks[5] = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	// Hidden settings
	PER_DEFAULT_LOG_SUBCONTACTS = DBGetContactSettingByte(NULL, MODULE_NAME, "PerDefaultLogSubcontacts", FALSE);
	PER_DEFAULT_NOTIFY_SUBCONTACTS = DBGetContactSettingByte(NULL, MODULE_NAME, "PerDefaultNotifySubcontacts", FALSE);
	HAS_METACONTACTS = ServiceExists(MS_MC_GETMETACONTACT);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
//	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
//		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

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

		upd.szBetaVersionURL = "http://pescuma.mirandaim.ru/miranda/historykeeper_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.mirandaim.ru/miranda/historykeeper#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"History Keeper ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/historykeeperW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/historykeeper.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	InitOptions();
	InitPopups();

	queue = new ContactAsyncQueue(&Process);

	loaded = TRUE;

	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	delete queue;

	DeInitPopups();
	DeInitOptions();

	for (int i = 0; i < MAX_REGS(hHooks); i++)
		if (hHooks[i] != NULL)
			UnhookEvent(hHooks[i]);

	return 0;
}


int PreBuildContactMenu(WPARAM wParam, LPARAM lParam) 
{
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof(clmi);

	HANDLE hContact = (HANDLE) wParam;
	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);

	for (int i = 0; i < NUM_TYPES; i++) 
	{
		if (!ProtocolEnabled(i, proto))
		{
			clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hEnableMenu[i], (LPARAM) &clmi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hDisableMenu[i], (LPARAM) &clmi);
		}
		else if (ContactEnabled(i, hContact))
		{
			clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hEnableMenu[i], (LPARAM) &clmi);

			clmi.flags = CMIM_FLAGS;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hDisableMenu[i], (LPARAM) &clmi);
		}
		else
		{
			clmi.flags = CMIM_FLAGS;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hEnableMenu[i], (LPARAM) &clmi);

			clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hDisableMenu[i], (LPARAM) &clmi);
		}
	}

	return 0;
}


int EnableHistory(WPARAM wParam, LPARAM lParam, LPARAM type) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	for (int i = 0; i <= NUM_ITEMS; i++)
		EnableItem(type, hContact, i, TRUE);

	return 0;
}

int DisableHistory(WPARAM wParam, LPARAM lParam, LPARAM type) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	for (int i = 0; i <= NUM_ITEMS; i++)
		EnableItem(type, hContact, i, FALSE);

	return 0;
}


int HistoryEnabled(WPARAM wParam, LPARAM lParam, LPARAM type) 
{
	return ContactEnabled(type, (HANDLE) wParam);
}


BOOL AllowProtocol(int type, const char *proto)
{	
	if (types[type].fAllowProtocol != NULL && !types[type].fAllowProtocol(proto))
		return FALSE;

	return TRUE;
}


BOOL ProtocolEnabled(int type, const char *proto)
{
	if (proto == NULL)
		return FALSE;
		
	if (!AllowProtocol(type, proto))
		return FALSE;

	char setting[256];
	mir_snprintf(setting, sizeof(setting), "%s_%sEnabled", types[type].name, proto);
	return (BOOL) DBGetContactSettingByte(NULL, MODULE_NAME, setting, TRUE);
}


BOOL ContactEnabled(int type, HANDLE hContact) 
{
	if (hContact == NULL)
		return FALSE;

	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (!ProtocolEnabled(type, proto))
		return FALSE;

	for (int i = 0; i <= NUM_ITEMS; i++)
	{
		if (ItemEnabled(type, hContact, i))
			return TRUE;
	}
	return FALSE;
}


BOOL ItemEnabled(int type, HANDLE hContact, int item)
{
	BYTE def = TRUE;

	// Is a subcontact?
	if (HAS_METACONTACTS) 
	{
		HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);
		if (hMetaContact != NULL)
		{
			def = FALSE;

			if (item < NUM_LOG_ITEMS)
			{
				if (PER_DEFAULT_LOG_SUBCONTACTS)
					def = ItemEnabled(type, hMetaContact, item);
			}
			else
			{
				if (PER_DEFAULT_NOTIFY_SUBCONTACTS)
					def = ItemEnabled(type, hMetaContact, item);
			}
		}
	}

	char setting[256];
	mir_snprintf(setting, MAX_REGS(setting), "%s_%s_Enabled", types[type].name, item_names[item]);
	return DBGetContactSettingByte(hContact, MODULE_NAME, setting, def);
}


BOOL EnableItem(int type, HANDLE hContact, int item, BOOL enable)
{
	char setting[256];
	mir_snprintf(setting, MAX_REGS(setting), "%s_%s_Enabled", types[type].name, item_names[item]);
	return DBWriteContactSettingByte(hContact, MODULE_NAME, setting, enable);
}


void LogToFile(HANDLE hContact, int typeNum, int templateNum, TCHAR **vars, int numVars)
{
	if (templateNum == 0 && !opts[typeNum].file_track_changes)
		return;
	if (templateNum == 1 && !opts[typeNum].file_track_removes)
		return;

	TCHAR *templ = (templateNum == 1 ? opts[typeNum].file_template_removed : opts[typeNum].file_template_changed);
	Buffer<TCHAR> txt;
	ReplaceTemplate(&txt, hContact, templ, vars, numVars);

	// Assert folder exists
	TCHAR *p = _tcschr(opts[typeNum].file_name, _T('\\'));
	if (p != NULL)
		p = _tcschr(p+1, _T('\\'));
	while(p != NULL)
	{
		*p = _T('\0');
		CreateDirectory(opts[typeNum].file_name, NULL);
		*p = _T('\\');
		p = _tcschr(p+1, _T('\\'));
	}

	FILE *fp = _tfopen(opts[typeNum].file_name, _T("at"));
	if (fp != NULL)
	{
		_ftprintf(fp, _T("%s\n"), txt.str);
		fclose(fp);
	}
}


void Speak(HANDLE hContact, int type, int templateNum, TCHAR **vars, int numVars)
{
	if (!ServiceExists(MS_SPEAK_SAY))
		return;
	
	if (templateNum == 0 && !opts[type].speak_track_changes)
		return;
	if (templateNum == 1 && !opts[type].speak_track_removes)
		return;

	TCHAR *templ = (templateNum == 1 ? opts[type].speak_template_removed : opts[type].speak_template_changed);
	Buffer<TCHAR> txt;
	ReplaceTemplate(&txt, hContact, templ, vars, numVars);

#ifdef UNICODE
	char *tmp = mir_t2a(txt.str);
	CallService(MS_SPEAK_SAY, (LPARAM) NULL, (WPARAM) tmp);
	mir_free(tmp);
#else
	CallService(MS_SPEAK_SAY, (LPARAM) NULL, (WPARAM) txt.str);
#endif
}


void Notify(HANDLE hContact, int type, BOOL found_old, int templateNum, TCHAR **vars, int numVars)
{
	if (!found_old && !types[type].canBeRemoved && !types[type].temporary)
		return;

	if (DBGetContactSettingDword(hContact, MODULE_NAME, "CreationTickCount", 0) 
					+ TIME_TO_WAIT_BEFORE_SHOW_POPUP_AFTER_CREATION > GetTickCount())
		return;

	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (proto == NULL || proto[0] == '\0')
		return;

	if (opts[type].dont_notify_on_connect)
	{
		char onOffSetting[256];
		mir_snprintf(onOffSetting, MAX_REGS(onOffSetting), "%s_OnOfflineTickCount", proto);
		if (DBGetContactSettingDword(NULL, MODULE_NAME, onOffSetting, 0) 
					+ TIME_TO_WAIT_BEFORE_NOTIFY_AFTER_CONNECTION > GetTickCount())
			return;
	}

	if (ItemEnabled(type, hContact, NOTIFY_POPUP))
		ShowPopup(hContact, type, templateNum, vars, numVars);

	if (ItemEnabled(type, hContact, NOTIFY_SOUND))
	{
		char tmp[256];
		mir_snprintf(tmp, MAX_REGS(tmp), templateNum == 0 ? "%s change" : "%s removal", types[type].description);
		SkinPlaySound(tmp);
	}

	if (ItemEnabled(type, hContact, NOTIFY_SPEAK))
		Speak(hContact, type, templateNum, vars, numVars);
}


void Log(int type, BOOL found_old, BOOL changed, HANDLE hContact, TCHAR *oldVal, TCHAR *newVal)
{
	if (newVal == NULL || newVal[0] == _T('\0'))
		newVal = TranslateT("<empty>");
	if (oldVal == NULL || oldVal[0] == _T('\0'))
		oldVal = TranslateT("<empty>");

	int numVars = 4 + 2 * types[type].numAddVars;
	TCHAR **vars = (TCHAR **) mir_alloc0(sizeof(TCHAR *) * numVars);
	vars[0] = _T("old");
	vars[1] = oldVal;
	vars[2] = _T("new");
	vars[3] = newVal;
	
	if (types[type].fAddVars != NULL)
		types[type].fAddVars(hContact, vars, 4);

	int templateNum = changed ? 0 : 1;

	if (ItemEnabled(type, hContact, LOG_HISTORY))
		HistoryEvents_AddToHistoryVars(hContact, types[type].eventType, templateNum, vars, numVars, DBEF_READ);

	if (ItemEnabled(type, hContact, LOG_FILE))
		LogToFile(hContact, type, templateNum, vars, numVars);
	
	Notify(hContact, type, found_old, templateNum, vars, numVars);

	for(int i = 0; i < types[type].numAddVars; i++)
		mir_free(vars[4 + 2 * i + 1]);
}


int inline CheckStr(TCHAR *str, int not_empty, int empty)
{
	if (str == NULL || str[0] == _T('\0'))
		return empty;
	else
		return not_empty;
}


int ContactAdded(WPARAM wParam, LPARAM lParam)
{
	DBWriteContactSettingDword((HANDLE) wParam, MODULE_NAME, "CreationTickCount", GetTickCount());
	return 0;
}


int ProtoAckHook(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (ack->szModule != NULL && ack->type == ACKTYPE_STATUS && ack->result == ACKRESULT_SUCCESS) 
	{
		const char *proto = ack->szModule;
		int status = (int) ack->lParam;
		char lastStatusSetting[256];
		mir_snprintf(lastStatusSetting, MAX_REGS(lastStatusSetting), "%s_LastStatus", proto);

		int oldStatus = DBGetContactSettingWord(NULL, MODULE_NAME, lastStatusSetting, ID_STATUS_OFFLINE);

		if ( (status > ID_STATUS_OFFLINE && oldStatus <= ID_STATUS_OFFLINE)
			 || (status <= ID_STATUS_OFFLINE && oldStatus > ID_STATUS_OFFLINE) )
		{
			char onOffSetting[256];
			mir_snprintf(onOffSetting, MAX_REGS(onOffSetting), "%s_OnOfflineTickCount", proto);

			DBWriteContactSettingDword(NULL, MODULE_NAME, onOffSetting, GetTickCount());
		}

		DBWriteContactSettingWord(NULL, MODULE_NAME, lastStatusSetting, status);
	}

	return 0;
}


int SettingChanged(WPARAM wParam, LPARAM lParam)
{
	if (!loaded)
		return 0;


	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (proto == NULL || proto[0] == '\0') // || (metacontacts_proto != NULL && !strcmp(proto, metacontacts_proto)))
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	for (int i = 0; i < NUM_TYPES; i++) 
	{
		HISTORY_TYPE &type = types[i];

		if (type.track.db.module == NULL || type.track.db.setting == NULL)
			continue;

		char *module;
		if (type.track.db.module == MODULE_PROTOCOL)
			module = proto;
		else
			module = type.track.db.module;

		if (!strcmp(cws->szModule, module) && !strcmp(cws->szSetting, type.track.db.setting))
		{
			if (opts[i].track_only_not_offline)
				if (DBGetContactSettingWord(hContact, proto, "Status", 0) <= ID_STATUS_OFFLINE)
					continue;

			if (!ContactEnabled(i, hContact))
				continue;

			queue->AddAndRemovePreviousConsiderParam(TIME_TO_WAIT_BEFORE_PROCESSING, hContact, (void *) i);
		}
	}

	return 0;
}


void TrackChangeString(int typeNum, HANDLE hContact)
{
	HISTORY_TYPE &type = types[typeNum];

	char *module;
	if (type.track.db.module == MODULE_PROTOCOL)
		module = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	else
		module = type.track.db.module;

	char current_setting[256];
	mir_snprintf(current_setting, MAX_REGS(current_setting), "%s_%s_Current", module, type.track.db.setting);

	DBVARIANT old = {0};
	BOOL found_old = (DBGetContactSettingTString(hContact, MODULE_NAME, current_setting, &old) == 0);

	DBVARIANT new_ = {0};
	BOOL found_new = (DBGetContactSettingTString(hContact, module, type.track.db.setting, &new_) == 0);

	TCHAR *oldVal;
	if (found_old)
		oldVal = old.ptszVal;
	else
		oldVal = (TCHAR *) type.defs.value;
	if (oldVal == NULL)
		oldVal = _T("");

	// 0 if not changed, 1 if changed, 2 if removed
	int ret = 0;
	if (!found_new)
	{
		ret = CheckStr(oldVal, 2, 0);
	}
	else
	{
		BOOL eq;
		if (type.fEquals != NULL)
			eq = type.fEquals(new_.ptszVal, oldVal);
		else
			eq = (lstrcmp(new_.ptszVal, oldVal) == 0);

		ret = (eq ? 0 : CheckStr(new_.ptszVal, 1, 2));
	}

	if (ret == 0)
	{
		// Check sub
	}

	BOOL track_removes;
	if (ret == 2)
		track_removes = type.canBeRemoved && 
						(opts[typeNum].popup_track_removes 
						|| opts[typeNum].file_track_removes 
						|| opts[typeNum].speak_track_removes 
						|| HistoryEvents_IsEnabledTemplate(type.eventType, 1));

	if (ret == 1 || (ret == 2 && track_removes))
	{
		if (type.fFormat != NULL)
		{
			TCHAR old_str[256];
			TCHAR new_str[256];

			type.fFormat(old_str, MAX_REGS(old_str), oldVal);
			type.fFormat(new_str, MAX_REGS(new_str), new_.ptszVal);
			Log(typeNum, found_old, ret == 1, hContact, old_str, new_str);
		}
		else
			Log(typeNum, found_old, ret == 1, hContact, oldVal, new_.ptszVal);

		// Copy new to current after notification, so old value can still be accessed
		if (ret == 2)
		{
			DBDeleteContactSetting(hContact, MODULE_NAME, current_setting);
		}
		else
		{
			DBWriteContactSettingTString(hContact, MODULE_NAME, current_setting, new_.ptszVal);
		}
	}

	if (found_old)
		DBFreeVariant(&old);
	if (found_new)
		DBFreeVariant(&new_);
}

inline static DWORD GetDWORD(const DBVARIANT &dbv)
{
	switch(dbv.type)
	{
		case DBVT_BYTE: return dbv.bVal;
		case DBVT_WORD: return dbv.wVal;
		case DBVT_DWORD: return dbv.dVal;
	}
	return 0;
}

void TrackChangeNumber(int typeNum, HANDLE hContact)
{
	HISTORY_TYPE &type = types[typeNum];

	char *module;
	if (type.track.db.module == MODULE_PROTOCOL)
		module = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	else
		module = type.track.db.module;

	char current_setting[256];
	mir_snprintf(current_setting, MAX_REGS(current_setting), "%s_%s_Current", module, type.track.db.setting);

	DBVARIANT old = {0};
	BOOL found_old = (DBGetContactSetting(hContact, MODULE_NAME, current_setting, &old) == 0);

	DBVARIANT new_ = {0};
	BOOL found_new = (DBGetContactSetting(hContact, module, type.track.db.setting, &new_) == 0);

	DWORD oldVal;
	if (found_old)
		oldVal = GetDWORD(old);
	else
		oldVal = type.defs.value;
	DWORD newVal = GetDWORD(new_);

	// 0 if not changed, 1 if changed, 2 if removed
	int ret = 0;
	if (!found_new || new_.type == DBVT_BLOB)
	{
		ret = (found_old ? 2 : 0);
	}
	else
	{
		ret = (newVal != oldVal ? 1 : 0);
	}

	BOOL track_removes;
	if (ret == 2)
		track_removes = type.canBeRemoved && 
						(opts[typeNum].popup_track_removes 
						|| opts[typeNum].file_track_removes 
						|| HistoryEvents_IsEnabledTemplate(type.eventType, ret - 1));

	if (ret == 1 || (ret == 2 && !track_removes))
	{
		TCHAR tmp_old[256];
		type.fFormat(tmp_old, MAX_REGS(tmp_old), (void *) oldVal);

		TCHAR tmp_new[256];
		if (found_new) 
			type.fFormat(tmp_new, MAX_REGS(tmp_new), (void *) newVal);
		else
			tmp_new[0] = _T('\0');

		Log(typeNum, found_old, ret == 1, hContact, tmp_old, tmp_new);

		// Copy new to current after notification, so old value can still be accessed
		if (ret == 2)
		{
			DBDeleteContactSetting(hContact, MODULE_NAME, current_setting);
		}
		else
		{
			switch(new_.type)
			{
				case DBVT_BYTE: DBWriteContactSettingByte(hContact, MODULE_NAME, current_setting, new_.bVal); break;
				case DBVT_WORD: DBWriteContactSettingWord(hContact, MODULE_NAME, current_setting, new_.wVal); break;
				case DBVT_DWORD: DBWriteContactSettingDword(hContact, MODULE_NAME, current_setting, new_.dVal); break;
			}
		}
	}

	if (found_old)
		DBFreeVariant(&old);
	if (found_new)
		DBFreeVariant(&new_);
}


void Process(HANDLE hContact, void *param)
{
	int typeNum = (int) param;

	if (types[typeNum].track.db.isString)
		TrackChangeString(typeNum, hContact);
	else
		TrackChangeNumber(typeNum, hContact);
}
