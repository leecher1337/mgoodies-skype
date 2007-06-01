/*
Avatar History Import
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
#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <shobjidl.h>
#include <shlguid.h>

#include <newpluginapi.h>
#include "sdk/m_folders.h"
#include <m_clist.h>
#include <m_skin.h>
#include <m_avatars.h>
#include <m_database.h>
#define _STATIC
#include <m_system.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_contacts.h>
#include <m_popup.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_langpack.h>
#include "sdk/m_metacontacts.h"
#include <m_history.h>
#include "sdk/m_avatarhist.h"
#include "sdk/m_imgsrvc.h"

#ifdef __GNUC__
#define mir_i64(x) (x##LL)
#else
#define mir_i64(x) (x##i64)
#endif

#define MODULE_NAME			"AvatarHistory"
#define DEFAULT_TEMPLATE_CHANGED	"changed his/her avatar"

#define WIDTH		32
#define TOPBIT		(1 << (WIDTH - 1))	/* MSB */
#define POLYNOMIAL	(0x488781ED)		/* This is the CRC Poly */

#ifndef CLSID_ShellLink
#define MDEF_CLSID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	const CLSID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

	MDEF_CLSID(CLSID_ShellLink, 0x00021401L, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
	MDEF_CLSID(IID_IShellLinkA, 0x000214ee, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
	MDEF_CLSID(IID_IShellLinkW, 0x000214f9, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
	MDEF_CLSID(IID_IPersistFile, 0x0000010b, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
#endif


HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hModulesLoadedHook = NULL;

HANDLE hFolder;

BYTE log_old_style;
BYTE log_keep_same_folder;

char profilePath[MAX_PATH+1];
TCHAR basedir[MAX_PATH+1];
TCHAR template_changed[MAX_PATH+1];


PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
#ifdef UNICODE
	"Avatar History Import (Unicode)",
#else
	"Avatar History Import",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,4),
	"This plugin converts AVH history to new format",
	"TioDuke, Pescuma",
	"tioduke@yahoo.ca",
	"© 2007 TioDuke",
	"http://www.miranda-im.org",
	UNICODE_AWARE,
	0		//doesn't replace anything built-in
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

char * CopyToANSI(char *out, TCHAR *in, size_t size)
{
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, in, -1, out, size, NULL, NULL);
#else
	lstrcpyn(out, in, size);
#endif
	return out;
}

// Temp buffer
char tmpANSI[1024];
char * ConvertToANSI(TCHAR *in)
{
	return CopyToANSI(tmpANSI, in, 1024);
}


HANDLE ContactLookup(TCHAR *proto, TCHAR *contactid)
{
	HANDLE hContact = NULL;
	
	for(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0); hContact; hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
	{
		CONTACTINFO cinfo = {0};

		cinfo.cbSize = sizeof(CONTACTINFO);
		cinfo.dwFlag = CNF_UNIQUEID;
#ifdef UNICODE
		cinfo.dwFlag |= CNF_UNICODE;
#endif
		cinfo.szProto = ConvertToANSI(proto);
		cinfo.hContact = hContact;

		if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&cinfo))
		{
			TCHAR uniqueid[MAX_PATH+1];

			if (cinfo.type == CNFT_ASCIIZ)
				lstrcpy(uniqueid, cinfo.pszVal);
			else if (cinfo.type == CNFT_WORD)
				_ltot(cinfo.wVal, uniqueid, 10);
			else if (cinfo.type == CNFT_DWORD)
				_ltot(cinfo.dVal, uniqueid, 10);
			else
				continue;

			if (!lstrcmpi(contactid, uniqueid))
				return hContact;
		}
	}

	return NULL;
}

int PathIsAbsolute(const char *path)
{
	if (!path || !(strlen(path) > 2)) 
		return 0;
	if ((path[1] == ':' && path[2] == '\\') || (path[0] == '\\' && path[1] == '\\')) return 1;
		return 0;
}

