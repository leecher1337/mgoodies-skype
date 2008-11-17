#pragma once

#include "Interface.h"
#include "DataBase.h"
#include "Services.h"

bool CompatibilityRegister();
bool CompatibilityUnRegister();

int CompGetContactCount(WPARAM wParam,LPARAM lParam);
int CompFindFirstContact(WPARAM wParam,LPARAM lParam);
int CompFindNextContact(WPARAM hContact,LPARAM lParam);
int CompDeleteContact(WPARAM hContact,LPARAM lParam);
int CompAddContact(WPARAM wParam,LPARAM lParam);
int CompIsDbContact(WPARAM hContact,LPARAM lParam);

int CompGetContactSetting(WPARAM hContact, LPARAM pSetting);
int CompGetContactSettingStr(WPARAM hContact, LPARAM pSetting);
int CompGetContactSettingStatic(WPARAM hContact, LPARAM pSetting);
int CompFreeVariant(WPARAM wParam, LPARAM pSetting);
int CompWriteContactSetting(WPARAM hContact, LPARAM pSetting);
int CompDeleteContactSetting(WPARAM hContact, LPARAM pSetting);
int CompEnumContactSettings(WPARAM hContact, LPARAM pEnum);

int CompGetEventCount(WPARAM wParam, LPARAM lParam);
int CompAddEvent(WPARAM hContact, LPARAM pEventInfo);
int CompDeleteEvent(WPARAM hContact, LPARAM hEvent);
int CompGetBlobSize(WPARAM hEvent, LPARAM lParam);
int CompGetEvent(WPARAM hEvent, LPARAM pEventInfo);
int CompMarkEventRead(WPARAM hContact, LPARAM hEvent);
int CompGetEventContact(WPARAM hEvent, LPARAM lParam);
int CompFindFirstEvent(WPARAM hContact, LPARAM lParam);
int CompFindFirstUnreadEvent(WPARAM hContact, LPARAM lParam);
int CompFindLastEvent(WPARAM hContact, LPARAM lParam);
int CompFindNextEvent(WPARAM hEvent, LPARAM lParam);
int CompFindPrevEvent(WPARAM hEvent, LPARAM lParam);


int CompEncodeString(WPARAM wParam, LPARAM lParam);
int CompDecodeString(WPARAM wParam, LPARAM lParam);

int CompGetProfileName(WPARAM cbBytes, LPARAM pszName);
int CompGetProfilePath(WPARAM cbBytes, LPARAM pszName);
