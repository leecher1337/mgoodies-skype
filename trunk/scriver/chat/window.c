/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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

#include "chat.h"
#include "../msgwindow.h"

#define __try
#define __except(x) if (0)
#define __finally

#define _try __try
#define _except __except
#define _finally __finally

extern HBRUSH		hEditBkgBrush;
extern HBRUSH		hListBkgBrush;
extern HANDLE		hSendEvent;
extern HINSTANCE	g_hInst;
extern HICON		hIcons[30];
extern struct		CREOleCallback reOleCallback;
extern HIMAGELIST	hImageList;
extern HMENU		g_hMenu;
extern BOOL			SmileyAddInstalled;
extern TABLIST *	g_TabList;
extern HIMAGELIST	hIconsList;
extern int eventMessageIcon;
extern int overlayIcon;

static WNDPROC OldSplitterProc;
static WNDPROC OldMessageProc;
static WNDPROC OldNicklistProc;
static WNDPROC OldTabProc;
static WNDPROC OldFilterButtonProc;
static WNDPROC OldLogProc;

extern void SubclassTabCtrl(HWND tabCtrl);
extern void UnsubclassTabCtrl(HWND tabCtrl);
extern HWND GetParentWindow(HANDLE hContact, BOOL bChat);


typedef struct
{
	time_t lastEnterTime;
	char szTabSave[20];
} MESSAGESUBDATA;


static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
		case WM_NCHITTEST:
			return HTCLIENT;
		case WM_SETCURSOR:
		{	RECT rc;
			GetClientRect(hwnd,&rc);
			SetCursor(rc.right>rc.bottom?LoadCursor(NULL, IDC_SIZENS):LoadCursor(NULL, IDC_SIZEWE));
			return TRUE;
		}
		case WM_LBUTTONDOWN:
			SetCapture(hwnd);
			return 0;
		case WM_MOUSEMOVE:
			if(GetCapture()==hwnd) {
				RECT rc;
				GetClientRect(hwnd,&rc);
				SendMessage(GetParent(hwnd),GC_SPLITTERMOVED,rc.right>rc.bottom?(short)HIWORD(GetMessagePos())+rc.bottom/2:(short)LOWORD(GetMessagePos())+rc.right/2,(LPARAM)hwnd);
			}
			return 0;
		case WM_LBUTTONUP:
			ReleaseCapture();
			PostMessage(GetParent(hwnd),WM_SIZE, 0, 0);
			return 0;
	}
	return CallWindowProc(OldSplitterProc,hwnd,msg,wParam,lParam);
}

static void	InitButtons(HWND hwndDlg, SESSION_INFO* si)
{
	MODULEINFO * pInfo = MM_FindModule(si->pszModule);

	SendDlgItemMessage(hwndDlg,IDC_CHAT_SMILEY,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(IDI_BSMILEY, "smiley", 0, 0 ));
	SendDlgItemMessage(hwndDlg,IDC_CHAT_BOLD,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(IDI_BBOLD, "bold", 0, 0 ));
	SendDlgItemMessage(hwndDlg,IDC_CHAT_ITALICS,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(IDI_BITALICS, "italics", 0, 0 ));
	SendDlgItemMessage(hwndDlg,IDC_CHAT_UNDERLINE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(IDI_BUNDERLINE, "underline", 0, 0 ));
	SendDlgItemMessage(hwndDlg,IDC_CHAT_COLOR,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(IDI_COLOR, "fgcol", 0, 0 ));
	SendDlgItemMessage(hwndDlg,IDC_CHAT_BKGCOLOR,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(IDI_BKGCOLOR, "bkgcol", 0, 0 ));
	SendDlgItemMessage(hwndDlg,IDC_CHAT_HISTORY,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(IDI_CHAT_HISTORY, "history", 0, 0 ));
	SendDlgItemMessage(hwndDlg,IDC_CHAT_CHANMGR,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(IDI_TOPICBUT, "settings", 0, 0 ));
	SendDlgItemMessage(hwndDlg,IDC_CHAT_CLOSE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(IDI_CLOSE, "close", 0, 0 ));
	SendDlgItemMessage(hwndDlg,IDC_CHAT_SHOWNICKLIST,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(si->bNicklistEnabled?IDI_NICKLIST:IDI_NICKLIST2, si->bNicklistEnabled?"nicklist":"nicklist2", 0, 0 ));
	SendDlgItemMessage(hwndDlg,IDC_CHAT_FILTER,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(si->bFilterEnabled?IDI_FILTER:IDI_FILTER2, si->bFilterEnabled?"filter":"filter2", 0, 0 ));

	SendDlgItemMessage(hwndDlg,IDC_CHAT_SMILEY, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg,IDC_CHAT_BOLD, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg,IDC_CHAT_ITALICS, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg,IDC_CHAT_UNDERLINE, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg,IDC_CHAT_BKGCOLOR, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg,IDC_CHAT_COLOR, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg,IDC_CHAT_HISTORY, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg,IDC_CHAT_SHOWNICKLIST, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg,IDC_CHAT_CHANMGR, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg,IDC_CHAT_FILTER, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg,IDC_CHAT_CLOSE, BUTTONSETASFLATBTN, 0, 0);

	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_SMILEY), BUTTONADDTOOLTIP, (WPARAM)Translate("Insert a smiley"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_BOLD), BUTTONADDTOOLTIP, (WPARAM)Translate("Make the text bold (CTRL+B)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_ITALICS), BUTTONADDTOOLTIP, (WPARAM)Translate("Make the text italicized (CTRL+I)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_UNDERLINE), BUTTONADDTOOLTIP, (WPARAM)Translate("Make the text underlined (CTRL+U)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_BKGCOLOR), BUTTONADDTOOLTIP, (WPARAM)Translate("Select a background color for the text (CTRL+L)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_COLOR), BUTTONADDTOOLTIP, (WPARAM)Translate("Select a foreground color for the text (CTRL+K)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_HISTORY), BUTTONADDTOOLTIP, (WPARAM)Translate("Show the history (CTRL+H)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_SHOWNICKLIST), BUTTONADDTOOLTIP, (WPARAM)Translate("Show/hide the nicklist (CTRL+N)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_CHANMGR), BUTTONADDTOOLTIP, (WPARAM)Translate("Control this room (CTRL+O)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_FILTER), BUTTONADDTOOLTIP, (WPARAM)Translate("Enable/disable the event filter (CTRL+F)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHAT_CLOSE), BUTTONADDTOOLTIP, (WPARAM)Translate("Close current tab (CTRL+F4)"), 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_BOLD, BUTTONSETASPUSHBTN, 0, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_ITALICS, BUTTONSETASPUSHBTN, 0, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_UNDERLINE, BUTTONSETASPUSHBTN, 0, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_COLOR, BUTTONSETASPUSHBTN, 0, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_BKGCOLOR, BUTTONSETASPUSHBTN, 0, 0);
//	SendDlgItemMessage(hwndDlg, IDC_CHAT_SHOWNICKLIST, BUTTONSETASPUSHBTN, 0, 0);
//	SendDlgItemMessage(hwndDlg, IDC_CHAT_FILTER, BUTTONSETASPUSHBTN, 0, 0);

	if (pInfo)
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_BOLD), pInfo->bBold);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_ITALICS), pInfo->bItalics);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_UNDERLINE), pInfo->bUnderline);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_COLOR), pInfo->bColor);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_BKGCOLOR), pInfo->bBkgColor);
		if(si->iType == GCW_CHATROOM)
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR), pInfo->bChanMgr);
	}
}






static int RoomWndResize(HWND hwndDlg,LPARAM lParam,UTILRESIZECONTROL *urc)
{
	RECT rcTabs;
	SESSION_INFO * si = (SESSION_INFO*)lParam;
	int			TabHeight;
	BOOL		bControl = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowTopButtons", 1);
	BOOL		bFormat = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowFormatButtons", 1);
	BOOL		bToolbar = bFormat || bControl;
	BOOL		bSend = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowSend", 0);
	BOOL		bNick = si->iType!=GCW_SERVER && si->bNicklistEnabled;
	BOOL		bTabs = g_Settings.TabsEnable;
	BOOL		bTabBottom = g_Settings.TabsAtBottom;

	GetClientRect(GetDlgItem(hwndDlg, IDC_CHAT_TAB), &rcTabs);
	TabHeight = rcTabs.bottom - rcTabs.top;
	TabCtrl_AdjustRect(GetDlgItem(hwndDlg, IDC_CHAT_TAB), FALSE, &rcTabs);
	TabHeight -= (rcTabs.bottom - rcTabs.top);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_SMILEY), SmileyAddInstalled&&bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_BOLD), bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_UNDERLINE), bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_ITALICS), bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_COLOR), bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_BKGCOLOR), bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_HISTORY), bControl?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_SHOWNICKLIST), bControl?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), bControl?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR), bControl?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDOK), bSend?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_SPLITTERX), bNick?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_CLOSE), g_Settings.TabsEnable?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_TAB), g_Settings.TabsEnable?SW_SHOW:SW_HIDE);
	if(si->iType != GCW_SERVER)
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), si->bNicklistEnabled?SW_SHOW:SW_HIDE);
	else
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), SW_HIDE);
	if(si->iType == GCW_SERVER)
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_SHOWNICKLIST), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR), FALSE);
	}
	else
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_SHOWNICKLIST), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), TRUE);
		if(si->iType == GCW_CHATROOM)
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR), MM_FindModule(si->pszModule)->bChanMgr);

	}

	switch(urc->wId) {
		case IDOK:
			urc->rcItem.left = bSend?315:urc->dlgNewSize.cx ;
			urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY+23;
			urc->rcItem.bottom = urc->dlgNewSize.cy -1;
			return RD_ANCHORX_RIGHT|RD_ANCHORY_CUSTOM;
		case IDC_CHAT_TAB:
			urc->rcItem.top = 1;
			urc->rcItem.left = 0;
			urc->rcItem.right = urc->dlgNewSize.cx- 24;
			urc->rcItem.bottom = bToolbar?(urc->dlgNewSize.cy - si->iSplitterY):(urc->dlgNewSize.cy - si->iSplitterY+20);
			return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;
		case IDC_CHAT_LOG:
			urc->rcItem.top = bTabs?(bTabBottom?0:rcTabs.top-1):0;
			urc->rcItem.left = 0;
			urc->rcItem.right = bNick?urc->dlgNewSize.cx - si->iSplitterX:urc->dlgNewSize.cx;
			urc->rcItem.bottom = bToolbar?(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight + 6:urc->dlgNewSize.cy - si->iSplitterY):(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight+26:urc->dlgNewSize.cy - si->iSplitterY+20);
			return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;
		case IDC_CHAT_LIST:
			urc->rcItem.top = bTabs?(bTabBottom?0:rcTabs.top-1):0;
			urc->rcItem.right = urc->dlgNewSize.cx ;
			urc->rcItem.left = urc->dlgNewSize.cx - si->iSplitterX + 2;
			urc->rcItem.bottom = bToolbar?(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight + 6:urc->dlgNewSize.cy - si->iSplitterY):(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight+26:urc->dlgNewSize.cy - si->iSplitterY+20);
			return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;
		case IDC_CHAT_SPLITTERX:
			urc->rcItem.right = urc->dlgNewSize.cx - si->iSplitterX+2;
			urc->rcItem.left = urc->dlgNewSize.cx - si->iSplitterX;
			urc->rcItem.bottom = bToolbar?(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight + 6:urc->dlgNewSize.cy - si->iSplitterY):(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight+26:urc->dlgNewSize.cy - si->iSplitterY+20);
			urc->rcItem.top = bTabs ?rcTabs.top:1;
			return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;
		case IDC_CHAT_SPLITTERY:
			urc->rcItem.top = bToolbar?urc->dlgNewSize.cy - si->iSplitterY:urc->dlgNewSize.cy - si->iSplitterY+20;
			urc->rcItem.bottom = bToolbar?(urc->dlgNewSize.cy - si->iSplitterY+2):(urc->dlgNewSize.cy - si->iSplitterY+22);
			return RD_ANCHORX_WIDTH|RD_ANCHORY_CUSTOM;
		case IDC_CHAT_MESSAGE:
			urc->rcItem.right = bSend?urc->dlgNewSize.cx - 64:urc->dlgNewSize.cx ;
			urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY+22;
			urc->rcItem.bottom = urc->dlgNewSize.cy -1 ;
			return RD_ANCHORX_LEFT|RD_ANCHORY_CUSTOM;
		case IDC_CHAT_SMILEY:
		case IDC_CHAT_ITALICS:
		case IDC_CHAT_BOLD:
		case IDC_CHAT_UNDERLINE:
		case IDC_CHAT_COLOR:
		case IDC_CHAT_BKGCOLOR:
			urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY+3;
			urc->rcItem.bottom = urc->dlgNewSize.cy - si->iSplitterY+19;
			return RD_ANCHORX_LEFT|RD_ANCHORY_CUSTOM;
		case IDC_CHAT_HISTORY:
		case IDC_CHAT_CHANMGR:
		case IDC_CHAT_SHOWNICKLIST:
		case IDC_CHAT_FILTER:
			urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY+3;
			urc->rcItem.bottom = urc->dlgNewSize.cy - si->iSplitterY+19;
			return RD_ANCHORX_RIGHT|RD_ANCHORY_CUSTOM;
		case IDC_CHAT_CLOSE:
			urc->rcItem.right = urc->dlgNewSize.cx-3;
			urc->rcItem.left = urc->dlgNewSize.cx - 19;
			urc->rcItem.bottom = bTabBottom?(bToolbar?urc->dlgNewSize.cy - si->iSplitterY-2:urc->dlgNewSize.cy - si->iSplitterY-2+20):19;
			urc->rcItem.top = bTabBottom?(bToolbar?urc->dlgNewSize.cy - si->iSplitterY-18:urc->dlgNewSize.cy - si->iSplitterY-18+20):3;
			return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;

	}
	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;

}

