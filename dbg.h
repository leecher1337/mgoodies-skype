/*

SIP RTC Plugin for Miranda IM

Copyright 2007 Paul Shmakov

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
//--------------------------------------------------------------------------------------------------
#include "database.h"

class CSipRtcTrace
{
public:
    enum TraceLevel
    {
        No,
        Errors,
        Warnings,
        Verbose
    };

                        CSipRtcTrace(void);
                        ~CSipRtcTrace(void);

    TraceLevel          GetLevel(void) const;
    void                SetLevel(TraceLevel level);

    void                Write(const wchar_t* fmt, ...) const;
    void                Write(const wchar_t* fmt, va_list ap) const;
    void                WriteIf(bool condition, const wchar_t* fmt, ...) const;
    void                WriteIf(TraceLevel level, const wchar_t* fmt, ...) const;

    void                WriteError(const wchar_t* fmt, ...) const;
    void                WriteWarning(const wchar_t* fmt, ...) const;
    void                WriteVerbose(const wchar_t* fmt, ...) const;

	void				SetDatabase(CDatabase* db);

private:
    // Copying is not allowed
                        CSipRtcTrace(const CSipRtcTrace&);
    CSipRtcTrace&             operator= (const CSipRtcTrace&);

private:
    TraceLevel          m_level;
    wchar_t             m_logFileName[MAX_PATH];
	CDatabase*			m_db;
};
//--------------------------------------------------------------------------------------------------

inline CSipRtcTrace::CSipRtcTrace(void) :
#if defined(_DEBUG) || defined(DEBUG)
    m_level(Verbose)
#else
    m_level(Verbose)  // TODO
#endif
{
    StringCbCopy(m_logFileName, sizeof(m_logFileName), L"SipRtc.log");

    wchar_t path[MAX_PATH] = { 0 };
    unsigned pathSize = sizeof(path) / sizeof(path[0]);
    if(::GetModuleFileNameW(0, path, pathSize) > 0)
    {
        wchar_t* slash = wcsrchr(path, L'\\');
        if(slash)
        {
            StringCchCopy(slash + 1, pathSize - (slash - path) - 1, L"SipRtc.log");
            StringCbCopy(m_logFileName, sizeof(m_logFileName), path);
        }
    }
}
//--------------------------------------------------------------------------------------------------

inline CSipRtcTrace::~CSipRtcTrace(void)
{
    Write(L"SipRtc trace stopped");
}
//--------------------------------------------------------------------------------------------------

inline CSipRtcTrace::TraceLevel CSipRtcTrace::GetLevel(void) const
{
    return m_level;
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcTrace::SetLevel(TraceLevel level)
{
    m_level = level;
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcTrace::Write(const wchar_t* fmt, ...) const
{
    va_list va;
    va_start(va, fmt);
    Write(fmt, va);
    va_end(va);
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcTrace::Write(const wchar_t* fmt, va_list va) const
{
    wchar_t text[1024];

    StringCbVPrintf(text, sizeof(text), fmt, va);

    ::OutputDebugStringW(text);

    if(text[0])
    {
		bool enableLog = m_db->GetMySettingBool("EnableLogFile", false);
		if (enableLog)
		{
			wchar_t date[16];
			::GetDateFormatW(LOCALE_USER_DEFAULT, 0, 0, L"yy'-'MM'-'dd' '", date,
				sizeof(date) / sizeof(date[0]));

			wchar_t timestamp[16];
			::GetTimeFormatW(LOCALE_USER_DEFAULT, 0, 0, L"HH':'mm':'ss' '", timestamp,
				sizeof(timestamp) / sizeof(timestamp[0]));

			HANDLE hFile = ::CreateFile(m_logFileName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, 0);

			if(INVALID_HANDLE_VALUE != hFile)
			{
				::SetFilePointer(hFile, 0, 0, FILE_END);

				DWORD written = 0;

				::WriteFile(hFile, date, lstrlenW(date) * sizeof(wchar_t), &written, 0);
				::WriteFile(hFile, timestamp, lstrlenW(timestamp) * sizeof(wchar_t), &written, 0);

				::WriteFile(hFile, text, lstrlenW(text) * sizeof(wchar_t), &written, 0);

				const wchar_t crlf[] = L"\r\n";
				::WriteFile(hFile, crlf, sizeof(wchar_t) * 2, &written, 0);

				::CloseHandle(hFile);
			}
		} //endif (enableLog)
    }
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcTrace::WriteIf(bool condition, const wchar_t* fmt, ...) const
{
    if(condition)
    {
        va_list va;
        va_start(va, fmt);
        Write(fmt, va);
        va_end(va);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcTrace::WriteIf(TraceLevel level, const wchar_t* fmt, ...) const
{
    if(level <= m_level)
    {
        va_list va;
        va_start(va, fmt);
        Write(fmt, va);
        va_end(va);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcTrace::WriteError(const wchar_t* fmt, ...) const
{
    if(Errors <= m_level)
    {
        va_list va;
        va_start(va, fmt);
        Write(fmt, va);
        va_end(va);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcTrace::WriteWarning(const wchar_t* fmt, ...) const
{
    if(Warnings <= m_level)
    {
        va_list va;
        va_start(va, fmt);
        Write(fmt, va);
        va_end(va);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcTrace::WriteVerbose(const wchar_t* fmt, ...) const
{
    if(Verbose <= m_level)
    {
        va_list va;
        va_start(va, fmt);
        Write(fmt, va);
        va_end(va);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcTrace::SetDatabase(CDatabase* db)
{
	m_db = db;
}
//--------------------------------------------------------------------------------------------------
