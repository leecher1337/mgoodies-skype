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

class CDatabase
{
public:
    HANDLE              FindContact(const wchar_t* uri) const;
    HANDLE              AddContact(const wchar_t* uri, const wchar_t* nick, bool temporary = false) const;
    HANDLE              FindOrAddContact(const wchar_t* uri, const wchar_t* nick, bool temporary = false) const;

    bool                IsContactTemporary(HANDLE hContact) const;

    void                SetContactStatus(const wchar_t* uri, int status) const;
    void                SetAllContactsStatusToOffline(void) const;

    bstr_t              GetMySettingWString(const char* param) const;
    bstr_t              GetMySettingWString(const char* module, const char* param) const;
    bstr_t              GetMyEncryptedSettingWString(const char* param) const;
    bool                GetMySettingBool(const char* param, bool defaultValue) const;

    void                WriteMySettingWString(const char* param, const wchar_t* value) const;
    void                WriteMyEncryptedSettingWString(const char* param, const wchar_t* value) const;
    void                WriteMySettingBool(const char* param, bool value) const;

    bstr_t              GetContactSettingWString(HANDLE hContact, const char* param) const;
    bstr_t              GetContactSettingWString(HANDLE hContact, const char* module, const char* param) const;
    int                 GetContactSettingInt(HANDLE hContact, const char* param, int defaultValue) const;

    void                WriteContactSettingWString(HANDLE hContact, const char* param, const wchar_t* value) const;
    void                WriteContactSettingInt(HANDLE hContact, const char* param, int value) const;

    //
    // Support for SiP plugin by TOXIC
    //
    bstr_t              GetMySettingWStringFromSiP(const char* param) const;
    bstr_t              GetContactSettingWStringFromSiP(HANDLE hContact, const char* param) const;

    void                Upgrade(void);
};
//--------------------------------------------------------------------------------------------------