static LRESULT CALLBACK MessageSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MESSAGESUBDATA *dat;
	SESSION_INFO *Parentsi;

	Parentsi=(SESSION_INFO *)GetWindowLong(GetParent(hwnd),GWL_USERDATA);
    dat = (MESSAGESUBDATA *) GetWindowLong(hwnd, GWL_USERDATA);
    switch (msg) {
        case EM_SUBCLASSED:
			dat = (MESSAGESUBDATA *) malloc(sizeof(MESSAGESUBDATA));

			SetWindowLong(hwnd, GWL_USERDATA, (LONG) dat);
			dat->szTabSave[0] = '\0';
			dat->lastEnterTime = 0;
			return 0;
		case WM_MOUSEWHEEL:
			SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_LOG), WM_MOUSEWHEEL, wParam, lParam);
			dat->lastEnterTime = 0;
			return TRUE;
			break;
		case EM_REPLACESEL:
			PostMessage(hwnd, EM_ACTIVATE, 0, 0);
			break;
		case EM_ACTIVATE:
			SetActiveWindow(GetParent(hwnd));
			break;
 		case WM_CHAR:
			if (GetWindowLong(hwnd, GWL_STYLE) & ES_READONLY)
				break;

			if (wParam == 9 && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-i (italics)
			{
				return TRUE;
			}
			if (wParam == VK_SPACE && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-space (paste clean text)
			{
				return TRUE;
			}

			if (wParam == '\n' || wParam == '\r')
			{
				if (((GetKeyState(VK_CONTROL) & 0x8000) != 0) ^ (0 != DBGetContactSettingByte(NULL, "Chat", "SendOnEnter", 1)))
				{

					PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
					return 0;
				}
				if (DBGetContactSettingByte(NULL, "Chat", "SendOnDblEnter", 0))
				{
					if (dat->lastEnterTime + 2 < time(NULL))
						dat->lastEnterTime = time(NULL);
					else
					{
						SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
						SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
						PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
						return 0;
					}
				}
			}
			else
				dat->lastEnterTime = 0;

			if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) {      //ctrl-a
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				return 0;
			}
		break;
		case WM_KEYDOWN:
			{
			static int start, end;
 			if (wParam == VK_RETURN)
			{
				dat->szTabSave[0] = '\0';
				if (((GetKeyState(VK_CONTROL) & 0x8000) != 0) ^ (0 != DBGetContactSettingByte(NULL, "Chat", "SendOnEnter", 1)))
				{
					return 0;
				}
				if (DBGetContactSettingByte(NULL, "Chat", "SendOnDblEnter", 0))
				{
					if (dat->lastEnterTime + 2 >= time(NULL))
					{
						return 0;
					}
					break;
				}
				break;
			}
 			if (wParam == VK_TAB && GetKeyState(VK_SHIFT) & 0x8000 && !(GetKeyState(VK_CONTROL) & 0x8000)) // SHIFT-TAB (go to nick list)
			{
				SetFocus(GetDlgItem(GetParent(hwnd), IDC_CHAT_LIST));
				return TRUE;

			}
 			if (wParam == VK_TAB && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_SHIFT) & 0x8000)) // CTRL-TAB (switch tab/window)
			{
				if(g_Settings.TabsEnable)
					SendMessage(GetParent(hwnd), GC_SWITCHNEXTTAB, 0, 0);
				else
					ShowRoom(SM_GetNextWindow(Parentsi), WINDOW_VISIBLE, TRUE);
				return TRUE;

			}
 			if (wParam == VK_TAB && GetKeyState(VK_CONTROL) & 0x8000 && GetKeyState(VK_SHIFT) & 0x8000) // CTRL_SHIFT-TAB (switch tab/window)
			{
				if(g_Settings.TabsEnable)
					SendMessage(GetParent(hwnd), GC_SWITCHPREVTAB, 0, 0);
				else
					ShowRoom(SM_GetPrevWindow(Parentsi), WINDOW_VISIBLE, TRUE);
				return TRUE;

			}
			if (wParam <= '9' && wParam >= '1' && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // CTRL + 1 -> 9 (switch tab)
			{
				if(g_Settings.TabsEnable)
				{
					SendMessage(GetParent(hwnd), GC_SWITCHTAB, 0, (LPARAM)((int)wParam - (int)'1'));

				}

			}
			if (wParam <= VK_NUMPAD9 && wParam >= VK_NUMPAD1 && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // CTRL + 1 -> 9 (switch tab)
			{
				if(g_Settings.TabsEnable)
				{
					SendMessage(GetParent(hwnd), GC_SWITCHTAB, 0, (LPARAM)((int)wParam - (int)VK_NUMPAD1));

				}

			}
			if (wParam == VK_TAB && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000)) {    //tab-autocomplete
                char *pszText = NULL;
                int iLen;
				GETTEXTLENGTHEX gtl = {0};
				GETTEXTEX gt = {0};
				LRESULT lResult = (LRESULT)SendMessageA(hwnd, EM_GETSEL, (WPARAM)NULL, (LPARAM)NULL);

				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
				start = LOWORD(lResult);
				end = HIWORD(lResult);
				SendMessage(hwnd, EM_SETSEL, end, end);
				gtl.flags = GTL_PRECISE;
				gtl.codepage = CP_ACP;
				iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, (LPARAM)NULL);
				if (iLen >0)
				{
					char *pszName = NULL;
					char *pszSelName = NULL;
					pszText = malloc(iLen+100);

					gt.cb = iLen+99;
					gt.flags = GT_DEFAULT;
					gt.codepage = CP_ACP;

					SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);
					while ( start >0 && pszText[start-1] != ' ' && pszText[start-1] != 13 && pszText[start-1] != VK_TAB)
						start--;
					while (end < iLen && pszText[end] != ' ' && pszText[end] != 13 && pszText[end-1] != VK_TAB)
						end ++;

					if( dat->szTabSave[0] =='\0')
					{
						lstrcpynA(dat->szTabSave, pszText+start, end-start+1);
					}
					pszSelName= malloc(end-start+1);
					lstrcpynA(pszSelName, pszText+start, end-start+1);
					pszName = UM_FindUserAutoComplete(Parentsi->pUsers, dat->szTabSave, pszSelName);
					if(pszName == NULL)
					{
						pszName = dat->szTabSave;
						SendMessage(hwnd, EM_SETSEL, start, end);
						if (end !=start)
							SendMessageA(hwnd, EM_REPLACESEL, FALSE, (LPARAM) pszName);
						dat->szTabSave[0] = '\0';
					}
					else
					{
						SendMessage(hwnd, EM_SETSEL, start, end);
						if (end !=start)
							SendMessageA(hwnd, EM_REPLACESEL, FALSE, (LPARAM) pszName);
					}
					free(pszText);
					free(pszSelName);

				}

				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				return 0;
			}
			else
			{
				if(dat->szTabSave[0] != '\0' && wParam != VK_RIGHT && wParam != VK_LEFT
					&& wParam != VK_SPACE && wParam != VK_RETURN && wParam != VK_BACK
					&& wParam != VK_DELETE )
				{

					if(g_Settings.AddColonToAutoComplete && start == 0)
					{
						SendMessageA(hwnd, EM_REPLACESEL, FALSE, (LPARAM) ": ");
					}
				}
				dat->szTabSave[0] = '\0';
			}
			}
			if (wParam == VK_F4 && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-F4 (close tab)
			{
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_CLOSE, BN_CLICKED), 0);
				return TRUE;
			}
			if (wParam == 0x49 && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-i (italics)
			{
				CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_ITALICS) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_ITALICS, 0), 0);
				return TRUE;
			}
			if (wParam == 0x42 && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000) ) // ctrl-b (bold)
			{
				CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BOLD) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_BOLD, 0), 0);
				return TRUE;

			}
			if (wParam == 0x55 && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-u (paste clean text)
			{
				CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_UNDERLINE) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_UNDERLINE, 0), 0);
				return TRUE;

			}
			if (wParam == 0x4b && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-k (paste clean text)
			{
				CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_COLOR) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_COLOR, 0), 0);
				return TRUE;

			}
			if (wParam == VK_SPACE && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-space (paste clean text)
			{
				CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, BST_UNCHECKED);
				CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, BST_UNCHECKED);
				CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, BST_UNCHECKED);
				CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, BST_UNCHECKED);
				CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_BKGCOLOR, 0), 0);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_COLOR, 0), 0);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_BOLD, 0), 0);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_UNDERLINE, 0), 0);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_ITALICS, 0), 0);
				return TRUE;

			}
			if (wParam == 0x4c && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-l (paste clean text)
			{
				CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BKGCOLOR) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_BKGCOLOR, 0), 0);
				return TRUE;

			}
			if (wParam == 0x46 && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-f (paste clean text)
			{
				if(IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHAT_FILTER)))
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_FILTER, 0), 0);
				return TRUE;

			}
			if (wParam == 0x4e && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-n (nicklist)
			{
				if(IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHAT_SHOWNICKLIST)))
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_SHOWNICKLIST, 0), 0);
				return TRUE;

			}
			if (wParam == 0x48 && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-h (history)
			{
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_HISTORY, 0), 0);
				return TRUE;

			}
			if (wParam == 0x4f && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-o (options)
			{
				if(IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHAT_CHANMGR)))
					SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_CHANMGR, 0), 0);
				return TRUE;

			}
			if ((wParam == 45 && GetKeyState(VK_SHIFT) & 0x8000 || wParam == 0x56 && GetKeyState(VK_CONTROL) & 0x8000 )&& !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-v (paste clean text)
			{
				SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
				return TRUE;

			}
			if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) // ctrl-w (close window)
			{
				PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
				return TRUE;

			}
			if (wParam == VK_NEXT || wParam == VK_PRIOR)
			{
				HWND htemp = GetParent(hwnd);
				SendDlgItemMessage(htemp, IDC_CHAT_LOG, msg, wParam, lParam);
		        dat->lastEnterTime = 0;
				return TRUE;
			}
			if (wParam == VK_UP && (GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000))
			{
			    int iLen;
				GETTEXTLENGTHEX gtl = {0};
				SETTEXTEX ste;
				LOGFONT lf;
				char *lpPrevCmd = SM_GetPrevCommand(Parentsi->pszID, Parentsi->pszModule);

				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

				LoadMsgDlgFont(17, &lf, NULL);
				ste.flags = ST_DEFAULT;
				ste.codepage = CP_ACP;
				if(lpPrevCmd)
					SendMessageA(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpPrevCmd);
				else
					SetWindowText(hwnd, _T(""));

				gtl.flags = GTL_PRECISE;
				gtl.codepage = CP_ACP;
				iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, (LPARAM)NULL);
				SendMessage(hwnd, EM_SCROLLCARET, 0,0);
				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				SendMessage(hwnd, EM_SETSEL,iLen,iLen);
		        dat->lastEnterTime = 0;
				return TRUE;
			}
			if (wParam == VK_DOWN && (GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000))
			{

				int iLen;
				GETTEXTLENGTHEX gtl = {0};
				SETTEXTEX ste;

				char *lpPrevCmd = SM_GetNextCommand(Parentsi->pszID, Parentsi->pszModule);
				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

				ste.flags = ST_DEFAULT;
				ste.codepage = CP_ACP;
				if(lpPrevCmd)
					SendMessageA(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM) lpPrevCmd);
				else
					SetWindowText(hwnd, _T(""));

				gtl.flags = GTL_PRECISE;
				gtl.codepage = CP_ACP;
				iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, (LPARAM)NULL);
				SendMessage(hwnd, EM_SCROLLCARET, 0,0);
				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				SendMessage(hwnd, EM_SETSEL,iLen,iLen);
		        dat->lastEnterTime = 0;
				return TRUE;

			}
			if (wParam == VK_RETURN)
                break;
            //fall through
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_KILLFOCUS:

			dat->lastEnterTime = 0;
            break;
		case WM_RBUTTONDOWN:
			{
				CHARRANGE sel, all = { 0, -1 };
				POINT pt;
				UINT uID = 0;
				HMENU hSubMenu;

				hSubMenu = GetSubMenu(g_hMenu, 4);
				CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
				SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) & sel);

				EnableMenuItem(hSubMenu, ID_MESSAGE_UNDO, SendMessage(hwnd, EM_CANUNDO, 0,0)?MF_ENABLED:MF_GRAYED);
				EnableMenuItem(hSubMenu, ID_MESSAGE_REDO, SendMessage(hwnd, EM_CANREDO, 0,0)?MF_ENABLED:MF_GRAYED);
				EnableMenuItem(hSubMenu, ID_MESSAGE_COPY, sel.cpMax!=sel.cpMin?MF_ENABLED:MF_GRAYED);
				EnableMenuItem(hSubMenu, ID_MESSAGE_CUT, sel.cpMax!=sel.cpMin?MF_ENABLED:MF_GRAYED);

				dat->lastEnterTime = 0;

				pt.x = (short) LOWORD(lParam);
				pt.y = (short) HIWORD(lParam);
				ClientToScreen(hwnd, &pt);

				uID = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
				switch (uID)
				{
				case 0:
					break;
               case ID_MESSAGE_UNDO:
					{
                    SendMessage(hwnd, EM_UNDO, 0, 0);
                    }break;
               case ID_MESSAGE_REDO:
					{
                    SendMessage(hwnd, EM_REDO, 0, 0);
                    }break;
                case ID_MESSAGE_COPY:
					{
                    SendMessage(hwnd, WM_COPY, 0, 0);
                    }break;
               case ID_MESSAGE_CUT:
					{
                    SendMessage(hwnd, WM_CUT, 0, 0);
                    }break;
               case ID_MESSAGE_PASTE:
					{
                    SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
                    }break;
                 case ID_MESSAGE_SELECTALL:
					{
                    SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & all);
                    }break;
                case ID_MESSAGE_CLEAR:
					{
                    SetWindowText(hwnd, _T( "" ));
                    }break;
			   default:
					break;

				}
				PostMessage(hwnd, WM_KEYUP, 0, 0 );

			}break;

        case WM_KEYUP:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
			{
			CHARFORMAT2 cf;
			UINT u = 0;
			UINT u2 = 0;
			COLORREF cr;

			LoadMsgDlgFont(17, NULL, &cr);

			cf.cbSize = sizeof(CHARFORMAT2);
			cf.dwMask = CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE|CFM_BACKCOLOR|CFM_COLOR;
			SendMessage(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

			if(MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bColor)
			{
				int index = GetColorIndex(Parentsi->pszModule, cf.crTextColor);
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_COLOR);

				if(index >= 0)
				{
					Parentsi->bFGSet = TRUE;
					Parentsi->iFG = index;
				}

				if(u == BST_UNCHECKED && cf.crTextColor != cr)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, BST_CHECKED);
				else if(u == BST_CHECKED && cf.crTextColor == cr)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, BST_UNCHECKED);
			}
			if(MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bBkgColor)
			{
				int index = GetColorIndex(Parentsi->pszModule, cf.crBackColor);
				COLORREF crB = (COLORREF)DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW));
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BKGCOLOR);

				if(index >= 0)
				{
					Parentsi->bBGSet = TRUE;
					Parentsi->iBG = index;
				}
				if(u == BST_UNCHECKED && cf.crBackColor != crB)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, BST_CHECKED);
				else if(u == BST_CHECKED && cf.crBackColor == crB)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, BST_UNCHECKED);
			}
			if(MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bBold)
			{
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BOLD);
				u2 = cf.dwEffects;
				u2 &= CFE_BOLD;
				if(u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, BST_CHECKED);
				else if(u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, BST_UNCHECKED);
			}

			if(MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bItalics)
			{
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_ITALICS);
				u2 = cf.dwEffects;
				u2 &= CFE_ITALIC;
				if(u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, BST_CHECKED);
				else if(u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, BST_UNCHECKED);
			}
			if(MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bUnderline)
			{
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_UNDERLINE);
				u2 = cf.dwEffects;
				u2 &= CFE_UNDERLINE;
				if(u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, BST_CHECKED);
				else if(u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, BST_UNCHECKED);
			}

            }break;

        case EM_UNSUBCLASSED:
			free(dat);
			return 0;
		default:break;
	}

	return CallWindowProc(OldMessageProc, hwnd, msg, wParam, lParam);

}

