#include "commons.h"


#define MIN_COLS 5
#define MAX_LINES 5
#define BORDER 5


struct EmoticonSelectionData
{
	Module *module;
	COLORREF background;
	int max_height;
	int max_width;
	int lines;
	int cols;
	int selection;

    int xPosition;
    int yPosition;
    int Direction;
    HWND hwndTarget;
    UINT targetMessage;
    LPARAM targetWParam;

	void SetSelection(HWND hwnd, int sel)
	{
		if (sel < 0)
			sel = -1;
		if (sel >= module->emoticons.getCount())
			sel = -1;
		if (sel != selection)
			InvalidateRect(hwnd, NULL, FALSE);
		selection = sel;
	}
};


HBITMAP CreateBitmap32(int cx, int cy)
{
   BITMAPINFO RGB32BitsBITMAPINFO; 
    UINT * ptPixels;
    HBITMAP DirectBitmap;

    ZeroMemory(&RGB32BitsBITMAPINFO,sizeof(BITMAPINFO));
    RGB32BitsBITMAPINFO.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    RGB32BitsBITMAPINFO.bmiHeader.biWidth=cx;//bm.bmWidth;
    RGB32BitsBITMAPINFO.bmiHeader.biHeight=cy;//bm.bmHeight;
    RGB32BitsBITMAPINFO.bmiHeader.biPlanes=1;
    RGB32BitsBITMAPINFO.bmiHeader.biBitCount=32;

    DirectBitmap = CreateDIBSection(NULL, 
                                    (BITMAPINFO *)&RGB32BitsBITMAPINFO, 
                                    DIB_RGB_COLORS,
                                    (void **)&ptPixels, 
                                    NULL, 0);
    return DirectBitmap;
}


