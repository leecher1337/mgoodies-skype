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


#include "options.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;


Options opts;


static BOOL CALLBACK ProtocolsOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);



// Functions //////////////////////////////////////////////////////////////////////////////////////


// Initializations needed by options
void LoadOptions()
{
	opts.poll_check_on_timer = DBGetContactSettingByte(NULL, MODULE_NAME, OPT_CHECK_ONTIMER, TRUE);
	opts.poll_check_on_status_change = DBGetContactSettingByte(NULL, MODULE_NAME, OPT_CHECK_ONSTATUSCHANGE, TRUE);
	opts.poll_check_on_status_change_timer = DBGetContactSettingByte(NULL, MODULE_NAME, OPT_CHECK_ONSTATUSCHANGETIMER, TRUE);
	opts.poll_timer_check = DBGetContactSettingWord(NULL, MODULE_NAME, OPT_CHECK_ONTIMER_TIMER, 10);
	opts.poll_timer_status = DBGetContactSettingWord(NULL, MODULE_NAME, OPT_CHECK_ONSTATUSTIMER_TIMER, 15);
	opts.poll_clear_on_status_change = DBGetContactSettingByte(NULL, MODULE_NAME, OPT_CLEAR_ONSTATUSCHANGE, TRUE);
	opts.always_clear = DBGetContactSettingByte(NULL, MODULE_NAME, OPT_ALWAYS_CLEAR, TRUE);

	PollSetTimer();
}

int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=0;
	odp.hInstance=hInst;
	odp.ptszGroup = TranslateT("Status");
	odp.ptszTitle = TranslateT("Status Msg Retrieve");
	odp.pfnDlgProc = ProtocolsOptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_PROTOCOLS);
    odp.flags=ODPF_BOLDGROUPS;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


void InitOptions()
{
	LoadOptions();

	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}

// Deinitializations needed by options
void DeInitOptions()
{
	UnhookEvent(hOptHook);
}


static OptPageControl pageControls[] = { 
	{ CONTROL_CHECKBOX, IDC_CHECK_ONTIMER, OPT_CHECK_ONTIMER, (BYTE) TRUE },
	{ CONTROL_CHECKBOX, IDC_CHECK_ONSTATUS, OPT_CHECK_ONSTATUSCHANGE, (BYTE) TRUE },
	{ CONTROL_CHECKBOX, IDC_CHECK_ONSTATUSTIMER, OPT_CHECK_ONSTATUSCHANGETIMER, (BYTE) TRUE },
	{ CONTROL_CHECKBOX, IDC_CLEAR_ON_STATUS, OPT_CLEAR_ONSTATUSCHANGE, (BYTE) TRUE },
	{ CONTROL_CHECKBOX, IDC_ALWAYS_CLEAR, OPT_ALWAYS_CLEAR, (BYTE) TRUE },
	{ CONTROL_SPIN,		IDC_CHECK_ONTIMER_TIMER, OPT_CHECK_ONTIMER_TIMER, (WORD) 10, IDC_CHECK_ONTIMER_TIMER_SPIN, (WORD) 1, (WORD) 255 },
	{ CONTROL_SPIN,		IDC_CHECK_ONSTATUSTIMER_TIMER, OPT_CHECK_ONSTATUSTIMER_TIMER, (WORD) 15, IDC_CHECK_ONSTATUSTIMER_TIMER_SPIN, (WORD) 1, (WORD) 255 }
};



typedef struct tagProtocolData 
{
	char setting[128];
} ProtocolData;


static BOOL CALLBACK ProtocolsOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	BOOL ret = SaveOptsDlgProc(pageControls, MAX_REGS(pageControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			// Fill list view
			HWND hwndProtocols = GetDlgItem(hwndDlg, IDC_PROTOCOLS);
			LVCOLUMN lvc;
			LVITEM lvi;
			PROTOCOLDESCRIPTOR **protos;
			int i,count;
			char szName[128];
			ProtocolData *pd;
			
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
				if (protos[i]->type != PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
					continue;
				
				CallProtoService(protos[i]->szName, PS_GETNAME, sizeof(szName), (LPARAM)szName);
				
				pd = (ProtocolData*)malloc(sizeof(ProtocolData));
				mir_snprintf(pd->setting, sizeof(pd->setting), OPT_PROTOCOL_GETMSG, protos[i]->szName);

				BOOL show = (BOOL)DBGetContactSettingByte(NULL, MODULE_NAME, pd->setting, FALSE);
				
				lvi.lParam = (LPARAM)pd;
				lvi.pszText = TranslateT(szName);
				lvi.iItem = ListView_InsertItem(hwndProtocols, &lvi);
				ListView_SetItemState(hwndProtocols, lvi.iItem, INDEXTOSTATEIMAGEMASK(show?2:1), LVIS_STATEIMAGEMASK);
			}
			
			ListView_SetColumnWidth(hwndProtocols, 0, LVSCW_AUTOSIZE);
			ListView_Arrange(hwndProtocols, LVA_ALIGNLEFT | LVA_ALIGNTOP);

			ret = TRUE;
			break;
		}
		case WM_DESTROY:
		{
			LVITEM lvi = {0};
			HWND hwndProtocols = GetDlgItem(hwndDlg, IDC_PROTOCOLS);
			int i;
			
			lvi.mask = (UINT) LVIF_PARAM;
			
			for (i = 0; i < ListView_GetItemCount(hwndProtocols); i++)
			{
				lvi.iItem = i;
				ListView_GetItem(hwndProtocols, &lvi);
				free((ProtocolData *) lvi.lParam);
			}
			
			ret = TRUE;
			break;
		}
		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom)
			{
				case IDC_PROTOCOLS:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case LVN_ITEMCHANGED:
						{
							NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
							
							if(IsWindowVisible(GetDlgItem(hwndDlg, IDC_PROTOCOLS)) && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK))
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

							ret = TRUE;

							break;
						}
					}
					break;
				}
				case 0:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							LVITEM lvi = {0};
							ProtocolData *pd;
							HWND hwndProtocols = GetDlgItem(hwndDlg, IDC_PROTOCOLS);
							int i;
							
							lvi.mask = (UINT) LVIF_PARAM;
							
							for (i = 0; i < ListView_GetItemCount(hwndProtocols); i++)
							{
								lvi.iItem = i;
								ListView_GetItem(hwndProtocols, &lvi);
								
								pd = (ProtocolData *)lvi.lParam;
								DBWriteContactSettingByte(NULL, MODULE_NAME, pd->setting, (BYTE)(BOOL)ListView_GetCheckState(hwndProtocols, i));
							}
							
							LoadOptions();

							ret = TRUE;
							break;
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

