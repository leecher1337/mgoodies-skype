/*
Scriver

Copyright 2000-2003 Miranda ICQ/IM project,
Copyright 2005 Piotr Piastucki

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include "commonheaders.h"
#include <ctype.h>
#include <mbstring.h>

int safe_wcslen(wchar_t *msg, int maxLen) {
    int i;
	for (i = 0; i < maxLen; i++) {
		if (msg[i] == (wchar_t)0)
			return i;
	}
	return 0;
}

TCHAR *a2tcp(const char *text, int cp) {
	if ( text != NULL ) {
	#if defined ( _UNICODE )
		int cbLen = MultiByteToWideChar( cp, 0, text, -1, NULL, 0 );
		TCHAR* result = ( TCHAR* )mir_alloc( sizeof(TCHAR)*( cbLen+1 ));
		if ( result == NULL )
			return NULL;
		MultiByteToWideChar(cp, 0, text, -1, result, cbLen);
		return result;
	#else
		return mir_strdup(text);
	#endif
	}
	return NULL;
}

char* u2a( const wchar_t* src, int codepage )
{
	int cbLen = WideCharToMultiByte( codepage, 0, src, -1, NULL, 0, NULL, NULL );
	char* result = ( char* )mir_alloc( cbLen+1 );
	if ( result == NULL )
		return NULL;

	WideCharToMultiByte( codepage, 0, src, -1, result, cbLen, NULL, NULL );
	result[ cbLen ] = 0;
	return result;
}

wchar_t* a2u( const char* src, int codepage )
{
	int cbLen = MultiByteToWideChar( codepage, 0, src, -1, NULL, 0 );
	wchar_t* result = ( wchar_t* )mir_alloc( sizeof(wchar_t)*(cbLen+1));
	if ( result == NULL )
		return NULL;
	MultiByteToWideChar( codepage, 0, src, -1, result, cbLen );
	result[ cbLen ] = 0;
	return result;
}

TCHAR *a2t(const char *text) {
	if ( text == NULL )
		return NULL;

	#if defined ( _UNICODE )
		return a2tcp(text, CallService( MS_LANGPACK_GETCODEPAGE, 0, 0 ));
	#else
		return a2tcp(text, CP_ACP);
	#endif
}

char* t2a( const TCHAR* src ) {
	#if defined( _UNICODE )
		return u2a( src, CallService( MS_LANGPACK_GETCODEPAGE, 0, 0 ) );
	#else
		return mir_strdup( src );
	#endif
}

char* t2acp( const TCHAR* src, int codepage ) {
	#if defined( _UNICODE )
		return u2a( src, codepage );
	#else
		return mir_strdup( src );
	#endif
}

static int mimFlags = 0;

enum MIMFLAGS {
	MIM_CHECKED = 1,
	MIM_UNICODE = 2
};

int IsUnicodeMIM() {
	if (!(mimFlags & MIM_CHECKED)) {
		char str[512];
		mimFlags = MIM_CHECKED;
		CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)500, (LPARAM)(char*)str);
		if(strstr(str, "Unicode")) {
			mimFlags |= MIM_UNICODE;
		}
	}
	return (mimFlags & MIM_UNICODE) != 0;
}

