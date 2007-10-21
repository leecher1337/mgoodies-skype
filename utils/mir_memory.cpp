/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


#include "mir_memory.h"

#define MIRANDA_VER 0x0700
#include <newpluginapi.h>
#include <m_system.h>


struct MM_INTERFACE mmi;
struct UTF8_INTERFACE utfi;

void init_mir_malloc()
{
	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);
}


void * mir_alloc0(size_t size) 
{
	void * ptr = mir_alloc(size);

	if (ptr != NULL)
		memset(ptr, 0, size);

	return ptr;
}

char *mir_dupToAscii(WCHAR *ptr)
{
	if (ptr == NULL)
		return NULL;

	size_t size = lstrlenW(ptr) + 1;
	char *tmp = (char *) mir_alloc0(size);

	WideCharToMultiByte(CP_ACP, 0, ptr, -1, tmp, size, NULL, NULL);

	return tmp;
}

WCHAR *mir_dupToUnicode(char *ptr)
{
	if (ptr == NULL)
		return NULL;

	size_t size = strlen(ptr) + 1;
	WCHAR *tmp = (WCHAR *) mir_alloc0(size * sizeof(WCHAR));

	MultiByteToWideChar(CP_ACP, 0, ptr, -1, tmp, size * sizeof(WCHAR));

	return tmp;
}

int strcmpnull(char *str1, char *str2)
{
	if ( str1 == NULL && str2 == NULL )
		return 0;
	if ( str1 != NULL && str2 == NULL )
		return 1;
	if ( str1 == NULL && str2 != NULL )
		return -1;

   return strcmp(str1, str2);
}

int strcmpnullW(WCHAR *str1, WCHAR *str2)
{
	if ( str1 == NULL && str2 == NULL )
		return 0;
	if ( str1 != NULL && str2 == NULL )
		return 1;
	if ( str1 == NULL && str2 != NULL )
		return -1;

   return lstrcmpW(str1, str2);
}
