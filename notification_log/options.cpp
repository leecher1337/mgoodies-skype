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


#include "options.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////


int NotifyOptionsInitialize(WPARAM wParam,LPARAM lParam);

BOOL CALLBACK DlgProcLog(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


// Functions //////////////////////////////////////////////////////////////////////////////////////


// Initializations needed by options
void InitOptions()
{
	HookEvent(ME_NOTIFY_OPT_INITIALISE, NotifyOptionsInitialize);
}


// Deinitializations needed by options
void DeInitOptions()
{
}


int NotifyOptionsInitialize(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LOG);
	odp.ptszTitle = TranslateT("Log");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pfnDlgProc = DlgProcLog;
	odp.position = 10;
	CallService(MS_NOTIFY_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}


static OptPageControl pageControls[] = { 
	{ CONTROL_CHECKBOX, IDC_LOG,		NFOPT_LOG_ENABLED, (BYTE) 0 },
	{ CONTROL_TEXT,		IDC_FILENAME,	NFOPT_LOG_FILENAMET, NULL },
	{ CONTROL_TEMPLATE, IDC_TEXT,		NFOPT_LOG_TEMPLATE_LINET, NULL }
};

TCHAR defLine[1024];
TCHAR defFile[1024];

static BOOL CALLBACK DlgProcLog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_USER+100:
		{
			HANDLE hNotify = (HANDLE)lParam;

			mir_sntprintf(defFile, MAX_REGS(defLine), _T("%s_log.txt"), 
							MNotifyGetTString(hNotify, NFOPT_TYPENAME, _T("notification")));

			pageControls[1].szDefVale = defFile;


			mir_sntprintf(defLine, MAX_REGS(defLine), _T("%s: %s"), 
							MNotifyGetTTemplate(hNotify, NFOPT_DEFTEMPL_TITLET, _T("%title%")), 
							MNotifyGetTTemplate(hNotify, NFOPT_DEFTEMPL_TEXTT, _T("%text%")));

			pageControls[2].szDefVale = defLine;

			break;
		}
        case WM_COMMAND: 
		{
			if (HIWORD(wParam) == BN_CLICKED) 
			{ 
				switch (LOWORD(wParam)) 
				{ 
                    case IDC_BTN_HELP: 
					{
						HANDLE hNotify = (HANDLE)GetWindowLong(hwndDlg, GWL_USERDATA);
						MNotifyShowTVariables(hNotify);

						return TRUE;
					}
				}
			}

			break;
		}
	}

	return SaveOptsDlgProc(pageControls, MAX_REGS(pageControls), hwndDlg, msg, wParam, lParam);
}

