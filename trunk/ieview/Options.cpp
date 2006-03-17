/*

IEView Plugin for Miranda IM
Copyright (C) 2005  Piotr Piastucki

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include <shlobj.h>
#include "Options.h"
#include "resource.h"
#include "Smiley.h"
#include "Template.h"
#include "Utils.h"
#include "m_MathModule.h"
#include "m_avatars.h"

#define UM_CHECKSTATECHANGE (WM_USER+100)
HANDLE hHookOptionsChanged;
static BOOL CALLBACK IEViewOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewGeneralOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
//static BOOL CALLBACK IEViewEmoticonsOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewSRMMOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewGroupChatsOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewHistoryOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND hwndCurrentTab, hwndPages[4];
static ProtocolSettings *currentProtoItem = NULL;
static HIMAGELIST hProtocolImageList = NULL;

#ifndef _MSC_VER
typedef struct tagTVKEYDOWN {
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
} NMTVKEYDOWN, FAR *LPNMTVKEYDOWN;

BOOL TreeView_SetCheckState(HWND hwndTreeView, HTREEITEM hItem, BOOL fCheck)
{
    TVITEM tvItem;

    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    // Image 1 in the tree-view check box image list is the
    // unchecked box. Image 2 is the checked box.

    tvItem.state = INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));

    return TreeView_SetItem(hwndTreeView, &tvItem);
}

BOOL TreeView_GetCheckState(HWND hwndTreeView, HTREEITEM hItem)
{
    TVITEM tvItem;

    // Prepare to receive the desired information.
    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    // Request the information.
    TreeView_GetItem(hwndTreeView, &tvItem);

    // Return zero if it's not checked, or nonzero otherwise.
    return ((BOOL)(tvItem.state >> 12) -1);
}
#endif

static LPARAM GetItemParam(HWND hwndTreeView, HTREEITEM hItem) {
	TVITEM tvi = {0};
	tvi.mask = TVIF_PARAM;
	tvi.hItem = hItem == NULL ? TreeView_GetSelection(hwndTreeView) : hItem;
	TreeView_GetItem(hwndTreeView, &tvi);
	return tvi.lParam;
}

static void SaveProtoSettings(HWND hwndDlg, ProtocolSettings *proto) {
	if (proto != NULL) {
		char path[MAX_PATH];
		GetDlgItemText(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, sizeof(path));
		proto->setSRMMCssFilenameTemp(path);
		GetDlgItemText(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, path, sizeof(path));
		proto->setSRMMCssFilenameRtl(path);
		GetDlgItemText(hwndDlg, IDC_TEMPLATES_FILENAME, path, sizeof(path));
		proto->setSRMMTemplateFilenameTemp(path);
		GetDlgItemText(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, path, sizeof(path));
		proto->setSRMMTemplateFilenameRtlTemp(path);
	}
}

static void UpdateFilenameInfo(HWND hwndDlg, ProtocolSettings *proto) {
	if (proto != NULL) {
		HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
		TreeView_SetCheckState(hProtoList, TreeView_GetSelection(hProtoList), proto->isEnableTemp());
		if (proto->getSRMMCssFilename() != NULL) {
			SetDlgItemText(hwndDlg, IDC_EXTERNALCSS_FILENAME, proto->getSRMMCssFilenameTemp());
		} else {
			SetDlgItemText(hwndDlg, IDC_EXTERNALCSS_FILENAME, "");
		}
		if (proto->getSRMMCssFilenameRtl() != NULL) {
			SetDlgItemText(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, proto->getSRMMCssFilenameRtlTemp());
		} else {
			SetDlgItemText(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, "");
		}
		if (proto->getSRMMTemplateFilenameTemp() != NULL) {
			SetDlgItemText(hwndDlg, IDC_TEMPLATES_FILENAME, proto->getSRMMTemplateFilenameTemp());
		} else {
			SetDlgItemText(hwndDlg, IDC_TEMPLATES_FILENAME, "");
		}
		if (proto->getSRMMTemplateFilenameRtlTemp() != NULL) {
			SetDlgItemText(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, proto->getSRMMTemplateFilenameRtlTemp());
		} else {
			SetDlgItemText(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, "");
		}
		currentProtoItem = proto;
	}
}

static void RefreshProtoIcons(HWND hwndDlg) {
	int i;
	HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
	TreeView_DeleteAllItems(hProtoList);
	ProtocolSettings *proto;
	if (hProtocolImageList != NULL) {
		ImageList_RemoveAll(hProtocolImageList);
	} else {
		for (i=0,proto=Options::getProtocolSettings();proto!=NULL;proto=proto->getNext(),i++);
		hProtocolImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
			ILC_MASK | ILC_COLOR32, i, 0);
	}
	for (i=0,proto=Options::getProtocolSettings();proto!=NULL;proto=proto->getNext(),i++) {
		HICON hIcon = NULL;
		if (i > 0 ) {
			hIcon=(HICON)CallProtoService(proto->getProtocolName(), PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
			if (hIcon == NULL)  {
				hIcon=(HICON)CallProtoService(proto->getProtocolName(), PS_LOADICON, PLI_PROTOCOL, 0);
			}
		}
		if (hIcon == NULL) {
			hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_SMILEY), IMAGE_ICON,
							GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		}
		ImageList_AddIcon(hProtocolImageList, hIcon);
		DestroyIcon(hIcon);
	}
	TreeView_SetImageList(hProtoList, hProtocolImageList, TVSIL_NORMAL);
//	refreshProtoList(hwndDlg, IsDlgButtonChecked(hwndDlg, IDC_PROTO_SMILEYS));
}

static void RefreshProtoList(HWND hwndDlg, bool protoTemplates) {
	int i;
    HTREEITEM hItem = NULL;
	HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
	TreeView_DeleteAllItems(hProtoList);
	ProtocolSettings *proto;
	for (i=0,proto=Options::getProtocolSettings();proto!=NULL;proto=proto->getNext(),i++) {
		char protoName[128];
		TVINSERTSTRUCT tvi = {0};
		tvi.hParent = TVI_ROOT;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_STATE | TVIF_SELECTEDIMAGE;
		tvi.item.stateMask = TVIS_SELECTED | TVIS_STATEIMAGEMASK;
		if (i==0) {
			strcpy(protoName, Translate("Default"));
			currentProtoItem = proto;
		} else {
			CallProtoService(proto->getProtocolName(), PS_GETNAME, sizeof(protoName), (LPARAM)protoName);
//			strcat(protoName, " ");
	//		strcat(protoName, Translate("protocol"));
		}
		tvi.item.pszText = protoName;
		tvi.item.lParam = (LPARAM)proto;
		tvi.item.iImage = i;
		tvi.item.iSelectedImage = i;
		tvi.item.state = INDEXTOSTATEIMAGEMASK(proto->isEnableTemp() ? 2 : 1);
		if (i==0) {
			hItem = TreeView_InsertItem(hProtoList, &tvi);
		} else {
			TreeView_InsertItem(hProtoList, &tvi);
		}
		if (!protoTemplates) break;
	}
	UpdateFilenameInfo(hwndDlg, Options::getProtocolSettings());
	TreeView_SelectItem(hProtoList, hItem);
}


static bool BrowseFile(HWND hwndDlg, TCHAR *filter, TCHAR *defExt,  TCHAR *path, int maxLen) {
	OPENFILENAMEA ofn={0};
	GetWindowText(hwndDlg, path, maxLen);
	ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
	ofn.hwndOwner = hwndDlg;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = filter;//"Templates (*.ivt)\0*.ivt\0All Files\0*.*\0\0";
	ofn.lpstrFile = path;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.nMaxFile = maxLen;
	ofn.nMaxFileTitle = maxLen;
	ofn.lpstrDefExt = defExt;//"ivt";
	if(GetOpenFileName(&ofn)) {
		SetWindowText(hwndDlg, path);
		return true;
	}
	return false;
}

int IEViewOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = hInstance;
	odp.pszGroup = Translate("Message Sessions");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle = Translate("IEView");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = IEViewOptDlgProc;
	odp.nIDBottomSimpleControl = 0;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) &odp);
	return 0;
}

static BOOL CALLBACK IEViewOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		{
            HWND tc;
			TCITEM tci;
			tc = GetDlgItem(hwndDlg, IDC_TABS);
			tci.mask = TCIF_TEXT;
			tci.pszText = TranslateT("General");
			TabCtrl_InsertItem(tc, 0, &tci);
			tci.pszText = TranslateT("Message Log");
			TabCtrl_InsertItem(tc, 1, &tci);
			tci.pszText = TranslateT("Group Chats");
			TabCtrl_InsertItem(tc, 2, &tci);
			tci.pszText = TranslateT("History");
			TabCtrl_InsertItem(tc, 3, &tci);

//			hwndEmoticons = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_EMOTICONS_OPTIONS), hwndDlg, IEViewEmoticonsOptDlgProc, (LPARAM) NULL);
	//		SetWindowPos(hwndEmoticons, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
			hwndPages[0] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_GENERAL_OPTIONS), hwndDlg, IEViewGeneralOptDlgProc, (LPARAM) NULL);
			SetWindowPos(hwndPages[0], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
			hwndPages[1] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_SRMM_OPTIONS), hwndDlg, IEViewSRMMOptDlgProc, (LPARAM) NULL);
			SetWindowPos(hwndPages[1], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
			hwndPages[2] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_GROUPCHATS_OPTIONS), hwndDlg, IEViewGroupChatsOptDlgProc, (LPARAM) NULL);
			SetWindowPos(hwndPages[2], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
			hwndPages[3] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_HISTORY_OPTIONS), hwndDlg, IEViewHistoryOptDlgProc, (LPARAM) NULL);
			SetWindowPos(hwndPages[3], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
			hwndCurrentTab = hwndPages[0];
			ShowWindow(hwndPages[0], SW_SHOW);
			return TRUE;
		}
	case WM_COMMAND:
		break;
	case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->code) {
			case TCN_SELCHANGE:
                switch (wParam) {
				case IDC_TABS:
					{
						HWND hwnd = hwndPages[TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TABS))];
						if (hwnd!=hwndCurrentTab) {
	                    	ShowWindow(hwnd, SW_SHOW);
	                    	ShowWindow(hwndCurrentTab, SW_HIDE);
	                    	hwndCurrentTab = hwnd;
						}
					}
					break;
				}
				break;
			case PSN_APPLY:
				for (int i = 0; i < 4; i++) {
                    SendMessage(hwndPages[i], WM_NOTIFY, wParam, lParam);
				}
				NotifyEventHooks(hHookOptionsChanged, 0, 0);
				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}

static BOOL CALLBACK IEViewGeneralOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int i;
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			if (Options::getGeneralFlags() & Options::GENERAL_ENABLE_BBCODES) {
				CheckDlgButton(hwndDlg, IDC_ENABLE_BBCODES, TRUE);
			}
			if (Options::getGeneralFlags() & Options::GENERAL_ENABLE_FLASH) {
				CheckDlgButton(hwndDlg, IDC_ENABLE_FLASH, TRUE);
			}
			if (Options::getGeneralFlags() & Options::GENERAL_ENABLE_MATHMODULE) {
				CheckDlgButton(hwndDlg, IDC_ENABLE_MATHMODULE, TRUE);
			}
			if (Options::getGeneralFlags() & Options::GENERAL_ENABLE_PNGHACK) {
				CheckDlgButton(hwndDlg, IDC_ENABLE_PNGHACK, TRUE);
			}
			if (Options::getGeneralFlags() & Options::GENERAL_SMILEYINNAMES) {
				CheckDlgButton(hwndDlg, IDC_SMILEYS_IN_NAMES, TRUE);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_MATHMODULE), Options::isMathModule());
			EnableWindow(GetDlgItem(hwndDlg, IDC_SMILEYS_IN_NAMES), Options::isSmileyAdd());
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_ENABLE_BBCODES:
            case IDC_ENABLE_FLASH:
            case IDC_ENABLE_MATHMODULE:
            case IDC_ENABLE_PNGHACK:
            case IDC_SMILEYS_IN_NAMES:
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;
	case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->code) {
			case PSN_APPLY:
				i = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_BBCODES)) {
					i |= Options::GENERAL_ENABLE_BBCODES;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_FLASH)) {
					i |= Options::GENERAL_ENABLE_FLASH;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_MATHMODULE)) {
					i |= Options::GENERAL_ENABLE_MATHMODULE;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_PNGHACK)) {
					i |= Options::GENERAL_ENABLE_PNGHACK;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_SMILEYS_IN_NAMES)) {
					i |= Options::GENERAL_SMILEYINNAMES;
				}
				Options::setGeneralFlags(i);
				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}

static BOOL CALLBACK IEViewSRMMOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int i;
	BOOL bChecked;
	char path[MAX_PATH];
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			if (Options::getSRMMFlags() & Options::LOG_IMAGE_SCROLL) {
				CheckDlgButton(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE, TRUE);
			}
			if (Options::getSRMMFlags() & Options::LOG_SHOW_NICKNAMES) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_NICKNAMES, TRUE);
			}
			if (Options::getSRMMFlags() & Options::LOG_SHOW_TIME) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_TIME, TRUE);
			}
			if (Options::getSRMMFlags() & Options::LOG_SHOW_DATE) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_DATE, TRUE);
			}
			if (Options::getSRMMFlags() & Options::LOG_SHOW_SECONDS) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_SECONDS, TRUE);
			}
			if (Options::getSRMMFlags() & Options::LOG_LONG_DATE) {
				CheckDlgButton(hwndDlg, IDC_LOG_LONG_DATE, TRUE);
			}
			if (Options::getSRMMFlags() & Options::LOG_RELATIVE_DATE) {
				CheckDlgButton(hwndDlg, IDC_LOG_RELATIVE_DATE, TRUE);
			}
			if (Options::getSRMMFlags() & Options::LOG_GROUP_MESSAGES) {
				CheckDlgButton(hwndDlg, IDC_LOG_GROUP_MESSAGES, TRUE);
			}

			bChecked = FALSE;
			if (Options::getSRMMMode() == Options::MODE_TEMPLATE) {
			    bChecked = TRUE;
			}
			CheckDlgButton(hwndDlg, IDC_MODE_TEMPLATE, bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATES_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_TEMPLATES), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATES_FILENAME_RTL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_TEMPLATES_RTL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_NICKNAMES), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_TIME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_DATE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_SECONDS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_LONG_DATE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_RELATIVE_DATE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_GROUP_MESSAGES), bChecked);

			bChecked = FALSE;
			if (Options::getSRMMMode() == Options::MODE_CSS) {
			    bChecked = TRUE;
			}
			CheckDlgButton(hwndDlg, IDC_MODE_CSS, bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS_RTL), bChecked);

			bChecked = FALSE;
			if (Options::getSRMMMode() == Options::MODE_COMPATIBLE) {
			    bChecked = TRUE;
				CheckDlgButton(hwndDlg, IDC_MODE_COMPATIBLE, TRUE);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE), bChecked);
			bChecked &= IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);

			Options::resetProtocolSettings();
			currentProtoItem = Options::getProtocolSettings();
			RefreshProtoIcons(hwndDlg);
			RefreshProtoList(hwndDlg, true);
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_BACKGROUND_IMAGE_FILENAME:
            case IDC_EXTERNALCSS_FILENAME:
            case IDC_EXTERNALCSS_FILENAME_RTL:
            case IDC_TEMPLATES_FILENAME:
            case IDC_TEMPLATES_FILENAME_RTL:
				if ((HWND)lParam==GetFocus() && HIWORD(wParam)==EN_CHANGE)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_SCROLL_BACKGROUND_IMAGE:
			case IDC_LOG_SHOW_NICKNAMES:
			case IDC_LOG_SHOW_TIME:
			case IDC_LOG_SHOW_DATE:
			case IDC_LOG_SHOW_SECONDS:
			case IDC_LOG_LONG_DATE:
			case IDC_LOG_RELATIVE_DATE:
			case IDC_LOG_GROUP_MESSAGES:
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_BACKGROUND_IMAGE:
                bChecked = IsDlgButtonChecked(hwndDlg, IDC_MODE_COMPATIBLE) && IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_BROWSE_TEMPLATES:
				{
					//if (BrowseFile(hwndDlg,
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Template (*.ivt)\0*.ivt\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "ivt";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_BROWSE_TEMPLATES_RTL:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Template (*.ivt)\0*.ivt\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "ivt";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_BROWSE_BACKGROUND_IMAGE:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "All Images (*.jpg,*.gif,*.png,*.bmp)\0*.jpg;*.gif;*.png;*.bmp\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "jpg";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg,IDC_BACKGROUND_IMAGE_FILENAME,path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_BROWSE_EXTERNALCSS:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Style Sheet (*.css)\0*.css\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "css";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_BROWSE_EXTERNALCSS_RTL:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Style Sheet (*.css)\0*.css\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "css";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_MODE_COMPATIBLE:
			case IDC_MODE_CSS:
			case IDC_MODE_TEMPLATE:

               	bChecked = IsDlgButtonChecked(hwndDlg, IDC_MODE_TEMPLATE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATES_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_TEMPLATES), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATES_FILENAME_RTL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_TEMPLATES_RTL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_NICKNAMES), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_TIME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_DATE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_SECONDS), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_LONG_DATE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_RELATIVE_DATE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_GROUP_MESSAGES), bChecked);

				bChecked = IsDlgButtonChecked(hwndDlg, IDC_MODE_CSS);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS_RTL), bChecked);

               	bChecked = IsDlgButtonChecked(hwndDlg, IDC_MODE_COMPATIBLE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE), bChecked);
				bChecked &= IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);

				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;
	case UM_CHECKSTATECHANGE:
		{
			ProtocolSettings *proto = (ProtocolSettings *)GetItemParam((HWND)wParam, (HTREEITEM) lParam);
			if (proto != NULL) {
				if (strcmpi(proto->getProtocolName(), "_default_")) {
					proto->setEnableTemp(TreeView_GetCheckState((HWND)wParam, (HTREEITEM) lParam));
				}
			}
			if ((HTREEITEM) lParam != TreeView_GetSelection((HWND)wParam)) {
				TreeView_SelectItem((HWND)wParam, (HTREEITEM) lParam);
			} else {
				UpdateFilenameInfo(hwndDlg, proto);
			}
			SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
		}
		break;
	case WM_NOTIFY:
		{
			if (((LPNMHDR)lParam)->idFrom == IDC_PROTOLIST) {
				switch (((LPNMHDR)lParam)->code) {
					case NM_CLICK:
						{
							TVHITTESTINFO ht = {0};
							DWORD dwpos = GetMessagePos();
							POINTSTOPOINT(ht.pt, MAKEPOINTS(dwpos));
							MapWindowPoints(HWND_DESKTOP, ((LPNMHDR)lParam)->hwndFrom, &ht.pt, 1);
							TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &ht);
							if (TVHT_ONITEMSTATEICON & ht.flags) {
                                PostMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom, (LPARAM)ht.hItem);
                                return FALSE;
							}
						}
						break;
					case TVN_KEYDOWN:
						 if (((LPNMTVKEYDOWN) lParam)->wVKey == VK_SPACE)
								PostMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom,
								(LPARAM)TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
						break;
					case TVN_SELCHANGED:
						{
							HWND hLstView = GetDlgItem(hwndDlg, IDC_PROTOLIST);
							ProtocolSettings *proto = (ProtocolSettings *)GetItemParam(hLstView, (HTREEITEM) NULL);
							SaveProtoSettings(hwndDlg, currentProtoItem);
							UpdateFilenameInfo(hwndDlg, proto);
						}
						break;
				}
				break;
			}
			switch (((LPNMHDR) lParam)->code) {
			case PSN_APPLY:
				i = Options::MODE_COMPATIBLE;
				if (IsDlgButtonChecked(hwndDlg, IDC_MODE_TEMPLATE)) {
					i = Options::MODE_TEMPLATE;
				} else if (IsDlgButtonChecked(hwndDlg, IDC_MODE_CSS)) {
					i = Options::MODE_CSS;
				}
				Options::setSRMMMode(i);

				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_NICKNAMES)) {
					i |= Options::LOG_SHOW_NICKNAMES;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_TIME)) {
					i |= Options::LOG_SHOW_TIME;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_DATE)) {
					i |= Options::LOG_SHOW_DATE;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_SECONDS)) {
					i |= Options::LOG_SHOW_SECONDS;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_LONG_DATE)) {
					i |= Options::LOG_LONG_DATE;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_RELATIVE_DATE)) {
					i |= Options::LOG_RELATIVE_DATE;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_GROUP_MESSAGES)) {
					i |= Options::LOG_GROUP_MESSAGES;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE)) {
					i |= Options::LOG_IMAGE_SCROLL;
				}
				Options::setSRMMFlags(i);
				GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, sizeof(path));
				Options::setBkgImageFile(path);
				SaveProtoSettings(hwndDlg, currentProtoItem);
				Options::saveProtocolSettings();
				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}

static BOOL CALLBACK IEViewHistoryOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int i;
	BOOL bChecked = FALSE;
	char path[MAX_PATH];
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			if (Options::getHistoryFlags() & Options::LOG_SHOW_NICKNAMES) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_NICKNAMES, TRUE);
			}
			if (Options::getHistoryFlags() & Options::LOG_SHOW_TIME) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_TIME, TRUE);
			}
			if (Options::getHistoryFlags() & Options::LOG_SHOW_DATE) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_DATE, TRUE);
			}
			if (Options::getHistoryFlags() & Options::LOG_SHOW_SECONDS) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_SECONDS, TRUE);
			}
			if (Options::getHistoryFlags() & Options::LOG_LONG_DATE) {
				CheckDlgButton(hwndDlg, IDC_LOG_LONG_DATE, TRUE);
			}
			if (Options::getHistoryFlags() & Options::LOG_RELATIVE_DATE) {
				CheckDlgButton(hwndDlg, IDC_LOG_RELATIVE_DATE, TRUE);
			}
			if (Options::getHistoryFlags() & Options::LOG_GROUP_MESSAGES) {
				CheckDlgButton(hwndDlg, IDC_LOG_GROUP_MESSAGES, TRUE);
			}

			EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATES_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_TEMPLATES), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATES_FILENAME_RTL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_TEMPLATES_RTL), bChecked);

			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_NICKNAMES), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_TIME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_DATE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_SECONDS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_LONG_DATE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_RELATIVE_DATE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_GROUP_MESSAGES), bChecked);

			if (Options::getHistoryTemplatesFile() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, Options::getHistoryTemplatesFile());
			}
			if (Options::getHistoryTemplatesFileRTL() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, Options::getHistoryTemplatesFileRTL());
			}

			EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS_RTL), bChecked);
			if (Options::getHistoryCSSFile() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, Options::getHistoryCSSFile());
			}
			if (Options::getHistoryCSSFileRTL() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, Options::getHistoryCSSFileRTL());
			}

			bChecked = !IsDlgButtonChecked(hwndDlg, IDC_TEMPLATES) && !IsDlgButtonChecked(hwndDlg, IDC_EXTERNALCSS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE), bChecked);
			if (Options::getHistoryFlags() & Options::LOG_IMAGE_ENABLED) {
				CheckDlgButton(hwndDlg, IDC_BACKGROUND_IMAGE, TRUE);
			} else {
				bChecked = FALSE;
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
			if (Options::getHistoryFlags() & Options::LOG_IMAGE_SCROLL) {
				CheckDlgButton(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE, TRUE);
			}
			if (Options::getBkgImageFile() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, Options::getBkgImageFile());
			}
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_BACKGROUND_IMAGE_FILENAME:
            case IDC_EXTERNALCSS_FILENAME:
            case IDC_EXTERNALCSS_FILENAME_RTL:
            case IDC_TEMPLATES_FILENAME:
            case IDC_TEMPLATES_FILENAME_RTL:
				if ((HWND)lParam==GetFocus() && HIWORD(wParam)==EN_CHANGE)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_SCROLL_BACKGROUND_IMAGE:
			case IDC_LOG_SHOW_NICKNAMES:
			case IDC_LOG_SHOW_TIME:
			case IDC_LOG_SHOW_DATE:
			case IDC_LOG_SHOW_SECONDS:
			case IDC_LOG_LONG_DATE:
			case IDC_LOG_RELATIVE_DATE:
			case IDC_LOG_GROUP_MESSAGES:
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_TEMPLATES:
				bChecked = IsDlgButtonChecked(hwndDlg, IDC_TEMPLATES);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATES_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_TEMPLATES), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATES_FILENAME_RTL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_TEMPLATES_RTL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_NICKNAMES), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_TIME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_DATE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_SECONDS), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_LONG_DATE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_RELATIVE_DATE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_GROUP_MESSAGES), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS), !bChecked);
//				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
	//			break;
			case IDC_EXTERNALCSS:
                bChecked = IsDlgButtonChecked(hwndDlg, IDC_TEMPLATES) || IsDlgButtonChecked(hwndDlg, IDC_EXTERNALCSS);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE), !bChecked);
               	bChecked = !IsDlgButtonChecked(hwndDlg, IDC_TEMPLATES) && IsDlgButtonChecked(hwndDlg, IDC_EXTERNALCSS);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS_RTL), bChecked);
		//		SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
			//	break;
			case IDC_BACKGROUND_IMAGE:
                bChecked = !IsDlgButtonChecked(hwndDlg, IDC_TEMPLATES) && !IsDlgButtonChecked(hwndDlg, IDC_EXTERNALCSS) && IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_BROWSE_TEMPLATES:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Templates (*.ivt)\0*.ivt\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "ivt";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_BROWSE_TEMPLATES_RTL:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Templates (*.ivt)\0*.ivt\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "ivt";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_BROWSE_BACKGROUND_IMAGE:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "All Images (*.jpg,*.gif,*.png,*.bmp)\0*.jpg;*.gif;*.png;*.bmp\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "jpg";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg,IDC_BACKGROUND_IMAGE_FILENAME,path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_BROWSE_EXTERNALCSS:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Style Sheet (*.css)\0*.css\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "css";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_BROWSE_EXTERNALCSS_RTL:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Style Sheet (*.css)\0*.css\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "css";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			}
		}
		break;
	case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->code) {
			case PSN_APPLY:
				i = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_NICKNAMES)) {
					i |= Options::LOG_SHOW_NICKNAMES;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_TIME)) {
					i |= Options::LOG_SHOW_TIME;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_DATE)) {
					i |= Options::LOG_SHOW_DATE;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_SECONDS)) {
					i |= Options::LOG_SHOW_SECONDS;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_LONG_DATE)) {
					i |= Options::LOG_LONG_DATE;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_RELATIVE_DATE)) {
					i |= Options::LOG_RELATIVE_DATE;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_GROUP_MESSAGES)) {
					i |= Options::LOG_GROUP_MESSAGES;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE)) {
					i |= Options::LOG_IMAGE_ENABLED;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE)) {
					i |= Options::LOG_IMAGE_SCROLL;
				}
				Options::setHistoryFlags(i);
				GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path, sizeof(path));
				Options::setHistoryTemplatesFile(path);
				GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, path, sizeof(path));
				Options::setHistoryTemplatesFileRTL(path);
				GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, sizeof(path));
//				Options::setBkgImageFile(path);
				GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, sizeof(path));
				Options::setHistoryCSSFile(path);
				GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, path, sizeof(path));
				Options::setHistoryCSSFileRTL(path);
				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}

static BOOL CALLBACK IEViewGroupChatsOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int i;
	BOOL bChecked;
	char path[MAX_PATH];
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			if (Options::getGroupChatCSSFile() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_GROUPCHAT_CSS_FILENAME, Options::getGroupChatCSSFile());
			}
			bChecked = FALSE;
			EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_TEMPLATES_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_TEMPLATES_BROWSE), bChecked);
			if (Options::getGroupChatTemplatesFile() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_GROUPCHAT_TEMPLATES_FILENAME, Options::getGroupChatTemplatesFile());
			}
			bChecked = !IsDlgButtonChecked(hwndDlg, IDC_GROUPCHAT_TEMPLATES);
			EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_CSS), bChecked);
/*			if (Options::getGroupChatFlags() & Options::CSS_ENABLED) {
				CheckDlgButton(hwndDlg, IDC_GROUPCHAT_CSS, TRUE);
			} else {
                bChecked = FALSE;
			}*/
			EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_CSS_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_CSS_BROWSE), bChecked);
			if (Options::getGroupChatCSSFile() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_GROUPCHAT_CSS_FILENAME, Options::getGroupChatCSSFile());
			}
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
            case IDC_GROUPCHAT_CSS_FILENAME:
				if ((HWND)lParam==GetFocus() && HIWORD(wParam)==EN_CHANGE)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_GROUPCHAT_TEMPLATES:
				bChecked = IsDlgButtonChecked(hwndDlg, IDC_GROUPCHAT_TEMPLATES);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_TEMPLATES_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_TEMPLATES_BROWSE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_CSS), !bChecked);
			case IDC_GROUPCHAT_CSS:
               	bChecked = !IsDlgButtonChecked(hwndDlg, IDC_GROUPCHAT_TEMPLATES) && IsDlgButtonChecked(hwndDlg, IDC_GROUPCHAT_CSS);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_CSS_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_CSS_BROWSE), bChecked);
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_GROUPCHAT_CSS_BROWSE:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_GROUPCHAT_CSS_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Style Sheet (*.css)\0*.css\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "css";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg, IDC_GROUPCHAT_CSS_FILENAME, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_GROUPCHAT_TEMPLATES_BROWSE:
				{
					OPENFILENAMEA ofn={0};
					GetDlgItemTextA(hwndDlg, IDC_GROUPCHAT_TEMPLATES_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Templates (*.ivt)\0*.ivt\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "ivt";
					if(GetOpenFileNameA(&ofn)) {
						SetDlgItemTextA(hwndDlg, IDC_GROUPCHAT_TEMPLATES_FILENAME, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			}
		}
		break;
	case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->code) {
			case PSN_APPLY:
				GetDlgItemTextA(hwndDlg, IDC_GROUPCHAT_CSS_FILENAME, path, sizeof(path));
				Options::setGroupChatCSSFile(path);
				i = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_GROUPCHAT_TEMPLATES)) {
//					i |= Options::TEMPLATES_ENABLED;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_GROUPCHAT_CSS)) {
		//			i |= Options::CSS_ENABLED;
				}
				Options::setGroupChatFlags(i);
				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}

