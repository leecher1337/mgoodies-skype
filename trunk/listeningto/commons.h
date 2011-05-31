/* 
ListeningTo plugin for Miranda IM
==========================================================================
Copyright	(C) 2005-2011 Ricardo Pescuma Domenecci
			(C) 2010-2011 Merlin_de

PRE-CONDITION to use this code under the GNU General Public License:
 1. you do not build another Miranda IM plugin with the code without written permission
    of the autor (peace for the project).
 2. you do not publish copies of the code in other Miranda IM-related code repositories.
    This project is already hosted in a SVN and you are welcome to become a contributing member.
 3. you do not create listeningTo-derivatives based on this code for the Miranda IM project.
    (feel free to do this for another project e.g. foobar)
 4. you do not distribute any kind of self-compiled binary of this plugin (we want continuity
    for the plugin users, who should know that they use the original) you can compile this plugin
    for your own needs, friends, but not for a whole branch of people (e.g. miranda plugin pack).
 5. This isn't free beer. If your jurisdiction (country) does not accept
    GNU General Public License, as a whole, you have no rights to the software
    until you sign a private contract with its author. !!!
 6. you always put these notes and copyright notice at the beginning of your code.
==========================================================================

in case you accept the pre-condition,
this is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef __COMMONS_H__
# define __COMMONS_H__

/***********************************************************************************************************
 * standard windows includes
 ***********************************************************************************************************/

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <functional>

// Help out windows:
#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif

/***********************************************************************************************************
 * Miranda IM SDK includes and macros
 ***********************************************************************************************************/

#define MIRANDA_VER 0x0800
#define MIRANDA_CUSTOM_LP		//coz we define MIRANDA_VER < 0x1000

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
#include <m_radio.h>
#include <m_toptoolbar.h>
#include <m_icolib.h>
#include <m_icq.h>
#include <m_variables.h>
#include <m_clui.h>
#include <m_cluiframes.h>
#include <m_genmenu.h>
#include <m_hotkeys.h>
#include <m_extraicons.h>

/***********************************************************************************************************
 * utils includes and macros
 ***********************************************************************************************************/

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"
#include "../utils/mir_icons.h"
#include "../utils/mir_buffer.h"
#include "../utils/utf8_helpers.h"

/***********************************************************************************************************
 * Used Plugins SDK includes and macros
 ***********************************************************************************************************/

#include "m_listeningto.h"
#include "music.h"
#include "players\player.h"
#include "players\watrack.h"
#include "players\generic.h"
#include "players\winamp.h"
#include "players\foobar.h"
#include "players\itunes.h"
#include "players\mswmp.h"
#include "players\mswlm.h"
#include "players\mRadio.h"
//#include "players\vlc.h"

#include "version.h"
#include "resource.h"
#include "options.h"

/***********************************************************************************************************
 * macros
 ***********************************************************************************************************/
static bool IsEmpty(const char *str)
{
	return str == NULL || str[0] == 0;
}
static bool IsEmpty(const WCHAR *str)
{
	return str == NULL || str[0] == 0;
}

#define DUP(_X_)			( IsEmpty(_X_) ? NULL : mir_tstrdup(_X_) )
#define DUPD(_X_, _DEF_)	( IsEmpty(_X_) ? mir_tstrdup(_DEF_) : mir_tstrdup(_X_) )
#define U2T(_X_)			( IsEmpty(_X_) ? NULL : mir_u2t(_X_) )
#define U2TD(_X_, _DEF_)	( IsEmpty(_X_) ? mir_u2t(_DEF_) : mir_u2t(_X_) )
#define MIR_FREE(_X_)		{ mir_free(_X_); _X_ = NULL; }
#define MAX_REGS(_A_)		( sizeof(_A_) / sizeof(_A_[0]) )
#define KILLTIMER(_X_)		{ if (_X_ != NULL) {KillTimer(NULL, _X_); _X_ = NULL; }}
#define PtrIsValid(p)		(((p)!=0)&&(((HANDLE)(p))!=INVALID_HANDLE_VALUE))

#ifdef DEBUG
#define DEBUGOUT(_x_,_y_)	{ OutputDebugStringA(_x_); OutputDebugStringA(_y_); OutputDebugStringA("\n");}
#else
 #define DEBUGOUT(_x_,_y_)
#endif

/***********************************************************************************************************
 * listeningto.cpp
 ***********************************************************************************************************/

#define MODULE_NAME		"ListeningTo"
//#define MIN_TIME_BEETWEEN_SETS 10000 // ms
#define MIN_TIME_BEETWEEN_SETS 1000 // ms

// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern BOOL loaded;

extern Player *players[NUM_PLAYERS];

void RebuildMenu();
void StartTimer();
int  ProtoServiceExists(const char *szModule, const char *szService);
struct ProtocolInfo
{
	char proto[128];
	TCHAR account[128];
	HANDLE hMenu;
	int old_xstatus;
	TCHAR old_xstatus_name[1024];
	TCHAR old_xstatus_message[1024];
};


ProtocolInfo *GetProtoInfo(char *proto);


#endif // __COMMONS_H__
