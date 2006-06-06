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
	"Quick Contacts",
	PLUGIN_MAKE_VERSION(0,0,1,1),
	"Open contact-specific windows by hotkey",
	"Heiko Schillinger, Ricardo Pescuma Domenecci",
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

long main_dialog_open = 0;
HWND hwndMain = NULL;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int EventAdded(WPARAM wparam, LPARAM lparam);
int ShowDialog(WPARAM wParam,LPARAM lParam);


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
		upd.szBetaUpdateURL = "http://br.geocities.com/ricardo_pescuma/quickcontacts.zip";

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

	// Get the icons for the listbox
	hIml = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST,0,0);

	// Add menu item
	CLISTMENUITEM mi;
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 500100001;
	mi.flags = 0;
	mi.ptszName = TranslateT("Quick Contacts...");
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


#define MAX_CONTACTS	512

// array where the contacts are put into
struct c_struct{
	TCHAR szname[120];
	HANDLE hcontact;
	char proto[10];
};

struct CONTACTSTRUCT{
	c_struct contact[MAX_CONTACTS];
	short int count;
};


CONTACTSTRUCT ns;



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
				if(lstrcmp(ns.contact[loop].proto,ns.contact[doop].proto)>0)
				{
					cs_temp=ns.contact[loop];
					ns.contact[loop]=ns.contact[doop];
					ns.contact[doop]=cs_temp;
				}
			}

		}
	}
}


void LoadContacts(BOOL show_all)
{
	// Read last-sent-to contact from db and set handle as window-userdata
	HANDLE hlastsent = (HANDLE)DBGetContactSettingDword(NULL, MODULE_NAME, "LastSentTo", -1);
	SetWindowLong(hwndMain, GWL_USERDATA, (LONG)hlastsent);

	// enumerate all contacts and write them to the array
	// item data of listbox-strings is the array position
	ns.count = 0;
	for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0); 
		hContact != NULL; 
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
	{
		char *pszProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if(pszProto != NULL)
		{
			if (!show_all)
			{
				// Check if proto offline
				if (opts.hide_from_offline_proto && CallProtoService(pszProto, PS_GETSTATUS, 0, 0) 
														<= ID_STATUS_OFFLINE)
					continue;

				// Check if is offline and have to show
				int status = DBGetContactSettingWord(hContact, pszProto, "Status", ID_STATUS_OFFLINE);
				if (status <= ID_STATUS_OFFLINE)
				{
					// See if has to show
					char setting[128];
					mir_snprintf(setting, sizeof(setting), "ShowOffline%s", pszProto);

					if (!DBGetContactSettingByte(NULL, MODULE_NAME, setting, FALSE))
						continue;
				}

				// Check if is subcontact
				if (opts.hide_subcontacts && ServiceExists(MS_MC_GETMETACONTACT))
				{
					HANDLE hMeta = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);
					if (hMeta != NULL) 
					{
						if (opts.keep_subcontacts_from_offline)
						{
							int meta_status = DBGetContactSettingWord(hMeta, "MetaContacts", "Status", ID_STATUS_OFFLINE);

							if (meta_status > ID_STATUS_OFFLINE)
								continue;
							else if (DBGetContactSettingByte(NULL, MODULE_NAME, "ShowOfflineMetaContacts", FALSE))
								continue;
						}
						else
						{
							continue;
						}
					}
				}
			}

			// Add to list
			lstrcpyn(ns.contact[ns.count].szname, 
					 (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR),
					 MAX_REGS(ns.contact[ns.count].szname));

			lstrcpyn(ns.contact[ns.count].proto, pszProto, MAX_REGS(ns.contact[ns.count].proto));

			ns.contact[ns.count++].hcontact = hContact;
		}
	}

	SortArray();
}

// check if the char(s) entered appears in a contacts name
int CheckText(HWND hdlg, TCHAR *sztext)
{
	int loop;

	if(!sztext[0])
		return 0;

	for(loop=0;loop<ns.count;loop++)
	{
		if(!strnicmp(sztext,ns.contact[loop].szname,lstrlen(sztext)))
		{
			int len = lstrlen(sztext);
			SendMessage(hdlg, WM_SETTEXT, 0, (LPARAM)ns.contact[loop].szname);
			SendMessage(hdlg, EM_SETSEL, (WPARAM)len, (LPARAM)-1);
			break;
		}
	}

	return 0;
}

