/*

SimpleAway plugin for Miranda-IM

Copyright © 2005 Harven, © 2006-2008 Dezeath

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
#include "simpleaway.h"

/*
** Random - Mitchell-Moore Algorithm
*/

static int	rgiState[2+55];

void init_mm( void ) {
	int			*piState;
	int			iState;
	time_t		rawtime;
	struct tm	*timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	piState	= &rgiState[2];

	piState[-2]	= 55 - 55;
	piState[-1]	= 55 - 24;

	piState[0] = ((int)timeinfo->tm_sec) & ((1 << 30) - 1);
	piState[1] = 1;
	for ( iState = 2; iState < 55; iState++ ) {
		piState[iState] = (piState[iState-1] + piState[iState-2]) & ((1 << 30) - 1);
	}
}

int number_mm( void ) {
	int *piState;
	int iState1;
	int iState2;
	int iRand;

	piState = &rgiState[2];
	iState1	= piState[-2];
	iState2	= piState[-1];
	iRand = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
	piState[iState1] = iRand;
	if ( ++iState1 == 55 )
		iState1 = 0;
	if ( ++iState2 == 55 )
		iState2 = 0;
	piState[-2] = iState1;
	piState[-1] = iState2;
	return iRand >> 6;
}

int ranfr( int from, int to ) { // Random number from range
	if ( (to-from) < 1 )
		return from;
	return ((number_mm() % (to-from+1)) + from);
}
