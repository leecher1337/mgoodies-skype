/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


#include "mydetails.h"


// Prototypes /////////////////////////////////////////////////////////////////////////////////////


HINSTANCE hInst;
PLUGINLINK *pluginLink;

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"My Details",
	PLUGIN_MAKE_VERSION(0,0,0,40),
	"Show and allows you to edit your details for all protocols.",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2006 Ricardo Pescuma Domenecci",
	"http://www.miranda-im.org/",
	0,		//not transient
	0		//doesn't replace anything built-in
};


HANDLE hTTB = NULL;

// Hooks
HANDLE hModulesLoadedHook = NULL;
HANDLE hTopToolBarLoadedHook = NULL;


// Hook called after init
static int MainInit(WPARAM wparam,LPARAM lparam);
static int InitTopToolbarButton(WPARAM wParam, LPARAM lParam) ;


// Services
static int PluginCommand_SetMyNicknameUI(WPARAM wParam,LPARAM lParam);
static int PluginCommand_SetMyNickname(WPARAM wParam,LPARAM lParam);
static int PluginCommand_GetMyNickname(WPARAM wParam,LPARAM lParam);
static int PluginCommand_SetMyAvatarUI(WPARAM wParam,LPARAM lParam);
static int PluginCommand_SetMyAvatar(WPARAM wParam,LPARAM lParam);
static int PluginCommand_GetMyAvatar(WPARAM wParam,LPARAM lParam);
static int PluginCommand_SetMyStatusMessageUI(WPARAM wParam,LPARAM lParam);
static int PluginCommand_CicleThroughtProtocols(WPARAM wParam,LPARAM lParam);


// Functions //////////////////////////////////////////////////////////////////////////////////////


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}


__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}


int __declspec(dllexport) Load(PLUGINLINK *link)
{
	// Copy data
	pluginLink = link;

	init_mir_malloc();
	init_list_interface();

	// Hook event to load messages and show first one
	hModulesLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	hTopToolBarLoadedHook = NULL;

	// Options
	InitOptions();

	// Register services
	CreateServiceFunction(MS_MYDETAILS_SETMYNICKNAME, PluginCommand_SetMyNickname);
	CreateServiceFunction(MS_MYDETAILS_SETMYNICKNAMEUI, PluginCommand_SetMyNicknameUI);
	CreateServiceFunction(MS_MYDETAILS_SETMYAVATAR, PluginCommand_SetMyAvatar);
	CreateServiceFunction(MS_MYDETAILS_SETMYAVATARUI, PluginCommand_SetMyAvatarUI);
	CreateServiceFunction(MS_MYDETAILS_GETMYNICKNAME, PluginCommand_GetMyNickname);
	CreateServiceFunction(MS_MYDETAILS_GETMYAVATAR, PluginCommand_GetMyAvatar);
	CreateServiceFunction(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, PluginCommand_SetMyStatusMessageUI);
	CreateServiceFunction(MS_MYDETAILS_SHOWNEXTPROTOCOL, PluginCommand_ShowNextProtocol);
	CreateServiceFunction(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, PluginCommand_ShowPreviousProtocol);
	CreateServiceFunction(MS_MYDETAILS_SHOWPROTOCOL, PluginCommand_ShowProtocol);
	CreateServiceFunction(MS_MYDETAILS_CICLE_THROUGHT_PROTOCOLS, PluginCommand_CicleThroughtProtocols);

	return 0;
}


int __declspec(dllexport) Unload(void)
{
	DestroyServiceFunction(MS_MYDETAILS_SETMYNICKNAME);
	DestroyServiceFunction(MS_MYDETAILS_SETMYNICKNAMEUI);
	DestroyServiceFunction(MS_MYDETAILS_SETMYAVATAR);
	DestroyServiceFunction(MS_MYDETAILS_SETMYAVATARUI);
	DestroyServiceFunction(MS_MYDETAILS_GETMYNICKNAME);
	DestroyServiceFunction(MS_MYDETAILS_GETMYAVATAR);
	DestroyServiceFunction(MS_MYDETAILS_SETMYSTATUSMESSAGEUI);
	DestroyServiceFunction(MS_MYDETAILS_SHOWNEXTPROTOCOL);
	DestroyServiceFunction(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL);
	DestroyServiceFunction(MS_MYDETAILS_SHOWPROTOCOL);
	DestroyServiceFunction(MS_MYDETAILS_CICLE_THROUGHT_PROTOCOLS);

	if (hModulesLoadedHook) UnhookEvent(hModulesLoadedHook);
	if (hTopToolBarLoadedHook) UnhookEvent(hTopToolBarLoadedHook);

	DeInitFrames();
	DeInitProtocolData();
	DeInitOptions();

	return 0;
}


