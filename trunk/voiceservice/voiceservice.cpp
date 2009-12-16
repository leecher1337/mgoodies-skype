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


PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
#ifdef UNICODE
	"Voice Service (Unicode)",
#else
	"Voice Service",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,8),
	"Provide services for protocols that support voice calls",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2007 Ricardo Pescuma Domenecci",
	"http://pescuma.mirandaim.ru/miranda/voiceservice",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
#ifdef UNICODE
	{ 0x1bfc449d, 0x8f6f, 0x4080, { 0x8f, 0x35, 0xf9, 0x40, 0xb3, 0xde, 0x12, 0x84 } } // {1BFC449D-8F6F-4080-8F35-F940B3DE1284}
#else
	{ 0x1bbe5b21, 0x238d, 0x4cbc, { 0xaf, 0xb8, 0xe, 0xef, 0xab, 0x1b, 0xf2, 0x69 } } // {1BBE5B21-238D-4cbc-AFB8-0EEFAB1BF269}
#endif
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;
MM_INTERFACE mmi;
UTF8_INTERFACE utfi;

HANDLE hModulesLoaded = NULL;
HANDLE hPreShutdownHook = NULL;
HANDLE hIconsChanged = NULL;
HANDLE hPreBuildContactMenu = NULL;

static HANDLE hCMCall = NULL;
static HANDLE hCMAnswer = NULL;
static HANDLE hCMDrop = NULL;
static HANDLE hCMHold = NULL;

char *metacontacts_proto = NULL;


static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int PreShutdown(WPARAM wParam, LPARAM lParam);
static int PreBuildContactMenu(WPARAM wParam,LPARAM lParam);

static int VoiceRegister(WPARAM wParam, LPARAM lParam);
static int VoiceUnregister(WPARAM wParam, LPARAM lParam);
static int VoiceState(WPARAM wParam, LPARAM lParam);
static int VoiceEndedCall(VOICE_CALL_INTERNAL *vc);

MODULE_INTERNAL * FindModule(const char *szModule);
VOICE_CALL_INTERNAL * FindVoiceCall(const char *szModule, const char *id, BOOL add);
VOICE_CALL_INTERNAL * FindVoiceCall(HANDLE hContact);

TCHAR *GetStateName(int state);
TCHAR *GetActionName(int action);
char *GetActionNameA(int state);

vector<MODULE_INTERNAL> modules;

vector<VOICE_CALL_INTERNAL *> calls;
CURRENT_CALL currentCall = {0};

HFONT fonts[NUM_FONTS] = {0};
COLORREF font_colors[NUM_FONTS] = {0};
int font_max_height;

COLORREF bkg_color = {0};
HBRUSH bk_brush = NULL;

HICON icons[NUM_ICONS] = {0};
char *icon_names[NUM_ICONS] = { "vc_talking", "vc_ringing", "vc_calling", "vc_on_hold", "vc_ended", 
					 "vca_call", "vca_answer" , "vca_hold", "vca_drop",
					 "vc_main"};

#define IDI_BASE IDI_TALKING 

static HANDLE HistoryLog(HANDLE hContact, TCHAR *log_text);

static int CListDblClick(WPARAM wParam,LPARAM lParam);

static int Service_CanCall(WPARAM wParam,LPARAM lParam);
static int Service_Call(WPARAM wParam,LPARAM lParam); 
static int CMAnswer(WPARAM wParam,LPARAM lParam); 
static int CMHold(WPARAM wParam,LPARAM lParam);
static int CMDrop(WPARAM wParam,LPARAM lParam);

static int IconsChanged(WPARAM wParam, LPARAM lParam);
static int ReloadFont(WPARAM wParam, LPARAM lParam);
static int ReloadColor(WPARAM wParam, LPARAM lParam);
static VOID CALLBACK ClearOldVoiceCalls(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

static BOOL CALLBACK DlgProcNewCall(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);



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


static const MUUID interfaces[] = { MIID_VOICESERVICE, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link) 
{
	pluginLink = link;

	CHECK_VERSION("Voice Service")

	// TODO Assert results here
	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);

	CreateServiceFunction(MS_VOICESERVICE_CLIST_DBLCLK, CListDblClick);
	CreateServiceFunction(MS_VOICESERVICE_STATE, VoiceState);
	CreateServiceFunction(MS_VOICESERVICE_REGISTER, VoiceRegister);
	CreateServiceFunction(MS_VOICESERVICE_UNREGISTER, VoiceUnregister);

	// Hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hPreShutdownHook = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	if (bk_brush != NULL)
		DeleteObject(bk_brush);

	return 0;
}