int PathToRelative(const char *pSrc, char *pOut)
{
	if (!pSrc || !strlen(pSrc) || strlen(pSrc) > MAX_PATH) return 0;
	if (!PathIsAbsolute(pSrc)) {
		mir_snprintf(pOut, MAX_PATH, "%s", pSrc);
		return strlen(pOut);
	}
	else {
		char szTmp[MAX_PATH+1];
		mir_snprintf(szTmp, MAX_PATH, "%s", pSrc);
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

BOOL isOldAVHEvent(HANDLE hDbEvent, char *oldAVHToken)
{
	BYTE blob[2048];
	DBEVENTINFO event={0};
	int i;

	event.pBlob = blob;
	event.cbBlob = sizeof(blob);
	event.cbSize = sizeof(event);
	if (CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&event))
		return FALSE;

	// Verify event type
	if (event.eventType != EVENTTYPE_AVATAR_CHANGE)
		return FALSE;
		
	// Verify event content
	for(i=(int)(event.cbBlob - 2); i >= 0 && event.pBlob[i] != 0; i--);
	if (i != (int)(event.cbBlob - 2) && i >= 0 && strstr((char *)(event.pBlob + i + 1), oldAVHToken))
		return TRUE;
	
	return FALSE;
}

void DeleteOldHistoryLogs(HANDLE hContact, TCHAR *proto, TCHAR *contactid)
{
	TCHAR path[MAX_PATH+1];
	char contactToken[MAX_PATH+1];
	HANDLE hDbEvent=NULL;
	if (!hContact)
		return;

	mir_sntprintf(path, MAX_PATH, _T("%s\\%s\\%s\\"), basedir, proto, contactid);
	PathToRelative(ConvertToANSI(path), contactToken);

	for(hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDFIRST, (WPARAM)hContact, 0); hDbEvent; )
	{
		if (isOldAVHEvent(hDbEvent, contactToken))
		{
			HANDLE hOld = hDbEvent;
			hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hDbEvent, 0);
			CallService(MS_DB_EVENT_DELETE, (WPARAM)hContact, (LPARAM)hOld);
		}
		else
		{
			hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hDbEvent, 0);
		}
	}

}

int GetFileHash(TCHAR* filename)
{
	int remainder = 0;
	char data[1024];
	DWORD dwRead;
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return 0;

	do
	{
		int byte;
		int bit;
		// Read file chunk
		dwRead = 0;
		ReadFile(hFile, data, 1024, &dwRead, NULL);

		/* loop through each byte of data */
		for (byte = 0; byte < (int) dwRead; ++byte) {
			/* store the next byte into the remainder */
			remainder ^= (data[byte] << (WIDTH - 8));
			/* calculate for all 8 bits in the byte */
			for (bit = 8; bit > 0; --bit) {
				/* check if MSB of remainder is a one */
				if (remainder & TOPBIT)
					remainder = (remainder << 1) ^ POLYNOMIAL;
				else
					remainder = (remainder << 1);
			}
		}
	} while(dwRead == 1024);

	CloseHandle(hFile);

	return remainder;
}

TCHAR *GenerateShortcutName(TCHAR *baseName, TCHAR *filename_with_extension)
{
	TCHAR aux[MAX_PATH+1];
	TCHAR *ext;
	static TCHAR shortcutName[MAX_PATH+1];

	lstrcpy(aux, baseName);
	ext = _tcsrchr(aux, _T('.'));
	if (ext)
		*ext = 0;

	ext = _tcsrchr(filename_with_extension, _T('.'));
	if (!ext)
		ext = _T("");

	mir_sntprintf(shortcutName, MAX_PATH, _T("%s%s.lnk"), aux, ext);
	return shortcutName;
}

BOOL CreateShortcut(TCHAR *file, TCHAR *shortcut)
{
	HRESULT hres;
	IShellLink *psl;
	IPersistFile *pPf;

	TCHAR szPath[MAX_PATH+1];
	if (!GetFullPathName(file, MAX_PATH, szPath, NULL))
		return FALSE;

	CoInitialize(NULL);

	hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, (void **)&psl);
	if (FAILED(hres))
	{
		CoUninitialize();
		return FALSE;
	}

	hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void **)&pPf);
	if (FAILED(hres))
	{
		psl->lpVtbl->Release(psl);
		CoUninitialize();
		return FALSE;
	}
	hres = psl->lpVtbl->SetPath(psl, szPath);
	if (FAILED(hres))
	{
		pPf->lpVtbl->Release(pPf);
		psl->lpVtbl->Release(psl);
		CoUninitialize();
		return FALSE;
	}
		
#ifdef UNICODE
	hres = pPf->lpVtbl->Save(pPf, shortcut, TRUE);
