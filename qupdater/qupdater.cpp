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
	"Q Updater",
	PLUGIN_MAKE_VERSION(0,0,0,1),
	"Simulates updater API to gather plugin info to use at Q search site",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2007 Ricardo Pescuma Domenecci",
	"http://q.mirandaim.ru",
	0,	//not transient
	0	//doesn't replace anything built-in
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hModulesLoaded = NULL;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int UpdateRegister(WPARAM wParam, LPARAM lParam);

vector<Update> plugins;


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
	InitOptions();

	CreateServiceFunction(MS_UPDATE_REGISTER, UpdateRegister);

	// hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	DeInitOptions();
	UnhookEvent(hModulesLoaded);
	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME, 0);

	return 0;
}


// Called when all the modules are loaded
int UpdateRegister(WPARAM wParam, LPARAM lParam) 
{
	Update *in = (Update *) lParam;
	if (in == NULL) // Some plugins don't respect this: || in->cbSize < sizeof(Update))
		return NULL;

	Update updt = {0};
	updt.szComponentName = mir_strdup(in->szComponentName);
	updt.szVersionURL = mir_strdup(in->szVersionURL);
	updt.pbVersionPrefix = (BYTE *) mir_strdup((char *) in->pbVersionPrefix);
	updt.szUpdateURL = mir_strdup(in->szUpdateURL);

	updt.szBetaVersionURL = mir_strdup(in->szBetaVersionURL);
	updt.pbBetaVersionPrefix = (BYTE *) mir_strdup((char *) in->pbBetaVersionPrefix);
	updt.szBetaUpdateURL = mir_strdup(in->szBetaUpdateURL);

	updt.pbVersion = (BYTE *) mir_strdup((char *) in->pbVersion);

	if (in->cbSize >= sizeof(Update))
		updt.szBetaChangelogURL = mir_strdup(in->szBetaChangelogURL);

	plugins.insert(plugins.begin(), updt);

	if (opts.dump_on_startup)
		DumpFile();

	return 0;
}

void DumpFile()
{
	FILE *arq = fopen(opts.csv_file, "w");
	if (arq == NULL)
		return;

	for(int i = 0; i < plugins.size(); i++)
	{
		fprintf(arq, "\"");
		fprintf(arq, plugins[i].szComponentName);
		fprintf(arq, "\",\"");
		if (plugins[i].pbVersion != NULL)
			fprintf(arq, (char *) plugins[i].pbVersion);
		fprintf(arq, "\",\"");

		if (plugins[i].szVersionURL != NULL)
			fprintf(arq, plugins[i].szVersionURL);
		fprintf(arq, "\",\"");
		if (plugins[i].pbVersionPrefix != NULL)
			fprintf(arq, (char *) plugins[i].pbVersionPrefix);
		fprintf(arq, "\",\"");
		if (plugins[i].szUpdateURL != NULL)
		{
			if (strcmp(plugins[i].szUpdateURL, UPDATER_AUTOREGISTER) == 0)
				fprintf(arq, "<From FL XML>");
			else
				fprintf(arq, plugins[i].szUpdateURL);
		}
		fprintf(arq, "\",\"");

		if (plugins[i].szBetaVersionURL != NULL)
			fprintf(arq, plugins[i].szBetaVersionURL);
		fprintf(arq, "\",\"");
		if (plugins[i].pbBetaVersionPrefix != NULL)
			fprintf(arq, (char *) plugins[i].pbBetaVersionPrefix);
		fprintf(arq, "\",\"");
		if (plugins[i].szBetaUpdateURL != NULL)
			fprintf(arq, plugins[i].szBetaUpdateURL);
		fprintf(arq, "\",\"");

		if (plugins[i].szBetaChangelogURL != NULL)
			fprintf(arq, plugins[i].szBetaChangelogURL);
		fprintf(arq, "\"\n");
	}

	fclose(arq);
}