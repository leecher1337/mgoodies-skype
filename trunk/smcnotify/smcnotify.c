/*
Status Message Change Notify plugin for Miranda IM.

Copyright © 2004-2005 NoName
Copyright © 2005-2007 Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci

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

#include "commonheaders.h"


// Prototypes ///////////////////////////////////////////////////////////////////////////

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	PLUGIN_NAME
#ifdef CUSTOMBUILD_CATCHICQSTATUSMSG
	" +ICQ"
#endif
#ifdef CUSTOMBUILD_OSDSUPPORT
	" +OSD"
#endif
#ifdef _UNICODE
	" (Unicode)"
#endif
	,
	PLUGIN_MAKE_VERSION(0,0,3,17),
	"Notifies, logs and stores history of your contacts' status messages changes",
	"Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci",
	"slotwin@users.berlios.de",
	"",
	"http://developer.berlios.de/projects/mgoodies",
	UNICODE_AWARE,
	0
};

int ProtoAck(WPARAM wParam, LPARAM lParam);

HANDLE heModulesLoaded;
HANDLE heOptionsInit;
HANDLE heProtoAck;
HANDLE hePreBuildCMenu;
HANDLE heContactSettingChanged;
HANDLE heUserInfoInit;

// Functions ////////////////////////////////////////////////////////////////////////////

extern BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst = hinstDLL;
	return TRUE;
}

extern __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) {
/*	if (mirandaVersion < PLUGIN_MAKE_VERSION(0,6,6,0))
	{
		//not translatable
		MessageBox(NULL, "Plugin requires Miranda 0.6.6.0 or later to run.", PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return NULL;
	}
*/	return &pluginInfo;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam) {

	CreateServiceFunction(MS_SMCNOTIFY_POPUPS, MenuItemCmd_PopUps);
	CreateServiceFunction(MS_SMCNOTIFY_LIST, MenuItemCmd_ShowList);
	CreateServiceFunction(MS_SMCNOTIFY_GOTOURL, MenuItemCmd_GoToURL);
	hListDlg = NULL;

	InitPopups();
	LoadIcons();
	InitMenuItems();

	heContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	heUserInfoInit = HookEvent(ME_USERINFO_INITIALISE, UserInfoInit);

	//Register with Updater plugin
	if (ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd;
		char szCurrentVersion[30];

		ZeroMemory(&upd, sizeof(upd));
		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://www.torun.mm.pl/~slotwin/smcnotify/version.txt";
		upd.szBetaChangelogURL = "http://www.torun.mm.pl/~slotwin/smcnotify/changelog.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"Status Message Change Notify ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://www.torun.mm.pl/~slotwin/smcnotify/smcnotifyW.zip";
#else
		upd.szBetaUpdateURL = "http://www.torun.mm.pl/~slotwin/smcnotify/smcnotify.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

		CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	//Add sound event
	SkinAddNewSoundEx("smcnotify", Translate("Status Notify"), Translate("User has changed status message"));

	//Add our module to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM)MODULE_NAME, 0);

	return 0;
}

extern int __declspec(dllexport) Load(PLUGINLINK *link) {
	INITCOMMONCONTROLSEX icex;

	//Ensure that the common control DLL is loaded.
	ZeroMemory(&icex, sizeof(icex));
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	pluginLink = link;

	init_mir_malloc();
	LoadOptions();

	//Hooks
	heModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	heOptionsInit = HookEvent(ME_OPT_INITIALISE, OptionsInit);
	heProtoAck = HookEvent(ME_PROTO_ACK, ProtoAck); // needed for "show popups when i connect"
	hePreBuildCMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildCMenu);

	return 0;
}

extern int __declspec(dllexport) Unload(void) {

	UnhookEvent(heModulesLoaded);
	UnhookEvent(heOptionsInit);
	UnhookEvent(heProtoAck);
	UnhookEvent(hePreBuildCMenu);
	UnhookEvent(heUserInfoInit);
	UnhookEvent(heContactSettingChanged);

	DeinitPopups();

	if (hListDlg != NULL)
		SendMessage(hListDlg, WM_CLOSE, 0, 0);

	return 0;
}
