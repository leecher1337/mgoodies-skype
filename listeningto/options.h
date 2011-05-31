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


#ifndef __OPTIONS_H__
# define __OPTIONS_H__


#include "commons.h"

#include <windows.h>


#define POPUP_ACTION_DONOTHING 0
#define POPUP_ACTION_CLOSEPOPUP 1
#define POPUP_ACTION_OPENHISTORY 2

#define POPUP_DELAY_DEFAULT 0
#define POPUP_DELAY_CUSTOM 1
#define POPUP_DELAY_PERMANENT 2

#define SET_XSTATUS 0
#define CHECK_XSTATUS 1
#define CHECK_XSTATUS_MUSIC 2
#define IGNORE_XSTATUS 3


struct Options {
	BOOL enable_sending;
	BOOL enable_music;
	BOOL enable_radio;
	BOOL enable_video;
	BOOL enable_others;

	TCHAR templ[1024];
	TCHAR unknown[128];

	BOOL override_contact_template;
	BOOL show_adv_icon;
	int adv_icon_slot;

//	BOOL get_info_from_watrack;	 //not used
	BOOL enable_other_players;
	BOOL enable_code_injection;
	int time_to_pool;

	WORD xstatus_set;
	TCHAR xstatus_name[1024];
	TCHAR xstatus_message[1024];
	TCHAR nothing[128];
};

extern Options opts;


// Initializations needed by options
void InitOptions();

// Deinitializations needed by options
void DeInitOptions();


// Loads the options from DB
// It don't need to be called, except in some rare cases
void LoadOptions();



BOOL IsTypeEnabled(LISTENINGTOINFO *lti);


#endif // __OPTIONS_H__
