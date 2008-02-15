/* 
Copyright (C) 2008 Ricardo Pescuma Domenecci

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

#include "options.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;

Options opts;


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ &opts.replace_in_input,		CONTROL_CHECKBOX,	IDC_INPUT_TOO,			"ReplaceInInput", TRUE },
	{ &opts.use_default_pack,		CONTROL_CHECKBOX,	IDC_USE_DEFAULT_PACK,	"UseDefaultPack", TRUE },
	{ &opts.only_replace_isolated,	CONTROL_CHECKBOX,	IDC_ONLY_ISOLATED,		"OnlyReplaceIsolatedEmoticons", FALSE },
};

static UINT optionsExpertControls[] = { 
	IDC_INPUT_TOO, IDC_USE_DEFAULT_PACK, IDC_ONLY_ISOLATED
};


// Functions //////////////////////////////////////////////////////////////////////////////////////


int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=0;
	odp.hInstance=hInst;
	odp.ptszGroup = TranslateT("Message Sessions");
	odp.ptszTitle = TranslateT("Emoticons");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
    odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.expertOnlyControls = optionsExpertControls;
	odp.nExpertOnlyControls = MAX_REGS(optionsExpertControls);
    CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
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


void LoadOptions()
{
	LoadOpts(optionsControls, MAX_REGS(optionsControls), MODULE_NAME);

	opts.pack[0] = '\0';

	DBVARIANT dbv;
	if (!DBGetContactSettingString(NULL, MODULE_NAME, "DefaultPack", &dbv))
	{
		strncpy(opts.pack, dbv.pszVal, MAX_REGS(opts.pack)-1);
		opts.pack[MAX_REGS(opts.pack)-1] = _T('\0');
		DBFreeVariant(&dbv);
	}
}


#define BORDER 5


struct PackData
{
	EmoticonPack *pack;
	int max_height;
	int max_width;
};


static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			if (packs.getCount() <= 0)
				break;

			int sel = 0;
			for(int i = 0; i < packs.getCount(); i++)
			{
				PackData *pd = new PackData();
				pd->pack = packs[i];

				pd->max_height = 0;
				pd->max_width = 0;
				srand(time(NULL));
				int prob = (pd->pack->images.getCount() - 15) / 30 + 1;
				for(int j = 0, count = 0; j < pd->pack->images.getCount() && count < 15; j++) {
					if (rand() % prob != 0)
						continue;
					pd->pack->images[j]->Load(pd->max_height, pd->max_width);
					count++;
				}

				SendDlgItemMessage(hwndDlg, IDC_PACK, LB_ADDSTRING, 0, (LONG) pd);
				SendDlgItemMessage(hwndDlg, IDC_PACK, LB_SETITEMDATA, i, (LONG) pd);

				if (strcmp(opts.pack, pd->pack->name) == 0)
					sel = i;
			}
			SendDlgItemMessage(hwndDlg, IDC_PACK, LB_SETCURSEL, sel, 0);

			break;
		}

		case WM_COMMAND:
		{
			if(LOWORD(wParam) == IDC_GETMORE)
				CallService(MS_UTILS_OPENURL, 1, (LPARAM) "http://addons.miranda-im.org/index.php?action=display&id=41");

			if (LOWORD(wParam) == IDC_PACK
					&& (HIWORD(wParam) == LBN_SELCHANGE && (HWND)lParam == GetFocus()))
			{
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				return 0;
			}

			break;
		}

		case WM_NOTIFY:
		{
			if (packs.getCount() <= 0)
				break;

			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				int sel = SendDlgItemMessage(hwndDlg, IDC_PACK, LB_GETCURSEL, 0, 0);
				if (sel >= packs.getCount())
					sel = 0;

				PackData *pd = (PackData *) SendDlgItemMessage(hwndDlg, IDC_PACK, LB_GETITEMDATA, sel, 0);
				EmoticonPack *pack = (pd == NULL ? NULL : pd->pack);
				if (pack == NULL)
					pack = packs[0];

				DBWriteContactSettingString(NULL, MODULE_NAME, "DefaultPack", pack->name);
				strcpy(opts.pack, pack->name);
				FillModuleImages(pack);
			}
			
			break;
		}

		case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *mi = (MEASUREITEMSTRUCT *) lParam;
			if (mi == NULL)
				break;

			PackData *pd = (PackData *) mi->itemData;
			mi->itemHeight = 2 * BORDER;

			if (pd->max_height > 0)
				mi->itemHeight += pd->max_height + BORDER;

			// Get font
			HFONT hFont = (HFONT) SendMessage(GetDlgItem(hwndDlg, IDC_PACK), WM_GETFONT, 0, 0);

			HDC hdc = GetDC(GetDlgItem(hwndDlg, IDC_PACK));

			// Create one +2px bold
			LOGFONT lf = {0};
			GetObject(hFont, sizeof(lf), &lf);
			lf.lfHeight = - abs(lf.lfHeight) - MulDiv(2, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			lf.lfWidth = 0;
			lf.lfWeight = FW_BOLD;

			hFont = CreateFontIndirect(&lf);
			
			HFONT hOldFont = (HFONT) SelectObject(hdc, hFont);

			// Get its metrics
			TEXTMETRIC tm = {0};
			GetTextMetrics(hdc, &tm);

			SelectObject(hdc, hOldFont);
			ReleaseDC(GetDlgItem(hwndDlg, IDC_PACK), hdc);
			DeleteObject(hFont);

			mi->itemHeight += tm.tmHeight;

			return TRUE;
		}

		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)  lParam;
			if (di->itemAction == ODA_FOCUS)
				break;

			PackData *pd = (PackData *) di->itemData;
			HDC hdc = di->hDC;
			RECT rc = di->rcItem;

			if (di->itemID == -1 || (di->itemState & ODS_SELECTED))
				FillRect(hdc, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
			else
				FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));

			if (di->itemID == -1)
				return TRUE;

			int old_bk_mode = SetBkMode(hdc, TRANSPARENT);
			COLORREF old_color = GetTextColor(hdc);

			rc.left += BORDER;
			rc.right -= BORDER;
			rc.top += BORDER;
			rc.bottom -= BORDER;

			HRGN rgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hdc, rgn);

			// Get font
			HFONT hFont = (HFONT) SendMessage(GetDlgItem(hwndDlg, IDC_PACK), WM_GETFONT, 0, 0);

			// Create one +2px bold
			LOGFONT lf = {0};
			GetObject(hFont, sizeof(lf), &lf);
			lf.lfHeight = - abs(lf.lfHeight) - MulDiv(2, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			lf.lfWidth = 0;
			lf.lfWeight = FW_BOLD;

			HFONT hBigFont = CreateFontIndirect(&lf);

			// Get its metrics
			HFONT old_font = (HFONT) SelectObject(hdc, hBigFont);

			RECT rc_tmp = { rc.left, rc.top, 0xFFFF, 0xFFFF };
			DrawText(hdc, pd->pack->description, lstrlen(pd->pack->description), &rc_tmp, DT_CALCRECT | DT_NOPREFIX | DT_TOP | DT_SINGLELINE);

			DrawText(hdc, pd->pack->description, lstrlen(pd->pack->description), &rc_tmp, DT_NOPREFIX | DT_BOTTOM | DT_SINGLELINE);

			rc_tmp.left = rc_tmp.right + BORDER;
			rc_tmp.right = rc.right;

			if (pd->pack->creator != NULL && pd->pack->creator[0] != _T('\0'))
			{
				TEXTMETRIC tmb = {0};
				GetTextMetrics(hdc, &tmb);

				SelectObject(hdc, hFont);

				TEXTMETRIC tms = {0};
				GetTextMetrics(hdc, &tms);

				rc_tmp.bottom -= tmb.tmDescent - tms.tmDescent;

				TCHAR tmp[256];
				mir_sntprintf(tmp, MAX_REGS(tmp), TranslateT("by %s"), pd->pack->creator);
				DrawText(hdc, tmp, lstrlen(tmp), &rc_tmp, DT_NOPREFIX | DT_BOTTOM | DT_SINGLELINE);
			}

			rc_tmp.left = rc.left;
			rc_tmp.right = rc.right;
			rc_tmp.top = rc_tmp.bottom + BORDER;
			rc_tmp.bottom = rc.bottom;

			for(int i = 0; i < pd->pack->images.getCount(); i++)
			{
				EmoticonImage *img = pd->pack->images[i];
				if (img == NULL || img->img == NULL)
					continue;

				BITMAP bmp;
				GetObject(img->img, sizeof(bmp), &bmp);

				if (rc_tmp.left + bmp.bmWidth > rc_tmp.right)
					break;

				HDC hdc_img = CreateCompatibleDC(hdc);
				HBITMAP old_bmp = (HBITMAP) SelectObject(hdc_img, img->img);

				int x = rc_tmp.left;
				int y = rc_tmp.top + ((rc_tmp.bottom - rc_tmp.top) - bmp.bmHeight) / 2;

				if (img->transparent)
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

				rc_tmp.left += bmp.bmWidth + 2;
			}

			SelectClipRgn(hdc, NULL);
			DeleteObject(rgn);
			SelectObject(hdc, old_font);
			SetTextColor(hdc, old_color);
			SetBkMode(hdc, old_bk_mode);
			DeleteObject(hBigFont);

			return TRUE;
		}

		case WM_DESTROY:
		{
			if (packs.getCount() <= 0)
				break;

			for(int i = 0; i < packs.getCount(); i++)
			{
				PackData *pd = (PackData *) SendDlgItemMessage(hwndDlg, IDC_PACK, LB_GETITEMDATA, i, 0);
				if (pd != NULL)
					delete pd;

				if (strcmp(opts.pack, packs[i]->name) != 0)
					for(int j = 0; j < packs[i]->images.getCount(); j++)
						if (packs[i]->images[j] != NULL)
							packs[i]->images[j]->Release();
			}

			break;
		}
	}

	return SaveOptsDlgProc(optionsControls, MAX_REGS(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}


