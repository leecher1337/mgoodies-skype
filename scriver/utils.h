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
#ifndef UTILS_H
#define UTILS_H

extern int IsUnicodeMIM();
extern int safe_wcslen(wchar_t *msg, int maxLen) ;
extern TCHAR *a2t(const char *text);
extern TCHAR *a2tcp(const char *text, int cp);
extern char* t2a(const TCHAR* src);
extern char* t2acp(const TCHAR* src, int cp);
extern char* u2a( const wchar_t* src, int codepage );
extern wchar_t* a2u( const char* src, int codepage );
#endif
