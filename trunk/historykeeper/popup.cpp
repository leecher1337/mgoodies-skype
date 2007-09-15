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



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

#define WMU_ACTION	(WM_USER + 1)


LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hPopupWindow = NULL;


struct PopupData
{
	HANDLE hContact;
	int type;

	PopupData(HANDLE aHContact, int aType) {
		hContact = aHContact;
		type = aType;
	}
};

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK DumbPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void ReplaceVars(Buffer<TCHAR> *buffer, HANDLE hContact, TCHAR **variables, int numVariables);



// Functions //////////////////////////////////////////////////////////////////////////////////////

// Initializations needed by popups
void InitPopups()
{
	// window needed for popup commands
	hPopupWindow = CreateWindowEx(WS_EX_TOOLWINDOW, _T("static"), _T(MODULE_NAME) _T("_PopupWindow"), 
		0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP,
		NULL, hInst, NULL);
	SetWindowLong(hPopupWindow, GWL_WNDPROC, (LONG)(WNDPROC)PopupWndProc);
}


// Deinitializations needed by popups
void DeInitPopups()
{
}


// Show an error popup
void ShowErrPopup(int typeNum, const TCHAR *description, const TCHAR *title)
{
	ShowPopupEx(NULL, title == NULL ? _T(MODULE_NAME) _T(" Error") : title, description,
			  NULL, POPUP_TYPE_ERROR, NULL, typeNum);
}


void ShowTestPopup(int typeNum, const TCHAR *title, const TCHAR *description, const Options *op)
{
	ShowPopupEx(NULL, title, description, NULL, POPUP_TYPE_TEST, op, typeNum);
}


void ShowPopup(HANDLE hContact, int typeNum, int templateNum, TCHAR **variables, int numVariables)
{
	// Only some time after creation
	if (DBGetContactSettingDword(hContact, MODULE_NAME, "CreationTickCount", 0) 
					+ TIME_TO_WAIT_BEFORE_SHOW_POPUP_AFTER_CREATION > GetTickCount())
		return;

	if (opts[typeNum].popup_dont_notfy_on_connect)
	{
		char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (proto != NULL 
			&& DBGetContactSettingDword(NULL, proto, MODULE_NAME "_OnOfflineTickCount", 0) 
					+ TIME_TO_WAIT_BEFORE_SHOW_POPUP_AFTER_CONNECTION > GetTickCount())
			return;
	}

	if (templateNum == 0 && !opts[typeNum].popup_track_changes)
		return;
	if (templateNum == 1 && !opts[typeNum].popup_track_removes)
		return;

	Buffer<TCHAR> txt;
	txt.append(templateNum == 1 ? opts[typeNum].popup_template_removed : opts[typeNum].popup_template_changed);
	ReplaceVars(&txt, hContact, variables, numVariables);
	txt.pack();

	ShowPopupEx(hContact, NULL, txt.str, new PopupData(hContact, typeNum), POPUP_TYPE_NORMAL, &opts[typeNum], typeNum);
}


