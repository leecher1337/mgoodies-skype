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
#include "../utils.h"

#define __try
#define __except(x) if (0)
#define __finally

#define _try __try
#define _except __except
#define _finally __finally

extern HBRUSH      hEditBkgBrush;
extern HBRUSH      hListBkgBrush;
extern HANDLE      hSendEvent;
extern HINSTANCE   g_hInst;
extern HICON      hIcons[30];
extern struct      CREOleCallback reOleCallback;
extern HMENU      g_hMenu;
extern BOOL         SmileyAddInstalled;
extern TABLIST *   g_TabList;
extern int eventMessageIcon;
extern HANDLE hHookWinPopup;

static WNDPROC OldSplitterProc;
static WNDPROC OldMessageProc;
static WNDPROC OldNicklistProc;
static WNDPROC OldFilterButtonProc;
static WNDPROC OldLogProc;


typedef struct
{
   time_t lastEnterTime;
   TCHAR  szTabSave[20];
} MESSAGESUBDATA;


static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
   switch(msg) {
   case WM_NCHITTEST:
      return HTCLIENT;

   case WM_SETCURSOR:
   {   RECT rc;
      GetClientRect(hwnd,&rc);
      SetCursor(rc.right>rc.bottom?LoadCursor(NULL, IDC_SIZENS):LoadCursor(NULL, IDC_SIZEWE));
      return TRUE;
   }
   case WM_LBUTTONDOWN:
      SetCapture(hwnd);
      return 0;

   case WM_MOUSEMOVE:
      if (GetCapture()==hwnd) {
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

static void   InitButtons(HWND hwndDlg, SESSION_INFO* si)
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
//   SendDlgItemMessage(hwndDlg, IDC_CHAT_SHOWNICKLIST, BUTTONSETASPUSHBTN, 0, 0);
//   SendDlgItemMessage(hwndDlg, IDC_CHAT_FILTER, BUTTONSETASPUSHBTN, 0, 0);

   if (pInfo)
   {
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_BOLD), pInfo->bBold);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_ITALICS), pInfo->bItalics);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_UNDERLINE), pInfo->bUnderline);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_COLOR), pInfo->bColor);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_BKGCOLOR), pInfo->bBkgColor);
      if (si->iType == GCW_CHATROOM)
         EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR), pInfo->bChanMgr);
   }
}






static int RoomWndResize(HWND hwndDlg,LPARAM lParam,UTILRESIZECONTROL *urc)
{
   SESSION_INFO* si = (SESSION_INFO*)lParam;
   BOOL      bControl = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowTopButtons", 1);
   BOOL      bFormat = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowFormatButtons", 1);
   BOOL      bToolbar = bFormat || bControl;
   BOOL      bSend = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowSend", 0);
   BOOL      bNick = si->iType!=GCW_SERVER && si->bNicklistEnabled;

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
   ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_CLOSE), SW_HIDE);
   if (si->iType != GCW_SERVER)
      ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), si->bNicklistEnabled?SW_SHOW:SW_HIDE);
   else
      ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), SW_HIDE);

   if (si->iType == GCW_SERVER) {
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_SHOWNICKLIST), FALSE);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), FALSE);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR), FALSE);
   }
   else {
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_SHOWNICKLIST), TRUE);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), TRUE);
      if (si->iType == GCW_CHATROOM)
         EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR), MM_FindModule(si->pszModule)->bChanMgr);

   }

   switch(urc->wId) {
   case IDOK:
      urc->rcItem.left = bSend?315:urc->dlgNewSize.cx ;
      urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY+23;
      urc->rcItem.bottom = urc->dlgNewSize.cy -1;
      return RD_ANCHORX_RIGHT|RD_ANCHORY_CUSTOM;

   case IDC_CHAT_LOG:
      urc->rcItem.top = 0;
      urc->rcItem.left = 0;
      urc->rcItem.right = bNick?urc->dlgNewSize.cx - si->iSplitterX:urc->dlgNewSize.cx;
      urc->rcItem.bottom = bToolbar?(urc->dlgNewSize.cy - si->iSplitterY):(urc->dlgNewSize.cy - si->iSplitterY+20);
      return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;

   case IDC_CHAT_LIST:
      urc->rcItem.top = 0;
      urc->rcItem.right = urc->dlgNewSize.cx ;
      urc->rcItem.left = urc->dlgNewSize.cx - si->iSplitterX + 2;
      urc->rcItem.bottom = bToolbar?(urc->dlgNewSize.cy - si->iSplitterY):(urc->dlgNewSize.cy - si->iSplitterY+20);
      return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;

   case IDC_CHAT_SPLITTERX:
      urc->rcItem.right = urc->dlgNewSize.cx - si->iSplitterX+2;
      urc->rcItem.left = urc->dlgNewSize.cx - si->iSplitterX;
      urc->rcItem.bottom = bToolbar?(urc->dlgNewSize.cy - si->iSplitterY):(urc->dlgNewSize.cy - si->iSplitterY+20);
      urc->rcItem.top = 1;
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
      urc->rcItem.bottom = 19;
      urc->rcItem.top = 3;
      return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;
   }
   return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}

