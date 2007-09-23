/*

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

#include "simpleaway.h"

#define I_ICON_DEL		0
#define I_ICON_HIST		1
#define I_ICON_MSG		2
#define I_ICON_ADD		3
#define I_ICON_CLEAR 	4

struct MsgBoxData
{
	char	*proto_name;
	int		status_mode;
	int		initial_status_mode;
	int		all_modes;
	int		all_modes_msg;
	HWND	status_cbex;
	HWND	recent_cbex;
	HWND	bclear;
	HWND	badd;
	HWND	bdel;
	HICON	icon[5];
	HIMAGELIST status_icons;
	HIMAGELIST other_icons;
	int		countdown;
	int		curr_sel_msg;
	int		max_hist_msgs;
	int		dlg_flags;
	int		status_flags;
	int		num_def_msgs;
	BOOL	predef_changed;
	BOOL	is_history;
	BOOL	variables_v;
};

HIMAGELIST AddOtherIconsToImageList(struct MsgBoxData *data)
{ 
    HIMAGELIST	himlIcons;
    int			i;

	if (IsWinVerXPPlus())
		himlIcons = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK, 4, 0); 
	else
		himlIcons = ImageList_Create(16, 16, ILC_COLOR16|ILC_MASK, 4, 0); 

	for (i=0; i<5; i++)
		ImageList_AddIcon(himlIcons, data->icon[i]);

    return himlIcons; 
}

HIMAGELIST AddStatusIconsToImageList(char *proto_name, int status_flags)
{ 
    HIMAGELIST	himlIcons;
    HICON		hicon;
	int			i, num_icons=1;

	for (i=0; i<9; i++)
	{
		if (Proto_Status2Flag(ID_STATUS_ONLINE+i) & status_flags)
			num_icons++;
	}

	if (IsWinVerXPPlus())
		himlIcons = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK, num_icons, 0); 
	else
		himlIcons = ImageList_Create(16, 16, ILC_COLOR16|ILC_MASK, num_icons, 0); 

	hicon = LoadSkinnedProtoIcon(proto_name, ID_STATUS_OFFLINE);
	ImageList_AddIcon(himlIcons, hicon); 

	for (i=0; i<9; i++)
	{
		if (Proto_Status2Flag(ID_STATUS_ONLINE+i) & status_flags)
		{
			hicon = LoadSkinnedProtoIcon(proto_name, ID_STATUS_ONLINE+i);
			ImageList_AddIcon(himlIcons, hicon); 
		}
	} 
    return himlIcons; 
}

HWND WINAPI CreateStatusComboBoxEx(HWND HwndMain, struct MsgBoxData *data)
{
	HWND			handle;
	COMBOBOXEXITEM	cbei;
	int				i,j=0, cur_sel=0;
	char			*status_desc;

	if (!(data->dlg_flags & DLG_SHOW_STATUS))
		return NULL;

	handle = CreateWindowEx(0, WC_COMBOBOXEX, NULL,
					WS_TABSTOP | CBS_NOINTEGRALHEIGHT | WS_VISIBLE |
					WS_CHILD | CBS_DROPDOWNLIST,
					0,// x
					0,// y
					0,// width
					240,// height
					HwndMain,
					NULL,
					hInst,
					NULL);


	if (!(data->dlg_flags & DLG_SHOW_STATUS_ICONS))
		cbei.mask = CBEIF_LPARAM | CBEIF_TEXT;
	else
		cbei.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;

	for(i=0; i<10; i++)
	{
		if ((Proto_Status2Flag(ID_STATUS_OFFLINE+i) & data->all_modes) || i == 0)
		{
			status_desc = (char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, ID_STATUS_OFFLINE+i, 0);
			cbei.iItem          = j;
			cbei.pszText        = (LPTSTR)status_desc;
			cbei.cchTextMax     = sizeof(status_desc);
			if (data->dlg_flags & DLG_SHOW_STATUS_ICONS)
			{
				cbei.iImage         = j;
				cbei.iSelectedImage = j;
			}
			cbei.lParam			= (LPARAM)ID_STATUS_OFFLINE+i;

			if (ID_STATUS_OFFLINE+i == data->initial_status_mode)
				cur_sel = j;

        if(SendMessage(handle,CBEM_INSERTITEM,0,(LPARAM)&cbei) == -1)
            break;
		j++;
		}
    }

	if (!(data->dlg_flags & DLG_SHOW_STATUS_ICONS))
		SendMessage(handle, CB_SETITEMHEIGHT, (WPARAM)0, (LPARAM)16);
	else
	{
		SendMessage(handle, CB_SETITEMHEIGHT, (WPARAM)0, (LPARAM)18);
		SendMessage(handle, CBEM_SETIMAGELIST, 0, (LPARAM)data->status_icons);
	}
	SetWindowPos(handle, NULL, 11, 11, 112, 20, SWP_NOACTIVATE);
	SendMessage(handle, CB_SETCURSEL, (WPARAM)cur_sel, 0);
	SendMessage(handle, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)16);
    return (handle);
}

#define HISTORY_MSG		1
#define CLEAR_HISTORY	2
#define PREDEFINED_MSG	3
#define ADD_MSG			4
#define DELETE_SELECTED	5
#define DEFAULT_MSG		6

HWND WINAPI CreateRecentComboBoxEx(HWND HwndMain, struct MsgBoxData *data)
{
	HWND			handle;
	COMBOBOXEXITEM	cbei;
	int				i, j;
	char			buff[16];
	BOOL			found=FALSE;
	DBVARIANT		dbv;
	char			text[128];

	handle = CreateWindowEx(0, WC_COMBOBOXEX, NULL,
					WS_TABSTOP | CBS_NOINTEGRALHEIGHT | WS_VISIBLE |
					WS_CHILD | CBS_DROPDOWNLIST,
					0,// x
					0,// y
					0,// width
					300,// height
					HwndMain,
					NULL,
					hInst,
					NULL);

	if (!(data->dlg_flags & DLG_SHOW_LIST_ICONS))
		cbei.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_INDENT;
	else
		cbei.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;

	j = DBGetContactSettingWord(NULL, "SimpleAway", "LMMsg", 1);

	for(i=1; i<=data->max_hist_msgs; i++) //history MSGS
	{
		if (j<1)
			j = data->max_hist_msgs;
		_snprintf(buff, sizeof(buff), "SMsg%d", j);
		j--;
		if (!DBGetContactSetting(NULL, "SimpleAway", buff, &dbv)) //0 - no error
		{
			if (dbv.pszVal)
			{
				if (!strlen(dbv.pszVal))
				{
					DBFreeVariant(&dbv);
					continue;
				}

				found = TRUE;
				cbei.iItem          = -1;
				cbei.pszText        = (LPTSTR)dbv.pszVal;
				cbei.cchTextMax     = sizeof(dbv.pszVal);
				if (data->dlg_flags & DLG_SHOW_LIST_ICONS)
				{
					cbei.iImage         = I_ICON_HIST;
					cbei.iSelectedImage = I_ICON_HIST;
				}
				else
					cbei.iIndent        = 0;
				cbei.lParam			= MAKELPARAM(HISTORY_MSG, j+1);

				if(SendMessage(handle,CBEM_INSERTITEM,0,(LPARAM)&cbei) == -1)
				{
					DBFreeVariant(&dbv);
					break;
				}
			}
			DBFreeVariant(&dbv);
		}
    }

	data->is_history = found;

	if (data->dlg_flags & DLG_SHOW_BUTTONS)
	{
		if (found)
			EnableWindow(data->bclear, TRUE);
		else
			EnableWindow(data->bclear, FALSE);
	}
	else if (data->dlg_flags & DLG_SHOW_BUTTONS_INLIST)
	{
		if (found)
		{
			if (data->dlg_flags & DLG_SHOW_LIST_ICONS)
			{
				_snprintf(text, sizeof(text), Translate("Clear History"));
				cbei.iImage         = I_ICON_CLEAR;
				cbei.iSelectedImage = I_ICON_CLEAR;
			}
			else
			{
				_snprintf(text, sizeof(text), "## %s ##", Translate("Clear History"));
				cbei.iIndent        = 1;
			}
			cbei.iItem          = -1;
			cbei.pszText        = (LPTSTR)text;
			cbei.cchTextMax     = sizeof(text);
			cbei.lParam			= MAKELPARAM(CLEAR_HISTORY, 0);
			SendMessage(handle,CBEM_INSERTITEM,0,(LPARAM)&cbei);
		}

		cbei.iItem          = -1;
		if (data->dlg_flags & DLG_SHOW_LIST_ICONS)
		{
			_snprintf(text, sizeof(text), Translate("Add to Predefined"));
			cbei.iImage         = I_ICON_ADD;
			cbei.iSelectedImage = I_ICON_ADD;
		}
		else
		{
			_snprintf(text, sizeof(text), "## %s ##", Translate("Add to Predefined"));
			cbei.iIndent        = 1;
		}
		cbei.pszText        = (LPTSTR)text;
		cbei.cchTextMax     = sizeof(text);
		cbei.lParam			= MAKELPARAM(ADD_MSG, 0);
		SendMessage(handle,CBEM_INSERTITEM,0,(LPARAM)&cbei);

		if (data->dlg_flags & DLG_SHOW_LIST_ICONS)
		{
			_snprintf(text, sizeof(text), Translate("Delete Selected"));
			cbei.iImage         = I_ICON_DEL;
			cbei.iSelectedImage = I_ICON_DEL;
		}
		else
		{
			cbei.iIndent        = 1;
			_snprintf(text, sizeof(text), "## %s ##", Translate("Delete Selected"));
		}
		cbei.iItem          = -1;
		cbei.pszText        = (LPTSTR)text;
		cbei.cchTextMax     = sizeof(text);
		cbei.lParam			= MAKELPARAM(DELETE_SELECTED, 0);
		SendMessage(handle,CBEM_INSERTITEM,0,(LPARAM)&cbei);
	}

	if (data->dlg_flags & DLG_SHOW_BUTTONS)
	{
		if (data->num_def_msgs || found)
			EnableWindow(data->bdel, TRUE);
		else
			EnableWindow(data->bdel, FALSE);
	}

	for(i=1; i<=data->num_def_msgs; i++) //predefined MSGS
	{
		_snprintf(buff, sizeof(buff), "DefMsg%d", i);
		if (!DBGetContactSetting(NULL, "SimpleAway", buff, &dbv)) //0 - no error
		{
			if (dbv.pszVal)
			{
				if (!strlen(dbv.pszVal))
				{
					DBFreeVariant(&dbv);
					continue;
				}

				cbei.iItem          = -1;
				cbei.pszText        = (LPTSTR)dbv.pszVal;
				cbei.cchTextMax     = sizeof(dbv.pszVal);
				if (data->dlg_flags & DLG_SHOW_LIST_ICONS)
				{
					cbei.iImage         = I_ICON_MSG;
					cbei.iSelectedImage = I_ICON_MSG;
				}
				else
					cbei.iIndent        = 0;
				cbei.lParam			= MAKELPARAM(PREDEFINED_MSG, i);

				if(SendMessage(handle,CBEM_INSERTITEM,0,(LPARAM)&cbei) == -1)
					break;
			}
			DBFreeVariant(&dbv);
		}
    }

	if (data->status_flags & STATUS_PUT_DEF_IN_LIST)
	{
		cbei.iItem          = -1;
		cbei.pszText        = (LPTSTR)GetDefaultMessage(data->status_mode);
		cbei.cchTextMax     = sizeof(GetDefaultMessage(data->status_mode));
		if (data->dlg_flags & DLG_SHOW_LIST_ICONS)
		{
			cbei.iImage         = I_ICON_MSG;
			cbei.iSelectedImage = I_ICON_MSG;
		}
		else
			cbei.iIndent        = 0;
		cbei.lParam			= MAKELPARAM(DEFAULT_MSG, 0);
		
		SendMessage(handle,CBEM_INSERTITEM,0,(LPARAM)&cbei);
	}

	if (data->dlg_flags & DLG_SHOW_LIST_ICONS)
		SendMessage(handle, CBEM_SETIMAGELIST, 0, (LPARAM)data->other_icons);
	if (!(data->dlg_flags & DLG_SHOW_STATUS))
	{
		SetWindowPos(handle, NULL, 11, 11, 290, 20, SWP_NOACTIVATE);
		SendMessage(handle, CB_SETDROPPEDWIDTH, (WPARAM)290, 0);
	}
	else
	{
		SetWindowPos(handle, NULL, 127, 11, 174, 20, SWP_NOACTIVATE);
		SendMessage(handle, CB_SETDROPPEDWIDTH, (WPARAM)250, 0);
	}
	SendMessage(handle, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)16);
	SendMessage(handle, CB_SETITEMHEIGHT, (WPARAM)0, (LPARAM)16);

	if ((data->dlg_flags & DLG_SHOW_BUTTONS) && !found && !data->num_def_msgs)
		EnableWindow(handle, FALSE);

	if (((!(data->dlg_flags & DLG_SHOW_BUTTONS)) && (!(data->dlg_flags & DLG_SHOW_BUTTONS_INLIST))) && !found && !data->num_def_msgs)
		EnableWindow(handle, FALSE);

    return (handle);
}

WNDPROC MainDlgProc;

VOID APIENTRY HandlePopupMenu(HWND hwnd, POINT pt) 
{ 
	HMENU 		hmenu;         
	HMENU 		hmenuTrackPopup;
	LPDWORD		sel_s=NULL, sel_e=NULL;
	int 		m_selection;
	HWND		edit_control = GetDlgItem(GetParent(hwnd), IDC_EDIT1);
 
    hmenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_EDITMENU)); 
    if (hmenu == NULL) 
        return; 
 
    hmenuTrackPopup = GetSubMenu(hmenu, 0); 
 
 	CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)hmenuTrackPopup, 0);
 
	ClientToScreen(hwnd, (LPPOINT) &pt);  
 
	SendMessage(edit_control, EM_GETSEL, (WPARAM)&sel_s, (LPARAM)&sel_e);
	if (sel_s == sel_e)
	{
		EnableMenuItem(hmenuTrackPopup, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hmenuTrackPopup, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
	}
	if (SendMessage(edit_control, WM_GETTEXTLENGTH, 0, 0) == 0)
		EnableMenuItem(hmenuTrackPopup, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);

	if (!ServiceExists(MS_VARS_FORMATSTRING))
		DeleteMenu(hmenuTrackPopup, 6, MF_BYPOSITION);
	else
		DeleteMenu(hmenuTrackPopup, ID__VARIABLES, MF_BYCOMMAND);
	
	if (!ServiceExists(MS_FORTUNEMSG_GETSTATUSMSG))
		DeleteMenu(hmenuTrackPopup, 5, MF_BYPOSITION);
	else
		DeleteMenu(hmenuTrackPopup, ID__FORTUNEAWAYMSG, MF_BYCOMMAND);

	m_selection = TrackPopupMenu(hmenuTrackPopup, TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
 
 	switch(m_selection)
 	{
		case IDM_COPY:
			SendMessage(edit_control, WM_COPY, 0, 0);
		break;
		case IDM_CUT:
            SendMessage(edit_control, WM_CUT, 0, 0);
        break;
        case IDM_PASTE:
			SendMessage(edit_control, WM_PASTE, 0, 0);
		break;
		case IDM_SELECTALL:
			SendMessage(edit_control, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		break;
		case IDM_DELETE:
			SendMessage(edit_control, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_EDIT1, EN_CHANGE), (LPARAM)edit_control);
		break;
		case ID__FORTUNEAWAYMSG:
			CallService(MS_UTILS_OPENURL,1,(LPARAM)"http://www.miranda-im.org/download/details.php?action=viewfile&id=1933");
		break;
		case ID__VARIABLES:
			CallService(MS_UTILS_OPENURL,1,(LPARAM)"http://www.cs.vu.nl/~pboon/variables.zip");
		break;
		default:
		{
			char	item_string[128];
			int		len;

			if (OpenClipboard(GetParent(hwnd)))
			{
				if (EmptyClipboard())
				{
					HGLOBAL hglbCopy;
					LPTSTR  lptstrCopy;

					GetMenuString(hmenu, m_selection, (LPTSTR)&item_string, 128, MF_BYCOMMAND);
			
					len = lstrlen(item_string);
					if (len)
					{
						hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(TCHAR)); 
						if (hglbCopy == NULL)
						{ 
							CloseClipboard(); 
							break; 
						}
						lptstrCopy = GlobalLock(hglbCopy); 
						memcpy(lptstrCopy, item_string, len*sizeof(TCHAR)); 
						lptstrCopy[len] = (TCHAR)0;    // null character 
						GlobalUnlock(hglbCopy);
						SetClipboardData(CF_TEXT, hglbCopy);
					}
				}
				CloseClipboard();
				SendMessage(edit_control, WM_PASTE, 0, 0);
			}
		}
		break;
	}
    DestroyMenu(hmenu); 
}

#ifndef GET_X_LPARAM
#define 	GET_X_LPARAM(lp)   ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define 	GET_Y_LPARAM(lp)   ((int)(short)HIWORD(lp))
#endif

#define DM_SIMPAWAY_SHUTDOWN WM_USER+10
#define DM_SIMPAWAY_CHANGEICONS WM_USER+11

void ChangeDlgStatus(HWND hwndDlg, struct MsgBoxData *msgbox_data, int status);

LRESULT CALLBACK EditBoxSubProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct MsgBoxData *msgbox_data;
	int	status = 0;

	msgbox_data=(struct MsgBoxData*)GetWindowLong(GetParent(hwndDlg),GWL_USERDATA);
	
	switch(uMsg)
	{
        case WM_RBUTTONUP:
 		{
			RECT rc;             
			POINT pt;
 
			GetClientRect(hwndDlg, (LPRECT) &rc); 
 
			pt.x = GET_X_LPARAM(lParam); 
			pt.y = GET_Y_LPARAM(lParam); 
 
			if (PtInRect((LPRECT) &rc, pt)) 
				HandlePopupMenu(hwndDlg, pt);
                
			return 0;
		}
        break; 
		case WM_CHAR:
			{
				if (wParam=='\n' && GetKeyState(VK_CONTROL)&0x8000)
				{
					PostMessage(GetParent(hwndDlg), WM_COMMAND, IDC_OK, 0);
					return 0;
				}

/*				if (GetKeyState(VK_CONTROL)&0x8000)
				{	
					switch (wParam)
					{
						case '1': status = ID_STATUS_ONLINE; break;
						case '2': status = ID_STATUS_AWAY; break;
						case '3': status = ID_STATUS_NA; break;
						case '4': status = ID_STATUS_OCCUPIED; break;
						case '5': status = ID_STATUS_DND; break;
						case '6': status = ID_STATUS_FREECHAT; break;
						case '7': status = ID_STATUS_INVISIBLE; break;
						case '8': status = ID_STATUS_ONTHEPHONE; break;
						case '9': status = ID_STATUS_OUTTOLUNCH; break;
					}
					if (status)

						if (status & msgbox_data->all_modes)
							ChangeDlgStatus(GetParent(hwndDlg), msgbox_data, status);
						return 0;		
					}
				}*/
			}
		break;
	}

	return CallWindowProc(MainDlgProc, hwndDlg, uMsg, wParam, lParam);
}

