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
	"Notification type that speak notifications aloud. Depends on Speak plugin.",
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
		MessageBox(NULL, TranslateT("Speak Notification requires Speak plugin to be installed"), 
			TranslateT("Error"), MB_OK | MB_ICONERROR);
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

	TCHAR def[1024];
	mir_sntprintf(def, MAX_REGS(def), _T("%s\r\n%s"), 
					MNotifyGetTTemplate(hNotify, NFOPT_DEFTEMPL_TITLET, _T("%title%")), 
					MNotifyGetTTemplate(hNotify, NFOPT_DEFTEMPL_TEXTT, _T("%text%")));

	// Get text
	TCHAR *log_text = MNotifyGetTParsedTemplate(hNotify, NFOPT_SPEAK_TEMPLATE_TEXTT, def);
	
	if (log_text != NULL)
	{
#ifdef _UNICODE
		// Speak does not have an unicode version
		char *tmp = mir_dupToAscii(log_text);

		CallService(MS_SPEAK_SAY, 0, (LPARAM) tmp);

		mir_free(tmp);
#else
		CallService(MS_SPEAK_SAY, 0, (LPARAM) log_text);
#endif

		mir_free(log_text);
	}

	return 0;
}
