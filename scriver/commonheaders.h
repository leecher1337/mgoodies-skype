/*
Scriver

Copyright 2000-2003 Miranda ICQ/IM project,
Copyright 2005 Piotr Piastucki

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#if defined(UNICODE)
   #define _UNICODE
#endif
#include <wchar.h>
#include <tchar.h>
#undef _WIN32_WINNT
#undef _WIN32_IE
#define COMPILE_MULTIMON_STUBS
#define _WINVER 0x5000
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0500
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include "resource.h"
#ifndef IMF_AUTOFONTSIZEADJUST
#define IMF_AUTOFONTSIZEADJUST	0x0010
#endif
#ifndef IMF_AUTOKEYBOARD
#define IMF_AUTOKEYBOARD		0x0001
#endif
#ifndef SES_EXTENDBACKCOLOR
#define SES_EXTENDBACKCOLOR	4
#endif
#ifndef ST_NEWCHARS
#define ST_NEWCHARS		4
#endif
#ifndef CFM_WEIGHT
#define	CFM_WEIGHT			0x00400000
#endif
#include <win2k.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_clist.h>
#include <m_clc.h>
#include <m_clui.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_userinfo.h>
#include <m_history.h>
#include <m_addcontact.h>
#include <m_message.h>
#include <m_file.h>
#include "cmdlist.h"
#include "msgs.h"
#include "globals.h"
#include "richutil.h"
#include "IcoLib.h"
#include "m_smileyadd.h"
#include "m_metacontacts.h"
#include "m_ieview.h"
#include "m_avatars.h"
#include "m_fontservice.h"
