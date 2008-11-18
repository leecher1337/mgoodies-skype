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


#ifndef __OPTIONS_H__
# define __OPTIONS_H__


#include <windows.h>

class Language;
class Voice;
class Variant;

struct Options {
	Language *default_language;
	Voice *default_voice;
	Variant *default_variant;

	BOOL disable_offline;
	BOOL disable_online;
	BOOL disable_away;
	BOOL disable_dnd;
	BOOL disable_na;
	BOOL disable_occupied;
	BOOL disable_freechat;
	BOOL disable_invisible;
	BOOL disable_onthephone;
	BOOL disable_outtolunch;
	BOOL enable_only_idle;

	BOOL use_flags;
	BOOL respect_sndvol_mute;
};

extern Options opts;


// Initializations needed by options
void InitOptions();

// Deinitializations needed by options
void DeInitOptions();


// Loads the options from DB
// It don't need to be called, except in some rare cases
void LoadOptions();


BOOL GetSettingBool(SPEAK_TYPE *type, char *setting, BOOL def);
BOOL GetSettingBool(SPEAK_TYPE *type, int templ, char *setting, BOOL def);
void WriteSettingBool(SPEAK_TYPE *type, char *setting, BOOL val);
void WriteSettingBool(SPEAK_TYPE *type, int templ, char *setting, BOOL val);
void WriteSettingTString(SPEAK_TYPE *type, int templ, char *setting, TCHAR *str);



#endif // __OPTIONS_H__
