/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci
Based on work (C) Heiko Schillinger

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
	"Quick Contacts (Unicode)",
#else
	"Quick Contacts",
#endif
	PLUGIN_MAKE_VERSION(0,0,3,1),
	"Open contact-specific windows by hotkey",
	"Ricardo Pescuma Domenecci, Heiko Schillinger",
	"",
	"� 2007 Ricardo Pescuma Domenecci",
	"http://pescuma.mirandaim.ru/miranda/quickcontacts",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
#ifdef UNICODE
	{ 0xc679e1c9, 0x7967, 0x40ce, { 0x8a, 0x40, 0x95, 0x5b, 0x51, 0xde, 0x64, 0x3b } } // {C679E1C9-7967-40ce-8A40-955B51DE643B}
#else
	{ 0xd3cc7943, 0xff2e, 0x4c2a, { 0xb3, 0xac, 0x6c, 0xe9, 0xbc, 0x83, 0x18, 0x78 } } // {D3CC7943-FF2E-4c2a-B3AC-6CE9BC831878}
#endif
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;
HIMAGELIST hIml;
LIST_INTERFACE li;

HANDLE hModulesLoaded = NULL;
HANDLE hEventAdded = NULL;
HANDLE hHotkeyPressed = NULL;

long main_dialog_open = 0;
HWND hwndMain = NULL;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int EventAdded(WPARAM wparam, LPARAM lparam);
int HotkeyPressed(WPARAM wParam, LPARAM lParam);
int ShowDialog(WPARAM wParam,LPARAM lParam);
void FreeContacts();

int hksModule = 0;
int hksAction = 0;

BOOL hasNewHotkeyModule = FALSE;

#define IDC_ICO 12344


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


static const MUUID interfaces[] = { MIID_QUICKCONTACTS, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


extern "C" __declspec(dllexport) int Load(PLUGINLINK *link) {
	CLISTMENUITEM mi = {0};
	
	pluginLink = link;
	
	CreateServiceFunction(MS_QC_SHOW_DIALOG, ShowDialog);

	// hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hEventAdded = HookEvent(ME_DB_EVENT_ADDED, EventAdded);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void) 
{
	FreeContacts();

	DeInitOptions();

	UnhookEvent(hModulesLoaded);
	UnhookEvent(hEventAdded);

	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	init_mir_malloc();
	mir_getLI(&li);

	InitOptions();

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

		upd.szBetaVersionURL = "http://pescuma.mirandaim.ru/miranda/quickcontacts_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.mirandaim.ru/miranda/quickcontacts#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"Quick Contacts ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/quickcontactsW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/quickcontacts.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	// Get number of protocols
	int pcount;
	PROTOCOLDESCRIPTOR** pdesc;

	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)(int*)&pcount,(LPARAM)(PROTOCOLDESCRIPTOR***)&pdesc);

	opts.num_protos = 0;
	for (int loop=0;loop<pcount;loop++)
	{
		if (pdesc[loop]->type==PROTOTYPE_PROTOCOL)
			opts.num_protos++;
	}

	// Add hotkey to multiple services

	if (ServiceExists(MS_HOTKEY_REGISTER))
	{
		hasNewHotkeyModule = TRUE;

		HOTKEYDESC hkd = {0};
		hkd.cbSize = sizeof(hkd);
		hkd.pszName = Translate("Quick Contacts/Open dialog");
		hkd.pszDescription = Translate("Open dialog");
		hkd.pszSection = Translate("Quick Contacts");
		hkd.pszService = MS_QC_SHOW_DIALOG;
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_ALT, 'Q');
		CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

		hkd.pszService = NULL;

		hkd.lParam = HOTKEY_VOICE;
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'V');
		hkd.pszName = Translate("Quick Contacts/Voice");
		hkd.pszDescription = Translate("Make a voice call");
		CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

		hkd.lParam = HOTKEY_FILE;
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'F');
		hkd.pszName = Translate("Quick Contacts/File");
		hkd.pszDescription = Translate("Send file");
		CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

		hkd.lParam = HOTKEY_URL;
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'U');
		hkd.pszName = Translate("Quick Contacts/URL");
		hkd.pszDescription = Translate("Send URL");
		CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

		hkd.lParam = HOTKEY_INFO;
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'I');
		hkd.pszName = Translate("Quick Contacts/Info");
		hkd.pszDescription = Translate("Open userinfo");
		CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);
		
		hkd.lParam = HOTKEY_HISTORY;
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'H');
		hkd.pszName = Translate("Quick Contacts/History");
		hkd.pszDescription = Translate("Open history");
		CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);
		
		hkd.lParam = HOTKEY_MENU;
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'M');
		hkd.pszName = Translate("Quick Contacts/Menu");
		hkd.pszDescription = Translate("Open contact menu");
		CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);
		
		hkd.lParam = HOTKEY_ALL_CONTACTS;
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'A');
		hkd.pszName = Translate("Quick Contacts/All Contacts");
		hkd.pszDescription = Translate("Show all contacts");
		CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);
	}

	hksModule = HKS_RegisterModule("Quick Contacts");
	if (hksModule >= 0)
	{
		hksAction = HKS_RegisterAction(hksModule, "Open dialog", MOD_CONTROL | MOD_ALT | MOD_GLOBAL, 'Q', 0);

		hHotkeyPressed = HookEvent(ME_HKS_KEY_PRESSED, HotkeyPressed);
	}

	if (ServiceExists(MS_SKIN_ADDHOTKEY))
	{
		SKINHOTKEYDESCEX hk = {0};
		hk.cbSize = sizeof(hk);
		hk.pszSection = Translate("Quick Contacts");
		hk.pszName = Translate("Open dialog");
		hk.pszDescription = Translate("Open dialog");
		hk.pszService = MS_QC_SHOW_DIALOG;
		hk.DefHotKey = 0;
		CallService(MS_SKIN_ADDHOTKEY, 0, (LPARAM)&hk);
	}

	if (ServiceExists(MS_HOTKEYSPLUS_ADDKEY)) 
		CallService(MS_HOTKEYSPLUS_ADDKEY, (WPARAM) MS_QC_SHOW_DIALOG, (LPARAM) "Open Quick Contacts dialog");

	// Get the icons for the listbox
	hIml = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST,0,0);

	// Add menu item
	CLISTMENUITEM mi;
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 500100001;
	mi.flags = 0;
	mi.pszName = Translate("Quick Contacts...");
	mi.pszService = MS_QC_SHOW_DIALOG;
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	return 0;
}


