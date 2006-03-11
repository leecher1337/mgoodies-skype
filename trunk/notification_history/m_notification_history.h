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


#ifndef __M_NOTIFICATION_HISTORY_H__
# define __M_NOTIFICATION_HISTORY_H__

#ifdef __cplusplus
extern "C" 
{
#endif


#define NFOPT_HISTORY_TEMPLATE_SYSTEM	"History/SystemTemplate"	// const char *
#define NFOPT_HISTORY_TEMPLATE_CONTACT	"History/ContactTemplate"	// const char *

#define NFOPT_HISTORY_TEMPLATE_SYSTEMW	"History/SystemTemplateW"	// const char *
#define NFOPT_HISTORY_TEMPLATE_CONTACTW	"History/ContactTemplateW"	// const char *

#define NFOPT_HISTORY_SYSTEM_LOG		"History/SystemLog"			// BYTE
#define NFOPT_HISTORY_SYSTEM_MARK_READ	"History/SystemMarkRead"	// BYTE
#define NFOPT_HISTORY_CONTACT_LOG		"History/ContactLog"		// BYTE
#define NFOPT_HISTORY_CONTACT_MARK_READ	"History/ContactMarkRead"	// BYTE
#define NFOPT_HISTORY_EVENTTYPE			"History/EventType"			// WORD


#define MS_HISTORY_SHOW					"History/Show"


// Default event type
#define EVENTTYPE_NOTIFICATION			16065




#ifdef _UNICODE

# define NFOPT_HISTORY_TEMPLATE_SYSTEMT NFOPT_HISTORY_TEMPLATE_SYSTEMW
# define NFOPT_HISTORY_TEMPLATE_CONTACTT NFOPT_HISTORY_TEMPLATE_CONTACTW

#else

# define NFOPT_HISTORY_TEMPLATE_SYSTEMT NFOPT_HISTORY_TEMPLATE_SYSTEM
# define NFOPT_HISTORY_TEMPLATE_CONTACTT NFOPT_HISTORY_TEMPLATE_CONTACT

#endif



#ifdef __cplusplus
}
#endif

#endif // __M_NOTIFICATION_HISTORY_H__
