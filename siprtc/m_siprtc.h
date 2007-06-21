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

#ifndef __INCLUDE_M_SIPRTC_H__
#define __INCLUDE_M_SIPRTC_H__
//--------------------------------------------------------------------------------------------------

#define SIPRTC_CALL_IDLE            0   // The session is idle.
#define SIPRTC_CALL_INCOMING        1   // The session has an incoming call.
#define SIPRTC_CALL_ANSWERING       2   // The session is answering an incoming session.
#define SIPRTC_CALL_INPROGRESS      3   // The session is in progress.
#define SIPRTC_CALL_CONNECTED       4   // The session is connected.
#define SIPRTC_CALL_DISCONNECTED    5   // The session is disconnected.
#define SIPRTC_CALL_HOLD            6   // The call is on hold.
#define SIPRTC_CALL_REFER           7   // The session has been referred to another user.
//--------------------------------------------------------------------------------------------------

/*
    Service: Returns currect call state.
    WPARAM - 0
    LPARAM - 0
    Return value: SIPRTC_CALL_XXXX constant.

    When state changes, the ME_SIPRTC_CALL_STATE_CHANGED event is fired.
*/
#define MS_SIPRTC_GET_CALL_STATE "/Services/GetCallState"
//--------------------------------------------------------------------------------------------------

struct SIPRTC_CALL
{
    unsigned        cbSize;             // sizeof SIPRTC_CALL
    const wchar_t*  uri;                // SIP URI or phone number of the calling/called party
    HANDLE          hContact;           // CList contact's handle (can be NULL)
};
//--------------------------------------------------------------------------------------------------

/*
    Event: the state of the call was changed.
    WPARAM - call state
    LPARAM - points to the SIPRTC_CALL structure.
    Return value: ignored

    If state is SIPRTC_CALL_INCOMING, use MS_SIPRTC_ACCEPT_CALL or MS_SIPRTC_REJECT_CALL services
    to accept/reject the call.
*/
#define ME_SIPRTC_CALL_STATE_CHANGED "/Events/CallStateChanged"
//--------------------------------------------------------------------------------------------------

/*
    Service: Accept an incomming call.
    WPARAM - 0
    LPARAM - 0
    Return value: 0 on success
*/
#define MS_SIPRTC_ACCEPT_CALL   "/Services/AcceptCall"
//--------------------------------------------------------------------------------------------------

/*
    Service: Reject an incomming call.
    WPARAM - 0
    LPARAM - 0
    Return value: 0 on success

    Notifies via ME_SIPRTC_CALL_STATE_CHANGED when the call is terminated.
*/
#define MS_SIPRTC_REJECT_CALL   "/Services/RejectCall"
//--------------------------------------------------------------------------------------------------

/*
    Service: Initiate a call.
    WPARAM - 0
    LPARAM - points to the SIPRTC_CALL structure. uri or hContact must be set.
    Return value: 0 on success

    Notifies via ME_SIPRTC_CALL_STATE_CHANGED about the result.
*/
#define MS_SIPRTC_CALL "/Services/Call"
//--------------------------------------------------------------------------------------------------

/*
    Service: Terminate the call.
    WPARAM - 0
    LPARAM - 0
    Return value: 0 on success

    Notifies via ME_SIPRTC_CALL_STATE_CHANGED when the call is terminated.
*/
#define MS_SIPRTC_TERMINATE_CALL "/Services/TerminateCall"
//--------------------------------------------------------------------------------------------------


#endif // __INCLUDE_M_SIPRTC_H__
//--------------------------------------------------------------------------------------------------
