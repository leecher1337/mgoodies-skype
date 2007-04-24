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

#include "options.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////


HANDLE hOptHook = NULL;

Options opts;


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK AutoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ &opts.resize_frame,					CONTROL_CHECKBOX,	IDC_FRAME_AUTOSIZE,	"FrameAutoSize", TRUE }
};


static OptPageControl popupsControls[] = { 
	{ &opts.popup_enable,					CONTROL_CHECKBOX,	IDC_POPUPS,			"PopupsEnable", FALSE },
	{ &opts.popup_bkg_color,				CONTROL_COLOR,		IDC_BGCOLOR,		"PopupsBgColor", RGB(255,255,255) },
	{ &opts.popup_text_color,				CONTROL_COLOR,		IDC_TEXTCOLOR,		"PopupsTextColor", RGB(0,0,0) },
	{ &opts.popup_use_win_colors,			CONTROL_CHECKBOX,	IDC_WINCOLORS,		"PopupsWinColors", FALSE },
	{ &opts.popup_use_default_colors,		CONTROL_CHECKBOX,	IDC_DEFAULTCOLORS,	"PopupsDefaultColors", FALSE },
	{ &opts.popup_delay_type,				CONTROL_RADIO,		IDC_DELAYFROMPU,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_DEFAULT },
	{ NULL,									CONTROL_RADIO,		IDC_DELAYCUSTOM,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_CUSTOM },
	{ NULL,									CONTROL_RADIO,		IDC_DELAYPERMANENT,	"PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_PERMANENT },
	{ &opts.popup_timeout,					CONTROL_SPIN,		IDC_DELAY,			"PopupsTimeout", 10, IDC_DELAY_SPIN, (WORD) 1, (WORD) 255 },
	{ &opts.popup_right_click_action,		CONTROL_COMBO,		IDC_RIGHT_ACTION,	"PopupsRightClick", POPUP_ACTION_CLOSEPOPUP },
	{ &opts.popup_left_click_action,		CONTROL_COMBO,		IDC_LEFT_ACTION,	"PopupsLeftClick", POPUP_ACTION_CLOSEPOPUP }
};

static UINT popupsExpertControls[] = { 
	IDC_COLOURS_G, IDC_BGCOLOR, IDC_BGCOLOR_L, IDC_TEXTCOLOR, IDC_TEXTCOLOR_L, IDC_WINCOLORS, IDC_DEFAULTCOLORS, 
	IDC_DELAY_G, IDC_DELAYFROMPU, IDC_DELAYCUSTOM, IDC_DELAYPERMANENT, IDC_DELAY, IDC_DELAY_SPIN,
	IDC_ACTIONS_G, IDC_RIGHT_ACTION_L, IDC_RIGHT_ACTION, IDC_LEFT_ACTION_L, IDC_LEFT_ACTION,
	IDC_PREV
};


// Functions //////////////////////////////////////////////////////////////////////////////////////

int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	if(ServiceExists(MS_POPUP_ADDPOPUPEX)
#ifdef UNICODE
		|| ServiceExists(MS_POPUP_ADDPOPUPW)
#endif
		)
	{
		ZeroMemory(&odp, sizeof(odp));
		odp.cbSize = sizeof(odp);
		odp.hInstance = hInst;
		odp.ptszGroup = TranslateT("Popups");
		odp.ptszTitle = TranslateT("Voice Calls");
		odp.pfnDlgProc = PopupsDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPS);
		odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
		odp.expertOnlyControls = popupsExpertControls;
		odp.nExpertOnlyControls = MAX_REGS(popupsExpertControls);
		CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	}

	ZeroMemory(&odp, sizeof(odp));
    odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.ptszTitle = TranslateT("Voice Calls");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTS);
    odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	ZeroMemory(&odp, sizeof(odp));
    odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.ptszGroup = TranslateT("Voice Calls");
	odp.ptszTitle = TranslateT("Auto actions");
	odp.pfnDlgProc = AutoDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_AUTO);
    odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


void InitOptions()
{
	LoadOptions();

	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}


void DeInitOptions()
{
	UnhookEvent(hOptHook);
}