// Hook called after init
static int MainInit(WPARAM wparam,LPARAM lparam) 
{
	InitProtocolData();

	// Add options to menu
	CLISTMENUITEM mi;

	if (protocols->CanSetAvatars())
	{
		ZeroMemory(&mi,sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.flags = 0;
		mi.popupPosition = 500050000;
		mi.pszPopupName = Translate("My Details");
		mi.position = 100001;
		mi.pszName = Translate("Set My Avatar...");
		mi.pszService = MS_MYDETAILS_SETMYAVATARUI;

		CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
	}

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = 0;
	mi.popupPosition = 500050000;
	mi.pszPopupName = Translate("My Details");
	mi.position = 100002;
	mi.pszName = Translate("Set My Nickname...");
	mi.pszService = MS_MYDETAILS_SETMYNICKNAMEUI;

	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = 0;
	mi.popupPosition = 500050000;
	mi.pszPopupName = Translate("My Details");
	mi.position = 100003;
	mi.pszName = Translate("Set My Status Message...");
	mi.pszService = MS_MYDETAILS_SETMYSTATUSMESSAGEUI;

	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	// Set protocols to show frame
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = 0;
	mi.popupPosition = 500050000;
	mi.pszPopupName = Translate("My Details");
	mi.position = 200001;
	mi.pszName = Translate("Show next protocol");
	mi.pszService = MS_MYDETAILS_SHOWNEXTPROTOCOL;

	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	// TopToolbar support
	hTopToolBarLoadedHook = HookEvent(ME_TTB_MODULELOADED, InitTopToolbarButton);

	InitFrames();


    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://geocities.yahoo.com.br/ricardo_pescuma/mydetails_version.txt";
		upd.szBetaChangelogURL = "http://geocities.yahoo.com.br/ricardo_pescuma/mydetails_version.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"My Details ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
		upd.szBetaUpdateURL = "http://geocities.yahoo.com.br/ricardo_pescuma/mydetails.zip";

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}


    return 0;
}

// Toptoolbar hook to put an icon in the toolbar
static int InitTopToolbarButton(WPARAM wParam, LPARAM lParam) 
{
/*
	TTBButton ttb;

	ZeroMemory(&ttb,sizeof(ttb));
	ttb.cbSize = sizeof(ttb);
	ttb.pszServiceDown = "MyDetails/DoIt";
	ttb.dwFlags = TTBBF_VISIBLE;
	ttb.name = "DoIt";
	
	hTTB = (HANDLE)CallService(MS_TTB_ADDBUTTON, (WPARAM)&ttb, 0);
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTB, (LPARAM)TTBST_RELEASED);
*/

	return 0;
}

// Set nickname ///////////////////////////////////////////////////////////////////////////////////

#define WMU_SETDATA (WM_USER+1)

