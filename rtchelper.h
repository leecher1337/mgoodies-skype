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

#include "m_langpack.h"
#include "statusmodes.h"
#include "m_siprtc.h"
//--------------------------------------------------------------------------------------------------

// Fake RTC status
#define RTCXS_PRESENCE_CONNECTING ((RTC_PRESENCE_STATUS)1000)
//--------------------------------------------------------------------------------------------------

inline long GenerateUniqueCookie(void)
{
    static long cookie = 1;
    return InterlockedIncrement(&cookie);
}
//--------------------------------------------------------------------------------------------------

inline bstr_t ComposeNickByUri(const bstr_t& uri)
{
    MTLASSERT(uri.length() > 0);

    bstr_t nick(uri);

    if(uri.length() > 0)
    {
        wchar_t* buf = (wchar_t*)_alloca((uri.length() + 1) * sizeof(wchar_t));
        StringCchCopyW(buf, uri.length() + 1, uri);

        wchar_t* p = buf;

        //
        // Cut off sip prefix
        //
        if(uri.length() > 4 && 0 == StrCmpNI(buf, L"sip:", 4))
        {
            p = &buf[4];
        }

        /*
        //
        // Cut off everything after '@'
        //
        wchar_t* pos = StrChr(p, L'@');
        if(0 != pos)
        {
            *pos = 0;
        }

        //
        // Replace '.', '_', '-' with spaces
        //
        for(wchar_t* i = p; *i; ++i)
        {
            if(L'.' == *i || L'_' == *i || L'-' == *i)
            {
                *i = L' ';
            }
        }
        */

        nick = p;
    }

    return nick;
}
//--------------------------------------------------------------------------------------------------

inline int MyRtcPresenceStatusToMirandaStatus(RTC_PRESENCE_STATUS rtcStatus, bool loggedOn)
{
    int status = ID_STATUS_OFFLINE;

    if(RTCXS_PRESENCE_CONNECTING == rtcStatus)
        status = ID_STATUS_CONNECTING;
    else if(loggedOn)
    {
        switch(rtcStatus)
        {
            case RTCXS_PRESENCE_OFFLINE:        status = ID_STATUS_INVISIBLE; break;
            case RTCXS_PRESENCE_ONLINE:         status = ID_STATUS_ONLINE; break;
            case RTCXS_PRESENCE_AWAY:           status = ID_STATUS_NA; break;
            case RTCXS_PRESENCE_IDLE:           status = ID_STATUS_IDLE; break; // ??? TODO
            case RTCXS_PRESENCE_BUSY:           status = ID_STATUS_OCCUPIED; break;
            case RTCXS_PRESENCE_BE_RIGHT_BACK:  status = ID_STATUS_AWAY; break; // ??? TODO
            case RTCXS_PRESENCE_ON_THE_PHONE:   status = ID_STATUS_ONTHEPHONE; break;
            case RTCXS_PRESENCE_OUT_TO_LUNCH:   status = ID_STATUS_OUTTOLUNCH; break;
            default:
                MTLASSERT(!"Unknown RTC status");
        }
    }

    return status;
}
//--------------------------------------------------------------------------------------------------

inline int ContactRtcPresenceStatusToMirandaStatus(RTC_PRESENCE_STATUS rtcStatus)
{
    int status = ID_STATUS_OFFLINE;
    switch(rtcStatus)
    {
        case RTCXS_PRESENCE_OFFLINE:        status = ID_STATUS_OFFLINE; break;
        case RTCXS_PRESENCE_ONLINE:         status = ID_STATUS_ONLINE; break;
        case RTCXS_PRESENCE_AWAY:           status = ID_STATUS_NA; break;
        case RTCXS_PRESENCE_BUSY:           status = ID_STATUS_OCCUPIED; break;
        case RTCXS_PRESENCE_IDLE:           //status = ID_STATUS_IDLE; break; // ??? TODO
        case RTCXS_PRESENCE_BE_RIGHT_BACK:  status = ID_STATUS_AWAY; break;
        case RTCXS_PRESENCE_ON_THE_PHONE:   status = ID_STATUS_ONTHEPHONE; break;
        case RTCXS_PRESENCE_OUT_TO_LUNCH:   status = ID_STATUS_OUTTOLUNCH; break;
        default:
            MTLASSERT(!"Unknown RTC status");
    }

    return status;
}
//--------------------------------------------------------------------------------------------------

