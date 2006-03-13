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
#include "m_MathModule.h"
#include "m_avatars.h"

#define UM_CHECKSTATECHANGE (WM_USER+100)
HANDLE hHookOptionsChanged;
static BOOL CALLBACK IEViewOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewGeneralOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
//static BOOL CALLBACK IEViewEmoticonsOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewTemplatesOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewGroupChatsOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewHistoryOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND hwndCurrentTab, hwndPages[4];

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

static void buildProtoList() {
	int protoCount;
	PROTOCOLDESCRIPTOR **pProtos;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&protoCount, (LPARAM)&pProtos);
	protoNum = protoCount+1;
	protoNames = new char[protoCount+1][128];
	protoFilenames = new char[protoCount+1][MAX_PATH];
	for (int i = 0; i < protoNum; i++) {
		protoNames[i][0] = '\0';
		if (i==0) {
			strcpy(protoNames[i], "");
		} else if ((pProtos[i-1]->type == PROTOTYPE_PROTOCOL) && strcmp(pProtos[i-1]->szName,"MetaContacts")) {
			strcpy(protoNames[i], pProtos[i-1]->szName);
		} else {
			continue;
		}
		char * path = (char *) Options::getSmileyFile(protoNames[i]);
		if (path != NULL) {
			strcpy (protoFilenames[i], path);
		} else {
			strcpy (protoFilenames[i], "");
		}
	}
}

static void updateSmileyInfo(HWND hwndDlg, int proto) {
	HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
	SetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, protoFilenames[proto]);
	SmileyMap *map = SmileyMap::getLibraryInfo(protoFilenames[proto]);
	TreeView_SetCheckState(hProtoList, TreeView_GetSelection(hProtoList), map!=NULL);
	if (map != NULL) {
		SetDlgItemText(hwndDlg, IDC_LIBNAME, map->getDescription());
		SetDlgItemText(hwndDlg, IDC_LIBAUTHOR, map->getAuthor());
		SetDlgItemText(hwndDlg, IDC_LIBVERSION, map->getVersion());
		delete map;
	} else {
		SetDlgItemText(hwndDlg, IDC_LIBNAME, Translate("Not loaded"));
		SetDlgItemText(hwndDlg, IDC_LIBAUTHOR, "");
		SetDlgItemText(hwndDlg, IDC_LIBVERSION, "");
	}
	currentProtoItem = proto;
}


static void refreshProtoList(HWND hwndDlg, BOOL protoSmileys) {
    HTREEITEM hItem = NULL;
	HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
	TreeView_DeleteAllItems(hProtoList);
	updateSmileyInfo(hwndDlg, 0);
	for (int i = 0; i < protoNum; i++) {
		if (i==0 || (protoSmileys && protoNames[i][0]!= '\0')) {
			char protoName[128];
			TVINSERTSTRUCT tvi = {0};
			tvi.hParent = TVI_ROOT;
			tvi.hInsertAfter = TVI_LAST;
			tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_STATE | TVIF_SELECTEDIMAGE;
			tvi.item.stateMask = TVIS_SELECTED | TVIS_STATEIMAGEMASK;
			if (i==0) {
				strcpy(protoName, Translate("Standard"));
			} else {
				CallProtoService(protoNames[i], PS_GETNAME, sizeof(protoName), (LPARAM)protoName);
			}
			strcat(protoName, " ");
			strcat(protoName, Translate("smileys"));
			tvi.item.pszText = protoName;
			tvi.item.iImage = i;
			tvi.item.iSelectedImage = i;
			tvi.item.state = INDEXTOSTATEIMAGEMASK(strlen(protoFilenames[i]) > 0 ? 2 : 1);
			if (i==0) {
                hItem = TreeView_InsertItem(hProtoList, &tvi);
			} else {
				TreeView_InsertItem(hProtoList, &tvi);
			}

		}
	}
	TreeView_SelectItem(hProtoList, hItem);
}