HANDLE GetSelectedContact(HWND hwndDlg)
{
	TCHAR cname[120];

	GetDlgItemText(hwndDlg, IDC_USERNAME, cname, MAX_REGS(cname));
			
	for(int loop = 0; loop < ns.count; loop++)
	{
		if(!lstrcmpi(cname, ns.contact[loop].szname))
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
	return 0;
}


WNDPROC wpEditMainProc;

// callback function for edit-box of the listbox
// without this the autofill function isn't possible
// this was done like ie does it..as far as spy++ could tell ;)
LRESULT CALLBACK EditProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	TCHAR sztext[120]="";

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
			if(wparam==VK_RETURN)
			{
				switch(SendMessage(GetParent(hdlg),CB_GETDROPPEDSTATE,0,0))
				{
					case FALSE:
						SendMessage(GetParent(GetParent(hdlg)),WM_COMMAND,MAKEWPARAM(IDC_MESSAGE,STN_CLICKED),0);
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
			wpEditMainProc = (WNDPROC) SetWindowLong(GetWindow(GetWindow(hwndDlg,GW_CHILD),GW_CHILD), GWL_WNDPROC, (LONG)EditProc);

			// Buttons
			SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), BUTTONADDTOOLTIP, (LPARAM) TranslateT("Send message"), 0);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
			SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), BUTTONSETDEFAULT, 0, 0);

			SendMessage(GetDlgItem(hwndDlg, IDC_FILE), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_FILE), BUTTONADDTOOLTIP, (LPARAM) TranslateT("Send file (Ctrl+F)"), 0);
			SendDlgItemMessage(hwndDlg, IDC_FILE, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadSkinnedIcon(SKINICON_EVENT_FILE));

			SendMessage(GetDlgItem(hwndDlg, IDC_URL), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_URL), BUTTONADDTOOLTIP, (LPARAM) TranslateT("Send URL (Ctrl+U)"), 0);
			SendDlgItemMessage(hwndDlg, IDC_URL, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadSkinnedIcon(SKINICON_EVENT_URL));

			SendMessage(GetDlgItem(hwndDlg, IDC_USERINFO), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_USERINFO), BUTTONADDTOOLTIP, (LPARAM) TranslateT("Open userinfo (Ctrl+I)"), 0);
			SendDlgItemMessage(hwndDlg, IDC_USERINFO, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(160),IMAGE_ICON,16,16,LR_DEFAULTCOLOR));

			SendMessage(GetDlgItem(hwndDlg, IDC_HISTORY), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_HISTORY), BUTTONADDTOOLTIP, (LPARAM) Translate("Open history (Ctrl+H)"), 0);
			SendDlgItemMessage(hwndDlg, IDC_HISTORY, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(174),IMAGE_ICON,16,16,LR_DEFAULTCOLOR));

			SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETEXTENDEDUI, (WPARAM)TRUE, 0);

			Utils_RestoreWindowPosition(hwndDlg, NULL, MODULE_NAME, "window");

			LoadContacts(FALSE);
			
			SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_RESETCONTENT, 0, 0);
			for(int loop = 0; loop < ns.count; loop++)
			{
				SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETITEMDATA, 
									(WPARAM)SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_ADDSTRING, 0, (LPARAM)ns.contact[loop].szname), 
									(LPARAM)loop);
			}

			if(DBGetContactSettingByte(NULL, MODULE_NAME, "EnableLastSentTo", 0))
			{
				SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETCURSEL,
						(WPARAM)GetItemPos((HANDLE) DBGetContactSettingDword(NULL, MODULE_NAME, "LastSentTo", -1)), 
											0);
			}

			SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));

			return TRUE;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_MESSAGE:
				{
					HANDLE hContact = GetSelectedContact(hwndDlg);
					if (hContact == NULL)
					{
						SetDlgItemText(hwndDlg, IDC_USERNAME, "");
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

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
						SetDlgItemText(hwndDlg, IDC_USERNAME, "");
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

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
						SetDlgItemText(hwndDlg, IDC_USERNAME, "");
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

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
						SetDlgItemText(hwndDlg, IDC_USERNAME, "");
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

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
						SetDlgItemText(hwndDlg, IDC_USERNAME, "");
						SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
						break;
					}

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
					TCHAR sztext[120] = "";

					if (SendMessage(hEdit, EM_GETSEL, (WPARAM)NULL, (LPARAM)NULL) != -1)
					{
						SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)_T(""));
					}

					SendMessage(hEdit, WM_GETTEXT, (WPARAM)MAX_REGS(sztext), (LPARAM)sztext);

					// Fill combo			
					BOOL all = IsDlgButtonChecked(hwndDlg, IDC_SHOW_ALL_CONTACTS);

					if (LOWORD(wParam) == HOTKEY_ALL_CONTACTS)
					{
						// Toggle checkbox
						all = !all;
						CheckDlgButton(hwndDlg, IDC_SHOW_ALL_CONTACTS, all ? BST_CHECKED : BST_UNCHECKED);
					}

					LoadContacts(all);
					
					SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_RESETCONTENT, 0, 0);
					for(int loop = 0; loop < ns.count; loop++)
					{
						SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETITEMDATA, 
											(WPARAM)SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_ADDSTRING, 0, (LPARAM)ns.contact[loop].szname), 
											(LPARAM)loop);
					}

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
				RECT rc_tmp = rc;

				rc_tmp.left = rc.right - min(tm.tmAveCharWidth * 10, (rc.right - rc_tmp.left) / 3);

				DrawText(lpdis->hDC, ns.contact[lpdis->itemData].proto, lstrlen(ns.contact[lpdis->itemData].proto), 
						 &rc_tmp, DT_END_ELLIPSIS | DT_NOPREFIX);

				rc.right = rc_tmp.left - 5;
			}

			// Draw text
			DrawText(lpdis->hDC, ns.contact[lpdis->itemData].szname, strlen(ns.contact[lpdis->itemData].szname),
					 &rc, DT_END_ELLIPSIS | DT_NOPREFIX);

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


// Show the main dialog
int ShowDialog(WPARAM wParam,LPARAM lParam) 
{
	if (!main_dialog_open) 
	{
		InterlockedExchange(&main_dialog_open, 1);

		hwndMain = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
	}

	SetForegroundWindow(hwndMain);
	SetFocus(hwndMain);
 	ShowWindow(hwndMain, SW_SHOW);

	return 0;
}