// called when a message/file/url was sent
// handle of contact is set as window-userdata
int EventAdded(WPARAM wparam, LPARAM lparam)
{
	DBEVENTINFO dbei;

    ZeroMemory(&dbei,sizeof(dbei));
    dbei.cbSize=sizeof(dbei);
    dbei.cbBlob=0;
    
    CallService(MS_DB_EVENT_GET,lparam,(LPARAM)&dbei);
    
	if(		!(dbei.flags & DBEF_SENT) 
			|| dbei.flags & DBEF_READ 
			|| !DBGetContactSettingByte(NULL, MODULE_NAME, "EnableLastSentTo", 0) 
			|| DBGetContactSettingWord(NULL, MODULE_NAME, "MsgTypeRec", TYPE_GLOBAL) != TYPE_GLOBAL) 
		return 0;

	DBWriteContactSettingDword(NULL, MODULE_NAME, "LastSentTo", (DWORD)(HANDLE)wparam);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define IDC_ENTER 2000	// Pseudo control to handle enter in the main window


// array where the contacts are put into
struct c_struct {
	TCHAR szname[120];
	TCHAR szgroup[50];
	HANDLE hcontact;
	char proto[20];
};

LIST<c_struct> contacts(200);
long max_proto_width;


// Get the name the contact has in list
// This was not made to be called by more than one thread!
TCHAR tmp_list_name[120];

TCHAR *GetListName(c_struct *cs)
{
	if (opts.group_append && cs->szgroup[0] != _T('\0'))
	{
		mir_sntprintf(tmp_list_name, MAX_REGS(tmp_list_name), _T("%s (%s)"), cs->szname, cs->szgroup);
		return tmp_list_name;
	}
	else
	{
		return cs->szname;
	}
}



// simple sorting function to have
// the contact array in alphabetical order
void SortArray(void)
{
	int loop,doop;
	c_struct *cs_temp;

	SortedList *sl = (SortedList *) &contacts;
	for(loop=0;loop<contacts.getCount();loop++)
	{
		for(doop=loop+1;doop<contacts.getCount();doop++)
		{
			if(lstrcmp(contacts[loop]->szname,contacts[doop]->szname)>0)
			{
				cs_temp=contacts[loop];
				sl->items[loop]=contacts[doop];
				sl->items[doop]=cs_temp;
			}
			else if(!lstrcmp(contacts[loop]->szname,contacts[doop]->szname))
			{
				if(strcmp(contacts[loop]->proto,contacts[doop]->proto)>0)
				{
					cs_temp=contacts[loop];
					sl->items[loop]=contacts[doop];
					sl->items[doop]=cs_temp;
				}
			}

		}
	}
}


void FreeContacts()
{
	for (int i = contacts.getCount() - 1; i >= 0; i--)
	{
		delete contacts[i];
		contacts.remove(i);
	}
}


void LoadContacts(HWND hwndDlg, BOOL show_all)
{
	// Read last-sent-to contact from db and set handle as window-userdata
	HANDLE hlastsent = (HANDLE)DBGetContactSettingDword(NULL, MODULE_NAME, "LastSentTo", -1);
	SetWindowLong(hwndMain, GWL_USERDATA, (LONG)hlastsent);

	// enumerate all contacts and write them to the array
	// item data of listbox-strings is the array position
	FreeContacts();
	for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0); 
		hContact != NULL; 
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
	{
		char *pszProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if(pszProto != NULL)
		{
			// Get meta
			HANDLE hMeta = NULL;
			if ( ( (!show_all && opts.hide_subcontacts) || opts.group_append )
				 && ServiceExists(MS_MC_GETMETACONTACT))
			{
				hMeta = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);
			}

			if (!show_all)
			{
				// Check if is offline and have to show
				int status = DBGetContactSettingWord(hContact, pszProto, "Status", ID_STATUS_OFFLINE);
				if (status <= ID_STATUS_OFFLINE)
				{
					// See if has to show
					char setting[128];
					mir_snprintf(setting, sizeof(setting), "ShowOffline%s", pszProto);

					if (!DBGetContactSettingByte(NULL, MODULE_NAME, setting, FALSE))
						continue;

					// Check if proto offline
					else if (opts.hide_from_offline_proto 
							&& CallProtoService(pszProto, PS_GETSTATUS, 0, 0) <= ID_STATUS_OFFLINE)
						continue;

				}

				// Check if is subcontact
				if (opts.hide_subcontacts && hMeta != NULL) 
				{
					if (!opts.keep_subcontacts_from_offline)
						continue;

					int meta_status = DBGetContactSettingWord(hMeta, "MetaContacts", "Status", ID_STATUS_OFFLINE);

					if (meta_status > ID_STATUS_OFFLINE)
						continue;
					else if (DBGetContactSettingByte(NULL, MODULE_NAME, "ShowOfflineMetaContacts", FALSE))
						continue;
				}
			}

			// Add to list

			// Get group
			c_struct *contact = new c_struct();
			
			if (opts.group_append)
			{
				DBVARIANT dbv;
				if (DBGetContactSettingTString(hMeta == NULL ? hContact : hMeta, "CList", "Group", &dbv) == 0)
				{
					if (dbv.ptszVal != NULL)
						lstrcpyn(contact->szgroup, dbv.ptszVal, MAX_REGS(contact->szgroup));

					DBFreeVariant(&dbv);
				}
			}

			// Make contact name
			TCHAR *tmp = (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
			lstrcpyn(contact->szname, tmp, MAX_REGS(contact->szname));

			strncpy(contact->proto, pszProto, sizeof(contact->proto)-1);
			contact->proto[sizeof(contact->proto)-1] = '\0';

			contact->hcontact = hContact;

			contacts.insert(contact);
		}
	}

	SortArray();
			
	SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_RESETCONTENT, 0, 0);
	for(int loop = 0; loop < contacts.getCount(); loop++)
	{
		SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETITEMDATA, 
							(WPARAM)SendDlgItemMessage(hwndDlg, IDC_USERNAME, 
											CB_ADDSTRING, 0, (LPARAM) GetListName(contacts[loop])), 
							(LPARAM)loop);
	}
}


