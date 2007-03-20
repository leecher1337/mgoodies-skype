/*
Status Message Change Notify plugin for Miranda IM.

Copyright © 2004-2005 NoName
Copyright © 2005-2006 Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci

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

#include "commonheaders.h"


int hmPopups;
int hmShowList;
int hmGoToURL;

char *pszStatusMsgURL;

char *pszIconId[ICONCOUNT] = {"smcnotify_pope", "smcnotify_popd", "smcnotify_list", "smcnotify_url", "smcnotify_history", "smcnotify_log"};

extern void LoadIcons(void) {
	SKINICONDESC sid;

	TCHAR *ptszIconName[ICONCOUNT] = {
		TranslateT("PopUps Enabled"),
		TranslateT("PopUps Disabled"),
		TranslateT("List Contacts"),
		TranslateT("Go To URL"),
		TranslateT("Status Message History"),
		TranslateT("Log To File")
	};
	int iIconId[ICONCOUNT] = {IDI_POPUP, IDI_NOPOPUP, IDI_LIST, IDI_URL, IDI_HISTORY, IDI_LOG};
	int i;
	ZeroMemory(&sid, sizeof(sid));
	sid.cbSize = sizeof(sid);
	sid.flags = SIDF_TCHAR;
	sid.ptszSection = TranslateT(PLUGIN_NAME);
	for (i = 0; i < ICONCOUNT; i++)
	{
		sid.pszName = pszIconId[i];
		sid.ptszDescription = ptszIconName[i];
		sid.hDefaultIcon = LoadIcon(hInst, MAKEINTRESOURCE(iIconId[i]));
		hLibIcons[i] = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}

	return;
}

static void UpdateMenuItems(BOOL set) {
	CLISTMENUITEM mi;

	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	if (set)
	{
		mi.pszName = Translate("Disable status message change notification");
		mi.hIcon = ICO_POPUP_E;
	}
	else
	{
		mi.pszName = Translate("Enable status message change notification");
		mi.hIcon = ICO_POPUP_D;
	}
	puopts.bEnable = set;
	mi.flags = CMIM_NAME | CMIM_ICON;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hmPopups, (LPARAM)&mi);

	return;
}

extern void InitMenuItems(void) {
	CLISTMENUITEM mi;

	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
//	mi.flags = CMIF_ICONFROMICOLIB;

	if (PopupActive)
	{
		//Disable/Enable status message change notification
		mi.position = 0;
//		mi.icolibItem = ICO_POPUP_E;
		mi.pszPopupName = Translate("PopUps");
		mi.pszService = MS_SMCNOTIFY_POPUPS;
		hmPopups = CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);

		mi.ptszPopupName = NULL;
	}

	mi.flags = CMIF_TCHAR | CMIF_ICONFROMICOLIB;

	//List Contacts with Status Message - main menu
	mi.position = 500021000;
	mi.icolibItem = ICO_LIST;
	mi.ptszName = TranslateT("List Contacts with Status Message");
	mi.pszService = MS_SMCNOTIFY_LIST;
	hmShowList = CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);

	mi.pszContactOwner = NULL;

	//Go To URL in Status Message - contact menu
	mi.position = -2000004000;
	mi.icolibItem = ICO_URL;
	mi.ptszName = TranslateT("Go To URL in Status Message");
	mi.pszService = MS_SMCNOTIFY_GOTOURL;
	hmGoToURL = CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	UpdateMenuItems(puopts.bEnable);
	return;
}

int MenuItemCmd_PopUps(WPARAM wParam, LPARAM lParam) {
	UpdateMenuItems(wParam ? lParam : !puopts.bEnable);
	return 0;
}

int MenuItemCmd_ShowList(WPARAM wParam, LPARAM lParam) {
	ShowList();
	return 0;
}

int MenuItemCmd_GoToURL(WPARAM wParam, LPARAM lParam) {
	CallService(MS_UTILS_OPENURL, 1, (LPARAM)pszStatusMsgURL);
	MIR_FREE(pszStatusMsgURL);

	return 0;
}

extern int PreBuildCMenu(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	CLISTMENUITEM clmi;
	char *str, *p;
	int c;
	str = NULL;

	ZeroMemory(&clmi, sizeof(clmi));
	clmi.cbSize = sizeof(clmi);
	clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
	if (!DBGetContactSettingTString((HANDLE)wParam, "CList", "StatusMsg", &dbv))
	{
#ifdef UNICODE
		if ((dbv.type == DBVT_ASCIIZ) || (dbv.type == DBVT_UTF8))
		{
			str = dbv.pszVal;
		}
		else if (dbv.type == DBVT_WCHAR)
		{
		str = mir_dupToAscii(dbv.pwszVal);
		}
#else
		if (dbv.type == DBVT_ASCIIZ)
		{
			str = dbv.pszVal;
		}
#endif

		if (lstrcmpA(str, ""))
		{
			p = strstr(str, "www.");
			if (p == NULL) p = strstr(str, "http://");
			if (p != NULL)
			{
				for (c = 0; p[c] != '\n' && p[c] != '\r' && p[c] != '\t' && p[c] != ' ' && p[c] != '\0'; c++);

				lstrcpynA(str, p, c + 1);
				mir_free(pszStatusMsgURL);
				pszStatusMsgURL = mir_strdup(str);

				clmi.flags = CMIM_FLAGS;
			}
		}
		DBFreeVariant(&dbv);
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hmGoToURL, (LPARAM)&clmi);

	mir_free(str);
	mir_free(p);

	return 0;
}