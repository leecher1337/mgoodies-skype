/*

IEView Plugin for Miranda IM
Copyright (C) 2005  Piotr Piastucki

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
#include "SmileyWindow.h"
#include "Options.h"
#include "resource.h"
#include "Utils.h"

static BOOL CALLBACK SmileySelectionDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

SmileyWindow::SmileyWindow(SmileyMap *map) {
	this->map = map;
 	hwnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_SMILEYSELECTION), NULL, SmileySelectionDlgProc, (LPARAM) this);
	view = new IEView(hwnd, this, 0, 0, 200, 200);
	created = false;
//	bkgColor = Options::getSmileyBackground();//0xFFFFFF;
    bkgColor= (((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16));
}

SmileyWindow::~SmileyWindow() {
	if (hwnd!=NULL) {
		DestroyWindow(hwnd);
	}
	if (view!=NULL) {
		delete view;
	}
}
void SmileyWindow::createSelection() {
	int outputSize;
	char *output = NULL;
    int cellWidthBorder = cellWidth + 1;
	int cellHeightBorder = cellHeight + 1;
	// init window
	Smiley * s;
	int i, j;

	view->clear();
	int totalNum = map->getVisibleSmileyNum();
	if (!totalNum) return;
	int maxInLine = maxWidth/(cellWidthBorder);
	int maxInColumn = maxHeight/(cellHeightBorder);
	int hSize = maxInLine;
	int vSize = (totalNum + hSize -1)  / hSize;
	DWORD frameColor = (0xFF0000 - (bkgColor & 0xFF0000)) | (0x00FF00 - (bkgColor & 0x00FF00)) | (0x0000FF - (bkgColor & 0x0000FF));
	for (;vSize < maxInColumn && vSize * cellHeightBorder < hSize * cellWidthBorder;) {
		hSize--;
        vSize = (totalNum + hSize -1)  / hSize;
	}
	viewHeight = min(vSize, maxInColumn) * cellHeightBorder +1;
	NONCLIENTMETRICS ncm;
	ncm.cbSize=sizeof(NONCLIENTMETRICS);
	if (vSize > maxInColumn) {
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS),&ncm,0);
	} else {
        ncm.iScrollWidth = 0;
	}
	viewWidth= hSize * cellWidthBorder + ncm.iScrollWidth + 1;
	SetWindowPos(hwnd, NULL, 0, 0, viewWidth+ 2, viewHeight + 2, SWP_NOMOVE | SWP_NOZORDER | SWP_HIDEWINDOW);
	view->setWindowPos(0, 0, viewWidth, viewHeight);

	Utils::appendText(&output, &outputSize, "<html><head><style type=\"text/css\"> \n\
.body {margin: 0px; background-color: #%06X; overflow: auto;}\n\
.link {color: #0000FF; text-decoration: underline;}\n\
.img {vertical-align: middle;}\n\
.table {border: 1px dotted #%06X; border-top: 0px; border-left: 0px; }\n\
.td { background-color: #%06X; border: 1px dotted #%06X; border-right: 0px; border-bottom: 0px; }\n\
div#outer { float:left; height: %dpx; width: %dpx; overflow: hidden; position: relative; }\n\
div#middle { position: absolute; top: 50%%; left: 50%%; }\n\
div#inner { position: relative; top: -50%%; left: -50%%; }\n\
</style></head><body class=\"body\">\n", bkgColor, frameColor, bkgColor, frameColor, cellHeight, cellWidth);
	Utils::appendText(&output, &outputSize, "<table class=\"table\" cellspacing=\"0\" cellpadding=\"0\">\n");
	for (i=j=0, s=map->getSmiley();s!=NULL && j<150;s=s->getNext(),i++) {
		if (s->isHidden()) continue;
		if (j%hSize == 0) {
			Utils::appendText(&output, &outputSize, "<tr>\n");
		}
		if (strstr(s->getFile(), ".png")!=NULL) {
			Utils::appendText(&output, &outputSize, "<td class=\"td\"><div id=\"outer\"><div id=\"middle\"><div id=\"inner\"><a href=\"/%d\"><img class=\"img\" style=\"height:1px;width:1px;filter:progid:DXImageTransform.Microsoft.AlphaImageLoader(src='%s',sizingMethod='image');\" alt=\"%s\" border=\"0\"/></a></div></div></div></td>\n",
							i, s->getFile(), s->getDescription());
		} else {
			Utils::appendText(&output, &outputSize, "<td class=\"td\"><div id=\"outer\"><div id=\"middle\"><div id=\"inner\"><a href=\"/%d\"><img class=\"img\" src=\"%s\" alt=\"%s\" border=\"0\"/></a></div></div></div></td>\n",
							i, s->getFile(), s->getDescription());
		}
		if (j%hSize == hSize-1) {
			Utils::appendText(&output, &outputSize, "</tr>\n");
		}
		j++;
	}
	for (;j%hSize != 0;j++) {
		Utils::appendText(&output, &outputSize, "<td class=\"td\"><div id=\"outer\"><div id=\"middle\"><div id=\"inner\">&nbsp;</div></div></div></td>\n");
		if (j%hSize == hSize-1) {
			Utils::appendText(&output, &outputSize, "</tr>\n");
		}
	}
	Utils::appendText(&output, &outputSize, "</table></body></html>\n");
	char *outputEnc = Utils::UTF8Encode(output);
	free(output);
	view->write(outputEnc);
	delete outputEnc;
}

void SmileyWindow::init(int cw, int ch) {
    maxWidth = 350;
    maxHeight = 250;
	cellWidth = cw;
	cellHeight = ch;
	created = true;
	createSelection();
}

void SmileyWindow::show(HWND hwndTarget, UINT targetMessage, WPARAM targetWParam, int x, int y) {
	int direction = 0;
	int xScreen = GetSystemMetrics(SM_CXSCREEN);
	int yScreen = GetSystemMetrics(SM_CYSCREEN);
	if (!created) {
		created = true;
		createSelection();
	}
	if(y + viewHeight > yScreen) {
		y -= 24;
		direction = 3;
	}
	if (x + viewWidth > xScreen) {
		x -= viewWidth;
	}
    switch (direction) { //   2 3
                       //   1 0
        case 1:
            x -= viewWidth;
            break;
        case 2:
            x -= viewWidth;
            y -= viewHeight;
            break;
        case 3:
            y -= viewHeight;
            break;
    }
	this->hwndTarget = hwndTarget;
	this->targetMessage = targetMessage;
	this->targetWParam = targetWParam;
	SetWindowPos(hwnd, NULL, x, y, viewWidth+ 2, viewHeight + 2, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
}

static BOOL CALLBACK SmileySelectionDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	SmileyWindow *smileyWindow;
	smileyWindow = (SmileyWindow *) GetWindowLong(hwndDlg, GWL_USERDATA);
    switch (msg) {
        case WM_INITDIALOG:
			smileyWindow = (SmileyWindow *) lParam;
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) smileyWindow);
			ShowWindow(hwndDlg, SW_HIDE);
            return TRUE;
        case WM_ACTIVATE:
            if (wParam == WA_INACTIVE) ShowWindow(hwndDlg, SW_HIDE);
            break;
        case WM_DESTROY:
            SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) NULL);
            if (smileyWindow != NULL) {
			}
            return TRUE;
    }
    return FALSE;
}

void SmileyWindow::choose(const char * smiley) {
	ShowWindow(hwnd, SW_HIDE);
	if (hwndTarget!=NULL) {
		const char *str = strstr(smiley, "/");
		if (str!=NULL) {
			int i = atoi(str+1);
			Smiley *s;
			for (s=map->getSmiley();i>0 && s!=NULL;s=s->getNext(),i--);
			if (s!=NULL) {
				SendMessage(hwndTarget, targetMessage, targetWParam, (LPARAM) s->getPatternString());
			}

		}
	}
}

void SmileyWindow::setBackground(DWORD color) {
    bkgColor= (((color & 0xFF) << 16) | (color & 0xFF00) | ((color & 0xFF0000) >> 16));
    createSelection();
}