// Show an popup
void ShowPopupEx(HANDLE hContact, const TCHAR *title, const TCHAR *description, 
			   void *plugin_data, int type, const Options *op, int typeNum)
{
#ifdef UNICODE
	if(ServiceExists(MS_POPUP_ADDPOPUPW)) 
	{
		// Make popup
		POPUPDATAW ppd;

		ZeroMemory(&ppd, sizeof(ppd)); 
		ppd.lchContact = hContact; 
		ppd.lchIcon = HistoryEvents_GetIcon(types[typeNum].eventType);

		if (title != NULL)
			lstrcpyn(ppd.lpwzContactName, title, MAX_REGS(ppd.lpwzContactName));
		else if (hContact != NULL)
			lstrcpyn(ppd.lpwzContactName, (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR), 
					MAX_REGS(ppd.lpwzContactName));

		if (description != NULL)
			lstrcpyn(ppd.lpwzText, description, MAX_REGS(ppd.lpwzText));

		if (type == POPUP_TYPE_NORMAL || type == POPUP_TYPE_TEST)
		{
			if (op->popup_use_default_colors)
			{
				ppd.colorBack = 0;
				ppd.colorText = 0;
			}
			else if (op->popup_use_win_colors)
			{
				ppd.colorBack = GetSysColor(COLOR_BTNFACE);
				ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			}
			else
			{
				ppd.colorBack = op->popup_bkg_color;
				ppd.colorText = op->popup_text_color;
			}
		}
		else // if (type == POPUP_TYPE_ERROR)
		{
			ppd.colorBack = RGB(200,0,0);
			ppd.colorText = RGB(255,255,255);
		}

		if (type == POPUP_TYPE_NORMAL)
		{
			ppd.PluginWindowProc = PopupDlgProc;
			ppd.PluginData = plugin_data;
		}
		else // if (type == POPUP_TYPE_TEST || type == POPUP_TYPE_ERROR)
		{
			ppd.PluginWindowProc = DumbPopupDlgProc;
		}
		
		if (type == POPUP_TYPE_NORMAL || type == POPUP_TYPE_TEST)
		{
			switch (op->popup_delay_type) 
			{
				case POPUP_DELAY_CUSTOM:
					ppd.iSeconds = op->popup_timeout;
					break;

				case POPUP_DELAY_PERMANENT:
					ppd.iSeconds = -1;
					break;

				//case POPUP_DELAY_DEFAULT:
				default:
					ppd.iSeconds = 0;
					break;
			}
		}
		else // if (type == POPUP_TYPE_ERROR)
		{
			ppd.iSeconds = 0;
		}

		// Now that every field has been filled, we want to see the popup.
		CallService(MS_POPUP_ADDPOPUPW, (WPARAM)&ppd,0);

		HistoryEvents_ReleaseIcon(ppd.lchIcon);
	}
	else
#endif
	if(ServiceExists(MS_POPUP_ADDPOPUPEX)) 
	{
		// Make popup
		POPUPDATAEX ppd;

		ZeroMemory(&ppd, sizeof(ppd)); 
		ppd.lchContact = hContact; 
		ppd.lchIcon = HistoryEvents_GetIcon(types[typeNum].eventType);

		if (title != NULL)
		{
			char *tmp = mir_t2a(title);
			lstrcpynA(ppd.lpzContactName, tmp, MAX_REGS(ppd.lpzContactName));
			mir_free(tmp);
		}
		else
			lstrcpynA(ppd.lpzContactName, (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0), 
					MAX_REGS(ppd.lpzContactName));

		if (description != NULL)
		{
			char *tmp = mir_t2a(description);
			lstrcpynA(ppd.lpzText, tmp, MAX_REGS(ppd.lpzText));
			mir_free(tmp);
		}

		if (type == POPUP_TYPE_NORMAL || type == POPUP_TYPE_TEST)
		{
			if (op->popup_use_default_colors)
			{
				ppd.colorBack = 0;
				ppd.colorText = 0;
			}
			else if (op->popup_use_win_colors)
			{
				ppd.colorBack = GetSysColor(COLOR_BTNFACE);
				ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			}
			else
			{
				ppd.colorBack = op->popup_bkg_color;
				ppd.colorText = op->popup_text_color;
			}
		}
		else // if (type == POPUP_TYPE_ERROR)
		{
			ppd.colorBack = RGB(200,0,0);
			ppd.colorText = RGB(255,255,255);
		}

		if (type == POPUP_TYPE_NORMAL)
		{
			ppd.PluginWindowProc = PopupDlgProc;
			ppd.PluginData = plugin_data;
		}
		else // if (type == POPUP_TYPE_TEST || type == POPUP_TYPE_ERROR)
		{
			ppd.PluginWindowProc = DumbPopupDlgProc;
		}
		
		if (type == POPUP_TYPE_NORMAL || type == POPUP_TYPE_TEST)
		{
			switch (op->popup_delay_type) 
			{
				case POPUP_DELAY_CUSTOM:
					ppd.iSeconds = op->popup_timeout;
					break;

				case POPUP_DELAY_PERMANENT:
					ppd.iSeconds = -1;
					break;

				//case POPUP_DELAY_DEFAULT:
				default:
					ppd.iSeconds = 0;
					break;
			}
		}
		else // if (type == POPUP_TYPE_ERROR)
		{
			ppd.iSeconds = 0;
		}

		// Now that every field has been filled, we want to see the popup.
		CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd,0);

		HistoryEvents_ReleaseIcon(ppd.lchIcon);
	}
}


// Handle to the hidden windows to handle actions for popup clicks
LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	if (uMsg == WMU_ACTION)
	{
		if (lParam == POPUP_ACTION_OPENHISTORY)
		{
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, wParam, 0);
		}
		else if (lParam == POPUP_ACTION_OPENSRMM)
		{
			CallService(MS_MSG_SENDMESSAGE, wParam, 0);
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


// Handle to popup events
static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_COMMAND:
		{
			PopupData *pd = (PopupData *) PUGetPluginData(hWnd);
			int action = opts[pd->type].popup_left_click_action;

			SendMessage(hPopupWindow, WMU_ACTION, (WPARAM) pd->hContact, action);

			if (action != POPUP_ACTION_DONOTHING)
				PUDeletePopUp(hWnd);

			return TRUE;
		}

		case WM_CONTEXTMENU: 
		{
			PopupData *pd = (PopupData *) PUGetPluginData(hWnd);
			int action = opts[pd->type].popup_right_click_action;

			SendMessage(hPopupWindow, WMU_ACTION, (WPARAM) pd->hContact, action);

			if (action != POPUP_ACTION_DONOTHING)
				PUDeletePopUp(hWnd);

			return TRUE;
		}

		case UM_FREEPLUGINDATA: 
		{
			delete (PopupData *) PUGetPluginData(hWnd);
			return TRUE;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


// Handle to popup events
static LRESULT CALLBACK DumbPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_COMMAND:
		{
			PUDeletePopUp(hWnd);
			return TRUE;
		}

		case WM_CONTEXTMENU: 
		{
			PUDeletePopUp(hWnd);
			return TRUE;
		}

		case UM_FREEPLUGINDATA: 
		{
			return TRUE;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

