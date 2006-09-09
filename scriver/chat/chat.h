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
#if defined(UNICODE)
#define _UNICODE
#endif

#ifndef _CHAT_H_
#define _CHAT_H_

#pragma warning( disable : 4786 ) // limitation in MSVC's debugger.
#pragma warning( disable : 4996 ) // limitation in MSVC's debugger.

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501

#include <tchar.h>
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <process.h>
#include <ole2.h>
#include <richole.h>
#include <malloc.h>
#include <commdlg.h>
#include <time.h>
#include <stdio.h>
#include <shellapi.h>
#include <win2k.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_options.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_button.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_addcontact.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_popup.h>
#include "resource.h"
#include "m_chat.h"
#include "m_ieview.h"
#include "m_smileyadd.h"
#include "IcoLib.h"
//#include "richutil.h"

#ifndef TVM_GETITEMSTATE
#define TVM_GETITEMSTATE        (TV_FIRST + 39)
#endif

#ifndef TreeView_GetItemState
#define TreeView_GetItemState(hwndTV, hti, mask) \
   (UINT)SNDMSG((hwndTV), TVM_GETITEMSTATE, (WPARAM)(hti), (LPARAM)(mask))
#endif



#ifndef NDEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#ifndef CFM_BACKCOLOR
#define CFM_BACKCOLOR		0x04000000
#endif

//defines
#define OPTIONS_FONTCOUNT 17
#define GC_UPDATETITLE			(WM_USER+100)
#define GC_SPLITTERMOVED		(WM_USER+101)
#define GC_CLOSEWINDOW			(WM_USER+103)
#define GC_GETITEMDATA			(WM_USER+104)
#define GC_SETITEMDATA			(WM_USER+105)
#define GC_SETVISIBILITY		(WM_USER+107)
#define GC_SETWNDPROPS			(WM_USER+108)
#define GC_REDRAWLOG			(WM_USER+109)
#define GC_FIREHOOK				(WM_USER+110)
#define GC_FILTERFIX			(WM_USER+111)
#define GC_CHANGEFILTERFLAG		(WM_USER+112)
#define GC_SHOWFILTERMENU		(WM_USER+113)
#define GC_SETWINDOWPOS			(WM_USER+114)
#define GC_SAVEWNDPOS			(WM_USER+115)
//#define	GC_NICKLISTCLEAR		(WM_USER+117)
#define GC_REDRAWWINDOW			(WM_USER+118)
#define GC_SHOWCOLORCHOOSER		(WM_USER+119)
#define GC_ADDLOG				(WM_USER+120)
#define GC_ACKMESSAGE			(WM_USER+121)
//#define GC_ADDUSER				(WM_USER+122)
//#define GC_REMOVEUSER			(WM_USER+123)
//#define GC_NICKCHANGE			(WM_USER+124)
#define GC_UPDATENICKLIST		(WM_USER+125)
//#define GC_MODECHANGE			(WM_USER+126)
#define GC_TABCHANGE			(WM_USER+127)
#define GC_ADDTAB				(WM_USER+128)
#define GC_SCROLLTOBOTTOM		(WM_USER+129)
#define GC_REMOVETAB			(WM_USER+130)
#define GC_SESSIONNAMECHANGE	(WM_USER+131)
#define GC_FIXTABICONS			(WM_USER+132)
#define GC_DROPPEDTAB			(WM_USER+133)
#define GC_TABCLICKED			(WM_USER+134)
#define GC_SWITCHNEXTTAB		(WM_USER+135)
#define GC_SWITCHPREVTAB		(WM_USER+136)
#define GC_SWITCHTAB			(WM_USER+137)
#define GC_SETTABHIGHLIGHT		(WM_USER+138)
#define GC_SETMESSAGEHIGHLIGHT	(WM_USER+139)
#define GC_REDRAWLOG2			(WM_USER+140)
#define GC_REDRAWLOG3			(WM_USER+141)

#define EM_SUBCLASSED			(WM_USER+200)
#define EM_UNSUBCLASSED			(WM_USER+201)
#define EM_ACTIVATE				(WM_USER+202)

#define TIMERID_FLASHWND		205

#define GCW_TABROOM				10
#define GCW_TABPRIVMSG			11

#define GC_EVENT_HIGHLIGHT		0x1000
#define STATE_TALK				0x0001