#else
	{
		WCHAR tmp[MAX_PATH+1];
		MultiByteToWideChar(CP_ACP, 0, shortcut, -1, (WCHAR *)tmp, MAX_PATH);
		hres = pPf->lpVtbl->Save(pPf, tmp, TRUE);
	}
#endif

	pPf->lpVtbl->Release(pPf);
	psl->lpVtbl->Release(psl);
	CoUninitialize();
	if (FAILED(hres))
		return FALSE;
	return TRUE;	
}

TCHAR *Move2NewFile(TCHAR *proto, TCHAR *contactid, TCHAR *history_filename)
{
	static TCHAR history_filename_new[MAX_PATH+1];

	TCHAR search[MAX_PATH+1];
	TCHAR hash[MAX_PATH+1];
	WIN32_FIND_DATA finddata;
	HANDLE hFind;
	TCHAR *ext;

	mir_sntprintf(hash, MAX_PATH, _T("AVS-HASH-%x"), GetFileHash(history_filename));

	if (log_keep_same_folder)
		mir_sntprintf(search, MAX_PATH, _T("%s\\%s.*"), basedir, hash);
	else
		mir_sntprintf(search, MAX_PATH, _T("%s\\%s\\%s.*"), basedir, proto, hash);


	hFind = FindFirstFile(search, &finddata);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			size_t len = _tcslen(finddata.cFileName);
			if (len > 4 && (!lstrcmpi(&finddata.cFileName[len-4], _T(".png")) || !lstrcmpi(&finddata.cFileName[len-4], _T(".bmp")) || !lstrcmpi(&finddata.cFileName[len-4], _T(".gif")) || !lstrcmpi(&finddata.cFileName[len-4], _T(".jpg")) || !lstrcmpi(&finddata.cFileName[len-5], _T(".jpeg"))))
			{
				if (log_keep_same_folder)
					mir_sntprintf(history_filename_new, MAX_PATH, _T("%s\\%s"), basedir, finddata.cFileName);
				else
					mir_sntprintf(history_filename_new, MAX_PATH, _T("%s\\%s\\%s"), basedir, proto, finddata.cFileName);
				FindClose(hFind);
				DeleteFile(history_filename);
				if (log_old_style)
					CreateShortcut(history_filename_new, GenerateShortcutName(history_filename, history_filename_new));
				return history_filename_new;
			}
		} while(FindNextFile(hFind, &finddata));
		FindClose(hFind);
	}

	ext = _tcsrchr(history_filename, _T('.'));
	if (ext)
		ext++;
	else
		ext = _T("");

	if (!lstrcmpi(ext, _T("bmp")) && ServiceExists(MS_AV_CANSAVEBITMAP) && CallService(MS_AV_CANSAVEBITMAP, 0, PA_FORMAT_PNG))
	{
		IMGSRVC_INFO ii = {0};

		// Store as PNG
		if (log_keep_same_folder)
			mir_sntprintf(history_filename_new, MAX_PATH, _T("%s\\%s.png"), basedir, hash);
		else
			mir_sntprintf(history_filename_new, MAX_PATH, _T("%s\\%s\\%s.png"), basedir, proto, hash);

		ii.cbSize = sizeof(ii);
#ifdef UNICODE
		ii.wszName = history_filename_new;
#else
		ii.szName = history_filename_new;
#endif
		ii.hbm = (HBITMAP) CallService(MS_IMG_LOAD, (WPARAM) history_filename, IMGL_TCHAR);
		ii.dwMask = IMGI_HBITMAP;
		ii.fif = FIF_UNKNOWN;
		if (!CallService(MS_IMG_SAVE, (WPARAM) &ii, IMGL_TCHAR))
			lstrcpy(history_filename_new, history_filename);
		else
		{
			DeleteFile(history_filename);
			if (log_old_style)
				CreateShortcut(history_filename_new, GenerateShortcutName(history_filename, history_filename_new));
		}
	}
	else
	{
		if (log_keep_same_folder)
			mir_sntprintf(history_filename_new, MAX_PATH, _T("%s\\%s.%s"), basedir, hash, ext);
		else
			mir_sntprintf(history_filename_new, MAX_PATH, _T("%s\\%s\\%s.%s"), basedir, proto, hash, ext);

		if(!CopyFile(history_filename, history_filename_new, TRUE))
			lstrcpy(history_filename_new, history_filename);
		else
		{
			DeleteFile(history_filename);
			if (log_old_style)
				CreateShortcut(history_filename_new, GenerateShortcutName(history_filename, history_filename_new));
		}
	}

	return history_filename_new;
}

