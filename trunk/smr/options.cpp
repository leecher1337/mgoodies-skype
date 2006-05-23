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

static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK GeneralOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
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
	opts.when_xstatus = (XStatusAction) DBGetContactSettingWord(NULL, MODULE_NAME, OPT_WHEN_XSTATUS, Clear);

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
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTS);
    odp.flags = ODPF_BOLDGROUPS;
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


void InitOptions()
{
	LoadOptions();

	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);

	InitMirOptions();
}

// Deinitializations needed by options
void DeInitOptions()
{
	UnhookEvent(hOptHook);

	FreeMirOptions();
}

// Options page

static ItemOption pages[] = {
	{ "General", IDD_OPT_GENERAL, GeneralOptionsDlgProc },
	{ "Protocols", IDD_OPT_PROTOCOLS, ProtocolsOptionsDlgProc }
};

static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	BOOL ret = TabsDlgProc(pages, MAX_REGS(pages), hInst, IDC_TAB, hwndDlg, msg, wParam, lParam);

	if (msg == WM_NOTIFY && ((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY)
		LoadOptions();

	return ret;
}


// General page

static OptPageControl generalControls[] = { 
	{ CONTROL_CHECKBOX, IDC_CHECK_ONTIMER, OPT_CHECK_ONTIMER, (BYTE) TRUE },
	{ CONTROL_CHECKBOX, IDC_CHECK_ONSTATUS, OPT_CHECK_ONSTATUSCHANGE, (BYTE) TRUE },
	{ CONTROL_CHECKBOX, IDC_CHECK_ONSTATUSTIMER, OPT_CHECK_ONSTATUSCHANGETIMER, (BYTE) TRUE },
	{ CONTROL_CHECKBOX, IDC_CLEAR_ON_STATUS, OPT_CLEAR_ONSTATUSCHANGE, (BYTE) TRUE },
	{ CONTROL_CHECKBOX, IDC_ALWAYS_CLEAR, OPT_ALWAYS_CLEAR, (BYTE) TRUE },
	{ CONTROL_SPIN,		IDC_CHECK_ONTIMER_TIMER, OPT_CHECK_ONTIMER_TIMER, (WORD) 10, IDC_CHECK_ONTIMER_TIMER_SPIN, (WORD) 1, (WORD) 255 },
	{ CONTROL_SPIN,		IDC_CHECK_ONSTATUSTIMER_TIMER, OPT_CHECK_ONSTATUSTIMER_TIMER, (WORD) 15, IDC_CHECK_ONSTATUSTIMER_TIMER_SPIN, (WORD) 1, (WORD) 255 }, 
	{ CONTROL_COMBO,	IDC_XSTATUS, OPT_WHEN_XSTATUS, (WORD) Clear }
};


static BOOL CALLBACK GeneralOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage(hwndDlg, IDC_XSTATUS, CB_ADDSTRING, 0, (LONG) TranslateT("Retrieve as usual"));
			SendDlgItemMessage(hwndDlg, IDC_XSTATUS, CB_ADDSTRING, 0, (LONG) TranslateT("Clear"));
			SendDlgItemMessage(hwndDlg, IDC_XSTATUS, CB_ADDSTRING, 0, (LONG) TranslateT("Clear only if XStatus message is set"));
			SendDlgItemMessage(hwndDlg, IDC_XSTATUS, CB_ADDSTRING, 0, (LONG) TranslateT("Set to XStatus Name"));
			SendDlgItemMessage(hwndDlg, IDC_XSTATUS, CB_ADDSTRING, 0, (LONG) TranslateT("Set to XStatus Message"));
			SendDlgItemMessage(hwndDlg, IDC_XSTATUS, CB_ADDSTRING, 0, (LONG) TranslateT("Set to XStatus Name: XStatus Message"));
			break;
		}
	}

	return SaveOptsDlgProc(generalControls, MAX_REGS(generalControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}


// Protocols page


static OptPageControl protocolControls[] = { 
	{ CONTROL_PROTOCOL_LIST,	IDC_PROTOCOLS, OPT_PROTOCOL_GETMSG, (BYTE) FALSE }
};


static BOOL CALLBACK ProtocolsOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	return SaveOptsDlgProc(protocolControls, MAX_REGS(protocolControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}

