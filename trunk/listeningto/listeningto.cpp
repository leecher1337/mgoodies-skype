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

// Service called by the main menu
#define MS_LISTENINGTO_MAINMENU		"ListeningTo/MainMenu"

// Service called by toptoolbar
#define MS_LISTENINGTO_TTB		"ListeningTo/TopToolBar"


PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
#ifdef UNICODE
	"ListeningTo (Unicode)",
#else
	"ListeningTo",
#endif
	PLUGIN_MAKE_VERSION(0,1,0,8),
	"Handle listening information to/for contacts",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2006 Ricardo Pescuma Domenecci",
	"http://miranda-im.org/",
	0,	//not transient
	0	//doesn't replace anything built-in
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;

static HANDLE hModulesLoaded = NULL;
static HANDLE hPreShutdownHook = NULL;
static HANDLE hTopToolBarLoadedHook = NULL;
static HANDLE hClistExtraListRebuildHook = NULL;
static HANDLE hSettingChangedHook = NULL;
static HANDLE hEnableStateChangedEvent = NULL;

static HANDLE hTTB = NULL;
static char *metacontacts_proto = NULL;
static BOOL loaded = FALSE;
static UINT hTimer = 0;
static HANDLE hExtraImage = NULL;

struct MenuItemInfo
{
	char *proto;
	HANDLE hMenu;

} *menu_itens = NULL;
int menu_itens_num = 0;


int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);
int PreBuildContactMenu(WPARAM wParam,LPARAM lParam);
int TopToolBarLoaded(WPARAM wParam, LPARAM lParam);
int ClistExtraListRebuild(WPARAM wParam, LPARAM lParam);
int SettingChanged(WPARAM wParam,LPARAM lParam);

int MainMenuClicked(WPARAM wParam, LPARAM lParam);
BOOL ListeningToEnabled(char *proto);
int ListeningToEnabled(WPARAM wParam, LPARAM lParam);
int EnableListeningTo(WPARAM wParam,LPARAM lParam);
int GetTextFormat(WPARAM wParam,LPARAM lParam);
int GetParsedFormat(WPARAM wParam,LPARAM lParam);
int GetOverrideContactOption(WPARAM wParam,LPARAM lParam);
void SetExtraIcon(HANDLE hContact, BOOL set);


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

	CoInitialize(NULL);

	CreateServiceFunction(MS_LISTENINGTO_ENABLED, ListeningToEnabled);
	CreateServiceFunction(MS_LISTENINGTO_ENABLE, EnableListeningTo);
	CreateServiceFunction(MS_LISTENINGTO_GETTEXTFORMAT, GetTextFormat);
	CreateServiceFunction(MS_LISTENINGTO_GETPARSEDTEXT, GetParsedFormat);
	CreateServiceFunction(MS_LISTENINGTO_OVERRIDECONTACTOPTION, GetOverrideContactOption);
	CreateServiceFunction(MS_LISTENINGTO_MAINMENU, MainMenuClicked);
	
	// hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hPreShutdownHook = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	hSettingChangedHook = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);

	hEnableStateChangedEvent = CreateHookableEvent(ME_LISTENINGTO_ENABLE_STATE_CHANGED);

	InitMusic();
	InitOptions();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	if (menu_itens != NULL) 
		free(menu_itens);

	CoUninitialize();

	return 0;
}


__inline static int ProtoServiceExists(const char *szModule, const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}

// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	EnableDisablePlayers();

	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME, 0);

	hClistExtraListRebuildHook = HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, ClistExtraListRebuild);

    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://eth0.dk/files/pescuma/listeningto_version.txt";
		upd.szBetaChangelogURL = "http://eth0.dk/files/pescuma/listeningto_changelog.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"ListeningTo ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://eth0.dk/files/pescuma/listeningtoW.zip";
