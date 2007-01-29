/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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

#include "chat.h"
#include <shlobj.h>
#include <shlwapi.h>

extern HANDLE			g_hInst;
extern HBRUSH 			hEditBkgBrush;
extern HBRUSH 			hListBkgBrush;
extern HICON			hIcons[30];
extern FONTINFO			aFonts[OPTIONS_FONTCOUNT];
extern BOOL				PopUpInstalled;


HANDLE			g_hOptions = NULL;

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
struct FontOptionsList
{
    TCHAR*   szDescr;
    COLORREF defColour;
    TCHAR*   szDefFace;
    BYTE     defCharset, defStyle;
    char     defSize;
    COLORREF colour;
    TCHAR    szFace[LF_FACESIZE];
    BYTE     charset, style;
    char     size;
}

//remeber to put these in the Translate( ) template file too
static fontOptionsList[] = {
	{_T("Timestamp"), RGB(50, 50, 240), _T("Terminal"), DEFAULT_CHARSET, 0, -8},
	{_T("Others nicknames"), RGB(0, 0, 0), _T("Verdana"), DEFAULT_CHARSET, FONTF_BOLD, -13},
	{_T("Your nickname"), RGB(0, 0, 0), _T("Verdana"), DEFAULT_CHARSET, FONTF_BOLD, -13},
	{_T("User has joined"), RGB(90, 160, 90), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("User has left"), RGB(160, 160, 90), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("User has disconnected"), RGB(160, 90, 90), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("User kicked ..."), RGB(100, 100, 100), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("User is now known as ..."), RGB(90, 90, 160), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("Notice from user"), RGB(160, 130, 60), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("Incoming message"), RGB(90, 90, 90), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("Outgoing message"), RGB(90, 90, 90), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("The topic is ..."), RGB(70, 70, 160), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("Information messages"), RGB(130, 130, 195), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("User enables status for ..."), RGB(70, 150, 70), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("User disables status for ..."), RGB(150, 70, 70), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("Action message"), RGB(160, 90, 160), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("Highlighted message"), RGB(180, 150, 80), _T("Verdana"), DEFAULT_CHARSET, 0, -13},
	{_T("Message typing area"), RGB(0, 0, 40), _T("Verdana"), DEFAULT_CHARSET, 0, -14},
	{_T("User list members (Online)"), RGB(0,0, 0), _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{_T("User list members (away)"), RGB(170, 170, 170), _T("Verdana"), DEFAULT_CHARSET, 0, -12},
};

struct branch_t
{
	TCHAR*    szDescr;
	char*     szDBName;
	int       iMode;
	BYTE      bDefault;
	HTREEITEM hItem;
};

static struct branch_t branch1[] = {
	{_T("Send message by pressing the Enter key"), "SendOnEnter", 0, 1, NULL},
	{_T("Send message by pressing the Enter key twice"), "SendOnDblEnter", 0,0, NULL},
	{_T("Flash window when someone speaks"), "FlashWindow", 0,0, NULL},
	{_T("Flash window when a word is highlighted"), "FlashWindowHighlight", 0,1, NULL},
	{_T("Show list of users in the chat room"), "ShowNicklist", 0,1, NULL},
	{_T("Show button for sending messages"), "ShowSend", 0, 0, NULL},
	{_T("Show buttons for controlling the chat room"), "ShowTopButtons", 0,1, NULL},
	{_T("Show buttons for formatting the text you are typing"), "ShowFormatButtons", 0,1, NULL},
	{_T("Show button menus when right clicking the buttons"), "RightClickFilter", 0,0, NULL},
	{_T("Show new windows cascaded"), "CascadeWindows", 0,1, NULL},
	{_T("Save the size and position of chat rooms"), "SavePosition", 0,0, NULL},
	{_T("Show the topic of the room on your contact list (if supported)"), "TopicOnClist", 0, 0, NULL},
	{_T("Do not play sounds when the chat room is focused"), "SoundsFocus", 0, 0, NULL},
	{_T("Do not pop up the window when joining a chat room"), "PopupOnJoin", 0,0, NULL},
	{_T("Toggle the visible state when double clicking in the contact list"), "ToggleVisibility", 0,0, NULL}
};
static struct branch_t branch2[] = {
	{_T("Prefix all events with a timestamp"), "ShowTimeStamp", 0,1, NULL},
	{_T("Only prefix with timestamp if it has changed"), "ShowTimeStampIfChanged", 0,0, NULL},
	{_T("Timestamp has same colour as the event"), "TimeStampEventColour", 0,0, NULL},
	{_T("Indent the second line of a message"), "LogIndentEnabled", 0,1, NULL},
	{_T("Limit user names in the message log to 20 characters"), "LogLimitNames", 0,1, NULL},
	{_T("Add \':\' to auto-completed user names"), "AddColonToAutoComplete", 0, 1, NULL},
	{_T("Strip colors from messages in the log"), "StripFormatting", 0, 0, NULL},
	{_T("Enable the \'event filter\' for new rooms"), "FilterEnabled", 0,0, NULL}
};
static struct branch_t branch3[] = {
	{_T("Show topic changes"), "FilterFlags", GC_EVENT_TOPIC, 0, NULL},
	{_T("Show users joining"), "FilterFlags", GC_EVENT_JOIN, 0, NULL},
	{_T("Show users disconnecting"), "FilterFlags", GC_EVENT_QUIT, 0, NULL},
	{_T("Show messages"), "FilterFlags", GC_EVENT_MESSAGE, 1, NULL},
	{_T("Show actions"), "FilterFlags", GC_EVENT_ACTION, 1, NULL},
	{_T("Show users leaving"), "FilterFlags", GC_EVENT_PART, 0, NULL},
	{_T("Show users being kicked"), "FilterFlags", GC_EVENT_KICK, 1, NULL},
	{_T("Show notices"), "FilterFlags", GC_EVENT_NOTICE, 1, NULL},
	{_T("Show users changing name"), "FilterFlags", GC_EVENT_NICK, 0, NULL},
	{_T("Show information messages"), "FilterFlags", GC_EVENT_INFORMATION, 1, NULL},
	{_T("Show status changes of users"), "FilterFlags", GC_EVENT_ADDSTATUS, 0, NULL},
};

static struct branch_t branch4[] = {
	{_T("Show icon for topic changes"), "IconFlags", GC_EVENT_TOPIC, 0, NULL},
	{_T("Show icon for users joining"), "IconFlags", GC_EVENT_JOIN, 1, NULL},
	{_T("Show icon for users disconnecting"), "IconFlags", GC_EVENT_QUIT, 0, NULL},
	{_T("Show icon for messages"), "IconFlags", GC_EVENT_MESSAGE, 0, NULL},
	{_T("Show icon for actions"), "IconFlags", GC_EVENT_ACTION, 0, NULL},
	{_T("Show icon for highlights"), "IconFlags", GC_EVENT_HIGHLIGHT, 0, NULL},
	{_T("Show icon for users leaving"), "IconFlags", GC_EVENT_PART, 0, NULL},
	{_T("Show icon for users kicking other user"), "IconFlags", GC_EVENT_KICK, 0, NULL},
	{_T("Show icon for notices "), "IconFlags", GC_EVENT_NOTICE, 0, NULL},
	{_T("Show icon for name changes"), "IconFlags", GC_EVENT_NICK, 0, NULL},
	{_T("Show icon for information messages"), "IconFlags", GC_EVENT_INFORMATION, 0, NULL},
	{_T("Show icon for status changes"), "IconFlags", GC_EVENT_ADDSTATUS, 0, NULL},
};

static struct branch_t branch5[] = {
	{_T("Show icons in tray only when the chat room is not active"), "TrayIconInactiveOnly", 0, 1, NULL},
	{_T("Show icon in tray for topic changes"), "TrayIconFlags", GC_EVENT_TOPIC, 0, NULL},
	{_T("Show icon in tray for users joining"), "TrayIconFlags", GC_EVENT_JOIN, 0, NULL},
	{_T("Show icon in tray for users disconnecting"), "TrayIconFlags", GC_EVENT_QUIT, 0, NULL},
	{_T("Show icon in tray for messages"), "TrayIconFlags", GC_EVENT_MESSAGE, 0, NULL},
	{_T("Show icon in tray for actions"), "TrayIconFlags", GC_EVENT_ACTION, 0, NULL},
	{_T("Show icon in tray for highlights"), "TrayIconFlags", GC_EVENT_HIGHLIGHT, 1, NULL},
	{_T("Show icon in tray for users leaving"), "TrayIconFlags", GC_EVENT_PART, 0, NULL},
	{_T("Show icon in tray for users kicking other user"), "TrayIconFlags", GC_EVENT_KICK, 0, NULL},
	{_T("Show icon in tray for notices "), "TrayIconFlags", GC_EVENT_NOTICE, 0, NULL},
	{_T("Show icon in tray for name changes"), "TrayIconFlags", GC_EVENT_NICK, 0, NULL},
	{_T("Show icon in tray for information messages"), "TrayIconFlags", GC_EVENT_INFORMATION, 0, NULL},
	{_T("Show icon in tray for status changes"), "TrayIconFlags", GC_EVENT_ADDSTATUS, 0, NULL},
};

static struct branch_t branch6[] = {
	{_T("Show pop-ups only when the chat room is not active"), "PopUpInactiveOnly", 0, 1, NULL},
	{_T("Show pop-up for topic changes"), "PopupFlags", GC_EVENT_TOPIC, 0, NULL},
	{_T("Show pop-up for users joining"), "PopupFlags", GC_EVENT_JOIN, 0, NULL},
	{_T("Show pop-up for users disconnecting"), "PopupFlags", GC_EVENT_QUIT, 0, NULL},
	{_T("Show pop-up for messages"), "PopupFlags", GC_EVENT_MESSAGE, 0, NULL},
	{_T("Show pop-up for actions"), "PopupFlags", GC_EVENT_ACTION, 0, NULL},
	{_T("Show pop-up for highlights"), "PopupFlags", GC_EVENT_HIGHLIGHT, 0, NULL},
	{_T("Show pop-up for users leaving"), "PopupFlags", GC_EVENT_PART, 0, NULL},
	{_T("Show pop-up for users kicking other user"), "PopupFlags", GC_EVENT_KICK, 0, NULL},
	{_T("Show pop-up for notices "), "PopupFlags", GC_EVENT_NOTICE, 0, NULL},
	{_T("Show pop-up for name changes"), "PopupFlags", GC_EVENT_NICK, 0, NULL},
	{_T("Show pop-up for information messages"), "PopupFlags", GC_EVENT_INFORMATION, 0, NULL},
	{_T("Show pop-up for status changes"), "PopupFlags", GC_EVENT_ADDSTATUS, 0, NULL},
};


static HTREEITEM InsertBranch(HWND hwndTree, TCHAR* pszDescr, BOOL bExpanded)
{
	TVINSERTSTRUCT tvis;

	tvis.hParent=NULL;
	tvis.hInsertAfter=TVI_LAST;
	tvis.item.mask=TVIF_TEXT|TVIF_STATE;
	tvis.item.pszText=TranslateTS(pszDescr);
	tvis.item.stateMask=bExpanded?TVIS_STATEIMAGEMASK|TVIS_EXPANDED:TVIS_STATEIMAGEMASK;
	tvis.item.state=bExpanded?INDEXTOSTATEIMAGEMASK(1)|TVIS_EXPANDED:INDEXTOSTATEIMAGEMASK(1);
	return TreeView_InsertItem(hwndTree, &tvis);
}

static void FillBranch(HWND hwndTree, HTREEITEM hParent, struct branch_t *branch, int nValues, DWORD defaultval)
{
	TVINSERTSTRUCT tvis;
	int i;
	int iState;

	if(hParent == 0)
		return;

	tvis.hParent=hParent;
	tvis.hInsertAfter=TVI_LAST;
	tvis.item.mask=TVIF_TEXT|TVIF_STATE;
	for(i=0;i<nValues;i++) {
		tvis.item.pszText = TranslateTS(branch[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		if (branch[i].iMode)
			iState = ((DBGetContactSettingDword(NULL, "Chat", branch[i].szDBName, defaultval)&branch[i].iMode)&branch[i].iMode)!=0?2:1;
		else
			iState = DBGetContactSettingByte(NULL, "Chat", branch[i].szDBName, branch[i].bDefault)!=0?2:1;
		tvis.item.state=INDEXTOSTATEIMAGEMASK(iState);
		branch[i].hItem = TreeView_InsertItem(hwndTree, &tvis);
	}
}
static void SaveBranch(HWND hwndTree, struct branch_t *branch, int nValues)
{
	TVITEM tvi;
	BYTE bChecked;
	int i;
	int iState = 0;

	tvi.mask=TVIF_HANDLE|TVIF_STATE;
	for(i=0;i<nValues;i++) {
		tvi.hItem = branch[i].hItem;
		TreeView_GetItem(hwndTree,&tvi);
		bChecked = ((tvi.state&TVIS_STATEIMAGEMASK)>>12==1)?0:1;
		if(branch[i].iMode)
		{
			if (bChecked)
				iState |= branch[i].iMode;
			if (iState&GC_EVENT_ADDSTATUS)
				iState |= GC_EVENT_REMOVESTATUS;
			DBWriteContactSettingDword(NULL, "Chat", branch[i].szDBName, (DWORD)iState);
		}
		else DBWriteContactSettingByte(NULL, "Chat", branch[i].szDBName, bChecked);
	}	}

static void CheckHeading(HWND hwndTree, HTREEITEM hHeading)
{
	BOOL bChecked = TRUE;
	TVITEM tvi;

	if(hHeading == 0)
		return;

	tvi.mask=TVIF_HANDLE|TVIF_STATE;
	tvi.hItem=TreeView_GetNextItem(hwndTree, hHeading, TVGN_CHILD);
	while(tvi.hItem && bChecked) {
		if(tvi.hItem != branch1[0].hItem && tvi.hItem != branch1[1].hItem )
		{
			TreeView_GetItem(hwndTree,&tvi);
			if(((tvi.state&TVIS_STATEIMAGEMASK)>>12==1))
				bChecked = FALSE;
		}
		tvi.hItem=TreeView_GetNextSibling(hwndTree,tvi.hItem);
	}
	tvi.stateMask = TVIS_STATEIMAGEMASK;
	tvi.state=INDEXTOSTATEIMAGEMASK(bChecked?2:1);
	tvi.hItem = hHeading;
	TreeView_SetItem(hwndTree,&tvi);
}
static void CheckBranches(HWND hwndTree, HTREEITEM hHeading)
{
	BOOL bChecked = TRUE;
	TVITEM tvi;

	if(hHeading == 0)
		return;

	tvi.mask=TVIF_HANDLE|TVIF_STATE;
	tvi.hItem = hHeading;
	TreeView_GetItem(hwndTree,&tvi);
	if(((tvi.state&TVIS_STATEIMAGEMASK)>>12==2))
		bChecked = FALSE;
	tvi.hItem=TreeView_GetNextItem(hwndTree, hHeading, TVGN_CHILD);
	tvi.stateMask = TVIS_STATEIMAGEMASK;
	while(tvi.hItem) {
		tvi.state=INDEXTOSTATEIMAGEMASK(bChecked?2:1);
		if(tvi.hItem != branch1[0].hItem && tvi.hItem != branch1[1].hItem )
			TreeView_SetItem(hwndTree,&tvi);
		tvi.hItem=TreeView_GetNextSibling(hwndTree,tvi.hItem);
	}
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	char szDir[MAX_PATH];
	switch(uMsg) {
		case BFFM_INITIALIZED:
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
			break;
		case BFFM_SELCHANGED:
			if (SHGetPathFromIDListA((LPITEMIDLIST) lp ,szDir))
					SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
			break;
	}
	return 0;
}

void LoadLogFonts(void)
{
	int i;

	for( i = 0; i<OPTIONS_FONTCOUNT; i++)
		Chat_LoadMsgDlgFont(i, &aFonts[i].lf, &aFonts[i].color);
}

void Chat_LoadMsgDlgFont(int i, LOGFONT* lf, COLORREF* colour)
{
    char str[32];
    int style;
    DBVARIANT dbv;

    if (colour) {
        wsprintfA(str, "Font%dCol", i);
        *colour = DBGetContactSettingDword(NULL, "ChatFonts", str, fontOptionsList[i].defColour);
    }
    if (lf) {
        wsprintfA(str, "Font%dSize", i);
        lf->lfHeight = (char) DBGetContactSettingByte(NULL, "ChatFonts", str, fontOptionsList[i].defSize);
        lf->lfWidth = 0;
        lf->lfEscapement = 0;
        lf->lfOrientation = 0;
        wsprintfA(str, "Font%dSty", i);
        style = DBGetContactSettingByte(NULL, "ChatFonts", str, fontOptionsList[i].defStyle);
        lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
        lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
        lf->lfUnderline = 0;
        lf->lfStrikeOut = 0;
        wsprintfA(str, "Font%dSet", i);
        lf->lfCharSet = DBGetContactSettingByte(NULL, "ChatFonts", str, fontOptionsList[i].defCharset);
        lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf->lfQuality = DEFAULT_QUALITY;
        lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        wsprintfA(str, "Font%d", i);
        if (DBGetContactSettingTString(NULL, "ChatFonts", str, &dbv))
            lstrcpy(lf->lfFaceName, fontOptionsList[i].szDefFace);
        else {
			lstrcpyn(lf->lfFaceName, dbv.ptszVal, SIZEOF(lf->lfFaceName));
            DBFreeVariant(&dbv);
        }
    }
}

void RegisterFonts( void )
{
	FontIDT fontid = {0};
	ColourIDT colourid;
	char idstr[10];
	int index = 0, i;

	fontid.cbSize = sizeof(FontIDT);
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID | FIDF_NEEDRESTART;
	for (i = 0; i < SIZEOF(fontOptionsList); i++, index++) {
		strncpy(fontid.dbSettingsGroup, "ChatFonts", sizeof(fontid.dbSettingsGroup));
		_tcsncpy(fontid.group, TranslateT("Chat Module"), SIZEOF(fontid.group));
		_tcsncpy(fontid.name, TranslateTS(fontOptionsList[i].szDescr), SIZEOF(fontid.name));
		sprintf(idstr, "Font%d", index);
		strncpy(fontid.prefix, idstr, sizeof(fontid.prefix));
		fontid.order = index;

		fontid.deffontsettings.charset = fontOptionsList[i].defCharset;
		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = fontOptionsList[i].defSize;
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		_tcsncpy(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, SIZEOF(fontid.deffontsettings.szFace));
		_tcsncpy(fontid.backgroundGroup, TranslateT("Chat Module"), SIZEOF(fontid.backgroundGroup));
		switch (i) {
		case 17:
			_tcsncpy(fontid.backgroundName, TranslateT("Message Background"), SIZEOF(fontid.backgroundName));
			break;
		case 18:
		case 19:
			_tcsncpy(fontid.backgroundName, TranslateT("Userlist Background"), SIZEOF(fontid.backgroundName));
			break;
		default:
			_tcsncpy(fontid.backgroundName, TranslateT("Background"), SIZEOF(fontid.backgroundName));
			break;
		}
		CallService(MS_FONT_REGISTERT, (WPARAM)&fontid, 0);
	}

	colourid.cbSize = sizeof(ColourIDT);
	colourid.order = 0;
	strncpy(colourid.dbSettingsGroup, "Chat", sizeof(colourid.dbSettingsGroup));

	strncpy(colourid.setting, "ColorLogBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, TranslateT("Background"), SIZEOF(colourid.name));
	_tcsncpy(colourid.group, TranslateT("Chat Module"), SIZEOF(colourid.group));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colourid, 0);

	strncpy(colourid.setting, "ColorMessageBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, TranslateT("Message Background"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colourid, 0);

	strncpy(colourid.setting, "ColorNicklistBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, TranslateT("Userlist Background"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colourid, 0);

	strncpy(colourid.setting, "ColorNicklistLines", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, TranslateT("Userlist Lines"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_INACTIVEBORDER);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colourid, 0);
}


// add icons to the skinning module
void AddIcons(void)
{
	if(ServiceExists(MS_SKIN2_ADDICON))
	{
		SKINICONDESC sid = {0};
		char szFile[MAX_PATH];

		// 16x16 icons
		sid.cx = sid.cy = 16;
		sid.cbSize = sizeof(SKINICONDESC);

		sid.pszSection = Translate("Chat windows");
		GetModuleFileNameA(g_hInst, szFile, MAX_PATH);
		sid.pszDefaultFile = szFile;

		// add them one by one
		sid.pszDescription = Translate("Window Icon");
		sid.pszName = "chat_window";
		sid.iDefaultIndex = -IDI_CHANMGR;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Text colour");
		sid.pszName = "chat_fgcol";
		sid.iDefaultIndex = -IDI_COLOR;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Background colour");
		sid.pszName = "chat_bkgcol";
		sid.iDefaultIndex = -IDI_BKGCOLOR;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Bold");
		sid.pszName = "chat_bold";
		sid.iDefaultIndex = -IDI_BBOLD;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Italics");
		sid.pszName = "chat_italics";
		sid.iDefaultIndex = -IDI_BITALICS;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Underlined");
		sid.pszName = "chat_underline";
		sid.iDefaultIndex = -IDI_BUNDERLINE;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Smiley button");
		sid.pszName = "chat_smiley";
		sid.iDefaultIndex = -IDI_BSMILEY;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Room history");
		sid.pszName = "chat_history";
		sid.iDefaultIndex = -IDI_CHAT_HISTORY;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);


		sid.pszDescription = Translate("Room settings");
		sid.pszName = "chat_settings";
		sid.iDefaultIndex = -IDI_TOPICBUT;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Event filter disabled");
		sid.pszName = "chat_filter";
		sid.iDefaultIndex = -IDI_FILTER;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Event filter enabled");
		sid.pszName = "chat_filter2";
		sid.iDefaultIndex = -IDI_FILTER2;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Hide userlist");
		sid.pszName = "chat_nicklist";
		sid.iDefaultIndex = -IDI_NICKLIST;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Show userlist");
		sid.pszName = "chat_nicklist2";
		sid.iDefaultIndex = -IDI_NICKLIST2;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Icon overlay");
		sid.pszName = "chat_overlay";
		sid.iDefaultIndex = -IDI_OVERLAY;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.cx = sid.cy = 10;
		sid.pszDescription = Translate("Status 1 (10x10)");
		sid.pszName = "chat_status0";
		sid.iDefaultIndex = -IDI_STATUS0;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Status 2 (10x10)");
		sid.pszName = "chat_status1";
		sid.iDefaultIndex = -IDI_STATUS1;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Status 3 (10x10)");
		sid.pszName = "chat_status2";
		sid.iDefaultIndex = -IDI_STATUS2;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Status 4 (10x10)");
		sid.pszName = "chat_status3";
		sid.iDefaultIndex = -IDI_STATUS3;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Status 5 (10x10)");
		sid.pszName = "chat_status4";
		sid.iDefaultIndex = -IDI_STATUS4;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Status 6 (10x10)");
		sid.pszName = "chat_status5";
		sid.iDefaultIndex = -IDI_STATUS5;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszSection = Translate("Chat log");
		sid.pszDescription = Translate("Message in (10x10)");
		sid.pszName = "chat_log_message_in";
		sid.iDefaultIndex = -IDI_MESSAGE;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Message out (10x10)");
		sid.pszName = "chat_log_message_out";
		sid.iDefaultIndex = -IDI_MESSAGEOUT;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Action (10x10)");
		sid.pszName = "chat_log_action";
		sid.iDefaultIndex = -IDI_ACTION;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Add Status (10x10)");
		sid.pszName = "chat_log_addstatus";
		sid.iDefaultIndex = -IDI_ADDSTATUS;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Remove status (10x10)");
		sid.pszName = "chat_log_removestatus";
		sid.iDefaultIndex = -IDI_REMSTATUS;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Join (10x10)");
		sid.pszName = "chat_log_join";
		sid.iDefaultIndex = -IDI_JOIN;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Leave (10x10)");
		sid.pszName = "chat_log_part";
		sid.iDefaultIndex = -IDI_PART;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Quit (10x10)");
		sid.pszName = "chat_log_quit";
		sid.iDefaultIndex = -IDI_QUIT;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Kick (10x10)");
		sid.pszName = "chat_log_kick";
		sid.iDefaultIndex = -IDI_KICK;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Nickchange (10x10)");
		sid.pszName = "chat_log_nick";
		sid.iDefaultIndex = -IDI_NICK;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Notice (10x10)");
		sid.pszName = "chat_log_notice";
		sid.iDefaultIndex = -IDI_CHAT_NOTICE;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Topic (10x10)");
		sid.pszName = "chat_log_topic";
		sid.iDefaultIndex = -IDI_TOPIC;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Highlight (10x10)");
		sid.pszName = "chat_log_highlight";
		sid.iDefaultIndex = -IDI_HIGHLIGHT;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Information (10x10)");
		sid.pszName = "chat_log_info";
		sid.iDefaultIndex = -IDI_INFO;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

	}
	return;
}

// load icons from the skinning module if available
HICON LoadIconEx(int iIndex, char * pszIcoLibName, int iX, int iY)
{
	if(ServiceExists(MS_SKIN2_ADDICON))
	{
		char szTemp[256];
		mir_snprintf(szTemp, SIZEOF(szTemp), "chat_%s", pszIcoLibName);
		return (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)szTemp);
	}

	return (HICON)LoadImage(g_hInst,MAKEINTRESOURCE(iIndex),IMAGE_ICON,iX,iY,LR_SHARED);
}

static void InitSetting(TCHAR** ppPointer, char* pszSetting, TCHAR* pszDefault)
{
	DBVARIANT dbv;
	if ( !DBGetContactSettingTString(NULL, "Chat", pszSetting, &dbv )) {
		replaceStr( ppPointer, dbv.ptszVal );
		DBFreeVariant(&dbv);
	}
	else replaceStr( ppPointer, pszDefault );
	}

#define OPT_FIXHEADINGS (WM_USER+1)
BOOL CALLBACK DlgProcOptions1(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static HTREEITEM hListHeading1 = 0;
	static HTREEITEM hListHeading2= 0;
	static HTREEITEM hListHeading3= 0;
	static HTREEITEM hListHeading4= 0;
	static HTREEITEM hListHeading5= 0;
	static HTREEITEM hListHeading6= 0;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_CHECKBOXES),GWL_STYLE,GetWindowLong(GetDlgItem(hwndDlg,IDC_CHAT_CHECKBOXES),GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN2,UDM_SETRANGE,0,MAKELONG(255,10));
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN2,UDM_SETPOS,0,MAKELONG(DBGetContactSettingByte(NULL,"Chat","NicklistRowDist",12),0));
		hListHeading1 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Appearance and functionality of chat room windows"), DBGetContactSettingByte(NULL, "Chat", "Branch1Exp", 0)?TRUE:FALSE);
		hListHeading2 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Appearance of the message log"), DBGetContactSettingByte(NULL, "Chat", "Branch2Exp", 0)?TRUE:FALSE);
		hListHeading3 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Default events to show in new chat rooms if the \'event filter\' is enabled"), DBGetContactSettingByte(NULL, "Chat", "Branch3Exp", 0)?TRUE:FALSE);
		hListHeading4 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Icons to display in the message log"), DBGetContactSettingByte(NULL, "Chat", "Branch4Exp", 0)?TRUE:FALSE);
		hListHeading5 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Icons to display in the tray"), DBGetContactSettingByte(NULL, "Chat", "Branch5Exp", 0)?TRUE:FALSE);
		if(PopUpInstalled)
			hListHeading6 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Pop-ups to display"), DBGetContactSettingByte(NULL, "Chat", "Branch6Exp", 0)?TRUE:FALSE);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1, branch1, SIZEOF(branch1), 0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2, branch2, SIZEOF(branch2), 0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading3, branch3, SIZEOF(branch3), 0x03E0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading4, branch4, SIZEOF(branch4), 0x0000);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading5, branch5, SIZEOF(branch5), 0x1000);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading6, branch6, SIZEOF(branch6), 0x0000);
		SendMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
		{
			TCHAR* pszGroup = NULL;
			InitSetting(&pszGroup, "AddToGroup", _T("Chat rooms"));
			SetWindowText(GetDlgItem(hwndDlg, IDC_CHAT_GROUP), pszGroup);
			mir_free(pszGroup);
		}
		break;

	case OPT_FIXHEADINGS:
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading3);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading4);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading5);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading6);
		break;

	case WM_COMMAND:
		if (	(LOWORD(wParam) == IDC_CHAT_NICKROW
			|| LOWORD(wParam) == IDC_CHAT_GROUP)
				&& (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))	return 0;

		if(lParam != (LPARAM)NULL)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
	{
		switch(((LPNMHDR)lParam)->idFrom)
		{
		case IDC_CHAT_CHECKBOXES:
			if(((LPNMHDR)lParam)->code==NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x=(short)LOWORD(GetMessagePos());
				hti.pt.y=(short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
				if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
					if(hti.flags&TVHT_ONITEMSTATEICON)
					{
						TVITEM tvi = {0};
						tvi.mask=TVIF_HANDLE|TVIF_STATE;
						tvi.hItem=hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
						if(tvi.hItem == branch1[0].hItem && INDEXTOSTATEIMAGEMASK(1)==tvi.state)
							TreeView_SetItemState(((LPNMHDR)lParam)->hwndFrom, branch1[1].hItem, INDEXTOSTATEIMAGEMASK(1),  TVIS_STATEIMAGEMASK);
						if(tvi.hItem == branch1[1].hItem && INDEXTOSTATEIMAGEMASK(1)==tvi.state)
							TreeView_SetItemState(((LPNMHDR)lParam)->hwndFrom, branch1[0].hItem, INDEXTOSTATEIMAGEMASK(1),  TVIS_STATEIMAGEMASK);

						if (tvi.hItem == hListHeading1)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1);
						else if (tvi.hItem == hListHeading2)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2);
						else if (tvi.hItem == hListHeading3)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading3);
						else if (tvi.hItem == hListHeading4)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading4);
						else if (tvi.hItem == hListHeading5)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading5);
						else if (tvi.hItem == hListHeading6)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading6);
						else
							PostMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}

			}

			break;

			case 0:
				switch (((LPNMHDR)lParam)->code)
				{
				case PSN_APPLY:
					{
						int iLen;
						char * pszText = NULL;

						iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_GROUP));
						if(iLen > 0)
						{
							pszText = mir_realloc(pszText, iLen+1);
							GetDlgItemTextA(hwndDlg, IDC_CHAT_GROUP, pszText,iLen+1);
							DBWriteContactSettingString(NULL, "Chat", "AddToGroup", pszText);
						}
						else DBWriteContactSettingString(NULL, "Chat", "AddToGroup", "");
						mir_free(pszText);

						iLen = SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN2,UDM_GETPOS,0,0);
						if(iLen > 0)
							DBWriteContactSettingByte(NULL, "Chat", "NicklistRowDist", (BYTE)iLen);
						else
							DBDeleteContactSetting(NULL, "Chat", "NicklistRowDist");
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch1, sizeof(branch1) / sizeof(branch1[0]));
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch2, sizeof(branch2) / sizeof(branch2[0]));
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch3, sizeof(branch3) / sizeof(branch3[0]));
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch4, sizeof(branch4) / sizeof(branch4[0]));
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch5, sizeof(branch5) / sizeof(branch5[0]));
						if(PopUpInstalled)
							SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch6, sizeof(branch6) / sizeof(branch6[0]));
						g_Settings.dwIconFlags = DBGetContactSettingDword(NULL, "Chat", "IconFlags", 0x0000);
						g_Settings.dwTrayIconFlags = DBGetContactSettingDword(NULL, "Chat", "TrayIconFlags", 0x1000);
						g_Settings.dwPopupFlags = DBGetContactSettingDword(NULL, "Chat", "PopupFlags", 0x0000);
						g_Settings.StripFormat = (BOOL)DBGetContactSettingByte(NULL, "Chat", "TrimFormatting", 0);
						g_Settings.TrayIconInactiveOnly = (BOOL)DBGetContactSettingByte(NULL, "Chat", "TrayIconInactiveOnly", 1);
						g_Settings.PopUpInactiveOnly = (BOOL)DBGetContactSettingByte(NULL, "Chat", "PopUpInactiveOnly", 1);

						g_Settings.LogIndentEnabled = (DBGetContactSettingByte(NULL, "Chat", "LogIndentEnabled", 1) != 0)?TRUE:FALSE;
						MM_FontsChanged();
						SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
					}
					return TRUE;
				}
		}
	}break;
	case WM_DESTROY:
		{
		BYTE b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
		DBWriteContactSettingByte(NULL, "Chat", "Branch1Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
		DBWriteContactSettingByte(NULL, "Chat", "Branch2Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading3, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
		DBWriteContactSettingByte(NULL, "Chat", "Branch3Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading4, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
		DBWriteContactSettingByte(NULL, "Chat", "Branch4Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading5, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
		DBWriteContactSettingByte(NULL, "Chat", "Branch5Exp", b);
		if(PopUpInstalled)
		{
			b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading6, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
			DBWriteContactSettingByte(NULL, "Chat", "Branch6Exp", b);
		}
		}break;

	default:break;
	}
	return FALSE;
}

BOOL CALLBACK DlgProcOptions2(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static HBRUSH hBkgColourBrush;
	static HBRUSH hListColourBrush;
	static HBRUSH hMessageColourBrush;
	switch (uMsg) {
	case WM_INITDIALOG:
	{
		char szTemp[MAX_PATH];

		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN2,UDM_SETRANGE,0,MAKELONG(5000,0));
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN2,UDM_SETPOS,0,MAKELONG(DBGetContactSettingWord(NULL,"Chat","LogLimit",100),0));
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN3,UDM_SETRANGE,0,MAKELONG(10000,0));
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN3,UDM_SETPOS,0,MAKELONG(DBGetContactSettingWord(NULL,"Chat","LoggingLimit",100),0));
		CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)g_Settings.pszLogDir, (LPARAM)szTemp );
		SetDlgItemTextA(hwndDlg, IDC_CHAT_LOGDIRECTORY, szTemp);
		SetDlgItemText(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS, g_Settings.pszHighlightWords);
		SetDlgItemText(hwndDlg, IDC_CHAT_LOGTIMESTAMP, g_Settings.pszTimeStampLog);
		SetDlgItemText(hwndDlg, IDC_CHAT_TIMESTAMP, g_Settings.pszTimeStamp);
		SetDlgItemText(hwndDlg, IDC_CHAT_OUTSTAMP, g_Settings.pszOutgoingNick);
		SetDlgItemText(hwndDlg, IDC_CHAT_INSTAMP, g_Settings.pszIncomingNick);
		SendDlgItemMessage(hwndDlg, IDC_CHAT_LOGBKG, CPM_SETCOLOUR,0,g_Settings.crLogBackground);
		SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGEBKG, CPM_SETCOLOUR,0,DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW)));
		SendDlgItemMessage(hwndDlg, IDC_CHAT_NICKLISTBKG, CPM_SETCOLOUR,0,DBGetContactSettingDword(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW)));
		SendDlgItemMessage(hwndDlg, IDC_CHAT_LOGBKG, CPM_SETDEFAULTCOLOUR, 0, g_Settings.crLogBackground);
		SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGEBKG, CPM_SETDEFAULTCOLOUR, 0, DBGetContactSettingDword(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW)));
		SendDlgItemMessage(hwndDlg, IDC_CHAT_NICKLISTBKG, CPM_SETDEFAULTCOLOUR, 0, DBGetContactSettingDword(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW)));
		hBkgColourBrush = CreateSolidBrush(SendDlgItemMessage(hwndDlg, IDC_CHAT_LOGBKG, CPM_GETCOLOUR, 0, 0));
		hListColourBrush = CreateSolidBrush(SendDlgItemMessage(hwndDlg, IDC_CHAT_NICKLISTBKG, CPM_GETCOLOUR, 0, 0));
		hMessageColourBrush = CreateSolidBrush(SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGEBKG, CPM_GETCOLOUR, 0, 0));
		CheckDlgButton(hwndDlg, IDC_CHAT_HIGHLIGHT, g_Settings.HighlightEnabled);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS), g_Settings.HighlightEnabled?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_CHAT_LOGGING, g_Settings.LoggingEnabled);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY), g_Settings.LoggingEnabled?TRUE:FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_FONTCHOOSE), g_Settings.LoggingEnabled?TRUE:FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIMIT), g_Settings.LoggingEnabled?TRUE:FALSE);

		{	int i;
			LOGFONT lf;
			for (i = 0; i < SIZEOF(fontOptionsList); i++) {
				Chat_LoadMsgDlgFont(i, &lf, &fontOptionsList[i].colour);
				lstrcpy(fontOptionsList[i].szFace, lf.lfFaceName);
				fontOptionsList[i].size = (char) lf.lfHeight;
				fontOptionsList[i].style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
				fontOptionsList[i].charset = lf.lfCharSet;
				//I *think* some OSs will fail LB_ADDSTRING if lParam==0
				SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_ADDSTRING, 0, i + 1);
			}
			SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_SETSEL, TRUE, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTCOLOR, CPM_SETCOLOUR, 0, fontOptionsList[0].colour);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTCOLOR, CPM_SETDEFAULTCOLOUR, 0, fontOptionsList[0].defColour);
		}
		break;
	}
	case WM_MEASUREITEM:
	{
		MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *) lParam;
		HFONT hFont, hoFont;
		HDC hdc;
		SIZE fontSize;
		int iItem = mis->itemData - 1;

		hFont = CreateFont(fontOptionsList[iItem].size, 0, 0, 0,
						fontOptionsList[iItem].style & FONTF_BOLD ? FW_BOLD : FW_NORMAL,
						fontOptionsList[iItem].style & FONTF_ITALIC ? 1 : 0, 0, 0, fontOptionsList[iItem].charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontOptionsList[iItem].szFace);
		hdc = GetDC(GetDlgItem(hwndDlg, mis->CtlID));
		hoFont = (HFONT) SelectObject(hdc, hFont);
		GetTextExtentPoint32(hdc, fontOptionsList[iItem].szDescr, lstrlen(fontOptionsList[iItem].szDescr), &fontSize);
		SelectObject(hdc, hoFont);
		ReleaseDC(GetDlgItem(hwndDlg, mis->CtlID), hdc);
		DeleteObject(hFont);
		mis->itemWidth = fontSize.cx;
		mis->itemHeight = fontSize.cy;
		return TRUE;
	}
	case WM_DRAWITEM:
	{
		DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT *) lParam;
		HFONT hFont, hoFont;
		TCHAR* pszText;
		int iItem = dis->itemData - 1;
		hFont = CreateFont(fontOptionsList[iItem].size, 0, 0, 0,
			fontOptionsList[iItem].style & FONTF_BOLD ? FW_BOLD : FW_NORMAL,
			fontOptionsList[iItem].style & FONTF_ITALIC ? 1 : 0, 0, 0, fontOptionsList[iItem].charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontOptionsList[iItem].szFace);
		hoFont = (HFONT) SelectObject(dis->hDC, hFont);
		SetBkMode(dis->hDC, TRANSPARENT);
		if(iItem == 18 || iItem == 19)
			FillRect(dis->hDC, &dis->rcItem, hListColourBrush);
		else if(iItem == 17)
			FillRect(dis->hDC, &dis->rcItem, hMessageColourBrush);
		else
			FillRect(dis->hDC, &dis->rcItem, hBkgColourBrush);
		if (dis->itemState & ODS_SELECTED)
			FrameRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
		SetTextColor(dis->hDC, fontOptionsList[iItem].colour);
		pszText = TranslateTS(fontOptionsList[iItem].szDescr);
		TextOut(dis->hDC, dis->rcItem.left, dis->rcItem.top, pszText, lstrlen(pszText));
		SelectObject(dis->hDC, hoFont);
		DeleteObject(hFont);
		return TRUE;
	}

	case WM_CTLCOLORLISTBOX:
        SetBkColor((HDC) wParam, SendDlgItemMessage(hwndDlg, IDC_CHAT_LOGBKG, CPM_GETCOLOUR, 0, 0));
        return (BOOL) hListColourBrush;

	case WM_COMMAND:
		if(	(LOWORD(wParam)		  == IDC_CHAT_INSTAMP
				|| LOWORD(wParam) == IDC_CHAT_OUTSTAMP
				|| LOWORD(wParam) == IDC_CHAT_TIMESTAMP
				|| LOWORD(wParam) == IDC_CHAT_LOGLIMIT
				|| LOWORD(wParam) == IDC_CHAT_HIGHLIGHTWORDS
				|| LOWORD(wParam) == IDC_CHAT_LOGDIRECTORY
				|| LOWORD(wParam) == IDC_CHAT_LOGTIMESTAMP
				|| LOWORD(wParam) == IDC_CHAT_LIMIT)
				&& (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))	return 0;

		switch (LOWORD(wParam)) {
		case IDC_CHAT_MESSAGEBKG:
			DeleteObject(hMessageColourBrush);
			hMessageColourBrush = CreateSolidBrush(SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGEBKG, CPM_GETCOLOUR, 0, 0));
			InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_FONTLIST), NULL, TRUE);
			break;
		case IDC_CHAT_NICKLISTBKG:
			DeleteObject(hListColourBrush);
			hListColourBrush = CreateSolidBrush(SendDlgItemMessage(hwndDlg, IDC_CHAT_NICKLISTBKG, CPM_GETCOLOUR, 0, 0));
			InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_FONTLIST), NULL, TRUE);
			break;
		case IDC_CHAT_LOGBKG:
			DeleteObject(hBkgColourBrush);
			hBkgColourBrush = CreateSolidBrush(SendDlgItemMessage(hwndDlg, IDC_CHAT_LOGBKG, CPM_GETCOLOUR, 0, 0));
			InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_FONTLIST), NULL, TRUE);
			break;
		case IDC_CHAT_LOGGING:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_FONTCHOOSE), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIMIT), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED?TRUE:FALSE);
			break;
		case IDC_CHAT_FONTLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				if (SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_GETSELCOUNT, 0, 0) > 1) {
					SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTCOLOR, CPM_SETCOLOUR, 0, GetSysColor(COLOR_3DFACE));
					SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTCOLOR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_WINDOWTEXT));
				}
			else {
				int i = SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_GETITEMDATA,
									   SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_GETCURSEL, 0, 0), 0) - 1;
				SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTCOLOR, CPM_SETCOLOUR, 0, fontOptionsList[i].colour);
				SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTCOLOR, CPM_SETDEFAULTCOLOUR, 0, fontOptionsList[i].defColour);
				}
			}
			if (HIWORD(wParam) != LBN_DBLCLK)
				return TRUE;
			//fall through
		case IDC_CHAT_CHOOSEFONT:
		{
			CHOOSEFONT cf = { 0 };
			LOGFONT lf = { 0 };
			int i = SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_GETCURSEL, 0, 0),0) - 1;
			lf.lfHeight = fontOptionsList[i].size;
			lf.lfWeight = fontOptionsList[i].style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
			lf.lfItalic = fontOptionsList[i].style & FONTF_ITALIC ? 1 : 0;
			lf.lfCharSet = fontOptionsList[i].charset;
			lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
			lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lf.lfQuality = DEFAULT_QUALITY;
			lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
			lstrcpy(lf.lfFaceName, fontOptionsList[i].szFace);
			cf.lStructSize = sizeof(cf);
			cf.hwndOwner = hwndDlg;
			cf.lpLogFont = &lf;
			cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
			if (ChooseFont(&cf)) {
			int selItems[ SIZEOF(fontOptionsList) ];
				int sel, selCount;

				selCount = SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_GETSELITEMS, sizeof(fontOptionsList) / sizeof(fontOptionsList[0]), (LPARAM) selItems);
				for (sel = 0; sel < selCount; sel++) {
					i = SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_GETITEMDATA, selItems[sel], 0) - 1;
					fontOptionsList[i].size = (char) lf.lfHeight;
					fontOptionsList[i].style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
					fontOptionsList[i].charset = lf.lfCharSet;
					lstrcpy(fontOptionsList[i].szFace, lf.lfFaceName);
					{
						MEASUREITEMSTRUCT mis = { 0 };
						mis.CtlID = IDC_CHAT_FONTLIST;
						mis.itemData = i + 1;
						SendMessage(hwndDlg, WM_MEASUREITEM, 0, (LPARAM) & mis);
						SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_SETITEMHEIGHT, selItems[sel], mis.itemHeight);
					}
				}
				InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_FONTLIST), NULL, TRUE);
				break;
			}
			return TRUE;
		}
		case  IDC_CHAT_FONTCHOOSE:
		{
			char szDirectory[MAX_PATH];
			LPITEMIDLIST idList;
			LPMALLOC psMalloc;
			BROWSEINFOA bi = {0};

			if(SUCCEEDED(CoGetMalloc(1,&psMalloc)))
			{
				char szTemp[MAX_PATH];
				bi.hwndOwner=hwndDlg;
				bi.pszDisplayName=szDirectory;
				bi.lpszTitle=Translate("Select Folder");
				bi.ulFlags=BIF_NEWDIALOGSTYLE|BIF_EDITBOX|BIF_RETURNONLYFSDIRS;
				bi.lpfn=BrowseCallbackProc;
				bi.lParam=(LPARAM)szDirectory;

				idList=SHBrowseForFolderA(&bi);
				if(idList) {
					SHGetPathFromIDListA(idList,szDirectory);
					lstrcatA(szDirectory,"\\");
				CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)szDirectory, (LPARAM)szTemp );
				SetWindowTextA(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY), lstrlenA(szTemp) > 1?szTemp:"Logs\\");
				}
				psMalloc->lpVtbl->Free(psMalloc,idList);
				psMalloc->lpVtbl->Release(psMalloc);
			}
			break;
		}
		case IDC_CHAT_FONTCOLOR:
		{
		int selItems[ SIZEOF(fontOptionsList) ];
			int sel, selCount, i;

			selCount = SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_GETSELITEMS, sizeof(fontOptionsList) / sizeof(fontOptionsList[0]), (LPARAM) selItems);
			for (sel = 0; sel < selCount; sel++) {
				i = SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTLIST, LB_GETITEMDATA, selItems[sel], 0) - 1;
				fontOptionsList[i].colour = SendDlgItemMessage(hwndDlg, IDC_CHAT_FONTCOLOR, CPM_GETCOLOUR, 0, 0);
			}
			InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_FONTLIST), NULL, FALSE);
			break;
		}
		case IDC_CHAT_HIGHLIGHT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS), IsDlgButtonChecked(hwndDlg, IDC_CHAT_HIGHLIGHT) == BST_CHECKED?TRUE:FALSE);
			break;
		}

		if(lParam != (LPARAM)NULL)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY ) {
			int iLen;
			char * pszText = NULL;
			char * p2 = NULL;

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS));
			if ( iLen > 0 ) {
			pszText = mir_realloc(pszText, iLen+1);
				GetDlgItemTextA(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS, pszText,iLen+1);
				p2 = strchr(pszText, ',');
				while ( p2 ) {
					*p2 = ' ';
					p2 = strchr(pszText, ',');
				}

				DBWriteContactSettingString(NULL, "Chat", "HighlightWords", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "HighlightWords");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY));
			if ( iLen > 0 ) {
			pszText = mir_realloc(pszText, iLen+1);
				GetDlgItemTextA(hwndDlg, IDC_CHAT_LOGDIRECTORY, pszText,iLen+1);
				DBWriteContactSettingString(NULL, "Chat", "LogDirectory", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "LogDirectory");

			CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)pszText, (LPARAM)g_Settings.pszLogDir);

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_LOGTIMESTAMP));
			if ( iLen > 0 ) {
			pszText = mir_realloc(pszText, iLen+1);
				GetDlgItemTextA(hwndDlg, IDC_CHAT_LOGTIMESTAMP, pszText,iLen+1);
				DBWriteContactSettingString(NULL, "Chat", "LogTimestamp", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "LogTimestamp");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_TIMESTAMP));
			if( iLen > 0 ) {
			pszText = mir_realloc(pszText, iLen+1);
				GetDlgItemTextA(hwndDlg, IDC_CHAT_TIMESTAMP, pszText,iLen+1);
				DBWriteContactSettingString(NULL, "Chat", "HeaderTime", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "HeaderTime");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_INSTAMP));
			if ( iLen > 0 ) {
			pszText = mir_realloc(pszText, iLen+1);
				GetDlgItemTextA(hwndDlg, IDC_CHAT_INSTAMP, pszText,iLen+1);
				DBWriteContactSettingString(NULL, "Chat", "HeaderIncoming", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "HeaderIncoming");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_OUTSTAMP));
			if( iLen > 0 ) {
			pszText = mir_realloc(pszText, iLen+1);
				GetDlgItemTextA(hwndDlg, IDC_CHAT_OUTSTAMP, pszText,iLen+1);
				DBWriteContactSettingString(NULL, "Chat", "HeaderOutgoing", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "HeaderOutgoing");

			g_Settings.HighlightEnabled = IsDlgButtonChecked(hwndDlg, IDC_CHAT_HIGHLIGHT) == BST_CHECKED?TRUE:FALSE;
			DBWriteContactSettingByte(NULL, "Chat", "HighlightEnabled", (BYTE)g_Settings.HighlightEnabled);

			g_Settings.LoggingEnabled = IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED?TRUE:FALSE;
			DBWriteContactSettingByte(NULL, "Chat", "LoggingEnabled", (BYTE)g_Settings.LoggingEnabled);
			if ( g_Settings.LoggingEnabled )
				if ( !PathIsDirectoryA( g_Settings.pszLogDir ))
					CreateDirectoryA( g_Settings.pszLogDir, NULL );

			iLen = SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN2,UDM_GETPOS,0,0);
			DBWriteContactSettingWord(NULL, "Chat", "LogLimit", (WORD)iLen);
			iLen = SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN3,UDM_GETPOS,0,0);
			DBWriteContactSettingWord(NULL, "Chat", "LoggingLimit", (WORD)iLen);

			DBWriteContactSettingDword(NULL, "Chat", "ColorLogBG", (DWORD)SendDlgItemMessage(hwndDlg,IDC_CHAT_LOGBKG,CPM_GETCOLOUR,0,0));
			DBWriteContactSettingDword(NULL, "Chat", "ColorMessageBG", (DWORD)SendDlgItemMessage(hwndDlg,IDC_CHAT_MESSAGEBKG,CPM_GETCOLOUR,0,0));
			DBWriteContactSettingDword(NULL, "Chat", "ColorNicklistBG", (DWORD)SendDlgItemMessage(hwndDlg,IDC_CHAT_NICKLISTBKG,CPM_GETCOLOUR,0,0));
		mir_free(pszText);
			if(hEditBkgBrush)
				DeleteObject(hEditBkgBrush);
			if(hListBkgBrush)
				DeleteObject(hEditBkgBrush);
			hEditBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW)));
			hListBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW)));

			{	int i;
				char str[32];
			for (i = 0; i < SIZEOF(fontOptionsList); i++) {
					wsprintfA(str, "Font%d", i);
					DBWriteContactSettingTString(NULL, "ChatFonts", str, fontOptionsList[i].szFace);
					wsprintfA(str, "Font%dSize", i);
					DBWriteContactSettingByte(NULL, "ChatFonts", str, fontOptionsList[i].size);
					wsprintfA(str, "Font%dSty", i);
					DBWriteContactSettingByte(NULL, "ChatFonts", str, fontOptionsList[i].style);
					wsprintfA(str, "Font%dSet", i);
					DBWriteContactSettingByte(NULL, "ChatFonts", str, fontOptionsList[i].charset);
					wsprintfA(str, "Font%dCol", i);
					DBWriteContactSettingDword(NULL, "ChatFonts", str, fontOptionsList[i].colour);
			}	}

			LoadLogFonts();
			FreeMsgLogBitmaps();
			LoadMsgLogBitmaps();
			{
				LOGFONT lf;
				HFONT hFont;
				int iText;

				Chat_LoadMsgDlgFont(0, &lf, NULL);
				hFont = CreateFontIndirect(&lf);
				iText = GetTextPixelSize(MakeTimeStamp(g_Settings.pszTimeStamp, time(NULL)),hFont, TRUE);
				DeleteObject(hFont);
				g_Settings.LogTextIndent = iText;
				g_Settings.LogTextIndent = g_Settings.LogTextIndent*12/10;
				g_Settings.LogIndentEnabled = (DBGetContactSettingByte(NULL, "Chat", "LogIndentEnabled", 1) != 0)?TRUE:FALSE;
			}
			MM_FontsChanged();
			MM_FixColors();
			SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
			return TRUE;
		}
		break;

	case WM_DESTROY:
		DeleteObject(hBkgColourBrush);
		DeleteObject(hListColourBrush);
		DeleteObject(hMessageColourBrush);
		break;
	}
	return FALSE;
}

