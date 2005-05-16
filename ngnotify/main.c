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

#include "newgenerationnotify.h"
#include <m_database.h>
#include <m_skin.h>
#include <m_clist.h>
#include <m_protocols.h>
//needed for ICQEVENTTYPE_* (Webpager & Emailexpress)
#include <m_protosvc.h>
#include <m_icq.h>
//needed for reply instead of read
#include <m_message.h>
#include <m_popup.h>

#include <m_system.h>
#include <m_options.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

int g_IsServiceAvail = 0;
int g_IsWindowAPI = 0;

extern PLUGIN_DATA* PopUpList[20];
//---------------------------
//---Debuging code

#include <m_popup.h>
int DebPrint(WPARAM wParam)
{
    char str[128];
    sprintf(str, "Debug: %d", wParam);
    PUShowMessage(str, SM_NOTIFY);
	return 0;
}

//---------------------------
//---Internal Hooks
//---(Workaround till CallServiceSync is available)

/*
The idea for this is taken from "NewStatusNotify" by Hrk, thx *g*
This is needed to send a message with safe multithrading.
We'll create a private hook and we'll call it via NotifyEventHooks, which brings execution
back to the main thread.
*/

HANDLE hHook_Workaround;
HANDLE hHookedWorkaround;
char WorkaroundService[128];

int HookedWorkaround(WPARAM wParam, LPARAM lParam)
{
    CallService(WorkaroundService, wParam, lParam);
    return 0;
}

int _WorkaroundInit()
{
    hHook_Workaround = CreateHookableEvent(ME_NGN_WORKAROUND);
    hHookedWorkaround = HookEvent(ME_NGN_WORKAROUND, HookedWorkaround);

    return 0;
}

int _Workaround_CallService(const char *name, WPARAM wParam, LPARAM lParam)
{
    strncpy(WorkaroundService, name, 128);
    NotifyEventHooks(hHook_Workaround, wParam, lParam);

    return 0;
}


//---------------------------
//---Some global variables for the plugin

PLUGIN_OPTIONS pluginOptions;
PLUGINLINK *pluginLink;
PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"NewGenerationNotify",
	PLUGIN_MAKE_VERSION(0, VER_MAJOR, VER_MINOR, VER_BUILD),
	"Notifies you when you receive a message, url, file or any other event by displaying a popup. Uses the PopUp-Plugin by hrk",
	"Prezes",
	"prezesso@klub.chip.pl",
	"GNU GPL",
	"http://www.miranda.kom.pl/dev/prezes",
	0,
	0
};

//---------------------------
//---Hooks

//---Handles to my hooks, needed to unhook them again
HANDLE hHookedInit;
HANDLE hHookedOpt;
HANDLE hHookedNewEvent;
HANDLE hHookedDeletedEvent;

//---Called when a new event is added to the database
int HookedNewEvent(WPARAM wParam, LPARAM lParam)
//wParam: contact-handle
//lParam: dbevent-handle
{
    DBEVENTINFO dbe;
	PLUGIN_DATA* pdata;

    //are popups currently enabled?
    if (pluginOptions.bDisable)
        return 0;

    //get DBEVENTINFO without pBlob
    dbe.cbSize = sizeof(dbe);
	dbe.cbBlob = 0;
	dbe.pBlob = NULL;
	CallService(MS_DB_EVENT_GET, (WPARAM)lParam, (LPARAM)&dbe);
	
	if (DBGetContactSettingDword((HANDLE)wParam, METACONTACTS_MODULE, METACONTACTS_HANDLE, FALSE))
		return 0;

	//is it an event info about online/offline status user
	if (dbe.eventType == 25368)
		return 0;

	//is it an event sent by the user? -> don't show
	if (dbe.flags & DBEF_SENT)
	{
		if (pluginOptions.bHideSend && NumberPopupData((HANDLE)wParam) != -1)
		{
			while(pdata = PopUpList[NumberPopupData((HANDLE)wParam)])
				PopupAct(pdata->hWnd, MASK_REMOVE|MASK_DISMISS, pdata);
		}		
	    return 0; 
	}
    //which status do we have, are we allowed to post popups?
    //UNDER CONSTRUCTION!!!
    CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
	
	if (dbe.eventType == EVENTTYPE_MESSAGE && (pluginOptions.bMsgWindowcheck && CheckMsgWnd(wParam)))
		return 0;
	if (NumberPopupData((HANDLE)wParam) != -1 && pluginOptions.bMergePopup && dbe.eventType == EVENTTYPE_MESSAGE)
	{
		PopupUpdate((HANDLE)wParam, (HANDLE)lParam);
	}
	else
	{
		//now finally show a plugin
		PopupShow(&pluginOptions, (HANDLE)wParam, (HANDLE)lParam, (UINT)dbe.eventType);
	}
    return 0;

}

