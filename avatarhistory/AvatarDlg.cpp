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

#include "AvatarHistory.h"

#include <commctrl.h>
#include <prsht.h>


extern HINSTANCE hInst;
HANDLE hMenu = 0; 
int OpenAvatarDialog(HANDLE hContact, char* fn);
DWORD WINAPI AvatarDialogThread(LPVOID param);
static BOOL CALLBACK AvatarDlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int ShowSaveDialog(HWND hwnd, TCHAR* fn);

int FillAvatarList(HWND list, HANDLE hContact, TCHAR* fn);
int UpdateAvatarPic(HWND hwnd, TCHAR* fn);
TCHAR* GetCurrentSelFile(HWND list, TCHAR* out);

static int ShowDialogSvc(WPARAM wParam, LPARAM lParam);

struct AvatarDialogData
{
	HANDLE hContact;
	TCHAR fn[MAX_PATH+1];
	HWND parent;
};

int OpenAvatarDialog(HANDLE hContact, char* fn)
{
	DWORD dwId;
	struct AvatarDialogData* avdlg;
	avdlg = (struct AvatarDialogData*)malloc(sizeof(struct AvatarDialogData));
	ZeroMemory(avdlg, sizeof(struct AvatarDialogData));
	avdlg->hContact = hContact;
	if (fn == NULL)
	{
		avdlg->fn[0] = _T('\0');
	}
	else
	{
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, fn, -1, avdlg->fn, MAX_REGS(avdlg->fn));
#else
		lstrcpyn(avdlg->fn, fn, sizeof(avdlg->fn));
#endif
	}

	CloseHandle(CreateThread(NULL, 0, AvatarDialogThread, (LPVOID)avdlg, 0, &dwId));
	return 0;
}

DWORD WINAPI AvatarDialogThread(LPVOID param)
{
	struct AvatarDialogData* data = (struct AvatarDialogData*)param;
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_AVATARDLG), data->parent, AvatarDlgProc, (LPARAM)param);
	return 0;
}

void EnableDisableControls(HWND hwnd)
{
	HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
	
	int cursel = SendMessage(list, LB_GETCURSEL, 0, 0);
	int count = SendMessage(list, LB_GETCOUNT, 0, 0);

	if (cursel == LB_ERR)
	{
		EnableWindow(GetDlgItem(hwnd, IDC_BACK), count > 0);
		EnableWindow(GetDlgItem(hwnd, IDC_NEXT), count > 0);
	}
	else
	{
		EnableWindow(GetDlgItem(hwnd, IDC_BACK), cursel > 0);
		EnableWindow(GetDlgItem(hwnd, IDC_NEXT), cursel < count-1);
	}

	EnableWindow(GetDlgItem(hwnd, IDC_SAVE), cursel != LB_ERR);
	EnableWindow(GetDlgItem(hwnd, IDC_DELETE), cursel != LB_ERR);
}