#define ICON_ACTION				0
#define ICON_ADDSTATUS			1
#define ICON_HIGHLIGHT			2
#define ICON_INFO				3
#define ICON_JOIN				4
#define ICON_KICK				5
#define ICON_MESSAGE			6
#define ICON_MESSAGEOUT			7
#define ICON_NICK				8
#define ICON_NOTICE				9
#define ICON_PART				10
#define ICON_QUIT				11
#define ICON_REMSTATUS			12
#define ICON_TOPIC				13

#define ICON_STATUS1			14
#define ICON_STATUS2			15
#define ICON_STATUS3			16
#define ICON_STATUS4			17
#define ICON_STATUS0			18
#define ICON_STATUS5			19

// special service for tweaking performance
#define MS_GC_GETEVENTPTR  "GChat/GetNewEventPtr"
typedef int (*GETEVENTFUNC)(WPARAM wParam, LPARAM lParam);
typedef struct  {
	GETEVENTFUNC pfnAddEvent;
}GCPTRS;

//structs

typedef struct  MODULE_INFO_TYPE{
	char *		pszModule;
	char *		pszModDispName;
	char *		pszHeader;
	BOOL		bBold;
	BOOL		bUnderline;
	BOOL		bItalics;
	BOOL		bColor;
	BOOL		bBkgColor;
	BOOL		bChanMgr;
	BOOL		bAckMsg;
	int			nColorCount;
	COLORREF*	crColors;
	HICON		hOnlineIcon;
	HICON		hOfflineIcon;
	HICON		hOnlineTalkIcon;
	HICON		hOfflineTalkIcon;
	int			OnlineIconIndex;
	int			OfflineIconIndex;
	int			iMaxText;
	struct MODULE_INFO_TYPE *next;
}MODULEINFO;

typedef struct COMMAND_INFO_TYPE
{
	char *lpCommand;
	struct COMMAND_INFO_TYPE *last, *next;
} COMMAND_INFO;

typedef struct{
	LOGFONT	lf;
	COLORREF color;
}FONTINFO;

typedef struct  LOG_INFO_TYPE{
	char *		pszText;
	char *		pszNick;
	char *		pszUID;
	char *		pszStatus;
	char *		pszUserInfo;
	BOOL		bIsMe;
	BOOL		bIsHighlighted;
	time_t		time;
	int			iType;
	struct LOG_INFO_TYPE *next;
	struct LOG_INFO_TYPE *prev;
}LOGINFO;

typedef struct STATUSINFO_TYPE{
	char *		pszGroup;
	HICON		hIcon;
	WORD		Status;
	struct STATUSINFO_TYPE *next;
}STATUSINFO;


typedef struct  USERINFO_TYPE{
	char *		pszNick;
	char *		pszUID;
	WORD 		Status;
	int			iStatusEx;
	struct USERINFO_TYPE *next;

}USERINFO;

typedef struct  TABLIST_TYPE{
	char *		pszID;
	char *		pszModule;
	struct TABLIST_TYPE *next;
} TABLIST;

typedef struct SESSION_INFO_TYPE
{
	HWND			hWnd;

	BOOL			bFGSet;
	BOOL			bBGSet;
	BOOL			bFilterEnabled;
	BOOL			bNicklistEnabled;
	BOOL			bInitDone;

	char *			pszID;
	char *			pszModule;
	char *			pszName;
	char *			pszStatusbarText;
	char *			pszTopic;

	int				iType;
	int				iFG;
	int				iBG;
	int				iSplitterY;
	int				iSplitterX;
	int				iLogFilterFlags;
	int				nUsersInNicklist;
	int				iEventCount;
	int				iX;
	int				iY;
	int				iWidth;
	int				iHeight;
	int				iStatusCount;

	WORD			wStatus;
	WORD			wState;
	WORD			wCommandsNum;
	DWORD			dwItemData;
	HANDLE			hContact;
	HWND			hwndStatus;
	time_t			LastTime;

	COMMAND_INFO *	lpCommands;
	COMMAND_INFO *	lpCurrentCommand;
	LOGINFO *		pLog;
	LOGINFO *		pLogEnd;
	USERINFO *		pUsers;
	USERINFO*		pMe;
	STATUSINFO *	pStatuses;
	struct SESSION_INFO_TYPE *next;
} SESSION_INFO;

