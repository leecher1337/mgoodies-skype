/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci

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


#include "commons.h"


Player *players[] = {
	new WATrack(),
	new WindowsMediaPlayer(),
	new Winamp(),
	new ITunes()
};


void InitMusic() 
{
}


void FreeMusic()
{
	for(int i = 0; i < NUM_PLAYERS; i++)
		delete players[i];
}


void EnableDisablePlayers()
{
	for(int i = 0; i < NUM_PLAYERS; i++)
		players[i]->EnableDisable();
}


void FreeListeningInfo(LISTENINGTOINFO *lti)
{
	MIR_FREE(lti->szArtist);
	MIR_FREE(lti->szAlbum);
	MIR_FREE(lti->szTitle);
	MIR_FREE(lti->szTrack);
	MIR_FREE(lti->szYear);
	MIR_FREE(lti->szGenre);
	MIR_FREE(lti->szLength);
	MIR_FREE(lti->szPlayer);
	MIR_FREE(lti->szType);
}


int ChangedListeningInfo()
{
	if (players[WATRACK]->enabled)
	{
		return players[WATRACK]->ChangedListeningInfo();
	}
	else
	{
		// Find a player playing
		BOOL removed = FALSE;

		for (int i = WATRACK + 1; i < NUM_PLAYERS; i++) 
		{
			if (players[i]->enabled)
			{
				int changed = players[i]->ChangedListeningInfo();

				if (changed < 0)
					removed = TRUE;

				else if (changed > 0)
					return 1;
			}
		}

		return removed ? -1 : 0;;
	}
}


BOOL GetListeningInfo(LISTENINGTOINFO *lti)
{
	// Free old data
	FreeListeningInfo(lti);

	if (players[WATRACK]->enabled)
	{
		return players[WATRACK]->GetListeningInfo(lti);
	}
	else
	{
		// Find a player playing
		for (int i = WATRACK + 1; i < NUM_PLAYERS; i++) {
			if (players[i]->GetListeningInfo(lti))
				return TRUE;
		}

		return FALSE;
	}
}