void LoadOptions()
{
	LoadOpts(optionsControls, MAX_REGS(optionsControls), MODULE_NAME);
	LoadOpts(popupsControls, MAX_REGS(popupsControls), MODULE_NAME);
}


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	return SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}


static void PopupsEnableDisableCtrls(HWND hwndDlg)
{
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_POPUPS);

	EnableWindow(GetDlgItem(hwndDlg, IDC_COLOURS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYFROMPU), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYCUSTOM), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYPERMANENT), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIONS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHT_ACTION_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHT_ACTION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LEFT_ACTION_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LEFT_ACTION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PREV), enabled);
	
	EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), enabled &&
			!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) && 
			!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR), enabled &&
			!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) && 
			!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULTCOLORS), enabled &&
			!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_WINCOLORS), enabled &&
			!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));

	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), enabled &&
			IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM));
}


static BOOL CALLBACK PopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Do nothing"));
			SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Close popup"));

			SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Do nothing"));
			SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LONG) TranslateT("Close popup"));

			// Needs to be called here in this case
			BOOL ret = SaveOptsDlgProc(popupsControls, MAX_REGS(popupsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

			PopupsEnableDisableCtrls(hwndDlg);

			return ret;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDC_POPUPS:
				case IDC_WINCOLORS:
				case IDC_DEFAULTCOLORS:
				case IDC_DELAYFROMPU:
				case IDC_DELAYPERMANENT:
				case IDC_DELAYCUSTOM:
				{
					if (HIWORD(wParam) == BN_CLICKED)
						PopupsEnableDisableCtrls(hwndDlg);

					break;
				}
				case IDC_PREV: 
				{
					Options op = opts;

					if (IsDlgButtonChecked(hwndDlg, IDC_DELAYFROMPU))
						op.popup_delay_type = POPUP_DELAY_DEFAULT;
					else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM))
						op.popup_delay_type = POPUP_DELAY_CUSTOM;
					else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYPERMANENT))
						op.popup_delay_type = POPUP_DELAY_PERMANENT;

					op.popup_timeout = GetDlgItemInt(hwndDlg,IDC_DELAY, NULL, FALSE);
					op.popup_bkg_color = SendDlgItemMessage(hwndDlg,IDC_BGCOLOR,CPM_GETCOLOUR,0,0);
					op.popup_text_color = SendDlgItemMessage(hwndDlg,IDC_TEXTCOLOR,CPM_GETCOLOUR,0,0);
					op.popup_use_win_colors = IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) != 0;
					op.popup_use_default_colors = IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS) != 0;

					ShowTestPopup(TranslateT("Test Contact"), TranslateT("Test description"), &op);

					break;
				}
			}
			break;
		}
	}

	return SaveOptsDlgProc(popupsControls, MAX_REGS(popupsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}


static void SetAllContactIcons(HWND hwndList)
{
	HANDLE hContact,hItem;

	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	do 
	{
		hItem=(HANDLE)SendMessage(hwndList,CLM_FINDCONTACT,(WPARAM)hContact,0);
		if(hItem) 
		{
			// Some Module can handle it?
			int i;
			for(i = 0; i < modules.size(); i++)
			{
				if (!(modules[i].flags & VOICE_CALL_CONTACT))
					continue;

				if (modules[i].is_protocol
					&& !CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM) hContact, (LPARAM) modules[i].name))
					continue;

				// Check if can call
				if ((modules[i].flags & VOICE_CALL_CONTACT_NEED_TEST) 
					&& !CallProtoService(modules[i].name, PS_VOICE_CALL_CONTACT_VALID, (WPARAM) hContact, FALSE))
					continue;

				// Oki, found a module that can handle it
				break;
			}

			if (i == modules.size())
			{
				SendMessage(hwndList,CLM_DELETEITEM,(WPARAM)hItem,0);
			}
			else
			{
				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(0,
					DBGetContactSettingWord(hContact, MODULE_NAME, "AutoAccept", AUTO_NOTHING) == AUTO_ACCEPT ? 1 : 0));
				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(1,
					DBGetContactSettingWord(hContact, MODULE_NAME, "AutoAccept", AUTO_NOTHING) == AUTO_DROP ? 2 : 0));
			}
		}
	} while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));
}