static BOOL CALLBACK FilterWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static SESSION_INFO * si = NULL;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		si = (SESSION_INFO *)lParam;
		CheckDlgButton(hwndDlg, IDC_CHAT_1, si->iLogFilterFlags&GC_EVENT_ACTION);
		CheckDlgButton(hwndDlg, IDC_CHAT_2, si->iLogFilterFlags&GC_EVENT_MESSAGE);
		CheckDlgButton(hwndDlg, IDC_CHAT_3, si->iLogFilterFlags&GC_EVENT_NICK);
		CheckDlgButton(hwndDlg, IDC_CHAT_4, si->iLogFilterFlags&GC_EVENT_JOIN);
		CheckDlgButton(hwndDlg, IDC_CHAT_5, si->iLogFilterFlags&GC_EVENT_PART);
		CheckDlgButton(hwndDlg, IDC_CHAT_6, si->iLogFilterFlags&GC_EVENT_TOPIC);
		CheckDlgButton(hwndDlg, IDC_CHAT_7, si->iLogFilterFlags&GC_EVENT_ADDSTATUS);
		CheckDlgButton(hwndDlg, IDC_CHAT_8, si->iLogFilterFlags&GC_EVENT_INFORMATION);
		CheckDlgButton(hwndDlg, IDC_CHAT_9, si->iLogFilterFlags&GC_EVENT_QUIT);
		CheckDlgButton(hwndDlg, IDC_CHAT_10, si->iLogFilterFlags&GC_EVENT_KICK);
		CheckDlgButton(hwndDlg, IDC_CHAT_11, si->iLogFilterFlags&GC_EVENT_NOTICE);

	}break;
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
	{
//		if((HWND)lParam==GetDlgItem(hwndDlg,IDC_CHAT_TEXTO))
		{
			SetTextColor((HDC)wParam,RGB(60,60,150));
			SetBkColor((HDC)wParam,GetSysColor(COLOR_WINDOW));
			return (BOOL)GetSysColorBrush(COLOR_WINDOW);
		}
	}break;

	case WM_ACTIVATE:
		{
			if(LOWORD(wParam) == WA_INACTIVE)
			{
				int iFlags = 0;

				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_1) == BST_CHECKED)
					iFlags |= GC_EVENT_ACTION;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_2) == BST_CHECKED)
					iFlags |= GC_EVENT_MESSAGE;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_3) == BST_CHECKED)
					iFlags |= GC_EVENT_NICK;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_4) == BST_CHECKED)
					iFlags |= GC_EVENT_JOIN;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_5) == BST_CHECKED)
					iFlags |= GC_EVENT_PART;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_6) == BST_CHECKED)
					iFlags |= GC_EVENT_TOPIC;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_7) == BST_CHECKED)
					iFlags |= GC_EVENT_ADDSTATUS;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_8) == BST_CHECKED)
					iFlags |= GC_EVENT_INFORMATION;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_9) == BST_CHECKED)
					iFlags |= GC_EVENT_QUIT;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_10) == BST_CHECKED)
					iFlags |= GC_EVENT_KICK;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_11) == BST_CHECKED)
					iFlags |= GC_EVENT_NOTICE;

				if (iFlags&GC_EVENT_ADDSTATUS)
					iFlags |= GC_EVENT_REMOVESTATUS;

				SendMessage(GetParent(hwndDlg), GC_CHANGEFILTERFLAG, 0, (LPARAM)iFlags);
				if(si->bFilterEnabled)
					SendMessage(GetParent(hwndDlg), GC_REDRAWLOG, 0, 0);
				PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
		}break;
	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
		default:break;
	}
	return(FALSE);
}
static LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
	{
	case WM_RBUTTONUP:
		{
			HWND hFilter = GetDlgItem(GetParent(hwnd), IDC_CHAT_FILTER);
			HWND hColor = GetDlgItem(GetParent(hwnd), IDC_CHAT_COLOR);
			HWND hBGColor = GetDlgItem(GetParent(hwnd), IDC_CHAT_BKGCOLOR);

			if (DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) != 0)
			{
				if(hFilter == hwnd)
					SendMessage(GetParent(hwnd), GC_SHOWFILTERMENU, 0, 0);
				if(hColor == hwnd)
					SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_COLOR);
				if(hBGColor == hwnd)
					SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_BKGCOLOR);
			}
		}break;

		default:break;
	}

	return CallWindowProc(OldFilterButtonProc, hwnd, msg, wParam, lParam);

}
static LRESULT CALLBACK LogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
	{

	case WM_LBUTTONUP:
		{
			CHARRANGE sel;

			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);
			if(sel.cpMin != sel.cpMax)
			{
				SendMessage(hwnd, WM_COPY, 0, 0);
				sel.cpMin = sel.cpMax ;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & sel);
			}
			SetFocus(GetDlgItem(GetParent(hwnd), IDC_CHAT_MESSAGE));
			break;


		}
	case WM_KEYDOWN:
			if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) // ctrl-w (close window)
			{
				PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
				return TRUE;

			}
			break;
	case WM_ACTIVATE:
		{
			if(LOWORD(wParam) == WA_INACTIVE)
			{
				CHARRANGE sel;
				SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);
				if(sel.cpMin != sel.cpMax)
				{
					sel.cpMin = sel.cpMax ;
					SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & sel);
				}

			}
		}break;
	case WM_CHAR:
		{
			SetFocus(GetDlgItem(GetParent(hwnd), IDC_CHAT_MESSAGE));
			SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_MESSAGE), WM_CHAR, wParam, lParam);
		}break;

		default:break;
	}

	return CallWindowProc(OldLogProc, hwnd, msg, wParam, lParam);

}

static LRESULT CALLBACK TabSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bDragging = FALSE;
	static int iBeginIndex = 0;
    switch (msg)
	{
	case WM_LBUTTONDOWN:
		{
			TCHITTESTINFO tci = {0};
			tci.pt.x=(short)LOWORD(GetMessagePos());
			tci.pt.y=(short)HIWORD(GetMessagePos());
			if(DragDetect(hwnd, tci.pt) && TabCtrl_GetItemCount(hwnd) >1 )
			{
				int i;
				tci.flags = TCHT_ONITEM;

				ScreenToClient(hwnd, &tci.pt);
				i= TabCtrl_HitTest(hwnd, &tci);
				if(i != -1)
				{
					TCITEM tc;
					SESSION_INFO * s = NULL;

					tc.mask = TCIF_PARAM;
					TabCtrl_GetItem(hwnd, i, &tc);
					s = (SESSION_INFO * ) tc.lParam;
					if(s)
					{
						BOOL bOnline = DBGetContactSettingWord(s->hContact, s->pszModule, "Status", ID_STATUS_OFFLINE) == ID_STATUS_ONLINE?TRUE:FALSE;
						bDragging = TRUE;
						iBeginIndex = i;
						ImageList_BeginDrag(hIconsList, bOnline?(MM_FindModule(s->pszModule))->OnlineIconIndex:(MM_FindModule(s->pszModule))->OfflineIconIndex, 8, 8);
						ImageList_DragEnter(hwnd,tci.pt.x, tci.pt.y);
						SetCapture(hwnd);
					}

					return TRUE;

				}
			}
			else
				PostMessage(GetParent(hwnd), GC_TABCLICKED, 0, 0 );
		}break;
	case WM_CAPTURECHANGED:
		{
				bDragging = FALSE;
				ImageList_DragLeave(hwnd);
				ImageList_EndDrag();

		}break;
	case WM_MOUSEMOVE:
		{
			if(bDragging)
			{
				TCHITTESTINFO tci = {0};
				tci.pt.x=(short)LOWORD(GetMessagePos());
				tci.pt.y=(short)HIWORD(GetMessagePos());
				ScreenToClient(hwnd, &tci.pt);
				ImageList_DragMove(tci.pt.x, tci.pt.y);

			}
		}break;
	case WM_LBUTTONUP:
		{
			if(bDragging && ReleaseCapture())
			{
				TCHITTESTINFO tci = {0};
				int i;
				tci.pt.x=(short)LOWORD(GetMessagePos());
				tci.pt.y=(short)HIWORD(GetMessagePos());
				tci.flags = TCHT_ONITEM;
				bDragging = FALSE;
				ImageList_DragLeave(hwnd);
				ImageList_EndDrag();

				ScreenToClient(hwnd, &tci.pt);
				i= TabCtrl_HitTest(hwnd, &tci);
				if(i != -1 && i != iBeginIndex)
				{
					SendMessage(GetParent(hwnd), GC_DROPPEDTAB, (WPARAM)i, (LPARAM)iBeginIndex);
				}


			}

		}break;
/*	case WM_MOUSEMOVE:
		{
			int  i = ReleaseCapture();
			i++;

		}break;
		*/
	case WM_LBUTTONDBLCLK:
		{
			TCHITTESTINFO tci = {0};
			int i = 0;

			tci.pt.x=(short)LOWORD(GetMessagePos());
			tci.pt.y=(short)HIWORD(GetMessagePos());
			tci.flags = TCHT_ONITEM;

			ScreenToClient(hwnd, &tci.pt);
			i = TabCtrl_HitTest(hwnd, &tci);
			if(i != -1 && g_Settings.TabCloseOnDblClick)
				PostMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_CLOSE, BN_CLICKED), 0);

		}break;
	case WM_MBUTTONUP:
		{
			TCHITTESTINFO tci = {0};
			int i = 0;

			tci.pt.x=(short)LOWORD(GetMessagePos());
			tci.pt.y=(short)HIWORD(GetMessagePos());
			tci.flags = TCHT_ONITEM;

			ScreenToClient(hwnd, &tci.pt);
			i = TabCtrl_HitTest(hwnd, &tci);
			if(i != -1 )
			{
				TCITEM tc;
				SESSION_INFO * si ;

				tc.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwnd, i, &tc);
				si = (SESSION_INFO * ) tc.lParam;
				if(si)
					SendMessage(GetParent(hwnd), GC_REMOVETAB, 1, (LPARAM) si );

/*
				if(TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_CHAT_TAB)) == i)
					PostMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_CLOSE, BN_CLICKED), 0);
				else
					TabCtrl_DeleteItem(hwnd, i);
*/
			}
		}break;

		default:break;
	}

	return CallWindowProc(OldTabProc, hwnd, msg, wParam, lParam);

}


