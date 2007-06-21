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

#include "RtcClient.h"
#include "RtcHelper.h"
#include "message.h"
#include "authreq.h"
#include "resource.h"
#include "t120.h"
#include "avsessioncontroller.h"
#include "m_siprtc.h"
#include "ui/signindlg.h"
#include "ui/authdlg.h"
#include "ui/optionsdlg.h"
#include "ui/annoyuser.h"
//--------------------------------------------------------------------------------------------------

//
// Async broadcasting
//
inline void __stdcall AsyncBroadcastAckWorker(void* param)
{
    MTLASSERT(param);

    if(param)
    {
        std::auto_ptr<ACKDATA> ack(reinterpret_cast<ACKDATA*>(param));

        ACKDATA* a = ack.get();
        g_env.Trace().WriteVerbose(L"Broadcasting %S, contact %d, type %d, result %d, process %d, param %d",
            a->szModule, a->hContact, a->type, a->result, a->hProcess, a->lParam);

        CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)ack.get());
    }
}
//--------------------------------------------------------------------------------------------------

//
// WARNING! DO NOT PASS POINTERS AS THE LAST PARAMETERS!
//
inline void ProtoAsyncBroadcastAck(const char* proto, HANDLE hContact, int type, int result,
    HANDLE hProcess, LPARAM param)
{
    MTLASSERT(proto);
    MTLASSERT(IsBadReadPtr((const void*)param, sizeof(LPARAM))); // We don't expect pointers here!

    ACKDATA* ack = new ACKDATA;

    ZeroMemory(ack, sizeof(ACKDATA));
    ack->cbSize   = sizeof(ACKDATA);
    ack->szModule = proto;
    ack->hContact = hContact;
    ack->type     = type;
    ack->result   = result;
    ack->hProcess = hProcess;
    ack->lParam   = param;

    ACKDATA* a = ack;
    g_env.Trace().WriteVerbose(L"Preparing to broadcast %S, contact %d, type %d, result %d, process %d, param %d",
        a->szModule, a->hContact, a->type, a->result, a->hProcess, a->lParam);

    CallFunctionAsync(AsyncBroadcastAckWorker, ack);
}
//--------------------------------------------------------------------------------------------------

class CRtcClientObserver : public IRtcObserver
{
public:
                        CRtcClientObserver(void);

    virtual bool        GetLoginInfo(bstr_t& uri, bstr_t& server, bstr_t& transport);
    virtual bool        GetAuthInfo(const bstr_t& realm, bstr_t& uri, bstr_t& account,
                            bstr_t& password, int connectionAttempt);

    virtual void        UpdateBuddyList(IRTCBuddy* pBuddy);
    virtual void        ClearBuddyList(IRTCBuddy* pBuddy);
    virtual void        ClearBuddyList(void);

    virtual void        UpdateWatcher(const bstr_t& uri, RTC_WATCHER_STATE state);
    virtual bool        GetWatcherPermission(const bstr_t& uri, const bstr_t& name);

    virtual void        OnLoginError(const wchar_t* message, HRESULT hr);
    virtual void        OnError(const wchar_t* message);
    virtual void        OnError(const _com_error& e, const char* source);

    virtual void        OnPresenceStatusChange(RTC_PRESENCE_STATUS oldStatus, RTC_PRESENCE_STATUS newStatus);
    virtual void        OnLoggingOn(void);
    virtual void        OnLoggedOn(IRTCProfile* profile);
    virtual void        OnLoggingOff(void);
    virtual void        OnLoggedOff(void);
    virtual void        OnSessionOperationComplete(long cookie, long status, const bstr_t& statusText);
    virtual void        OnRefreshBuddy(const bstr_t& uri);
    virtual void        OnSearchStarted(const std::vector<SearchTerm>& terms, long cookie);

    virtual void        DeliverMessage(IRTCParticipant* pParticipant, const bstr_t& contentType,
                            const bstr_t& message);
    virtual void        DeliverUserStatus(IRTCParticipant* pParticipant,
                            RTC_MESSAGING_USER_STATUS status);

    virtual void        DeliverUserSearchResult(long cookie, HRESULT status, const bstr_t& uri,
                            const bstr_t& nick, const bstr_t& email);

    virtual void        DeliverMedia(long mediaType, RTC_MEDIA_EVENT_TYPE type,
                            RTC_MEDIA_EVENT_REASON reason);

