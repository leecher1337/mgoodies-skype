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

#ifndef __SMCNOTIFY_COMMONHEADERS_H
#define __SMCNOTIFY_COMMONHEADERS_H

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>

#define MIRANDA_VER 0x0670

//Miranda headers
#include "newpluginapi.h"
#include "m_system.h"
#include "m_protocols.h"
#include "m_protosvc.h"
#include "m_clist.h"
#include "m_ignore.h"
#include "m_contacts.h"
#include "m_message.h"
#include "m_userinfo.h"
#include "m_skin.h"
#include "m_icolib.h"
#include "m_langpack.h"
#include "m_history.h"
#include "m_database.h"
#include "m_options.h"
#include "m_utils.h"
#include "m_button.h"
#include "m_clc.h"
#include "m_popup.h"
#include "statusmodes.h"

#include "m_toptoolbar.h"
#include "m_updater.h"

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"

#include "resource.h"
#include "m_smcnotify.h"
#include "options.h"
#include "smc.h"
#include "menu.h"
#include "popup.h"


#define MODULE_NAME		"SMCNotify"
#define PLUGIN_NAME		"Status Message Change Notify"


// Global Variables
HINSTANCE hInst;
PLUGINLINK *pluginLink;

void ShowList(void);
HWND hListDlg;

TCHAR* GetStr(STATUSMSGINFO *n, const TCHAR *tmplt);
char* BuildSetting(WORD index, char *suffix);
BOOL FreeSmiStr(STATUSMSGINFO *smi);
WCHAR *mir_dupToUnicodeEx(char *ptr, UINT CodePage);
TCHAR* MyDBGetContactSettingTString_dup(HANDLE hContact, const char *szModule, const char *szSetting, TCHAR *out);

//#define CUSTOMBUILD_CATCHICQSTATUSMSG
//#define CUSTOMBUILD_OSDSUPPORT
//#define CUSTOMBUILD_COLORHISTORY

#endif // __SMCNOTIFY_COMMONHEADERS_H
