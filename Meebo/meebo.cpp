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


PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
#ifdef UNICODE
		"Meebo (Unicode)",
#else
		"Meebo",
#endif
		PLUGIN_MAKE_VERSION(0,0,0,1),
		"Meebo plugin for Jabber protocol",
		"Ricardo Pescuma Domenecci",
		"",
		"© 2007 Ricardo Pescuma Domenecci",
		"http://pescuma.mirandaim.ru/miranda/meebo",
		UNICODE_AWARE,
		0,		//doesn't replace anything built-in
#if defined( _UNICODE )
	{ 0x2dcd3555, 0x9be9, 0x4fbf, { 0x9c, 0xfb, 0x24, 0x29, 0xaf, 0xee, 0xeb, 0x96 } } // {2DCD3555-9BE9-4fbf-9CFB-2429AFEEEB96}
#else
	{ 0xeb3c7d40, 0xbb83, 0x463d, { 0xb2, 0xbf, 0x7e, 0x5, 0x7c, 0xdb, 0x39, 0xc4 } } // {EB3C7D40-BB83-463d-B2BF-7E057CDB39C4}
#endif
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;
LIST_INTERFACE li;

HANDLE hHooks[4] = {0};

LIST<JABBER_DATA> jabbers(2);

TCHAR *servers[] = {
	_T("guest.meebo.org"),
	_T("guest1.meebo.org"),
	_T("guest2.meebo.org"),
	_T("guest3.meebo.org"),
	_T("guest4.meebo.org"),
	_T("guest5.meebo.org"),
	_T("guest6.meebo.org"),
	_T("guest7.meebo.org"),
	_T("guest8.meebo.org"),
	_T("guest9.meebo.org"),
};


#define JABBER_FEAT_NICK	_T("http://jabber.org/protocol/nick")



int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);
int DBEventAdded(WPARAM wParam, LPARAM lParam);
int DBSettingChanged(WPARAM wParam, LPARAM lParam);

void RegisterJabberPlugin(const char *proto);
__inline static int ProtoServiceExists(const char *szModule,const char *szService);
static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


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


static const MUUID interfaces[] = { MIID_MEEBO, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link) 
{
	pluginLink = link;
	
	init_mir_malloc();

	li.cbSize = sizeof(li);
	CallService(MS_SYSTEM_GET_LI, 0, (LPARAM) &li);

	// hooks
	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hHooks[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	hHooks[2] = HookEvent(ME_DB_EVENT_ADDED, DBEventAdded);
	hHooks[3] = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
	
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	return 0;
}

// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];
		
		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;
		
		upd.szUpdateURL = UPDATER_AUTOREGISTER;
		
		upd.szBetaVersionURL = "http://pescuma.mirandaim.ru/miranda/meebo_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.mirandaim.ru/miranda/meebo#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"Meebo ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/meeboW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/meebo.zip";
#endif
		
		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);
		
        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

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
		RegisterJabberPlugin(protos[i]->szName);
	}

	
	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < MAX_REGS(hHooks); ++i)
		if (hHooks[i] != NULL)
			UnhookEvent(hHooks[i]);
	
	return 0;
}


BOOL OptAutoAuth(JABBER_DATA *data)
{
	return DBGetContactSettingByte(NULL, data->protocolName, "Meebome_AutoAuth", TRUE);
}


BOOL OptRemoveContactsWithoutHistory(JABBER_DATA *data)
{
	return DBGetContactSettingByte(NULL, data->protocolName, "Meebome_RemoveContactsWithoutHistory", TRUE);
}


BOOL OptRemoveContactsWithHistory(JABBER_DATA *data)
{
	return DBGetContactSettingByte(NULL, data->protocolName, "Meebome_RemoveContactsWithHistory", FALSE);
}


BOOL OptMove(JABBER_DATA *data)
{
	return DBGetContactSettingByte(NULL, data->protocolName, "Meebome_Move", FALSE);
}


JABBER_DATA * IsMeeboProtocol(const char *proto)
{
	for (int i = 0; i < jabbers.getCount(); i++)
		if (strcmp(jabbers[i]->protocolName, proto) == 0)
			return jabbers[i];

	return NULL;
}


BOOL IsMeeboMeContact(const char *proto, HANDLE hContact, TCHAR *jid = NULL)
{
	BOOL ret = FALSE;

	DBVARIANT dbv;
	if (jid != NULL || !DBGetContactSettingTString(hContact, proto, "jid", &dbv))
	{
		TCHAR *server = _tcschr(jid != NULL ? jid : dbv.ptszVal, _T('@'));
		if (server != NULL)
		{
			server++;

			for (int i = 0; i < MAX_REGS(servers); i++)
			{
				if (lstrcmp(server, servers[i]) == 0)
				{
					ret = TRUE;
					break;
				}
			}
		}

		if (jid == NULL)
			DBFreeVariant(&dbv);
	}

	if (!ret)
	{
		if (!DBGetContactSettingTString(hContact, proto, "MirVer", &dbv))
		{
			ret = (lstrcmp(_T("meebome"), dbv.ptszVal) == 0);
			DBFreeVariant(&dbv);
		}
	}

	return ret;
}


