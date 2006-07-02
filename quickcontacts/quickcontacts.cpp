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


PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
#ifdef UNICODE
	"Quick Contacts (Unicode)",
#else
	"Quick Contacts",
#endif
	PLUGIN_MAKE_VERSION(0,0,2,1),
	"Open contact-specific windows by hotkey",
	"Ricardo Pescuma Domenecci, Heiko Schillinger",
	"",
	"",
	"http://miranda-im.org/",
	0,	//not transient
	0	//doesn't replace anything built-in
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;
HIMAGELIST hIml;

HANDLE hModulesLoaded = NULL;
HANDLE hEventAdded = NULL;
HANDLE hHotkeyPressed = NULL;

long main_dialog_open = 0;
HWND hwndMain = NULL;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int EventAdded(WPARAM wparam, LPARAM lparam);
int HotkeyPressed(WPARAM wParam, LPARAM lParam);
int ShowDialog(WPARAM wParam,LPARAM lParam);

int hksModule = 0;
int hksAction = 0;


// Functions ////////////////////////////////////////////////////////////////////////////


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	hInst = hinstDLL;
	return TRUE;
}


__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) 
{
	return &pluginInfo;
}


int __declspec(dllexport) Load(PLUGINLINK *link) {
	CLISTMENUITEM mi = {0};
	
	pluginLink = link;
	
	CreateServiceFunction(MS_QC_SHOW_DIALOG, ShowDialog);

	// hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hEventAdded = HookEvent(ME_DB_EVENT_ADDED, EventAdded);

	return 0;
}

