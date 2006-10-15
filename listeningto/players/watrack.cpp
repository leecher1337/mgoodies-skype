/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


#include "..\\commons.h"


WATrack *instance = NULL;

int NewStatusCallback(WPARAM wParam, LPARAM lParam) 
{
	if (instance != NULL)
		instance->NewStatus(wParam, lParam);
	return 0;
}


WATrack::WATrack()
{
	instance = this;
	hNewStatusHook = NULL;
}



WATrack::~WATrack()
{
	if (hNewStatusHook != NULL) 
		UnhookEvent(hNewStatusHook);
}


void WATrack::NewStatus(int event, int value)
{
	if (enabled)
	{
		EnterCriticalSection(&cs);

		if (event == WAT_EVENT_PLUGINSTATUS && value != 0)
		{
			FreeData();
			LeaveCriticalSection(&cs);
			NotifyInfoChanged();
			return;
		}

		GetData();

		LeaveCriticalSection(&cs);
	}
}


void WATrack::EnableDisable()
{
	if (!ServiceExists(MS_WAT_GETMUSICINFO))
	{
		enabled = FALSE;
		return;
	}

	if (hNewStatusHook == NULL)
		hNewStatusHook = HookEvent(ME_WAT_NEWSTATUS, NewStatusCallback);

	EnterCriticalSection(&cs);

	if (enabled)
		GetData();
	else
		FreeData();

	LeaveCriticalSection(&cs);
}


void WATrack::GetData()
{
	SONGINFO *si = NULL;

	int playing = CallService(MS_WAT_GETMUSICINFO, 0, (LPARAM) &si);

	// See if something is playing
	if (playing !=  0
		|| si == NULL
		|| si->status != 1
		|| ( (si->artist == NULL || si->artist[0] == L'0') 
			 && (si->title == NULL || si->title[0] == L'0') ) )
	{
		if (listening_info.cbSize != 0)
		{
			FreeData();
			NotifyInfoChanged();
		}
		return;
	}

	// Copy new data

	changed = TRUE;
	FreeData();

	if (si->album != NULL && si->album[0] != L'\0')
		listening_info.ptszAlbum = mir_dupTW(si->album);

	if (si->artist != NULL && si->artist[0] != L'\0')
		listening_info.ptszArtist = mir_dupTW(si->artist);

	if (si->title != NULL && si->title[0] != L'\0')
		listening_info.ptszTitle = mir_dupTW(si->title);

	if (si->year != NULL && si->year[0] != L'\0')
		listening_info.ptszYear = mir_dupTW(si->year);

	if (si->track > 0)
	{
		listening_info.ptszTrack = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));
		_itot(si->track, listening_info.ptszTrack, 10);
	}

	if (si->genre != NULL && si->genre[0] != L'\0')
		listening_info.ptszGenre = mir_dupTW(si->genre);

	if (si->total > 0)
	{
		listening_info.ptszLength = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));

		int s = si->total % 60;
		int m = (si->total / 60) % 60;
		int h = (si->total / 60) / 60;

		if (h > 0)
			mir_sntprintf(listening_info.ptszLength, 9, _T("%d:%02d:%02d"), h, m, s);
		else
			mir_sntprintf(listening_info.ptszLength, 9, _T("%d:%02d"), m, s);
	}

	if (si->width > 0)
		listening_info.ptszType = mir_dupT(_T("Video"));
	else
		listening_info.ptszType = mir_dupT(_T("Music"));

	listening_info.ptszPlayer = mir_dupTW(si->player);

	listening_info.cbSize = sizeof(listening_info);
	listening_info.dwFlags = LTI_TCHAR;

	NotifyInfoChanged();
}
