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


#define SIZEOF_HISTORY_EVENT_ADD_V1 36


// Prototypes ///////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
#ifdef UNICODE
	"History Events (Unicode)",
#else
	"History Events",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,8),
	"A service plugin to handle custom history events",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2007 Ricardo Pescuma Domenecci",
	"http://pescuma.mirandaim.ru/miranda/historyevents",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
#ifdef UNICODE
	{ 0x25b9a055, 0x1e7f, 0x4505, { 0x99, 0xef, 0x9b, 0xc7, 0x6e, 0x3f, 0x1b, 0xd0 } } // {25B9A055-1E7F-4505-99EF-9BC76E3F1BD0}
#else
	{ 0xe502920c, 0xd4b9, 0x44d0, { 0xad, 0x29, 0xf0, 0x3, 0x93, 0x75, 0xc4, 0xf9 } } // {E502920C-D4B9-44d0-AD29-F0039375C4F9}
#endif
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;
MM_INTERFACE mmi;
UTF8_INTERFACE utfi;
LIST_INTERFACE li;

int SortHandlers(const HISTORY_EVENT_HANDLER *type1, const HISTORY_EVENT_HANDLER *type2);
LIST<HISTORY_EVENT_HANDLER> handlers(20, SortHandlers);

static HANDLE hHooks[4] = {0};

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);
int DbEventFilterAdd(WPARAM wParam, LPARAM lParam);
int DbEventAdded(WPARAM wParam, LPARAM lParam);

int ServiceGetCount(WPARAM wParam, LPARAM lParam);
int ServiceGetEvent(WPARAM wParam, LPARAM lParam);
int ServiceRegister(WPARAM wParam, LPARAM lParam);
int ServiceCanHandle(WPARAM wParam, LPARAM lParam);
int ServiceGetIcon(WPARAM wParam, LPARAM lParam);
int ServiceGetFlags(WPARAM wParam, LPARAM lParam);
int ServiceGetText(WPARAM wParam, LPARAM lParam);
int ServiceReleaseText(WPARAM wParam, LPARAM lParam);
int ServiceAddToHistory(WPARAM wParam, LPARAM lParam);
int ServiceIsEnabledTemplate(WPARAM wParam, LPARAM lParam);

HANDLE hDeleteThreadEvent;
DWORD WINAPI DeleteThread(LPVOID vParam);
static CRITICAL_SECTION cs;
int shuttingDown = 0;

HISTORY_EVENT_HANDLER *GetHandler(WORD eventType);
void RegisterDefaultEventType(char *name, char *description, WORD eventType, int flags = 0, char *icon = NULL, fGetHistoryEventText pfGetHistoryEventText = NULL);
void * GetDefaultHistoryEventText(HANDLE hContact, HANDLE hDbEvent, DBEVENTINFO *dbe, int flags);
void * GetMessageHistoryEventText(HANDLE hContact, HANDLE hDbEvent, DBEVENTINFO *dbe, int format);
void * GetFileHistoryEventText(HANDLE hContact, HANDLE hDbEvent, DBEVENTINFO *dbe, int flags);
template<class T>
char * ConvertToRTF(T *line, HISTORY_EVENT_HANDLER *heh = NULL, BYTE *extra = NULL);

// SRMM settings
#define LOADHISTORY_UNREAD    0
#define LOADHISTORY_COUNT     1
#define LOADHISTORY_TIME      2

// a list of db events - we'll check them for the 'read' flag periodically and delete them whwen marked as read
struct HistoryEventNode {
	HANDLE hContact;
	HANDLE hDBEvent;
	HistoryEventNode *next;
};

HistoryEventNode *firstHistoryEvent = NULL;
HistoryEventNode *lastHistoryEvent = NULL;

BOOL ItsTimeToDelete(HANDLE hContact, HANDLE hDbEvent, DBEVENTINFO *dbe = NULL, int eventNum = -1);
void AppendHistoryEvent(HANDLE hContact, HANDLE hDbEvent);
BOOL MsgWndOpen(HANDLE hContact);


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


