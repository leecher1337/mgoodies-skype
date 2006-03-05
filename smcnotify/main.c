/*
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

// constants for the icons
char *szIconId[ICONCOUNT] = {"scn_list", "scn_url", "scn_hist", "scn_ext", "scn_int", "scn_pop", "scn_pope", "scn_popd"};

char szStatusMsgURL[1024];

void StatusMsgChanged(WPARAM wParam, DBCONTACTWRITESETTING* cws);
BOOL CheckStatusMessage(HANDLE hContact, char str[255]);
void InitStatusUpdate();
void EndStatusUpdate();


PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"StatusMessageChangeNotify",
	PLUGIN_MAKE_VERSION(0,0,3,0),
	"Notify you when someone changes his/her status message",
	"Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci",
	"",
	"",
	"http://miranda-im.org/",
	0,	//not transient
	0	//doesn't replace anything built-in
};

void UpdateMenu(BOOL bState) {
	CLISTMENUITEM mi;
	ZeroMemory(&mi, sizeof(mi));
	// main menu item
	mi.cbSize = sizeof(mi);
	if (bState) {
		mi.pszName = Translate("Enable &status message change notification");
		mi.hIcon = ICO_POPUP_D;
	}
	else {
		mi.pszName = Translate("Disable &status message change notification");
		mi.hIcon = ICO_POPUP_E;
	}
	options.bDisablePopUps = bState;
	mi.flags = CMIM_ICON | CMIM_NAME;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hEnableDisableMenu, (LPARAM)&mi);
	// contact menu item
	mi.flags = CMIM_ICON;
	mi.hIcon = ICO_MENU;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hContactMenu, (LPARAM)&mi);
	mi.hIcon = ICO_URL;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hGoToURLMenu, (LPARAM)&mi);
	// update main menu icon
	mi.hIcon = ICO_LIST;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hShowListMenu, (LPARAM)&mi);
}


/*
Return TRUE is smcn is enabled for this protocol

wParam: protocol as string
*/
static int ServiceEnabledForProtocol(WPARAM wParam, LPARAM lParam) {
	return HasToGetStatusMsgForProtocol((char *) wParam);
}


/*
Return TRUE is smcn is enabled for this user and this protocol (smcn can be disabled per user,
if protocol is enabled)

wParam: protocol as string
lParam: hContact
*/
static int ServiceEnabledForUser(WPARAM wParam, LPARAM lParam) {
	return HasToGetStatusMsgForProtocol((char *) wParam) && !HasToIgnoreContact((HANDLE) lParam, (char *) wParam);
}


static int MenuItemCmd(WPARAM wParam, LPARAM lParam) {
	UpdateMenu(!options.bDisablePopUps);
	DBWriteContactSettingByte(NULL, MODULE_NAME, OPT_DISPOPUPS, (BYTE)options.bDisablePopUps);
	return 0;
}

static int ContactMenuItemCmd(WPARAM wParam, LPARAM lParam) {
	ShowHistory((HANDLE)wParam);
	return 0;
}

static int ShowListMenuItemCmd(WPARAM wParam, LPARAM lParam) {
	ShowList();
	if(hTopToolbarButtonShowList != NULL)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTopToolbarButtonShowList, TTBST_RELEASED);
	return 0;
}

static int GoToURLContactMenuItemCmd(WPARAM wParam, LPARAM lParam) {
	CallService(MS_UTILS_OPENURL, 1, (LPARAM)&szStatusMsgURL);
	return 0;
}

static int ContactPopUpsMenuItemCmd(WPARAM wParam, LPARAM lParam) {
	BOOL state = (BOOL)DBGetContactSettingByte((HANDLE)wParam, MODULE_NAME, "Popup", 1);
	DBWriteContactSettingByte((HANDLE)wParam, MODULE_NAME, "Popup", (BYTE)!state);
	return 0;
}

static int ContactIcqCheckMenuItemCmd(WPARAM wParam, LPARAM lParam){
	BOOL state = (BOOL)DBGetContactSettingByte((HANDLE)wParam, MODULE_NAME, OPT_CONTACT_GETMSG, DEFAULT_ICQCHECK);
	DBWriteContactSettingByte((HANDLE)wParam, MODULE_NAME, OPT_CONTACT_GETMSG, (BYTE)!state);
	return 0;
}

