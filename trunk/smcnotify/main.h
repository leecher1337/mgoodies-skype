
#include <windows.h>
#include <commctrl.h>

#include <stdio.h>

// Miranda headers
#include <newpluginapi.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_ignore.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_userinfo.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_history.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_button.h>
#include <m_popup.h>
#include <statusmodes.h>
#include <icolib.h>
#include <m_toptoolbar.h>

#include "../utils/mir_dblists.h"
#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"

#include "resource.h"
#include "smcn.h"
#include "m_smcn.h"

// Option struct
typedef struct {
	HINSTANCE hInst;
	// popups
	BOOL bDisablePopUps;
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

	// Notifications
	BOOL bShowOnConnect;
	BOOL bShowOnStatusChange;

	// general settings
	BOOL bHideSettingsMenu;
	BOOL bLogToFile;
	DWORD dHistMax;
	BOOL bShowMsgChanges;
	BOOL bUseBgImage;
	COLORREF colListBack;
	COLORREF colListText;

	// Status pooling
	BOOL pool_check_msgs;
	BOOL pool_check_on_status_change;
	DWORD pool_timer_check;
	DWORD pool_timer_status;

	// strings
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

// Global Variables
HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE hContactSettingChanged;
CLISTMENUITEM menuitem;
HANDLE hMenuitemNotify;
BOOL bNotify;
HANDLE hHookedInit;
HANDLE hHookedOpt;
HANDLE hHookedNewEvent;
HANDLE hHookSkinIconsChanged;
PLUGIN_OPTIONS options;
HANDLE hWindowList;
int hEnableDisableMenu;
int hGoToURLMenu;
int hContactMenu;
int hShowListMenu;
int hContactPopUpsMenu;
int hContactIcqCheckMenu;
HANDLE hPopupContact;
HWND hPopupWindow;
HWND hTimerWindow;
HANDLE hLibIcons[ICONCOUNT];
HANDLE hProtoAck;
HANDLE hTopToolbarLoaded;
HANDLE hTopToolbarButtonShowList;
HANDLE hPreBuildCMenu;
HANDLE hStatusMsgProcess;

// declarations
// main.c
void UpdateMenu(BOOL State);

// options.c
void OptionsRead(void);
void OptionsAdd(WPARAM addInfo);

// history.c
char* BuildSetting(historyLast);
void ShowHistory(HANDLE hContact);
//void InitHistoryDialog(void);

// list.c
//void UpdateContactsStatusMsgList(HWND hwnd, HWND hList, HANDLE hContact);
void ShowList(void);

// popup.c
TCHAR* GetStr(STATUSMSGINFO n, const TCHAR *dis);
LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ShowPopup(STATUSMSGINFO n);
int ProtoAck(WPARAM wParam, LPARAM lParam);


VOID CALLBACK StatusMsgCheckTimerProc(HWND hWnd,UINT uMsg,UINT idEvent,DWORD dwTime);

BOOL HasToGetStatusMsgForProtocol(const char *szProto);
BOOL HasToIgnoreContact(HANDLE hContact, const char* szProto);
void MessageGotForContact(HANDLE hContact);
void StatusChangeAddContact(HANDLE hContact, const char* proto);
