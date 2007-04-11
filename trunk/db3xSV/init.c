/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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
*/

#include "commonheaders.h"
#include "database.h"
#ifdef SECUREDB
  #include "SecureDB.h"
#endif
#include "initmenu.h"
#include <m_plugins.h>
#include "dblists.h"

struct MM_INTERFACE   mmi;
struct LIST_INTERFACE li;
struct UTF8_INTERFACE utfi;

extern char szDbPath[MAX_PATH];

HINSTANCE g_hInst=NULL;
PLUGINLINK *pluginLink;

static int getCapability( int flag )
{
	return 0;
}

// returns 0 if the profile is created, EMKPRF*
static int makeDatabase(char * profile, int * error)
{
	HANDLE hFile=CreateFile(profile, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		CreateDbHeaders(hFile);
		CloseHandle(hFile);
		return 0;
	}
	if ( error != NULL ) *error=EMKPRF_CREATEFAILED;
	return 1;
}

// returns 0 if the given profile has a valid header
static int grokHeader( char * profile, int * error )
{
	int rc=1;
	int chk=0;
#ifdef SECUREDB
	int sec=0;
#endif
	struct DBHeader hdr;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dummy=0;

	hFile = CreateFile(profile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) { 		
		if ( error != NULL ) *error=EGROKPRF_CANTREAD;
		return 1;
	}

	// read the header, which can fail (for various reasons)
	if ( !
#ifdef SECUREDB
		EncReadFile
#else
		ReadFile //Yes! Not VirtualReadFile
#endif
          (hFile, &hdr, sizeof(struct DBHeader), &dummy, NULL) ) {
		if ( error != NULL) *error=EGROKPRF_CANTREAD;
		CloseHandle(hFile);
		return 1;
	}

	chk=CheckDbHeaders(&hdr
#ifdef SECUREDB
	  ,&sec
#endif
	  );



	if ( chk == 0 ) {
		// all the internal tests passed, hurrah
		rc=0;
		if ( error != NULL ) *error=0;
	} else {
		// didn't pass at all, or some did.
		switch ( chk ) {
			case 1:
			{
				// "Miranda ICQ DB" wasn't present
				if ( error != NULL ) *error = EGROKPRF_UNKHEADER;
				break;
			}
			case 2:
			{
				// header was present, but version information newer
				if ( error != NULL ) *error= EGROKPRF_VERNEWER;
				break;
			}
			case 3:
			{
				// header/version OK, internal data missing
				if ( error != NULL ) *error=EGROKPRF_DAMAGED;
				break;
			}
		} // switch
	} //if
	CloseHandle(hFile);
	return rc;
}

char* deprecatedUtf8Decode( char* str, WCHAR** ucs2 );
char* deprecatedUtf8Encode( const char* str );
char* deprecatedUtf8EncodeW( const WCHAR* wstr );
// returns 0 if all the APIs are injected otherwise, 1
int LoadDatabase( char * profile, void * plink )
{
	PLUGINLINK *link = plink;
#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	// don't need thread notifications
	strncpy(szDbPath, profile, sizeof(szDbPath));
	szDbPath[sizeof(szDbPath)-1]=0;
	// this is like Load()'s pluginLink
	pluginLink=link;
	// set the memory manager
	memoryManagerInterface.cbSize=sizeof(struct MM_INTERFACE);
	CallService(MS_SYSTEM_GET_MMI,0,(LPARAM)&memoryManagerInterface);
	// set the lists manager;
	li.cbSize = sizeof( li );
	if ( mir_getLI( &li ) == CALLSERVICE_NOTFOUND )
	{		//Will use local Lists implementation
		li.List_Create   = NULL;
		li.List_Destroy  = List_Destroy;
		li.List_Find     = NULL;
		li.List_GetIndex = List_GetIndex;
		li.List_Insert   = List_Insert;
		li.List_Remove   = List_Remove;
		li.List_IndexOf  = NULL;
	}
	mir_getMMI( &mmi );
	if (mir_getUTFI( &utfi ) == CALLSERVICE_NOTFOUND ) {
		// older core version. use local functions.
		utfi.utf8_decode   = deprecatedUtf8Decode;
		utfi.utf8_decodecp = NULL;
		utfi.utf8_encode   = deprecatedUtf8Encode;
		utfi.utf8_encodecp = NULL;
		utfi.utf8_encodeW  = deprecatedUtf8EncodeW;
	}
	{
		char *p;
		strncpy(szDbDir,szDbPath,sizeof(szDbDir));
		p = strrchr(szDbDir, '\\');
		if ( p != NULL )
			*(p+1)=0;
		uiDbDirLen = strlen(szDbDir);

		szDbDirUtf8 = mir_utf8encode(szDbDir);
		uiDbDirLenUtf8 = strlen(szDbDirUtf8);
	}

	extraOnLoad();

	// inject all APIs and hooks into the core
	return LoadDatabaseModule();
}