int __declspec(dllexport) Unload(void) 
{
	DeInitOptions();

	UnhookEvent(hModulesLoaded);
	UnhookEvent(hEventAdded);

	HKS_Unregister(hksAction);
	HKS_Unregister(hksModule);

	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	init_mir_malloc();

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

		upd.szBetaVersionURL = "http://br.geocities.com/ricardo_pescuma/quickcontacts_version.txt";
		upd.szBetaChangelogURL = "http://br.geocities.com/ricardo_pescuma/quickcontacts_changelog.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"Quick Contacts ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://br.geocities.com/ricardo_pescuma/quickcontactsW.zip";
#else
		upd.szBetaUpdateURL = "http://br.geocities.com/ricardo_pescuma/quickcontacts.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
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

	hksModule = HKS_RegisterModule("Quick Contacts");
	if (hksModule >= 0)
	{
		hksAction = HKS_RegisterAction(hksModule, "Open dialog", "Ctrl+Alt+Q", 0);

		hHotkeyPressed = HookEvent(ME_HKS_KEY_PRESSED, HotkeyPressed);
	}
	else
	{
		// Add hotkey
		SKINHOTKEYDESCEX hk;
		ZeroMemory(&hk,sizeof(hk));
		hk.cbSize = sizeof(hk);
		hk.pszSection = Translate("Quick Contacts");
		hk.pszName = Translate("Show dialog");
		hk.pszDescription = Translate("Show dialog to select contact");
		hk.pszService = MS_QC_SHOW_DIALOG;
		hk.DefHotKey = 0;
		CallService(MS_SKIN_ADDHOTKEY, 0, (LPARAM)&hk);
	}

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


#define MAX_CONTACTS	512

// array where the contacts are put into
struct c_struct {
	TCHAR szname[120];
	TCHAR szgroup[50];
	HANDLE hcontact;
	char proto[20];
};

struct CONTACTSTRUCT {
	c_struct contact[MAX_CONTACTS];
	short int count;
	long max_proto_width;
};


CONTACTSTRUCT ns;


// Get the name the contact has in list
// This was not made to be called by more than one thread!
TCHAR tmp_list_name[120];

TCHAR *GetListName(c_struct &cs)
{
	if (opts.group_append && cs.szgroup[0] != _T('\0'))
	{
		mir_sntprintf(tmp_list_name, MAX_REGS(tmp_list_name), _T("%s (%s)"), cs.szname, cs.szgroup);
		return tmp_list_name;
	}
	else
	{
		return cs.szname;
	}
}



// simple sorting function to have
// the contact array in alphabetical order
void SortArray(void)
{
	int loop,doop;
	c_struct cs_temp;

	for(loop=0;loop<ns.count;loop++)
	{
		for(doop=loop+1;doop<ns.count;doop++)
		{
			if(lstrcmp(ns.contact[loop].szname,ns.contact[doop].szname)>0)
			{
				cs_temp=ns.contact[loop];
				ns.contact[loop]=ns.contact[doop];
				ns.contact[doop]=cs_temp;
			}
			else if(!lstrcmp(ns.contact[loop].szname,ns.contact[doop].szname))
			{
				if(strcmp(ns.contact[loop].proto,ns.contact[doop].proto)>0)
				{
					cs_temp=ns.contact[loop];
					ns.contact[loop]=ns.contact[doop];
					ns.contact[doop]=cs_temp;
				}
			}

		}
	}
}


void LoadContacts(HWND hwndDlg, BOOL show_all)
{
	// Read last-sent-to contact from db and set handle as window-userdata
	HANDLE hlastsent = (HANDLE)DBGetContactSettingDword(NULL, MODULE_NAME, "LastSentTo", -1);
	SetWindowLong(hwndMain, GWL_USERDATA, (LONG)hlastsent);

	// enumerate all contacts and write them to the array
	// item data of listbox-strings is the array position
	ns.count = 0;
	ns.max_proto_width = 0;
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
			memset(&ns.contact[ns.count], 0, sizeof(ns.contact[ns.count]));
			
			if (opts.group_append)
			{
				DBVARIANT dbv;
				if (DBGetContactSettingTString(hMeta == NULL ? hContact : hMeta, "CList", "Group", &dbv) == 0)
				{
					if (dbv.ptszVal != NULL)
						lstrcpyn(ns.contact[ns.count].szgroup, dbv.ptszVal, MAX_REGS(ns.contact[ns.count].szgroup));

					DBFreeVariant(&dbv);
				}
			}

			// Make contact name
			TCHAR *tmp = (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
			lstrcpyn(ns.contact[ns.count].szname, tmp, MAX_REGS(ns.contact[ns.count].szname));

			strncpy(ns.contact[ns.count].proto, pszProto, sizeof(ns.contact[ns.count].proto)-1);
			ns.contact[ns.count].proto[sizeof(ns.contact[ns.count].proto)-1] = '\0';

			ns.contact[ns.count].hcontact = hContact;

			ns.count++;
		}
	}

	SortArray();
			
	SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_RESETCONTENT, 0, 0);
	for(int loop = 0; loop < ns.count; loop++)
	{
		SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETITEMDATA, 
							(WPARAM)SendDlgItemMessage(hwndDlg, IDC_USERNAME, 
											CB_ADDSTRING, 0, (LPARAM) GetListName(ns.contact[loop])), 
							(LPARAM)loop);
	}
}


// Enable buttons for the selected contact
void EnableButtons(HWND hwndDlg, HANDLE hContact)
{
	if (hContact == NULL)
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_URL), FALSE);
	}
	else
	{
		// Is a meta?
		HANDLE hSub = (HANDLE) CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM) hContact, 0);
		if (hSub != NULL)
			hContact = hSub;

		// Get caps
		int caps = 0;

		char *pszProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (pszProto != NULL)
			caps = CallProtoService(pszProto, PS_GETCAPS, PFLAGNUM_1, 0);

		EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), caps & PF1_IMSEND ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), caps & PF1_FILESEND ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_URL), caps & PF1_URLSEND ? TRUE : FALSE);
	}
}


