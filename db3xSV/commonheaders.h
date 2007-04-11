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

#define _WIN32_WINNT 0x0501
#include <windows.h>

#include <malloc.h>

#ifdef _DEBUG
#	define _ALPHA_BASE_ 1	// defined for CVS builds
#	define _ALPHA_FUSE_ 1	// defined for fuse powered core
#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>
#endif

#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <io.h>
#include <string.h>
#include <direct.h>
#include "resource.h"
#include <newpluginapi.h>
#include <win2k.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_clist.h>

extern PLUGINLINK *pluginLink;
extern char szMirandaDir[MAX_PATH];
extern size_t uiMirandaDirLen;
extern char szDbDir[MAX_PATH];
extern size_t uiDbDirLen;
extern char *szMirandaDirUtf8;
extern size_t uiMirandaDirLenUtf8;
extern char *szDbDirUtf8;
extern size_t uiDbDirLenUtf8;

extern struct MM_INTERFACE memoryManagerInterface;
extern struct LIST_INTERFACE li;

#define mir_alloc(n) memoryManagerInterface.mmi_malloc(n)
#define mir_free(ptr) memoryManagerInterface.mmi_free(ptr)
#define mir_realloc(ptr,size) memoryManagerInterface.mmi_realloc(ptr,size)

#ifdef __GNUC__
#define mir_i64(x) (x##LL)
#else
#define mir_i64(x) (x##i64)
#endif


#define DBVT_ASCIIZ_PATH    250	  //pszVal is valid
#define DBVT_UTF8_PATH	    249	  //pszVal is valid
#define DBVT_ASCIIZ_DB_PATH 248	  //pszVal is valid
#define DBVT_UTF8_DB_PATH	247	  //pszVal is valid
