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
#include <stdio.h>
#include <tchar.h>

extern "C"
{
#include <newpluginapi.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_system.h>
}


// WinXP stuff

HMODULE hUxTheme = NULL;

#ifndef ETDT_ENABLETAB
# define ETDT_ENABLETAB      0x00000006
#endif

typedef BOOL (WINAPI *fEnableThemeDialogTexture)(HANDLE, DWORD);
fEnableThemeDialogTexture pfEnableThemeDialogTexture = NULL;


void InitMirOptions()
{
	hUxTheme = LoadLibraryA("uxtheme.dll");
    if(hUxTheme == NULL)
        return;

    pfEnableThemeDialogTexture = (fEnableThemeDialogTexture) GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
}

void FreeMirOptions()
{
    if(hUxTheme != NULL)
        FreeLibrary(hUxTheme);
}

static TCHAR* MyDBGetContactSettingTString(HANDLE hContact, char* module, char* setting, TCHAR* out, size_t len, TCHAR *def)
{
	DBVARIANT dbv;

	out[0] = _T('\0');

	if (!DBGetContactSettingTString(hContact, module, setting, &dbv))
	{
#ifdef UNICODE
		if (dbv.type == DBVT_ASCIIZ)
		{
			MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, out, len);
		}
		else if (dbv.type == DBVT_UTF8)
		{
			MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, out, len);
		}
		else if (dbv.type == DBVT_WCHAR)
		{
			lstrcpyn(out, dbv.pwszVal, len);
		}
#else
		if (dbv.type == DBVT_ASCIIZ)
		{
			lstrcpyn(out, dbv.pszVal, len);
		}
#endif
		else
		{
			if (def != NULL)
				lstrcpyn(out, def, len);
		}
		
		DBFreeVariant(&dbv);
	}
	else
	{
		if (def != NULL)
			lstrcpyn(out, def, len);
	}

	return out;
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
	int selected_item;
}; 


// DoLockDlgRes - loads and locks a dialog template resource. 
// Returns the address of the locked resource. 
// lpszResName - name of the resource 

static DLGTEMPLATE * DoLockDlgRes(HINSTANCE hInst, LPCTSTR lpszResName) 
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
	rc_item.left = rc_tab.left + (rc_tab.right - rc_tab.left - rc_item.right) / 2;

	// Old style (centered)
	//rc_item.bottom -= rc_item.top;	// height
	//rc_item.top = rc_tab.top + (rc_tab.bottom - rc_tab.top - rc_item.bottom) / 2;
	// New style (at top)
	rc_item.top = rc_tab.top;
	rc_item.bottom = rc_tab.bottom - rc_tab.top;

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

				if (pfEnableThemeDialogTexture != NULL)
					pfEnableThemeDialogTexture(data->items[i].hwnd, ETDT_ENABLETAB);

				ShowWindow(data->items[i].hwnd, SW_HIDE);

				tie.pszText = TranslateTS(optItens[i].name); 
				TabCtrl_InsertItem(hwndTab, i, &tie);
			}

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