int AddToPredefined(HWND hwndDlg, struct MsgBoxData *data)
{
	COMBOBOXEXITEM	newitem;
	int				len=0, sel;
	char			msg[1024];

	if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1)))
		len = GetDlgItemText(hwndDlg,IDC_EDIT1,msg,sizeof(msg));

	if (!len)
		return -1;

	data->num_def_msgs++;
	data->predef_changed = TRUE;

	if (data->dlg_flags & DLG_SHOW_LIST_ICONS)
		newitem.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
	else
		newitem.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_INDENT;

	newitem.iItem = -1;
	newitem.pszText	= (LPTSTR)msg;
	newitem.cchTextMax = sizeof(msg);
	if (data->dlg_flags & DLG_SHOW_LIST_ICONS)
	{
		newitem.iImage			= I_ICON_MSG;
		newitem.iSelectedImage	= I_ICON_MSG;
	}
	else
		newitem.iIndent			= 0;

	newitem.lParam = MAKELPARAM(PREDEFINED_MSG, 0);
	sel = SendMessage(data->recent_cbex,CBEM_INSERTITEM,0,(LPARAM)&newitem);
	return (int)sel;
}

void ClearHistory(struct MsgBoxData *data, int cur_sel)
{
	COMBOBOXEXITEM	histitem;
	int				i, num_items;
	char			text[16];

	for (i=1; i<=data->max_hist_msgs; i++)
	{
		_snprintf(text, sizeof(text), "SMsg%d", i);
		DBWriteContactSettingString(NULL, "SimpleAway", text, "");
	}
	DBWriteContactSettingString(NULL, "SimpleAway", "LastMsg", "");
	DBWriteContactSettingWord(NULL, "SimpleAway", "LMMsg", (WORD)data->max_hist_msgs);
	SendMessage(data->recent_cbex, CB_SETCURSEL, -1, 0);
	num_items = SendMessage(data->recent_cbex, CB_GETCOUNT, 0, 0);
	if (num_items == CB_ERR)
		return;

	if (!(data->dlg_flags & DLG_SHOW_BUTTONS))
		SendMessage(data->recent_cbex, CBEM_DELETEITEM, (WPARAM)cur_sel, 0);

	for (i = num_items;i>=0;i--)
	{
		histitem.mask = CBEIF_LPARAM;
		histitem.iItem = i;

		SendMessage(data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&histitem);

		if (LOWORD(histitem.lParam) == HISTORY_MSG)
		{
			SendMessage(data->recent_cbex, CBEM_DELETEITEM, (WPARAM)i, 0);
		}
	}
}

