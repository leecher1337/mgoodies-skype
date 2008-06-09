#include "Services.h"

HANDLE gServices[40] = {0};

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
		return DBT_INVALIDPARAM;

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
	TDBTContactIterFilter fil = {0};
	if (pFilter == NULL)
	{
		pFilter = (WPARAM)&fil;
		fil.cbSize = sizeof(fil);
	}

	if (((PDBTContactIterFilter)pFilter)->cbSize != sizeof(TDBTContactIterFilter))
		return DBT_INVALIDPARAM;

	if (((PDBTContactIterFilter)pFilter)->fDontHasFlags & ((PDBTContactIterFilter)pFilter)->fHasFlags)
		return DBT_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().IterationInit(*(PDBTContactIterFilter)pFilter, hParent);
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
		return DBT_INVALIDPARAM;

	return gDataBase->getContacts().DeleteContact(hContact);
}
int DBContactCreate(WPARAM hParent, LPARAM Flags)
{
	if (hParent == 0)
		hParent = gDataBase->getContacts().getRootContact();

	Flags = Flags & ~(DBT_CF_IsRoot | DBT_CF_HasChildren | DBT_CF_IsVirtual | DBT_CF_HasVirtuals); // forbidden flags...
	return gDataBase->getContacts().CreateContact(hParent, Flags);
}

int DBVirtualContactCreate(WPARAM hContact, LPARAM hParent)
{
	if ((hContact == 0) || (hContact == gDataBase->getContacts().getRootContact()))
		return DBT_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getContacts().getRootContact();

	return gDataBase->getContacts().VirtualCreate(hContact, hParent);
}
int DBVirtualContactGetParent(WPARAM hVirtuaContact, LPARAM lParam)
{
	if ((hVirtuaContact == 0) || (hVirtuaContact == gDataBase->getContacts().getRootContact()))
		return DBT_INVALIDPARAM;

	return gDataBase->getContacts().VirtualGetParent(hVirtuaContact);
}
int DBVirtualContactGetFirst(WPARAM hContact, LPARAM lParam)
{
	if ((hContact == 0) || (hContact == gDataBase->getContacts().getRootContact()))
		return DBT_INVALIDPARAM;
	
	return gDataBase->getContacts().VirtualGetFirst(hContact);
}
int DBVirtualContactGetNext(WPARAM hVirtualContact, LPARAM lParam)
{
	if ((hVirtualContact == 0) || (hVirtualContact == gDataBase->getContacts().getRootContact()))
		return DBT_INVALIDPARAM;

	return gDataBase->getContacts().VirtualGetNext(hVirtualContact);
}


