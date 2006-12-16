/*
Avatar History Plugin
---------

 This plugin uses the event provided by Avatar Service to 
 automatically back up contacts' avatars when they change.
 Copyright (C) 2006  Matthew Wild - Email: mwild1@gmail.com

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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#include "AvatarHistory.h"

// #define DBGPOPUPS

HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE hModulesHook;
HANDLE hAvatarChange;
HANDLE hHookoptsinit;

HANDLE hFolder;

char profilePath[MAX_PATH+1];		// database profile path (read at startup only)
TCHAR basedir[MAX_PATH+1];

static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int AvatarChanged(WPARAM wParam, LPARAM lParam);
int OptInit(WPARAM wParam,LPARAM lParam);

int GetFileHash(char* fn);
int GetUIDFromHContact(HANDLE contact, char* protoout, TCHAR* uinout, size_t uinout_len);
void InitFolders();
void InitMenuItem();

// Services
static int IsEnabled(WPARAM wParam, LPARAM lParam);

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
#ifdef UNICODE
	"Avatar History (Unicode)",
#else
	"Avatar History",
#endif
	PLUGIN_MAKE_VERSION(0,0,1,5),
	"This plugin keeps backups of all your contacts' avatar changes and/or shows popups",
	"Matthew Wild (MattJ), Ricardo Pescuma Domenecci",
	"mwild1@gmail.com",
	"© 2006 Matthew Wild",
	"http://mattj.xmgfree.com/",
	0,		//not transient
	0		//doesn't replace anything built-in
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink=link;

	init_mir_malloc();
	LoadOptions();

	hModulesHook = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	CreateServiceFunction("AvatarHistory/IsEnabled", IsEnabled);

	if(CallService(MS_DB_GETPROFILEPATH, MAX_PATH+1, (LPARAM)profilePath) != 0)
		strcpy(profilePath, "."); // Failed, use current dir
    _strlwr(profilePath);

	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	InitFolders();
	hAvatarChange = HookEvent(ME_AV_CONTACTAVATARCHANGED, AvatarChanged);
	hHookoptsinit = HookEvent(ME_OPT_INITIALISE,OptInit);
	SetupIcoLib();
	if(opts.show_menu)
		InitMenuItem();
	return 0;
}


BOOL ProtocolEnabled(const char *proto)
{
	if (proto == NULL)
		return FALSE;
		
	char setting[256];
	mir_snprintf(setting, sizeof(setting), "%sEnabled", proto);
	return (BOOL) DBGetContactSettingByte(NULL, MODULE_NAME, setting, TRUE);
}


BOOL ContactEnabled(HANDLE hContact, char *setting, int def) 
{
	if (hContact == NULL)
		return FALSE;

	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (!ProtocolEnabled(proto))
		return FALSE;

	BYTE globpref = db_byte_get(NULL, MODULE_NAME, setting, def);
	BYTE userpref = db_byte_get(hContact, MODULE_NAME, setting, BST_INDETERMINATE);

	return (globpref && userpref == BST_INDETERMINATE) || userpref == BST_CHECKED;
}


HANDLE HistoryLog(HANDLE hContact, TCHAR *log_text, char *filename)
{
	if (log_text != NULL)
	{
		DBEVENTINFO event = { 0 };

		event.cbSize = sizeof(event);

		size_t len = lstrlen(log_text) + 1;
		size_t size = len;
#ifdef UNICODE
		size *= 3;
#endif
		if (filename != NULL)
			size += strlen(filename) + 1;

		BYTE *tmp = (BYTE *) mir_alloc0(size);
#ifdef UNICODE
		WideCharToMultiByte(CP_ACP, 0, log_text, -1, (char *) tmp, size, NULL, NULL);
		lstrcpynW((WCHAR *) &tmp[len], log_text, len);
		len *= 3;
#else
		strcpy((char *) tmp, log_text);
#endif
		if (filename != NULL)
			strcpy((char *) &tmp[len], filename);

		event.pBlob = tmp;
		event.cbBlob = size;

		event.eventType = EVENTTYPE_AVATAR_CHANGE;
		event.flags = DBEF_READ;
		event.timestamp = (DWORD) time(NULL);

		event.szModule = MODULE_NAME;
		
		// Is a subcontact?
		if (ServiceExists(MS_MC_GETMETACONTACT)) 
		{
			HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

			if (hMetaContact != NULL && ContactEnabled(hMetaContact, "LogToHistory", AVH_DEF_LOGTOHISTORY))
				CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&event);
		}

		HANDLE ret = (HANDLE) CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&event);

		mir_free(tmp);

		return ret;
	}
	else
	{
		return NULL;
	}
}

static int PathIsAbsolute(const TCHAR *path)
{
    if (!path || !(lstrlen(path) > 2)) 
        return 0;
    if ((path[1]==_T(':')&&path[2]==_T('\\'))||(path[0]==_T('\\')&&path[1]==_T('\\'))) return 1;
    return 0;
}

int PathToRelative(const TCHAR *pSrc, char *pOut)
{
    if (!pSrc||!lstrlen(pSrc)||lstrlen(pSrc)>MAX_PATH) return 0;
    if (!PathIsAbsolute(pSrc)) {
#ifdef UNICODE
        mir_snprintf(pOut, MAX_PATH, "%S", pSrc);
#else
        mir_snprintf(pOut, MAX_PATH, "%s", pSrc);
#endif
        return strlen(pOut);
    }
    else {
        char szTmp[MAX_PATH];
#ifdef UNICODE
        mir_snprintf(szTmp, MAX_REGS(szTmp), "%S", pSrc);
#else
        mir_snprintf(szTmp, MAX_REGS(szTmp), "%s", pSrc);
#endif
        _strlwr(szTmp);
        if (strstr(szTmp, profilePath)) {
#ifdef UNICODE
            mir_snprintf(pOut, MAX_PATH, "%S", pSrc + strlen(profilePath) + 1);
#else
            mir_snprintf(pOut, MAX_PATH, "%s", pSrc + strlen(profilePath) + 1);
#endif
            return strlen(pOut);
        }
        else {
#ifdef UNICODE
            mir_snprintf(pOut, MAX_PATH, "%S", pSrc);
#else
            mir_snprintf(pOut, MAX_PATH, "%s", pSrc);
#endif
            return strlen(pOut);
        }
    }

	return 0;
}

// fired when the contacts avatar changes
// wParam = hContact
// lParam = struct avatarCacheEntry *cacheEntry
// the event CAN pass a NULL pointer in lParam which means that the avatar has changed,
// but is no longer valid (happens, when a contact removes his avatar, for example).
// DONT DESTROY the bitmap handle passed in the struct avatarCacheEntry *
// 
// It is also possible that this event passes 0 as wParam (hContact), in which case,
// a protocol picture (pseudo - avatar) has been changed. 
static int AvatarChanged(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	CONTACTAVATARCHANGEDNOTIFICATION* avatar = (CONTACTAVATARCHANGEDNOTIFICATION*)lParam;

	if (wParam == 0)
	{
#ifdef DBGPOPUPS
		if(wParam == 0)
			ShowPopup(NULL, _T("AVH Debug"), _T("Invalid contact... skipping"));
#endif
		return 0;
	}

	if (!opts.track_changes && avatar != NULL)
		return 0;

	if (!opts.track_removes && avatar == NULL)
		return 0;
	
	char oldhash[1024] = "";
	DBVARIANT dbv = {0};
	if (!db_get(hContact, "AvatarHistory", "AvatarHash", &dbv))
	{
		if (dbv.type == DBVT_ASCIIZ)
			strncpy(oldhash, dbv.pszVal, sizeof(oldhash));

		DBFreeVariant(&dbv);
	}

	if(
		(avatar != NULL && !strcmp(oldhash, avatar->hash)) // Changed it
		|| (avatar == NULL && oldhash[0] == '\0') // Removed it
		)
	{
#ifdef DBGPOPUPS
		ShowPopup(NULL, "AVH Debug", "Hashes are the same... skipping");
#endif
		return 0;
	}

	if (avatar != NULL)
	{
		db_string_set(hContact, "AvatarHistory", "AvatarHash", avatar->hash);
	}
	else
	{
		DBDeleteContactSetting(hContact, "AvatarHistory", "AvatarHash");
	}

	TCHAR history_filename[MAX_PATH+1] = _T("");

	if (avatar != NULL)
	{
		if (ContactEnabled(hContact, "LogToDisk", AVH_DEF_LOGTODISK))
		{
			// Needed because path in event is char*
#ifdef UNICODE
			TCHAR file[MAX_PATH+1];
			MultiByteToWideChar(CP_ACP, 0, avatar->filename, -1, file, MAX_REGS(file));
#else
			TCHAR *file = avatar->filename;
#endif
			TCHAR ext[11];
			SYSTEMTIME curtime;

			lstrcpyn(ext, _tcsrchr(file, _T('.'))+1, 10);

			GetContactFolder(hContact, history_filename);
		
			GetLocalTime(&curtime);
			mir_sntprintf(history_filename, MAX_REGS(history_filename), 
				_T("%s\\%04d-%02d-%02d %02dh%02dm%02ds.%s"), history_filename, 
				curtime.wYear, curtime.wMonth, curtime.wDay, 
				curtime.wHour, curtime.wMinute, curtime.wSecond, 
				ext);
			if(CopyFile(file, history_filename, TRUE) == 0)
			{
				ShowPopup(hContact, _T("Avatar History"), _T("Unable to save avatar"));
			}
#ifdef DBGPOPUPS
			else
			{
				ShowPopup(hContact, _T("AVH Debug"), _T("File copied successfully"));
			}
#endif
		}
	}

	if (ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS))
	{
		if (avatar != NULL)
			ShowPopup(hContact, NULL, opts.template_changed);
		else
			ShowPopup(hContact, NULL, opts.template_removed);
	}

	if (ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY))
	{
		if (avatar != NULL)
		{
			char file[MAX_PATH];
			PathToRelative(history_filename, file);
			HistoryLog(hContact, opts.template_changed, file);
		}
		else
			HistoryLog(hContact, opts.template_removed, NULL);
	}
	return 0;
}

int GetUIDFromHContact(HANDLE contact, char* protoout, TCHAR* uinout, size_t uinout_len)
{
	CONTACTINFO cinfo;
	char* proto;
	
	proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)contact,0);
	if(proto)
		strcpy(protoout, proto);
	else strcpy(protoout, Translate("Unknown Protocol"));

	ZeroMemory(&cinfo,sizeof(CONTACTINFO));
	cinfo.cbSize = sizeof(CONTACTINFO);
	cinfo.hContact = contact;
	cinfo.dwFlag = CNF_UNIQUEID;
#ifdef UNICODE
	cinfo.dwFlag |= CNF_UNICODE;
#endif

	BOOL found = TRUE;
	if(CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&cinfo)==0)
	{
		if(cinfo.type == CNFT_ASCIIZ)
		{
			lstrcpyn(uinout, cinfo.pszVal, uinout_len);
			// It is up to us to free the string
			// The catch? We need to use Miranda's free(), not our CRT's :)
			mir_free(cinfo.pszVal);
		}
		else if(cinfo.type == CNFT_DWORD)
		{
			_itot(cinfo.dVal,uinout,10);
		}
		else if(cinfo.type == CNFT_WORD)
		{
			_itot(cinfo.wVal,uinout,10);
		}
		else found = FALSE;
	}
	else found = FALSE;

	if (!found)
	{
#ifdef UNICODE
		// Try non unicode ver
		cinfo.dwFlag = CNF_UNIQUEID;

		found = TRUE;
		if(CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&cinfo)==0)
		{
			if(cinfo.type == CNFT_ASCIIZ)
			{
				MultiByteToWideChar(CP_ACP, 0, (char *) cinfo.pszVal, -1, uinout, uinout_len);
				// It is up to us to free the string
				// The catch? We need to use Miranda's free(), not our CRT's :)
				mir_free(cinfo.pszVal);
			}
			else if(cinfo.type == CNFT_DWORD)
			{
				_itot(cinfo.dVal,uinout,10);
			}
			else if(cinfo.type == CNFT_WORD)
			{
				_itot(cinfo.wVal,uinout,10);
			}
			else found = FALSE;
		}
		else found = FALSE;

		if (!found)
#endif
			lstrcpy(uinout, TranslateT("Unknown UIN"));
	}
	return 0;
}


extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}


static int IsEnabled(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	return ContactEnabled(hContact, "LogToDisk", AVH_DEF_LOGTODISK) 
		|| ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS)
		|| ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY);
}


void InitFolders()
{
#ifdef UNICODE
	mir_sntprintf(basedir, MAX_REGS(basedir), _T("%S\\Avatars History"), profilePath);
#else
	mir_sntprintf(basedir, MAX_REGS(basedir), _T("%s\\Avatars History"), profilePath);
#endif

	hFolder = (HANDLE)FoldersRegisterCustomPathT(Translate("Avatars"), Translate("Avatar History"), 
		_T(PROFILE_PATH) _T("\\") _T(CURRENT_PROFILE) _T("\\Avatars History"));
}


TCHAR* GetContactFolder(HANDLE hContact, TCHAR* fn)
{
	TCHAR uin[MAX_PATH+1];
	char proto[50+1];
	FoldersGetCustomPathT(hFolder, fn, MAX_PATH+1, basedir);
	CreateDirectory(fn, NULL);		
	GetUIDFromHContact(hContact, proto, uin, MAX_REGS(uin));
#ifdef UNICODE
	mir_sntprintf(fn, MAX_PATH+1, _T("%s\\%S"), fn, proto);
#else
	mir_sntprintf(fn, MAX_PATH+1, _T("%s\\%s"), fn, proto);
#endif
	CreateDirectory(fn, NULL);
	mir_sntprintf(fn, MAX_PATH+1, _T("%s\\%s"), fn, uin);
	CreateDirectory(fn, NULL);
	
#ifdef DBGPOPUPS
	char log[1024];
#ifdef UNICODE
	mir_snprintf(log, MAX_REGS(log), "Path: %S\nProto: %s\nUIN: %S", fn, proto, uin);
#else
	mir_snprintf(log, MAX_REGS(log), "Path: %s\nProto: %s\nUIN: %s", fn, proto, uin);
#endif
	ShowPopup(NULL, "AVH Debug: GetContactFolder", log);
#endif

	return fn;
}

