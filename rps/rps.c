/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2005 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

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

Created by Pescuma 
Based on work by nullbie

*/
#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_database.h>


#define INI_FILE_NAME  "RemovePersonalSettings.ini"

#define PLUGINDISABLELIST "PluginDisable"

#define METACONTACTS_PROTOCOL_NAME "MetaContacts"


HINSTANCE hInst;
PLUGINLINK *pluginLink;
char gIniFile[MAX_PATH];
char gMirandaDir[MAX_PATH];


PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"Remove Personal Settings",
	PLUGIN_MAKE_VERSION(0,1,0,3),
	"Remove personal settings to allow to send a profile to other user(s) without sending personal data.",
	"Pescuma",
	"",
	"© 2005 Ricardo Pescuma Domenecci",
	"http://www.miranda-im.org/",
	0,		//not transient
	0		//doesn't replace anything built-in
};


int RemoveAllService(WPARAM wParam,LPARAM lParam);
void SetProtocolsOffline();
void RemoveUsers();
void RemoveSettings();
void ExecuteServices();
void RemoveDirectories();
void DisablePlugins();

// Ini access functions
BOOL GetSettingBool(const char *section, const char *key, BOOL defaultValue);
BOOL GetSettings(const char *section, char *buffer, size_t bufferSize);


// Utils
void DeleteFileOrFolder(const char *name);
void DeleteSetting(const char *setting);
void DeleteSettingEx(const char *szModule, const char *szSetting);
BOOL isMetaContact(HANDLE hContact);



BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	CLISTMENUITEM mi;
	char *strTmp;

	pluginLink=link;
	CreateServiceFunction("RemovePersonalSettings/RemoveAll",RemoveAllService);
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFF;
	mi.flags=0;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.pszName="Remove Personal Settings...";
	mi.pszService="RemovePersonalSettings/RemoveAll";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	// Get ini file name
	gMirandaDir[0] = '\0';
	GetModuleFileName(GetModuleHandle(NULL),gMirandaDir,sizeof(gMirandaDir));

	// Remove last name
	strTmp = strrchr(gMirandaDir,'\\');
	if(strTmp != NULL)
		*strTmp = '\0';

	// Set vars
	strcat(gMirandaDir, "\\");
	strcpy(gIniFile, gMirandaDir);

	// Store last pos
	strTmp = &gIniFile[strlen(gIniFile)];

	// Lets try fist name
	strcpy(strTmp, INI_FILE_NAME);

	if (_access(gIniFile, 4) != 0)
	{
		// Not found, lets try the other aproach
		strcpy(strTmp, "plugins\\" INI_FILE_NAME);

		if (_access(gIniFile, 4) != 0)
		{
			// Not found :(
			gIniFile[0] = '\0';
		}
	}

	return 0;
}

int __declspec(dllexport) Unload(void)
{
	return 0;
}


int RemoveAllService(WPARAM wParam,LPARAM lParam)
{
	if (gIniFile[0] == '\0')
	{
		MessageBox(NULL, Translate("Configuration file could not be found!"), Translate("Remove Personal Settings"), MB_OK | MB_ICONERROR);
		return -1;
	}

	if (MessageBox(NULL,Translate("All your personal settings will be erased!\n"
					"Make sure you are running this from a copy of your profile (and not over the original one).\n"
					"Running this will erase files/folders under Miranda main folder.\n"
					"\n"
					"Are you sure you want to remove all your personal settings?\n"
					"\n"
					"(You cannot say that I don't told you about the risks :P )"), Translate("Remove Personal Settings"),MB_YESNO)
		== IDYES)
	{
		SetProtocolsOffline();
		RemoveUsers();
		RemoveSettings();
		ExecuteServices();
		RemoveDirectories();
		DisablePlugins();

		MessageBox(NULL,Translate("Settings are deleted now."), Translate("Remove Personal Settings"),MB_OK | MB_ICONINFORMATION);
	}

	return 0;
}