//---Called when all the modules are loaded
int HookedInit(WPARAM wParam, LPARAM lParam)
{
	hHookedNewEvent = HookEvent(ME_DB_EVENT_ADDED, HookedNewEvent);
	// Plugin sweeper support
//	DBWriteContactSettingString(NULL, "Uninstall", "NewGenerationNotify", MODULE);
	if (ServiceExists("PluginSweeper/Add"))
        CallService("PluginSweeper/Add", (WPARAM)MODULE, (LPARAM)MODULE);

	if (ServiceExists(MS_MSG_GETWINDOWDATA))
		g_IsWindowAPI = 1;
	else 
		g_IsWindowAPI = 0;
	if (ServiceExists(MS_MSG_MOD_MESSAGEDIALOGOPENED))
		g_IsServiceAvail = 1;
	else
		g_IsServiceAvail = 0;

	return 0;
}

//---Called when an options dialog has to be created
int HookedOptions(WPARAM wParam, LPARAM lParam)
{
    OptionsAdd(hInst, wParam);

    return 0;
}

//---------------------------
//---Exportet Functions

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
    pluginLink = link;
    hHookedInit = HookEvent(ME_SYSTEM_MODULESLOADED, HookedInit);
    hHookedOpt = HookEvent(ME_OPT_INITIALISE, HookedOptions);

    OptionsInit(&pluginOptions);
    pluginOptions.hInst = hInst;

    if (pluginOptions.bMenuitem)
        MenuitemInit(!pluginOptions.bDisable);

    _WorkaroundInit();

	return 0;
}

int __declspec(dllexport) Unload(void)
{
    UnhookEvent(hHookedNewEvent);
    UnhookEvent(hHookedOpt);
    UnhookEvent(hHookedInit);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

//-------------------------------------
//---Check Window Message function

// Took this snippet of code from "EventNotify" by micron-x, thx *g*
// checks if the message-dialog window is already opened
// return values:
//	0 - No window found
//	1 - Split-mode window found
//	2 - Single-mode window found

int CheckMsgWnd(WPARAM contact)
{
	if (g_IsWindowAPI) {
		MessageWindowData mwd;
		MessageWindowInputData mwid;
		mwid.cbSize = sizeof(MessageWindowInputData); 
		mwid.hContact = (HANDLE) contact;
		mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
		mwd.cbSize = sizeof(MessageWindowData);
		mwd.hContact = (HANDLE) contact;
		if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM) &mwid, (LPARAM) &mwd)) {
			if (mwd.hwndWindow != NULL && (mwd.uState & MSG_WINDOW_STATE_EXISTS)) return 1;
		}
	}
	if(g_IsServiceAvail) {				// use the service provided by tabSRMM
		if(CallService(MS_MSG_MOD_MESSAGEDIALOGOPENED, (WPARAM) contact, 0))
			return 1;
		else 
			return 0;
	} // if(ServiceExists(MS_MSG_MOD_MESSAGEDIALOGOPENED)) 
	else 
	{					// old way: find it by using the window class & title
		char newtitle[256];
		char *szProto,*szStatus,*contactName;

		szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,contact,0);
		contactName=(char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,contact,0);
		szStatus=(char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,szProto==NULL?ID_STATUS_OFFLINE:DBGetContactSettingWord((HANDLE)contact,szProto,"Status",ID_STATUS_OFFLINE),0);
		
		_snprintf(newtitle,sizeof(newtitle),"%s  (%s)",contactName,szStatus);
		if(FindWindow("TMsgWindow",newtitle))
			return 2;

		_snprintf(newtitle,sizeof(newtitle),"[%s  (%s)]",contactName,szStatus);
		if(FindWindow("TfrmContainer",newtitle))
			return 1;
		return 0;
	}
}
