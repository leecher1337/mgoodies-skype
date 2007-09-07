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


#ifndef __COMMONS_H__
# define __COMMONS_H__


#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>

// Disable "...truncated to '255' characters in the debug information" warnings
#pragma warning(disable: 4786)

#include <map>
using namespace std;


// Miranda headers
#define MIRANDA_VER 0x0600

#include <newpluginapi.h>
#include <m_system.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_history.h>
#include <m_updater.h>
#include <m_icolib.h>
#include <m_clist.h>
#include <m_message.h>

#include "../utils/mir_memory.h"

#include "m_historyevents.h"
#include "resource.h"


#define MODULE_NAME		"HistoryEvents"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )

#include "../utils/mir_buffer.h"


#include "options.h"

HICON LoadIcon(char* iconName);
HICON LoadIcon(HISTORY_EVENT_HANDLER *heh);
HICON LoadIconEx(HISTORY_EVENT_HANDLER *heh, bool copy);
HICON LoadIconEx(char* iconName, bool copy);
void ReleaseIcon(HICON hIcon);


#define KEEP_FLAG(_x_) ((_x_) & 0xFF00)


#endif // __COMMONS_H__