bool Options::isInited = false;
bool Options::bMathModule = false;
bool  Options::bSmileyAdd = false;
int  Options::avatarServiceFlags = 0;
int Options::generalFlags;

int Options::srmmMode;
int Options::srmmFlags;
char *Options::bkgFilename = NULL;

int Options::groupChatFlags;
char *Options::groupChatCSSFilename = NULL;
char *Options::groupChatTemplatesFilename = NULL;

int Options::historyFlags;
char *Options::historyCSSFilename = NULL;
char *Options::historyCSSFilenameRTL = NULL;
char *Options::historyTemplatesFilename = NULL;
char *Options::historyTemplatesFilenameRTL = NULL;

ProtocolSettings *Options::protocolList = NULL;

ProtocolSettings::ProtocolSettings(const char *protocolName) {
	this->protocolName = Utils::dupString(protocolName);
	next = NULL;
	enable = false;
	srmmCssFilename = Utils::dupString("");
	srmmCssFilenameRtl = Utils::dupString("");
	srmmTemplateFilename = Utils::dupString("");
	srmmTemplateFilenameRtl = Utils::dupString("");

	srmmCssFilenameTemp = Utils::dupString("");
	srmmCssFilenameRtlTemp = Utils::dupString("");
	srmmTemplateFilenameTemp = Utils::dupString("");
	srmmTemplateFilenameRtlTemp = Utils::dupString("");
}