static void SetListGroupIcons(HWND hwndList,HANDLE hFirstItem,HANDLE hParentItem,int *groupChildCount)
{
	int typeOfFirst;
	int iconOn[2]={1,1};
	int childCount[2]={0,0},i;
	int iImage;
	HANDLE hItem,hChildItem;

	typeOfFirst=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);
	//check groups
	if(typeOfFirst==CLCIT_GROUP) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		hChildItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if(hChildItem) SetListGroupIcons(hwndList,hChildItem,hItem,childCount);
		for( i=0; i < MAX_REGS(iconOn); i++)
			if(iconOn[i] && SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,i)==0) iconOn[i]=0;
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}
	//check contacts
	if(typeOfFirst==CLCIT_CONTACT) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);
	while(hItem) {
		for ( i=0; i < MAX_REGS(iconOn); i++) {
			iImage=SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,i);
			if(iconOn[i] && iImage==0) iconOn[i]=0;
			if(iImage!=0xFF) childCount[i]++;
		}
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
	//set icons
	if (hParentItem != NULL) {
		for( i=0; i < MAX_REGS(iconOn); i++) {
			SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hParentItem,MAKELPARAM(i,childCount[i]?(iconOn[i]?i+1:0):0xFF));
			if(groupChildCount) groupChildCount[i]+=childCount[i];
		}
	}
}


static void SetAllChildIcons(HWND hwndList,HANDLE hFirstItem,int iColumn,int iImage)
{
	int typeOfFirst,iOldIcon;
	HANDLE hItem,hChildItem;

	typeOfFirst=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);
	//check groups
	if(typeOfFirst==CLCIT_GROUP) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		hChildItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if(hChildItem) SetAllChildIcons(hwndList,hChildItem,iColumn,iImage);
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}
	//check contacts
	if(typeOfFirst==CLCIT_CONTACT) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);
	while(hItem) {
		iOldIcon=SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,iColumn);
		if(iOldIcon!=0xFF && iOldIcon!=iImage) SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(iColumn,iImage));
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
}


static void ResetListOptions(HWND hwndList)
{
	int i;

	SendMessage(hwndList,CLM_SETBKBITMAP,0,(LPARAM)(HBITMAP)NULL);
	SendMessage(hwndList,CLM_SETBKCOLOR,GetSysColor(COLOR_WINDOW),0);
	SendMessage(hwndList,CLM_SETGREYOUTFLAGS,0,0);
	SendMessage(hwndList,CLM_SETLEFTMARGIN,2,0);
	SendMessage(hwndList,CLM_SETINDENT,10,0);
	for(i=0;i<=FONTID_MAX;i++)
		SendMessage(hwndList,CLM_SETTEXTCOLOR,i,GetSysColor(COLOR_WINDOWTEXT));
	SetWindowLong(hwndList,GWL_STYLE,GetWindowLong(hwndList,GWL_STYLE)|CLS_SHOWHIDDEN);
}


int ImageList_AddIcon_NotShared(HIMAGELIST hIml, HINSTANCE hInstance, LPCTSTR szResource) 
{   
    HICON hTempIcon=LoadIcon(hInstance, szResource);
    int res=ImageList_AddIcon(hIml, hTempIcon);
    DestroyIcon(hTempIcon); 
    return res;
}


