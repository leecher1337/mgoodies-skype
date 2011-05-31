/* 
ListeningTo plugin for Miranda IM
==========================================================================
Copyright	(C) 2005-2011 Ricardo Pescuma Domenecci
			(C) 2010-2011 Merlin_de

PRE-CONDITION to use this code under the GNU General Public License:
 1. you do not build another Miranda IM plugin with the code without written permission
    of the autor (peace for the project).
 2. you do not publish copies of the code in other Miranda IM-related code repositories.
    This project is already hosted in a SVN and you are welcome to become a contributing member.
 3. you do not create listeningTo-derivatives based on this code for the Miranda IM project.
    (feel free to do this for another project e.g. foobar)
 4. you do not distribute any kind of self-compiled binary of this plugin (we want continuity
    for the plugin users, who should know that they use the original) you can compile this plugin
    for your own needs, friends, but not for a whole branch of people (e.g. miranda plugin pack).
 5. This isn't free beer. If your jurisdiction (country) does not accept
    GNU General Public License, as a whole, you have no rights to the software
    until you sign a private contract with its author. !!!
 6. you always put these notes and copyright notice at the beginning of your code.
==========================================================================

in case you accept the pre-condition,
this is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "..\commons.h"

WATrack::WATrack(int index)
: Player(index)
{
	m_name = _T("WATrack");
	m_hNewStatusHook = NULL;
}

WATrack::~WATrack()
{
	EVT_Unhook(&m_hNewStatusHook);
}

void
WATrack::EnableDisable()
{
	if (!ServiceExists(MS_WAT_GETMUSICINFO)) {
		m_enabled = FALSE;
		return;
	}

	if (!m_hNewStatusHook) {
		//hook ME_WAT_NEWSTATUS
		int (__cdecl WATrack::*hookProc)(WPARAM, LPARAM);
		hookProc = &WATrack::NewStatus;
		m_hNewStatusHook = HookEventObj(ME_WAT_NEWSTATUS, (MIRANDAHOOKOBJ)*(void **)&hookProc, this);
	}
}

int __cdecl
WATrack::NewStatus(WPARAM wParam, LPARAM lParam)
{
	if (!loaded) return 0;
	EnterCriticalSection(&cs);

	(wParam == WAT_EVENT_PLUGINSTATUS && lParam != 0) ? FreeData() : GetData();

	LeaveCriticalSection(&cs);

	NotifyInfoChanged();
	return 0;
}

void 
WATrack::GetData()
{
#ifdef UNICODE
	SONGINFO *si = NULL;
	INT_PTR playing = CallService(MS_WAT_GETMUSICINFO, WAT_INF_UNICODE, (LPARAM) &si);
#else
	SONGINFOA *si = NULL;
	INT_PTR playing = CallService(MS_WAT_GETMUSICINFO, WAT_INF_ANSI, (LPARAM) &si);
#endif

	FreeData();

	// See if something is playing
	if (playing !=  WAT_PLS_NORMAL
		|| si == NULL
		|| si->status != 1
		|| ( IsEmpty(si->artist) && IsEmpty(si->title) ) )
	{
		return;
	}

	// Copy new data

	m_listening_info.ptszAlbum	= DUP(si->album);
	m_listening_info.ptszArtist	= DUP(si->artist);
	m_listening_info.ptszTitle	= DUP(si->title);
	m_listening_info.ptszYear	= DUP(si->year);

	if (si->track > 0)
	{
		m_listening_info.ptszTrack = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));
		_i64tot(si->track, m_listening_info.ptszTrack, 10);
	}

	m_listening_info.ptszGenre = DUP(si->genre);

	if (si->total > 0)
	{
		m_listening_info.ptszLength = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));

		int s = si->total % 60;
		int m = (si->total / 60) % 60;
		int h = (si->total / 60) / 60;

		if (h > 0)
			mir_sntprintf(m_listening_info.ptszLength, 9, _T("%d:%02d:%02d"), h, m, s);
		else
			mir_sntprintf(m_listening_info.ptszLength, 9, _T("%d:%02d"), m, s);
	}

	if (si->width > 0)
		m_listening_info.ptszType = mir_tstrdup(_T("Video"));
	else
		m_listening_info.ptszType = mir_tstrdup(_T("Music"));

	m_listening_info.ptszPlayer = DUPD(si->player, m_name);

	m_listening_info.cbSize = sizeof(m_listening_info);
	m_listening_info.dwFlags = LTI_TCHAR;
}