ProtocolSettings::~ProtocolSettings() {
	if (srmmCssFilename != NULL) {
		delete srmmCssFilename;
	}
	if (srmmCssFilenameRtl != NULL) {
		delete srmmCssFilenameRtl;
	}
	if (srmmCssFilenameTemp != NULL) {
		delete srmmCssFilenameTemp;
	}
	if (srmmCssFilenameRtlTemp != NULL) {
		delete srmmCssFilenameRtlTemp;
	}
	if (srmmTemplateFilename != NULL) {
		delete srmmTemplateFilename;
	}
	if (srmmTemplateFilenameRtl != NULL) {
		delete srmmTemplateFilenameRtl;
	}
	if (srmmTemplateFilenameTemp != NULL) {
		delete srmmTemplateFilenameTemp;
	}
	if (srmmTemplateFilenameRtlTemp != NULL) {
		delete srmmTemplateFilenameRtlTemp;
	}
}

void ProtocolSettings::copyToTemp() {
	setSRMMCssFilenameTemp(getSRMMCssFilename());
	setSRMMCssFilenameRtlTemp(getSRMMCssFilenameRtl());
	setSRMMTemplateFilenameTemp(getSRMMTemplateFilename());
	setSRMMTemplateFilenameRtlTemp(getSRMMTemplateFilenameRtl());
	setEnableTemp(isEnable());
}