#else
		upd.szBetaUpdateURL = "http://eth0.dk/files/pescuma/listeningto.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	if (opts.enable_menu_item)
	{
		CLISTMENUITEM mi = {0};
		mi.cbSize = sizeof(mi);
		mi.popupPosition = 500080000;
		mi.pszPopupName = "Listening to";
		mi.position = 0;
		mi.pszService = MS_LISTENINGTO_MAINMENU;

		int allocated = 10;
		menu_itens = (MenuItemInfo *) malloc(allocated * sizeof(MenuItemInfo));
		
		// Add all protos

		mi.pszName = Translate("Send to all protocols");
		mi.flags = ListeningToEnabled(NULL) ? CMIF_CHECKED : 0;
		menu_itens[menu_itens_num].hMenu = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
		menu_itens[menu_itens_num].proto = NULL;

		// clist classic :(
		mi.flags |= CMIM_FLAGS;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) menu_itens[menu_itens_num].hMenu, (LPARAM) &mi);


		mi.popupPosition++;
		menu_itens_num++;

		mi.position = 100000;

		// Add each proto

		PROTOCOLDESCRIPTOR **protos;
		int count;
		CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

		for (int i = 0; i < count; i++)
		{
			if (protos[i]->type != PROTOTYPE_PROTOCOL)
				continue;
			
			if (!ProtoServiceExists(protos[i]->szName, PS_SET_LISTENINGTO))
				continue;

			char name[128];
			CallProtoService(protos[i]->szName, PS_GETNAME, sizeof(name), (LPARAM)name);

			char text[256];
			mir_snprintf(text, MAX_REGS(text), Translate("Send to %s"), name);

			if (menu_itens_num >= allocated)
			{
				allocated += 10;
				menu_itens = (MenuItemInfo *) realloc(menu_itens, allocated * sizeof(MenuItemInfo));
			}

			mi.pszName = text;
			mi.flags = ListeningToEnabled(protos[i]->szName) ? CMIF_CHECKED : 0;

			menu_itens[menu_itens_num].hMenu = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
			menu_itens[menu_itens_num].proto = protos[i]->szName;

			// clist classic :(
			mi.flags |= CMIM_FLAGS;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) menu_itens[menu_itens_num].hMenu, (LPARAM) &mi);

			mi.position++;
			mi.popupPosition++;
			menu_itens_num++;
		}
	}

	StartTimer();

	hTopToolBarLoadedHook = HookEvent(ME_TTB_MODULELOADED, TopToolBarLoaded);

	loaded = TRUE;

	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	DeInitOptions();

	DestroyHookableEvent(hEnableStateChangedEvent);

	UnhookEvent(hModulesLoaded);
	UnhookEvent(hPreShutdownHook);
	if (hTopToolBarLoadedHook) UnhookEvent(hTopToolBarLoadedHook);
	UnhookEvent(hClistExtraListRebuildHook);
	UnhookEvent(hSettingChangedHook);

	if (hTimer != NULL)
		KillTimer(NULL, hTimer);

	FreeMusic();

	loaded = FALSE;

	return 0;
}


int TopToolBarClick(WPARAM wParam, LPARAM lParam)
{
	BOOL enabled = !ListeningToEnabled(NULL);

	EnableListeningTo(NULL, enabled);

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM) hTTB, (LPARAM) (enabled ? TTBST_PUSHED : TTBST_RELEASED));

	return 0;
}


// Toptoolbar hook to put an icon in the toolbar
int TopToolBarLoaded(WPARAM wParam, LPARAM lParam) 
{
	BOOL enabled = ListeningToEnabled(NULL);

	CreateServiceFunction(MS_LISTENINGTO_TTB, TopToolBarClick);

	TTBButton ttb = {0};
	ttb.cbSize = sizeof(ttb);
	ttb.hbBitmapUp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TTB_UP_DISABLED));
	ttb.hbBitmapDown = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TTB_UP_ENABLED));
	ttb.pszServiceUp = MS_LISTENINGTO_TTB;
	ttb.pszServiceDown = MS_LISTENINGTO_TTB;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | (enabled ? TTBBF_PUSHED : 0);
	ttb.name = Translate("Enable/Disable sending Listening To info (to all protocols)");
	
	hTTB = (HANDLE)CallService(MS_TTB_ADDBUTTON, (WPARAM)&ttb, 0);

	return 0;
}


int MainMenuClicked(WPARAM wParam, LPARAM lParam)
{
	if (!loaded)
		return -1;

	int pos = wParam == 0 ? 0 : wParam - 500080000;

	if (pos >= menu_itens_num || pos < 0)
		return 0;

	EnableListeningTo((WPARAM) menu_itens[pos].proto, (LPARAM) !ListeningToEnabled(menu_itens[pos].proto));
	return 0;
}


BOOL ListeningToEnabled(char *proto) 
{
	if (!opts.enable_sending)
		return FALSE;

	if (proto == NULL)
	{
		// Check all protocols
		BOOL enabled = TRUE;

		PROTOCOLDESCRIPTOR **protos;
		int count;
		CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

		for (int i = 0; i < count; i++)
		{
			if (protos[i]->type != PROTOTYPE_PROTOCOL)
				continue;
			
			if (!ProtoServiceExists(protos[i]->szName, PS_SET_LISTENINGTO))
				continue;

			if (!ListeningToEnabled(protos[i]->szName))
			{
				enabled = FALSE;
				break;
			}
		}

		return enabled;
	}
	else
	{
		char setting[256];
		mir_snprintf(setting, sizeof(setting), "%sEnabled", proto);
		return (BOOL) DBGetContactSettingByte(NULL, MODULE_NAME, setting, FALSE);
	}
}


