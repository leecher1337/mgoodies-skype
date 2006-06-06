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


#include "options.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;


Options opts;

static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);



// Functions //////////////////////////////////////////////////////////////////////////////////////


// Initializations needed by options
void LoadOptions()
{
	opts.last_sent_enable = DBGetContactSettingByte(NULL, MODULE_NAME, "EnableLastSentTo", TRUE);
	opts.last_sent_msg_type = DBGetContactSettingWord(NULL, MODULE_NAME, "MsgTypeRec", TYPE_GLOBAL);
	opts.hide_from_offline_proto = DBGetContactSettingByte(NULL, MODULE_NAME, "HideFromOfflineProto", TRUE);
	opts.hide_subcontacts = DBGetContactSettingByte(NULL, MODULE_NAME, "HideSubcontacts", TRUE);
	opts.keep_subcontacts_from_offline = DBGetContactSettingByte(NULL, MODULE_NAME, "KeepSubcontactsFromOffline", TRUE);
}

int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=0;
	odp.hInstance=hInst;
	odp.ptszGroup = TranslateT("Plugins");
	odp.ptszTitle = TranslateT("Quick Contacts");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT);
    odp.flags = ODPF_BOLDGROUPS;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


void InitOptions()
{
	LoadOptions();

	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}

// Deinitializations needed by options
void DeInitOptions()
{
	UnhookEvent(hOptHook);
}

// Options page

static OptPageControl controls[] = { 
	{ CONTROL_CHECKBOX,			IDC_LASTSENTTO,		"EnableLastSentTo",				(BYTE) TRUE },
	{ CONTROL_RADIO,			IDC_GLOBAL,			"MsgTypeRec",					(WORD) TYPE_GLOBAL, TYPE_GLOBAL },
	{ CONTROL_RADIO,			IDC_LOCAL,			"MsgTypeRec",					(WORD) TYPE_GLOBAL, TYPE_LOCAL },
	{ CONTROL_PROTOCOL_LIST_ALL,IDC_PROTOCOLS,		"ShowOffline%s",				(BYTE) FALSE },
	{ CONTROL_CHECKBOX,			IDC_HIDE_OFFLINE,	"HideFromOfflineProto",			(BYTE) TRUE },
	{ CONTROL_CHECKBOX,			IDC_SUBCONTACTS,	"HideSubcontacts",				(BYTE) TRUE },
	{ CONTROL_CHECKBOX,			IDC_KEEP_OFFLINE,	"KeepSubcontactsFromOffline",	(BYTE) TRUE }
};

static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	int ret = SaveOptsDlgProc(controls, MAX_REGS(controls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_LASTSENTTO);
			EnableWindow(GetDlgItem(hwndDlg, IDC_GLOBAL), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOCAL), enabled);
			
			enabled = IsDlgButtonChecked(hwndDlg, IDC_SUBCONTACTS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_KEEP_OFFLINE), enabled);

			if (!ServiceExists(MS_MC_GETMETACONTACT))
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_SUBCONTACTS), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_KEEP_OFFLINE), SW_HIDE);
			}

			return TRUE;
		}
		case WM_COMMAND:
		{
			if(LOWORD(wParam) == IDC_LASTSENTTO)
			{
				BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_LASTSENTTO);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GLOBAL), enabled);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOCAL), enabled);
			}

			if(LOWORD(wParam) == IDC_SUBCONTACTS)
			{
				BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_SUBCONTACTS);
				EnableWindow(GetDlgItem(hwndDlg, IDC_KEEP_OFFLINE), enabled);
			}

			break;
		}
		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom) 
			{
				case 0:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							LoadOptions();

							return TRUE;
						}
					}
					break;
				}
			}
			break;
		}
	}

	return ret;
}