void ProtocolSettings::copyFromTemp() {
	setSRMMCssFilename(getSRMMCssFilenameTemp());
	setSRMMCssFilenameRtl(getSRMMCssFilenameRtlTemp());
	setSRMMTemplateFilename(getSRMMTemplateFilenameTemp());
	setSRMMTemplateFilenameRtl(getSRMMTemplateFilenameRtlTemp());
	setEnable(isEnableTemp());
}

void ProtocolSettings::setNext(ProtocolSettings *next) {
	this->next = next;
}

void ProtocolSettings::setSRMMCssFilename(const char *filename) {
	if (srmmCssFilename != NULL) {
		delete srmmCssFilename;
	}
	srmmCssFilename = Utils::dupString(filename);
}

void ProtocolSettings::setSRMMCssFilenameRtl(const char *filename) {
	if (srmmCssFilenameRtl != NULL) {
		delete srmmCssFilenameRtl;
	}
	srmmCssFilenameRtl = Utils::dupString(filename);
}

void ProtocolSettings::setSRMMCssFilenameTemp(const char *filename) {
	if (srmmCssFilenameTemp != NULL) {
		delete srmmCssFilenameTemp;
	}
	srmmCssFilenameTemp = Utils::dupString(filename);
}

void ProtocolSettings::setSRMMCssFilenameRtlTemp(const char *filename) {
	if (srmmCssFilenameRtlTemp != NULL) {
		delete srmmCssFilenameRtlTemp;
	}
	srmmCssFilenameRtlTemp = Utils::dupString(filename);
}

