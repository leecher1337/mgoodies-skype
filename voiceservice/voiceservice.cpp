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
#ifdef UNICODE
	"Voice Service (Unicode)",
#else
	"Voice Service",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,2),
	"Provide services for protocols that support voice calls",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2006 Ricardo Pescuma Domenecci",
	"http://pescuma.mirandaim.ru",
	0,	//not transient
	0	//doesn't replace anything built-in
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;

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
static int VoiceState(WPARAM wParam, LPARAM lParam);

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

static int CMCall(WPARAM wParam,LPARAM lParam); 
static int CMAnswer(WPARAM wParam,LPARAM lParam); 
static int CMHold(WPARAM wParam,LPARAM lParam);
static int CMDrop(WPARAM wParam,LPARAM lParam);

static int IconsChanged(WPARAM wParam, LPARAM lParam);
static int ReloadFont(WPARAM wParam, LPARAM lParam);
static int ReloadColor(WPARAM wParam, LPARAM lParam);
static VOID CALLBACK ClearOldVoiceCalls(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);


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

	CreateServiceFunction(MS_VOICESERVICE_CLIST_DBLCLK, CListDblClick);
	CreateServiceFunction(MS_VOICESERVICE_STATE, VoiceState);
	CreateServiceFunction(MS_VOICESERVICE_REGISTER, VoiceRegister);

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

		upd.szBetaVersionURL = "http://pescuma.mirandaim.ru/miranda/voiceservice_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.mirandaim.ru/miranda/voiceservice#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"Voice Service ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/voiceserviceW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/voiceservice.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
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

	CreateServiceFunction(MS_VOICESERVICE_CM_CALL, CMCall);
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

	// Hook protocol calls
	PROTOCOLDESCRIPTOR **protos;
	int count;

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

	for (int i = 0; i < count; i++)
	{
		if (protos[i]->type != PROTOTYPE_PROTOCOL && protos[i]->type != PROTOTYPE_VOICE)
			continue;

		if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0')
			continue;

		if (!ProtoServiceExists(protos[i]->szName, PS_VOICE_GETINFO))
			continue;

		// Found a protocol
		MODULE_INTERNAL m = {0};

		m.name = protos[i]->szName;
		m.is_protocol = TRUE;
		m.flags = CallProtoService(protos[i]->szName, PS_VOICE_GETINFO, 0, 0);

		char notify[128];
		mir_snprintf(notify, MAX_REGS(notify), "%s" PE_VOICE_CALL_STATE, protos[i]->szName);
		m.state_hook = HookEvent(notify, VoiceState);

		modules.insert(modules.end(), m);
	}

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


static void CopyVoiceCallData(VOICE_CALL_INTERNAL *out, VOICE_CALL *in)
{
	if (in->flags & VOICE_CALL_CONTACT)
		out->flags = VOICE_CALL_CONTACT;
	else
		out->flags = VOICE_CALL_STRING | VOICE_TCHAR;

	char description[128];
	if (ProtoServiceExists(in->szModule, PS_GETNAME))
		CallProtoService(in->szModule, PS_GETNAME, MAX_REGS(description),(LPARAM) description);
	else
		strncpy(description, Translate(in->szModule), MAX_REGS(description));

	if (in->flags & VOICE_CALL_CONTACT)
	{
		if (in->hContact != NULL)
		{
			out->hContact = in->hContact;
			mir_sntprintf(out->ptszContact, MAX_REGS(out->ptszContact), _T("%s (") _T(_S) _T(")"), 
				(TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) in->hContact, GCDNF_TCHAR),
				description);
		}
	}
	else if (in->flags & VOICE_UNICODE)
	{
		out->hContact = NULL;
		if (in->pwszContact[0] != L'\0')
			mir_sntprintf(out->ptszContact, MAX_REGS(out->ptszContact), _T(_SI) _T(" (") _T(_S) _T(")"), 
				in->pwszContact,
				description);
	}
	else
	{
		out->hContact = NULL;
		if (in->pszContact[0] != '\0')
			mir_sntprintf(out->ptszContact, MAX_REGS(out->ptszContact), _T(_S) _T(" (") _T(_S) _T(")"), 
				in->pszContact,
				description);
	}
}


