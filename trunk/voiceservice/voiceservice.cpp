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
	PLUGIN_MAKE_VERSION(0,0,2,1),
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

char *metacontacts_proto = NULL;


int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);

static int VoiceRinging(WPARAM wParam, LPARAM lParam);
static int VoiceEndedCall(WPARAM wParam, LPARAM lParam);
static int VoiceStartedCall(WPARAM wParam, LPARAM lParam);
static int VoiceHoldedCall(WPARAM wParam, LPARAM lParam);

TCHAR *GetStateName(int state);
TCHAR *GetActionName(int action);

vector<VOICE_CALL_INTERNAL> calls;
CURRENT_CALL currentCall = {0};

HFONT fonts[NUM_FONTS] = {0};
COLORREF font_colors[NUM_FONTS] = {0};
int font_max_height;

HICON icons[NUM_ICONS] = {0};
char *icon_names[NUM_ICONS] = { "vc_talking", "vc_ringing", "vc_on_hold", "vc_ended", 
					 "vca_call", "vca_answer" , "vca_hold", "vca_drop",
					 "vc_main"};

#define IDI_BASE IDI_TALKING 

static int IconsChanged(WPARAM wParam, LPARAM lParam);
static int ReloadFont(WPARAM wParam, LPARAM lParam);



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

	// Hooks
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

	InitOptions();
	InitFrames();

	// Hook protocol calls
	PROTOCOLDESCRIPTOR **protos;
	int count;

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

	for (int i = 0; i < count; i++)
	{
		if (protos[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0')
			continue;

		// Found a protocol
		char notify[128];

		mir_snprintf(notify, MAX_REGS(notify), "%s%s", protos[i]->szName, PE_VOICE_RINGING);
		HookEvent(notify, VoiceRinging);
		
		mir_snprintf(notify, MAX_REGS(notify), "%s%s", protos[i]->szName, PE_VOICE_ENDEDCALL);
		HookEvent(notify, VoiceEndedCall);

		mir_snprintf(notify, MAX_REGS(notify), "%s%s", protos[i]->szName, PE_VOICE_STARTEDCALL);
		HookEvent(notify, VoiceStartedCall);

		mir_snprintf(notify, MAX_REGS(notify), "%s%s", protos[i]->szName, PE_VOICE_HOLDEDCALL);
		HookEvent(notify, VoiceHoldedCall);
	}

	CreateServiceFunction("Voice/Ringing", VoiceRinging);
	CreateServiceFunction("Voice/EndedCall", VoiceEndedCall);
	CreateServiceFunction("Voice/StartedCall", VoiceStartedCall);
	CreateServiceFunction("Voice/HoldedCall", VoiceHoldedCall);

	return 0;
}

int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	DeInitFrames();
	DeInitOptions();

	UnhookEvent(hModulesLoaded);
	UnhookEvent(hPreShutdownHook);
	UnhookEvent(hIconsChanged);

	return 0;
}


static void CopyVoiceCallData(VOICE_CALL_INTERNAL *out, VOICE_CALL *in)
{
	out->cbSize = sizeof(VOICE_CALL);
	if (in->flags & VOICE_CALL_CONTACT)
		out->flags = VOICE_CALL_CONTACT;
	else
		out->flags = VOICE_CALL_STRING | VOICE_TCHAR;

	char description[128];
	if (ProtoServiceExists(in->szModule, PS_GETNAME))
		CallProtoService(in->szModule, PS_GETNAME, MAX_REGS(description),(LPARAM) description);
	else
		strncpy(description, in->szModule, MAX_REGS(description));

	if (in->flags & VOICE_CALL_CONTACT)
	{
		out->hContact = in->hContact;
		mir_sntprintf(out->ptszContact, MAX_REGS(out->ptszContact), _T("%s (") _T(_S) _T(")"), 
			(TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) in->hContact, GCDNF_TCHAR),
			description);
	}
	else if (in->flags & VOICE_UNICODE)
	{
		out->hContact = NULL;
		mir_sntprintf(out->ptszContact, MAX_REGS(out->ptszContact), _T(_SI) _T(" (") _T(_S) _T(")"), 
			in->pwszContact,
			description);
	}
	else
	{
		out->hContact = NULL;
		mir_sntprintf(out->ptszContact, MAX_REGS(out->ptszContact), _T(_S) _T(" (") _T(_S) _T(")"), 
			in->pszContact,
			description);
	}
}


static VOICE_CALL_INTERNAL * FindVoiceCall(const char *szModule, const char *id, BOOL add)
{
	for(int i = 0; i < calls.size(); i++)
	{
		if (strcmp(calls[i].szModule, szModule) == 0 
			&& strcmp(calls[i].id, id) == 0)
		{
			return &calls[i];
		}
	}

	if (add)
	{
		VOICE_CALL_INTERNAL tmp = {0};
		tmp.szModule = mir_strdup(szModule);
		tmp.id = mir_strdup(id);
		calls.insert(calls.end(), tmp);
		return &calls[calls.size()-1];
	}

	return NULL;
}


static void RemoveVoiceCall(const char *szModule, const char *id)
{
	for(vector<VOICE_CALL_INTERNAL>::iterator it = calls.begin(); it != calls.end(); it++)
	{
		if (strcmp((*it).szModule, szModule) == 0 
			&& strcmp((*it).id, id) == 0)
		{
			calls.erase(it);
			break;
		}
	}
}


TCHAR *GetStateName(int state)
{
	switch(state)
	{
		case TALKING: return TranslateT("Talking");
		case ENDED: return TranslateT("Ended");
		case ON_HOLD: return TranslateT("On Hold");
		case RINGING: return TranslateT("Ringing");
	}

	return NULL;
}