void LoadOpts(OptPageControl *controls, int controlsSize, char *module)
{
	for (int i = 0 ; i < controlsSize ; i++)
	{
		OptPageControl *ctrl = &controls[i];

		if (ctrl->var != NULL)
		{
			switch(ctrl->type)
			{
				case CONTROL_CHECKBOX:
				{
					*((BYTE *) ctrl->var) = DBGetContactSettingByte(NULL, module, ctrl->setting, ctrl->dwDefValue);
					break;
				}
				case CONTROL_SPIN:
				{
					*((WORD *) ctrl->var) = DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue);
					break;
				}
				case CONTROL_COLOR:
				{
					*((COLORREF *) ctrl->var) = (COLORREF) DBGetContactSettingDword(NULL, module, ctrl->setting, ctrl->dwDefValue);
					break;
				}
				case CONTROL_RADIO:
				{
					*((WORD *) ctrl->var) = DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue);
					break;
				}
				case CONTROL_COMBO:
				{
					*((WORD *) ctrl->var) = DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue);
					break;
				}
				case CONTROL_PROTOCOL_LIST:
				{
					break;
				}
				case CONTROL_TEXT:
				{
					MyDBGetContactSettingTString(NULL, module, ctrl->setting, ((TCHAR *) ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
					break;
				}
				case CONTROL_COMBO_TEXT:
				case CONTROL_COMBO_ITEMDATA:
				{
					MyDBGetContactSettingTString(NULL, module, ctrl->setting, ((TCHAR *) ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
					break;
				}
			}
		}
	}
}



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
						CheckDlgButton(hwndDlg, ctrl->nID, DBGetContactSettingByte(NULL, module, ctrl->setting, ctrl->dwDefValue) == 1 ? BST_CHECKED : BST_UNCHECKED);
						break;
					}
					case CONTROL_SPIN:
					{
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, ctrl->nID),0);
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETRANGE, 0, MAKELONG(ctrl->max, ctrl->min));
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETPOS,0, MAKELONG(DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue), 0));
						break;
					}
					case CONTROL_COLOR:
					{
						SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_SETCOLOUR, 0, (COLORREF) DBGetContactSettingDword(NULL, module, ctrl->setting, ctrl->dwDefValue));
						break;
					}
					case CONTROL_RADIO:
					{
						CheckDlgButton(hwndDlg, ctrl->nID, DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue) == ctrl->value ? BST_CHECKED : BST_UNCHECKED);
						break;
					}
					case CONTROL_COMBO:
					{
						SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SETCURSEL, DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue), 0);
						break;
					}
					case CONTROL_PROTOCOL_LIST:
					{
						// Fill list view
						HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
						LVCOLUMN lvc;
						LVITEM lvi;
						PROTOCOLDESCRIPTOR **protos;
						int i,count;
						char szName[128];
						
						ListView_SetExtendedListViewStyle(hwndProtocols, LVS_EX_CHECKBOXES);
						
						ZeroMemory(&lvc, sizeof(lvc));
						lvc.mask = LVCF_FMT;
						lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
						ListView_InsertColumn(hwndProtocols, 0, &lvc);
						
						ZeroMemory(&lvi, sizeof(lvi));
						lvi.mask = LVIF_TEXT | LVIF_PARAM;
						lvi.iSubItem = 0;
						lvi.iItem = 1000;
						
						CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);
						
						for (i = 0; i < count; i++)
						{
							if (protos[i]->type != PROTOTYPE_PROTOCOL)
								continue;
							
							if (ctrl->allowProtocol != NULL && !ctrl->allowProtocol(protos[i]->szName))
								continue;

							CallProtoService(protos[i]->szName, PS_GETNAME, sizeof(szName), (LPARAM)szName);
							
							char *setting = (char *) mir_alloc0(128 * sizeof(char));
							mir_snprintf(setting, 128, ctrl->setting, protos[i]->szName);

							BOOL show = (BOOL)DBGetContactSettingByte(NULL, module, setting, ctrl->dwDefValue);
							
							lvi.lParam = (LPARAM)setting;
#ifdef UNICODE
							WCHAR szwName[128];
							MultiByteToWideChar(CP_ACP, 0, szName, -1, szwName, 128);
							lvi.pszText = TranslateTS(szwName);
#else
							lvi.pszText = TranslateTS(szName);
#endif
							lvi.iItem = ListView_InsertItem(hwndProtocols, &lvi);
							ListView_SetItemState(hwndProtocols, lvi.iItem, INDEXTOSTATEIMAGEMASK(show?2:1), LVIS_STATEIMAGEMASK);
						}
						
						ListView_SetColumnWidth(hwndProtocols, 0, LVSCW_AUTOSIZE);
						ListView_Arrange(hwndProtocols, LVA_ALIGNLEFT | LVA_ALIGNTOP);
						break;
					}
					case CONTROL_TEXT:
					{
						TCHAR tmp[1024];
						SetDlgItemText(hwndDlg, ctrl->nID, MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue)));

						SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), 0);
						break;
					}
					case CONTROL_COMBO_TEXT:
					{
						TCHAR tmp[1024];
						MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
						SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SELECTSTRING, 0, (WPARAM) tmp);
						break;
					}
					case CONTROL_COMBO_ITEMDATA:
					{
						TCHAR tmp[1024];
						MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
						int count = SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCOUNT, 0, 0);
						int i;
						for(i = 0; i < count; i++)
						{
							TCHAR *id = (TCHAR *) SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETITEMDATA, (WPARAM) i, 0);
							if (lstrcmp(id, tmp) == 0)
								break;
						}
						if (i < count)
							SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SETCURSEL, i, 0);
						break;
					}
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			for (int i = 0 ; i < controlsSize ; i++)
			{
				OptPageControl *ctrl = &controls[i];

				if (LOWORD(wParam) == ctrl->nID)
				{
					switch(ctrl->type)
					{
						case CONTROL_SPIN:
						{
							// Don't make apply enabled during buddy set
							if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
								return 0;

							break;
						}
						case CONTROL_COMBO_ITEMDATA:
						case CONTROL_COMBO_TEXT:
						case CONTROL_COMBO:
						{
							if (HIWORD(wParam) != CBN_SELCHANGE || (HWND)lParam != GetFocus())
								return 0;

							break;
						}
					}
				}
			}

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				for (int i = 0 ; i < controlsSize ; i++)
				{
					OptPageControl *ctrl = &controls[i];

					switch(ctrl->type)
					{
						case CONTROL_CHECKBOX:
						{
							DBWriteContactSettingByte(NULL, module, ctrl->setting, (BYTE) IsDlgButtonChecked(hwndDlg, ctrl->nID));
							break;
						}
						case CONTROL_SPIN:
						{
							DBWriteContactSettingWord(NULL, module, ctrl->setting, (WORD) SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_GETPOS, 0, 0));
							break;
						}
						case CONTROL_COLOR:
						{
							DBWriteContactSettingDword(NULL, module, ctrl->setting, (DWORD) SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_GETCOLOUR, 0, 0));
							break;
						}
						case CONTROL_RADIO:
						{
							if (IsDlgButtonChecked(hwndDlg, ctrl->nID))
								DBWriteContactSettingWord(NULL, module, ctrl->setting, (BYTE) ctrl->value);
							break;
						}
						case CONTROL_COMBO:
						{
							DBWriteContactSettingWord(NULL, module, ctrl->setting, (WORD) SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCURSEL, 0, 0));
							break;
						}
						case CONTROL_PROTOCOL_LIST:
						{
							LVITEM lvi = {0};
							HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
							int i;
							
							lvi.mask = (UINT) LVIF_PARAM;
							
							for (i = 0; i < ListView_GetItemCount(hwndProtocols); i++)
							{
								lvi.iItem = i;
								ListView_GetItem(hwndProtocols, &lvi);
								
								char *setting = (char *)lvi.lParam;
								DBWriteContactSettingByte(NULL, module, setting, (BYTE)ListView_GetCheckState(hwndProtocols, i));
							}
							break;
						}
						case CONTROL_TEXT:
						{
							TCHAR tmp[1024];
							GetDlgItemText(hwndDlg, ctrl->nID, tmp, 1024);
							DBWriteContactSettingTString(NULL, module, ctrl->setting, tmp);
							break;
						}
						case CONTROL_COMBO_TEXT:
						{
							TCHAR tmp[1024];
							GetDlgItemText(hwndDlg, ctrl->nID, tmp, 1024);
							DBWriteContactSettingTString(NULL, module, ctrl->setting, tmp);
							break;
						}
						case CONTROL_COMBO_ITEMDATA:
						{
							int sel = SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCURSEL, 0, 0);
							DBWriteContactSettingTString(NULL, module, ctrl->setting, (TCHAR *) SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETITEMDATA, (WPARAM) sel, 0));
							break;
						}
					}
				}
				
				LoadOpts(controls, controlsSize, module);

				return TRUE;
			}
			else if (lpnmhdr->idFrom != 0 && lpnmhdr->code == LVN_ITEMCHANGED)
			{
				// Changed for protocols
				for (int i = 0 ; i < controlsSize ; i++)
				{
					OptPageControl *ctrl = &controls[i];

					if (ctrl->type == CONTROL_PROTOCOL_LIST && ctrl->nID == lpnmhdr->idFrom)
					{
						NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
						
						if(IsWindowVisible(GetDlgItem(hwndDlg, ctrl->nID)) && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK))
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

						break;
					}
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			for (int i = 0 ; i < controlsSize ; i++)
			{
				OptPageControl *ctrl = &controls[i];

				switch(ctrl->type)
				{
					case CONTROL_PROTOCOL_LIST:
					{
						LVITEM lvi = {0};
						HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
						int i;
						
						lvi.mask = (UINT) LVIF_PARAM;
						
						for (i = 0; i < ListView_GetItemCount(hwndProtocols); i++)
						{
							lvi.iItem = i;
							ListView_GetItem(hwndProtocols, &lvi);
							mir_free((char *) lvi.lParam);
						}
						break;
					}
				}
			}
			break;
		}
	}

	return 0;
}
