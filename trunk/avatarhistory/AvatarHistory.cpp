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
#include <windows.h>
#include <stdio.h>


#include <newpluginapi.h>
#include <m_folders.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_avatars.h>
#include <m_database.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_contacts.h>
#include <m_popup.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_langpack.h>
// #include <m_updater.h>

#include "AvatarHistory.h"
#include "resource.h"

// #define DBGPOPUPS

HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE hModulesHook;
HANDLE hAvatarChange;
HANDLE hHookoptsinit;

HANDLE hFolder;

char basedir[MAX_PATH+1];

static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int AvatarChanged(WPARAM wParam, LPARAM lParam);
int OptInit(WPARAM wParam,LPARAM lParam);

int GetFileHash(char* fn);
int GetUIDFromHContact(HANDLE contact, char* protoout, char* uinout);
int ShowPopup(HANDLE hContact, char* title, char* text);
void InitFolders();
void InitMenuItem();

// Services
static int IsEnabled(WPARAM wParam, LPARAM lParam);

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"Avatar History",
	PLUGIN_MAKE_VERSION(0,0,1,0),
	"This plugin keeps backups of all your contacts' avatar changes and/or shows popups",
	"Matthew Wild (MattJ)",
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

	hModulesHook = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	CreateServiceFunction("AvatarHistory/IsEnabled", IsEnabled);
	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	InitFolders();
	if(db_byte_get(NULL, "AvatarHistory", "ShowContactMenu", AVH_DEF_SHOWMENU))
		InitMenuItem();
	hAvatarChange = HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	hHookoptsinit = HookEvent(ME_OPT_INITIALISE,OptInit);
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
	SYSTEMTIME curtime;
	AVATARCACHEENTRY* avatar = (AVATARCACHEENTRY*)lParam;
	char fn[MAX_PATH+1], ext[11];
	unsigned int oldhash, newhash;
	if(wParam == 0||avatar == NULL)
	{
#ifdef DBGPOPUPS
		if(wParam == 0)
			ShowPopup(NULL, "AVH Debug", "Invalid contact... skipping");
		if(lParam == 0)
			ShowPopup(NULL, "AVH Debug", "Invalid avatar data... skipping");
#endif
		return 0;
	}
	
	oldhash = db_dword_get((HANDLE)wParam, "AvatarHistory", "AvatarHash", 0);
	newhash = GetFileHash(avatar->szFilename);
	if(newhash == 0 || (oldhash !=0 && (newhash == oldhash)))
	{
#ifdef DBGPOPUPS
		if(newhash==0)
			ShowPopup(NULL, "AVH Debug", "New hash failed... skipping");
		if(oldhash == 0)
			ShowPopup(NULL, "AVH Debug", "Old hash does not exist in db... skipping");
		if(oldhash == newhash)
			ShowPopup(NULL, "AVH Debug", "Hashes are the same... skipping");
#endif
		return 0;
	}
	if(oldhash == newhash) // <-- Too scared to remove this atm :P
	{
#ifdef DBGPOPUPS
		ShowPopup(NULL, "AVH Debug", "New hash same as old hash... skipping");
#endif
		return 0;
	}
	db_dword_set((HANDLE)wParam, "AvatarHistory", "AvatarHash", newhash);
	
	char globpref = db_byte_get(NULL, "AvatarHistory", "LogToDisk", AVH_DEF_LOGTODISK);
	char userpref = db_byte_get((HANDLE)wParam, "AvatarHistory", "LogUser", BST_INDETERMINATE);
	if(globpref && (userpref != 0) || (userpref == 1) )
	{
		strncpy(ext, strrchr(avatar->szFilename, '.')+1, 10);

		GetContactFolder((HANDLE)wParam, fn);
	
		GetLocalTime(&curtime);
		sprintf(fn, "%s\\%04d-%02d-%02d %02dh%02dm%02ds.%s",fn, curtime.wYear, curtime.wMonth, curtime.wDay, curtime.wHour, curtime.wMinute, curtime.wSecond, ext);
		if(CopyFile(avatar->szFilename, fn, TRUE) == 0)
		{
			MessageBox(NULL, fn, "Unable to save avatar", MB_OK);
		}
#ifdef DBGPOPUPS
		else
		{
			ShowPopup(NULL, "AVH Debug", "File copied successfully");
		}
#endif
	}

	userpref = db_byte_get((HANDLE)wParam, "AvatarHistory", "PopupUser", BST_INDETERMINATE);
	globpref = db_byte_get(NULL, "AvatarHistory", "AvatarPopups", AVH_DEF_AVPOPUPS);
	if(globpref && (userpref != 0) || (userpref == 1) )
		ShowPopup((HANDLE)wParam, NULL, Translate("changed his/her avatar"));
	return 0;
}

