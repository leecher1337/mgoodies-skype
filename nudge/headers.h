// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>

#include "..\..\include\newpluginapi.h"	//CallService,UnHookEvent
#include "..\..\include\m_utils.h"			//window broadcasting
#include "..\..\include\m_clist.h"			
#include "..\..\include\m_langpack.h"	
#include "..\..\include\m_system.h"	
#include "..\..\include\m_popup.h"	
#include "..\..\include\m_clui.h"
#include "..\..\include\m_message.h"
#include "..\..\include\m_contacts.h"
#include "..\..\include\m_protocols.h"
#include "..\..\include\m_protomod.h"
#include "..\..\include\m_options.h"
#include "..\..\include\m_skin.h"
#include "..\..\include\m_database.h"
#include "..\..\include\m_protosvc.h"
#include "..\..\include\m_trigger.h"
#include "include\m_metacontacts.h"
#include "include\m_updater.h"	
#include "..\..\include\m_icolib.h"	
#include "resource.h"
#include "m_nudge.h"


/*
*
****************************/
int InitOptions();

/*
*
****************************/
int Preview();

/*
*
****************************/
HANDLE Nudge_GethContact(HANDLE);