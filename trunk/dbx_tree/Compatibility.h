#pragma once

#include "Interface.h"
#include "DataBase.h"
#include "Services.h"

bool RegisterCompatibilityServices();
bool UnRegisterCompatibilityServices();

int GetContactCount(WPARAM wParam,LPARAM lParam);
int FindFirstContact(WPARAM wParam,LPARAM lParam);
int FindNextContact(WPARAM hContact,LPARAM lParam);
int DeleteContact(WPARAM hContact,LPARAM lParam);
int AddContact(WPARAM wParam,LPARAM lParam);
int IsDbContact(WPARAM hContact,LPARAM lParam);
