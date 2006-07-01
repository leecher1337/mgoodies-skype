/*
Avatar History Plugin
 Copyright (C) 2006  Matthew Wild - Email: mwild1@gmail.com

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <prsht.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_options.h> // Miranda header
#include <m_utils.h>
#include <m_langpack.h>
#include "resource.h"
#include "AvatarHistory.h"

extern HINSTANCE hInst;

static BOOL CALLBACK OptionsDialogProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

int OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=0;
	odp.hInstance=hInst;
	odp.pszTemplate=MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszGroup= Translate("Events"); // group to put your item under
	odp.pszTitle=Translate("Avatar History"); // name of the item
	odp.pfnDlgProc=OptionsDialogProc; // dlg proc of the dialog
	odp.expertOnlyControls=NULL;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	
	return 0;
}

static BOOL CALLBACK OptionsDialogProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	 switch(uMsg)
	 {
	 case WM_INITDIALOG:
		 {
			 SendMessage(GetDlgItem(hwnd, IDC_POPUPFG), CPM_SETCOLOUR, 0, db_dword_get(NULL, "AvatarHistory", "PopupFG", AVH_DEF_POPUPFG ));
			 SendMessage(GetDlgItem(hwnd, IDC_POPUPBG), CPM_SETCOLOUR, 0, db_dword_get(NULL, "AvatarHistory", "PopupBG", AVH_DEF_POPUPBG ));
			 CheckDlgButton(hwnd, IDC_AVATARPOPUPS, (UINT)db_byte_get(NULL, "AvatarHistory", "AvatarPopups", AVH_DEF_AVPOPUPS));
			 CheckDlgButton(hwnd, IDC_LOGTODISK, (UINT)db_byte_get(NULL, "AvatarHistory", "LogToDisk", AVH_DEF_LOGTODISK));
			 CheckDlgButton(hwnd, IDC_DEFPOPUPS, (UINT)db_byte_get(NULL, "AvatarHistory", "UsePopupDefault", AVH_DEF_DEFPOPUPS));
			 CheckDlgButton(hwnd, IDC_SHOWMENU, (UINT)db_byte_get(NULL, "AvatarHistory", "ShowContactMenu", AVH_DEF_SHOWMENU));
			 EnableWindow(GetDlgItem(hwnd, IDC_POPUPFG), !IsDlgButtonChecked(hwnd, IDC_DEFPOPUPS));
			 EnableWindow(GetDlgItem(hwnd, IDC_POPUPBG), !IsDlgButtonChecked(hwnd, IDC_DEFPOPUPS));
			 EnableWindow(GetDlgItem(hwnd, IDC_PUFGTEXT), !IsDlgButtonChecked(hwnd, IDC_DEFPOPUPS));
			 EnableWindow(GetDlgItem(hwnd, IDC_PUBGTEXT), !IsDlgButtonChecked(hwnd, IDC_DEFPOPUPS));
			 TranslateDialogDefault(hwnd);
		 }
		 break;
	 case WM_COMMAND:
		 SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		 if(LOWORD(wParam) == IDC_DEFPOPUPS && HIWORD(wParam) == BN_CLICKED)
		 {
			 int chkstate = IsDlgButtonChecked(hwnd, IDC_DEFPOPUPS);
			 EnableWindow(GetDlgItem(hwnd, IDC_POPUPFG),  !chkstate);
			 EnableWindow(GetDlgItem(hwnd, IDC_POPUPBG),  !chkstate);
			 EnableWindow(GetDlgItem(hwnd, IDC_PUFGTEXT), !chkstate);
			 EnableWindow(GetDlgItem(hwnd, IDC_PUBGTEXT), !chkstate);
			 return TRUE;
		 }
		 break;
	 case WM_NOTIFY:
		 {
				//Here we have pressed either the OK or the APPLY button.
				switch(((LPNMHDR)lParam)->idFrom)
				{
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
					case PSN_APPLY:
						{
							// Here be dragons (no, just kididng: here you put your ok/apply code)
							DWORD colour = SendMessage(GetDlgItem(hwnd, IDC_POPUPFG), CPM_GETCOLOUR, 0, 0);
							if(colour != AVH_DEF_POPUPFG)
								db_dword_set(NULL, "AvatarHistory", "PopupFG", colour);
							
							colour = SendMessage(GetDlgItem(hwnd, IDC_POPUPBG), CPM_GETCOLOUR, 0, 0);
							if(colour != AVH_DEF_POPUPBG)
								db_dword_set(NULL, "AvatarHistory", "PopupBG", colour);
							db_byte_set(NULL, "AvatarHistory", "AvatarPopups", (char)IsDlgButtonChecked(hwnd, IDC_AVATARPOPUPS));
							db_byte_set(NULL, "AvatarHistory", "LogToDisk", (char)IsDlgButtonChecked(hwnd, IDC_LOGTODISK));
							db_byte_set(NULL, "AvatarHistory", "UsePopupDefault", (char)IsDlgButtonChecked(hwnd, IDC_DEFPOPUPS));
							db_byte_set(NULL, "AvatarHistory", "ShowContactMenu", (char)IsDlgButtonChecked(hwnd, IDC_SHOWMENU));
							return TRUE;
						}
					} // switch code
					break;
				} //switch idFrom
		 }
		 break;
	 default:
		 return FALSE;
	 }
	 return FALSE;
}