// Called when all the modules are loaded
static int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME, 0);

	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://pescuma.org/miranda/voiceservice_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.org/miranda/voiceservice#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"Voice Service ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/voiceserviceW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/voiceservice.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	// Init icons
	if (ServiceExists(MS_SKIN2_ADDICON)) 
	{
		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);
		sid.flags = SIDF_TCHAR;
		sid.ptszSection = TranslateT("Voice Calls");

		int p = 0, i;
		for(i = 0; i < NUM_STATES; i++, p++)
		{
			sid.ptszDescription = GetStateName(i);
			sid.pszName = icon_names[p];
			sid.hDefaultIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_BASE + p), IMAGE_ICON, ICON_SIZE, ICON_SIZE, 0);
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		}

		for(i = 0; i < NUM_ACTIONS; i++, p++)
		{
			sid.ptszDescription = GetActionName(i);
			sid.pszName = icon_names[p];
			sid.hDefaultIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_BASE + p), IMAGE_ICON, ICON_SIZE, ICON_SIZE, 0);
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		}

		sid.ptszDescription = TranslateT("Main");
		sid.pszName = icon_names[p];
		sid.hDefaultIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_BASE + p), IMAGE_ICON, ICON_SIZE, ICON_SIZE, 0);
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		IconsChanged(0, 0);
		hIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, IconsChanged);
	}
	else
	{		
		for(int i = 0; i < NUM_ICONS; i++)
			icons[i] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_BASE + i), IMAGE_ICON, ICON_SIZE, ICON_SIZE, 0);
	}

	// Init fonts
	{
		FontIDT fi = {0};
		fi.cbSize = sizeof(fi);
		lstrcpyn(fi.group, TranslateT("Voice Calls"), MAX_REGS(fi.group));
		strncpy(fi.dbSettingsGroup, MODULE_NAME, MAX_REGS(fi.dbSettingsGroup));

		for (int i = 0; i < NUM_FONTS; i++)
		{
			fi.order = i;
			lstrcpyn(fi.name, GetStateName(i), MAX_REGS(fi.name));
			strncpy(fi.prefix, icon_names[i], MAX_REGS(fi.prefix));

			CallService(MS_FONT_REGISTERT, (WPARAM) &fi, 0);
		}

		ReloadFont(0,0);
		HookEvent(ME_FONT_RELOAD, ReloadFont);
	}

	// Init bkg color
	{
		ColourIDT ci = {0};
		ci.cbSize = sizeof(ci);
		lstrcpyn(ci.group, TranslateT("Voice Calls"), MAX_REGS(ci.group));
		lstrcpyn(ci.name, TranslateT("Background"), MAX_REGS(ci.name));
		strncpy(ci.dbSettingsGroup, MODULE_NAME, MAX_REGS(ci.dbSettingsGroup));
		strncpy(ci.setting, "BkgColor", MAX_REGS(ci.setting));
		ci.defcolour = GetSysColor(COLOR_BTNFACE);

		CallService(MS_COLOUR_REGISTERT, (WPARAM) &ci, 0);

		ReloadColor(0,0);
		HookEvent(ME_COLOUR_RELOAD, ReloadColor);
	}

	InitOptions();
	InitFrames();

	// Add menu items
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.position = -2000020000;

	CreateServiceFunction(MS_VOICESERVICE_CM_CALL, Service_Call);
	mi.pszName = GetActionNameA(ACTION_CALL);
	mi.hIcon = icons[NUM_STATES + ACTION_CALL];
	mi.pszService = MS_VOICESERVICE_CM_CALL;
	hCMCall = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);

	CreateServiceFunction(MS_VOICESERVICE_CM_ANSWER, CMAnswer);
	mi.position++;
	mi.pszName = GetActionNameA(ACTION_ANSWER);
	mi.hIcon = icons[VOICE_STATE_TALKING];
	mi.pszService = MS_VOICESERVICE_CM_ANSWER;
	hCMAnswer = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);

	CreateServiceFunction(MS_VOICESERVICE_CM_HOLD, CMHold);
	mi.position++;
	mi.position++;
	mi.pszName = GetActionNameA(ACTION_HOLD);
	mi.hIcon = icons[VOICE_STATE_ON_HOLD];
	mi.pszService = MS_VOICESERVICE_CM_HOLD;
	hCMHold = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);

	CreateServiceFunction(MS_VOICESERVICE_CM_DROP, CMDrop);
	mi.position++;
	mi.pszName = GetActionNameA(ACTION_DROP);
	mi.hIcon = icons[VOICE_STATE_ENDED];
	mi.pszService = MS_VOICESERVICE_CM_DROP;
	hCMDrop = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);

	hPreBuildContactMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);

	// Util services
	CreateServiceFunction(MS_VOICESERVICE_CALL, Service_Call);
	CreateServiceFunction(MS_VOICESERVICE_CAN_CALL, Service_CanCall);

	// Sounds
	SKINSOUNDDESCEX ssd = {0};
	ssd.cbSize = sizeof(ssd);
	ssd.pszSection = "Voice Calls";

	ssd.pszName = "voice_calling";
	ssd.pszDescription = "Calling a contact";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&ssd);

	ssd.pszName = "voice_ringing";
	ssd.pszDescription = "Ringing";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&ssd);

	ssd.pszName = "voice_started";
	ssd.pszDescription = "Started talking";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&ssd);

	ssd.pszName = "voice_holded";
	ssd.pszDescription = "Put a call on Hold";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&ssd);

	ssd.pszName = "voice_ended";
	ssd.pszDescription = "End of a Call";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&ssd);

	ssd.pszName = "voice_busy";
	ssd.pszDescription = "Busy signal";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&ssd);


	SetTimer(NULL, 0, 1000, ClearOldVoiceCalls);

	return 0;
}