int GetUIDFromHContact(HANDLE contact, char* protoout, char* uinout)
{
	struct MM_INTERFACE mmi;
	CONTACTINFO cinfo;
	char* proto;
	
	proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)contact,0);
	if(proto)
		strcpy(protoout, proto);
	else strcpy(protoout, "NULL");

	ZeroMemory(&cinfo,sizeof(CONTACTINFO));
	cinfo.cbSize = sizeof(CONTACTINFO);
	cinfo.hContact = contact;
	cinfo.dwFlag = CNF_UNIQUEID;
	if(CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&cinfo)==0)
	{
		if(cinfo.type==CNFT_ASCIIZ)
		{
			strcpy(uinout,cinfo.pszVal);
			// It is up to us to free the string
			// The catch? We need to use Miranda's free(), not our CRT's :)
			mmi.cbSize = sizeof(mmi);
			CallService(MS_SYSTEM_GET_MMI,0,(LPARAM)&mmi);
			mmi.mmi_free(cinfo.pszVal);
		}
		else if(cinfo.type == CNFT_DWORD)
		{
			char struin[MAX_PATH+1];
			_itoa(cinfo.dVal,struin,10);
			strcpy(uinout,struin);
		}
		else strcpy(uinout,"NULL");
	}
	else strcpy(uinout,"NULL");
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}


#define POLYNOMIAL (0x488781ED) /* This is the CRC Poly */
#define TOPBIT (1 << (WIDTH - 1)) /* MSB */
#define WIDTH 32

int GetFileHash(char* fn)
{
   int remainder = 0, byte;
   char* data = (char*)malloc(1024); int nBytes;
   DWORD dwRead = 0; HANDLE hFile;
   unsigned char bit;
   hFile = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
   if(hFile == INVALID_HANDLE_VALUE || data == NULL)
   {
#ifdef DBGPOPUPS
	LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
	ShowPopup(NULL, "AVH Debug", (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
#endif
	   return 0;
   }
#ifdef DBGPOPUPS
   else
   {
		ShowPopup(NULL, "AVH Debug", "File opened");
   }
   long cycles = 0;
#endif
   do
   {
	   // Read file chunk
	   ReadFile(hFile, data, 1024, &dwRead, NULL);
	   nBytes = dwRead;
#ifdef DBGPOPUPS
	   cycles++;
#endif
		/* loop through each byte of data */
		for (byte = 0; byte < nBytes; ++byte) {
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
  }while(dwRead == 1024);
   free(data);
   CloseHandle(hFile);
#ifdef DBGPOPUPS
   char text[MAX_PATH+1];
   sprintf(text, "Hashing: %d cycles, ending with %d bytes");
   ShowPopup(NULL, "AVH Debug", text);
	if(remainder == 0)
		ShowPopup(NULL, "AVH Debug", "Uh-oh...");
#endif
   return (remainder);
}

static int IsEnabled(WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_COMMAND:
			if (HIWORD(wParam) == STN_CLICKED)
			{ //It was a click on the Popup.
				PUDeletePopUp(hWnd);
				return TRUE;
			}
			break;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}



int ShowPopup(HANDLE hContact, char* title, char* text)
{
	POPUPDATA ppd;
	COLORREF colorBack = 0;
	COLORREF colorText = 0;
	char *sProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
	if(ServiceExists(MS_POPUP_ADDPOPUP))
	{
		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NEWAVATAR));
		if(!db_byte_get(NULL, "AvatarHistory", "UsePopupDefault", AVH_DEF_DEFPOPUPS))
		{
			colorBack = db_dword_get(NULL, "AvatarHistory", "PopupBG", AVH_DEF_POPUPBG);
			colorText = db_dword_get(NULL, "AvatarHistory", "PopupFG", AVH_DEF_POPUPFG);
		}
		ZeroMemory((void*)&ppd, sizeof(ppd)); //This is always a good thing to do.
		ppd.lchContact = (HANDLE)hContact; //Be sure to use a GOOD handle, since this will not be checked.
		ppd.lchIcon = hIcon;
		lstrcpy(ppd.lpzContactName, title?title:(char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0));
		lstrcpy(ppd.lpzText, text);
		ppd.colorBack = colorBack;
		ppd.colorText = colorText;
		ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
		ppd.PluginData = NULL;
		return CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
	}
	else
	{
		MessageBox(NULL, text, title?title:(char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0), MB_OK);
		return 0;
	}
}

void InitFolders()
{
	if(CallService(MS_DB_GETPROFILEPATH, MAX_PATH+1, (LPARAM)basedir) != 0)
			strcpy(basedir, "."); // Failed, use current dir
	
	strcat(basedir, "\\Avatars History");
	hFolder = (HANDLE)FoldersRegisterCustomPath(Translate("Avatars"), Translate("Avatar History"), basedir);
	FoldersGetCustomPath(hFolder, basedir, MAX_PATH+1, basedir);
}

char* GetContactFolder(HANDLE hContact, char* fn)
{
	char uin[MAX_PATH+1], proto[50+1];
		FoldersGetCustomPath(hFolder, fn, MAX_PATH+1, basedir);
		CreateDirectory(fn, NULL);		
		GetUIDFromHContact(hContact, proto, uin);
		sprintf(fn, "%s\\%s", fn, proto);
		CreateDirectory(fn, NULL);
		sprintf(fn, "%s\\%s",fn, uin);
		CreateDirectory(fn, NULL);
		return fn;
}