// Enable buttons for the selected contact
void EnableButtons(HWND hwndDlg, HANDLE hContact)
{
	if (hContact == NULL)
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_VOICE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_URL), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_USERINFO), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_HISTORY), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_MENU), FALSE);

		SendMessage(GetDlgItem(hwndDlg, IDC_ICO), STM_SETICON, 0, 0);
	}
	else
	{
		// Is a meta?
		if (ServiceExists(MS_MC_GETMOSTONLINECONTACT)) 
		{
			HANDLE hSub = (HANDLE) CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM) hContact, 0);
			if (hSub != NULL)
				hContact = hSub;
		}

		// Get caps
		int caps = 0;

		char *pszProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (pszProto != NULL)
			caps = CallProtoService(pszProto, PS_GETCAPS, PFLAGNUM_1, 0);

		BOOL voice = (ServiceExists(MS_VOICESERVICE_CAN_CALL) 
			&& CallService(MS_VOICESERVICE_CAN_CALL, (WPARAM)hContact, 0) > 0);

		EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), caps & PF1_IMSEND ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_VOICE), voice);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), caps & PF1_FILESEND ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_URL), caps & PF1_URLSEND ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_USERINFO), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_HISTORY), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_MENU), TRUE);

		HICON ico = ImageList_GetIcon(hIml, CallService(MS_CLIST_GETCONTACTICON, (WPARAM) hContact, 0), ILD_IMAGE);
		SendMessage(GetDlgItem(hwndDlg, IDC_ICO), STM_SETICON, (WPARAM) ico, 0);
	}
}