static LRESULT CALLBACK NicklistSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
	{
	case WM_ERASEBKGND:
		{
		HDC dc = (HDC)wParam;
		SESSION_INFO * parentdat =(SESSION_INFO *)GetWindowLong(GetParent(hwnd),GWL_USERDATA);
		if(dc)
		{
			int height, index, items = 0;

			index = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
			if(index != LB_ERR && parentdat->nUsersInNicklist > 0)
			{
				items = parentdat->nUsersInNicklist - index;
				height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);

				if(height != LB_ERR)
				{
					RECT rc = {0};
					GetClientRect(hwnd, &rc);

					if(rc.bottom-rc.top > items * height)
					{
						rc.top = items*height;
						FillRect(dc, &rc, hListBkgBrush);
					}

				}
			}
			else
				return 0;
		}
		}
		return 1;
	case WM_KEYDOWN:
			if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) // ctrl-w (close window)
			{
				PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
				return TRUE;
			}
/*
			if (wParam == VK_TAB )
			{
				SetFocus(GetDlgItem(GetParent(hwnd), IDC_CHAT_MESSAGE));
				return TRUE;
			}

			if (wParam == VK_RETURN )
			{
				int item;
				USERINFO * ui;
				SESSION_INFO *parentdat =(SESSION_INFO *)GetWindowLong(GetParent(hwnd),GWL_USERDATA);

				item = SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_LIST), LB_GETCURSEL, 0, 0);
				if(item != LB_ERR)
				{
					ui = SM_GetUserFromIndex(parentdat->pszID, parentdat->pszModule, item);
					DoEventHookAsync(GetParent(hwnd), parentdat->pszID, parentdat->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, (LPARAM)NULL);
				}
				return TRUE;
			}
*/
			break;
	case WM_RBUTTONDOWN:
		{
			SendMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
		}break;
	case WM_RBUTTONUP:
		{
			SendMessage(hwnd, WM_LBUTTONUP, wParam, lParam);
		}break;

	case WM_CONTEXTMENU:
		{
			TVHITTESTINFO hti;
			int item;
			int height;
			USERINFO * ui;
			SESSION_INFO *parentdat =(SESSION_INFO *)GetWindowLong(GetParent(hwnd),GWL_USERDATA);


			hti.pt.x = (short) LOWORD(lParam);
			hti.pt.y = (short) HIWORD(lParam);
			if(hti.pt.x == -1 && hti.pt.y == -1)
			{
				int index = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
				int top = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
				height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);
				hti.pt.x = 4;
				hti.pt.y = (index - top)*height + 1;
			}
			else
				ScreenToClient(hwnd,&hti.pt);
			item = LOWORD(SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
			ui = SM_GetUserFromIndex(parentdat->pszID, parentdat->pszModule, item);
//			ui = (USERINFO *)SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_LIST), LB_GETITEMDATA, item, 0);
			if(ui)
			{
				HMENU hMenu = 0;
				UINT uID;
				USERINFO uinew;

				memcpy(&uinew, ui, sizeof(USERINFO));
				if(hti.pt.x == -1 && hti.pt.y == -1)
					hti.pt.y += height - 4;
				ClientToScreen(hwnd, &hti.pt);
				uID = CreateGCMenu(hwnd, &hMenu, 0, hti.pt, parentdat, uinew.pszUID, NULL);

				switch (uID)
				{
				case 0:
					break;
                case ID_MESS:
					{
						DoEventHookAsync(GetParent(hwnd), parentdat->pszID, parentdat->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, (LPARAM)NULL);
					}break;
 				default:
					DoEventHookAsync(GetParent(hwnd), parentdat->pszID, parentdat->pszModule, GC_USER_NICKLISTMENU, ui->pszUID, NULL, (LPARAM)uID);
					break;

				}
				DestroyGCMenu(&hMenu, 1);
				return TRUE;
			}


		}break;

		default:break;
	}

	return CallWindowProc(OldNicklistProc, hwnd, msg, wParam, lParam);

}
static int RestoreWindowPosition(HWND hwnd, HANDLE hContact, char * szModule, char * szNamePrefix, UINT showCmd)
{
	WINDOWPLACEMENT wp;
	char szSettingName[64];
	int x,y, width, height;;

	wp.length=sizeof(wp);
	GetWindowPlacement(hwnd,&wp);
//	if (hContact)
//	{
		wsprintfA(szSettingName,"%sx",szNamePrefix);
		x=DBGetContactSettingDword(hContact,szModule,szSettingName,-1);
		wsprintfA(szSettingName,"%sy",szNamePrefix);
		y=(int)DBGetContactSettingDword(hContact,szModule,szSettingName,-1);
		wsprintfA(szSettingName,"%swidth",szNamePrefix);
		width=DBGetContactSettingDword(hContact,szModule,szSettingName,-1);
		wsprintfA(szSettingName,"%sheight",szNamePrefix);
		height=DBGetContactSettingDword(hContact,szModule,szSettingName,-1);

		if(x==-1)
			return 0;
		wp.rcNormalPosition.left=x;
		wp.rcNormalPosition.top=y;
		wp.rcNormalPosition.right=wp.rcNormalPosition.left+width;
		wp.rcNormalPosition.bottom=wp.rcNormalPosition.top+height;
		wp.showCmd = showCmd;
		SetWindowPlacement(hwnd,&wp);
		return 1;
//	}
	return 0;

}
int GetTextPixelSize(char * pszText, HFONT hFont, BOOL bWidth)
{
	HDC hdc;
	HFONT hOldFont;
	RECT rc = {0};
	int i;

	if (!pszText || !hFont)
		return 0;
	hdc=GetDC(NULL);
	hOldFont = SelectObject(hdc, hFont);
	i = DrawTextA(hdc, pszText , -1, &rc, DT_CALCRECT);
	SelectObject(hdc, hOldFont);
	ReleaseDC(NULL,hdc);
	return bWidth?rc.right-rc.left:rc.bottom-rc.top;
}



struct FORK_ARG {
	HANDLE hEvent;
	void (__cdecl *threadcode)(void*);
	unsigned (__stdcall *threadcodeex)(void*);
	void *arg;
};
static void __cdecl forkthread_r(void *param)
{
	struct FORK_ARG *fa=(struct FORK_ARG*)param;
	void (*callercode)(void*)=fa->threadcode;
	void *arg=fa->arg;

	CallService(MS_SYSTEM_THREAD_PUSH,0,0);

	SetEvent(fa->hEvent);

	__try {
		callercode(arg);
	} __finally {
		CallService(MS_SYSTEM_THREAD_POP,0,0);
	}

	return;
}

static unsigned long forkthread (	void (__cdecl *threadcode)(void*),unsigned long stacksize,void *arg)
{
	unsigned long rc;
	struct FORK_ARG fa;

	fa.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	fa.threadcode=threadcode;
	fa.arg=arg;

	rc=_beginthread(forkthread_r,stacksize,&fa);

	if ((unsigned long)-1L != rc) {
		WaitForSingleObject(fa.hEvent,INFINITE);
	}
	CloseHandle(fa.hEvent);

	return rc;
}


static void __cdecl phase2(void * lParam)
{
	SESSION_INFO * si = (SESSION_INFO *) lParam;
	Sleep(30);
	if(si && si->hWnd)
		PostMessage(si->hWnd, GC_REDRAWLOG3, 0, 0);
}
BOOL CALLBACK RoomWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	SESSION_INFO * si;
	si = (SESSION_INFO *)GetWindowLong(hwndDlg,GWL_USERDATA);
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SESSION_INFO * psi = (SESSION_INFO*)lParam;
			int mask;

			TranslateDialogDefault(hwndDlg);
			SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)psi);
			OldSplitterProc=(WNDPROC)SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERX),GWL_WNDPROC,(LONG)SplitterSubclassProc);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERY),GWL_WNDPROC,(LONG)SplitterSubclassProc);
			OldNicklistProc=(WNDPROC)SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_LIST),GWL_WNDPROC,(LONG)NicklistSubclassProc);
			SubclassTabCtrl(GetDlgItem(hwndDlg,IDC_CHAT_TAB));
			OldLogProc=(WNDPROC)SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_LOG),GWL_WNDPROC,(LONG)LogSubclassProc);
			OldFilterButtonProc=(WNDPROC)SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_FILTER),GWL_WNDPROC,(LONG)ButtonSubclassProc);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_COLOR),GWL_WNDPROC,(LONG)ButtonSubclassProc);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_BKGCOLOR),GWL_WNDPROC,(LONG)ButtonSubclassProc);
			OldMessageProc = (WNDPROC)SetWindowLong(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), GWL_WNDPROC,(LONG)MessageSubclassProc);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SUBCLASSED, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_AUTOURLDETECT, 1, 0);
			mask = (int)SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_GETEVENTMASK, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_SETEVENTMASK, 0, mask | ENM_LINK | ENM_MOUSEEVENTS);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_LIMITTEXT, (WPARAM)sizeof(TCHAR)*0x7FFFFFFF, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_SETOLECALLBACK, 0, (LPARAM) & reOleCallback);

//			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
//			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_CHAT_LOG));

			TabCtrl_SetMinTabWidth(GetDlgItem(hwndDlg, IDC_CHAT_TAB), 80);
			TabCtrl_SetImageList(GetDlgItem(hwndDlg, IDC_CHAT_TAB), hIconsList);


			// restore previous tabs
			if(g_Settings.TabsEnable && DBGetContactSettingByte(NULL, "Chat", "TabRestore", 0))
			{
				TABLIST * node = g_TabList;
				while (node)
				{

					SESSION_INFO * s = SM_FindSession(node->pszID, node->pszModule);
					if(s)
					{
//						SendMessage(hwndDlg, GC_ADDTAB, -1, (LPARAM)s);
					}
					node = node->next;
				}
			}
			TabM_RemoveAll();

