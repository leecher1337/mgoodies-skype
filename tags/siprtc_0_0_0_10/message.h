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

class CMirandaMessage
{
public:
                        CMirandaMessage(const bstr_t& contentType, const bstr_t& message);
                        ~CMirandaMessage(void);

    const char*         GetBuffer(void) const;

private:
    // Non copyable
                        CMirandaMessage(const CMirandaMessage&);
    CMirandaMessage&    operator= (const CMirandaMessage&);

private:
    char*               m_buffer;
};
//--------------------------------------------------------------------------------------------------

inline CMirandaMessage::CMirandaMessage(const bstr_t& /*contentType*/, const bstr_t& message) :
    m_buffer(0)
{
    MTLASSERT(message.length() > 0);

    // TODO: Ensure that everything is safe here

    unsigned bufSize = (message.length() + 2) * (sizeof(wchar_t) + sizeof(char));
    m_buffer = new char[bufSize];
    ZeroMemory(m_buffer, bufSize);

    int res = WideCharToMultiByte(CP_THREAD_ACP, 0, message, message.length(),
        m_buffer, message.length() + 1, 0, 0);

    MTLASSERT(res);

    m_buffer[message.length()] = 0;

    HRESULT hr = StringCbCopyW((wchar_t*)(&m_buffer[message.length() + 1]),
        bufSize - message.length() - 1, message);
    MTLASSERT(SUCCEEDED(hr));
}
//--------------------------------------------------------------------------------------------------

inline CMirandaMessage::~CMirandaMessage(void)
{
    if(m_buffer)
        delete[] m_buffer;
    m_buffer = 0;
}
//--------------------------------------------------------------------------------------------------

inline const char* CMirandaMessage::GetBuffer(void) const
{
    MTLASSERT(m_buffer);
    return m_buffer;
}
//--------------------------------------------------------------------------------------------------