typedef struct {
    char *	buffer;
    int		bufferOffset, bufferLen;
	HWND	hwnd;
	LOGINFO* lin;
	BOOL	bStripFormat;
	BOOL	bRedraw;
	SESSION_INFO * si;
} LOGSTREAMDATA;


struct CREOleCallback {
	IRichEditOleCallbackVtbl *lpVtbl;
	unsigned refCount;
	IStorage *pictStg;
	int nextStgId;
};
/*
typedef struct  {
	BOOL		bFilterEnabled;
	BOOL		bFGSet;
	BOOL		bBGSet;
	int			nUsersInNicklist;
	int			iLogFilterFlags;
	int			iType;
	char *		pszModule;
	char *		pszID;
	char *		pszName;
	char *		pszStatusbarText;
	char *		pszTopic;
	USERINFO*	pMe;
	int			iSplitterY;
	int			iSplitterX;
	int			iFG;
	int			iBG;
	time_t		LastTime;
	LPARAM		ItemData;
//	STATUSINFO* pStatusList;
//	USERINFO*	pUserList;
//	LOGINFO*	pEventListStart;
//	LOGINFO*	pEventListEnd;
	UINT		iEventCount;
	HWND		hwndStatus;
	HANDLE		hContact;

}CHATWNDDATA;
*/



struct GlobalLogSettings_t {
	BOOL		ShowTime;
    BOOL		ShowTimeIfChanged;
	BOOL		LoggingEnabled;
	BOOL		FlashWindow;
	BOOL		HighlightEnabled;
	BOOL		LogIndentEnabled;
	BOOL		StripFormat;
	BOOL		SoundsFocus;
	BOOL		PopUpInactiveOnly;
	BOOL		TrayIconInactiveOnly;
	BOOL		AddColonToAutoComplete;
	BOOL		TabsEnable;
	BOOL		TabCloseOnDblClick;
	BOOL		TabRestore;
	BOOL		LogLimitNames;
	BOOL		TabsAtBottom;
	BOOL		TimeStampEventColour;
	DWORD		dwIconFlags;
	DWORD		dwTrayIconFlags;
	DWORD		dwPopupFlags;
	int			LogTextIndent;
	int			LoggingLimit;
	int			iEventLimit;
	int			iPopupStyle;
	int			iPopupTimeout;
	int			iSplitterX;
	int			iSplitterY;
	int			iX;
	int			iY;
	int			iWidth;
	int			iHeight;
	char *		pszTimeStamp;
	char *		pszTimeStampLog;
	char *		pszIncomingNick;
	char *		pszOutgoingNick;
	char *		pszHighlightWords;
	char *		pszLogDir;
	HFONT		UserListFont;
	HFONT		UserListHeadingsFont;
	HFONT		MessageBoxFont;
	HFONT		NameFont;
	COLORREF	crLogBackground;
	COLORREF	crUserListColor;
	COLORREF	crUserListBGColor;
	COLORREF	crUserListHeadingsColor;
	COLORREF	crPUTextColour;
	COLORREF	crPUBkgColour;
};
extern struct GlobalLogSettings_t g_Settings;

typedef struct{
  MODULEINFO* pModule;
  int xPosition;
  int yPosition;
  HWND hWndTarget;
  BOOL bForeground;
  SESSION_INFO * si;
}COLORCHOOSER;

//main.c
void				LoadIcons(void);
void				LoadLogIcons(void);
void				FreeIcons(void);
void				UpgradeCheck(void);

//colorchooser.c
BOOL CALLBACK		DlgProcColorToolWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

//log.c
void				Log_StreamInEvent(HWND hwndDlg, LOGINFO* lin, SESSION_INFO* si, BOOL bRedraw, BOOL bPhaseTwo);
void				LoadMsgLogBitmaps(void);
void				FreeMsgLogBitmaps(void);
void				ValidateFilename (char * filename);
char *				MakeTimeStamp(char * pszStamp, time_t time);
char *				Log_CreateRtfHeader(MODULEINFO * mi);

//window.c
BOOL CALLBACK		RoomWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int					GetTextPixelSize(char * pszText, HFONT hFont, BOOL bWidth);