static BOOL CALLBACK DlgProcOptionsPopup(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
		TranslateDialogDefault(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_CHAT_BKG, CPM_SETCOLOUR,0,g_Settings.crPUBkgColour);
		SendDlgItemMessage(hwndDlg, IDC_CHAT_TEXT, CPM_SETCOLOUR,0,g_Settings.crPUTextColour);

		if(g_Settings.iPopupStyle ==2)
			CheckDlgButton(hwndDlg, IDC_CHAT_RADIO2, BST_CHECKED);
		else if(g_Settings.iPopupStyle ==3)
			CheckDlgButton(hwndDlg, IDC_CHAT_RADIO3, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHAT_RADIO1, BST_CHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_BKG), IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO3) ==BST_CHECKED?TRUE:FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_TEXT), IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO3) ==BST_CHECKED?TRUE:FALSE);

		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN1,UDM_SETRANGE,0,MAKELONG(100,-1));
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN1,UDM_SETPOS,0,MAKELONG(g_Settings.iPopupTimeout,0));
		}break;

	case WM_COMMAND:
		if(	(LOWORD(wParam)		  == IDC_CHAT_TIMEOUT)
				&& (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))	return 0;

		if(lParam != (LPARAM)NULL)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		switch (LOWORD(wParam)) {

		case IDC_CHAT_RADIO1:
		case IDC_CHAT_RADIO2:
		case IDC_CHAT_RADIO3:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_BKG), IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO3) ==BST_CHECKED?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_TEXT), IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO3) ==BST_CHECKED?TRUE:FALSE);
			break;

		default:break;
		}

		break;
	case WM_NOTIFY:
	{
		switch(((LPNMHDR)lParam)->idFrom)
		{
			case 0:
				switch (((LPNMHDR)lParam)->code)
				{
				case PSN_APPLY:
					{
						int iLen;

						if(IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO2) == BST_CHECKED)
							iLen = 2;
						else if(IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO3) == BST_CHECKED)
							iLen = 3;
						else
							iLen = 1;

						g_Settings.iPopupStyle = iLen;
						DBWriteContactSettingByte(NULL, "Chat", "PopupStyle", (BYTE)iLen);

						iLen = SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN1,UDM_GETPOS,0,0);
						g_Settings.iPopupTimeout = iLen;
						DBWriteContactSettingWord(NULL, "Chat", "PopupTimeout", (WORD)iLen);

						g_Settings.crPUBkgColour = SendDlgItemMessage(hwndDlg,IDC_CHAT_BKG,CPM_GETCOLOUR,0,0);
						DBWriteContactSettingDword(NULL, "Chat", "PopupColorBG", (DWORD)SendDlgItemMessage(hwndDlg,IDC_CHAT_BKG,CPM_GETCOLOUR,0,0));
						g_Settings.crPUTextColour = SendDlgItemMessage(hwndDlg,IDC_CHAT_TEXT,CPM_GETCOLOUR,0,0);
						DBWriteContactSettingDword(NULL, "Chat", "PopupColorText", (DWORD)SendDlgItemMessage(hwndDlg,IDC_CHAT_TEXT,CPM_GETCOLOUR,0,0));

					}
					return TRUE;
				}
		}
	}break;

	default:break;
	}
	return FALSE;
}