static BOOL CALLBACK DlgProcSetNickname(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			SendMessage(GetDlgItem(hwndDlg, IDC_NICKNAME), EM_LIMITTEXT, 
					MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE - 1, 0);

			return TRUE;
		}

		case WMU_SETDATA:
		{
			int proto_num = (int)wParam;

			SetWindowLong(hwndDlg, GWL_USERDATA, proto_num);

			if (proto_num == -1)
			{
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));

				// All protos have the same nick?
				if (protocols->GetSize() > 0)
				{
					char *nick = protocols->Get(0)->nickname;

					bool foundDefNick = true;
					for(int i = 1 ; foundDefNick && i < protocols->GetSize() ; i++)
					{
						if (stricmp(protocols->Get(i)->nickname, nick) != 0)
						{
							foundDefNick = false;
							break;
						}
					}

					if (foundDefNick)
					{
						if (stricmp(protocols->default_nick, nick) != 0)
							lstrcpy(protocols->default_nick, nick);
					}
				}

				SetDlgItemText(hwndDlg, IDC_NICKNAME, protocols->default_nick);
				SendDlgItemMessage(hwndDlg, IDC_NICKNAME, EM_LIMITTEXT, MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE, 0);
			}
			else
			{
				Protocol *proto = protocols->Get(proto_num);

				char tmp[128];
				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Nickname for %s"), proto->description);

				SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)tmp);

				HICON hIcon = (HICON)CallProtoService(proto->name, PS_LOADICON, PLI_PROTOCOL, 0);
				if (hIcon != NULL)
				{
					SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
					DestroyIcon(hIcon);
				}

				SetDlgItemText(hwndDlg, IDC_NICKNAME, proto->nickname);
				SendDlgItemMessage(hwndDlg, IDC_NICKNAME, EM_LIMITTEXT, 
						min(MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE, proto->GetNickMaxLength()), 0);
			}

			return TRUE;
		}
		case WM_COMMAND:
			switch(wParam)
			{
				case IDOK:
				{
					char tmp[MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE];
					GetDlgItemText(hwndDlg, IDC_NICKNAME, tmp, sizeof(tmp));

					int proto_num = (int)GetWindowLong(hwndDlg, GWL_USERDATA);
					if (proto_num == -1)
					{
						protocols->SetNicks(tmp);
					}
					else
					{
						protocols->Get(proto_num)->SetNick(tmp);
					}

					DestroyWindow(hwndDlg);
					break;
				}
				case IDCANCEL:
				{
 					DestroyWindow(hwndDlg);
					break;
				}
			}
			break;

		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;
	}
	
	return FALSE;
}

static int PluginCommand_SetMyNicknameUI(WPARAM wParam,LPARAM lParam)
{
	char * proto = (char *)lParam;
	int proto_num = -1;

	if (proto != NULL)
	{
		for(int i = 0 ; i < protocols->GetSize() ; i++)
		{
			if (stricmp(protocols->Get(i)->name, proto) == 0)
			{
				proto_num = i;
				break;
			}
		}

		if (proto_num == -1)
			return -1;

		if (!protocols->Get(i)->CanSetNick())
			return -2;

	}

	HWND hwndSetNickname = CreateDialog(hInst, MAKEINTRESOURCE( IDD_SETNICKNAME ), NULL, DlgProcSetNickname );
	
	SendMessage(hwndSetNickname, WMU_SETDATA, proto_num, 0);
	SetForegroundWindow( hwndSetNickname );
	SetFocus( hwndSetNickname );
 	ShowWindow( hwndSetNickname, SW_SHOW );

	return 0;
}


static int PluginCommand_SetMyNickname(WPARAM wParam,LPARAM lParam)
{
	char * proto = (char *)wParam;

	if (proto != NULL)
	{
		for(int i = 0 ; i < protocols->GetSize() ; i++)
		{
			if (stricmp(protocols->Get(i)->name, proto) == 0)
			{
				if (!protocols->Get(i)->CanSetNick())
				{
					return -2;
				}
				else
				{
					protocols->Get(i)->SetNick((char *)lParam);
					return 0;
				}
			}
		}

		return -1;
	}
	else
	{
		protocols->SetNicks((char *)lParam);

		return 0;
	}
}


static int PluginCommand_GetMyNickname(WPARAM wParam,LPARAM lParam)
{
	char * ret = (char *)lParam;
	char * proto = (char *)wParam;

	if (ret == NULL)
		return -1;

	if (proto == NULL)
	{
		if (protocols->default_nick != NULL)
			lstrcpyn(ret, protocols->default_nick, MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE);
		else
			ret[0] = '\0';

		return 0;
	}
	else
	{
		Protocol *protocol = protocols->Get(proto);

		if (protocol != NULL)
		{
			lstrcpyn(ret, protocol->nickname, MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE);
			return 0;
		}

		return -1;
	}
}


// Set avatar /////////////////////////////////////////////////////////////////////////////////////