static int PreBuildContactMenu(WPARAM wParam,LPARAM lParam) {
	CLISTMENUITEM clmi;
	char str[2048], *p;
	int c;
	ZeroMemory(&clmi, sizeof(clmi));
	clmi.cbSize = sizeof(clmi);
	clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
	if (CheckStatusMessage((HANDLE)wParam, str)) {
		p = strstr(str, "www.");
		if (p == NULL) p = strstr(str, "http://");
		if (p != NULL) {
			for (c = 0; p[c]!='\n' && p[c]!='\r' && p[c]!='\t' && p[c]!=' ' && p[c]!='\0'; c++);
			lstrcpyn(szStatusMsgURL, p, min(c + 1, 1024));
			clmi.flags = CMIM_FLAGS;
		}
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hGoToURLMenu, (LPARAM)&clmi);
	// contact smcn popups disable/enable
	if (!options.bHideSettingsMenu) {
		clmi.flags = CMIM_FLAGS | CMIM_ICON | CMIM_NAME;
		if (DBGetContactSettingByte((HANDLE)wParam, MODULE_NAME, "Popup", 1) == 0) {
			clmi.pszName = Translate("Enable SMCN PopUps");
			clmi.hIcon = ICO_POPUP_D;
		}
		else {
			clmi.pszName = Translate("Disable SMCN PopUps");
			clmi.hIcon = ICO_POPUP_E;
		}
	}
	else clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hContactPopUpsMenu, (LPARAM)&clmi);
	// retrieve contact's status message disable/enable
	p = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (p && HasToGetStatusMsgForProtocol(p) && !options.bHideSettingsMenu) 
	{
		clmi.flags = CMIM_FLAGS | CMIM_ICON | CMIM_NAME;
		if (DBGetContactSettingByte((HANDLE)wParam, MODULE_NAME, OPT_CONTACT_GETMSG, DEFAULT_ICQCHECK) == 0) {
			clmi.pszName = Translate("Enable Status Message Check");
			clmi.hIcon = LoadSkinnedProtoIcon(p, ID_STATUS_DND);
		}
		else {
			clmi.pszName = Translate("Disable Status Message Check");
			clmi.hIcon = LoadSkinnedProtoIcon(p, ID_STATUS_AWAY);
		}
	}
	else clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hContactIcqCheckMenu,(LPARAM)&clmi);
	return 0;
}

static int Create_TopToolbarShowList(WPARAM wParam, LPARAM lParam) {
	//DBVARIANT dbv;
	//char path[512];
	if (ServiceExists(MS_TTB_ADDBUTTON)) {
		TTBButton ttbb;
		ZeroMemory(&ttbb, sizeof(ttbb));
		ttbb.cbSize = sizeof(ttbb);
	//	ttbb.dwFlags = 0;
	//	strcpy(path, Translate("List Contacts with Status Message"));
	//	strcat(path, "_BmpUp");
	//	if(!DBGetContactSetting(NULL, "TopToolBar", path, &dbv)) {
	//		if (!strcmp(dbv.pszVal, "")) ttbb.hbBitmapUp = ttbb.hbBitmapDown = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LIST));
	//		else ttbb.hbBitmapUp = ttbb.hbBitmapDown = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)dbv.pszVal);
	//		DBFreeVariant(&dbv);
	//	}
	//	else {
	//		ttbb.hbBitmapUp = ttbb.hbBitmapDown = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LIST));
	//	}
		ttbb.hbBitmapUp = ttbb.hbBitmapDown = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LIST));
		ttbb.dwFlags = TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP;
		ttbb.pszServiceUp = ttbb.pszServiceDown = MS_SMCN_LIST;
		ttbb.name = Translate("List Contacts with Status Message");
		hTopToolbarButtonShowList = (HANDLE)CallService(MS_TTB_ADDBUTTON, (WPARAM)&ttbb, 0);
		if((int)hTopToolbarButtonShowList == -1) {
			hTopToolbarButtonShowList = NULL;
			return 1;
		}
		CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM((WORD)TTBO_TIPNAME, (WORD)hTopToolbarButtonShowList),
			(LPARAM)Translate("List Contacts with Status Message"));
	}
	return 0;
}

