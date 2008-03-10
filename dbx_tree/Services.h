#pragma once

#include "Interface.h"
#include "DataBase.h"

bool RegisterServices();


int DBContactGetRoot(WPARAM wParam, LPARAM lParam);
int DBContactChildCount(WPARAM hContact, LPARAM lParam);
int DBContactGetParent(WPARAM hContact, LPARAM lParam);
int DBContactSetParent(WPARAM hContact, LPARAM hParent);
int DBContactGetFirstChild(WPARAM hParent, LPARAM lParam);
int DBContactGetLastChild(WPARAM hParent, LPARAM lParam);
int DBContactGetNextSilbing(WPARAM hContact, LPARAM lParam);
int DBContactGetPrevSilbing(WPARAM hContact, LPARAM lParam);
int DBContactGetFlags(WPARAM hContact, LPARAM lParam);
int DBContactIterInit(WPARAM pFilter, LPARAM hParent);
int DBContactIterNext(WPARAM hIteration, LPARAM lParam);
int DBContactIterClose(WPARAM hIteration, LPARAM lParam);
int DBContactDelete(WPARAM hContact, LPARAM lParam);
int DBContactCreate(WPARAM hParent, LPARAM Flags);

int DBVirtualContactCreate(WPARAM hContact, LPARAM hParent);
int DBVirtualContactGetParent(WPARAM hVirtuaContact, LPARAM lParam);
int DBVirtualContactGetFirst(WPARAM hContact, LPARAM lParam);
int DBVirtualContactGetNext(WPARAM hVirtualContact, LPARAM lParam);

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
