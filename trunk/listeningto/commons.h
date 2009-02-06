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
#define MIRANDA_VER 0x0600

#include <newpluginapi.h>
#include <win2k.h>
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
#include <m_proto_listeningto.h>
#include <m_music.h>
#include <m_toptoolbar.h>
#include <m_icolib.h>
#include <m_icq.h>
#include <m_variables.h>
#include <m_clui.h>
#include "m_cluiframes.h"
#include <m_hotkeys.h>


#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"
#include "../utils/mir_icons.h"
#include "../utils/mir_buffer.h"
#include "../utils/utf8_helpers.h"

#include "m_listeningto.h"
#include "music.h"
#include "resource.h"
#include "options.h"


#define MODULE_NAME		"ListeningTo"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define MIR_FREE(_X_) { mir_free(_X_); _X_ = NULL; }
#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


#ifdef UNICODE

#define TCHAR_TO_CHAR(dest, orig)	mir_snprintf(dest, MAX_REGS(dest), "%S", orig)
#define CHAR_TO_TCHAR(dest, orig)	mir_sntprintf(dest, MAX_REGS(dest), "%S", orig)

#else

#define TCHAR_TO_CHAR(dest, orig)	lstrcpynA(dest, orig, MAX_REGS(dest))
#define CHAR_TO_TCHAR(dest, orig)	lstrcpynA(dest, orig, MAX_REGS(dest))

#endif


#define MIN_TIME_BEETWEEN_SETS 10000 // ms

void StartTimer();
int ProtoServiceExists(const char *szModule, const char *szService);


struct ProtocolInfo
{
	char *proto;
	HANDLE hMenu;
	int old_xstatus;
	TCHAR old_xstatus_name[1024];
	TCHAR old_xstatus_message[1024];
};


#endif // __COMMONS_H__
