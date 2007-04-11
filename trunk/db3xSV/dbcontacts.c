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

static int GetContactCount(WPARAM wParam,LPARAM lParam);
static int FindFirstContact(WPARAM wParam,LPARAM lParam);
static int FindNextContact(WPARAM wParam,LPARAM lParam);
static int DeleteContact(WPARAM wParam,LPARAM lParam);
static int AddContact(WPARAM wParam,LPARAM lParam);
static int IsDbContact(WPARAM wParam,LPARAM lParam);

extern CRITICAL_SECTION csDbAccess;
extern struct DBHeader dbHeader;
static HANDLE hContactDeletedEvent,hContactAddedEvent;

extern HANDLE hCacheHeap;
extern SortedList lContacts;

int InitContacts(void)
{
	CreateServiceFunction(MS_DB_CONTACT_GETCOUNT,GetContactCount);
	CreateServiceFunction(MS_DB_CONTACT_FINDFIRST,FindFirstContact);
	CreateServiceFunction(MS_DB_CONTACT_FINDNEXT,FindNextContact);
	CreateServiceFunction(MS_DB_CONTACT_DELETE,DeleteContact);
	CreateServiceFunction(MS_DB_CONTACT_ADD,AddContact);
	CreateServiceFunction(MS_DB_CONTACT_IS,IsDbContact);
	hContactDeletedEvent=CreateHookableEvent(ME_DB_CONTACT_DELETED);
	hContactAddedEvent=CreateHookableEvent(ME_DB_CONTACT_ADDED);
	return 0;
}

void UninitContacts(void)
{
}