MODULE_INTERNAL * FindModule(const char *szModule) 
{
	MODULE_INTERNAL *module = NULL;
	for(int i = 0; i < modules.size(); i++)
	{
		if (strcmp(modules[i].name, szModule) == 0)
		{
			module = &modules[i];
			break;
		}
	}

	return module;
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
		if (strcmp((*it)->module->name, szModule) == 0 
			&& strcmp((*it)->id, id) == 0)
		{
			delete *it;
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
		if ((*it)->state == VOICE_STATE_ENDED && (*it)->end_time + TIME_TO_SHOW_ENDED_CALL < now)
		{
			delete *it;
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


static int VoiceRegister(WPARAM wParam, LPARAM lParam)
{
	VOICE_MODULE *in = (VOICE_MODULE *) wParam;
	if (in == NULL || in->cbSize < sizeof(VOICE_MODULE))
		return -1;

	if (FindModule(in->name) != NULL)
		return -2;

	MODULE_INTERNAL m = {0};
	m.name = mir_strdup(in->name);
	m.is_protocol = FALSE;
	m.flags = in->flags;
	modules.insert(modules.end(), m);

	return 0;
}


static int VoiceCalling(VOICE_CALL *in)
{
	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);
	if (vc == NULL)
		return 0;

	// Remove old notifications
	if (vc->state == VOICE_STATE_RINGING)
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM) vc->hContact, (LPARAM) vc->last_dbe);

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = VOICE_STATE_CALLING;

	// Notify
	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Calling %s"), vc->ptszContact);

	// history
	vc->last_dbe = HistoryLog(vc->hContact, text); 

	// frame
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	// popup
	ShowPopup(NULL, TranslateT("Voice call"), text);

	return 0;
}


static int VoiceRinging(VOICE_CALL *in)
{
	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);
	if (vc == NULL)
		return 0;

	// Remove old notifications
	if (vc->state == VOICE_STATE_RINGING)
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM) vc->hContact, (LPARAM) vc->last_dbe);

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = VOICE_STATE_RINGING;

	// Notify
	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Ringing call from %s"), vc->ptszContact);

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
	}

	// frame
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	return 0;
}

static int VoiceEndedCall(VOICE_CALL *in)
{
	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);
	if (vc == NULL)
		return 0;

	// Remove old notifications
	if (vc->state == VOICE_STATE_RINGING)
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM) vc->hContact, (LPARAM) vc->last_dbe);

	// Set data
	CopyVoiceCallData(vc, in);
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
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Call from %s has ended"), vc->ptszContact);

	// history
	vc->last_dbe = HistoryLog(vc->hContact, text); 

	// frame
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	// popup
	ShowPopup(NULL, TranslateT("Voice call ended"), text);

	// RemoveVoiceCall(in->szModule, in->id);

	// Need to answer other one?
	if (currentCall.hungry_call != NULL)
		AnswerCall(currentCall.hungry_call);

	return 0;
}

static int VoiceStartedCall(VOICE_CALL *in)
{
	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);
	if (vc == NULL)
		return 0;

	// Remove old notifications
	if (vc->state == VOICE_STATE_RINGING)
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM) vc->hContact, (LPARAM) vc->last_dbe);

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = VOICE_STATE_TALKING;

	if (currentCall.call != NULL)
	{
		// Well, can't do much more than try to hold/drop the current call
		if (currentCall.call->module->flags & VOICE_CAN_HOLD)
			CallProtoService(currentCall.call->module->name, PS_VOICE_HOLDCALL, (WPARAM) currentCall.call->id, 0);
		else
			CallProtoService(currentCall.call->module->name, PS_VOICE_DROPCALL, (WPARAM) currentCall.call->id, 0);
	}

	currentCall.call = vc;

	// Notify
	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Call from %s started"), vc->ptszContact);

	// history
	vc->last_dbe = HistoryLog(vc->hContact, text); 

	// frame
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	// popup
	ShowPopup(NULL, TranslateT("Voice call started"), text);

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
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Call from %s is on hold"), vc->ptszContact);

	// history
	vc->last_dbe = HistoryLog(vc->hContact, text); 

	// frame
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	// popup
	ShowPopup(NULL, TranslateT("Voice call on hold"), text);

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
	}

	return 0;
}