//options.c
int					OptionsInit(void);
int					OptionsUnInit(void);
void				LoadMsgDlgFont(int i, LOGFONT * lf, COLORREF * colour);
void				LoadGlobalSettings(void);
void				AddIcons(void);
HICON				LoadIconEx(int iIndex, char * pszIcoLibName, int iX, int iY);

//services.c
void				HookEvents(void);
void				UnhookEvents(void);
void				CreateServiceFunctions(void);
void           DestroyServiceFunctions(void);
void				CreateHookableEvents(void);
void				TabsInit(void);
int					ModulesLoaded(WPARAM wParam,LPARAM lParam);
int					SmileyOptionsChanged(WPARAM wParam,LPARAM lParam);
int					PreShutdown(WPARAM wParam,LPARAM lParam);
int					Chat_IconsChanged(WPARAM wParam,LPARAM lParam);
void				ShowRoom(SESSION_INFO * si, WPARAM wp, BOOL bSetForeground);
int					Service_Register(WPARAM wParam, LPARAM lParam);
int					Service_AddEvent(WPARAM wParam, LPARAM lParam);
int					Service_GetAddEventPtr(WPARAM wParam, LPARAM lParam);
int					Service_NewChat(WPARAM wParam, LPARAM lParam);
int					Service_ItemData(WPARAM wParam, LPARAM lParam);
int					Service_SetSBText(WPARAM wParam, LPARAM lParam);
int					Service_SetVisibility(WPARAM wParam, LPARAM lParam);
int					Service_GetCount(WPARAM wParam,LPARAM lParam);
int					Service_GetInfo(WPARAM wParam,LPARAM lParam);

