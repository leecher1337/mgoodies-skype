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

#include "rtcsession.h"
#include "ui/acceptT120.h"
//--------------------------------------------------------------------------------------------------

class CT120SessionController : public ISessionController
{
public:
    explicit            CT120SessionController(RTC_T120_APPLET appletType = RTCTA_WHITEBOARD);
    virtual             ~CT120SessionController(void);

    virtual void        SetSession(CRtcSession& session);

    virtual void        OnStateChanged(RTC_SESSION_STATE state);
    virtual void        OnParticipantStateChanged(IRTCParticipantPtr participant,
                            RTC_PARTICIPANT_STATE state);

private:
    // Non-copyable
                        CT120SessionController(const CT120SessionController&);
    CT120SessionController& operator= (const CT120SessionController&);

private:
    RTC_T120_APPLET     m_appletType;
    CRtcSession         m_session;
    std::auto_ptr<CAcceptT120Dialog> m_acceptDialog;
};
//--------------------------------------------------------------------------------------------------

inline CT120SessionController::CT120SessionController(RTC_T120_APPLET appletType) :
    m_appletType(appletType)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline CT120SessionController::~CT120SessionController(void)
{
    if(m_acceptDialog.get())
    {
        m_acceptDialog->Close();
    }
}
//--------------------------------------------------------------------------------------------------

inline void CT120SessionController::SetSession(CRtcSession& session)
{
    m_session = session;
}
//--------------------------------------------------------------------------------------------------

inline void CT120SessionController::OnStateChanged(RTC_SESSION_STATE state)
{
    MTLASSERT(m_session.IsValid());

    if(RTCSS_INCOMING == state)
    {
        try
        {
            if(!m_acceptDialog.get())
                m_acceptDialog.reset(new CAcceptT120Dialog());

            m_acceptDialog->Show(m_session.GetParticipantName(), &m_session);
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to get participant info: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }
    else
        if(m_acceptDialog.get())
           m_acceptDialog->Close();
}
//--------------------------------------------------------------------------------------------------

inline void CT120SessionController::OnParticipantStateChanged(IRTCParticipantPtr participant,
    RTC_PARTICIPANT_STATE state)
{
    MTLASSERT(m_session.IsValid());

    if(RTCPS_CONNECTED == state)
    {
        // The participant has connected to the session. Start T120 applet

        try
        {
            switch(m_appletType)
            {
            case RTCTA_WHITEBOARD:
                m_session.StartWhiteboard();
                break;
            case RTCTA_APPSHARING:
                m_session.StartApplicationSharing();
                break;
            default:
                MTLASSERT(!"Unknown applet type!");
            }
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to launch T120 applet: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }
}
//--------------------------------------------------------------------------------------------------
