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
//--------------------------------------------------------------------------------------------------

class CAVSessionController : public ISessionController
{
public:
                        CAVSessionController(void);

    static void         Initialize(void);

    virtual void        SetSession(CRtcSession& session);

    virtual void        OnStateChanged(RTC_SESSION_STATE state);
    virtual void        OnParticipantStateChanged(IRTCParticipantPtr participant,
                            RTC_PARTICIPANT_STATE state);

private:
    // Non-copyable
                        CAVSessionController(const CAVSessionController&);
    CAVSessionController& operator= (const CAVSessionController&);

    void                FireEvent(unsigned state);

private:
    CRtcSession         m_session;

    //
    // Events
    //
    static HANDLE       m_hCallStateChanged;
    static HANDLE       m_hVoiceStateEvent;
};
//--------------------------------------------------------------------------------------------------