//---Called when contact's settings has changed
static int ContactSettingChanged(WPARAM wParam, LPARAM lParam) {
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	HANDLE hContact = (HANDLE)wParam;

	//We need to exit from this function as fast as possible, so we'll first check
	//if the setting regards us (exit) or if it's not related to status (exit).
	//If we're there, it means it's a status change we *could* notify, so let's see
	//if we are ignoring that event. This means if *Miranda* ignores it (ignores every
	//notification) and then if *We* ignore the event.
	//Lastly, we check if we're offline: this will happen only once per connection, while
	//the checks above will happen more frequently.

	// The setting must belong to a contact different from myself (NULL) and needs
	// to be related to the status. Otherwise, exit as fast as possible.
	if (hContact == NULL) 
			return 0;

	// Check if changed status to get status message and write it to DB
	if (!lstrcmp(cws->szSetting, "Status")) 
	{
		char *lpzProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0);
		if (lpzProto != NULL && !lstrcmp(cws->szModule, lpzProto))
		{
			if (PoolCheckProtocol(lpzProto))
			{
				// Mark time to ignore changes from this contact
				DBWriteContactSettingDword(hContact, MODULE_NAME, szProtocol, GetTickCount());

				if (options.pool_check_on_status_change && PoolCheckContact(hContact))
					PoolStatusChangeAddContact(hContact, lpzProto);
			}
		}
	}
	// Exit if setting changed is not status message
	else if (lstrcmp(cws->szSetting, "StatusMsg") 
			&& lstrcmp(cws->szSetting, "StatusDescr") 
			&& lstrcmp(cws->szSetting, "YMsg"))
	{
		StatusMsgChanged(wParam, cws);
	}

	return 0;
}

//---Called when icon changed from the option
static int HookedSkinIconsChanged(WPARAM wParam,LPARAM lParam) {
	int i;
	for (i = 0; i < ICONCOUNT; i++) {
		hLibIcons[i] = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[i]);
	}
	// update menu icons
	UpdateMenu(options.bDisablePopUps);
	return 0;
}

//---Called when all the modules are loaded
static int HookedInit(WPARAM wParam, LPARAM lParam) {
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	// first time using this version - turn off popup and external logging for ignored contacts
	if ((int)DBGetContactSettingWord(NULL, MODULE_NAME, "Ver", 1) < 3) {
		HANDLE hContact;
		// start looking for other weather stations
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact) {
			if (CallService(MS_IGNORE_ISIGNORED, (WPARAM)hContact, IGNOREEVENT_USERONLINE) != 0) {
				DBWriteContactSettingByte(hContact, MODULE_NAME, "Popup", FALSE);
				DBWriteContactSettingByte(hContact, MODULE_NAME, "External", FALSE);
				DBWriteContactSettingByte(hContact, MODULE_NAME, "Internal", TRUE);
			}
			else {
				DBWriteContactSettingByte(hContact, MODULE_NAME, "Popup", TRUE);
				DBWriteContactSettingByte(hContact, MODULE_NAME, "External", TRUE);
				DBWriteContactSettingByte(hContact, MODULE_NAME, "Internal", TRUE);
			}
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}
		DBWriteContactSettingWord(NULL, MODULE_NAME, "Ver", 3);
	}
	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM)MODULE_NAME, 0);
	// register for IcoLib
	if (!ServiceExists(MS_SKIN2_ADDICON)) {
		hLibIcons[0] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_LIST));
		hLibIcons[1] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_URL));
		hLibIcons[2] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_HIST));
		hLibIcons[3] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_EXT));
		hLibIcons[4] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_INT));
		hLibIcons[5] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_POPUP));
		hLibIcons[6] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_POPUP));
		hLibIcons[7] = LoadIcon(hInst,MAKEINTRESOURCE(IDI_NOPOPUP));
	}
	else {
		SKINICONDESC sid;
		char ModuleName[1024];
		char *szIconName[ICONCOUNT] = {Translate("List Status Messages"), Translate("Go To URL in Status Message"), Translate("Contact Menu Icon"), Translate("External History"), Translate("Internal History"), Translate("PopUp Notify"), Translate("PopUp Enabled"), Translate("PopUp Disabled")};
		int iIconId[ICONCOUNT] = {-IDI_LIST, -IDI_URL, -IDI_HIST, -IDI_EXT, -IDI_INT, -IDI_POPUP, -IDI_POPUP, -IDI_NOPOPUP};
		int i;
		ZeroMemory(&sid, sizeof(sid));
		sid.cbSize = sizeof(sid);
		sid.pszSection = Translate("Status Message Change Notify");
		GetModuleFileName(hInst, ModuleName, sizeof(ModuleName));
		sid.pszDefaultFile = ModuleName;
		for (i = 0; i < ICONCOUNT; i++) {
			sid.pszName = szIconId[i];
			sid.pszDescription = szIconName[i];
			sid.iDefaultIndex = iIconId[i];
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			hLibIcons[i] = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[i]);
		}
		hHookSkinIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, HookedSkinIconsChanged);
	}
	UpdateMenu(options.bDisablePopUps);
	hTopToolbarLoaded = HookEvent(ME_TTB_MODULELOADED, Create_TopToolbarShowList);
	return 0;
}

