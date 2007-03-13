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
DWORD mirVer;

HANDLE hHooks[6] = {0};
HANDLE hServices[3] = {0};

HANDLE hFolder = NULL;

char *metacontacts_proto = NULL;

char profilePath[MAX_PATH+1];		// database profile path (read at startup only)
TCHAR basedir[MAX_PATH+1];

static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int PreShutdown(WPARAM wParam, LPARAM lParam);
static int AvatarChanged(WPARAM wParam, LPARAM lParam);
int OptInit(WPARAM wParam,LPARAM lParam);

TCHAR * GetProtocolFolder(char *proto, TCHAR* fn, size_t size);
void InitFolders();
void InitMenuItem();

// Services
static int IsEnabled(WPARAM wParam, LPARAM lParam);
static int GetCachedAvatar(WPARAM wParam, LPARAM lParam);
TCHAR * GetCachedAvatar(char *proto, char *hash);
TCHAR* GetOldStyleContactFolder(HANDLE hContact, TCHAR* fn);
BOOL CreateShortcut(TCHAR *file, TCHAR *shortcut);

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
#ifdef UNICODE
	"Avatar History (Unicode)",
#else
	"Avatar History",
#endif
	PLUGIN_MAKE_VERSION(0,0,2,5),
	"This plugin keeps backups of all your contacts' avatar changes and/or shows popups",
	"Matthew Wild (MattJ), Ricardo Pescuma Domenecci",
	"mwild1@gmail.com",
	"� 2006 Matthew Wild, Ricardo Pescuma Domenecci",
	"http://pescuma.mirandaim.ru/miranda/avatarhist",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
#ifdef UNICODE
	{ 0xdbe8c990, 0x7aa0, 0x458d, { 0xba, 0xb7, 0x33, 0xeb, 0x7, 0x23, 0x8e, 0x71 } } // {DBE8C990-7AA0-458d-BAB7-33EB07238E71}
#else
	{ 0x4079923c, 0x8aa1, 0x4a2e, { 0x95, 0x8b, 0x9d, 0xc, 0xd0, 0xe8, 0x2e, 0xb2 } } // {4079923C-8AA1-4a2e-958B-9D0CD0E82EB2}
#endif

};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	mirVer = mirandaVersion;
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*) &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	mirVer = mirandaVersion;
	return &pluginInfo;
}

