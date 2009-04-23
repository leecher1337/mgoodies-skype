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
#include "virtdb.h"

int ProfileManager(char *szDbDest,int cbDbDest);
int ShouldAutoCreate(void);
int CreateDbHeaders(HANDLE hFile);
int InitialiseDbHeaders(void);
int InitSettings(void);
void UninitSettings(void);
int InitContacts(void);
void UninitContacts(void);
int InitEvents(void);
void UninitEvents(void);
int InitCrypt(void);
int InitTime(void);
int InitModuleNames(void);
void UninitModuleNames(void);
int InitCache(void);
void UninitCache(void);
int InitIni(void);
void UninitIni(void);

HANDLE hDbFile=INVALID_HANDLE_VALUE;
CRITICAL_SECTION csDbAccess;
struct DBHeader dbHeader;
char szDbPath[MAX_PATH];
char szDbDir[MAX_PATH];
size_t uiDbDirLen;
char szMirandaDir[MAX_PATH];
size_t uiMirandaDirLen;
char *szMirandaDirUtf8;
size_t uiMirandaDirLenUtf8;
char *szDbDirUtf8;
size_t uiDbDirLenUtf8;

static void UnloadDatabase(void)
{
	if (isDBvirtual) free(virtualdb);
	else CloseHandle(hDbFile);
}

DWORD CreateNewSpace(int bytes)
{
	DWORD ofsNew;

	ofsNew=dbHeader.ofsFileEnd;
	dbHeader.ofsFileEnd+=bytes;
	DBWrite(0,&dbHeader,sizeof(dbHeader));
	log2("newspace %d@%08x",bytes,ofsNew);
	return ofsNew;
}

void DeleteSpace(DWORD ofs,int bytes)
{
	PBYTE buf;
	log2("deletespace %d@%08x",bytes,ofs);
	dbHeader.slackSpace+=bytes;
	DBWrite(0,&dbHeader,sizeof(dbHeader));
	buf=(PBYTE)mir_alloc(bytes);
	memset(buf,0,bytes);
	DBWrite(ofs,buf,bytes);
	mir_free(buf);
}

void UnloadDatabaseModule(void)
{
	//UninitIni();
	UninitEvents();
	UninitSettings();
	UninitContacts();
	UninitModuleNames();
	UninitCache();
	UnloadDatabase();
	DeleteCriticalSection(&csDbAccess);
}

static INT_PTR GetProfileName(WPARAM wParam, LPARAM lParam)
{
	char * p = 0;
	p = strrchr(szDbPath, '\\');
	if ( p == 0 ) return 1;
	p++;
	strncpy((char*)lParam, p, (size_t) wParam);
	return 0;
}

static INT_PTR GetProfilePath(WPARAM wParam, LPARAM lParam)
{
	char * dst = (char*)lParam;
	char * p = 0;
	strncpy(dst,szDbPath,wParam);
	p = strrchr(dst, '\\');
	if ( p == NULL ) return 1;
	*p=0;
	return 0;
}

int LoadDatabaseModule(void)
{
#ifdef SECUREDB
	static int sec=0;
#endif
	InitializeCriticalSection(&csDbAccess);
	parseIniSettings();
	log0("DB logging running");
	{
		DWORD dummy=0;
		hDbFile=CreateFileA(szDbPath,GENERIC_READ|(virtOnBoot?0:GENERIC_WRITE), FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
		if ( hDbFile == INVALID_HANDLE_VALUE ) {
			return 1;
		}

		if ( !
#ifdef SECUREDB
			  EncReadFile
#else
			  ReadFile
#endif
			  (hDbFile,&dbHeader,sizeof(dbHeader),&dummy,NULL) ) {
			CloseHandle(hDbFile);
			return 1;
		}
	}

	CheckDbHeaders(&dbHeader
#ifdef SECUREDB
	  ,&sec
#endif
	  );
#ifdef SECUREDB
	if(sec && EncGetPassword(&dbHeader,szDbPath)){
		return 1;
	}
#endif

	{
		char *p;
		GetModuleFileName(GetModuleHandle(NULL),szMirandaDir,sizeof(szMirandaDir));
		p = strrchr(szMirandaDir,'\\');
		if( p != NULL )
			*(p+1)=0;
		uiMirandaDirLen = strlen(szMirandaDir);

		szMirandaDirUtf8 = mir_utf8encode(szMirandaDir);
		uiMirandaDirLenUtf8 = strlen(szMirandaDirUtf8);
	}

	if (virtOnBoot) virtualizeDB();

	//if(ParseCommandLine()) return 1;
	if(InitCache()) return 1;
	if(InitModuleNames()) return 1;
	if(InitContacts()) return 1;
	if(InitSettings()) return 1;
	if(InitEvents()) return 1;
	if(InitCrypt()) return 1;
	//if(InitTime()) return 1;
	//if(InitIni()) return 1;
	CreateServiceFunction(MS_DB_GETPROFILENAME,GetProfileName);
	CreateServiceFunction(MS_DB_GETPROFILEPATH,GetProfilePath);
	return 0;
}

static DWORD DatabaseCorrupted=0;

void __cdecl dbpanic(void *arg)
{
	
	MessageBox(0,TranslateT("Miranda has detected corruption in your database. This corruption maybe fixed by DBTool.  Please download it from http://www.miranda-im.org. Miranda will now shutdown."),TranslateT("Database Panic"),MB_SETFOREGROUND|MB_TOPMOST|MB_APPLMODAL|MB_ICONWARNING|MB_OK);
	TerminateProcess(GetCurrentProcess(),255);
	return;
}

void DatabaseCorruption(void)
{
	int kill=0;

	EnterCriticalSection(&csDbAccess);
	if (DatabaseCorrupted==0) {
		DatabaseCorrupted++;
		kill++;
	} else {
		/* db is already corrupted, someone else is dealing with it, wait here
		so that we don't do any more damage */
		LeaveCriticalSection(&csDbAccess);
		Sleep(INFINITE);
		return;
	}
	LeaveCriticalSection(&csDbAccess);
	if (kill) {
		_beginthread(dbpanic,0,NULL);
		Sleep(INFINITE);
	}
}

#ifdef DBLOGGING
__inline static int mir_vsnprintf(char *buffer, size_t count, const char* fmt, va_list va) {
	int len;

	len = _vsnprintf(buffer, count-1, fmt, va);
	buffer[count-1] = 0;
	return len;
}

void DBLog(const char *file,int line,const char *fmt,...)
{
	FILE *fp;
	va_list vararg;
	char str[1024];

	va_start(vararg,fmt);
	mir_vsnprintf(str,sizeof(str),fmt,vararg);
	va_end(vararg);
	fp=fopen("c:\\mirandadatabase.log.txt","at");
	fprintf(fp,"%u: %s %d: %s\n",GetTickCount(),file,line,str);
	fclose(fp);
}
#endif