//			if(SmileyAddInstalled)
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_SMILEY), TRUE);

			SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_HIDESELECTION, TRUE, 0);

			SendMessage(hwndDlg, GC_SETWNDPROPS, 0, 0);
			SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
			SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
			SendMessage(hwndDlg, GC_SETWINDOWPOS, 0, 0);

			SendMessage(GetParent(hwndDlg), CM_ADDCHILD, (WPARAM) hwndDlg, (LPARAM) psi->hContact);
			SendMessage(GetParent(hwndDlg), CM_ACTIVATECHILD, 0, (LPARAM) hwndDlg);

		} break;





		case GC_SETWNDPROPS:
		{
			HICON hIcon;
			LoadGlobalSettings();
			InitButtons(hwndDlg, si);

			hIcon = si->wStatus==ID_STATUS_ONLINE?MM_FindModule(si->pszModule)->hOnlineIcon:MM_FindModule(si->pszModule)->hOfflineIcon;
			// stupid hack to make icons show. I dunno why this is needed currently
			if(!hIcon)
			{
				MM_IconsChanged();
				hIcon = si->wStatus==ID_STATUS_ONLINE?MM_FindModule(si->pszModule)->hOnlineIcon:MM_FindModule(si->pszModule)->hOfflineIcon;
			}

			SendMessage(hwndDlg, GC_FIXTABICONS, 0, 0);
			SendMessage(hwndDlg,WM_SETICON,ICON_BIG,(LPARAM)LoadIconEx(IDI_CHANMGR, "window", 0, 0));

			SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_SETBKGNDCOLOR , 0, g_Settings.crLogBackground);

			if(g_Settings.TabsEnable)
			{
				int mask = (int)GetWindowLong(GetDlgItem(hwndDlg, IDC_CHAT_TAB), GWL_STYLE);
				if(g_Settings.TabsAtBottom)
					mask |= TCS_BOTTOM;
				else
					mask &= ~TCS_BOTTOM;
				SetWindowLong(GetDlgItem(hwndDlg, IDC_CHAT_TAB), GWL_STYLE, (LONG)mask);
			}

			{ //messagebox
				COLORREF	crFore;

				CHARFORMAT2 cf;
				LoadMsgDlgFont(17, NULL, &crFore);
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = CFM_COLOR|CFM_BOLD|CFM_UNDERLINE|CFM_BACKCOLOR;
				cf.dwEffects = 0;
				cf.crTextColor = crFore;
				cf.crBackColor = (COLORREF)DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW));
				SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_SETBKGNDCOLOR , 0, DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW)));
				SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, WM_SETFONT, (WPARAM) g_Settings.MessageBoxFont, MAKELPARAM(TRUE, 0));
				SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, (WPARAM)SCF_ALL , (LPARAM)&cf);
			}
			{ // nicklist
				int ih;
				int ih2;
				int font;
				int height;

				ih = GetTextPixelSize("AQGglö", g_Settings.UserListFont,FALSE);
				ih2 = GetTextPixelSize("AQGglö", g_Settings.UserListHeadingsFont,FALSE);
				height = DBGetContactSettingByte(NULL, "Chat", "NicklistRowDist", 12);
				font = ih > ih2?ih:ih2;

				SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_SETITEMHEIGHT, 0, (LPARAM)height > font ? height : font);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NULL, TRUE);
			}
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			SendMessage(hwndDlg, GC_REDRAWLOG2, 0, 0);

		}break;

		case DM_UPDATETITLEBAR:
		{
			TitleBarData tbd;
			char szTemp [100];
			switch(si->iType)
			{
			case GCW_CHATROOM:
				mir_snprintf(szTemp, sizeof(szTemp), si->nUsersInNicklist ==1?Translate("%s: Chat Room (%u user)"):Translate("%s: Chat Room (%u users)"), si->pszName, si->nUsersInNicklist);
				break;
			case GCW_PRIVMESS:
				mir_snprintf(szTemp, sizeof(szTemp), si->nUsersInNicklist ==1?Translate("%s: Message Session"):Translate("%s: Message Session (%u users)"), si->pszName, si->nUsersInNicklist);
				break;
			case GCW_SERVER:
				mir_snprintf(szTemp, sizeof(szTemp), "%s: Server", si->pszName);
				break;
			default:break;
			}
			tbd.iFlags = TBDF_TEXT;
			tbd.pszText = charToTchar(szTemp, -1, CP_ACP);
			SendMessage(GetParent(hwndDlg), CM_UPDATETITLEBAR, (WPARAM) &tbd, (LPARAM) hwndDlg);
			free(tbd.pszText);
		} break;



		case DM_UPDATESTATUSBAR:
		{
			StatusBarData sbd;
			HICON hIcon;
			int iStatusbarParts[2];
			char *pszDispName = MM_FindModule(si->pszModule)->pszModDispName;
			char szTemp[512];
			hIcon = si->wStatus==ID_STATUS_ONLINE?MM_FindModule(si->pszModule)->hOnlineIcon:MM_FindModule(si->pszModule)->hOfflineIcon;
			// stupid hack to make icons show. I dunno why this is needed currently
			if(!hIcon)
			{
				MM_IconsChanged();
				hIcon = si->wStatus==ID_STATUS_ONLINE?MM_FindModule(si->pszModule)->hOnlineIcon:MM_FindModule(si->pszModule)->hOfflineIcon;
			}

			mir_snprintf(szTemp, SIZEOF(szTemp), "%s : %s", pszDispName, si->pszStatusbarText?si->pszStatusbarText:"");
			sbd.iItem = 0;
			sbd.iFlags = SBDF_TEXT | SBDF_ICON;
			sbd.hIcon = hIcon;
			sbd.pszText = charToTchar(szTemp, -1, CP_ACP);
			SendMessage(GetParent(hwndDlg), CM_UPDATESTATUSBAR, (WPARAM) &sbd, (LPARAM) hwndDlg);
			free(sbd.pszText);
			sbd.iItem = 1;
			sbd.hIcon = NULL;
			sbd.pszText	= _T("");
			SendMessage(GetParent(hwndDlg), CM_UPDATESTATUSBAR, (WPARAM) &sbd, (LPARAM) hwndDlg);
			sbd.iItem = 2;
			SendMessage(GetParent(hwndDlg), CM_UPDATESTATUSBAR, (WPARAM) &sbd, (LPARAM) hwndDlg);

			SendMessage(hwndDlg, GC_FIXTABICONS, 0, 0);
			return TRUE;
		} break;



		case GC_SETWINDOWPOS:
		{
			SESSION_INFO * pActive = GetActiveSession();
			WINDOWPLACEMENT wp;
			RECT screen;
			int savePerContact = DBGetContactSettingByte(NULL, "Chat", "SavePosition", 0);

			wp.length=sizeof(wp);
			GetWindowPlacement(hwndDlg,&wp);
			SystemParametersInfo(SPI_GETWORKAREA, 0,  &screen, 0);

			if (si->iX)
			{
				wp.rcNormalPosition.left = si->iX;
				wp.rcNormalPosition.top = si->iY;
				wp.rcNormalPosition.right = wp.rcNormalPosition.left + si->iWidth;
				wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + si->iHeight;
				wp.showCmd = SW_HIDE;
				SetWindowPlacement(hwndDlg,&wp);
//				SetWindowPos(hwndDlg, 0, si->iX,si->iY, si->iWidth, si->iHeight, SWP_NOZORDER |SWP_HIDEWINDOW|SWP_NOACTIVATE);
				break;
			}
			if(savePerContact)
			{
				if (RestoreWindowPosition(hwndDlg, g_Settings.TabsEnable?NULL:si->hContact, "Chat", "room", SW_HIDE))
					break;
				SetWindowPos(hwndDlg, 0, (screen.right-screen.left)/2- (550)/2,(screen.bottom-screen.top)/2- (400)/2, (550), (400), SWP_NOZORDER |SWP_HIDEWINDOW|SWP_NOACTIVATE);
			}
			else
				SetWindowPos(hwndDlg, 0, (screen.right-screen.left)/2- (550)/2,(screen.bottom-screen.top)/2- (400)/2, (550), (400), SWP_NOZORDER |SWP_HIDEWINDOW|SWP_NOACTIVATE);

			if(!g_Settings.TabsEnable && pActive && pActive->hWnd && DBGetContactSettingByte(NULL, "Chat", "CascadeWindows", 1))
			{
				RECT rcThis, rcNew;
				int dwFlag = SWP_NOZORDER|SWP_NOACTIVATE;
				if(!IsWindowVisible ((HWND)wParam))
					dwFlag |= SWP_HIDEWINDOW;

				GetWindowRect(hwndDlg, &rcThis);
				GetWindowRect(pActive->hWnd, &rcNew);

				{
					int offset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
					SetWindowPos((HWND) hwndDlg, 0, rcNew.left + offset, rcNew.top + offset, rcNew.right-rcNew.left, rcNew.bottom-rcNew.top, dwFlag);
				}

			}


		}break;




		case GC_SAVEWNDPOS:
		{
//			RECT rc = { 0 };
			WINDOWPLACEMENT wp = { 0 };

			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
//			GetWindowRect(hwndDlg, &rc);
/*
			// fix for when the taskbar is set to the top of the screen
			if(!IsIconic(hwndDlg) && !IsZoomed(hwndDlg))
			{
				g_Settings.iX = rc.left;
				g_Settings.iY = rc.top;
				g_Settings.iWidth = rc.right - rc.left;
				g_Settings.iHeight = rc.bottom - rc.top;
			}
			else
*/
			{
				g_Settings.iX = wp.rcNormalPosition.left;
				g_Settings.iY = wp.rcNormalPosition.top;
				g_Settings.iWidth = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
				g_Settings.iHeight = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
			}

			if(!lParam)
			{
				si->iX = g_Settings.iX;
				si->iY = g_Settings.iY;
				si->iWidth = g_Settings.iWidth;
				si->iHeight = g_Settings.iHeight;
			}
		}break;





		case WM_SIZE:
		{
			UTILRESIZEDIALOG urd;

			if(wParam == SIZE_MAXIMIZED)
				PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);

			if(IsIconic(hwndDlg)) break;
			ZeroMemory(&urd,sizeof(urd));
			urd.cbSize=sizeof(urd);
			urd.hInstance=g_hInst;
			urd.hwndDlg=hwndDlg;
			urd.lParam=(LPARAM)si;
			urd.lpTemplate=MAKEINTRESOURCEA(IDD_CHANNEL);
			urd.pfnResizer=RoomWndResize;
			CallService(MS_UTILS_RESIZEDIALOG,0,(LPARAM)&urd);

			RedrawWindow(GetDlgItem(hwndDlg,IDC_CHAT_MESSAGE), NULL, NULL, RDW_INVALIDATE);
			RedrawWindow(GetDlgItem(hwndDlg,IDOK), NULL, NULL, RDW_INVALIDATE);
			SendMessage(hwndDlg,GC_SAVEWNDPOS,0,1);


		} break;

/*
		AUTOLOCALE SUPPORT SOMETIME WILL GO HERE


        case WM_INPUTLANGCHANGE:
        if (GetFocus() == hwndDlg && GetForegroundWindow() == hwndDlg && GetActiveWindow() == hwndDlg)
		{
			char szKLName[KL_NAMELENGTH + 1];

			if((HKL)lParam != hkl)
			{
				hkl = (HKL)lParam;
				ActivateKeyboardLayout(hkl, 0);
				GetKeyboardLayoutNameA(szKLName);
				DBWriteContactSettingString(si->hContact, SRMSGMOD_T, "locale", szKLName);
				GetLocaleID(dat, szKLName);
				UpdateReadChars(hwndDlg, dat);
			}
        }break;

        case DM_LOADLOCALE:

            if(dat->dwFlags & MWF_WASBACKGROUNDCREATE)
                break;
            if (myGlobals.m_AutoLocaleSupport && dat->hContact != 0) {
                DBVARIANT dbv;
                int res;
                char szKLName[KL_NAMELENGTH+1];
                UINT flags = KLF_ACTIVATE;

                res = DBGetContactSetting(dat->hContact, SRMSGMOD_T, "locale", &dbv);
                if (res == 0 && dbv.type == DBVT_ASCIIZ) {

                    dat->hkl = LoadKeyboardLayoutA(dbv.pszVal, KLF_ACTIVATE);
                    PostMessage(hwndDlg, DM_SETLOCALE, 0, 0);
                    GetLocaleID(dat, dbv.pszVal);
                    DBFreeVariant(&dbv);
                } else {
                    GetKeyboardLayoutNameA(szKLName);
                    dat->hkl = LoadKeyboardLayoutA(szKLName, 0);
                    DBWriteContactSettingString(dat->hContact, SRMSGMOD_T, "locale", szKLName);
                    GetLocaleID(dat, szKLName);
                }
                UpdateReadChars(hwndDlg, dat);
            }
            return 0;
        case DM_SETLOCALE:
            if(dat->dwFlags & MWF_WASBACKGROUNDCREATE)
                break;
            if (dat->pContainer->hwndActive == hwndDlg && myGlobals.m_AutoLocaleSupport && dat->hContact != 0 && dat->pContainer->hwnd == GetForegroundWindow() && dat->pContainer->hwnd == GetActiveWindow()) {
                if (lParam == 0) {
                    if (GetKeyboardLayout(0) != dat->hkl) {
                        ActivateKeyboardLayout(dat->hkl, 0);
                    }
                } else {
                    dat->hkl = (HKL) lParam;
                    ActivateKeyboardLayout(dat->hkl, 0);
                }
            }
            return 0;

*/


		case GC_REDRAWWINDOW:
		{
			InvalidateRect(hwndDlg, NULL, TRUE);
		} break;


		case GC_REDRAWLOG:
		{
			si->LastTime = 0;
			if(si->pLog)
			{
				LOGINFO * pLog = si->pLog;
				if(si->iEventCount > 60)
				{
					int index = 0;
					while(index < 59)
					{
						if(pLog->next == NULL)
							break;
						pLog = pLog->next;
						if(si->iType != GCW_CHATROOM || !si->bFilterEnabled || (si->iLogFilterFlags&pLog->iType) != 0)
							index++;
					}
					Log_StreamInEvent(hwndDlg, pLog, si, TRUE, FALSE);
					forkthread(phase2, 0, (void *)si);
				}
				else
					Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE, FALSE);
			}
			else
				SendMessage(hwndDlg, GC_EVENT_CONTROL + WM_USER+500, WINDOW_CLEARLOG, 0);
		} break;

		case GC_REDRAWLOG2:
		{
			si->LastTime = 0;
			if(si->pLog)
				Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE, FALSE);
		} break;

		case GC_REDRAWLOG3:
		{
			si->LastTime = 0;
			if(si->pLog)
				Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE, TRUE);
		} break;


		case GC_ADDLOG:
		{
			if(si->pLogEnd)
				Log_StreamInEvent(hwndDlg, si->pLog, si, FALSE, FALSE);
			else
				SendMessage(hwndDlg, GC_EVENT_CONTROL + WM_USER+500, WINDOW_CLEARLOG, 0);
		} break;


		case GC_SWITCHTAB:
		{
			int total = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_CHAT_TAB));
			int i = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CHAT_TAB));
			if (i != -1 && total != -1 && total != 1 && i != lParam && total > lParam)
			{
				TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_CHAT_TAB), lParam);
				PostMessage(hwndDlg, GC_TABCLICKED, 0, 0 );
			}

		}break;


		case GC_REMOVETAB:
		{
			SESSION_INFO * s2;
			int i = -1;
			int tabId = 0;
			SESSION_INFO * s1 = (SESSION_INFO *) lParam;

			tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_CHAT_TAB));

			if(s1)
			{

				if(tabId)
				{
					for (i = 0; i < tabId; i++)
					{
						int ii;
						TCITEM tci = {0};
						tci.mask = TCIF_PARAM ;
						ii = TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &tci);
						if(ii != -1)
						{
							s2 = (SESSION_INFO *)tci.lParam;
							if (s1 == s2)
							{
								goto END_REMOVETAB;
							}
						}
					}
				}
			}
			else
				i = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CHAT_TAB));
