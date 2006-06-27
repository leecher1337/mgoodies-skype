/*
StatusMessageChangeNotify plugin for Miranda IM.

Copyright © 2004-2005 NoName
Copyright © 2005-2006 Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci

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

#include "main.h"

/*
Constants for the icons, removed "scn_int", "scn_pop"
*/
char *szIconId[ICONCOUNT] = {_T("smcn_list"), _T("smcn_url"), _T("smcn_hist"), _T("smcn_ext"), _T("smcn_pope"), _T("smcn_popd")};

char szStatusMsgURL[1024];

void StatusMsgChanged(WPARAM wParam, STATUSMSGINFO* smi);
BOOL CheckStatusMessage(HANDLE hContact, char str[255]);


PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"StatusMessageChangeNotify",
	PLUGIN_MAKE_VERSION(0,0,3,5),
	"Notify you via popup when someone changes his/her status message",
	"Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci",
	"slotwin@users.berlios.de",
	"",
	"http://developer.berlios.de/projects/mgoodies",
	0,	//not transient
	0	//doesn't replace anything built-in
};

/*
Menu update command, menu items commands
*/
void UpdateMenu(BOOL bState) {
	CLISTMENUITEM mi;
	ZeroMemory(&mi, sizeof(mi));
	//Main menu item
	mi.cbSize = sizeof(mi);
	if (bState)
	{
		mi.pszName = TranslateT("Enable &status message change notification");
		mi.hIcon = ICO_POPUP_D;
	}
	else
	{
		mi.pszName = TranslateT("Disable &status message change notification");
		mi.hIcon = ICO_POPUP_E;
	}
	options.bDisablePopUps = bState;
	mi.flags = CMIM_ICON | CMIM_NAME;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hEnableDisableMenu, (LPARAM)&mi);
	//Contact's menu item
	mi.flags = CMIM_ICON;
	mi.hIcon = ICO_HIST;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hContactMenu, (LPARAM)&mi);
	mi.hIcon = ICO_URL;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hGoToURLMenu, (LPARAM)&mi);
	//Update main menu icon
	mi.hIcon = ICO_LIST;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hShowListMenu, (LPARAM)&mi);
}

static int MenuItemCmd_PopUps(WPARAM wParam, LPARAM lParam) {
	UpdateMenu(!options.bDisablePopUps);
	DBWriteContactSettingByte(NULL, MODULE, OPT_DISPOPUPS, (BYTE)options.bDisablePopUps);
	return 0;
}

static int MenuItemCmd_ShowHistory(WPARAM wParam, LPARAM lParam) {
	ShowHistory((HANDLE)wParam);
	return 0;
}

static int MenuItemCmd_ShowList(WPARAM wParam, LPARAM lParam) {
	ShowList();
	if (hTopToolbarButtonShowList != NULL)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTopToolbarButtonShowList, TTBST_RELEASED);
	return 0;
}

static int MenuItemCmd_GoToURL(WPARAM wParam, LPARAM lParam) {
	CallService(MS_UTILS_OPENURL, 1, (LPARAM)&szStatusMsgURL);
	return 0;
}

static int MenuItemCmd_ContactPopUps(WPARAM wParam, LPARAM lParam) {
	DWORD mask = DBGetContactSettingDword((HANDLE)wParam, IGNORE_MODULE, IGNORE_MASK, 0);
	DBWriteContactSettingDword((HANDLE)wParam, IGNORE_MODULE, IGNORE_MASK, mask ^ IGNORE_POP);
	return 0;
}

static int PreBuildContactMenu(WPARAM wParam,LPARAM lParam) {
	CLISTMENUITEM clmi;
	char str[2048], *p;
	int c;
	ZeroMemory(&clmi, sizeof(clmi));
	clmi.cbSize = sizeof(clmi);
	clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
	if (CheckStatusMessage((HANDLE)wParam, str))
	{
		p = strstr(str, _T("www."));
		if (p == NULL) p = strstr(str, _T("http://"));
		if (p != NULL)
		{
			for (c = 0; p[c] != _T('\n') && p[c] != _T('\r') && p[c] != _T('\t') && p[c] != _T(' ') && p[c] != _T('\0'); c++);
			lstrcpyn(szStatusMsgURL, p, min(c + 1, 1024));
			clmi.flags = CMIM_FLAGS;
		}
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hGoToURLMenu, (LPARAM)&clmi);
	//Contact smcn popups disable/enable
//	if (!options.bHideSettingsMenu) {
// removing this option, you can do this with GenMenu
	clmi.flags = CMIM_FLAGS | CMIM_ICON | CMIM_NAME;
		if (DBGetContactSettingDword((HANDLE)wParam, IGNORE_MODULE, IGNORE_MASK, 0) & IGNORE_POP)
		{
			clmi.pszName = TranslateT("Enable SMCN PopUps");
			clmi.hIcon = ICO_POPUP_D;
		}
		else
		{
			clmi.pszName = TranslateT("Disable SMCN PopUps");
			clmi.hIcon = ICO_POPUP_E;
		}
//	}
//	else clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hContactPopUpsMenu, (LPARAM)&clmi);

	return 0;
}

