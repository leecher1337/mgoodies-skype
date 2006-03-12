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


#ifndef __M_NOTIFICATION_LOG_H__
# define __M_NOTIFICATION_LOG_H__

#ifdef __cplusplus
extern "C" 
{
#endif


#define NFOPT_LOG_FILENAME			"Log/FileName"			// const char *
#define NFOPT_LOG_FILENAMEW			"Log/FileNameW"			// const WCHAR *

#define NFOPT_LOG_TEMPLATE_LINE		"Log/LineTemplate"		// const char *
#define NFOPT_LOG_TEMPLATE_LINEW	"Log/LineTemplateW"		// const WCHAR *

#define NFOPT_LOG_ENABLED			"Log/Enabled"			// BYTE


#define MS_LOG_SHOW					"Log/Show"





#ifdef _UNICODE

# define NFOPT_LOG_FILENAMET NFOPT_LOG_FILENAMEW
# define NFOPT_LOG_TEMPLATE_LINET NFOPT_LOG_TEMPLATE_LINEW

#else

# define NFOPT_LOG_FILENAMET NFOPT_LOG_FILENAME
# define NFOPT_LOG_TEMPLATE_LINET NFOPT_LOG_TEMPLATE_LINE

#endif



#ifdef __cplusplus
}
#endif

#endif // __M_NOTIFICATION_LOG_H__
