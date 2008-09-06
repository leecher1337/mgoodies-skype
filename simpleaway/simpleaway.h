/*

SimpleAway plugin for Miranda-IM

Copyright © 2005 Harven, © 2006-2008 Dezeath

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

// this plugin requires Miranda 0.6 or newer
#define MIRANDA_VER 0x0600

// for Visual Studio 2008
#define _CRT_SECURE_NO_DEPRECATE

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "../../include/newpluginapi.h"
#include "../../include/m_button.h"
#include "../../include/m_clist.h"
#include "../../include/m_skin.h"
#include "../../include/m_system.h"
#include "../../include/m_options.h"
#include "../../include/m_langpack.h"
#include "../../include/m_protocols.h"
#include "../../include/m_protosvc.h"
#include "../../include/m_utils.h"
#include "../../include/m_database.h"
#include "../../include/m_awaymsg.h"
#include "../../include/m_icolib.h"
#include "../../include/win2k.h"
#include "include/m_variables.h"
#include "include/m_toptoolbar.h"
#include "include/m_fortunemsg.h"
#include "include/m_statusplugins.h"
//#include "include/AggressiveOptimize.h"
#include "m_simpleaway.h"
#include "resource.h"
#ifndef _WIN32_IE
#define _WIN32_IE 0x0400
#endif
#include <commctrl.h>

#define DLG_SHOW_STATUS				1
#define DLG_SHOW_STATUS_ICONS		2
#define DLG_SHOW_LIST_ICONS			4
#define DLG_SHOW_BUTTONS			8
#define DLG_SHOW_BUTTONS_INLIST		16
#define DLG_SHOW_BUTTONS_FLAT		32
#define	DLG_SHOW_STATUS_PROFILES	64
//NOTE: MAX 128

#define STATUS_SHOW_DLG			1
#define STATUS_EMPTY_MSG		2
#define STATUS_DEFAULT_MSG		4
#define STATUS_LAST_MSG			8
#define STATUS_THIS_MSG			16
#define STATUS_LAST_STATUS_MSG	32

#define PROTO_NO_MSG	1
#define PROTO_THIS_MSG	2
#define PROTO_POPUPDLG	4

#define	ID_STATUS_CURRENT	40082

#ifndef MS_CLIST_ADDSTATUSMENUITEM
#define MS_CLIST_ADDSTATUSMENUITEM			"CList/AddStatusMenuItem"
#endif

#ifndef ME_CLIST_PREBUILDSTATUSMENU
#define ME_CLIST_PREBUILDSTATUSMENU			"CList/PreBuildStatusMenu"
#endif

/*
** Icons
*/

#define ICON_DELETE	"SimpleAway_Cross"
#define ICON_RECENT "SimpleAway_Recent"
#define ICON_PREDEF "SimpleAway_Predef"
#define ICON_ADD	"SimpleAway_Add"
#define ICON_CLEAR	"SimpleAway_Clear"
#define ICON_COPY	"SimpleAway_Copy"
#define	ICON_CSMSG	"SimpleAway_CSMsg"

#define NUM_ICONS 7

extern char 	*sa_ico_name[NUM_ICONS];
extern char 	*sa_ico_descr[NUM_ICONS];
extern int 		sa_ico_id[NUM_ICONS];

struct MsgBoxInitData
{
	char	*proto_name;
	int		status_mode;
	int		all_modes;
	int		all_modes_msg;
	BOOL	ttchange;
	BOOL	onstartup;
};

extern HINSTANCE	hInst;
extern DWORD		ProtoStatusMsgFlags;
extern BOOL			check_winamp;
extern BOOL			removeCR;
extern BOOL			ShowCopy;
extern int			ProtoCount;
extern int			ProtoStatusCount;
extern int			ProtoStatusMsgCount;
extern HWND			hwndSAMsgDialog;

int LoadAwayMsgModule(void);
int AwayMsgPreShutdown(void);

int InitOptions(WPARAM wParam, LPARAM lParam);
int InitStatusOptions(WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK AwayMsgBoxDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetStatusMessage(char *proto_name, int initial_status_mode, int status_mode, char *message, BOOL on_startup);
int GetCurrentStatus(char *proto_name);
int GetStartupStatus(char *proto_name);
char *StatusModeToDbSetting(int status,const char *suffix);
char *GetDefaultMessage(int status);

void init_mm(void);
int ranfr( int from, int to );
