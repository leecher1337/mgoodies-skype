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



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;

Options opts;


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ &opts.csv_file,			CONTROL_TEXT,		IDC_CSV,			"CSVFile",	(DWORD) _T("plugins.csv") },
	{ &opts.dump_on_startup,	CONTROL_CHECKBOX,	IDC_DUMP_STARTUP,	"DumpOnStart",	FALSE }
};



// Functions //////////////////////////////////////////////////////////////////////////////////////


int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=0;
	odp.hInstance=hInst;
	odp.ptszGroup = TranslateT("Services");
	odp.ptszTitle = TranslateT("Q Updater");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
    odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_EXPERTONLY;
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
}


void ListView_SetItemTextA( HWND hwndLV, int i, int iSubItem, char* pszText )
{
	LV_ITEMA _ms_lvi;
	_ms_lvi.iSubItem = iSubItem;
	_ms_lvi.pszText = pszText;
	SendMessageA( hwndLV, LVM_SETITEMTEXTA, i, (LPARAM)&_ms_lvi);
}


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			HWND hwndList = GetDlgItem(hwndDlg,IDC_PLUGINS);
			
			LVCOLUMN col = {0};
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText=TranslateT("Plugin Name");
			col.cx=100;
			ListView_InsertColumn(hwndList,0,&col);
			
			col.pszText=TranslateT("Version");
			col.cx=60;
			ListView_InsertColumn(hwndList,1,&col);

		
			col.pszText=TranslateT("Version URL");
			col.cx=200;
			ListView_InsertColumn(hwndList,2,&col);
		
			col.pszText=TranslateT("Version Prefix");
			col.cx=100;
			ListView_InsertColumn(hwndList,3,&col);
		
			col.pszText=TranslateT("Update URL");
			col.cx=200;
			ListView_InsertColumn(hwndList,4,&col);

		
			col.pszText=TranslateT("Beta Version URL");
			col.cx=200;
			ListView_InsertColumn(hwndList,5,&col);
		
			col.pszText=TranslateT("Beta Version Prefix");
			col.cx=100;
			ListView_InsertColumn(hwndList,6,&col);
		
			col.pszText=TranslateT("Beta Update URL");
			col.cx=200;
			ListView_InsertColumn(hwndList,7,&col);

		
			col.pszText=TranslateT("Beta Changelog URL");
			ListView_InsertColumn(hwndList,8,&col);
			
			// XXX: Won't work on windows 95 without IE3+ or 4.70
			ListView_SetExtendedListViewStyleEx(hwndList, 0, LVS_EX_FULLROWSELECT );

			for(int i = 0; i < plugins.size(); i++)
			{
				LVITEMA it = {0};
				it.mask = LVIF_TEXT;
				it.pszText = plugins[i].szComponentName;
				int iRow = SendMessageA( hwndList, LVM_INSERTITEMA, 0, (LPARAM)&it );
				if (plugins[i].pbVersion != NULL)
					ListView_SetItemTextA(hwndList, iRow, 1, (char *) plugins[i].pbVersion);

				if (plugins[i].szVersionURL != NULL)
					ListView_SetItemTextA(hwndList, iRow, 2, plugins[i].szVersionURL);
				if (plugins[i].pbVersionPrefix != NULL)
					ListView_SetItemTextA(hwndList, iRow, 3, (char *) plugins[i].pbVersionPrefix);
				if (plugins[i].szUpdateURL != NULL)
				{
					if (strcmp(plugins[i].szUpdateURL, UPDATER_AUTOREGISTER) == 0)
						ListView_SetItemTextA(hwndList, iRow, 4, "<From FL XML>");
					else
						ListView_SetItemTextA(hwndList, iRow, 4, plugins[i].szUpdateURL);
				}

				if (plugins[i].szBetaVersionURL != NULL)
					ListView_SetItemTextA(hwndList, iRow, 5, plugins[i].szBetaVersionURL);
				if (plugins[i].pbBetaVersionPrefix != NULL)
					ListView_SetItemTextA(hwndList, iRow, 6, (char *) plugins[i].pbBetaVersionPrefix);
				if (plugins[i].szBetaUpdateURL != NULL)
					ListView_SetItemTextA(hwndList, iRow, 7, plugins[i].szBetaUpdateURL);

				if (plugins[i].szBetaChangelogURL != NULL)
					ListView_SetItemTextA(hwndList, iRow, 8, plugins[i].szBetaChangelogURL);
			}

			// sort out the headers
			ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);

			break;
		}

		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDC_DUMP)
				DumpFile();
			
			break;
		}
	}

	return SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}

