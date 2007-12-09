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

#include "stdafx.h"

#include "database.h"
#include "m_database.h"
#include "m_langpack.h"
#include "statusmodes.h"
#include "rtchelper.h"
//--------------------------------------------------------------------------------------------------

HANDLE CDatabase::FindContact(const wchar_t* uri) const
{
    MTLASSERT(uri);

    HANDLE hContact = reinterpret_cast<HANDLE>(CallService(MS_DB_CONTACT_FINDFIRST, 0, 0));
    while(hContact)
    {
        const char* protocol = reinterpret_cast<const char*>(CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0));
        if(protocol && !lstrcmpA(protocol, g_env.ProtocolName()))
        {
            if(DBGetContactSettingByte(hContact, g_env.ProtocolName(), "ChatRoom", 0) == 0)  // Chat API support
            {
                DBVARIANT dbv;
                if(S_OK == DBGetContactSettingWString(hContact, g_env.ProtocolName(), "uri", &dbv))
                {
                    MTLASSERT(DBVT_WCHAR == dbv.type);

                    bool found = 0 == lstrcmpiW(dbv.pwszVal, uri);

                    DBFreeVariant(&dbv);

                    if(found)
                    {
                        return hContact;
                    }
                }
            }
        }

        hContact = reinterpret_cast<HANDLE>(CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0));
    }

    return 0;
}
//--------------------------------------------------------------------------------------------------

HANDLE CDatabase::AddContact(const wchar_t* uri, const wchar_t* nick, bool temporary) const
{
    MTLASSERT(uri);
    MTLASSERT(0 == FindContact(uri));

    HANDLE hContact = reinterpret_cast<HANDLE>(CallService(MS_DB_CONTACT_ADD, 0, 0));
    if(hContact)
    {
        if(S_OK == CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)g_env.ProtocolName()))
        {
            DBWriteContactSettingWString(hContact, g_env.ProtocolName(), "uri", uri);
            DBWriteContactSettingWString(hContact, g_env.ProtocolName(), "Nick",
                nick && nick[0] ? nick : ComposeNickByUri(uri));
            if(temporary)
            {
                DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
            }
        }
        else
        {
            CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
            MTLASSERT(false);
            hContact = 0;
        }
    }

    MTLASSERT(hContact);

    return hContact;
}
//--------------------------------------------------------------------------------------------------

HANDLE CDatabase::FindOrAddContact(const wchar_t* uri, const wchar_t* nick, bool temporary) const
{
    MTLASSERT(uri);

    HANDLE hContact = FindContact(uri);
    if(!hContact)
        hContact = AddContact(uri, nick, temporary);

    MTLASSERT(hContact);

    return hContact;
}
//--------------------------------------------------------------------------------------------------

bool CDatabase::IsContactTemporary(HANDLE hContact) const
{
    MTLASSERT(hContact);
    return 0 != DBGetContactSettingByte(hContact, "CList", "NotOnList", 0);
}
//--------------------------------------------------------------------------------------------------

void CDatabase::SetContactStatus(const wchar_t* uri, int status) const
{
    HANDLE hContact = FindContact(uri);
    if(hContact)
    {
        const char* protocol = reinterpret_cast<const char*>(CallService(MS_PROTO_GETCONTACTBASEPROTO,
            (WPARAM)hContact, 0));
        if(protocol && !lstrcmpA(protocol, g_env.ProtocolName()))
        {
            DBWriteContactSettingWord(hContact, g_env.ProtocolName(), "Status", status);
        }
        else
            MTLASSERT(!"Not RTC contact");
    }
}
//--------------------------------------------------------------------------------------------------

void CDatabase::SetAllContactsStatusToOffline(void) const
{
    HANDLE hContact = reinterpret_cast<HANDLE>(CallService(MS_DB_CONTACT_FINDFIRST, 0, 0));
    while(hContact)
    {
        const char* protocol = reinterpret_cast<const char*>(CallService(MS_PROTO_GETCONTACTBASEPROTO,
            (WPARAM)hContact, 0));
        if(protocol && 0 == lstrcmpA(protocol, g_env.ProtocolName()))
        {
            if(DBGetContactSettingByte(hContact, g_env.ProtocolName(), "ChatRoom", 0) == 0) // Chat API support
            {
                DBWriteContactSettingWord(hContact, g_env.ProtocolName(), "Status", ID_STATUS_OFFLINE);
                DBDeleteContactSetting(hContact, "CList", "StatusMsg");
            }
        }

        hContact = reinterpret_cast<HANDLE>(CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0));
    }
}
//--------------------------------------------------------------------------------------------------

bstr_t CDatabase::GetMySettingWString(const char* param) const
{
    return GetMySettingWString(g_env.ProtocolName(), param);
}
//--------------------------------------------------------------------------------------------------

