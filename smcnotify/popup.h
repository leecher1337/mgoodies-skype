/*
Status Message Change Notify plugin for Miranda IM.

Copyright © 2004-2005 NoName
Copyright © 2005-2006 Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci

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

#ifndef __SMCNOTIFY_POPUP_H
#define __SMCNOTIFY_POPUP_H


BOOL PopupActive;
HANDLE hPopupWindow;

void PopupNotify(STATUSMSGINFO *smi, SMCNOTIFY_PUOPTIONS *puo);
void PopupCheck(STATUSMSGINFO *smi);

void InitPopups(void);
void DeinitPopups(void);

#define TMR_CONNECTIONTIMEOUT		6000

#endif // __SMCNOTIFY_POPUP_H
