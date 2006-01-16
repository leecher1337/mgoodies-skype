/*
  Name: NewGenerationNotify - Plugin for Miranda ICQ
  File: main.c - Main DLL procedures
  Version: 0.0.4
  Description: Notifies you about some events
  Author: prezes, <prezesso@klub.chip.pl>
  Date: 01.09.04 / Update: 12.05.05 17:00
  Copyright: (C) 2002 Starzinger Michael

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//---------------------------
//---Includes

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>
#include "resource.h"
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_contacts.h>
#include <m_popup.h>

//VERY_PUBLIC Begin ... will be moved to m_neweventnotify.h
#define MS_NGN_MENUNOTIFY "NewGenerationNotify/MenuitemNotifyCommand"
//VERY_PUBLIC End

#define MS_MSG_MOD_MESSAGEDIALOGOPENED "SRMsg_MOD/MessageDialogOpened"

//---------------------------
//---Debuging code (see main.c)

int DebPrint(WPARAM wParam);

//---------------------------
//---Internal Hooks (see main.c)
//---(Workaround till CallServiceSync is available)

#define ME_NGN_WORKAROUND "NewGenerationNotify/_CallService"
int _Workaround_CallService(const char *name, WPARAM wParam, LPARAM lParam);

//---------------------------
//---Definitions

#define MODULE "NewGenerationNotify"
#define VER_MAJOR 1
#define VER_MINOR 9
#define VER_BUILD 0
#define MAX_POPUPS 20

#define DEFAULT_COLBACK RGB(255,255,128)
#define DEFAULT_COLTEXT RGB(0,0,0)
#define DEFAULT_MASKNOTIFY (MASK_MESSAGE|MASK_URL|MASK_FILE|MASK_OTHER)
#define DEFAULT_MASKACTL (MASK_OPEN|MASK_REMOVE|MASK_DISMISS)
#define DEFAULT_MASKACTR (MASK_REMOVE|MASK_DISMISS)
#define DEFAULT_MASKACTE (MASK_DISMISS)
#define DEFAULT_DELAY -1

#define MASK_MESSAGE    0x0001
#define MASK_URL        0x0002
#define MASK_FILE       0x0004
#define MASK_OTHER      0x0008

#define MASK_DISMISS    0x0001
#define MASK_OPEN       0x0002
#define MASK_REMOVE     0x0004

#define SETTING_LIFETIME_MIN		1
#define SETTING_LIFETIME_MAX		60
#define SETTING_LIFETIME_DEFAULT	4

#define MAX_DATASIZE	24
#define METACONTACTS_MODULE	"MetaContacts"
#define	METACONTACTS_HANDLE	"Handle"
#ifndef WM_MOUSEWHEEL 
	#define WM_MOUSEWHEEL 0x020A
#endif
#define WM_UPDATETEXT (WM_USER + 0x0400)
#define WM_POPUPACTION (WM_USER + 0x0401)

#define TIMER_TO_ACTION 50685

//Entries in the database, don't translate
#define OPT_DISABLE "Disabled"
#define OPT_PREVIEW "Preview"
#define OPT_MENUITEM "MenuItem"
#define OPT_COLDEFAULT_MESSAGE "DefaultColorMsg"
#define OPT_COLBACK_MESSAGE "ColorBackMsg"
#define OPT_COLTEXT_MESSAGE "ColorTextMsg"
#define OPT_COLDEFAULT_URL "DefaultColorUrl"
#define OPT_COLBACK_URL "ColorBackUrl"
#define OPT_COLTEXT_URL "ColorTextUrl"
#define OPT_COLDEFAULT_FILE "DefaultColorFile"
#define OPT_COLBACK_FILE "ColorBackFile"
#define OPT_COLTEXT_FILE "ColorTextFile"
#define OPT_COLDEFAULT_OTHERS "DefaultColorOthers"
#define OPT_COLBACK_OTHERS "ColorBackOthers"
#define OPT_COLTEXT_OTHERS "ColorTextOthers"
#define OPT_MASKNOTIFY "Notify"
#define OPT_MASKACTL "ActionLeft"
#define OPT_MASKACTR "ActionRight"
#define OPT_MASKACTTE "ActionTimeExpires"
#define OPT_MSGWINDOWCHECK "WindowCheck"
#define OPT_MERGEPOPUP "MergePopup"
#define OPT_DELAY_MESSAGE "DelayMessage"
#define OPT_DELAY_URL "DelayUrl"
#define OPT_DELAY_FILE "DelayFile"
#define OPT_DELAY_OTHERS "DelayOthers"
#define OPT_SHOW_DATE "ShowDate"
#define OPT_SHOW_TIME "ShowTime"
#define OPT_SHOW_HEADERS "ShowHeaders"
#define OPT_NUMBER_MSG "NumberMsg"
#define OPT_SHOW_ON "ShowOldOrNew"
#define OPT_HIDESEND "HideSend"
#define OPT_NORSS "NoRSSAnnounces"
#define OPT_READCHECK "ReadCheck"
//---------------------------
//---Translateable Strings

#define POPUP_COMMENT_MESSAGE "Message"
#define POPUP_COMMENT_URL "URL"
#define POPUP_COMMENT_FILE "File"
#define POPUP_COMMENT_CONTACTS "Contacts"
#define POPUP_COMMENT_ADDED "You were added!"
#define POPUP_COMMENT_AUTH "Requests your authorisation"
#define POPUP_COMMENT_WEBPAGER "ICQ Web pager"
#define POPUP_COMMENT_EMAILEXP "ICQ Email express"
#define POPUP_COMMENT_OTHER "Unknown Event"

#define OPTIONS_GROUP "PopUps"
#define OPTIONS_TITLE "Event Notify"
#define OPTIONS_MESSAGE_TITLE "Event Notify (Message)"

#define MENUITEM_NAME "Notify of new events"

#define MENUITEM_ENABLE "Enable new event notification"
#define MENUITEM_DISABLE "Disable new event notification"

//---------------------------
//---Structures

typedef struct PLUGIN_OPTIONS_struct{

    HINSTANCE hInst;
    BOOL bDisable;
    BOOL bPreview;
    BOOL bDefaultColorMsg;
	BOOL bDefaultColorUrl;
	BOOL bDefaultColorFile;
	BOOL bDefaultColorOthers;
    COLORREF colBackMsg;
    COLORREF colTextMsg;
	COLORREF colBackUrl;
    COLORREF colTextUrl;
	COLORREF colBackFile;
    COLORREF colTextFile;
	COLORREF colBackOthers;
    COLORREF colTextOthers;
    UINT maskNotify;
    UINT maskActL;
    UINT maskActR;
	UINT maskActTE;
    BOOL bMsgWindowcheck;
    BOOL bMsgReplywindow;
	int iDelayMsg;
	int iDelayUrl;
	int iDelayFile;
	int iDelayOthers;
	int iDelayDefault;
	BOOL bMergePopup;
	BOOL bShowDate;
	BOOL bShowTime;
	BOOL bShowHeaders;
	BYTE iNumberMsg;
	BOOL bShowON;
	BOOL bHideSend;
	BOOL bNoRSS;
} PLUGIN_OPTIONS;

typedef struct EVENT_DATA_EX{
	HANDLE hEvent;
	DWORD timestamp;
	char szText[MAX_SECONDLINE];
} EVENT_DATA_EX;

typedef struct PLUGIN_DATA_struct {
    UINT eventType;
    HANDLE hContact;
    PLUGIN_OPTIONS* pluginOptions;
	POPUPDATAEX* pud;
	HWND hWnd;
	struct EVENT_DATA_EX* eventData;
	long firstShownEvent;
	long allocedEvents;
	long countEvent;
	long iSeconds;
	int iLock;
} PLUGIN_DATA;


//---------------------------
//---External Procedure Definitions

int PopupShow(PLUGIN_OPTIONS* pluginOptions, HANDLE hContact, HANDLE hEvent, UINT eventType);
int PopupUpdate(HANDLE hContact, HANDLE hEvent);
int PopupPreview(PLUGIN_OPTIONS* pluginOptions);
int PopupAct(HWND hWnd, UINT mask, PLUGIN_DATA* pdata);
int OptionsInit(PLUGIN_OPTIONS* pluginOptions);
int OptionsAdd(HINSTANCE hInst, WPARAM addInfo);
int Opt_DisableNGN(BOOL Status);
int MenuitemInit(BOOL bStatus);
int MenuitemUpdate(BOOL bStatus);
int NumberPopupData(HANDLE hContact);
int CheckMsgWnd(WPARAM contact);

extern HINSTANCE hInst;