int DBSettingFind(WPARAM pSettingDescriptor, LPARAM lParam)
{
	if (pSettingDescriptor == NULL)
		return DBT_INVALIDPARAM;

	if (((PDBTSettingDescriptor)pSettingDescriptor)->cbSize != sizeof(TDBTSettingDescriptor))
		return DBT_INVALIDPARAM;

	if (((PDBTSettingDescriptor)pSettingDescriptor)->pszSettingName == NULL)
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().FindSetting(*((PDBTSettingDescriptor)pSettingDescriptor));
}
int DBSettingDelete(WPARAM pSettingDescriptor, LPARAM lParam)
{
	if (pSettingDescriptor == NULL)
		return DBT_INVALIDPARAM;

	if (((PDBTSettingDescriptor)pSettingDescriptor)->cbSize != sizeof(TDBTSettingDescriptor))
		return DBT_INVALIDPARAM;

	if (((PDBTSettingDescriptor)pSettingDescriptor)->pszSettingName == NULL)
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().DeleteSetting(*((PDBTSettingDescriptor)pSettingDescriptor));
}
int DBSettingDeleteHandle(WPARAM hSetting, LPARAM lParam)
{
	if (hSetting == 0)
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().DeleteSetting(hSetting);
}
int DBSettingWrite(WPARAM pSetting, LPARAM lParam)
{
	if (pSetting == NULL)		
		return DBT_INVALIDPARAM;

	if (((PDBTSetting)pSetting)->cbSize != sizeof(TDBTSetting))
		return DBT_INVALIDPARAM;

	if (((PDBTSetting)pSetting)->Descriptor == NULL)
		return DBT_INVALIDPARAM;

	if (((PDBTSetting)pSetting)->Descriptor->cbSize != sizeof(TDBTSettingDescriptor))
		return DBT_INVALIDPARAM;

	if (((PDBTSetting)pSetting)->Descriptor->pszSettingName == NULL)
		return DBT_INVALIDPARAM;

	if ((((PDBTSetting)pSetting)->Type & DBT_STF_VariableLength) && (((PDBTSetting)pSetting)->Value.pBlob == NULL))
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().WriteSetting(*((PDBTSetting)pSetting));
}
int DBSettingWriteHandle(WPARAM pSetting, LPARAM hSetting)
{
	if (pSetting == NULL)		
		return DBT_INVALIDPARAM;

	if (((PDBTSetting)pSetting)->cbSize != sizeof(TDBTSetting))
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().WriteSetting(*((PDBTSetting)pSetting), hSetting);
}
int DBSettingRead(WPARAM pSetting, LPARAM lParam)
{
	if (pSetting == NULL)		
		return DBT_INVALIDPARAM;

	if (((PDBTSetting)pSetting)->cbSize != sizeof(TDBTSetting))
		return DBT_INVALIDPARAM;

	if (((PDBTSetting)pSetting)->Descriptor == NULL)
		return DBT_INVALIDPARAM;

	if (((PDBTSetting)pSetting)->Descriptor->cbSize != sizeof(TDBTSettingDescriptor))
		return DBT_INVALIDPARAM;

	if (((PDBTSetting)pSetting)->Descriptor->pszSettingName == NULL)
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().ReadSetting(*((PDBTSetting)pSetting));
}
int DBSettingReadHandle(WPARAM pSetting, LPARAM hSetting)
{
	if ((pSetting == NULL) || (hSetting == 0))
		return DBT_INVALIDPARAM;

	if (((PDBTSetting)pSetting)->cbSize != sizeof(TDBTSetting))
		return DBT_INVALIDPARAM;

	if ((((PDBTSetting)pSetting)->Descriptor != NULL) && (((PDBTSetting)pSetting)->Descriptor->cbSize != sizeof(TDBTSettingDescriptor)))
		return DBT_INVALIDPARAM;
	
	return gDataBase->getSettings().ReadSetting(*((PDBTSetting)pSetting), hSetting);
}
int DBSettingIterInit(WPARAM pFilter, LPARAM lParam)
{
	if (pFilter == NULL)
		return DBT_INVALIDPARAM;

	if (((PDBTSettingIterFilter)pFilter)->cbSize != sizeof(TDBTSettingIterFilter))
		return DBT_INVALIDPARAM;

	if ((((PDBTSettingIterFilter)pFilter)->Descriptor != NULL) && (((PDBTSettingIterFilter)pFilter)->Descriptor->cbSize != sizeof(TDBTSettingDescriptor)))
		return DBT_INVALIDPARAM;

	if ((((PDBTSettingIterFilter)pFilter)->Setting != NULL) && (((PDBTSettingIterFilter)pFilter)->Setting->cbSize != sizeof(TDBTSetting)))
		return DBT_INVALIDPARAM;

	if ((((PDBTSettingIterFilter)pFilter)->Setting != NULL) && (((PDBTSettingIterFilter)pFilter)->Setting->Descriptor != NULL) && (((PDBTSettingIterFilter)pFilter)->Setting->Descriptor->cbSize != sizeof(TDBTSettingIterFilter)))
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().IterationInit(*((PDBTSettingIterFilter)pFilter));
}
int DBSettingIterNext(WPARAM hIteration, LPARAM lParam)
{
	return gDataBase->getSettings().IterationNext(hIteration);
}
int DBSettingIterClose(WPARAM hIteration, LPARAM lParam)
{
	return gDataBase->getSettings().IterationClose(hIteration);
}