int ListeningToEnabled(WPARAM wParam, LPARAM lParam) 
{
	if (!loaded)
		return -1;

	return ListeningToEnabled((char *)wParam) ;
}


void SetListeningInfo(char *proto, BOOL set)
{
	LISTENINGTOINFO lti = {0};

	if (!opts.enable_sending || !set || !GetListeningInfo(&lti))
	{
		CallProtoService(proto, PS_SET_LISTENINGTO, 0, NULL);
	}
	else
	{
		CallProtoService(proto, PS_SET_LISTENINGTO, 0, (LPARAM) &lti);
	}
}


int EnableListeningTo(WPARAM wParam,LPARAM lParam) 
{
	if (!loaded)
		return -1;

	char *proto = (char *)wParam;

	if (proto == NULL)
	{
		// For all protocols
		BOOL enabled = TRUE;

		PROTOCOLDESCRIPTOR **protos;
		int count;
		CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

		for (int i = 0; i < count; i++)
		{
			if (protos[i]->type != PROTOTYPE_PROTOCOL)
				continue;
			
			if (!ProtoServiceExists(protos[i]->szName, PS_SET_LISTENINGTO))
				continue;

			EnableListeningTo((WPARAM) protos[i]->szName, lParam);
		}
	}
	else
	{
		char setting[256];
		mir_snprintf(setting, sizeof(setting), "%sEnabled", proto);
		DBWriteContactSettingByte(NULL, MODULE_NAME, setting, (BOOL) lParam);

		if (menu_itens != NULL)
		{
			// Modify menu info
			for (int i = 1; i < menu_itens_num; i++)
			{
				if (strcmp(proto, menu_itens[i].proto) == 0)
				{
					CLISTMENUITEM clmi = {0};
					clmi.cbSize = sizeof(clmi);
					clmi.flags = CMIM_FLAGS | (lParam ? CMIF_CHECKED : 0);
					CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) menu_itens[i].hMenu, (LPARAM) &clmi);

					SetListeningInfo(proto, lParam);
					break;
				}
			}

			BOOL enabled = (lParam && ListeningToEnabled(NULL));

			CLISTMENUITEM clmi = {0};
			clmi.cbSize = sizeof(clmi);
			clmi.flags = CMIM_FLAGS | (enabled ? CMIF_CHECKED : 0);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) menu_itens[0].hMenu, (LPARAM) &clmi);

			if (hTTB != NULL)
				CallService(MS_TTB_SETBUTTONSTATE, (WPARAM) hTTB, (LPARAM) (enabled ? TTBST_PUSHED : TTBST_RELEASED));
		}
	}

	StartTimer();

	NotifyEventHooks(hEnableStateChangedEvent, wParam, lParam);

	return 0;
}


int GetTextFormat(WPARAM wParam,LPARAM lParam) 
{
	if (!loaded)
		return NULL;

	return (int) mir_dupT(opts.templ);
}


int GetParsedFormat(WPARAM wParam,LPARAM lParam) 
{
	if (!loaded)
		return NULL;

	LISTENINGTOINFO *lti = (LISTENINGTOINFO *) lParam;

	if (lti == NULL)
		return NULL;

	TCHAR *ret = mir_dupT( opts.templ );

	TCHAR *fr[] = { 
		_T("%artist%"), lti->ptszArtist,
		_T("%album%"), lti->ptszAlbum,
		_T("%title%"), lti->ptszTitle,
		_T("%track%"), lti->ptszTrack,
		_T("%year%"), lti->ptszYear,
		_T("%genre%"), lti->ptszGenre,
		_T("%length%"), lti->ptszLength,
		_T("%player%"), lti->ptszPlayer,
		_T("%type%"), lti->ptszType
	};

	for (int i = 0; i < MAX_REGS(fr); i+=2) {
		TCHAR *find = fr[i];
		TCHAR *replace = fr[i+1] ? fr[i+1] : _T("");

		size_t len_find = lstrlen(find);
		size_t len_replace = lstrlen(replace);

		for (TCHAR *p = _tcsstr(ret, find); p != NULL; p = _tcsstr(p + len_replace, find)) {
			if (len_find < len_replace) {
				int pos = p - ret;
				ret = (TCHAR *) mir_realloc(ret, (lstrlen(ret) + len_replace - len_find + 1) * sizeof(TCHAR));
				p = ret + pos;
			}
			memmove(p + len_replace, p + len_find, (lstrlen(p + len_find) + 1) * sizeof(TCHAR));
			memmove(p, replace, len_replace * sizeof(TCHAR));
		}
	}

	return (int) ret;
}


