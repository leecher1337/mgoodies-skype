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

struct SIPRTC_SEARCH_RESULT : public PROTOSEARCHRESULT
{
    wchar_t             uri[255];
};
//--------------------------------------------------------------------------------------------------

struct SearchTerm
{
    bstr_t  term;
    bstr_t  value;
    SearchTerm(const bstr_t& t, const bstr_t& v) : term(t), value(v) {}
};
//--------------------------------------------------------------------------------------------------

struct CheekySearchData
{
    wchar_t             uri[255];
    long                cookie;
};
//--------------------------------------------------------------------------------------------------

inline DWORD CALLBACK CheekyBasicSearchThread(LPVOID param)
{
    std::auto_ptr<CheekySearchData> data((CheekySearchData*)param);

    Sleep(100);

    bstr_t uri;
    if(0 != StrCmpNI(data->uri, L"sip:", 4) && 0 != StrCmpNI(data->uri, L"tel:", 4))
    {
        uri = L"sip:";
    }
    uri += data->uri;

    SIPRTC_SEARCH_RESULT sr;
    ZeroMemory(&sr, sizeof(sr));
    sr.cbSize = sizeof(SIPRTC_SEARCH_RESULT);
    bstr_t nick(data->uri);
    sr.nick = const_cast<char*>(static_cast<const char*>(nick));
    sr.firstName = "";
    sr.lastName = "";
    sr.email = "";
    StringCbCopyW(sr.uri, sizeof(sr.uri), uri);

    HANDLE hProcess = reinterpret_cast<HANDLE>(data->cookie);

    ProtoBroadcastAck(g_env.ProtocolName(), 0, ACKTYPE_SEARCH, ACKRESULT_DATA, hProcess, (LPARAM)&sr);
    ProtoBroadcastAck(g_env.ProtocolName(), 0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, hProcess, 0);

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline HANDLE StartCheekyBasicSearch(const bstr_t& uri, long cookie)
{
    MTLASSERT(uri.length() > 0);

    if(uri.length() > 0)
    {
        CheekySearchData* data = new CheekySearchData;
        StringCbCopyW(data->uri, sizeof(data->uri), uri);
        data->cookie = cookie;

        DWORD tid = 0;
        CloseHandle(CreateThread(0, 0, CheekyBasicSearchThread, data, 0, &tid));
    }

    return reinterpret_cast<HANDLE>(cookie);
}
//--------------------------------------------------------------------------------------------------
