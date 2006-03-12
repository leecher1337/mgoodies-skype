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

extern "C"
{
#include <newpluginapi.h>
#include <m_system.h>
}

static struct MM_INTERFACE mmInterface;


void init_mir_malloc()
{
	mmInterface.cbSize = sizeof(mmInterface);
	CallService(MS_SYSTEM_GET_MMI, 0, (LPARAM)&mmInterface);
}


void * mir_alloc(size_t size) 
{
	if (size <= 0)
		return NULL;

	return mmInterface.mmi_malloc(size);
}

void * mir_alloc0(size_t size) 
{
	void * ptr = mir_alloc(size);

	if (ptr != NULL)
		memset(ptr, 0, size);

	return ptr;
}

void * mir_realloc(void *ptr, size_t size) 
{
	return mmInterface.mmi_realloc(ptr, size);
}

void mir_free(void *ptr) 
{
	if (ptr)
		mmInterface.mmi_free(ptr);
}

char *mir_dup(const char *ptr) 
{
	if (ptr)
	{
		char *ret = (char *) mir_alloc(strlen(ptr)+1);
		if (ret != NULL)
			strcpy(ret, ptr);
		return ret;
	}
	else
	{
		return NULL;
	}
}

wchar_t *mir_dupW(const wchar_t *ptr) 
{
	if (ptr)
	{
		wchar_t *ret = (wchar_t *) mir_alloc((wcslen(ptr) + 1) * sizeof(wchar_t));
		if (ret != NULL)
			wcscpy(ret, ptr);
		return ret;
	}
	else
	{
		return NULL;
	}
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

