/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

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

#ifdef __cplusplus
extern "C" 
{
#endif


#include <windows.h>
#include <newpluginapi.h>
#include <time.h>
#include <win2k.h>
#include <m_system.h>
#include <m_plugins.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_protocols.h>

#include <m_notify.h>
#include <m_metacontacts.h>

#include "resource.h"


#define MODULE_NAME "NotificationHistory"

extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern PLUGININFO pluginInfo;

#include "m_notification_history.h"
#include "options.h"
#include "../utils/mir_memory.h"
#include "../utils/mir_options_notify.h"


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )




#ifdef __cplusplus
}
#endif

#endif // __COMMONS_H__