static int UnloadDatabase(int wasLoaded)
{
	if ( !wasLoaded) return 0;
	mir_free(szDbDirUtf8);
	UnloadDatabaseModule();
	return 0;
}

static int getFriendlyName( char * buf, size_t cch, int shortName )
{
	strncpy(buf,shortName ? 

#ifdef SECUREDB
	"Miranda VirtSecDB"
#else
	"Miranda VirtualDB"
#endif
	: 
#ifdef SECUREDB
	"Miranda Virtualizable SecureDB support"
#else
	"Miranda Virtualizable database support"
#endif
	,cch);
	return 0;
}


static DATABASELINK dblink = {
	sizeof(DATABASELINK),
	getCapability,
	getFriendlyName,
	makeDatabase,
	grokHeader,
	LoadDatabase,
	UnloadDatabase,
};

static PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
#ifdef SECUREDB
	"Miranda Virtualizable SecureDB driver",
#else
	"Miranda Virtualizable database driver",
#endif
	PLUGIN_MAKE_VERSION(0,6,0,8),
#ifdef SECUREDB
	"Provides Miranda database support: encryption, virtualisation, global settings, contacts, history, settings per contact, optimized paths.",
	"Miranda-IM project; YB; Piotr Pawluczuk; Pescuma",
	"yb@saaplugin.no-ip.info",
	"Copyright 2000-2007 Miranda-IM project; Piotr Pawluczuk; YB; Pescuma",
#else
	"Provides Miranda database support: virtualisation, global settings, contacts, history, settings per contact, optimized paths.",
	"Miranda-IM project; YB; Pescuma",
	"yb@saaplugin.no-ip.info; piotrek@piopawlu.net",
	"Copyright 2000-2007 Miranda-IM project; YB; Pescuma",
#endif
	"http://saaplugin.no-ip.info/db3xV/",
	0,
	DEFMOD_DB,
#ifdef SECUREDB
	{0x54f6aa80,0x56de,0x478f,{0x97,0x1b,0x66,0xbf,0x9e,0x65,0x1a,0x0c}} // 54f6aa80-56de-478f-971b-66bf9e651a0c
#else
	{0xb0190e91,0x39cf,0x4ee2,{0xb6,0x39,0xf1,0x84,0xae,0x19,0xd4,0xc1}} // b0190e91-39cf-4ee2-b639-f184ae19d4c1
#endif
};


BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	return TRUE;
}

__declspec(dllexport) DATABASELINK* DatabasePluginInfo(void * reserved)
{
	return &dblink;
}

__declspec(dllexport) PLUGININFOEX * MirandaPluginInfo(DWORD mirandaVersion)
{
	if ( mirandaVersion < PLUGIN_MAKE_VERSION(0,4,0,0)) {
		MessageBox( NULL, _T("The db3x plugin cannot be loaded. Your Miranda is too old."), _T("db3x Plugin"), MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
		return NULL;
	}
	if ( mirandaVersion < PLUGIN_MAKE_VERSION( 0,7,0,17 )) pluginInfo.cbSize = sizeof( PLUGININFO );
	return &pluginInfo;
}

__declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof( PLUGININFOEX );
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_DATABASE, MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	return 1;
}

int __declspec(dllexport) Unload(void)
{
	return 0;
}
