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
		Frame.tname = TranslateT("Voice Calls");
		Frame.hWnd = hwnd_frame;
		Frame.align = alBottom;
		Frame.Flags = F_VISIBLE | F_NOBORDER | F_LOCKED | F_TCHAR;
		Frame.height = 0;
		Frame.hIcon = icons[MAIN_ICON];

		frame_id = CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);

		PostMessage(hwnd_frame, WMU_RESIZE_FRAME, 0, 0);
	}
}


void DeInitFrames()
{
	if (ServiceExists(MS_CLIST_FRAMES_REMOVEFRAME) && frame_id != -1) 
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)frame_id, 0);

	if (hwnd_frame != NULL) 
		DestroyWindow(hwnd_frame);
}


static int GetMaxLineHeight() {
	return max(ICON_SIZE, font_max_height) + 1;
}


BOOL FrameIsFloating(int frame_id) 
{
	if (frame_id == -1) 
		return TRUE; // no frames, always floating
	
	return CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, frame_id), 0);
}

void ResizeFrame(int frame_id, HWND hwnd)
{
	int height = calls.getCount() * GetMaxLineHeight();
	if (CanCallNumber())
	{
		RECT dp;
		GetWindowRect(GetDlgItem(hwnd, IDC_NUMBER), &dp);
		height += dp.bottom - dp.top + 1;

		if (SendMessage(GetDlgItem(hwnd, IDC_DIALPAD), BM_GETCHECK, 0, 0) == BST_CHECKED)
		{
			RECT first, last;
			GetWindowRect(GetDlgItem(hwnd, IDC_1), &first);
			GetWindowRect(GetDlgItem(hwnd, IDC_SHARP), &last);

			height += last.bottom - first.top + 1;
		}
	}

	if (FrameIsFloating(frame_id)) 
	{
		HWND parent = GetParent(hwnd);
		if (parent == NULL)
			return;

		RECT r_client;
		GetClientRect(hwnd, &r_client);

		if (r_client.bottom - r_client.top == height)
			return;

		RECT parent_client, parent_window, r_window;
		GetClientRect(parent, &parent_client);
		GetWindowRect(parent, &parent_window);
		GetWindowRect(hwnd, &r_window);

		int diff = (parent_window.bottom - parent_window.top) - (parent_client.bottom - parent_client.top);
		if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
			diff += (r_window.top - parent_window.top);

		SetWindowPos(parent, 0, 0, 0, parent_window.right - parent_window.left, height + diff, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	else
	{
		int old_height = CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, frame_id), 0);
		if (old_height == height)
			return;
		
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, frame_id), (LPARAM) height);
		CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)(FU_TBREDRAW | FU_FMREDRAW | FU_FMPOS));
	}
}

void ShowFrame(int frame_id, HWND hwnd, int show)
{
	BOOL is_visible = IsWindowVisible(hwnd);
	if ((is_visible && show == SW_SHOW) || (!is_visible && show == SW_HIDE))
		return;

	if (ServiceExists(MS_CLIST_FRAMES_SHFRAME) && frame_id != -1) 
		CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)frame_id, 0);
	else 
		ShowWindow(GetParent(hwnd), show);
}


static int dialCtrls[] = {
	IDC_DIALPAD, IDC_NUMBER, IDC_CALL,
	IDC_1, IDC_2, IDC_3,
	IDC_4, IDC_5, IDC_6,
	IDC_7, IDC_8, IDC_9,
	IDC_AST, IDC_0, IDC_SHARP
};



static void InvalidateAll(HWND hwnd)
{
	InvalidateRect(GetDlgItem(hwnd, IDC_CALLS), NULL, FALSE);
	for(int i = 0; i < MAX_REGS(dialCtrls); ++i)
		InvalidateRect(GetDlgItem(hwnd, dialCtrls[i]), NULL, FALSE);
	InvalidateRect(hwnd, NULL, FALSE);
}


