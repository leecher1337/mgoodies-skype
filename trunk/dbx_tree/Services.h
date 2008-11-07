#pragma once

#include "Interface.h"
#include "DataBase.h"

bool RegisterServices();


int DBEntityGetRoot(WPARAM wParam, LPARAM lParam);
int DBEntityChildCount(WPARAM hEntity, LPARAM lParam);
int DBEntityGetParent(WPARAM hEntity, LPARAM lParam);
int DBEntitySetParent(WPARAM hEntity, LPARAM hParent);
int DBEntityGetFlags(WPARAM hEntity, LPARAM lParam);
int DBEntityIterInit(WPARAM pFilter, LPARAM hParent);
int DBEntityIterNext(WPARAM hIteration, LPARAM lParam);
int DBEntityIterClose(WPARAM hIteration, LPARAM lParam);
int DBEntityDelete(WPARAM hEntity, LPARAM lParam);
int DBEntityCreate(WPARAM pEntity, LPARAM lParam);
int DBEntityGetAccount(WPARAM hEntity, LPARAM lParam);

int DBVirtualEntityCreate(WPARAM hEntity, LPARAM hParent);
int DBVirtualEntityGetParent(WPARAM hVirtualEntity, LPARAM lParam);
int DBVirtualEntityGetFirst(WPARAM hEntity, LPARAM lParam);
int DBVirtualEntityGetNext(WPARAM hVirtualEntity, LPARAM lParam);

int DBSettingFind(WPARAM pSettingDescriptor, LPARAM lParam);
int DBSettingDelete(WPARAM pSettingDescriptor, LPARAM lParam);
int DBSettingDeleteHandle(WPARAM hSetting, LPARAM lParam);
int DBSettingWrite(WPARAM pSetting, LPARAM lParam);
int DBSettingWriteHandle(WPARAM pSetting, LPARAM hSetting);
int DBSettingRead(WPARAM pSetting, LPARAM lParam);
int DBSettingReadHandle(WPARAM pSetting, LPARAM hSetting);
int DBSettingIterInit(WPARAM pFilter, LPARAM lParam);
int DBSettingIterNext(WPARAM hIteration, LPARAM lParam);
int DBSettingIterClose(WPARAM hIteration, LPARAM lParam);

int DBEventGetBlobSize(WPARAM hEvent, LPARAM lParam);
int DBEventGet(WPARAM hEvent, LPARAM pEvent);
int DBEventGetCount(WPARAM hEntity, LPARAM lParam);
int DBEventDelete(WPARAM hEntity, LPARAM hEvent);
int DBEventAdd(WPARAM hEntity, LPARAM pEvent);
int DBEventMarkRead(WPARAM hEntity, LPARAM hEvent);
int DBEventWriteToDisk(WPARAM hEntity, LPARAM hEvent);
int DBEventHardLink(WPARAM pHardLink, LPARAM lParam);
int DBEventGetEntity(WPARAM hEvent, LPARAM lParam);
int DBEventIterInit(WPARAM pFilter, LPARAM lParam);
int DBEventIterNext(WPARAM hIteration, LPARAM lParam);
int DBEventIterClose(WPARAM hIteration, LPARAM lParam);