inline RTC_PRESENCE_STATUS MyMirandaStatusToRtcPresenceStatus(int mirStatus)
{
    RTC_PRESENCE_STATUS rtcStatus = RTCXS_PRESENCE_OFFLINE;
    switch(mirStatus)
    {
        case ID_STATUS_INVISIBLE:           rtcStatus = RTCXS_PRESENCE_OFFLINE; break;
        case ID_STATUS_FREECHAT:
        case ID_STATUS_ONLINE:              rtcStatus = RTCXS_PRESENCE_ONLINE; break;
        case ID_STATUS_AWAY:                rtcStatus = RTCXS_PRESENCE_BE_RIGHT_BACK; break;
        case ID_STATUS_NA:                  rtcStatus = RTCXS_PRESENCE_AWAY; break;
        case ID_STATUS_DND:
        case ID_STATUS_OCCUPIED:            rtcStatus = RTCXS_PRESENCE_BUSY; break;
        case ID_STATUS_ONTHEPHONE:          rtcStatus = RTCXS_PRESENCE_ON_THE_PHONE; break;
        case ID_STATUS_OUTTOLUNCH:          rtcStatus = RTCXS_PRESENCE_OUT_TO_LUNCH; break;
        case ID_STATUS_IDLE:                rtcStatus = RTCXS_PRESENCE_IDLE; break;
        case ID_STATUS_OFFLINE:
            MTLASSERT(!"Convertion of ID_STATUS_OFFLINE to RTC! LogOff must be called instead!"); break;
        default:
            MTLASSERT(!"Unknown Miranda status");
    }

    return rtcStatus;
}
//--------------------------------------------------------------------------------------------------

inline bstr_t RtcRegistrationStateToString(RTC_REGISTRATION_STATE state)
{
    MTLASSERT(state <= RTCRS_REMOTE_PA_LOGGED_OFF);
    const wchar_t* names[] =
    {
        L"RTCRS_NOT_REGISTERED",
        L"RTCRS_REGISTERING",
        L"RTCRS_REGISTERED",
        L"RTCRS_REJECTED",
        L"RTCRS_UNREGISTERING",
        L"RTCRS_ERROR",
        L"RTCRS_LOGGED_OFF",
        L"RTCRS_LOCAL_PA_LOGGED_OFF",
        L"RTCRS_REMOTE_PA_LOGGED_OFF"
    };
    return names[state];
}
//--------------------------------------------------------------------------------------------------

inline bstr_t RtcSessionStateToString(RTC_SESSION_STATE state)
{
    MTLASSERT(state <= RTCSS_REFER);
    const wchar_t* names[] =
    {
        L"RTCSS_IDLE",
        L"RTCSS_INCOMING",
        L"RTCSS_ANSWERING",
        L"RTCSS_INPROGRESS",
        L"RTCSS_CONNECTED",
        L"RTCSS_DISCONNECTED",
        L"RTCSS_HOLD",
        L"RTCSS_REFER"
    };
    return names[state];
}
//--------------------------------------------------------------------------------------------------

inline bstr_t RtcParticipantStateToString(RTC_PARTICIPANT_STATE state)
{
    MTLASSERT(state <= RTCPS_DISCONNECTED);
    const wchar_t* names[] =
    {
        L"RTCPS_IDLE",
        L"RTCPS_PENDING",
        L"RTCPS_INCOMING",
        L"RTCPS_ANSWERING",
        L"RTCPS_INPROGRESS",
        L"RTCPS_ALERTING",
        L"RTCPS_CONNECTED",
        L"RTCPS_DISCONNECTING",
        L"RTCPS_DISCONNECTED"
    };
    return names[state];
}
//--------------------------------------------------------------------------------------------------

