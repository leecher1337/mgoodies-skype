#include "Services.h"

HANDLE gServices[18] = {0};

int DBEntryGetRoot(WPARAM wParam, LPARAM lParam)
{
	return gDataBase->getRootEntry();
}
int DBEntryChildCount(WPARAM hEntry, LPARAM lParam)
{
	if (hEntry == 0)
		hEntry = gDataBase->getRootEntry();

	return gDataBase->getEntries().getChildCount(hEntry);
}
int DBEntryGetParent(WPARAM hEntry, LPARAM lParam)
{
	if (hEntry == 0)
		hEntry = gDataBase->getRootEntry();

	return gDataBase->getEntries().getParent(hEntry);
}
int DBEntrySetParent(WPARAM hEntry, LPARAM hParent)
{
	if ((hEntry == 0) || (hEntry == gDataBase->getRootEntry()))
		return DB_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getRootEntry();

	return gDataBase->getEntries().setParent(hEntry, hParent);
}
int DBEntryGetFirstChild(WPARAM hParent, LPARAM lParam)
{
	if (hParent == 0)
		hParent = gDataBase->getRootEntry();

	return gDataBase->getEntries().getFirstChild(hParent);
}
int DBEntryGetLastChild(WPARAM hParent, LPARAM lParam)
{
	if (hParent == 0)
		hParent = gDataBase->getRootEntry();

	return gDataBase->getEntries().getLastChild(hParent);
}
int DBEntryGetNextSilbing(WPARAM hEntry, LPARAM lParam)
{
	if (hEntry == 0)
		hEntry = gDataBase->getRootEntry();

	return gDataBase->getEntries().getNextSilbing(hEntry);
}
int DBEntryGetPrevSilbing(WPARAM hEntry, LPARAM lParam)
{
	if (hEntry == 0)
		hEntry = gDataBase->getRootEntry();

	return gDataBase->getEntries().getPrevSilbing(hEntry);
}
int DBEntryGetFlags(WPARAM hEntry, LPARAM lParam)
{
	if (hEntry == 0)
		hEntry = gDataBase->getRootEntry();

	return gDataBase->getEntries().getFlags(hEntry);
}
int DBEntryIterInit(WPARAM Filter, LPARAM lParam)
{
	TDBEntryIterFilter fil = {0};
	if (Filter == NULL)
	{
		Filter = (WPARAM)&fil;
		fil.cbSize = sizeof(fil);
	}

	if (((PDBEntryIterFilter)Filter)->cbSize != sizeof(TDBEntryIterFilter))
		return DB_INVALIDPARAM;

	if (((PDBEntryIterFilter)Filter)->fDontHasFlags & ((PDBEntryIterFilter)Filter)->fHasFlags)
		return DB_INVALIDPARAM;

	if (((PDBEntryIterFilter)Filter)->hParentEntry == 0)
		((PDBEntryIterFilter)Filter)->hParentEntry = gDataBase->getRootEntry();

	return gDataBase->getEntries().IterationInit(*(PDBEntryIterFilter)Filter);
}
int DBEntryIterNext(WPARAM hIteration, LPARAM lParam)
{
	return gDataBase->getEntries().IterationNext(hIteration);
}
int DBEntryIterClose(WPARAM hIteration, LPARAM lParam)
{
	return gDataBase->getEntries().IterationClose(hIteration);
}
int DBEntryDelete(WPARAM hEntry, LPARAM lParam)
{
	if ((hEntry == 0) || (hEntry == gDataBase->getRootEntry()))
		return DB_INVALIDPARAM;

	return gDataBase->getEntries().DeleteEntry(hEntry);
}
int DBEntryCreate(WPARAM hParent, LPARAM Flags)
{
	if (hParent == 0)
		hParent = gDataBase->getRootEntry();

	Flags = Flags & ~(DB_EF_HasChilds | DB_EF_IsVirtual | DB_EF_HasVirtuals); // forbidden flags...
	return gDataBase->getEntries().CreateEntry(hParent, Flags);
}

int DBVirtualEntryCreate(WPARAM hEntry, LPARAM hParent)
{
	if ((hEntry == 0) || (hEntry == gDataBase->getRootEntry()))
		return DB_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getRootEntry();

	return gDataBase->getEntries().CreateVirtualEntry(hEntry, hParent);
}
int DBVirtualEntryGetParent(WPARAM hVirtuaEntry, LPARAM lParam)
{
	if ((hVirtuaEntry == 0) || (hVirtuaEntry == gDataBase->getRootEntry()))
		return DB_INVALIDPARAM;

	return gDataBase->getVirtuals().getParent(hVirtuaEntry);
}
int DBVirtualEntryGetFirst(WPARAM hEntry, LPARAM lParam)
{
	if ((hEntry == 0) || (hEntry == gDataBase->getRootEntry()))
		return DB_INVALIDPARAM;
	
	return gDataBase->getVirtuals().getFirst(hEntry);
}
int DBVirtualEntryGetNext(WPARAM hVirtualEntry, LPARAM lParam)
{
	if ((hVirtualEntry == 0) || (hVirtualEntry == gDataBase->getRootEntry()))
		return DB_INVALIDPARAM;

	return gDataBase->getVirtuals().getNext(hVirtualEntry);
}




bool RegisterServices()
{
	gServices[ 0] = CreateServiceFunction(MS_DB_ENTRY_GETROOT, DBEntryGetRoot);
	gServices[ 1] = CreateServiceFunction(MS_DB_ENTRY_CHILDCOUNT, DBEntryChildCount);
	gServices[ 2] = CreateServiceFunction(MS_DB_ENTRY_GETPARENT, DBEntryGetParent);
	gServices[ 3] = CreateServiceFunction(MS_DB_ENTRY_SETPARENT, DBEntrySetParent);
	gServices[ 4] = CreateServiceFunction(MS_DB_ENTRY_GETFIRSTCHILD, DBEntryGetFirstChild);
	gServices[ 5] = CreateServiceFunction(MS_DB_ENTRY_GETLASTCHILD, DBEntryGetLastChild);
	gServices[ 6] = CreateServiceFunction(MS_DB_ENTRY_GETNEXTSILBING, DBEntryGetNextSilbing);
	gServices[ 7] = CreateServiceFunction(MS_DB_ENTRY_GETPREVSILBING, DBEntryGetPrevSilbing);
	gServices[ 8] = CreateServiceFunction(MS_DB_ENTRY_GETFLAGS, DBEntryGetFlags);
	gServices[ 9] = CreateServiceFunction(MS_DB_ENTRY_ITER_INIT, DBEntryIterInit);
	gServices[10] = CreateServiceFunction(MS_DB_ENTRY_ITER_NEXT, DBEntryIterNext);
	gServices[11] = CreateServiceFunction(MS_DB_ENTRY_ITER_CLOSE, DBEntryIterClose);
	gServices[12] = CreateServiceFunction(MS_DB_ENTRY_DELETE, DBEntryDelete);
	gServices[13] = CreateServiceFunction(MS_DB_ENTRY_CREATE, DBEntryCreate);

	gServices[14] = CreateServiceFunction(MS_DB_VIRTUALENTRY_CREATE, DBVirtualEntryCreate);
	gServices[15] = CreateServiceFunction(MS_DB_VIRTUALENTRY_GETPARENT, DBVirtualEntryGetParent);
	gServices[16] = CreateServiceFunction(MS_DB_VIRTUALENTRY_GETFIRST, DBVirtualEntryGetFirst);
	gServices[17] = CreateServiceFunction(MS_DB_VIRTUALENTRY_GETNEXT, DBVirtualEntryGetNext);


	return true;
}