void DropCall(VOICE_CALL_INTERNAL * vc)
{
	// Sanity check
	if (vc == NULL || vc->state == VOICE_STATE_ENDED)
		return;

	CallProtoService(vc->module->name, PS_VOICE_DROPCALL, (WPARAM) vc->id, 0);

	if (currentCall.call == vc)
		currentCall.stopping = TRUE;
}

void AnswerCall(VOICE_CALL_INTERNAL * vc)
{
	// Sanity check
	if (vc == NULL || (vc->state != VOICE_STATE_RINGING && vc->state != VOICE_STATE_ON_HOLD))
		return;

	if (currentCall.call != NULL && currentCall.call != vc)
	{
		// We got a problem. We must first wait current one get on hold
		currentCall.hungry_call = vc;

		if (!currentCall.stopping)
		{
			if (currentCall.call->module->flags & VOICE_CAN_HOLD)
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
	if (vc == NULL || vc->state != VOICE_STATE_TALKING || !(vc->module->flags & VOICE_CAN_HOLD))
		return;

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
	AnswerCall((VOICE_CALL_INTERNAL *) ce->lParam);
	return 0;
}


static int CMCall(WPARAM wParam,LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return -1;

	// Find a module that can call that contact
	// TODO: Add options to call from more than one module
	int i;
	for(i = 0; i < modules.size(); i++)
	{
		if (!(modules[i].flags & VOICE_CALL_CONTACT))
			continue;

		if (modules[i].is_protocol
			&& !CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM) hContact, (LPARAM) modules[i].name))
			continue;

		if ((modules[i].flags & VOICE_CALL_CONTACT_NEED_TEST) 
			&& !CallProtoService(modules[i].name, PS_VOICE_CALL_CONTACT_VALID, (WPARAM) hContact, TRUE))
			continue;

		// Oki, can call
		break;
	}

	if (i == modules.size())
		return 0;

	CallProtoService(modules[i].name, PS_VOICE_CALL, (WPARAM) hContact, 0);
	return 0;
}


static int CMAnswer(WPARAM wParam,LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return -1;

	AnswerCall(FindVoiceCall(hContact));
	return 0;
}


static int CMHold(WPARAM wParam,LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return -1;

	HoldCall(FindVoiceCall(hContact));
	return 0;
}


static int CMDrop(WPARAM wParam,LPARAM lParam) 
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return -1;

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

	// Some Module can handle it?
	int i;
	for(i = 0; i < modules.size(); i++)
	{
		if (!(modules[i].flags & VOICE_CALL_CONTACT))
			continue;

		if (modules[i].is_protocol
			&& !CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM) hContact, (LPARAM) modules[i].name))
			continue;

		// Oki, found a module that can handle it
		break;
	}

	if (i == modules.size())
		return 0;

	// From now on unhide it
	mi.flags = CMIM_FLAGS;

	// There is a current call already?
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(hContact);
	if (vc == NULL)
	{
		// Check if can call
		if ((modules[i].flags & VOICE_CALL_CONTACT_NEED_TEST) 
			&& !CallProtoService(modules[i].name, PS_VOICE_CALL_CONTACT_VALID, (WPARAM) hContact, TRUE))
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
				if (vc->module->flags & VOICE_CAN_HOLD)
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