int DBEventGetBlobSize(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().GetBlobSize(hEvent);
}

int DBEventGet(WPARAM hEvent, LPARAM pEvent)
{
	if ((pEvent == NULL) || (((PDBTEvent)pEvent)->cbSize != sizeof(TDBTEvent)))
		return DBT_INVALIDPARAM;

	return gDataBase->getEvents().Get(hEvent, *((PDBTEvent)pEvent));
}

int DBEventGetCount(WPARAM hContact, LPARAM lParam)
{
	return gDataBase->getEvents().GetCount(hContact);
}

int DBEventDelete(WPARAM hContact, LPARAM hEvent)
{
	return gDataBase->getEvents().Delete(hContact, hEvent);
}

int DBEventAdd(WPARAM hContact, LPARAM pEvent)
{
	if ((pEvent == NULL) || (((PDBTEvent)pEvent)->cbSize != sizeof(TDBTEvent)) || (((PDBTEvent)pEvent)->pBlob == NULL) || (((PDBTEvent)pEvent)->cbBlob == 0))
		return DBT_INVALIDPARAM;
	
	return gDataBase->getEvents().Add(hContact, *((PDBTEvent)pEvent));
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
	if ((pHardLink == NULL) || (((PDBTEventHardLink)pHardLink)->cbSize != sizeof(TDBTEventHardLink)))
		return DBT_INVALIDPARAM;

	return gDataBase->getEvents().HardLink(*((PDBTEventHardLink)pHardLink));
}

int DBEventGetContact(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().GetContact(hEvent);
}

