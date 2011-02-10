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


#ifndef __MUSIC_H__
# define __MUSIC_H__

// First non polling ones
#define WATRACK			0
#define GENERIC			1

#define FIRST_PLAYER	2

#define WLM				2
#define WMP				3
#define WINAMP			4
#define ITUNES			5
#define FOOBAR			6
//#define VIDEOLAN		7
#define NUM_PLAYERS		7


void InitMusic();
void FreeMusic();
void EnableDisablePlayers();

int ChangedListeningInfo();
LISTENINGTOINFO * GetListeningInfo();

// Helper functions to players
void FreeListeningInfo(LISTENINGTOINFO *lti);
void CopyListeningInfo(LISTENINGTOINFO *dest, const LISTENINGTOINFO * const src);


#endif // __MUSIC_H__