BOOL lstreq(TCHAR *a, TCHAR *b, size_t len = -1)
{
#ifdef UNICODE
	a = CharLower(_tcsdup(a));
	b = CharLower(_tcsdup(b));
	BOOL ret;
	if (len > 0)
		ret = !_tcsncmp(a, b, len);
	else
		ret = !_tcscmp(a, b);
	free(a);
	free(b);
	return ret;
#else
	if (len > 0)
		return !_tcsnicmp(a, b, len);
	else
		return !_tcsicmp(a, b);
#endif
}

// check if the char(s) entered appears in a contacts name
int CheckText(HWND hdlg, TCHAR *sztext, BOOL only_enable = FALSE)
{
	EnableButtons(hwndMain, NULL);

	if(sztext == NULL || sztext[0] == _T('\0'))
		return 0;

	int len = lstrlen(sztext);

	int loop;
	for(loop=0;loop<contacts.getCount();loop++)
	{
		if (only_enable)
		{
			if(lstreq(sztext, contacts[loop]->szname) || lstreq(sztext, GetListName(contacts[loop])))
			{
				EnableButtons(hwndMain, contacts[loop]->hcontact);
				return 0;
			}
		}
		else
		{
			if(lstreq(sztext, GetListName(contacts[loop]), len))
			{
				SendMessage(hdlg, WM_SETTEXT, 0, (LPARAM) GetListName(contacts[loop]));
				SendMessage(hdlg, EM_SETSEL, (WPARAM) len, (LPARAM) -1);
				EnableButtons(hwndMain, contacts[loop]->hcontact);
				return 0;
			}
		}
	}

	EnableButtons(hwndMain, NULL);
	return 0;
}

HANDLE GetSelectedContact(HWND hwndDlg)
{
	// First try selection
	int sel = SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_GETCURSEL, 0, 0);

	if (sel != CB_ERR)
	{
		int pos = SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_GETITEMDATA, sel, 0);
		if (pos != CB_ERR)
			return contacts[pos]->hcontact;
	}

	// Now try the name
	TCHAR cname[120] = _T("");

	GetDlgItemText(hwndDlg, IDC_USERNAME, cname, MAX_REGS(cname));
			
	for(int loop = 0; loop < contacts.getCount(); loop++)
	{
		if(!lstrcmpi(cname, GetListName(contacts[loop])))
			return contacts[loop]->hcontact;
	}

	return NULL;
}

// get array position from handle
int GetItemPos(HANDLE hcontact)
{
	int loop;

	for(loop=0;loop<contacts.getCount();loop++)
	{
		if(hcontact==contacts[loop]->hcontact)
			return loop;
	}
	return -1;
}


WNDPROC wpEditMainProc;

// callback function for edit-box of the listbox
// without this the autofill function isn't possible
// this was done like ie does it..as far as spy++ could tell ;)
LRESULT CALLBACK EditProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg)
	{
		case WM_CHAR:
		{
			if (wparam<32 && wparam != VK_BACK) 
				break;

			TCHAR sztext[120] = _T("");
			DWORD start;
			DWORD end;

			int ret = SendMessage(hdlg,EM_GETSEL,(WPARAM)&start,(LPARAM)&end);

			SendMessage(hdlg,WM_GETTEXT,(WPARAM)MAX_REGS(sztext),(LPARAM)sztext);

			BOOL at_end = (lstrlen(sztext) == (int)end);

			if (ret != -1)
			{
				if (wparam == VK_BACK)
				{
					if (start > 0)
						SendMessage(hdlg,EM_SETSEL,(WPARAM)start-1,(LPARAM)end);

					sztext[0]=0;
				}
				else
				{
					sztext[0]=wparam;
					sztext[1]=0;
				}

				SendMessage(hdlg,EM_REPLACESEL,(WPARAM)0,(LPARAM)sztext);
				SendMessage(hdlg,WM_GETTEXT,(WPARAM)MAX_REGS(sztext),(LPARAM)sztext);
			}

			CheckText(hdlg, sztext, !at_end);

			return 1;
		}
		case WM_KEYUP:
		{
			TCHAR sztext[120] = _T("");

			if (wparam == VK_RETURN)
			{
				switch(SendMessage(GetParent(hdlg),CB_GETDROPPEDSTATE,0,0))
				{
					case FALSE:
						SendMessage(GetParent(GetParent(hdlg)),WM_COMMAND,MAKEWPARAM(IDC_ENTER,STN_CLICKED),0);
						break;

					case TRUE:
						SendMessage(GetParent(hdlg),CB_SHOWDROPDOWN,(WPARAM)FALSE,0);
						break;
				}
			}
			else if (wparam == VK_DELETE)
			{
				SendMessage(hdlg,WM_GETTEXT,(WPARAM)MAX_REGS(sztext),(LPARAM)sztext);
				CheckText(hdlg, sztext, TRUE);
			}

			return 0;
		}
		case WM_GETDLGCODE:
			return DLGC_WANTCHARS|DLGC_WANTARROWS;

	}

	return CallWindowProc(wpEditMainProc,hdlg,msg,wparam,lparam);
}