static int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	DeInitFrames();
	DeInitOptions();

	UnhookEvent(hModulesLoaded);
	UnhookEvent(hPreShutdownHook);
	UnhookEvent(hPreBuildContactMenu);
	UnhookEvent(hIconsChanged);

	return 0;
}


HANDLE ConvertMetacontact(HANDLE hContact)
{
	if (ServiceExists(MS_MC_GETMOSTONLINECONTACT))
	{
		HANDLE hTmp = (HANDLE) CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM) hContact, NULL);
		if (hTmp != NULL)
			return hTmp;
	}
	return hContact;
}


static void CopyVoiceCallData(VOICE_CALL_INTERNAL *out, VOICE_CALL *in)
{
	if (in->hContact != NULL)
		out->hContact = in->hContact;

	if (in->ptszNumber != NULL)
	{
		TCHAR tmp[256];
		if (in->flags == VOICE_UNICODE)
			lstrcpyn(tmp, WcharToTchar(in->pwszNumber), MAX_REGS(tmp));
		else
			lstrcpyn(tmp, CharToTchar(in->pszNumber), MAX_REGS(tmp));

		if (tmp[0] != 0)
			lstrcpyn(out->number, tmp, MAX_REGS(out->number));
	}

	TCHAR *contact = NULL;
	if (out->hContact != NULL)
		contact = (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) out->hContact, GCDNF_TCHAR);

	if (contact != NULL && out->number[0] != 0)
	{
		mir_sntprintf(out->displayName, MAX_REGS(out->displayName), _T("%s (%s)"), contact, out->number);
	}
	else if (contact != NULL)
	{
		lstrcpyn(out->displayName, contact, MAX_REGS(out->displayName));
	}
	else if (out->number[0] != 0)
	{
		lstrcpyn(out->displayName, out->number, MAX_REGS(out->displayName));
	}
	else
	{
		lstrcpyn(out->displayName, TranslateT("Unknown number"), MAX_REGS(out->displayName));
	}
}


MODULE_INTERNAL * FindModule(const char *szModule) 
{
	for(unsigned int i = 0; i < modules.size(); i++)
		if (strcmp(modules[i].name, szModule) == 0)
			return &modules[i];

	return NULL;
}

VOICE_CALL_INTERNAL * FindVoiceCall(const char *szModule, const char *id, BOOL add)
{
	for(int i = 0; i < calls.size(); i++)
	{
		if (strcmp(calls[i]->module->name, szModule) == 0 
			&& strcmp(calls[i]->id, id) == 0)
		{
			return calls[i];
		}
	}

	if (add)
	{
		MODULE_INTERNAL *module = FindModule(szModule);
		if (module == NULL)
			return NULL;

		VOICE_CALL_INTERNAL *tmp = new VOICE_CALL_INTERNAL();
		tmp->module = module;
		tmp->id = mir_strdup(id);
		tmp->state = -1;
		tmp->end_time = 0;
		calls.insert(calls.end(), tmp);
		return tmp;
	}

	return NULL;
}


VOICE_CALL_INTERNAL * FindVoiceCall(HANDLE hContact)
{
	for(int i = 0; i < calls.size(); i++)
	{
		if (calls[i]->state != VOICE_STATE_ENDED && calls[i]->hContact == hContact)
		{
			return calls[i];
		}
	}

	return NULL;
}


static void RemoveVoiceCall(const char *szModule, const char *id)
{
	for(vector<VOICE_CALL_INTERNAL *>::iterator it = calls.begin(); it != calls.end(); it++)
	{
		VOICE_CALL_INTERNAL *call = *it;
		if (strcmp(call->module->name, szModule) == 0 && strcmp(call->id, id) == 0)
		{
			delete call;
			calls.erase(it);
			break;
		}
	}
}