static LRESULT CALLBACK MessageSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   MESSAGESUBDATA *dat;
   SESSION_INFO* Parentsi;

   Parentsi=(SESSION_INFO*)GetWindowLong(GetParent(hwnd),GWL_USERDATA);
   dat = (MESSAGESUBDATA *) GetWindowLong(hwnd, GWL_USERDATA);
   switch (msg) {
   case EM_SUBCLASSED:
      dat = (MESSAGESUBDATA *) mir_alloc(sizeof(MESSAGESUBDATA));

      SetWindowLong(hwnd, GWL_USERDATA, (LONG) dat);
      dat->szTabSave[0] = '\0';
      dat->lastEnterTime = 0;
      return 0;

   case WM_MOUSEWHEEL:
	  if ((GetWindowLong(hwnd, GWL_STYLE) & WS_VSCROLL) == 0) {
		SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_LOG), WM_MOUSEWHEEL, wParam, lParam);
	  }
      dat->lastEnterTime = 0;
      return TRUE;

   case EM_REPLACESEL:
      PostMessage(hwnd, EM_ACTIVATE, 0, 0);
      break;

   case EM_ACTIVATE:
      SetActiveWindow(GetParent(hwnd));
      break;

   case WM_CHAR:
      {
         BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
         BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
         BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;

         if (GetWindowLong(hwnd, GWL_STYLE) & ES_READONLY)
            break;

         if (wParam == 9 && isCtrl && !isAlt) // ctrl-i (italics)
            return TRUE;

         if (wParam == VK_SPACE && isCtrl && !isAlt) // ctrl-space (paste clean text)
            return TRUE;

		if (wParam == 19 && isCtrl && isShift) {     // ctrl-shift-s
			SendMessage(GetParent(GetParent(hwnd)), DM_SWITCHSTATUSBAR, 0, 0);
			return 0;
		}
		if (wParam == 13 && isCtrl && isShift) {     // ctrl-shift-m
			SendMessage(GetParent(GetParent(hwnd)), DM_SWITCHTITLEBAR, 0, 0);
			return 0;
		}
         if (wParam == 1 && isCtrl && !isAlt) {      //ctrl-a
            SendMessage(hwnd, EM_SETSEL, 0, -1);
            return 0;
      }   }
      break;

	case WM_KEYDOWN:
	{
		static int start, end;
        BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
        BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
        BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;


        if (wParam == VK_RETURN) {
            dat->szTabSave[0] = '\0';
			if (( isCtrl != 0 ) ^ (0 != DBGetContactSettingByte(NULL, "Chat", "SendOnEnter", 1))) {
			   PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			   return 0;
			}
			if (DBGetContactSettingByte(NULL, "Chat", "SendOnDblEnter", 0)) {
			   if (dat->lastEnterTime + 2 < time(NULL))
				  dat->lastEnterTime = time(NULL);
			   else {
				  SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
				  SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
				  PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
				  return 0;
		}   }	}

        if (wParam == VK_TAB && isShift && !isCtrl) { // SHIFT-TAB (go to nick list)
           SetFocus(GetDlgItem(GetParent(hwnd), IDC_CHAT_LIST));
           return TRUE;
        }

        if (wParam == VK_TAB && isCtrl && !isShift) { // CTRL-TAB (switch tab/window)
           SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATENEXT, 0, (LPARAM)GetParent(hwnd));
           return TRUE;
        }

        if (wParam == VK_TAB && isCtrl && isShift) { // CTRL_SHIFT-TAB (switch tab/window)
           SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEPREV, 0, (LPARAM)GetParent(hwnd));
           return TRUE;
        }

		if (isCtrl && !isAlt) {
			if (wParam == VK_PRIOR) { // page up
				SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEPREV, 0, (LPARAM)GetParent(hwnd));
				return 0;
			}
			if (wParam == VK_NEXT) { // page down
				SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATENEXT, 0, (LPARAM)GetParent(hwnd));
				return 0;
			}
		}

         if (wParam == VK_TAB && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000)) {    //tab-autocomplete
            TCHAR* pszText = NULL;
            int iLen;
            GETTEXTLENGTHEX gtl = {0};
            GETTEXTEX gt = {0};
            LRESULT lResult = (LRESULT)SendMessage(hwnd, EM_GETSEL, (WPARAM)NULL, (LPARAM)NULL);

            SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
            start = LOWORD(lResult);
            end = HIWORD(lResult);
            SendMessage(hwnd, EM_SETSEL, end, end);
            gtl.flags = GTL_PRECISE;
            gtl.codepage = CP_ACP;
            iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, (LPARAM)NULL);
            if (iLen >0) {
               TCHAR *pszName = NULL;
               TCHAR *pszSelName = NULL;
               pszText = mir_alloc(sizeof(TCHAR)*(iLen+100));

               gt.cb = iLen+99;
               gt.flags = GT_DEFAULT;
               #if defined( _UNICODE )
                  gt.codepage = 1200;
               #else
                  gt.codepage = CP_ACP;
               #endif

               SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);
               while ( start >0 && pszText[start-1] != ' ' && pszText[start-1] != 13 && pszText[start-1] != VK_TAB)
                  start--;
               while (end < iLen && pszText[end] != ' ' && pszText[end] != 13 && pszText[end-1] != VK_TAB)
                  end ++;

               if ( dat->szTabSave[0] =='\0')
                  lstrcpyn( dat->szTabSave, pszText+start, end-start+1 );

               pszSelName = mir_alloc( sizeof(TCHAR)*( end-start+1 ));
               lstrcpyn( pszSelName, pszText+start, end-start+1);
               pszName = UM_FindUserAutoComplete(Parentsi->pUsers, dat->szTabSave, pszSelName);
               if (pszName == NULL) {
                  pszName = dat->szTabSave;
                  SendMessage(hwnd, EM_SETSEL, start, end);
                  if (end !=start)
                     SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM) pszName);
                  dat->szTabSave[0] = '\0';
               }
               else {
                  SendMessage(hwnd, EM_SETSEL, start, end);
                  if (end !=start)
                     SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM) pszName);
               }
               mir_free(pszText);
               mir_free(pszSelName);
            }

            SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
            return 0;
         }

         if (dat->szTabSave[0] != '\0' && wParam != VK_RIGHT && wParam != VK_LEFT
               && wParam != VK_SPACE && wParam != VK_RETURN && wParam != VK_BACK
               && wParam != VK_DELETE ) {
            if (g_Settings.AddColonToAutoComplete && start == 0)
               SendMessageA(hwnd, EM_REPLACESEL, FALSE, (LPARAM) ": ");

            dat->szTabSave[0] = '\0';
         }

		if(wParam == VK_ESCAPE && isShift) {
			ShowWindow(GetParent(GetParent(hwnd)), SW_MINIMIZE);
			return 0;
		}

         if (wParam == VK_F4 && isCtrl && !isAlt) { // ctrl-F4 (close tab)
            SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_CLOSE, BN_CLICKED), 0);
            return TRUE;
         }

         if (wParam == 0x49 && isCtrl && !isAlt) { // ctrl-i (italics)
            CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_ITALICS) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
            SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_ITALICS, 0), 0);
            return TRUE;
         }

         if (wParam == 0x42 && isCtrl && !isAlt) { // ctrl-b (bold)
            CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BOLD) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
            SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_BOLD, 0), 0);
            return TRUE;
         }

         if (wParam == 0x55 && isCtrl && !isAlt) { // ctrl-u (paste clean text)
            CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_UNDERLINE) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
            SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_UNDERLINE, 0), 0);
            return TRUE;
         }

         if (wParam == 0x4b && GetKeyState(VK_CONTROL) & 0x8000 && !(GetKeyState(VK_MENU) & 0x8000)) { // ctrl-k (paste clean text)
            CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_COLOR) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
            SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_COLOR, 0), 0);
            return TRUE;
         }

         if (wParam == VK_SPACE && isCtrl && !isAlt) { // ctrl-space (paste clean text)
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

         if (wParam == 0x4c && isCtrl && !isAlt) { // ctrl-l (paste clean text)
            CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BKGCOLOR) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
            SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_BKGCOLOR, 0), 0);
            return TRUE;
         }

         if (wParam == 0x46 && isCtrl && !isAlt) { // ctrl-f (paste clean text)
            if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHAT_FILTER)))
               SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_FILTER, 0), 0);
            return TRUE;
         }

         if (wParam == 0x4e && isCtrl && !isAlt) { // ctrl-n (nicklist)
            if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHAT_SHOWNICKLIST)))
               SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_SHOWNICKLIST, 0), 0);
            return TRUE;
         }

         if (wParam == 0x48 && isCtrl && !isAlt) { // ctrl-h (history)
            SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_HISTORY, 0), 0);
            return TRUE;
         }

         if (wParam == 0x4f && isCtrl && !isAlt) { // ctrl-o (options)
            if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHAT_CHANMGR)))
               SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_CHANMGR, 0), 0);
            return TRUE;
         }

         if (((wParam == 45 && isShift) || (wParam == 0x56 && isCtrl)) && !isAlt) { // ctrl-v (paste clean text)
            SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
            return TRUE;
         }

         if (wParam == 0x57 && isCtrl && !isAlt) { // ctrl-w (close window)
            PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
            return TRUE;
         }

         if (wParam == VK_NEXT || wParam == VK_PRIOR) {
            HWND htemp = GetParent(hwnd);
            SendDlgItemMessage(htemp, IDC_CHAT_LOG, msg, wParam, lParam);
            dat->lastEnterTime = 0;
            return TRUE;
         }

         if (wParam == VK_UP && isCtrl && !isAlt) {
            int iLen;
            GETTEXTLENGTHEX gtl = {0};
            SETTEXTEX ste;
            LOGFONT lf;
            char* lpPrevCmd = SM_GetPrevCommand(Parentsi->ptszID, Parentsi->pszModule);

            SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

            Chat_LoadMsgDlgFont(17, &lf, NULL);
            ste.flags = ST_DEFAULT;
            ste.codepage = CP_ACP;
            if (lpPrevCmd)
               SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpPrevCmd);
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

         if (wParam == VK_DOWN && isCtrl && !isAlt) {
            int iLen;
            GETTEXTLENGTHEX gtl = {0};
            SETTEXTEX ste;

            char* lpPrevCmd = SM_GetNextCommand(Parentsi->ptszID, Parentsi->pszModule);
            SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

            ste.flags = ST_DEFAULT;
            ste.codepage = CP_ACP;
            if (lpPrevCmd)
               SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM) lpPrevCmd);
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
      }
      //fall through

   case WM_LBUTTONDOWN:
   case WM_MBUTTONDOWN:
   case WM_KILLFOCUS:
      dat->lastEnterTime = 0;
      break;

	case WM_SYSKEYDOWN:
		if ((wParam == VK_LEFT) && GetKeyState(VK_MENU) & 0x8000) {
			SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEPREV, 0, (LPARAM)GetParent(hwnd));
			return TRUE;
		}
		if ((wParam == VK_RIGHT) && GetKeyState(VK_MENU) & 0x8000) {
			SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATENEXT, 0, (LPARAM)GetParent(hwnd));
			return TRUE;
		}
		break;
	case WM_SYSKEYUP:
		if ((wParam == VK_LEFT) && GetKeyState(VK_MENU) & 0x8000) {
			return TRUE;
		}
		if ((wParam == VK_RIGHT) && GetKeyState(VK_MENU) & 0x8000) {
			return TRUE;
		}
		break;

   case WM_CONTEXTMENU:
      {
         CHARRANGE sel, all = { 0, -1 };
         POINT pt;
         UINT uID = 0;
         HMENU hMenu, hSubMenu;
		MessageWindowPopupData mwpd;

		 hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
         hSubMenu = GetSubMenu(hMenu, 4);
         CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
         SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) & sel);

         EnableMenuItem(hSubMenu, ID_MESSAGE_UNDO, SendMessage(hwnd, EM_CANUNDO, 0,0)?MF_ENABLED:MF_GRAYED);
         EnableMenuItem(hSubMenu, ID_MESSAGE_REDO, SendMessage(hwnd, EM_CANREDO, 0,0)?MF_ENABLED:MF_GRAYED);
         EnableMenuItem(hSubMenu, ID_MESSAGE_COPY, sel.cpMax!=sel.cpMin?MF_ENABLED:MF_GRAYED);
         EnableMenuItem(hSubMenu, ID_MESSAGE_CUT, sel.cpMax!=sel.cpMin?MF_ENABLED:MF_GRAYED);

         dat->lastEnterTime = 0;

		if (lParam == 0xFFFFFFFF) {
			SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM) & pt, (LPARAM) sel.cpMax);
			ClientToScreen(hwnd, &pt);
		}
		else {
			pt.x = (short) LOWORD(lParam);
			pt.y = (short) HIWORD(lParam);
		}
		// First notification
		mwpd.cbSize = sizeof(mwpd);
		mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
		mwpd.uFlags = MSG_WINDOWPOPUP_INPUT;
		mwpd.hContact = Parentsi->hContact;
		mwpd.hwnd = hwnd;
		mwpd.hMenu = hSubMenu;
		mwpd.selection = 0;
		mwpd.pt = pt;
		NotifyEventHooks(hHookWinPopup, 0, (LPARAM)&mwpd);

         uID = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, GetParent(hwnd), NULL);

		// Second notification
		mwpd.selection = uID;
		mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
		NotifyEventHooks(hHookWinPopup, 0, (LPARAM)&mwpd);

		switch (uID) {
         case 0:
            break;

         case ID_MESSAGE_UNDO:
            SendMessage(hwnd, EM_UNDO, 0, 0);
            break;

         case ID_MESSAGE_REDO:
            SendMessage(hwnd, EM_REDO, 0, 0);
            break;

         case ID_MESSAGE_COPY:
            SendMessage(hwnd, WM_COPY, 0, 0);
            break;

         case ID_MESSAGE_CUT:
            SendMessage(hwnd, WM_CUT, 0, 0);
            break;

         case ID_MESSAGE_PASTE:
            SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
            break;

         case ID_MESSAGE_SELECTALL:
            SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & all);
            break;

         case ID_MESSAGE_CLEAR:
            SetWindowText(hwnd, _T( "" ));
            break;
         }
		DestroyMenu(hMenu);
         PostMessage(hwnd, WM_KEYUP, 0, 0 );
      }
		return TRUE;

   case WM_KEYUP:
   case WM_LBUTTONUP:
   case WM_RBUTTONUP:
   case WM_MBUTTONUP:
      {
         CHARFORMAT2 cf;
         UINT u = 0;
         UINT u2 = 0;
         COLORREF cr;

         Chat_LoadMsgDlgFont(17, NULL, &cr);

         cf.cbSize = sizeof(CHARFORMAT2);
         cf.dwMask = CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE|CFM_BACKCOLOR|CFM_COLOR;
         SendMessage(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

         if (MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bColor) {
            int index = GetColorIndex(Parentsi->pszModule, cf.crTextColor);
            u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_COLOR);

            if (index >= 0) {
               Parentsi->bFGSet = TRUE;
               Parentsi->iFG = index;
            }

            if (u == BST_UNCHECKED && cf.crTextColor != cr)
               CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, BST_CHECKED);
            else if (u == BST_CHECKED && cf.crTextColor == cr)
               CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, BST_UNCHECKED);
         }

         if (MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bBkgColor) {
            int index = GetColorIndex(Parentsi->pszModule, cf.crBackColor);
            COLORREF crB = (COLORREF)DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW));
            u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BKGCOLOR);

            if (index >= 0) {
               Parentsi->bBGSet = TRUE;
               Parentsi->iBG = index;
            }
            if (u == BST_UNCHECKED && cf.crBackColor != crB)
               CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, BST_CHECKED);
            else if (u == BST_CHECKED && cf.crBackColor == crB)
               CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, BST_UNCHECKED);
         }

         if (MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bBold) {
            u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BOLD);
            u2 = cf.dwEffects;
            u2 &= CFE_BOLD;
            if (u == BST_UNCHECKED && u2)
               CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, BST_CHECKED);
            else if (u == BST_CHECKED && u2 == 0)
               CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, BST_UNCHECKED);
         }

         if (MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bItalics) {
            u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_ITALICS);
            u2 = cf.dwEffects;
            u2 &= CFE_ITALIC;
            if (u == BST_UNCHECKED && u2)
               CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, BST_CHECKED);
            else if (u == BST_CHECKED && u2 == 0)
               CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, BST_UNCHECKED);
         }

         if (MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bUnderline) {
            u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_UNDERLINE);
            u2 = cf.dwEffects;
            u2 &= CFE_UNDERLINE;
            if (u == BST_UNCHECKED && u2)
               CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, BST_CHECKED);
            else if (u == BST_CHECKED && u2 == 0)
               CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, BST_UNCHECKED);
      }   }
      break;

   case EM_UNSUBCLASSED:
      mir_free(dat);
      return 0;
   }

   return CallWindowProc(OldMessageProc, hwnd, msg, wParam, lParam);
}