HACCEL hAcct;
HHOOK hHook;

// This function filters the message queue and translates
// the keyboard accelerators
LRESULT CALLBACK HookProc(int code, WPARAM wparam, LPARAM lparam)
{
	MSG *msg;
	HWND htemp;

	if (code!=MSGF_DIALOGBOX) 
		return 0;

	msg = (MSG*)lparam;


	if (hasNewHotkeyModule)
	{
		int action = CallService(MS_HOTKEY_CHECK, (WPARAM) msg, (LPARAM) "Quick Contacts");
		if (action != 0)
		{
			SendMessage(hwndMain, WM_COMMAND, action, 0);
			return 1;
		}
	}
	else
	{
		htemp = msg->hwnd;
		msg->hwnd = hwndMain;

		if (TranslateAccelerator(msg->hwnd, hAcct, msg))
			return 1;
		
		msg->hwnd=htemp;
	}

	if (msg->message == WM_KEYDOWN && msg->wParam == VK_ESCAPE)
	{
		switch(SendMessage(GetDlgItem(hwndMain, IDC_USERNAME), CB_GETDROPPEDSTATE, 0, 0))
		{
			case FALSE:
				SendMessage(hwndMain, WM_CLOSE, 0, 0);
				break;

			case TRUE:
				SendMessage(GetDlgItem(hwndMain, IDC_USERNAME), CB_SHOWDROPDOWN, (WPARAM)FALSE, 0);
				break;
		}
	}
	
	return 0;
}

BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	BOOL ret;

	POINT pt;

	pt.x = lpRect->left;
	pt.y = lpRect->top;

	ret = ScreenToClient(hWnd, &pt);

	if (!ret) return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;


	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}


BOOL MoveWindow(HWND hWnd, const RECT &rect, BOOL bRepaint)
{
	return MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, bRepaint);
}


static void FillButton(HWND hwndDlg, int dlgItem, char *name, char *key, HICON icon)
{
	char tmp[256];
	char *full;
	if (key == NULL)
		full = Translate(name);
	else
		mir_snprintf(full = tmp, MAX_REGS(tmp), "%s (%s)", Translate(name), key);

	SendMessage(GetDlgItem(hwndDlg, dlgItem), BUTTONSETASFLATBTN, 0, 0);
	SendMessageA(GetDlgItem(hwndDlg, dlgItem), BUTTONADDTOOLTIP, (LPARAM) full, 0);
	SendDlgItemMessage(hwndDlg, dlgItem, BM_SETIMAGE, IMAGE_ICON, (LPARAM) icon);
}


static void FillCheckbox(HWND hwndDlg, int dlgItem, char *name, char *key)
{
	char tmp[256];
	char *full;
	if (key == NULL)
		full = Translate(name);
	else
		mir_snprintf(full = tmp, MAX_REGS(tmp), "%s (%s)", Translate(name), key);

	SendMessageA(GetDlgItem(hwndDlg, dlgItem), WM_SETTEXT, 0, (LPARAM) full);
}


