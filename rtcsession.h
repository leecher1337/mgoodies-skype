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

#include <vector>
#include <algorithm>
#include "rtchelper.h"

#define SESSION_TTL_SECONDS (5 * 60) // 5 minutes
//--------------------------------------------------------------------------------------------------

class CRtcSession
{
public:
                        CRtcSession(IRTCSessionPtr session = 0);

    bool                IsValid(void) const;

    void                Accept(void);
    void                Reject(void);

    void                SendIMMessage(const bstr_t& message, long cookie);
    void                SendUserIsTyping(bool isTyping);

    void                StartApplicationSharing(void);
    void                StartWhiteboard(void);

    void                Terminate(void);

    bstr_t              GetParticipantURI(void);
    bstr_t              GetParticipantName(void);
    bool                ContainsActiveParticipant(const bstr_t& uri);

    RTC_SESSION_TYPE    GetType(void) const;
    RTC_SESSION_STATE   GetState(void) const;

    bstr_t              ToString(void) const;

    bool                operator== (const CRtcSession& other);

private:
    void                StartT120Applet(RTC_T120_APPLET applet);

private:
    IRTCSessionPtr      m_session;
};
//--------------------------------------------------------------------------------------------------

class ISessionController
{
public:
    virtual             ~ISessionController(void) {};

    virtual void        SetSession(CRtcSession& session) {};

    virtual void        OnStateChanged(RTC_SESSION_STATE state) {};
    virtual void        OnParticipantStateChanged(IRTCParticipantPtr participant,
                            RTC_PARTICIPANT_STATE state) {};
};
//--------------------------------------------------------------------------------------------------

class CRtcActiveSessions
{
public:
    CRtcSession         Add(ISessionController* controller, IRTCSessionPtr session);
    void                Remove(IRTCSessionPtr session);

    bool                FindByParticipant(const bstr_t& uri, RTC_SESSION_TYPE sessionType,
                            CRtcSession** pSession);
    bool                FindAVSession(CRtcSession** pSession);

    bool                IsAVSessionInProgress(void) const;
    RTC_SESSION_STATE   GetAVSessionState(void) const;

    void                TerminateAllSessions(void);
    void                Cleanup(void);

    void                SessionStateChanged(IRTCSessionPtr session, RTC_SESSION_STATE state);
    void                ParticipantStateChanged(IRTCSessionPtr session, IRTCParticipantPtr participant,
                            RTC_PARTICIPANT_STATE state);

private:
    ISessionController* FindController(IRTCSessionPtr session);
    void                CollectExpiredSessions(void);

private:
    struct SessionInfo
    {
        ISessionController* controller;
        CRtcSession         session;
        __int64             lastTimeUsed;
        bool                terminating;
    };


    typedef std::vector<SessionInfo> sessions_t;

    sessions_t          m_sessions;
};
//--------------------------------------------------------------------------------------------------

