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


#define KEEP_FLAG_TO_COMBO(_x_) (((_x_) & 0xFF00)>>8)
#define KEEP_COMBO_TO_FLAG(_x_) (((_x_) & 0xFF)<<8)

#define FLAGS "Flags"
#define RESPECT_TEXT_FORMAT "RespectTextFormat"
#define RESPECT_TEXT_FONT "RespectTextFont"
#define TEMPLATE_ENABLED "TemplateEnabled"
#define TEMPLATE_TEXT "TemplateText"


DWORD GetSettingDword(HISTORY_EVENT_HANDLER *heh, char *setting, DWORD def);
WORD GetSettingWord(HISTORY_EVENT_HANDLER *heh, char *setting, WORD def);
BYTE GetSettingByte(HISTORY_EVENT_HANDLER *heh, char *setting, BYTE def);
BOOL GetSettingBool(HISTORY_EVENT_HANDLER *heh, char *setting, BOOL def);
BOOL GetSettingBool(HISTORY_EVENT_HANDLER *heh, int templ, char *setting, BOOL def);

void WriteSettingDword(HISTORY_EVENT_HANDLER *heh, char *setting, DWORD val);
void WriteSettingWord(HISTORY_EVENT_HANDLER *heh, char *setting, WORD val);
void WriteSettingByte(HISTORY_EVENT_HANDLER *heh, char *setting, BYTE val);
void WriteSettingBool(HISTORY_EVENT_HANDLER *heh, char *setting, BOOL val);
void WriteSettingBool(HISTORY_EVENT_HANDLER *heh, int templ, char *setting, BOOL val);
void WriteSettingTString(HISTORY_EVENT_HANDLER *heh, int templ, char *setting, TCHAR *str);


// Initializations needed by options
void InitOptions();

// Deinitializations needed by options
void DeInitOptions();



#endif // __OPTIONS_H__
