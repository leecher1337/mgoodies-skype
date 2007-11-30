#include "Services.h"

HANDLE gServices[28] = {0};

int DBEntryGetRoot(WPARAM wParam, LPARAM lParam)
{
	return gDataBase->getEntries().getRootEntry();
}
int DBEntryChildCount(WPARAM hEntry, LPARAM lParam)
{
	if (hEntry == 0)
		hEntry = gDataBase->getEntries().getRootEntry();

	return gDataBase->getEntries().getChildCount(hEntry);
}
int DBEntryGetParent(WPARAM hEntry, LPARAM lParam)
{
	if (hEntry == 0)
		hEntry = gDataBase->getEntries().getRootEntry();

	return gDataBase->getEntries().getParent(hEntry);
}
int DBEntrySetParent(WPARAM hEntry, LPARAM hParent)
{
	if ((hEntry == 0) || (hEntry == gDataBase->getEntries().getRootEntry()))
		return DB_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getEntries().getRootEntry();

	return gDataBase->getEntries().setParent(hEntry, hParent);
}
int DBEntryGetFirstChild(WPARAM hParent, LPARAM lParam)
{
	if (hParent == 0)
		hParent = gDataBase->getEntries().getRootEntry();

	return gDataBase->getEntries().getFirstChild(hParent);
}
int DBEntryGetLastChild(WPARAM hParent, LPARAM lParam)
{
	if (hParent == 0)
		hParent = gDataBase->getEntries().getRootEntry();

	return gDataBase->getEntries().getLastChild(hParent);
}
int DBEntryGetNextSilbing(WPARAM hEntry, LPARAM lParam)
{
	if (hEntry == 0)
		hEntry = gDataBase->getEntries().getRootEntry();

	return gDataBase->getEntries().getNextSilbing(hEntry);
}
int DBEntryGetPrevSilbing(WPARAM hEntry, LPARAM lParam)
{
	if (hEntry == 0)
		hEntry = gDataBase->getEntries().getRootEntry();

	return gDataBase->getEntries().getPrevSilbing(hEntry);
}
int DBEntryGetFlags(WPARAM hEntry, LPARAM lParam)
{
	if (hEntry == 0)
		hEntry = gDataBase->getEntries().getRootEntry();

	return gDataBase->getEntries().getFlags(hEntry);
}
int DBEntryIterInit(WPARAM pFilter, LPARAM lParam)
{
	TDBEntryIterFilter fil = {0};
	if (pFilter == NULL)
	{
		pFilter = (WPARAM)&fil;
		fil.cbSize = sizeof(fil);
	}

	if (((PDBEntryIterFilter)pFilter)->cbSize != sizeof(TDBEntryIterFilter))
		return DB_INVALIDPARAM;

	if (((PDBEntryIterFilter)pFilter)->fDontHasFlags & ((PDBEntryIterFilter)pFilter)->fHasFlags)
		return DB_INVALIDPARAM;

	if (((PDBEntryIterFilter)pFilter)->hParentEntry == 0)
		((PDBEntryIterFilter)pFilter)->hParentEntry = gDataBase->getEntries().getRootEntry();

	return gDataBase->getEntries().IterationInit(*(PDBEntryIterFilter)pFilter);
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
	if ((hEntry == 0) || (hEntry == gDataBase->getEntries().getRootEntry()))
		return DB_INVALIDPARAM;

	return gDataBase->getEntries().DeleteEntry(hEntry);
}
int DBEntryCreate(WPARAM hParent, LPARAM Flags)
{
	if (hParent == 0)
		hParent = gDataBase->getEntries().getRootEntry();

	Flags = Flags & ~(DB_EF_HasChildren | DB_EF_IsVirtual | DB_EF_HasVirtuals); // forbidden flags...
	return gDataBase->getEntries().CreateEntry(hParent, Flags);
}

int DBVirtualEntryCreate(WPARAM hEntry, LPARAM hParent)
{
	if ((hEntry == 0) || (hEntry == gDataBase->getEntries().getRootEntry()))
		return DB_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getEntries().getRootEntry();

	return gDataBase->getEntries().VirtualCreate(hEntry, hParent);
}
int DBVirtualEntryGetParent(WPARAM hVirtuaEntry, LPARAM lParam)
{
	if ((hVirtuaEntry == 0) || (hVirtuaEntry == gDataBase->getEntries().getRootEntry()))
		return DB_INVALIDPARAM;

	return gDataBase->getEntries().VirtualGetParent(hVirtuaEntry);
}
int DBVirtualEntryGetFirst(WPARAM hEntry, LPARAM lParam)
{
	if ((hEntry == 0) || (hEntry == gDataBase->getEntries().getRootEntry()))
		return DB_INVALIDPARAM;
	
	return gDataBase->getEntries().VirtualGetFirst(hEntry);
}
int DBVirtualEntryGetNext(WPARAM hVirtualEntry, LPARAM lParam)
{
	if ((hVirtualEntry == 0) || (hVirtualEntry == gDataBase->getEntries().getRootEntry()))
		return DB_INVALIDPARAM;

	return gDataBase->getEntries().VirtualGetNext(hVirtualEntry);
}