static const MUUID interfaces[] = { MIID_HISTORYEVENTS, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link) 
{
	pluginLink = link;

	// TODO Assert results here
	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);
	mir_getLI(&li);

	hDeleteThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Default events
	RegisterDefaultEventType("message", "Message", EVENTTYPE_MESSAGE, 
							 HISTORYEVENTS_FLAG_SHOW_IM_SRMM 
							 | HISTORYEVENTS_FLAG_USE_SENT_FLAG
							 | HISTORYEVENTS_FLAG_EXPECT_CONTACT_NAME_BEFORE, 
							 "core_main_1", GetMessageHistoryEventText);
	RegisterDefaultEventType("url", "URL", EVENTTYPE_URL, 
							 HISTORYEVENTS_FLAG_SHOW_IM_SRMM 
							 | HISTORYEVENTS_FLAG_USE_SENT_FLAG
							 | HISTORYEVENTS_FLAG_EXPECT_CONTACT_NAME_BEFORE, 
							 "core_main_2", GetDefaultHistoryEventText);
	//RegisterDefaultEventType("contacts", "Contacts", EVENTTYPE_CONTACTS);
	//RegisterDefaultEventType("added", "Added", EVENTTYPE_ADDED);
	//RegisterDefaultEventType("authrequest", "Auth Request", EVENTTYPE_AUTHREQUEST);
	RegisterDefaultEventType("file", "File Transfer", EVENTTYPE_FILE, 
							 HISTORYEVENTS_FLAG_SHOW_IM_SRMM 
							 | HISTORYEVENTS_FLAG_USE_SENT_FLAG
							 | HISTORYEVENTS_FLAG_EXPECT_CONTACT_NAME_BEFORE, 
							 "core_main_3", GetFileHistoryEventText);

	CreateServiceFunction(MS_HISTORYEVENTS_GET_COUNT, ServiceGetCount);
	CreateServiceFunction(MS_HISTORYEVENTS_GET_EVENT, ServiceGetEvent);
	CreateServiceFunction(MS_HISTORYEVENTS_REGISTER, ServiceRegister);
	CreateServiceFunction(MS_HISTORYEVENTS_CAN_HANDLE, ServiceCanHandle);
	CreateServiceFunction(MS_HISTORYEVENTS_GET_ICON, ServiceGetIcon);
	CreateServiceFunction(MS_HISTORYEVENTS_GET_FLAGS, ServiceGetFlags);
	CreateServiceFunction(MS_HISTORYEVENTS_GET_TEXT, ServiceGetText);
	CreateServiceFunction(MS_HISTORYEVENTS_RELEASE_TEXT, ServiceReleaseText);
	CreateServiceFunction(MS_HISTORYEVENTS_ADD_TO_HISTORY, ServiceAddToHistory);
	CreateServiceFunction(MS_HISTORYEVENTS_IS_ENABLED_TEMPLATE, ServiceIsEnabledTemplate);
	
	// hooks
	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hHooks[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	hHooks[2] = HookEvent(ME_DB_EVENT_FILTER_ADD, DbEventFilterAdd);
	hHooks[3] = HookEvent(ME_DB_EVENT_ADDED, DbEventAdded);

	InitOptions();

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

    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://pescuma.org/miranda/historyevents_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.org/miranda/historyevents#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"HistoryEvents ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/historyeventsW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/historyevents.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	InitializeCriticalSection(&cs);
	DWORD threadID;
	CreateThread(NULL, 0, DeleteThread, NULL, 0, &threadID);

	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	if (shuttingDown == 0)
		shuttingDown = 1;
	SetEvent(hDeleteThreadEvent);
	int count = 0;
	while(shuttingDown != 2 && ++count < 50)
		Sleep(10);

	DeInitOptions();

	for(int i = 0; i < MAX_REGS(hHooks); i++)
		if (hHooks[i] != NULL)
			UnhookEvent(hHooks[i]);

	while(handlers.getCount() > 0)
	{
		mir_free(handlers[0]->templates);
		mir_free(handlers[0]);
		handlers.remove(0);
	}

	return 0;
}


// Avoid add to db if KEEP_DONT
int DbEventFilterAdd(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO *dbe = (DBEVENTINFO *) lParam;
	if (dbe == NULL)
		return 0;

	// Get handler
	HISTORY_EVENT_HANDLER *heh = GetHandler(dbe->eventType);
	if (heh == NULL)
		return 0;

	int keepInDB = KEEP_FLAG(heh->flags);

	if (keepInDB == HISTORYEVENTS_FLAG_KEEP_DONT)
		return 1;

	if (keepInDB == HISTORYEVENTS_FLAG_KEEP_FOR_SRMM && !(heh->flags & HISTORYEVENTS_FLAG_SHOW_IM_SRMM))
		return 1;

	return 0;
}

HICON LoadIconEx(HISTORY_EVENT_HANDLER *heh, bool copy)
{
	if (heh->defaultIcon != NULL)
		return IcoLib_LoadIcon((char *) heh->defaultIcon, copy);

	char name[128];
	mir_snprintf(name, sizeof(name), "historyevent_%s", heh->name);
	return IcoLib_LoadIcon(name, copy);
}


int GetHandlerID(WORD eventType)
{
	for (int i = 0; i < handlers.getCount(); i++)
	{
		if (handlers[i]->eventType == eventType)
			return i;
	}

	return -1;
}


HISTORY_EVENT_HANDLER *GetHandler(WORD eventType)
{
	int pos = GetHandlerID(eventType);

	if (pos >= 0)
		return handlers[pos];
	else
		return NULL;
}


void RegisterDefaultEventType(char *name, char *description, WORD eventType, int flags, char *icon, fGetHistoryEventText pfGetHistoryEventText)
{
	HISTORY_EVENT_HANDLER *heh = (HISTORY_EVENT_HANDLER *) mir_alloc0(sizeof(HISTORY_EVENT_HANDLER));
	heh->name = name;
	heh->description = Translate(description);
	heh->eventType = eventType;
	heh->defaultIconName = icon;
	heh->supports = HISTORYEVENTS_FORMAT_CHAR | HISTORYEVENTS_FORMAT_WCHAR | HISTORYEVENTS_FORMAT_RICH_TEXT;
	heh->flags = GetSettingDword(heh, FLAGS, flags) | HISTORYEVENTS_FLAG_DEFAULT | HISTORYEVENTS_REGISTERED_IN_ICOLIB;
	heh->pfGetHistoryEventText = pfGetHistoryEventText;

	if (icon == NULL)
	{
		char name[128];
		mir_snprintf(name, sizeof(name), "historyevent_%s", heh->name);
		heh->defaultIconName = strdup(name);

		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);
		sid.flags = SIDF_SORTED;
		sid.pszSection = Translate("History/Events");
		sid.pszDescription = heh->description;
		sid.pszName = heh->defaultIconName;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM) &sid);
	}

	handlers.insert(heh);
}