HWND CreateTooltip(HWND hwnd, RECT &rect, TCHAR *text)
{
          // struct specifying control classes to register
    INITCOMMONCONTROLSEX iccex; 
    HWND hwndTT;                 // handle to the ToolTip control
          // struct specifying info about tool in ToolTip control
    TOOLINFO ti;
    unsigned int uid = 0;       // for ti initialization

	// Load the ToolTip class from the DLL.
    iccex.dwSize = sizeof(iccex);
    iccex.dwICC  = ICC_BAR_CLASSES;

    if(!InitCommonControlsEx(&iccex))
       return NULL;

    /* CREATE A TOOLTIP WINDOW */
    hwndTT = CreateWindowEx(WS_EX_TOPMOST,
        TOOLTIPS_CLASS,
        NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hwnd,
        NULL,
        hInst,
        NULL
        );

	/* Gives problem with mToolTip
    SetWindowPos(hwndTT,
        HWND_TOPMOST,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	*/

    /* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hwnd;
    ti.hinst = hInst;
    ti.uId = uid;
    ti.lpszText = text;
        // ToolTip control will cover the whole window
    ti.rect.left = rect.left;    
    ti.rect.top = rect.top;
    ti.rect.right = rect.right;
    ti.rect.bottom = rect.bottom;

    /* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
    SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
	SendMessage(hwndTT, TTM_SETDELAYTIME, (WPARAM) (DWORD) TTDT_AUTOPOP, (LPARAM) MAKELONG(24 * 60 * 60 * 1000, 0));	

	return hwndTT;
} 


void AssertInsideScreen(RECT &rc)
{
	// Make sure it is inside screen
	if (IsWinVer98Plus()) {
		static BOOL loaded = FALSE;
		static HMONITOR (WINAPI *MyMonitorFromRect)(LPCRECT,DWORD) = NULL;
		static BOOL (WINAPI *MyGetMonitorInfo)(HMONITOR,LPMONITORINFO) = NULL;

		if (!loaded) {
			HMODULE hUser32 = GetModuleHandleA("user32");
			if (hUser32) {
				MyMonitorFromRect = (HMONITOR(WINAPI*)(LPCRECT,DWORD))GetProcAddress(hUser32,"MonitorFromRect");
				MyGetMonitorInfo = (BOOL(WINAPI*)(HMONITOR,LPMONITORINFO))GetProcAddress(hUser32,"GetMonitorInfoA");
				if (MyGetMonitorInfo == NULL)
					MyGetMonitorInfo = (BOOL(WINAPI*)(HMONITOR,LPMONITORINFO))GetProcAddress(hUser32,"GetMonitorInfo");
			}
			loaded = TRUE;
		}

		if (MyMonitorFromRect != NULL && MyGetMonitorInfo != NULL) {
			HMONITOR hMonitor;
			MONITORINFO mi;

			hMonitor = MyMonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
			mi.cbSize = sizeof(mi);
			MyGetMonitorInfo(hMonitor, &mi);

			if (rc.bottom > mi.rcWork.bottom)
				OffsetRect(&rc, 0, mi.rcWork.bottom - rc.bottom);
			if (rc.bottom < mi.rcWork.top)
				OffsetRect(&rc, 0, mi.rcWork.top - rc.top);
			if (rc.top > mi.rcWork.bottom)
				OffsetRect(&rc, 0, mi.rcWork.bottom - rc.bottom);
			if (rc.top < mi.rcWork.top)
				OffsetRect(&rc, 0, mi.rcWork.top - rc.top);
			if (rc.right > mi.rcWork.right)
				OffsetRect(&rc, mi.rcWork.right - rc.right, 0);
			if (rc.right < mi.rcWork.left)
				OffsetRect(&rc, mi.rcWork.left - rc.left, 0);
			if (rc.left > mi.rcWork.right)
				OffsetRect(&rc, mi.rcWork.right - rc.right, 0);
			if (rc.left < mi.rcWork.left)
				OffsetRect(&rc, mi.rcWork.left - rc.left, 0);
		}
	}
}


INT_PTR CALLBACK EmoticonSeletionDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch(msg) 
	{
		case WM_INITDIALOG: 
		{
			EmoticonSelectionData *ssd = (EmoticonSelectionData *) lParam;
			SetWindowLong(hwnd, GWL_USERDATA, (LONG) ssd);

			ssd->selection = -1;

			// Load emoticons
			ssd->max_height = 4;
			ssd->max_width = 4;

			HDC hdc = GetDC(hwnd);

			int num_emotes = ssd->module->emoticons.getCount();
			int i;
			for(i = 0; i < num_emotes; i++)
			{
				Emoticon *e = ssd->module->emoticons[i];
				if (e->img != NULL)
					e->img->Load(ssd->max_height, ssd->max_width);

				if (e->img == NULL || e->img->img == NULL)
				{
					HFONT hFont;
					if (ssd->hwndTarget != NULL)
					{
						CHARFORMAT2 cf;
						ZeroMemory(&cf, sizeof(cf));
						cf.cbSize = sizeof(cf);
						cf.dwMask = CFM_FACE | CFM_ITALIC | CFM_CHARSET | CFM_FACE | CFM_WEIGHT | CFM_SIZE;
						SendMessage(ssd->hwndTarget, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);

						LOGFONT lf = {0};
						lf.lfHeight = -MulDiv(cf.yHeight / 20, GetDeviceCaps(hdc, LOGPIXELSY), 72);
						lf.lfWeight = cf.wWeight;
						lf.lfItalic = (cf.dwEffects & CFE_ITALIC) == CFE_ITALIC;
						lf.lfCharSet = cf.bCharSet;
						lf.lfPitchAndFamily = cf.bPitchAndFamily;
						lstrcpyn(lf.lfFaceName, cf.szFaceName, MAX_REGS(lf.lfFaceName));

						hFont = CreateFontIndirect(&lf);
					}
					else
						hFont = (HFONT) SendMessage(hwnd, WM_GETFONT, 0, 0);

					if (hFont != NULL)
						SelectObject(hdc, hFont);

					RECT rc = { 0, 0, 0xFFFF, 0xFFFF };
					DrawText(hdc, e->texts[0], lstrlen(e->texts[0]), &rc, DT_CALCRECT | DT_NOPREFIX);

					ssd->max_height = max(ssd->max_height, rc.bottom - rc.top + 1);
					ssd->max_width = max(ssd->max_width, rc.right - rc.left + 1);

					if (ssd->hwndTarget != NULL)
						DeleteObject(hFont);
				}
			}

			ReleaseDC(hwnd, hdc);

			ssd->cols = num_emotes / MAX_LINES;
			if (num_emotes % MAX_LINES != 0)
				ssd->cols++;
			ssd->cols = max(ssd->cols, MIN_COLS);

			ssd->lines = num_emotes / ssd->cols;
			if (num_emotes % ssd->cols != 0)
				ssd->lines++;

			// Calc position
			int width = ssd->max_width * ssd->cols + (ssd->cols + 1) * BORDER + 1;
			int height = ssd->max_height * ssd->lines + (ssd->lines + 1) * BORDER + 1;

			int x = ssd->xPosition;
			int y = ssd->yPosition;
			switch (ssd->Direction) 
			{
				case 1: 
					x -= width;
					break;
				case 2:
					x -= width;
					y -= height;
					break;
				case 3:
					y -= height;
					break;
			}

			// Get background
			ssd->background = RGB(255, 255, 255);
			if (ssd->hwndTarget != NULL)
			{
				ssd->background = SendMessage(ssd->hwndTarget, EM_SETBKGNDCOLOR, 0, ssd->background);
				SendMessage(ssd->hwndTarget, EM_SETBKGNDCOLOR, 0, ssd->background);
			}

			RECT rc = { x, y, x + width, y + height };
			AssertInsideScreen(rc);
			SetWindowPos(hwnd, HWND_TOPMOST, rc.left, rc.top, width, height, 0);

			for(i = 0; i < ssd->lines; i++)
			{
				for(int j = 0; j < ssd->cols; j++)
				{
					int index = i * ssd->cols + j;
					if (index >= ssd->module->emoticons.getCount())
						break;
					
					Emoticon *e = ssd->module->emoticons[index];

					RECT fr;
					fr.left = BORDER + j * (ssd->max_width + BORDER) - 1;
					fr.right = fr.left + ssd->max_width + 2;
					fr.top = BORDER + i * (ssd->max_height + BORDER) - 1;
					fr.bottom = fr.top + ssd->max_height + 2;

					Buffer<TCHAR> tt;
					if (e->description[0] != _T('\0'))
					{
						tt += _T(" ");
						tt += e->description;
						tt.translate();
						tt += _T(" ");
					}

					for(int k = 0; k < e->texts.getCount(); k++)
					{
						tt += _T(" ");
						tt += e->texts[k];
						tt += _T(" ");
					}
					tt.pack();

					e->tt = CreateTooltip(hwnd, fr, tt.str);
				}
			}

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER | TME_LEAVE;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);

			return TRUE;
		}

		case WM_PAINT:
		{
			RECT r;
			if (GetUpdateRect(hwnd, &r, FALSE)) 
			{
				EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

				PAINTSTRUCT ps;

				HDC hdc_orig = BeginPaint(hwnd, &ps);

				RECT rc;
				GetClientRect(hwnd, &rc);

				// Create double buffer
				HDC hdc = CreateCompatibleDC(hdc_orig);
				HBITMAP hBmp = CreateBitmap32(rc.right, rc.bottom);
				SelectObject(hdc, hBmp);

				SetBkMode(hdc, TRANSPARENT);

				// Erase background
				HBRUSH hB = CreateSolidBrush(ssd->background);
				FillRect(hdc, &rc, hB);
				DeleteObject(hB);

				// Draw emoticons
				for(int i = 0; i < ssd->lines; i++)
				{
					for(int j = 0; j < ssd->cols; j++)
					{
						int index = i * ssd->cols + j;
						if (index >= ssd->module->emoticons.getCount())
							break;
						
						Emoticon *e = ssd->module->emoticons[index];
						if (e->img == NULL || e->img->img == NULL)
						{
							HFONT hFont;
							if (ssd->hwndTarget != NULL)
							{
								CHARFORMAT2 cf;
								ZeroMemory(&cf, sizeof(cf));
								cf.cbSize = sizeof(cf);
								cf.dwMask = CFM_FACE | CFM_ITALIC | CFM_CHARSET | CFM_FACE | CFM_WEIGHT | CFM_SIZE | CFM_COLOR;
								SendMessage(ssd->hwndTarget, EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM) &cf);

								LOGFONT lf = {0};
								lf.lfHeight = -MulDiv(cf.yHeight / 20, GetDeviceCaps(hdc, LOGPIXELSY), 72);
								lf.lfWeight = cf.wWeight;
								lf.lfItalic = (cf.dwEffects & CFE_ITALIC) == CFE_ITALIC;
								lf.lfCharSet = cf.bCharSet;
								lf.lfPitchAndFamily = cf.bPitchAndFamily;
								lstrcpyn(lf.lfFaceName, cf.szFaceName, MAX_REGS(lf.lfFaceName));

								hFont = CreateFontIndirect(&lf);
								SetTextColor(hdc, cf.crTextColor);
							}
							else
								hFont = (HFONT) SendMessage(hwnd, WM_GETFONT, 0, 0);

							if (hFont != NULL)
								SelectObject(hdc, hFont);

							RECT rc = { 0, 0, 0xFFFF, 0xFFFF };
							DrawText(hdc, e->texts[0], lstrlen(e->texts[0]), &rc, DT_CALCRECT | DT_NOPREFIX);

							int height = rc.bottom - rc.top + 1;
							int width = rc.right - rc.left + 1;

							rc.left = BORDER + j * (ssd->max_width + BORDER) + (ssd->max_width - width) / 2;
							rc.top = BORDER + i * (ssd->max_height + BORDER) + (ssd->max_height - height) / 2;

							rc.right = rc.left + width;
							rc.bottom = rc.top + height;

							DrawText(hdc, e->texts[0], lstrlen(e->texts[0]), &rc, DT_NOPREFIX);

							if (ssd->hwndTarget != NULL)
								DeleteObject(hFont);
						}
						else
						{
							BITMAP bmp;
							GetObject(e->img->img, sizeof(bmp), &bmp);

							int x = BORDER + j * (ssd->max_width + BORDER) + (ssd->max_width - bmp.bmWidth) / 2;
							int y = BORDER + i * (ssd->max_height + BORDER) + (ssd->max_height - bmp.bmHeight) / 2;

							HDC hdc_img = CreateCompatibleDC(hdc);
							HBITMAP old_bmp = (HBITMAP) SelectObject(hdc_img, e->img->img);

							if (e->img->transparent)
							{
								BLENDFUNCTION bf = {0};
								bf.SourceConstantAlpha = 255;
								bf.AlphaFormat = AC_SRC_ALPHA;
								AlphaBlend(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hdc_img, 0, 0, bmp.bmWidth, bmp.bmHeight, bf);
							}
							else
							{
								BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hdc_img, 0, 0, SRCCOPY);
							}

							SelectObject(hdc_img, old_bmp);
							DeleteDC(hdc_img);

						}

						if (ssd->selection == index)
						{
							RECT fr;
							fr.left = BORDER + j * (ssd->max_width + BORDER) - 1;
							fr.right = fr.left + ssd->max_width + 2;
							fr.top = BORDER + i * (ssd->max_height + BORDER) - 1;
							fr.bottom = fr.top + ssd->max_height + 2;
							FrameRect(hdc, &fr, (HBRUSH) GetStockObject(GRAY_BRUSH));
						}
					}
				}

				// Copy buffer to screen
				BitBlt(hdc_orig, rc.left, rc.top, rc.right - rc.left, 
						rc.bottom - rc.top, hdc, rc.left, rc.top, SRCCOPY);
				DeleteDC(hdc);
				DeleteObject(hBmp);

				EndPaint(hwnd, &ps);
			}
			
			return TRUE;
		}

		case WM_MOUSELEAVE:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
		}
		case WM_NCMOUSEMOVE:
		{
			EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);
			ssd->SetSelection(hwnd, -1);
			break;
		}

		case WM_MOUSEHOVER:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
		}
		case WM_MOUSEMOVE:
		{
			EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

			POINT p;
			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 

			int col;
			if (p.x % (BORDER + ssd->max_width) < BORDER)
				col = -1;
			else
				col = p.x / (BORDER + ssd->max_width);

			int line;
			if (p.y % (BORDER + ssd->max_height) < BORDER)
				line = -1;
			else
				line = p.y / (BORDER + ssd->max_height);

			int index = line * ssd->cols + col;

			if (col >= 0 && line >= 0 && index < ssd->module->emoticons.getCount())
			{
				ssd->SetSelection(hwnd, index);
			}
			else
			{
				ssd->SetSelection(hwnd, -1);
			}

			break;
		}

		case WM_GETDLGCODE:
		{
			if (lParam != NULL)
			{
				static DWORD last_time = 0;

				MSG *msg = (MSG* ) lParam;
				if (msg->message == WM_KEYDOWN && msg->time != last_time)
				{
					last_time = msg->time;

					if (msg->wParam == VK_UP)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

						if (ssd->selection < 0)
						{
							ssd->SetSelection(hwnd, (ssd->lines - 1) * ssd->cols);
						}
						else
						{
							int index = (ssd->selection - ssd->cols) % ssd->module->emoticons.getCount();
							if (index < 0)
								index += ssd->module->emoticons.getCount();
							ssd->SetSelection(hwnd, index);
						}
					}
					else if (msg->wParam == VK_DOWN)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

						if (ssd->selection < 0)
						{
							ssd->SetSelection(hwnd, 0);
						}
						else
						{
							ssd->SetSelection(hwnd, (ssd->selection + ssd->cols) % ssd->module->emoticons.getCount());
						}
					}
					else if (msg->wParam == VK_LEFT)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

						if (ssd->selection < 0)
						{
							ssd->SetSelection(hwnd, ssd->cols - 1);
						}
						else
						{
							int index = (ssd->selection - 1) % ssd->module->emoticons.getCount();
							if (index < 0)
								index += ssd->module->emoticons.getCount();
							ssd->SetSelection(hwnd, index);
						}
					}
					else if (msg->wParam == VK_RIGHT)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);

						if (ssd->selection < 0)
						{
							ssd->SetSelection(hwnd, 0);
						}
						else
						{
							ssd->SetSelection(hwnd, (ssd->selection + 1) % ssd->module->emoticons.getCount());
						}
					}
					else if (msg->wParam == VK_HOME)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);
						ssd->SetSelection(hwnd, 0);
					}
					else if (msg->wParam == VK_END)
					{
						EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);
						ssd->SetSelection(hwnd, ssd->module->emoticons.getCount() - 1);
					}
				}
			}

			return DLGC_WANTALLKEYS;
		}
		
	    case WM_ACTIVATE:
		{
			if (wParam == WA_INACTIVE) 
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDOK:
					PostMessage(hwnd, WM_LBUTTONUP, 0, 0);
					break;

				case IDCANCEL:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
			}
			break;
		}

		case WM_LBUTTONUP:
		{
			EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);
			if (ssd->selection >= 0 && ssd->hwndTarget != NULL)
			{
				if (opts.only_replace_isolated)
				{
					TCHAR tmp[16];
					mir_sntprintf(tmp, MAX_REGS(tmp), _T(" %s "), ssd->module->emoticons[ssd->selection]->texts[0]);
					SendMessage(ssd->hwndTarget, ssd->targetMessage, ssd->targetWParam, (LPARAM) tmp);
				}
				else
					SendMessage(ssd->hwndTarget, ssd->targetMessage, ssd->targetWParam, (LPARAM) ssd->module->emoticons[ssd->selection]->texts[0]);
			}

			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		}

		case WM_CLOSE:
		{
			EmoticonSelectionData *ssd = (EmoticonSelectionData *) GetWindowLong(hwnd, GWL_USERDATA);
			SetWindowLong(hwnd, GWL_USERDATA, NULL);

			for(int i = 0; i < ssd->module->emoticons.getCount(); i++)
			{
				Emoticon *e = ssd->module->emoticons[i];

				if (e->tt != NULL)
				{
					DestroyWindow(e->tt);
					e->tt = NULL;
				}
			}

			DestroyWindow(hwnd);

			SetFocus(ssd->hwndTarget);
			delete ssd;
			break;
		}
	}

	return FALSE;
}

int ShowSelectionService(WPARAM wParam, LPARAM lParam)
{
    SMADD_SHOWSEL3 *sss = (SMADD_SHOWSEL3 *)lParam;
	if (sss == NULL || sss->cbSize < sizeof(SMADD_SHOWSEL3)) 
		return FALSE;

	const char *proto = NULL;
	if (sss->hContact != NULL)
	{
		HANDLE hReal = GetRealContact(sss->hContact);
		proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hReal, 0);
	}
	if (proto == NULL)
		proto = sss->Protocolname;
	if (proto == NULL)
		return FALSE;

	Module *m = GetModule(proto);
	if (m == NULL)
		return FALSE;

	EmoticonSelectionData * ssd = new EmoticonSelectionData();
	ssd->module = m;

	ssd->xPosition = sss->xPosition;
	ssd->yPosition = sss->yPosition;
	ssd->Direction = sss->Direction;

	ssd->hwndTarget = sss->hwndTarget;
	ssd->targetMessage = sss->targetMessage;
	ssd->targetWParam = sss->targetWParam;

	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EMOTICON_SELECTION), sss->hwndParent, 
					  EmoticonSeletionDlgProc, (LPARAM) ssd);

    return TRUE;
}

