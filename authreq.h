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

class CAuthRequestBlob
{
public:
                        CAuthRequestBlob(HANDLE hContact, const bstr_t& uri, const bstr_t& nick);
                        CAuthRequestBlob(const BYTE* blob, unsigned size);
                        ~CAuthRequestBlob(void);

    const char*         GetBuffer(void) const;
    unsigned            GetSize(void) const;

    bstr_t              GetURI(void) const;
    bstr_t              GetNick(void) const;
    HANDLE              GetHContact(void) const;

private:
    // Non copyable
                        CAuthRequestBlob(const CAuthRequestBlob&);
    CAuthRequestBlob&   operator= (const CAuthRequestBlob&);

private:
    char*               m_buffer;
    unsigned            m_size;
    bstr_t              m_uri;
    bstr_t              m_nick;
    HANDLE              m_hContact;
};
//--------------------------------------------------------------------------------------------------

inline CAuthRequestBlob::CAuthRequestBlob(HANDLE hContact, const bstr_t& uri, const bstr_t& nick) :
    m_buffer(0),
    m_size(0),
    m_hContact(hContact),
    m_uri(uri),
    m_nick(nick)
{
    MTLASSERT(hContact);
    MTLASSERT(uri.length() > 0);
    MTLASSERT(nick.length() > 0);

    //Common blob format is:
    //  uin( DWORD ), hContact( HANDLE ), nick( ASCIIZ ), first( ASCIIZ ), last( ASCIIZ ), email( ASCIIZ ), reason( ASCIIZ )
    // Our blob format is:
    // 0( DWORD ), hContact( HANDLE ), nick( ASCIIZ ), ""( ASCIIZ ), ""( ASCIIZ ), uri( ASCIIZ ), ""( ASCIIZ )

    unsigned size = sizeof(DWORD) + sizeof(HANDLE) + nick.length() + uri.length() + 5;
    char* blob = new char[size];
    ZeroMemory(blob, size);
    char* p = blob;

    *((DWORD*)p) = 0;
    p += sizeof(DWORD);
    *((HANDLE*)p) = hContact;
    p += sizeof(HANDLE);
    strcpy(p, nick.length() > 0 ? nick : "");
    p += nick.length() + 1;
    *p = 0; ++p;       //firstName
    *p = 0; ++p;       //lastName
    strcpy(p, uri.length() > 0 ? uri : "");
    p += uri.length() + 1;
    *p = 0;            //reason

    m_buffer = blob;
    m_size = size;
}
//--------------------------------------------------------------------------------------------------

inline CAuthRequestBlob::CAuthRequestBlob(const BYTE* blob, unsigned size) :
    m_buffer(0),
    m_size(0),
    m_hContact(0),
    m_uri(L""),
    m_nick(L"")
{
    MTLASSERT(blob);
    MTLASSERT(size > sizeof(DWORD) + sizeof(HANDLE) + 5);

    if(size > sizeof(DWORD) + sizeof(HANDLE) + 5)
    {
        const BYTE* p = blob;
        p += sizeof(DWORD); // skip uin
        m_hContact = *(HANDLE*)p;
        MTLASSERT(m_hContact);
        p += sizeof(HANDLE); // skip hContact

        m_nick = reinterpret_cast<const char*>(p);
        p += m_nick.length() + 1;

        MTLASSERT(0 == *p); // firstName must be empty
        ++p;                // skip firstName
        MTLASSERT(0 == *p); // lastName must be empty
        ++p;                // skip lastName

        m_uri = reinterpret_cast<const char*>(p);
        MTLASSERT(m_uri.length() > 0);
    }
}
//--------------------------------------------------------------------------------------------------

inline CAuthRequestBlob::~CAuthRequestBlob(void)
{
    if(m_buffer)
        delete[] m_buffer;
    m_buffer = 0;
    m_size = 0;
}
//--------------------------------------------------------------------------------------------------

inline const char* CAuthRequestBlob::GetBuffer(void) const
{
    MTLASSERT(m_buffer);
    return m_buffer;
}
//--------------------------------------------------------------------------------------------------

inline unsigned CAuthRequestBlob::GetSize(void) const
{
    return m_size;
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CAuthRequestBlob::GetNick(void) const
{
    return m_nick;
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CAuthRequestBlob::GetURI(void) const
{
    return m_uri;
}
//--------------------------------------------------------------------------------------------------

inline HANDLE CAuthRequestBlob::GetHContact(void) const
{
    return m_hContact;
}
//--------------------------------------------------------------------------------------------------