int ServiceRegister(WPARAM wParam, LPARAM lParam)
{
	HISTORY_EVENT_HANDLER *orig = (HISTORY_EVENT_HANDLER *) wParam;
	if (orig == NULL
		|| orig->cbSize < sizeof(HISTORY_EVENT_HANDLER)
		|| orig->name == NULL
		|| orig->name[0] == '\0'
		|| orig->description == NULL
		|| orig->description[0] == '\0'
		|| orig->eventType == 0
		|| (orig->pfGetHistoryEventText != NULL && (orig->supports & (HISTORYEVENTS_FORMAT_CHAR | HISTORYEVENTS_FORMAT_WCHAR)) == 0)
		|| orig->flags == -1)
		return -1;

	// Already have it?
	if (GetHandler(orig->eventType) != NULL)
		return -2;

	// Lets add it
	HISTORY_EVENT_HANDLER *heh = (HISTORY_EVENT_HANDLER *) mir_alloc0(sizeof(HISTORY_EVENT_HANDLER));
	*heh = *orig;
	heh->name = strdup(orig->name);
	heh->description = Translate(orig->description);
	if (heh->description == orig->description)
		heh->description = strdup(orig->description);
	heh->flags = GetSettingDword(orig, FLAGS, orig->flags) | HISTORYEVENTS_REGISTERED_IN_ICOLIB;
	if (orig->flags & HISTORYEVENTS_REGISTERED_IN_ICOLIB)
	{
		heh->defaultIconName = strdup(orig->defaultIconName);
	}
	else
	{
		char name[128];
		mir_snprintf(name, sizeof(name), "historyevent_%s", orig->name);
		heh->defaultIconName = strdup(name);

		// Add to icolib
		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);
		sid.flags = SIDF_SORTED;
		sid.pszSection = Translate("History/Events");
		sid.pszDescription = heh->description;
		sid.pszName = heh->defaultIconName;
		sid.hDefaultIcon = orig->defaultIcon;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM) &sid);
	}

	if (orig->pfGetHistoryEventText == NULL)
	{
		heh->pfGetHistoryEventText = GetDefaultHistoryEventText;
		heh->supports = HISTORYEVENTS_FORMAT_CHAR | HISTORYEVENTS_FORMAT_WCHAR | HISTORYEVENTS_FORMAT_RICH_TEXT;
	}

	if (orig->numTemplates > 0)
	{
		char **templates = (char **) mir_alloc0(orig->numTemplates * sizeof(char *));
		for(int i = 0; i < orig->numTemplates; i++)
			templates[i] = strdup(orig->templates[i] == NULL ? "" : orig->templates[i]);
		heh->templates = templates;
	}

	handlers.insert(heh);
	return 0;
}


int ServiceCanHandle(WPARAM wParam, LPARAM lParam)
{
	HISTORY_EVENT_HANDLER *heh = GetHandler(wParam);
	return heh != NULL;
}


int ServiceGetIcon(WPARAM wParam, LPARAM lParam)
{
	// Get handler
	HISTORY_EVENT_HANDLER *heh = GetHandler(wParam);
	if (heh == NULL)
		return NULL;

	// Get icon
	return (int) LoadIconEx(heh, lParam != FALSE);
}


int ServiceGetFlags(WPARAM wParam, LPARAM lParam)
{
	// Get handler
	HISTORY_EVENT_HANDLER *heh = GetHandler(wParam);
	if (heh == NULL)
		return -1;

	// Get icon
	return heh->flags;
}


