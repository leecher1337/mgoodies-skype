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
#include "avsessioncontroller.h"
#include "m_siprtc.h"
#include "m_utils.h"
#include "rtchelper.h"
#include "SDK/m_voice.h"
//--------------------------------------------------------------------------------------------------

//
// Events
//
HANDLE CAVSessionController::m_hCallStateChanged = 0;
HANDLE CAVSessionController::m_hVoiceStateEvent = 0;
//--------------------------------------------------------------------------------------------------

CAVSessionController::CAVSessionController(void)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

void CAVSessionController::Initialize(void)
{
    MTLASSERT(0 == m_hCallStateChanged);

    char eventName[300];
    mir_snprintf(eventName, sizeof(eventName), "%s%s", g_env.ProtocolName(), ME_SIPRTC_CALL_STATE_CHANGED);
    m_hCallStateChanged = CreateHookableEvent(eventName);
    mir_snprintf(eventName, sizeof(eventName), "%s%s", g_env.ProtocolName(), PE_VOICE_CALL_STATE);
    m_hVoiceStateEvent = CreateHookableEvent(eventName);
}
//--------------------------------------------------------------------------------------------------

void CAVSessionController::SetSession(CRtcSession& session)
{
    m_session = session;
}
//--------------------------------------------------------------------------------------------------

void CAVSessionController::FireEvent(unsigned state)
{
    MTLASSERT(m_session.IsValid());

    SIPRTC_CALL call = { 0 };
    call.cbSize = sizeof(call);
    call.uri = m_session.GetParticipantURI();
    if(m_session.GetParticipantURI().length() > 0)
        call.hContact = g_env.DB().FindOrAddContact(m_session.GetParticipantURI(),
                            m_session.GetParticipantName(), true);

    MTLASSERT(m_hCallStateChanged);
    NotifyEventHooks(m_hCallStateChanged, state, (LPARAM)&call);

    VOICE_CALL voiceCall = { 0 };
    voiceCall.cbSize = sizeof(voiceCall);
    voiceCall.szModule = g_env.ProtocolName();
    voiceCall.id = "TODO";
    voiceCall.flags = VOICE_CALL_CONTACT;
    voiceCall.hContact = call.hContact;

    switch(state)
    {
    case SIPRTC_CALL_IDLE:          voiceCall.state = VOICE_STATE_ON_HOLD; break; // ???
    case SIPRTC_CALL_INCOMING:      voiceCall.state = VOICE_STATE_RINGING; break;
    case SIPRTC_CALL_ANSWERING:     voiceCall.state = VOICE_STATE_RINGING; break; // ???
    case SIPRTC_CALL_INPROGRESS:    voiceCall.state = VOICE_STATE_TALKING; break;
    case SIPRTC_CALL_CONNECTED:     voiceCall.state = VOICE_STATE_TALKING; break; // ???
    case SIPRTC_CALL_DISCONNECTED:  voiceCall.state = VOICE_STATE_ENDED; break;
    case SIPRTC_CALL_HOLD:          voiceCall.state = VOICE_STATE_ON_HOLD; break;
    case SIPRTC_CALL_REFER:         voiceCall.state = VOICE_STATE_ENDED; break; // ???
    }

    NotifyEventHooks(m_hVoiceStateEvent, (WPARAM)&voiceCall, 0);
}
//--------------------------------------------------------------------------------------------------

void CAVSessionController::OnStateChanged(RTC_SESSION_STATE state)
{
    MTLASSERT(m_session.IsValid());

    FireEvent(RtcSessionStateToMSessionState(state));
}
//--------------------------------------------------------------------------------------------------

void CAVSessionController::OnParticipantStateChanged(IRTCParticipantPtr participant,
    RTC_PARTICIPANT_STATE state)
{
    MTLASSERT(m_session.IsValid());

    if(RTCPS_CONNECTED == state)
    {
    }
}
//--------------------------------------------------------------------------------------------------
