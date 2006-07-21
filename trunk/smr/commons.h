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


#include <windows.h>
#include <commctrl.h>
#include <stdio.h>



#ifdef __cplusplus
extern "C" 
{
#endif

// Miranda headers
#include <newpluginapi.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_ignore.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_userinfo.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_updater.h>

#include "../utils/mir_dblists.h"
#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"
#include "../utils/mir_log.h"

#include "resource.h"
#include "m_smr.h"
#include "poll.h"
#include "status_msg.h"
#include "status.h"
#include "options.h"


#define MODULE_NAME		"StatusMsgRetriver"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define INITIAL_TIMER 5000
#define ONLINE_TIMER 5000
#define UPDATE_DELAY 4000
#define ERROR_DELAY 15000
#define POOL_DELAY 1000
#define WAIT_TIME 5000


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


// See if a protocol service exists
__inline static int ProtoServiceExists(const char *szModule,const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}




#ifdef __cplusplus
}
#endif

#endif // __COMMONS_H__
