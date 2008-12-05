/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2008 Michael "Protogenes" Kunz,

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

#include "Services.h"

HANDLE gServices[40] = {0};

int DBEntityGetRoot(WPARAM wParam, LPARAM lParam)
{
	return gDataBase->getEntities().getRootEntity();
}
int DBEntityChildCount(WPARAM hEntity, LPARAM lParam)
{
	if (hEntity == 0)
		hEntity = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().getChildCount(hEntity);
}
int DBEntityGetParent(WPARAM hEntity, LPARAM lParam)
{
	if (hEntity == 0)
		hEntity = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().getParent(hEntity);
}
int DBEntitySetParent(WPARAM hEntity, LPARAM hParent)
{
	if ((hEntity == 0) || (hEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().setParent(hEntity, hParent);
}
int DBEntityGetFlags(WPARAM hEntity, LPARAM lParam)
{
	if (hEntity == 0)
		hEntity = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().getFlags(hEntity);
}
int DBEntityIterInit(WPARAM pFilter, LPARAM hParent)
{
	TDBTEntityIterFilter fil = {0};
	if (pFilter == NULL)
	{
		pFilter = (WPARAM)&fil;
		fil.cbSize = sizeof(fil);
	}

	if (((PDBTEntityIterFilter)pFilter)->cbSize != sizeof(TDBTEntityIterFilter))
		return DBT_INVALIDPARAM;

	if (((PDBTEntityIterFilter)pFilter)->fDontHasFlags & ((PDBTEntityIterFilter)pFilter)->fHasFlags)
		return DBT_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().IterationInit(*(PDBTEntityIterFilter)pFilter, hParent);
}
int DBEntityIterNext(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getEntities().IterationNext(hIteration);
}
int DBEntityIterClose(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getEntities().IterationClose(hIteration);
}
int DBEntityDelete(WPARAM hEntity, LPARAM lParam)
{
	if ((hEntity == 0) || (hEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return gDataBase->getEntities().DeleteEntity(hEntity);
}
int DBEntityCreate(WPARAM pEntity, LPARAM lParam)
{
	if (((PDBTEntity)pEntity)->bcSize != sizeof(TDBTEntity))
		return DBT_INVALIDPARAM;

	if (((PDBTEntity)pEntity)->hParentEntity == 0)
		((PDBTEntity)pEntity)->hParentEntity = gDataBase->getEntities().getRootEntity();

	((PDBTEntity)pEntity)->fFlags = ((PDBTEntity)pEntity)->fFlags & ~(DBT_NF_IsRoot | DBT_NF_HasChildren | DBT_NF_IsVirtual | DBT_NF_HasVirtuals); // forbidden flags...
	return gDataBase->getEntities().CreateEntity(*((PDBTEntity)pEntity));
}

int DBEntityGetAccount(WPARAM hEntity, LPARAM lParam)
{
	return gDataBase->getEntities().getAccount(hEntity);
}

int DBVirtualEntityCreate(WPARAM hEntity, LPARAM hParent)
{
	if ((hEntity == 0) || (hEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().VirtualCreate(hEntity, hParent);
}
int DBVirtualEntityGetParent(WPARAM hVirtualEntity, LPARAM lParam)
{
	if ((hVirtualEntity == 0) || (hVirtualEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return gDataBase->getEntities().VirtualGetParent(hVirtualEntity);
}
int DBVirtualEntityGetFirst(WPARAM hEntity, LPARAM lParam)
{
	if ((hEntity == 0) || (hEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;
	
	return gDataBase->getEntities().VirtualGetFirst(hEntity);
}
int DBVirtualEntityGetNext(WPARAM hVirtualEntity, LPARAM lParam)
{
	if ((hVirtualEntity == 0) || (hVirtualEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return gDataBase->getEntities().VirtualGetNext(hVirtualEntity);
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
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getSettings().IterationNext(hIteration);
}
int DBSettingIterClose(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

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

int DBEventGetCount(WPARAM hEntity, LPARAM lParam)
{
	return gDataBase->getEvents().GetCount(hEntity);
}

int DBEventDelete(WPARAM hEntity, LPARAM hEvent)
{
	return gDataBase->getEvents().Delete(hEntity, hEvent);
}

int DBEventAdd(WPARAM hEntity, LPARAM pEvent)
{
	if ((pEvent == NULL) || (((PDBTEvent)pEvent)->cbSize != sizeof(TDBTEvent)) || (((PDBTEvent)pEvent)->pBlob == NULL) || (((PDBTEvent)pEvent)->cbBlob == 0))
		return DBT_INVALIDPARAM;
	
	return gDataBase->getEvents().Add(hEntity, *((PDBTEvent)pEvent));
}

int DBEventMarkRead(WPARAM hEntity, LPARAM hEvent)
{
	return gDataBase->getEvents().MarkRead(hEntity, hEvent);
}

int DBEventWriteToDisk(WPARAM hEntity, LPARAM hEvent)
{
	return gDataBase->getEvents().WriteToDisk(hEntity, hEvent);
}

int DBEventHardLink(WPARAM pHardLink, LPARAM lParam)
{
	if ((pHardLink == NULL) || (((PDBTEventHardLink)pHardLink)->cbSize != sizeof(TDBTEventHardLink)))
		return DBT_INVALIDPARAM;

	return gDataBase->getEvents().HardLink(*((PDBTEventHardLink)pHardLink));
}

int DBEventGetEntity(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().getEntity(hEvent);
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
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getEvents().IterationNext(hIteration);
}

int DBEventIterClose(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getEvents().IterationClose(hIteration);
}


bool RegisterServices()
{
	gServices[ 0] = CreateServiceFunction(MS_DBT_ENTITY_GETROOT,          DBEntityGetRoot);
	gServices[ 1] = CreateServiceFunction(MS_DBT_ENTITY_CHILDCOUNT,       DBEntityChildCount);
	gServices[ 2] = CreateServiceFunction(MS_DBT_ENTITY_GETPARENT,        DBEntityGetParent);
	gServices[ 3] = CreateServiceFunction(MS_DBT_ENTITY_SETPARENT,        DBEntitySetParent);
	gServices[ 8] = CreateServiceFunction(MS_DBT_ENTITY_GETFLAGS,         DBEntityGetFlags);
	gServices[ 9] = CreateServiceFunction(MS_DBT_ENTITY_ITER_INIT,        DBEntityIterInit);
	gServices[10] = CreateServiceFunction(MS_DBT_ENTITY_ITER_NEXT,        DBEntityIterNext);
	gServices[11] = CreateServiceFunction(MS_DBT_ENTITY_ITER_CLOSE,       DBEntityIterClose);
	gServices[12] = CreateServiceFunction(MS_DBT_ENTITY_DELETE,           DBEntityDelete);
	gServices[13] = CreateServiceFunction(MS_DBT_ENTITY_CREATE,           DBEntityCreate);
	gServices[13] = CreateServiceFunction(MS_DBT_ENTITY_GETACCOUNT,       DBEntityGetAccount);

	gServices[14] = CreateServiceFunction(MS_DBT_VIRTUALENTITY_CREATE,    DBVirtualEntityCreate);
	gServices[15] = CreateServiceFunction(MS_DBT_VIRTUALENTITY_GETPARENT, DBVirtualEntityGetParent);
	gServices[16] = CreateServiceFunction(MS_DBT_VIRTUALENTITY_GETFIRST,  DBVirtualEntityGetFirst);
	gServices[17] = CreateServiceFunction(MS_DBT_VIRTUALENTITY_GETNEXT,   DBVirtualEntityGetNext);

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
	gServices[35] = CreateServiceFunction(MS_DBT_EVENT_GETENTITY,         DBEventGetEntity);
	// hardlinking is disabled, because it would destroy compatibility with FindNext/Prev Event services
	//gServices[36] = CreateServiceFunction(MS_DBT_EVENT_HARDLINK,           DBEventHardLink);
	gServices[37] = CreateServiceFunction(MS_DBT_EVENT_ITER_INIT,          DBEventIterInit);
	gServices[38] = CreateServiceFunction(MS_DBT_EVENT_ITER_NEXT,          DBEventIterNext);
	gServices[39] = CreateServiceFunction(MS_DBT_EVENT_ITER_CLOSE,         DBEventIterClose);


	return true;
}
