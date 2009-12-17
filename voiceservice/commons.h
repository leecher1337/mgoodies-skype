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
#include <commctrl.h>


// Disable "...truncated to '255' characters in the debug information" warnings
#pragma warning(disable: 4786)

#include <vector>
using namespace std;



// Miranda headers
#define MIRANDA_VER 0x0900
#include <win2k.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_clc.h>
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
#include <m_skin.h>
#include <portaudio.h>

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"
#include "../utils/utf8_helpers.h"

#include "m_voice.h"
#include "m_voiceservice.h"

#include "resource.h"
#include "options.h"
#include "frame.h"
#include "popup.h"


#define MODULE_NAME		"VoiceService"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


#define NUM_STATES 6

#define ACTION_CALL 0
#define ACTION_ANSWER 1
#define ACTION_HOLD 2
#define ACTION_DROP 3

#define NUM_ACTIONS 4

#define MAIN_ICON (NUM_STATES + NUM_ACTIONS)
#define NUM_ICONS (NUM_STATES + NUM_ACTIONS + 1)

#define NUM_FONTS NUM_STATES

#define AUTO_NOTHING 0
#define AUTO_ACCEPT 1
#define AUTO_DROP 2

extern HICON icons[NUM_ICONS];
extern HFONT fonts[NUM_FONTS];
extern COLORREF font_colors[NUM_FONTS];
extern int font_max_height;
extern COLORREF bkg_color;
extern HBRUSH bk_brush;




class VoiceProvider
{
public:
	TCHAR description[256];
	char name[256];
	int flags;
	bool is_protocol;

	VoiceProvider(const char *name, const TCHAR *description, int flags);
	~VoiceProvider();

	bool CanCall(const TCHAR *number);
	bool CanCall(HANDLE hContact, BOOL now = TRUE);

	void Call(HANDLE hContact, const TCHAR *number);

	bool CanHold();

private:
	bool canHold;
	HANDLE state_hook;
};


class VoiceCall 
{
public:
	VoiceProvider *module;
	char *id;					// Protocol especific ID for this call
	HANDLE hContact;
	TCHAR number[256];
	TCHAR displayName[256];
	int state;
	DWORD end_time;

	VoiceCall(VoiceProvider *module, const char *id);
	~VoiceCall();

	void AppendCallerID(HANDLE hContact, const TCHAR *number);

	void SetState(int state);

	void Drop();
	void Answer();
	void Hold();

	bool CanDrop();
	bool CanAnswer();
	bool CanHold();

	bool IsFinished();

	void Notify(bool history = true, bool popup = true, bool sound = true, bool clist = true);
	void SetNewCallHWND(HWND hwnd);

private:
	HWND hwnd;
	HANDLE last_dbe;
	bool clistBlinking;

	void RemoveNotifications();
	void CreateDisplayName();

};


extern OBJLIST<VoiceProvider> modules;
extern OBJLIST<VoiceCall> calls;

void Answer(VoiceCall *call);
bool CanCall(HANDLE hContact, BOOL now = TRUE);
bool CanCall(const TCHAR *number);
bool CanCallNumber();
bool IsFinalState(int state);


// See if a protocol service exists
__inline static int ProtoServiceExists(const char *szModule,const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	mir_snprintf(str, MAX_REGS(str), "%s%s", szModule, szService);
	return ServiceExists(str);
}


static TCHAR *lstrtrim(TCHAR *str)
{
	int len = lstrlen(str);

	int i;
	for(i = len - 1; i >= 0 && (str[i] == ' ' || str[i] == '\t'); --i) ;
	if (i < len - 1)
	{
		++i;
		str[i] = _T('\0');
		len = i;
	}

	for(i = 0; i < len && (str[i] == ' ' || str[i] == '\t'); ++i) ;
	if (i > 0)
		memmove(str, &str[i], (len - i + 1) * sizeof(TCHAR));

	return str;
}


#define ICON_SIZE 16

#define TIME_TO_SHOW_ENDED_CALL		5000 // ms


#define MS_VOICESERVICE_CLIST_DBLCLK "VoiceService/CList/RingingDblClk"

#define MS_VOICESERVICE_CM_CALL "VoiceService/ContactMenu/Call"
#define MS_VOICESERVICE_CM_ANSWER "VoiceService/ContactMenu/Answer"
#define MS_VOICESERVICE_CM_HOLD "VoiceService/ContactMenu/Hold"
#define MS_VOICESERVICE_CM_DROP "VoiceService/ContactMenu/Drop"



static TCHAR *stateNames[] = {
	_T("Talking"),
	_T("Ringing"),
	_T("Calling"),
	_T("On Hold"),
	_T("Ended"),
	_T("Busy"),
};

static struct {
	char *name;
	char *description;
} sounds[] = {
	{ "voice_started", "Started talking"},
	{ "voice_ringing", "Ringing"},
	{ "voice_calling", "Calling a contact"},
	{ "voice_holded", "Put a call on Hold"},
	{ "voice_ended", "End of call"},
	{ "voice_busy", "Busy signal"},
};

static TCHAR *stateTexts[] = {
	_T("Call from %s has started"),
	_T("Call from %s is ringing"),
	_T("Calling %s"),
	_T("Call from %s is on hold"),
	_T("Call from %s has ended"),
	_T("%s is busy"),
};

static TCHAR *popupTitles[] = {
	_T("Voice call started"),
	_T("Voice call ringing"),
	_T("Voice call"),
	_T("Voice call on hold"),
	_T("Voice call ended"),
	_T("Voice call busy"),
};

static TCHAR *actionNames[] = {
	_T("Voice Call"),
	_T("Answer Voice Call"),
	_T("Hold Voice Call"),
	_T("Drop Voice Call"),
};



#endif // __COMMONS_H__