static int GetContactCount(WPARAM wParam,LPARAM lParam)
{
	int ret;
	
	EnterCriticalSection(&csDbAccess);
	ret=dbHeader.contactCount;
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static int FindFirstContact(WPARAM wParam,LPARAM lParam)
{
	int ret = 0;
	EnterCriticalSection(&csDbAccess);
	ret = (int)(HANDLE)dbHeader.ofsFirstContact;
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static int FindNextContact(WPARAM wParam,LPARAM lParam)
{
	int ret, index;
	struct DBContact *dbc;
	DBCachedContactValueList *VL = NULL;
	
	EnterCriticalSection(&csDbAccess);
	{
		DBCachedContactValueList VLtemp;
		VLtemp.hContact = (HANDLE)wParam;
		if ( li.List_GetIndex(&lContacts,&VLtemp,&index)) {
			VL = ( DBCachedContactValueList* )lContacts.items[index];
			if ( VL->hNext != NULL ) {
				LeaveCriticalSection(&csDbAccess);
				return (int)VL->hNext;
	}	}	}

	dbc=(struct DBContact*)DBRead(wParam,sizeof(struct DBContact),NULL);
	if(dbc->signature!=DBCONTACT_SIGNATURE)
		ret=(int)(HANDLE)NULL;
	else {
		if ( VL == NULL ) {
			VL = (DBCachedContactValueList*)HeapAlloc(hCacheHeap,HEAP_NO_SERIALIZE+HEAP_ZERO_MEMORY,sizeof(DBCachedContactValueList));
			VL->hContact = (HANDLE)wParam;
			li.List_Insert(&lContacts,VL,index);
		}
		VL->hNext = (HANDLE)dbc->ofsNext;
		ret=(int)(HANDLE)dbc->ofsNext;
	}
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static int DeleteContact(WPARAM wParam,LPARAM lParam)
{
	struct DBContact *dbc,*dbcPrev;
	DWORD ofsThis,ofsNext,ofsFirstEvent;
	struct DBContactSettings *dbcs;
	struct DBEvent *dbe;
	int index;
	
	if((HANDLE)wParam==NULL) return 1;
	EnterCriticalSection(&csDbAccess);
	dbc=(struct DBContact*)DBRead(wParam,sizeof(struct DBContact),NULL);
	if(dbc->signature!=DBCONTACT_SIGNATURE) {
		LeaveCriticalSection(&csDbAccess);
		return 1;
	}
	if ( (HANDLE)wParam == (HANDLE)dbHeader.ofsUser ) {
		LeaveCriticalSection(&csDbAccess);
		log0("FATAL: del of user chain attempted.");
		return 1;
	}
	log0("del contact");
	LeaveCriticalSection(&csDbAccess);
	//call notifier while outside mutex
	NotifyEventHooks(hContactDeletedEvent,wParam,0);
	//get back in
	EnterCriticalSection(&csDbAccess);

	{	DBCachedContactValueList VLtemp;
		VLtemp.hContact = (HANDLE)wParam;
		if ( li.List_GetIndex(&lContacts,&VLtemp,&index))
		{
			DBCachedContactValueList *VL = ( DBCachedContactValueList* )lContacts.items[index];
			DBCachedContactValue* V = VL->first;
			while ( V != NULL ) {
				DBCachedContactValue* V1 = V->next;
				if ( V->value.type == DBVT_ASCIIZ )
					HeapFree( hCacheHeap, HEAP_NO_SERIALIZE, V->value.pszVal );
				HeapFree( hCacheHeap, HEAP_NO_SERIALIZE, V );
				V = V1;
			}
			HeapFree( hCacheHeap, HEAP_NO_SERIALIZE, VL );

			li.List_Remove(&lContacts,index);
	}	}

	dbc=(struct DBContact*)DBRead(wParam,sizeof(struct DBContact),NULL);
	//delete settings chain
	ofsThis=dbc->ofsFirstSettings;
	ofsFirstEvent=dbc->ofsFirstEvent;
	while(ofsThis) {
		dbcs=(struct DBContactSettings*)DBRead(ofsThis,sizeof(struct DBContactSettings),NULL);
		ofsNext=dbcs->ofsNext;
		DeleteSpace(ofsThis,offsetof(struct DBContactSettings,blob)+dbcs->cbBlob);
		ofsThis=ofsNext;
	}
	//delete event chain
	ofsThis=ofsFirstEvent;
	while(ofsThis) {
		dbe=(struct DBEvent*)DBRead(ofsThis,sizeof(struct DBEvent),NULL);
		ofsNext=dbe->ofsNext;
		DeleteSpace(ofsThis,offsetof(struct DBEvent,blob)+dbe->cbBlob);
		ofsThis=ofsNext;
	}
	//find previous contact in chain and change ofsNext
	dbc=(struct DBContact*)DBRead(wParam,sizeof(struct DBContact),NULL);
	if(dbHeader.ofsFirstContact==wParam) {
		dbHeader.ofsFirstContact=dbc->ofsNext;
		DBWrite(0,&dbHeader,sizeof(dbHeader));
	}
	else {
		ofsNext=dbc->ofsNext;
		ofsThis=dbHeader.ofsFirstContact;
		dbcPrev=(struct DBContact*)DBRead(ofsThis,sizeof(struct DBContact),NULL);
		while(dbcPrev->ofsNext!=wParam) {
			if(dbcPrev->ofsNext==0) DatabaseCorruption();
			ofsThis=dbcPrev->ofsNext;
			dbcPrev=(struct DBContact*)DBRead(ofsThis,sizeof(struct DBContact),NULL);
		}
		dbcPrev->ofsNext=ofsNext;
		DBWrite(ofsThis,dbcPrev,sizeof(struct DBContact));
		{
			DBCachedContactValueList VLtemp;
			VLtemp.hContact = (HANDLE)ofsThis;
			if ( li.List_GetIndex(&lContacts,&VLtemp,&index))
			{
				DBCachedContactValueList *VL = ( DBCachedContactValueList* )lContacts.items[index];
				VL->hNext = ( HANDLE )ofsNext;
		}	}
	}
	//delete contact
	DeleteSpace(wParam,sizeof(struct DBContact));
	//decrement contact count
	dbHeader.contactCount--;
	DBWrite(0,&dbHeader,sizeof(dbHeader));
	DBFlush(0);
	//quit
	LeaveCriticalSection(&csDbAccess);
	return 0;
}

static int AddContact(WPARAM wParam,LPARAM lParam)
{
	struct DBContact dbc;
	DWORD ofsNew;

	log0("add contact");
	EnterCriticalSection(&csDbAccess);
	ofsNew=CreateNewSpace(sizeof(struct DBContact));
	dbc.signature=DBCONTACT_SIGNATURE;
	dbc.eventCount=0;
	dbc.ofsFirstEvent=dbc.ofsLastEvent=0;
	dbc.ofsFirstSettings=0;
	dbc.ofsNext=dbHeader.ofsFirstContact;
	dbc.ofsFirstUnreadEvent=0;
	dbc.timestampFirstUnread=0;
	dbHeader.ofsFirstContact=ofsNew;
	dbHeader.contactCount++;
	DBWrite(ofsNew,&dbc,sizeof(struct DBContact));
	DBWrite(0,&dbHeader,sizeof(dbHeader));
	DBFlush(0);

	{	int index;

		DBCachedContactValueList *VL = (DBCachedContactValueList*)HeapAlloc(hCacheHeap,HEAP_NO_SERIALIZE+HEAP_ZERO_MEMORY,sizeof(DBCachedContactValueList));
		VL->hContact = (HANDLE)ofsNew;

		li.List_GetIndex(&lContacts,VL,&index);
		li.List_Insert(&lContacts,VL,index);
	}

	LeaveCriticalSection(&csDbAccess);
	NotifyEventHooks(hContactAddedEvent,(WPARAM)ofsNew,0);
	return (int)ofsNew;
} 

static int IsDbContact(WPARAM wParam,LPARAM lParam)
{
	struct DBContact dbc;
	DWORD ofsContact=(DWORD)wParam;
	int ret;

	EnterCriticalSection(&csDbAccess);
	{
		int index;
		DBCachedContactValueList VLtemp,*VL;
		VLtemp.hContact = (HANDLE)wParam;
		if ( li.List_GetIndex(&lContacts,&VLtemp,&index))
			ret = TRUE;
		else {
			dbc=*(struct DBContact*)DBRead(ofsContact,sizeof(struct DBContact),NULL);
			ret=dbc.signature==DBCONTACT_SIGNATURE;

			if (ret) {
				VL = (DBCachedContactValueList*)HeapAlloc(hCacheHeap,HEAP_NO_SERIALIZE+HEAP_ZERO_MEMORY,sizeof(DBCachedContactValueList));
				VL->hContact = (HANDLE)wParam;
				li.List_Insert(&lContacts,VL,index);
	}	}	}

	LeaveCriticalSection(&csDbAccess);
	return ret;
}
