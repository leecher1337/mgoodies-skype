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
int DBEntryIterInitBF(WPARAM Filter, LPARAM lParam);
int DBEntryIterInitDF(WPARAM Filter, LPARAM lParam);
int DBEntryIterNext(WPARAM hIteration, LPARAM lParam);
int DBEntryIterClose(WPARAM wParam, LPARAM lParam);
int DBEntryDelete(WPARAM hEntry, LPARAM lParam);
int DBEntryCreate(WPARAM hParent, LPARAM Flags);
int DBVirtualEntryCreate(WPARAM hEntry, LPARAM hParent);
int DBVirtualEntryGetParent(WPARAM hVirtuaEntry, LPARAM lParam);
int DBVirtualEntryGetFirst(WPARAM hEntry, LPARAM lParam);
int DBVirtualEntryGetNext(WPARAM hVirtuaEntry, LPARAM lParam);