static int PluginCommand_SetMyAvatarUI(WPARAM wParam,LPARAM lParam)
{
	char * proto = (char *)lParam;
	int proto_num = -1;

	if (proto != NULL)
	{
		for(int i = 0 ; i < protocols->GetSize() ; i++)
		{
			if (stricmp(protocols->Get(i)->name, proto) == 0)
			{
				proto_num = i;
				break;
			}
		}

		if (proto_num == -1)
			return -1;

		if (!protocols->Get(i)->CanSetAvatar())
		{
			return -2;
		}
	}

	if (proto_num == -1)
	{
		protocols->SetAvatars(NULL);
	}
	else
	{
		protocols->Get(proto_num)->SetAvatar(NULL);
	}

	return 0;
}


static int PluginCommand_SetMyAvatar(WPARAM wParam,LPARAM lParam)
{
	char * proto = (char *)wParam;

	if (proto != NULL)
	{
		for(int i = 0 ; i < protocols->GetSize() ; i++)
		{
			if (stricmp(protocols->Get(i)->name, proto) == 0)
			{
				if (!protocols->Get(i)->CanSetAvatar())
				{
					return -2;
				}
				else
				{
					protocols->Get(i)->SetAvatar((char *)lParam);
					return 0;
				}
			}
		}

		return -1;
	}
	else
	{
		protocols->SetAvatars((char *)lParam);

		return 0;
	}

	return 0;
}


int Status2SkinIcon(int status)
{
	switch(status) {
		case ID_STATUS_AWAY: return SKINICON_STATUS_AWAY;
		case ID_STATUS_NA: return SKINICON_STATUS_NA;
		case ID_STATUS_DND: return SKINICON_STATUS_DND;
		case ID_STATUS_OCCUPIED: return SKINICON_STATUS_OCCUPIED;
		case ID_STATUS_FREECHAT: return SKINICON_STATUS_FREE4CHAT;
		case ID_STATUS_ONLINE: return SKINICON_STATUS_ONLINE;
		case ID_STATUS_OFFLINE: return SKINICON_STATUS_OFFLINE;
		case ID_STATUS_INVISIBLE: return SKINICON_STATUS_INVISIBLE;
		case ID_STATUS_ONTHEPHONE: return SKINICON_STATUS_ONTHEPHONE;
		case ID_STATUS_OUTTOLUNCH: return SKINICON_STATUS_OUTTOLUNCH;
		case ID_STATUS_IDLE: return SKINICON_STATUS_AWAY;
	}
	return SKINICON_STATUS_OFFLINE;
}



static int PluginCommand_GetMyAvatar(WPARAM wParam,LPARAM lParam)
{
	char * ret = (char *)lParam;
	char * proto = (char *)wParam;

	if (ret == NULL)
		return -1;

	if (proto == NULL)
	{
		if (protocols->default_avatar_file != NULL)
			lstrcpyn(ret, protocols->default_avatar_file, MS_MYDETAILS_GETMYAVATAR_BUFFER_SIZE);
		else 
			ret[0] = '\0';

		return 0;
	}
	else
	{
		for(int i = 0 ; i < protocols->GetSize() ; i++)
		{
			if (stricmp(protocols->Get(i)->name, proto) == 0)
			{
				if (!protocols->Get(i)->CanGetAvatar())
				{
					return -2;
				}
				else
				{
					protocols->Get(i)->GetAvatar();

					if (protocols->Get(i)->avatar_file != NULL)
						lstrcpyn(ret, protocols->Get(i)->avatar_file, MS_MYDETAILS_GETMYAVATAR_BUFFER_SIZE);
					else 
						ret[0] = '\0';

					return 0;
				}
			}
		}
	}

	return -1;
}

static BOOL CALLBACK DlgProcSetStatusMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			SendMessage(GetDlgItem(hwndDlg, IDC_STATUSMESSAGE), EM_LIMITTEXT, 
					MS_MYDETAILS_GETMYSTATUSMESSAGE_BUFFER_SIZE - 1, 0);

			return TRUE;
		}

		case WMU_SETDATA:
		{
			int status = (int)wParam;

			SetWindowLong(hwndDlg, GWL_USERDATA, status);

			if (status != 0)
			{
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(Status2SkinIcon(status)));

				char title[256];
				mir_snprintf(title, sizeof(title), Translate("Set My Status Message for %s"), 
					CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, status, 0));
				SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)title);

				SetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, protocols->GetDefaultStatusMsg(status));
			}
			else
			{
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));

				SetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, protocols->GetDefaultStatusMsg());
			}

			return TRUE;
		}
		case WM_COMMAND:
			switch(wParam)
			{
				case IDOK:
				{
					char tmp[MS_MYDETAILS_GETMYSTATUSMESSAGE_BUFFER_SIZE];
					GetDlgItemText(hwndDlg, IDC_STATUSMESSAGE, tmp, sizeof(tmp));

					int status = GetWindowLong(hwndDlg, GWL_USERDATA);

					if (status == 0)
						protocols->SetStatusMsgs(tmp);
					else
						protocols->SetStatusMsgs(status, tmp);

					DestroyWindow(hwndDlg);
					break;
				}
				case IDCANCEL:
				{
 					DestroyWindow(hwndDlg);
					break;
				}
			}
			break;

		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;
	}
	
	return FALSE;
}

