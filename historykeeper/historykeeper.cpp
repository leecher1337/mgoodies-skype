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
	PLUGIN_MAKE_VERSION(0,0,0,1),
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

HANDLE hHooks[5] = {0};

HANDLE hEnableMenu[NUM_TYPES]; 
HANDLE hDisableMenu[NUM_TYPES]; 

char *metacontacts_proto = NULL;
BOOL loaded = FALSE;
ContactAsyncQueue *queue;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreBuildContactMenu(WPARAM wParam,LPARAM lParam);
int SettingChanged(WPARAM wParam,LPARAM lParam);
int ContactAdded(WPARAM wParam,LPARAM lParam);
int ProtoAckHook(WPARAM wParam, LPARAM lParam);

int EnableHistory(WPARAM wParam, LPARAM lParam, LPARAM type);
int DisableHistory(WPARAM wParam, LPARAM lParam, LPARAM type);
int HistoryEnabled(WPARAM wParam, LPARAM lParam, LPARAM type);

BOOL ContactEnabled(int type, HANDLE hContact);
BOOL ProtocolEnabled(int type, const char *protocol);

void Process(HANDLE hContact, void *param);




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
/*
	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM) MODULE_NAME "/CreationTickCount");

	PROTOCOLDESCRIPTOR **protos;
	int i, count;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);
	char tmp[256];
	for (i = 0; i < count; i++)
	{
		if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0')
			continue;

		mir_snprintf(tmp, MAX_REGS(tmp), "%s/" MODULE_NAME "_OnlineTickCount", protos[i]->szName);
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM) MODULE_NAME "_OnlineTickCount");

		mir_snprintf(tmp, MAX_REGS(tmp), "%s/" MODULE_NAME "_LastStatus", protos[i]->szName);
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM) MODULE_NAME "_LastStatus");
	}
*/

	// Add menu item to enable/disable status message check
	char name[256];
	char service[256];
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.position = 1000100010;
	mi.flags = CMIF_NOTOFFLIST;
	mi.pszName = name;
	mi.pszService = service;

	for (int i = 0; i < NUM_TYPES; i++) 
	{
		// History

		char tmp[128];
		strncpy(tmp, types[i].description, MAX_REGS(tmp));
		CharLowerA(tmp);

		char change[256];
		mir_snprintf(change, MAX_REGS(change), "Change\nchanged his/her %s to %%new%%\n%%old%%\tOld value\n%%new%%\tNew value",
			tmp);

		char remove[256];
		mir_snprintf(remove, MAX_REGS(remove), "Removal\nremoved his/her %s\n%%old%%\tOld value",
			tmp);

		char *templates[] = { change, remove };

		char desc[128];
		mir_snprintf(desc, MAX_REGS(desc), "%s Change", types[i].description);

		HICON hIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(types[i].icon), IMAGE_ICON, 16, 16, 0);
		HistoryEvents_RegisterMessageStyle(MODULE_NAME, types[i].name, desc, types[i].eventType, 
			hIcon, HISTORYEVENTS_FLAG_SHOW_IM_SRMM | HISTORYEVENTS_FLAG_EXPECT_CONTACT_NAME_BEFORE,
			templates, MAX_REGS(templates));
		DestroyIcon(hIcon);


		// Menus and services

		mi.hIcon = HistoryEvents_GetIcon(types[i].eventType);

		mir_snprintf(service, MAX_REGS(service), "%s/Disable", types[i].name);
		CreateServiceFunctionParam(service, DisableHistory, i);

		mir_snprintf(name, MAX_REGS(name), "Don't log %s changes", types[i].description);
		hDisableMenu[i] = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

		mir_snprintf(service, MAX_REGS(service), "%s/Enable", types[i].name);
		CreateServiceFunctionParam(service, EnableHistory, i);
		
		mir_snprintf(name, MAX_REGS(name), "Log %s changes", types[i].description);
		hEnableMenu[i] = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

		mir_snprintf(service, MAX_REGS(service), "%s/Enabled", types[i].name);
		CreateServiceFunctionParam(service, HistoryEnabled, i);

		HistoryEvents_ReleaseIcon(mi.hIcon);
	}
	
	// hooks
	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hHooks[1] = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);
	hHooks[2] = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);
	hHooks[3] = HookEvent(ME_DB_CONTACT_ADDED, ContactAdded);
	hHooks[4] = HookEvent(ME_PROTO_ACK, ProtoAckHook);

	InitOptions();
	InitPopups();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	// TODO PreShutdown

	delete queue;

	DeInitPopups();
	DeInitOptions();

	for (int i = 0; i < MAX_REGS(hHooks); i++)
		if (hHooks[i] != NULL)
			UnhookEvent(hHooks[i]);
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

	queue = new ContactAsyncQueue(&Process);

	loaded = TRUE;

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

	char setting[256];
	mir_snprintf(setting, sizeof(setting), "%sEnabled", types[type].name);
	DBWriteContactSettingByte(hContact, MODULE_NAME, setting, TRUE);

	return 0;
}