int ServiceGetText(WPARAM wParam, LPARAM lParam)
{
	HISTORY_EVENT_PARAM *hep = (HISTORY_EVENT_PARAM *) wParam;
	if (hep == NULL
		|| hep->cbSize < sizeof(HISTORY_EVENT_PARAM)
		|| (hep->format & (HISTORYEVENTS_FORMAT_CHAR | HISTORYEVENTS_FORMAT_WCHAR | HISTORYEVENTS_FORMAT_RICH_TEXT)) == 0
		|| hep->hDbEvent == NULL)
		return NULL;

	// Get event type
	DBEVENTINFO dbeTmp = {0};
	DBEVENTINFO *dbe;
	if (hep->dbe != NULL && hep->dbe->cbBlob != NULL)
	{
		dbe = hep->dbe;
	}
	else
	{
		dbeTmp.cbSize = sizeof(dbeTmp);
		dbeTmp.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (LPARAM) hep->hDbEvent, 0);
		if (dbeTmp.cbBlob <= 0)
			return NULL;
		if (dbeTmp.cbBlob > 0)
			dbeTmp.pBlob = (PBYTE) malloc(dbeTmp.cbBlob);

		if (CallService(MS_DB_EVENT_GET, (LPARAM) hep->hDbEvent, (WPARAM) &dbeTmp) != 0)
		{
			free(dbeTmp.pBlob);
			return NULL;
		}
		dbe = &dbeTmp;
	}

	// Get handler
	HISTORY_EVENT_HANDLER *heh = GetHandler(dbe->eventType);
	if (heh == NULL)
	{
		if (hep->dbe != dbe && dbe->pBlob != NULL)
			free(dbe->pBlob);
		return NULL;
	}

	HANDLE hContact = (HANDLE) CallService(MS_DB_EVENT_GETCONTACT, (WPARAM) hep->hDbEvent, 0);

	if (ItsTimeToDelete(hContact, hep->hDbEvent, dbe))
		AppendHistoryEvent(hContact, hep->hDbEvent);


	// Get text
	void *ret;
	if (hep->format & HISTORYEVENTS_FORMAT_CHAR)
	{
		if (heh->supports & HISTORYEVENTS_FORMAT_CHAR)
		{
			ret = heh->pfGetHistoryEventText(hContact, hep->hDbEvent, dbe, HISTORYEVENTS_FORMAT_CHAR);
		}
		else
		{
			wchar_t *tmp = (wchar_t *) heh->pfGetHistoryEventText(hContact, hep->hDbEvent, dbe, HISTORYEVENTS_FORMAT_WCHAR);
			ret = mir_u2a(tmp);
			mir_free(tmp);
		}
	}
	else if (hep->format & HISTORYEVENTS_FORMAT_WCHAR)
	{
		if (heh->supports & HISTORYEVENTS_FORMAT_WCHAR)
		{
			ret = heh->pfGetHistoryEventText(hContact, hep->hDbEvent, dbe, HISTORYEVENTS_FORMAT_WCHAR);
		}
		else
		{
			char *tmp = (char *) heh->pfGetHistoryEventText(hContact, hep->hDbEvent, dbe, HISTORYEVENTS_FORMAT_CHAR);
			ret = mir_a2u(tmp);
			mir_free(tmp);
		}
	}
	else if (hep->format & HISTORYEVENTS_FORMAT_RICH_TEXT)
	{
		if (heh->supports & HISTORYEVENTS_FORMAT_RICH_TEXT)
		{
			ret = heh->pfGetHistoryEventText(hContact, hep->hDbEvent, dbe, HISTORYEVENTS_FORMAT_RICH_TEXT);
		}
		else if (heh->supports & HISTORYEVENTS_FORMAT_WCHAR)
		{
			wchar_t *tmp = (wchar_t *) heh->pfGetHistoryEventText(hContact, hep->hDbEvent, dbe, HISTORYEVENTS_FORMAT_WCHAR);
			ret = ConvertToRTF(tmp);
			mir_free(tmp);
		}
		else
		{
			char *tmp = (char *) heh->pfGetHistoryEventText(hContact, hep->hDbEvent, dbe, HISTORYEVENTS_FORMAT_CHAR);
			ret = ConvertToRTF(tmp);
			mir_free(tmp);
		}
	}

	if (hep->dbe != dbe && dbe->pBlob != NULL)
		free(dbe->pBlob);
	return (int) ret;
}


int ServiceReleaseText(WPARAM wParam, LPARAM lParam)
{
	mir_free((void *) wParam);
	return 0;
}


void GetTemplare(Buffer<TCHAR> *buffer, HISTORY_EVENT_HANDLER *heh, int templ)
{
	DBVARIANT dbv;

	char setting[128];
	mir_snprintf(setting, MAX_REGS(setting), "%d_%d_" TEMPLATE_TEXT, heh->eventType, templ);

	if (!DBGetContactSettingTString(NULL, heh->module == NULL ? MODULE_NAME : heh->module, setting, &dbv))
	{
		buffer->append(dbv.ptszVal);
		DBFreeVariant(&dbv);
	}
	else
	{
		// Get default
		char *tmp = heh->templates[templ];
		tmp = strchr(tmp, '\n');
		if (tmp == NULL)
			return;
		tmp++;
		char *end = strchr(tmp, '\n');
		size_t len = (end == NULL ? strlen(tmp) : end - tmp);

		buffer->append(tmp, len);
	}
}


HANDLE GetMetaContact(HANDLE hContact)
{
	if (!ServiceExists(MS_MC_GETMETACONTACT))
		return NULL;

	return (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM) hContact, 0);
}