static VOID CALLBACK ClearOldVoiceCalls(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	DWORD now = GetTickCount();
	BOOL refresh = FALSE;
	for(vector<VOICE_CALL_INTERNAL *>::iterator it = calls.begin(); it != calls.end(); )
	{
		VOICE_CALL_INTERNAL *call = *it;

		if (call->state == VOICE_STATE_ENDED && call->end_time + TIME_TO_SHOW_ENDED_CALL < now)
		{
			delete call;
			it = calls.erase(it);
			refresh = TRUE;
		}
		else
		{
			it++;
		}
	}

	if (refresh && hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);
}


TCHAR *GetStateName(int state)
{
	switch(state)
	{
		case VOICE_STATE_TALKING: return TranslateT("Talking");
		case VOICE_STATE_CALLING: return TranslateT("Calling");
		case VOICE_STATE_ENDED: return TranslateT("Ended");
		case VOICE_STATE_BUSY: return TranslateT("Busy");
		case VOICE_STATE_ON_HOLD: return TranslateT("On Hold");
		case VOICE_STATE_RINGING: return TranslateT("Ringing");
	}

	return NULL;
}


TCHAR *GetActionName(int state)
{
	switch(state)
	{
		case ACTION_CALL: return TranslateT("Voice Call");
		case ACTION_ANSWER: return TranslateT("Answer Voice Call");
		case ACTION_HOLD: return TranslateT("Hold Voice Call");
		case ACTION_DROP: return TranslateT("Drop Voice Call");
	}

	return NULL;
}

char *GetActionNameA(int state)
{
	switch(state)
	{
		case ACTION_CALL: return Translate("Voice Call");
		case ACTION_ANSWER: return Translate("Answer Voice Call");
		case ACTION_HOLD: return Translate("Hold Voice Call");
		case ACTION_DROP: return Translate("Drop Voice Call");
	}

	return NULL;
}


static bool IsProtocol(char *module)
{
	PROTOACCOUNT **protos;
	int count;

	BOOL hasAccounts = ServiceExists(MS_PROTO_ENUMACCOUNTS);

	if (hasAccounts)
		CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&count, (LPARAM)&protos);
	else
		CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);
	
	for (int i = 0; i < count; i++)
	{
		if (protos[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (protos[i]->szModuleName == NULL || protos[i]->szModuleName[0] == '\0')
			continue;

		if (strcmp(module, protos[i]->szModuleName) == 0)
			return true;
	}

	return false;
}


static int VoiceRegister(WPARAM wParam, LPARAM lParam)
{
	VOICE_MODULE *in = (VOICE_MODULE *) wParam;
	if (in == NULL || in->cbSize < sizeof(VOICE_MODULE))
		return -1;

	if (FindModule(in->name) != NULL)
		return -2;

	MODULE_INTERNAL m = {0};
	strncpy(m.name, in->name, MAX_REGS(m.name));
	m.name[MAX_REGS(m.name)] = 0;
	lstrcpyn(m.description, in->description, MAX_REGS(m.description));
	m.flags = in->flags;
	m.is_protocol = IsProtocol(in->name);

	modules.push_back(m);

	return 0;
}


static int VoiceUnregister(WPARAM wParam, LPARAM lParam)
{
	char *moduleName = (char *) wParam;
	if (moduleName == NULL || moduleName[0] == 0)
		return -1;

	MODULE_INTERNAL *module = FindModule(moduleName);
	if (module == NULL)
		return -2;

	{
		for(vector<VOICE_CALL_INTERNAL *>::iterator it = calls.begin(); it != calls.end(); )
		{
			VOICE_CALL_INTERNAL *call = *it;
			if (call->module != module)
			{
				++it;
			}
			else
			{
				DropCall(call);
				VoiceEndedCall(call);

				delete call;
				it = calls.erase(it);
			}
		}
	}
	{
		for(vector<MODULE_INTERNAL>::iterator it = modules.begin(); it != modules.end(); ++it)
		{
			if (&*it == module)
			{
				modules.erase(it);
				break;
			}
		}
	}

	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	return 0;
}


static int VoiceCalling(VOICE_CALL *in)
{
	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);
	if (vc == NULL)
		return 0;

	if (vc->hwnd != NULL)
	{
		// Destroy old window
		DestroyWindow(vc->hwnd);
		vc->hwnd = NULL;
	}

	if (vc->state == VOICE_STATE_CALLING)
		// Already here
		return 0;

	// Remove old notifications
	if (vc->state == VOICE_STATE_RINGING)
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM) vc->hContact, (LPARAM) vc->last_dbe);

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = VOICE_STATE_CALLING;

	// Notify
	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Calling %s"), vc->displayName);

	// history
	vc->last_dbe = HistoryLog(vc->hContact, text); 

	// frame
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	// sound
	SkinPlaySound("voice_calling");

	// popup
	ShowPopup(NULL, TranslateT("Voice Call"), text);

	return 0;
}