//---Called when an options dialog has to be created
static int HookedOptions(WPARAM wParam, LPARAM lParam) {
	OptionsAdd(wParam);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst = hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) {
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0,3,2,0)) {
		// not translatable
		MessageBox(NULL, "StatusMsgChangeNotify requires Miranda 0.3.2.0 or later to run.", "StatusMsgChangeNotify", MB_OK|MB_ICONERROR);
		return NULL;
	}
	return &pluginInfo;
}

int __declspec(dllexport) Load(PLUGINLINK *link) {
	CLISTMENUITEM mi;

	INITCOMMONCONTROLSEX icex;
	// Ensure that the common control DLL is loaded.
	ZeroMemory(&icex, sizeof(icex));
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	pluginLink = link;

	// Init thread
	InitStatusUpdate();
	OptionsRead();
	options.hInst = hInst;
	hWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	ZeroMemory(&mi, sizeof(mi));
	// main menu item - toggle popups
	CreateServiceFunction(MS_SMCN_TGLPOPUP, MenuItemCmd);
	mi.cbSize = sizeof(mi);
	mi.flags = 0;
	mi.position = 0;
	mi.pszPopupName = Translate("PopUps");
	mi.pszService = MS_SMCN_TGLPOPUP;
	hEnableDisableMenu = CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
	// list contacts with status message menu item
	CreateServiceFunction(MS_SMCN_LIST, ShowListMenuItemCmd);
	mi.position = 500021000;
	mi.pszName = Translate("List Contacts with Status Message");
	mi.pszPopupName = NULL;
	mi.pszService = MS_SMCN_LIST;
	hShowListMenu = CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
	// contact menu item - go to url
	CreateServiceFunction(MS_SMCN_GOTOURL, GoToURLContactMenuItemCmd);
	mi.position = -2000004000;
	mi.pszName = Translate("Go To URL in Status Message");
	mi.pszService = MS_SMCN_GOTOURL;
	mi.pszContactOwner = NULL;
	hGoToURLMenu = CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	// contact menu item - status msg history
	CreateServiceFunction(MS_SMCN_HIST, ContactMenuItemCmd);
	mi.position = 1000090200;
	mi.pszName = Translate("View Status Message History");
	mi.pszService = MS_SMCN_HIST;
	hContactMenu = CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	// contact menu item - disable/enable popups
	CreateServiceFunction(MS_SMCN_CPOPUP, ContactPopUpsMenuItemCmd);
	mi.position = 1000100010;
	mi.pszName = Translate("Disable SMCN PopUps");
	mi.pszService = MS_SMCN_CPOPUP;
	hContactPopUpsMenu = CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	// contact menu item - disable/enable ICQ status message check
	CreateServiceFunction(MS_SMCN_CICQ, ContactIcqCheckMenuItemCmd);
	mi.position = 1000100020;
	mi.pszName = Translate("Disable Status Message Check");
	mi.pszService = MS_SMCN_CICQ;
	hContactIcqCheckMenu = CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	// hooks
	hHookedInit = HookEvent(ME_SYSTEM_MODULESLOADED, HookedInit);
	hHookedOpt = HookEvent(ME_OPT_INITIALISE, HookedOptions);
	// Protocol status: we use this for "PopUps on Connection".
	hProtoAck = HookEvent(ME_PROTO_ACK, ProtoAck);
	// prebuild contact menu
	hPreBuildCMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);
	// add sound event
	SkinAddNewSound("statusmsgchanged", Translate("Status Message Changed"), "");
	// window needed for the ICQ status msg check timer routines.
	hTimerWindow = CreateWindowEx(WS_EX_TOOLWINDOW, "static", "smcn_TimerWindow",0,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP,
		NULL, hInst, NULL);
	// window needed for popup commands
	hPopupWindow = CreateWindowEx(WS_EX_TOOLWINDOW, "static", "smcn_PopupWindow", 0,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP,
		NULL, hInst, NULL);
	SetWindowLong(hPopupWindow, GWL_WNDPROC, (LONG)(WNDPROC)PopupWndProc);
	return 0;
}

int __declspec(dllexport) Unload(void) {
	EndStatusUpdate();
	if (!ServiceExists(MS_SKIN2_ADDICON)) {
		int i;
		for (i = 0; i < ICONCOUNT; i++) { DestroyIcon(hLibIcons[i]); }
		UnhookEvent(hHookSkinIconsChanged);
	}
	if(hTopToolbarLoaded) UnhookEvent(hTopToolbarLoaded);
	UnhookEvent(hContactSettingChanged);
	UnhookEvent(hHookedOpt);
	UnhookEvent(hHookedInit);
	UnhookEvent(hProtoAck);
	UnhookEvent(hPreBuildCMenu);
	return 0;
}
