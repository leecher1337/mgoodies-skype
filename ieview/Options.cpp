#include "Options.h"
#include "resource.h"
#include "Smiley.h"
#include "Template.h"

static BOOL CALLBACK IEViewOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewBasicOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewEmoticonsOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK IEViewTemplatesOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static int protoNum;
static char (*protoNames)[128];
static char (*protoFilenames)[MAX_PATH];
static HWND hwndBasic, hwndEmoticons, hwndTemplates, hwndCurrentTab;
static int lastProtoItem;
static HICON smileyIcon;

int IEViewOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = hInstance;
	odp.pszGroup = Translate("Message Sessions");
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszTitle = Translate("IEView plugin");
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
			tci.pszText = Translate("Basic");
			TabCtrl_InsertItem(tc, 0, &tci);
			tci.pszText = Translate("Emoticons");
			TabCtrl_InsertItem(tc, 1, &tci);
			tci.pszText = Translate("Templates");
			TabCtrl_InsertItem(tc, 2, &tci);
			
			hwndBasic = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_BASIC_OPTIONS), hwndDlg, IEViewBasicOptDlgProc, (LPARAM) NULL);
			SetWindowPos(hwndBasic, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			ShowWindow(hwndBasic, SW_SHOW);
			hwndEmoticons = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_EMOTICONS_OPTIONS), hwndDlg, IEViewEmoticonsOptDlgProc, (LPARAM) NULL);
			SetWindowPos(hwndEmoticons, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			hwndTemplates = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_TEMPLATES_OPTIONS), hwndDlg, IEViewTemplatesOptDlgProc, (LPARAM) NULL);
			SetWindowPos(hwndTemplates, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			hwndCurrentTab = hwndBasic;
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
						HWND hwnd;
						switch (TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TABS))) {
						default:
						case 0:
							hwnd = hwndBasic;
							break;
						case 1:
							hwnd = hwndEmoticons;
							break;
						case 2:
							hwnd = hwndTemplates;
							break;
						}
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
				SendMessage(hwndBasic, WM_NOTIFY, wParam, lParam);
				SendMessage(hwndEmoticons, WM_NOTIFY, wParam, lParam);
				SendMessage(hwndTemplates, WM_NOTIFY, wParam, lParam);
				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}