int ServiceAddToHistory(WPARAM wParam, LPARAM lParam)
{
	HISTORY_EVENT_ADD * heaIn = (HISTORY_EVENT_ADD *) wParam;
	if (heaIn == NULL || (heaIn->cbSize < sizeof(HISTORY_EVENT_ADD) && heaIn->cbSize != SIZEOF_HISTORY_EVENT_ADD_V1) || heaIn->templateNum < 0)
		return NULL;
	HISTORY_EVENT_ADD hea = {0};
	memmove(&hea, heaIn, heaIn->cbSize);

	HISTORY_EVENT_HANDLER *heh = GetHandler(hea.eventType);
	if (heh == NULL || heh->numTemplates < 1 || heh->numTemplates < hea.templateNum)
		return NULL;

	if (!GetSettingBool(heh, hea.templateNum, TEMPLATE_ENABLED, TRUE))
		return NULL;

	if (heh->flags & HISTORYEVENTS_FLAG_ONLY_LOG_IF_SRMM_OPEN)
		if (!MsgWndOpen(hea.hContact))
			return NULL;

	Buffer<TCHAR> templ;
	GetTemplare(&templ, heh, hea.templateNum);
	templ.pack();

	Buffer<TCHAR> buffer;
	ReplaceTemplate(&buffer, hea.hContact, templ.str, hea.variables, hea.numVariables);
	buffer.pack();
	if (buffer.str == NULL)
		return NULL;

	char *text = mir_utf8encodeT(buffer.str);
	buffer.free();

	DBEVENTINFO event = { 0 };
	event.cbSize = sizeof(event);
	event.eventType = heh->eventType;
	event.flags = DBEF_UTF | hea.flags;
	event.timestamp = (hea.timestamp == 0 ? (DWORD) time(NULL) : hea.timestamp);
	event.szModule = heh->module;

	size_t size = strlen(text) + 1;

	if (hea.additionalDataSize > 0 && hea.additionalData != NULL)
	{
		text = (char *) mir_realloc(text, size + hea.additionalDataSize);
		memmove(&text[size], hea.additionalData, hea.additionalDataSize);
		size += hea.additionalDataSize;
	}

	event.pBlob = (PBYTE) text;
	event.cbBlob = size;

	HANDLE ret = (HANDLE) CallService(MS_DB_EVENT_ADD, (WPARAM) hea.hContact, (LPARAM) &event);

	if (hea.addToMetaToo)
	{
		HANDLE hMeta = GetMetaContact(hea.hContact);
		if (hMeta != NULL)
			CallService(MS_DB_EVENT_ADD, (WPARAM) hMeta, (LPARAM) &event);
	}

	mir_free(text);

	return (int) ret;
}

int ServiceIsEnabledTemplate(WPARAM wParam, LPARAM lParam)
{
	WORD eventType = wParam;
	int templateNum = lParam;

	HISTORY_EVENT_HANDLER *heh = GetHandler(eventType);
	if (heh == NULL || heh->numTemplates < 1 || heh->numTemplates < templateNum)
		return FALSE;

	return GetSettingBool(heh, templateNum, TEMPLATE_ENABLED, TRUE);
}


char * TrimRight(char *str) {
	int e;
	for(e = strlen(str)-1; e >= 0 && (str[e] == ' ' || str[e] == '\t' || str[e] == '\r' || str[e] == '\n'); e--) ;
	str[e+1] = '\0';
	return str;
}

wchar_t * TrimRight(wchar_t *str) {
	int e;
	for(e = lstrlenW(str)-1; e >= 0 && (str[e] == L' ' || str[e] == L'\t' || str[e] == L'\r' || str[e] == L'\n'); e--) ;
	str[e+1] = L'\0';
	return str;
}


void * GetMessageHistoryEventText(HANDLE hContact, HANDLE hDbEvent, DBEVENTINFO *dbe, int format)
{
	void *ret;

	if (format & HISTORYEVENTS_FORMAT_CHAR)
	{
		ret = TrimRight(DbGetEventTextA(dbe, CP_ACP));
	}
	else if (format & HISTORYEVENTS_FORMAT_WCHAR)
	{
		ret = TrimRight(DbGetEventTextW(dbe, CP_ACP));
	}
	else if (format & HISTORYEVENTS_FORMAT_RICH_TEXT)
	{
		TCHAR *tmp = TrimRight(DbGetEventTextT(dbe, CP_ACP));

		// Check if there is any extra info
		BYTE *extra = NULL;
		// Removed by now
		//if (dbe->flags & DBEF_UTF)
		//{
		//	size_t size = strlen((char *) dbe->pBlob) + 1;
		//	if (size < dbe->cbBlob)
		//		extra = &dbe->pBlob[size];
		//}

		ret = ConvertToRTF(tmp, GetHandler(dbe->eventType), extra);
		mir_free(tmp);
	}

	return ret;
}


void * GetDefaultHistoryEventText(HANDLE hContact, HANDLE hDbEvent, DBEVENTINFO *dbe, int format)
{
	void *ret;

	if (format & HISTORYEVENTS_FORMAT_CHAR)
	{
		ret = TrimRight(DbGetEventTextA(dbe, CP_ACP));
	}
	else if (format & HISTORYEVENTS_FORMAT_WCHAR)
	{
		ret = TrimRight(DbGetEventTextW(dbe, CP_ACP));
	}
	else if (format & HISTORYEVENTS_FORMAT_RICH_TEXT)
	{
		TCHAR *tmp = TrimRight(DbGetEventTextT(dbe, CP_ACP));
		ret = ConvertToRTF(tmp);
		mir_free(tmp);
	}

	return ret;
}