static const MUUID interfaces[] = { MIID_AVATAR_CHANGE_LOGGER, MIID_AVATAR_CHANGE_NOTIFIER, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink=link;

	init_mir_malloc();
	LoadOptions();

	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	hHooks[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	hServices[0] = CreateServiceFunction(MS_AVATARHISTORY_ENABLED, IsEnabled);
	hServices[1] = CreateServiceFunction(MS_AVATARHISTORY_GET_CACHED_AVATAR, GetCachedAvatar);

	if(CallService(MS_DB_GETPROFILEPATH, MAX_PATH+1, (LPARAM)profilePath) != 0)
		strcpy(profilePath, "."); // Failed, use current dir
	_strlwr(profilePath);

	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
#ifdef UNICODE
	mir_sntprintf(basedir, MAX_REGS(basedir), _T("%S\\Avatars History"), profilePath);
#else
	mir_sntprintf(basedir, MAX_REGS(basedir), _T("%s\\Avatars History"), profilePath);
#endif

	hFolder = (HANDLE)FoldersRegisterCustomPathT(Translate("Avatars"), Translate("Avatar History"), 
		_T(PROFILE_PATH) _T("\\") _T(CURRENT_PROFILE) _T("\\Avatars History"));

	hHooks[2] = HookEvent(ME_AV_CONTACTAVATARCHANGED, AvatarChanged);
	hHooks[3] = HookEvent(ME_OPT_INITIALISE, OptInit);
	SetupIcoLib();
	InitMenuItem();
	InitPopups();

	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://pescuma.mirandaim.ru/miranda/avatarhist_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.mirandaim.ru/miranda/avatarhist#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"Avatar History ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/avatarhistW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/avatarhist.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	return 0;
}

static int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	int i;

	for (i = 0; i < MAX_REGS(hHooks); i++)
		UnhookEvent(hHooks[i]);

	for (i = 0; i < MAX_REGS(hServices); i++)
		DestroyServiceFunction(hServices[i]);

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

// Returns true if the unicode buffer only contains 7-bit characters.
BOOL IsUnicodeAscii(const WCHAR * pBuffer, int nSize)
{
	BOOL bResult = TRUE;
	int nIndex;

	for (nIndex = 0; nIndex < nSize; nIndex++) {
		if (pBuffer[nIndex] > 0x7F) {
			bResult = FALSE;
			break;
		}
	}
	return bResult;
}


HANDLE HistoryLog(HANDLE hContact, TCHAR *log_text, char *filename)
{
	if (log_text == NULL)
		return NULL;

	DBEVENTINFO event = { 0 };
	BYTE *tmp = NULL;

	event.cbSize = sizeof(event);

	size_t file_len;
	if (filename != NULL)
		file_len = strlen(filename) + 1;
	else
		file_len = 0;

#ifdef UNICODE

	size_t needed = WideCharToMultiByte(CP_ACP, 0, log_text, -1, NULL, 0, NULL, NULL);
	size_t len = lstrlen(log_text) + 1;
	size_t size;
	BOOL isAscii = IsUnicodeAscii(log_text, len);

	if (isAscii)
		size = needed + (filename != NULL ? 2 : 0);
	else
		size = needed + len * sizeof(WCHAR);

	tmp = (BYTE *) mir_alloc0(size + file_len);

	WideCharToMultiByte(CP_ACP, 0, log_text, -1, (char *) tmp, needed, NULL, NULL);

	if (isAscii)
	{
		if (filename != NULL)
		{
			tmp[needed] = 0;
			tmp[needed+1] = 0;
		}
	}
	else
	{
		lstrcpyn((WCHAR *) &tmp[needed], log_text, len);
	}

	if (filename != NULL)
		strcpy((char *) &tmp[size], filename);

	event.pBlob = tmp;
	event.cbBlob = size + file_len;

#else

	if (filename != NULL)
	{
		size_t len = lstrlen(log_text) + 1;
		tmp = (BYTE *) mir_alloc0(len + 2 + file_len);

		strcpy((char *) tmp, log_text);
		tmp[len] = 0;
		tmp[len + 1] = 0;
		strcpy((char *) &tmp[len + 2], filename);

		event.pBlob = tmp;
		event.cbBlob = len + 2 + file_len;
	}
	else
	{
		event.pBlob = (PBYTE) log_text;
		event.cbBlob = strlen(log_text) + 1;
	}

#endif

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

int PathIsAbsolute(const char *path)
{
    if (!path || !(strlen(path) > 2)) 
        return 0;
    if ((path[1]==':'&&path[2]=='\\')||(path[0]=='\\'&&path[1]=='\\')) return 1;
    return 0;
}

int PathToRelative(const char *pSrc, char *pOut)
{
    if (!pSrc||!strlen(pSrc)||strlen(pSrc)>MAX_PATH) return 0;
    if (!PathIsAbsolute(pSrc)) {
        mir_snprintf(pOut, MAX_PATH, "%s", pSrc);
        return strlen(pOut);
    }
    else {
        char szTmp[MAX_PATH];
        mir_snprintf(szTmp, MAX_REGS(szTmp), "%s", pSrc);
        _strlwr(szTmp);
        if (strstr(szTmp, profilePath)) {
            mir_snprintf(pOut, MAX_PATH, "%s", pSrc + strlen(profilePath) + 1);
            return strlen(pOut);
        }
        else {
            mir_snprintf(pOut, MAX_PATH, "%s", pSrc);
            return strlen(pOut);
        }
    }

	return 0;
}

int PathToAbsolute(char *pSrc, char *pOut) {
    if (!pSrc||!strlen(pSrc)||strlen(pSrc)>MAX_PATH) return 0;
    if (PathIsAbsolute(pSrc)||!isalnum(pSrc[0])) {
        mir_snprintf(pOut, MAX_PATH, "%s", pSrc);
        return strlen(pOut);
    }
    else {
        mir_snprintf(pOut, MAX_PATH, "%s\\%s", profilePath, pSrc);
        return strlen(pOut);
    }
}

void ConvertToFilename(char *str, size_t size) {
	for(size_t i = 0; i < size && str[i] != '\0'; i++) {
		switch(str[i]) {
			case '/':
			case '\\':
			case ':':
			case '*':
			case '?':
			case '"':
			case '<':
			case '>':
			case '|':
			case '.':
				str[i] = '_';
		}
	}
}


void CreateOldStyleShortcut(HANDLE hContact, char *proto, TCHAR *history_filename)
{
	TCHAR shortcut[MAX_PATH+1] = _T("");
	TCHAR ext[11];
	SYSTEMTIME curtime;

	lstrcpyn(ext, _tcsrchr(history_filename, _T('.'))+1, 10);

	GetOldStyleContactFolder(hContact, shortcut);

	GetLocalTime(&curtime);
	mir_sntprintf(shortcut, MAX_REGS(shortcut), 
		_T("%s\\%04d-%02d-%02d %02dh%02dm%02ds.%s.lnk"), shortcut, 
		curtime.wYear, curtime.wMonth, curtime.wDay, 
		curtime.wHour, curtime.wMinute, curtime.wSecond, 
		ext);

	if (!CreateShortcut(history_filename, shortcut))
	{
		ShowPopup(hContact, _T("Avatar History"), _T("Unable to create shortcut"));
	}
#ifdef DBGPOPUPS
	else
	{
		ShowPopup(hContact, _T("AVH Debug"), _T("Shortcut created successfully"));
	}
#endif
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

	if (hContact == NULL)
	{
#ifdef DBGPOPUPS
		ShowPopup(NULL, _T("AVH Debug"), _T("Invalid contact/avatar... skipping"));
#endif
		return 0;
	}

	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (proto == NULL)
	{
#ifdef DBGPOPUPS
		ShowPopup(NULL, _T("AVH Debug"), _T("Invalid protocol... skipping"));
#endif
		return 0;
	}
	else if (metacontacts_proto != NULL && strcmp(metacontacts_proto, proto) == 0)
	{
#ifdef DBGPOPUPS
		ShowPopup(NULL, _T("AVH Debug"), _T("Ignoring metacontacts notification"));
#endif
		return 0;
	}

	BOOL removed = (avatar == NULL && DBGetContactSettingWord(hContact, "ContactPhoto", "Format", 0) == 0);

	if (!opts.track_removes && removed)
		return 0;
	
	char oldhash[1024] = "";
	char * ret = MyDBGetString(hContact, "AvatarHistory", "AvatarHash", oldhash, sizeof(oldhash));

	BOOL first_time = (ret == NULL);

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

	if (removed)
	{
		db_string_set(hContact, "AvatarHistory", "AvatarHash", "");

		if (!first_time && ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS))
			ShowPopup(hContact, NULL, opts.template_removed);

		if (ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY))
			HistoryLog(hContact, opts.template_removed, NULL);
	}
	else if (avatar == NULL)
	{
		if (!strcmp(oldhash, "-"))
		{
#ifdef DBGPOPUPS
			ShowPopup(NULL, "AVH Debug", "Changed from a flash avatar to a flash avatar... skipping");
#endif
			return 0;
		}

		// Is a flash avatar or avs could not load it
		db_string_set(hContact, "AvatarHistory", "AvatarHash", "-");

		if (!first_time && ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS))
			ShowPopup(hContact, NULL, opts.template_changed);

		if (ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY))
			HistoryLog(hContact, opts.template_changed, NULL);
	}
	else
	{
		db_string_set(hContact, "AvatarHistory", "AvatarHash", avatar->hash);

		TCHAR history_filename[MAX_PATH] = _T("");

		if (ContactEnabled(hContact, "LogToDisk", AVH_DEF_LOGTODISK))
		{
			// See if we already have the avatar
			char hash[128];
			lstrcpynA(hash, avatar->hash, sizeof(hash));
			ConvertToFilename(hash, sizeof(hash));
			TCHAR *file = GetCachedAvatar(proto, hash);

			if (file != NULL)
			{
				lstrcpyn(history_filename, file, MAX_REGS(history_filename));
				mir_free(file);
			}
			else
			{
				// Needed because path in event is char*
#ifdef UNICODE
				TCHAR file[1024];
				MultiByteToWideChar(CP_ACP, 0, avatar->filename, -1, file, MAX_REGS(file));
#else
				TCHAR *file = avatar->filename;
#endif

				GetProtocolFolder(proto, history_filename, MAX_REGS(history_filename));

				TCHAR *ext = _tcsrchr(file, _T('.'));
				if (ext != NULL)
					ext++;
				else
					ext = _T("");

				if (lstrcmpi(ext, _T("bmp")) == 0 
					&& ServiceExists(MS_AV_CANSAVEBITMAP)
					&& CallService(MS_AV_CANSAVEBITMAP, 0, PA_FORMAT_PNG))
				{
					// Store as PNG
#ifdef UNICODE
					mir_sntprintf(history_filename, MAX_REGS(history_filename), 
						_T("%s\\%S.png"), history_filename, hash);
#else
					mir_sntprintf(history_filename, MAX_REGS(history_filename), 
						_T("%s\\%s.png"), history_filename, hash);
#endif

					HBITMAP hBmp = (HBITMAP) CallService(MS_AV_LOADBITMAP32, 0, (LPARAM) avatar->filename);
#ifdef UNICODE
					char tmp[1024];
					WideCharToMultiByte(CP_ACP, 0, history_filename, -1, tmp, MAX_REGS(tmp), NULL, NULL);
					if (CallService(MS_AV_SAVEBITMAP, (WPARAM) hBmp, (LPARAM) tmp) != 0)
#else
					if (CallService(MS_AV_SAVEBITMAP, (WPARAM) hBmp, (LPARAM) history_filename) != 0)
#endif
						ShowPopup(hContact, _T("Avatar History"), _T("Unable to save avatar"));
#ifdef DBGPOPUPS
					else
						ShowPopup(hContact, _T("AVH Debug"), _T("File copied successfully"));
#endif
				}
				else
				{
#ifdef UNICODE
					mir_sntprintf(history_filename, MAX_REGS(history_filename), 
						_T("%s\\%S.%s"), history_filename, hash, ext);
#else
					mir_sntprintf(history_filename, MAX_REGS(history_filename), 
						_T("%s\\%s.%s"), history_filename, hash, ext);
#endif

					if(CopyFile(file, history_filename, TRUE) == 0)
						ShowPopup(hContact, _T("Avatar History"), _T("Unable to save avatar"));
#ifdef DBGPOPUPS
					else
						ShowPopup(hContact, _T("AVH Debug"), _T("File copied successfully"));
#endif
				}
			}

			if (opts.log_old_style)
			{
				CreateOldStyleShortcut(hContact, proto, history_filename);

				if (ServiceExists(MS_MC_GETMETACONTACT)) 
				{
					HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

					if (hMetaContact != NULL && ContactEnabled(hMetaContact, "LogToHistory", AVH_DEF_LOGTOHISTORY))
						CreateOldStyleShortcut(hMetaContact, metacontacts_proto, history_filename);
				}
			}
		}


		if (!first_time && ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS))
			ShowPopup(hContact, NULL, opts.template_changed);

		if (ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY))
		{
			char rel_path[MAX_PATH] = "";
#ifdef UNICODE
			char tmp[MAX_PATH];
			TCHAR_TO_CHAR(tmp, history_filename);
			PathToRelative(tmp, rel_path);
#else
			PathToRelative(history_filename, rel_path);
#endif
			HistoryLog(hContact, opts.template_changed, rel_path);
		}
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