BOOL HasUsefullHistory(const char *proto, HANDLE hContact)
{
	return DBGetContactSettingByte(hContact, proto, "Meebome_KeepOnList", FALSE);
}


void DeleteContact(const char *proto, HANDLE hContact)
{
	// Check if protocol uses server side lists
	DWORD caps = (DWORD) CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0);
	if (caps & PF1_SERVERCLIST) 
	{
		int status = CallProtoService(proto, PS_GETSTATUS, 0, 0);
		if (status <= ID_STATUS_OFFLINE) 
		{
			// Set a flag so we remember to delete the contact when the protocol goes online the next time
			DBWriteContactSettingByte(hContact, "CList", "Delete", 1);
			DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
			return;
		}	
	}

	CallService(MS_DB_CONTACT_DELETE, (WPARAM) hContact, 0);
}


void CheckAndDeleteContact(JABBER_DATA *data, HANDLE hContact)
{
	if (IsMeeboMeContact(data->protocolName, hContact)) 
	{
		if (HasUsefullHistory(data->protocolName, hContact)) 
		{
			if (OptRemoveContactsWithHistory(data))
				DeleteContact(data->protocolName, hContact);
		}
		else
		{
			if (OptRemoveContactsWithoutHistory(data))
				DeleteContact(data->protocolName, hContact);
		}
	}
}


void CheckAndDeleteAllContacts(JABBER_DATA *data)
{
	if (!OptRemoveContactsWithHistory(data) && !OptRemoveContactsWithoutHistory(data))
		return;

	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL) 
	{
		HANDLE hNext = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);

		char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (proto != NULL && strcmp(proto, data->protocolName) == 0) 
			CheckAndDeleteContact(data, hContact);
		
		hContact = hNext;
	}
}

void OnDisconnect(void *param)
{
	JABBER_DATA *data = jabbers[(int) param];
	if (data == NULL)
		return;

	CheckAndDeleteAllContacts(data);
}


void FetchNick(JABBER_DATA *data, IXmlNode *node, HANDLE hContact)
{
	IXmlNode *nickNode = data->pfXmlGetChildByName(node, "nick");
	if (nickNode == NULL)
		return;

	const TCHAR *xmlns = data->pfXmlGetAttrValueStr(nickNode, "xmlns");
	if (xmlns == NULL || lstrcmp(xmlns, JABBER_FEAT_NICK) != 0)
		return;

	const TCHAR *nick = data->pfXmlGetNodeText(nickNode);
	if (nick == NULL)
		return;

	DBWriteContactSettingTString(hContact, data->protocolName, "Nick", nick);
}

void AfterXmlReceived(void *param, IXmlNode *node)
{
	JABBER_DATA *data = jabbers[(int) param];
	if (data == NULL)
		return;

	if (strcmp(data->pfXmlGetNodeName(node), "message") != 0)
		return;

	const TCHAR *from = data->pfXmlGetAttrValueStr(node, "from");
	if (from == NULL )
		return;

	HANDLE hContact = data->pfHContactFromJID(from);
	if (hContact == NULL)
		return;

	if (DBGetContactSettingByte(hContact, data->protocolName, "ChatRoom", 0))
		return;

	FetchNick(data, node, hContact);

	if (OptAutoAuth(data))
	{
		if (DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
		{
			if (OptMove(data))
			{
				DBVARIANT dbv;
				if (!DBGetContactSettingTString(NULL, data->protocolName, "Meebome_Group", &dbv))
				{
					DBWriteContactSettingTString(hContact, "CList", "Group", dbv.ptszVal);
					DBFreeVariant(&dbv);
				}
			}

			DBDeleteContactSetting(hContact, "CList", "NotOnList");
		}
	}
}


// Called when a new contact is added an an auth request must be shown
// Return TRUE to avoid further processing.
BOOL ProcessAuthRequest(void *param, HANDLE hContact, TCHAR *jid, TCHAR *nick, IXmlNode *node)
{
	JABBER_DATA *data = jabbers[(int) param];
	if (data == NULL || hContact == NULL || jid == NULL || nick == NULL || node == NULL)
		return FALSE;

	if (!IsMeeboMeContact(data->protocolName, hContact, jid))
		return FALSE;

	DBWriteContactSettingTString(hContact, data->protocolName, "MirVer", _T("meebome"));

	if (!OptAutoAuth(data))
		return FALSE;

	if (nick != NULL)
		DBWriteContactSettingTString(hContact, data->protocolName, "Nick", nick);

	if (OptMove(data))
	{
		DBVARIANT dbv;
		if (!DBGetContactSettingTString(NULL, data->protocolName, "Meebome_Group", &dbv))
		{
			DBWriteContactSettingTString(hContact, "CList", "Group", dbv.ptszVal);
			DBFreeVariant(&dbv);
		}
	}
	else
	{
		const TCHAR *group = data->pfXmlGetAttrValueStr(node, "group");
		if (group != NULL) 
			DBWriteContactSettingTString(hContact, "CList", "Group", group);
	}

	DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);

	IXmlNode *presence = data->pfXmlCreateNode("presence"); 
	data->pfXmlAddAttr(presence, "to", jid); 
	data->pfXmlAddAttr(presence, "type", _T("subscribed")); 
	data->pfSendNode(presence);
	data->pfXmlDeleteNode(presence);

	DBDeleteContactSetting(hContact, "CList", "NotOnList");

	return TRUE;
}


