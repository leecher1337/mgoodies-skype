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

BOOL CALLBACK DlgProcHistory(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


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
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_HISTORY);
	odp.pszTitle = Translate("History");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcHistory;
	CallService(MS_NOTIFY_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}


static OptPageControl pageControls[] = { 
	{ CONTROL_CHECKBOX, IDC_SYSTEM_LOG,			NFOPT_HISTORY_SYSTEM_LOG, (BYTE) 0 },
	{ CONTROL_CHECKBOX, IDC_SYSTEM_MARK_READ,	NFOPT_HISTORY_SYSTEM_MARK_READ, (BYTE) 1 },
	{ CONTROL_CHECKBOX, IDC_CONTACT_LOG,		NFOPT_HISTORY_CONTACT_LOG, (BYTE) 0 },
	{ CONTROL_CHECKBOX, IDC_CONTACT_MARK_READ,	NFOPT_HISTORY_CONTACT_MARK_READ, (BYTE) 1 }
};

static BOOL CALLBACK DlgProcHistory(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return SaveOptsDlgProc(pageControls, MAX_REGS(pageControls), hwndDlg, msg, wParam, lParam);
}