static BOOL CALLBACK MainDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			RECT rc;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_USERNAME), &rc);
			ScreenToClient(hwndDlg, &rc);

			HWND icon = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE, 
                    rc.left - 20, rc.top + (rc.bottom - rc.top - 16) / 2, 16, 16, hwndDlg, (HMENU) IDC_ICO, 
					hInst, NULL);

			if (!hasNewHotkeyModule)
				hAcct = LoadAccelerators(hInst, MAKEINTRESOURCE(ACCEL_TABLE));

			hHook = SetWindowsHookEx(WH_MSGFILTER, HookProc, hInst, GetCurrentThreadId());

			// Combo
			SendMessage(GetDlgItem(hwndDlg, IDC_USERNAME), EM_LIMITTEXT, (WPARAM)119,0);
			wpEditMainProc = (WNDPROC) SetWindowLong(GetWindow(GetDlgItem(hwndDlg, IDC_USERNAME),GW_CHILD), GWL_WNDPROC, (LONG)EditProc);

			// Buttons
			FillCheckbox(hwndDlg, IDC_SHOW_ALL_CONTACTS, "Show all contacts", hasNewHotkeyModule ? NULL : "Ctrl+A");
			FillButton(hwndDlg, IDC_MESSAGE, "Send message", NULL, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));

			if (ServiceExists(MS_VOICESERVICE_CAN_CALL))
			{
				FillButton(hwndDlg, IDC_VOICE, "Make a voice call", hasNewHotkeyModule ? NULL : "Ctrl+V", (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "vca_call"));
			}
			else
			{
				GetWindowRect(GetDlgItem(hwndDlg, IDC_VOICE), &rc);
				ScreenToClient(hwndDlg, &rc);
				MoveWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), rc, FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_VOICE), SW_HIDE);
			}

			FillButton(hwndDlg, IDC_FILE, "Send file", hasNewHotkeyModule ? NULL : "Ctrl+F", LoadSkinnedIcon(SKINICON_EVENT_FILE));
			FillButton(hwndDlg, IDC_URL, "Send URL", hasNewHotkeyModule ? NULL : "Ctrl+U", LoadSkinnedIcon(SKINICON_EVENT_URL));
			FillButton(hwndDlg, IDC_USERINFO, "Open userinfo", hasNewHotkeyModule ? NULL : "Ctrl+I", (HICON) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(160),IMAGE_ICON,16,16,LR_DEFAULTCOLOR));
			FillButton(hwndDlg, IDC_HISTORY, "Open history", hasNewHotkeyModule ? NULL : "Ctrl+H", (HICON) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(174),IMAGE_ICON,16,16,LR_DEFAULTCOLOR));
			FillButton(hwndDlg, IDC_MENU, "Open contact menu", hasNewHotkeyModule ? NULL : "Ctrl+M", (HICON) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(264),IMAGE_ICON,16,16,LR_DEFAULTCOLOR));

			SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETEXTENDEDUI, (WPARAM)TRUE, 0);

			MagneticWindows_AddWindow(hwndDlg);

			Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE_NAME, "window");

			LoadContacts(hwndDlg, FALSE);

			EnableButtons(hwndDlg, NULL);
			if (DBGetContactSettingByte(NULL, MODULE_NAME, "EnableLastSentTo", 0))
			{
				int pos = GetItemPos((HANDLE) DBGetContactSettingDword(NULL, MODULE_NAME, "LastSentTo", -1));

				if (pos != -1)
				{
					SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETCURSEL, (WPARAM) pos, 0);
					EnableButtons(hwndDlg, contacts[pos]->hcontact);
				}
			}

			SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));

			return TRUE;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_USERNAME:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int pos = SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_GETCURSEL, 0, 0);
						EnableButtons(hwndDlg, pos < contacts.getCount() ? contacts[pos]->hcontact : NULL);
					}
					break;
				}
				case IDC_ENTER:
				{
					HANDLE hContact = GetSelectedContact(hwndDlg);
					if (hContact == NULL)
						break;

					CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM) hContact, 0);

					DBWriteContactSettingDword(NULL, MODULE_NAME, "LastSentTo", (DWORD) hContact);
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				}
				case IDC_MESSAGE:
				{
					HANDLE hContact = GetSelectedContact(hwndDlg);
					if (hContact == NULL)
					{
						SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

					// Is button enabled?
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_MESSAGE)))
						break;

					// don't know why it doesn't work with MS_MSG_SENDMESSAGE
					// when convers is enabled
					if (ServiceExists("SRMsg/LaunchMessageWindow"))
						CallService("SRMsg/LaunchMessageWindow", (WPARAM) hContact, 0);
					else
						CallService(MS_MSG_SENDMESSAGE, (WPARAM) hContact, 0);

					DBWriteContactSettingDword(NULL, MODULE_NAME, "LastSentTo", (DWORD) hContact);
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				}
				case HOTKEY_VOICE:
				case IDC_VOICE:
				{
					HANDLE hContact = GetSelectedContact(hwndDlg);
					if (hContact == NULL)
					{
						SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

					// Is button enabled?
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_VOICE)))
						break;

					if (!ServiceExists(MS_VOICESERVICE_CALL))
						break;

					CallService(MS_VOICESERVICE_CALL, (WPARAM) hContact, 0);

					DBWriteContactSettingDword(NULL, MODULE_NAME, "LastSentTo", (DWORD) hContact);
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				}
				case HOTKEY_FILE:
				case IDC_FILE:
				{
					HANDLE hContact = GetSelectedContact(hwndDlg);
					if (hContact == NULL)
					{
						SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

					// Is button enabled?
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_FILE)))
						break;

					CallService(MS_FILE_SENDFILE, (WPARAM) hContact, 0);

					DBWriteContactSettingDword(NULL, MODULE_NAME, "LastSentTo", (DWORD) hContact);
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				}
				case HOTKEY_URL:
				case IDC_URL:
				{
					HANDLE hContact = GetSelectedContact(hwndDlg);
					if (hContact == NULL)
					{
						SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

					// Is button enabled?
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_URL)))
						break;

					CallService(MS_URL_SENDURL, (WPARAM) hContact, 0);

					DBWriteContactSettingDword(NULL, MODULE_NAME, "LastSentTo", (DWORD) hContact);
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				}
				case HOTKEY_INFO:
				case IDC_USERINFO:
				{
					HANDLE hContact = GetSelectedContact(hwndDlg);
					if (hContact == NULL)
					{
						SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

					// Is button enabled?
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_USERINFO)))
						break;

					CallService(MS_USERINFO_SHOWDIALOG, (WPARAM) hContact, 0);

					DBWriteContactSettingDword(NULL, MODULE_NAME, "LastSentTo", (DWORD) hContact);
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				}
				case HOTKEY_HISTORY:
				case IDC_HISTORY:
				{
					HANDLE hContact = GetSelectedContact(hwndDlg);
					if (hContact == NULL)
					{
						SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

					// Is button enabled?
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_HISTORY)))
						break;

					CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM) hContact, 0);

					DBWriteContactSettingDword(NULL, MODULE_NAME, "LastSentTo", (DWORD) hContact);
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				}
				case HOTKEY_MENU:
				case IDC_MENU:
				{
					HANDLE hContact = GetSelectedContact(hwndDlg);
					if (hContact == NULL)
					{
						SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

					// Is button enabled?
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_MENU)))
						break;

                    RECT rc;
                    GetWindowRect(GetDlgItem(hwndDlg, IDC_MENU), &rc);
                    HMENU hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM) hContact, 0);
                    int ret = TrackPopupMenu(hMenu, TPM_TOPALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, rc.left, rc.bottom, 0, hwndDlg, NULL);
                    DestroyMenu(hMenu);

					if(ret)
					{
						SendMessage(hwndDlg, WM_CLOSE, 0, 0);
						CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(ret),MPCF_CONTACTMENU),(LPARAM) hContact);
					}

					DBWriteContactSettingDword(NULL, MODULE_NAME, "LastSentTo", (DWORD) hContact);
					break;
				}
				case HOTKEY_ALL_CONTACTS:
				case IDC_SHOW_ALL_CONTACTS:
				{
					// Get old text
					HWND hEdit = GetWindow(GetWindow(hwndDlg,GW_CHILD),GW_CHILD);
					TCHAR sztext[120] = _T("");

					if (SendMessage(hEdit, EM_GETSEL, (WPARAM)NULL, (LPARAM)NULL) != -1)
						SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)_T(""));

					SendMessage(hEdit, WM_GETTEXT, (WPARAM)MAX_REGS(sztext), (LPARAM)sztext);

					// Fill combo			
					BOOL all = IsDlgButtonChecked(hwndDlg, IDC_SHOW_ALL_CONTACTS);

					if (LOWORD(wParam) == HOTKEY_ALL_CONTACTS)
					{
						// Toggle checkbox
						all = !all;
						CheckDlgButton(hwndDlg, IDC_SHOW_ALL_CONTACTS, all ? BST_CHECKED : BST_UNCHECKED);
					}

					LoadContacts(hwndDlg, all);

					// Return selection
					CheckText(hEdit, sztext);

					break;
				}
			}

			break;
		}

		case WM_CLOSE:
		{
			Utils_SaveWindowPosition(hwndDlg, NULL, MODULE_NAME, "window");
			MagneticWindows_RemoveWindow(hwndDlg);
			DestroyWindow(hwndDlg);
			break;
		}

		case WM_DESTROY:
		{
			UnhookWindowsHookEx(hHook);
			hwndMain = NULL;
			FreeContacts();
			InterlockedExchange(&main_dialog_open, 0);
			break;
		}

		case WM_NCLBUTTONDBLCLK:
		{
			MagneticWindows_SnapWindowToList(hwndDlg, MS_MW_STL_List_Left | MS_MW_STL_List_Top
													| MS_MW_STL_Wnd_Right | MS_MW_STL_Wnd_Top);
			break;
		}
		
		case WM_DRAWITEM:
		{
			// add icons and protocol to listbox
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

			// Handle contact menu
			if(lpdis->CtlID != IDC_USERNAME) 
			{
				if (lpdis->CtlType == ODT_MENU)
					return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
				else
					break;
			}

			// Handle combo
			if(lpdis->itemID == -1) 
				break;

			TEXTMETRIC tm;
			int icon_width=0, icon_height=0;
			RECT rc;

			GetTextMetrics(lpdis->hDC, &tm);
			ImageList_GetIconSize(hIml, &icon_width, &icon_height);

			COLORREF clrfore = SetTextColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_SELECTED?COLOR_HIGHLIGHTTEXT:COLOR_WINDOWTEXT));
			COLORREF clrback = SetBkColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_SELECTED?COLOR_HIGHLIGHT:COLOR_WINDOW));

			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));

			// Draw icon
			rc.left = lpdis->rcItem.left + 5;
			rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - icon_height) / 2;
			ImageList_Draw(hIml, CallService(MS_CLIST_GETCONTACTICON, (WPARAM)contacts[lpdis->itemData]->hcontact, 0), 
							lpdis->hDC, rc.left, rc.top, ILD_NORMAL);

			// Make rect for text
			rc.left += icon_width + 5;
			rc.right = lpdis->rcItem.right - 1;
			rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;
			rc.bottom = rc.top + tm.tmHeight;

			// Draw Protocol
			if (opts.num_protos > 1)
			{
				if (max_proto_width == 0)
				{
					// Has to be done, else the DC isnt the right one
					// Dont ask me why
					for(int loop = 0; loop < contacts.getCount(); loop++)
					{
						RECT rcc = { 0, 0, 0x7FFF, 0x7FFF };

						DrawTextA(lpdis->hDC, contacts[loop]->proto, strlen(contacts[loop]->proto), 
								 &rcc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_CALCRECT);
						max_proto_width = max(max_proto_width, rcc.right - rcc.left);
					}

					// Fix max_proto_width
					if (opts.group_append && opts.group_column)
						max_proto_width = min(max_proto_width, (rc.right - rc.left) / 5);
					else if (opts.group_append)
						max_proto_width = min(max_proto_width, (rc.right - rc.left) / 4);
					else
						max_proto_width = min(max_proto_width, (rc.right - rc.left) / 3);
				}

				RECT rc_tmp = rc;

				rc_tmp.left = rc_tmp.right - max_proto_width;

				DrawTextA(lpdis->hDC, contacts[lpdis->itemData]->proto, strlen(contacts[lpdis->itemData]->proto), 
						 &rc_tmp, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

				rc.right = rc_tmp.left - 5;
			}

			// Draw group
			if (opts.group_append && opts.group_column)
			{
				RECT rc_tmp = rc;

				if (opts.group_column_left)
				{
					rc_tmp.right = rc_tmp.left + (rc.right - rc.left) / 3;
					rc.left = rc_tmp.right + 5;
				}
				else
				{
					rc_tmp.left = rc_tmp.right - (rc.right - rc.left) / 3;
					rc.right = rc_tmp.left - 5;
				}

				DrawText(lpdis->hDC, contacts[lpdis->itemData]->szgroup, lstrlen(contacts[lpdis->itemData]->szgroup),
						 &rc_tmp, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
			}

			// Draw text
			TCHAR *name;
			if (opts.group_append && !opts.group_column)
				name = GetListName(contacts[lpdis->itemData]);
			else
				name = contacts[lpdis->itemData]->szname;

			DrawText(lpdis->hDC, name, lstrlen(name), &rc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

			// Restore old colors
			SetTextColor(lpdis->hDC, clrfore);
			SetBkColor(lpdis->hDC, clrback);

			return TRUE;
		}

		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;

			// Handle contact menu
			if(lpmis->CtlID != IDC_USERNAME) 
			{
				if (lpmis->CtlType == ODT_MENU)
					return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);
				else
					break;
			}

			// Handle combo

			TEXTMETRIC tm;
			int icon_width = 0, icon_height=0;

			GetTextMetrics(GetDC(hwndDlg), &tm);
			ImageList_GetIconSize(hIml, &icon_width, &icon_height);

			lpmis->itemHeight = max(icon_height, tm.tmHeight);
				
			return TRUE;
		}
	}
	
	return FALSE;
}


