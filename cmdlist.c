/*
Scriver

Copyright 2000-2005 Miranda ICQ/IM project, 
Copyright 2005 Piotr Piastucki

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commonheaders.h"

static unsigned long tcmdlist_hash(const TCHAR *data) {
	unsigned long hash = 0;
	int i, shift = 0;

	for(i=0; data[i]; i++) {
		hash ^= data[i]<<shift;
		if(shift>24) hash ^= (data[i]>>(32-shift))&0x7F;
		shift = (shift+5)&0x1F;
	}
	return hash;
}

TCmdList *tcmdlist_append(TCmdList *list, TCHAR *data) {
	TCmdList *n;
	TCmdList *new_list = mir_alloc(sizeof(TCmdList));
	TCmdList *attach_to = NULL;
	
	if (!data) {
		mir_free(new_list);
		return list;
	}
	new_list->next = NULL;
	new_list->szCmd = mir_tstrdup(data);
	new_list->hash = tcmdlist_hash(data);
	for (n=list; n!=NULL; n=n->next) {
		attach_to = n;
	}
	if (attach_to==NULL) {
		new_list->prev = NULL;
		return new_list;
	} 
	else {
		new_list->prev = attach_to;
		attach_to->next = new_list;
		if (tcmdlist_len(list)>20) {
			list = tcmdlist_remove(list, list->szCmd);
		}
		return list;
	}
}

TCmdList *tcmdlist_remove_first(TCmdList *list) {
	TCmdList *n = list;
	if (n->next) n->next->prev = n->prev;
	if (n->prev) n->prev->next = n->next;
	list = n->next;
	mir_free(n->szCmd);
	mir_free(n);
	return list;
}


TCmdList *tcmdlist_remove(TCmdList *list, TCHAR *data) {
	TCmdList *n;
	unsigned long hash;

	if (!data) return list;
	hash = tcmdlist_hash(data);
	for (n=list; n!=NULL; n=n->next) {
		if (n->hash==hash&&!_tcscmp(n->szCmd, data)) {
			if (n->next) n->next->prev = n->prev;
			if (n->prev) n->prev->next = n->next;
			if (n==list) list = n->next;
			mir_free(n->szCmd);
			mir_free(n);
			return list;
		}
	}
	return list;
}

TCmdList *tcmdlist_append2(TCmdList *list, HANDLE hContact, TCHAR *data) {
	TCmdList *n;
	TCmdList *new_list = mir_alloc(sizeof(TCmdList));
	TCmdList *attach_to = NULL;
	
	if (!data) {
		mir_free(new_list);
		return list;
	}
	new_list->next = NULL;
	new_list->hContact = hContact;
	new_list->szCmd = mir_tstrdup(data);
	new_list->hash = tcmdlist_hash(data);
	list = tcmdlist_remove2(list, hContact);
	for (n=list; n!=NULL; n=n->next) {
		attach_to = n;
	}
	if (attach_to==NULL) {
		new_list->prev = NULL;
		return new_list;
	} 
	else {
		new_list->prev = attach_to;
		attach_to->next = new_list;
		return list;
	}
}

TCmdList *tcmdlist_remove2(TCmdList *list, HANDLE hContact) {
	TCmdList *n;
	for (n=list; n!=NULL; n=n->next) {
		if (n->hContact==hContact) {
			if (n->next) n->next->prev = n->prev;
			if (n->prev) n->prev->next = n->next;
			if (n==list) list = n->next;
			mir_free(n->szCmd);
			mir_free(n);
			return list;
		}
	}
	return list;
}

TCmdList *tcmdlist_get2(TCmdList *list, HANDLE hContact) {
	TCmdList *n;
	for (n=list; n!=NULL; n=n->next) {
		if (n->hContact==hContact) {
			return n;
		}
	}
	return NULL;
}

int tcmdlist_len(TCmdList *list) {
	TCmdList *n;
	int i = 0;

	for (n=list; n!=NULL; n=n->next) {
		i++;
	}
	return i;
}

TCmdList *tcmdlist_last(TCmdList *list) {
	TCmdList *n;

	for (n=list; n!=NULL; n=n->next) {
		if (!n->next) 
			return n;
	}
	return NULL;
}

void tcmdlist_free(TCmdList *list) {
	TCmdList *n = list, *next;

	while (n!=NULL) {
		next = n->next;
		mir_free(n->szCmd);
		mir_free(n);
		n = next;
	}
}

