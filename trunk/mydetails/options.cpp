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


#include "commons.h"
#include "options.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

Options opts;


static BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


// Functions //////////////////////////////////////////////////////////////////////////////////////


// Initializations needed by options
void LoadOptions()
{
	opts.cicle_throught_protocols = DBGetContactSettingByte(NULL,"MyDetails","CicleThroughtProtocols",1) == 1;
	opts.seconds_to_show_protocol = DBGetContactSettingWord(NULL,"MyDetails","CicleTime",5);
	opts.replace_smileys = DBGetContactSettingByte(NULL,"MyDetails","ReplaceSmileys",1) == 1;
	opts.resize_smileys = DBGetContactSettingByte(NULL,"MyDetails","ResizeSmileys",0) == 1;
	opts.use_contact_list_smileys = DBGetContactSettingByte(NULL,"MyDetails","UseContactListSmileys",0) == 1;

	opts.draw_show_protocol_name = DBGetContactSettingByte(NULL,"MyDetails","ShowProtocolName",1) == 1;

	opts.draw_avatar_custom_size = DBGetContactSettingByte(NULL,"MyDetails","AvatarCustomSize",0) == 1;
	opts.draw_avatar_custom_size_pixels = DBGetContactSettingWord(NULL,"MyDetails","AvatarCustomSizePixels",30);
	opts.draw_avatar_allow_to_grow = DBGetContactSettingByte(NULL,"MyDetails","AvatarAllowToGrow",0) == 1;

	opts.draw_avatar_border = DBGetContactSettingByte(NULL,"MyDetails","AvatarDrawBorders",1) == 1 ;
	opts.draw_avatar_border_color = (COLORREF) DBGetContactSettingDword(NULL,"MyDetails","AvatarBorderColor",RGB(0,0,0));

	opts.draw_avatar_round_corner = DBGetContactSettingByte(NULL,"MyDetails","AvatarRoundCorners",1) == 1;
	opts.draw_avatar_use_custom_corner_size = DBGetContactSettingByte(NULL,"MyDetails","AvatarUseCustomCornerSize",0) == 1;
	opts.draw_avatar_custom_corner_size = DBGetContactSettingWord(NULL,"MyDetails","AvatarCustomCornerSize",4);

	opts.borders[TOP] = DBGetContactSettingWord(NULL,"MyDetails","BorderTop",8);
	opts.borders[LEFT] = DBGetContactSettingWord(NULL,"MyDetails","BorderLeft",8);
	opts.borders[BOTTOM] = DBGetContactSettingWord(NULL,"MyDetails","BorderBottom",8);
	opts.borders[RIGHT] = DBGetContactSettingWord(NULL,"MyDetails","BorderRight",8);

	opts.use_avatar_space_to_draw_text = DBGetContactSettingByte(NULL,"MyDetails","AvatarUseFreeSpaceToDrawText",1) == 1;
	opts.resize_frame = DBGetContactSettingByte(NULL,"MyDetails","ResizeFrame",0) == 1;

	opts.draw_text_rtl = DBGetContactSettingByte(NULL,"MyDetails","TextRTL",0) == 1;
	opts.draw_text_align_right = DBGetContactSettingByte(NULL,"MyDetails","TextAlignRight",0) == 1;

	opts.global_on_avatar = DBGetContactSettingByte(NULL,"MyDetails","GlobalOnAvatar",0) == 1;
	opts.global_on_nickname = DBGetContactSettingByte(NULL,"MyDetails","GlobalOnNickname",0) == 1;
	opts.global_on_status = DBGetContactSettingByte(NULL,"MyDetails","GlobalOnStatus",0) == 1;
	opts.global_on_status_message = DBGetContactSettingByte(NULL,"MyDetails","GlobalOnStatusMessage",0) == 1;

	// This is created here to assert that this key always exists
	opts.refresh_status_message_timer = DBGetContactSettingWord(NULL,"MyDetails","RefreshStatusMessageTimer",12);
	DBWriteContactSettingWord(NULL,"MyDetails","RefreshStatusMessageTimer", opts.refresh_status_message_timer);


	SetCicleTime();
	RefreshFrameAndCalcRects();
}

int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=-200000000;
	odp.hInstance=hInst;
    odp.pfnDlgProc=DlgProcOpts;
    odp.pszTemplate=MAKEINTRESOURCE(IDD_OPTS);
    odp.pszGroup=Translate("Customize");
    odp.pszTitle=Translate("My Details");
    odp.flags=ODPF_BOLDGROUPS;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


void InitOptions()
{
	LoadOptions();

	HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}

// Deinitializations needed by options
void DeInitOptions()
{
}


