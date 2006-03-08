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


#include "notification_speak.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////



HINSTANCE hInst;
PLUGINLINK *pluginLink;
MNOTIFYLINK *notifyLink;

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"Speak Notification",
	PLUGIN_MAKE_VERSION(0,0,0,1),
	"Notification type that speak notifications aloud",
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

int SpeakShow(WPARAM wParam, LPARAM lParam);



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
	if (ServiceExists(MS_SPEAK_SAY))
	{
		MNotifyGetLink();
		LoadNotifyImp();
		InitOptions();
	}
	else
	{
		MessageBox(NULL, Translate("Speak Notification requires Speak plugin to be installed"), 
			Translate("Error"), MB_OK | MB_ICONERROR);
	}

	return 0;
}





void LoadNotifyImp()
{
	hhkNotificationShow = HookEvent(ME_NOTIFY_SHOW, SpeakShow);

	CreateServiceFunction(MS_SPEAK_SHOW, SpeakShow);
}


void UnloadNotifyImp()
{
	UnhookEvent(hhkNotificationShow);
}


int SpeakShow(WPARAM wParam, LPARAM lParam)
{
	HANDLE hNotify = (HANDLE)lParam;

	if (!MNotifyGetByte(hNotify, NFOPT_SPEAK_SAY, 0))
		return 0;

	char *log_text;
	bool free = false;

	// Get text
	log_text = (char *) MNotifyGetString(hNotify, NFOPT_SPEAK_TEXT, 0);
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
		CallService(MS_SPEAK_SAY, 0, (LPARAM) log_text);

	if (free)
		mir_free(log_text);

	return 0;
}
