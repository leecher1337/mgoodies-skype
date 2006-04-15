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


#include "mir_log.h"

#include <stdio.h>

extern "C"
{
#include <newpluginapi.h>
#include <m_netlib.h>
#include <m_protocols.h>
#include <m_clist.h>
}



int mlog(const char *module, const char *function, const char *fmt, ...)
{
#ifdef ENABLE_LOG

    va_list va;
    char text[1024];
	size_t len;

	mir_snprintf(text, sizeof(text) - 10, "[%08u - %08u] [%s] [%s] ", 
				 GetCurrentThreadId(), GetTickCount(), module, function);
	len = strlen(text);

    va_start(va, fmt);
    mir_vsnprintf(&text[len], sizeof(text) - len, fmt, va);
    va_end(va);

#ifdef LOG_TO_NETLIB

    return CallService(MS_NETLIB_LOG, NULL, (LPARAM) text);

#else
	char file[256];
	_snprintf(file, sizeof(file), "c:\\miranda_%s.log.txt", module);

	FILE *fp = fopen(file,"at");

	if (fp != NULL)
	{
		fprintf(fp, "%s\n", text);
		fclose(fp);
		return 0;
	}
	else
	{
		return -1;
	}
	
#endif

#else

	return 0;

#endif
}

int mlogC(const char *module, const char *function, HANDLE hContact, const char *fmt, ...)
{
#ifdef ENABLE_LOG

    va_list va;
    char text[1024];
	size_t len;
	char *name = (char*) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

	mir_snprintf(text, sizeof(text) - 10, "[%08u - %08u] [%s] [%s] [%08d - %s - %s] ", 
				 GetCurrentThreadId(), GetTickCount(), module, function,
				 hContact, proto == NULL ? "" : proto, name == NULL ? "" : name);
	len = strlen(text);

    va_start(va, fmt);
    mir_vsnprintf(&text[len], sizeof(text) - len, fmt, va);
    va_end(va);

#ifdef LOG_TO_NETLIB

    return CallService(MS_NETLIB_LOG, NULL, (LPARAM) text);

#else
	char file[256];
	_snprintf(file, sizeof(file), "c:\\miranda_%s.log.txt", module);

	FILE *fp = fopen(file,"at");

	if (fp != NULL)
	{
		fprintf(fp, "%s\n", text);
		fclose(fp);
		return 0;
	}
	else
	{
		return -1;
	}
	
#endif

#else

	return 0;

#endif
}
