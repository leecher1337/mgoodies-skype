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
	"ersatz",
	PLUGIN_MAKE_VERSION(0,0,1,0),
	"Hacks the PS_SETAWAYMSG service in order to provide a new one: PS_GETMYAWAYMSG",
	"TioDuke, Ricardo Pescuma Domenecci",
	"tioduke@yahoo.ca",
	"(c) 2006 TioDuke",
	"http://miranda-im.org",
	0,	//not transient
	0	//not used
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;

static HANDLE hModulesLoaded = NULL;
static HANDLE hPreShutdownHook = NULL;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);

int ErsatzEnabled(WPARAM wParam, LPARAM lParam);


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
	
	InitServices();

	// hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hPreShutdownHook = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	ModulesLoadedServices();

	CreateServiceFunction(MS_ERSATZ_ENABLED, ErsatzEnabled);

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
		upd.szBetaVersionURL = "http://eth0.dk/files/pescuma/ersatz_version.txt";
		upd.szBetaChangelogURL = "http://eth0.dk/files/pescuma/ersatz_changelog.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"ersatz ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
		upd.szBetaUpdateURL = "http://eth0.dk/files/pescuma/ersatz.zip";
		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	DestroyServices();

	UnhookEvent(hModulesLoaded);
	UnhookEvent(hPreShutdownHook);

	return 0;
}


int ErsatzEnabled(WPARAM wParam, LPARAM lParam)
{
	return 1;
}