// check if the char(s) entered appears in a contacts name
int CheckText(HWND hdlg, TCHAR *sztext)
{
	if(sztext == NULL || sztext[0] == _T('\0'))
		return 0;

	int len = lstrlen(sztext);

	int loop;
	for(loop=0;loop<ns.count;loop++)
	{
		if(!_tcsnicmp(sztext, ns.contact[loop].szname, len))
		{
			SendMessage(hdlg, WM_SETTEXT, 0, (LPARAM) GetListName(ns.contact[loop]));
			SendMessage(hdlg, EM_SETSEL, (WPARAM) len, (LPARAM) -1);
			EnableButtons(hwndMain, ns.contact[loop].hcontact);
			return 0;
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
			return ns.contact[pos].hcontact;
	}

	// Now try the name
	TCHAR cname[120] = _T("");

	GetDlgItemText(hwndDlg, IDC_USERNAME, cname, MAX_REGS(cname));
			
	for(int loop = 0; loop < ns.count; loop++)
	{
		if(!lstrcmpi(cname, GetListName(ns.contact[loop])))
			return ns.contact[loop].hcontact;
	}

	return NULL;
}

// get array position from handle
int GetItemPos(HANDLE hcontact)
{
	int loop;

	for(loop=0;loop<ns.count;loop++)
	{
		if(hcontact==ns.contact[loop].hcontact)
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
	TCHAR sztext[120] = _T("");

	switch(msg)
	{
		case WM_CHAR:
			if(wparam<32) break;

			if(SendMessage(hdlg,EM_GETSEL,(WPARAM)NULL,(LPARAM)NULL)!=-1)
			{
				sztext[0]=wparam;
				sztext[1]=0;
				SendMessage(hdlg,EM_REPLACESEL,(WPARAM)0,(LPARAM)sztext);
			}

			SendMessage(hdlg,WM_GETTEXT,(WPARAM)MAX_REGS(sztext),(LPARAM)sztext);
			CheckText(hdlg,sztext);
			return 1;

		case WM_KEYUP:
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

			return 0;

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
	htemp = msg->hwnd;
	msg->hwnd = hwndMain;

	if(!TranslateAccelerator(msg->hwnd, hAcct, msg))
		msg->hwnd=htemp;

	if (msg->message == WM_KEYUP && msg->wParam == VK_ESCAPE)
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

static BOOL CALLBACK MainDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			hAcct = LoadAccelerators(hInst, MAKEINTRESOURCE(ACCEL_TABLE));

			hHook = SetWindowsHookEx(WH_MSGFILTER, HookProc, hInst, GetCurrentThreadId());

			// Combo
			SendMessage(GetDlgItem(hwndDlg, IDC_USERNAME), EM_LIMITTEXT, (WPARAM)119,0);
			wpEditMainProc = (WNDPROC) SetWindowLong(GetWindow(GetDlgItem(hwndDlg, IDC_USERNAME),GW_CHILD), GWL_WNDPROC, (LONG)EditProc);

			// Buttons
			SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), BUTTONADDTOOLTIP, (LPARAM) Translate("Send message"), 0);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));

			SendMessage(GetDlgItem(hwndDlg, IDC_FILE), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_FILE), BUTTONADDTOOLTIP, (LPARAM) Translate("Send file (Ctrl+F)"), 0);
			SendDlgItemMessage(hwndDlg, IDC_FILE, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadSkinnedIcon(SKINICON_EVENT_FILE));

			SendMessage(GetDlgItem(hwndDlg, IDC_URL), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_URL), BUTTONADDTOOLTIP, (LPARAM) Translate("Send URL (Ctrl+U)"), 0);
			SendDlgItemMessage(hwndDlg, IDC_URL, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadSkinnedIcon(SKINICON_EVENT_URL));

			SendMessage(GetDlgItem(hwndDlg, IDC_USERINFO), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_USERINFO), BUTTONADDTOOLTIP, (LPARAM) Translate("Open userinfo (Ctrl+I)"), 0);
			SendDlgItemMessage(hwndDlg, IDC_USERINFO, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(160),IMAGE_ICON,16,16,LR_DEFAULTCOLOR));

			SendMessage(GetDlgItem(hwndDlg, IDC_HISTORY), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_HISTORY), BUTTONADDTOOLTIP, (LPARAM) Translate("Open history (Ctrl+H)"), 0);
			SendDlgItemMessage(hwndDlg, IDC_HISTORY, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(174),IMAGE_ICON,16,16,LR_DEFAULTCOLOR));

			SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETEXTENDEDUI, (WPARAM)TRUE, 0);

			MagneticWindows_AddWindow(hwndDlg);

			Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE_NAME, "window");

			LoadContacts(hwndDlg, FALSE);

			if (DBGetContactSettingByte(NULL, MODULE_NAME, "EnableLastSentTo", 0))
			{
				int pos = GetItemPos((HANDLE) DBGetContactSettingDword(NULL, MODULE_NAME, "LastSentTo", -1));

				if (pos != -1)
				{
					SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETCURSEL, (WPARAM) pos, 0);
					EnableButtons(hwndDlg, ns.contact[pos].hcontact);
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
						EnableButtons(hwndDlg, pos < ns.count ? ns.contact[pos].hcontact : NULL);
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
			TEXTMETRIC tm;
			int icon_width=0, icon_height=0;
			RECT rc;

			// add icons and protocol to listbox
			LPDRAWITEMSTRUCT lpdis=(LPDRAWITEMSTRUCT)lParam;

			if(lpdis->itemID == -1) 
				return 0;

			GetTextMetrics(lpdis->hDC, &tm);
			ImageList_GetIconSize(hIml, &icon_width, &icon_height);

			COLORREF clrfore = SetTextColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_SELECTED?COLOR_HIGHLIGHTTEXT:COLOR_WINDOWTEXT));
			COLORREF clrback = SetBkColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_SELECTED?COLOR_HIGHLIGHT:COLOR_WINDOW));

			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));

			// Draw icon
			rc.left = lpdis->rcItem.left + 5;
			rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - icon_height) / 2;
			ImageList_Draw(hIml, CallService(MS_CLIST_GETCONTACTICON, (WPARAM)ns.contact[lpdis->itemData].hcontact,0), 
							lpdis->hDC, rc.left, rc.top, ILD_NORMAL);

			// Make rect for text
			rc.left += icon_width + 5;
			rc.right = lpdis->rcItem.right - 1;
			rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;
			rc.bottom = rc.top + tm.tmHeight;

			// Draw Protocol
			if (opts.num_protos > 1)
			{
				if (ns.max_proto_width == 0)
				{
					// Has to be done, else the DC isnt the right one
					// Dont ask me why
					for(int loop = 0; loop < ns.count; loop++)
					{
						RECT rcc = { 0, 0, 0x7FFF, 0x7FFF };

						DrawTextA(lpdis->hDC, ns.contact[loop].proto, strlen(ns.contact[loop].proto), 
								 &rcc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_CALCRECT);
						ns.max_proto_width = max(ns.max_proto_width, rcc.right - rcc.left);
					}

					// Fix max_proto_width
					if (opts.group_append && opts.group_column)
						ns.max_proto_width = min(ns.max_proto_width, (rc.right - rc.left) / 5);
					else if (opts.group_append)
						ns.max_proto_width = min(ns.max_proto_width, (rc.right - rc.left) / 4);
					else
						ns.max_proto_width = min(ns.max_proto_width, (rc.right - rc.left) / 3);
				}

				RECT rc_tmp = rc;

				rc_tmp.left = rc_tmp.right - ns.max_proto_width;

				DrawTextA(lpdis->hDC, ns.contact[lpdis->itemData].proto, strlen(ns.contact[lpdis->itemData].proto), 
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

				DrawText(lpdis->hDC, ns.contact[lpdis->itemData].szgroup, lstrlen(ns.contact[lpdis->itemData].szgroup),
						 &rc_tmp, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
			}

			// Draw text
			TCHAR *name;
			if (opts.group_append && !opts.group_column)
				name = GetListName(ns.contact[lpdis->itemData]);
			else
				name = ns.contact[lpdis->itemData].szname;

			DrawText(lpdis->hDC, name, lstrlen(name), &rc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

			// Restore old colors
			SetTextColor(lpdis->hDC, clrfore);
			SetBkColor(lpdis->hDC, clrback);

			return TRUE;
		}

		case WM_MEASUREITEM:
		{
			TEXTMETRIC tm;
			int icon_width = 0, icon_height=0;

			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;

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

	// Show it
	SetForegroundWindow(hwndMain);
	SetFocus(hwndMain);
 	ShowWindow(hwndMain, SW_SHOW);

	return 0;
}