static BOOL CALLBACK AvatarDlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static struct AvatarDialogData* data;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			data = (struct AvatarDialogData*) lParam;
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM) overlayedBigIcon);
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) overlayedIcon);
			FillAvatarList(GetDlgItem(hwnd, IDC_AVATARLIST), data->hContact, data->fn);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (ULONG_PTR)data->hContact);
			UpdateAvatarPic(hwnd, NULL);
			CheckDlgButton(hwnd, IDC_LOGUSER, (UINT)db_byte_get(data->hContact, "AvatarHistory", "LogToDisk", BST_INDETERMINATE));
			CheckDlgButton(hwnd, IDC_POPUPUSER, (UINT)db_byte_get(data->hContact, "AvatarHistory", "AvatarPopups", BST_INDETERMINATE));
			CheckDlgButton(hwnd, IDC_HISTORYUSER, (UINT)db_byte_get(data->hContact, "AvatarHistory", "LogToHistory", BST_INDETERMINATE));
			TranslateDialogDefault(hwnd);
			EnableDisableControls(hwnd);
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
				HANDLE hContact = (HANDLE) GetWindowLongPtr(hwnd, GWLP_USERDATA);

				if(IsDlgButtonChecked(hwnd, IDC_POPUPUSER) != BST_INDETERMINATE)
				{
					db_byte_set(hContact, "AvatarHistory", "AvatarPopups", (BYTE) IsDlgButtonChecked(hwnd, IDC_POPUPUSER));
				}
				else
				{
					DBDeleteContactSetting(hContact, "AvatarHistory", "AvatarPopups");
				}
		
				if(IsDlgButtonChecked(hwnd, IDC_LOGUSER) != BST_INDETERMINATE)
				{
					db_byte_set(hContact, "AvatarHistory", "LogToDisk", (BYTE) IsDlgButtonChecked(hwnd, IDC_LOGUSER));
				}
				else
				{
					DBDeleteContactSetting(hContact, "AvatarHistory", "LogToDisk");
				}
		
				if(IsDlgButtonChecked(hwnd, IDC_HISTORYUSER) != BST_INDETERMINATE)
				{
					db_byte_set(hContact, "AvatarHistory", "LogToHistory", (BYTE) IsDlgButtonChecked(hwnd, IDC_HISTORYUSER));
				}
				else
				{					
					DBDeleteContactSetting(hContact, "AvatarHistory", "LogToHistory");
				}

				EndDialog(hwnd, 0);
				return TRUE;
			}
			break;
		case IDC_AVATARLIST:
			if(HIWORD(wParam) == LBN_SELCHANGE)
			{
				UpdateAvatarPic(hwnd, NULL);
				EnableDisableControls(hwnd);
				return TRUE;
			}
			break;
		case IDC_NEXT:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
				SendMessage(list, LB_SETCURSEL, SendMessage(list, LB_GETCURSEL, 0, 0) +1, 0);
				UpdateAvatarPic(hwnd, NULL);
				EnableDisableControls(hwnd);
				return TRUE;
			}
			break;
		case IDC_BACK:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
				int cursel = SendMessage(list, LB_GETCURSEL, 0, 0);
				if (cursel == LB_ERR)
					SendMessage(list, LB_SETCURSEL, SendMessage(list, LB_GETCOUNT, 0, 0) -1, 0);
				else
					SendMessage(list, LB_SETCURSEL, cursel -1, 0);
				UpdateAvatarPic(hwnd, NULL);
				EnableDisableControls(hwnd);
				return TRUE;
			}
			break;
		case IDC_SAVE:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				TCHAR avfile[MAX_PATH+1];
				HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
				GetCurrentSelFile(list, avfile);
				if(avfile[0]!=_T('\0'))
					ShowSaveDialog(hwnd, avfile);
				else
					MessageBox(hwnd, TranslateT("Please select an avatar from the list"), TranslateT("No avatar selected"), MB_OK|MB_ICONEXCLAMATION);
				return TRUE;
			}
			break;
		case IDC_DELETE:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				TCHAR avfile[MAX_PATH+1];
				HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
				int cursel = SendMessage(list, LB_GETCURSEL, 0, 0);
				if (cursel == LB_ERR)
					break;
				GetCurrentSelFile(list, avfile);
				if(avfile[0] == _T('\0'))
				{
					MessageBox(hwnd, TranslateT("Please select an avatar from the list"), TranslateT("No avatar selected"), MB_OK|MB_ICONEXCLAMATION);
					return TRUE;
				}
				if(MessageBox(hwnd, TranslateT("Are you sure you wish to delete this archived avatar?"), TranslateT("Delete avatar?"), MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_SETFOREGROUND|MB_TOPMOST) == IDYES)
				{
					DeleteFile(avfile);
					SendMessage(list, LB_DELETESTRING, SendMessage(list, LB_GETCURSEL, 0, 0), 0);
				}
				int count = SendMessage(list, LB_GETCOUNT, 0, 0);
				if (count > 0)
				{
					if (cursel >= count)
						cursel = count -1;
					SendMessage(list, LB_SETCURSEL, cursel, 0);
				}
				EnableDisableControls(hwnd);
				return TRUE;
			}
			break;
		case IDC_OPENFOLDER:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				TCHAR avfolder[MAX_PATH+1];
				HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
				HANDLE hContact = (HANDLE)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				GetContactFolder(hContact, avfolder);
				ShellExecute(NULL, db_byte_get(NULL, "AvatarHistory", "OpenFolderMethod", 0) 
					? _T("explore") : _T("open"), avfolder, NULL, NULL, SW_SHOWNORMAL);
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

