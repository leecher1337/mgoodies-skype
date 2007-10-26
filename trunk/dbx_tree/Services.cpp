#include "Services.h"

HANDLE gServices[18] = {0};

int DBEntryGetRoot(WPARAM wParam, LPARAM lParam)
{
	return gDataBase->getRootEntry();
}
int DBEntryChildCount(WPARAM hEntry, LPARAM lParam)
{
	return gDataBase->getEntries().getChildCount(hEntry);
}
int DBEntryGetParent(WPARAM hEntry, LPARAM lParam)
{
	return gDataBase->getEntries().getParent(hEntry);
}
int DBEntrySetParent(WPARAM hEntry, LPARAM hParent)
{
	return 0;
}
int DBEntryGetFirstChild(WPARAM hParent, LPARAM lParam)
{
	return gDataBase->getEntries().getFirstChild(hParent);
}
int DBEntryGetLastChild(WPARAM hParent, LPARAM lParam)
{
	return gDataBase->getEntries().getLastChild(hParent);
}
int DBEntryGetNextSilbing(WPARAM hEntry, LPARAM lParam)
{
	return gDataBase->getEntries().getNextSilbing(hEntry);
}
int DBEntryGetPrevSilbing(WPARAM hEntry, LPARAM lParam)
{
	return gDataBase->getEntries().getPrevSilbing(hEntry);
}
int DBEntryIterInitBF(WPARAM Filter, LPARAM lParam)
{
	return 0;
}
int DBEntryIterInitDF(WPARAM Filter, LPARAM lParam)
{
	return 0;
}
int DBEntryIterNext(WPARAM hIteration, LPARAM lParam)
{
	return 0;
}
int DBEntryIterClose(WPARAM wParam, LPARAM lParam)
{
	return 0;
}
int DBEntryDelete(WPARAM hEntry, LPARAM lParam)
{
	return gDataBase->getEntries().DeleteEntry(hEntry);
}
int DBEntryCreate(WPARAM hParent, LPARAM Flags)
{
	return gDataBase->getEntries().CreateEntry(hParent, Flags);
}

int DBVirtualEntryCreate(WPARAM hEntry, LPARAM hParent)
{
	return 0;
}
int DBVirtualEntryGetParent(WPARAM hVirtuaEntry, LPARAM lParam)
{
	return 0;
}
int DBVirtualEntryGetFirst(WPARAM hEntry, LPARAM lParam)
{
	return 0;
}
int DBVirtualEntryGetNext(WPARAM hVirtuaEntry, LPARAM lParam)
{
	return 0;
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
	gServices[ 8] = CreateServiceFunction(MS_DB_ENTRY_ITER_INITBF, DBEntryIterInitBF);
	gServices[ 9] = CreateServiceFunction(MS_DB_ENTRY_ITER_INITDF, DBEntryIterInitDF);
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