int DBEventIterInit(WPARAM pFilter, LPARAM lParam)
{
	if ((pFilter == NULL) || (((PDBTEventIterFilter)pFilter)->cbSize != sizeof(TDBTEventIterFilter)))
		return DBT_INVALIDPARAM;

	if ((((PDBTEventIterFilter)pFilter)->Event != NULL) && (((PDBTEventIterFilter)pFilter)->Event->cbSize != sizeof(TDBTEvent)))
		return DBT_INVALIDPARAM;

	return gDataBase->getEvents().IterationInit(*((PDBTEventIterFilter)pFilter));
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
	gServices[ 0] = CreateServiceFunction(MS_DBT_CONTACT_GETROOT,          DBContactGetRoot);
	gServices[ 1] = CreateServiceFunction(MS_DBT_CONTACT_CHILDCOUNT,       DBContactChildCount);
	gServices[ 2] = CreateServiceFunction(MS_DBT_CONTACT_GETPARENT,        DBContactGetParent);
	gServices[ 3] = CreateServiceFunction(MS_DBT_CONTACT_SETPARENT,        DBContactSetParent);
	gServices[ 4] = CreateServiceFunction(MS_DBT_CONTACT_GETFIRSTCHILD,    DBContactGetFirstChild);
	gServices[ 5] = CreateServiceFunction(MS_DBT_CONTACT_GETLASTCHILD,     DBContactGetLastChild);
	gServices[ 6] = CreateServiceFunction(MS_DBT_CONTACT_GETNEXTSILBING,   DBContactGetNextSilbing);
	gServices[ 7] = CreateServiceFunction(MS_DBT_CONTACT_GETPREVSILBING,   DBContactGetPrevSilbing);
	gServices[ 8] = CreateServiceFunction(MS_DBT_CONTACT_GETFLAGS,         DBContactGetFlags);
	gServices[ 9] = CreateServiceFunction(MS_DBT_CONTACT_ITER_INIT,        DBContactIterInit);
	gServices[10] = CreateServiceFunction(MS_DBT_CONTACT_ITER_NEXT,        DBContactIterNext);
	gServices[11] = CreateServiceFunction(MS_DBT_CONTACT_ITER_CLOSE,       DBContactIterClose);
	gServices[12] = CreateServiceFunction(MS_DBT_CONTACT_DELETE,           DBContactDelete);
	gServices[13] = CreateServiceFunction(MS_DBT_CONTACT_CREATE,           DBContactCreate);

	gServices[14] = CreateServiceFunction(MS_DBT_VIRTUALCONTACT_CREATE,    DBVirtualContactCreate);
	gServices[15] = CreateServiceFunction(MS_DBT_VIRTUALCONTACT_GETPARENT, DBVirtualContactGetParent);
	gServices[16] = CreateServiceFunction(MS_DBT_VIRTUALCONTACT_GETFIRST,  DBVirtualContactGetFirst);
	gServices[17] = CreateServiceFunction(MS_DBT_VIRTUALCONTACT_GETNEXT,   DBVirtualContactGetNext);

	gServices[18] = CreateServiceFunction(MS_DBT_SETTING_FIND,             DBSettingFind);
	gServices[19] = CreateServiceFunction(MS_DBT_SETTING_DELETE,           DBSettingDelete);
	gServices[20] = CreateServiceFunction(MS_DBT_SETTING_DELETEHANDLE,     DBSettingDeleteHandle);
	gServices[21] = CreateServiceFunction(MS_DBT_SETTING_WRITE,            DBSettingWrite);
	gServices[22] = CreateServiceFunction(MS_DBT_SETTING_WRITEHANDLE,      DBSettingWriteHandle);
	gServices[23] = CreateServiceFunction(MS_DBT_SETTING_READ,             DBSettingRead);
	gServices[24] = CreateServiceFunction(MS_DBT_SETTING_READHANDLE,       DBSettingReadHandle);
	gServices[25] = CreateServiceFunction(MS_DBT_SETTING_ITER_INIT,        DBSettingIterInit);
	gServices[26] = CreateServiceFunction(MS_DBT_SETTING_ITER_NEXT,        DBSettingIterNext);
	gServices[27] = CreateServiceFunction(MS_DBT_SETTING_ITER_CLOSE,       DBSettingIterClose);

	gServices[28] = CreateServiceFunction(MS_DBT_EVENT_GETBLOBSIZE,        DBEventGetBlobSize);
	gServices[29] = CreateServiceFunction(MS_DBT_EVENT_GET,                DBEventGet);
	gServices[30] = CreateServiceFunction(MS_DBT_EVENT_GETCOUNT,           DBEventGetCount);
	gServices[31] = CreateServiceFunction(MS_DBT_EVENT_DELETE,             DBEventDelete);
	gServices[32] = CreateServiceFunction(MS_DBT_EVENT_ADD,                DBEventAdd);
	gServices[33] = CreateServiceFunction(MS_DBT_EVENT_MARKREAD,           DBEventMarkRead);
	gServices[34] = CreateServiceFunction(MS_DBT_EVENT_WRITETODISK,        DBEventWriteToDisk);
	gServices[35] = CreateServiceFunction(MS_DBT_EVENT_GETCONTACT,         DBEventGetContact);
	// hardlinking is disabled, because it would destroy compatibility with FindNext/Prev Event services
	//gServices[36] = CreateServiceFunction(MS_DBT_EVENT_HARDLINK,           DBEventHardLink);
	gServices[37] = CreateServiceFunction(MS_DBT_EVENT_ITER_INIT,          DBEventIterInit);
	gServices[38] = CreateServiceFunction(MS_DBT_EVENT_ITER_NEXT,          DBEventIterNext);
	gServices[39] = CreateServiceFunction(MS_DBT_EVENT_ITER_CLOSE,         DBEventIterClose);


	return true;
}
