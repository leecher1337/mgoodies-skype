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


#ifndef __OPTIONS_H__
# define __OPTIONS_H__


#include "commons.h"

#include <windows.h>

#define POPUP_ACTION_DONOTHING 0
#define POPUP_ACTION_CLOSEPOPUP 1
#define POPUP_ACTION_OPENHISTORY 2
#define POPUP_ACTION_OPENSRMM 3

#define POPUP_DELAY_DEFAULT 0
#define POPUP_DELAY_CUSTOM 1
#define POPUP_DELAY_PERMANENT 2


struct Options {
	BYTE track_only_not_offline;
	BYTE dont_notify_on_connect;
	int ttw;

	// File
	TCHAR file_name[1024];
	BYTE file_track_changes;
	BYTE file_track_removes;
	TCHAR file_template_changed[1024];
	TCHAR file_template_removed[1024];

	// Speak
	BYTE speak_track_changes;
	BYTE speak_track_removes;
	TCHAR speak_template_changed[1024];
	TCHAR speak_template_removed[1024];

	// Popup
	BYTE popup_track_changes;
	BYTE popup_track_removes;
	TCHAR popup_template_changed[1024];
	TCHAR popup_template_removed[1024];
	WORD popup_delay_type;
	WORD popup_timeout;
	BYTE popup_use_win_colors;
	BYTE popup_use_default_colors;
	COLORREF popup_bkg_color;
	COLORREF popup_text_color;
	WORD popup_left_click_action;
	WORD popup_right_click_action;
};

extern Options opts[];

DWORD GetSettingDword(int type, char *setting, DWORD def);
WORD GetSettingWord(int type, char *setting, WORD def);
BYTE GetSettingByte(int type, char *setting, BYTE def);
BOOL GetSettingBool(int type, char *setting, BOOL def);
void GetSettingTString(int type, char *setting, TCHAR *str, size_t str_size, TCHAR *def);

void WriteSettingDword(int type, char *setting, DWORD val);
void WriteSettingWord(int type, char *setting, WORD val);
void WriteSettingByte(int type, char *setting, BYTE val);
void WriteSettingBool(int type, char *setting, BOOL val);
void WriteSettingTString(int type, char *setting, TCHAR *str);



// Initializations needed by options
void InitOptions();

// Deinitializations needed by options
void DeInitOptions();


// Loads the options from DB
// It don't need to be called, except in some rare cases
void LoadOptions();



#endif // __OPTIONS_H__
