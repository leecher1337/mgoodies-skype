#pragma once

#include "Interface.h"
#include "DataBase.h"

bool RegisterServices();


int DBEntryGetRoot(WPARAM wParam, LPARAM lParam);
int DBEntryChildCount(WPARAM hEntry, LPARAM lParam);
int DBEntryGetParent(WPARAM hEntry, LPARAM lParam);
int DBEntrySetParent(WPARAM hEntry, LPARAM hParent);
int DBEntryGetFirstChild(WPARAM hParent, LPARAM lParam);
int DBEntryGetLastChild(WPARAM hParent, LPARAM lParam);
int DBEntryGetNextSilbing(WPARAM hEntry, LPARAM lParam);
int DBEntryGetPrevSilbing(WPARAM hEntry, LPARAM lParam);
int DBEntryGetFlags(WPARAM hEntry, LPARAM lParam);
int DBEntryIterInit(WPARAM pFilter, LPARAM lParam);
int DBEntryIterNext(WPARAM hIteration, LPARAM lParam);
int DBEntryIterClose(WPARAM hIteration, LPARAM lParam);
int DBEntryDelete(WPARAM hEntry, LPARAM lParam);
int DBEntryCreate(WPARAM hParent, LPARAM Flags);

int DBVirtualEntryCreate(WPARAM hEntry, LPARAM hParent);
int DBVirtualEntryGetParent(WPARAM hVirtuaEntry, LPARAM lParam);
int DBVirtualEntryGetFirst(WPARAM hEntry, LPARAM lParam);
int DBVirtualEntryGetNext(WPARAM hVirtualEntry, LPARAM lParam);

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
