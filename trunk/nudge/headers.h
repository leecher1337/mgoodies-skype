#include <wchar.h>
#include <tchar.h>
#include <windows.h>
#include <commctrl.h>
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
#include "..\..\include\m_protocols.h"
#include "..\..\include\m_options.h"
#include "..\..\include\m_skin.h"
#include "..\..\include\m_database.h"
#include "include\m_trigger.h"
#include "include\m_metacontacts.h"
#include "include\m_updater.h"	
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