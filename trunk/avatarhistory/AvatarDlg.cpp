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
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include "resource.h"
#include "AvatarHistory.h"

extern HINSTANCE hInst;

int OpenAvatarDialog(HANDLE hContact, char* fn);
DWORD WINAPI AvatarDialogThread(LPVOID param);
static BOOL CALLBACK AvatarDlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int ShowSaveDialog(HWND hwnd, char* fn);

int FillAvatarList(HWND list, HANDLE hContact, char* fn);
int UpdateAvatarPic(HWND hwnd, char* fn);
char* GetCurrentSelFile(HWND list, char* out);

static int ShowDialogSvc(WPARAM wParam, LPARAM lParam);

struct AvatarDialogData
{
	HANDLE hContact;
	char fn[MAX_PATH+1];
	HWND parent;
};

int OpenAvatarDialog(HANDLE hContact, char* fn)
{
	DWORD dwId;
	struct AvatarDialogData* avdlg;
	avdlg = (struct AvatarDialogData*)malloc(sizeof(struct AvatarDialogData));
	ZeroMemory(avdlg, sizeof(struct AvatarDialogData));
	avdlg->hContact = hContact;
	strncpy(avdlg->fn, fn?fn:"", MAX_PATH);
	CloseHandle(CreateThread(NULL, 0, AvatarDialogThread, (LPVOID)avdlg, 0, &dwId));
	return 0;
}

DWORD WINAPI AvatarDialogThread(LPVOID param)
{
	struct AvatarDialogData* data = (struct AvatarDialogData*)param;
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_AVATARDLG), data->parent, AvatarDlgProc, (LPARAM)param);
	return 0;
}

static BOOL CALLBACK AvatarDlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static struct AvatarDialogData* data;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			data = (struct AvatarDialogData*) lParam;
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon(hInst, MAKEINTRESOURCE(IDI_AVATARHIST)));
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon(hInst, MAKEINTRESOURCE(IDI_AVATARHIST)));
			FillAvatarList(GetDlgItem(hwnd, IDC_AVATARLIST), data->hContact, data->fn);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (ULONG_PTR)data->hContact);
			UpdateAvatarPic(hwnd, NULL);
			CheckDlgButton(hwnd, IDC_LOGUSER, (UINT)db_byte_get(data->hContact, "AvatarHistory", "LogUser", BST_INDETERMINATE));
			CheckDlgButton(hwnd, IDC_POPUPUSER, (UINT)db_byte_get(data->hContact, "AvatarHistory", "PopupUser", BST_INDETERMINATE));
			TranslateDialogDefault(hwnd);
			free(data);
			data = NULL;	
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				if(IsDlgButtonChecked(hwnd, IDC_POPUPUSER) != BST_INDETERMINATE)
				{
					db_byte_set((HANDLE)GetWindowLongPtr(hwnd, GWLP_USERDATA), "AvatarHistory", "PopupUser", (char)IsDlgButtonChecked(hwnd, IDC_POPUPUSER));
				}
				else
				{
					DBCONTACTGETSETTING dbdcs;
					dbdcs.szModule = "AvatarHistory";
					dbdcs.szSetting = "PopupUser";
					CallService(MS_DB_CONTACT_DELETESETTING, (WPARAM)GetWindowLongPtr(hwnd, GWLP_USERDATA), (LPARAM)&dbdcs);
				}
		
				if(IsDlgButtonChecked(hwnd, IDC_LOGUSER) != BST_INDETERMINATE)
				{
					db_byte_set((HANDLE)GetWindowLongPtr(hwnd, GWLP_USERDATA), "AvatarHistory", "LogUser", (char)IsDlgButtonChecked(hwnd, IDC_LOGUSER));
				}
				else
				{
					DBCONTACTGETSETTING dbdcs;
					dbdcs.szModule = "AvatarHistory";
					dbdcs.szSetting = "LogUser";
					CallService(MS_DB_CONTACT_DELETESETTING, (WPARAM)GetWindowLongPtr(hwnd, GWLP_USERDATA), (LPARAM)&dbdcs);
				}
				EndDialog(hwnd, 0);
				return TRUE;
			}
			break;
		case IDC_AVATARLIST:
			if(HIWORD(wParam) == LBN_SELCHANGE)
			{
				UpdateAvatarPic(hwnd, NULL);
				return TRUE;
			}
			break;
		case IDC_NEXT:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
				SendMessage(list, LB_SETCURSEL, SendMessage(list, LB_GETCURSEL, 0, 0) +1, 0);
				UpdateAvatarPic(hwnd, NULL);
				return TRUE;
			}
			break;
		case IDC_BACK:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
				SendMessage(list, LB_SETCURSEL, SendMessage(list, LB_GETCURSEL, 0, 0) -1, 0);
				UpdateAvatarPic(hwnd, NULL);
				return TRUE;
			}
			break;
		case IDC_SAVE:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				char avfile[MAX_PATH+1];
				HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
				GetCurrentSelFile(list, avfile);
				if(avfile[0]!='\0')
					ShowSaveDialog(hwnd, avfile);
				else
					MessageBox(hwnd, Translate("Please select an avatar from the list"), Translate("No avatar selected"), MB_OK|MB_ICONEXCLAMATION);
				return TRUE;
			}
			break;
		case IDC_DELETE:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				char avfile[MAX_PATH+1];
				HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
				GetCurrentSelFile(list, avfile);
				if(avfile[0] == '\0')
				{
					MessageBox(hwnd, Translate("Please select an avatar from the list"), Translate("No avatar selected"), MB_OK|MB_ICONEXCLAMATION);
					return TRUE;
				}
				if(MessageBox(hwnd, Translate("Are you sure you wish to delete this archived avatar?"), Translate("Delete avatar?"), MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_SETFOREGROUND|MB_TOPMOST) == IDYES)
				{
					DeleteFile(avfile);
					SendMessage(list, LB_DELETESTRING, SendMessage(list, LB_GETCURSEL, 0, 0), 0);
				}
				return TRUE;
			}
			break;
		case IDC_OPENFOLDER:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				char avfolder[MAX_PATH+1];
				HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
				HANDLE hContact = (HANDLE)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				GetContactFolder(hContact, avfolder);
				ShellExecute(NULL, db_byte_get(NULL, "AvatarHistory", "OpenFolderMethod", 0)?"explore":"open", avfolder, NULL, NULL, SW_SHOWNORMAL);
				return TRUE;
			}
			break;
		default:
			break;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
}