void * GetFileHistoryEventText(HANDLE hContact, HANDLE hDbEvent, DBEVENTINFO *dbe, int format)
{
	void *ret;

	char tmp[1024];
	char *filename = (char *) dbe->pBlob + sizeof(DWORD);
	char *descr = filename + lstrlenA(filename) + 1;
	if (*descr != 0)
		mir_snprintf(tmp, MAX_REGS(tmp), "%s (%s)", filename, descr);
	else
		mir_snprintf(tmp, MAX_REGS(tmp), "%s", filename);

	if (format & HISTORYEVENTS_FORMAT_CHAR)
	{
		ret = mir_strdup(tmp);
	}
	else if (format & HISTORYEVENTS_FORMAT_WCHAR)
	{
		ret = mir_a2u(tmp);
	}
	else if (format & HISTORYEVENTS_FORMAT_RICH_TEXT)
	{
		ret = ConvertToRTF(tmp);
	}

	return ret;
}


#define MESSAGE_INFO_FORMAT 1
#define MESSAGE_FORMAT_BOLD 1
#define MESSAGE_FORMAT_ITALIC 2
#define MESSAGE_FORMAT_UNDERLINE 4
#define MESSAGE_FORMAT_STRIKETHROUGH 8
#define MESSAGE_FORMAT_COLOR 16
#define MESSAGE_FORMAT_FONT 32


void ReadNextExtraInfo(BYTE *&extra, BYTE &format, int &start, BYTE &r, BYTE &g, BYTE &b, char *&font)
{
	format = 0;

	if (extra == NULL)
		return;

	if (*extra != MESSAGE_INFO_FORMAT)
		return;
	extra++;

	format = *extra; extra++;
	start = *((DWORD *) extra); extra+=4;

	if (format & MESSAGE_FORMAT_COLOR) 
	{
		r = *extra; extra++;
		g = *extra; extra++;
		b = *extra; extra++;
	}

	if (format & MESSAGE_FORMAT_FONT) 
	{
		font = (char *) extra;
		extra += strlen(font);
	}

	if (*extra != 0)
		OutputDebugStringA("Wrong extra format");

	extra++;
}


template<class T>
char * ConvertToRTF(T *line, HISTORY_EVENT_HANDLER *heh, BYTE *extra)
{
	Buffer<char> buffer;
	buffer.append("{\\uc1 ", 6);

	BYTE format;
	int start;
	BYTE r, g, b;
	char *font;

	ReadNextExtraInfo(extra, format, start, r, g, b, font);

	BOOL insideBlock = FALSE;
	T *orig = line;
	for (; *line; line++) 
	{
		if (format != 0 && line - orig == start)
		{
			if (insideBlock)
				buffer.append('}');

			if ((heh == NULL || GetSettingBool(heh, RESPECT_TEXT_FORMAT, TRUE))
				&& (format & (MESSAGE_FORMAT_BOLD | MESSAGE_FORMAT_ITALIC 
							  | MESSAGE_FORMAT_UNDERLINE | MESSAGE_FORMAT_STRIKETHROUGH)))
			{
				buffer.append('{');
				if (format & MESSAGE_FORMAT_BOLD)
					buffer.append("\\b", 2);
				if (format & MESSAGE_FORMAT_ITALIC)
					buffer.append("\\i", 2);
				if (format & MESSAGE_FORMAT_UNDERLINE)
					buffer.append("\\ul", 3);
				if (format & MESSAGE_FORMAT_STRIKETHROUGH)
					buffer.append("\\strike", 7);
				buffer.append(' ');

				insideBlock = TRUE;
			}

			ReadNextExtraInfo(extra, format, start, r, g, b, font);
		}

		if (*line == (T)'\r' && line[1] == (T)'\n') {
			buffer.append("\\line ", 6);
			line++;
		}
		else if (*line == (T)'\n') {
			buffer.append("\\line ", 6);
		}
		else if (*line == (T)'\t') {
			buffer.append("\\tab ", 5);
		}
		else if (*line == (T)'\\' || *line == (T)'{' || *line == (T)'}') {
			buffer.append('\\');
			buffer.append((char) *line);
		}
		else if (*line < 128) {
			buffer.append((char) *line);
		}
		else 
			buffer.appendPrintf("\\u%d ?", *line);
	}

	if (insideBlock)
		buffer.append('}');
	buffer.append('}');
	buffer.pack();

	return buffer.detach();
}


BOOL MsgWndOpen(HANDLE hContact)
{
	if (ServiceExists("SRMsg_MOD/MessageDialogOpened"))		// tabSRMM service
	{
		return CallService("SRMsg_MOD/MessageDialogOpened", (WPARAM) hContact, 0);
	}
	else
	{
		MessageWindowData mwd = {0};
		MessageWindowInputData mwid = {0};

		mwid.cbSize = sizeof(MessageWindowInputData); 
		mwid.hContact = hContact;
		mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
		mwd.cbSize = sizeof(MessageWindowData);
		mwd.hContact = hContact;
		if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM) &mwid, (LPARAM) &mwd))
			if (mwd.hwndWindow != NULL && (mwd.uState & MSG_WINDOW_STATE_EXISTS)) 
				return TRUE;
	}

	return FALSE;
}


