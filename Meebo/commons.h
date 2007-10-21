/* 
Copyright (C) 2007 Ricardo Pescuma Domenecci

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


// Miranda headers
#define MIRANDA_VER 0x0720
#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_updater.h>
#include <m_popup.h>
#include <m_message.h>
#include "../../protocols/JabberG/m_jabber_plugin.h"

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"

#include "resource.h"



// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


#define MIID_MEEBO { 0x8b29c468, 0xe4bc, 0x4f73, { 0x89, 0xb2, 0x3, 0x89, 0x58, 0x13, 0x4e, 0xbb } }



#endif // __COMMONS_H__
