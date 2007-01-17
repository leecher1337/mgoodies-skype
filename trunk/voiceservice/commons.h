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

#include <vector>
using namespace std;



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
#include <m_clui.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_updater.h>
#include <m_popup.h>
#include <m_cluiframes.h>
#include <m_icolib.h>
#include <m_metacontacts.h>
#include <m_fontservice.h>

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"

#include "m_voice.h"
#include "m_voiceservice.h"

#include "resource.h"
#include "options.h"
#include "frame.h"
#include "popup.h"

#ifdef __cplusplus
}
#endif


#define MODULE_NAME		"VoiceService"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


#define NUM_STATES 5

#define ACTION_CALL 0
#define ACTION_ANSWER 1
#define ACTION_HOLD 2
#define ACTION_DROP 3

#define NUM_ACTIONS 4

#define MAIN_ICON (NUM_STATES + NUM_ACTIONS)
#define NUM_ICONS (NUM_STATES + NUM_ACTIONS + 1)

#define NUM_FONTS NUM_STATES

extern HICON icons[NUM_ICONS];
extern HFONT fonts[NUM_FONTS];
extern COLORREF font_colors[NUM_FONTS];
extern int font_max_height;



struct MODULE_INTERNAL
{
	const char *name;
	int flags;
	BOOL is_protocol;
	HANDLE state_hook;
};


struct VOICE_CALL_INTERNAL 
{
	MODULE_INTERNAL *module;
	char *id;					// Protocol especific ID for this call
	int flags;					// Can be VOICE_CALL_CONTACT or VOICE_CALL_STRING
	HANDLE hContact;
	TCHAR ptszContact[128];
	int state;
	DWORD end_time;
	HANDLE last_dbe;

	~VOICE_CALL_INTERNAL()
	{
		mir_free(id);
	}
};


typedef struct {
	VOICE_CALL_INTERNAL *call;
	VOICE_CALL_INTERNAL *hungry_call;
	BOOL stopping;
	
} CURRENT_CALL;


extern vector<MODULE_INTERNAL> modules;
extern vector<VOICE_CALL_INTERNAL *> calls;
extern CURRENT_CALL currentCall;

TCHAR *GetStateName(int state);

void AnswerCall(VOICE_CALL_INTERNAL * vc);
void DropCall(VOICE_CALL_INTERNAL * vc);
void HoldCall(VOICE_CALL_INTERNAL * vc);


// See if a protocol service exists
__inline static int ProtoServiceExists(const char *szModule,const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}


#define ICON_SIZE 16

#define TIME_TO_SHOW_ENDED_CALL		5000 // ms


#ifdef UNICODE
# define _S "%S"
# define _SI "%s"
#else
# define _S "%s"
# define _SI "%S"
#endif



#define MS_VOICESERVICE_CLIST_DBLCLK "VoiceService/CList/RingingDblClk"

#define MS_VOICESERVICE_CM_CALL "VoiceService/ContactMenu/Call"
#define MS_VOICESERVICE_CM_ANSWER "VoiceService/ContactMenu/Answer"
#define MS_VOICESERVICE_CM_HOLD "VoiceService/ContactMenu/Hold"
#define MS_VOICESERVICE_CM_DROP "VoiceService/ContactMenu/Drop"









#endif // __COMMONS_H__
