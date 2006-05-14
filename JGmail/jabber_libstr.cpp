/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-06  George Hazan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_libstr.cpp,v $
Revision       : $Revision: 1.5 $
Last change on : $Date: 2006/05/12 20:13:35 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"

void __stdcall replaceStr( char*& dest, const char* src )
{
	if ( src != NULL ) {
		if ( dest != NULL )
			mir_free( dest );
		dest = mir_strdup( src );
	}
	else dest = NULL;
}

void __stdcall replaceStr( WCHAR*& dest, const WCHAR* src )
{
	if ( src != NULL ) {
		if ( dest != NULL )
			mir_free( dest );
		dest = mir_wstrdup( src );
	}
	else dest = NULL;
}

char* __stdcall rtrim( char *string )
{
   char* p = string + strlen( string ) - 1;

   while ( p >= string )
   {  if ( *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' )
         break;

		*p-- = 0;
   }
   return string;
}