void SetProtocolsOffline()
{
	if ( GetSettingBool("GlobalSettings", "SetProtocolsOffline", TRUE) )
	{
		PROTOCOLDESCRIPTOR **protos;
		int i,count;

		CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

		for (i = 0; i < count; i++)
		{
			if (protos[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0')
				continue;

			CallProtoService(protos[i]->szName, PS_SETSTATUS, ID_STATUS_OFFLINE, 0);
		}

		// Give some time to make it really offline
		Sleep(2000);
	}
}

void RemoveUsers()
{
	if ( GetSettingBool("GlobalSettings", "RemoveAllUsers", TRUE) )
	{
		HANDLE hContact;
		HANDLE hDbEvent;
		HANDLE hContactOld;

		// To be faster, remove first all metacontacts (because it syncs histories)
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while(hContact != NULL)
		{
			hContactOld = hContact;
			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);

			if ( isMetaContact(hContactOld) )
				CallService(MS_DB_CONTACT_DELETE, (WPARAM) hContactOld, 0);
		}

		// Now delete all left-overs
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);

		while(hContact != NULL)
		{
			CallService(MS_DB_CONTACT_DELETE, (WPARAM) hContact, 0);

			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		}

		// Delete events for contacts not in list
		hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDFIRST, 0, 0);

		while(hDbEvent != NULL)
		{
			int ret = CallService(MS_DB_EVENT_DELETE, 0, (WPARAM) hDbEvent);

			hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDFIRST, 0, 0);
		}

		// Now delete groups
		DeleteSettingEx("CListGroups", NULL);
	}
}

void RemoveProtocolSettings(const char * protocolName)
{
	char buffer[10000];

	// Remove protocol module settings
	if ( GetSettingBool("GlobalSettings", "RemoveWholeProtocolModule", FALSE) )
	{
		DeleteSettingEx(protocolName, NULL);
	}
	else if ( GetSettings("ProtocolSettings", buffer, sizeof(buffer)) )
	{
		char *name;
		char *value;

		name = buffer;
		while(name[0] != '\0')
		{
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[strlen(name)];

			// Has " ?
			if (*name == '"' && *(value-1) == '"')
			{
				name++;
				*(value-1) = '\0';
			}

			// Disable it
			if (name[0] != '\0')
				DeleteSettingEx(protocolName, name);

			// Get next one
			name = value + strlen(value) + 1;
		}
	}

	// Remove modules by protocol sufixes
	if ( GetSettings("ProtocolModuleSufixes", buffer, sizeof(buffer)) )
	{
		char *name;
		char *value;
		char moduleName[256];

		name = buffer;
		while(name[0] != '\0')
		{
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[strlen(name)];

			// Has " ?
			if (*name == '"' && *(value-1) == '"')
			{
				name++;
				*(value-1) = '\0';
			}

			// Delete it
			if (name[0] != '\0')
			{
				mir_snprintf(moduleName, sizeof(moduleName), "%s%s", protocolName, name);
				DeleteSettingEx(moduleName, NULL);
			}

			// Get next one
			name = value + strlen(value) + 1;
		}
	}
}

