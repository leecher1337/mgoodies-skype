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


// Miranda headers
#include <newpluginapi.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_updater.h>
#include <m_metacontacts.h>
#include <m_popup.h>
#include <m_history.h>
#include <m_message.h>
#include <m_icq.h>

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"
#include "../utils/mir_buffer.h"
#include "../utils/ContactAsyncQueue.h"
#include "../historyevents/m_historyevents.h"

#include "resource.h"
#include "m_historykeeper.h"
#include "options.h"
#include "popup.h"


#define MODULE_NAME		"HistoryKeeper"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


typedef BOOL (*pfAllowProtocol)(const char *proto);
typedef void (*pfFormat)(TCHAR *out, size_t out_size, void *val);
typedef BOOL (*pfEquals)(TCHAR *a, TCHAR *b);
typedef void (*pfAddVars)(HANDLE hContact, TCHAR **vars, int startAt);

struct HISTORY_TYPE {
	char *name;
	char *description;
	int icon;
	WORD eventType;
	pfAllowProtocol fAllowProtocol;
	pfEquals fEquals;
	pfFormat fFormat;
	int historyFlags;
	BOOL canBeRemoved;

	struct {
		struct {
			char *module; // -1 -> protocol
			char *setting;
			BOOL isString;
		} db;
	} track;

	struct {
		DWORD value;
		BYTE track_only_not_offline;
	} defs;

	int numAddVars;
	pfAddVars fAddVars;
};

extern HISTORY_TYPE types[];
#define NUM_TYPES 5


BOOL AllowProtocol(int type, const char *proto);


#define TIME_TO_WAIT_BEFORE_SHOW_POPUP_AFTER_CREATION 30000 // ms
#define TIME_TO_WAIT_BEFORE_SHOW_POPUP_AFTER_CONNECTION 10000 // ms
#define TIME_TO_WAIT_BEFORE_PROCESSING 3000 // ms


#endif // __COMMONS_H__
