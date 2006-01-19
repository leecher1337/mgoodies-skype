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
class SmileyWindow;

#ifndef SMILEYWINDOW_INCLUDED
#define SMILEYWINDOW_INCLUDED

#include "IEView.h"
#include "Smiley.h"
#include "ieview_common.h"

class SmileyWindow {

private:
	HWND    hwnd;
	IEView* view;
	SmileyMap *map;
	bool    created;
	DWORD 	bkgColor;
	int 	cellWidth, maxWidth, maxHeight, cellHeight;
	int     viewWidth, viewHeight;
    HWND 	hwndTarget;
	UINT 	targetMessage;
  	LPARAM 	targetWParam;
  	void    createSelection();
public:
	SmileyWindow(SmileyMap *);
	~SmileyWindow();
	void    show(HWND hwndTarget, UINT targetMessage, WPARAM targetWParam, int , int);
	void    setBackground(DWORD color);
	void    init(int cw, int ch);
	void    choose(const char *);
};

#endif
