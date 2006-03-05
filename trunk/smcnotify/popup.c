
#include "main.h"

// make display and history strings
TCHAR* GetStr(STATUSMSGINFO n, const TCHAR *dis)
{
	TCHAR chr, *str, tmp[128];
	int i;
	int len;
	SYSTEMTIME time;
	str = (TCHAR *) malloc(512 * sizeof(TCHAR));
	str[0] = _T('\0');
	GetLocalTime(&time);
	len = lstrlen(dis);
	for (i = 0; i < len; i++)
	{
		tmp[0] = _T('\0');

		if (dis[i] == _T('%'))
		{
			i++;
			chr = dis[i];
			switch (chr)
			{
				case 'D':
					mir_sntprintf(tmp, sizeof(tmp), "%02i", time.wDay);
					break;
				case 'H':
					mir_sntprintf(tmp, sizeof(tmp), "%i", time.wHour);
					break;
				case 'M':
					mir_sntprintf(tmp, sizeof(tmp), "%02i", time.wMonth);
					break;
				case 'Y':
					mir_sntprintf(tmp, sizeof(tmp), "%i", time.wYear);
					break;
				case 'a':
					if (time.wHour > 11) strcat(tmp, "PM");
					if (time.wHour < 12) strcat(tmp, "AM");
					break;
				case 'c':
					lstrcpyn(tmp, n.cust, sizeof(tmp));
					break;
				case 'h':
					mir_sntprintf(tmp, sizeof(tmp), "%i", time.wHour%12 == 0 ? 12 : time.wHour%12);
					break;
				case 'm':
					mir_sntprintf(tmp, sizeof(tmp), "%02i", time.wMinute);
					break;
				case 'n':
					if (!strcmp(n.newstatusmsg, "")) strcat(tmp, TranslateT("<empty status message>"));
					else lstrcpyn(tmp, n.newstatusmsg, sizeof(tmp));
					break;
				case 'o':
					if (!strcmp(n.oldstatusmsg, "")) strcat(tmp, Translate("<empty status message>"));
					else lstrcpyn(tmp, n.oldstatusmsg, sizeof(tmp));
					break;
				case 's':
					mir_sntprintf(tmp, sizeof(tmp), "%02i", time.wSecond);
					break;
				default:
					strcat(tmp, "%");
					i--;
					break;
			}
		}
		else if (dis[i] == _T('\\'))
		{
			i++;
			chr = dis[i];
			switch (chr)
			{
				case 'n':
					strcat(tmp, "\r\n");
					break;
				case 't':
					strcat(tmp, "\t");
					break;
				default:
					strcat(tmp, "\\");
					i--;
					break;
			}
		}
		else mir_sntprintf(tmp, sizeof(tmp), "%c", dis[i]);

		if (tmp[0] != _T('\0'))
		{
			if (lstrlen(tmp) + lstrlen(str) < 508)
			{
				lstrcat(str, tmp);
			}
			else
			{
				lstrcat(str, _T("..."));
				break;
			}
		}
	}
	return str;
}

// popup dialog pocess
// for selecting actions when click on the popup window
// use for displaying contact menu
static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	DWORD ID;
	HANDLE hContact;
	hContact = PUGetContact(hWnd);
	switch(message) {
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
	if ((HANDLE)wParam != NULL) {
		switch (uMsg) {
			case IDM_M2: // brief info
				CallService(MS_MSG_SENDMESSAGE, wParam, 0);
				break;
			case IDM_M3: // read complete forecast
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

void ShowPopup(STATUSMSGINFO n)
{
	POPUPDATAEX ppd;
	char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)n.hContact, 0);
	TCHAR *str = GetStr(n, options.popuptext);
	// OSD
	if (ServiceExists("OSD/Announce") && options.bUseOSD)
	{
		int timerOSD = options.bInfiniteDelay ? 2147483647 : (int)options.dSec * 1000;
		//char *szOSDText = (char*)n.cust;
		//lstrcat(szOSDText, options.popuptext);
		//CallService("OSD/Announce", (WPARAM)GetStr(n, szOSDText), (int)options.dSec*1000);
		CallService("OSD/Announce", (WPARAM)str, timerOSD);
		return;
	}
	//if (options.bUseOSD) return;
	ZeroMemory(&ppd, sizeof(POPUPDATAEX));
	ppd.lchContact = n.hContact;
	ppd.lchIcon = LoadSkinnedProtoIcon(szProto, DBGetContactSettingWord(n.hContact, szProto, "Status", ID_STATUS_ONLINE));
	lstrcpy(ppd.lpzContactName, n.cust);
	lstrcpy(ppd.lpzText, str);
	ppd.colorBack = (options.bDefaultColor)?GetSysColor(COLOR_BTNFACE):options.colBack;
	ppd.colorText = (options.bDefaultColor)?GetSysColor(COLOR_WINDOWTEXT):options.colText;
	ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
	ppd.PluginData = NULL;
	ppd.iSeconds = options.bInfiniteDelay ? -1 : options.dSec;
	CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
	if (str) free(str);
}

int ProtoAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (ack->type == ACKTYPE_STATUS)
	{
		//We get here on a status change, or a status notification (meaning:
		//old status and new status are just like the same)
		WORD newStatus = (WORD)ack->lParam;
		WORD oldStatus = (WORD)ack->hProcess;
		char *szProtocol = (char*)ack->szModule;
		//Now we have the statuses and (a pointer to the string representing) the protocol.
		if (oldStatus == newStatus) return 0; //Useless message.
		if (newStatus == ID_STATUS_OFFLINE) {
			//The protocol switched to offline. Disable the popups for this protocol
			DBWriteContactSettingDword(NULL, MODULE, szProtocol, 0);
		}
		else if ((oldStatus < ID_STATUS_ONLINE) && (newStatus >= ID_STATUS_ONLINE)) {
			//The protocol changed from a disconnected status to a connected status.
			//Enable the popups for this protocol.
			DBWriteContactSettingDword(NULL, MODULE, szProtocol, GetTickCount());
		}

		if (HasToGetStatusMsgForProtocol(szProtocol))
		{
			// treat going from invisible to anything as connecting
			if ((oldStatus == ID_STATUS_INVISIBLE) && (newStatus >= ID_STATUS_ONLINE))
			{
				DBWriteContactSettingDword(NULL, MODULE, szProtocol, GetTickCount());
			}
		}
		return 0;
	}
	else if (ack->type == ACKTYPE_AWAYMSG)
	{
		// Remove from lists
		MessageGotForContact(ack->hContact);

		// Store in db?
		if (ack->result == ACKRESULT_SUCCESS && HasToGetStatusMsgForProtocol(ack->szModule) &&
			!HasToIgnoreContact(ack->hContact, ack->szModule))
		{
			// Store in db
			DBWriteContactSettingString(ack->hContact, "CList", "StatusMsg", (const char*)ack->lParam);
		}
	}

	return 0; //The protocol changed in a way we don't care.
}

