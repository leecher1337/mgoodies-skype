#include "Exception.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <windows.h>

CException::CException(CException & Other)
{
	int len = strlen(Other.m_Message) + 1;
	m_Message = new char[len];
	strcpy_s(m_Message, len, Other.m_Message);

	m_File = Other.m_File;
	m_Line = Other.m_Line;
	m_Function = Other.m_Function;
	m_SysError = Other.m_SysError;
	
	if (m_SysError)
	{
		len = strlen(Other.m_SysMessage) + 1;
		m_SysMessage = new char[len];
		strcpy_s(m_SysMessage, len, Other.m_SysMessage);
	}
}
CException::CException(char * File, int Line, char * Function, char * Format, ...)
{
	m_SysError = GetLastError();
	m_SysMessage = NULL;

	va_list va;
	va_start(va, Format);

	char buf[2048];
	int len = vsprintf_s(buf, 1024, Format, va);
	m_Message = new char[len + 1];
	strcpy_s(m_Message, len + 1, buf);
	va_end(va);

	if (m_SysError)
	{
		len = 1 + FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, m_SysError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 2048, NULL);
		m_SysMessage = new char[len];
		strcpy_s(m_SysMessage, len, buf);
	}

	m_File = File;
	m_Line = Line;
	m_Function = Function;
}
CException::~CException()
{
	delete [] m_Message;
	if (m_SysError)
		delete [] m_SysMessage;
}

void CException::ShowMessage()
{
	char buf[8192];
	if (m_SysError)
	{
		sprintf_s(buf, 8192, "Error occoured in \"%s\" (%i) in function \"%s\":\n\n%s\n\nSystem Error state: %i\n%s", m_File, m_Line, m_Function, m_Message, m_SysError, m_SysMessage);
	} else {
		sprintf_s(buf, 8192, "Error occoured in \"%s\" (%i) in function \"%s\":\n\n%s", m_File, m_Line, m_Function, m_Message);
	}
	MessageBoxA(0, buf, NULL, MB_OK | MB_ICONERROR); 
}