/*
TopToolbar stuff
*/
static int Create_TopToolbarShowList(WPARAM wParam, LPARAM lParam) {
	if (ServiceExists(MS_TTB_ADDBUTTON))
	{
		TTBButtonV2 ttbb;
		ZeroMemory(&ttbb, sizeof(ttbb));
		ttbb.cbSize = sizeof(ttbb);
		ttbb.hIconUp = ttbb.hIconDn = ICO_LIST;
		ttbb.dwFlags = TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP;
		ttbb.pszServiceUp = ttbb.pszServiceDown = MS_SMCN_LIST;
		ttbb.name = TranslateT("List Contacts with Status Message");
		hTopToolbarButtonShowList = (HANDLE)CallService(MS_TTB_ADDBUTTON, (WPARAM)&ttbb, 0);
		if((int)hTopToolbarButtonShowList == -1)
		{
			hTopToolbarButtonShowList = NULL;
			return 1;
		}
		CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM((WORD)TTBO_TIPNAME, (WORD)hTopToolbarButtonShowList),
			(LPARAM)TranslateT("List Contacts with Status Message"));
	}
	return 0;
}

/*
Called when contact's settings has changed
*/
static int ContactSettingChanged(WPARAM wParam, LPARAM lParam) {
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	STATUSMSGINFO smi;

	//The setting must belong to a contact different from myself (NULL) and needs
	//to be related to the status. Otherwise, exit as fast as possible.
	if ((HANDLE)wParam == NULL) return 0;

	ZeroMemory(&smi, sizeof(smi));
	smi.proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)wParam, 0);

	//if contact changed status write TickCount to db
//	if (!lstrcmp(cws->szSetting, "OldStatus"))
	if (!lstrcmp(cws->szSetting, "Status") && !lstrcmp(cws->szModule, smi.proto))
	{
		DBWriteContactSettingDword((HANDLE)wParam, "UserOnline", "LastStatusChange", GetTickCount());
		//PUShowMessage("Status changed", SM_NOTIFY);
		return 0;
	}

	//Exit if setting changed is not status message
	if (lstrcmp(cws->szSetting, "StatusMsg")/*&& lstrcmp(cws->szSetting, "StatusDescr") && lstrcmp(cws->szSetting, "YMsg")*/)
	{
		return 0;
	}

	//If we're offline (=> we've just switched to offline), exit as fast as possible.
	if (smi.proto != NULL && CallProtoService(smi.proto, PS_GETSTATUS, 0, 0) != ID_STATUS_OFFLINE)
	{
//		if (CallProtoService(smi.proto, PS_GETSTATUS, 0, 0) == ID_STATUS_OFFLINE) return 0;
		smi.hContact = (HANDLE)wParam;
		if (cws->value.type == DBVT_DELETED) smi.newstatusmsg = "";
		else smi.newstatusmsg = _strdup(cws->value.pszVal);
		smi.bIsEmpty = !lstrcmp(smi.newstatusmsg, "");
		StatusMsgChanged(wParam, &smi);
	}

	//free memory
	if (smi.oldstatusmsg) free(smi.oldstatusmsg);
	if (smi.newstatusmsg) free(smi.newstatusmsg);
	if (smi.cust) free(smi.cust);
	return 0;
}

/*
Called when icon changed from the option
*/
static int HookedSkinIconsChanged(WPARAM wParam,LPARAM lParam) {
	int i;
	for (i = 0; i < ICONCOUNT; i++)
	{
		hLibIcons[i] = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[i]);
	}
	//Update menu icons
	UpdateMenu(options.bDisablePopUps);
	return 0;
}

/*
Called when all the modules are loaded
*/
static int HookedInit(WPARAM wParam, LPARAM lParam) {
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);

	//First time using this version - turn off popup and external logging for ignored contacts