static int VoiceRinging(VOICE_CALL *in)
{
	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);
	if (vc == NULL)
		return 0;

	if (vc->hwnd != NULL)
	{
		// Destroy old window
		DestroyWindow(vc->hwnd);
		vc->hwnd = NULL;
	}

	if (vc->state == VOICE_STATE_RINGING)
		// Already here
		return 0;

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = VOICE_STATE_RINGING;

	// Notify
	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Ringing call from %s"), vc->displayName);

	// history
	vc->last_dbe = HistoryLog(vc->hContact, text); 

	int aut = DBGetContactSettingWord(vc->hContact, MODULE_NAME, "AutoAccept", AUTO_NOTHING);
	if (aut == AUTO_ACCEPT)
	{
		AnswerCall(vc);
	}
	else if (aut == AUTO_DROP)
	{
		DropCall(vc);
	}
	else
	{
		// clist
		CLISTEVENT ce = {0};
		ce.cbSize = sizeof(ce);
		ce.hContact = vc->hContact;
		ce.hIcon = icons[VOICE_STATE_RINGING];
		ce.hDbEvent = vc->last_dbe;
		ce.pszService = MS_VOICESERVICE_CLIST_DBLCLK;
		ce.lParam = (LPARAM) vc;
		CallService(MS_CLIST_ADDEVENT, 0, (LPARAM) &ce);

		// popup
		ShowPopup(NULL, TranslateT("Voice call ringing"), text);

		// sound
		SkinPlaySound("voice_ringing");
	}

	// frame
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	return 0;
}


static int VoiceEndedCall(VOICE_CALL_INTERNAL *vc)
{
	if (vc->hwnd != NULL)
	{
		// Destroy old window
		DestroyWindow(vc->hwnd);
		vc->hwnd = NULL;
	}

	if (vc->state == VOICE_STATE_ENDED)
		// Already here
		return 0;

	// Remove old notifications
	if (vc->state == VOICE_STATE_RINGING)
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM) vc->hContact, (LPARAM) vc->last_dbe);

	// Set data
	vc->state = VOICE_STATE_ENDED;
	vc->end_time = GetTickCount();

	if (currentCall.call == vc)
	{
		currentCall.stopping = FALSE;
		currentCall.call = NULL;
		AnswerCall(currentCall.hungry_call);
	}

	// Notify
	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Call from %s has ended"), vc->displayName);

	// history
	vc->last_dbe = HistoryLog(vc->hContact, text); 

	// frame
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	// popup
	ShowPopup(NULL, TranslateT("Voice call ended"), text);

	// sound
	SkinPlaySound("voice_ended");

	return 0;
}


static int VoiceEndedCall(VOICE_CALL *in)
{
	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, FALSE);
	if (vc == NULL)
		return 0;

	return VoiceEndedCall(vc);
}

static int VoiceStartedCall(VOICE_CALL *in)
{
	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);
	if (vc == NULL)
		return 0;

	if (vc->hwnd != NULL)
	{
		// Destroy old window
		DestroyWindow(vc->hwnd);
		vc->hwnd = NULL;
	}

	if (vc->state == VOICE_STATE_TALKING)
		// Already here
		return 0;

	// Remove old notifications
	if (vc->state == VOICE_STATE_RINGING)
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM) vc->hContact, (LPARAM) vc->last_dbe);

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = VOICE_STATE_TALKING;

	if (currentCall.call != NULL && currentCall.call != vc)
	{
		// Well, can't do much more than try to hold/drop the current call
		if (currentCall.call->module->flags & VOICE_CAPS_CAN_HOLD)
			CallProtoService(currentCall.call->module->name, PS_VOICE_HOLDCALL, (WPARAM) currentCall.call->id, 0);
		else
			CallProtoService(currentCall.call->module->name, PS_VOICE_DROPCALL, (WPARAM) currentCall.call->id, 0);
	}

	currentCall.call = vc;

	// Notify
	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Call from %s started"), vc->displayName);

	// history
	vc->last_dbe = HistoryLog(vc->hContact, text); 

	// frame
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	// popup
	ShowPopup(NULL, TranslateT("Voice call started"), text);

	// sound
	SkinPlaySound("voice_started");

	if (currentCall.hungry_call == vc)
		currentCall.hungry_call = NULL;
	currentCall.stopping = FALSE;

	return 0;
}

