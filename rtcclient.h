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

#include <algorithm>
#include <vector>
#include <map>
#include "RtcEvents.h"
#include "rtcsession.h"
#include "rtchelper.h"
#include "search.h"
//--------------------------------------------------------------------------------------------------

class IRtcObserver
{
public:
    virtual bool        GetLoginInfo(bstr_t& uri, bstr_t& server, bstr_t& transport) = 0;
    virtual bool        GetAuthInfo(const bstr_t& realm, bstr_t& uri, bstr_t& account,
                            bstr_t& password, int connectionAttempt) = 0;

    virtual void        UpdateBuddyList(IRTCBuddy* pBuddy) = 0;
    virtual void        ClearBuddyList(IRTCBuddy* pBuddy) = 0;
    virtual void        ClearBuddyList(void) = 0;

    virtual void        UpdateWatcher(const bstr_t& uri, RTC_WATCHER_STATE state) = 0;
    virtual bool        GetWatcherPermission(const bstr_t& uri, const bstr_t& name) = 0;

    virtual void        OnLoginError(const wchar_t* message, HRESULT hr) = 0;
    virtual void        OnError(const wchar_t* message) = 0;
    virtual void        OnError(const _com_error& e, const char* source) = 0;

    virtual void        OnPresenceStatusChange(RTC_PRESENCE_STATUS oldStatus, RTC_PRESENCE_STATUS newStatus) = 0;
    virtual void        OnLoggingOn(void) = 0;
    virtual void        OnLoggedOn(IRTCProfile* profile) = 0;
    virtual void        OnLoggingOff(void) = 0;
    virtual void        OnLoggedOff(void) = 0;
    virtual void        OnRefreshBuddy(const bstr_t& uri) = 0;
    virtual void        OnSearchStarted(const std::vector<SearchTerm>& terms, long cookie) = 0;

    virtual void        OnSessionOperationComplete(long cookie, long status, const bstr_t& statusText) = 0;

    virtual void        DeliverMessage(IRTCParticipant* pParticipant, const bstr_t& contentType,
                            const bstr_t& message) = 0;
    virtual void        DeliverUserStatus(IRTCParticipant* pParticipant,
                            RTC_MESSAGING_USER_STATUS status) = 0;

    virtual void        DeliverUserSearchResult(long cookie, HRESULT status, const bstr_t& uri,
                            const bstr_t& nick, const bstr_t& email) = 0;

    virtual void        DeliverMedia(long mediaType, RTC_MEDIA_EVENT_TYPE type,
                            RTC_MEDIA_EVENT_REASON reason) = 0;

    virtual ISessionController* GetSessionController(RTC_SESSION_TYPE sessionType) = 0;
};
//--------------------------------------------------------------------------------------------------

class CRtcClient
{
public:
                        CRtcClient(IRtcObserver* observer);
                        ~CRtcClient(void);

    bool                Initialize(const bstr_t& mirandaVersion, const bstr_t& profilePath);
    void                PrepareForShutdown(void);
    void                Terminate(void);

    bool                IsLoggedOn(void) const;
    RTC_REGISTRATION_STATE GetRegistrationState(void) const;
    RTC_PRESENCE_STATUS GetPresenceStatus(void) const;

    void                LogOff(void);
    void                SetPresenceStatus(RTC_PRESENCE_STATUS enStatus);
    void                SetPresenceNotes(RTC_PRESENCE_STATUS enStatus, const bstr_t& notes);

    void                SendIMMessage(const bstr_t& uri, const bstr_t& name, const bstr_t& message,
                            long cookie = 0);
    void                SendUserIsTyping(const bstr_t& uri, bool isTyping);

    void                EstablishPcToPcSession(ISessionController* controller, const bstr_t& uri,
                            const bstr_t& name);
    void                EstablishAVSession(ISessionController* controller, const bstr_t& uri,
                            const bstr_t& name);
    void                TerminateSession(const bstr_t& uri);
    void                TerminateAVSession(void);
    void                AcceptAVSession(void);
    void                RejectAVSession(void);

    bool                IsAVSessionInProgress(void) const;
    RTC_SESSION_STATE   GetAVSessionState(void) const;

    void                StartApplicationSharing(const bstr_t& uri, const bstr_t& name);
    void                StartWhiteboard(const bstr_t& uri, const bstr_t& name);

    void                SearchContact(const std::vector<SearchTerm>& terms, long cookie);

    void                AddBuddy(const bstr_t& uri, const bstr_t& name, bool temporary = false);
    void                RemoveBuddy(const bstr_t& uri);
    void                RefreshBuddy(const bstr_t& uri);

    void                AddGroup(const bstr_t& name);
    void                RemoveGroup(const bstr_t& name);
    void                AddBuddyToGroup(const bstr_t& group, const bstr_t& buddyUri);
    void                RemoveBuddyFromGroup(const bstr_t& group, const bstr_t& buddyUri);

    void                AuthorizeWatcher(const bstr_t& uri, bool allow);

private:
    void                CreateEventWindow(void);
    static LRESULT CALLBACK EventWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void                DoReportError(const wchar_t* message);

    // Logon
    void                DoLogOn(void);
    void                OnLoggingOn(void);
    void                OnLoggedOn(void);

    // Logoff
    void                DoLogOff(void);
    void                OnLoggingOff(void);
    void                OnLoggedOff(void);

    // Authentication
    void                DoAuth(const bstr_t& uri, const bstr_t& account, const bstr_t& password);

    // Enable presence
    void                DoEnablePresence(bool enable);

    void                DoSetPresence(RTC_PRESENCE_STATUS enStatus);

    // In-band provisioning
    void                DoGetProfile(const bstr_t& uri, const bstr_t& server, const bstr_t& transport);

    // Enable profile
    void                DoEnableProfile(bool enable, long lRegisterFlags, long lRoamingFlags);

    // Register with server
    void                DoRegister(void);

    void                ConfigureAllowedAuthentication(void);

    void                PopulateBuddyList(void);
    void                PopulateWatcherList(void);

    CRtcSession         DoCall(ISessionController* controller, RTC_SESSION_TYPE enType,
                            const bstr_t& uri, const bstr_t& name);

    void                OnRTCEvent(RTC_EVENT type, IDispatch* pEvent);

    //
    // RTC event handlers
    //
    void                OnRTCRegistrationStateChangeEvent(IRTCRegistrationStateChangeEvent* pEvent);
    void                OnRTCSessionStateChangeEvent(IRTCSessionStateChangeEvent* pEvent);
    void                OnRTCSessionOperationCompleteEvent(IRTCSessionOperationCompleteEvent* pEvent);
    void                OnRTCParticipantStateChangeEvent(IRTCParticipantStateChangeEvent* pEvent);
    void                OnRTCMessagingEvent(IRTCMessagingEvent* pEvent);
    void                OnRTCMediaEvent(IRTCMediaEvent* pEvent);
    void                OnRTCIntensityEvent(IRTCIntensityEvent* pEvent);
    void                OnRTCClientEvent(IRTCClientEvent* pEvent);
    void                OnRTCBuddyEvent(IRTCBuddyEvent2* pEvent);
    void                OnRTCBuddyGroupEvent(IRTCBuddyGroupEvent* pEvent);
    void                OnRTCWatcherEvent(IRTCWatcherEvent2* pEvent);
    void                OnRTCUserSearchResultsEvent(IRTCUserSearchResultsEvent* pEvent);
    void                OnRTCRoamingEvent(IRTCRoamingEvent* pEvent);
    void                OnRTCProfileEvent(IRTCProfileEvent2* pEvent);
    void                OnRTCPresencePropertyEvent(IRTCPresencePropertyEvent* pEvent);
    void                OnRTCPresenceDataEvent(IRTCPresenceDataEvent* pEvent);
    void                OnRTCPresenceStatusEvent(IRTCPresenceStatusEvent* pEvent);
    void                OnRTCMediaRequestEvent(IRTCMediaRequestEvent* pEvent);

private:
    IRtcObserver*       m_observer;
    IRTCClient2Ptr      m_rtcClient;
    IRTCProfile2Ptr     m_rtcProfile;
    bstr_t              m_mirandaVersion;
    bstr_t              m_profilePath;
    RTC_REGISTRATION_STATE m_registrationState;
    RTC_PRESENCE_STATUS m_presenceStatus;
    CRtcEvents*         m_pEvents;
    HWND                m_hEventWnd;
    bool                m_fPresenceEnabled;
    RTC_PRESENCE_STATUS m_statusToSet;
    unsigned            m_connectionAttempt;

    CRtcActiveSessions  m_sessions;

    IRTCUserSearchPtr   m_userSearch;

    typedef std::map<RTC_PRESENCE_STATUS, bstr_t> statusnotes_t;
    statusnotes_t       m_statusNotes;
};
//--------------------------------------------------------------------------------------------------

inline CRtcClient::CRtcClient(IRtcObserver* observer) :
    m_observer(observer),
    m_registrationState(RTCRS_NOT_REGISTERED),
    m_presenceStatus(RTCXS_PRESENCE_OFFLINE),
    m_pEvents(0),
    m_hEventWnd(0),
    m_fPresenceEnabled(false),
    m_statusToSet(RTCXS_PRESENCE_OFFLINE),
    m_connectionAttempt(0)
{
    MTLASSERT(m_observer);
}
//--------------------------------------------------------------------------------------------------