void DisplayCharsCount(struct MsgBoxData *dlg_data, HWND hwndDlg)
{
	char	msg[1024];
	char	status_text[128];
	int		len, lines=1;

	if (dlg_data->countdown != -2)
		return;

	len = GetDlgItemText(hwndDlg, IDC_EDIT1, msg, sizeof(msg));

	if (removeCR)
	{
		int	i, index, num_lines;
											
		num_lines = SendMessage(GetDlgItem(hwndDlg, IDC_EDIT1), EM_GETLINECOUNT, 0, 0);
		for (i=0; i<num_lines; i++)
		{
			if (i < 1)
				continue;
				
			index = SendMessage(GetDlgItem(hwndDlg, IDC_EDIT1), EM_LINEINDEX, (WPARAM)i, 0);
			if (msg[index-1] == '\n')
				lines++;
		}
	}
	_snprintf(status_text, sizeof(status_text), Translate("OK (%d)"), len-(lines-1));
	SendMessage(GetDlgItem(hwndDlg, IDC_OK), WM_SETTEXT, (WPARAM)0, (LPARAM)status_text);
}

void SetEditControlText(struct MsgBoxData *data, HWND hwndDlg, int status_mode)
{
	int			flags, lines=1;
	DBVARIANT	dbv,dbv2;

	flags = DBGetContactSettingByte(NULL, "SimpleAway", (char *)StatusModeToDbSetting(status_mode, "Flags"), STATUS_SHOW_DLG|STATUS_LAST_MSG);
	if (flags & STATUS_LAST_MSG)
	{
		if (!DBGetContactSetting(NULL, "SimpleAway", "LastMsg", &dbv))
		{
			if (dbv.pszVal)
			{
				if (!DBGetContactSetting(NULL, "SimpleAway", dbv.pszVal, &dbv2) && strlen(dbv.pszVal))
				{
					if (dbv2.pszVal)
					{
						if (strlen(dbv2.pszVal))
						{
							SetDlgItemText(hwndDlg, IDC_EDIT1, dbv2.pszVal);
						}
					}
					DBFreeVariant(&dbv2);
				}
			}
			DBFreeVariant(&dbv);
		}
	}
	else if (flags & STATUS_DEFAULT_MSG)
	{
		SetDlgItemText(hwndDlg, IDC_EDIT1, GetDefaultMessage(status_mode));
	}
	else if (flags & STATUS_THIS_MSG)
	{
		if (!DBGetContactSetting(NULL, "SRAway", StatusModeToDbSetting(status_mode, "Default"), &dbv))
		{
			SetDlgItemText(hwndDlg, IDC_EDIT1, dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	else if (flags & STATUS_LAST_STATUS_MSG)
	{
		if (!DBGetContactSetting(NULL, "SRAway", StatusModeToDbSetting(status_mode, "Msg"), &dbv))
		{
			SetDlgItemText(hwndDlg, IDC_EDIT1, dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
}

void ChangeDlgStatus(HWND hwndDlg, struct MsgBoxData *msgbox_data, int status)
{
	char			title[256];
	
	_snprintf(title, sizeof(title), Translate("%s Status Message: %s"), msgbox_data->proto_name ? msgbox_data->proto_name : Translate("Global"), (char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, status, 0));
	SetWindowText(hwndDlg, title);
	SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) LoadSkinnedProtoIcon(msgbox_data->proto_name, status)); //Set the window icon

	if ((Proto_Status2Flag(status) & msgbox_data->all_modes_msg) || (status == ID_STATUS_OFFLINE && (Proto_Status2Flag(ID_STATUS_INVISIBLE) & msgbox_data->all_modes_msg)))
	{
		int			num_items;
									
		num_items = SendMessage(msgbox_data->recent_cbex, CB_GETCOUNT, 0, 0);

		if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1)))
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), TRUE);
		if (!IsWindowEnabled(msgbox_data->recent_cbex) && (num_items))
			EnableWindow(msgbox_data->recent_cbex, TRUE);

		if (msgbox_data->dlg_flags & DLG_SHOW_BUTTONS)
		{
			if (!IsWindowEnabled(msgbox_data->badd))
				EnableWindow(msgbox_data->badd, TRUE);
			if (num_items)
			{
				if (!IsWindowEnabled(msgbox_data->bdel))
					EnableWindow(msgbox_data->bdel, TRUE);

				if (msgbox_data->is_history)
				{
					if (!IsWindowEnabled(msgbox_data->bclear))
						EnableWindow(msgbox_data->bclear, TRUE);
				}
			}
		}
	}
	else
	{
		if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1)))
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), FALSE);
		if (IsWindowEnabled(msgbox_data->recent_cbex))
			EnableWindow(msgbox_data->recent_cbex, FALSE);

		if (msgbox_data->dlg_flags & DLG_SHOW_BUTTONS)
		{
			if (IsWindowEnabled(msgbox_data->badd))
				EnableWindow(msgbox_data->badd, FALSE);
			if (IsWindowEnabled(msgbox_data->bclear))
				EnableWindow(msgbox_data->bclear, FALSE);
			if (IsWindowEnabled(msgbox_data->bdel))
				EnableWindow(msgbox_data->bdel, FALSE);
		}
	}
}

