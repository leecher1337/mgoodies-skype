#include "Services.h"

HANDLE gServices[28] = {0};

int DBContactGetRoot(WPARAM wParam, LPARAM lParam)
{
	return gDataBase->getContacts().getRootContact();
}
int DBContactChildCount(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().getChildCount(hContact);
}
int DBContactGetParent(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().getParent(hContact);
}
int DBContactSetParent(WPARAM hContact, LPARAM hParent)
{
	if ((hContact == 0) || (hContact == gDataBase->getContacts().getRootContact()))
		return DB_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().setParent(hContact, hParent);
}
int DBContactGetFirstChild(WPARAM hParent, LPARAM lParam)
{
	if (hParent == 0)
		hParent = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().getFirstChild(hParent);
}
int DBContactGetLastChild(WPARAM hParent, LPARAM lParam)
{
	if (hParent == 0)
		hParent = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().getLastChild(hParent);
}
int DBContactGetNextSilbing(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().getNextSilbing(hContact);
}
int DBContactGetPrevSilbing(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().getPrevSilbing(hContact);
}
int DBContactGetFlags(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().getFlags(hContact);
}
int DBContactIterInit(WPARAM pFilter, LPARAM hParent)
{
	TDBContactIterFilter fil = {0};
	if (pFilter == NULL)
	{
		pFilter = (WPARAM)&fil;
		fil.cbSize = sizeof(fil);
	}

	if (((PDBContactIterFilter)pFilter)->cbSize != sizeof(TDBContactIterFilter))
		return DB_INVALIDPARAM;

	if (((PDBContactIterFilter)pFilter)->fDontHasFlags & ((PDBContactIterFilter)pFilter)->fHasFlags)
		return DB_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().IterationInit(*(PDBContactIterFilter)pFilter, hParent);
}
int DBContactIterNext(WPARAM hIteration, LPARAM lParam)
{
	return gDataBase->getContacts().IterationNext(hIteration);
}
int DBContactIterClose(WPARAM hIteration, LPARAM lParam)
{
	return gDataBase->getContacts().IterationClose(hIteration);
}
int DBContactDelete(WPARAM hContact, LPARAM lParam)
{
	if ((hContact == 0) || (hContact == gDataBase->getContacts().getRootContact()))
		return DB_INVALIDPARAM;

	return gDataBase->getContacts().DeleteContact(hContact);
}
int DBContactCreate(WPARAM hParent, LPARAM Flags)
{
	if (hParent == 0)
		hParent = gDataBase->getContacts().getRootContact();

	Flags = Flags & ~(DB_CF_IsRoot | DB_CF_HasChildren | DB_CF_IsVirtual | DB_CF_HasVirtuals); // forbidden flags...
	return gDataBase->getContacts().CreateContact(hParent, Flags);
}

int DBVirtualContactCreate(WPARAM hContact, LPARAM hParent)
{
	if ((hContact == 0) || (hContact == gDataBase->getContacts().getRootContact()))
		return DB_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().VirtualCreate(hContact, hParent);
}
int DBVirtualContactGetParent(WPARAM hVirtuaContact, LPARAM lParam)
{
	if ((hVirtuaContact == 0) || (hVirtuaContact == gDataBase->getContacts().getRootContact()))
		return DB_INVALIDPARAM;

	return gDataBase->getContacts().VirtualGetParent(hVirtuaContact);
}
int DBVirtualContactGetFirst(WPARAM hContact, LPARAM lParam)
{
	if ((hContact == 0) || (hContact == gDataBase->getContacts().getRootContact()))
		return DB_INVALIDPARAM;
	
	return gDataBase->getContacts().VirtualGetFirst(hContact);
}
int DBVirtualContactGetNext(WPARAM hVirtualContact, LPARAM lParam)
{
	if ((hVirtualContact == 0) || (hVirtualContact == gDataBase->getContacts().getRootContact()))
		return DB_INVALIDPARAM;

	return gDataBase->getContacts().VirtualGetNext(hVirtualContact);
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
	gServices[ 0] = CreateServiceFunction(MS_DB_CONTACT_GETROOT, DBContactGetRoot);
	gServices[ 1] = CreateServiceFunction(MS_DB_CONTACT_CHILDCOUNT, DBContactChildCount);
	gServices[ 2] = CreateServiceFunction(MS_DB_CONTACT_GETPARENT, DBContactGetParent);
	gServices[ 3] = CreateServiceFunction(MS_DB_CONTACT_SETPARENT, DBContactSetParent);
	gServices[ 4] = CreateServiceFunction(MS_DB_CONTACT_GETFIRSTCHILD, DBContactGetFirstChild);
	gServices[ 5] = CreateServiceFunction(MS_DB_CONTACT_GETLASTCHILD, DBContactGetLastChild);
	gServices[ 6] = CreateServiceFunction(MS_DB_CONTACT_GETNEXTSILBING, DBContactGetNextSilbing);
	gServices[ 7] = CreateServiceFunction(MS_DB_CONTACT_GETPREVSILBING, DBContactGetPrevSilbing);
	gServices[ 8] = CreateServiceFunction(MS_DB_CONTACT_GETFLAGS, DBContactGetFlags);
	gServices[ 9] = CreateServiceFunction(MS_DB_CONTACT_ITER_INIT, DBContactIterInit);
	gServices[10] = CreateServiceFunction(MS_DB_CONTACT_ITER_NEXT, DBContactIterNext);
	gServices[11] = CreateServiceFunction(MS_DB_CONTACT_ITER_CLOSE, DBContactIterClose);
	gServices[12] = CreateServiceFunction(MS_DB_CONTACT_DELETE, DBContactDelete);
	gServices[13] = CreateServiceFunction(MS_DB_CONTACT_CREATE, DBContactCreate);

	gServices[14] = CreateServiceFunction(MS_DB_VIRTUALCONTACT_CREATE, DBVirtualContactCreate);
	gServices[15] = CreateServiceFunction(MS_DB_VIRTUALCONTACT_GETPARENT, DBVirtualContactGetParent);
	gServices[16] = CreateServiceFunction(MS_DB_VIRTUALCONTACT_GETFIRST, DBVirtualContactGetFirst);
	gServices[17] = CreateServiceFunction(MS_DB_VIRTUALCONTACT_GETNEXT, DBVirtualContactGetNext);

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