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

#include "m_siprtc.h"
#include "SDK/m_voice.h"
#include "SDK/m_voiceservice.h"
#include "rtcclient.h"
//--------------------------------------------------------------------------------------------------

class CCallServices
{
public:
                        CCallServices(CRtcClient& client);

    BEGIN_PROTO_SERVICES_MAP(CCallServices)
        
        PROTO_SERVICE_MTHREAD(MS_SIPRTC_ACCEPT_CALL,    SvcAcceptCall)
        PROTO_SERVICE_MTHREAD(MS_SIPRTC_REJECT_CALL,    SvcRejectCall)
        PROTO_SERVICE_MTHREAD(MS_SIPRTC_CALL,           SvcCall)
        PROTO_SERVICE_MTHREAD(MS_SIPRTC_TERMINATE_CALL, SvcTerminateCall)
        PROTO_SERVICE_MTHREAD(MS_SIPRTC_GET_CALL_STATE, SvcGetCallState)

        // 
        // Voice service support
        //
        PROTO_SERVICE(PS_VOICE_GETINFO,                 SvcVoiceGetInfo)
        PROTO_SERVICE_MTHREAD(PS_VOICE_CALL,            SvcVoiceCall)
        PROTO_SERVICE_MTHREAD(PS_VOICE_ANSWERCALL,      SvcVoiceAnswerCall)
        PROTO_SERVICE_MTHREAD(PS_VOICE_DROPCALL,        SvcVoiceDropCall)
        PROTO_SERVICE_MTHREAD(PS_VOICE_HOLDCALL,        SvcVoiceHoldCall)
        PROTO_SERVICE(PS_VOICE_CALL_STRING_VALID,		SvcVoiceCallStringValid)
        PROTO_SERVICE_MTHREAD(PS_VOICE_CALL_CONTACT_VALID, SvcVoiceCallContactValid)

    END_PROTO_SERVICES_MAP()

private:
    // Non copyable
                        CCallServices(const CCallServices&);
    CCallServices&      operator= (const CCallServices&);

    int                 SvcAcceptCall(WPARAM, LPARAM);
    int                 SvcRejectCall(WPARAM, LPARAM);
    int                 SvcCall(WPARAM, SIPRTC_CALL* call);
    int                 SvcTerminateCall(WPARAM, LPARAM);
    int                 SvcGetCallState(WPARAM, LPARAM);

    int                 SvcVoiceGetInfo(WPARAM, LPARAM);
    int                 SvcVoiceCall(HANDLE hContact, LPARAM);
    int                 SvcVoiceAnswerCall(const char* id, LPARAM);
    int                 SvcVoiceDropCall(const char* id, LPARAM);
    int                 SvcVoiceHoldCall(const char* id, LPARAM);
    bool                SvcVoiceCallStringValid(const wchar_t* str, LPARAM);
    bool                SvcVoiceCallContactValid(HANDLE hContact, bool canCallRightNow);

private:
    CRtcClient&         m_client;
};
//--------------------------------------------------------------------------------------------------

