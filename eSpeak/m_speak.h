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


#ifndef __M_SPEAK_H__
# define __M_SPEAK_H__


#define MIID_SPEAK { 0x1ef72725, 0x6a83, 0x483b, { 0xaa, 0x50, 0x89, 0x53, 0xe3, 0x59, 0xee, 0xad } }


/*
Speak a text

wParam: (HANDLE) hContact
lParam: (char *) text
return: 0 on success
*/
#define MS_SPEAK_SAY_A	"Speak/Say"


/*
Speak a unicode text

wParam: (HANDLE) hContact
lParam: (WCHAR *) text
return: 0 on success
*/
#define MS_SPEAK_SAY_W	"Speak/SayW"


#ifdef UNICODE
# define MS_SPEAK_SAY MS_SPEAK_SAY_W
#else
# define MS_SPEAK_SAY MS_SPEAK_SAY_A
#endif


#endif // __M_SPEAK_H__