/*	if ((int)DBGetContactSettingWord(NULL, MODULE, "Ver", 1) < 3)
	{
		HANDLE hContact;
		//Start looking for other weather stations
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact)
		{
			if (CallService(MS_IGNORE_ISIGNORED, (WPARAM)hContact, IGNOREEVENT_USERONLINE) != 0)
			{
				DBWriteContactSettingByte(hContact, MODULE, "Popup", FALSE);
				DBWriteContactSettingByte(hContact, MODULE, "External", FALSE);
				DBWriteContactSettingByte(hContact, MODULE, "Internal", TRUE);
			}
			else
			{
				DBWriteContactSettingByte(hContact, MODULE, "Popup", TRUE);
				DBWriteContactSettingByte(hContact, MODULE, "External", TRUE);
				DBWriteContactSettingByte(hContact, MODULE, "Internal", TRUE);
			}
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}
		DBWriteContactSettingWord(NULL, MODULE, "Ver", 3);
	}*/
	if ((int)DBGetContactSettingWord(NULL, MODULE, "Ver", 1) <= 3)
	{
		HANDLE hContact;
		BOOL bIgnoreOnline;
		BOOL bFirstRun = 0;

		if (DBGetContactSettingWord(NULL, MODULE, "Ver", 1) == 1) bFirstRun = 1;

		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact)
		{
			BYTE ignore;
			bIgnoreOnline = CallService(MS_IGNORE_ISIGNORED, (WPARAM)hContact, IGNOREEVENT_USERONLINE);

			ignore = DBGetContactSettingByte(hContact, MODULE, "Popup", bFirstRun?!bIgnoreOnline:TRUE)?0:IGNORE_POP;
			ignore = DBGetContactSettingByte(hContact, MODULE, "External", bFirstRun?!bIgnoreOnline:TRUE)?ignore:ignore | IGNORE_EXT;
			ignore = DBGetContactSettingByte(hContact, MODULE, "Internal", TRUE)?ignore:ignore | IGNORE_INT;
			DBWriteContactSettingDword(hContact, IGNORE_MODULE, IGNORE_MASK, ignore);

			DBDeleteContactSetting(hContact, MODULE, "Popup");
			DBDeleteContactSetting(hContact, MODULE, "External");
			DBDeleteContactSetting(hContact, MODULE, "Internal");

			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}
		DBWriteContactSettingWord(NULL, MODULE, "Ver", 4);

		//if user set "Show Popups when I connect" change the dbkey to new "format" but preserve
		//old behaviour so he won't be flooded with popups
		//if he really wants 1000s of popups let him change the setting himself so he knows that's him who fucked up ;)
		if (DBGetContactSettingByte(NULL, MODULE, OPT_SHOWONC, 0))
		{
			DBWriteContactSettingByte(NULL, MODULE, OPT_SHOWONC, 3);
			options.bShowOnConnect = options.bOnlyIfChanged = TRUE;
		}
	}

	//Add our module to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM)MODULE, 0);

	// register for IcoLib
	if (!ServiceExists(MS_SKIN2_ADDICON))
	{
		hLibIcons[0] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_LIST));
		hLibIcons[1] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_URL));
		hLibIcons[2] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_HIST));
		hLibIcons[3] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_EXT));
		hLibIcons[4] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_POPUP));
		hLibIcons[5] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_NOPOPUP));
	}
	else
	{
		SKINICONDESC sid;
		char ModuleName[1024];
		char *szIconName[ICONCOUNT] = {
			TranslateT("List Status Messages"),
			TranslateT("Go To URL in Status Message"),
			TranslateT("Status Message History"),
			TranslateT("Log To File"),
			TranslateT("PopUps Enabled"),
			TranslateT("PopUps Disabled")
		};
		int iIconId[ICONCOUNT] = {IDI_LIST, IDI_URL, IDI_HIST, IDI_EXT, IDI_POPUP, IDI_NOPOPUP};
		int i;
		ZeroMemory(&sid, sizeof(sid));
		sid.cbSize = sizeof(sid);
		sid.pszSection = TranslateT("Status Message Change Notify");
		GetModuleFileName(hInst, ModuleName, sizeof(ModuleName));
		sid.pszDefaultFile = ModuleName;
		for (i = 0; i < ICONCOUNT; i++)
		{
			sid.pszName = szIconId[i];
			sid.pszDescription = szIconName[i];
			sid.iDefaultIndex = -iIconId[i];
//			sid.hDefaultIcon = LoadIcon(hInst, MAKEINTRESOURCE(iIconId[i]));
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			hLibIcons[i] = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[i]);
		}
		hHookSkinIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, HookedSkinIconsChanged);
	}

	UpdateMenu(options.bDisablePopUps);

	hTopToolbarLoaded = HookEvent(ME_TTB_MODULELOADED, Create_TopToolbarShowList);

	hUserInfoInitialise = HookEvent(ME_USERINFO_INITIALISE, HookedUserInfo);

	return 0;
}

