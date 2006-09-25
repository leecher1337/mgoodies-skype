/*
Scriver

Copyright 2000-2003 Miranda ICQ/IM project,
Copyright 2005 Piotr Piastucki

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#ifndef MSGWINDOW_H
#define MSGWINDOW_H

/* container services */
#define CM_ADDCHILD          (WM_USER+0x180)
#define CM_REMOVECHILD		 (WM_USER+0x181)
#define CM_ACTIVATECHILD	 (WM_USER+0x182)
#define CM_ACTIVATEPREV		 (WM_USER+0x183)
#define CM_ACTIVATENEXT		 (WM_USER+0x184)
#define CM_UPDATETITLEBAR    (WM_USER+0x190)
#define CM_UPDATESTATUSBAR   (WM_USER+0x191)
#define CM_UPDATETABCONTROL  (WM_USER+0x192)
#define CM_STARTFLASHING	 (WM_USER+0x1A0)
/* child window services */
#define DM_UPDATETITLEBAR    (WM_USER+0x200)
#define DM_UPDATESTATUSBAR   (WM_USER+0x201)
#define DM_UPDATETABCONTROL  (WM_USER+0x202)
#define DM_SETPARENT	 	 (WM_USER+0x203)
#define DM_ACTIVATE			 (WM_USER+0x206)

#define SBDF_TEXT  1
#define SBDF_ICON  2

typedef struct StatusBarDataStruct
{
	int iItem;
	int iFlags;
	TCHAR *pszText;
	HICON hIcon;
} StatusBarData;

#define TBDF_TEXT 1
#define TBDF_ICON 2

typedef struct TitleBarDataStruct
{
	int iFlags;
	TCHAR *pszText;
	HICON hIcon;
} TitleBarData;

#define TCDF_TEXT 1
#define TCDF_ICON 2

typedef struct TabControlDataStruct
{
	int iFlags;
	TCHAR *pszText;
	int iconIdx;
} TabControlData;

extern TCHAR *charToTchar(const char *text, int textlen, int cp);
extern TCHAR* GetWindowTitle(HANDLE *hContact, const char *szProto);
extern TCHAR* GetTabName(HANDLE *hContact);

#endif
