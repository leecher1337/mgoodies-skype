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


#include "commons.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;

static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void GetTemplare(Buffer<TCHAR> *buffer, HISTORY_EVENT_HANDLER *heh, int templates);




// Functions //////////////////////////////////////////////////////////////////////////////////////


DWORD GetSettingDword(HISTORY_EVENT_HANDLER *heh, char *setting, DWORD def)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%s", (int) heh->eventType, setting);
	return DBGetContactSettingDword(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, def);
}

WORD GetSettingWord(HISTORY_EVENT_HANDLER *heh, char *setting, WORD def)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%s", (int) heh->eventType, setting);
	return DBGetContactSettingWord(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, def);
}

BYTE GetSettingByte(HISTORY_EVENT_HANDLER *heh, char *setting, BYTE def)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%s", (int) heh->eventType, setting);
	return DBGetContactSettingByte(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, def);
}

BOOL GetSettingBool(HISTORY_EVENT_HANDLER *heh, char *setting, BOOL def)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%s", (int) heh->eventType, setting);
	return DBGetContactSettingByte(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, def) != 0;
}

BOOL GetSettingBool(HISTORY_EVENT_HANDLER *heh, int templ, char *setting, BOOL def)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%d_%s", (int) heh->eventType, templ, setting);
	return DBGetContactSettingByte(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, def) != 0;
}


void WriteSettingDword(HISTORY_EVENT_HANDLER *heh, char *setting, DWORD val)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%s", (int) heh->eventType, setting);
	DBWriteContactSettingDword(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, val);
}

void WriteSettingWord(HISTORY_EVENT_HANDLER *heh, char *setting, WORD val)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%s", (int) heh->eventType, setting);
	DBWriteContactSettingWord(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, val);
}

void WriteSettingByte(HISTORY_EVENT_HANDLER *heh, char *setting, BYTE val)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%s", (int) heh->eventType, setting);
	DBWriteContactSettingByte(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, val);
}

void WriteSettingBool(HISTORY_EVENT_HANDLER *heh, char *setting, BOOL val)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%s", (int) heh->eventType, setting);
	DBWriteContactSettingByte(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, val ? 1 : 0);
}

void WriteSettingBool(HISTORY_EVENT_HANDLER *heh, int templ, char *setting, BOOL val)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%d_%s", (int) heh->eventType, templ, setting);
	DBWriteContactSettingByte(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, val ? 1 : 0);
}

void WriteSettingTString(HISTORY_EVENT_HANDLER *heh, int templ, char *setting, TCHAR *str)
{
	char tmp[128];
	mir_snprintf(tmp, MAX_REGS(tmp), "%d_%d_%s", (int) heh->eventType, templ, setting);
	DBWriteContactSettingTString(NULL, heh->module == NULL ? MODULE_NAME : heh->module, tmp, str);
}


int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
    odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.ptszGroup = TranslateT("History");
	odp.ptszTitle = TranslateT("Events");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	return 0;
}


void LoadOptions()
{
}


void InitOptions()
{
	LoadOptions();

	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}


void DeInitOptions()
{
	UnhookEvent(hOptHook);
}


BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	BOOL ret;

	POINT pt;

	pt.x = lpRect->left;
	pt.y = lpRect->top;

	ret = ScreenToClient(hWnd, &pt);

	if (!ret) return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;


	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}