// Callback to add option dialogs
void AddOptions(void *param, WPARAM wParam)
{
	JABBER_DATA *data = jabbers[(int) param];
	if (data == NULL)
		return;

	char name[128];
	CallProtoService(data->protocolName, PS_GETNAME, MAX_REGS(name), (LPARAM) name);

	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize      = sizeof( odp );
	odp.hInstance   = hInst;
	odp.pszGroup    = "Network";
	odp.pszTitle    = name;
	odp.pszTab      = "Meebo";
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MEEBO);
	odp.pfnDlgProc  = OptionsDlgProc;
	odp.flags       = ODPF_BOLDGROUPS | ODPF_EXPERTONLY;
	odp.dwInitParam = (LPARAM) param;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) &odp);
}


void RegisterJabberPlugin(const char *proto)
{
	if (!ProtoServiceExists(proto, PS_REGISTER_JABBER_PLUGIN))
		return;

	JABBER_PLUGIN_DATA info = {
		sizeof(JABBER_PLUGIN_DATA),
		"Meebo",
		"This is a meebo server",
		FALSE,
		(void *) jabbers.getCount(),
		NULL,
		OnDisconnect,
		NULL,
		AfterXmlReceived,
		NULL,
		ProcessAuthRequest,
		AddOptions
	};
	
	
	JABBER_DATA *data = (JABBER_DATA *) CallProtoService(proto, PS_REGISTER_JABBER_PLUGIN, (WPARAM) &info, 1);
	if (data == NULL)
		// We are disabled / ignored
		return;
	
	jabbers.insert(data, jabbers.getCount());

	CheckAndDeleteAllContacts(data);
}


int DBEventAdded(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL || lParam == NULL)
		return 0;

	HANDLE hContact = (HANDLE) wParam;
	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);

	// Is it our protocol?
	JABBER_DATA *data = IsMeeboProtocol(proto);
	if (data == NULL)
		return 0;

	// Is it a meebome contact?
	if (!IsMeeboMeContact(proto, hContact))
		return 0;

	// Is this a valid event?
	HANDLE hDbEvent = (HANDLE) lParam;
	DBEVENTINFO dbe = {0};
	dbe.cbSize = sizeof(dbe);
	if (CallService(MS_DB_EVENT_GET, (LPARAM) hDbEvent, (WPARAM) &dbe) != 0)
		return 0;

	if (dbe.eventType != EVENTTYPE_MESSAGE && dbe.eventType != EVENTTYPE_URL && dbe.eventType != EVENTTYPE_FILE)
		return 0;

	// Ok, let's keep the contact
	DBWriteContactSettingByte(hContact, proto, "Meebome_KeepOnList", TRUE);

	return 0;
}


int DBSettingChanged(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL || lParam == NULL)
		return 0;

	DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*) lParam;
	if (strcmp(cws->szSetting, "Status") != 0 || cws->value.type != DBVT_WORD || cws->value.wVal > ID_STATUS_OFFLINE)
		return 0;

	HANDLE hContact = (HANDLE) wParam;
	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (proto == NULL || proto[0] == '\0')
		return 0;

	JABBER_DATA *data = IsMeeboProtocol(proto);
	if (data == NULL)
		return 0;
		
	CheckAndDeleteContact(data, hContact);

	return 0;
}


//call a specific protocol service. See the PS_ constants in m_protosvc.h
__inline static int ProtoServiceExists(const char *szModule,const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ NULL,	CONTROL_CHECKBOX,	IDC_AUTO_AUTHORIZE,			"Meebome_AutoAuth", TRUE },
	{ NULL,	CONTROL_CHECKBOX,	IDC_REMOVE_NO_HISTORY,		"Meebome_RemoveContactsWithoutHistory", TRUE },
	{ NULL,	CONTROL_CHECKBOX,	IDC_ALWAYS_REMOVE_HISTORY,	"Meebome_RemoveContactsWithHistory", FALSE },
	{ NULL,	CONTROL_CHECKBOX,	IDC_MOVE,					"Meebome_Move", FALSE },
	{ NULL,	CONTROL_TEXT,		IDC_GROUP,					"Meebome_Group", (DWORD) _T("meebo me") },
};

static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			JABBER_DATA *data = jabbers[(int) lParam];
			if (data == NULL)
				break;
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) data->protocolName);
			break;
		}
	}

	char *proto = (char *) GetWindowLong(hwndDlg, GWL_USERDATA);
	if (proto == NULL)
		return FALSE;

	return SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), proto, hwndDlg, msg, wParam, lParam);
}


