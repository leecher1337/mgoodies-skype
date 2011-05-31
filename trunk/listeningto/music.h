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
#define MRADIO			7
//#define VIDEOLAN		8
#define NUM_PLAYERS		8


void InitMusic();
void FreeMusic();
void EnableDisablePlayers();

int ChangedListeningInfo();
LISTENINGTOINFO * GetListeningInfo();

// Helper functions to players
void FreeListeningInfo(LISTENINGTOINFO *lti);
void CopyListeningInfo(LISTENINGTOINFO *dest, const LISTENINGTOINFO * const src);


#endif // __MUSIC_H__
