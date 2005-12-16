/*

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _KBDNOTIFY_
#define _KBDNOTIFY_

//Enables all notifications (for use by BossKey)
//wParam=0
//lParam=0
//returns 0
#define MS_KBDNOTIFY_ENABLE         "KeyboardNotify/Enable"


//Disables all notifications (for use by BossKey)
//wParam=0
//lParam=0
//returns 0
#define MS_KBDNOTIFY_DISABLE        "KeyboardNotify/Disable"


//Makes the flashing begin
//wParam=(unsigned int)eventCount
//lParam=(char *)szFlashingSequence or NULL if you want the plugin to use current settings
//returns 0
#define MS_KBDNOTIFY_STARTBLINK     "KeyboardNotify/StartBlinking"


//Receives the number of events that were opened (usuful for the 'until events opened' setting)
//wParam=(unsigned int)eventCount
//lParam=0
//returns 0
#define MS_KBDNOTIFY_EVENTSOPENED   "KeyboardNotify/EventsWereOpened"


//Informs if the flashing is active
//wParam=0
//lParam=0
//returns 0 if the flashing is inactive or a pointer to the string representing the sequence being used 
#define MS_KBDNOTIFY_FLASHINGACTIVE "KeyboardNotify/IsFlashingActive"


typedef struct {
	int cbSize;				//size of the structure in bytes
	WORD timer_max;			//maximum amount if time, in seconds (0 = do not use max. timer)
	char *szCustomSequence;	//pointer to the string representing the flashing sequence to use or NULL to use the current one
} KBDNOTIFYOPT;

//Makes the flashing begin
//wParam=(unsigned int)eventCount
//lParam=(KBDNOTIFYOPT *)pointer to the flashing options or NULL if you want the plugin to use current settings
//returns 0
#define MS_KBDNOTIFY_STARTBLINKEXT  "KeyboardNotify/StartBlinkingExt"


#endif