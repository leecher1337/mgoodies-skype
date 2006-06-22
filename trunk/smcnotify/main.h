
#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <commctrl.h>

#include <stdio.h>

//Miranda headers
#include "newpluginapi.h"
#include "m_system.h"
#include "m_protocols.h"
#include "m_protosvc.h"
#include "m_clist.h"
#include "m_ignore.h"
#include "m_contacts.h"
#include "m_message.h"
#include "m_userinfo.h"
#include "m_skin.h"
#include "m_langpack.h"
#include "m_history.h"
#include "m_database.h"
#include "m_options.h"
#include "m_utils.h"
#include "m_button.h"
#include "m_popup.h"
#include "statusmodes.h"

#include "m_icolib.h"
#include "m_toptoolbar.h"

#include "resource.h"
#include "smcn.h"

//Option struct
typedef struct {
	HINSTANCE hInst;
	//popups
	BOOL bDisablePopUps;
	BOOL bShowOnConnect;
	BOOL bOnlyIfChanged;
	BOOL bIgnoreEmptyPopup;
	BOOL bIgnoreEmptyAll;
	BOOL bUseOSD;
	BOOL bDefaultColor;
	COLORREF colBack;
	COLORREF colText;
	DWORD dSec;
	BOOL bInfiniteDelay;
	int LeftClickAction;
	int RightClickAction;
	//general settings
//	BOOL bHideSettingsMenu;
	BOOL bLogToFile;
	DWORD dHistMax;
	BOOL bShowMsgChanges;
	BOOL bUseBgImage;
	COLORREF colListBack;
	COLORREF colListText;
	//strings
	/*TCHAR*/char popuptext[MAXPOPUPLEN];
	/*TCHAR*/char logfile[MAX_PATH];
	/*TCHAR*/char log[MAXSTRLEN];
	/*TCHAR*/char his[MAXSTRLEN];
	/*TCHAR*/char msgcleared[MAXSTRLEN];
	/*TCHAR*/char msgchanged[MAXSTRLEN];
	/*TCHAR*/char listbgimage[MAX_PATH];
} PLUGIN_OPTIONS;

typedef struct {
	HANDLE hContact;
	TCHAR *cust;
	TCHAR *oldstatusmsg;
	TCHAR *newstatusmsg;
	TCHAR *proto;
	BOOL bIsEmpty;
	DWORD dTimeStamp;
} STATUSMSGINFO;

typedef struct {
	HANDLE hContact;
	int bSortAscending;
	int iLastColumnSortIndex;
	int iProtoSort;
} LVDLGDAT;

typedef struct {
	HANDLE hContact;
	int iiProto;
	int iStatus;
	char *nick;
	char *proto;
} LVITEMDAT;

//Global Variables
HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE hContactSettingChanged;
//CLISTMENUITEM menuitem;
//HANDLE hMenuitemNotify;
//BOOL bNotify;
HANDLE hHookedInit;
HANDLE hHookedOpt;
//HANDLE hHookedNewEvent;
HANDLE hHookSkinIconsChanged;
PLUGIN_OPTIONS options;
HANDLE hWindowList;
int hEnableDisableMenu;
int hGoToURLMenu;
int hContactMenu;
int hShowListMenu;
int hContactPopUpsMenu;
HANDLE hPopupContact;
HWND hPopupWindow;
HANDLE hLibIcons[ICONCOUNT];
HANDLE hProtoAck;
HANDLE hTopToolbarLoaded;
HANDLE hTopToolbarButtonShowList;
HANDLE hUserInfoInitialise;
HANDLE hPreBuildCMenu;
//HANDLE hStatusMsgProcess;

//declarations
// main.c
void UpdateMenu(BOOL State);

// options.c
void OptionsRead(void);
void OptionsAdd(WPARAM addInfo);

// history.c
char* BuildSetting(short historyLast, BOOL bTS);
int HookedUserInfo(WPARAM wParam, LPARAM lParam);
void ShowHistory(HANDLE hContact);

// list.c
//void UpdateContactsStatusMsgList(HWND hwnd, HWND hList, HANDLE hContact);
void ShowList(void);

// popup.c
LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ShowPopup(STATUSMSGINFO *n);

// utils.c
TCHAR* GetStr(STATUSMSGINFO *n, const TCHAR *dis);
int ProtoAck(WPARAM wParam, LPARAM lParam);

#endif // MAIN_H