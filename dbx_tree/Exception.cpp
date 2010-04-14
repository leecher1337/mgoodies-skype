/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#include "Exception.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <windows.h>
#ifndef _MSC_VER
#include "savestrings_gcc.h"
#endif

CException::CException(const CException & Other)
:	m_File(Other.m_File),
	m_Line(Other.m_Line),
	m_Function(Other.m_Function)
{
	size_t len = _tcslen(Other.m_Message) + 1;
	m_Message = new TCHAR[len];
	_tcscpy_s(m_Message, len, Other.m_Message);

	m_SysError = Other.m_SysError;

	if (m_SysError)
	{
		len = _tcslen(Other.m_SysMessage) + 1;
		m_SysMessage = new TCHAR[len];
		_tcscpy_s(m_SysMessage, len, Other.m_SysMessage);
	}
}
CException::CException(const TCHAR * File, const int Line, const TCHAR * Function, const TCHAR * Format, ...)
:	m_File(File),
	m_Line(Line),
	m_Function(Function)
{
	m_SysError = GetLastError();
	m_SysMessage = NULL;

	va_list va;
	va_start(va, Format);

	TCHAR buf[2048];
	size_t len = _vstprintf_s(buf, Format, va);
	m_Message = new TCHAR[len + 1];
	_tcscpy_s(m_Message, len + 1, buf);
	va_end(va);

	if (m_SysError)
	{
		len = 1 + FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, m_SysError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 2048, NULL);
		m_SysMessage = new TCHAR[len];
		_tcscpy_s(m_SysMessage, len, buf);
	}

	ShowMessage();
}
CException::~CException()
{
	delete [] m_Message;
	if (m_SysError)
		delete [] m_SysMessage;
}

void CException::ShowMessage()
{
	TCHAR buf[8192];
	if (m_SysError)
	{
		_stprintf_s(buf, _T("Error occoured in \"%s\" (%i) in function \"%s\":\n\n%s\n\nSystem Error state: %i\n%s"), m_File, m_Line, m_Function, m_Message, m_SysError, m_SysMessage);
	} else {
		_stprintf_s(buf, _T("Error occoured in \"%s\" (%i) in function \"%s\":\n\n%s"), m_File, m_Line, m_Function, m_Message);
	}
	MessageBox(0, buf, NULL, MB_OK | MB_ICONERROR);
}