void ProtocolSettings::setSRMMTemplateFilename(const char *filename) {
	if (srmmTemplateFilename != NULL) {
		delete srmmTemplateFilename;
	}
	srmmTemplateFilename = Utils::dupString(filename);
	TemplateMap::loadTemplates(getSRMMTemplateFilename(), getSRMMTemplateFilename());
}

void ProtocolSettings::setSRMMTemplateFilenameRtl(const char *filename) {
	if (srmmTemplateFilenameRtl != NULL) {
		delete srmmTemplateFilenameRtl;
	}
	srmmTemplateFilenameRtl = Utils::dupString(filename);
	TemplateMap::loadTemplates(getSRMMTemplateFilenameRtl(), getSRMMTemplateFilenameRtl());
}

void ProtocolSettings::setSRMMTemplateFilenameTemp(const char *filename) {
	if (srmmTemplateFilenameTemp != NULL) {
		delete srmmTemplateFilenameTemp;
	}
	srmmTemplateFilenameTemp = Utils::dupString(filename);
}

void ProtocolSettings::setSRMMTemplateFilenameRtlTemp(const char *filename) {
	if (srmmTemplateFilenameRtlTemp != NULL) {
		delete srmmTemplateFilenameRtlTemp;
	}
	srmmTemplateFilenameRtlTemp = Utils::dupString(filename);
}

