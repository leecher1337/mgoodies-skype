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
	"Contact Left Channel History",
	PLUGIN_MAKE_VERSION(0,0,0,1),
	"Log when contact left channel to history",
	"Ricardo Pescuma Domenecci",
	"",
	"",
	"http://miranda-im.org/",
	0,	//not transient
	0	//doesn't replace anything built-in
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hModulesLoaded = NULL;

char *metacontacts_proto = NULL;
BOOL loaded = FALSE;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int ContactLeftChannel(WPARAM wParam,LPARAM lParam);


#define DEFAULT_TEMPLATE "left channel"


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

	// hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	UnhookEvent(hModulesLoaded);
	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	// Hook events
	PROTOCOLDESCRIPTOR **protos;
	int count;

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

	for (int i = 0; i < count; i++)
	{
		if (protos[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0' 
			|| (metacontacts_proto != NULL && !strcmp(metacontacts_proto, protos[i]->szName)))
			continue;

		char evtname[250];
		mir_snprintf(evtname, MAX_REGS(evtname), "%s/ContactLeftChannel", protos[i]->szName);
		HookEvent(evtname, ContactLeftChannel);
	}


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

		upd.szBetaVersionURL = "http://br.geocities.com/ricardo_pescuma/clch_version.txt";
		upd.szBetaChangelogURL = "http://br.geocities.com/ricardo_pescuma/clch_changelog.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"Contact Left Channel History ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
		upd.szBetaUpdateURL = "http://br.geocities.com/ricardo_pescuma/clch.zip";

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	loaded = TRUE;

	return 0;
}


HANDLE HistoryLog(HANDLE hContact, char *log_text)
{
	if (log_text != NULL)
	{
		DBEVENTINFO event = { 0 };;

		event.cbSize = sizeof(event);

		event.pBlob = (PBYTE) log_text;
		event.cbBlob = strlen(log_text) + 1;

		event.eventType = EVENTTYPE_CONTACTLEFTCHANNEL;
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

void AddToHistory(HANDLE hContact)
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

	HistoryLog(hContact, templ);
}

BOOL MsgWndOpen(HANDLE hContact)
{
	if (ServiceExists("SRMsg_MOD/MessageDialogOpened"))		// tabSRMM service
	{
		return CallService("SRMsg_MOD/MessageDialogOpened", (WPARAM) hContact, 0);
	}
	else
	{
		MessageWindowInputData mwid;
		mwid.cbSize = sizeof(MessageWindowInputData);
		mwid.hContact = hContact;
		mwid.uFlags = MSG_WINDOW_STATE_EXISTS;

		MessageWindowData mwd = {0};

		if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM) &mwid, (LPARAM) &mwd))
			if (mwd.hwndWindow != NULL && (mwd.uState & MSG_WINDOW_STATE_EXISTS)) 
				return TRUE;
	}

	return FALSE;
}

int ContactLeftChannel(WPARAM wParam,LPARAM lParam)
{
	if (!loaded)
		return 0;

	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (proto == NULL)
		return 0;

	if (DBGetContactSettingWord(hContact, proto, "Status", ID_STATUS_OFFLINE) <= ID_STATUS_OFFLINE)
		return 0;

	// See if window is open
	if (!MsgWndOpen(hContact))
	{
		HANDLE hMetaContact = NULL;

		if (ServiceExists(MS_MC_GETMETACONTACT)) 
			hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

		if (hMetaContact == NULL)
			return 0;

		if (!MsgWndOpen(hMetaContact))
			return 0;
	}

	AddToHistory(hContact);

	return 0;
}