inline int RtcSessionStateToMSessionState(RTC_SESSION_STATE state)
{
    int res = SIPRTC_CALL_DISCONNECTED;

    switch(state)
    {
    case RTCSS_IDLE:            res = SIPRTC_CALL_IDLE; break;
    case RTCSS_INCOMING:        res = SIPRTC_CALL_INCOMING; break;
    case RTCSS_ANSWERING:       res = SIPRTC_CALL_ANSWERING; break;
    case RTCSS_INPROGRESS:      res = SIPRTC_CALL_INPROGRESS; break;
    case RTCSS_CONNECTED:       res = SIPRTC_CALL_CONNECTED; break;
    case RTCSS_DISCONNECTED:    res = SIPRTC_CALL_DISCONNECTED; break;
    case RTCSS_HOLD:            res = SIPRTC_CALL_HOLD; break;
    case RTCSS_REFER:           res = SIPRTC_CALL_REFER; break;
    default:
        MTLASSERT(!"Unknown session state");
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline bstr_t GetRTCErrorMessage(HRESULT hr)
{
    MTLASSERT(FAILED(hr));

    bstr_t res(L"");

    if(FAILED(hr))
    {
        WCHAR* pText = 0;

        if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                ::GetModuleHandleW(L"RTCRES.DLL"),
                hr, 0, (LPWSTR)&pText, 0, 0) > 0)
        {
            res = pText;
            LocalFree(pText);
        }
        else if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                0, hr, 0, (LPWSTR)&pText, 0, 0) > 0)
        {
            res = pText;
            LocalFree(pText);
        }
        else
        {
            const wchar_t* fmt = TranslateW(L"Unknown error %X");
            unsigned bufSize = lstrlenW(fmt) + 10;
            wchar_t* buf = (wchar_t*)alloca(bufSize * sizeof(wchar_t));
            MTLASSERT(buf);
            mir_sntprintf(buf, bufSize, fmt, hr);
            res = buf;
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int GetLoginFailureReason(HRESULT hr)
{
    // Assume all errors as Network failure
    int reason = LOGINERR_NONETWORK;

    switch(hr)
    {
    case RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED:
    case RTC_E_SIP_AUTH_FAILED:
    case RTC_E_STATUS_CLIENT_UNAUTHORIZED:
    case RTC_E_STATUS_CLIENT_PAYMENT_REQUIRED:
    case RTC_E_STATUS_CLIENT_FORBIDDEN:
    case RTC_E_STATUS_CLIENT_PROXY_AUTHENTICATION_REQUIRED:

        reason = LOGINERR_WRONGPASSWORD;
        break;

    case RTC_E_SIP_SSL_TUNNEL_FAILED:
    case RTC_E_SIP_SSL_NEGOTIATION_TIMEOUT:
    case RTC_E_SIP_DNS_FAIL:
    case RTC_E_SIP_TCP_FAIL:
    case RTC_E_SIP_TLS_FAIL:
    case RTC_E_STATUS_REDIRECT_USE_PROXY:
    case RTC_E_TRANSIENT_SERVER_DISCONNECT:
    case RTC_E_STATUS_CLIENT_TEMPORARILY_NOT_AVAILABLE:
    case RTC_E_STATUS_SERVER_INTERNAL_ERROR:
    case RTC_E_STATUS_SERVER_SERVICE_UNAVAILABLE:
    case RTC_E_STATUS_SERVER_SERVER_TIMEOUT:

        reason = LOGINERR_NONETWORK;
        break;

    case RTC_E_INVALID_PROXY_ADDRESS:

        reason = LOGINERR_PROXYFAILURE;
        break;

    case RTC_E_INVALID_SIP_URL:
    case RTC_E_REGISTRATION_REJECTED:

        reason = LOGINERR_BADUSERID;
        break;

    case RTC_E_SIP_TIMEOUT:
    case RTC_E_STATUS_CLIENT_REQUEST_TIMEOUT:

        reason = LOGINERR_TIMEOUT;
        break;

    case RTC_E_SIP_TRANSPORT_NOT_SUPPORTED:
    case RTC_E_NO_TRANSPORT:
    case RTC_E_UDP_NOT_SUPPORTED:

        reason = LOGINERR_WRONGPROTOCOL;
        break;

    case RTC_E_REGISTRATION_UNREGISTERED:
    case RTC_E_PROFILE_DUPLICATE_USER_URI_AND_SERVER:

        reason = LOGINERR_OTHERLOCATION;
        break;
    }

    return reason;
}
//--------------------------------------------------------------------------------------------------

inline __int64 GetCurrentTimestamp(void)
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    ULARGE_INTEGER li;
    memcpy(&li, &ft, sizeof(ft));
    return li.QuadPart;
}
//--------------------------------------------------------------------------------------------------

inline bstr_t RemoveQualifiersFromUri(const bstr_t& uri)
{
    MTLASSERT(uri.length() > 0);

    bstr_t res = uri;
    if(0 == StrCmpNI(uri, L"sip:", 4) || 0 == StrCmpNI(uri, L"tel:", 4))
    {
        res = static_cast<const wchar_t*>(uri) + 4;
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline bool UrisAreEqual(const bstr_t& uri1, const bstr_t& uri2)
{
    bool res = 0 == lstrcmpiW(uri1, uri2);

    if(!res)
    {
        bstr_t uri1NoQ = RemoveQualifiersFromUri(uri1);
        bstr_t uri2NoQ = RemoveQualifiersFromUri(uri2);

        res = 0 == lstrcmpiW(uri1NoQ, uri2NoQ);
    }

    return res;
}
//--------------------------------------------------------------------------------------------------