void RemoveSettings()
{
	char buffer[10000];

	// Delete protocol settings
	if ( GetSettingBool("GlobalSettings", "RemoveProtocolSettings", TRUE) )
	{
		PROTOCOLDESCRIPTOR **protos;
		int i,count;

		CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

		for (i = 0; i < count; i++)
		{
			if (protos[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0')
				continue;

			RemoveProtocolSettings(protos[i]->szName);
		}

		// Get disabled protocols
		if ( GetSettings("DisabledProtocols", buffer, sizeof(buffer)) )
		{
			char *name;
			char *value;

			name = buffer;
			while(name[0] != '\0')
			{
				value = strchr(name, '=');
				if (value == NULL)
					value = &name[strlen(name)];

				// Has " ?
				if (*name == '"' && *(value-1) == '"')
				{
					name++;
					*(value-1) = '\0';
				}

				// Disable it
				if (name[0] != '\0')
					RemoveProtocolSettings(name);

				// Get next one
				name = value + strlen(value) + 1;
			}
		}
	}


	// Delete other settings
	if ( GetSettings("RemoveSettings", buffer, sizeof(buffer)) )
	{
		char *name;
		char *value;

		name = buffer;
		while(name[0] != '\0')
		{
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[strlen(name)];
 
			// Has " ?
			if (*name == '"' && *(value-1) == '"')
			{
				name++;
				*(value-1) = '\0';
			}

			// Disable it
			if (name[0] != '\0')
				DeleteSetting(name);

			// Get next one
			name = value + strlen(value) + 1;
		}
	}
}

void ExecuteServices()
{
	char buffer[10000];

	if ( GetSettings("ExecuteServices", buffer, sizeof(buffer)) )
	{
		char *name;
		char *value;

		name = buffer;
		while(name[0] != '\0')
		{
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[strlen(name)];
 
			// Has " ?
			if (*name == '"' && *(value-1) == '"')
			{
				name++;
				*(value-1) = '\0';
			}

			// Disable it
			if (name[0] != '\0')
				if (ServiceExists(name))
					CallService(name,0,0);

			// Get next one
			name = value + strlen(value) + 1;
		}
	}
}

void RemoveDirectories()
{
	char buffer[10000];
	char dir[MAX_PATH];

	// Remove protocol folders
	if (GetSettingBool("GlobalSettings", "RemoveProtocolFolders", TRUE))
	{
		PROTOCOLDESCRIPTOR **protos;
		int i,count;

		CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

		for (i = 0; i < count; i++)
		{
			if (protos[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0')
				continue;

			mir_snprintf(dir, sizeof(dir), "%s%s", gMirandaDir, protos[i]->szName);
			DeleteFileOrFolder(dir);
		}
	}

	// Remove other folders
	if ( GetSettings("RemoveFilesOrFolders", buffer, sizeof(buffer)) )
	{
		char *name;
		char *value;

		name = buffer;
		while(name[0] != '\0')
		{
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[strlen(name)];
 
			// Has " ?
			if (*name == '"' && *(value-1) == '"')
			{
				name++;
				*(value-1) = '\0';
			}

			// Delete it
			if (name[0] != '\0')
			{
				mir_snprintf(dir, sizeof(dir), "%s%s", gMirandaDir, name);
				DeleteFileOrFolder(dir);
			}

			// Get next one
			name = value + strlen(value) + 1;
		}
	}
}

void DisablePlugins()
{
	char buffer[10000];

	if ( GetSettings("DisablePlugins", buffer, sizeof(buffer)) )
	{
		char *name;
		char *value;

		name = buffer;
		while(name[0] != '\0')
		{
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[strlen(name)];
 
			// Has " ?
			if (*name == '"' && *(value-1) == '"')
			{
				name++;
				*(value-1) = '\0';
			}

			// Disable it
			if (name[0] != '\0')
			{
				CharLower(name);
				if (DBGetContactSettingByte(NULL, PLUGINDISABLELIST, name, 0) != 1)
				{
					DBWriteContactSettingByte(NULL, PLUGINDISABLELIST, name, 1);
				}
			}

			// Get next one
			name = value + strlen(value) + 1;
		}
	}
}



// Ini access functions

BOOL GetSettingBool(const char *section, const char *key, BOOL defaultValue)
{
	char tmp[16];

	if ( GetPrivateProfileString(section, key, defaultValue ? "true" : "false", tmp, sizeof(tmp), gIniFile) == 0 )
	{
		return defaultValue;
	}
	else
	{
		return stricmp(tmp, "true") == 0;
	}
}


BOOL GetSettings(const char *section, char *buffer, size_t bufferSize)
{
	buffer[0] = '\0\0';

	return GetPrivateProfileSection(section, buffer, bufferSize, gIniFile) != 0;
}



// Utils

void DeleteFileOrFolder(const char *name)
{
	int attibs = GetFileAttributes(name);

	if (attibs == 0xFFFFFFFF) // Not exists
	{
		// Try to find it
		WIN32_FIND_DATA findData;
		HANDLE hwnd;
		char tmp[MAX_PATH];
		char *strTmp;

		// Delete files
		hwnd = FindFirstFile(name, &findData);
		if (hwnd != INVALID_HANDLE_VALUE)
		{
			strcpy(tmp, name);
			strTmp = strrchr(tmp,'\\');
			if(strTmp != NULL)
			{
				strTmp++;
				*strTmp = '\0';
			}
			else
			{
				strcat(tmp, "\\");
				strTmp = &tmp[strlen(tmp)];
			}

			do 
			{
				if (strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, ".."))
				{
					strcpy(strTmp, findData.cFileName);
					DeleteFileOrFolder(tmp);
				}
			}
			while(FindNextFile(hwnd, &findData) != 0);

			FindClose(hwnd);
		}
	}
	else if (attibs & FILE_ATTRIBUTE_DIRECTORY)	// Is a directory
	{
		// Get all files and delete then
		WIN32_FIND_DATA findData;
		HANDLE hwnd;
		char tmp[MAX_PATH];

		mir_snprintf(tmp, sizeof(tmp), "%s\\*.*", name);

		// Delete files
		hwnd = FindFirstFile(tmp, &findData);
		if (hwnd != INVALID_HANDLE_VALUE)
		{
			do 
			{
				if (strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, ".."))
				{
					mir_snprintf(tmp, sizeof(tmp), "%s\\%s", name, findData.cFileName);
					DeleteFileOrFolder(tmp);
				}
			}
			while(FindNextFile(hwnd, &findData) != 0);

			FindClose(hwnd);
		}

		// Delete directory
		RemoveDirectory(name);
	}
	else // Is a File	
	{
		SetFileAttributes(name, FILE_ATTRIBUTE_ARCHIVE);
		DeleteFile(name);
	}
}


BOOL isMetaContact(HANDLE hContact)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING dbcgs;
	char name[32];

	dbv.type=DBVT_ASCIIZ;
	dbv.pszVal=name;
	dbv.cchVal=sizeof(name);
	dbcgs.pValue=&dbv;
	dbcgs.szModule="Protocol";
	dbcgs.szSetting="p";

	if(CallService(MS_DB_CONTACT_GETSETTINGSTATIC,(WPARAM)hContact,(LPARAM)&dbcgs)) 
		return FALSE;

	return strcmp(dbcgs.pValue->pszVal, METACONTACTS_PROTOCOL_NAME) == 0;
}



typedef struct {
	char buffer[10000];
	int pos;
} DeleteModuleStruct;


int EnumProc(const char *szSetting, LPARAM lParam)
{
	DeleteModuleStruct *dms = (DeleteModuleStruct *) lParam;
	size_t len = strlen(szSetting);

	// Add to the struct
	if (len > 0 && len < sizeof(dms->buffer) - dms->pos - 2)
	{
		strcpy(&dms->buffer[dms->pos], szSetting);
		dms->pos += len + 1;
	}

	return 0;
}


void DeleteSettingEx(const char *szModule, const char *szSetting)
{
	// Do it
	if (szModule == NULL)
	{
		return;
	}
	else if (szSetting == NULL)
	{
		DeleteModuleStruct dms;
		DBCONTACTENUMSETTINGS dbces;

		ZeroMemory(&dms, sizeof(dms));

		dbces.pfnEnumProc = EnumProc;
		dbces.lParam = (LPARAM) &dms;
		dbces.szModule = szModule;
		dbces.ofsSettings = 0;

		CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM) &dbces);

		// Delete then
		szSetting = dms.buffer;
		while(szSetting[0] != '\0')
		{
			DBDeleteContactSetting(NULL, szModule, szSetting);

			// Get next one
			szSetting += strlen(szSetting) + 1;
		}
	}
	else
	{
		size_t len = strlen(szSetting);
		if (szSetting[0] == '*' || szSetting[len-1] == '*')
		{
			DeleteModuleStruct dms;
			DBCONTACTENUMSETTINGS dbces;
			char *s;

			ZeroMemory(&dms, sizeof(dms));

			dbces.pfnEnumProc = EnumProc;
			dbces.lParam = (LPARAM) &dms;
			dbces.szModule = szModule;
			dbces.ofsSettings = 0;

			CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM) &dbces);

			// Delete then
			s = dms.buffer;
			while(s[0] != '\0')
			{
				size_t lenS = strlen(s);

				if (lenS >= len-1)
				{
					if (szSetting[0] == '*')
					{
						if (strcmp(&szSetting[1], &s[lenS-(len-1)]) == 0)
						{
							DBDeleteContactSetting(NULL, szModule, s);
						}
					}
					else // if (szSetting[len-1] == '*')
					{
						if (strncmp(szSetting, s, len-1) == 0)
						{
							DBDeleteContactSetting(NULL, szModule, s);
						}
					}
				}
				

				// Get next one
				s += strlen(s) + 1;
			}
		}
		else
		{
			DBDeleteContactSetting(NULL, szModule, szSetting);
		}
	}
}

void DeleteSetting(const char *setting)
{
	char *szModule;
	char *szSetting;

	if (setting == NULL || setting[0] == '\0')
	{
		return;
	}

	// Split setting
	szModule = strdup(setting);
	szSetting = strrchr(szModule, '/');
	if (szSetting != NULL)
	{
		*szSetting = '\0';
		szSetting ++;
	}

	DeleteSettingEx(szModule, szSetting);
}