//manager.c
void				SetActiveSession(char * pszID, char * pszModule);
void				SetActiveSessionEx(SESSION_INFO * si);
SESSION_INFO *		GetActiveSession(void);
SESSION_INFO *		SM_AddSession(char * pszID, char * pszModule);
int					SM_RemoveSession(char * pszID, char * pszModule);
SESSION_INFO *		SM_FindSession(char *pszID, char * pszModule);
USERINFO *			SM_AddUser(char *pszID, char * pszModule, char * pszUID, char * pszNick, WORD wStatus);
BOOL				SM_ChangeUID(char *pszID, char * pszModule, char * pszUID, char * pszNewUID);
BOOL				SM_ChangeNick(char *pszID, char * pszModule, GCEVENT * gce);
BOOL				SM_RemoveUser(char *pszID, char * pszModule, char * pszUID);
BOOL				SM_SetOffline(char *pszID, char * pszModule);
BOOL				SM_SetTabbedWindowHwnd(SESSION_INFO * si, HWND hwnd);
HICON				SM_GetStatusIcon(SESSION_INFO * si, USERINFO * ui);
BOOL				SM_SetStatus(char *pszID, char * pszModule, int wStatus);
BOOL				SM_SetStatusEx(char *pszID, char * pszModule, char * pszText, int onlyMe );
BOOL				SM_SendUserMessage(char *pszID, char * pszModule, char * pszText);
STATUSINFO *		SM_AddStatus(char *pszID, char * pszModule, char * pszStatus);
SESSION_INFO *		SM_GetNextWindow(SESSION_INFO * si);
SESSION_INFO *		SM_GetPrevWindow(SESSION_INFO * si);
BOOL				SM_AddEventToAllMatchingUID(GCEVENT * gce);
BOOL				SM_AddEvent(char *pszID, char * pszModule, GCEVENT * gce, BOOL bIsHighlighted);
LRESULT				SM_SendMessage(char *pszID, char * pszModule, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL				SM_PostMessage(char *pszID, char * pszModule, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL				SM_BroadcastMessage(char * pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync);
BOOL				SM_RemoveAll (void);
BOOL				SM_GiveStatus(char *pszID, char * pszModule, char * pszUID,  char * pszStatus);
BOOL				SM_TakeStatus(char *pszID, char * pszModule, char * pszUID,  char * pszStatus);
BOOL				SM_MoveUser(char *pszID, char * pszModule, char * pszUID);
void				SM_AddCommand(char *pszID, char * pszModule, const char *lpNewCommand);
char *				SM_GetPrevCommand(char *pszID, char * pszModule);
char *				SM_GetNextCommand(char *pszID, char * pszModule);
int					SM_GetCount(char * pszModule);
SESSION_INFO *		SM_FindSessionByIndex(char * pszModule, int iItem);
char *				SM_GetUsers(SESSION_INFO * si);
USERINFO *			SM_GetUserFromIndex(char *pszID, char * pszModule, int index);
MODULEINFO *		MM_AddModule(char* pszModule);
MODULEINFO	*		MM_FindModule(char * pszModule);
void				MM_FixColors();
void				MM_FontsChanged(void);
void				MM_IconsChanged(void);
BOOL				MM_RemoveAll (void);
BOOL 				TabM_AddTab(char * pszID, char * pszModule);
BOOL				TabM_RemoveAll (void);
STATUSINFO *		TM_AddStatus(STATUSINFO** ppStatusList, char * pszStatus, int * iCount);
STATUSINFO *		TM_FindStatus(STATUSINFO* pStatusList, char* pszStatus);
WORD				TM_StringToWord(STATUSINFO* pStatusList, char* pszStatus);
char *				TM_WordToString(STATUSINFO* pStatusList, WORD Status);
BOOL				TM_RemoveAll (STATUSINFO** pStatusList);
BOOL				UM_SetStatusEx(USERINFO* pUserList,char* pszText, int onlyMe );
USERINFO*			UM_AddUser(STATUSINFO* pStatusList, USERINFO** pUserList, char * pszUID, char * pszNick, WORD wStatus);
USERINFO *			UM_SortUser(USERINFO** ppUserList, char * pszUID);
USERINFO*			UM_FindUser(USERINFO* pUserList, char* pszUID);
USERINFO*			UM_FindUserFromIndex(USERINFO* pUserList, int index);
USERINFO*			UM_GiveStatus(USERINFO* pUserList, char* pszUID, WORD status);
USERINFO*			UM_TakeStatus(USERINFO* pUserList, char* pszUID, WORD status);
char*				UM_FindUserAutoComplete(USERINFO* pUserList, char * pszOriginal, char* pszCurrent);
BOOL				UM_RemoveUser(USERINFO** pUserList, char* pszUID);
BOOL				UM_RemoveAll (USERINFO** ppUserList);
LOGINFO *			LM_AddEvent(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd);
BOOL				LM_TrimLog(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd, int iCount);
BOOL				LM_RemoveAll (LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd);

//clist.c
HANDLE				CList_AddRoom(char * pszModule, char * pszRoom, char * pszDisplayName, int  iType);
BOOL				CList_SetOffline(HANDLE hContact, BOOL bHide);
BOOL				CList_SetAllOffline(BOOL bHide);
int					CList_RoomDoubleclicked(WPARAM wParam,LPARAM lParam);
int					CList_EventDoubleclicked(WPARAM wParam,LPARAM lParam);
void				CList_CreateGroup(char *group);
BOOL				CList_AddEvent(HANDLE hContact, HICON Icon, HANDLE event, int type, char * fmt, ... ) ;
HANDLE				CList_FindRoom (char * pszModule, char * pszRoom) ;
int					WCCmp(char* wild, char *string);

//tools.c
char *				RemoveFormatting(char * pszText);
BOOL				DoSoundsFlashPopupTrayStuff(SESSION_INFO * si, GCEVENT * gce, BOOL bHighlight, int bManyFix);
int					GetColorIndex(char * pszModule, COLORREF cr);
void				CheckColorsInModule(char * pszModule);
char*				my_strstri(char *s1, char *s2) ;
int					GetRichTextLength(HWND hwnd);
BOOL				IsHighlighted(SESSION_INFO * si, char * pszText);
UINT				CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO * si, char * pszUID, char * pszWordText);
void				DestroyGCMenu(HMENU *hMenu, int iIndex);
BOOL				DoEventHookAsync(HWND hwnd, char * pszID, char * pszModule, int iType, char * pszUID, char * pszText, DWORD dwItem);
BOOL				DoEventHook(char * pszID, char * pszModule, int iType, char * pszUID, char * pszText, DWORD dwItem);
BOOL				LogToFile(SESSION_INFO * si, GCEVENT * gce);

// message.c
char *				Message_GetFromStream(HWND hwndDlg, SESSION_INFO* si);
BOOL				DoRtfToTags(char * pszText, SESSION_INFO * si);

#pragma comment(lib,"comctl32.lib")

#endif
