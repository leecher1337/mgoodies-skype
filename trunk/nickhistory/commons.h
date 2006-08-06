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


#ifdef __cplusplus
extern "C" 
{
#endif

// Miranda headers
#include <newpluginapi.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_updater.h>
#include <m_metacontacts.h>
#include <m_popupw.h>
#include <m_popup.h>
#include <m_history.h>

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"

#include "resource.h"
#include "m_nickhistory.h"
#include "options.h"
#include "popup.h"


#define MODULE_NAME		"NickHistory"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )

#define DEFAULT_TEMPLATE_CHANGED "changed his/her nickname to %s"
#define DEFAULT_TEMPLATE_REMOVED "removed his/her nickname"


#ifdef UNICODE

#define TCHAR_TO_CHAR(dest, orig)	mir_snprintf(dest, MAX_REGS(dest), "%S", orig)
#define CHAR_TO_TCHAR(dest, orig)	mir_sntprintf(dest, MAX_REGS(dest), "%S", orig)

#else

#define TCHAR_TO_CHAR(dest, orig)	lstrcpynA(dest, orig, MAX_REGS(dest))
#define CHAR_TO_TCHAR(dest, orig)	lstrcpynA(dest, orig, MAX_REGS(dest))

#endif


BOOL AllowProtocol(const char *proto);


#ifdef __cplusplus
}
#endif

#endif // __COMMONS_H__