int GetOverrideContactOption(WPARAM wParam,LPARAM lParam) 
{
	return opts.override_contact_template;
}


void SetListeningInfos(LISTENINGTOINFO *lti)
{
	PROTOCOLDESCRIPTOR **protos;
	int count;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

	for (int i = 0; i < count; i++)
	{
		if (protos[i]->type != PROTOTYPE_PROTOCOL)
			continue;
		
		if (!ProtoServiceExists(protos[i]->szName, PS_SET_LISTENINGTO))
			continue;

		if (ListeningToEnabled(protos[i]->szName))
		{
			CallProtoService(protos[i]->szName, PS_SET_LISTENINGTO, 0, (LPARAM) lti);
		}
	}
}

static void CALLBACK GetInfoTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	if (hTimer != NULL)
	{
		KillTimer(NULL, hTimer);
		hTimer = NULL;
	}

	if (!opts.enable_sending)
	{
		SetListeningInfos(NULL);
		return;
	}

	LISTENINGTOINFO lti = {0};

	int changed = ChangedListeningInfo();
	if (changed > 0)
	{
		// Get new info
		if (!GetListeningInfo(&lti))
			changed = -1;
	}

	// Set it
	if (changed < 0)
		SetListeningInfos(NULL);
	else if (changed > 0)
		SetListeningInfos(&lti);

	StartTimer();
}

void StartTimer()
{
	// See if any protocol want Listening info
	BOOL want = FALSE;

	if (opts.enable_sending)
	{
		if (!players[WATRACK]->enabled)
		{
			// See if any player needs it
			BOOL needPoll = FALSE;
			int i;
			for (i = FIRST_PLAYER; i < NUM_PLAYERS; i++)
			{
				if (players[i]->needPoll)
				{
					needPoll = TRUE;
					break;
				}
			}

			if (needPoll)
			{
				// Now see protocols
				PROTOCOLDESCRIPTOR **protos;
				int count;
				CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

				for (i = 0; i < count; i++)
				{
					if (protos[i]->type != PROTOTYPE_PROTOCOL)
						continue;
					
					if (!ProtoServiceExists(protos[i]->szName, PS_SET_LISTENINGTO))
						continue;

					if (ListeningToEnabled(protos[i]->szName))
					{
						want = TRUE;
						break;
					}
				}
			}
		}
	}

	if (want)
	{
		if (hTimer == NULL)
			hTimer = SetTimer(NULL, NULL, opts.time_to_pool * 1000, GetInfoTimer);
	}
	else
	{
		if (hTimer != NULL)
		{
			KillTimer(NULL, hTimer);
			hTimer = NULL;

			// To be sure that no one was left behind
			SetListeningInfos(NULL);
		}
	}
}

void HasNewListeningInfo()
{
	if (hTimer != NULL)
	{
		KillTimer(NULL, hTimer);
		hTimer = NULL;
	}

	hTimer = SetTimer(NULL, NULL, 100, GetInfoTimer);
}


int ClistExtraListRebuild(WPARAM wParam, LPARAM lParam)
{
	hExtraImage = (HANDLE) CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM) LoadIcon(hInst, MAKEINTRESOURCE(IDI_LISTENINGTO)), 0);
	return 0;
}

void SetExtraIcon(HANDLE hContact, BOOL set)
{
	if (opts.show_adv_icon && hExtraImage != NULL)
	{
		IconExtraColumn iec;
		iec.cbSize = sizeof(iec);
		iec.hImage = set ? hExtraImage : (HANDLE)-1;
		if (opts.adv_icon_slot < 2)
		{
			iec.ColumnType = opts.adv_icon_slot + EXTRA_ICON_ADV1;
		}
		else 
		{
			int first = CallService(MS_CLUI_GETCAPS, 0, CLUIF2_USEREXTRASTART);
			iec.ColumnType = opts.adv_icon_slot - 2 + first;
		}

		CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM)hContact, (LPARAM)&iec);
	}
}

int SettingChanged(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (strcmp(cws->szSetting, "ListeningTo") != 0)
		return 0;

	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (proto == NULL)
		return 0;

	if (strcmp(cws->szModule, proto) != 0)
		return 0;

	if (cws->value.type == DBVT_DELETED)
		SetExtraIcon(hContact, FALSE);
	else
		SetExtraIcon(hContact, TRUE);

	return 0;
}