static BOOL CALLBACK AutoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HICON hAnswerIcon,hDropIcon;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			{
				HIMAGELIST hIml;
				hIml=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),(IsWinVerXPPlus()?ILC_COLOR32:ILC_COLOR16)|ILC_MASK,3,3);
				ImageList_AddIcon_NotShared(hIml,GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_SMALLDOT));
				ImageList_AddIcon(hIml,icons[NUM_STATES + ACTION_ANSWER]);
				ImageList_AddIcon(hIml,icons[NUM_STATES + ACTION_DROP]);
				SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_SETEXTRAIMAGELIST,0,(LPARAM)hIml);
				hAnswerIcon=ImageList_GetIcon(hIml,1,ILD_NORMAL);
				SendDlgItemMessage(hwndDlg,IDC_ANSWER,STM_SETICON,(WPARAM)hAnswerIcon,0);
				hDropIcon=ImageList_GetIcon(hIml,2,ILD_NORMAL);
				SendDlgItemMessage(hwndDlg,IDC_DROP,STM_SETICON,(WPARAM)hDropIcon,0);
			}

			ResetListOptions(GetDlgItem(hwndDlg,IDC_LIST));
			SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_SETEXTRACOLUMNS,2,0);

			SetAllContactIcons(GetDlgItem(hwndDlg,IDC_LIST));
			SetListGroupIcons(GetDlgItem(hwndDlg,IDC_LIST),(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETNEXTITEM,CLGN_ROOT,0),NULL,NULL);
			return TRUE;
		}
		case WM_SETFOCUS:
			SetFocus(GetDlgItem(hwndDlg,IDC_LIST));
			break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case IDC_LIST:
					switch (((LPNMHDR)lParam)->code)
					{
						case CLN_NEWCONTACT:
						case CLN_LISTREBUILT:
							SetAllContactIcons(GetDlgItem(hwndDlg,IDC_LIST));
							//fall through
						case CLN_CONTACTMOVED:
							SetListGroupIcons(GetDlgItem(hwndDlg,IDC_LIST),(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETNEXTITEM,CLGN_ROOT,0),NULL,NULL);
							break;
						case CLN_OPTIONSCHANGED:
							ResetListOptions(GetDlgItem(hwndDlg,IDC_LIST));
							break;
						case NM_CLICK:
						{	HANDLE hItem;
							NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
							DWORD hitFlags;
							int iImage;
							int itemType;

							// Make sure we have an extra column
							if (nm->iColumn == -1)
								break;

							// Find clicked item
							hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x,nm->pt.y));
							// Nothing was clicked
							if (hItem == NULL) break; 
							// It was not a visbility icon
							if (!(hitFlags & CLCHT_ONITEMEXTRA)) break;

							// Get image in clicked column (0=none, 1=visible, 2=invisible)
							iImage = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
							if (iImage == 0)
								iImage=nm->iColumn + 1;
							else
								if (iImage == 1 || iImage == 2)
									iImage = 0;

							// Get item type (contact, group, etc...)
							itemType = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETITEMTYPE, (WPARAM)hItem, 0);

							// Update list, making sure that the options are mutually exclusive
							if (itemType == CLCIT_CONTACT) { // A contact
								SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, iImage));
								if (iImage && SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(nm->iColumn?0:1,0))!=0xFF)
									SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn?0:1, 0));
							}
							else if (itemType == CLCIT_GROUP) { // A group
								hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
								if (hItem) {
									SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn, iImage);
									if (iImage)
										SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn?0:1, 0);
								}
							}
							// Update the all/none icons
							SetListGroupIcons(GetDlgItem(hwndDlg, IDC_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), NULL, NULL);

							// Activate Apply button
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							break;
						}
					}
					break;
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{	HANDLE hContact,hItem;
							int set,i,iImage;

							hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
							do {
								hItem=(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_FINDCONTACT,(WPARAM)hContact,0);
								if(hItem) 
								{
									set=0;
									for(i=0;i<2;i++) {
										iImage=SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(i,0));
										if(iImage==i+1) {
											DBWriteContactSettingWord(hContact, MODULE_NAME, "AutoAccept", iImage==1 ? AUTO_ACCEPT : AUTO_DROP);
											set=1;
											break;
										}
									}
									if(!set) DBWriteContactSettingWord(hContact, MODULE_NAME, "AutoAccept", AUTO_NOTHING);
								}
							} while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));
							return TRUE;
						}
					}
					break;
			}
			break;
		case WM_DESTROY:
			DestroyIcon(hAnswerIcon);
			DestroyIcon(hDropIcon);
			{
				HIMAGELIST hIml=(HIMAGELIST)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGELIST,0,0);
				ImageList_Destroy(hIml);
			}
			break;
	}
	return FALSE;

}