int HotkeyPressed(WPARAM wParam, LPARAM lParam) 
{
	THKSEvent *ev = (THKSEvent *) wParam;

	if (ev->moduleId == hksModule && ev->itemId == hksAction)
		ShowDialog(0, 0);

	return 0;
}


// Show the main dialog
int ShowDialog(WPARAM wParam,LPARAM lParam) 
{
	if (!main_dialog_open) 
	{
		InterlockedExchange(&main_dialog_open, 1);

		hwndMain = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
	}

/* This is inside miranda now

	// Make sure it is inside screen
	RECT rc;
	GetWindowRect(hwndMain, &rc);

	HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);

	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMonitor, &mi);

	RECT screen_rc = mi.rcWork;

	if (rc.bottom > screen_rc.bottom)
		OffsetRect(&rc, 0, screen_rc.bottom - rc.bottom);

	if (rc.bottom < screen_rc.top)
		OffsetRect(&rc, 0, screen_rc.top - rc.top);

	if (rc.top > screen_rc.bottom)
		OffsetRect(&rc, 0, screen_rc.bottom - rc.bottom);

	if (rc.top < screen_rc.top)
		OffsetRect(&rc, 0, screen_rc.top - rc.top);

	if (rc.right > screen_rc.right)
		OffsetRect(&rc, screen_rc.right - rc.right, 0);

	if (rc.right < screen_rc.left)
		OffsetRect(&rc, screen_rc.left - rc.left, 0);

	if (rc.left > screen_rc.right)
		OffsetRect(&rc, screen_rc.right - rc.right, 0);

	if (rc.left < screen_rc.left)
		OffsetRect(&rc, screen_rc.left - rc.left, 0);

	MoveWindow(hwndMain, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
*/

	// Show it
	SetForegroundWindow(hwndMain);
	SetFocus(hwndMain);
 	ShowWindow(hwndMain, SW_SHOW);

	return 0;
}