END_REMOVETAB:
			if(i != -1 && i < tabId)
			{
				TCITEM id = {0};
				SESSION_INFO * s;
				TabCtrl_DeleteItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i);
				id.mask = TCIF_PARAM;
				if(!TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &id))
				{
					if(!TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i-1, &id))
						{
							SendMessage(hwndDlg, WM_CLOSE, 0, 0);
							break;
						}
				}
				s = (SESSION_INFO *)id.lParam;
				if(s)
					ShowRoom(s, (WPARAM)WINDOW_VISIBLE, wParam == 1?FALSE:TRUE);
			}
		}break;
		case DM_UPDATETABCONTROL:
		{
			TabControlData tcd;
			char szTemp [30];
			SESSION_INFO *s1 = (SESSION_INFO *) lParam;
			lstrcpynA(szTemp, s1->pszName, 21);
			if(lstrlenA(s1->pszName) >20)
				lstrcpynA(szTemp+20, "...", 4);

			tcd.iFlags = TCDF_TEXT;
			tcd.pszText = charToTchar(szTemp, -1, CP_ACP);
			SendMessage(GetParent(hwndDlg), CM_UPDATETABCONTROL, (WPARAM) &tcd, (LPARAM) hwndDlg);
			free(tcd.pszText);

			SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)s1);

		}break;

		case GC_FIXTABICONS:
		{
			SESSION_INFO * s = (SESSION_INFO *) lParam;
			SESSION_INFO * s2;
			int i;
			if(s)
			{
				TCITEM tci;
				int tabId;

				tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_CHAT_TAB));
				for (i = 0; i < tabId; i++)
				{
					tci.mask = TCIF_PARAM|TCIF_IMAGE ;
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &tci);
					s2 = (SESSION_INFO *)tci.lParam;
					if (s2 && s == s2)
					{
						int image = 0;
						if(!(s2->wState&GC_EVENT_HIGHLIGHT))
						{
							image = s2->wStatus==ID_STATUS_ONLINE?MM_FindModule(s2->pszModule)->OnlineIconIndex:MM_FindModule(s2->pszModule)->OfflineIconIndex;
							if(s2->wState&STATE_TALK)
								image++;
						}
						if(tci.iImage != image)
						{
							tci.mask = TCIF_IMAGE ;
							tci.iImage = image;
							TabCtrl_SetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &tci);
						}
					}


				}
			}
			else
				RedrawWindow(GetDlgItem(hwndDlg, IDC_CHAT_TAB), NULL, NULL, RDW_INVALIDATE);
		}break;
		case GC_SETMESSAGEHIGHLIGHT:
		{
			SESSION_INFO * s = (SESSION_INFO *) lParam;
			SESSION_INFO * s2;
			int i;
			if(s)
			{
				TCITEM tci;
				int tabId;

				tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_CHAT_TAB));
				for (i = 0; i < tabId; i++)
				{
					tci.mask = TCIF_PARAM ;
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &tci);
					s2 = (SESSION_INFO *)tci.lParam;
					if (s2 && s == s2)
					{ // highlight
						s2->wState |= GC_EVENT_HIGHLIGHT;
						if(SM_FindSession(si->pszID, si->pszModule) == s2)
							si->wState = s2->wState;
						SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)s2);
						if(DBGetContactSettingByte(NULL, "Chat", "FlashWindowHighlight", 0) != 0 && GetActiveWindow() != hwndDlg && GetForegroundWindow() != hwndDlg)
							SetTimer(hwndDlg, TIMERID_FLASHWND, 900, NULL);
						break;
					}


				}
			}
			else
				RedrawWindow(GetDlgItem(hwndDlg, IDC_CHAT_TAB), NULL, NULL, RDW_INVALIDATE);
		}break;
		case GC_SETTABHIGHLIGHT:
		{
			SESSION_INFO * s = (SESSION_INFO *) lParam;
			SESSION_INFO * s2;
			int i;
			if(s)
			{
				TCITEM tci;
				int tabId;

				tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_CHAT_TAB));
				for (i = 0; i < tabId; i++)
				{
					tci.mask = TCIF_PARAM ;
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &tci);
					s2 = (SESSION_INFO *)tci.lParam;
					if (s2 && s == s2)
					{ // highlight
						SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)s2);
					if(g_Settings.FlashWindow && GetActiveWindow() != hwndDlg && GetForegroundWindow() != hwndDlg)
						SetTimer(hwndDlg, TIMERID_FLASHWND, 900, NULL);

						break;
					}


				}
			}
			else
				RedrawWindow(GetDlgItem(hwndDlg, IDC_CHAT_TAB), NULL, NULL, RDW_INVALIDATE);
		}break;

		case GC_TABCLICKED:
		{
			int i;
			i = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CHAT_TAB));
			if(i != -1)
			{
				SESSION_INFO * s;
				TCITEM id = {0};

				id.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &id);
				s = (SESSION_INFO *)id.lParam;
				if(s)
				{
					if(s->wState&STATE_TALK)
					{
						s->wState &= ~STATE_TALK;

						DBWriteContactSettingWord(s->hContact, s->pszModule ,"ApparentMode",(LPARAM) 0);
					}

					if(s->wState&GC_EVENT_HIGHLIGHT)
					{
						s->wState &= ~GC_EVENT_HIGHLIGHT;

						if(CallService(MS_CLIST_GETEVENT, (WPARAM)s->hContact, (LPARAM)0))
							CallService(MS_CLIST_REMOVEEVENT, (WPARAM)s->hContact, (LPARAM)"chaticon");
					}

					SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)s);
					if(!s->hWnd)
					{
						ShowRoom(s, (WPARAM)WINDOW_VISIBLE, TRUE);
						SendMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
					}
				}

			}
		}break;



		case GC_SESSIONNAMECHANGE:
		{
			TCITEMA tci;
			int i;
			int tabId;
			SESSION_INFO * s2;
			SESSION_INFO * s1 = (SESSION_INFO * ) lParam;

			tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_CHAT_TAB));
			for (i = 0; i < tabId; i++)
			{
				int j;
				tci.mask = TCIF_PARAM ;
				j = TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &tci);
				if(j != -1)
				{
					s2 = (SESSION_INFO *)tci.lParam;
					if (s1 == s2)
					{
						tci.mask = TCIF_TEXT ;
						tci.pszText = s1->pszName ;
						SendMessageA( GetDlgItem(hwndDlg, IDC_CHAT_TAB), TCM_SETITEMA, (WPARAM)i, (LPARAM)&tci );
//						TabCtrl_SetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &tci);
					}
				}
			}
		}break;


		case GC_ACKMESSAGE:
		{
			SendDlgItemMessage(hwndDlg,IDC_CHAT_MESSAGE,EM_SETREADONLY,FALSE,0);
			SendDlgItemMessage(hwndDlg,IDC_CHAT_MESSAGE,WM_SETTEXT,0, (LPARAM)"");
			return TRUE;
		} break;




		case WM_CTLCOLORLISTBOX:
			SetBkColor((HDC) wParam, g_Settings.crUserListBGColor);
			return (BOOL) hListBkgBrush;


		case WM_MEASUREITEM:
		{

			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *) lParam;
			int ih = GetTextPixelSize("AQGglö", g_Settings.UserListFont,FALSE);
			int ih2 = GetTextPixelSize("AQGglö", g_Settings.UserListHeadingsFont,FALSE);
			int font = ih > ih2?ih:ih2;
			int height = DBGetContactSettingByte(NULL, "Chat", "NicklistRowDist", 12);

			mis->itemHeight = height > font?height:font;
			return TRUE;

		}

		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
			if(dis->CtlID == IDC_CHAT_LIST)
			{
				HFONT hFont, hoFont;
				HICON hIcon;
				char *pszText;
				int offset;
				int height;
				int index = dis->itemID;
				USERINFO * ui = SM_GetUserFromIndex(si->pszID, si->pszModule, index);
				if(ui)
				{

					height = dis->rcItem.bottom - dis->rcItem.top;

					if(height&1)
						height++;
					if(height == 10)
						offset = 0;
					else
						offset = height/2 - 4;
					hIcon = SM_GetStatusIcon(si, ui);
					hFont = ui->iStatusEx == 0?g_Settings.UserListFont:g_Settings.UserListHeadingsFont;
					hoFont = (HFONT) SelectObject(dis->hDC, hFont);
					SetBkMode(dis->hDC, TRANSPARENT);
					/*
					FillRect(dis->hDC, &dis->rcItem, hListBkgBrush);
					DrawIconEx(dis->hDC,2, dis->rcItem.top + offset,hIcon,10,10,0,NULL, DI_NORMAL);
					if (dis->itemAction == ODA_FOCUS && dis->itemState & ODS_SELECTED)
						FrameRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
					else if(dis->itemState & ODS_INACTIVE)
						FrameRect(dis->hDC, &dis->rcItem, hListBkgBrush);
					*/
					if (dis->itemAction == ODA_FOCUS && dis->itemState & ODS_SELECTED)
						FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
					else //if(dis->itemState & ODS_INACTIVE)
						FillRect(dis->hDC, &dis->rcItem, hListBkgBrush);
					DrawIconEx(dis->hDC,2, dis->rcItem.top + offset,hIcon,10,10,0,NULL, DI_NORMAL);



					SetTextColor(dis->hDC, ui->iStatusEx == 0?g_Settings.crUserListColor:g_Settings.crUserListHeadingsColor);
					pszText = ui->pszNick;
					TextOutA(dis->hDC, dis->rcItem.left+14, dis->rcItem.top, pszText, lstrlenA(pszText));
					SelectObject(dis->hDC, hoFont);
				}
				return TRUE;
			}
		}
		case GC_UPDATENICKLIST:
		{
			int i = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_GETTOPINDEX, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_SETCOUNT, si->nUsersInNicklist, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_SETTOPINDEX, i, 0);
			SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
		}break;

		case GC_EVENT_CONTROL + WM_USER+500:
		{
			switch(wParam)
			{
			case SESSION_OFFLINE:
				{
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
					SendMessage(si->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);

				}
				return TRUE;
			case SESSION_ONLINE:
				{
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
				}
				return TRUE;
			case WINDOW_HIDDEN:
				SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
				return TRUE;
			case WINDOW_CLEARLOG:
				SetDlgItemText(hwndDlg, IDC_CHAT_LOG, _T(""));
				return TRUE;
			case SESSION_TERMINATE:
				SendMessage(hwndDlg,GC_SAVEWNDPOS,0,0);
				if (DBGetContactSettingByte(NULL, "Chat", "SavePosition", 0))
				{
					DBWriteContactSettingDword(si->hContact, "Chat", "roomx", si->iX);
					DBWriteContactSettingDword(si->hContact, "Chat", "roomy", si->iY);
					DBWriteContactSettingDword(si->hContact, "Chat", "roomwidth" , si->iWidth);
					DBWriteContactSettingDword(si->hContact, "Chat", "roomheight", si->iHeight);
				}
				if(CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
					CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)"chaticon");
				si->wState &= ~STATE_TALK;
				DBWriteContactSettingWord(si->hContact, si->pszModule ,"ApparentMode",(LPARAM) 0);
				SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
				return TRUE;
			case WINDOW_MINIMIZE:
				ShowWindow(hwndDlg, SW_MINIMIZE);
				goto LABEL_SHOWWINDOW;
			case WINDOW_MAXIMIZE:
				ShowWindow(hwndDlg, SW_MAXIMIZE);
				goto LABEL_SHOWWINDOW;
			case SESSION_INITDONE:
				if(DBGetContactSettingByte(NULL, "Chat", "PopupOnJoin", 0)!=0)
					return TRUE;
				// fall through
			case WINDOW_VISIBLE:
				{
					if (IsIconic(hwndDlg))
						ShowWindow(hwndDlg, SW_NORMAL);
LABEL_SHOWWINDOW:
					SendMessage(hwndDlg, WM_SIZE, 0, 0);
					SendMessage(hwndDlg, GC_REDRAWLOG, 0, 0);
					SendMessage(hwndDlg, GC_UPDATENICKLIST, 0, 0);
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
					ShowWindow(hwndDlg, SW_SHOW);
					SendMessage(hwndDlg, WM_SIZE, 0, 0);
					SetForegroundWindow(hwndDlg);
				}
				return TRUE;
			default:break;
			}
		}break;



		case GC_SPLITTERMOVED:
		{	POINT pt;
			RECT rc;
			RECT rcLog;
			BOOL bFormat = IsWindowVisible(GetDlgItem(hwndDlg,IDC_CHAT_SMILEY));

			static int x = 0;

			GetWindowRect(GetDlgItem(hwndDlg,IDC_CHAT_LOG),&rcLog);
			if((HWND)lParam==GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERX)) {
				int oldSplitterX;
				GetClientRect(hwndDlg,&rc);
				pt.x=wParam; pt.y=0;
				ScreenToClient(hwndDlg,&pt);

				oldSplitterX=si->iSplitterX;
				si->iSplitterX=rc.right-pt.x+1;
				if(si->iSplitterX < 35)
					si->iSplitterX=35;
				if(si->iSplitterX > rc.right-rc.left-35)
					si->iSplitterX = rc.right-rc.left-35;
				g_Settings.iSplitterX = si->iSplitterX;
			}
			else if((HWND)lParam==GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERY)) {
				int oldSplitterY;
				GetClientRect(hwndDlg,&rc);
				pt.x=0; pt.y=wParam;
				ScreenToClient(hwndDlg,&pt);

				oldSplitterY=si->iSplitterY;
				si->iSplitterY=bFormat?rc.bottom-pt.y+1:rc.bottom-pt.y+20;
				if(si->iSplitterY<63)
					si->iSplitterY=63;
				if(si->iSplitterY>rc.bottom-rc.top-40)
					si->iSplitterY = rc.bottom-rc.top-40;
				g_Settings.iSplitterY = si->iSplitterY;
			}
			if(x==2)
			{
				PostMessage(hwndDlg,WM_SIZE,0,0);
				x = 0;
			}
			else
				x++;
		}break;





		case GC_FIREHOOK:
		{
			if (lParam)
			{
				GCHOOK * gch = (GCHOOK *) lParam;
				NotifyEventHooks(hSendEvent,0,(WPARAM)gch);
				if (gch->pDest)
				{
					if (gch->pDest->pszID)
						free(gch->pDest->pszID);
					if (gch->pDest->pszModule)
						free((char*)gch->pDest->pszModule);
					free(gch->pDest);
				}
				if (gch->pszText)
					free(gch->pszText);
				if (gch->pszUID)
					free(gch->pszUID);
				free(gch);
			}
		}	break;





		case GC_CHANGEFILTERFLAG:
		{
			si->iLogFilterFlags = lParam;
		}	break;





		case GC_SHOWFILTERMENU:
			{
			RECT rc;
    		HWND hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), hwndDlg, FilterWndProc, (LPARAM)si);
			GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), &rc);
			SetWindowPos(hwnd, HWND_TOP, rc.left-85, (IsWindowVisible(GetDlgItem(hwndDlg, IDC_CHAT_FILTER))||IsWindowVisible(GetDlgItem(hwndDlg, IDC_CHAT_BOLD)))?rc.top-206:rc.top-186, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
			}break;





		case GC_SHOWCOLORCHOOSER:
		{
			HWND ColorWindow;
			RECT rc;
			BOOL bFG = lParam == IDC_CHAT_COLOR?TRUE:FALSE;
			COLORCHOOSER * pCC = malloc(sizeof(COLORCHOOSER));

			GetWindowRect(GetDlgItem(hwndDlg, bFG?IDC_CHAT_COLOR:IDC_CHAT_BKGCOLOR), &rc);
			pCC->hWndTarget = GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE);
			pCC->pModule = MM_FindModule(si->pszModule);
			pCC->xPosition = rc.left+3;
			pCC->yPosition = IsWindowVisible(GetDlgItem(hwndDlg, IDC_CHAT_COLOR))?rc.top-1:rc.top+20;
			pCC->bForeground = bFG;
			pCC->si = si;

			ColorWindow= CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_COLORCHOOSER), hwndDlg, DlgProcColorToolWindow, (LPARAM) pCC);

		}break;


		case GC_SCROLLTOBOTTOM:
			{
				SCROLLINFO si = { 0 };
				if ((GetWindowLong(GetDlgItem(hwndDlg, IDC_CHAT_LOG), GWL_STYLE) & WS_VSCROLL) != 0)
				{
					CHARRANGE sel;
					si.cbSize = sizeof(si);
					si.fMask = SIF_PAGE | SIF_RANGE;
					GetScrollInfo(GetDlgItem(hwndDlg, IDC_CHAT_LOG), SB_VERT, &si);
					si.fMask = SIF_POS;
					si.nPos = si.nMax - si.nPage + 1;
					SetScrollInfo(GetDlgItem(hwndDlg, IDC_CHAT_LOG), SB_VERT, &si, TRUE);
					sel.cpMin = sel.cpMax = GetRichTextLength(GetDlgItem(hwndDlg, IDC_CHAT_LOG));
					SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_EXSETSEL, 0, (LPARAM) & sel);
					PostMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
				}

			}break;




 		case WM_TIMER:
		{
			if (wParam == TIMERID_FLASHWND)
			{
				FlashWindow(hwndDlg, TRUE);
			}
		}break;






		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) != WA_ACTIVE)
				break;
 		}

		//fall through
		case WM_MOUSEACTIVATE:
			{
			WINDOWPLACEMENT wp = { 0 };

//			InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NULL, FALSE);
			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			g_Settings.iX = wp.rcNormalPosition.left;
			g_Settings.iY = wp.rcNormalPosition.top;
			g_Settings.iWidth = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			g_Settings.iHeight = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

			if(g_Settings.TabsEnable)
			{
				int i = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CHAT_TAB));
				if(i != -1)
				{
					SESSION_INFO * s;
					TCITEM tci;

					tci.mask = TCIF_PARAM;
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &tci);
					s = (SESSION_INFO *) tci.lParam;
					if (s)
					{
						s->wState &= ~GC_EVENT_HIGHLIGHT;
						s->wState &= ~STATE_TALK;
						SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)s);
					}


				}
			}

			if(uMsg != WM_ACTIVATE)
				SetFocus(GetDlgItem(hwndDlg,IDC_CHAT_MESSAGE));

			SetActiveSession(si->pszID, si->pszModule);

			if (KillTimer(hwndDlg, TIMERID_FLASHWND))
				FlashWindow(hwndDlg, FALSE);
			if(DBGetContactSettingWord(si->hContact, si->pszModule ,"ApparentMode", 0) != 0)
				DBWriteContactSettingWord(si->hContact, si->pszModule ,"ApparentMode",(LPARAM) 0);
			if(CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
				CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)"chaticon");

			}break;





		case WM_NOTIFY:
		{
			LPNMHDR pNmhdr;

			pNmhdr = (LPNMHDR)lParam;
			switch (pNmhdr->code)
			{
			case NM_RCLICK:
			{
				if (pNmhdr->idFrom == IDC_CHAT_TAB  )
				{
					int i = TabCtrl_GetCurSel(pNmhdr->hwndFrom);

					if(i != -1)
					{
						SESSION_INFO * s;
						HMENU hSubMenu;
						TCHITTESTINFO tci = {0};
						TCITEM id = {0};
						int i = 0;
						id.mask = TCIF_PARAM;

						tci.pt.x=(short)LOWORD(GetMessagePos());
						tci.pt.y=(short)HIWORD(GetMessagePos());
						tci.flags = TCHT_ONITEM;

						ScreenToClient(GetDlgItem(hwndDlg, IDC_CHAT_TAB), &tci.pt);
						i = TabCtrl_HitTest(pNmhdr->hwndFrom, &tci);
						if(i != -1)
						{
							TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &id);
							s = (SESSION_INFO *)id.lParam;

							ClientToScreen(GetDlgItem(hwndDlg, IDC_CHAT_TAB), &tci.pt);
							hSubMenu = GetSubMenu(g_hMenu, 5);
							if(s)
							{
								WORD w = DBGetContactSettingWord(s->hContact, s->pszModule, "TabPosition", 0);
								if( w == 0)
									CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND|MF_UNCHECKED);
								else
									CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND|MF_CHECKED);

							}
							else
								CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND|MF_UNCHECKED);

							switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, tci.pt.x, tci.pt.y, 0, hwndDlg, NULL))
							{
							case ID_CLOSE:
								{
									if(TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CHAT_TAB)) == i)
										PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CHAT_CLOSE, BN_CLICKED), 0);
									else
									{
										TabCtrl_DeleteItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i);
									}
								}break;
							case ID_CLOSEOTHER:
								{
									int tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_CHAT_TAB)) - 1;
									if(tabId > 0)
									{
										if(TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CHAT_TAB)) != i)
										{
											if(s)
											{
												ShowRoom(s, WINDOW_VISIBLE, TRUE);
											}
										}
										for(; tabId >= 0; tabId --)
										{
											if(tabId == i)
												continue;
											TabCtrl_DeleteItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), tabId);

										}

									}
								}break;
							case ID_LOCKPOSITION:
								{
									TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), i, &id);
									if(!(GetMenuState(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND)&MF_CHECKED))
									{
										if(s->hContact)
											DBWriteContactSettingWord(s->hContact, s->pszModule, "TabPosition", (WORD)(i + 1));
									}
									else
										DBDeleteContactSetting(s->hContact, s->pszModule, "TabPosition");

								}break;
							default:break;

							}

						}
					}
				}



			}break;

			case EN_MSGFILTER:
			{
				if (pNmhdr->idFrom = IDC_CHAT_LOG && ((MSGFILTER *) lParam)->msg == WM_RBUTTONUP)
				{
					CHARRANGE sel, all = { 0, -1 };
					POINT pt;
					UINT uID = 0;
					HMENU hMenu = 0;
					char pszWord[4096];

					pt.x = (short) LOWORD(((ENLINK *) lParam)->lParam);
					pt.y = (short) HIWORD(((ENLINK *) lParam)->lParam);
					ClientToScreen(pNmhdr->hwndFrom, &pt);

					{ // fixing stuff for searches
						long iCharIndex, iLineIndex, iChars, start, end, iRes;
						POINTL ptl;

						pszWord[0] = '\0';
						ptl.x = (LONG)pt.x;
						ptl.y = (LONG)pt.y;
						ScreenToClient(GetDlgItem(hwndDlg, IDC_CHAT_LOG), (LPPOINT)&ptl);
						iCharIndex = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_CHARFROMPOS, 0, (LPARAM)&ptl);
						if (iCharIndex < 0)
							break;
						iLineIndex = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_EXLINEFROMCHAR, 0, (LPARAM)iCharIndex);
						iChars = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_LINEINDEX, (WPARAM)iLineIndex, 0 );
						start = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_FINDWORDBREAK, WB_LEFT, iCharIndex);//-iChars;
						end = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_FINDWORDBREAK, WB_RIGHT, iCharIndex);//-iChars;

						if(end - start > 0)
						{
							TEXTRANGEA tr;
							CHARRANGE cr;
							static char szTrimString[] = ":;,.!?\'\"><()[]- \r\n";
							ZeroMemory(&tr, sizeof(TEXTRANGE));

							cr.cpMin = start;
							cr.cpMax = end;
							tr.chrg = cr;
							tr.lpstrText = pszWord;
							iRes = SendMessageA(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_GETTEXTRANGE, 0, (LPARAM)&tr);

							if(iRes > 0)
							{
								int iLen = lstrlenA(pszWord)-1;
								while(iLen >= 0 && strchr(szTrimString, pszWord[iLen]))
								{
									pszWord[iLen] = '\0';
									iLen--;
								}
							}

						}

					}

					uID = CreateGCMenu(hwndDlg, &hMenu, 1, pt, si, NULL, pszWord);
					switch (uID)
					{
					case 0:
						PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
						break;
                     case ID_COPYALL:
						{
                         SendMessage(pNmhdr->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);
						SendMessage(pNmhdr->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & all);
                        SendMessage(pNmhdr->hwndFrom, WM_COPY, 0, 0);
						SendMessage(pNmhdr->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & sel);
        				PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
                        }break;
                     case ID_CLEARLOG:
						{
							SESSION_INFO * s = SM_FindSession(si->pszID, si->pszModule);
							if(s)
							{
								SetDlgItemText(hwndDlg, IDC_CHAT_LOG, _T(""));
								LM_RemoveAll(&s->pLog, &s->pLogEnd);
								s->iEventCount = 0;
								s->LastTime = 0;
								si->iEventCount = 0;
								si->LastTime = 0;
								si->pLog = s->pLog;
								si->pLogEnd = s->pLogEnd;
								PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
							}
						}break;
                     case ID_SEARCH_GOOGLE:
						{
							char szURL[4096] = "http://www.google.com/search?q=";
							if(pszWord[0])
							{
								lstrcatA(szURL, pszWord);
								CallService(MS_UTILS_OPENURL, 1, (LPARAM) szURL);
							}

							PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
						}break;
                     case ID_SEARCH_WIKIPEDIA:
						{
							char szURL[4096] = "http://en.wikipedia.org/wiki/";
							if(pszWord[0])
							{
								lstrcatA(szURL, pszWord);
								CallService(MS_UTILS_OPENURL, 1, (LPARAM) szURL);
							}
							PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
						}break;
				   default:
						PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
						DoEventHookAsync(hwndDlg, si->pszID, si->pszModule, GC_USER_LOGMENU, NULL, NULL, (LPARAM)uID);
						break;

					}
					DestroyGCMenu(&hMenu, 5);
				}
			}break;

			case EN_LINK:
			if(pNmhdr->idFrom = IDC_CHAT_LOG)
				switch (((ENLINK *) lParam)->msg)
				{
					case WM_RBUTTONDOWN:
					case WM_LBUTTONUP:
					{
						TEXTRANGEA tr;
						CHARRANGE sel;

						SendMessage(pNmhdr->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);
						if (sel.cpMin != sel.cpMax)
							break;
						tr.chrg = ((ENLINK *) lParam)->chrg;
						tr.lpstrText = malloc(tr.chrg.cpMax - tr.chrg.cpMin + 1);
						SendMessage(pNmhdr->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM) & tr);

						if (((ENLINK *) lParam)->msg == WM_RBUTTONDOWN)
						{
							HMENU hSubMenu;
							POINT pt;

							hSubMenu = GetSubMenu(g_hMenu, 2);
							CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
							pt.x = (short) LOWORD(((ENLINK *) lParam)->lParam);
							pt.y = (short) HIWORD(((ENLINK *) lParam)->lParam);
							ClientToScreen(((NMHDR *) lParam)->hwndFrom, &pt);
							switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL))
							{
								case ID_NEW:
									CallService(MS_UTILS_OPENURL, 1, (LPARAM) tr.lpstrText);
									break;
								case ID_CURR:
									CallService(MS_UTILS_OPENURL, 0, (LPARAM) tr.lpstrText);
									break;
								case ID_COPY:
								{
									HGLOBAL hData;
									if (!OpenClipboard(hwndDlg))
										break;
									EmptyClipboard();
									hData = GlobalAlloc(GMEM_MOVEABLE, lstrlenA(tr.lpstrText) + 1);
									lstrcpyA((char *) GlobalLock(hData), tr.lpstrText);
									GlobalUnlock(hData);
									SetClipboardData(CF_TEXT, hData);
									CloseClipboard();
									SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
									break;
								}
								default:break;
							}
							free(tr.lpstrText);
							return TRUE;
						}
						else
						{
							CallService(MS_UTILS_OPENURL, 1, (LPARAM) tr.lpstrText);
							SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
						}

						free(tr.lpstrText);
						break;
					}
				}
				break;
			}

		}break;





		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_CHAT_LIST:
			{
				if(HIWORD(wParam) == LBN_DBLCLK)
				{
					TVHITTESTINFO hti;
					int item;
					USERINFO * ui;

					hti.pt.x=(short)LOWORD(GetMessagePos());
					hti.pt.y=(short)HIWORD(GetMessagePos());
					ScreenToClient(GetDlgItem(hwndDlg, IDC_CHAT_LIST),&hti.pt);


					item = LOWORD(SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
					ui = SM_GetUserFromIndex(si->pszID, si->pszModule, item);
//					ui = (USERINFO *)SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_GETITEMDATA, item, 0);
					if(ui)
					{
						if(GetKeyState(VK_SHIFT) & 0x8000)
						{
							LRESULT lResult = (LRESULT)SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_GETSEL, (WPARAM)NULL, (LPARAM)NULL);
							int start = LOWORD(lResult);
							char * pszName = (char *) malloc(lstrlenA(ui->pszUID) + 3);
							if(start == 0)
								mir_snprintf(pszName, lstrlenA(ui->pszUID)+3, "%s: ", ui->pszUID);
							else
								mir_snprintf(pszName, lstrlenA(ui->pszUID)+2, "%s ", ui->pszUID);

							SendMessageA(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_REPLACESEL, FALSE, (LPARAM) pszName);
							PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0);
							free(pszName);

						}
						else
							DoEventHookAsync(hwndDlg, si->pszID, si->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, (LPARAM)NULL);
					}

					return TRUE;


				}
				else if(HIWORD(wParam) == LBN_KILLFOCUS)
				{
					RedrawWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NULL, NULL, RDW_INVALIDATE);
					//InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NULL, FALSE);
				}

			}break;
			case IDOK:
				{
                char *pszText = NULL;
				char * p1 = NULL;
				if(!IsWindowEnabled(GetDlgItem(hwndDlg,IDOK)))
					break;

				pszText = Message_GetFromStream(hwndDlg, si);
				SM_AddCommand(si->pszID, si->pszModule, pszText);
				DoRtfToTags(pszText, si); // TRUE if success
				p1 = strchr(pszText, '\0');

				//remove trailing linebreaks
				while(p1 > pszText && (*p1 == '\0' || *p1 == '\r' || *p1 == '\n'))
				{
					*p1 = '\0';
					p1--;
				}

				if(MM_FindModule(si->pszModule)->bAckMsg)
				{
					EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_MESSAGE),FALSE);
					SendDlgItemMessage(hwndDlg,IDC_CHAT_MESSAGE,EM_SETREADONLY,TRUE,0);
				}
				else
					SendDlgItemMessage(hwndDlg,IDC_CHAT_MESSAGE,WM_SETTEXT,0,(LPARAM)"");

				EnableWindow(GetDlgItem(hwndDlg,IDOK),FALSE);

				DoEventHookAsync(hwndDlg, si->pszID, si->pszModule, GC_USER_MESSAGE, NULL, pszText, (LPARAM)NULL);
				free(pszText);
				SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));

				}break;
			case IDC_CHAT_SHOWNICKLIST:
				{
					if(!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_SHOWNICKLIST)))
						break;
					if(si->iType == GCW_SERVER)
						break;

					si->bNicklistEnabled = !si->bNicklistEnabled;

					SendDlgItemMessage(hwndDlg,IDC_CHAT_SHOWNICKLIST,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(si->bNicklistEnabled?IDI_NICKLIST:IDI_NICKLIST2, si->bNicklistEnabled?"nicklist":"nicklist2", 0, 0 ));
					SendMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
					SendMessage(hwndDlg, WM_SIZE, 0, 0);
				}break;
			case IDC_CHAT_MESSAGE:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDOK), GetRichTextLength(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE)) != 0);
				}break;
			case IDC_CHAT_SMILEY:
				{
					SMADD_SHOWSEL smaddInfo;
					RECT rc;

					GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_SMILEY), &rc);

					smaddInfo.cbSize = sizeof(SMADD_SHOWSEL);
					smaddInfo.hwndTarget = GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE);
					smaddInfo.targetMessage = EM_REPLACESEL;
					smaddInfo.targetWParam = TRUE;
					smaddInfo.Protocolname = si->pszModule;
					smaddInfo.Direction = 3;
					smaddInfo.xPosition = rc.left+3;
					smaddInfo.yPosition = rc.top-1;

					if(SmileyAddInstalled)
						CallService(MS_SMILEYADD_SHOWSELECTION, 0, (LPARAM) &smaddInfo);

				}break;
			case IDC_CHAT_HISTORY:
				{
					char szFile[MAX_PATH];
					char szName[MAX_PATH];
					char szFolder[MAX_PATH];
					MODULEINFO * pInfo = MM_FindModule(si->pszModule);

					if(!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_HISTORY)))
						break;

					if (pInfo)
					{

						mir_snprintf(szName, MAX_PATH,"%s",pInfo->pszModDispName?pInfo->pszModDispName:si->pszModule);
						ValidateFilename(szName);
						mir_snprintf(szFolder, MAX_PATH,"%s\\%s", g_Settings.pszLogDir, szName );

						mir_snprintf(szName, MAX_PATH,"%s.log",si->pszID);
						ValidateFilename(szName);

						mir_snprintf(szFile, MAX_PATH,"%s\\%s", szFolder, szName );

						ShellExecuteA(hwndDlg, "open", szFile, NULL, NULL, SW_SHOW);
					}

				}break;
			case IDC_CHAT_CLOSE:
				{
					SendMessage(hwndDlg, GC_REMOVETAB, 0, 0);
				}break;
			case IDC_CHAT_CHANMGR:
				{
				if(!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_CHANMGR)))
					break;
				DoEventHookAsync(hwndDlg, si->pszID, si->pszModule, GC_USER_CHANMGR, NULL, NULL, (LPARAM)NULL);
				}break;
			case IDC_CHAT_FILTER:
				{
				if(!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_FILTER)))
					break;
				si->bFilterEnabled = !si->bFilterEnabled;
				SendDlgItemMessage(hwndDlg,IDC_CHAT_FILTER,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(si->bFilterEnabled?IDI_FILTER:IDI_FILTER2, si->bFilterEnabled?"filter":"filter2", 0, 0 ));
				if (si->bFilterEnabled && DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) == 0)
				{
					SendMessage(hwndDlg, GC_SHOWFILTERMENU, 0, 0);
					break;
				}
				SendMessage(hwndDlg, GC_REDRAWLOG, 0, 0);
				}break;
			case IDC_CHAT_BKGCOLOR:
				{
					CHARFORMAT2 cf;

					cf.cbSize = sizeof(CHARFORMAT2);
					cf.dwEffects = 0;

					if(!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_BKGCOLOR)))
						break;
					if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_BKGCOLOR) )
					{
						if(DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) == 0)
						{
							SendMessage(hwndDlg, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_BKGCOLOR);
						}
						else if (si->bBGSet)
						{
							cf.dwMask = CFM_BACKCOLOR;
							cf.crBackColor = MM_FindModule(si->pszModule)->crColors[si->iBG];
							SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
						}

					}
					else
					{
						cf.dwMask = CFM_BACKCOLOR;
						cf.crBackColor = (COLORREF)DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW));
						SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

					}
				}break;
			case IDC_CHAT_COLOR:
				{
					CHARFORMAT2 cf;

					cf.cbSize = sizeof(CHARFORMAT2);
					cf.dwEffects = 0;

					if(!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_COLOR)))
						break;
					if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_COLOR) )
					{
						if(DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) == 0)
						{
							SendMessage(hwndDlg, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_COLOR);
						}
						else if (si->bFGSet)
						{
							cf.dwMask = CFM_COLOR;
							cf.crTextColor = MM_FindModule(si->pszModule)->crColors[si->iFG];
							SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
						}

					}
					else
					{
						COLORREF cr;

						LoadMsgDlgFont(17, NULL, &cr);
						cf.dwMask = CFM_COLOR;
						cf.crTextColor = cr;
						SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

					}
				}break;
			case IDC_CHAT_BOLD:
			case IDC_CHAT_ITALICS:
			case IDC_CHAT_UNDERLINE:
				{
					CHARFORMAT2 cf;
					cf.cbSize = sizeof(CHARFORMAT2);
					cf.dwMask = CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE;
					cf.dwEffects = 0;

					if(LOWORD(wParam) == IDC_CHAT_BOLD && !IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_BOLD)))
						break;
					if(LOWORD(wParam) == IDC_CHAT_ITALICS && !IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_ITALICS)))
						break;
					if(LOWORD(wParam) == IDC_CHAT_UNDERLINE && !IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_UNDERLINE)))
						break;
					if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_BOLD))
						cf.dwEffects |= CFE_BOLD;
					if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_ITALICS))
						cf.dwEffects |= CFE_ITALIC;
					if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_UNDERLINE))
						cf.dwEffects |= CFE_UNDERLINE;

					SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

				}break;

			default:break;
			}
		} break;





		case WM_KEYDOWN:
			SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));





		case WM_MOVE:
			{
				SendMessage(hwndDlg,GC_SAVEWNDPOS,0,1);
			}break;


		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = si->iSplitterX + 43;
			if(mmi->ptMinTrackSize.x < 350)
				mmi->ptMinTrackSize.x = 350;

			mmi->ptMinTrackSize.y = si->iSplitterY + 80;


		} break;


		case WM_LBUTTONDBLCLK:
		{
			if(LOWORD(lParam) < 30)
				PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
		} break;


		case WM_CLOSE:
		{
			if(g_Settings.TabsEnable && g_Settings.TabRestore && lParam != 1)
			{
				SESSION_INFO * s;
				TCITEM id = {0};
				int j = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_CHAT_TAB)) - 1;
				id.mask = TCIF_PARAM;
				for(; j >= 0; j--)
				{
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_CHAT_TAB), j, &id);
					s = (SESSION_INFO *)id.lParam;
					if(s)
						TabM_AddTab(s->pszID, s->pszModule);
				}
			}
			SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
		} break;




		case GC_CLOSEWINDOW:
		{

			if(g_Settings.TabsEnable)
				SM_SetTabbedWindowHwnd(0, 0);
			DestroyWindow(hwndDlg);
		} break;




		case WM_DESTROY:
		{
			SendMessage(hwndDlg,GC_SAVEWNDPOS,0,0);

			si->hWnd = NULL;

			SetWindowLong(hwndDlg,GWL_USERDATA,0);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERX),GWL_WNDPROC,(LONG)OldSplitterProc);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERY),GWL_WNDPROC,(LONG)OldSplitterProc);
 			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_LIST),GWL_WNDPROC,(LONG)OldNicklistProc);
			UnsubclassTabCtrl(GetDlgItem(hwndDlg,IDC_CHAT_TAB));
          SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_UNSUBCLASSED, 0, 0);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_MESSAGE),GWL_WNDPROC,(LONG)OldMessageProc);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_LOG),GWL_WNDPROC,(LONG)OldLogProc);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_FILTER),GWL_WNDPROC,(LONG)OldFilterButtonProc);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_COLOR),GWL_WNDPROC,(LONG)OldFilterButtonProc);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_BKGCOLOR),GWL_WNDPROC,(LONG)OldFilterButtonProc);
		}break;

		default:break;
	}
	return(FALSE);
}


