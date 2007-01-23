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


#ifndef __M_VOICESERVICE_H__
# define __M_VOICESERVICE_H__

#include "m_voice.h"

/*
This services are a mirror of the services/notifications in m_voice.h,
with the difference that that ones are to be used by protocols, and this ones
are to be used by plugins that can make calls to contacts in multiple protocols.
*/


/*
Notifies that a voice call changed state

wParam: const VOICE_CALL *
lParam: ignored
return: 0 on success
*/
#define MS_VOICESERVICE_STATE			"VoiceService/State"



struct VOICE_MODULE
{
	int cbSize;			// sizeof(VOICE_MODULE)
	char *name;			// The internal name of the plugin. All PS_* serivces (except PS_VOICE_GETINFO)
						// defined in m_voide.h need to be created based in this name. For example, 
						// PS_VOICE_CALL (/Voice/Call) need to be created as <name>/Voice/Call
	int flags;			// VOICE_* from m_voice.h
};
/*
Register a new plugin that can make/receive voice calls.

wParam: const VOICE_MODULE *
lParam: ignored
return: 0 on success
*/
#define MS_VOICESERVICE_REGISTER		"VoiceService/Register"





#endif // __M_VOICESERVICE_H__
