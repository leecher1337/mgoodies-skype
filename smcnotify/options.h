/*
Status Message Change Notify plugin for Miranda IM.

Copyright © 2004-2005 NoName
Copyright © 2005-2006 Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci

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

#ifndef __SMCNOTIFY_OPTIONS_H
#define __SMCNOTIFY_OPTIONS_H

#define TEMPLATEMAXLEN	1024

typedef struct {
	BYTE bIgnoreRemove;
	BYTE bLogEnable;
#ifdef UNICODE
	BYTE bLogAscii;
#endif
	BYTE bHistoryEnable;
	WORD dHistoryMax;
	BYTE bDBEnable;
	BYTE bListUseBkImage;
	COLORREF colListBack;
	COLORREF colListText;
	//strings
	TCHAR msgremoved[TEMPLATEMAXLEN];
	TCHAR msgchanged[TEMPLATEMAXLEN];
	TCHAR history[TEMPLATEMAXLEN];
	TCHAR log[TEMPLATEMAXLEN];
	TCHAR logfile[MAX_PATH];
	TCHAR listbkimage[MAX_PATH];
} SMCNOTIFY_OPTIONS;

SMCNOTIFY_OPTIONS opts;

typedef struct {
	BYTE bEnable;
	BYTE bOnConnect;
	BYTE bIfChanged;
	BYTE bIgnoreRemove;
#ifdef CUSTOMBUILD_OSDSUPPORT
	BYTE bUseOSD;
#endif
	WORD bColorType;
	COLORREF colBack;
	COLORREF colText;
	WORD dDelay;
	WORD bDelayType;
	WORD LeftClickAction;
	WORD RightClickAction;
	TCHAR text[TEMPLATEMAXLEN];
} SMCNOTIFY_PUOPTIONS;

SMCNOTIFY_PUOPTIONS puopts;

void ClearAllHistory();

BOOL AllowProtocol(const char *proto);

void LoadOptions(void);
int OptionsInit(WPARAM wParam, LPARAM lParam);
int UserInfoInit(WPARAM wParam, LPARAM lParam);

BOOL CALLBACK IgnoreDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define POPUP_DELAY_DEFAULT		0
#define POPUP_DELAY_CUSTOM		1
#define POPUP_DELAY_PERMANENT	2

#define POPUP_COLOR_DEFAULT		0
#define POPUP_COLOR_WINDOWS		1
#define POPUP_COLOR_CUSTOM		2

#define POPUP_ACTION_NOTHING	0
#define POPUP_ACTION_CLOSE		1
#define POPUP_ACTION_MESSAGE	2
#define POPUP_ACTION_MENU		3
#define POPUP_ACTION_INFO		4
#define POPUP_ACTION_HISTORY	5

#define DEFAULT_TEMPLATE_POPUP		"changed his/her status message to %n"
#define DEFAULT_TEMPLATE_HISTORY	"[%Y/%M/%D %h:%m %a]\\n%n"
#define DEFAULT_TEMPLATE_REMOVED	"removes his/her status message"
#define DEFAULT_TEMPLATE_CHANGED	"changes his/her status message to %n"
#define DEFAULT_TEMPLATE_LOG		"[%Y/%M/%D %h:%m %a] %c: %n"

#define DEFAULT_BGIMAGE_FILENAME	"SMCNotify.bmp"
#define DEFAULT_LOG_FILENAME		"SMCNotify.txt"

#endif // __SMCNOTIFY_OPTIONS_H
