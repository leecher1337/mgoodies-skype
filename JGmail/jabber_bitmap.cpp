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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_bitmap.cpp,v $
Revision       : $Revision: 1.6 $
Last change on : $Date: 2006/05/12 20:13:35 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"

/////////////////////////////////////////////////////////////////////////////////////////
// JabberEnterBitmapName - enters the picture filename

int __stdcall JabberEnterBitmapName( char* szDest )
{
	*szDest = 0;

	char szFilter[ 512 ];
	JCallService( MS_UTILS_GETBITMAPFILTERSTRINGS, sizeof szFilter, ( LPARAM )szFilter );

	char str[ MAX_PATH ]; str[0] = 0;
	OPENFILENAMEA ofn = {0};
#ifndef OPENFILENAME_SIZE_VERSION_400
	ofn.lStructSize = sizeof( OPENFILENAME );
#else
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#endif
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szDest;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "bmp";
	if ( !GetOpenFileNameA( &ofn ))
		return 1;

	return ERROR_SUCCESS;
}