static OptPageControl pageControls[] = { 
	{ NULL, CONTROL_CHECKBOX, IDC_CICLE_THROUGHT_PROTOS, "CicleThroughtProtocols", (BYTE) 1 },
	{ NULL, CONTROL_SPIN,		IDC_CICLE_TIME, "CicleTime", (WORD) 5, IDC_CICLE_TIME_SPIN, (WORD) 1, (WORD) 255 },
	{ NULL, CONTROL_CHECKBOX, IDC_SHOW_PROTO_NAME, "ShowProtocolName", (BYTE) 1 },
	{ NULL, CONTROL_CHECKBOX, IDC_TEXT_RTL, "TextRTL", (BYTE) 0 },
	{ NULL, CONTROL_CHECKBOX, IDC_TEXT_ALIGN_RIGHT, "TextAlignRight", (BYTE) 0 },
	{ NULL, CONTROL_CHECKBOX, IDC_REPLACE_SMILEYS, "ReplaceSmileys", (BYTE) 1 },
	{ NULL, CONTROL_CHECKBOX, IDC_RESIZE_SMILEYS, "ResizeSmileys", (BYTE) 0 },
	{ NULL, CONTROL_CHECKBOX, IDC_USE_CONTACT_LIST_SMILEYS, "UseContactListSmileys", (BYTE) 0 },
	{ NULL, CONTROL_CHECKBOX, IDC_GLOBAL_ON_AVATAR, "GlobalOnAvatar", (BYTE) 0 },
	{ NULL, CONTROL_CHECKBOX, IDC_GLOBAL_ON_NICKNAME, "GlobalOnNickname", (BYTE) 0 },
	{ NULL, CONTROL_CHECKBOX, IDC_GLOBAL_ON_STATUS, "GlobalOnStatus", (BYTE) 0 },
	{ NULL, CONTROL_CHECKBOX, IDC_GLOBAL_ON_STATUS_MESSAGE, "GlobalOnStatusMessage", (BYTE) 0 },
	{ NULL, CONTROL_CHECKBOX, IDC_AVATAR_ALLOW_TO_GROW, "AvatarAllowToGrow", (BYTE) 0 },
	{ NULL, CONTROL_CHECKBOX, IDC_AVATAR_CUSTOM_SIZE_CHK, "AvatarCustomSize", (BYTE) 0 },
	{ NULL, CONTROL_SPIN,		IDC_AVATAR_CUSTOM_SIZE, "AvatarCustomSizePixels", (WORD) 30, IDC_AVATAR_CUSTOM_SIZE_SPIN, (WORD) 1, (WORD) 255 },
	{ NULL, CONTROL_CHECKBOX, IDC_AVATAR_DRAW_BORDER, "AvatarDrawBorders", (BYTE) 0 },
	{ NULL, CONTROL_COLOR,	IDC_AVATAR_BORDER_COLOR, "AvatarBorderColor", (DWORD) RGB(0,0,0) },
	{ NULL, CONTROL_CHECKBOX, IDC_AVATAR_ROUND_CORNERS, "AvatarRoundCorners", (BYTE) 1 },
	{ NULL, CONTROL_CHECKBOX, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK, "AvatarUseCustomCornerSize", (BYTE) 0 },
	{ NULL, CONTROL_SPIN,		IDC_AVATAR_CUSTOM_CORNER_SIZE, "AvatarCustomCornerSize", (WORD) 4, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN, (WORD) 1, (WORD) 255 },
	{ NULL, CONTROL_CHECKBOX, IDC_AVATAR_USE_FREE_SPACE, "AvatarUseFreeSpaceToDrawText", (BYTE) 1 },
	{ NULL, CONTROL_CHECKBOX, IDC_RESIZE_FRAME, "ResizeFrame", (BYTE) 0 },
	{ NULL, CONTROL_SPIN,		IDC_BORDER_RIGHT, "BorderRight", (WORD) 8, IDC_BORDER_RIGHT_SPIN, (WORD) 0, (WORD) 255 },
	{ NULL, CONTROL_SPIN,		IDC_BORDER_LEFT, "BorderLeft", (WORD) 8, IDC_BORDER_LEFT_SPIN, (WORD) 0, (WORD) 255 },
	{ NULL, CONTROL_SPIN,		IDC_BORDER_TOP, "BorderTop", (WORD) 8, IDC_BORDER_TOP_SPIN, (WORD) 0, (WORD) 255 },
	{ NULL, CONTROL_SPIN,		IDC_BORDER_BOTTOM, "BorderBottom", (WORD) 8, IDC_BORDER_BOTTOM_SPIN, (WORD) 0, (WORD) 255 },
	{ NULL, CONTROL_COLOR,	IDC_AVATAR_BKG_COLOR, "BackgroundColor", (DWORD) GetSysColor(COLOR_BTNFACE) }
};

static BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL ret = SaveOptsDlgProc(pageControls, MAX_REGS(pageControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			if(!IsDlgButtonChecked(hwndDlg,IDC_AVATAR_DRAW_BORDER)) 
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_AVATAR_BORDER_COLOR_L),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_AVATAR_BORDER_COLOR),FALSE);
			}
			if(!IsDlgButtonChecked(hwndDlg,IDC_AVATAR_ROUND_CORNERS)) 
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_AVATAR_CUSTOM_CORNER_SIZE),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN),FALSE);
			}
			if (!ServiceExists(MS_SMILEYADD_BATCHPARSE))
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_REPLACE_SMILEYS),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_USE_CONTACT_LIST_SMILEYS),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_RESIZE_SMILEYS),FALSE);
			}
			if (!ServiceExists(MS_CLIST_FRAMES_SETFRAMEOPTIONS))
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_RESIZE_FRAME),FALSE);
			}

			break;
		}
		case WM_COMMAND:
		{
			if (LOWORD(wParam)==IDC_AVATAR_DRAW_BORDER)
			{
				BOOL enabled = IsDlgButtonChecked(hwndDlg,IDC_AVATAR_DRAW_BORDER);
				EnableWindow(GetDlgItem(hwndDlg,IDC_AVATAR_BORDER_COLOR_L),enabled);
				EnableWindow(GetDlgItem(hwndDlg,IDC_AVATAR_BORDER_COLOR),enabled);
			}

			if (LOWORD(wParam)==IDC_AVATAR_ROUND_CORNERS)
			{
				BOOL enabled = IsDlgButtonChecked(hwndDlg,IDC_AVATAR_ROUND_CORNERS);
				EnableWindow(GetDlgItem(hwndDlg,IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK),enabled);
				EnableWindow(GetDlgItem(hwndDlg,IDC_AVATAR_CUSTOM_CORNER_SIZE),enabled);
				EnableWindow(GetDlgItem(hwndDlg,IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN),enabled);
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