static BOOL CALLBACK FilterWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
   static SESSION_INFO* si = NULL;
   switch (uMsg) {
   case WM_INITDIALOG:
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
      break;

   case WM_CTLCOLOREDIT:
   case WM_CTLCOLORSTATIC:
      SetTextColor((HDC)wParam,RGB(60,60,150));
      SetBkColor((HDC)wParam,GetSysColor(COLOR_WINDOW));
      return (BOOL)GetSysColorBrush(COLOR_WINDOW);

   case WM_ACTIVATE:
      if (LOWORD(wParam) == WA_INACTIVE) {
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

         SendMessage(si->hWnd, GC_CHANGEFILTERFLAG, 0, (LPARAM)iFlags);
         if (si->bFilterEnabled)
            SendMessage(si->hWnd, GC_REDRAWLOG, 0, 0);
         PostMessage(hwndDlg, WM_CLOSE, 0, 0);
      }
      break;

   case WM_CLOSE:
      DestroyWindow(hwndDlg);
      break;
   }

   return(FALSE);
}

static LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg) {
   case WM_RBUTTONUP:
      {
         HWND hFilter = GetDlgItem(GetParent(hwnd), IDC_CHAT_FILTER);
         HWND hColor = GetDlgItem(GetParent(hwnd), IDC_CHAT_COLOR);
         HWND hBGColor = GetDlgItem(GetParent(hwnd), IDC_CHAT_BKGCOLOR);

         if (DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) != 0) {
            if (hFilter == hwnd)
               SendMessage(GetParent(hwnd), GC_SHOWFILTERMENU, 0, 0);
            if (hColor == hwnd)
               SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_COLOR);
            if (hBGColor == hwnd)
               SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_BKGCOLOR);
      }   }
      break;
   }

   return CallWindowProc(OldFilterButtonProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK LogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg) {
   case WM_LBUTTONUP:
      {
         CHARRANGE sel;

         SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);
         if (sel.cpMin != sel.cpMax)
         {
            SendMessage(hwnd, WM_COPY, 0, 0);
            sel.cpMin = sel.cpMax ;
            SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & sel);
         }
         SetFocus(GetDlgItem(GetParent(hwnd), IDC_CHAT_MESSAGE));
         break;
      }
   case WM_KEYDOWN:
      if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
         PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
         return TRUE;
      }
      break;

   case WM_ACTIVATE:
      if (LOWORD(wParam) == WA_INACTIVE) {
         CHARRANGE sel;
         SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);
         if (sel.cpMin != sel.cpMax) {
            sel.cpMin = sel.cpMax ;
            SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & sel);
      }   }
      break;

   case WM_CHAR:
      SetFocus(GetDlgItem(GetParent(hwnd), IDC_CHAT_MESSAGE));
      SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_MESSAGE), WM_CHAR, wParam, lParam);
      break;
   }

   return CallWindowProc(OldLogProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK NicklistSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg) {
   case WM_ERASEBKGND:
      {
         HDC dc = (HDC)wParam;
         SESSION_INFO* parentdat =(SESSION_INFO*)GetWindowLong(GetParent(hwnd),GWL_USERDATA);
         if (dc) {
            int height, index, items = 0;

            index = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
            if (index == LB_ERR || parentdat->nUsersInNicklist <= 0)
               return 0;

            items = parentdat->nUsersInNicklist - index;
            height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);

            if (height != LB_ERR) {
               RECT rc = {0};
               GetClientRect(hwnd, &rc);

               if (rc.bottom-rc.top > items * height) {
                  rc.top = items*height;
                  FillRect(dc, &rc, hListBkgBrush);
      }   }   }   }
      return 1;

   case WM_KEYDOWN:
      if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
         PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
         return TRUE;
      }
      break;

   case WM_RBUTTONDOWN:
      SendMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
      break;

   case WM_RBUTTONUP:
      SendMessage(hwnd, WM_LBUTTONUP, wParam, lParam);
      break;

   case WM_CONTEXTMENU:
      {
         TVHITTESTINFO hti;
         int item;
         int height=0;
         USERINFO * ui;
         SESSION_INFO* parentdat =(SESSION_INFO*)GetWindowLong(GetParent(hwnd),GWL_USERDATA);


         hti.pt.x = (short) LOWORD(lParam);
         hti.pt.y = (short) HIWORD(lParam);
         if (hti.pt.x == -1 && hti.pt.y == -1) {
            int index = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
            int top = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
            height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);
            hti.pt.x = 4;
            hti.pt.y = (index - top)*height + 1;
         }
         else ScreenToClient(hwnd,&hti.pt);

         item = LOWORD(SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
         ui = SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, item);
         // ui = (USERINFO *)SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_LIST), LB_GETITEMDATA, item, 0);
         if (ui) {
            HMENU hMenu = 0;
            UINT uID;
            USERINFO uinew;

            memcpy(&uinew, ui, sizeof(USERINFO));
            if (hti.pt.x == -1 && hti.pt.y == -1)
               hti.pt.y += height - 4;
            ClientToScreen(hwnd, &hti.pt);
            uID = CreateGCMenu(hwnd, &hMenu, 0, hti.pt, parentdat, uinew.pszUID, NULL);

            switch (uID) {
            case 0:
               break;

            case ID_MESS:
               DoEventHookAsync(GetParent(hwnd), parentdat->ptszID, parentdat->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, (LPARAM)NULL);
               break;

            default:
               DoEventHookAsync(GetParent(hwnd), parentdat->ptszID, parentdat->pszModule, GC_USER_NICKLISTMENU, ui->pszUID, NULL, (LPARAM)uID);
               break;
            }
            DestroyGCMenu(&hMenu, 1);
            return TRUE;
      }   }
      break;

	case WM_MOUSEMOVE:
		{
			SESSION_INFO* parentdat =(SESSION_INFO*)GetWindowLong(GetParent(hwnd),GWL_USERDATA);
			if ( parentdat ) {
				POINT p;
				GetCursorPos(&p);
				SendMessage( parentdat->hwndTooltip,TTM_TRACKPOSITION,0,(LPARAM)MAKELPARAM(p.x + 15,p.y + 15));
//				SendMessage( parentdat->hwndTooltip, TTM_ACTIVATE, TRUE, 0 );
		}	}
		break;
	}

   return CallWindowProc(OldNicklistProc, hwnd, msg, wParam, lParam);
}


