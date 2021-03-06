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


#ifndef __M_NOTIFICATION_SPEAK_H__
# define __M_NOTIFICATION_SPEAK_H__

#ifdef __cplusplus
extern "C" 
{
#endif


#define NFOPT_SPEAK_TEMPLATE_TEXT	"Speak/Text"				// const char *
#define NFOPT_SPEAK_TEMPLATE_TEXTW	"Speak/TextW"				// const WCHAR *

#define NFOPT_SPEAK_SAY		"Speak/Say"					// BYTE


#define MS_SPEAK_SHOW		"Speak/Show"



#ifdef _UNICODE

# define NFOPT_SPEAK_TEMPLATE_TEXTT NFOPT_SPEAK_TEMPLATE_TEXTW

#else

# define NFOPT_SPEAK_TEMPLATE_TEXTT NFOPT_SPEAK_TEMPLATE_TEXT

#endif




#ifdef __cplusplus
}
#endif

#endif // __M_NOTIFICATION_SPEAK_H__
