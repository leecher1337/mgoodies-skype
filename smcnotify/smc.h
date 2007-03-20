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

#ifndef __SMCNOTIFY_SMC_H
#define __SMCNOTIFY_SMC_H


typedef struct {
	HANDLE hContact;
	TCHAR *cust;
	TCHAR *oldstatusmsg;
	TCHAR *newstatusmsg;
	char *proto;
	int compare;
	DWORD ignore;
	DWORD dTimeStamp;
} STATUSMSGINFO;

#define EVENTTYPE_STATUSCHANGE		25368

#define SMII_POPUP		1
#define SMII_HISTORY	2
#define SMII_LOG		4
#define SMII_ALL		7

int ContactSettingChanged(WPARAM wParam, LPARAM lParam);

#endif // __SMCNOTIFY_SMC_H
