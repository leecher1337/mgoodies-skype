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


#include "mir_options.h"
#include "mir_memory.h"

#include <commctrl.h>

extern "C"
{
#include <newpluginapi.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <tchar.h>
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Multiple tabs per dialog
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct ItemOptionData 
{ 
	HWND hwnd;				// dialog handle
}; 

struct WndItemsData 
{ 
	ItemOptionData *items;
	HWND hwndDisplay;
	int selected_item;
}; 


// DoLockDlgRes - loads and locks a dialog template resource. 
// Returns the address of the locked resource. 
// lpszResName - name of the resource 

static DLGTEMPLATE * DoLockDlgRes(HINSTANCE hInst, LPCSTR lpszResName) 
{ 
	HRSRC hrsrc = FindResource(hInst, lpszResName, RT_DIALOG); 
	HGLOBAL hglb = LoadResource(hInst, hrsrc); 
	return (DLGTEMPLATE *) LockResource(hglb); 
} 

static BOOL __inline ScreenToClientRect(HWND hWnd, LPRECT lpRect)
{
	BOOL ret;

	POINT pt;

	pt.x = lpRect->left;
	pt.y = lpRect->top;

	ret = ScreenToClient(hWnd, &pt);

	if (!ret) return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;


	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}

static void ChangeTab(HWND hwndDlg, UINT idc_tab, WndItemsData *data, int sel)
{
	HWND hwndTab;
	RECT rc_tab;
	RECT rc_item;

	hwndTab = GetDlgItem(hwndDlg, idc_tab);

	// Get avaible space
	GetWindowRect(hwndTab, &rc_tab);
	ScreenToClientRect(hwndDlg, &rc_tab);
	TabCtrl_AdjustRect(hwndTab, FALSE, &rc_tab);
	
	// Get item size
	GetWindowRect(data->items[sel].hwnd, &rc_item);

	// Fix rc_item
	rc_item.right -= rc_item.left;	// width
	rc_item.left = 0;
	rc_item.bottom -= rc_item.top;	// height
	rc_item.top = 0;

	rc_item.left = rc_tab.left + (rc_tab.right - rc_tab.left - rc_item.right) / 2;
	rc_item.top = rc_tab.top + (rc_tab.bottom - rc_tab.top - rc_item.bottom) / 2;

	// Set pos
	SetWindowPos(data->items[sel].hwnd, HWND_TOP, rc_item.left, rc_item.top, 
		rc_item.right, rc_item.bottom, SWP_SHOWWINDOW);

	data->selected_item = sel;
}

BOOL CALLBACK TabsDlgProc(ItemOption *optItens, int optItensSize, HINSTANCE hInst, UINT idc_tab, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			HWND hwndTab;
			WndItemsData *data;
			int i;
			TCITEM tie; 
			RECT rc_tab;
			
			TranslateDialogDefault(hwndDlg);

			data = (WndItemsData *) mir_alloc0(sizeof(WndItemsData));
			data->selected_item = -1;
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) data); 

			hwndTab = GetDlgItem(hwndDlg, idc_tab);

			// Add tabs
			tie.mask = TCIF_TEXT | TCIF_IMAGE; 
			tie.iImage = -1; 

			data->items = (ItemOptionData *) mir_alloc0(sizeof(ItemOptionData) * optItensSize);
			
			for (i = 0 ; i < optItensSize ; i++)
			{
				DLGTEMPLATE *templ;

				templ = DoLockDlgRes(hInst, MAKEINTRESOURCE(optItens[i].id));
				data->items[i].hwnd = CreateDialogIndirect(hInst, templ, hwndDlg, 
													 optItens[i].wnd_proc); 
				ShowWindow(data->items[i].hwnd, SW_HIDE);

				tie.pszText = TranslateT(optItens[i].name); 
				TabCtrl_InsertItem(hwndTab, i, &tie);
			}

			// Get avaible space
			GetWindowRect(hwndTab, &rc_tab);
			ScreenToClientRect(hwndTab, &rc_tab);
			TabCtrl_AdjustRect(hwndTab, FALSE, &rc_tab); 

			// Create big display
			data->hwndDisplay = CreateWindow("STATIC", "", WS_CHILD|WS_VISIBLE, 
								rc_tab.left, rc_tab.top, 
								rc_tab.right-rc_tab.left, rc_tab.bottom-rc_tab.top, 
								hwndTab, NULL, hInst, NULL); 

			// Show first item
			ChangeTab(hwndDlg, idc_tab, data, 0);

			return TRUE;
			break;
		}
		case WM_NOTIFY: 
		{
			switch (((LPNMHDR)lParam)->code) 
			{
				case TCN_SELCHANGING:
				{
					WndItemsData *data = (WndItemsData *) GetWindowLong(hwndDlg, GWL_USERDATA);
					ShowWindow(data->items[data->selected_item].hwnd, SW_HIDE);
					break;
				}
				case TCN_SELCHANGE: 
				{
					ChangeTab(hwndDlg, idc_tab, 
							  (WndItemsData *)GetWindowLong(hwndDlg, GWL_USERDATA), 
							  TabCtrl_GetCurSel(GetDlgItem(hwndDlg, idc_tab)));
					break; 
				}

				case PSN_APPLY:
				{
					if (((LPNMHDR)lParam)->idFrom == 0)
					{
						WndItemsData *data = (WndItemsData *) GetWindowLong(hwndDlg, GWL_USERDATA);
						int i;
						for (i = 0 ; i < optItensSize ; i++)
						{
							SendMessage(data->items[i].hwnd, msg, wParam, lParam);
						}
						return TRUE;
					}
					break;
				}
			}
			break;
		} 
		case PSM_CHANGED:
		{
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		case WM_DESTROY: 
		{
			int i;
			WndItemsData *data = (WndItemsData *) GetWindowLong(hwndDlg, GWL_USERDATA);

			DestroyWindow(data->hwndDisplay); 

			for (i = 0 ; i < optItensSize ; i++)
			{
				DestroyWindow(data->items[i].hwnd); 
			}

			mir_free(data->items); 
			mir_free(data); 
			break;
		}
	}

	return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dialog to save options
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