    virtual ISessionController* GetSessionController(RTC_SESSION_TYPE sessionType);

private:
    bool                m_loggedOn;
    bstr_t              m_getInfoRequestUri;
    bstr_t              m_searchedUri;
};
//--------------------------------------------------------------------------------------------------

inline CRtcClientObserver::CRtcClientObserver(void) :
    m_loggedOn(false)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcClientObserver::GetLoginInfo(bstr_t& uri, bstr_t& server, bstr_t& transport)
{
    bool res = true;

    uri       = g_env.DB().GetMySettingWString("URI");
    bool autoConnect = g_env.DB().GetMySettingBool("AutoConnect", true);
    server    = g_env.DB().GetMySettingWString("Server");
    transport = g_env.DB().GetMySettingWString("Transport");

    if(0 == uri.length())
    {
        //
        // Support for SiP plugin by TOXIC
        //
        uri       = g_env.DB().GetMySettingWStringFromSiP("login");
        server    = g_env.DB().GetMySettingWStringFromSiP("server");

        CSignInDialog dlg;
        dlg.Init(uri, autoConnect, server, transport);

        if(IDOK == dlg.DoModal(::GetDesktopWindow()))
        {
            uri         = dlg.GetUri();
            autoConnect = dlg.GetAutoConnect();
            server      = dlg.GetServer();
            transport   = dlg.GetTransport();

            g_env.DB().WriteMySettingWString("URI",       uri);
            g_env.DB().WriteMySettingBool("AutoConnect",  autoConnect);
            g_env.DB().WriteMySettingWString("Server",    server);
            g_env.DB().WriteMySettingWString("Transport", transport);

            bstr_t nick = g_env.DB().GetMySettingWString("Nick");
            if(0 == nick.length())
            {
                g_env.DB().WriteMySettingWString("Nick", ComposeNickByUri(uri));
            }
        }
    }

    if(autoConnect)
    {
        server = L"";
        transport = L"";
    }

    return uri.length() > 0;
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcClientObserver::GetAuthInfo(const bstr_t& realm, bstr_t& uri, bstr_t& account,
    bstr_t& password, int connectionAttempt)
{
    bool res = true;

    uri       = g_env.DB().GetMySettingWString("URI");
    bool autoAuth = g_env.DB().GetMySettingBool("AutoAuth", true);
    account   = g_env.DB().GetMySettingWString("Account");
    password  = g_env.DB().GetMyEncryptedSettingWString("Password");

    if(uri.length() > 0 && !autoAuth && (account.length() > 0 || password.length() > 0) &&
        1 == connectionAttempt)
    {
        //
        // If this is the first attempt to log on, and we have account information in the database,
        // try to connect using this information
        //
        res = true;
    }
    else
    {
        //
        // Otherwise, ask user for information
        //

        CAuthDialog dlg;
        dlg.Init(realm, uri, account, password, password.length() > 0);

        if(IDOK == dlg.DoModal(::GetDesktopWindow()))
        {
            uri       = dlg.GetUri();
            account   = dlg.GetAccount();
            password  = dlg.GetPassword();

            g_env.DB().WriteMySettingBool("AutoAuth",     false);
            g_env.DB().WriteMySettingWString("URI",       uri);
            g_env.DB().WriteMySettingWString("Account",   account);
            g_env.DB().WriteMyEncryptedSettingWString("Password",  dlg.GetSavePassword() ? password : L"");

            bstr_t nick = g_env.DB().GetMySettingWString("Nick");
            if(0 == nick.length())
            {
                g_env.DB().WriteMySettingWString("Nick", ComposeNickByUri(uri));
            }

            res = uri.length() > 0;
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::UpdateWatcher(const bstr_t& uri, RTC_WATCHER_STATE state)
{
    if(0 == uri.length()) // Some watchers have an empty URI for some reason
    {
        g_env.Trace().WriteVerbose(L"UpdateWatcher: empty URI");
        return;
    }

    HANDLE hContact = g_env.DB().FindContact(uri);

    if(hContact)
    {
        int oldMode = DBGetContactSettingWord(hContact, g_env.ProtocolName(), "ApparentMode", 0);
        int newMode = RTCWS_ALLOWED == state ? 0 : ID_STATUS_OFFLINE;

        if(newMode != oldMode)
        {
            DBWriteContactSettingWord(hContact, g_env.ProtocolName(), "ApparentMode", newMode);
        }
    }
}
//--------------------------------------------------------------------------------------------------

inline bool CRtcClientObserver::GetWatcherPermission(const bstr_t& uri, const bstr_t& name)
{
    bool allow = true;

    bool requireAuth = g_env.DB().GetMySettingBool("RequireAuth", true);

    if(requireAuth)
    {
        g_env.Trace().WriteVerbose(L"Ask user to allow/deny the watcher");

        allow = false;

        if(uri.length() > 0)  // Some watchers have an empty URI for some reason
        {
            // If unknown contact - add temporary contact
            HANDLE hContact = g_env.DB().FindOrAddContact(uri, name, true);
            bstr_t nick = name.length() > 0 ? name : ComposeNickByUri(uri);

            CAuthRequestBlob blob(hContact, uri, nick);

            PROTORECVEVENT recv;
            recv.flags = 0;
            recv.timestamp = (DWORD)time(NULL);
            recv.szMessage = const_cast<char*>(blob.GetBuffer());
            recv.lParam    = blob.GetSize();

            CCSDATA ccs;
            ccs.hContact = hContact;
            ccs.wParam = 0;
            ccs.szProtoService = PSR_AUTH;
            ccs.lParam = (LPARAM)&recv;

            CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
        }
        else
            g_env.Trace().WriteVerbose(L"GetWatcherPermission: empty URI (name: %s)",
                static_cast<const wchar_t*>(name));
    }
    else
    {
        g_env.Trace().WriteVerbose(L"Automatically allow the watcher");
        allow = true;
    }

    return allow;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::UpdateBuddyList(IRTCBuddy* pBuddy)
{
    if(!pBuddy)
    {
        g_env.Trace().WriteError(L"UpdateBuddyList: null buddy");
        return;
    }

    try
    {
        // Get the buddy status
        RTC_PRESENCE_STATUS enStatus = RTCXS_PRESENCE_OFFLINE;

        HRESULT hr = pBuddy->get_Status(&enStatus);
        if(FAILED(hr))
        {
            g_env.Trace().WriteError(L"Buddy::get_Status Failed %X", hr);
            enStatus = RTCXS_PRESENCE_OFFLINE;
        }

        g_env.Trace().WriteVerbose(L"  Buddy status %d", enStatus);

        bstr_t uri;
        ComCheck(pBuddy->get_PresentityURI(uri.GetAddress()));
        MTLASSERT(uri.length() > 0);

        g_env.Trace().WriteVerbose(L"  Buddy uri %s", static_cast<const wchar_t*>(uri));

        // Get the buddy name
        IRTCBuddy2Ptr pBuddy2;
        ComCheck(pBuddy->QueryInterface(__uuidof(IRTCBuddy2), (void**)&pBuddy2));

        bstr_t name;
        hr = pBuddy2->get_PresenceProperty(RTCPP_DISPLAYNAME, name.GetAddress());

        if(FAILED(hr) || 0 == name.length())
        {
            if(FAILED(pBuddy->get_Name(name.GetAddress())) || 0 == name.length())
            {
                name = ComposeNickByUri(uri);
            }
        }

        bstr_t phone;
        if(FAILED(pBuddy2->get_PresenceProperty(RTCPP_PHONENUMBER, phone.GetAddress())) || 0 == phone.length())
        {
            phone = L"";
        }

        bstr_t email;
        if(FAILED(pBuddy2->get_PresenceProperty(RTCPP_EMAIL, email.GetAddress())) || 0 == email.length())
        {
            email = L"";
        }

        RTC_BUDDY_SUBSCRIPTION_TYPE subscription;
        if(SUCCEEDED(pBuddy2->get_SubscriptionType(&subscription)))
        {
            g_env.Trace().WriteVerbose(L"  Allowed subscription: %d", subscription);
        }

        bstr_t notes;
        if(FAILED(pBuddy->get_Notes(notes.GetAddress())) || 0 == notes.length())
        {
            notes = L"";
        }

        g_env.Trace().WriteVerbose(L"  Buddy notes %s", static_cast<const wchar_t*>(notes));


#if defined(RTC_GROUPS_SUPPORT)
        bstr_t groupName(L"");

        IRTCEnumGroupsPtr pEnum;
        hr = pBuddy2->EnumerateGroups(&pEnum);
        if(SUCCEEDED(hr))
        {
            IRTCBuddyGroupPtr group;
            if(S_OK == pEnum->Next(1, &group, NULL))
            {
                if(FAILED(group->get_Name(groupName.GetAddress())))
                {
                    groupName = L"";
                }
            }
        }

        g_env.Trace().WriteVerbose(L"  Group %s", static_cast<const wchar_t*>(groupName));

#endif // RTC_GROUPS_SUPPORT

        std::vector<bstr_t> deviceNames;
        IRTCEnumPresenceDevicesPtr pDevEnum;
        if(SUCCEEDED(pBuddy2->EnumeratePresenceDevices(&pDevEnum)))
        {
            IRTCPresenceDevicePtr device;
            if(S_OK == pDevEnum->Next(1, &device, NULL))
            {
                bstr_t name;
                if(SUCCEEDED(device->get_PresenceProperty(RTCPP_DEVICE_NAME, name.GetAddress())) &&
                    name.length() > 0)
                {
                    deviceNames.push_back(name);
                }
            }
        }

        g_env.Trace().WriteVerbose(L"  Buddy has %d device(s)", deviceNames.size());

        HANDLE hContact = g_env.DB().FindOrAddContact(uri, name);
        MTLASSERT(hContact);

        if(hContact)
        {
            DBDeleteContactSetting(hContact, "CList", "Hidden");
            //DBDeleteContactSetting(hContact, "CList", "NotOnList");

            DBWriteContactSettingWString(hContact, g_env.ProtocolName(), "Nick", name);
            DBWriteContactSettingWord(hContact, g_env.ProtocolName(), "Status",
                ContactRtcPresenceStatusToMirandaStatus(enStatus));

            if(notes.length() > 0)
            {
                DBWriteContactSettingWString(hContact, "CList", "StatusMsg", notes);
            }
            else
            {
                DBDeleteContactSetting(hContact, "CList", "StatusMsg");
            }

            if(email.length() > 0)
            {
                DBWriteContactSettingWString(hContact, g_env.ProtocolName(), "e-mail", email);
            }
            else
            {
                DBDeleteContactSetting(hContact, g_env.ProtocolName(), "e-mail");
            }

            if(phone.length() > 0)
            {
                DBWriteContactSettingWString(hContact, g_env.ProtocolName(), "Phone", phone);
            }
            else
            {
                DBDeleteContactSetting(hContact, g_env.ProtocolName(), "Phone");
            }

            bstr_t aboutText = L"";

            if(!deviceNames.empty())
            {
                for(unsigned i = 0; i < deviceNames.size(); ++i)
                {
                    if(aboutText.length() > 0)
                    {
                        aboutText += L"\r\n";
                    }

                    aboutText += deviceNames[i];
                }
            }

            if(aboutText.length() > 0)
            {
                DBWriteContactSettingWString(hContact, g_env.ProtocolName(), "About", aboutText);
                if(0 == StrCmpNI(aboutText, L"Miranda", 7))
                {
                    DBWriteContactSettingWString(hContact, g_env.ProtocolName(), "MirVer", aboutText);
                }
                else
                {
                    DBDeleteContactSetting(hContact, g_env.ProtocolName(), "MirVer");
                }
            }
            else
            {
                DBDeleteContactSetting(hContact, g_env.ProtocolName(), "About");
                DBDeleteContactSetting(hContact, g_env.ProtocolName(), "MirVer");
            }


#if defined(RTC_GROUPS_SUPPORT)

            if(groupName.length() > 0)
            {
                DBWriteContactSettingWString(hContact, "CList", "Group", groupName);
            }
            else
            {
                DBDeleteContactSetting(hContact, "CList", "Group");
            }

#endif // RTC_GROUPS_SUPPORT

            g_env.Trace().WriteVerbose(L"  Status updated");

            if(m_getInfoRequestUri.length() > 0 && 0 == lstrcmpiW(uri, m_getInfoRequestUri))
            {
                m_getInfoRequestUri = L"";

                ProtoBroadcastAck(g_env.ProtocolName(), hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS,
                    (HANDLE)1, 0);
            }
        }
    }
    catch(_com_error& e)
    {
        OnError(e, __FUNCTION__);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::ClearBuddyList(IRTCBuddy* pBuddy)
{
    if(!pBuddy)
    {
        return;
    }

    try
    {
        bstr_t uri;
        ComCheck(pBuddy->get_PresentityURI(uri.GetAddress()));
        MTLASSERT(uri.length() > 0);

        if(uri.length() > 0)
        {
            g_env.DB().SetContactStatus(uri, ContactRtcPresenceStatusToMirandaStatus(RTCXS_PRESENCE_OFFLINE));
        }
    }
    catch(_com_error& e)
    {
        OnError(e, __FUNCTION__);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::ClearBuddyList(void)
{
    if(g_pluginLink.IsInitialized())
    {
        g_env.DB().SetAllContactsStatusToOffline();
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnLoginError(const wchar_t* message, HRESULT hr)
{
    MTLASSERT(message);

    ProtoBroadcastAck(g_env.ProtocolName(), NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, 0, GetLoginFailureReason(hr));

    if(message)
    {
        wchar_t* fmt  = TranslateW(L"%s\n\n%s\nError: %X");

        bstr_t description = GetRTCErrorMessage(hr);
        if(0 == description.length())
        {
            description = L"";
        }

        message = TranslateW(message);

        unsigned bufSize = lstrlenW(message) + lstrlenW(fmt) + description.length() + 8 + 10;
        wchar_t* buffer = (wchar_t*)_alloca(sizeof(wchar_t) * bufSize);

        MTLVERIFY(S_OK == StringCchPrintf(buffer, bufSize, fmt, message,
            static_cast<const wchar_t*>(description), hr));
        OnError(buffer);

        if(g_env.DB().GetMySettingBool("ShowLoginErrors", true))
        {
            AnnoyUser(buffer);
        }
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnError(const _com_error& e, const char* source)
{
    wchar_t message[512];
    StringCbPrintfW(message, sizeof(message), L"Error: %X\nDescription: %s\nSource: %S", e.Error(),
        static_cast<const wchar_t*>(e.Description()), source);
    OnError(message);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnError(const wchar_t* message)
{
    MTLASSERT(message);

    if(message)
    {
        g_env.Trace().WriteError(message);
        // MessageBoxW(0, message, TranslateW(L"SIP RTC Protocol"), MB_ICONERROR);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnPresenceStatusChange(RTC_PRESENCE_STATUS oldStatus,
    RTC_PRESENCE_STATUS newStatus)
{
    ProtoAsyncBroadcastAck(g_env.ProtocolName(), NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS,
        reinterpret_cast<HANDLE>(MyRtcPresenceStatusToMirandaStatus(oldStatus, m_loggedOn)),
        MyRtcPresenceStatusToMirandaStatus(newStatus, m_loggedOn));
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnLoggingOn(void)
{
    ProtoAsyncBroadcastAck(g_env.ProtocolName(), NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS,
        (HANDLE)ID_STATUS_OFFLINE, ID_STATUS_CONNECTING);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnLoggedOn(IRTCProfile* profile)
{
    m_loggedOn = true;

    if(profile)
    {
        //
        // Update our nick
        //
        try
        {
            bstr_t name(L"");
            if(SUCCEEDED(profile->get_UserName(name.GetAddress())) &&
                name.length() > 0)
            {
                g_env.DB().WriteMySettingWString("Nick", name);
            }
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to get local user name: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnLoggingOff(void)
{
    // TODO
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnLoggedOff(void)
{
    g_env.Trace().WriteVerbose(L"CRtcClientObserver::OnLoggedOff. Broadcasting offline");

    m_loggedOn = false;
    m_getInfoRequestUri = L"";

    ProtoAsyncBroadcastAck(g_env.ProtocolName(), NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS,
        (HANDLE)ID_STATUS_ONLINE, ID_STATUS_OFFLINE);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnSessionOperationComplete(long cookie, long status,
    const bstr_t& statusText)
{
    if(cookie)
    {
        bstr_t errorDescription(statusText);
        if(0 == errorDescription.length() && FAILED(status))
        {
            errorDescription = GetRTCErrorMessage((HRESULT)status);
        }

        ProtoBroadcastAck(g_env.ProtocolName(), (HANDLE)cookie,
            ACKTYPE_MESSAGE,
            SUCCEEDED(status) ? ACKRESULT_SUCCESS : ACKRESULT_FAILED,
            (HANDLE)cookie,
            LPARAM(FAILED(status) ? static_cast<const char*>(errorDescription): 0));
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnRefreshBuddy(const bstr_t& uri)
{
    MTLASSERT(uri.length() > 0);
    m_getInfoRequestUri = uri;
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::OnSearchStarted(const std::vector<SearchTerm>& terms, long cookie)
{
    m_searchedUri = L"";
    for(unsigned i = 0; i < terms.size(); ++i)
    {
        if(0 == lstrcmpiW(L"msRTCURI", terms[i].term) || 0 == lstrcmpiW(L"msRTCMail", terms[i].term))
        {
            m_searchedUri = terms[i].value;
            break;
        }
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::DeliverMessage(IRTCParticipant* pParticipant,
    const bstr_t& contentType, const bstr_t& message)
{
    MTLASSERT(pParticipant);
    MTLASSERT(message.length() > 0);

    if(0 == message.length())
    {
        return;
    }

    bstr_t uri;
    ComCheck(pParticipant->get_UserURI(uri.GetAddress()));
    MTLASSERT(uri.length() > 0);

    bstr_t name;
    if(FAILED(pParticipant->get_Name(name.GetAddress())))
    {
        name = ComposeNickByUri(uri);
    }

    HANDLE hContact = g_env.DB().FindOrAddContact(uri, name, true); // If unknown contact - add temporary contact

    CMirandaMessage messageBuffer(contentType, message);

    PROTORECVEVENT recv;
    recv.flags = PREF_UNICODE;
    recv.timestamp = (DWORD)time(NULL);
    recv.szMessage = const_cast<char*>(messageBuffer.GetBuffer());
    recv.lParam = 0;

    CCSDATA ccs;
    ccs.hContact = hContact;
    ccs.wParam = 0;
    ccs.szProtoService = PSR_MESSAGE;
    ccs.lParam = (LPARAM)&recv;

    CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, PROTOTYPE_CONTACTTYPING_OFF);
    CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::DeliverUserStatus(IRTCParticipant* pParticipant,
    RTC_MESSAGING_USER_STATUS status)
{
    MTLASSERT(pParticipant);

    bstr_t uri;
    ComCheck(pParticipant->get_UserURI(uri.GetAddress()));
    MTLASSERT(uri.length() > 0);

    bstr_t name;
    if(FAILED(pParticipant->get_Name(name.GetAddress())))
    {
        name = ComposeNickByUri(uri);
    }

    HANDLE hContact = g_env.DB().FindOrAddContact(uri, name, true);

    if(RTCMUS_TYPING == status)
    {
        CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, 5);
    }
    else
    {
        CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, PROTOTYPE_CONTACTTYPING_OFF);
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::DeliverUserSearchResult(long cookie, HRESULT status, const bstr_t& uri,
    const bstr_t& nick, const bstr_t& email)
{
    if(SUCCEEDED(status))
    {
        if(uri.length() > 0)
        {
            SIPRTC_SEARCH_RESULT sr;
            ZeroMemory(&sr, sizeof(sr));
            sr.cbSize = sizeof(SIPRTC_SEARCH_RESULT);
            sr.nick = nick;
            sr.firstName = "";
            sr.lastName = "";
            sr.email = email;
            MTLVERIFY(SUCCEEDED(StringCbCopy(sr.uri, sizeof(sr.uri), uri)));

            ProtoBroadcastAck(g_env.ProtocolName(), 0, ACKTYPE_SEARCH, ACKRESULT_DATA,
                (HANDLE)cookie, (LPARAM)&sr);
        }
        else
        {
            ProtoBroadcastAck(g_env.ProtocolName(), 0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS,
                (HANDLE)cookie, 0);
        }
    }
    else
    {
        if(m_searchedUri.length() > 0)
        {
            //
            // Search failed, but if the user searched by SIP URI, let him add this URI to the contact list
            //
            StartCheekyBasicSearch(m_searchedUri, cookie);
        }
        else
        {
            //
            //  Otherwise, just report error (looks like Miranda ignores this error)
            //
            ProtoBroadcastAck(g_env.ProtocolName(), 0, ACKTYPE_SEARCH, ACKRESULT_FAILED,
                (HANDLE)cookie, 0);
        }
    }
}
//--------------------------------------------------------------------------------------------------

inline void CRtcClientObserver::DeliverMedia(long mediaType, RTC_MEDIA_EVENT_TYPE type,
    RTC_MEDIA_EVENT_REASON reason)
{
    // TODO
}
//--------------------------------------------------------------------------------------------------

inline ISessionController* CRtcClientObserver::GetSessionController(RTC_SESSION_TYPE sessionType)
{
    ISessionController* controller = 0;

    switch(sessionType)
    {
    case RTCST_PC_TO_PHONE:
    case RTCST_PC_TO_PC:
        controller = new CAVSessionController(); // TODO!!!! - CT120SessionController???
        break;
    }

    return controller;
}
//--------------------------------------------------------------------------------------------------