static void GetTextMetric(HFONT hFont, TEXTMETRIC *tm)
{
	HDC hdc = GetDC(NULL);
	HFONT hOldFont = (HFONT) SelectObject(hdc, hFont);
	GetTextMetrics(hdc, tm);
	SelectObject(hdc, hOldFont);
	ReleaseDC(NULL, hdc);
}


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	static int avaiable = 0;
	static int total = 0;
	static int current = 0;
	static int lineHeigth = 0;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			RECT rc;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_EVENT_TYPES), &rc);

			POINT pt = { rc.left, rc.bottom + 5 };
			ScreenToClient(hwndDlg, &pt);
			int origY = pt.y;

			GetClientRect(hwndDlg, &rc);

			HFONT hFont = (HFONT) SendMessage(hwndDlg, WM_GETFONT, 0, 0);
			TEXTMETRIC font;
			GetTextMetric(hFont, &font);

			int height = max(font.tmHeight, 16) + 4;
			int width = rc.right - rc.left - 50;

			lineHeigth = height;

			// Create all items
			int id = IDC_EVENT_TYPES + 1;
			for (int k = 0; k < handlers.getCount(); k++)
			{
				HISTORY_EVENT_HANDLER *heh = handlers[k];

				int x = pt.x;

				// Event type

				HWND icon = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE, 
                        x, pt.y + (height - 16) / 2, 16, 16, hwndDlg, NULL, hInst, NULL);
				x += 20;

				SendMessage(icon, STM_SETICON, (WPARAM) LoadIconEx(heh, TRUE), 0);

				HWND tmp = CreateWindowA("STATIC", heh->description, WS_CHILD | WS_VISIBLE, 
                        x, pt.y + (height - font.tmHeight) / 2, width - (x - pt.x), font.tmHeight, 
						hwndDlg, NULL, hInst, NULL);
				SendMessage(tmp, WM_SETFONT, (WPARAM) hFont, FALSE);

				// Show in SRMM

/*				if (!(heh->flags & HISTORYEVENTS_FLAG_DEFAULT))
				{
					pt.y += height + 3;
					x = pt.x + 20;

					HWND chk = CreateWindow(_T("BUTTON"), TranslateT("Show in message window"), 
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX, 
						    x, pt.y, width - (x - pt.x), height, hwndDlg, (HMENU) id, hInst, NULL);
					SendMessage(chk, BM_SETCHECK, heh->flags & HISTORYEVENTS_FLAG_SHOW_IM_SRMM ? BST_CHECKED : BST_UNCHECKED, 0);
					SendMessage(chk, WM_SETFONT, (WPARAM) hFont, FALSE);
				}
*/				id++;

				// Message settings