INT_PTR CALLBACK AwayMsgBoxDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct MsgBoxData *msgbox_data;

	msgbox_data=(struct MsgBoxData*)GetWindowLong(hwndDlg,GWL_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				char					title[256];
				char					format[256];
				struct					MsgBoxInitData *init_data;
				struct					MsgBoxData *copy_init_data;
				HICON					hIcon;
				INITCOMMONCONTROLSEX	icex;

				InitCommonControls();

				icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
				icex.dwICC = ICC_USEREX_CLASSES;

				InitCommonControlsEx(&icex);

				TranslateDialogDefault(hwndDlg);
				init_data = (struct MsgBoxInitData*)lParam;

				GetWindowText(hwndDlg, format, sizeof(format));
				_snprintf(title, sizeof(title), format, init_data->proto_name ? init_data->proto_name : Translate("Global"), (char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,init_data->status_mode,0));
				SetWindowText(hwndDlg, title); //Set the window title

				hIcon = LoadSkinnedProtoIcon(init_data->proto_name, init_data->status_mode);
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) hIcon); //Set the window icon

				copy_init_data = (struct MsgBoxData *) mir_alloc (sizeof(struct MsgBoxData));

				SendDlgItemMessage(hwndDlg, IDC_EDIT1, EM_LIMITTEXT, 1024, 0);

				HookEventMessage(ME_SYSTEM_PRESHUTDOWN, hwndDlg, DM_SIMPAWAY_SHUTDOWN);
				HookEventMessage(ME_SKIN2_ICONSCHANGED, hwndDlg, DM_SIMPAWAY_CHANGEICONS);

				copy_init_data->num_def_msgs = DBGetContactSettingWord(NULL, "SimpleAway", "DefMsgCount", 0);
				copy_init_data->max_hist_msgs = DBGetContactSettingByte(NULL, "SimpleAway", "MaxHist", 10);
				copy_init_data->dlg_flags = DBGetContactSettingByte(NULL, "SimpleAway", "DlgFlags", DLG_SHOW_STATUS|DLG_SHOW_STATUS_ICONS|DLG_SHOW_LIST_ICONS|DLG_SHOW_BUTTONS);
				copy_init_data->status_flags = DBGetContactSettingByte(NULL, "SimpleAway", (char *)StatusModeToDbSetting(init_data->status_mode, "Flags"), STATUS_SHOW_DLG|STATUS_LAST_MSG);
				copy_init_data->proto_name = init_data->proto_name;
				copy_init_data->status_mode = init_data->status_mode;
				copy_init_data->all_modes = init_data->all_modes;
				copy_init_data->all_modes_msg = init_data->all_modes_msg;
				copy_init_data->initial_status_mode = init_data->status_mode;

				//Load Icons
				if (ServiceExists(MS_SKIN2_GETICON))
				{
					int	i;
					
					for (i=0; i<5; i++)
						copy_init_data->icon[i] = (HICON)CallService(MS_SKIN2_GETICON, (WPARAM)0, (LPARAM)sa_ico_name[i]);
				}
				else
				{
					int	i;
					
					for (i=0; i<5; i++)
						copy_init_data->icon[i] = (HICON)LoadImage(hInst, MAKEINTRESOURCE(sa_ico_id[i]), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
				}

				if (copy_init_data->dlg_flags & DLG_SHOW_STATUS_ICONS)
					copy_init_data->status_icons = AddStatusIconsToImageList(init_data->proto_name, copy_init_data->all_modes);
				if (copy_init_data->dlg_flags & DLG_SHOW_LIST_ICONS)
					copy_init_data->other_icons = AddOtherIconsToImageList(copy_init_data);
					
				if ((copy_init_data->dlg_flags & DLG_SHOW_BUTTONS) || (copy_init_data->dlg_flags & DLG_SHOW_BUTTONS_FLAT))
				{
					copy_init_data->badd = CreateWindow(MIRANDABUTTONCLASS, "", WS_VISIBLE | WS_CHILD, 227, 115, 23, 23, hwndDlg, NULL, hInst, NULL);
					SendMessage(copy_init_data->badd, BUTTONADDTOOLTIP, (WPARAM)Translate("Add to Predefined"), 0);
					SendMessage(copy_init_data->badd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)copy_init_data->icon[I_ICON_ADD]);
					EnableWindow(copy_init_data->badd, TRUE);

					copy_init_data->bdel = CreateWindow(MIRANDABUTTONCLASS, "", WS_VISIBLE | WS_CHILD, 252, 115, 23, 23, hwndDlg, NULL, hInst, NULL);
					SendMessage(copy_init_data->bdel, BUTTONADDTOOLTIP, (WPARAM)Translate("Delete Selected"), 0);
					SendMessage(copy_init_data->bdel, BM_SETIMAGE, IMAGE_ICON, (LPARAM)copy_init_data->icon[I_ICON_DEL]);

					copy_init_data->bclear = CreateWindow(MIRANDABUTTONCLASS, "", WS_VISIBLE | WS_CHILD, 277, 115, 23, 23, hwndDlg, NULL, hInst, NULL);
					SendMessage(copy_init_data->bclear, BUTTONADDTOOLTIP, (WPARAM)Translate("Clear History"), 0);
					SendMessage(copy_init_data->bclear, BM_SETIMAGE, IMAGE_ICON, (LPARAM)copy_init_data->icon[I_ICON_CLEAR]);
					if (copy_init_data->dlg_flags & DLG_SHOW_BUTTONS_FLAT)
					{
						SendMessage(copy_init_data->badd, BUTTONSETASFLATBTN, 0, 0);
						SendMessage(copy_init_data->bdel, BUTTONSETASFLATBTN, 0, 0);
						SendMessage(copy_init_data->bclear, BUTTONSETASFLATBTN, 0, 0);
					}
				}
				else
				{
					SetWindowPos(GetDlgItem(hwndDlg, IDC_OK), NULL, 52, 115, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_CANCEL), NULL, 160, 115, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
				}
				copy_init_data->status_cbex = CreateStatusComboBoxEx(hwndDlg, copy_init_data);
				copy_init_data->recent_cbex = CreateRecentComboBoxEx(hwndDlg, copy_init_data);
				copy_init_data->curr_sel_msg = -1;
				copy_init_data->predef_changed = FALSE;
				copy_init_data->variables_v = FALSE;

				SetEditControlText(copy_init_data, hwndDlg, copy_init_data->status_mode);

				SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)copy_init_data);

				if (DBGetContactSettingByte(NULL, "SimpleAway", "AutoClose", 1))
				{
					copy_init_data->countdown = DBGetContactSettingByte(NULL, "SimpleAway", "DlgTime", 5);
					SendMessage(hwndDlg, WM_TIMER, 0, 0);
					SetTimer(hwndDlg, 1, 1000, 0);
				}
				else
				{
					copy_init_data->countdown = -2;
					DisplayCharsCount(copy_init_data, hwndDlg);
				}

				mir_free(init_data);

				MainDlgProc = (WNDPROC)SetWindowLong(GetDlgItem(hwndDlg, IDC_EDIT1), GWL_WNDPROC, (LONG)EditBoxSubProc);
				SetFocus(GetDlgItem(hwndDlg, IDC_OK));
				return FALSE;
			}
		break;
		case WM_TIMER:
			if(msgbox_data->countdown == -1)
			{
				SendMessage(hwndDlg, WM_COMMAND, (WPARAM)IDC_OK, 0);
				msgbox_data->countdown = -2;
				DisplayCharsCount(msgbox_data, hwndDlg);
				break;
			}
			else
			{	
				char str[64];
				_snprintf(str, sizeof(str), Translate("Closing in %d"), msgbox_data->countdown);
				SetDlgItemText(hwndDlg, IDC_OK, str);
			}
			msgbox_data->countdown--;
		break;
		case WM_COMMAND:
			{
				switch(LOWORD(wParam))
				{
					case IDC_OK:
						{
							char	msg[1024];
							int		len=0;

							if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1)))
								len = GetDlgItemText(hwndDlg, IDC_EDIT1, msg, sizeof(msg));

							if (len == 0)
							{
								DBWriteContactSettingString(NULL, "SimpleAway", "LastMsg", "");
								DBWriteContactSettingString(NULL, "SRAway", StatusModeToDbSetting(msgbox_data->status_mode,"Msg"), ""); //for compatibility with some plugins
								SetStatusMessage(msgbox_data->proto_name, msgbox_data->initial_status_mode, msgbox_data->status_mode, 0);
							}
							else
							{
								char		buff[64];
								int			i;
								DBVARIANT	dbv;
								BOOL		found=FALSE;

								for (i=1; i<=msgbox_data->max_hist_msgs; i++)
								{
									_snprintf(buff, sizeof(buff), "SMsg%d", i);
									if (!DBGetContactSetting(NULL, "SimpleAway", buff, &dbv))
									{
										if (!strcmp(dbv.pszVal, msg))
										{
											found = TRUE;
											DBWriteContactSettingString(NULL, "SimpleAway", "LastMsg", buff);
											DBFreeVariant(&dbv);
											break;
										}
									}
								}

								if (!found)
								{
									int	last_modified_msg;

									last_modified_msg = DBGetContactSettingWord(NULL, "SimpleAway", "LMMsg", msgbox_data->max_hist_msgs);
									if (last_modified_msg == msgbox_data->max_hist_msgs)
										last_modified_msg = 1;
									else
										last_modified_msg++;

									_snprintf(buff, sizeof(buff), "SMsg%d", last_modified_msg);
									DBWriteContactSettingString(NULL, "SimpleAway", buff, msg);
									DBWriteContactSettingString(NULL, "SimpleAway", "LastMsg", buff);
									DBWriteContactSettingWord(NULL, "SimpleAway", "LMMsg", (WORD)last_modified_msg);
								}
								DBWriteContactSettingString(NULL, "SRAway", StatusModeToDbSetting(msgbox_data->status_mode,"Msg"), msg); //for compatibility with some plugins

								SetStatusMessage(msgbox_data->proto_name, msgbox_data->initial_status_mode, msgbox_data->status_mode, msg);
							}
							DestroyWindow(hwndDlg);
						}
					break;
					case IDCANCEL:
					case IDC_CANCEL:
						{
							DestroyWindow(hwndDlg);
						}
					break;
					case IDC_EDIT1: //Notification from the edit control
						{
							if (msgbox_data->countdown > -2)
							{
								KillTimer(hwndDlg,1);
								msgbox_data->countdown = -2;
								DisplayCharsCount(msgbox_data, hwndDlg);
							}

							switch(HIWORD(wParam))
							{
								case EN_CHANGE:
									{
										DisplayCharsCount(msgbox_data, hwndDlg);
										SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, -1, 0);
									}
								break;
							}
						}
					break;
				}
				if((HWND)lParam == msgbox_data->status_cbex)
				{
					if (msgbox_data->countdown > -2)
					{
						KillTimer(hwndDlg,1);
						msgbox_data->countdown = -2;
						DisplayCharsCount(msgbox_data, hwndDlg);
					}
					switch(HIWORD(wParam))
					{
						case CBN_DBLCLK:
						case CBN_SELENDOK:
						case CBN_SELCHANGE:
							{
								int				cur_sel;
								COMBOBOXEXITEM	cbitem;

								cur_sel = SendMessage(msgbox_data->status_cbex, CB_GETCURSEL, 0, 0);
								cbitem.mask = CBEIF_LPARAM;
								cbitem.iItem = cur_sel;
								SendMessage(msgbox_data->status_cbex, CBEM_GETITEM, 0, (LPARAM)&cbitem);
								msgbox_data->status_mode = cbitem.lParam;
								ChangeDlgStatus(hwndDlg, msgbox_data, (int)cbitem.lParam);
							}
						break;
					}
				}
				if((HWND)lParam == msgbox_data->recent_cbex)
				{
					if (msgbox_data->countdown > -2)
					{
						KillTimer(hwndDlg,1);
						msgbox_data->countdown = -2;
						DisplayCharsCount(msgbox_data, hwndDlg);
					}
					switch(HIWORD(wParam))
					{
						case CBN_SELENDOK:
							{
								char			text[1024];
								char			buff[16];
								int				cur_sel;
								COMBOBOXEXITEM	cbitem;

								cur_sel = SendMessage(msgbox_data->recent_cbex, CB_GETCURSEL, 0, 0);

								cbitem.mask = CBEIF_LPARAM|CBEIF_TEXT;
								cbitem.iItem = cur_sel;
								cbitem.cchTextMax = sizeof(text);
								cbitem.pszText = text;

								SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&cbitem);
								if (LOWORD(cbitem.lParam) == HISTORY_MSG || LOWORD(cbitem.lParam) == PREDEFINED_MSG || LOWORD(cbitem.lParam) == DEFAULT_MSG)
								{
									SetDlgItemText(hwndDlg, IDC_EDIT1, text);
									DisplayCharsCount(msgbox_data, hwndDlg);
								}
								else if (LOWORD(cbitem.lParam) == CLEAR_HISTORY)
									ClearHistory(msgbox_data, cur_sel);
								else if (LOWORD(cbitem.lParam) == DELETE_SELECTED)
								{
									COMBOBOXEXITEM	histitem;

									histitem.mask = CBEIF_LPARAM;
									histitem.iItem = msgbox_data->curr_sel_msg;

									SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&histitem);

									if (LOWORD(histitem.lParam) == HISTORY_MSG)
									{
										_snprintf(buff, sizeof(buff), "SMsg%d", (int)HIWORD(histitem.lParam));
										DBWriteContactSettingString(NULL, "SimpleAway", buff, "");
										SendMessage(msgbox_data->recent_cbex, CBEM_DELETEITEM, (WPARAM)msgbox_data->curr_sel_msg, 0);
									}
									if (LOWORD(histitem.lParam) == PREDEFINED_MSG)
									{
										msgbox_data->predef_changed = TRUE;
										SendMessage(msgbox_data->recent_cbex, CBEM_DELETEITEM, (WPARAM)msgbox_data->curr_sel_msg, 0);
									}
									SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, -1, 0);
								}
								else if (LOWORD(cbitem.lParam) == ADD_MSG)
								{
									int sel;
									sel = AddToPredefined(hwndDlg, msgbox_data);
									SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, (WPARAM)sel, 0);
									msgbox_data->curr_sel_msg = sel;
									break;
								}
								msgbox_data->curr_sel_msg = cur_sel;
							}
					}
				}
				if((HWND)lParam == msgbox_data->badd)
				{
					switch(HIWORD(wParam))
					{
						case BN_CLICKED:
						{
							int sel;

							sel = AddToPredefined(hwndDlg, msgbox_data);

							if (sel != -1)
							{
								if (!IsWindowEnabled(msgbox_data->recent_cbex))
									EnableWindow(msgbox_data->recent_cbex, TRUE);
								if (!IsWindowEnabled(msgbox_data->bdel))
									EnableWindow(msgbox_data->bdel, TRUE);

								SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, (WPARAM)sel, 0);
								msgbox_data->curr_sel_msg = sel;
							}
						}
						break;
					}
				}
				if((HWND)lParam == msgbox_data->bclear)
				{
					switch(HIWORD(wParam))
					{
						case BN_CLICKED:
						{
							int num_items;

							ClearHistory(msgbox_data, 0);

							num_items = SendMessage(msgbox_data->recent_cbex, CB_GETCOUNT, 0, 0);

							if (!num_items)
							{
								if (IsWindowEnabled(msgbox_data->recent_cbex))
								{
									EnableWindow(msgbox_data->bdel, FALSE);
									EnableWindow(msgbox_data->recent_cbex, FALSE);
								}
							}
							EnableWindow(msgbox_data->bclear, FALSE);
						}
					}
				}
				if((HWND)lParam == msgbox_data->bdel)
				{
					switch(HIWORD(wParam))
					{
						case BN_CLICKED:
						{
							int				cur_sel;
							char			buff[16];
							int				left_items=0;
							COMBOBOXEXITEM	histitem;

							cur_sel = SendMessage(msgbox_data->recent_cbex, CB_GETCURSEL, 0, 0);

							histitem.mask = CBEIF_LPARAM;
							histitem.iItem = msgbox_data->curr_sel_msg;

							SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&histitem);

							if (LOWORD(histitem.lParam) == HISTORY_MSG)
							{
								_snprintf(buff, sizeof(buff), "SMsg%d", (int)HIWORD(histitem.lParam));
								DBWriteContactSettingString(NULL, "SimpleAway", buff, "");
								left_items = SendMessage(msgbox_data->recent_cbex, CBEM_DELETEITEM, (WPARAM)msgbox_data->curr_sel_msg, 0);
							}
							if (LOWORD(histitem.lParam) == PREDEFINED_MSG)
							{
								msgbox_data->predef_changed = TRUE;
								left_items = SendMessage(msgbox_data->recent_cbex, CBEM_DELETEITEM, (WPARAM)msgbox_data->curr_sel_msg, 0);
							}

							if (!left_items)
							{
								if (IsWindowEnabled(msgbox_data->recent_cbex))
									EnableWindow(msgbox_data->recent_cbex, FALSE);
								if (IsWindowEnabled(msgbox_data->bclear))
									EnableWindow(msgbox_data->bclear, FALSE);
								EnableWindow(msgbox_data->bdel, FALSE);
							}
							else
							{
								if (cur_sel-1 >= 0)
									cur_sel--;
								msgbox_data->curr_sel_msg = cur_sel;
								SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, (WPARAM)cur_sel, 0);
							}
						}
					}
				}
			}
		break;
		case DM_SIMPAWAY_SHUTDOWN:
			{
				DestroyWindow(hwndDlg);
			}
		break;
		case DM_SIMPAWAY_CHANGEICONS:
			{
				int i;
				
				if (!ServiceExists(MS_SKIN2_GETICON))
					return FALSE;
	
				for (i=0; i<5; i++)
				{
					msgbox_data->icon[i] = (HICON)CallService(MS_SKIN2_GETICON, (WPARAM)0, (LPARAM)sa_ico_name[i]);
	
					if (msgbox_data->dlg_flags & DLG_SHOW_LIST_ICONS)
						ImageList_ReplaceIcon(msgbox_data->other_icons, i, msgbox_data->icon[i]);
				}

				if (msgbox_data->dlg_flags & DLG_SHOW_BUTTONS)
				{
					SendMessage(msgbox_data->badd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)msgbox_data->icon[I_ICON_ADD]);
					SendMessage(msgbox_data->bclear, BM_SETIMAGE, IMAGE_ICON, (LPARAM)msgbox_data->icon[I_ICON_CLEAR]);
					SendMessage(msgbox_data->bdel, BM_SETIMAGE, IMAGE_ICON, (LPARAM)msgbox_data->icon[I_ICON_DEL]);
				}
			}
		break;
		case WM_DESTROY:
			if (msgbox_data->predef_changed)
			{
				int					i, num_items, new_num_def_msgs=0;
				COMBOBOXEXITEM		cbitem;
				char				text[1024];
				char				buff[64];

				num_items = SendMessage(msgbox_data->recent_cbex, CB_GETCOUNT, 0, 0);

				num_items--;
				for (i=1; i<=msgbox_data->num_def_msgs; i++)
				{
					cbitem.mask = CBEIF_LPARAM|CBEIF_TEXT;
					cbitem.iItem = num_items;
					cbitem.cchTextMax = sizeof(text);
					cbitem.pszText = text;

					SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&cbitem);
					_snprintf(buff, sizeof(buff), "DefMsg%d", i);
					if (LOWORD(cbitem.lParam) == PREDEFINED_MSG)
					{
						new_num_def_msgs++;
						DBWriteContactSettingString(NULL, "SimpleAway", buff, text);
					}
					else
						DBDeleteContactSetting(NULL, "SimpleAway", buff);
					num_items--;
				}
				DBWriteContactSettingWord(NULL, "SimpleAway", "DefMsgCount", (WORD)new_num_def_msgs);
			}
			
			if (!ServiceExists(MS_SKIN2_GETICON))
			{
				int i;
				
				for (i=0; i<5; i++)
					DestroyIcon(msgbox_data->icon[i]);
			}
			ImageList_Destroy(msgbox_data->status_icons);
			ImageList_Destroy(msgbox_data->other_icons);
			SetWindowLong(GetDlgItem(hwndDlg, IDC_EDIT1), GWL_WNDPROC, (LONG)MainDlgProc); 
			if (msgbox_data)
				mir_free(msgbox_data);
			hwndSAMsgDialog = NULL;
		break;
	}
	return FALSE;
}