const char *ProtocolSettings::getProtocolName() {
	return protocolName;
}

ProtocolSettings * ProtocolSettings::getNext() {
	return next;
}

const char *ProtocolSettings::getSRMMCssFilename() {
	return srmmCssFilename;
}

const char *ProtocolSettings::getSRMMCssFilenameRtl() {
	return srmmCssFilenameRtl;
}

const char *ProtocolSettings::getSRMMCssFilenameTemp() {
	return srmmCssFilenameTemp;
}

const char *ProtocolSettings::getSRMMCssFilenameRtlTemp() {
	return srmmCssFilenameRtlTemp;
}

const char *ProtocolSettings::getSRMMTemplateFilename() {
	return srmmTemplateFilename;
}

const char *ProtocolSettings::getSRMMTemplateFilenameRtl() {
	return srmmTemplateFilenameRtl;
}

const char *ProtocolSettings::getSRMMTemplateFilenameTemp() {
	return srmmTemplateFilenameTemp;
}

const char *ProtocolSettings::getSRMMTemplateFilenameRtlTemp() {
	return srmmTemplateFilenameRtlTemp;
}


void ProtocolSettings::setEnable(bool enable) {
	this->enable = enable;
}

bool ProtocolSettings::isEnable() {
	return enable;
}

void ProtocolSettings::setEnableTemp(bool enable) {
	this->enableTemp = enable;
}

bool ProtocolSettings::isEnableTemp() {
	return enableTemp;
}


