#include "status.h"

HANDLE hContactSettingChanged;

static int ContactSettingChanged(WPARAM wParam, LPARAM lParam);




void InitStatus()
{
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
}


void FreeStatus()
{
	UnhookEvent(hContactSettingChanged);
}


static int ContactSettingChanged(WPARAM wParam, LPARAM lParam) {
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	HANDLE hContact = (HANDLE)wParam;

	// Check contact status changed
	if (hContact != NULL && strcmp(cws->szSetting, "Status") == 0) 
	{
		PoolStatusChangeAddContact(hContact);
	}

	return 0;
}