static BOOL CALLBACK IEViewBasicOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int i;
	BOOL bChecked;
	char path[MAX_PATH];
	switch (msg) {
	case WM_INITDIALOG:
		{
			char *path;
			TranslateDialogDefault(hwndDlg);
			bChecked = FALSE;
			if (Options::getBkgImageFlags() & Options::BKGIMAGE_ENABLED) {
			    bChecked = TRUE;
				CheckDlgButton(hwndDlg, IDC_BACKGROUND_IMAGE, TRUE);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
			if (Options::getBkgImageFlags() & Options::BKGIMAGE_SCROLL) {
				CheckDlgButton(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE, TRUE);
			}
			path = (char *)Options::getBkgImageFile();
			if (path != NULL) {
                SetDlgItemText(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path);
			}
			bChecked = FALSE;
			if (Options::getExternalCSSFlags() & Options::EXTERNALCSS_ENABLED) {
			    bChecked = TRUE;
				CheckDlgButton(hwndDlg, IDC_EXTERNALCSS, TRUE);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS), bChecked);
			path = (char *)Options::getExternalCSSFile();
			if (path != NULL) {
                SetDlgItemText(hwndDlg, IDC_EXTERNALCSS_FILENAME, path);
			}
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_BACKGROUND_IMAGE_FILENAME:
            case IDC_EXTERNALCSS_FILENAME:
				if ((HWND)lParam==GetFocus() && HIWORD(wParam)==EN_CHANGE)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_SCROLL_BACKGROUND_IMAGE:
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_BACKGROUND_IMAGE:
				bChecked = IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_EXTERNALCSS:
				bChecked = IsDlgButtonChecked(hwndDlg, IDC_EXTERNALCSS);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS), bChecked);
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_BROWSE_BACKGROUND_IMAGE:
				{
					OPENFILENAME ofn={0};
					GetDlgItemText(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "All Images (*.jpg,*.gif,*.png,*.bmp)\0*.jpg;*.gif;*.png;*.bmp\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "jpg";
					if(GetOpenFileName(&ofn)) {
						SetDlgItemText(hwndDlg,IDC_BACKGROUND_IMAGE_FILENAME,path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_BROWSE_EXTERNALCSS:
				{
					OPENFILENAME ofn={0};
					GetDlgItemText(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Style Sheet (*.css)\0*.css\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "css";
					if(GetOpenFileName(&ofn)) {
						SetDlgItemText(hwndDlg, IDC_EXTERNALCSS_FILENAME, path);
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
				if (IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE)) {
					i |= Options::BKGIMAGE_ENABLED;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE)) {
					i |= Options::BKGIMAGE_SCROLL;
				}
				Options::setBkgImageFlags(i);
				GetDlgItemText(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, sizeof(path));
				Options::setBkgImageFile(path);
				i = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_EXTERNALCSS)) {
					i |= Options::BKGIMAGE_ENABLED;
				}
				Options::setExternalCSSFlags(i);
				GetDlgItemText(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, sizeof(path));
				Options::setExternalCSSFile(path);
				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}

static BOOL CALLBACK IEViewEmoticonsOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int i;
	BOOL bChecked;
	char path[MAX_PATH];
	switch (msg) {
	case WM_INITDIALOG:
		{
			char *path;
			TranslateDialogDefault(hwndDlg);
			SendDlgItemMessage(hwndDlg, IDC_SMILEYS_PREVIEW, BM_SETIMAGE, IMAGE_ICON, (LPARAM) smileyIcon);

			bChecked = FALSE;
			i = Options::getSmileyFlags();
			if (i&Options::SMILEY_ENABLED) {
			    bChecked = TRUE;
				CheckDlgButton(hwndDlg, IDC_SMILEYS, TRUE);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_SMILEYS_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_SMILEYS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ISOLATED_SMILEYS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SURROUND_SMILEYS), bChecked);
//			EnableWindow(GetDlgItem(hwndDlg, IDC_REPLACE_SMILEYADD), bChecked);
			if (Options::getSmileyFlags() & Options::SMILEY_PROTOCOLS) {
				CheckDlgButton(hwndDlg, IDC_PROTO_SMILEYS, TRUE);
			} else {
				bChecked = FALSE;
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROTOLIST), bChecked);
			if (i&Options::SMILEY_ISOLATED) {
				CheckDlgButton(hwndDlg, IDC_ISOLATED_SMILEYS, TRUE);
			}
			if (i&Options::SMILEY_SURROUND) {
				CheckDlgButton(hwndDlg, IDC_SURROUND_SMILEYS, TRUE);
			}
//			if (i&Options::SMILEY_SMILEYADD) {
	//			CheckDlgButton(hwndDlg, IDC_REPLACE_SMILEYADD, TRUE);
		//	}
			PROTOCOLDESCRIPTOR **protoList;
			int protoCount;
			CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&protoCount, (LPARAM)&protoList);
			protoNames = new char[protoCount+1][128];
			protoFilenames = new char[protoCount+1][MAX_PATH];
			protoNum = 0;
			for (i = 0; i < protoCount+1; i++) {
    			char * protocolName;
    			char protoName[128];
    			char displayName[256];
				if (i==0) {
                    strcpy(protoNames[protoNum], "");
                    protocolName = "Standard";
				} else if (protoList[i-1]->type == PROTOTYPE_PROTOCOL) {
	    			strcpy(protoNames[protoNum], protoList[i-1]->szName);
	    			CallProtoService(protoList[i-1]->szName, PS_GETNAME, sizeof(protoName), (LPARAM)protoName);
	    			protocolName = protoName;//protoList[i-1]->szName;
				} else {
					continue;
				}
				strcpy(displayName, protocolName);
				strcat(displayName, " smileys");
				path = (char *) Options::getSmileyFile(protoNames[protoNum]);
				if (path != NULL) {
					strcpy (protoFilenames[protoNum], path);
				} else {
					strcpy (protoFilenames[protoNum], "");
				}
    			SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, LB_ADDSTRING, 0, (LPARAM)displayName);
    			protoNum++;
			}
			SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, LB_SETCURSEL, 0, 0);
			SetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, protoFilenames[0]);
			lastProtoItem = 0;
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_SMILEYS_FILENAME:
				if ((HWND)lParam==GetFocus() && HIWORD(wParam)==EN_CHANGE)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_SMILEYS_PREVIEW:
				{
					int iItem = SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, LB_GETCURSEL, 0, 0);
					GetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, path, sizeof(path));
					if (SmileyMap::loadLibrary("IEVIewPreview", path)) {
					    SmileyMap *map = SmileyMap::getSmileyMap("IEVIewPreview");
					    if (map!=NULL) {
	                        RECT rc;
						    GetWindowRect(GetDlgItem(hwndDlg, IDC_SMILEYS_PREVIEW), &rc);
							map->getWindow()->show(NULL, 0, 0, rc.left, rc.bottom);
						}
					} else {
	                	SetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, protoFilenames[iItem]);
					}
				}
			    break;
			case IDC_ISOLATED_SMILEYS:
			case IDC_SURROUND_SMILEYS:
