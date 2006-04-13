/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


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
	const char *proto = cws->szModule;

	// Check contact status changed
	if (hContact != NULL && proto != NULL) 
	{
		if (strcmp(cws->szSetting, "Status") == 0)
		{
			PollStatusChangeAddContact(hContact);
		}
		else if (strcmp(cws->szSetting, "XStatusName") == 0 
			|| strcmp(cws->szSetting, "XStatusMsg") == 0
			|| strcmp(cws->szSetting, "XStatusId") == 0)
		{
			if (opts.when_xstatus != Normal && PollCheckProtocol(proto))
				ProtocolStatusCheckMsg(hContact, proto);
		}
	}

	return 0;
}