inline CCallServices::CCallServices(CRtcClient& client) :
    m_client(client)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline int CCallServices::SvcAcceptCall(WPARAM, LPARAM)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(SIPRTC_CALL_INCOMING == SvcGetCallState(0, 0));

    int res = E_FAIL;

    if(SIPRTC_CALL_INCOMING == SvcGetCallState(0, 0))
    {
        try
        {
            m_client.AcceptAVSession();

            res = S_OK;
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to accept the AV session: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int CCallServices::SvcRejectCall(WPARAM, LPARAM)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(SIPRTC_CALL_INCOMING == SvcGetCallState(0, 0));

    int res = E_FAIL;

    if(SIPRTC_CALL_INCOMING == SvcGetCallState(0, 0))
    {
        try
        {
            m_client.RejectAVSession();

            res = S_OK;
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to reject the AV session: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int CCallServices::SvcCall(WPARAM, SIPRTC_CALL* call)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(call);
    MTLASSERT(call->uri || call->hContact);
    MTLASSERT(SIPRTC_CALL_DISCONNECTED == SvcGetCallState(0, 0));

    g_env.Trace().WriteVerbose(L"SvcCall: Current AV state: %d", SvcGetCallState(0, 0));

    int res = E_FAIL;

    if(SIPRTC_CALL_DISCONNECTED == SvcGetCallState(0, 0) && call && sizeof(*call) == call->cbSize
        && (call->uri || call->hContact))
    {
        HANDLE hContact = call->hContact;
        bstr_t uri(call->uri);
        bstr_t name(L"");

        if(0 == uri.length())
        {
            if(hContact)
            {
                uri  = g_env.DB().GetContactSettingWString(hContact, "uri");
            }
        }
        else
        {
            hContact = g_env.DB().FindContact(uri);
        }

        if(uri.length() > 0)
        {
            bstr_t name = hContact ? g_env.DB().GetContactSettingWString(hContact, "Nick") : L"";

            try
            {
                ISessionController* controller = new CAVSessionController();

                m_client.EstablishAVSession(controller, uri, name);

                res = S_OK;
            }
            catch(_com_error& e)
            {
                g_env.Trace().WriteError(L"Failed to establish the AV session: %X (%s)", e.Error(),
                    static_cast<const wchar_t*>(e.Description()));
            }
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int CCallServices::SvcTerminateCall(WPARAM, LPARAM)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(SIPRTC_CALL_DISCONNECTED != SvcGetCallState(0, 0));

    int res = E_FAIL;

    if(SIPRTC_CALL_DISCONNECTED != SvcGetCallState(0, 0))
    {
        try
        {
            m_client.TerminateAVSession();

            res = S_OK;
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to hang up: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int CCallServices::SvcGetCallState(WPARAM, LPARAM)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());

    int res = SIPRTC_CALL_DISCONNECTED;

    try
    {
        res = RtcSessionStateToMSessionState(m_client.GetAVSessionState());
    }
    catch(_com_error& e)
    {
        g_env.Trace().WriteError(L"Failed to get AV session status: %X (%s)", e.Error(),
            static_cast<const wchar_t*>(e.Description()));
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int CCallServices::SvcVoiceGetInfo(WPARAM, LPARAM)
{
    g_env.Trace().WriteVerbose(L"SvcVoiceGetInfo called");

     return VOICE_SUPPORTED | VOICE_CALL_CONTACT | VOICE_CALL_STRING | 
        VOICE_CALL_CONTACT_NEED_TEST | VOICE_CAN_SET_DEVICE | VOICE_CAN_HOLD;
}
//--------------------------------------------------------------------------------------------------

inline int CCallServices::SvcVoiceCall(HANDLE hContact, LPARAM)
{
    g_env.Trace().WriteVerbose(L"SvcVoiceCall called");

    SIPRTC_CALL call = { 0 };
    call.cbSize = sizeof(call);
    call.hContact = hContact;

    return SvcCall(0, &call);
}
//--------------------------------------------------------------------------------------------------

inline int CCallServices::SvcVoiceAnswerCall(const char* id, LPARAM)
{
    g_env.Trace().WriteVerbose(L"SvcVoiceAnswerCall called");
    return SvcAcceptCall(0, 0);
}
//--------------------------------------------------------------------------------------------------

inline int CCallServices::SvcVoiceDropCall(const char* id, LPARAM)
{
    g_env.Trace().WriteVerbose(L"SvcVoiceDropCall called");
    return SvcRejectCall(0, 0);
}
//--------------------------------------------------------------------------------------------------

inline int CCallServices::SvcVoiceHoldCall(const char* id, LPARAM)
{
    g_env.Trace().WriteVerbose(L"SvcVoiceHoldCall called");

    return 1; // TODO
}
//--------------------------------------------------------------------------------------------------

inline bool CCallServices::SvcVoiceCallStringValid(const wchar_t* str, LPARAM)
{
    g_env.Trace().WriteVerbose(L"SvcVoiceCallStringValid(%s) called", str ? str : L"(null)");

    return 0 != str && str[0]; // TODO
}
//--------------------------------------------------------------------------------------------------

inline bool CCallServices::SvcVoiceCallContactValid(HANDLE hContact, bool canCallRightNow)
{
    g_env.Trace().WriteVerbose(L"SvcVoiceCallContactValid(%d,%d) called", hContact, canCallRightNow);
    
    bool valid = !canCallRightNow || SIPRTC_CALL_DISCONNECTED == SvcGetCallState(0, 0);
    
    if(valid)
    {
        bstr_t uri  = g_env.DB().GetContactSettingWString(hContact, "uri");
        valid = uri.length() > 0;
        g_env.Trace().WriteVerbose(L"  Contact uri %s, can be called: %d", static_cast<const wchar_t*>(uri), valid);
    }
    else
        g_env.Trace().WriteVerbose(L"  Cannot call any contacts now");

    return valid;
}
//--------------------------------------------------------------------------------------------------