void Options::init() {
	if (isInited) return;
	isInited = true;
	DBVARIANT dbv;

	/* TODO: move to buildProtocolList method */
	int protoCount;
	PROTOCOLDESCRIPTOR **pProtos;
	ProtocolSettings *lastProto = NULL;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&protoCount, (LPARAM)&pProtos);
	for (int i = 0; i < protoCount+1; i++) {
		ProtocolSettings *proto;
		char tmpPath[MAX_PATH];
		char dbsName[256];
		if (i==0) {
			proto = new ProtocolSettings("_default_");
			proto->setEnable(true);
		} else if ((pProtos[i-1]->type == PROTOTYPE_PROTOCOL) && strcmp(pProtos[i-1]->szName,"MetaContacts")) {
			proto = new ProtocolSettings(pProtos[i-1]->szName);
			sprintf(dbsName, "%s.%s", proto->getProtocolName(), DBS_SRMM_ENABLE);
			proto->setEnable(DBGetContactSettingByte(NULL, ieviewModuleName, dbsName, FALSE));
		} else {
			continue;
		}
		sprintf(dbsName, "%s.%s", proto->getProtocolName(), DBS_SRMM_TEMPLATE);
		if (!DBGetContactSetting(NULL,  ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)) {
				CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
			}
			proto->setSRMMTemplateFilename(tmpPath);
			DBFreeVariant(&dbv);
		}
		sprintf(dbsName, "%s.%s", proto->getProtocolName(), DBS_SRMM_TEMPLATE_RTL);
		if (!DBGetContactSetting(NULL,  ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)) {
				CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
			}
			proto->setSRMMTemplateFilenameRtl(tmpPath);
			DBFreeVariant(&dbv);
		}
		sprintf(dbsName, "%s.%s", proto->getProtocolName(), DBS_SRMM_CSS);
		if (!DBGetContactSetting(NULL,  ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			if (ServiceExists(MS_UTILS_PATHTOABSOLUTE) && strncmp(tmpPath, "http://", 7)) {
				CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
			}
			proto->setSRMMCssFilename(tmpPath);
			DBFreeVariant(&dbv);
		}
		sprintf(dbsName, "%s.%s", proto->getProtocolName(), DBS_SRMM_CSS_RTL);
		if (!DBGetContactSetting(NULL,  ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
	    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE) && strncmp(tmpPath, "http://", 7)) {
				CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
			}
			proto->setSRMMCssFilenameRtl(tmpPath);
			DBFreeVariant(&dbv);
		}
		proto->copyToTemp();
		if (lastProto != NULL) {
			lastProto->setNext(proto);
		} else {
			protocolList = proto;
		}
		lastProto = proto;
	}

	bMathModule = (bool) ServiceExists(MTH_GET_GIF_UNICODE);
	bSmileyAdd = (bool) ServiceExists(MS_SMILEYADD_BATCHPARSE);
	avatarServiceFlags = 0;
	if (ServiceExists(MS_AV_GETAVATARBITMAP)) {
		avatarServiceFlags = AVATARSERVICE_PRESENT;
	}
	generalFlags = DBGetContactSettingDword(NULL, ieviewModuleName, DBS_BASICFLAGS, 0);
	srmmMode = DBGetContactSettingByte(NULL, ieviewModuleName, DBS_SRMMMODE, 0);
	srmmFlags = DBGetContactSettingDword(NULL, ieviewModuleName, DBS_SRMMFLAGS, 0);
	if (!DBGetContactSetting(NULL,  ieviewModuleName, DBS_BACKGROUNDIMAGEFILE, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE) && strncmp(tmpPath, "http://", 7)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}
		bkgFilename = new char[strlen(tmpPath)+1];
		strcpy(bkgFilename, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        bkgFilename = new char[1];
        strcpy(bkgFilename, "");
	}
	groupChatFlags = DBGetContactSettingDword(NULL, ieviewModuleName, DBS_GROUPCHATFLAGS, FALSE);
	if (!DBGetContactSetting(NULL,  ieviewModuleName, DBS_GROUPCHATCSSFILE, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE) && strncmp(tmpPath, "http://", 7)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}
		groupChatCSSFilename = new char[strlen(tmpPath)+1];
		strcpy(groupChatCSSFilename, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        groupChatCSSFilename = new char[1];
        strcpy(groupChatCSSFilename, "");
	}
	if (!DBGetContactSetting(NULL,  ieviewModuleName, DBS_GROUPCHATTEMPLATESFILE, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}
		groupChatTemplatesFilename = new char[strlen(tmpPath)+1];
		strcpy(groupChatTemplatesFilename, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        groupChatTemplatesFilename = new char[1];
        strcpy(groupChatTemplatesFilename, "");
	}
	historyFlags = DBGetContactSettingDword(NULL, ieviewModuleName, DBS_HISTORYFLAGS, FALSE);
	if (!DBGetContactSetting(NULL,  ieviewModuleName, DBS_HISTORYCSSFILE, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE) && strncmp(tmpPath, "http://", 7)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}
		historyCSSFilename = new char[strlen(tmpPath)+1];
		strcpy(historyCSSFilename, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        historyCSSFilename = new char[1];
        strcpy(historyCSSFilename, "");
	}
	if (!DBGetContactSetting(NULL,  ieviewModuleName, DBS_HISTORYCSSFILE_RTL, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE) && strncmp(tmpPath, "http://", 7)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}
		historyCSSFilenameRTL = new char[strlen(tmpPath)+1];
		strcpy(historyCSSFilenameRTL, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        historyCSSFilenameRTL = new char[1];
        strcpy(historyCSSFilenameRTL, "");
	}

	TemplateMap::loadTemplates("groupchat_default", groupChatTemplatesFilename);
//	mathModuleFlags = ServiceExists(MTH_GET_HTML_SOURCE) ? GENERAL_ENABLE_MATHMODULE : 0;
}

void Options::setBkgImageFile(const char *filename) {
	if (bkgFilename != NULL) {
		delete [] bkgFilename;
	}
	bkgFilename = new char[strlen(filename)+1];
	strcpy(bkgFilename, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE)&& strncmp(tmpPath, "http://", 7)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_BACKGROUNDIMAGEFILE, tmpPath);
}

const char * Options::getBkgImageFile() {
	return bkgFilename;
}

void Options::setGeneralFlags(int flags) {
	generalFlags = flags;
	DBWriteContactSettingDword(NULL, ieviewModuleName, DBS_BASICFLAGS, (DWORD) flags);
}

int	Options::getGeneralFlags() {
	return generalFlags;
}

void Options::setSRMMMode(int mode) {
	srmmMode = mode;
	DBWriteContactSettingByte(NULL, ieviewModuleName, DBS_SRMMMODE, (DWORD) mode);
}

int	Options::getSRMMMode() {
	return srmmMode;
}

void Options::setSRMMFlags(int flags) {
	srmmFlags = flags;
	DBWriteContactSettingDword(NULL, ieviewModuleName, DBS_SRMMFLAGS, (DWORD) flags);
}

int	Options::getSRMMFlags() {
	return srmmFlags;
}

void Options::setGroupChatCSSFile(const char *filename) {
	if (groupChatCSSFilename != NULL) {
		delete [] groupChatCSSFilename;
	}
	groupChatCSSFilename = new char[strlen(filename)+1];
	strcpy(groupChatCSSFilename, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE) && strncmp(tmpPath, "http://", 7)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_GROUPCHATCSSFILE, tmpPath);
}

