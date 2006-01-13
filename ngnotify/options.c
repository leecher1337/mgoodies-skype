/*
  Name: NewGenerationNotify - Plugin for Miranda ICQ
  File: main.c - Main DLL procedures
  Version: 0.0.4
  Description: Notifies you about some events
  Author: prezes, <prezesso@klub.chip.pl>
  Date: 01.09.04 / Update: 12.05.05 17:00
  Copyright: (C) 2002 Starzinger Michael

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "newgenerationnotify.h"
#include <m_options.h>
#include <m_database.h>
#include <m_utils.h>

PLUGIN_OPTIONS* options;
BOOL bWmNotify;

int OptionsRead(void)
{
    options->bDisable = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_DISABLE, FALSE);
    options->bDefaultColorMsg = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_COLDEFAULT_MESSAGE, FALSE);
	options->bDefaultColorUrl = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_COLDEFAULT_URL, FALSE);
	options->bDefaultColorFile = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_COLDEFAULT_FILE, FALSE);
	options->bDefaultColorOthers = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_COLDEFAULT_OTHERS, FALSE);
    options->colBackMsg = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLBACK_MESSAGE, DEFAULT_COLBACK);
    options->colTextMsg = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLTEXT_MESSAGE, DEFAULT_COLTEXT);
    options->colBackUrl = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLBACK_URL, DEFAULT_COLBACK);
    options->colTextUrl = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLTEXT_URL, DEFAULT_COLTEXT);
    options->colBackFile = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLBACK_FILE, DEFAULT_COLBACK);
    options->colTextFile = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLTEXT_FILE, DEFAULT_COLTEXT);
    options->colBackOthers = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLBACK_OTHERS, DEFAULT_COLBACK);
    options->colTextOthers = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLTEXT_OTHERS, DEFAULT_COLTEXT);
    options->maskNotify = (UINT)DBGetContactSettingByte(NULL, MODULE, OPT_MASKNOTIFY, DEFAULT_MASKNOTIFY);
    options->maskActL = (UINT)DBGetContactSettingByte(NULL, MODULE, OPT_MASKACTL, DEFAULT_MASKACTL);
    options->maskActR = (UINT)DBGetContactSettingByte(NULL, MODULE, OPT_MASKACTR, DEFAULT_MASKACTR);
	options->maskActTE = (UINT)DBGetContactSettingByte(NULL, MODULE, OPT_MASKACTTE, DEFAULT_MASKACTR);
    options->bMsgWindowcheck = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_MSGWINDOWCHECK, TRUE);
	options->bMergePopup = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_MERGEPOPUP, TRUE);
	options->iDelayMsg = (int)DBGetContactSettingDword(NULL, MODULE, OPT_DELAY_MESSAGE, DEFAULT_DELAY);
	options->iDelayUrl = (int)DBGetContactSettingDword(NULL, MODULE, OPT_DELAY_URL, DEFAULT_DELAY);
	options->iDelayFile = (int)DBGetContactSettingDword(NULL, MODULE, OPT_DELAY_FILE, DEFAULT_DELAY);
	options->iDelayOthers = (int)DBGetContactSettingDword(NULL, MODULE, OPT_DELAY_OTHERS, DEFAULT_DELAY);
	options->iDelayDefault = (int)DBGetContactSettingRangedWord(NULL, "PopUp", "Seconds",
		SETTING_LIFETIME_DEFAULT, SETTING_LIFETIME_MIN, SETTING_LIFETIME_MAX);
	options->bShowDate = (BYTE)DBGetContactSettingByte(NULL, MODULE, OPT_SHOW_DATE, TRUE);
	options->bShowTime = (BYTE)DBGetContactSettingByte(NULL, MODULE, OPT_SHOW_TIME, TRUE);
	options->bShowHeaders = (BYTE)DBGetContactSettingByte(NULL, MODULE, OPT_SHOW_HEADERS, TRUE);
	options->iNumberMsg = (BYTE)DBGetContactSettingByte(NULL, MODULE, OPT_NUMBER_MSG, TRUE);
	options->bShowON = (BYTE)DBGetContactSettingByte(NULL, MODULE, OPT_SHOW_ON, TRUE);
	options->bHideSend = (BYTE)DBGetContactSettingByte(NULL, MODULE, OPT_HIDESEND, TRUE);
	options->bNoRSS = (BOOL)DBGetContactSettingByte(NULL, MODULE, OPT_NORSS, FALSE);
    return 0;
}

int OptionsWrite(void)
{
    DBWriteContactSettingByte(NULL, MODULE, OPT_DISABLE, (BYTE)options->bDisable);
    DBWriteContactSettingByte(NULL, MODULE, OPT_COLDEFAULT_MESSAGE, (BYTE)options->bDefaultColorMsg);
    DBWriteContactSettingByte(NULL, MODULE, OPT_COLDEFAULT_URL, (BYTE)options->bDefaultColorUrl);
    DBWriteContactSettingByte(NULL, MODULE, OPT_COLDEFAULT_FILE, (BYTE)options->bDefaultColorFile);
    DBWriteContactSettingByte(NULL, MODULE, OPT_COLDEFAULT_OTHERS, (BYTE)options->bDefaultColorOthers);
    DBWriteContactSettingDword(NULL, MODULE, OPT_COLBACK_MESSAGE, (DWORD)options->colBackMsg);
    DBWriteContactSettingDword(NULL, MODULE, OPT_COLTEXT_MESSAGE, (DWORD)options->colTextMsg);
    DBWriteContactSettingDword(NULL, MODULE, OPT_COLBACK_URL, (DWORD)options->colBackUrl);
    DBWriteContactSettingDword(NULL, MODULE, OPT_COLTEXT_URL, (DWORD)options->colTextUrl);
    DBWriteContactSettingDword(NULL, MODULE, OPT_COLBACK_FILE, (DWORD)options->colBackFile);
    DBWriteContactSettingDword(NULL, MODULE, OPT_COLTEXT_FILE, (DWORD)options->colTextFile);
    DBWriteContactSettingDword(NULL, MODULE, OPT_COLBACK_OTHERS, (DWORD)options->colBackOthers);
    DBWriteContactSettingDword(NULL, MODULE, OPT_COLTEXT_OTHERS, (DWORD)options->colTextOthers);
    DBWriteContactSettingByte(NULL, MODULE, OPT_MASKNOTIFY, (BYTE)options->maskNotify);
    DBWriteContactSettingByte(NULL, MODULE, OPT_MASKACTL, (BYTE)options->maskActL);
    DBWriteContactSettingByte(NULL, MODULE, OPT_MASKACTR, (BYTE)options->maskActR);
	DBWriteContactSettingByte(NULL, MODULE, OPT_MASKACTTE, (BYTE)options->maskActTE);
    DBWriteContactSettingByte(NULL, MODULE, OPT_MSGWINDOWCHECK, (BYTE)options->bMsgWindowcheck);
	DBWriteContactSettingByte(NULL, MODULE, OPT_MERGEPOPUP, (BYTE)options->bMergePopup);
	DBWriteContactSettingDword(NULL, MODULE, OPT_DELAY_MESSAGE, (DWORD)options->iDelayMsg);
	DBWriteContactSettingDword(NULL, MODULE, OPT_DELAY_URL, (DWORD)options->iDelayUrl);
	DBWriteContactSettingDword(NULL, MODULE, OPT_DELAY_FILE, (DWORD)options->iDelayFile);
	DBWriteContactSettingDword(NULL, MODULE, OPT_DELAY_OTHERS, (DWORD)options->iDelayOthers);
	DBWriteContactSettingByte(NULL, MODULE, OPT_SHOW_DATE, (BYTE)options->bShowDate);
	DBWriteContactSettingByte(NULL, MODULE, OPT_SHOW_TIME, (BYTE)options->bShowTime);
	DBWriteContactSettingByte(NULL, MODULE, OPT_SHOW_HEADERS, (BYTE)options->bShowHeaders);
	DBWriteContactSettingByte(NULL, MODULE, OPT_NUMBER_MSG, (BYTE)options->iNumberMsg);
	DBWriteContactSettingByte(NULL, MODULE, OPT_SHOW_ON, (BYTE)options->bShowON);
	DBWriteContactSettingByte(NULL, MODULE, OPT_HIDESEND, (BYTE)options->bHideSend);
	DBWriteContactSettingByte(NULL, MODULE, OPT_NORSS, (BYTE)options->bNoRSS);
    return 0;
}

static BOOL CALLBACK OptionsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
    {
        case WM_INITDIALOG:
                TranslateDialogDefault(hWnd);
                //make dialog represent the current options
                bWmNotify = TRUE;
				SendDlgItemMessage(hWnd, IDC_COLBACK_MESSAGE, CPM_SETCOLOUR, 0, options->colBackMsg);
                SendDlgItemMessage(hWnd, IDC_COLTEXT_MESSAGE, CPM_SETCOLOUR, 0, options->colTextMsg);
                SendDlgItemMessage(hWnd, IDC_COLBACK_URL, CPM_SETCOLOUR, 0, options->colBackUrl);
                SendDlgItemMessage(hWnd, IDC_COLTEXT_URL, CPM_SETCOLOUR, 0, options->colTextUrl);
                SendDlgItemMessage(hWnd, IDC_COLBACK_FILE, CPM_SETCOLOUR, 0, options->colBackFile);
                SendDlgItemMessage(hWnd, IDC_COLTEXT_FILE, CPM_SETCOLOUR, 0, options->colTextFile);
                SendDlgItemMessage(hWnd, IDC_COLBACK_OTHERS, CPM_SETCOLOUR, 0, options->colBackOthers);
                SendDlgItemMessage(hWnd, IDC_COLTEXT_OTHERS, CPM_SETCOLOUR, 0, options->colTextOthers);
       			CheckDlgButton(hWnd, IDC_CHKDEFAULTCOL_MESSAGE, options->bDefaultColorMsg?BST_CHECKED:BST_UNCHECKED);
       			CheckDlgButton(hWnd, IDC_CHKDEFAULTCOL_URL, options->bDefaultColorUrl?BST_CHECKED:BST_UNCHECKED);
       			CheckDlgButton(hWnd, IDC_CHKDEFAULTCOL_FILE, options->bDefaultColorFile?BST_CHECKED:BST_UNCHECKED);
       			CheckDlgButton(hWnd, IDC_CHKDEFAULTCOL_OTHERS, options->bDefaultColorOthers?BST_CHECKED:BST_UNCHECKED);
       			CheckDlgButton(hWnd, IDC_CHKDISABLE, options->bDisable?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_CHKMERGEPOPUP, options->bMergePopup?BST_CHECKED:BST_UNCHECKED);
       			CheckDlgButton(hWnd, IDC_CHKNOTIFY_MESSAGE, options->maskNotify & MASK_MESSAGE);
       			CheckDlgButton(hWnd, IDC_CHKNOTIFY_URL, options->maskNotify & MASK_URL);
       			CheckDlgButton(hWnd, IDC_CHKNOTIFY_FILE, options->maskNotify & MASK_FILE);
       			CheckDlgButton(hWnd, IDC_CHKNOTIFY_OTHER, options->maskNotify & MASK_OTHER);
       			CheckDlgButton(hWnd, IDC_CHKACTL_DISMISS, options->maskActL & MASK_DISMISS);
       			CheckDlgButton(hWnd, IDC_CHKACTL_OPEN, options->maskActL & MASK_OPEN);
       			CheckDlgButton(hWnd, IDC_CHKACTL_REMOVE, options->maskActL & MASK_REMOVE);
       			CheckDlgButton(hWnd, IDC_CHKACTR_DISMISS, options->maskActR & MASK_DISMISS);
       			CheckDlgButton(hWnd, IDC_CHKACTR_OPEN, options->maskActR & MASK_OPEN);
       			CheckDlgButton(hWnd, IDC_CHKACTR_REMOVE, options->maskActR & MASK_REMOVE);
				CheckDlgButton(hWnd, IDC_CHKACTTE_DISMISS, options->maskActTE & MASK_DISMISS);
       			CheckDlgButton(hWnd, IDC_CHKACTTE_OPEN, options->maskActTE & MASK_OPEN);
       			CheckDlgButton(hWnd, IDC_CHKACTTE_REMOVE, options->maskActTE & MASK_REMOVE);
       			CheckDlgButton(hWnd, IDC_CHKWINDOWCHECK, options->bMsgWindowcheck?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_CHKSHOWDATE, options->bShowDate?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_CHKSHOWTIME, options->bShowTime?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_CHKSHOWHEADERS, options->bShowHeaders?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_RDNEW, options->bShowON?BST_UNCHECKED:BST_CHECKED);
				CheckDlgButton(hWnd, IDC_RDOLD, options->bShowON?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_CHKHIDESEND, options->bHideSend?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_SUPRESSRSS, options->bNoRSS?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_CHKINFINITE_MESSAGE, options->iDelayMsg == -1?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_CHKINFINITE_URL, options->iDelayUrl == -1?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_CHKINFINITE_FILE, options->iDelayFile == -1?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hWnd, IDC_CHKINFINITE_OTHERS, options->iDelayOthers == -1?BST_CHECKED:BST_UNCHECKED);
				SetDlgItemInt(hWnd, IDC_DELAY_MESSAGE, options->iDelayMsg != -1?options->iDelayMsg:0, TRUE);
				SetDlgItemInt(hWnd, IDC_DELAY_URL, options->iDelayUrl != -1?options->iDelayUrl:0, TRUE);
				SetDlgItemInt(hWnd, IDC_DELAY_FILE, options->iDelayFile != -1?options->iDelayFile:0, TRUE);
				SetDlgItemInt(hWnd, IDC_DELAY_OTHERS, options->iDelayOthers != -1?options->iDelayOthers:0, TRUE);
				SetDlgItemInt(hWnd, IDC_NUMBERMSG, options->iNumberMsg, FALSE);
       			//disable color picker when using default colors
                EnableWindow(GetDlgItem(hWnd, IDC_COLBACK_MESSAGE), !options->bDefaultColorMsg);
                EnableWindow(GetDlgItem(hWnd, IDC_COLTEXT_MESSAGE), !options->bDefaultColorMsg);
                EnableWindow(GetDlgItem(hWnd, IDC_COLBACK_URL), !options->bDefaultColorUrl);
                EnableWindow(GetDlgItem(hWnd, IDC_COLTEXT_URL), !options->bDefaultColorUrl);
                EnableWindow(GetDlgItem(hWnd, IDC_COLBACK_FILE), !options->bDefaultColorFile);
                EnableWindow(GetDlgItem(hWnd, IDC_COLTEXT_FILE), !options->bDefaultColorFile);
                EnableWindow(GetDlgItem(hWnd, IDC_COLBACK_OTHERS), !options->bDefaultColorOthers);
                EnableWindow(GetDlgItem(hWnd, IDC_COLTEXT_OTHERS), !options->bDefaultColorOthers);
				//disable merge messages options when is not using
				EnableWindow(GetDlgItem(hWnd, IDC_CHKSHOWDATE), options->bMergePopup);
				EnableWindow(GetDlgItem(hWnd, IDC_CHKSHOWTIME), options->bMergePopup);
				EnableWindow(GetDlgItem(hWnd, IDC_CHKSHOWHEADERS), options->bMergePopup);
				EnableWindow(GetDlgItem(hWnd, IDC_CMDEDITHEADERS), options->bMergePopup && options->bShowHeaders);
				EnableWindow(GetDlgItem(hWnd, IDC_NUMBERMSG), options->bMergePopup);
				EnableWindow(GetDlgItem(hWnd, IDC_LBNUMBERMSG), options->bMergePopup);
				EnableWindow(GetDlgItem(hWnd, IDC_RDNEW), options->bMergePopup && options->iNumberMsg);
				EnableWindow(GetDlgItem(hWnd, IDC_RDOLD), options->bMergePopup && options->iNumberMsg);
				//disable delay textbox when infinite is checked
				EnableWindow(GetDlgItem(hWnd, IDC_DELAY_MESSAGE), options->iDelayMsg != -1);
				EnableWindow(GetDlgItem(hWnd, IDC_DELAY_URL), options->iDelayUrl != -1);
				EnableWindow(GetDlgItem(hWnd, IDC_DELAY_FILE), options->iDelayFile != -1);
				EnableWindow(GetDlgItem(hWnd, IDC_DELAY_OTHERS), options->iDelayOthers != -1);
       			bWmNotify = FALSE;
                return TRUE;
        case WM_COMMAND:
			if (!bWmNotify)
			{
				switch (LOWORD(wParam))
					{
						case IDC_PREVIEW:
						    PopupPreview(options);
							break;
						default:
						    //update options
							options->maskNotify = (IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_MESSAGE)?MASK_MESSAGE:0) |
														(IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_URL)?MASK_URL:0) |
				                                        (IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_FILE)?MASK_FILE:0) |
					                                    (IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_OTHER)?MASK_OTHER:0);
						    options->maskActL = (IsDlgButtonChecked(hWnd, IDC_CHKACTL_DISMISS)?MASK_DISMISS:0) |
							                            (IsDlgButtonChecked(hWnd, IDC_CHKACTL_OPEN)?MASK_OPEN:0) |
								                        (IsDlgButtonChecked(hWnd, IDC_CHKACTL_REMOVE)?MASK_REMOVE:0);
							options->maskActR = (IsDlgButtonChecked(hWnd, IDC_CHKACTR_DISMISS)?MASK_DISMISS:0) |
	                                                    (IsDlgButtonChecked(hWnd, IDC_CHKACTR_OPEN)?MASK_OPEN:0) |
		                                                (IsDlgButtonChecked(hWnd, IDC_CHKACTR_REMOVE)?MASK_REMOVE:0);
							options->maskActTE = (IsDlgButtonChecked(hWnd, IDC_CHKACTTE_DISMISS)?MASK_DISMISS:0) |
	                                                    (IsDlgButtonChecked(hWnd, IDC_CHKACTTE_OPEN)?MASK_OPEN:0) |
		                                                (IsDlgButtonChecked(hWnd, IDC_CHKACTTE_REMOVE)?MASK_REMOVE:0);
			                options->bDefaultColorMsg = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_MESSAGE);
				            options->bDefaultColorUrl = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_URL);
					        options->bDefaultColorFile = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_FILE);
						    options->bDefaultColorOthers = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_OTHERS);
							options->bDisable = IsDlgButtonChecked(hWnd, IDC_CHKDISABLE);
							options->iDelayMsg = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_MESSAGE)?-1:(DWORD)GetDlgItemInt(hWnd, IDC_DELAY_MESSAGE, NULL, FALSE);
							options->iDelayUrl = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_URL)?-1:(DWORD)GetDlgItemInt(hWnd, IDC_DELAY_URL, NULL, FALSE);
							options->iDelayFile = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_FILE)?-1:(DWORD)GetDlgItemInt(hWnd, IDC_DELAY_FILE, NULL, FALSE);
							options->iDelayOthers = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_OTHERS)?-1:(DWORD)GetDlgItemInt(hWnd, IDC_DELAY_OTHERS, NULL, FALSE);
							options->bMergePopup = IsDlgButtonChecked(hWnd, IDC_CHKMERGEPOPUP);
							options->bMsgWindowcheck = IsDlgButtonChecked(hWnd, IDC_CHKWINDOWCHECK);
				            options->bShowDate = IsDlgButtonChecked(hWnd, IDC_CHKSHOWDATE);
							options->bShowTime = IsDlgButtonChecked(hWnd, IDC_CHKSHOWTIME);
							options->bShowHeaders = IsDlgButtonChecked(hWnd, IDC_CHKSHOWHEADERS);
							options->bShowON = IsDlgButtonChecked(hWnd, IDC_RDOLD);
							options->bShowON = !IsDlgButtonChecked(hWnd, IDC_RDNEW);
							options->bHideSend = IsDlgButtonChecked(hWnd, IDC_CHKHIDESEND);
							options->iNumberMsg = GetDlgItemInt(hWnd, IDC_NUMBERMSG, NULL, FALSE);
							options->bNoRSS = IsDlgButtonChecked(hWnd, IDC_SUPRESSRSS);
							EnableWindow(GetDlgItem(hWnd, IDC_COLBACK_MESSAGE), !options->bDefaultColorMsg);
							EnableWindow(GetDlgItem(hWnd, IDC_COLTEXT_MESSAGE), !options->bDefaultColorMsg);
				            EnableWindow(GetDlgItem(hWnd, IDC_COLBACK_URL), !options->bDefaultColorUrl);
							EnableWindow(GetDlgItem(hWnd, IDC_COLTEXT_URL), !options->bDefaultColorUrl);
					        EnableWindow(GetDlgItem(hWnd, IDC_COLBACK_FILE), !options->bDefaultColorFile);
							EnableWindow(GetDlgItem(hWnd, IDC_COLTEXT_FILE), !options->bDefaultColorFile);
							EnableWindow(GetDlgItem(hWnd, IDC_COLBACK_OTHERS), !options->bDefaultColorOthers);
							EnableWindow(GetDlgItem(hWnd, IDC_COLTEXT_OTHERS), !options->bDefaultColorOthers);
							//disable merge messages options when is not using
							EnableWindow(GetDlgItem(hWnd, IDC_CHKSHOWDATE), options->bMergePopup);
							EnableWindow(GetDlgItem(hWnd, IDC_CHKSHOWTIME), options->bMergePopup);
							EnableWindow(GetDlgItem(hWnd, IDC_CHKSHOWHEADERS), options->bMergePopup);
							EnableWindow(GetDlgItem(hWnd, IDC_CMDEDITHEADERS), options->bMergePopup && options->bShowHeaders);
							EnableWindow(GetDlgItem(hWnd, IDC_NUMBERMSG), options->bMergePopup);
							EnableWindow(GetDlgItem(hWnd, IDC_LBNUMBERMSG), options->bMergePopup);
							EnableWindow(GetDlgItem(hWnd, IDC_RDNEW), options->bMergePopup && options->iNumberMsg);
							EnableWindow(GetDlgItem(hWnd, IDC_RDOLD), options->bMergePopup && options->iNumberMsg);
							//disable delay textbox when infinite is checked
							EnableWindow(GetDlgItem(hWnd, IDC_DELAY_MESSAGE), options->iDelayMsg != -1);
							EnableWindow(GetDlgItem(hWnd, IDC_DELAY_URL), options->iDelayUrl != -1);
							EnableWindow(GetDlgItem(hWnd, IDC_DELAY_FILE), options->iDelayFile != -1);
							EnableWindow(GetDlgItem(hWnd, IDC_DELAY_OTHERS), options->iDelayOthers != -1);
							if (HIWORD(wParam) == CPN_COLOURCHANGED)
							{
								options->colBackMsg = SendDlgItemMessage(hWnd, IDC_COLBACK_MESSAGE, CPM_GETCOLOUR, 0, 0);
								options->colTextMsg = SendDlgItemMessage(hWnd, IDC_COLTEXT_MESSAGE, CPM_GETCOLOUR, 0, 0);
								options->colBackUrl = SendDlgItemMessage(hWnd, IDC_COLBACK_URL, CPM_GETCOLOUR, 0, 0);
								options->colTextUrl = SendDlgItemMessage(hWnd, IDC_COLTEXT_URL, CPM_GETCOLOUR, 0, 0);
								options->colBackFile = SendDlgItemMessage(hWnd, IDC_COLBACK_FILE, CPM_GETCOLOUR, 0, 0);
								options->colTextFile = SendDlgItemMessage(hWnd, IDC_COLTEXT_FILE, CPM_GETCOLOUR, 0, 0);
								options->colBackOthers = SendDlgItemMessage(hWnd, IDC_COLBACK_OTHERS, CPM_GETCOLOUR, 0, 0);
								options->colTextOthers = SendDlgItemMessage(hWnd, IDC_COLTEXT_OTHERS, CPM_GETCOLOUR, 0, 0);
							}
							//send changes to menuitem
							MenuitemUpdate(!options->bDisable);
							//enable "Apply" button
							SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
							break;
						}
				}
                break;
        case WM_NOTIFY:
				switch (((LPNMHDR)lParam)->code)
				{
                      case PSN_APPLY:
                            OptionsWrite();
                      case PSN_RESET:
                            OptionsRead();

                            //maybe something changed with the menuitem
                            MenuitemUpdate(!options->bDisable);
				}
				break;
        default:
                break;
    }
    return FALSE;
}

int OptionsAdd(HINSTANCE hInst, WPARAM addInfo)
{
    OPTIONSDIALOGPAGE odp;
    //	if (ServiceExists(MS_POPUP_ADDPOPUP)) {
//  do we need this dialog if popup.dll is not there???

	odp.cbSize = sizeof(odp);
//	odp.position = 100000000;
	odp.hInstance = hInst;	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT);
	odp.pszTitle = Translate(OPTIONS_TITLE);
	odp.pszGroup = Translate(OPTIONS_GROUP);
//	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = OptionsDlgProc;
	CallService(MS_OPT_ADDPAGE, addInfo, (LPARAM)&odp);

    return 0;
}

int OptionsInit(PLUGIN_OPTIONS* pluginOptions)
{
    options = pluginOptions;

    OptionsRead();

    return 0;
}

int Opt_DisableNGN(BOOL Status)
{
    options->bDisable = Status;
    OptionsWrite();

    //BUG!
    //Update options dialog if open!

	return 0;
}