DWORD filename2timestamp(TCHAR *filename)
{
	char tmp[MAX_PATH+1];
	SYSTEMTIME lt={0}, st;
	TIME_ZONE_INFORMATION timezone;
	FILETIME filetime;
	LARGE_INTEGER liFiletime;

	CopyToANSI(tmp, filename, MAX_PATH);

	lt.wYear = (WORD)atol(strtok(tmp, "-"));
	lt.wMonth = (WORD)atol(strtok(NULL, "-"));
	lt.wDay = (WORD)atol(strtok(NULL, " "));
	lt.wHour = (WORD)atol(strtok(NULL, "h"));
	lt.wMinute = (WORD)atol(strtok(NULL, "m"));
	lt.wSecond = (WORD)atol(strtok(NULL, "s"));

	GetTimeZoneInformation(&timezone);
	TzSpecificLocalTimeToSystemTime(&timezone, &lt, &st);
	SystemTimeToFileTime(&st, &filetime);
	
	liFiletime.LowPart = filetime.dwLowDateTime;
	liFiletime.HighPart = filetime.dwHighDateTime;
	return (DWORD)(liFiletime.QuadPart / 10000000 - mir_i64(11644473600));
}

BOOL ProtocolEnabled(const char *proto)
{
	char setting[256];

	if (proto == NULL)
		return FALSE;
		
	mir_snprintf(setting, sizeof(setting), "%sEnabled", proto);
	return (BOOL) DBGetContactSettingByte(NULL, "AvatarHistory", setting, TRUE);
}

BOOL ContactEnabled(HANDLE hContact, char *setting, int def) 
{
	char *proto;
	BYTE globpref;
	BYTE userpref;

	if (hContact == NULL)
		return FALSE;

	proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (!ProtocolEnabled(proto))
		return FALSE;

	globpref = db_byte_get(NULL, "AvatarHistory", setting, def);
	userpref = db_byte_get(hContact, "AvatarHistory", setting, BST_INDETERMINATE);

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


HANDLE HistoryLog(HANDLE hContact, TCHAR *log_text, char *filename, DWORD timestamp)
{
	DBEVENTINFO event = { 0 };
	BYTE *tmp = NULL;
	size_t file_len;
	HANDLE ret;

	if (log_text == NULL)
		return NULL;
	
	event.cbSize = sizeof(event);

	if (filename != NULL)
		file_len = strlen(filename) + 1;
	else
		file_len = 0;

#ifdef UNICODE

	{
		size_t needed = WideCharToMultiByte(CP_ACP, 0, log_text, -1, NULL, 0, NULL, NULL);
		size_t len = lstrlen(log_text) + 1;
		size_t size;
		BOOL isAscii = IsUnicodeAscii(log_text, len);

		if (isAscii)
			size = needed + (filename != NULL ? 2 : 0);
		else
			size = needed + len * sizeof(WCHAR);

		tmp = (BYTE *) malloc(size + file_len);

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
	}

#else

	if (filename != NULL)
	{
		size_t len = lstrlen(log_text) + 1;
		tmp = (BYTE *) malloc(len + 2 + file_len);

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
	event.timestamp = timestamp;

	event.szModule = MODULE_NAME;
	
	// Is a subcontact?
	if (ServiceExists(MS_MC_GETMETACONTACT)) 
	{
		HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);

		if (hMetaContact != NULL && ContactEnabled(hMetaContact, "LogToHistory", 1))
			CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&event);
	}

	ret = (HANDLE) CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&event);

	free(tmp);

	return ret;
}