BOOL CALLBACK SaveOptsDlgProc(OptPageControl *controls, int controlsSize, char *module, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			for (int i = 0 ; i < controlsSize ; i++)
			{
				OptPageControl *ctrl = &controls[i];

				switch(ctrl->type)
				{
					case CONTROL_CHECKBOX:
					{
						CheckDlgButton(hwndDlg, ctrl->nID, DBGetContactSettingByte(NULL, module, ctrl->setting, ctrl->defValue) == 1 ? BST_CHECKED : BST_UNCHECKED);
						break;
					}
					case CONTROL_SPIN:
					{
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, ctrl->nID),0);
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETRANGE, 0, MAKELONG(ctrl->max, ctrl->min));
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETPOS,0, MAKELONG(DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->defValue), 0));

						break;
					}
					case CONTROL_COLOR:
					{
						SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_SETCOLOUR, 0, (COLORREF) DBGetContactSettingDword(NULL, module, ctrl->setting, ctrl->defValue));

						break;
					}
					case CONTROL_RADIO:
					{
						CheckDlgButton(hwndDlg, ctrl->nID, DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->defValue) == ctrl->value ? BST_CHECKED : BST_UNCHECKED);
						break;
					}
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			// Don't make apply enabled during buddy set crap
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
			{
				for (int i = 0 ; i < controlsSize ; i++)
				{
					if (controls[i].type == CONTROL_SPIN && LOWORD(wParam) == controls[i].nID)
					{
						return 0;
					}
				}
			}

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
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
							for (int i = 0 ; i < controlsSize ; i++)
							{
								OptPageControl *ctrl = &controls[i];

								switch(ctrl->type)
								{
									case CONTROL_CHECKBOX:
									{
										DBWriteContactSettingByte(NULL, module, ctrl->setting, (BYTE)IsDlgButtonChecked(hwndDlg, ctrl->nID));
										break;
									}
									case CONTROL_SPIN:
									{
										DBWriteContactSettingWord(NULL, module, ctrl->setting, (WORD)SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_GETPOS, 0, 0));
										break;
									}
									case CONTROL_COLOR:
									{
										DBWriteContactSettingDword(NULL, module, ctrl->setting, (DWORD)SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_GETCOLOUR, 0, 0));
										break;
									}
									case CONTROL_RADIO:
									{
										if (IsDlgButtonChecked(hwndDlg, ctrl->nID))
											DBWriteContactSettingWord(NULL, module, ctrl->setting, (BYTE)ctrl->value);
										break;
									}
								}
							}
							

							return TRUE;
						}
					}
					break;
				}
			}
			break;
		}
	}

	return 0;
}