static int PluginCommand_SetMyStatusMessageUI(WPARAM wParam,LPARAM lParam)
{
	char * proto_name = (char *)lParam;
	int proto_num = -1;
	Protocol *proto = NULL;
	TCHAR status_message[256];

	if (proto_name != NULL)
	{
		for(int i = 0 ; i < protocols->GetSize() ; i++)
		{
			proto = protocols->Get(i);

			if (stricmp(proto->name, proto_name) == 0)
			{
				proto_num = i;
				break;
			}
		}

		if (proto_num == -1)
			return -1;

		if (protocols->CanSetStatusMsgPerProtocol() && !proto->CanSetStatusMsg())
		{
			return -2;
		}
	}

	if (ServiceExists(MS_NAS_INVOKESTATUSWINDOW))
	{
		NAS_ISWINFO iswi;

		ZeroMemory(&iswi, sizeof(iswi));

		iswi.cbSize = sizeof(NAS_ISWINFO);

		if (proto != NULL)
		{
			// Has to get the unparsed message
			NAS_PROTOINFO pi, *pii;

			ZeroMemory(&pi, sizeof(pi));
			pi.cbSize = sizeof(NAS_PROTOINFO);
			pi.szProto = proto->name;
			pi.status = proto->status;
			pi.szMsg = NULL;

			pii = &pi;

			if (CallService(MS_NAS_GETSTATE, (WPARAM) &pii, 1) == 0)
			{
				if (pi.szMsg == NULL)
				{
					pi.szProto = NULL;

					if (CallService(MS_NAS_GETSTATE, (WPARAM) &pii, 1) == 0)
					{
						if (pi.szMsg != NULL)
						{
							lstrcpyn(status_message, pi.szMsg, MAX_REGS(status_message));
							mir_free(pi.szMsg);
						}
					}
				}
				else // if (pi.szMsg != NULL)
				{
					lstrcpyn(status_message, pi.szMsg, MAX_REGS(status_message));
					mir_free(pi.szMsg);
				}
			}

			iswi.szProto = proto->name;
			iswi.szMsg = status_message;
		}
		else
		{
			iswi.szMsg = protocols->GetDefaultStatusMsg();
		}

		iswi.Flags = ISWF_NOCOUNTDOWN;

		CallService(MS_NAS_INVOKESTATUSWINDOW, (WPARAM) &iswi, 0);

		return 0;
	}
	else if (ServiceExists(MS_SA_CHANGESTATUSMSG))
	{
		if (proto == NULL)
		{
			CallService(MS_SA_CHANGESTATUSMSG, protocols->GetGlobalStatus(), NULL);
		}
		else
		{
			CallService(MS_SA_CHANGESTATUSMSG, proto->status, (LPARAM) proto_name);
		}

		return 0;
	}
	else
	{
		HWND hwndSet = CreateDialog(hInst, MAKEINTRESOURCE( IDD_SETSTATUSMESSAGE ), NULL, DlgProcSetStatusMessage );
		
		SendMessage(hwndSet, WMU_SETDATA, proto ? proto->status : 0, 0);
		SetForegroundWindow( hwndSet );
		SetFocus( hwndSet );
 		ShowWindow( hwndSet, SW_SHOW );

		return 0;
	}
}


static int PluginCommand_CicleThroughtProtocols(WPARAM wParam,LPARAM lParam)
{
	DBWriteContactSettingByte(NULL,"MyDetails","CicleThroughtProtocols", (BYTE) wParam);

	LoadOptions();

	return 0;
}