void ImportContactAVH(TCHAR *proto, TCHAR *contactid)
{
	TCHAR search[MAX_PATH+1];
	WIN32_FIND_DATA finddata;
	HANDLE hFind;
	HANDLE hContact = ContactLookup(proto, contactid);
	if (!hContact)
		return;

	DeleteOldHistoryLogs(hContact, proto, contactid);

	mir_sntprintf(search, MAX_PATH, _T("%s\\%s\\%s\\*.*"), basedir, proto, contactid);

	hFind = FindFirstFile(search, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		size_t len = _tcslen(finddata.cFileName);
		if (len > 4 && (!lstrcmpi(&finddata.cFileName[len-4], _T(".png")) || !lstrcmpi(&finddata.cFileName[len-4], _T(".bmp")) || !lstrcmpi(&finddata.cFileName[len-4], _T(".gif")) || !lstrcmpi(&finddata.cFileName[len-4], _T(".jpg")) || !lstrcmpi(&finddata.cFileName[len-5], _T(".jpeg"))))
		{
			char rel_path[MAX_PATH+1];
			TCHAR history_filename[MAX_PATH+1];

			mir_sntprintf(history_filename, MAX_PATH, _T("%s\\%s\\%s\\%s"), basedir, proto, contactid, finddata.cFileName);
			PathToRelative(ConvertToANSI(Move2NewFile(proto, contactid, history_filename)), rel_path);
			if (ContactEnabled(hContact, "LogToHistory", 1))
				HistoryLog(hContact, template_changed, rel_path, filename2timestamp(finddata.cFileName));
		}
	} while(FindNextFile(hFind, &finddata));

	FindClose(hFind);
}

void ImportProtocolAVH(TCHAR *proto)
{
	TCHAR search[MAX_PATH+1];
	WIN32_FIND_DATA finddata;
	HANDLE hFind;
	mir_sntprintf(search, MAX_PATH, _T("%s\\%s\\*"), basedir, proto);

	hFind = FindFirstFile(search, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			if (lstrcmpi(finddata.cFileName, _T(".")) && lstrcmpi(finddata.cFileName, _T("..")))
				ImportContactAVH(proto, finddata.cFileName);
	} while(FindNextFile(hFind, &finddata));

	FindClose(hFind);
}
	
void __cdecl ImportAVH(void *dummy)
{
	TCHAR search[MAX_PATH+1];
	WIN32_FIND_DATA finddata;
	HANDLE hFind;
	

	if (MessageBox(NULL, TranslateT("Avatar History will be imported.\nPlease, don't close Miranda before it ends."), 
				_T("Avatar History Import"), MB_OKCANCEL) == IDCANCEL)
		return;

	mir_sntprintf(search, MAX_PATH, _T("%s\\*"), basedir);
	hFind = FindFirstFile(search, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			if (lstrcmpi(finddata.cFileName, _T(".")) && lstrcmpi(finddata.cFileName, _T("..")))
				ImportProtocolAVH(finddata.cFileName);
	} while(FindNextFile(hFind, &finddata));

	FindClose(hFind);
	DBWriteContactSettingByte(NULL, MODULE_NAME, "avh_imported", 1);
	MessageBox(NULL, TranslateT("Previous Avatar History has been successfully imported.\nYou can remove Avatar History Importer plugin now."), 
				_T("Avatar History Import"), MB_OK);
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	if (DBGetContactSettingByte(NULL, MODULE_NAME, "avh_imported", 0))
		return 0;

	if(CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)profilePath) != 0)
		strcpy(profilePath, "."); // Failed, use current dir
	_strlwr(profilePath);

#ifdef UNICODE
	mir_sntprintf(basedir, MAX_PATH, _T("%S\\Avatars History"), profilePath);
#else
	mir_sntprintf(basedir, MAX_PATH, _T("%s\\Avatars History"), profilePath);
#endif

	hFolder = (HANDLE)FoldersRegisterCustomPathT(Translate("Avatars"), Translate("Avatar History"), _T(PROFILE_PATH) _T("\\") _T(CURRENT_PROFILE) _T("\\Avatars History"));
	if (hFolder) {
		TCHAR customdir[MAX_PATH+1];
		FoldersGetCustomPathT(hFolder, customdir, MAX_PATH, basedir);
		lstrcpy(basedir, customdir);
	}

	log_old_style = DBGetContactSettingByte(NULL, MODULE_NAME, "LogPerContactFolders", 0);
	log_keep_same_folder = DBGetContactSettingByte(NULL, MODULE_NAME, "LogKeepSameFolder", 0);

	if (!DBGetContactSettingTString(NULL, MODULE_NAME, "TemplateChanged", &dbv))
	{
		mir_sntprintf(template_changed, MAX_PATH, _T("%s"), dbv.ptszVal);
		DBFreeVariant(&dbv);
	}
	else
		mir_sntprintf(template_changed, MAX_PATH, _T(DEFAULT_TEMPLATE_CHANGED));

	mir_forkthread((pThreadFunc)ImportAVH, NULL);

	return 0;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink=link;

	hModulesLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hModulesLoadedHook);
	return 0;
}