static int VoiceHoldedCall(VOICE_CALL *in)
{
	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);
	if (vc == NULL)
		return 0;

	if (vc->hwnd != NULL)
	{
		// Destroy old window
		DestroyWindow(vc->hwnd);
		vc->hwnd = NULL;
	}

	if (vc->state == VOICE_STATE_ON_HOLD)
		// Already here
		return 0;

	// Remove old notifications
	if (vc->state == VOICE_STATE_RINGING)
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM) vc->hContact, (LPARAM) vc->last_dbe);

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = VOICE_STATE_ON_HOLD;

	if (currentCall.call == vc)
	{
		currentCall.stopping = FALSE;
		currentCall.call = NULL;
		AnswerCall(currentCall.hungry_call);
	}

	// Notify
	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Call from %s is on hold"), vc->displayName);

	// history
	vc->last_dbe = HistoryLog(vc->hContact, text); 

	// frame
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	// popup
	ShowPopup(NULL, TranslateT("Voice Call on hold"), text);

	// sound
	SkinPlaySound("voice_holded");

	return 0;
}


static int VoiceState(WPARAM wParam, LPARAM lParam)
{
	VOICE_CALL *in = (VOICE_CALL *) wParam;
	if (in == NULL || in->cbSize < sizeof(VOICE_CALL))
		return 0;

	switch (in->state)
	{
		case VOICE_STATE_TALKING: VoiceStartedCall(in); break;
		case VOICE_STATE_RINGING: VoiceRinging(in); break;
		case VOICE_STATE_ON_HOLD: VoiceHoldedCall(in); break;
		case VOICE_STATE_CALLING: VoiceCalling(in); break;
		case VOICE_STATE_ENDED: VoiceEndedCall(in); break;
		case VOICE_STATE_BUSY: VoiceEndedCall(in); break;
	}

	return 0;
}


void DropCall(VOICE_CALL_INTERNAL * vc)
{
	// Sanity check
	if (vc == NULL || vc->state == VOICE_STATE_ENDED)
		return;

	if (vc->hwnd != NULL)
	{
		// Destroy old window
		DestroyWindow(vc->hwnd);
		vc->hwnd = NULL;
	}

	CallProtoService(vc->module->name, PS_VOICE_DROPCALL, (WPARAM) vc->id, 0);

	if (currentCall.call == vc)
		currentCall.stopping = TRUE;
}

void AnswerCall(VOICE_CALL_INTERNAL * vc)
{
	// Sanity check
	if (vc == NULL || (vc->state != VOICE_STATE_RINGING && vc->state != VOICE_STATE_ON_HOLD))
		return;

	if (vc->hwnd != NULL)
	{
		// Destroy old window
		DestroyWindow(vc->hwnd);
		vc->hwnd = NULL;
	}

	if (currentCall.call != NULL && currentCall.call != vc)
	{
		// We got a problem. We must first wait current one get on hold
		currentCall.hungry_call = vc;

		if (!currentCall.stopping)
		{
			if (currentCall.call->module->flags & VOICE_CAPS_CAN_HOLD)
				CallProtoService(currentCall.call->module->name, PS_VOICE_HOLDCALL, (WPARAM) currentCall.call->id, 0);
			else
				CallProtoService(currentCall.call->module->name, PS_VOICE_DROPCALL, (WPARAM) currentCall.call->id, 0);

			currentCall.stopping = TRUE;
		}
	}
	else
	{
		CallProtoService(vc->module->name, PS_VOICE_ANSWERCALL, (WPARAM) vc->id, 0);

		if (currentCall.hungry_call == vc)
			currentCall.hungry_call = NULL;
	}
}

void HoldCall(VOICE_CALL_INTERNAL * vc)
{
	// Sanity check
	if (vc == NULL || vc->state != VOICE_STATE_TALKING || !(vc->module->flags & VOICE_CAPS_CAN_HOLD))
		return;

	if (vc->hwnd != NULL)
	{
		// Destroy old window
		DestroyWindow(vc->hwnd);
		vc->hwnd = NULL;
	}

	CallProtoService(vc->module->name, PS_VOICE_HOLDCALL, (WPARAM) vc->id, 0);

	if (currentCall.call == vc)
		currentCall.stopping = TRUE;
}


static int IconsChanged(WPARAM wParam, LPARAM lParam)
{
	for(int i = 0; i < NUM_ICONS; i++)
		icons[i] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) icon_names[i]);

	return 0;
}


