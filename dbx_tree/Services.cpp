#include "Services.h"

HANDLE gServices[41] = {0};

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
int DBSettingIterInit(WPARAM pFilter, LPARAM lParam)
{
	if (pFilter == NULL)
		return DB_INVALIDPARAM;

	if (((PDBSettingIterFilter)pFilter)->cbSize != sizeof(TDBSettingIterFilter))
		return DB_INVALIDPARAM;

	if ((((PDBSettingIterFilter)pFilter)->Descriptor != NULL) && (((PDBSettingIterFilter)pFilter)->Descriptor->cbSize != sizeof(TDBSettingDescriptor)))
		return DB_INVALIDPARAM;

	if ((((PDBSettingIterFilter)pFilter)->Setting != NULL) && (((PDBSettingIterFilter)pFilter)->Setting->cbSize != sizeof(TDBSetting)))
		return DB_INVALIDPARAM;

	if ((((PDBSettingIterFilter)pFilter)->Setting != NULL) && (((PDBSettingIterFilter)pFilter)->Setting->Descriptor != NULL) && (((PDBSettingIterFilter)pFilter)->Setting->Descriptor->cbSize != sizeof(TDBSettingIterFilter)))
		return DB_INVALIDPARAM;

	return gDataBase->getSettings().IterationInit(*((PDBSettingIterFilter)pFilter));
}
int DBSettingIterNext(WPARAM hIteration, LPARAM lParam)
{
	return gDataBase->getSettings().IterationNext(hIteration);
}
int DBSettingIterClose(WPARAM hIteration, LPARAM lParam)
{
	return gDataBase->getSettings().IterationClose(hIteration);
}


int DBEventTypeRegister(WPARAM pType, LPARAM lParam)
{
	if ((pType == NULL) || (((PDBEventTypeDescriptor)pType)->cbSize != sizeof(TDBEventTypeDescriptor)))
		return DB_INVALIDPARAM;

	return gDataBase->getEvents().TypeRegister(*((PDBEventTypeDescriptor)pType));
}

int DBEventTypeGet(WPARAM pModuleName, LPARAM EventType)
{
	if (pModuleName == NULL)
		return DB_INVALIDPARAM;

	return (int) gDataBase->getEvents().TypeGet((char*)pModuleName, EventType);
}

int DBEventGetBlobSize(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().GetBlobSize(hEvent);
}

int DBEventGet(WPARAM hEvent, LPARAM pEvent)
{
	if ((pEvent == NULL) || (((PDBEvent)pEvent)->cbSize != sizeof(TDBEvent)))
		return DB_INVALIDPARAM;

	return gDataBase->getEvents().Get(hEvent, *((PDBEvent)pEvent));
}

int DBEventDelete(WPARAM hContact, LPARAM hEvent)
{
	return gDataBase->getEvents().Delete(hContact, hEvent);
}

int DBEventAdd(WPARAM hContact, LPARAM pEvent)
{
	if ((pEvent == NULL) || (((PDBEvent)pEvent)->cbSize != sizeof(TDBEvent)) || (((PDBEvent)pEvent)->pBlob == NULL) || (((PDBEvent)pEvent)->cbBlob == 0))
		return DB_INVALIDPARAM;
	
	return gDataBase->getEvents().Add(hContact, *((PDBEvent)pEvent));
}

int DBEventMarkRead(WPARAM hContact, LPARAM hEvent)
{
	return gDataBase->getEvents().MarkRead(hContact, hEvent);
}

int DBEventWriteToDisk(WPARAM hContact, LPARAM hEvent)
{
	return gDataBase->getEvents().WriteToDisk(hContact, hEvent);
}

int DBEventHardLink(WPARAM pHardLink, LPARAM lParam)
{
	if ((pHardLink == NULL) || (((PDBEventHardLink)pHardLink)->cbSize != sizeof(TDBEventHardLink)))
		return DB_INVALIDPARAM;

	return gDataBase->getEvents().HardLink(*((PDBEventHardLink)pHardLink));
}

int DBEventGetContact(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().GetContact(hEvent);
}

int DBEventIterInit(WPARAM pFilter, LPARAM lParam)
{
	if ((pFilter == NULL) || (((PDBEventIterFilter)pFilter)->cbSize != sizeof(TDBEventIterFilter)))
		return DB_INVALIDPARAM;

	if ((((PDBEventIterFilter)pFilter)->Event != NULL) && (((PDBEventIterFilter)pFilter)->Event->cbSize != sizeof(TDBEvent)))
		return DB_INVALIDPARAM;

	return gDataBase->getEvents().IterationInit(*((PDBEventIterFilter)pFilter));
}

int DBEventIterNext(WPARAM hIteration, LPARAM lParam)
{
	return gDataBase->getEvents().IterationNext(hIteration);
}

int DBEventIterClose(WPARAM hIteration, LPARAM lParam)
{
	return gDataBase->getEvents().IterationClose(hIteration);
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

	gServices[28] = CreateServiceFunction(MS_DB_EVENT_REGISTERTYPE, DBEventTypeRegister);
	gServices[29] = CreateServiceFunction(MS_DB_EVENT_GETTYPE, DBEventTypeGet);
	gServices[30] = CreateServiceFunction(MS_DB_EVENT_GETBLOBSIZE, DBEventGetBlobSize);
	gServices[31] = CreateServiceFunction(MS_DB_EVENT_GET, DBEventGet);
	gServices[32] = CreateServiceFunction(MS_DB_EVENT_DELETE, DBEventDelete);
	gServices[33] = CreateServiceFunction(MS_DB_EVENT_ADD, DBEventAdd);
	gServices[34] = CreateServiceFunction(MS_DB_EVENT_MARKREAD, DBEventMarkRead);
	gServices[35] = CreateServiceFunction(MS_DB_EVENT_WRITETODISK, DBEventWriteToDisk);
	gServices[36] = CreateServiceFunction(MS_DB_EVENT_GETCONTACT, DBEventGetContact);
	gServices[37] = CreateServiceFunction(MS_DB_EVENT_HARDLINK, DBEventHardLink);
	gServices[38] = CreateServiceFunction(MS_DB_EVENT_ITER_INIT, DBEventIterInit);
	gServices[39] = CreateServiceFunction(MS_DB_EVENT_ITER_NEXT, DBEventIterNext);
	gServices[40] = CreateServiceFunction(MS_DB_EVENT_ITER_CLOSE, DBEventIterClose);


	return true;
}