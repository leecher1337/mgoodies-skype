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


#ifndef __M_NICKHISTORY_H__
# define __M_NICKHISTORY_H__


#define MIID_NICKNAME_CHANGE_LOGGER { 0x478be45e, 0xd331, 0x4d63, { 0xa6, 0x57, 0x85, 0xda, 0x45, 0xf8, 0xc, 0xe0 } }
#define MIID_NICKNAME_CHANGE_NOTIFIER { 0xc749d46a, 0x885e, 0x46bf, { 0xaa, 0x4c, 0xe1, 0xae, 0xc5, 0xc9, 0xd0, 0x93 } }

#define EVENTTYPE_NICKNAME_CHANGE 9001


/*
Return TRUE is Nick History is enabled for this contact

wParam: hContact
lParam: ignored
*/
#define MS_NICKHISTORY_ENABLED		"NickHistory/Enabled"


/*
Enable Nick History for a contact

wParam: hContact
lParam: ignored
*/
#define MS_NICKHISTORY_ENABLE		"NickHistory/Enable"


/*
Disable Nick History for a contact

wParam: hContact
lParam: ignored
*/
#define MS_NICKHISTORY_DISABLE		"NickHistory/Disable"





#endif // __M_NICKHISTORY_H__
