/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2009 Michael "Protogenes" Kunz,

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

#pragma once

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN(__FUNCTION__)

#ifdef _MSC_VER
	#ifdef UNICODE
		#define throwException(Format, ...) throw CException(__WFILE__, __LINE__, __WFUNCTION__, Format, __VA_ARGS__)
	#else
		#define throwException(Format, ...) throw CException(__FILE__, __LINE__, __FUNCTION__, Format, __VA_ARGS__)
	#endif
	#define assertThrow(Assertion, Format, ...) if (!(Assertion)) throwException(Format, __VA_ARGS__)
#else

	#ifdef UNICODE
		#define throwException(Format, ...) throw CException(__WFILE__, __LINE__, __WFUNCTION__, Format, ##__VA_ARGS__)
	#else
		#define throwException(Format, ...) throw CException(__FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
	#endif
	#define assertThrow(Assertion, Format, ...) if (!(Assertion)) throwException(Format, ##__VA_ARGS__)
#endif

#include <tchar.h>

class CException
{
private:
	TCHAR * m_Message;
	const TCHAR * m_File;
	const int     m_Line;
	const TCHAR * m_Function;

	int     m_SysError;
	TCHAR * m_SysMessage;

public:
	CException(const CException & Other);
	CException(const TCHAR * File, const int Line, const TCHAR * Function, const TCHAR * Format, ...);
	~CException();

	void ShowMessage();
};