const char *Options::getGroupChatCSSFile() {
	return groupChatCSSFilename;
}

void Options::setGroupChatFlags(int flags) {
	groupChatFlags = flags;
	DBWriteContactSettingDword(NULL, ieviewModuleName, DBS_GROUPCHATFLAGS, (DWORD) flags);
}

int	Options::getGroupChatFlags() {
	return groupChatFlags;
}

void Options::setGroupChatTemplatesFile(const char *filename) {
	if (groupChatTemplatesFilename != NULL) {
		delete [] groupChatTemplatesFilename;
	}
	groupChatTemplatesFilename = new char[strlen(filename)+1];
	strcpy(groupChatTemplatesFilename, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_GROUPCHATTEMPLATESFILE, tmpPath);
	TemplateMap::loadTemplates("groupchat", groupChatTemplatesFilename);
}

const char *Options::getGroupChatTemplatesFile() {
	return groupChatTemplatesFilename;
}


void Options::setHistoryCSSFile(const char *filename) {
	if (historyCSSFilename != NULL) {
		delete [] historyCSSFilename;
	}
	historyCSSFilename = new char[strlen(filename)+1];
	strcpy(historyCSSFilename, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE) && strncmp(tmpPath, "http://", 7)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_HISTORYCSSFILE, tmpPath);
}

const char *Options::getHistoryCSSFile() {
	return historyCSSFilename;
}

void Options::setHistoryCSSFileRTL(const char *filename) {
	if (historyCSSFilenameRTL != NULL) {
		delete [] historyCSSFilenameRTL;
	}
	historyCSSFilenameRTL = new char[strlen(filename)+1];
	strcpy(historyCSSFilenameRTL, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE) && strncmp(tmpPath, "http://", 7)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_HISTORYCSSFILE_RTL, tmpPath);
}

const char *Options::getHistoryCSSFileRTL() {
	return historyCSSFilenameRTL;
}

void Options::setHistoryTemplatesFile(const char *filename) {
	if (historyTemplatesFilename != NULL) {
		delete [] historyTemplatesFilename;
	}
	historyTemplatesFilename = new char[strlen(filename)+1];
	strcpy(historyTemplatesFilename, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_HISTORYTEMPLATESFILE, tmpPath);

	TemplateMap::loadTemplates("history_default", historyTemplatesFilename);
}

void Options::setHistoryTemplatesFileRTL(const char *filename) {
	if (historyTemplatesFilenameRTL != NULL) {
		delete [] historyTemplatesFilenameRTL;
	}
	historyTemplatesFilenameRTL = new char[strlen(filename)+1];
	strcpy(historyTemplatesFilenameRTL, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_HISTORYTEMPLATESFILE_RTL, tmpPath);

	TemplateMap::loadTemplates("history_default_rtl", historyTemplatesFilenameRTL);
}

const char *Options::getHistoryTemplatesFile() {
	return historyTemplatesFilename;
}

const char *Options::getHistoryTemplatesFileRTL() {
	return historyTemplatesFilenameRTL;
}

void Options::setHistoryFlags(int flags) {
	historyFlags = flags;
	DBWriteContactSettingDword(NULL, ieviewModuleName, DBS_HISTORYFLAGS, (DWORD) flags);
}

int	Options::getHistoryFlags() {
	return historyFlags;
}


bool Options::isMathModule() {
	return bMathModule;
}

bool Options::isSmileyAdd() {
	return bSmileyAdd;
}

int Options::getAvatarServiceFlags() {
	return avatarServiceFlags;
}

ProtocolSettings * Options::getProtocolSettings() {
	return protocolList;
}

ProtocolSettings * Options::getProtocolSettings(const char *protocolName) {
	for (ProtocolSettings *proto=protocolList;proto!=NULL;proto=proto->getNext()) {
		if (!strcmpi(proto->getProtocolName(), protocolName)) {
			return proto;
		}
	}
	return NULL;
}

void Options::resetProtocolSettings() {
	for (ProtocolSettings *proto=Options::getProtocolSettings();proto!=NULL;proto=proto->getNext()) {
		proto->copyToTemp();
	}
}

void Options::saveProtocolSettings() {
	ProtocolSettings *proto;
	int i;
	for (i=0,proto=Options::getProtocolSettings();proto!=NULL;proto=proto->getNext(),i++) {
		char dbsName[256];
		char tmpPath[MAX_PATH];
		proto->copyFromTemp();
		sprintf(dbsName, "%s.%s", proto->getProtocolName(), DBS_SRMM_ENABLE);
		DBWriteContactSettingByte(NULL, ieviewModuleName, dbsName, proto->isEnable());
		sprintf(dbsName, "%s.%s", proto->getProtocolName(), DBS_SRMM_TEMPLATE);
		strcpy (tmpPath, proto->getSRMMTemplateFilename());
		if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
			CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)proto->getSRMMTemplateFilename(), (LPARAM)tmpPath);
		}
		DBWriteContactSettingString(NULL, ieviewModuleName, dbsName, tmpPath);
		sprintf(dbsName, "%s.%s", proto->getProtocolName(), DBS_SRMM_TEMPLATE_RTL);
		strcpy (tmpPath, proto->getSRMMTemplateFilenameRtl());
		if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
			CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)proto->getSRMMTemplateFilenameRtl(), (LPARAM)tmpPath);
		}
		DBWriteContactSettingString(NULL, ieviewModuleName, dbsName, tmpPath);
		sprintf(dbsName, "%s.%s", proto->getProtocolName(), DBS_SRMM_CSS);
		strcpy (tmpPath, proto->getSRMMCssFilename());
		if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
			CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)proto->getSRMMCssFilename(), (LPARAM)tmpPath);
		}
		DBWriteContactSettingString(NULL, ieviewModuleName, dbsName, tmpPath);
		sprintf(dbsName, "%s.%s", proto->getProtocolName(), DBS_SRMM_CSS_RTL);
		strcpy (tmpPath, proto->getSRMMCssFilenameRtl());
		if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
			CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)proto->getSRMMCssFilenameRtl(), (LPARAM)tmpPath);
		}
		DBWriteContactSettingString(NULL, ieviewModuleName, dbsName, tmpPath);
	//	sprintf(dbsName, "%s", proto->getProtocolName());
//		sprintf(dbsName, "%s.RTL", proto->getProtocolName());
	}
}