inline CRtcClient::~CRtcClient(void)
{
    try
    {
        Terminate();
    }
    catch(...)
    {
        // dtors must never throw exceptions
    }
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcClient::Initialize(const bstr_t& mirandaVersion, const bstr_t& profilePath)
{
    m_mirandaVersion = mirandaVersion;
    m_profilePath = profilePath;

    if(m_profilePath.length() > 0)
    {
        m_profilePath += L"\\";
    }

    bool res = false;

    IRTCClient2Ptr rtcClient;
    HRESULT hr = rtcClient.CreateInstance(__uuidof(RTCClient));
    if(FAILED(hr))
    {
        g_env.Trace().WriteError(L"Failed to create RTC client, %X", hr);
        return false;
    }

    long rtcVersion = 0;
    ComCheck(rtcClient->get_Version(&rtcVersion));
    g_env.Trace().WriteVerbose(L"RTC version %X", rtcVersion);

    if(rtcVersion < 0x00010003)
    {
        g_env.Trace().WriteError(L"Unsupported RTC version %X", rtcVersion);
        return false;
    }

    wchar_t rtcVersionText[MAX_PATH] = { 0 };
    StringCbPrintfW(rtcVersionText, sizeof(rtcVersionText), L" (RTC %d.%d)", HIWORD(rtcVersion), LOWORD(rtcVersion));
    m_mirandaVersion += rtcVersionText;

    ComCheck(rtcClient->Initialize());

    long lFlags = RTCEF_REGISTRATION_STATE_CHANGE |
                  RTCEF_SESSION_STATE_CHANGE |
                  RTCEF_SESSION_OPERATION_COMPLETE |
                  RTCEF_PARTICIPANT_STATE_CHANGE |
                  RTCEF_MESSAGING |
                  RTCEF_MEDIA |
                  RTCEF_INTENSITY |
                  RTCEF_CLIENT |
                  RTCEF_BUDDY |
                  RTCEF_BUDDY2 |
                  RTCEF_WATCHER |
                  RTCEF_WATCHER2 |
                  RTCEF_GROUP |
                  RTCEF_USERSEARCH |
                  RTCEF_ROAMING |
                  RTCEF_PROFILE |
                  RTCEF_PRESENCE_PROPERTY |
                  RTCEF_PRESENCE_DATA |
                  RTCEF_PRESENCE_STATUS |
                  RTCEF_MEDIA_REQUEST;

    // Set the event filter for the RTC client
    ComCheck(rtcClient->put_EventFilter(lFlags));

    // Set the listen mode for RTC client
    // RTCLM_BOTH opens the standard SIP port 5060, as well as
    // a dynamic port.
    ComCheck(rtcClient->put_AllowedPorts(RTCTR_TCP, RTCLM_BOTH));
    ComCheck(rtcClient->put_AllowedPorts(RTCTR_UDP, RTCLM_BOTH));

    // Answer Mode Calls
    ComCheck(rtcClient->put_AnswerMode(RTCST_PC_TO_PC, RTCAM_OFFER_SESSION_EVENT));
    ComCheck(rtcClient->put_AnswerMode(RTCST_IM, RTCAM_AUTOMATICALLY_ACCEPT));
    ComCheck(rtcClient->put_AnswerMode(RTCST_MULTIPARTY_IM, RTCAM_AUTOMATICALLY_ACCEPT));
    ComCheck(rtcClient->put_AnswerMode(RTCST_APPLICATION, RTCAM_OFFER_SESSION_EVENT));
    ComCheck(rtcClient->put_AnswerMode(RTCST_PC_TO_PHONE, RTCAM_AUTOMATICALLY_ACCEPT));//RTCAM_AUTOMATICALLY_REJECT));
    ComCheck(rtcClient->put_AnswerMode(RTCST_PHONE_TO_PHONE, RTCAM_AUTOMATICALLY_ACCEPT));//RTCAM_AUTOMATICALLY_REJECT));

    CreateEventWindow();

    m_pEvents = new CRtcEvents;

    ComCheck(m_pEvents->Advise(rtcClient, m_hEventWnd));

    ComCheck(rtcClient->put_ClientName(bstr_t(L"Miranda IM")));
    ComCheck(rtcClient->put_ClientCurVer(m_mirandaVersion));

    long caps = 0;
    if(SUCCEEDED(rtcClient->get_MediaCapabilities(&caps)))
    {
        g_env.Trace().WriteVerbose(L"Media capabilities: %X", caps);
    }

    long mediaType = 0;
    ComCheck(rtcClient->get_PreferredMediaTypes(&mediaType));
    // Enable T120
    ComCheck(rtcClient->SetPreferredMediaTypes(mediaType |
             RTCMT_T120_SENDRECV | RTCMT_AUDIO_RECEIVE | RTCMT_AUDIO_SEND, // todo
             VARIANT_TRUE));

    m_rtcClient = rtcClient;

    return true;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::PrepareForShutdown(void)
{
    g_env.Trace().WriteVerbose(L"CRtcClient::PrepareForShutdown");

    // Cleanup the existing session windows
    m_sessions.Cleanup();

    // Cleanup the user search windows
    //CleanupUserSearches();

    // Cleanup the watcher windows
    //CleanupWatchers();

    if(m_rtcClient)
    {
        //ComCheck(m_rtcClient->StopT120Applets());

        // Prepare the RTC client object for shutdown
        ComCheck(m_rtcClient->PrepareForShutdown());
    }

    g_env.Trace().WriteVerbose(L"CRtcClient::PrepareForShutdown finished");
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::Terminate(void)
{
    g_env.Trace().WriteVerbose(L"CRtcClient::Terminate");

    m_rtcProfile = 0;

    // Cleanup the buddy list
    m_observer->ClearBuddyList();

    if(m_rtcClient)
    {
        if(m_pEvents)
        {
            // Unadvise for events from the RTC client
            m_pEvents->Unadvise(m_rtcClient);
            m_pEvents = 0;
        }

        // Shutdown the RTC client
        m_rtcClient->Shutdown();
    }

    if(m_hEventWnd)
    {
        ::DestroyWindow(m_hEventWnd);
        m_hEventWnd = 0;
    }

    m_rtcClient = 0;

    g_env.Trace().WriteVerbose(L"CRtcClient::Terminate finished");
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcClient::IsLoggedOn(void) const
{
    return RTCRS_REGISTERED == m_registrationState;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCEvent(RTC_EVENT type, IDispatch* pDisp)
{
    MTLASSERT(pDisp);

    // Based on the RTC_EVENT type, query for the
    // appropriate event interface and call a helper
    // method to handle the event

    switch(type)
    {
    case RTCE_REGISTRATION_STATE_CHANGE:
        {
            IRTCRegistrationStateChangeEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCRegistrationStateChangeEvent),
                (void**)&event)))
            {
                OnRTCRegistrationStateChangeEvent(event);
            }
        }
        break;

    case RTCE_SESSION_STATE_CHANGE:
        {
            IRTCSessionStateChangeEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCSessionStateChangeEvent),
                (void**)&event)))
            {
                OnRTCSessionStateChangeEvent(event);
            }
        }
        break;

    case RTCE_SESSION_OPERATION_COMPLETE:
        {
            IRTCSessionOperationCompleteEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCSessionOperationCompleteEvent),
                (void**)&event)))
            {
                OnRTCSessionOperationCompleteEvent(event);
            }
        }
        break;

    case RTCE_PARTICIPANT_STATE_CHANGE:
        {
            IRTCParticipantStateChangeEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCParticipantStateChangeEvent),
                (void**)&event)))
            {
                OnRTCParticipantStateChangeEvent(event);
            }
        }
        break;

    case RTCE_MESSAGING:
        {
            IRTCMessagingEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCMessagingEvent),
                (void**)&event)))
            {
                OnRTCMessagingEvent(event);
            }
        }
        break;

    case RTCE_MEDIA:
        {
            IRTCMediaEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCMediaEvent), (void**)&event)))
            {
                OnRTCMediaEvent(event);
            }
        }
        break;

    case RTCE_MEDIA_REQUEST:
        {
            IRTCMediaRequestEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCMediaRequestEvent),
                (void**)&event)))
            {
                OnRTCMediaRequestEvent(event);
            }
        }
        break;

    case RTCE_INTENSITY:
        {
            IRTCIntensityEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCIntensityEvent),
                (void**)&event)))
            {
                OnRTCIntensityEvent(event);
            }
        }
        break;

    case RTCE_CLIENT:
        {
            IRTCClientEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCClientEvent), (void**)&event)))
            {
                OnRTCClientEvent(event);
            }
        }
        break;

    case RTCE_BUDDY:
        {
            IRTCBuddyEvent2Ptr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCBuddyEvent2),
                (void**)&event)))
            {
                OnRTCBuddyEvent(event);
            }
        }
        break;

    case RTCE_WATCHER:
        {
            IRTCWatcherEvent2Ptr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCWatcherEvent2),
                (void**)&event)))
            {
                OnRTCWatcherEvent(event);
            }
        }
        break;

    case RTCE_GROUP:
        {
            IRTCBuddyGroupEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCBuddyGroupEvent),
                (void**)&event)))
            {
                OnRTCBuddyGroupEvent(event);
            }
        }
        break;

    case RTCE_USERSEARCH:
        {
            IRTCUserSearchResultsEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCUserSearchResultsEvent),
                (void**)&event)))
            {
                OnRTCUserSearchResultsEvent(event);
            }
        }
        break;

    case RTCE_ROAMING:
        {
            IRTCRoamingEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCRoamingEvent),
                (void**)&event)))
            {
                OnRTCRoamingEvent(event);
            }
        }
        break;

    case RTCE_PROFILE:
        {
            IRTCProfileEvent2Ptr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCProfileEvent2),
                (void**)&event)))
            {
                OnRTCProfileEvent(event);
            }
        }
        break;

    case RTCE_PRESENCE_PROPERTY:
        {
            IRTCPresencePropertyEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCPresencePropertyEvent),
                (void**)&event)))
            {
                OnRTCPresencePropertyEvent(event);
            }
        }
        break;

    case RTCE_PRESENCE_DATA:
        {
            IRTCPresenceDataEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCPresenceDataEvent),
                (void**)&event)))
            {
                OnRTCPresenceDataEvent(event);
            }
        }
        break;

    case RTCE_PRESENCE_STATUS:
        {
            IRTCPresenceStatusEventPtr event;

            if(SUCCEEDED(pDisp->QueryInterface(__uuidof(IRTCPresenceStatusEvent),
                (void**)&event)))
            {
                OnRTCPresenceStatusEvent(event);
            }
        }
        break;

    default:
        g_env.Trace().WriteVerbose(L"Unimplemented RTC_EVENT %d!", type);
    }

    // Release the event
    if(pDisp)
        pDisp->Release();
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::CreateEventWindow(void)
{
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC)&CRtcClient::EventWindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hIcon         = 0;
    wc.hCursor       = 0;
    wc.lpszClassName = _T("SipRtcEvent");

    ::RegisterClass(&wc);

    m_hEventWnd = CreateWindowExW(
                0,
                _T("SipRtcEvent"),
                _T("SipRtcEvent"),
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                0, 0,
                NULL,
                NULL,
                GetModuleHandle(0),
                NULL);

    SetWindowLongPtr(m_hEventWnd, GWLP_USERDATA, (LONG_PTR)this);

    MTLASSERT(m_hEventWnd);
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CALLBACK CRtcClient::EventWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT res = 0;

    if(WM_RTC_EVENT == uMsg)
    {
        CRtcClient* pThis = (CRtcClient*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        MTLASSERT(pThis);

        if(pThis)
        {
            try
            {
                pThis->OnRTCEvent((RTC_EVENT)wParam, (IDispatch*)lParam);
            }
            catch(_com_error& e)
            {
                g_env.Trace().WriteError(L"Event processing error: %X, '%s'", e.Error(),
                    static_cast<const wchar_t*>(e.Description()));
                pThis->m_observer->OnError(e, __FUNCTION__);
            }
        }
    }
    else
        res = DefWindowProc(hWnd, uMsg, wParam, lParam);

    return res;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCRegistrationStateChangeEvent(IRTCRegistrationStateChangeEvent* pEvent)
{
    // This event occurs when our registration state has changed. For example, this may occur
    // when we EnableProfileEx(), or we are successfully registered, or logged off from the server.

    // Get the registration state

    g_env.Trace().WriteVerbose(L"OnRTCRegistrationStateChangeEvent event");

    RTC_REGISTRATION_STATE enState;
    ComCheck(pEvent->get_State(&enState));

    // Get the status code
    long lStatusCode;
    ComCheck(pEvent->get_StatusCode(&lStatusCode));

    HRESULT hr = lStatusCode;

    g_env.Trace().WriteVerbose(L"State: %d (%s), Status code: %X", enState,
        (const wchar_t*)RtcRegistrationStateToString(enState), lStatusCode);


    RTC_REGISTRATION_STATE oldState = m_registrationState;
    m_registrationState = enState;

    switch(enState)
    {
    case RTCRS_UNREGISTERING:
        // Logoff in progress
        OnLoggingOff();
        break;

    case RTCRS_NOT_REGISTERED:
        // Logged off
        OnLoggedOff();
        break;

    case RTCRS_REGISTERING:
        // Logon in progress
        OnLoggingOn();
        break;

    case RTCRS_REGISTERED:
        // Logged on
        OnLoggedOn();
        break;

    case RTCRS_REJECTED:
    case RTCRS_ERROR:
        {
            g_env.Trace().WriteError(L"Logon failed. Most likely the server could no be found, or the user needs to authenticate.");

            // Logon failed. Most likely the server could no be found,
            // or the user needs to authenticate.

            // Check if we need to authenticate
            if (m_connectionAttempt < 3 &&
                ((RTC_E_STATUS_CLIENT_FORBIDDEN == hr) ||
                (RTC_E_STATUS_CLIENT_UNAUTHORIZED == hr) ||
                (RTC_E_STATUS_CLIENT_PROXY_AUTHENTICATION_REQUIRED == hr)))
            {
                ++m_connectionAttempt;

                g_env.Trace().WriteVerbose(L"The user needs to authenticate");

                bstr_t realm;

                hr = m_rtcProfile->get_Realm(realm.GetAddress());

                if (FAILED(hr))
                {
                    g_env.Trace().WriteError(L"get_Realm failed %x", hr);
                }

                bstr_t uri;
                bstr_t account;
                bstr_t password;

                // Display the authentication dialog
                MTLASSERT(m_observer);

                if(!m_observer->GetAuthInfo(realm, uri, account, password, m_connectionAttempt))
                {
                    DoLogOff();
                    OnLoggedOff();

                    //DoReportError(L"Logon failed!");
                    return;
                }

                // Do the authentication
                try
                {
                    DoAuth(uri, account, password);
                }
                catch(_com_error& e)
                {
                    g_env.Trace().WriteError(L"Authentication failed: %X, %s", e.Error(),
                        static_cast<const wchar_t*>(e.Description()));

                    DoLogOff();
                    OnLoggedOff();

                    m_observer->OnLoginError(L"Authentication failed", e.Error());
                }
            }
            else
            {
                // Logon failed
                DoLogOff();
                OnLoggedOff();

                // If we were logging on the show error
                if(RTCRS_REGISTERING == oldState)
                {
                    m_observer->OnLoginError(L"Logon failed", hr);
                }
            }
        }
        break;

    case RTCRS_LOGGED_OFF:
        g_env.Trace().WriteVerbose(L"RTCRS_LOGGED_OFF %x", hr);
        // The user logged on at another client
        // The user is logged off from this client
        DoReportError(L"The Server has logged you off (Perhaps you logged in from another location)");
        DoLogOff();
        OnLoggedOff();
        break;

    case RTCRS_LOCAL_PA_LOGGED_OFF:
        g_env.Trace().WriteVerbose(L"RTCRS_LOCAL_PA_LOGGED_OFF %x", hr);
        g_env.Trace().WriteWarning(L"The user logged on at another client. The user's presence state is no longer sent from this client.");

        m_observer->OnLoginError(L"The user logged on at another client. The user's presence state is no longer sent from this client.",
            RTC_E_REGISTRATION_UNREGISTERED);

        // The user logged on at another client
        // The user's presence state is no longer sent from this client

        //SetStatusText(L"Logged on (Presence disabled)");
        break;

    case RTCRS_REMOTE_PA_LOGGED_OFF:
        g_env.Trace().WriteVerbose(L"RTCRS_REMOTE_PA_LOGGED_OFF %x", hr);
        // The user logged off on another client that was sending his
        // presence state. We can ignore this.
        break;
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCSessionStateChangeEvent(IRTCSessionStateChangeEvent* pEvent)
{
    // This event occurs when our session state has changed. For example, this may occur
    // when we AddParticipant() to a Session, or we Terminate() a session.

    g_env.Trace().WriteVerbose(L"OnRTCSessionStateChangeEvent event");

    RTC_SESSION_STATE enState;
    ComCheck(pEvent->get_State(&enState));

    g_env.Trace().WriteVerbose(L"State: %d (%s)", enState,
        (const wchar_t*)RtcSessionStateToString(enState));

    IRTCSessionPtr pSession;
    ComCheck(pEvent->get_Session(&pSession));


    switch(enState)
    {
    case RTCSS_INCOMING:
        {
            // This is a new session
            RTC_SESSION_TYPE enType;
            ComCheck(pSession->get_Type(&enType));

            g_env.Trace().WriteVerbose(L"Incoming session, type: %d", enType);


            if(RTCST_PC_TO_PC == enType || RTCST_PC_TO_PHONE == enType)
            {
                g_env.Trace().WriteVerbose(L"AV call");

                /*
                // This is an AV call
                if (CRTCAVSession::m_Singleton != NULL)
                {
                    // If another AV call is in progress, then
                    // we are already busy.
                    pSession->Terminate(RTCTR_BUSY);

                    SAFE_RELEASE(pSession);
                    return;
                }

                // Get the participant object
                IRTCEnumParticipants * pEnum = NULL;
                IRTCParticipant * pParticipant = NULL;

                hr = pSession->EnumerateParticipants(&pEnum);

                if (FAILED(hr))
                {
                    // EnumerateParticipants failed
                    SAFE_RELEASE(pSession);
                    return;
                }

                hr = pEnum->Next(1, &pParticipant, NULL);

                SAFE_RELEASE(pEnum);

                if (hr != S_OK)
                {
                    // Next failed
                    SAFE_RELEASE(pSession);
                    return;
                }

                // Get the participant URI
                BSTR bstrURI = NULL;

                hr = pParticipant->get_UserURI(&bstrURI);

                if (FAILED(hr))
                {
                    // get_UserURI failed
                    SAFE_RELEASE(pSession);
                    SAFE_RELEASE(pParticipant);
                    return;
                }

                // Get the participant name
                BSTR bstrName = NULL;

                hr = pParticipant->get_Name(&bstrName);

                SAFE_RELEASE(pParticipant);

                if (FAILED(hr) && (hr != RTC_E_NOT_EXIST))
                {
                    // get_Name failed
                    SAFE_FREE_STRING(bstrURI);
                    SAFE_RELEASE(pSession);
                    return;
                }

                // Ring the bell
                m_pClient->PlayRing(RTCRT_PHONE, VARIANT_TRUE);

                // Show the session dialog
                BOOL fAccept;

                hr = ShowSessionDialog(m_hWnd, bstrName, bstrURI, &fAccept);

                SAFE_FREE_STRING(bstrURI);
                SAFE_FREE_STRING(bstrName);

                if (FAILED(hr))
                {
                    // ShowSessionDialog failed
                    SAFE_RELEASE(pSession);
                    return;
                }

                if (fAccept)
                {
                    // Accept the session
                    hr = pSession->Answer();

                    if (FAILED(hr))
                    {
                        // Answer failed
                        SAFE_RELEASE(pSession);
                        return;
                    }
                }
                else
                {
                    // Reject the session
                    pSession->Terminate(RTCTR_REJECT);

                    SAFE_RELEASE(pSession);
                    return;
                }
                */
            }
            else
            {
                // This is an IM call
                g_env.Trace().WriteVerbose(L"IM call");
            }

            // Add the session to the session list
            m_sessions.Add(m_observer->GetSessionController(enType), pSession);
        }
        break;

    case RTCSS_DISCONNECTED:
        {
            g_env.Trace().WriteVerbose(L"Session disconnected");

            m_sessions.SessionStateChanged(pSession, enState);

            m_sessions.Remove(pSession);
        }
        break;
    }

    if(RTCSS_DISCONNECTED != enState)
    {
        // Deliver the session state to the session list
        m_sessions.SessionStateChanged(pSession, enState);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCSessionOperationCompleteEvent(IRTCSessionOperationCompleteEvent* pEvent)
{
    // A session operation has completed

    g_env.Trace().WriteVerbose(L"OnRTCSessionStateChangeEvent event");

    LONG_PTR cookie = 0;
    ComCheck(pEvent->get_Cookie(&cookie));
    g_env.Trace().WriteVerbose(L"Cookie: %d", cookie);

    LONG status = S_OK;
    ComCheck(pEvent->get_StatusCode(&status));
    g_env.Trace().WriteVerbose(L"Status: %X", status);

    bstr_t text;
    if(FAILED(pEvent->get_StatusText(text.GetAddress())))
    {
        text = L"";
    }
    g_env.Trace().WriteVerbose(L"Status text: %s", static_cast<const wchar_t*>(text));

    IRTCSessionPtr pSession;
    ComCheck(pEvent->get_Session(&pSession));

    // Is this session in our session list?
    //hr = FindSession(pSession, &pSessWindow);

    m_observer->OnSessionOperationComplete(static_cast<long>(cookie), status, text);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCParticipantStateChangeEvent(IRTCParticipantStateChangeEvent* pEvent)
{
    // This event occurs when our registration state has changed. For example, this may occur
    // when we AddParticipant() or RemoveParticipant() on a Session, or a Participant leaves a Session.

    g_env.Trace().WriteVerbose(L"OnRTCParticipantStateChangeEvent event");

    IRTCParticipantPtr pParticipant;
    ComCheck(pEvent->get_Participant(&pParticipant));

    bstr_t uri;
    ComCheck(pParticipant->get_UserURI(uri.GetAddress()));
    g_env.Trace().WriteVerbose(L"Participant: <%s>", static_cast<const wchar_t*>(uri));

    IRTCSessionPtr pSession;
    ComCheck(pParticipant->get_Session(&pSession));

    //hr = FindSession(pSession, &pSessWindow);

    // Get the participant state
    RTC_PARTICIPANT_STATE enState;
    ComCheck(pEvent->get_State(&enState));

    g_env.Trace().WriteVerbose(L"Participant state: %d (%s)", enState,
        (const wchar_t*)RtcParticipantStateToString(enState));

    if(RTCPS_DISCONNECTED != enState)
    {
        //
    }
    else
    {
        //
    }

    // Deliver the participant state to the session list
    m_sessions.ParticipantStateChanged(pSession, pParticipant, enState);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCMessagingEvent(IRTCMessagingEvent* pEvent)
{
    g_env.Trace().WriteVerbose(L"OnRTCMessagingEvent event");

    // This event occurs when we receive a message. This may be a either an instant message
    // or a message status message.

    IRTCSessionPtr pSession;
    ComCheck(pEvent->get_Session(&pSession));

    RTC_MESSAGING_EVENT_TYPE enType;
    ComCheck(pEvent->get_EventType(&enType));

    g_env.Trace().WriteVerbose(L"Type: %d", enType);

    IRTCParticipantPtr pParticipant;
    ComCheck(pEvent->get_Participant(&pParticipant));

    bstr_t uri;
    ComCheck(pParticipant->get_UserURI(uri.GetAddress()));
    g_env.Trace().WriteVerbose(L"Participant: <%s>", static_cast<const wchar_t*>(uri));

    if(RTCMSET_MESSAGE == enType)
    {
        bstr_t contentType;
        ComCheck(pEvent->get_MessageHeader(contentType.GetAddress()));

        g_env.Trace().WriteVerbose(L"Content type: <%s>", static_cast<const wchar_t*>(contentType));

        bstr_t message;
        ComCheck(pEvent->get_Message(message.GetAddress()));

        g_env.Trace().WriteVerbose(L"Message: <%s>", static_cast<const wchar_t*>(message));

        // Deliver the message
        m_observer->DeliverMessage(pParticipant, contentType, message);
    }
    else if(RTCMSET_STATUS == enType)
    {
        RTC_MESSAGING_USER_STATUS enStatus;
        ComCheck(pEvent->get_UserStatus(&enStatus));

        g_env.Trace().WriteVerbose(L"Typing Status: %d", enStatus);

        // Deliver the user status to the session window
        m_observer->DeliverUserStatus(pParticipant, enStatus);
    }
    else
        MTLASSERT(false);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCMediaEvent(IRTCMediaEvent* pEvent)
{
    g_env.Trace().WriteVerbose(L"OnRTCMediaEvent event");

    // This event occurs when our media status changes. For example, this may occur
    // when we Add or Remove audio, video, or T120 streams.

    long lMediaType;
    ComCheck(pEvent->get_MediaType(&lMediaType));

    RTC_MEDIA_EVENT_TYPE enType;
    ComCheck(pEvent->get_EventType(&enType));

    RTC_MEDIA_EVENT_REASON enReason;
    ComCheck(pEvent->get_EventReason(&enReason));

    g_env.Trace().WriteVerbose(L"Media type %d, event type %d, reason %d",
        lMediaType, enType, enReason);

    m_observer->DeliverMedia(lMediaType, enType, enReason);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCIntensityEvent(IRTCIntensityEvent* pEvent)
{
    //g_env.Trace().WriteVerbose(L"OnRTCIntensityEvent event");
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCClientEvent(IRTCClientEvent* pEvent)
{
    g_env.Trace().WriteVerbose(L"OnRTCClientEvent event");
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCBuddyEvent(IRTCBuddyEvent2* pEvent)
{
    // This event occurs when our buddy's state is changed, the subscription state changes,
    // a buddy is added/removed, a buddy's attributes are updated, or a buddy is roamed.

    g_env.Trace().WriteVerbose(L"OnRTCBuddyEvent event");

    RTC_BUDDY_EVENT_TYPE enType;
    ComCheck(pEvent->get_EventType(&enType));

    // Get the status
    long lStatus = S_OK;
    ComCheck(pEvent->get_StatusCode(&lStatus));

    g_env.Trace().WriteVerbose(L"Event type: %d, Status code: %X", enType, lStatus);

    // Get the buddy object
    IRTCBuddyPtr pBuddy;

    ComCheck(pEvent->get_Buddy(&pBuddy));

    MTLASSERT(m_observer);

    switch(enType)
    {
    case RTCBET_BUDDY_ADD:
        {
            g_env.Trace().WriteVerbose(L"RTCBET_BUDDY_ADD %x", lStatus);

            if(SUCCEEDED(lStatus))
            {
                // Update the buddy list entry
                m_observer->UpdateBuddyList(pBuddy);
            }
            else
            {
                // Delete the buddy from the list
                m_observer->ClearBuddyList(pBuddy);
            }
        }
        break;

    case RTCBET_BUDDY_REMOVE:
        {
            g_env.Trace().WriteVerbose(L"RTCBET_BUDDY_REMOVE %x", lStatus);

            if(SUCCEEDED(lStatus))
            {
                // Delete the buddy from the list
                m_observer->ClearBuddyList(pBuddy);
            }
            else
            {
                // Update the buddy list entry
                m_observer->UpdateBuddyList(pBuddy);
            }
        }
        break;

    case RTCBET_BUDDY_UPDATE:
        {
            g_env.Trace().WriteVerbose(L"RTCBET_BUDDY_UPDATE %x", lStatus);

            // Update the buddy list entry
            m_observer->UpdateBuddyList(pBuddy);
        }
        break;

    case RTCBET_BUDDY_SUBSCRIBED:
        {
            if(FAILED(lStatus))
                m_observer->UpdateBuddyList(pBuddy);
        }
        break;


    case RTCBET_BUDDY_STATE_CHANGE:
        {
            g_env.Trace().WriteVerbose(L"RTCBET_BUDDY_STATE_CHANGE %x", lStatus);

            // Update the buddy list entry
            m_observer->UpdateBuddyList(pBuddy);
        }
        break;
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCBuddyGroupEvent(IRTCBuddyGroupEvent* pEvent)
{
    g_env.Trace().WriteVerbose(L"OnRTCBuddyGroupEvent event");
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCWatcherEvent(IRTCWatcherEvent2* pEvent)
{
    g_env.Trace().WriteVerbose(L"OnRTCWatcherEvent event");

    // This event occurs when our watcher's state is changed,
    // a watcher is added/removed, a watcher's attributes are updated, or a watcher is roamed.

    RTC_WATCHER_EVENT_TYPE enType;
    ComCheck(pEvent->get_EventType(&enType));

    // Get the status
    long lStatus = E_FAIL;
    ComCheck(pEvent->get_StatusCode(&lStatus));

    g_env.Trace().WriteVerbose(L"Event type: %d, Status code: %X", enType, lStatus);

    // Get the watcher object
    IRTCWatcherPtr pWatcher;
    ComCheck(pEvent->get_Watcher(&pWatcher));

    // Get the watcher URI
    bstr_t uri;
    ComCheck(pWatcher->get_PresentityURI(uri.GetAddress()));

    g_env.Trace().WriteVerbose(L"Watcher URI %s", static_cast<const wchar_t*>(uri));

    // Get the watcher name
    bstr_t name;
    if(FAILED(pWatcher->get_Name(name.GetAddress())))
    {
        name = L"";
    }

    switch(enType)
    {
    case RTCWET_WATCHER_ADD:
        g_env.Trace().WriteVerbose(L"RTCWET_WATCHER_ADD");
        break;

    case RTCWET_WATCHER_REMOVE:
        g_env.Trace().WriteVerbose(L"RTCWET_WATCHER_REMOVE");
        break;

    case RTCWET_WATCHER_UPDATE:
        g_env.Trace().WriteVerbose(L"RTCWET_WATCHER_UPDATE");
        break;

    case RTCWET_WATCHER_OFFERING:
        {
            g_env.Trace().WriteVerbose(L"RTCWET_WATCHER_OFFERING");

            // Show the incoming watcher dialog
            bool allow = m_observer->GetWatcherPermission(uri, name);

            // Set the watcher to be allowed or blocked
            ComCheck(pWatcher->put_State(allow ? RTCWS_ALLOWED : RTCWS_BLOCKED));
        }
        break;
    }

    RTC_WATCHER_STATE state = RTCWS_UNKNOWN;
    ComCheck(pWatcher->get_State(&state));

    // Update the watcher entry
    m_observer->UpdateWatcher(uri, state);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCUserSearchResultsEvent(IRTCUserSearchResultsEvent* pEvent)
{
    g_env.Trace().WriteVerbose(L"OnRTCUserSearchResultsEvent event");

    MTLASSERT(m_userSearch.GetInterfacePtr());

    // Get the search cookie
    LONG_PTR cookie = 0;
    ComCheck(pEvent->get_Cookie(&cookie));
    g_env.Trace().WriteVerbose(L"Cookie: %d", cookie);

    // Get the search status
    long status = S_OK;
    ComCheck(pEvent->get_StatusCode(&status));
    g_env.Trace().WriteVerbose(L"Status: %X", status);

    // Deliver the participant state
    if(SUCCEEDED(status))
    {
        // Get the search results
        IRTCEnumUserSearchResultsPtr pEnum;
        ComCheck(pEvent->EnumerateResults(&pEnum));

        IRTCUserSearchResultPtr result;

        while(S_OK == pEnum->Next(1, &result, 0))
        {
            bstr_t uri;
            bstr_t nick;
            bstr_t email;

            ComCheck(result->get_Value(RTCUSC_URI, uri.GetAddress()));
            MTLASSERT(uri.length() > 0);

            if(FAILED(result->get_Value(RTCUSC_DISPLAYNAME, nick.GetAddress())))
            {
                nick = L"";
            }

            if(FAILED(result->get_Value(RTCUSC_EMAIL, email.GetAddress())))
            {
                email = L"";
            }

            if(uri.length() > 0)
            {
                m_observer->DeliverUserSearchResult(static_cast<long>(cookie), (HRESULT)status, uri, nick, email);
            }

            result = 0;
        }

        m_observer->DeliverUserSearchResult(static_cast<long>(cookie), (HRESULT)status, L"", L"", L"");
    }
    else
    {
        m_observer->DeliverUserSearchResult(static_cast<long>(cookie), (HRESULT)status, L"", L"", L"");
    }

    m_userSearch = 0;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCRoamingEvent(IRTCRoamingEvent* pEvent)
{
    g_env.Trace().WriteVerbose(L"OnRTCRoamingEvent event");
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCProfileEvent(IRTCProfileEvent2* pEvent)
{
    // This event occurs when our profile is created or updated.
    // A profile may be created with GetProfile(). A profile may be updated from the server
    // if profile roaming is requested, and the server supports the feature.

    g_env.Trace().WriteVerbose(L"OnRTCProfileEvent event");

    RTC_PROFILE_EVENT_TYPE eventType;
    ComCheck(pEvent->get_EventType(&eventType));

    long statusCode = 0;
    ComCheck(pEvent->get_StatusCode(&statusCode));

    g_env.Trace().WriteVerbose(L"Event type: %d, Status code: %X", eventType, statusCode);

    HRESULT hr = statusCode;

    switch(eventType)
    {
    case RTCPFET_PROFILE_GET:
        {
            if(FAILED(hr))
            {
                g_env.Trace().WriteError(L"Provisioning failed %x", hr);

                // Provisioning failed.
                DoLogOff();
                OnLoggedOff();

                m_observer->OnLoginError(L"Logon failed. Provisioning failed.", hr);
                return;
            }
            else
            {
                // Provisioning was successful.
                // Get the RTC profile object from the event
                IRTCProfilePtr profile;

                hr = pEvent->get_Profile(&profile);
                if(SUCCEEDED(hr))
                {
                   hr = profile->QueryInterface(__uuidof(IRTCProfile2), (void**)&m_rtcProfile);
                }
                else
                   hr = E_NOINTERFACE;

                if(FAILED(hr))
                {
                    g_env.Trace().WriteError(L"Failed to get profile %x", hr);

                    DoLogOff();
                    OnLoggedOff();

                    m_observer->OnLoginError(L"Logon failed. Can't get profile.", hr);
                    return;
                }

                if(CSipRtcTrace::Verbose == g_env.Trace().GetLevel())
                {
                    bstr_t value;
                    if(SUCCEEDED(profile->get_ProviderName(value.GetAddress())))
                        g_env.Trace().WriteVerbose(L"  Provider of profile: %s",
                            static_cast<const wchar_t*>(value));

                    if(SUCCEEDED(profile->get_XML(value.GetAddress())))
                        g_env.Trace().WriteVerbose(L"  Profile XML: %s",
                            static_cast<const wchar_t*>(value));

                    IRTCProfile3Ptr profile3;
                    if(SUCCEEDED(profile->QueryInterface(__uuidof(IRTCProfile3), (void**)&profile3)))
                    {
                        LONG transport = 0;
                        if(SUCCEEDED(profile3->GetServer(value.GetAddress(), &transport)))
                            g_env.Trace().WriteVerbose(L"  Server: %s, transport: %d",
                                static_cast<const wchar_t*>(value), transport);
                    }
                }

                ConfigureAllowedAuthentication();

                // Register the profile
                try
                {
                    DoRegister();
                }
                catch(_com_error& e)
                {
                    g_env.Trace().WriteError(L"DoRegister failed %X, %s", e.Error(),
                        static_cast<const wchar_t*>(e.Description()));

                    // DoEnableRoaming failed
                    DoLogOff();
                    OnLoggedOff();

                    m_observer->OnLoginError(L"Logon failed. Can't enable presence or register the profile.",
                        e.Error());

                    return;
                }
            }
        }
        break;

    case RTCPFET_PROFILE_UPDATE:
        // Ignore updates
        break;
   }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCPresencePropertyEvent(IRTCPresencePropertyEvent* pEvent)
{
    g_env.Trace().WriteVerbose(L"OnRTCPresencePropertyEvent event");
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCPresenceDataEvent(IRTCPresenceDataEvent* pEvent)
{
    g_env.Trace().WriteVerbose(L"OnRTCPresenceDataEvent event");
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCPresenceStatusEvent(IRTCPresenceStatusEvent* pEvent)
{
    // This event occurs when our device's presence status is updated.
    // (the status code indicates whether the operation was successful).

    g_env.Trace().WriteVerbose(L"OnRTCPresenceStatusEvent event");

    long lStatusCode;

    HRESULT hr = pEvent->get_StatusCode(&lStatusCode);

    if (FAILED(hr))
    {
        // get_StatusCode failed
        g_env.Trace().WriteError(L"get_StatusCode failed %x", hr);
        return;
    }

    bstr_t notes;
    RTC_PRESENCE_STATUS enStatus;

    hr = pEvent->GetLocalPresenceInfo(&enStatus, notes.GetAddress());

    if(FAILED(hr) && (hr != RTC_E_NOT_EXIST))
    {
        // GetLocalPresenceInfo failed
        g_env.Trace().WriteError(L"GetLocalPresenceInfo failed %x", hr);
        return;
    }

    g_env.Trace().WriteVerbose(L"Presence status changed from %d to %d (notes <%s>)",
        m_presenceStatus, enStatus, static_cast<LPWSTR>(notes));

    RTC_PRESENCE_STATUS oldStatus = m_presenceStatus;
    m_presenceStatus = enStatus;
    m_observer->OnPresenceStatusChange(oldStatus, enStatus);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnRTCMediaRequestEvent(IRTCMediaRequestEvent* pEvent)
{
    g_env.Trace().WriteVerbose(L"OnRTCMediaRequestEvent event");

    long curMediaType;
    ComCheck(pEvent->get_CurrentMedia(&curMediaType));

    long proposedMediaType;
    ComCheck(pEvent->get_ProposedMedia(&proposedMediaType));

    g_env.Trace().WriteVerbose(L"Current media type is %d, proposed is %d",
        curMediaType, proposedMediaType);

    //TODO: add code to prompt for Media Request Event;

    pEvent->Accept(proposedMediaType);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::DoLogOn(void)
{
    if(m_rtcProfile)
    {
        // Already logged on
        DoReportError(L"Already logged on!");

        return;
    }

    m_connectionAttempt = 0;

    bstr_t uri(L"");
    bstr_t server(L"");
    bstr_t transport(L"");

    MTLASSERT(m_observer);
    if(m_observer->GetLoginInfo(uri, server, transport))
    {
        MTLASSERT(uri.length() > 0);

        g_env.Trace().WriteVerbose(L"Logging on as %s (server %s, transport %s)",
            static_cast<const wchar_t*>(uri), static_cast<const wchar_t*>(server),
            static_cast<const wchar_t*>(transport));

        RTC_PRESENCE_STATUS oldStatus = m_presenceStatus;
        m_presenceStatus = RTCXS_PRESENCE_CONNECTING;
        m_observer->OnPresenceStatusChange(oldStatus, m_presenceStatus);

        DoGetProfile(uri, server, transport);

        g_env.Trace().WriteVerbose(L"Finding server...");
    }
    else
    {
        g_env.Trace().WriteVerbose(L"Logon was cancelled by a user");

        // TODO: !!!!!
        //DoReportError(L"Logon failed!");
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnLoggingOn(void)
{
    m_observer->OnLoggingOn();
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnLoggedOn(void)
{
    g_env.Trace().WriteVerbose(L"Logged on");

    if(RTCXS_PRESENCE_OFFLINE != m_statusToSet)
        DoSetPresence(m_statusToSet);

    MTLASSERT(0 != m_rtcProfile);
    m_observer->OnLoggedOn(m_rtcProfile);

    PopulateBuddyList();
    PopulateWatcherList();
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::DoLogOff(void)
{
    if(0 == m_rtcProfile)
    {
        g_env.Trace().WriteVerbose(L"Already logged off");

        OnLoggedOff();
        return;
    }

    // Disable profile
    DoEnableProfile(false, 0, 0);

    m_rtcProfile = 0;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnLoggingOff(void)
{
    m_observer->OnLoggingOff();
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::OnLoggedOff(void)
{
    g_env.Trace().WriteVerbose(L"Logged off");

    //// Disable presence
    DoEnablePresence(FALSE);

    m_sessions.TerminateAllSessions();
    //// Cleanup the user search windows
    //CleanupUserSearches();

    //// Cleanup the watcher windows
    //CleanupWatchers();

    //// Cleanup the group windows
    //CleanupGroups();

    m_observer->OnLoggedOff();
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::DoAuth(const bstr_t& uri, const bstr_t& account, const bstr_t& password)
{
    // This code demonstrates how to set the credentials on a profile.
    // This can be called from a Registration State Change event if the failure is due to invalid authentication.

    g_env.Trace().WriteVerbose(L"Authenticating as %s, account %s, password ****",
        static_cast<const wchar_t*>(uri), static_cast<const wchar_t*>(account));

    // Update the credentials in the profile
    ComCheck(m_rtcProfile->SetCredentials(uri, account, password));

    // Re-register
    DoRegister();
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::DoEnablePresence(bool enable)
{
    g_env.Trace().WriteVerbose(L"DoEnablePresence(%d)", enable);

    if(m_fPresenceEnabled == enable)
    {
        g_env.Trace().WriteVerbose(L"Already in correct state");
        // Already in correct state
        return;
    }

    // Cleanup the buddy list
    m_observer->ClearBuddyList();

    IRTCClientPresence2Ptr presence;
    // Get the RTC client presence interface
    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence2), (void**)&presence));

    if(enable)
    {
        // Build the filename for presence storage
        // from the user URI
        bstr_t uri;
        ComCheck(m_rtcProfile->get_UserURI(uri.GetAddress()));

        wchar_t* pch = uri;
//        size_t cch;
//        cch = wcslen(uri) + wcslen(L"siprtc_.xml") + 1;

        while(*pch != L'\0')
        {
            // Replace all non-alphanumeric characters
            // in the URI with underscore
            if (!((*pch >= L'a') && (*pch <= L'z')) &&
                !((*pch >= L'A') && (*pch <= L'Z')) &&
                !((*pch >= L'0') && (*pch <= L'9')))
            {
                *pch = L'_';
            }

            ++pch;
        }

        variant_t storage = m_profilePath + L"siprtc_" + uri + ".xml";
        // Enable presence
        ComCheck(presence->EnablePresenceEx(m_rtcProfile, storage, 0));


        // This code demonstrates how to set a presence property for this particular device.

        // Set a presence property
        bstr_t propName(L"http://schemas.microsoft.com/rtc/rtcsample");
        bstr_t propVal(L"<name>Miranda IM</name>");

        HRESULT hr = presence->SetPresenceData(propName, propVal);

        if(FAILED(hr))
        {
            // SetPresenceData failed
            g_env.Trace().WriteError(L"SetPresenceData failed %x", hr);
        }

        ComCheck(presence->put_PresenceProperty(RTCPP_DEVICE_NAME, m_mirandaVersion));

        ComCheck(presence->put_OfferWatcherMode(RTCOWM_OFFER_WATCHER_EVENT));
    }
    else
    {
        // Disable presence
        HRESULT hr = presence->DisablePresence();
        if(FAILED(hr) && RTC_E_PRESENCE_NOT_ENABLED != hr) // Presence can be already disabled. That's ok.
        {
            ComCheck(hr);
        }

        m_presenceStatus = RTCXS_PRESENCE_OFFLINE;
    }

    // Set the enabled flag
    m_fPresenceEnabled = enable;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::DoGetProfile(const bstr_t& uri, const bstr_t& server, const bstr_t& transport)
{
    g_env.Trace().WriteVerbose(L"Getting the profile...");

    // Find transport
    long lTransport = 0;

    if(transport.length() > 0)
    {
        if(0 == _wcsicmp(transport, L"UDP"))
            lTransport = RTCTR_UDP;
        else if(0 == _wcsicmp(transport, L"TCP"))
            lTransport = RTCTR_TCP;
        else if(0 == _wcsicmp(transport, L"TLS"))
            lTransport = RTCTR_TLS;
    }

    // Get the RTC client provisioning interface
    IRTCClientProvisioning2Ptr prov;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientProvisioning2), (void**)&prov));

    // Get the profile
    ComCheck(prov->GetProfile(
            NULL,           // bstrUserAccount
            NULL,           // bstrUserPassword
            uri,        // bstrUserURI
            server,     // bstrServer
            lTransport,     // lTransport
            0               // lCookie
            ));
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::DoEnableProfile(bool enable, long lRegisterFlags, long lRoamingFlags)
{
    // Enable a Profile (which you can create using GetProfile).
    // This is called from DoRegister. We need to EnablePresenceEx first (which is done in DoRegister)

    // Get the RTC client provisioning interface
    IRTCClientProvisioning2Ptr prov;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientProvisioning2), (void**)&prov));

    if(enable)
    {
        g_env.Trace().WriteVerbose(L"Enabling the profile");
        // Enable the RTC profile object
        HRESULT hr = prov->EnableProfileEx(m_rtcProfile, lRegisterFlags, lRoamingFlags);

        // Roaming is not supported when the Registrar server specifies UDP transport
        if(RTC_E_UDP_NOT_SUPPORTED == hr)
        {
            g_env.Trace().WriteVerbose(L"Cannot enable roaming - UDP transport");
            hr = prov->EnableProfileEx(m_rtcProfile, lRegisterFlags, 0);
        }

        ComCheck(hr);
    }
    else
    {
        g_env.Trace().WriteVerbose(L"Disabling the profile");

        // Disable and release the RTC profile object
        HRESULT hr = prov->DisableProfile(m_rtcProfile);
        m_rtcProfile = 0;
        ComCheck(hr);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::DoRegister(void)
{
    // We will call DoEnablePresence in order to Enable Presence on a Profile (this needs to
    // be done in order to use presence features on the profile).
    // We will then call DoEnableProfile in order to enable the profile and perform the actual
    // registration.

    // Enable presence
    DoEnablePresence(true);

    // Enable the RTC profile object
    DoEnableProfile(true,
        RTCRF_REGISTER_ALL,
        RTCRMF_BUDDY_ROAMING |
        RTCRMF_WATCHER_ROAMING |
        RTCRMF_PRESENCE_ROAMING |
        RTCRMF_PROFILE_ROAMING);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::ConfigureAllowedAuthentication(void)
{
    /*
    The following list provides the transport types that are allowed for each of the supported
    authentication methods:

    Basic           Requires the TLS transport. This authentication method will be rejected if TLS
                    is not specified in the profile.

    Digest          Supported over the TCP, TLS, or UDP transport.

    Kerberos        Supported over the TCP or TLS transport. This authentication method will be
                    rejected if UDP is specified as the transport in the profile.

    NTLM            Supported over the TCP or TLS transport. This authentication method will be
                    rejected if UDP is specified as the transport in the profile.

    USE_LOGON_CRED  Supported over the TCP or TLS transport. This authentication method will be
                    rejected if UDP is specified as the transport in the profile. This authentication
                    method requires that at least one of the other authentication methods be present
                    in the profile.

    We don't know an actual transport that was used to generate the profile.
    That's why we try to guess.
        TLS - basic, digest, kerberos, ntlm, logon_cred
        TCP - digest, kerberos, ntlm, logon_cred
        UDP - digest
    */

    const long TLS_ALLOWED_AUTH = RTCAU_BASIC | RTCAU_DIGEST | RTCAU_NTLM | RTCAU_KERBEROS | RTCAU_USE_LOGON_CRED;
    const long TCP_ALLOWED_AUTH = RTCAU_DIGEST | RTCAU_NTLM | RTCAU_KERBEROS | RTCAU_USE_LOGON_CRED;
    const long UDP_ALLOWED_AUTH = RTCAU_DIGEST;

    // Assume TLS
    HRESULT hr = m_rtcProfile->put_AllowedAuth(TLS_ALLOWED_AUTH);
    if(FAILED(hr))
    {
        g_env.Trace().WriteVerbose(L"Failed to set the allowed authentication methods for TLS, %X", hr);

        MTLASSERT(RTC_E_UDP_NOT_SUPPORTED == hr || RTC_E_BASIC_AUTH_SET_TLS == hr);

        if(RTC_E_UDP_NOT_SUPPORTED == hr)
        {
            // Looks like the transport is UDP
            ComCheck(m_rtcProfile->put_AllowedAuth(UDP_ALLOWED_AUTH));
            g_env.Trace().WriteVerbose(L"Set the allowed authentication methods for UDP");
        }
        else
        {
            // TCP
            ComCheck(m_rtcProfile->put_AllowedAuth(TCP_ALLOWED_AUTH));
            g_env.Trace().WriteVerbose(L"Set the allowed authentication methods for TCP");
        }
    }
    else
        g_env.Trace().WriteVerbose(L"Set the allowed authentication methods for TLS");
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::SetPresenceStatus(RTC_PRESENCE_STATUS enStatus)
{
    g_env.Trace().WriteVerbose(L"SetPresenceStatus: new status %d, current state %d",
        enStatus, m_registrationState);

    if(RTCRS_REGISTERING == m_registrationState)
    {
        g_env.Trace().WriteVerbose(L"Registration is already in progress");
        m_statusToSet = enStatus;
        return;
    }
    else if(RTCRS_REGISTERED != m_registrationState)
    {
        DoLogOn();
        m_statusToSet = enStatus;
    }
    else
    {
        DoSetPresence(enStatus);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::SetPresenceNotes(RTC_PRESENCE_STATUS enStatus, const bstr_t& notes)
{
    m_statusNotes[enStatus] = notes;

    if(IsLoggedOn() && enStatus == m_presenceStatus)
    {
        // Update away message right now
        DoSetPresence(enStatus);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::DoSetPresence(RTC_PRESENCE_STATUS enStatus)
{
    // Get the RTC client presence interface
    IRTCClientPresencePtr presence;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence), (void**)&presence));

    g_env.Trace().WriteVerbose(L"Setting presence status to %d", enStatus);

    // Set the local presence status
    ComCheck(presence->SetLocalPresenceInfo(enStatus, m_statusNotes[enStatus]));
}
//--------------------------------------------------------------------------------------------------

inline RTC_REGISTRATION_STATE CRtcClient::GetRegistrationState(void) const
{
    return m_registrationState;
}
//--------------------------------------------------------------------------------------------------

inline RTC_PRESENCE_STATUS CRtcClient::GetPresenceStatus(void) const
{
    return m_presenceStatus;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::DoReportError(const wchar_t* message)
{
    g_env.Trace().WriteError(L"Reporting error '%s'", message);

    m_observer->OnError(message);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::PopulateBuddyList(void)
{
    MTLASSERT(m_observer);

    g_env.Trace().WriteVerbose(L"Populating the buddy list");

    IRTCClientPresencePtr presence;
    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence), (void**)&presence));

    // Enumerate buddies and populate list
    IRTCEnumBuddiesPtr pEnum;

    ComCheck(presence->EnumerateBuddies(&pEnum));

    IRTCBuddyPtr buddy;
    while(S_OK == pEnum->Next(1, &buddy, 0))
    {
        // Update the buddy list entry
        m_observer->UpdateBuddyList(buddy);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::PopulateWatcherList(void)
{
    MTLASSERT(m_observer);

    g_env.Trace().WriteVerbose(L"Populating the watcher list");

    IRTCClientPresencePtr presence;
    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence), (void**)&presence));

    // Enumerate buddies and populate list
    IRTCEnumWatchersPtr pEnum;
    ComCheck(presence->EnumerateWatchers(&pEnum));

    IRTCWatcherPtr watcher;
    while(S_OK == pEnum->Next(1, &watcher, 0))
    {
        bstr_t uri;
        ComCheck(watcher->get_PresentityURI(uri.GetAddress()));
        RTC_WATCHER_STATE state = RTCWS_UNKNOWN;
        ComCheck(watcher->get_State(&state));

        // Update the watcher entry
        m_observer->UpdateWatcher(uri, state);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::AddBuddy(const bstr_t& uri, const bstr_t& name, bool temporary)
{
    g_env.Trace().WriteVerbose(L"Adding the buddy %s (%s), temporary %d",
        static_cast<wchar_t*>(uri), static_cast<wchar_t*>(name), temporary ? 1 : 0);

    // Get the RTC client presence interface
    IRTCClientPresencePtr presence;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence), (void**)&presence));

    // Add the buddy
    IRTCBuddyPtr buddy;

    ComCheck(presence->AddBuddy(uri, name, 0, !temporary ? VARIANT_TRUE : VARIANT_FALSE, NULL, 0, &buddy));

    // Update the buddy list entry
    MTLASSERT(m_observer);
    m_observer->UpdateBuddyList(buddy);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::RemoveBuddy(const bstr_t& uri)
{
    g_env.Trace().WriteVerbose(L"Removing the buddy %s", static_cast<wchar_t*>(uri));

    MTLASSERT(uri.length() > 0);

    // Get the RTC client presence interface
    IRTCClientPresencePtr presence;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence), (void**)&presence));

    // Add the buddy
    IRTCBuddyPtr buddy;
    ComCheck(presence->get_Buddy(uri, &buddy));

    ComCheck(presence->RemoveBuddy(buddy));
    /*
        TODO

    // Update the buddy list entry
    MTLASSERT(m_observer);
    m_observer->UpdateBuddyList(buddy);
    */
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::RefreshBuddy(const bstr_t& uri)
{
    g_env.Trace().WriteVerbose(L"Refreshing %s' info", static_cast<wchar_t*>(uri));

    MTLASSERT(uri.length() > 0);

    // Get the RTC client presence interface
    IRTCClientPresencePtr presence;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence), (void**)&presence));

    // Add the buddy
    IRTCBuddyPtr buddy;
    ComCheck(presence->get_Buddy(uri, &buddy));

    IRTCBuddy2Ptr buddy2;
    ComCheck(buddy->QueryInterface(__uuidof(IRTCBuddy2), (void**)&buddy2));

    ComCheck(buddy2->Refresh());

    m_observer->OnRefreshBuddy(uri);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::LogOff(void)
{
    DoLogOff();
}
//--------------------------------------------------------------------------------------------------

inline CRtcSession CRtcClient::DoCall(ISessionController* controller, RTC_SESSION_TYPE enType,
    const bstr_t& uri, const bstr_t& name)
{
    std::auto_ptr<ISessionController> controllerHoder(controller);

    CRtcSession* existingSession = 0;
    if(m_sessions.FindByParticipant(uri, enType, &existingSession))
    {
        g_env.Trace().WriteVerbose(L"An existing session with %s has been found.",
            static_cast<wchar_t*>(uri));

        MTLASSERT(existingSession);
        return *existingSession;
    }
    else
    {
        g_env.Trace().WriteVerbose(L"Establishing a new session with %s of type %d",
            static_cast<wchar_t*>(uri), enType);

        if(RTCST_PC_TO_PC == enType || RTCST_PC_TO_PHONE == enType)
        {
            // Is there already an AV session? We can only
            // allow one at a time
            if(m_sessions.IsAVSessionInProgress())
            {
                //ShowMessageBox(L"An audio/video call is in progress!"); TODO
            }
        }

        // Create new session
        IRTCSessionPtr session;
        ComCheck(m_rtcClient->CreateSession(enType, NULL, NULL, 0, &session));

        // Add the session to the session list
        CRtcSession res = m_sessions.Add(controllerHoder.release(), session);

        // Add the participant to the session
        HRESULT hr = session->AddParticipant(uri, name, 0);

        if(FAILED(hr))
            m_sessions.Remove(session);

        ComCheck(hr);

        return res;
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::SendIMMessage(const bstr_t& uri, const bstr_t& name, const bstr_t& message,
    long cookie)
{
    g_env.Trace().WriteVerbose(L"Sending an IM message to %s (%s). Message text: <%s>, Cookie: %d",
        static_cast<wchar_t*>(uri), static_cast<wchar_t*>(name), static_cast<wchar_t*>(message), cookie);

    MTLASSERT(uri.length() > 0);
    MTLASSERT(message.length() > 0);

    CRtcSession session = DoCall(0, RTCST_MULTIPARTY_IM, uri, name);
    // TODO: AddParticipant takes time - send after CONNECTED event.
    // Actually, we need some investigation here. Looks like RTC performs special processing of
    // this particular case, and we don't have any issues here.
    // Otherwise, we can supply special controller, that sends a message after the CONNECTED event.
    session.SendIMMessage(message, cookie);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::SendUserIsTyping(const bstr_t& uri, bool isTyping)
{
    g_env.Trace().WriteVerbose(L"Sending typing status to %s (%s).",
        static_cast<wchar_t*>(uri), isTyping ? L"typing" : L"stopped typing");

    MTLASSERT(uri.length() > 0);

    CRtcSession* existingSession = 0;
    if(m_sessions.FindByParticipant(uri, RTCST_MULTIPARTY_IM, &existingSession))
    {
        CRtcSession session = DoCall(0, RTCST_MULTIPARTY_IM, uri, bstr_t());
        // TODO: AddParticipant takes time - send after CONNECTED event.
        // Actually, we need some investigation here. Looks like RTC performs special processing of
        // this particular case, and we don't have any issues here.
        // Otherwise, we can supply special controller, that sends a status after the CONNECTED event.
        session.SendUserIsTyping(isTyping);
    }
    else
        g_env.Trace().WriteVerbose(L"No existing IM session with %s was found. Ignoring typing notification.",
            static_cast<wchar_t*>(uri));
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::EstablishPcToPcSession(ISessionController* controller, const bstr_t& uri,
    const bstr_t& name)
{
    g_env.Trace().WriteVerbose(L"Establishing a PC to PC session with %s (%s).",
        static_cast<wchar_t*>(uri), static_cast<wchar_t*>(name));

    MTLASSERT(controller);
    MTLASSERT(uri.length() > 0);

    DoCall(controller, RTCST_PC_TO_PC, uri, name);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::EstablishAVSession(ISessionController* controller, const bstr_t& uri,
    const bstr_t& name)
{
    g_env.Trace().WriteVerbose(L"Establishing a AV session with %s (%s).",
        static_cast<wchar_t*>(uri), static_cast<wchar_t*>(name));

    MTLASSERT(uri.length() > 0);

    VARIANT_BOOL isTuned = VARIANT_FALSE;
    ComCheck(m_rtcClient->get_IsTuned(&isTuned));
    g_env.Trace().WriteVerbose(L"Is tuned %d", isTuned);

    if(!isTuned)
    {
        ComCheck(m_rtcClient->InvokeTuningWizard(0));
    }


    RTC_SESSION_TYPE type = RTCST_PC_TO_PHONE;
    if(0 == StrCmpNI(uri, L"sip:", 4))
    {
        type = RTCST_PC_TO_PC;
    }

    DoCall(controller, type, uri, name);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::TerminateSession(const bstr_t& uri)
{
    g_env.Trace().WriteVerbose(L"Terminating the session with %s.",
        static_cast<wchar_t*>(uri));

    //MTLASSERT(controller);
    MTLASSERT(uri.length() > 0);

    CRtcSession* existingSession = 0;
    if(m_sessions.FindByParticipant(uri, RTCST_PC_TO_PC, &existingSession) ||
        m_sessions.FindByParticipant(uri, RTCST_PC_TO_PHONE, &existingSession))
    {
        g_env.Trace().WriteVerbose(L"The session with %s has been found. Terminating...",
            static_cast<wchar_t*>(uri));

        MTLASSERT(existingSession);

        existingSession->Terminate();
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::TerminateAVSession(void)
{
    g_env.Trace().WriteVerbose(L"Terminating the AV session");

    MTLASSERT(m_sessions.IsAVSessionInProgress());

    CRtcSession* existingSession = 0;
    if(m_sessions.FindAVSession(&existingSession))
    {
        g_env.Trace().WriteVerbose(L"The AV session has been found. Terminating...");

        MTLASSERT(existingSession);

        existingSession->Terminate();
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::AcceptAVSession(void)
{
    g_env.Trace().WriteVerbose(L"Accepting the AV session");

    MTLASSERT(m_sessions.IsAVSessionInProgress());

    CRtcSession* existingSession = 0;
    if(m_sessions.FindAVSession(&existingSession))
    {
        g_env.Trace().WriteVerbose(L"The AV session has been found. Accepting...");

        MTLASSERT(existingSession);

        existingSession->Accept();
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::RejectAVSession(void)
{
    g_env.Trace().WriteVerbose(L"Rejecting the AV session");

    MTLASSERT(m_sessions.IsAVSessionInProgress());

    CRtcSession* existingSession = 0;
    if(m_sessions.FindAVSession(&existingSession))
    {
        g_env.Trace().WriteVerbose(L"The AV session has been found. Rejecting...");

        MTLASSERT(existingSession);

        existingSession->Reject();
    }
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcClient::IsAVSessionInProgress(void) const
{
    return RTCSS_DISCONNECTED != m_sessions.GetAVSessionState();
}
//--------------------------------------------------------------------------------------------------

inline RTC_SESSION_STATE CRtcClient::GetAVSessionState(void) const
{
    return m_sessions.GetAVSessionState();
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::SearchContact(const std::vector<SearchTerm>& terms, long cookie)
{
    g_env.Trace().WriteVerbose(L"Starting a search (cookie %d)...", cookie);

    MTLASSERT(!terms.empty());

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCUserSearch), (void**)&m_userSearch));

    IRTCUserSearchQueryPtr query;

    ComCheck(m_userSearch->CreateQuery(&query));

    for(unsigned i = 0; i < terms.size(); ++i)
    {
        g_env.Trace().WriteVerbose(L"  %s = %s",
            static_cast<const wchar_t*>(terms[i].term),
            static_cast<const wchar_t*>(terms[i].value));

        MTLASSERT(terms[i].term.length() > 0);
        ComCheck(query->put_SearchTerm(terms[i].term, terms[i].value));
    }

    // Set max number of results to 20
    ComCheck(query->put_SearchPreference(RTCUSP_MAX_MATCHES, 20));

    // Execute the user search
    ComCheck(m_userSearch->ExecuteSearch(query, 0, cookie));

    m_observer->OnSearchStarted(terms, cookie);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::AddGroup(const bstr_t& name)
{
    MTLASSERT(name.length() > 0);

    // Get the RTC client presence interface
    IRTCClientPresence2Ptr presence;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence2), (void**)&presence));

    IRTCBuddyGroupPtr group;

    HRESULT hr = presence->AddGroup(name, 0, 0, 0, &group);

    // Did the group already exist?
    if(FAILED(hr) && RTC_E_DUPLICATE_GROUP != hr)
    {
        ComCheck(hr);
    }

    // TODO: m_observer->AddGroup(name);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::RemoveGroup(const bstr_t& name)
{
    MTLASSERT(name.length() > 0);

    // Get the RTC client presence interface
    IRTCClientPresence2Ptr presence;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence2), (void**)&presence));

    IRTCBuddyGroupPtr group;

    HRESULT hr = presence->get_Group(name, &group);
    if(SUCCEEDED(hr))
    {
        ComCheck(presence->RemoveGroup(group));
    }
    else if(RTC_E_NOT_EXIST != hr) // // Did the group exist?
    {
        ComCheck(hr);
    }

    // TODO: m_observer->RemoveGroup(name);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::AddBuddyToGroup(const bstr_t& groupName, const bstr_t& buddyUri)
{
    MTLASSERT(groupName.length() > 0);
    MTLASSERT(buddyUri.length() > 0);

    // Get the RTC client presence interface
    IRTCClientPresence2Ptr presence;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence2), (void**)&presence));

    IRTCBuddyGroupPtr group;
    ComCheck(presence->get_Group(groupName, &group));

    IRTCBuddyPtr buddy;
    ComCheck(presence->get_Buddy(buddyUri, &buddy));

    ComCheck(group->AddBuddy(buddy));
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::RemoveBuddyFromGroup(const bstr_t& groupName, const bstr_t& buddyUri)
{
    MTLASSERT(groupName.length() > 0);
    MTLASSERT(buddyUri.length() > 0);

    // Get the RTC client presence interface
    IRTCClientPresence2Ptr presence;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence2), (void**)&presence));

    IRTCBuddyGroupPtr group;
    ComCheck(presence->get_Group(groupName, &group));

    IRTCBuddyPtr buddy;
    ComCheck(presence->get_Buddy(buddyUri, &buddy));

    ComCheck(group->RemoveBuddy(buddy));
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClient::AuthorizeWatcher(const bstr_t& uri, bool allow)
{
    g_env.Trace().WriteVerbose(L"Authorizing the watcher %s - %s",
        static_cast<const wchar_t*>(uri), allow ? L"Allow" : L"Block");

    MTLASSERT(uri.length() > 0);

    // Get the RTC client presence interface
    IRTCClientPresence2Ptr presence;

    ComCheck(m_rtcClient->QueryInterface(__uuidof(IRTCClientPresence2), (void**)&presence));

    IRTCWatcherPtr watcher;
    ComCheck(presence->get_Watcher(uri, &watcher));

    ComCheck(watcher->put_State(allow ? RTCWS_ALLOWED : RTCWS_BLOCKED));
}
//--------------------------------------------------------------------------------------------------