//			case IDC_REPLACE_SMILEYADD:
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_SMILEYS:
				bChecked = IsDlgButtonChecked(hwndDlg, IDC_SMILEYS);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SMILEYS_FILENAME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_SMILEYS), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ISOLATED_SMILEYS), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SURROUND_SMILEYS), bChecked);
//				EnableWindow(GetDlgItem(hwndDlg, IDC_REPLACE_SMILEYADD), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PROTO_SMILEYS), bChecked);
			case IDC_PROTO_SMILEYS:
				bChecked = IsDlgButtonChecked(hwndDlg, IDC_PROTO_SMILEYS) & IsDlgButtonChecked(hwndDlg, IDC_SMILEYS);
				if (!bChecked ) {
					SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, LB_SETCURSEL, 0, 0);
					GetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, path, sizeof(path));
					strcpy(protoFilenames[lastProtoItem], path);
	                SetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, protoFilenames[0]);
					lastProtoItem = 0;
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_PROTOLIST), bChecked);
				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_BROWSE_SMILEYS:
				{
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
						SetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, path);
						SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
					}
				}
				break;
			}
			case IDC_PROTOLIST:
				if (HIWORD(wParam) == LBN_SELCHANGE) {
					int iItem = SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, LB_GETCURSEL, 0, 0);
					GetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, path, sizeof(path));
					strcpy(protoFilenames[lastProtoItem], path);
	                SetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, protoFilenames[iItem]);
					lastProtoItem = iItem;
				}
		}
		break;
	case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->code) {
			case PSN_APPLY:
				i = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_SMILEYS)) {
					i |= Options::SMILEY_ENABLED;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_ISOLATED_SMILEYS)) {
					i |= Options::SMILEY_ISOLATED;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_SURROUND_SMILEYS)) {
					i |= Options::SMILEY_SURROUND;
				}
