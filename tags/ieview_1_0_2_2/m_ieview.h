/*

IEView Plugin for Miranda IM
Copyright (C) 2005  Piotr Piastucki

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

#ifndef M_IEVIEW_INCLUDED
#define M_IEVIEW_INCLUDED

#define MS_IEVIEW_WINDOW  "IEVIEW/NewWindow"
#define MS_IEVIEW_EVENT	  "IEVIEW/Event"
#define MS_IEVIEW_UTILS   "IEVIEW/Utils"
#define MS_IEVIEW_SHOWSMILEYSELECTION  "IEVIEW/ShowSmileySelection"

#define ME_IEVIEW_NOTIFICATION  "IEVIEW/Notification"

#define IEW_CREATE  1               // create new window (control)
#define IEW_DESTROY 2               // destroy control
#define IEW_SETPOS  3               // set window position and size

#define IEWM_SRMM     0             // regular SRMM
#define IEWM_TABSRMM  1             // TabSRMM-compatible HTML builder
#define IEWM_HTML     2             // HTML
#define IEWM_SCRIVER  3             // HTML

typedef struct {
	int			cbSize;             // size of the strusture
	int			iType;				// one of IEW_* values
	DWORD		dwMode;				// compatibility mode - one of IEWM_* values
	DWORD		dwFlags;			// flags, one of IEWF_* values
	HWND		parent;             // parent window HWND
	HWND 		hwnd;               // IEW_CREATE returns WebBrowser control's HWND here
	int			x;                  // IE control horizontal position
	int			y;                  // IE control vertical position
	int			cx;                 // IE control horizontal size
	int			cy;                 // IE control vertical size
} IEVIEWWINDOW;

#define IEE_LOG_EVENTS  	1       // log specified number of DB events
#define IEE_CLEAR_LOG		2       // clear log
#define IEE_GET_SELECTION	3       // get selected text

#define IEEF_RTL        1           // turn on RTL support
#define IEEF_NO_UNICODE 2           // disable Unicode support

typedef struct {
	int			cbSize;             // size of the strusture
	int			iType;				// one of IEE_* values
	DWORD		dwFlags;			// one of IEEF_* values
	HWND		hwnd;               // HWND returned by IEW_CREATE
	HANDLE      hContact;           // contact
	HANDLE 		hDbEventFirst;      // first event to log, when IEE_LOG_EVENTS returns it will contain
	                                // the last event actually logged or NULL if no event was logged
	int 		count;              // number of events to log
	int         codepage;           // ANSI codepage
} IEVIEWEVENT;


#define IEN_SETTINGS_CHANGED	1

typedef struct {
	int			cbSize;             // size of the strusture
	int			iType;				// one of IEN_* values
} IEVIEWNOTIFICATION;


typedef struct {
	int cbSize;                //size of the structure
	const char* Protocolname;  //protocol to use... if you have defined a protocol, u can
                             //use your own protocol name. Smiley add wil automatically
                             //select the smileypack that is defined for your protocol.
                             //Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
                             //if you prefer those icons.
                             //If not found or NULL: "Standard" will be used
	int xPosition;             //Postition to place the selectwindow
	int yPosition;             // "
	int Direction;             //Direction (i.e. size upwards/downwards/etc) of the window 0, 1, 2, 3
  	HWND hwndTarget;           //Window, where to send the message when smiley is selected.
	UINT targetMessage;        //Target message, to be sent.
	LPARAM targetWParam;       //Target WParam to be sent (LParam will be char* to select smiley)
                             //see the example file.
} IEVIEWSHOWSMILEYSEL;

#endif

