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
#define IEWM_MUCC     4             // MUCC group chats GUI
#define IEWM_CHAT     5             // chat.dll group chats GUI

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
#define IEE_SAVE_DOCUMENT	4       // save current document

#define IEEF_RTL          1           // turn on RTL support
#define IEEF_NO_UNICODE   2           // disable Unicode support
#define IEEF_NO_SCROLLING 4           // do not scroll logs to bottom

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

#define IEEDF_UNICODE 		1          // if set pszText is a pointer to wchar_t string instead of char string
/* The following flags are valid only for message events (IEED_EVENT_MESSAGE) */
#define IEEDF_FORMAT_FONT	0x00000100 // if set pszFont (font name) is valid and should be used
#define IEEDF_FORMAT_SIZE	0x00000200 // if set fontSize is valid and should be used
#define IEEDF_FORMAT_COLOR	0x00000400 // if set color is valid and should be used
#define IEEDF_FORMAT_STYLE	0x00000800 // if set fontSize is valid and should be used
		
		
#define IEED_EVENT_MESSAGE		0x0001 // message
#define IEED_EVENT_TOPIC		0x0002 // topic change
#define IEED_EVENT_JOINED		0x0003 // user joined
#define IEED_EVENT_LEFT			0x0004 // user left
#define IEED_EVENT_ERROR		0x0005 // error

#define IEED_GC_EVENT_HIGHLIGHT 	0x8000
#define IEED_GC_EVENT_MESSAGE   	0x0001
#define IEED_GC_EVENT_TOPIC     	0x0002
#define IEED_GC_EVENT_JOIN      	0x0003
#define IEED_GC_EVENT_PART      	0x0004
#define IEED_GC_EVENT_QUIT      	0x0006
#define IEED_GC_EVENT_NICK      	0x0007
#define IEED_GC_EVENT_ACTION    	0x0008
#define IEED_GC_EVENT_KICK      	0x0009
#define IEED_GC_EVENT_NOTICE    	0x000A
#define IEED_GC_EVENT_INFORMATION   0x000B
#define IEED_GC_EVENT_ADDSTATUS     0x000C
#define IEED_GC_EVENT_REMOVESTATUS  0x000D

#define IE_FONT_BOLD			0x000100	// Bold font flag
#define IE_FONT_ITALIC			0x000200	// Italic font flag
#define IE_FONT_UNDERLINE		0x000400	// Underlined font flags 

typedef struct tagIEVIEWEVENTDATA {
	int			cbSize;
	int			iType;				// Event type, one of MUCC_EVENT_* values
	DWORD		dwFlags;			// Event flags - IEEF_*
	const char *fontName;			// Text font name
	int			fontSize;			// Text font size (in pixels)
	int         fontStyle;          // Text font style (combination of IE_FONT_* flags)
	COLORREF	color;				// Text color
	const char *pszProto;			// Name of the protocol
//	const char *pszID;				// Unique identifier of the chat room corresponding to the event,
//	const char *pszName;			// Name of the chat room visible to the user
//	const char *pszUID;				// User identifier, usage depends on type of event
	const char *pszNick;			// Nick, usage depends on type of event
	const char *pszText;			// Text, usage depends on type of event
	DWORD		dwData;				// DWORD data e.g. status
	BOOL		bIsMe;				// TRUE if the event is related to the user
	time_t		time;				// Time of the event
	struct tagIEVIEWEVENTDATA *next;
} IEVIEWEVENTDATA;

#endif