static void ShowHideDialpad(HWND hwnd)
{
	if (!CanCallNumber())
	{
		for(int i = 0; i < MAX_REGS(dialCtrls); ++i)
			ShowWindow(GetDlgItem(hwnd, dialCtrls[i]), SW_HIDE);
	}
	else
	{
		int i;
		for(i = 0; i < 3; ++i)
			ShowWindow(GetDlgItem(hwnd, dialCtrls[i]), SW_SHOW);

		bool showDialpad = (SendMessage(GetDlgItem(hwnd, IDC_DIALPAD), BM_GETCHECK, 0, 0) == BST_CHECKED);

		for(i = 3; i < MAX_REGS(dialCtrls); ++i)
			ShowWindow(GetDlgItem(hwnd, dialCtrls[i]), showDialpad ? SW_SHOW : SW_HIDE);

		VoiceCall *talking = NULL;
		bool ringing = false;
		bool calling = false;
		for(i = 0; i < calls.getCount(); i++)
		{
			VoiceCall *call = &calls[i];
			if (call->state == VOICE_STATE_TALKING)
				talking = call;
			else if (call->state == VOICE_STATE_CALLING)
				calling = true;
			else if (call->state == VOICE_STATE_RINGING)
				ringing = true;
		}

		TCHAR number[1024];
		GetDlgItemText(hwnd, IDC_NUMBER, number, MAX_REGS(number));
		lstrtrim(number);

		if (ringing && number[0] != 0)
		{
			SetWindowText(GetDlgItem(hwnd, IDC_NUMBER), _T(""));
			number[0] = 0;
		}

		if (ringing || calling)
		{
			for(i = 0; i < MAX_REGS(dialCtrls); ++i)
				EnableWindow(GetDlgItem(hwnd, dialCtrls[i]), FALSE);
		}
		else if (talking)
		{
			if (!talking->CanSendDTMF())
			{
				for(i = 0; i < MAX_REGS(dialCtrls); ++i)
					EnableWindow(GetDlgItem(hwnd, dialCtrls[i]), FALSE);
			}
			else if (!showDialpad)
			{
				for(i = 0; i < MAX_REGS(dialCtrls); ++i)
					EnableWindow(GetDlgItem(hwnd, dialCtrls[i]), FALSE);

				EnableWindow(GetDlgItem(hwnd, IDC_DIALPAD), TRUE);
			}
			else
			{
				for(i = 0; i < MAX_REGS(dialCtrls); ++i)
					EnableWindow(GetDlgItem(hwnd, dialCtrls[i]), TRUE);

				EnableWindow(GetDlgItem(hwnd, IDC_NUMBER), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_CALL), FALSE);
			}
		}
		else
		{
			for(i = 0; i < MAX_REGS(dialCtrls); ++i)
				EnableWindow(GetDlgItem(hwnd, dialCtrls[i]), TRUE);

			EnableWindow(GetDlgItem(hwnd, IDC_CALL), CanCall(number));
		}
	}

	InvalidateAll(hwnd);
}



static LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch(msg) 
	{
		case WM_CREATE: 
		case WM_INITDIALOG:
		{
			SendDlgItemMessage(hwnd, IDC_DIALPAD, BUTTONSETASFLATBTN, TRUE, 0);
			SendDlgItemMessage(hwnd, IDC_DIALPAD, BUTTONSETASPUSHBTN, TRUE, 0);
			SendDlgItemMessageA(hwnd, IDC_DIALPAD, BUTTONADDTOOLTIP, (LPARAM) Translate("Show dialpad"), 0);
			SendDlgItemMessage(hwnd, IDC_DIALPAD, BM_SETIMAGE, IMAGE_ICON, (LPARAM) IcoLib_LoadIcon("vc_dialpad", TRUE));

			SendDlgItemMessage(hwnd, IDC_CALL, BUTTONSETASFLATBTN, TRUE, 0);
			SendDlgItemMessageA(hwnd, IDC_CALL, BUTTONADDTOOLTIP, (LPARAM) Translate("Make call"), 0);
			SendDlgItemMessage(hwnd, IDC_CALL, BM_SETIMAGE, IMAGE_ICON, (LPARAM) IcoLib_LoadIcon("vca_call", TRUE));

			ShowHideDialpad(hwnd);

			InvalidateAll(hwnd);
			break;
		}

		case WM_DESTROY:
		{
			break;
		}

		case WM_SIZE:
		{
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);

			if (CanCallNumber())
			{
				bool showDialpad = (SendMessage(GetDlgItem(hwnd, IDC_DIALPAD), BM_GETCHECK, 0, 0) == BST_CHECKED);

				RECT rc;
				GetWindowRect(hwnd, &rc);

				RECT first = {0}, last = {0};
				GetWindowRect(GetDlgItem(hwnd, IDC_1), &first);
				GetWindowRect(GetDlgItem(hwnd, IDC_SHARP), &last);

				int dialpad_height = last.bottom - first.top;
				int dialpad_width = last.right - first.left;


				int call_height = 23;
				int call_width = 25;
				int top = height - call_height;

				if (showDialpad)
					top -= dialpad_height + 1;

				MoveWindow(GetDlgItem(hwnd, IDC_DIALPAD), 1, top, call_width -2, call_height, TRUE);
				MoveWindow(GetDlgItem(hwnd, IDC_NUMBER), call_width, top, width - 2 * call_width, call_height, TRUE);
				MoveWindow(GetDlgItem(hwnd, IDC_CALL), width - call_width, top -1, call_width, call_height +2, TRUE);

				
				int dialpad_top = top + call_height + 1;
				int dialpad_left = ((rc.right - rc.left) - dialpad_width) / 2;
				int deltaX = dialpad_left - first.left;
				int deltaY = dialpad_top - first.top;
				for(int i = 3; i < MAX_REGS(dialCtrls); ++i)
				{
					GetWindowRect(GetDlgItem(hwnd, dialCtrls[i]), &rc);
					MoveWindow(GetDlgItem(hwnd, dialCtrls[i]), rc.left + deltaX, rc.top + deltaY, 
								rc.right - rc.left, rc.bottom - rc.top, TRUE);
				}


				height -= call_height + 1;
				if (showDialpad)
					height -= dialpad_height + 1;;
			}

			if (height <= 0)
			{
				ShowWindow(GetDlgItem(hwnd, IDC_CALLS), SW_HIDE);
			}
			else
			{
				MoveWindow(GetDlgItem(hwnd, IDC_CALLS), 0, 0, width, height, TRUE);
				ShowWindow(GetDlgItem(hwnd, IDC_CALLS), SW_SHOW);
			}

			InvalidateAll(hwnd);
			break;
		}

		case WMU_REFRESH:
		{
			HWND list = GetDlgItem(hwnd, IDC_CALLS);

			SendMessage(list, LB_RESETCONTENT, 0, 0);

			for(int i = 0; i < calls.getCount(); i++)
			{
				TCHAR text[512];
				mir_sntprintf(text, MAX_REGS(text), _T("%d %s"), calls[i].state, calls[i].displayName);

				int pos = SendMessage(list, LB_ADDSTRING, 0, (LPARAM) text);
				if (pos == LB_ERR)
					// TODO Show error
					break;

				SendMessage(list, LB_SETITEMDATA, pos, (LPARAM) &calls[i]);
			}

			ShowHideDialpad(hwnd);

			// Fall throught
		}

		case WMU_RESIZE_FRAME:
		{
			if (opts.resize_frame)
			{
				if (calls.getCount() == 0 && !CanCallNumber()) 
				{
					ShowFrame(frame_id, hwnd, SW_HIDE);
				}
				else 
				{
					ResizeFrame(frame_id, hwnd);
					ShowFrame(frame_id, hwnd, SW_SHOW);
				}
			}
			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_CALL:
				{
					TCHAR number[1024];
					GetDlgItemText(hwnd, IDC_NUMBER, number, MAX_REGS(number));
					lstrtrim(number);

					for(int i = 0; i < modules.getCount(); i++)
					{
						if (!modules[i].CanCall(number))
							continue;

						modules[i].Call(NULL, number);

						break;
					}
					break;
				}
				case IDC_NUMBER:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						ShowHideDialpad(hwnd);
					}
					break;
				}
				case IDC_DIALPAD:
				{
					ShowHideDialpad(hwnd);
					SendMessage(hwnd, WMU_RESIZE_FRAME, 0, 0);
					break;
				}
				case IDC_1:
				case IDC_2:
				case IDC_3:
				case IDC_4:
				case IDC_5:
				case IDC_6:
				case IDC_7:
				case IDC_8:
				case IDC_9:
				case IDC_AST:
				case IDC_0:
				case IDC_SHARP:
				{
					TCHAR text[2];
					switch(LOWORD(wParam))
					{
						case IDC_1: text[0] = _T('1'); break;
						case IDC_2: text[0] = _T('2'); break;
						case IDC_3: text[0] = _T('3'); break;
						case IDC_4: text[0] = _T('4'); break;
						case IDC_5: text[0] = _T('5'); break;
						case IDC_6: text[0] = _T('6'); break;
						case IDC_7: text[0] = _T('7'); break;
						case IDC_8: text[0] = _T('8'); break;
						case IDC_9: text[0] = _T('9'); break;
						case IDC_AST: text[0] = _T('*'); break;
						case IDC_0: text[0] = _T('0'); break;
						case IDC_SHARP: text[0] = _T('#'); break;
					}
					text[1] = 0;

					SkinPlaySound("voice_dialpad");

					VoiceCall *call = GetTalkingCall();
					if (call == NULL)
					{
						SendMessage(GetDlgItem(hwnd, IDC_NUMBER), EM_REPLACESEL, TRUE, (LPARAM) text);
					}
					else
					{
						TCHAR tmp[1024];

						GetWindowText(GetDlgItem(hwnd, IDC_NUMBER), tmp, MAX_REGS(tmp));

						tmp[MAX_REGS(tmp)-2] = 0;
						lstrcat(tmp, text);

						SetWindowText(GetDlgItem(hwnd, IDC_NUMBER), tmp);

						call->SendDTMF(text[0]);
					}
					break;
				}
				case IDC_CALLS:
				{
					if (HIWORD(wParam) != LBN_SELCHANGE)
						break;

					HWND list = GetDlgItem(hwnd, IDC_CALLS);

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

					VoiceCall *call = (VoiceCall *) SendMessage(list, LB_GETITEMDATA, pos, 0);
					switch (call->state)
					{
						case VOICE_STATE_TALKING:
						{
							if (action == 1)
								call->Hold();
							else
								call->Drop();
							break;
						}
						case VOICE_STATE_RINGING:
						case VOICE_STATE_ON_HOLD:
						{
							if (action == 1)
								Answer(call);
							else
								call->Drop();
							break;
						}
						case VOICE_STATE_CALLING:
						{
							if (action == 2)
								call->Drop();
							break;
						}
					}
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

			if (pos >= calls.getCount())
				break;

			if (IsFinalState(calls[pos].state))
				break;

			// Just to get things strait
			SendMessage(list, LB_SETCURSEL, pos, 0);

			HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENUS));
			HMENU submenu = GetSubMenu(menu, 0);
			CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)submenu, 0);

			switch (calls[pos].state)
			{
				case VOICE_STATE_CALLING:
				{
					DeleteMenu(menu, ID_FRAMEPOPUP_ANSWERCALL, MF_BYCOMMAND);
					DeleteMenu(menu, ID_FRAMEPOPUP_HOLDCALL, MF_BYCOMMAND);
					break;
				}
				case VOICE_STATE_TALKING:
				{
					DeleteMenu(menu, ID_FRAMEPOPUP_ANSWERCALL, MF_BYCOMMAND);
					if (!calls[pos].module->CanHold())
						DeleteMenu(menu, ID_FRAMEPOPUP_HOLDCALL, MF_BYCOMMAND);
					break;
				}
			}

			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 
			int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
			DestroyMenu(menu);

			switch(ret)
			{
				case ID_FRAMEPOPUP_DROPCALL:
				{
					calls[pos].Drop();
					break;
				}
				case ID_FRAMEPOPUP_ANSWERCALL:
				{
					Answer(&calls[pos]);
					break;
				}
				case ID_FRAMEPOPUP_HOLDCALL:
				{
					calls[pos].Hold();
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

			mis->itemHeight = GetMaxLineHeight();
			
			return TRUE;
		}

		case WM_CTLCOLORLISTBOX:
		{
			return (LRESULT) bk_brush;
		}

		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;

			if (dis->CtlID != IDC_CALLS || dis->itemID == -1)
				break;

			VoiceCall *call = (VoiceCall *) dis->itemData;

			RECT rc = dis->rcItem;
			rc.left += H_SPACE;
			rc.right -= H_SPACE;
			rc.bottom --;

			int old_bk_mode = SetBkMode(dis->hDC, TRANSPARENT);

			// Draw status
			DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[call->state], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

			// TODO: Draw voice provider icon

			// Draw contact
			rc.left += ICON_SIZE + H_SPACE;
			rc.right -= 2 * (ICON_SIZE + H_SPACE);

			HFONT old_font = (HFONT) SelectObject(dis->hDC, fonts[call->state]);
			COLORREF old_color = SetTextColor(dis->hDC, font_colors[call->state]);

			DrawText(dis->hDC, call->displayName, -1, &rc, DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS | DT_VCENTER);

			SelectObject(dis->hDC, old_font);
			SetTextColor(dis->hDC, old_color);

			// Draw action icons
			rc = dis->rcItem;
			rc.right -= H_SPACE;
			rc.bottom --;

			switch (call->state)
			{
				case VOICE_STATE_CALLING:
				{
					rc.left = rc.right - ICON_SIZE;
					DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[NUM_STATES + ACTION_DROP], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
					break;
				}
				case VOICE_STATE_TALKING:
				{
					rc.left = rc.right - ICON_SIZE;
					DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[NUM_STATES + ACTION_DROP], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

					if (call->module->CanHold())
					{
						rc.right -= ICON_SIZE + H_SPACE;
						rc.left = rc.right - ICON_SIZE;
						DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[NUM_STATES + ACTION_HOLD], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
					}

					break;
				}
				case VOICE_STATE_RINGING:
				case VOICE_STATE_ON_HOLD:
				{
					rc.left = rc.right - ICON_SIZE;
					DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[NUM_STATES + ACTION_DROP], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

					rc.right -= ICON_SIZE + H_SPACE;
					rc.left = rc.right - ICON_SIZE;
					DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - 16)/2, icons[NUM_STATES + ACTION_ANSWER], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

					break;
				}
			}
			
			SetBkMode(dis->hDC, old_bk_mode);
			return TRUE;
		}

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

