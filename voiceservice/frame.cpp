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


HWND hwnd_frame = NULL;
HWND hwnd_container = NULL;

int frame_id = -1;

static LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


#define H_SPACE 2


// Functions //////////////////////////////////////////////////////////////////////////////////////

void InitFrames()
{
	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
	{
		hwnd_frame = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CALLS), 
			(HWND) CallService(MS_CLUI_GETHWND, 0, 0), (DLGPROC) FrameWndProc); 

		CLISTFrame Frame = {0};
		Frame.cbSize = sizeof(CLISTFrame);
		Frame.name = Translate("Voice Calls");
		Frame.hWnd = hwnd_frame;
		Frame.align = alBottom;
		Frame.Flags = F_VISIBLE | F_SHOWTB | F_NOBORDER | F_LOCKED;
		Frame.height = 50;
		Frame.hIcon = icons[MAIN_ICON];

		frame_id = CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);
	}
}


void DeInitFrames()
{
	if (ServiceExists(MS_CLIST_FRAMES_REMOVEFRAME) && frame_id != -1) 
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)frame_id, 0);

	if (hwnd_frame != NULL) 
		DestroyWindow(hwnd_frame);
}


static LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch(msg) 
	{
		case WM_CREATE: 
		case WM_INITDIALOG:
		{
			break;
		}

		case WM_DESTROY:
		{
			break;
		}

		case WM_SIZE:
		{
			MoveWindow(GetDlgItem(hwnd, IDC_CALLS), 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
			break;
		}

		case WMU_REFRESH:
		{
			HWND list = GetDlgItem(hwnd, IDC_CALLS);

			SendMessage(list, LB_RESETCONTENT, 0, 0);

			for(int i = 0; i < calls.size(); i++)
			{
				TCHAR text[512];
				mir_sntprintf(text, MAX_REGS(text), _T("%d %s"), calls[i].state, calls[i].ptszContact);

				int pos = SendMessage(list, LB_ADDSTRING, 0, (LPARAM) text);
				if (pos == LB_ERR)
					// TODO Show error
					break;

				SendMessage(list, LB_SETITEMDATA, pos, (LPARAM) &calls[i]);
			}

			break;
		}

		case WM_COMMAND:
		{
			HWND list = GetDlgItem(hwnd, IDC_CALLS);
			int i = HIWORD(lParam);
			if ((HANDLE) lParam != list || HIWORD(wParam) != LBN_SELCHANGE)
				break;

			int pos = SendMessage(list, LB_GETCURSEL, 0, 0);
			if (pos == LB_ERR)
				break;

			POINT p;
			GetCursorPos(&p);
			ScreenToClient(list, &p);

			int ret = SendMessage(list, LB_ITEMFROMPOINT, 0, MAKELONG(p.x, p.y));
			if (HIWORD(ret))
				break;
			if (pos != LOWORD(ret))
				break;

			RECT rc;
			SendMessage(list, LB_GETITEMRECT, pos, (LPARAM) &rc);
			int x = rc.right - p.x;

			int action;
			if (x >= H_SPACE && x <= ICON_SIZE + H_SPACE)
				action = 2;
			else if (x >= ICON_SIZE + 2 * H_SPACE && x <= 2 * (ICON_SIZE + H_SPACE))
				action = 1;
			else
				break;

			VOICE_CALL_INTERNAL *vc = (VOICE_CALL_INTERNAL *) SendMessage(list, LB_GETITEMDATA, pos, 0);
			switch (vc->state)
			{
				case TALKING:
				{
					if (action == 1)
						HoldCall(vc);
					else
						DropCall(vc);
					break;
				}
				case RINGING:
				case ON_HOLD:
				{
					if (action == 1)
						AnswerCall(vc);
					else
						DropCall(vc);
					break;
				}
			}

			break;
		}

		case WM_CONTEXTMENU:
		{
			HWND list = GetDlgItem(hwnd, IDC_CALLS);
			if ((HANDLE) wParam != list)
				break;

			POINT p;
			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 
			ScreenToClient(list, &p);

			int pos = SendMessage(list, LB_ITEMFROMPOINT, 0, MAKELONG(p.x, p.y));
			if (HIWORD(pos))
				break;
			pos = LOWORD(pos);

			if (pos >= calls.size())
				break;

			if (calls[pos].state == ENDED)
				break;

			// Just to get things strait
			SendMessage(list, LB_SETCURSEL, pos, 0);

			HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENUS));
			HMENU submenu = GetSubMenu(menu, 0);
			CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)submenu, 0);

			if (calls[pos].state == TALKING)
				DeleteMenu(menu, ID_FRAMEPOPUP_ANSWERCALL, MF_BYCOMMAND);
			
			if (calls[pos].state != TALKING || !(calls[pos].module->flags & VOICE_CAN_HOLD))
				DeleteMenu(menu, ID_FRAMEPOPUP_HOLDCALL, MF_BYCOMMAND);

			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 
			int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
			DestroyMenu(menu);

			switch(ret)
			{
				case ID_FRAMEPOPUP_DROPCALL:
				{
					DropCall(&calls[pos]);
					break;
				}
				case ID_FRAMEPOPUP_ANSWERCALL:
				{
					AnswerCall(&calls[pos]);
					break;
				}
				case ID_FRAMEPOPUP_HOLDCALL:
				{
					HoldCall(&calls[pos]);
					break;
				}
			}

			break;
		}

		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT) lParam;
			if (mis->CtlID != IDC_CALLS) 
				break;

			mis->itemHeight = max(ICON_SIZE, font_max_height) + 1;
			
			return TRUE;
		}

		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlID != IDC_CALLS || dis->itemID == -1) 
				break;

			VOICE_CALL_INTERNAL *vc = (VOICE_CALL_INTERNAL *)dis->itemData;

			RECT rc = dis->rcItem;
			rc.left += H_SPACE;
			rc.right -= H_SPACE;
			rc.bottom --;

			// Draw status
			DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[vc->state], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

			// Draw contact
			rc.left += ICON_SIZE + H_SPACE;
			rc.right -= 2 * (ICON_SIZE + H_SPACE);

			HFONT old_font = (HFONT) SelectObject(dis->hDC, fonts[vc->state]);
			COLORREF old_color = SetTextColor(dis->hDC, font_colors[vc->state]);

			DrawText(dis->hDC, vc->ptszContact, -1, &rc, DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS | DT_VCENTER);

			SelectObject(dis->hDC, old_font);
			SetTextColor(dis->hDC, old_color);

			// Draw action icons
			rc = dis->rcItem;
			rc.right -= H_SPACE;
			rc.bottom --;

			switch (vc->state)
			{
				case TALKING:
				{
					rc.left = rc.right - ICON_SIZE;
					DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[NUM_STATES + ACTION_DROP], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

					if (vc->module->flags & VOICE_CAN_HOLD)
					{
						rc.right -= ICON_SIZE + H_SPACE;
						rc.left = rc.right - ICON_SIZE;
						DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[NUM_STATES + ACTION_HOLD], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
					}

					break;
				}
				case RINGING:
				case ON_HOLD:
				{
					rc.left = rc.right - ICON_SIZE;
					DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[NUM_STATES + ACTION_DROP], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

					rc.right -= ICON_SIZE + H_SPACE;
					rc.left = rc.right - ICON_SIZE;
					DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[NUM_STATES + ACTION_ANSWER], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

					break;
				}
			}

			
			return TRUE;
		}

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

