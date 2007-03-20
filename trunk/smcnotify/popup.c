/*
Status Message Change Notify plugin for Miranda IM.

Copyright © 2004-2005 NoName
Copyright © 2005-2006 Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci

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

#include "commonheaders.h"


static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


void PopupNotify(STATUSMSGINFO *smi, SMCNOTIFY_PUOPTIONS *puo) {
	POPUPDATAT ppd;
	TCHAR *str = GetStr(smi, puo->text);

#ifdef CUSTOMBUILD_OSDSUPPORT
	if (ServiceExists("OSD/Announce") && puo->bUseOSD)
	{
		int timerOSD = options.bInfiniteDelay ? 2147483647 : (int)(options.dSec * 1000);
		//char *szOSDText = (char*)smi->cust;
		//lstrcat(szOSDText, options.popuptext);
		CallService("OSD/Announce", (WPARAM)str, timerOSD);
		MIR_FREE(str);
		return;
	}
#endif

	ZeroMemory(&ppd, sizeof(ppd));
	ppd.lchContact = smi->hContact;
	ppd.lchIcon = LoadSkinnedProtoIcon(smi->proto, DBGetContactSettingWord(smi->hContact, smi->proto, "Status", ID_STATUS_ONLINE));
	lstrcpyn(ppd.lptzContactName, smi->cust, MAX_CONTACTNAME);
	lstrcpyn(ppd.lptzText, str, MAX_SECONDLINE);
	if (puo->bColorType == POPUP_COLOR_DEFAULT)
	{
		ppd.colorBack = 0;
		ppd.colorText = 0;
	}
	else if (puo->bColorType == POPUP_COLOR_WINDOWS)
	{
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
	}
	else if (puo->bColorType == POPUP_COLOR_CUSTOM)
	{
		ppd.colorBack = puo->colBack;
		ppd.colorText = puo->colText;
	}
	ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
	ppd.PluginData = NULL;
	if (puo->bDelayType == POPUP_DELAY_DEFAULT)
		ppd.iSeconds = 0;
	else if (puo->bDelayType == POPUP_DELAY_CUSTOM)
		ppd.iSeconds = puo->dDelay;
	else if (puo->bDelayType == POPUP_DELAY_PERMANENT)
		ppd.iSeconds = -1;
	CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, 0);

	MIR_FREE(str);
	return;
}

// return TRUE if timeout is over
static BOOL TimeoutCheck(HANDLE hContact, const char *module, const char *setting) {
	if (DBGetContactSettingDword(hContact, module, setting, 0) == 1) return TRUE;
	if ((GetTickCount() - DBGetContactSettingDword(hContact, module, setting, 0)) > TMR_CONNECTIONTIMEOUT)
	{
		DBWriteContactSettingDword(hContact, module, setting, 1);
		return TRUE;
	}
	return FALSE;
}

void PopupCheck(STATUSMSGINFO *smi) {
	BOOL ret = TRUE;

	if (puopts.bOnConnect && !puopts.bIfChanged && (smi->compare == 0))
		smi->ignore = SMII_ALL;

	if (puopts.bIgnoreRemove && (smi->compare == 2))
		ret = FALSE;

	else if (!TimeoutCheck(NULL, MODULE_NAME, smi->proto))
		ret = (puopts.bOnConnect && (smi->compare || !puopts.bIfChanged));

	else if ((BOOL)DBGetContactSettingByte(NULL, MODULE_NAME, "IgnoreAfterStatusChange", 0))
		ret = TimeoutCheck(smi->hContact, "UserOnline", "LastStatusChange");

	if (ret)
	{
		PopupNotify(smi, &puopts);
		SkinPlaySound("smcnotify");
	}

	return;
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	DWORD ID;
	HANDLE hContact;
	hContact = PUGetContact(hWnd);
	switch (msg)
	{
		case WM_COMMAND:
		case WM_CONTEXTMENU:
			if (msg == WM_COMMAND) // left click
				ID = puopts.LeftClickAction;
			else if (msg == WM_CONTEXTMENU) // right click
				ID = puopts.RightClickAction;

			if (ID == POPUP_ACTION_CLOSE) PUDeletePopUp(hWnd);
			SendMessage(hPopupWindow, WM_USER + 10 + ID, (WPARAM)hContact, 0);
			break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

static LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if ((HANDLE)wParam != NULL)
	{
		switch (msg - WM_USER - 10)
		{
			case POPUP_ACTION_MESSAGE: // open message window
				CallService(MS_MSG_SENDMESSAGE, wParam, 0);
				CallService("SRMsg/LaunchMessageWindow", wParam, (LONG)NULL);
				break;
			case POPUP_ACTION_MENU: // display contact menu
			{
				int retcmd;
				POINT pt;
				HMENU hMenu;
				hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, wParam, 0);
				GetCursorPos(&pt);
				retcmd = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
				CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(retcmd, MPCF_CONTACTMENU), (LPARAM)wParam);
				DestroyMenu(hMenu);
				break;
			}
			case POPUP_ACTION_INFO: // display contact detail
				CallService(MS_USERINFO_SHOWDIALOG, wParam, 0);
				break;
			case POPUP_ACTION_HISTORY: // view status message history
				if (ServiceExists("UserInfo/Reminder/AggrassiveBackup"))
					DBWriteContactSettingTString(NULL, "UserInfoEx", "LastItem", TranslateT("Status Message History"));
				else
					DBWriteContactSettingTString(NULL, "UserInfo", "LastTab", TranslateT("Status Message History"));
				CallService(MS_USERINFO_SHOWDIALOG, wParam, 0);
				break;
		}
	}
	if (msg == WM_MEASUREITEM) // Needed by the contact's context menu
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
	else if (msg == WM_DRAWITEM) // Needed by the contact's context menu
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

extern void InitPopups(void) {
	if(ServiceExists(MS_POPUP_ADDPOPUPEX)
#ifdef UNICODE
		|| ServiceExists(MS_POPUP_ADDPOPUPW)
#endif
		)
	{
		PopupActive = TRUE;

		//Window needed for popup commands
		hPopupWindow = CreateWindowEx(WS_EX_TOOLWINDOW, _T("static"), _T(MODULE_NAME) _T("_PopupWindow"),
			0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP,
			NULL, hInst, NULL);
		SetWindowLong(hPopupWindow, GWL_WNDPROC, (LONG)(WNDPROC)PopupWndProc);

	}
	else PopupActive = FALSE;

	return;
}

extern void DeinitPopups(void) {
	if (hPopupWindow)
	{
		SendMessage(hPopupWindow, WM_CLOSE, 0, 0);
	}

	return;
}