bstr_t CDatabase::GetMySettingWString(const char* module, const char* param) const
{
    MTLASSERT(param && lstrlenA(param) > 0);

    bstr_t res(L"");

    DBVARIANT dbv;
    if(S_OK == DBGetContactSettingWString(0, module, param, &dbv))
    {
        MTLASSERT(DBVT_WCHAR == dbv.type);

        res = bstr_t(dbv.pwszVal);

        DBFreeVariant(&dbv);
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

bool CDatabase::GetMySettingBool(const char* param, bool defaultValue) const
{
    MTLASSERT(param && lstrlenA(param) > 0);
    return 0 != DBGetContactSettingByte(0, g_env.ProtocolName(), param, defaultValue);
}
//--------------------------------------------------------------------------------------------------

void CDatabase::WriteMySettingWString(const char* param, const wchar_t* value) const
{
    MTLASSERT(param && lstrlenA(param) > 0);
    MTLASSERT(value);

    int res = DBWriteContactSettingWString(0, g_env.ProtocolName(), param, value);
    MTLASSERT(S_OK == res);
}
//--------------------------------------------------------------------------------------------------

bstr_t CDatabase::GetMyEncryptedSettingWString(const char* param) const
{
    // TODO: MS_DB_CRYPT_DECODESTRING doesn't support unicode
    return GetMySettingWString(param);
}
//--------------------------------------------------------------------------------------------------

void CDatabase::WriteMyEncryptedSettingWString(const char* param, const wchar_t* value) const
{
    // TODO: MS_DB_CRYPT_ENCODESTRING doesn't support unicode
    WriteMySettingWString(param, value);
}
//--------------------------------------------------------------------------------------------------

void CDatabase::WriteMySettingBool(const char* param, bool value) const
{
    MTLASSERT(param && lstrlenA(param) > 0);

    int res = DBWriteContactSettingByte(0, g_env.ProtocolName(), param, value);
    MTLASSERT(S_OK == res);
}
//--------------------------------------------------------------------------------------------------


bstr_t CDatabase::GetContactSettingWString(HANDLE hContact, const char* param) const
{
    return GetContactSettingWString(hContact, g_env.ProtocolName(), param);
}
//--------------------------------------------------------------------------------------------------

bstr_t CDatabase::GetContactSettingWString(HANDLE hContact, const char* module,
    const char* param) const
{
    MTLASSERT(param && lstrlenA(param) > 0);

    bstr_t res(L"");

    DBVARIANT dbv;
    if(S_OK == DBGetContactSettingWString(hContact, module, param, &dbv))
    {
        MTLASSERT(DBVT_WCHAR == dbv.type);

        res = bstr_t(dbv.pwszVal);

        DBFreeVariant(&dbv);
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

int CDatabase::GetContactSettingInt(HANDLE hContact, const char* param, int defaultValue) const
{
    MTLASSERT(hContact);
    MTLASSERT(param && lstrlenA(param) > 0);
    return (int)DBGetContactSettingWord(hContact, g_env.ProtocolName(), param, defaultValue);
}
//--------------------------------------------------------------------------------------------------


void CDatabase::WriteContactSettingWString(HANDLE hContact, const char* param,
    const wchar_t* value) const
{
    MTLASSERT(param && lstrlenA(param) > 0);
    MTLASSERT(value);

    int res = DBWriteContactSettingWString(hContact, g_env.ProtocolName(), param, value);
    MTLASSERT(S_OK == res);
}
//--------------------------------------------------------------------------------------------------

void CDatabase::WriteContactSettingInt(HANDLE hContact, const char* param, int value) const
{
    MTLASSERT(hContact);
    MTLASSERT(param && lstrlenA(param) > 0);
    int res = DBWriteContactSettingWord(hContact, g_env.ProtocolName(), param, value);
    MTLASSERT(S_OK == res);
}
//--------------------------------------------------------------------------------------------------


//
// Support for SiP plugin by TOXIC
//
bstr_t CDatabase::GetMySettingWStringFromSiP(const char* param) const
{
    MTLASSERT(param && lstrlenA(param) > 0);

    bstr_t res(L"");

    DBVARIANT dbv;

    DBCONTACTGETSETTING cgs;
    cgs.szModule  = "SiP";
    cgs.szSetting = param;
    cgs.pValue    = &dbv;
    dbv.type      = DBVT_ASCIIZ;

    if(S_OK == CallService(MS_DB_CONTACT_GETSETTING_STR, 0, (LPARAM)&cgs))
    {
        MTLASSERT(DBVT_ASCIIZ == dbv.type);

        res = bstr_t(dbv.pszVal);

        DBFreeVariant(&dbv);
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

//
// Support for SiP plugin by TOXIC
//
bstr_t CDatabase::GetContactSettingWStringFromSiP(HANDLE hContact, const char* param) const
{
    MTLASSERT(param && lstrlenA(param) > 0);

    bstr_t res(L"");

    DBVARIANT dbv;

    DBCONTACTGETSETTING cgs;
    cgs.szModule  = "SiP";
    cgs.szSetting = param;
    cgs.pValue    = &dbv;
    dbv.type      = DBVT_ASCIIZ;

    if(S_OK == CallService(MS_DB_CONTACT_GETSETTING_STR, (WPARAM)hContact, (LPARAM)&cgs))
    {
        MTLASSERT(DBVT_ASCIIZ == dbv.type);

        res = bstr_t(dbv.pszVal);

        DBFreeVariant(&dbv);
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

void CDatabase::Upgrade(void)
{
    const BYTE NOT_FOUND = 44;

    if(NOT_FOUND == DBGetContactSettingByte(0, g_env.ProtocolName(), "AutoConnect", NOT_FOUND))
    {
        bstr_t server = GetMySettingWString("Server");
        WriteMySettingBool("AutoConnect", 0 == server.length());
    }

    if(NOT_FOUND == DBGetContactSettingByte(0, g_env.ProtocolName(), "AutoAuth", NOT_FOUND))
    {
        bstr_t account = GetMySettingWString("Account");
        WriteMySettingBool("AutoAuth", 0 == account.length());
    }
}
//--------------------------------------------------------------------------------------------------