inline CRtcSession::CRtcSession(IRTCSessionPtr session) :
    m_session(session)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcSession::IsValid(void) const
{
    return 0 != m_session;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcSession::Accept(void)
{
    MTLASSERT(0 != m_session);

    RTC_SESSION_STATE state = RTCSS_DISCONNECTED;
    ComCheck(m_session->get_State(&state));
    MTLASSERT(RTCSS_INCOMING == state);

    ComCheck(m_session->Answer());
}
//--------------------------------------------------------------------------------------------------

inline void CRtcSession::Reject(void)
{
    MTLASSERT(0 != m_session);

    ComCheck(m_session->Terminate(RTCTR_REJECT));
}
//--------------------------------------------------------------------------------------------------

inline void CRtcSession::SendIMMessage(const bstr_t& message, long cookie)
{
    MTLASSERT(0 != m_session);
    MTLASSERT(message.length() > 0);

    ComCheck(m_session->SendMessage(bstr_t(L"text/plain"), message, cookie));
}
//--------------------------------------------------------------------------------------------------

inline void CRtcSession::SendUserIsTyping(bool isTyping)
{
    MTLASSERT(0 != m_session);

    ComCheck(m_session->SendMessageStatus(isTyping ? RTCMUS_TYPING : RTCMUS_IDLE, 0));
}
//--------------------------------------------------------------------------------------------------

inline void CRtcSession::StartApplicationSharing(void)
{
    MTLASSERT(RTCST_PC_TO_PC == GetType());
    StartT120Applet(RTCTA_APPSHARING);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcSession::StartWhiteboard(void)
{
    MTLASSERT(RTCST_PC_TO_PC == GetType());
    StartT120Applet(RTCTA_WHITEBOARD);
}
//--------------------------------------------------------------------------------------------------


inline void CRtcSession::Terminate(void)
{
    MTLASSERT(0 != m_session);

    ComCheck(m_session->Terminate(RTCTR_SHUTDOWN));
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CRtcSession::GetParticipantURI(void)
{
    MTLASSERT(0 != m_session);

    bstr_t res = L"";

    IRTCEnumParticipantsPtr pEnum;
    ComCheck(m_session->EnumerateParticipants(&pEnum));

    IRTCParticipantPtr pParticipant;
    if(S_OK == pEnum->Next(1, &pParticipant, NULL))
    {
        HRESULT hr = pParticipant->get_UserURI(res.GetAddress());
        if(FAILED(hr))
        {
            g_env.Trace().WriteError(L"Failed to retrieve the participant's URI, hr=%X", hr);
            res = L"";
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CRtcSession::GetParticipantName(void)
{
    MTLASSERT(0 != m_session);

    bstr_t res = L"";

    IRTCEnumParticipantsPtr pEnum;
    ComCheck(m_session->EnumerateParticipants(&pEnum));

    IRTCParticipantPtr pParticipant;
    if(S_OK == pEnum->Next(1, &pParticipant, NULL))
    {
        if(FAILED(pParticipant->get_Name(res.GetAddress())) || 0 == res.length())
            res = L"";
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcSession::ContainsActiveParticipant(const bstr_t& participantUri)
{
    MTLASSERT(participantUri.length() > 0);

    if(0 == participantUri.length())
        return false;

    bool found = false;

    IRTCEnumParticipantsPtr pEnum;
    ComCheck(m_session->EnumerateParticipants(&pEnum));

    IRTCParticipantPtr pParticipant;
    while(S_OK == pEnum->Next(1, &pParticipant, NULL))
    {
        bstr_t uri;
        ComCheck(pParticipant->get_UserURI(uri.GetAddress()));
        if(UrisAreEqual(participantUri, uri))
        {
            RTC_PARTICIPANT_STATE state = RTCPS_DISCONNECTED;
            ComCheck(pParticipant->get_State(&state));
            found = RTCPS_DISCONNECTED != state;
            break;
        }
    }

    return found;
}
//--------------------------------------------------------------------------------------------------

inline RTC_SESSION_TYPE CRtcSession::GetType(void) const
{
    MTLASSERT(0 != m_session);
    RTC_SESSION_TYPE type = RTCST_MULTIPARTY_IM;
    ComCheck(m_session->get_Type(&type));
    return type;
}
//--------------------------------------------------------------------------------------------------

inline RTC_SESSION_STATE CRtcSession::GetState(void) const
{
    MTLASSERT(0 != m_session);
    RTC_SESSION_STATE state = RTCSS_DISCONNECTED;
    ComCheck(m_session->get_State(&state));
    return state;
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CRtcSession::ToString(void) const
{
    MTLASSERT(0 != m_session);

    bstr_t res(L"");

    if(0 != m_session)
    {
        wchar_t buffer[1024];
        StringCbPrintf(buffer, sizeof(buffer), L"{Session %X", m_session.GetInterfacePtr());
        res += buffer;

        RTC_SESSION_TYPE type = RTCST_MULTIPARTY_IM;
        HRESULT hr = m_session->get_Type(&type);
        if(SUCCEEDED(hr))
            StringCbPrintf(buffer, sizeof(buffer), L",type %d", type);
        else
            StringCbPrintf(buffer, sizeof(buffer), L",type failed %X", hr);
        res += buffer;

        RTC_SESSION_STATE state = RTCSS_DISCONNECTED;
        hr = m_session->get_State(&state);
        if(SUCCEEDED(hr))
            StringCbPrintf(buffer, sizeof(buffer), L",state %d", state);
        else
            StringCbPrintf(buffer, sizeof(buffer), L",state failed %X", hr);
        res += buffer;

        IRTCEnumParticipantsPtr pEnum;
        hr = m_session->EnumerateParticipants(&pEnum);
        if(SUCCEEDED(hr))
        {
            res += L",buddies:";

            unsigned count = 0;
            IRTCParticipantPtr pParticipant;
            while(S_OK == pEnum->Next(1, &pParticipant, NULL))
            {
                bstr_t uri;
                hr = pParticipant->get_UserURI(uri.GetAddress());
                if(SUCCEEDED(hr))
                    StringCbPrintf(buffer, sizeof(buffer), L" %d. %s ", count, static_cast<const wchar_t*>(uri));
                else
                    StringCbPrintf(buffer, sizeof(buffer), L" %d. uri failed %X ", count, hr);
                res += buffer;

                RTC_PARTICIPANT_STATE pstate = RTCPS_DISCONNECTED;
                hr = pParticipant->get_State(&pstate);
                if(SUCCEEDED(hr))
                    StringCbPrintf(buffer, sizeof(buffer), L" (%d)", pstate);
                else
                    StringCbPrintf(buffer, sizeof(buffer), L" (state failed %X)", hr);
                res += buffer;

                ++count;
            }
        }
        else
        {
            StringCbPrintf(buffer, sizeof(buffer), L",buddies failed %X", hr);
            res += buffer;
        }

        res += L"}";
    }
    else
        res = L"{Null session}";

    return res;
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcSession::operator== (const CRtcSession& other)
{
    return m_session == other.m_session;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcSession::StartT120Applet(RTC_T120_APPLET applet)
{
    IRTCClientPtr rtcClient;
    ComCheck(m_session->get_Client(&rtcClient));

    VARIANT_BOOL appletRunning = FALSE;
    ComCheck(rtcClient->get_IsT120AppletRunning(applet, &appletRunning));

    if(!appletRunning)
    {
        ComCheck(rtcClient->StartT120Applet(applet));
    }
}
//--------------------------------------------------------------------------------------------------


inline CRtcSession CRtcActiveSessions::Add(ISessionController* controller, IRTCSessionPtr pSession)
{
    CollectExpiredSessions();

    g_env.Trace().WriteVerbose(L"Storing new session");

    MTLASSERT(0 != pSession);

    RTC_SESSION_TYPE enType;
    ComCheck(pSession->get_Type(&enType));

    // Is this an audio/video session?
    BOOL fAVSession = (RTCST_PC_TO_PC == enType || RTCST_PC_TO_PHONE == enType);

    CRtcSession session(pSession);

    g_env.Trace().WriteVerbose(static_cast<const wchar_t*>(session.ToString()));

    if(controller)
        controller->SetSession(session);

    SessionInfo si = { controller, session, GetCurrentTimestamp(), false };

    m_sessions.push_back(si);

    return si.session;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcActiveSessions::Remove(IRTCSessionPtr pSession)
{
    g_env.Trace().WriteVerbose(L"Removing disconnected session");

    MTLASSERT(0 != pSession);

    for(sessions_t::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
    {
        if(CRtcSession(pSession) == i->session)
        {
            g_env.Trace().WriteVerbose(static_cast<const wchar_t*>(i->session.ToString()));

            if(i->controller)
            {
                delete i->controller;
                i->controller = 0;
            }

            m_sessions.erase(i);
            g_env.Trace().WriteVerbose(L"Session removed");
            break;
        }
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcActiveSessions::CollectExpiredSessions(void)
{
    if(!m_sessions.empty())
    {
        g_env.Trace().WriteVerbose(L"Collecting expired sessions...");

        __int64 now = GetCurrentTimestamp();

        for(sessions_t::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
        {
            __int64 ageInSeconds = (now - i->lastTimeUsed) / (1000 * 1000 * 10);

            if(ageInSeconds > SESSION_TTL_SECONDS && !i->terminating)
            {
                g_env.Trace().WriteVerbose(L"Session expired. Age %d secs. Terminating. %s",
                    (int)ageInSeconds,
                    static_cast<const wchar_t*>(i->session.ToString()));

                i->terminating = true;

                try
                {
                    i->session.Terminate();
                }
                catch(_com_error& e)
                {
                    g_env.Trace().WriteVerbose(L"Session Terminate() failed with %X.", e.Error());
                }
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcActiveSessions::FindByParticipant(const bstr_t& uri, RTC_SESSION_TYPE sessionType,
    CRtcSession** pSession)
{
    g_env.Trace().WriteVerbose(L"Searching for an active session with '%s' of type %d",
        static_cast<const wchar_t*>(uri), sessionType);

    MTLASSERT(uri.length() > 0);
    MTLASSERT(0 != pSession);

    *pSession = 0;

    CollectExpiredSessions();

    if(uri.length() > 0)
    {
        for(sessions_t::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
        {
            if(i->terminating)
                continue;

            RTC_SESSION_TYPE activeSessionType = i->session.GetType();

            g_env.Trace().WriteVerbose(L"Active session %s", static_cast<const wchar_t*>(i->session.ToString()));

            if(activeSessionType == sessionType && i->session.ContainsActiveParticipant(uri))
            {
                g_env.Trace().WriteVerbose(L"Coincidence!");
                *pSession = &i->session;
                i->lastTimeUsed = GetCurrentTimestamp();
                break;
            }
        }
    }

    if(0 == *pSession)
        g_env.Trace().WriteVerbose(L"Active session wasn't found");

    return 0 != *pSession;
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcActiveSessions::FindAVSession(CRtcSession** pSession)
{
    MTLASSERT(0 != pSession);

    *pSession = 0;

    for(sessions_t::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
    {
        if(i->terminating)
            continue;

        if(RTCST_PC_TO_PC == i->session.GetType() || RTCST_PC_TO_PHONE == i->session.GetType())
        {
            *pSession = &i->session;
            i->lastTimeUsed = GetCurrentTimestamp();
            break;
        }
    }

    return 0 != *pSession;
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcActiveSessions::IsAVSessionInProgress(void) const
{
    bool res = false;

    for(sessions_t::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
    {
        if(RTCST_PC_TO_PC == i->session.GetType() || RTCST_PC_TO_PHONE == i->session.GetType())
        {
            res = true;
            break;
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline RTC_SESSION_STATE CRtcActiveSessions::GetAVSessionState(void) const
{
    RTC_SESSION_STATE res = RTCSS_DISCONNECTED;

    for(sessions_t::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
    {
        if(RTCST_PC_TO_PC == i->session.GetType() || RTCST_PC_TO_PHONE == i->session.GetType())
        {
            res = i->session.GetState();
            break;
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcActiveSessions::TerminateAllSessions(void)
{
    g_env.Trace().WriteVerbose(L"Terminating all active sessions");

    for(sessions_t::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
    {
        try
        {
            i->session.Terminate();
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteVerbose(L"Session Terminate() failed with %X.", e.Error());
        }
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcActiveSessions::Cleanup(void)
{
    g_env.Trace().WriteVerbose(L"Removing all active sessions");

    for(sessions_t::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
    {
        if(i->controller)
        {
            delete i->controller;
            i->controller = 0;
        }

        try
        {
            i->session.Terminate();
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteVerbose(L"Session Terminate() failed with %X.", e.Error());
        }
    }

    m_sessions.clear();
}
//--------------------------------------------------------------------------------------------------

inline ISessionController* CRtcActiveSessions::FindController(IRTCSessionPtr session)
{
    MTLASSERT(0 != session);

    ISessionController* res = 0;

    for(sessions_t::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
    {
        if(CRtcSession(session) == i->session)
        {
            res = i->controller;
            break;
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcActiveSessions::SessionStateChanged(IRTCSessionPtr session, RTC_SESSION_STATE state)
{
    ISessionController* controller = FindController(session);
    if(controller)
        controller->OnStateChanged(state);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcActiveSessions::ParticipantStateChanged(IRTCSessionPtr session,
    IRTCParticipantPtr participant, RTC_PARTICIPANT_STATE state)
{
    ISessionController* controller = FindController(session);
    if(controller)
        controller->OnParticipantStateChanged(participant, state);
}
//--------------------------------------------------------------------------------------------------