TCHAR* GetProtocolFolder(char *proto, TCHAR* fn, size_t size)
{
	FoldersGetCustomPathT(hFolder, fn, size, basedir);
	CreateDirectory(fn, NULL);

	if (!opts.log_keep_same_folder)
	{
		if (proto == NULL)
			proto = Translate("Unknown Protocol");

#ifdef UNICODE
		mir_sntprintf(fn, size, _T("%s\\%S"), fn, proto);
#else
		mir_sntprintf(fn, size, _T("%s\\%s"), fn, proto);
#endif
		CreateDirectory(fn, NULL);
	}
	
	return fn;
}


/*
Get cached avatar

wParam: (char *) protocol name
lParam: (char *) hash 
return: (TCHAR *) NULL if none is found or the path to the avatar. You need to free this string 
        with mir_free.
*/
static int GetCachedAvatar(WPARAM wParam, LPARAM lParam)
{
	char hash[128];
	lstrcpynA(hash, (char *) lParam, sizeof(hash));
	ConvertToFilename(hash, sizeof(hash));
	return (int) GetCachedAvatar((char *) wParam, hash);
}

TCHAR * GetCachedAvatar(char *proto, char *hash)
{
	TCHAR *ret = NULL;
	TCHAR file[1024] = _T("");
	TCHAR search[1024] = _T("");
	GetProtocolFolder(proto, file, MAX_REGS(file));

#ifdef UNICODE
	mir_sntprintf(search, MAX_REGS(search), _T("%s\\%S.*"), file, hash);
#else
	mir_sntprintf(search, MAX_REGS(search), _T("%s\\%s.*"), file, hash);
#endif

	WIN32_FIND_DATA finddata;
	HANDLE hFind = FindFirstFile(search, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return NULL;

	do
	{
		size_t len = lstrlen(finddata.cFileName);
		if (len > 4 
			&& (!lstrcmpi(&finddata.cFileName[len-4], _T(".png"))
				|| !lstrcmpi(&finddata.cFileName[len-4], _T(".bmp"))
				|| !lstrcmpi(&finddata.cFileName[len-4], _T(".gif"))
				|| !lstrcmpi(&finddata.cFileName[len-4], _T(".jpg"))
				|| !lstrcmpi(&finddata.cFileName[len-5], _T(".jpeg"))))
		{
			mir_sntprintf(file, MAX_REGS(file), _T("%s\\%s"), file, finddata.cFileName);
			ret = mir_tstrdup(file);
			break;
		}
	} while(FindNextFile(hFind, &finddata));
	FindClose(hFind);

	return ret;
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


TCHAR* GetOldStyleContactFolder(HANDLE hContact, TCHAR* fn)
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


#include <ShObjIdl.h>
#include <ShlGuid.h>

BOOL CreateShortcut(TCHAR *file, TCHAR *shortcut)
{
	CoInitialize(NULL);

    IShellLink* psl = NULL;

    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **) &psl);

    if (SUCCEEDED(hr)) 
    {
        psl->SetPath(file); 

        IPersistFile* ppf = NULL; 
        hr = psl->QueryInterface(IID_IPersistFile,  (void **) &ppf); 

        if (SUCCEEDED(hr))
        {
#ifdef UNICODE
			hr = ppf->Save(shortcut, TRUE); 
#else
			WCHAR tmp[MAX_PATH]; 
            MultiByteToWideChar(CP_ACP, 0, shortcut, -1, tmp, MAX_PATH); 
            hr = ppf->Save(tmp, TRUE); 
#endif
            ppf->Release(); 
        }

        psl->Release(); 
    } 

	return SUCCEEDED(hr);
}


BOOL ResolveShortcut(TCHAR *shortcut, TCHAR *file)
{
	CoInitialize(NULL);

    IShellLink* psl = NULL;

    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **) &psl);

    if (SUCCEEDED(hr)) 
    {
        IPersistFile* ppf = NULL; 
		hr = psl->QueryInterface(IID_IPersistFile,  (void **) &ppf); 

        if (SUCCEEDED(hr))
		{
#ifdef UNICODE
			hr = ppf->Load(shortcut, STGM_READ); 
#else
			WCHAR tmp[MAX_PATH]; 
			MultiByteToWideChar(CP_ACP, 0, shortcut, -1, tmp, MAX_PATH); 
			hr = ppf->Load(tmp, STGM_READ); 
#endif

			if (SUCCEEDED(hr))
			{
				hr = psl->Resolve(NULL, SLR_UPDATE); 

				if (SUCCEEDED(hr))
				{
					WIN32_FIND_DATA wfd;
					hr = psl->GetPath(file, MAX_PATH, &wfd, SLGP_RAWPATH); 
				}
			}

            ppf->Release(); 
		}
        psl->Release(); 
    }

	return SUCCEEDED(hr);
}