int DBSettingFind(WPARAM pSettingDescriptor, LPARAM lParam)
{
	if (pSettingDescriptor == NULL)
		return DB_INVALIDPARAM;

	if (((PDBSettingDescriptor)pSettingDescriptor)->cbSize != sizeof(TDBSettingDescriptor))
		return DB_INVALIDPARAM;

	if (((PDBSettingDescriptor)pSettingDescriptor)->pszSettingName == NULL)
		return DB_INVALIDPARAM;

	return gDataBase->getSettings().FindSetting(*((PDBSettingDescriptor)pSettingDescriptor));
}
int DBSettingDelete(WPARAM pSettingDescriptor, LPARAM lParam)
{
	if (pSettingDescriptor == NULL)
		return DB_INVALIDPARAM;

	if (((PDBSettingDescriptor)pSettingDescriptor)->cbSize != sizeof(TDBSettingDescriptor))
		return DB_INVALIDPARAM;

	if (((PDBSettingDescriptor)pSettingDescriptor)->pszSettingName == NULL)
		return DB_INVALIDPARAM;

	return gDataBase->getSettings().DeleteSetting(*((PDBSettingDescriptor)pSettingDescriptor));
}
int DBSettingDeleteHandle(WPARAM hSetting, LPARAM lParam)
{
	if (hSetting == 0)
		return DB_INVALIDPARAM;

	return gDataBase->getSettings().DeleteSetting(hSetting);
}
int DBSettingWrite(WPARAM pSetting, LPARAM lParam)
{
	if (pSetting == NULL)		
		return DB_INVALIDPARAM;

	if (((PDBSetting)pSetting)->cbSize != sizeof(TDBSetting))
		return DB_INVALIDPARAM;

	if (((PDBSetting)pSetting)->Descriptor == NULL)
		return DB_INVALIDPARAM;

	if (((PDBSetting)pSetting)->Descriptor->cbSize != sizeof(TDBSettingDescriptor))
		return DB_INVALIDPARAM;

	if (((PDBSetting)pSetting)->Descriptor->pszSettingName == NULL)
		return DB_INVALIDPARAM;

	return gDataBase->getSettings().WriteSetting(*((PDBSetting)pSetting));
}
int DBSettingWriteHandle(WPARAM pSetting, LPARAM hSetting)
{
	if (pSetting == NULL)		
		return DB_INVALIDPARAM;

	if (((PDBSetting)pSetting)->cbSize != sizeof(TDBSetting))
		return DB_INVALIDPARAM;

	return gDataBase->getSettings().WriteSetting(*((PDBSetting)pSetting), hSetting);
}
int DBSettingRead(WPARAM pSetting, LPARAM lParam)
{
	if (pSetting == NULL)		
		return DB_INVALIDPARAM;

	if (((PDBSetting)pSetting)->cbSize != sizeof(TDBSetting))
		return DB_INVALIDPARAM;

	if (((PDBSetting)pSetting)->Descriptor == NULL)
		return DB_INVALIDPARAM;

	if (((PDBSetting)pSetting)->Descriptor->cbSize != sizeof(TDBSettingDescriptor))
		return DB_INVALIDPARAM;

	if (((PDBSetting)pSetting)->Descriptor->pszSettingName == NULL)
		return DB_INVALIDPARAM;

	return gDataBase->getSettings().ReadSetting(*((PDBSetting)pSetting));
}
int DBSettingReadHandle(WPARAM pSetting, LPARAM hSetting)
{
	if ((pSetting == NULL) || (hSetting == 0))
		return DB_INVALIDPARAM;

	if (((PDBSetting)pSetting)->cbSize != sizeof(TDBSetting))
		return DB_INVALIDPARAM;

	if ((((PDBSetting)pSetting)->Descriptor != NULL) && (((PDBSetting)pSetting)->Descriptor->cbSize != sizeof(TDBSettingDescriptor)))
		return DB_INVALIDPARAM;
	
	return gDataBase->getSettings().ReadSetting(*((PDBSetting)pSetting), hSetting);
}
int DBSettingIterInit(WPARAM Filter, LPARAM lParam)
{
	return 0;
}
int DBSettingIterNext(WPARAM hIteration, LPARAM lParam)
{
	return 0;
}
int DBSettingIterClose(WPARAM hIteration, LPARAM lParam)
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

	gServices[18] = CreateServiceFunction(MS_DB_SETTING_FIND, DBSettingFind);
	gServices[19] = CreateServiceFunction(MS_DB_SETTING_DELETE, DBSettingDelete);
	gServices[20] = CreateServiceFunction(MS_DB_SETTING_DELETEHANDLE, DBSettingDeleteHandle);
	gServices[21] = CreateServiceFunction(MS_DB_SETTING_WRITE, DBSettingWrite);
	gServices[22] = CreateServiceFunction(MS_DB_SETTING_WRITEHANDLE, DBSettingWriteHandle);
	gServices[23] = CreateServiceFunction(MS_DB_SETTING_READ, DBSettingRead);
	gServices[24] = CreateServiceFunction(MS_DB_SETTING_READHANDLE, DBSettingReadHandle);
	gServices[25] = CreateServiceFunction(MS_DB_SETTING_ITER_INIT, DBSettingIterInit);
	gServices[26] = CreateServiceFunction(MS_DB_SETTING_ITER_NEXT, DBSettingIterNext);
	gServices[27] = CreateServiceFunction(MS_DB_SETTING_ITER_CLOSE, DBSettingIterClose);


	return true;
}