static int OptionsInitialize(WPARAM wParam, LPARAM lParam)
{

	OPTIONSDIALOGPAGE odp = {0};
	if(PopUpInstalled)
	{
		odp.cbSize = sizeof(odp);
		odp.position = 910000002;
		odp.hInstance = g_hInst;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONSPOPUP);
		odp.ptszTitle = _T("Chat");
		odp.ptszGroup = _T("Popups");
		odp.pfnDlgProc = DlgProcOptionsPopup;
		odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
		odp.ptszTab = NULL;
		CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	}

	return 0;
}

void LoadGlobalSettings(void)
{
	LOGFONT lf;

	g_Settings.LogLimitNames = DBGetContactSettingByte(NULL, "Chat", "LogLimitNames", 1);
	g_Settings.ShowTime = DBGetContactSettingByte(NULL, "Chat", "ShowTimeStamp", 1);
	g_Settings.SoundsFocus = DBGetContactSettingByte(NULL, "Chat", "SoundsFocus", 0);
	g_Settings.ShowTimeIfChanged = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowTimeStampIfChanged", 0);
	g_Settings.TimeStampEventColour = (BOOL)DBGetContactSettingByte(NULL, "Chat", "TimeStampEventColour", 0);
	g_Settings.iEventLimit = DBGetContactSettingWord(NULL, "Chat", "LogLimit", 100);
	g_Settings.dwIconFlags = DBGetContactSettingDword(NULL, "Chat", "IconFlags", 0x0000);
	g_Settings.dwTrayIconFlags = DBGetContactSettingDword(NULL, "Chat", "TrayIconFlags", 0x1000);
	g_Settings.dwPopupFlags = DBGetContactSettingDword(NULL, "Chat", "PopupFlags", 0x0000);
	g_Settings.LoggingLimit = DBGetContactSettingWord(NULL, "Chat", "LoggingLimit", 100);
	g_Settings.LoggingEnabled = (BOOL)DBGetContactSettingByte(NULL, "Chat", "LoggingEnabled", 0);
	g_Settings.FlashWindow = (BOOL)DBGetContactSettingByte(NULL, "Chat", "FlashWindow", 0);
	g_Settings.HighlightEnabled = (BOOL)DBGetContactSettingByte(NULL, "Chat", "HighlightEnabled", 1);
	g_Settings.crUserListColor = (BOOL)DBGetContactSettingDword(NULL, "ChatFonts", "Font18Col", RGB(0,0,0));
	g_Settings.crUserListBGColor = (BOOL)DBGetContactSettingDword(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW));
	g_Settings.crUserListHeadingsColor = (BOOL)DBGetContactSettingDword(NULL, "ChatFonts", "Font19Col", RGB(170,170,170));
	g_Settings.crLogBackground = (BOOL)DBGetContactSettingDword(NULL, "Chat", "ColorLogBG", GetSysColor(COLOR_WINDOW));
	g_Settings.StripFormat = (BOOL)DBGetContactSettingByte(NULL, "Chat", "StripFormatting", 0);
	g_Settings.TrayIconInactiveOnly = (BOOL)DBGetContactSettingByte(NULL, "Chat", "TrayIconInactiveOnly", 1);
	g_Settings.PopUpInactiveOnly = (BOOL)DBGetContactSettingByte(NULL, "Chat", "PopUpInactiveOnly", 1);
	g_Settings.AddColonToAutoComplete = (BOOL)DBGetContactSettingByte(NULL, "Chat", "AddColonToAutoComplete", 1);
	g_Settings.iPopupStyle = DBGetContactSettingByte(NULL, "Chat", "PopupStyle", 1);
	g_Settings.iPopupTimeout = DBGetContactSettingWord(NULL, "Chat", "PopupTimeout", 3);
	g_Settings.crPUBkgColour = DBGetContactSettingDword(NULL, "Chat", "PopupColorBG", GetSysColor(COLOR_WINDOW));
	g_Settings.crPUTextColour = DBGetContactSettingDword(NULL, "Chat", "PopupColorText", 0);

	InitSetting( &g_Settings.pszTimeStamp, "HeaderTime", _T("[%H:%M]"));
	InitSetting( &g_Settings.pszTimeStampLog, "LogTimestamp", _T("[%d %b %y %H:%M]"));
	InitSetting( &g_Settings.pszIncomingNick, "HeaderIncoming", _T("%n:"));
	InitSetting( &g_Settings.pszOutgoingNick, "HeaderOutgoing", _T("%n:"));
	InitSetting( &g_Settings.pszHighlightWords, "HighlightWords", _T("%m"));

	{
		char pszTemp[MAX_PATH];
		DBVARIANT dbv;
		g_Settings.pszLogDir = (char *)mir_realloc(g_Settings.pszLogDir, MAX_PATH);
		if (!DBGetContactSetting(NULL, "Chat", "LogDirectory", &dbv) && dbv.type == DBVT_ASCIIZ)
		{
			lstrcpynA(pszTemp, dbv.pszVal, MAX_PATH);
			DBFreeVariant(&dbv);
		}
		else lstrcpynA(pszTemp, "Logs\\", MAX_PATH);

		CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)pszTemp, (LPARAM)g_Settings.pszLogDir);
	}

	g_Settings.LogIndentEnabled = (DBGetContactSettingByte(NULL, "Chat", "LogIndentEnabled", 1) != 0)?TRUE:FALSE;

	if(g_Settings.MessageBoxFont)
		DeleteObject(g_Settings.MessageBoxFont);
	Chat_LoadMsgDlgFont(17, &lf, NULL);
	g_Settings.MessageBoxFont = CreateFontIndirect(&lf);

	if(g_Settings.UserListFont)
		DeleteObject(g_Settings.UserListFont);
	Chat_LoadMsgDlgFont(18, &lf, NULL);
	g_Settings.UserListFont = CreateFontIndirect(&lf);

	if(g_Settings.UserListHeadingsFont)
		DeleteObject(g_Settings.UserListHeadingsFont);
	Chat_LoadMsgDlgFont(19, &lf, NULL);
	g_Settings.UserListHeadingsFont = CreateFontIndirect(&lf);
}