/*				if (IsDlgButtonChecked(hwndDlg, IDC_REPLACE_SMILEYADD)) {
					i |= Options::SMILEY_SMILEYADD;
				}
	*/			if (IsDlgButtonChecked(hwndDlg, IDC_PROTO_SMILEYS)) {
					i |= Options::SMILEY_PROTOCOLS;
				}
				Options::setSmileyFlags(i);
				GetDlgItemText(hwndDlg, IDC_SMILEYS_FILENAME, path, sizeof(path));
				strcpy(protoFilenames[lastProtoItem], path);
				for (i = 0; i < protoNum; i++) {
                    Options::setSmileyFile(protoNames[i], protoFilenames[i]);
				}
				return TRUE;
				
			}
		}
		break;
	case WM_DESTROY:
		if (protoNames != NULL) {
			delete[] protoNames;
			protoNames = NULL;
		}
		if (protoFilenames != NULL) {
			delete[] protoFilenames;
			protoFilenames = NULL;
		}
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
			char *path;
			TranslateDialogDefault(hwndDlg);
			bChecked = FALSE;
			if (Options::getTemplatesFlags() & Options::TEMPLATES_ENABLED) {
			    bChecked = TRUE;
				CheckDlgButton(hwndDlg, IDC_TEMPLATES, TRUE);
			}
			if (Options::getTemplatesFlags() & Options::LOG_SHOW_FILE) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_FILE, TRUE);
			}
			if (Options::getTemplatesFlags() & Options::LOG_SHOW_URL) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_URL, TRUE);
			}
			if (Options::getTemplatesFlags() & Options::LOG_SHOW_STATUSCHANGE) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_STATUSCHANGE, TRUE);
			}
			if (Options::getTemplatesFlags() & Options::LOG_SHOW_NICKNAMES) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_NICKNAMES, TRUE);
			}
			if (Options::getTemplatesFlags() & Options::LOG_SHOW_TIME) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_TIME, TRUE);
			}
			if (Options::getTemplatesFlags() & Options::LOG_SHOW_DATE) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_DATE, TRUE);
			}
			if (Options::getTemplatesFlags() & Options::LOG_SHOW_SECONDS) {
				CheckDlgButton(hwndDlg, IDC_LOG_SHOW_SECONDS, TRUE);
			}
			if (Options::getTemplatesFlags() & Options::LOG_LONG_DATE) {
				CheckDlgButton(hwndDlg, IDC_LOG_LONG_DATE, TRUE);
			}
			if (Options::getTemplatesFlags() & Options::LOG_RELATIVE_DATE) {
				CheckDlgButton(hwndDlg, IDC_LOG_RELATIVE_DATE, TRUE);
			}
			if (Options::getTemplatesFlags() & Options::LOG_GROUP_MESSAGES) {
				CheckDlgButton(hwndDlg, IDC_LOG_GROUP_MESSAGES, TRUE);
			}

			EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATES_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_TEMPLATES), bChecked);

			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_FILE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_URL), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_STATUSCHANGE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_NICKNAMES), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_TIME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_DATE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_SECONDS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_LONG_DATE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_RELATIVE_DATE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_GROUP_MESSAGES), bChecked);

			path = (char *)Options::getTemplatesFile();
			if (path != NULL) {
                SetDlgItemText(hwndDlg, IDC_TEMPLATES_FILENAME, path);
			}
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
            case IDC_TEMPLATES_FILENAME:
				if ((HWND)lParam==GetFocus() && HIWORD(wParam)==EN_CHANGE)
					SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_LOG_SHOW_FILE:
			case IDC_LOG_SHOW_URL:
			case IDC_LOG_SHOW_STATUSCHANGE:
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
				
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_FILE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_URL), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_STATUSCHANGE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_NICKNAMES), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_TIME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_DATE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_SECONDS), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_LONG_DATE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_RELATIVE_DATE), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_GROUP_MESSAGES), bChecked);

				SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				break;
			case IDC_BROWSE_TEMPLATES:
				{
					OPENFILENAME ofn={0};
					GetDlgItemText(hwndDlg, IDC_TEMPLATES_FILENAME, path, sizeof(path));
					ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = "Templates (*.ivt)\0*.ivt\0All Files\0*.*\0\0";
					ofn.lpstrFile = path;
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.nMaxFile = sizeof(path);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "ivt";
					if(GetOpenFileName(&ofn)) {
						SetDlgItemText(hwndDlg, IDC_TEMPLATES_FILENAME, path);
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
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_FILE)) {
					i |= Options::LOG_SHOW_FILE;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_URL)) {
					i |= Options::LOG_SHOW_URL;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_STATUSCHANGE)) {
					i |= Options::LOG_SHOW_STATUSCHANGE;
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
				Options::setTemplatesFlags(i);
				GetDlgItemText(hwndDlg, IDC_TEMPLATES_FILENAME, path, sizeof(path));
				Options::setTemplatesFile(path);
				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}

char *Options::bkgFilename = NULL;
int Options::bkgFlags;
int Options::smileyFlags;
char *Options::externalCSSFilename = NULL;
int Options::externalCSSFlags;
char *Options::templatesFilename= NULL;
int Options::templatesFlags;

