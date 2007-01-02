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
HWND hwnd_list = NULL;
HWND hwnd_container = NULL;

int frame_id = -1;

static LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


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
			HWND hwnd_list = GetDlgItem(hwnd, IDC_CALLS);

			SendMessage(hwnd_list, LB_RESETCONTENT, 0, 0);

			for(int i = 0; i < calls.size(); i++)
			{
				TCHAR text[512];
				mir_sntprintf(text, MAX_REGS(text), _T("%s: %s"), GetStateName(calls[i].state), calls[i].ptszContact);
				SendMessage(hwnd_list, LB_ADDSTRING, 0, (LPARAM) text);
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
			
			if (calls[pos].state != TALKING || !CanHoldCall(&calls[pos]))
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
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

