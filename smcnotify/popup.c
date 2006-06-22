
#include "main.h"

// popup dialog pocess
// for selecting actions when click on the popup window
// use for displaying contact menu
static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	DWORD ID;
	HANDLE hContact;
	hContact = PUGetContact(hWnd);
	switch(message)
	{
		case WM_COMMAND:
		case WM_CONTEXTMENU:
			if (message == WM_COMMAND) // left click
				ID = options.LeftClickAction;
			else if (message == WM_CONTEXTMENU) // right click
				ID = options.RightClickAction;
			// kill the popup if no menu need to display, otherwise, keep the popup
			if (ID != IDM_M5) PUDeletePopUp(hWnd);
			SendMessage(hPopupWindow, ID, (WPARAM)hContact, 0);
			return TRUE;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// process for the popup window
// containing the code for popup actions
LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	POINT pt;
	HMENU hMenu;
	if ((HANDLE)wParam != NULL)
	{
		switch (uMsg)
		{
			case IDM_M2: // open message window
				CallService(MS_MSG_SENDMESSAGE, wParam, 0);
				break;
			case IDM_M3: // view status message history
				ShowHistory((HANDLE)wParam);
				break;
			case IDM_M4: // display contact detail
				CallService(MS_USERINFO_SHOWDIALOG, wParam, 0);
				break;
			case IDM_M5: // display contact menu
				hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, wParam, 0);
				GetCursorPos(&pt);
				hPopupContact = (HANDLE)wParam;
				TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
				DestroyMenu(hMenu);
				break;
			case WM_COMMAND: // Needed by the contact's context menu
				if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)hPopupContact))
					break;
				return FALSE;
			case WM_MEASUREITEM: // Needed by the contact's context menu
				return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
			case WM_DRAWITEM: // Needed by the contact's context menu
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
			default:
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void ShowPopup(STATUSMSGINFO *smi)
{
	POPUPDATAEX ppd;
	TCHAR *str = GetStr(smi, options.popuptext);

	// OSD
	if (ServiceExists("OSD/Announce") && options.bUseOSD)
	{
		int timerOSD = options.bInfiniteDelay ? 2147483647 : (int)(options.dSec * 1000);
		//char *szOSDText = (char*)smi->cust;
		//lstrcat(szOSDText, options.popuptext);
		//CallService("OSD/Announce", (WPARAM)GetStr(smi, szOSDText), (int)options.dSec*1000);
		CallService("OSD/Announce", (WPARAM)str, timerOSD);
		return;
	}

	ZeroMemory(&ppd, sizeof(POPUPDATAEX));
	ppd.lchContact = smi->hContact;
	ppd.lchIcon = LoadSkinnedProtoIcon(smi->proto, DBGetContactSettingWord(smi->hContact, smi->proto, "Status", ID_STATUS_ONLINE));
	lstrcpy(ppd.lpzContactName, smi->cust);
	lstrcpy(ppd.lpzText, str);
	ppd.colorBack = (options.bDefaultColor)?GetSysColor(COLOR_BTNFACE):options.colBack;
	ppd.colorText = (options.bDefaultColor)?GetSysColor(COLOR_WINDOWTEXT):options.colText;
	ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
	ppd.PluginData = NULL;
	ppd.iSeconds = options.bInfiniteDelay ? -1 : options.dSec;
	CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
	if (str) free(str);
}
