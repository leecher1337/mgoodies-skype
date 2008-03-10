#include "Compatibility.h"

static HANDLE hContactDeletedEvent,hContactAddedEvent;

bool RegisterCompatibilityServices()
{
	CreateServiceFunction(MS_DB_CONTACT_GETCOUNT,GetContactCount);
	CreateServiceFunction(MS_DB_CONTACT_FINDFIRST,FindFirstContact);
	CreateServiceFunction(MS_DB_CONTACT_FINDNEXT,FindNextContact);
	CreateServiceFunction(MS_DB_CONTACT_DELETE,DeleteContact);
	CreateServiceFunction(MS_DB_CONTACT_ADD,AddContact);
	CreateServiceFunction(MS_DB_CONTACT_IS,IsDbContact);
	hContactDeletedEvent=CreateHookableEvent(ME_DB_CONTACT_DELETED);
	hContactAddedEvent=CreateHookableEvent(ME_DB_CONTACT_ADDED);
	return true;
}
bool UnRegisterCompatibilityServices()
{
	DestroyHookableEvent(hContactDeletedEvent);
	DestroyHookableEvent(hContactAddedEvent);
	return true;
}
int AddContact(WPARAM wParam,LPARAM lParam)
{
	int ret;
	ret=DBContactCreate(DBContactGetRoot(0,0),0);
	if(ret == DB_INVALIDPARAM)
		return 1;
	NotifyEventHooks(hContactAddedEvent,(WPARAM)ret,0);
	return ret;
}
int DeleteContact(WPARAM hContact,LPARAM lParam)
{
	int ret;
	ret=DBContactDelete(hContact,0); //what about settings and events?
	if(ret==DB_INVALIDPARAM)
		return 1;
	NotifyEventHooks(hContactDeletedEvent,hContact,0);
	return ret;
}
int IsDbContact(WPARAM hContact,LPARAM lParam)
{
	int flags = DBContactGetFlags(hContact, 0);
	return (flags != DB_INVALIDPARAM) && ((flags & DB_CF_IsGroup) == 0);
}
int GetContactCount(WPARAM wParam,LPARAM lParam)
{
	TDBContactHandle hContact=NULL;
	TDBContactIterFilter IterFilter = {0};
	IterFilter.cbSize = sizeof(IterFilter);
	IterFilter.fDontHasFlags=DB_CF_IsGroup|DB_CF_IsVirtual;
	TDBContactIterationHandle hIter=DBContactIterInit((WPARAM)&IterFilter,0);
	int nCount=0;
	while(hIter!=DB_INVALIDPARAM && hIter!=0)
	{
		hContact=DBContactIterNext(hIter,0);
		if(hContact!=0 && hContact!= DB_INVALIDPARAM)
			nCount++;
	}
	DBContactIterClose(hIter,0);
	return nCount;
}
int FindFirstContact(WPARAM wParam,LPARAM lParam)
{
	TDBContactHandle hContact=NULL;
	TDBContactIterFilter IterFilter = {0};
	IterFilter.cbSize = sizeof(IterFilter);
	IterFilter.fDontHasFlags=DB_CF_IsGroup|DB_CF_IsVirtual;
	TDBContactIterationHandle hIter=DBContactIterInit((WPARAM)&IterFilter,0);
	if(hIter!=DB_INVALIDPARAM && hIter!=0)
		hContact=DBContactIterNext(hIter,0);
	DBContactIterClose(hIter,0);
	if(hContact==0 || hContact== DB_INVALIDPARAM)
		return NULL;
	else
		return hContact;
}
int FindNextContact(WPARAM hContact,LPARAM lParam)
{
	TDBContactHandle res = 0;
	TDBContactIterFilter filter;
	filter.cbSize = sizeof(filter);
	filter.fDontHasFlags = DB_CF_IsGroup | DB_CF_IsVirtual;
	filter.Options = DB_CIFO_OSC_AC | DB_CIFO_OC_AC;

	if ((hContact == 0) || (hContact == gDataBase->getContacts().getRootContact()))
	{
		TDBContactIterationHandle hiter = DBContactIterInit((WPARAM)&filter, 0);
		if ((hiter == 0) || (hiter == DB_INVALIDPARAM))
			return 0;

		res = DBContactIterNext(hiter, 0);
		if (res == DB_INVALIDPARAM)
			res = 0;

		DBContactIterClose(hiter, 0);
	} else {
		TDBContactIterationHandle hiter = DBContactIterInit((WPARAM)&filter, 0);
		if ((hiter == 0) || (hiter == DB_INVALIDPARAM))
			return 0;

		res = DBContactIterNext(hiter, 0);
		while ((res != 0) && (res != DB_INVALIDPARAM) && (res != hContact))
			res = DBContactIterNext(hiter, 0);

		if ((res != 0) && (res != DB_INVALIDPARAM))
		{
			res = DBContactIterNext(hiter, 0);
			if (res == DB_INVALIDPARAM)
				res = 0;
		} else {
			res = 0;
		}

		DBContactIterClose(hiter, 0);		
	}

	return res;
}