TCHAR *GetActionName(int state)
{
	switch(state)
	{
		case ACTION_CALL: return TranslateT("Call");
		case ACTION_ANSWER: return TranslateT("Answer");
		case ACTION_HOLD: return TranslateT("Hold");
		case ACTION_DROP: return TranslateT("Drop");
	}

	return NULL;
}


static int VoiceRinging(WPARAM wParam, LPARAM lParam)
{
	VOICE_CALL *in = (VOICE_CALL *) wParam;
	if (in == NULL || in->cbSize < sizeof(VOICE_CALL))
		return 0;

	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = RINGING;

	// Notify
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Ringing call from %s"), vc->ptszContact);
	ShowPopup(NULL, TranslateT("Voice call ringing"), text);

	return 0;
}

static int VoiceEndedCall(WPARAM wParam, LPARAM lParam)
{
	VOICE_CALL *in = (VOICE_CALL *) wParam;
	if (in == NULL || in->cbSize < sizeof(VOICE_CALL))
		return 0;

	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);
	if (vc == NULL)
		return 0;

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = ENDED;

	if (currentCall.call == vc)
	{
		currentCall.stopping = FALSE;
		currentCall.call = NULL;
		AnswerCall(currentCall.hungry_call);
	}

	// Notify
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Call from %s has ended"), vc->ptszContact);
	ShowPopup(NULL, TranslateT("Voice call ended"), text);

	// RemoveVoiceCall(in->szModule, in->id);

	// Need to answer other one?
	if (currentCall.hungry_call != NULL)
		AnswerCall(currentCall.hungry_call);

	return 0;
}

static int VoiceStartedCall(WPARAM wParam, LPARAM lParam)
{
	VOICE_CALL *in = (VOICE_CALL *) wParam;
	if (in == NULL || in->cbSize < sizeof(VOICE_CALL))
		return 0;

	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = TALKING;

	if (currentCall.call != NULL)
	{
		// Well, can't do much more than try to hold/drop the current call
		if (CanHoldCall(currentCall.call))
			CallProtoService(currentCall.call->szModule, PS_VOICE_HOLDCALL, (WPARAM) currentCall.call->id, 0);
		else
			CallProtoService(currentCall.call->szModule, PS_VOICE_DROPCALL, (WPARAM) currentCall.call->id, 0);
	}

	currentCall.call = vc;

	// Notify
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Call from %s started"), vc->ptszContact);
	ShowPopup(NULL, TranslateT("Voice call started"), text);

	if (currentCall.hungry_call == vc)
		currentCall.hungry_call = NULL;
	currentCall.stopping = FALSE;

	return 0;
}

static int VoiceHoldedCall(WPARAM wParam, LPARAM lParam)
{
	VOICE_CALL *in = (VOICE_CALL *) wParam;
	if (in == NULL || in->cbSize < sizeof(VOICE_CALL))
		return 0;

	// Check if the call is aready in list
	VOICE_CALL_INTERNAL *vc = FindVoiceCall(in->szModule, in->id, TRUE);

	// Set data
	CopyVoiceCallData(vc, in);
	vc->state = ON_HOLD;

	if (currentCall.call == vc)
	{
		currentCall.stopping = FALSE;
		currentCall.call = NULL;
		AnswerCall(currentCall.hungry_call);
	}

	// Notify
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, WMU_REFRESH, 0, 0);

	TCHAR text[512];
	mir_sntprintf(text, MAX_REGS(text), TranslateT("Call from %s is holded"), vc->ptszContact);
	ShowPopup(NULL, TranslateT("Voice call on hold"), text);

	return 0;
}


BOOL CanHoldCall(const VOICE_CALL_INTERNAL * vc)
{
	return !ProtoServiceExists(vc->szModule, PS_VOICE_GETINFO)
			|| (CallProtoService(vc->szModule, PS_VOICE_GETINFO, 0, 0) & VOICE_CAN_HOLD);
}


void DropCall(VOICE_CALL_INTERNAL * vc)
{
	// Sanity check
	if (vc == NULL || vc->state == ENDED)
		return;

	CallProtoService(vc->szModule, PS_VOICE_DROPCALL, (WPARAM) vc->id, 0);

	if (currentCall.call == vc)
		currentCall.stopping = TRUE;
}

void AnswerCall(VOICE_CALL_INTERNAL * vc)
{
	// Sanity check
	if (vc == NULL || vc->state != RINGING && vc->state != ON_HOLD)
		return;

	if (currentCall.call != NULL && currentCall.call != vc)
	{
		// We got a problem. We must first wait current one get on hold
		currentCall.hungry_call = vc;

		if (!currentCall.stopping)
		{
			if (CanHoldCall(vc))
				CallProtoService(currentCall.call->szModule, PS_VOICE_HOLDCALL, (WPARAM) currentCall.call->id, 0);
			else
				CallProtoService(currentCall.call->szModule, PS_VOICE_DROPCALL, (WPARAM) currentCall.call->id, 0);

			currentCall.stopping = TRUE;
		}
	}
	else
	{
		CallProtoService(vc->szModule, PS_VOICE_ANSWERCALL, (WPARAM) vc->id, 0);

		if (currentCall.hungry_call == vc)
			currentCall.hungry_call = NULL;
	}
}

void HoldCall(VOICE_CALL_INTERNAL * vc)
{
	// Sanity check
	if (vc == NULL || vc->state != TALKING)
		return;

	CallProtoService(vc->szModule, PS_VOICE_HOLDCALL, (WPARAM) vc->id, 0);

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
	
	return 0;
}