/*				if (heh->eventType == EVENTTYPE_MESSAGE)
				{
					pt.y += height + 3;
					x = pt.x + 20;

					HWND chk = CreateWindow(_T("BUTTON"), TranslateT("Respect text format"), 
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX, 
						    x, pt.y, width - (x - pt.x), height, hwndDlg, (HMENU) id, hInst, NULL);
					SendMessage(chk, WM_SETFONT, (WPARAM) hFont, FALSE);
					SendMessage(chk, BM_SETCHECK, GetSettingBool(heh, RESPECT_TEXT_FORMAT, TRUE) ? BST_CHECKED : BST_UNCHECKED, 0);

					pt.y += height + 3;

					chk = CreateWindow(_T("BUTTON"), TranslateT("Respect text font"), 
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX, 
						    x, pt.y, width - (x - pt.x), height, hwndDlg, (HMENU) (id+1), hInst, NULL);
					SendMessage(chk, WM_SETFONT, (WPARAM) hFont, FALSE);
					SendMessage(chk, BM_SETCHECK, GetSettingBool(heh, RESPECT_TEXT_FONT, FALSE) ? BST_CHECKED : BST_UNCHECKED, 0);
				}
*/				id+=2;

				// Only if SRMM open

				if (!(heh->flags & HISTORYEVENTS_FLAG_DEFAULT))
				{
					pt.y += height + 3;
					x = pt.x + 20;

					HWND chk = CreateWindow(_T("BUTTON"), TranslateT("Only add if message window is open"), 
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX, 
						    x, pt.y, width - (x - pt.x), height, hwndDlg, (HMENU) id, hInst, NULL);
					SendMessage(chk, BM_SETCHECK, heh->flags & HISTORYEVENTS_FLAG_ONLY_LOG_IF_SRMM_OPEN ? BST_CHECKED : BST_UNCHECKED, 0);
					SendMessage(chk, WM_SETFONT, (WPARAM) hFont, FALSE);
				}
				id++;

				// Templates

				Buffer<char> name;
				Buffer<TCHAR> templ;
				for(int i = 0; i < heh->numTemplates; i++)
				{
					pt.y += height + 3;
					x = pt.x + 20;

					name.clear();
					char *end = strchr(heh->templates[i], '\n');
					size_t len = (end == NULL ? strlen(heh->templates[i]) : end - heh->templates[i]);
					name.append(heh->templates[i], len);
					name.translate();
					name.append(':');
					name.pack();

					HWND chk = CreateWindowA("BUTTON", name.str, 
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX, 
						    x, pt.y, 120, height, hwndDlg, (HMENU) (id + 2 * i + 1), hInst, NULL);
					SendMessage(chk, WM_SETFONT, (WPARAM) hFont, FALSE);
					SendMessage(chk, BM_SETCHECK, GetSettingBool(heh, i, TEMPLATE_ENABLED, TRUE) ? BST_CHECKED : BST_UNCHECKED, 0);
					x += 120;

					templ.clear();
					GetTemplare(&templ, heh, i);
					templ.pack();

					HWND edit = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), templ.str, 
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, 
						    x, pt.y, width - (x - pt.x), height, hwndDlg, (HMENU) (id + 2 * i + 2), hInst, NULL);
					SendMessage(edit, WM_SETFONT, (WPARAM) hFont, FALSE);
				}

				// Keep

				pt.y += height + 3;
				x = pt.x + 36;

				tmp = CreateWindow(_T("STATIC"), TranslateT("Keep in database:"), WS_CHILD | WS_VISIBLE, 
                        x, pt.y + (height - font.tmHeight) / 2, 104, font.tmHeight, 
						hwndDlg, NULL, hInst, NULL);
				SendMessage(tmp, WM_SETFONT, (WPARAM) hFont, FALSE);
				x += 104;

				HWND combo = CreateWindow(_T("COMBOBOX"), _T(""), 
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, 
                        x, pt.y, 200, height + 60, 
						hwndDlg, (HMENU) id, hInst, NULL);
				SendMessage(combo, WM_SETFONT, (WPARAM) hFont, FALSE);

				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM) TranslateT("Forever"));
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM) TranslateT("For 1 year"));
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM) TranslateT("For 6 months"));
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM) TranslateT("For 1 month"));
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM) TranslateT("For 1 week"));
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM) TranslateT("For 1 day"));
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM) TranslateT("Only to view in message window"));
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM) TranslateT("Max 10 events"));
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM) TranslateT("Max 100 events"));

				if (!(heh->flags & HISTORYEVENTS_FLAG_DEFAULT))
					SendMessage(combo, CB_ADDSTRING, 0, (LPARAM) TranslateT("Don't store"));

				SendMessage(combo, CB_SETCURSEL, KEEP_FLAG_TO_COMBO(heh->flags), 0);

				pt.y += height + 10;
				id += 21;
			}

			avaiable = rc.bottom - rc.top;
			total = pt.y - 7;
			current = 0;

			SCROLLINFO si; 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
			si.nMin   = 0; 
			si.nMax   = total; 
			si.nPage  = avaiable; 
			si.nPos   = current; 
			SetScrollInfo(hwndDlg, SB_VERT, &si, TRUE); 

			break;
		}

		case WM_VSCROLL: 
		{ 
			int yDelta;     // yDelta = new_pos - current_pos 
			int yNewPos;    // new position 
 
			switch (LOWORD(wParam)) 
			{ 
				case SB_PAGEUP: 
					yNewPos = current - avaiable / 2; 
					break;  
				case SB_PAGEDOWN: 
					yNewPos = current + avaiable / 2; 
					break; 
				case SB_LINEUP: 
					yNewPos = current - lineHeigth; 
					break; 
				case SB_LINEDOWN: 
					yNewPos = current + lineHeigth; 
					break; 
				case SB_THUMBPOSITION: 
					yNewPos = HIWORD(wParam); 
					break; 
				case SB_THUMBTRACK:
					yNewPos = HIWORD(wParam); 
					break;
				default: 
					yNewPos = current; 
			} 

			yNewPos = min(total - avaiable, max(0, yNewPos)); 
 
			if (yNewPos == current) 
				break; 
 
			yDelta = yNewPos - current; 
			current = yNewPos; 
 
			// Scroll the window. (The system repaints most of the 
			// client area when ScrollWindowEx is called; however, it is 
			// necessary to call UpdateWindow in order to repaint the 
			// rectangle of pixels that were invalidated.) 
 
			ScrollWindowEx(hwndDlg, 0, -yDelta, (CONST RECT *) NULL, 
				(CONST RECT *) NULL, (HRGN) NULL, (LPRECT) NULL, 
				SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN); 
			UpdateWindow(hwndDlg); 
 
			// Reset the scroll bar. 
 
			SCROLLINFO si; 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_POS; 
			si.nPos   = current; 
			SetScrollInfo(hwndDlg, SB_VERT, &si, TRUE); 

			break; 
		}

		case WM_COMMAND:
		{
			if ((HWND) lParam != GetFocus())
				break;

			int id = (LOWORD(wParam) - IDC_EVENT_TYPES - 1) % 25;
			if (id == 0 || id == 1 || id == 2 || id == 3 || (id >= 5 && (id - 5) % 2 == 0))
			{
				// Checkboxes
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			else if (id == 4)
			{
				// Combo
				if (HIWORD(wParam) == CBN_SELCHANGE)
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			else if (id >= 5 && (id - 5) % 2 == 1)
			{
				if (HIWORD(wParam) == EN_CHANGE)
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			if (lpnmhdr->idFrom != 0 || lpnmhdr->code != PSN_APPLY)
				break;

			int id = IDC_EVENT_TYPES + 1;
			for (int k = 0; k < handlers.getCount(); k++)
			{
				HISTORY_EVENT_HANDLER *heh = handlers[k];

				// Show in SRMM

/*				if (!(heh->flags & HISTORYEVENTS_FLAG_DEFAULT))
				{
					if (IsDlgButtonChecked(hwndDlg, id))
						heh->flags |= HISTORYEVENTS_FLAG_SHOW_IM_SRMM;
					else
						heh->flags &= ~HISTORYEVENTS_FLAG_SHOW_IM_SRMM;
				}
*/				id++;

				// Message settings

/*				if (heh->eventType == EVENTTYPE_MESSAGE)
				{
					WriteSettingBool(heh, RESPECT_TEXT_FORMAT, IsDlgButtonChecked(hwndDlg, id));
					WriteSettingBool(heh, RESPECT_TEXT_FONT, IsDlgButtonChecked(hwndDlg, id+1));
				}
*/				id+=2;

				// Show in SRMM

				if (!(heh->flags & HISTORYEVENTS_FLAG_DEFAULT))
				{
					if (IsDlgButtonChecked(hwndDlg, id))
						heh->flags |= HISTORYEVENTS_FLAG_ONLY_LOG_IF_SRMM_OPEN;
					else
						heh->flags &= ~HISTORYEVENTS_FLAG_ONLY_LOG_IF_SRMM_OPEN;
				}
				id++;

				// Templates

				Buffer<char> name;
				Buffer<TCHAR> templ;
				for(int i = 0; i < heh->numTemplates; i++)
				{
					WriteSettingBool(heh, i, TEMPLATE_ENABLED, IsDlgButtonChecked(hwndDlg, id + 2 * i + 1));

					TCHAR tmp[1024];
					GetDlgItemText(hwndDlg, id + 2 * i + 2, tmp, 1024);
					WriteSettingTString(heh, i, TEMPLATE_TEXT, tmp);
				}

				// Keep

				heh->flags -= KEEP_FLAG(heh->flags);
				heh->flags |= KEEP_COMBO_TO_FLAG(SendDlgItemMessage(hwndDlg, id, CB_GETCURSEL, 0, 0));
				WriteSettingDword(heh, FLAGS, heh->flags);

				id += 21;
			}

			return TRUE;
		}

	}

	return 0;
}