BOOL ItsTimeToDelete(HANDLE hContact, HANDLE hDbEvent, DBEVENTINFO *dbe, int eventNum)
{
	// Get all info
	DBEVENTINFO dbeTmp = {0};
	if (dbe == NULL)
	{
		dbeTmp.cbSize = sizeof(dbeTmp);
		if (CallService(MS_DB_EVENT_GET, (LPARAM) hDbEvent, (WPARAM) &dbeTmp) != 0)
			return FALSE;
		dbe = &dbeTmp;
	}

	// Get handler
	HISTORY_EVENT_HANDLER *heh = GetHandler(dbe->eventType);
	if (heh == NULL)
		return FALSE;

	DWORD keepInDB = KEEP_FLAG(heh->flags);

	if (keepInDB != HISTORYEVENTS_FLAG_KEEP_DONT && !(dbe->flags & DBEF_SENT) && !(dbe->flags & DBEF_READ))
		return FALSE;

	switch(keepInDB)
	{
		case HISTORYEVENTS_FLAG_KEEP_DONT:
		{
			return TRUE;
		}
		case HISTORYEVENTS_FLAG_KEEP_ONE_YEAR:
		{
			return dbe->timestamp + 365 * 24 * 60 * 60 < (DWORD) time(NULL);
		}
		case HISTORYEVENTS_FLAG_KEEP_SIX_MONTHS:
		{
			return dbe->timestamp + (6 * 30 + 3) * 24 * 60 * 60 < (DWORD) time(NULL);
		}
		case HISTORYEVENTS_FLAG_KEEP_ONE_MONTH:
		{
			return dbe->timestamp + 31 * 24 * 60 * 60 < (DWORD) time(NULL);
		}
		case HISTORYEVENTS_FLAG_KEEP_ONE_WEEK:
		{
			return dbe->timestamp + 7 * 24 * 60 * 60 < (DWORD) time(NULL);
		}
		case HISTORYEVENTS_FLAG_KEEP_ONE_DAY:
		{
			return dbe->timestamp + 24 * 60 * 60 < (DWORD) time(NULL);
		}
		case HISTORYEVENTS_FLAG_KEEP_MAX_TEN:
		{
			return (eventNum > 10);
		}
		case HISTORYEVENTS_FLAG_KEEP_MAX_HUNDRED:
		{
			return (eventNum > 100);
		}
		case HISTORYEVENTS_FLAG_KEEP_FOR_SRMM:
		{
			// If it is open, let it be
			if (MsgWndOpen(hContact))
				return FALSE;

			// Check if it will be shown next time
			int load = DBGetContactSettingByte(NULL, "SRMM", "LoadHistory", LOADHISTORY_UNREAD);
			switch(load)
			{
				case LOADHISTORY_TIME:
				{
					DWORD dt = DBGetContactSettingWord(NULL, "SRMM", "LoadTime", 10) * 60;
					return dbe->timestamp + dt < (DWORD) time(NULL);
				}
				case LOADHISTORY_COUNT:
				{
					int count = DBGetContactSettingWord(NULL, "SRMM", "LoadCount", 10);

					HANDLE tmp = (HANDLE) CallService(MS_DB_EVENT_FINDLAST, (WPARAM) hContact, 0);

					int num = 1;
					while(tmp != NULL)
					{
						if (hDbEvent == tmp)
							return FALSE;

						// Get event
						DBEVENTINFO dbeTmp2 = {0};
						dbeTmp2.cbSize = sizeof(dbeTmp2);
						if (CallService(MS_DB_EVENT_GET, (LPARAM) tmp, (WPARAM) &dbeTmp2) != 0)
							continue;
											
						// Get handler
						HISTORY_EVENT_HANDLER *heh2 = GetHandler(dbeTmp2.eventType);
						if (heh2 == NULL)
							continue;

						if (!(heh2->flags & HISTORYEVENTS_FLAG_SHOW_IM_SRMM))
							continue;

						num++;
						if (num > count)
							break;

						tmp = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) tmp, 0);
					}
					return TRUE;
				}
				case LOADHISTORY_UNREAD:
				{
					return TRUE;
				}
			}

			return FALSE;
		}
	}

	return FALSE;
}


void wait(int time)
{
	if (!shuttingDown)
		WaitForSingleObject(hDeleteThreadEvent, time);
}


void DoFullPass()
{
	DWORD lastFulPass = DBGetContactSettingDword(NULL, MODULE_NAME, "LastFullPass", 0);
	DWORD now = (DWORD) time(NULL);
	if (now < lastFulPass + 24 * 60 * 60) // 1 day
		return;

	// Start after 1 minute
	wait(60 * 1000);
	if (shuttingDown)
		return;

//	DWORD t = GetTickCount();

	int *counters = (int *) malloc(sizeof(int) * handlers.getCount());

	int count = 0;
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL && !shuttingDown)
	{
		memset(counters, 0, sizeof(int) * handlers.getCount());

		HANDLE hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDLAST, (WPARAM) hContact, 0);
		while(hDbEvent != NULL && !shuttingDown)
		{
			HANDLE hDbEventProx = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) hDbEvent, 0);

			DBEVENTINFO dbe = {0};
			dbe.cbSize = sizeof(dbe);
			if (CallService(MS_DB_EVENT_GET, (LPARAM) hDbEvent, (WPARAM) &dbe) == 0)
			{
				int pos = GetHandlerID(dbe.eventType);
				if (pos >= 0)
				{
					counters[pos]++;

					if (ItsTimeToDelete(hContact, hDbEvent, &dbe, counters[pos]))
					{
						CallService(MS_DB_EVENT_DELETE, (WPARAM) hContact, (LPARAM) hDbEvent);
						wait(10);
					}
				}
			} 

			if (++count > 30)
			{
				count = 0;
				wait(10);
			}

			hDbEvent = hDbEventProx;
		}
		
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}

	free(counters);