int GetTextPixelSize( TCHAR* pszText, HFONT hFont, BOOL bWidth)
{
   HDC hdc;
   HFONT hOldFont;
   RECT rc = {0};
   int i;

   if (!pszText || !hFont)
      return 0;

   hdc = GetDC(NULL);
   hOldFont = SelectObject(hdc, hFont);
   i = DrawText(hdc, pszText , -1, &rc, DT_CALCRECT);
   SelectObject(hdc, hOldFont);
   ReleaseDC(NULL,hdc);
   return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
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
}   }

static unsigned long forkthread (   void (__cdecl *threadcode)(void*),unsigned long stacksize,void *arg)
{
   unsigned long rc;
   struct FORK_ARG fa;

   fa.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
   fa.threadcode=threadcode;
   fa.arg=arg;

   rc=_beginthread(forkthread_r,stacksize,&fa);

   if ((unsigned long)-1L != rc)
      WaitForSingleObject(fa.hEvent,INFINITE);

   CloseHandle(fa.hEvent);
   return rc;
}

static void __cdecl phase2(void * lParam)
{
   SESSION_INFO* si = (SESSION_INFO*) lParam;
   Sleep(30);
   if (si && si->hWnd)
      PostMessage(si->hWnd, GC_REDRAWLOG3, 0, 0);
}