int FillAvatarList(HWND list, HANDLE hContact, TCHAR* def_sel)
{
	TCHAR dir[MAX_PATH+1], patt[MAX_PATH+1];
	long idx = 0, sel = 0, newsel = 0;
	WIN32_FIND_DATA finddata;
	HANDLE hFind = NULL;
	GetContactFolder(hContact, dir);
	mir_sntprintf(patt, MAX_REGS(patt), _T("%s\\*.*"), dir);
	hFind = FindFirstFile(patt, &finddata);
	do
	{
		if(finddata.cFileName[0] != _T('.'))
		{
			// Add to list
			idx = SendMessage(list,LB_ADDSTRING, 0, (LPARAM)finddata.cFileName);
			SendMessage(list, LB_SETITEMDATA, idx, (LPARAM)hContact);
		}
	}while(FindNextFile(hFind, &finddata));
	FindClose(hFind);
	if(def_sel != NULL && def_sel[0] != _T('\0'))
		SendMessage(list, LB_SELECTSTRING, 1, (LPARAM)def_sel);
	else
		SendMessage(list, LB_SETCURSEL, 0, 0); // Set to first item
	return 0;
}

int UpdateAvatarPic(HWND hwnd, TCHAR* fn)
{
	HWND hwndpic = GetDlgItem(hwnd, IDC_AVATAR);
	HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
	HBITMAP avpic = NULL;
	TCHAR avfn[MAX_PATH+1];
	if(!hwnd || !hwndpic)
		return -1;
	if(!fn || fn[0] == _T('\0'))
	{
		GetCurrentSelFile(list, avfn);
		fn = avfn;
	}

	// Miranda dont have this service in unicode format
#ifdef UNICODE
	char tmp[MAX_PATH+1];
	WideCharToMultiByte(CP_ACP, 0, fn, -1, tmp, MAX_REGS(tmp), NULL, NULL);

	avpic = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM) tmp);
#else
	avpic = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM) fn);
#endif

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
	mi.hIcon = overlayedIcon;
	mi.pszService = "AvatarHistory/ShowDialog";
	CreateServiceFunction("AvatarHistory/ShowDialog", ShowDialogSvc);
	hMenu = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
}

static int ShowDialogSvc(WPARAM wParam, LPARAM lParam)
{
	OpenAvatarDialog((HANDLE)wParam, (char*)lParam);
	return 0;
}

TCHAR* GetCurrentSelFile(HWND list, TCHAR* out)
{
	TCHAR contactdir[MAX_PATH+1], file[MAX_PATH+1];
	int cursel = SendMessage(list, LB_GETCURSEL, 0, 0);
	HANDLE hContact;
	if(cursel>-1)
	{
		hContact = (HANDLE)GetWindowLongPtr(GetParent(list), GWLP_USERDATA);
		GetContactFolder(hContact, contactdir);
		SendMessage(list, LB_GETTEXT, cursel, (LPARAM)file);
		mir_sntprintf(out, MAX_PATH+1, _T("%s\\%s"), contactdir, file);
	}
	else
	{
		out[0] = _T('\0');
	}
	return out;
}

int ShowSaveDialog(HWND hwnd, TCHAR* fn)
{
	TCHAR initdir[MAX_PATH+1];
	char filter[MAX_PATH+1];
	TCHAR file[MAX_PATH+1];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	MyDBGetString(NULL, "AvatarHistory", "SavedAvatarFolder", initdir, MAX_PATH+1);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = hInst;

	// Miranda dont have the unicode version of this servicce
	CallService(MS_UTILS_GETBITMAPFILTERSTRINGS, MAX_PATH, (LPARAM)filter);
#ifdef UNICODE
	TCHAR filterT[MAX_PATH+1];
	MultiByteToWideChar(CP_ACP, 0, filter, -1, filterT, MAX_REGS(filterT));
#else
	ofn.lpstrFilter = filter;
#endif
	
	ofn.nFilterIndex = 1;
	lstrcpy(file, _tcsrchr(fn, '\\')+1);
	ofn.lpstrFile = file;

	ofn.nMaxFile = MAX_PATH+1;
	ofn.Flags = OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = _tcsrchr(fn, '.')+1;
	ofn.lpstrInitialDir = initdir[0]?initdir:NULL;
	if(GetSaveFileName(&ofn))
		CopyFile(fn, file, FALSE);
	return 0;
}

TCHAR* MyDBGetString(HANDLE hContact, char* module, char* setting, TCHAR* out, size_t len)
{
	DBCONTACTGETSETTING dbgcs;
	DBVARIANT dbv;
	dbgcs.szModule = module;
	dbgcs.szSetting = setting;
	dbgcs.pValue = &dbv;

#ifdef UNICODE
	dbv.type = DBVT_WCHAR;
#else
	dbv.type = DBVT_ASCIIZ;
#endif
	dbv.ptszVal = out;
	dbv.cchVal = len;
	ZeroMemory((void*)out, len);
	CallService(MS_DB_CONTACT_GETSETTINGSTATIC, (WPARAM)hContact, (LPARAM)&dbgcs);
	return out;
}