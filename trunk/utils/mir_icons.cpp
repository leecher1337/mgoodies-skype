/*
Copyright (C) 2007 Ricardo Pescuma Domenecci

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


#include "mir_icons.h"

#include <newpluginapi.h>
#include <m_system.h>
#include <m_icolib.h>


HICON LoadIconEx(char *iconName, BOOL copy)
{
	if (!ServiceExists(MS_SKIN2_GETICON))
		return NULL;
	
	HICON hIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconName);
	if (copy)
	{
		hIcon = CopyIcon(hIcon);
		CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM) iconName);
	}
	return hIcon;
}


void ReleaseIconEx(HICON hIcon)
{
	if (ServiceExists(MS_SKIN2_RELEASEICON))
		CallService(MS_SKIN2_RELEASEICON, (WPARAM) hIcon, 0);
	else
		DestroyIcon(hIcon);
}
