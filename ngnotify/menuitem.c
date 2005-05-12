/*
  Name: NewGenerationNotify - Plugin for Miranda ICQ
  File: main.c - Main DLL procedures
  Version: 0.0.4
  Description: Notifies you about some events
  Author: prezes, <prezesso@klub.chip.pl>
  Date: 01.09.04 / Update: 12.05.05 17:00
  Copyright: (C) 2002 Starzinger Michael

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "newgenerationnotify.h"
#include <m_clist.h>

CLISTMENUITEM menuitem;
HANDLE hMenuitemNotify;
BOOL bNotify;

static int MenuitemNotifyCmd(WPARAM wParam,LPARAM lParam)
{
    bNotify = !bNotify;
    MenuitemUpdate(bNotify);

    //write changes to options->bDisable and into database
    Opt_DisableNGN(!bNotify);

    return 0;
}

int MenuitemUpdate(BOOL bStatus)
{
    menuitem.flags = CMIM_FLAGS | (bStatus ? CMIF_CHECKED : 0);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuitemNotify, (LPARAM)&menuitem);

	return 0;
}

int MenuitemInit(BOOL bStatus)
{
	CreateServiceFunction(MS_NGN_MENUNOTIFY, MenuitemNotifyCmd);

	menuitem.cbSize = sizeof(CLISTMENUITEM);
	menuitem.pszName = Translate(MENUITEM_NAME);
	menuitem.flags = 0;
	menuitem.position = -0x7FFFFFFE;
	menuitem.pszService = MS_NGN_MENUNOTIFY;
	hMenuitemNotify = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menuitem);

	bNotify = bStatus;
	MenuitemUpdate(bNotify);

	return 0;
}
