/*

IEView Plugin for Miranda IM
Copyright (C) 2005  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "IEView.h"
#include "m_ieview.h"
#include "ieview_services.h"
#include "Options.h"
#include "Utils.h"
//#include "SmileyWindow.h"

char *ieviewModuleName;
HINSTANCE hInstance;
PLUGINLINK *pluginLink;
IEView *debugView;
TCHAR *workingDir;
static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int PreShutdown(WPARAM wParam, LPARAM lParam);

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"IEView",
	PLUGIN_MAKE_VERSION(1,0,9,4),
	"IE Based Chat Log (1.0.9.4	"__DATE__")",
	"Piotr Piastucki",
	"the_leech@users.berlios.de",
	"(c) 2005-2006 Piotr Piastucki",
	"http://developer.berlios.de/projects/mgoodies",
	0,
	0
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFO *MirandaPluginInfo(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0,4,0,0)) {
		MessageBoxA(NULL, "The IEView plugin cannot be loaded. It requires Miranda IM 0.4 or later.", "IEView Plugin", MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return NULL;
	}
	return &pluginInfo;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	char text[_MAX_PATH];
	char *p, *q;

	int wdsize = GetCurrentDirectory(0, NULL);
	workingDir = new TCHAR[wdsize];
	GetCurrentDirectory(wdsize, workingDir);
	Utils::convertPath(workingDir);

	GetModuleFileNameA(hInstance, text, sizeof(text));
	p = strrchr(text, '\\');
	p++;
	q = strrchr(p, '.');
	*q = '\0';
	ieviewModuleName = _strdup(p);
	_strupr(ieviewModuleName);

	pluginLink = link;

	HookEvent(ME_OPT_INITIALISE, IEViewOptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	CreateServiceFunction(MS_IEVIEW_WINDOW, HandleIEWindow);
	CreateServiceFunction(MS_IEVIEW_EVENT, HandleIEEvent);

	hHookOptionsChanged = CreateHookableEvent(ME_IEVIEW_OPTIONSCHANGED);

	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	IEView::init();
	Options::init();
	return 0;
}

static int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	IEView::release();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	delete workingDir;
	return 0;
}