int FillAvatarList(HWND list, HANDLE hContact, char* def_sel)
{
	char dir[MAX_PATH+1], patt[MAX_PATH+1];
	long idx = 0, sel = 0, newsel = 0;
	WIN32_FIND_DATA finddata;
	HANDLE hFind = NULL;
	GetContactFolder(hContact, dir);
	strcpy(patt, dir);
	strcat(patt, "\\*.*");
	hFind = FindFirstFile(patt, &finddata);
	do
	{
		if(finddata.cFileName[0] != '.')
		{
			// Add to list
			idx = SendMessage(list,LB_ADDSTRING, 0, (LPARAM)finddata.cFileName);
			SendMessage(list, LB_SETITEMDATA, idx, (LPARAM)hContact);
		}
	}while(FindNextFile(hFind, &finddata));
	FindClose(hFind);
	if(def_sel != NULL && def_sel[0] != '\0')
		SendMessage(list, LB_SELECTSTRING, 1, (LPARAM)def_sel);
	else
		SendMessage(list, LB_SETCURSEL, 0, 0); // Set to first item
	return 0;
}

int UpdateAvatarPic(HWND hwnd, char* fn)
{
	HWND hwndpic = GetDlgItem(hwnd, IDC_AVATAR);
	HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
	HBITMAP avpic = NULL;
	char avfn[MAX_PATH+1];
	if(!hwnd || !hwndpic)
		return -1;
	if(!fn || fn[0] == '\0')
		GetCurrentSelFile(list, avfn);
	avpic = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)(fn?fn:avfn));
	avpic = (HBITMAP)SendMessage(hwndpic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)avpic);
	if(avpic)
		DeleteObject(avpic);
	return 0;
}

void InitMenuItem()
{
	CLISTMENUITEM mi;
	ZeroMemory(&mi, sizeof(CLISTMENUITEM));
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.pszName = Translate("Avatar history...");
	mi.position = 100;
	mi.hIcon = (HICON)LoadIcon(hInst, MAKEINTRESOURCE(IDI_AVATARHIST));
	mi.pszService = "AvatarHistory/ShowDialog";
	CreateServiceFunction("AvatarHistory/ShowDialog", ShowDialogSvc);
	CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
}

static int ShowDialogSvc(WPARAM wParam, LPARAM lParam)
{
	OpenAvatarDialog((HANDLE)wParam, (char*)lParam);
	return 0;
}

char* GetCurrentSelFile(HWND list, char* out)
{
	char contactdir[MAX_PATH+1], file[MAX_PATH+1];
	int cursel = SendMessage(list, LB_GETCURSEL, 0, 0);
	HANDLE hContact;
	if(cursel>-1)
	{
		hContact = (HANDLE)GetWindowLongPtr(GetParent(list), GWLP_USERDATA);
		GetContactFolder(hContact, contactdir);
		SendMessage(list, LB_GETTEXT, cursel, (LPARAM)file);
		sprintf(out, "%s\\%s", contactdir, file);
	}
	else
	{
		strcpy(out, "");
	}
	return out;
}

int ShowSaveDialog(HWND hwnd, char* fn)
{
	char initdir[MAX_PATH+1], filter[MAX_PATH+1];
	char file[MAX_PATH+1];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	MyDBGetString(NULL, "AvatarHistory", "SavedAvatarFolder", initdir, MAX_PATH+1);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = hInst;
	
	CallService(MS_UTILS_GETBITMAPFILTERSTRINGS, MAX_PATH, (LPARAM)filter);
	ofn.lpstrFilter = filter;
	
	ofn.nFilterIndex = 1;
	strcpy(file, strrchr(fn, '\\')+1);
	ofn.lpstrFile = file;

	ofn.nMaxFile = MAX_PATH+1;
	ofn.Flags = OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = strrchr(fn, '.')+1;
	ofn.lpstrInitialDir = initdir[0]?initdir:NULL;
	if(GetSaveFileName(&ofn))
		CopyFile(fn, file, FALSE);
	return 0;
}

char* MyDBGetString(HANDLE hContact, char* module, char* setting, char* out, size_t len)
{
	DBCONTACTGETSETTING dbgcs;
	DBVARIANT dbv;
	dbgcs.szModule = module;
	dbgcs.szSetting = setting;
	dbgcs.pValue = &dbv;

	dbv.type = DBVT_ASCIIZ;
	dbv.pszVal = out;
	dbv.cchVal = len-1;
	ZeroMemory((void*)out, len);
	CallService(MS_DB_CONTACT_GETSETTINGSTATIC, (WPARAM)hContact, (LPARAM)&dbgcs);
	return out;
}