/*

Jabber Protocol Plugin (GMail mod) for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-06  George Hazan
Copyright ( C ) 2006  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL$
Revision       : $Rev$
Last change on : $Date$
Last change by : $Author$

*/
#include <m_popup.h>
#include "m_popupw.h"

// Unicode Popup Info
typedef struct {
	HANDLE lchContact;
	HICON lchIcon;
#ifdef _UNICODE
	WCHAR lptzContactName[MAX_CONTACTNAME];
	WCHAR lptzText[MAX_SECONDLINE];
#else
	char lptzContactName[MAX_CONTACTNAME];
	char lptzText[MAX_SECONDLINE];
#endif
	COLORREF colorBack;
	COLORREF colorText;
	WNDPROC PluginWindowProc;
	void * PluginData;
	int iSeconds;                         //Custom delay time in seconds. -1 means "forever", 0 means "default time".
	char cZero[16];                       //some unused bytes which may come useful in the future.
} POPUPDATAT, *LPPOPUPDATAT;

#ifdef _UNICODE
#define MS_POPUP_ADDPOPUPT MS_POPUP_ADDPOPUPW
#else
#define MS_POPUP_ADDPOPUPT MS_POPUP_ADDPOPUPEX
#endif