void Options::init() {
	DBVARIANT dbv;
	bkgFlags = DBGetContactSettingDword(NULL, muccModuleName, DBS_BACKGROUNDIMAGEFLAGS, 0);
	if (!DBGetContactSetting(NULL,  muccModuleName, DBS_BACKGROUNDIMAGEFILE, &dbv)) {
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
	smileyFlags = DBGetContactSettingDword(NULL, muccModuleName, DBS_SMILEYSFLAGS, 1);
	PROTOCOLDESCRIPTOR **protoList;
	int protoCount;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&protoCount, (LPARAM)&protoList);
	for (int i = 0; i < protoCount; i++) {
		if (protoList[i]->type == PROTOTYPE_PROTOCOL) {
			char dbName[1024];
			strcpy(dbName, protoList[i]->szName);
			strcat(dbName, "SmileyFile");
			if (!DBGetContactSetting(NULL,  muccModuleName, dbName, &dbv)) {
			   	char tmpPath[MAX_PATH];
            	strcpy(tmpPath, dbv.pszVal);
            	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)) {
           	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
           		}   	   	
				SmileyMap::loadLibrary(protoList[i]->szName, tmpPath);
				DBFreeVariant(&dbv);
			}
		}
	}
	if (!DBGetContactSetting(NULL,  muccModuleName, "SmileyFile", &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}   	   	
		SmileyMap::loadLibrary("", tmpPath);
		DBFreeVariant(&dbv);
	}
	externalCSSFlags = DBGetContactSettingDword(NULL, muccModuleName, DBS_EXTERNALCSSFLAGS, FALSE);
	if (!DBGetContactSetting(NULL,  muccModuleName, DBS_EXTERNALCSSFILE, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE) && strncmp(tmpPath, "http://", 7)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}   	   	
		externalCSSFilename = new char[strlen(tmpPath)+1];
		strcpy(externalCSSFilename, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        externalCSSFilename = new char[1];
        strcpy(externalCSSFilename, "");
	}
	templatesFlags = DBGetContactSettingDword(NULL, muccModuleName, DBS_TEMPLATESFLAGS, FALSE);
	if (!DBGetContactSetting(NULL,  muccModuleName, DBS_TEMPLATESFILE, &dbv)) {
    	char tmpPath[MAX_PATH];
    	strcpy(tmpPath, dbv.pszVal);
    	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)) {
   	    	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
   		}   	   	
		templatesFilename = new char[strlen(tmpPath)+1];
		strcpy(templatesFilename, tmpPath);
		DBFreeVariant(&dbv);
	} else {
        templatesFilename = new char[1];
        strcpy(templatesFilename, "");
	}
	TemplateMap::loadTemplates("default", templatesFilename);
	smileyIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_SMILEY), IMAGE_ICON, 0, 0, 0);
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
	DBWriteContactSettingString(NULL, muccModuleName, DBS_BACKGROUNDIMAGEFILE, tmpPath);
}

const char * Options::getBkgImageFile() {
	return bkgFilename;
}

void Options::setBkgImageFlags(int flags) {
	bkgFlags = flags;
	DBWriteContactSettingDword(NULL, muccModuleName, DBS_BACKGROUNDIMAGEFLAGS, (DWORD) flags);
}

int	Options::getBkgImageFlags() {
	return bkgFlags;
}

void Options::setSmileyFile(const char *proto, const char *filename) {
	char dbName[1024];
	strcpy(dbName, proto);
	strcat(dbName, "SmileyFile");
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}   	
	DBWriteContactSettingString(NULL, muccModuleName, dbName, tmpPath);
	SmileyMap::loadLibrary(proto, filename);
}

const char *Options::getSmileyFile(const char *proto) {
	SmileyMap *map = SmileyMap::getSmileyMap(proto);
	if (map != NULL) {
		return map->getFilename();
	} 
    return NULL;
}

void Options::setSmileyFlags(int flags) {
	smileyFlags = flags;
	DBWriteContactSettingDword(NULL, muccModuleName, DBS_SMILEYSFLAGS, (DWORD) flags);
}

int	Options::getSmileyFlags() {
	return smileyFlags;
}

void Options::setExternalCSSFile(const char *filename) {
	if (externalCSSFilename != NULL) {
		delete [] externalCSSFilename;
	}
	externalCSSFilename = new char[strlen(filename)+1];
	strcpy(externalCSSFilename, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE) && strncmp(tmpPath, "http://", 7)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}   	
	DBWriteContactSettingString(NULL, muccModuleName, DBS_EXTERNALCSSFILE, tmpPath);
}

const char *Options::getExternalCSSFile() {
	return externalCSSFilename;
}

void Options::setExternalCSSFlags(int flags) {
	externalCSSFlags = flags;
	DBWriteContactSettingDword(NULL, muccModuleName, DBS_EXTERNALCSSFLAGS, (DWORD) flags);
}


int	Options::getExternalCSSFlags() {
	return externalCSSFlags;
}

void Options::setTemplatesFile(const char *filename) {
	if (templatesFilename != NULL) {
		delete [] templatesFilename;
	}
	templatesFilename = new char[strlen(filename)+1];
	strcpy(templatesFilename, filename);
    char tmpPath[MAX_PATH];
    strcpy (tmpPath, filename);
    if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
    	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)filename, (LPARAM)tmpPath);
   	}   	
	DBWriteContactSettingString(NULL, muccModuleName, DBS_TEMPLATESFILE, tmpPath);
	
	TemplateMap::loadTemplates("default", templatesFilename);
}

const char *Options::getTemplatesFile() {
	return templatesFilename;
}

void Options::setTemplatesFlags(int flags) {
	templatesFlags = flags;
	DBWriteContactSettingDword(NULL, muccModuleName, DBS_TEMPLATESFLAGS, (DWORD) flags);
}

int	Options::getTemplatesFlags() {
	return templatesFlags;
}