//	t = GetTickCount() - t;
//	char tmp[128];
//	mir_snprintf(tmp, 128, "TIME: %d\n", (int) t);
//	OutputDebugStringA(tmp);

	if (!shuttingDown)
		DBWriteContactSettingDword(NULL, MODULE_NAME, "LastFullPass", now);
}


HistoryEventNode *GetFirstHistoryEvent()
{
	EnterCriticalSection(&cs);
	HistoryEventNode *ret = firstHistoryEvent;
	LeaveCriticalSection(&cs);
	return ret;
}


HistoryEventNode *GetNextHistoryEvent(HistoryEventNode *node)
{
	EnterCriticalSection(&cs);
	HistoryEventNode *ret = node->next;
	LeaveCriticalSection(&cs);
	return ret;
}


void DeleteHistoryEvent(HistoryEventNode *node, HistoryEventNode *prev)
{
	EnterCriticalSection(&cs);

	if (prev != NULL)
		prev->next = node->next;

	if (firstHistoryEvent == node)
		firstHistoryEvent = node->next;

	if (lastHistoryEvent == node)
		lastHistoryEvent = prev;

	LeaveCriticalSection(&cs);

	free(node);
}


void AppendHistoryEvent(HANDLE hContact, HANDLE hDbEvent)
{
	HistoryEventNode *node = (HistoryEventNode *) malloc(sizeof(HistoryEventNode));
	node->hContact = hContact;
	node->hDBEvent = hDbEvent;
	node->next = NULL;

	EnterCriticalSection(&cs);

	if (lastHistoryEvent == NULL)
		lastHistoryEvent = firstHistoryEvent = node;
	else
		lastHistoryEvent = lastHistoryEvent->next = node;

	LeaveCriticalSection(&cs);

	SetEvent(hDeleteThreadEvent);
}


DWORD WINAPI DeleteThread(LPVOID vParam)
{
	// First check if must pass through all contacts
	DoFullPass();

	HistoryEventNode *node = NULL;
	HistoryEventNode *prev = NULL;

	// Now delete the new events added
	while(!shuttingDown)
	{
		// Process next
		if (node == NULL)
			node = GetFirstHistoryEvent();
		else
			node = GetNextHistoryEvent(node);

		if (node == NULL)
		{
			wait(INFINITE);
		}
		else if (ItsTimeToDelete(node->hContact, node->hDBEvent))
		{
			int ret = CallService(MS_DB_EVENT_DELETE, (WPARAM) node->hContact, (LPARAM) node->hDBEvent);
			DeleteHistoryEvent(node, prev);
			node = prev;

			// Breath baby, breath
			wait(100);
		}
		prev = node;
	}

	shuttingDown = 2;
	return 0;
}


int DbEventAdded(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	HANDLE hDbEvent = (HANDLE) lParam;

	// Get all info
	DBEVENTINFO dbe = {0};
	dbe.cbSize = sizeof(dbe);
	if (CallService(MS_DB_EVENT_GET, (LPARAM) hDbEvent, (WPARAM) &dbe) != 0)
		return 0;

	// Get handler
	HISTORY_EVENT_HANDLER *heh = GetHandler(dbe.eventType);
	if (heh == NULL)
		return 0;

	if (KEEP_FLAG(heh->flags) != HISTORYEVENTS_FLAG_KEEP_FOR_SRMM)
		return 0;

	AppendHistoryEvent(hContact, hDbEvent);

	return 0;
}


int ServiceGetCount(WPARAM wParam, LPARAM lParam)
{
	return handlers.getCount();
}


int ServiceGetEvent(WPARAM wParam, LPARAM lParam)
{
	int pos = (int) wParam;
	if (pos >= 0)
	{
		if (pos < handlers.getCount())
			return (int) handlers[pos];
		else
			return NULL;
	}

	int type = (int) lParam;
	if (type != -1)
		return (int) GetHandler(type);

	return NULL;
}


int SortHandlers(const HISTORY_EVENT_HANDLER *type1, const HISTORY_EVENT_HANDLER *type2)
{
	if ((type1->flags & HISTORYEVENTS_FLAG_DEFAULT) && !(type2->flags & HISTORYEVENTS_FLAG_DEFAULT))
		return -1;
	else if (!(type1->flags & HISTORYEVENTS_FLAG_DEFAULT) && (type2->flags & HISTORYEVENTS_FLAG_DEFAULT))
		return 1;
	else
		return stricmp(type1->description, type2->description);
}