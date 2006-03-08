/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci
Based on work by tweety, nullbyte

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#include "notification_history.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////



HINSTANCE hInst;
PLUGINLINK *pluginLink;
MNOTIFYLINK *notifyLink;

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"History Notification",
	PLUGIN_MAKE_VERSION(0,0,0,1),
	"Notification type that log notifications to system/contacts history",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2006 Ricardo Pescuma Domenecci",
	"http://www.miranda-im.org/",
	0,		//not transient
	0		//doesn't replace anything built-in
};


HANDLE hhkNotificationShow = NULL;
HANDLE hhkModulesLoaded = NULL;


int ModulesLoaded(WPARAM wParam,LPARAM lParam);
void LoadNotifyImp();
void UnloadNotifyImp();

int HistoryShow(WPARAM wParam, LPARAM lParam);



// Functions //////////////////////////////////////////////////////////////////////////////////////


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}


__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}


int __declspec(dllexport) Load(PLUGINLINK *link)
{
	// Copy data
	pluginLink = link;

	init_mir_malloc();

	hhkModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	return 0;
}


int __declspec(dllexport) Unload(void)
{
	UnloadNotifyImp();
	UnhookEvent(hhkModulesLoaded);
	return 0;
}


int ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	MNotifyGetLink();
	LoadNotifyImp();
	InitOptions();
	return 0;
}





void LoadNotifyImp()
{
	hhkNotificationShow = HookEvent(ME_NOTIFY_SHOW, HistoryShow);

	CreateServiceFunction(MS_HISTORY_SHOW, HistoryShow);
}


void UnloadNotifyImp()
{
	UnhookEvent(hhkNotificationShow);
}


void HistoryLog(HANDLE hNotify, HANDLE hContact, BOOL log, BOOL read)
{
	if (log)
	{
		char *log_text;
		bool free = false;

		// Get text
		log_text = (char *) MNotifyGetString(hNotify, NFOPT_HISTORY_TEXT, 0);
		if (log_text == NULL)
		{
			const char *title = MNotifyGetString(hNotify, NFOPT_TITLE, 0);
			const char *text = MNotifyGetString(hNotify, NFOPT_TEXT, 0);

			if (title != NULL && text != NULL)
			{
				size_t size = strlen(title) + 2 + strlen(text) + 1;

				free = true;
				log_text = (char *) mir_alloc(size * sizeof(char));
				mir_snprintf(log_text, size, "%s\r\n%s", title, text);
			}
			else if (title != NULL)
			{
				free = true;
				log_text = mir_dup(title);
			}
			else if (text != NULL)
			{
				free = true;
				log_text = mir_dup(text);
			}
		}

		if (log_text != NULL)
		{
			DBEVENTINFO event = { 0 };;

			event.cbSize = sizeof(event);
			event.pBlob = (PBYTE) log_text;
			event.cbBlob = strlen(log_text) + 1;
			event.eventType = MNotifyGetWord(hNotify, NFOPT_HISTORY_EVENTTYPE, EVENTTYPE_NOTIFICATION);
			event.flags = read ? DBEF_READ : 0;
			event.timestamp = (DWORD) time(NULL);

			event.szModule = MODULE_NAME;

			if (hContact != NULL)
			{
				HANDLE hMetaContact = NULL;

				//event.szModule = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

				if (ServiceExists(MS_MC_GETMETACONTACT)) //try to retrieve the metacontact if some
					hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

				if (hMetaContact != NULL) //metacontact
				{
					CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&event);
					event.flags = DBEF_READ;
				}
			}

			CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&event);

			if (free)
				mir_free(log_text);
		}
	}
}


int HistoryShow(WPARAM wParam, LPARAM lParam)
{
	HANDLE hNotify = (HANDLE)lParam;

	HANDLE hContact = (HANDLE)MNotifyGetDWord(hNotify, NFOPT_CONTACT, 0);
	if (hContact == NULL)
	{
		// Log to System
		HistoryLog(	hNotify, NULL, 
					MNotifyGetByte(hNotify, NFOPT_HISTORY_SYSTEM_LOG, 0), 
					MNotifyGetByte(hNotify, NFOPT_HISTORY_SYSTEM_MARK_READ, 1));

	}
	else
	{
		// Log to Contact
		HistoryLog(	hNotify, hContact, 
					MNotifyGetByte(hNotify, NFOPT_HISTORY_CONTACT_LOG, 0), 
					MNotifyGetByte(hNotify, NFOPT_HISTORY_CONTACT_MARK_READ, 1));
	}

	return 0;
}