int DisableHistory(WPARAM wParam, LPARAM lParam, LPARAM type) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	char setting[256];
	mir_snprintf(setting, sizeof(setting), "%sEnabled", types[type].name);
	DBWriteContactSettingByte(hContact, MODULE_NAME, setting, FALSE);

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

	BYTE def = TRUE;

	// Is a subcontact?
	if (ServiceExists(MS_MC_GETMETACONTACT)) 
	{
		HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

		if (hMetaContact != NULL)
			def = ContactEnabled(type, hMetaContact);
	}

	char setting[256];
	mir_snprintf(setting, MAX_REGS(setting), "%sEnabled", types[type].name);
	return DBGetContactSettingByte(hContact, MODULE_NAME, setting, def);
}


void Notify(int type, BOOL changed, HANDLE hContact, TCHAR *oldVal, TCHAR *newVal)
{
	if (newVal == NULL || newVal[0] == _T('\0'))
		newVal = TranslateT("<empty>");
	if (oldVal == NULL || oldVal[0] == _T('\0'))
		oldVal = TranslateT("<empty>");
	TCHAR *vars[] = { _T("%old%"), oldVal, _T("%new%"), newVal };

	int templateNum = changed ? 0 : 1;

	HistoryEvents_AddToHistoryVars(hContact, types[type].eventType, templateNum, vars, MAX_REGS(vars), DBEF_READ);
	ShowPopup(hContact, type, templateNum, vars, MAX_REGS(vars));
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
		int oldStatus = DBGetContactSettingWord(NULL, proto, MODULE_NAME "_LastStatus", ID_STATUS_OFFLINE);

		if (status > ID_STATUS_OFFLINE && oldStatus <= ID_STATUS_OFFLINE)
			DBWriteContactSettingDword(NULL, proto, MODULE_NAME "_OnlineTickCount", GetTickCount());

		DBWriteContactSettingWord(NULL, proto, MODULE_NAME "_LastStatus", status);
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
	if (proto == NULL || (metacontacts_proto != NULL && !strcmp(proto, metacontacts_proto)))
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	for (int i = 0; i < NUM_TYPES; i++) 
	{
		HISTORY_TYPE &type = types[i];

		if (type.track.db.module == NULL || type.track.db.setting == NULL)
			continue;

		char *module;
		if ((int) type.track.db.module == -1)
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
	if ((int) type.track.db.module == -1)
		module = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	else
		module = type.track.db.module;

	char current_setting[256];
	mir_snprintf(current_setting, MAX_REGS(current_setting), "%sCurrent", type.track.db.setting);

	DBVARIANT old = {0};
	BOOL found_old = (DBGetContactSettingTString(hContact, module, current_setting, &old) == 0);

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

	BOOL track_removes;
	if (ret != 0)
		track_removes = opts[typeNum].popup_track_removes || HistoryEvents_IsEnabledTemplate(type.eventType, ret - 1);

	if (ret == 1 || (ret == 2 && track_removes))
	{
		// Copy new to current
		if (ret == 2)
			DBWriteContactSettingTString(hContact, module, current_setting, _T(""));
		else
			DBWriteContactSettingTString(hContact, module, current_setting, new_.ptszVal);

		if (type.fFormat != NULL)
		{
			TCHAR old_str[256];
			TCHAR new_str[256];

			type.fFormat(old_str, MAX_REGS(old_str), oldVal);
			type.fFormat(new_str, MAX_REGS(new_str), new_.ptszVal);
			Notify(typeNum, ret == 1, hContact, old_str, new_str);
		}
		else
			Notify(typeNum, ret == 1, hContact, oldVal, new_.ptszVal);
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
	if ((int) type.track.db.module == -1)
		module = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	else
		module = type.track.db.module;

	char current_setting[256];
	mir_snprintf(current_setting, MAX_REGS(current_setting), "%sCurrent", type.track.db.setting);

	DBVARIANT old = {0};
	BOOL found_old = (DBGetContactSetting(hContact, module, current_setting, &old) == 0);

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
	if (ret != 0)
		track_removes = opts[typeNum].popup_track_removes || HistoryEvents_IsEnabledTemplate(type.eventType, ret - 1);

	if (ret == 1 || (ret == 2 && !track_removes))
	{
		// Copy new to current
		if (ret == 2 || newVal == type.defs.value)
		{
			DBDeleteContactSetting(hContact, module, current_setting);
		}
		else
		{
			switch(new_.type)
			{
				case DBVT_BYTE: DBWriteContactSettingByte(hContact, module, current_setting, new_.bVal); break;
				case DBVT_WORD: DBGetContactSettingWord(hContact, module, current_setting, new_.wVal); break;
				case DBVT_DWORD: DBGetContactSettingDword(hContact, module, current_setting, new_.dVal); break;
			}
		}

		TCHAR tmp_old[256];
		type.fFormat(tmp_old, MAX_REGS(tmp_old), (void *) oldVal);

		TCHAR tmp_new[256];
		if (found_new) 
			type.fFormat(tmp_new, MAX_REGS(tmp_new), (void *) newVal);
		else
			tmp_new[0] = _T('\0');

		Notify(typeNum, ret == 1, hContact, tmp_old, tmp_new);
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
