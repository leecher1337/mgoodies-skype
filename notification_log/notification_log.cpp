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


#include "notification_log.h"


#define FOLDER_LOGSW                   PROFILE_PATHW L"\\" CURRENT_PROFILEW L"\\logs"

#ifdef _UNICODE
# define FOLDER_LOGST FOLDER_LOGSW
#else
# define FOLDER_LOGST FOLDER_LOGS
#endif


// Prototypes /////////////////////////////////////////////////////////////////////////////////////



HINSTANCE hInst;
PLUGINLINK *pluginLink;
MNOTIFYLINK *notifyLink;

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"Log Notification",
	PLUGIN_MAKE_VERSION(0,0,0,1),
	"Notification type that log to a file",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2006 Ricardo Pescuma Domenecci",
	"http://www.miranda-im.org/",
	0,		//not transient
	0		//doesn't replace anything built-in
};


HANDLE hhkNotificationShow = NULL;
HANDLE hhkModulesLoaded = NULL;
HANDLE hLogFolder = 0;
TCHAR szLogPath[MAX_PATH];		// database profile path (read at startup only)


int ModulesLoaded(WPARAM wParam,LPARAM lParam);
void LoadNotifyImp();
void UnloadNotifyImp();

int LogShow(WPARAM wParam, LPARAM lParam);



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

	// Folders plugin support
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		hLogFolder = (HANDLE) FoldersRegisterCustomPathT("Logs", "Logs Path", FOLDER_LOGST);
	}
	else
	{
		char path[MAX_PATH];
		CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)path);

#ifdef _UNICODE
		mir_sntprintf(szLogPath, sizeof(szLogPath), L"%S\\Logs", path);
#else
		mir_sntprintf(szLogPath, sizeof(szLogPath), "%s\\Logs", path);
#endif
	}


	return 0;
}



void LoadNotifyImp()
{
	hhkNotificationShow = HookEvent(ME_NOTIFY_SHOW, LogShow);

	CreateServiceFunction(MS_LOG_SHOW, LogShow);
}


void UnloadNotifyImp()
{
	UnhookEvent(hhkNotificationShow);
}


int LogShow(WPARAM wParam, LPARAM lParam)
{
	HANDLE hNotify = (HANDLE)lParam;

	if (!MNotifyGetByte(hNotify, NFOPT_LOG_ENABLED, 0))
		return 0;

	TCHAR defFile[1024];
	mir_sntprintf(defFile, MAX_REGS(defFile), _T("%s_log.txt"), 
				MNotifyGetTString(hNotify, NFOPT_TYPENAME, _T("notifications")));
	const TCHAR *filename = MNotifyGetTString(hNotify, NFOPT_LOG_FILENAMET, defFile);

	if (filename == NULL)
		return 0;

	TCHAR defLine[1024];
	mir_sntprintf(defLine, MAX_REGS(defLine), _T("%s: %s"), 
					MNotifyGetTTemplate(hNotify, NFOPT_DEFTEMPL_TITLET, _T("%title%")), 
					MNotifyGetTTemplate(hNotify, NFOPT_DEFTEMPL_TEXTT, _T("%text%")));

	// Get text
	TCHAR *log_text = MNotifyGetTParsedTemplate(hNotify, NFOPT_LOG_TEMPLATE_LINET, defLine);
	
	if (log_text != NULL)
	{
		TCHAR fullFilename[2048];

		if (_tcschr(filename, ':') == NULL)
		{
			TCHAR path[MAX_PATH];
			FoldersGetCustomPathT(hLogFolder, path, MAX_PATH, szLogPath);
			mir_sntprintf(fullFilename, MAX_REGS(fullFilename), _T("%s\\%s"), path, filename);
		}
		else
		{
			lstrcpyn(fullFilename, filename, MAX_REGS(fullFilename));
		}

		unsigned long dwBytesWritten = 0;
		HANDLE hFile = CreateFile(fullFilename, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != NULL)
		{
			SetFilePointer(hFile, 0, 0, FILE_END);
			WriteFile(hFile, log_text, lstrlen(log_text) * sizeof(TCHAR), &dwBytesWritten, NULL);
			WriteFile(hFile, _T("\r\n"), 2 * sizeof(TCHAR), &dwBytesWritten, NULL);
			CloseHandle(hFile);
		}

		mir_free(log_text);
	}

	return 0;
}