static int ReloadFont(WPARAM wParam, LPARAM lParam) 
{
	LOGFONT log_font;
	FontIDT fi = {0};
	fi.cbSize = sizeof(fi);
	lstrcpyn(fi.group, TranslateT("Voice Calls"), MAX_REGS(fi.group));

	font_max_height = 0;
	for (int i = 0; i < NUM_FONTS; i++)
	{
		if (fonts[i] != 0) DeleteObject(fonts[i]);

		lstrcpyn(fi.name, GetStateName(i), MAX_REGS(fi.name));
		font_colors[i] = CallService(MS_FONT_GETT, (WPARAM) &fi, (LPARAM) &log_font);
		fonts[i] = CreateFontIndirect(&log_font);

		font_max_height = max(font_max_height, log_font.lfHeight);
	}
	
	if (hwnd_frame != NULL)
		InvalidateRect(hwnd_frame, NULL, FALSE);

	return 0;
}


static int ReloadColor(WPARAM wParam, LPARAM lParam) 
{
	ColourIDT ci = {0};
	ci.cbSize = sizeof(ci);
	lstrcpyn(ci.group, TranslateT("Voice Calls"), MAX_REGS(ci.group));
	lstrcpyn(ci.name, TranslateT("Background"), MAX_REGS(ci.name));

	bkg_color = CallService(MS_COLOUR_GETT, (WPARAM) &ci, 0);

	if (bk_brush != NULL)
		DeleteObject(bk_brush);
	bk_brush = CreateSolidBrush(bkg_color);

	if (hwnd_frame != NULL)
		InvalidateRect(hwnd_frame, NULL, TRUE);
	
	return 0;
}


// Returns true if the unicode buffer only contains 7-bit characters.
static BOOL IsUnicodeAscii(const WCHAR * pBuffer, int nSize)
{
	BOOL bResult = TRUE;
	int nIndex;

	for (nIndex = 0; nIndex < nSize; nIndex++) {
		if (pBuffer[nIndex] > 0x7F) {
			bResult = FALSE;
			break;
		}
	}
	return bResult;
}


static HANDLE HistoryLog(HANDLE hContact, TCHAR *log_text)
{
	if (log_text != NULL)
	{
		DBEVENTINFO event = { 0 };
		BYTE *tmp = NULL;

		event.cbSize = sizeof(event);

#ifdef UNICODE

		size_t needed = WideCharToMultiByte(CP_ACP, 0, log_text, -1, NULL, 0, NULL, NULL);
		size_t len = lstrlen(log_text);
		size_t size;

		if (IsUnicodeAscii(log_text, len))
			size = needed;
		else
			size = needed + (len + 1) * sizeof(WCHAR);

		tmp = (BYTE *) mir_alloc0(size);

		WideCharToMultiByte(CP_ACP, 0, log_text, -1, (char *) tmp, needed, NULL, NULL);

		if (size > needed)
			lstrcpyn((WCHAR *) &tmp[needed], log_text, len + 1);

		event.pBlob = tmp;
		event.cbBlob = size;

#else

		event.pBlob = (PBYTE) log_text;
		event.cbBlob = strlen(log_text) + 1;

#endif

		event.eventType = EVENTTYPE_VOICE_CALL;
		event.flags = DBEF_READ;
		event.timestamp = (DWORD) time(NULL);

		event.szModule = MODULE_NAME;

		HANDLE ret = (HANDLE) CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&event);

		mir_free(tmp);

		return ret;
	}
	else
	{
		return NULL;
	}
}


static int CListDblClick(WPARAM wParam,LPARAM lParam) 
{
	CLISTEVENT *ce = (CLISTEVENT *) lParam;
	
	VOICE_CALL_INTERNAL *vc = (VOICE_CALL_INTERNAL *) ce->lParam;

	if (vc->hwnd != NULL)
		// Destroy old window
		DestroyWindow(vc->hwnd);

	vc->hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_NEW_CALL), NULL, DlgProcNewCall, (LPARAM) vc);
 	ShowWindow(vc->hwnd, SW_SHOWNORMAL);

	return 0;
}


static int Service_CanCall(WPARAM wParam,LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	// Find a module that can call that contact
	// TODO: Add options to call from more than one module
	int i, avaiable = 0;
	for(i = 0; i < modules.size(); i++)
	{
		if (!modules[i].CanCall(hContact))
			continue;

		// Oki, can call
		avaiable ++;
		break;
	}

	return avaiable;
}


static int Service_Call(WPARAM wParam, LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	// Find a module that can call that contact
	// TODO: Add options to call from more than one module
	int i;
	for(i = 0; i < modules.size(); i++)
	{
		if (!modules[i].CanCall(hContact))
			continue;

		// Oki, can call
		break;
	}

	if (i == modules.size())
		return 1;

	CallProtoService(modules[i].name, PS_VOICE_CALL, (WPARAM) hContact, 0);
	return 0;
}