static int getSelProto(HWND hLstView, HTREEITEM hItem) {
	TVITEM tvi = {0};
	tvi.mask = TVIF_IMAGE;
	tvi.hItem = hItem == NULL ? TreeView_GetSelection(hLstView) : hItem;
	TreeView_GetItem(hLstView, &tvi);
	return tvi.iImage;
}

static bool browseSmileys(HWND hwndDlg) {
	char path[MAX_PATH];
	OPENFILENAME ofn={0};
	GetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, path, sizeof(path));
	ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
	ofn.hwndOwner = hwndDlg;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "Smiley Library (*.asl)\0*.asl\0All Files\0*.*\0\0";
	ofn.lpstrFile = path;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.nMaxFile = sizeof(path);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "asl";
	if(GetOpenFileName(&ofn)) {
        strcpy(protoFilenames[currentProtoItem], path);
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
			hwndPages[1] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_SRMM_OPTIONS), hwndDlg, IEViewTemplatesOptDlgProc, (LPARAM) NULL);
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

static BOOL CALLBACK IEViewTemplatesOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int i;
	BOOL bChecked;
	char path[MAX_PATH];
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			bChecked = FALSE;
			if (Options::getSRMMFlags() & Options::TEMPLATES_ENABLED) {
			    bChecked = TRUE;
				CheckDlgButton(hwndDlg, IDC_TEMPLATES, TRUE);
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

			if (Options::getSRMMTemplatesFile() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, Options::getSRMMTemplatesFile());
			}
			if (Options::getSRMMTemplatesFileRTL() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, Options::getSRMMTemplatesFileRTL());
			}

			bChecked = !IsDlgButtonChecked(hwndDlg, IDC_TEMPLATES);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS), bChecked);
			if (Options::getSRMMFlags() & Options::CSS_ENABLED) {
				CheckDlgButton(hwndDlg, IDC_EXTERNALCSS, TRUE);
			} else {
                bChecked = FALSE;
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS_RTL), bChecked);
			if (Options::getSRMMCSSFile() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, Options::getSRMMCSSFile());
			}
			if (Options::getSRMMCSSFileRTL() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, Options::getSRMMCSSFileRTL());
			}

			bChecked = !IsDlgButtonChecked(hwndDlg, IDC_TEMPLATES) && !IsDlgButtonChecked(hwndDlg, IDC_EXTERNALCSS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE), bChecked);
			if (Options::getSRMMFlags() & Options::IMAGE_ENABLED) {
				CheckDlgButton(hwndDlg, IDC_BACKGROUND_IMAGE, TRUE);
			} else {
				bChecked = FALSE;
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
			if (Options::getSRMMFlags() & Options::IMAGE_SCROLL) {
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
				if (IsDlgButtonChecked(hwndDlg, IDC_TEMPLATES)) {
					i |= Options::TEMPLATES_ENABLED;
				}
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
					i |= Options::IMAGE_ENABLED;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE)) {
					i |= Options::IMAGE_SCROLL;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_EXTERNALCSS)) {
					i |= Options::CSS_ENABLED;
				}
				Options::setSRMMFlags(i);
				GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path, sizeof(path));
				Options::setSRMMTemplatesFile(path);
				GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME_RTL, path, sizeof(path));
				Options::setSRMMTemplatesFileRTL(path);
				GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, sizeof(path));
				Options::setBkgImageFile(path);
				GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, sizeof(path));
				Options::setSRMMCSSFile(path);
				GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, path, sizeof(path));
				Options::setSRMMCSSFileRTL(path);
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
	BOOL bChecked;
	char path[MAX_PATH];
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			bChecked = FALSE;
			if (Options::getHistoryFlags() & Options::TEMPLATES_ENABLED) {
			    bChecked = TRUE;
				CheckDlgButton(hwndDlg, IDC_TEMPLATES, TRUE);
			}
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

			bChecked = !IsDlgButtonChecked(hwndDlg, IDC_TEMPLATES);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS), bChecked);
			if (Options::getHistoryFlags() & Options::CSS_ENABLED) {
				CheckDlgButton(hwndDlg, IDC_EXTERNALCSS, TRUE);
			} else {
                bChecked = FALSE;
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
			if (Options::getHistoryFlags() & Options::IMAGE_ENABLED) {
				CheckDlgButton(hwndDlg, IDC_BACKGROUND_IMAGE, TRUE);
			} else {
				bChecked = FALSE;
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
			if (Options::getHistoryFlags() & Options::IMAGE_SCROLL) {
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
				if (IsDlgButtonChecked(hwndDlg, IDC_TEMPLATES)) {
					i |= Options::TEMPLATES_ENABLED;
				}
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
					i |= Options::IMAGE_ENABLED;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE)) {
					i |= Options::IMAGE_SCROLL;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_EXTERNALCSS)) {
					i |= Options::CSS_ENABLED;
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
			if (Options::getGroupChatFlags() & Options::TEMPLATES_ENABLED) {
				CheckDlgButton(hwndDlg, IDC_GROUPCHAT_TEMPLATES, TRUE);
			    bChecked = TRUE;
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_TEMPLATES_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_TEMPLATES_BROWSE), bChecked);
			if (Options::getGroupChatTemplatesFile() != NULL) {
                SetDlgItemTextA(hwndDlg, IDC_GROUPCHAT_TEMPLATES_FILENAME, Options::getGroupChatTemplatesFile());
			}
			bChecked = !IsDlgButtonChecked(hwndDlg, IDC_GROUPCHAT_TEMPLATES);
			EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT_CSS), bChecked);
			if (Options::getGroupChatFlags() & Options::CSS_ENABLED) {
				CheckDlgButton(hwndDlg, IDC_GROUPCHAT_CSS, TRUE);
			} else {
                bChecked = FALSE;
			}
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
					i |= Options::TEMPLATES_ENABLED;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_GROUPCHAT_CSS)) {
					i |= Options::CSS_ENABLED;
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
int Options::srmmFlags;
char *Options::srmmTemplatesFilename = NULL;
char *Options::srmmTemplatesFilenameRTL = NULL;
char *Options::bkgFilename = NULL;
char *Options::srmmCSSFilename = NULL;
char *Options::srmmCSSFilenameRTL = NULL;

int Options::groupChatFlags;
char *Options::groupChatCSSFilename = NULL;
char *Options::groupChatTemplatesFilename = NULL;

int Options::historyFlags;
char *Options::historyCSSFilename = NULL;
char *Options::historyCSSFilenameRTL = NULL;
char *Options::historyTemplatesFilename = NULL;
char *Options::historyTemplatesFilenameRTL = NULL;


void Options::init() {
	if (isInited) return;
	isInited = true;
	DBVARIANT dbv;
	bMathModule = (bool) ServiceExists(MTH_GET_GIF_UNICODE);
	bSmileyAdd = (bool) ServiceExists(MS_SMILEYADD_BATCHPARSE);
	avatarServiceFlags = 0;
	if (ServiceExists(MS_AV_GETAVATARBITMAP)) {
		avatarServiceFlags = AVATARSERVICE_PRESENT;
	}
	generalFlags = DBGetContactSettingDword(NULL, ieviewModuleName, DBS_BASICFLAGS, 0);
	srmmFlags = DBGetContactSettingDword(NULL, ieviewModuleName, DBS_SRMMFLAGS, FALSE);
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
	if (!DBGetContactSetting(NULL,  ieviewModuleName, DBS_EXTERNALCSSFILE, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE) && strncmp(tmpPath, "http://", 7)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}
		srmmCSSFilename = new char[strlen(tmpPath)+1];
		strcpy(srmmCSSFilename, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        srmmCSSFilename = new char[1];
        strcpy(srmmCSSFilename, "");
	}
	if (!DBGetContactSetting(NULL,  ieviewModuleName, DBS_EXTERNALCSSFILE_RTL, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE) && strncmp(tmpPath, "http://", 7)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}
		srmmCSSFilenameRTL = new char[strlen(tmpPath)+1];
		strcpy(srmmCSSFilenameRTL, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        srmmCSSFilenameRTL = new char[1];
        strcpy(srmmCSSFilenameRTL, "");
	}
	if (!DBGetContactSetting(NULL,  ieviewModuleName, DBS_TEMPLATESFILE, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}
		srmmTemplatesFilename = new char[strlen(tmpPath)+1];
		strcpy(srmmTemplatesFilename, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        srmmTemplatesFilename = new char[1];
        strcpy(srmmTemplatesFilename, "");
	}
	if (!DBGetContactSetting(NULL,  ieviewModuleName, DBS_TEMPLATESFILE_RTL, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}
		srmmTemplatesFilenameRTL = new char[strlen(tmpPath)+1];
		strcpy(srmmTemplatesFilenameRTL, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        srmmTemplatesFilenameRTL = new char[1];
        strcpy(srmmTemplatesFilenameRTL, "");
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

	TemplateMap::loadTemplates("srmm_default", srmmTemplatesFilename);
	TemplateMap::loadTemplates("srmm_default_rtl", srmmTemplatesFilenameRTL);
	TemplateMap::loadTemplates("groupchat_default", groupChatTemplatesFilename);
//	TemplateMap::loadTemplates("groupchat", groupChatTemplatesFilename);
//	TemplateMap::loadTemplates("history_default", historyTemplatesFilename);
//	TemplateMap::loadTemplates("history_default_rtl", historyTemplatesFilenameRTL);
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

void Options::setSRMMCSSFile(const char *filename) {
	if (srmmCSSFilename != NULL) {
		delete [] srmmCSSFilename;
	}
	srmmCSSFilename = new char[strlen(filename)+1];
	strcpy(srmmCSSFilename, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE) && strncmp(tmpPath, "http://", 7)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_EXTERNALCSSFILE, tmpPath);
}

const char *Options::getSRMMCSSFile() {
	return srmmCSSFilename;
}

void Options::setSRMMCSSFileRTL(const char *filename) {
	if (srmmCSSFilenameRTL != NULL) {
		delete [] srmmCSSFilenameRTL;
	}
	srmmCSSFilenameRTL = new char[strlen(filename)+1];
	strcpy(srmmCSSFilenameRTL, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE) && strncmp(tmpPath, "http://", 7)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_EXTERNALCSSFILE_RTL, tmpPath);
}

const char *Options::getSRMMCSSFileRTL() {
	return srmmCSSFilenameRTL;
}

void Options::setSRMMTemplatesFile(const char *filename) {
	if (srmmTemplatesFilename != NULL) {
		delete [] srmmTemplatesFilename;
	}
	srmmTemplatesFilename = new char[strlen(filename)+1];
	strcpy(srmmTemplatesFilename, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_TEMPLATESFILE, tmpPath);

	TemplateMap::loadTemplates("srmm_default", srmmTemplatesFilename);
}

void Options::setSRMMTemplatesFileRTL(const char *filename) {
	if (srmmTemplatesFilenameRTL != NULL) {
		delete [] srmmTemplatesFilenameRTL;
	}
	srmmTemplatesFilenameRTL = new char[strlen(filename)+1];
	strcpy(srmmTemplatesFilenameRTL, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}
	DBWriteContactSettingString(NULL, ieviewModuleName, DBS_TEMPLATESFILE_RTL, tmpPath);

	TemplateMap::loadTemplates("srmm_default_rtl", srmmTemplatesFilenameRTL);
}

const char *Options::getSRMMTemplatesFile() {
	return srmmTemplatesFilename;
}

const char *Options::getSRMMTemplatesFileRTL() {
	return srmmTemplatesFilenameRTL;
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