BOOL CALLBACK RoomWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	SESSION_INFO * si;
	si = (SESSION_INFO *)GetWindowLong(hwndDlg,GWL_USERDATA);
	if (!si && uMsg!=WM_INITDIALOG) return FALSE;
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			SESSION_INFO* psi = (SESSION_INFO*)lParam;
			int mask;
			HWND hNickList = GetDlgItem(hwndDlg,IDC_CHAT_LIST);
			NotifyLocalWinEvent(psi->hContact, hwndDlg, MSG_WINDOW_EVT_OPENING);

			TranslateDialogDefault(hwndDlg);
			SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)psi);
			si = psi;
			OldSplitterProc=(WNDPROC)SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERX),GWL_WNDPROC,(LONG)SplitterSubclassProc);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERY),GWL_WNDPROC,(LONG)SplitterSubclassProc);
			OldNicklistProc=(WNDPROC)SetWindowLong(hNickList,GWL_WNDPROC,(LONG)NicklistSubclassProc);
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

			// enable tooltips
			psi->iOldItemID = -1;
			psi->hwndTooltip = CreateWindow(TOOLTIPS_CLASS,NULL,TTS_ALWAYSTIP,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,hNickList,(HMENU)NULL,g_hInst,NULL);
			SetWindowPos(psi->hwndTooltip, HWND_TOPMOST,0, 0, 0, 0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			{
				TOOLINFO ti = {0};
				ti.cbSize = sizeof(TOOLINFO);
				ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRANSPARENT;
				ti.hwnd   = hwndDlg;
				ti.hinst  = g_hInst;
				ti.uId    = (UINT)hNickList;
				ti.lpszText  = LPSTR_TEXTCALLBACK;
//				GetClientRect( hNickList, &ti.rect );
				SendMessage( psi->hwndTooltip, TTM_ADDTOOL, 0, ( LPARAM )&ti );
				SendMessage( psi->hwndTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 20000 );
				SendMessage( psi->hwndTooltip, TTM_SETMAXTIPWIDTH, 0, 300);


				//SendMessage( psi->hwndTooltip, TTM_TRACKACTIVATE, TRUE, ( LPARAM )&ti );
			}

			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_SMILEY), TRUE);

			SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_HIDESELECTION, TRUE, 0);

			SendMessage(hwndDlg, GC_SETWNDPROPS, 0, 0);
			SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
			SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);

			SendMessage(GetParent(hwndDlg), CM_ADDCHILD, (WPARAM) hwndDlg, (LPARAM) psi->hContact);
			PostMessage(hwndDlg, GC_UPDATENICKLIST, 0, 0);
			NotifyLocalWinEvent(psi->hContact, hwndDlg, MSG_WINDOW_EVT_OPEN);
		}
		break;

   case GC_SETWNDPROPS:
      {
         LoadGlobalSettings();
         InitButtons(hwndDlg, si);

         SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
         SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
         SendMessage(hwndDlg, GC_FIXTABICONS, 0, 0);

         SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_SETBKGNDCOLOR , 0, g_Settings.crLogBackground);

         { //messagebox
            COLORREF   crFore;

            CHARFORMAT2 cf;
            Chat_LoadMsgDlgFont(17, NULL, &crFore);
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

            ih = GetTextPixelSize( _T("AQGglo'"), g_Settings.UserListFont,FALSE);
            ih2 = GetTextPixelSize( _T("AQGglo'"), g_Settings.UserListHeadingsFont,FALSE);
            height = DBGetContactSettingByte(NULL, "Chat", "NicklistRowDist", 12);
            font = ih > ih2?ih:ih2;

            SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_SETITEMHEIGHT, 0, (LPARAM)height > font ? height : font);
            InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NULL, TRUE);
         }
         SendMessage(hwndDlg, WM_SIZE, 0, 0);
         SendMessage(hwndDlg, GC_REDRAWLOG2, 0, 0);
      }
      break;

   case DM_UPDATETITLEBAR:
      {
         TitleBarData tbd;
         TCHAR szTemp [100];
         switch(si->iType) {
         case GCW_CHATROOM:
            mir_sntprintf(szTemp, SIZEOF(szTemp),
               (si->nUsersInNicklist ==1) ? TranslateT("%s: Chat Room (%u user)") : TranslateT("%s: Chat Room (%u users)"),
               si->ptszName, si->nUsersInNicklist);
            break;
         case GCW_PRIVMESS:
            mir_sntprintf(szTemp, SIZEOF(szTemp),
               (si->nUsersInNicklist ==1) ? TranslateT("%s: Message Session") : TranslateT("%s: Message Session (%u users)"),
               si->ptszName, si->nUsersInNicklist);
            break;
         case GCW_SERVER:
            mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s: Server"), si->ptszName);
            break;
         }
         tbd.iFlags = TBDF_TEXT | TBDF_ICON;
         tbd.pszText = szTemp;
         tbd.hIcon = LoadIconEx(IDI_CHANMGR, "window", 0, 0);
         SendMessage(GetParent(hwndDlg), CM_UPDATETITLEBAR, (WPARAM) &tbd, (LPARAM) hwndDlg);
         SendMessage(hwndDlg, DM_UPDATETABCONTROL, 0, 0);
      }
      break;

	case DM_UPDATESTATUSBAR:
		{
			StatusIconData sid;
			StatusBarData sbd;
			HICON hIcon;
			MODULEINFO* mi = MM_FindModule(si->pszModule);
			TCHAR* ptszDispName = a2tf((TCHAR*)mi->pszModDispName, 0);
			TCHAR szTemp[512];
			hIcon = si->wStatus==ID_STATUS_ONLINE ? mi->hOnlineIcon : mi->hOfflineIcon;
			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s : %s"), ptszDispName, si->ptszStatusbarText ? si->ptszStatusbarText : _T(""));
			sbd.iItem = 0;
			sbd.iFlags = SBDF_TEXT | SBDF_ICON;
			sbd.hIcon = hIcon;
			sbd.pszText = szTemp;
			SendMessage(GetParent(hwndDlg), CM_UPDATESTATUSBAR, (WPARAM) &sbd, (LPARAM) hwndDlg);
			sbd.iItem = 1;
			sbd.hIcon = NULL;
			sbd.pszText   = _T("");
			SendMessage(GetParent(hwndDlg), CM_UPDATESTATUSBAR, (WPARAM) &sbd, (LPARAM) hwndDlg);
			mir_free( ptszDispName );
			sid.cbSize = sizeof(sid);
			sid.szModule = SRMMMOD;
			sid.dwId = 0;
   #if defined( _UNICODE )
			sid.flags = 0;
   #else
			sid.flags = MBF_DISABLED;
   #endif
			CallService(MS_MSG_MODIFYICON, (WPARAM)si->hContact, (LPARAM) &sid);
      //   SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)si);
		}
		break;
	case DM_GETCODEPAGE:
		SetWindowLong(hwndDlg, DWL_MSGRESULT, si->codePage);
		return TRUE;
	case DM_SETCODEPAGE:
		si->codePage = (int) lParam;
		si->pszHeader = Log_CreateRtfHeader(MM_FindModule(si->pszModule), si);
        SendMessage(hwndDlg, GC_REDRAWLOG2, 0, 0);
		break;

   case WM_SIZE:
      {
         UTILRESIZEDIALOG urd;

         if (wParam == SIZE_MAXIMIZED)
            PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);

         if (IsIconic(hwndDlg)) break;
         ZeroMemory(&urd,sizeof(urd));
         urd.cbSize=sizeof(urd);
         urd.hInstance=g_hInst;
         urd.hwndDlg=hwndDlg;
         urd.lParam=(LPARAM)si;
         urd.lpTemplate=MAKEINTRESOURCEA(IDD_CHANNEL);
         urd.pfnResizer=RoomWndResize;
         CallService(MS_UTILS_RESIZEDIALOG,0,(LPARAM)&urd);

         RedrawWindow(GetDlgItem(hwndDlg,IDC_CHAT_LOG), NULL, NULL, RDW_INVALIDATE);
         RedrawWindow(GetDlgItem(hwndDlg,IDC_CHAT_LIST), NULL, NULL, RDW_INVALIDATE);
         RedrawWindow(GetDlgItem(hwndDlg,IDC_CHAT_MESSAGE), NULL, NULL, RDW_INVALIDATE);
         RedrawWindow(GetDlgItem(hwndDlg,IDOK), NULL, NULL, RDW_INVALIDATE);
      }
      break;

   case GC_REDRAWWINDOW:
      InvalidateRect(hwndDlg, NULL, TRUE);
      break;

   case GC_REDRAWLOG:
      si->LastTime = 0;
      if (si->pLog) {
         LOGINFO * pLog = si->pLog;
         if (si->iEventCount > 60) {
            int index = 0;
            while ( index < 59) {
               if (pLog->next == NULL)
                  break;

               pLog = pLog->next;
               if (si->iType != GCW_CHATROOM || !si->bFilterEnabled || (si->iLogFilterFlags&pLog->iType) != 0)
                  index++;
            }
            Log_StreamInEvent(hwndDlg, pLog, si, TRUE, FALSE);
            forkthread(phase2, 0, (void *)si);
         }
         else Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE, FALSE);
      }
      else SendMessage(hwndDlg, GC_EVENT_CONTROL + WM_USER+500, WINDOW_CLEARLOG, 0);
      break;

   case GC_REDRAWLOG2:
      si->LastTime = 0;
      if (si->pLog)
         Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE, FALSE);
      break;

   case GC_REDRAWLOG3:
      si->LastTime = 0;
      if (si->pLog)
         Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE, TRUE);
      break;

   case GC_ADDLOG:
      if (si->pLogEnd)
         Log_StreamInEvent(hwndDlg, si->pLog, si, FALSE, FALSE);
      else
         SendMessage(hwndDlg, GC_EVENT_CONTROL + WM_USER+500, WINDOW_CLEARLOG, 0);
      break;


   case DM_UPDATETABCONTROL:
      {
         TabControlData tcd;
         tcd.iFlags = TCDF_TEXT;
         tcd.pszText = si->ptszName;
         SendMessage(GetParent(hwndDlg), CM_UPDATETABCONTROL, (WPARAM) &tcd, (LPARAM) hwndDlg);

      }
   case GC_FIXTABICONS:
      {
         TabControlData tcd;

         int image = eventMessageIcon;
         if (!(si->wState&GC_EVENT_HIGHLIGHT))
         {
            image = si->wStatus==ID_STATUS_ONLINE?MM_FindModule(si->pszModule)->OnlineIconIndex:MM_FindModule(si->pszModule)->OfflineIconIndex;
            if (si->wState&STATE_TALK)
               image++;
         }
         tcd.iFlags = TCDF_ICON;
         tcd.iconIdx = image;
         SendMessage(GetParent(hwndDlg), CM_UPDATETABCONTROL, (WPARAM) &tcd, (LPARAM) hwndDlg);
      }
      break;

      case GC_SETMESSAGEHIGHLIGHT:
      {
         si->wState |= GC_EVENT_HIGHLIGHT;
         SendMessage(si->hWnd, GC_FIXTABICONS, 0, 0);
         if (DBGetContactSettingByte(NULL, "Chat", "FlashWindowHighlight", 0) != 0 && GetActiveWindow() != hwndDlg && GetForegroundWindow() != GetParent(hwndDlg))
            SendMessage(GetParent(si->hWnd), CM_STARTFLASHING, 0, 0);
      }
      break;

   case GC_SETTABHIGHLIGHT:
      {
         SendMessage(si->hWnd, GC_FIXTABICONS, 0, 0);
         if (g_Settings.FlashWindow && GetActiveWindow() != GetParent(hwndDlg) && GetForegroundWindow() != GetParent(hwndDlg))
            SendMessage(GetParent(si->hWnd), CM_STARTFLASHING, 0, 0);
      }
      break;

   case DM_ACTIVATE:
      {
         if (si->wState & STATE_TALK) {
            si->wState &= ~STATE_TALK;

            DBWriteContactSettingWord(si->hContact, si->pszModule ,"ApparentMode",(LPARAM) 0);
         }

         if (si->wState & GC_EVENT_HIGHLIGHT) {
            si->wState &= ~GC_EVENT_HIGHLIGHT;

            if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
               CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)"chaticon");
         }

         SendMessage(hwndDlg, GC_FIXTABICONS, 0, 0);
         if (!si->hWnd) {
            ShowRoom(si, (WPARAM)WINDOW_VISIBLE, TRUE);
            SendMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
      }   }
      break;



   case GC_ACKMESSAGE:
      SendDlgItemMessage(hwndDlg,IDC_CHAT_MESSAGE,EM_SETREADONLY,FALSE,0);
      SendDlgItemMessage(hwndDlg,IDC_CHAT_MESSAGE,WM_SETTEXT,0, (LPARAM)_T(""));
      return TRUE;

   case WM_CTLCOLORLISTBOX:
      SetBkColor((HDC) wParam, g_Settings.crUserListBGColor);
      return (BOOL) hListBkgBrush;

   case WM_MEASUREITEM:
      {
         MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *) lParam;
         int ih = GetTextPixelSize( _T("AQGgl'"), g_Settings.UserListFont,FALSE);
         int ih2 = GetTextPixelSize( _T("AQGg'"), g_Settings.UserListHeadingsFont,FALSE);
         int font = ih > ih2?ih:ih2;
         int height = DBGetContactSettingByte(NULL, "Chat", "NicklistRowDist", 12);

         mis->itemHeight = height > font?height:font;
         return TRUE;
      }

   case WM_DRAWITEM:
      {
         DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
         if (dis->CtlID == IDC_CHAT_LIST) {
            HFONT  hFont, hOldFont;
            HICON  hIcon;
            int offset;
            int height;
            int index = dis->itemID;
            USERINFO * ui = SM_GetUserFromIndex(si->ptszID, si->pszModule, index);
            if (ui) {
               height = dis->rcItem.bottom - dis->rcItem.top;

               if (height&1)
                  height++;
               if (height == 10)
                  offset = 0;
               else
                  offset = height/2 - 4;
               hIcon = SM_GetStatusIcon(si, ui);
               hFont = (ui->iStatusEx == 0) ? g_Settings.UserListFont : g_Settings.UserListHeadingsFont;
               hOldFont = (HFONT) SelectObject(dis->hDC, hFont);
               SetBkMode(dis->hDC, TRANSPARENT);

               if (dis->itemAction == ODA_FOCUS && dis->itemState & ODS_SELECTED)
                  FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
               else //if (dis->itemState & ODS_INACTIVE)
                  FillRect(dis->hDC, &dis->rcItem, hListBkgBrush);
               DrawIconEx(dis->hDC,2, dis->rcItem.top + offset,hIcon,10,10,0,NULL, DI_NORMAL);

               SetTextColor(dis->hDC, ui->iStatusEx == 0?g_Settings.crUserListColor:g_Settings.crUserListHeadingsColor);
               TextOut(dis->hDC, dis->rcItem.left+14, dis->rcItem.top, ui->pszNick, lstrlen(ui->pszNick));
               SelectObject(dis->hDC, hOldFont);
            }
            return TRUE;
      }   }

   case GC_UPDATENICKLIST:
      {
         int i = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_GETTOPINDEX, 0, 0);
         SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_SETCOUNT, si->nUsersInNicklist, 0);
         SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_SETTOPINDEX, i, 0);
         SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
      }
      break;

   case GC_EVENT_CONTROL + WM_USER+500:
      {
         switch(wParam) {
         case SESSION_OFFLINE:
            SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
            SendMessage(si->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);
            return TRUE;

         case SESSION_ONLINE:
            SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
            return TRUE;

         case WINDOW_HIDDEN:
            SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
            return TRUE;

         case WINDOW_CLEARLOG:
            SetDlgItemText(hwndDlg, IDC_CHAT_LOG, _T(""));
            return TRUE;

         case SESSION_TERMINATE:
            if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
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
            if (DBGetContactSettingByte(NULL, "Chat", "PopupOnJoin", 0)!=0)
               return TRUE;
            // fall through
         case WINDOW_VISIBLE:
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
            return TRUE;
      }   }
      break;

   case GC_SPLITTERMOVED:
      {   POINT pt;
         RECT rc;
         RECT rcLog;
         BOOL bFormat = IsWindowVisible(GetDlgItem(hwndDlg,IDC_CHAT_SMILEY));

         static int x = 0;

         GetWindowRect(GetDlgItem(hwndDlg,IDC_CHAT_LOG),&rcLog);
         if ((HWND)lParam==GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERX)) {
            int oldSplitterX;
            GetClientRect(hwndDlg,&rc);
            pt.x=wParam; pt.y=0;
            ScreenToClient(hwndDlg,&pt);

            oldSplitterX=si->iSplitterX;
            si->iSplitterX=rc.right-pt.x+1;
            if (si->iSplitterX < 35)
               si->iSplitterX=35;
            if (si->iSplitterX > rc.right-rc.left-35)
               si->iSplitterX = rc.right-rc.left-35;
            g_Settings.iSplitterX = si->iSplitterX;
         }
         else if ((HWND)lParam==GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERY)) {
            int oldSplitterY;
            GetClientRect(hwndDlg,&rc);
            pt.x=0; pt.y=wParam;
            ScreenToClient(hwndDlg,&pt);

            oldSplitterY=si->iSplitterY;
            si->iSplitterY=bFormat?rc.bottom-pt.y+1:rc.bottom-pt.y+20;
            if (si->iSplitterY<63)
               si->iSplitterY=63;
            if (si->iSplitterY>rc.bottom-rc.top-40)
               si->iSplitterY = rc.bottom-rc.top-40;
            g_Settings.iSplitterY = si->iSplitterY;
         }
         if (x==2) {
            PostMessage(hwndDlg,WM_SIZE,0,0);
            x = 0;
         }
         else x++;
      }
      break;

   case GC_FIREHOOK:
      if (lParam) {
         GCHOOK* gch = (GCHOOK *) lParam;
         NotifyEventHooks(hSendEvent,0,(WPARAM)gch);
         if ( gch->pDest ) {
            mir_free( gch->pDest->pszID );
            mir_free( gch->pDest->pszModule );
            mir_free( gch->pDest );
         }
         mir_free( gch->ptszText );
         mir_free( gch->ptszUID );
         mir_free( gch );
      }
      break;

   case GC_CHANGEFILTERFLAG:
      si->iLogFilterFlags = lParam;
      break;

   case GC_SHOWFILTERMENU:
      {
         RECT rc;
         HWND hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), hwndDlg, FilterWndProc, (LPARAM)si);
         GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), &rc);
         SetWindowPos(hwnd, HWND_TOP, rc.left-85, (IsWindowVisible(GetDlgItem(hwndDlg, IDC_CHAT_FILTER))||IsWindowVisible(GetDlgItem(hwndDlg, IDC_CHAT_BOLD)))?rc.top-206:rc.top-186, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
      }
      break;

   case GC_SHOWCOLORCHOOSER:
      {
         HWND ColorWindow;
         RECT rc;
         BOOL bFG = lParam == IDC_CHAT_COLOR?TRUE:FALSE;
         COLORCHOOSER * pCC = mir_alloc(sizeof(COLORCHOOSER));

         GetWindowRect(GetDlgItem(hwndDlg, bFG?IDC_CHAT_COLOR:IDC_CHAT_BKGCOLOR), &rc);
         pCC->hWndTarget = GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE);
         pCC->pModule = MM_FindModule(si->pszModule);
         pCC->xPosition = rc.left+3;
         pCC->yPosition = IsWindowVisible(GetDlgItem(hwndDlg, IDC_CHAT_COLOR))?rc.top-1:rc.top+20;
         pCC->bForeground = bFG;
         pCC->si = si;

         ColorWindow= CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_COLORCHOOSER), hwndDlg, DlgProcColorToolWindow, (LPARAM) pCC);
      }
      break;

   case GC_SCROLLTOBOTTOM:
      {
         SCROLLINFO si = { 0 };
         if ((GetWindowLong(GetDlgItem(hwndDlg, IDC_CHAT_LOG), GWL_STYLE) & WS_VSCROLL) != 0){
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
      }   }
      break;






   case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_ACTIVE)
         break;

      //fall through
   case WM_MOUSEACTIVATE:
      {
         if (uMsg != WM_ACTIVATE)
            SetFocus(GetDlgItem(hwndDlg,IDC_CHAT_MESSAGE));

         SetActiveSession(si->ptszID, si->pszModule);

         if (DBGetContactSettingWord(si->hContact, si->pszModule ,"ApparentMode", 0) != 0)
            DBWriteContactSettingWord(si->hContact, si->pszModule ,"ApparentMode",(LPARAM) 0);
         if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
            CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)"chaticon");
      }
      break;

   case WM_NOTIFY:
      {
         LPNMHDR pNmhdr;

         pNmhdr = (LPNMHDR)lParam;
         switch (pNmhdr->code) {
         case EN_MSGFILTER:
            if (pNmhdr->idFrom == IDC_CHAT_LOG && ((MSGFILTER *) lParam)->msg == WM_RBUTTONUP){
               CHARRANGE sel, all = { 0, -1 };
               POINT pt;
               UINT uID = 0;
               HMENU hMenu = 0;
               TCHAR pszWord[4096];

               pt.x = (short) LOWORD(((ENLINK *) lParam)->lParam);
               pt.y = (short) HIWORD(((ENLINK *) lParam)->lParam);
               ClientToScreen(pNmhdr->hwndFrom, &pt);

               { // fixing stuff for searches
                  long iCharIndex, iLineIndex, iChars, start, end, iRes;
                  POINTL ptl;

                  pszWord[0] = _T('\0');
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

                  if (end - start > 0) {
                     TEXTRANGE tr;
                     CHARRANGE cr;
                     static TCHAR szTrimString[] = _T(":;,.!?\'\"><()[]- \r\n");
                     ZeroMemory(&tr, sizeof(TEXTRANGE));

                     cr.cpMin = start;
                     cr.cpMax = end;
                     tr.chrg = cr;
                     tr.lpstrText = pszWord;
                     iRes = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_GETTEXTRANGE, 0, (LPARAM)&tr);
                     if (iRes > 0) {
                        int iLen = lstrlen(pszWord)-1;
                        while(iLen >= 0 && _tcschr(szTrimString, pszWord[iLen])) {
                           pszWord[iLen] = _T('\0');
                           iLen--;
               }   }   }   }

               uID = CreateGCMenu(hwndDlg, &hMenu, 1, pt, si, NULL, pszWord);
               switch (uID) {
               case 0:
                  PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
                  break;

               case ID_COPYALL:
                  SendMessage(pNmhdr->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);
                  SendMessage(pNmhdr->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & all);
                  SendMessage(pNmhdr->hwndFrom, WM_COPY, 0, 0);
                  SendMessage(pNmhdr->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & sel);
                  PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
                  break;

               case ID_CLEARLOG:
                  {
                     SESSION_INFO* s = SM_FindSession(si->ptszID, si->pszModule);
                     if (s)
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
                  }   }
                  break;

               case ID_SEARCH_GOOGLE:
                  {
                     char szURL[4096];
                     if (pszWord[0]) {
                        mir_snprintf( szURL, sizeof( szURL ), "http://www.google.com/search?q=" TCHAR_STR_PARAM, pszWord );
                        CallService(MS_UTILS_OPENURL, 1, (LPARAM) szURL);
                     }
                     PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
                  }
                  break;

               case ID_SEARCH_WIKIPEDIA:
                  {
                     char szURL[4096];
                     if (pszWord[0]) {
                        mir_snprintf( szURL, sizeof( szURL ), "http://en.wikipedia.org/wiki/" TCHAR_STR_PARAM, pszWord );
                        CallService(MS_UTILS_OPENURL, 1, (LPARAM) szURL);
                     }
                     PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
                  }
                  break;

               default:
                  PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
                  DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_LOGMENU, NULL, NULL, (LPARAM)uID);
                  break;
               }
               DestroyGCMenu(&hMenu, 5);
            }
            break;

         case EN_LINK:
			if (pNmhdr->idFrom == IDC_CHAT_LOG) {
				switch (((ENLINK *) lParam)->msg) {
				case WM_RBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_LBUTTONDBLCLK:
					{
						TEXTRANGE tr;
						CHARRANGE sel;
						char* pszUrl;

						SendMessage(pNmhdr->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);
						if (sel.cpMin != sel.cpMax)
							break;
						tr.chrg = ((ENLINK *) lParam)->chrg;
						tr.lpstrText = mir_alloc(sizeof(TCHAR)*(tr.chrg.cpMax - tr.chrg.cpMin + 1));
						SendMessage(pNmhdr->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM) & tr);
						pszUrl = t2a( tr.lpstrText );

						if (((ENLINK *) lParam)->msg == WM_RBUTTONDOWN) {
							HMENU hSubMenu;
							POINT pt;

							hSubMenu = GetSubMenu(g_hMenu, 2);
							CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
							pt.x = (short) LOWORD(((ENLINK *) lParam)->lParam);
							pt.y = (short) HIWORD(((ENLINK *) lParam)->lParam);
							ClientToScreen(((NMHDR *) lParam)->hwndFrom, &pt);
							switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL)) {
							case ID_NEW:
								CallService(MS_UTILS_OPENURL, 1, (LPARAM) pszUrl);
								break;

							case ID_CURR:
								CallService(MS_UTILS_OPENURL, 0, (LPARAM) pszUrl);
								break;

							case ID_COPY:
								{
									HGLOBAL hData;
									if (!OpenClipboard(hwndDlg))
										break;
									EmptyClipboard();
									hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR)*(lstrlen(tr.lpstrText) + 1));
									lstrcpy(( TCHAR* )GlobalLock(hData), tr.lpstrText);
									GlobalUnlock(hData);
									#if defined( _UNICODE )
										SetClipboardData(CF_UNICODETEXT, hData);
									#else
										SetClipboardData(CF_TEXT, hData);
									#endif
									CloseClipboard();
									SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
									break;
							}	}
							mir_free(tr.lpstrText);
							mir_free(pszUrl);
							return TRUE;
						}

						CallService(MS_UTILS_OPENURL, 1, (LPARAM) pszUrl);
						SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
						mir_free(tr.lpstrText);
						mir_free(pszUrl);
						break;
			}	}	}
			break;
			case TTN_NEEDTEXT:
				if (pNmhdr->idFrom == (UINT)GetDlgItem(hwndDlg,IDC_CHAT_LIST))
				{
					LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lParam;
					POINT p;
					int item;
					USERINFO * ui;
					SESSION_INFO* parentdat =(SESSION_INFO*)GetWindowLong(hwndDlg,GWL_USERDATA);

					GetCursorPos( &p );
					ScreenToClient( hwndDlg, &p );
					item = LOWORD(SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(p.x, p.y)));
					ui = SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, item);
					if ( ui != NULL ) {
						static TCHAR ptszBuf[ 1024 ];
						mir_sntprintf( ptszBuf, SIZEOF(ptszBuf), _T("%s: %s\r\n%s: %s\r\n%s: %s"),
							TranslateT( "Nick name" ), ui->pszNick,
							TranslateT( "Unique id" ), ui->pszUID,
							TranslateT( "Status" ), TM_WordToString( parentdat->pStatuses, ui->Status ));
						lpttd->lpszText = ptszBuf;
				}	}
				break;
		}	}
		break;
   case WM_COMMAND:
      switch (LOWORD(wParam)) {
      case IDC_CHAT_LIST:
         if (HIWORD(wParam) == LBN_DBLCLK) {
            TVHITTESTINFO hti;
            int item;
            USERINFO * ui;

            hti.pt.x=(short)LOWORD(GetMessagePos());
            hti.pt.y=(short)HIWORD(GetMessagePos());
            ScreenToClient(GetDlgItem(hwndDlg, IDC_CHAT_LIST),&hti.pt);

            item = LOWORD(SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
            ui = SM_GetUserFromIndex(si->ptszID, si->pszModule, item);
            if (ui) {
               if (GetKeyState(VK_SHIFT) & 0x8000){
                  LRESULT lResult = (LRESULT)SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_GETSEL, (WPARAM)NULL, (LPARAM)NULL);
                  int start = LOWORD(lResult);
                  TCHAR* pszName = (TCHAR*)alloca(sizeof(TCHAR)*(lstrlen(ui->pszUID) + 3));
                  if (start == 0)
                     mir_sntprintf(pszName, lstrlen(ui->pszUID)+3, _T("%s: "), ui->pszUID);
                  else
                     mir_sntprintf(pszName, lstrlen(ui->pszUID)+2, _T("%s "), ui->pszUID);

                  SendMessageA(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_REPLACESEL, FALSE, (LPARAM) pszName);
                  PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0);
               }
               else DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, (LPARAM)NULL);
            }

            return TRUE;
         }

         if ( HIWORD(wParam) == LBN_KILLFOCUS )
            RedrawWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NULL, NULL, RDW_INVALIDATE);
         break;

      case IDOK:
         {
            char*  pszRtf;
            TCHAR* ptszText, *p1;
            if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDOK)))
               break;

            pszRtf = Message_GetFromStream(hwndDlg, si);
            SM_AddCommand(si->ptszID, si->pszModule, pszRtf);
            ptszText = DoRtfToTags(pszRtf, si);
            p1 = _tcschr(ptszText, '\0');

            //remove trailing linebreaks
            while ( p1 > ptszText && (*p1 == '\0' || *p1 == '\r' || *p1 == '\n')) {
               *p1 = '\0';
               p1--;
            }

            if ( MM_FindModule(si->pszModule)->bAckMsg ) {
               EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_MESSAGE),FALSE);
               SendDlgItemMessage(hwndDlg,IDC_CHAT_MESSAGE,EM_SETREADONLY,TRUE,0);
            }
            else SendDlgItemMessage(hwndDlg,IDC_CHAT_MESSAGE,WM_SETTEXT,0,(LPARAM)_T(""));

            EnableWindow(GetDlgItem(hwndDlg,IDOK),FALSE);

            DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_MESSAGE, NULL, ptszText, (LPARAM)NULL);
            mir_free(pszRtf);
            #if defined( _UNICODE )
               mir_free(ptszText);
            #endif
            SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
         }
         break;

      case IDC_CHAT_SHOWNICKLIST:
         if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_SHOWNICKLIST)))
            break;
         if (si->iType == GCW_SERVER)
            break;

         si->bNicklistEnabled = !si->bNicklistEnabled;

         SendDlgItemMessage(hwndDlg,IDC_CHAT_SHOWNICKLIST,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(si->bNicklistEnabled?IDI_NICKLIST:IDI_NICKLIST2, si->bNicklistEnabled?"nicklist":"nicklist2", 0, 0 ));
         SendMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
         SendMessage(hwndDlg, WM_SIZE, 0, 0);
         break;

      case IDC_CHAT_MESSAGE:
         EnableWindow(GetDlgItem(hwndDlg, IDOK), GetRichTextLength(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE)) != 0);
         break;

      case IDC_CHAT_SMILEY:
         {
            SMADD_SHOWSEL3 smaddInfo;
            RECT rc;

            GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_SMILEY), &rc);

            smaddInfo.cbSize = sizeof(SMADD_SHOWSEL3);
			smaddInfo.hwndParent = GetParent(hwndDlg);
            smaddInfo.hwndTarget = GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE);
            smaddInfo.targetMessage = EM_REPLACESEL;
            smaddInfo.targetWParam = TRUE;
            smaddInfo.Protocolname = si->pszModule;
            //smaddInfo.Direction = 3;
			smaddInfo.Direction = 0;
            smaddInfo.xPosition = rc.left;
            smaddInfo.yPosition = rc.bottom;
            smaddInfo.hContact = si->hContact;
			CallService(MS_SMILEYADD_SHOWSELECTION, 0, (LPARAM) &smaddInfo);
         }
         break;

      case IDC_CHAT_HISTORY:
         {
            char szFile[MAX_PATH];
            char szName[MAX_PATH];
            char szFolder[MAX_PATH];
			char *pszSessionName;
            MODULEINFO * pInfo = MM_FindModule(si->pszModule);

            if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_HISTORY)))
               break;

            if ( pInfo ) {
               mir_snprintf(szName, MAX_PATH, "%s", pInfo->pszModDispName);
               ValidateFilename(szName);
               mir_snprintf(szFolder, MAX_PATH,"%s\\%s", g_Settings.pszLogDir, szName );

				pszSessionName = t2a( si->ptszID );
				mir_snprintf( szName, MAX_PATH,"%s.log", pszSessionName );
				ValidateFilename(szName);
				mir_free( pszSessionName );

               mir_snprintf(szFile, MAX_PATH,"%s\\%s", szFolder, szName );

               ShellExecuteA(hwndDlg, "open", szFile, NULL, NULL, SW_SHOW);
         }   }
         break;

      case IDC_CHAT_CHANMGR:
         if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_CHANMGR)))
            break;
         DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_CHANMGR, NULL, NULL, (LPARAM)NULL);
         break;

      case IDC_CHAT_FILTER:
         if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_FILTER)))
            break;

         si->bFilterEnabled = !si->bFilterEnabled;
         SendDlgItemMessage(hwndDlg,IDC_CHAT_FILTER,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx(si->bFilterEnabled?IDI_FILTER:IDI_FILTER2, si->bFilterEnabled?"filter":"filter2", 0, 0 ));
         if (si->bFilterEnabled && DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) == 0) {
            SendMessage(hwndDlg, GC_SHOWFILTERMENU, 0, 0);
            break;
         }
         SendMessage(hwndDlg, GC_REDRAWLOG, 0, 0);
         break;

      case IDC_CHAT_BKGCOLOR:
         {
            CHARFORMAT2 cf;

            cf.cbSize = sizeof(CHARFORMAT2);
            cf.dwEffects = 0;

            if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_BKGCOLOR)))
               break;

            if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_BKGCOLOR)) {
               if (DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) == 0)
                  SendMessage(hwndDlg, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_BKGCOLOR);
               else if (si->bBGSet){
                  cf.dwMask = CFM_BACKCOLOR;
                  cf.crBackColor = MM_FindModule(si->pszModule)->crColors[si->iBG];
                  SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
            }   }
            else {
               cf.dwMask = CFM_BACKCOLOR;
               cf.crBackColor = (COLORREF)DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW));
               SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
         }   }
         break;

      case IDC_CHAT_COLOR:
         {
            CHARFORMAT2 cf;
            cf.cbSize = sizeof(CHARFORMAT2);
            cf.dwEffects = 0;

            if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_COLOR)))
               break;

            if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_COLOR) ) {
               if (DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) == 0)
                  SendMessage(hwndDlg, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_COLOR);
               else if (si->bFGSet) {
                  cf.dwMask = CFM_COLOR;
                  cf.crTextColor = MM_FindModule(si->pszModule)->crColors[si->iFG];
                  SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
            }   }
            else {
               COLORREF cr;

               Chat_LoadMsgDlgFont(17, NULL, &cr);
               cf.dwMask = CFM_COLOR;
               cf.crTextColor = cr;
               SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

         }   }
         break;

      case IDC_CHAT_BOLD:
      case IDC_CHAT_ITALICS:
      case IDC_CHAT_UNDERLINE:

         {
            CHARFORMAT2 cf;
            cf.cbSize = sizeof(CHARFORMAT2);
            cf.dwMask = CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE;
            cf.dwEffects = 0;

            if (LOWORD(wParam) == IDC_CHAT_BOLD && !IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_BOLD)))
               break;
            if (LOWORD(wParam) == IDC_CHAT_ITALICS && !IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_ITALICS)))
               break;
            if (LOWORD(wParam) == IDC_CHAT_UNDERLINE && !IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHAT_UNDERLINE)))
               break;
            if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_BOLD))
               cf.dwEffects |= CFE_BOLD;
            if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_ITALICS))
               cf.dwEffects |= CFE_ITALIC;
            if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_UNDERLINE))
               cf.dwEffects |= CFE_UNDERLINE;

            SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
      }   }
      break;

   case WM_KEYDOWN:
      SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
      break;



      case WM_GETMINMAXINFO:
      {
         MINMAXINFO* mmi = (MINMAXINFO*)lParam;
         mmi->ptMinTrackSize.x = si->iSplitterX + 43;
         if (mmi->ptMinTrackSize.x < 350)
            mmi->ptMinTrackSize.x = 350;

         mmi->ptMinTrackSize.y = si->iSplitterY + 80;
      }
      break;

   case WM_LBUTTONDBLCLK:
      if (LOWORD(lParam) < 30)
         PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
      break;


   case WM_CLOSE:
      SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
      break;

   case GC_CLOSEWINDOW:
      DestroyWindow(hwndDlg);
      break;

   case WM_DESTROY:

		NotifyLocalWinEvent(si->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSING);

		si->hWnd = NULL;

		if (si->hwndTooltip != NULL) {
			HWND hNickList = GetDlgItem(hwndDlg,IDC_CHAT_LIST);
			TOOLINFO ti = { 0 };
			ti.cbSize = sizeof(TOOLINFO);
			ti.uId = (UINT)hNickList;
			ti.hwnd = hNickList;
			SendMessage( si->hwndTooltip, TTM_DELTOOL, 0, (LPARAM)(LPTOOLINFO)&ti );
		}
		DestroyWindow( si->hwndTooltip );
		si->hwndTooltip = NULL;

		SetWindowLong(hwndDlg,GWL_USERDATA,0);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERX),GWL_WNDPROC,(LONG)OldSplitterProc);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_SPLITTERY),GWL_WNDPROC,(LONG)OldSplitterProc);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_LIST),GWL_WNDPROC,(LONG)OldNicklistProc);
		SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_UNSUBCLASSED, 0, 0);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_MESSAGE),GWL_WNDPROC,(LONG)OldMessageProc);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_LOG),GWL_WNDPROC,(LONG)OldLogProc);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_FILTER),GWL_WNDPROC,(LONG)OldFilterButtonProc);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_COLOR),GWL_WNDPROC,(LONG)OldFilterButtonProc);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_BKGCOLOR),GWL_WNDPROC,(LONG)OldFilterButtonProc);

		SendMessage(GetParent(hwndDlg), CM_REMOVECHILD, 0, (LPARAM) hwndDlg);

		NotifyLocalWinEvent(si->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSE);
		break;
   }
   return(FALSE);
}
