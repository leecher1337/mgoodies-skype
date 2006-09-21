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


extern void HasNewListeningInfo();


void CopyListeningInfo(LISTENINGTOINFO *dest, const LISTENINGTOINFO * const src)
{
	FreeListeningInfo(dest);

	dest->cbSize = src->cbSize;
	dest->szArtist = mir_dupT(src->szArtist);
	dest->szAlbum = mir_dupT(src->szAlbum);
	dest->szTitle = mir_dupT(src->szTitle);
	dest->szTrack = mir_dupT(src->szTrack);
	dest->szYear = mir_dupT(src->szYear);
	dest->szGenre = mir_dupT(src->szGenre);
	dest->szLength = mir_dupT(src->szLength);
	dest->szPlayer = mir_dupT(src->szPlayer);
	dest->szType = mir_dupT(src->szType);
}



Player::Player() 
{
	enabled = FALSE;
	needPoll = FALSE;
	ZeroMemory(&listening_info, sizeof(listening_info));
}

Player::~Player()
{
	FreeData();
}


void Player::NotifyInfoChanged()
{
	HasNewListeningInfo();
}


BOOL Player::GetListeningInfo(LISTENINGTOINFO *lti)
{
	if (!enabled)
	{
		FreeData();
		return FALSE;
	}

	if (listening_info.cbSize == 0)
		return FALSE;

	CopyListeningInfo(lti, &listening_info);
	return TRUE;
}

void Player::FreeData()
{
	FreeListeningInfo(&listening_info);
	listening_info.cbSize = 0;
}



PollPlayer::PollPlayer()
{
	needPoll = TRUE;
}



CallbackPlayer::CallbackPlayer()
{
	changed = FALSE;
	InitializeCriticalSection(&cs);
}

CallbackPlayer::~CallbackPlayer()
{
	DeleteCriticalSection(&cs);
}

void CallbackPlayer::FreeData()
{
	EnterCriticalSection(&cs);

	if (listening_info.cbSize != 0)
	{
		Player::FreeData();
		changed = TRUE;
	}

	LeaveCriticalSection(&cs);
}

int CallbackPlayer::ChangedListeningInfo()
{
	int ret;

	EnterCriticalSection(&cs);

	if (!enabled)
	{
		if (listening_info.cbSize == 0)
		{
			ret = 0;
		}
		else
		{
			FreeData();
			ret = -1;
		}
	}

	if (changed)
	{
		changed = FALSE;
		if (listening_info.cbSize == 0)
			ret = -1;
		else
			ret = 1;
	}
	else
	{
		ret = 0;
	}

	LeaveCriticalSection(&cs);

	return ret;
}