static int CMAnswer(WPARAM wParam,LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	AnswerCall(FindVoiceCall(hContact));
	return 0;
}


static int CMHold(WPARAM wParam,LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	HoldCall(FindVoiceCall(hContact));
	return 0;
}


static int CMDrop(WPARAM wParam,LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	DropCall(FindVoiceCall(hContact));
	return 0;
}


static int PreBuildContactMenu(WPARAM wParam,LPARAM lParam) 
{
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hCMCall, (LPARAM) &mi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hCMAnswer, (LPARAM) &mi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hCMHold, (LPARAM) &mi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hCMDrop, (LPARAM) &mi);

	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	// From now on unhide it
	mi.flags = CMIM_FLAGS;

	// There is a current call already?
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(hContact);
	if (vc == NULL)
	{
		// Some module can handle it?
		int i;
		for(i = 0; i < modules.size(); i++)
		{
			if (!modules[i].CanCall(hContact))
				continue;

			// Oki, found a module that can handle it
			break;
		}

		if (i == modules.size())
			return 0;

		// Just call contact
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hCMCall, (LPARAM) &mi);
	}
	else
	{		
		switch (vc->state)
		{
			case VOICE_STATE_CALLING:
			{
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hCMDrop, (LPARAM) &mi);
				break;
			}
			case VOICE_STATE_TALKING:
			{
				if (vc->module->flags & VOICE_CAPS_CAN_HOLD)
					CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hCMHold, (LPARAM) &mi);
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hCMDrop, (LPARAM) &mi);
				break;
			}
			case VOICE_STATE_RINGING:
			case VOICE_STATE_ON_HOLD:
			{
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hCMAnswer, (LPARAM) &mi);
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hCMDrop, (LPARAM) &mi);
				break;
			}
		}
	}

	return 0;
}


static BOOL CALLBACK DlgProcNewCall(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			VOICE_CALL_INTERNAL *vc = (VOICE_CALL_INTERNAL *) lParam;

			TranslateDialogDefault(hwndDlg);

			TCHAR text[1024];
			mir_sntprintf(text, MAX_REGS(text), TranslateT("%s wants to start a voice call with you. What you want to do?"),
				vc->displayName);

			SendMessage(GetDlgItem(hwndDlg, IDC_TEXT), WM_SETTEXT, 0, (LPARAM) text);

			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) icons[VOICE_STATE_RINGING]);

			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) vc);

			return TRUE;
		}

		case WM_COMMAND:
		{
			switch(wParam)
			{
				case ID_ANSWER:
				{
					VOICE_CALL_INTERNAL *vc = (VOICE_CALL_INTERNAL *) GetWindowLong(hwndDlg, GWL_USERDATA);
					if (IsDlgButtonChecked(hwndDlg, IDC_AUTO))
						DBWriteContactSettingWord(vc->hContact, MODULE_NAME, "AutoAccept", AUTO_ACCEPT);

					AnswerCall(vc);

					DestroyWindow(hwndDlg);
					break;
				}
				case ID_DROP:
				{
					VOICE_CALL_INTERNAL *vc = (VOICE_CALL_INTERNAL *) GetWindowLong(hwndDlg, GWL_USERDATA);
					if (IsDlgButtonChecked(hwndDlg, IDC_AUTO))
						DBWriteContactSettingWord(vc->hContact, MODULE_NAME, "AutoAccept", AUTO_DROP);

					DropCall(vc);

					DestroyWindow(hwndDlg);
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;

		case WM_DESTROY:
			VOICE_CALL_INTERNAL *vc = (VOICE_CALL_INTERNAL *) GetWindowLong(hwndDlg, GWL_USERDATA);
			vc->hwnd = NULL;
			break;
	}
	
	return FALSE;
}



bool MODULE_INTERNAL::CanCall(HANDLE hContact, BOOL now)
{
	if ((flags & VOICE_CAPS_CALL_CONTACT) == 0)
		return false;

	if (ProtoServiceExists(name, PS_VOICE_CALL_CONTACT_VALID))
		return CallProtoService(name, PS_VOICE_CALL_CONTACT_VALID, (WPARAM) hContact, now) != 0;

	if (is_protocol)
		return CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM) hContact, (LPARAM) name) != 0;

	return true;
}

bool MODULE_INTERNAL::CanCall(const TCHAR *number)
{
	if ((flags & VOICE_CAPS_CALL_STRING) == 0)
		return false;

	if (ProtoServiceExists(name, PS_VOICE_CALL_STRING_VALID))
		return CallProtoService(name, PS_VOICE_CALL_STRING_VALID, (WPARAM) number, 0) != 0;

	return true;
}