/*
Called when an options dialog has to be created
*/
static int HookedOptions(WPARAM wParam, LPARAM lParam) {
	OptionsAdd(wParam);
	return 0;
}

/*
Called when User Info dialog has to be created
*/

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst = hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) {
/*	if (mirandaVersion < PLUGIN_MAKE_VERSION(0,3,2,0))
	{
		//not translatable
		MessageBox(NULL, "StatusMsgChangeNotify requires Miranda 0.3.2.0 or later to run.", "StatusMsgChangeNotify", MB_OK|MB_ICONERROR);
		return NULL;
	}
*/	return &pluginInfo;
}

/*
Load
*/
int __declspec(dllexport) Load(PLUGINLINK *link) {
	CLISTMENUITEM mi;

	INITCOMMONCONTROLSEX icex;
	//Ensure that the common control DLL is loaded.
	ZeroMemory(&icex, sizeof(icex));
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	pluginLink = link;

	OptionsRead();
	options.hInst = hInst;
	hWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	ZeroMemory(&mi, sizeof(mi));

	//Main menu item - toggle popups
	CreateServiceFunction(MS_SMCN_POPUPS, MenuItemCmd_PopUps);
	mi.cbSize = sizeof(mi);
	mi.flags = 0;
	mi.position = 0;
	mi.pszPopupName = TranslateT("PopUps");
	mi.pszService = MS_SMCN_POPUPS;
	hEnableDisableMenu = CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
	//List contacts with status message menu item
	CreateServiceFunction(MS_SMCN_LIST, MenuItemCmd_ShowList);
	mi.position = 500021000;
	mi.pszName = TranslateT("List Contacts with Status Message");
	mi.pszPopupName = NULL;
	mi.pszService = MS_SMCN_LIST;
	hShowListMenu = CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
	//Contact menu item - go to url
	CreateServiceFunction(MS_SMCN_GOTOURL, MenuItemCmd_GoToURL);
	mi.position = -2000004000;
	mi.pszName = TranslateT("Go To URL in Status Message");
	mi.pszService = MS_SMCN_GOTOURL;
	mi.pszContactOwner = NULL;
	hGoToURLMenu = CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	//Contact menu item - status msg history
	CreateServiceFunction(MS_SMCN_HIST, MenuItemCmd_ShowHistory);
	mi.position = 1000090200;
	mi.pszName = TranslateT("View Status Message History");
	mi.pszService = MS_SMCN_HIST;
	hContactMenu = CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	//Contact menu item - disable/enable popups
	CreateServiceFunction(MS_SMCN_CPOPUP, MenuItemCmd_ContactPopUps);
	mi.position = 1000100010;
	mi.pszName = TranslateT("Disable SMCN PopUps");
	mi.pszService = MS_SMCN_CPOPUP;
	hContactPopUpsMenu = CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	//Hooks
	hHookedInit = HookEvent(ME_SYSTEM_MODULESLOADED, HookedInit);
	hHookedOpt = HookEvent(ME_OPT_INITIALISE, HookedOptions);
	//Protocol status: we use this for "PopUps on Connection".
	hProtoAck = HookEvent(ME_PROTO_ACK, ProtoAck);
	//Prebuild contact menu
	hPreBuildCMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);
	//Add sound event
	SkinAddNewSound("statusmsgchanged", TranslateT("Status Message Changed"), "");

	//Window needed for popup commands
	hPopupWindow = CreateWindowEx(WS_EX_TOOLWINDOW, _T("static"), _T("smcn_PopupWindow"), 0,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP,
		NULL, hInst, NULL);
	SetWindowLong(hPopupWindow, GWL_WNDPROC, (LONG)(WNDPROC)PopupWndProc);

	return 0;
}

int __declspec(dllexport) Unload(void) {
	if (!ServiceExists(MS_SKIN2_ADDICON))
	{
		int i;
		for (i = 0; i < ICONCOUNT; i++)
		{
			DestroyIcon(hLibIcons[i]);
		}
		UnhookEvent(hHookSkinIconsChanged);
	}
	if (hTopToolbarLoaded) UnhookEvent(hTopToolbarLoaded);
	UnhookEvent(hContactSettingChanged);
	UnhookEvent(hHookedOpt);
	UnhookEvent(hHookedInit);
	UnhookEvent(hProtoAck);
	UnhookEvent(hPreBuildCMenu);
	UnhookEvent(hUserInfoInitialise);
	return 0;
}