static void FreeGlobalSettings(void)
{
	mir_free(g_Settings.pszTimeStamp);
	mir_free(g_Settings.pszTimeStampLog);
	mir_free(g_Settings.pszIncomingNick);
	mir_free(g_Settings.pszOutgoingNick);
	mir_free(g_Settings.pszHighlightWords);
	mir_free(g_Settings.pszLogDir);
	if(g_Settings.MessageBoxFont)
		DeleteObject(g_Settings.MessageBoxFont);
	if(g_Settings.UserListFont)
		DeleteObject(g_Settings.UserListFont);
	if(g_Settings.UserListHeadingsFont)
		DeleteObject(g_Settings.UserListHeadingsFont);
}

int OptionsInit(void)
{
	LOGFONT lf;

	g_hOptions = HookEvent(ME_OPT_INITIALISE, OptionsInitialize);

	LoadLogFonts();
	Chat_LoadMsgDlgFont(18, &lf, NULL);
	lstrcpy(lf.lfFaceName, _T("MS Shell Dlg"));
	lf.lfUnderline = lf.lfItalic = lf.lfStrikeOut = 0;
	lf.lfHeight = -17;
	lf.lfWeight = FW_BOLD;
	g_Settings.NameFont = CreateFontIndirect(&lf);
	g_Settings.UserListFont = NULL;
	g_Settings.UserListHeadingsFont = NULL;
	g_Settings.MessageBoxFont = NULL;
	g_Settings.iSplitterX = DBGetContactSettingWord(NULL, "Chat", "SplitterX", 105);
	g_Settings.iSplitterY = DBGetContactSettingWord(NULL, "Chat", "SplitterY", 90);
	LoadGlobalSettings();

	hEditBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW)));
	hListBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW)));

	SkinAddNewSoundEx("ChatMessage", "Chat", Translate("Incoming message"));
	SkinAddNewSoundEx("ChatHighlight", "Chat", Translate("Message is highlighted"));
	SkinAddNewSoundEx("ChatAction", "Chat", Translate("User has performed an action"));
	SkinAddNewSoundEx("ChatJoin", "Chat", Translate("User has joined"));
	SkinAddNewSoundEx("ChatPart", "Chat", Translate("User has left"));
	SkinAddNewSoundEx("ChatKick", "Chat", Translate("User has kicked some other user"));
	SkinAddNewSoundEx("ChatMode", "Chat", Translate("User´s status was changed"));
	SkinAddNewSoundEx("ChatNick", "Chat", Translate("User has changed name"));
	SkinAddNewSoundEx("ChatNotice", "Chat", Translate("User has sent a notice"));
	SkinAddNewSoundEx("ChatQuit", "Chat", Translate("User has disconnected"));
	SkinAddNewSoundEx("ChatTopic", "Chat", Translate("The topic has been changed"));

	if(g_Settings.LoggingEnabled)
		if(!PathIsDirectoryA(g_Settings.pszLogDir))
			CreateDirectoryA(g_Settings.pszLogDir, NULL);
	{
		LOGFONT lf;
		HFONT hFont;
		int iText;

		Chat_LoadMsgDlgFont(0, &lf, NULL);
		hFont = CreateFontIndirect(&lf);
		iText = GetTextPixelSize(MakeTimeStamp(g_Settings.pszTimeStamp, time(NULL)),hFont, TRUE);
		DeleteObject(hFont);
		g_Settings.LogTextIndent = iText;
		g_Settings.LogTextIndent = g_Settings.LogTextIndent*12/10;
	}

	return 0;
}


int OptionsUnInit(void)
{
	FreeGlobalSettings();
	UnhookEvent(g_hOptions);
	DeleteObject(hEditBkgBrush);
	DeleteObject(hListBkgBrush);
	DeleteObject(g_Settings.NameFont);
	return 0;
}

