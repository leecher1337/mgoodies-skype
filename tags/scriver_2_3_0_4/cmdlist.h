/*
AOL Instant Messenger Plugin for Miranda IM

Copyright (c) 2003 Robert Rainwater

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
#ifndef SRMM_CMDLIST_H
#define SRMM_CMDLIST_H

#ifdef _UNICODE
	#define CMDCHAR TCHAR
#else
	#define CMDCHAR char
#endif

typedef struct _TCmdList {
	struct _TCmdList *next;
	struct _TCmdList *prev;
	HANDLE  hContact;
	TCHAR *szCmd;
	unsigned long hash;
} TCmdList;

TCmdList *tcmdlist_append(TCmdList *list, CMDCHAR *data);
TCmdList *tcmdlist_remove(TCmdList *list, CMDCHAR *data);
TCmdList *tcmdlist_append2(TCmdList *list, HANDLE hContact, CMDCHAR *data);
TCmdList *tcmdlist_remove2(TCmdList *list, HANDLE hContact);
TCmdList *tcmdlist_get2(TCmdList *list, HANDLE hContact);
int tcmdlist_len(TCmdList *list);
TCmdList *tcmdlist_last(TCmdList *list);
void tcmdlist_free(TCmdList * list);

#endif
