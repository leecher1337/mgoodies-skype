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

//
// Uncomment to enable RTC group support. Partly implemented.
//
// There's a problem with RTC groups: a buddy can be added to multiple groups.
// This doesn't work with Miranda's way (one group per contact).
// That's why the group support is currently turned off.
//
// #define RTC_GROUPS_SUPPORT
//--------------------------------------------------------------------------------------------------

#include <shellapi.h>
#include "m_system.h"
#include "m_utils.h"
#include "m_options.h"
#include "m_clist.h"
#include "m_chat.h"
#include "SDK/m_updater.h"

#include "RtcClient.h"
#include "RtcHelper.h"
#include "clientobserver.h"
#include "message.h"
#include "authreq.h"
#include "resource.h"
#include "firstrun.h"
#include "version.h"
#include "search.h"
#include "ctx.h"
#include "callservices.h"
#include "ui/optionsdlg.h"
#include "ui/clistframe.h"
//--------------------------------------------------------------------------------------------------

#define MS_SIP_START_APPSHARING "StartAppSharing"
#define MS_SIP_START_WHITEBOARD "StartWhiteboard"
#define MS_SIP_CALL             "Call"
#define MS_SIP_HANGUP           "HangUp"
//--------------------------------------------------------------------------------------------------

#define MIID_SIPRTC    {0xb393074f, 0xf887, 0x4619, {0xa0, 0x94, 0x69, 0xeb, 0xce, 0xcb, 0x22, 0x55}}
//--------------------------------------------------------------------------------------------------

class CSipRtcPlugin : public CMirandaProtocolPlugin<CSipRtcPlugin>
{
public:
    BEGIN_PLUGIN_INFO(CSipRtcPlugin)
        PLUGIN_NAME         ("SIP RTC Protocol")
        PLUGIN_VERSION      (PLUGIN_MAKE_VERSION(SIPRTC_VERSION_V1,SIPRTC_VERSION_V2,SIPRTC_VERSION_V3,SIPRTC_VERSION_V4))
        PLUGIN_DESCRIPTION  ("Provides support for SIP protocol")
        PLUGIN_AUTHOR       ("Paul Shmakov, Zhao Wang")
        PLUGIN_EMAIL        ("paul.shmakov@gmail.com")
        PLUGIN_COPYRIGHT    ("© 2007 Paul Shmakov, Zhao Wang")
        PLUGIN_HOMEPAGE     ("http://forums.miranda-im.org/showthread.php?t=9282")
        PLUGIN_FLAGS        (UNICODE_AWARE)
        PLUGIN_UUID         (MIID_SIPRTC)
    END_PLUGIN_INFO()

    BEGIN_PLUGIN_INTERFACES(CSipRtcPlugin)
        PLUGIN_INTERFACE(MIID_PROTOCOL)
    END_PLUGIN_INTERFACES()

                        CSipRtcPlugin(void);

    int                 OnLoad(void);
    int                 OnUnload(void);

    const char*         GetProtocolName(void) const;

    BEGIN_PROTO_SERVICES_MAP(CSipRtcPlugin)
        PROTO_SERVICE(PS_GETCAPS,               SvcGetCaps)
        PROTO_SERVICE(PS_GETNAME,               SvcGetHumanReadableName)
        PROTO_SERVICE(PS_LOADICON,              SvcLoadIcons)

        PROTO_SERVICE_MTHREAD(PS_GETSTATUS,     SvcGetStatus)
        PROTO_SERVICE_MTHREAD(PS_SETSTATUS,     SvcSetStatus)
        PROTO_SERVICE_MTHREAD(PS_SETAWAYMSG,    SvcSetAwayMessage)

        PROTO_SERVICE_MTHREAD(PSS_MESSAGE,      SvcSendMessage)
        PROTO_SERVICE_MTHREAD(PSS_USERISTYPING, SvcSendUserIsTyping)
        PROTO_SERVICE(PSR_MESSAGE,              SvcReceiveMessage)
        PROTO_SERVICE(PSR_AUTH,                 SvcReceiveAuthRequest)

        PROTO_SERVICE_MTHREAD(PS_BASICSEARCH,   SvcBasicSearch)
        PROTO_SERVICE_MTHREAD(PS_SEARCHBYEMAIL, SvcSearchByEmail)
        PROTO_SERVICE_MTHREAD(PS_SEARCHBYNAME,  SvcSearchByName)

        PROTO_SERVICE_MTHREAD(PS_ADDTOLIST,     SvcAddToListFromSearch)
        PROTO_SERVICE_MTHREAD(PS_ADDTOLISTBYEVENT, SvcAddToListByEvent)
        PROTO_SERVICE_MTHREAD(PSS_ADDED,        SvcOnContactAdded)

        PROTO_SERVICE_MTHREAD(PSS_GETINFO,      SvcGetContactInfo)

        PROTO_SERVICE_MTHREAD(PS_AUTHALLOW,     SvcAuthAllow)
        PROTO_SERVICE_MTHREAD(PS_AUTHDENY,      SvcAuthDeny)
        PROTO_SERVICE_MTHREAD(PSS_SETAPPARENTMODE, SvcSetApparentMode)

        //
        // Contact menu handlers
        //
        PROTO_SERVICE_MTHREAD(MS_SIP_START_APPSHARING, SvcStartAppSharing)
        PROTO_SERVICE_MTHREAD(MS_SIP_START_WHITEBOARD, SvcStartWhiteboard)
        PROTO_SERVICE_MTHREAD(MS_SIP_CALL,             SvcCallBuddy)
        PROTO_SERVICE_MTHREAD(MS_SIP_HANGUP,           SvcHangUp)

        CHAIN_PROTO_SERVICES_MAP(m_callServices)
    END_PROTO_SERVICES_MAP()

private:
    int                 EvtOnModulesLoaded(WPARAM wparam, LPARAM lparam);
    int                 EvtOnPreShutdown(WPARAM wparam, LPARAM lparam);
    int                 EvtOnShutdown(WPARAM wparam, LPARAM lparam);
    int                 EvtOnOptionsInitialize(WPARAM wparam, LPARAM lparam);
    int                 EvtOnDbSettingChanged(HANDLE hContact, DBCONTACTWRITESETTING* cws);

    int                 SvcGetCaps(int flag, LPARAM);
    int                 SvcGetHumanReadableName(int cbName, char* szName);
    HICON               SvcLoadIcons(int witchIcon, LPARAM);

    int                 SvcGetStatus(WPARAM, LPARAM);
    int                 SvcSetStatus(int newStatus, LPARAM);
    int                 SvcSetAwayMessage(int status, const char* message);

    HANDLE              SvcSendMessage(WPARAM, CCSDATA* ccsdata);
    int                 SvcSendUserIsTyping(HANDLE hContact, int typing);
    int                 SvcReceiveMessage(WPARAM, CCSDATA* ccsdata);
    int                 SvcReceiveAuthRequest(WPARAM, CCSDATA* ccsdata);

    HANDLE              SvcBasicSearch(WPARAM, const char* uri);
    HANDLE              SvcSearchByEmail(WPARAM, const char* email);
    HANDLE              SvcSearchByName(WPARAM, const PROTOSEARCHBYNAME* sbn);

    HANDLE              SvcAddToListFromSearch(int flags, SIPRTC_SEARCH_RESULT* psr);
    HANDLE              SvcAddToListByEvent(int flags, HANDLE hDbEvent);
    int                 SvcOnContactAdded(WPARAM, CCSDATA* ccs);
    int                 EvtOnContactDeleted(WPARAM wparam, LPARAM lparam);

    int                 SvcGetContactInfo(WPARAM, CCSDATA* ccs);

    int                 SvcAuthAllow(HANDLE hDbEvent, WPARAM);
    int                 SvcAuthDeny(HANDLE hDbEvent, const char* reason);
    bool                AuthorizeWatcher(HANDLE hDbEvent, bool allow);
    int                 SvcSetApparentMode(WPARAM, CCSDATA* ccs);

    int                 SvcStartAppSharing(HANDLE hContact, HWND hCListWnd);
    int                 SvcStartWhiteboard(HANDLE hContact, HWND hCListWnd);
    int                 SvcCallBuddy(HANDLE hContact, HWND hCListWnd);
    int                 SvcHangUp(HANDLE hContact, HWND hCListWnd);

    void                RegisterInUpdater(void);
    void                RegisterContactMenu(void);
    void                RegisterInChatApi(void);

private:
    CRtcClientObserver  m_clientObserver;
    CRtcClient          m_client;
    CCallServices       m_callServices;
    CListFrameDialog    m_clistFrame;
};
//--------------------------------------------------------------------------------------------------

inline CSipRtcPlugin::CSipRtcPlugin(void) :
    m_client(&m_clientObserver),
    m_callServices(m_client)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline const char* CSipRtcPlugin::GetProtocolName(void) const
{
    return "SIP";
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::OnLoad(void)
{
    // COM Initialization
    // We use the apartment threaded model because RTC objects are not thread safe.
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    //
    // Initializing the common environment
    //
    g_env.SetProtocolName(GetProtocolName());
    g_env.SetInstance(Instance());

    g_env.DB().Upgrade();

	// [notrom, 20071202] The first line of SipRtc.log. This call was in the
	// construction function of CSipRtcTrace, but accessing the database for
	// parameter "EnableLogFile" is impossible there.
	g_env.Trace().Write(L"SipRtc trace started. CSipRtcTrace level: %d", g_env.Trace().GetLevel());

    try
    {
        char mirandaVersion[MAX_PATH] = { 0 };
        MTLVERIFY(S_OK == CallService(MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, (LPARAM)mirandaVersion));

        char versionText[MAX_PATH] = { 0 };
        StringCbPrintfA(versionText, sizeof(versionText), "Miranda IM %s (SIP %d.%d.%d.%d %c)",
            mirandaVersion,
            SIPRTC_VERSION_V1,
            SIPRTC_VERSION_V2,
            SIPRTC_VERSION_V3,
            SIPRTC_VERSION_V4,
#if defined(UNICODE)
            'U'
#else
            'A'
#endif
        );

        char profilePath[MAX_PATH] = { 0 };
        MTLVERIFY(S_OK == CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)profilePath));

        CActivationContext activationContext(Instance());

        if(!m_client.Initialize(versionText, profilePath))
        {
            if(IDRETRY == MessageBoxW(0,
                TranslateW(L"Cannot initialize the SIP Protocol because\nMicrosoft RTC (Real-time Communications) 1.3 is not installed on this computer.\nClick Retry to get more information."),
                TranslateW(L"SIP Protocol"), MB_RETRYCANCEL | MB_ICONERROR))
            {
                CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)GetPluginInfo()->homepage);
            }

            return E_FAIL;
        }
    }
    catch(_com_error& e)
    {
        wchar_t message[255];
        StringCbPrintfW(message, sizeof(message), L"SIP RTC protocol initialization failed!\nError: %x, %s",
            e.Error(), static_cast<const wchar_t*>(e.Description()));
        m_clientObserver.OnError(message);
        return E_FAIL;
    }

    RegisterProtoServices();

    HookEvent(ME_SYSTEM_MODULESLOADED,  &CSipRtcPlugin::EvtOnModulesLoaded);
    HookEvent(ME_SYSTEM_SHUTDOWN,       &CSipRtcPlugin::EvtOnShutdown);
    HookEvent(ME_SYSTEM_PRESHUTDOWN,    &CSipRtcPlugin::EvtOnPreShutdown);
    HookEvent(ME_DB_CONTACT_DELETED,    &CSipRtcPlugin::EvtOnContactDeleted);
    HookEvent(ME_OPT_INITIALISE,        &CSipRtcPlugin::EvtOnOptionsInitialize);

#if defined(RTC_GROUPS_SUPPORT)
    HookEvent(ME_DB_CONTACT_SETTINGCHANGED, &CSipRtcPlugin::EvtOnDbSettingChanged);
#endif

    CAVSessionController::Initialize();

    g_env.DB().SetAllContactsStatusToOffline();

    CFirstRun firstRun;

    return S_OK;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::OnUnload(void)
{
    ::CoUninitialize();
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::EvtOnModulesLoaded(WPARAM, LPARAM)
{
    // TODO: m_clistFrame.Initialize();

    RegisterInUpdater();

    RegisterContactMenu();

    RegisterInChatApi();

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::EvtOnPreShutdown(WPARAM, LPARAM)
{
    RTC_TRY
        m_client.PrepareForShutdown();
    RTC_CATCH

    RTC_TRY
        m_client.Terminate();
    RTC_CATCH

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::EvtOnShutdown(WPARAM wparam, LPARAM lparam)
{

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::EvtOnOptionsInitialize(WPARAM addInfo, LPARAM)
{
    static COptionsDialog optionsDialog;

    OPTIONSDIALOGPAGE page = { 0 };

    page.cbSize     = sizeof(page);
    page.ptszTitle  = L"SIP";
    page.hInstance  = Instance();
    page.flags      = ODPF_UNICODE;
    page.ptszGroup  = L"Network";
    page.pfnDlgProc = optionsDialog.GetDLGPROC();
    page.pszTemplate = MAKEINTRESOURCEA(COptionsDialog::IDD);

    CallService(MS_OPT_ADDPAGE, addInfo, (LPARAM)&page);

    return 0;
}
//--------------------------------------------------------------------------------------------------

#if defined(RTC_GROUPS_SUPPORT)

inline int CSipRtcPlugin::EvtOnDbSettingChanged(HANDLE hContact, DBCONTACTWRITESETTING* cws)
{
    //
    // Look for contact's settings changes
    //
    if(!m_client.IsLoggedOn())
    {
        return 0;
    }

    if(0 == hContact && !lstrcmpA(cws->szModule, "CListGroups"))
    {
        try
        {
            if(DBVT_DELETED == cws->value.type)
            {
                g_env.Trace().WriteVerbose(L"Group deleted from CList, deleting it from server");

                // TODO: Implement group deletion
            }
            else
            {
                g_env.Trace().WriteVerbose(L"Group added or renamed, updating server");

                bstr_t groupName;
                if(DBVT_WCHAR == cws->value.type)
                {
                    groupName = cws->value.pwszVal + 1;
                }
                else if(DBVT_ASCIIZ == cws->value.type)
                {
                    groupName = cws->value.pszVal + 1;
                }
                else
                {
                    groupName = g_env.DB().GetMySettingWString("CListGroups", cws->szSetting);
                    if(groupName.length() > 0)
                    {
                        groupName = static_cast<const wchar_t*>(groupName) + 1;
                    }
                }

                MTLASSERT(groupName.length() > 0);

                if(groupName.length() > 0)
                {
                    m_client.AddGroup(groupName);
                }
            }
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to modify server group: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }

        return 0;
    }

    if(hContact && 0 == lstrcmpA(cws->szModule, "CList"))
    {
        const char* proto = (const char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
        if(proto && 0 == lstrcmpA(proto, GetProtocolName()) && 0 == lstrcmpA(cws->szSetting, "Group"))
        {
            try
            {
                bstr_t uri = g_env.DB().GetContactSettingWString(hContact, "uri");
                MTLASSERT(uri.length() > 0);


                if(DBVT_DELETED == cws->value.type)
                {
                     g_env.Trace().WriteVerbose(L"Contact removed from group, updating server");

                     // TODO: Remove contact from group
                }
                else
                {
                    g_env.Trace().WriteVerbose(L"Contact added to group, updating server");

                    bstr_t group(L"");

                    if(DBVT_ASCIIZ == cws->value.type)
                    {
                        group = cws->value.pszVal;
                    }
                    else if(DBVT_WCHAR == cws->value.type)
                    {
                        group = cws->value.pwszVal;
                    }
                    else
                    {
                        group = g_env.DB().GetContactSettingWString(hContact, cws->szModule, cws->szSetting);
                    }

                    MTLASSERT(group.length() > 0);

                    if(group.length() > 0 && uri.length() > 0)
                    {
                        m_client.AddGroup(group); // TODO: Not sure that this should be here
                        m_client.AddBuddyToGroup(group, uri);
                    }
                }
            }
            catch(_com_error& e)
            {
                g_env.Trace().WriteError(L"Failed to modify buddy membership: %X (%s)", e.Error(),
                    static_cast<const wchar_t*>(e.Description()));
            }

            return 0;
        }
    }

    return 0;
}
//--------------------------------------------------------------------------------------------------

#endif // RTC_GROUPS_SUPPORT

inline int CSipRtcPlugin::SvcGetCaps(int flag, LPARAM)
{
    int res = 0;
    switch(flag)
    {
    case PFLAGNUM_1:
        res = PF1_IM | PF1_MODEMSG | PF1_BASICSEARCH | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME |
              PF1_ADDSEARCHRES | PF1_VISLIST | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_CHAT;
        break;

    case PFLAGNUM_2:
        res = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_IDLE |
              PF2_LIGHTDND | PF2_ONTHEPHONE | PF2_OUTTOLUNCH;
        break;

    case PFLAGNUM_3:
        res = PF2_SHORTAWAY;
        break;

    case PFLAGNUM_4:
        res = PF4_SUPPORTTYPING | PF4_FORCEADDED | PF4_FORCEAUTH | PF4_NOCUSTOMAUTH;
        break;

    case PFLAG_UNIQUEIDTEXT:
        res = PtrToInt("SIP Address");
        break;

    case PFLAG_MAXCONTACTSPERPACKET:
        res = 20;
        break;

    case PFLAG_UNIQUEIDSETTING:
        res = PtrToInt("uri");
        break;

    case PFLAG_MAXLENOFMESSAGE:
        res = 1000;
        break;
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcGetHumanReadableName(int cbName, char* szName)
{
    int res = E_FAIL;
    if(szName && cbName)
    {
        res = (int)StringCchCopyA(szName, cbName, "SIP");
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcGetStatus(WPARAM, LPARAM)
{
    // Actually it can leave in any thread
    // MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());

    int status = ID_STATUS_OFFLINE;

    RTC_TRY

    status = MyRtcPresenceStatusToMirandaStatus(m_client.GetPresenceStatus(), m_client.IsLoggedOn());

    RTC_CATCH

    //g_env.Trace().WriteVerbose(L"GetStatus returning: %d", status);

    return status;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcSetStatus(int newStatus, LPARAM)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());

    RTC_TRY

    if(ID_STATUS_OFFLINE == newStatus)
    {
        m_client.LogOff();
    }
    else
    {
        m_client.SetPresenceStatus(MyMirandaStatusToRtcPresenceStatus(newStatus));
    }

    RTC_CATCH

    return S_OK;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcSetAwayMessage(int status, const char* message)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());

    RTC_TRY

    if(ID_STATUS_OFFLINE != status)
    {
        m_client.SetPresenceNotes(MyMirandaStatusToRtcPresenceStatus(status), bstr_t(message));
    }

    RTC_CATCH

    return S_OK;
}
//--------------------------------------------------------------------------------------------------

inline HICON CSipRtcPlugin::SvcLoadIcons(int whichIcon, LPARAM)
{
    return (HICON)LoadImage(Instance(), MAKEINTRESOURCE(IDI_SIPRTC), IMAGE_ICON,
        GetSystemMetrics(whichIcon & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
        GetSystemMetrics(whichIcon & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}
//--------------------------------------------------------------------------------------------------

inline HANDLE CSipRtcPlugin::SvcSendMessage(WPARAM, CCSDATA* ccs)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(ccs);

    HANDLE hProcess = 0;

    MTLASSERT(ccs->hContact);

    bstr_t uri  = g_env.DB().GetContactSettingWString(ccs->hContact, "uri");
    bstr_t name = g_env.DB().GetContactSettingWString(ccs->hContact, "Nick");

    MTLASSERT(uri.length() > 0);
    if(0 == uri.length())
    {
        ProtoAsyncBroadcastAck(GetProtocolName(), ccs->hContact,
            ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)1, 0 );
        return 0;
    }

    const char* buffer = (char*)ccs->lParam;
    bstr_t message;

    if(ccs->wParam & PREF_UNICODE)
    {
        message = (const wchar_t*)&buffer[lstrlenA(buffer) + 1];
    }
    else
    {
        message = buffer;
    }

    MTLASSERT(message.length() > 0);

    if(message.length() > 0)
    {
        long cookie = reinterpret_cast<long>(ccs->hContact); //GenerateUniqueCookie();
        hProcess = (HANDLE)cookie;

        try
        {
            m_client.SendIMMessage(uri, name, message, cookie);
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to send a message to %s: %X (%s)",
                static_cast<const wchar_t*>(uri),  e.Error(),
                static_cast<const wchar_t*>(e.Description()));

            ProtoAsyncBroadcastAck(GetProtocolName(), ccs->hContact,
                ACKTYPE_MESSAGE, ACKRESULT_FAILED, hProcess, 0);
        }
    }

    return hProcess;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcSendUserIsTyping(HANDLE hContact, int typing)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());

    MTLASSERT(hContact);

    bstr_t uri  = g_env.DB().GetContactSettingWString(hContact, "uri");

    MTLASSERT(uri.length() > 0);
    if(0 == uri.length())
    {
        g_env.Trace().WriteError(L"Failed to get URI of %X", hContact);
        return 1;
    }

    bool isTyping = PROTOTYPE_SELFTYPING_ON == typing;

    try
    {
        m_client.SendUserIsTyping(uri, isTyping);
    }
    catch(_com_error& e)
    {
        g_env.Trace().WriteError(L"Failed to send `UserIsTyping`: %X (%s)", e.Error(),
            static_cast<const wchar_t*>(e.Description()));
        return 1;
    }

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcReceiveMessage(WPARAM, CCSDATA* ccs)
{
    MTLASSERT(ccs);

    PROTORECVEVENT* pre = (PROTORECVEVENT*)ccs->lParam;
    MTLASSERT(pre);

    DBEVENTINFO dbei = { 0 };
    dbei.cbSize = sizeof(dbei);
    dbei.szModule = const_cast<char*>(GetProtocolName());
    dbei.timestamp = pre->timestamp;
    dbei.flags = pre->flags & PREF_CREATEREAD ? DBEF_READ : 0;
    dbei.eventType = EVENTTYPE_MESSAGE;
    dbei.cbBlob = lstrlenA(pre->szMessage) + 1;
    if(pre->flags & PREF_UNICODE)
    {
        dbei.cbBlob *= (sizeof(wchar_t) + 1);
    }

    dbei.pBlob = (PBYTE)pre->szMessage;

    CallService(MS_DB_EVENT_ADD, (WPARAM)ccs->hContact, (LPARAM)&dbei);

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcReceiveAuthRequest(WPARAM, CCSDATA* ccs)
{
    MTLASSERT(ccs);

    PROTORECVEVENT* pre = (PROTORECVEVENT*)ccs->lParam;
    MTLASSERT(pre);

    DBEVENTINFO dbei = { 0 };
    dbei.cbSize = sizeof(dbei);
    dbei.szModule = const_cast<char*>(GetProtocolName());
    dbei.timestamp = pre->timestamp;
    dbei.flags = pre->flags & PREF_CREATEREAD ? DBEF_READ : 0;
    dbei.eventType = EVENTTYPE_AUTHREQUEST;
    dbei.cbBlob = (DWORD)pre->lParam;
    dbei.pBlob = (PBYTE)pre->szMessage;

    CallService(MS_DB_EVENT_ADD, (WPARAM)ccs->hContact, (LPARAM)&dbei);

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline HANDLE CSipRtcPlugin::SvcBasicSearch(WPARAM, const char* uri)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(uri);
    MTLASSERT(uri[0]);

    HANDLE hProcess = 0;

    if(uri && uri[0])
    {
        long cookie = GenerateUniqueCookie();

        //
        // Cheeky search - finds everything when Ctrl is hold down
        //
        if(GetKeyState(VK_CONTROL) & 0x8000)
        {
            hProcess = StartCheekyBasicSearch(uri, cookie);
        }
        else
        {
            //
            // Real, normal search
            //
            try
            {
                std::vector<SearchTerm> terms;
                terms.push_back(SearchTerm(bstr_t(L"msRTCURI"), bstr_t(uri)));

                m_client.SearchContact(terms, cookie);

                hProcess = (HANDLE)cookie;
            }
            catch(_com_error& e)
            {
                g_env.Trace().WriteError(L"Search failed: %X (%s)", e.Error(),
                    static_cast<const wchar_t*>(e.Description()));
            }

            if(!hProcess)
                hProcess = StartCheekyBasicSearch(uri, cookie);
        }
    }

    return hProcess;
}
//--------------------------------------------------------------------------------------------------

inline HANDLE CSipRtcPlugin::SvcSearchByEmail(WPARAM, const char* email)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(email);
    MTLASSERT(email[0]);

    HANDLE hProcess = 0;

    if(email && email[0])
    {
        try
        {
            long cookie = GenerateUniqueCookie();
            std::vector<SearchTerm> terms;
            terms.push_back(SearchTerm(bstr_t(L"msRTCMail"), bstr_t(email)));

            m_client.SearchContact(terms, cookie);

            hProcess = (HANDLE)cookie;
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Search by email failed: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }

    return hProcess;
}
//--------------------------------------------------------------------------------------------------

inline HANDLE CSipRtcPlugin::SvcSearchByName(WPARAM, const PROTOSEARCHBYNAME* sbn)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(sbn);

    HANDLE hProcess = 0;

    try
    {
        long cookie = GenerateUniqueCookie();

        std::vector<SearchTerm> terms;

        if(sbn->pszNick && sbn->pszNick[0])
        {
            terms.push_back(SearchTerm(bstr_t(L"displayName"), bstr_t(sbn->pszNick)));
        }

        if(sbn->pszFirstName && sbn->pszFirstName[0])
        {
            terms.push_back(SearchTerm(bstr_t(L"givenName"), bstr_t(sbn->pszFirstName)));
        }

        if(sbn->pszLastName && sbn->pszLastName[0])
        {
            terms.push_back(SearchTerm(bstr_t(L"sn"), bstr_t(sbn->pszLastName)));
        }

        if(!terms.empty())
        {
           m_client.SearchContact(terms, cookie);

           hProcess = (HANDLE)cookie;
        }
    }
    catch(_com_error& e)
    {
        g_env.Trace().WriteError(L"Search by name failed: %X (%s)", e.Error(),
            static_cast<const wchar_t*>(e.Description()));
    }

    return hProcess;
}
//--------------------------------------------------------------------------------------------------

inline HANDLE CSipRtcPlugin::SvcAddToListFromSearch(int flags, SIPRTC_SEARCH_RESULT* psr)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(psr);
    MTLASSERT(sizeof(SIPRTC_SEARCH_RESULT) == psr->cbSize);
    MTLASSERT(lstrlenW(psr->uri) > 0);

    HANDLE hContact = 0;

    if(sizeof(SIPRTC_SEARCH_RESULT) == psr->cbSize && lstrlenW(psr->uri) > 0)
    {
        hContact = g_env.DB().FindContact(psr->uri);

        if(!hContact || g_env.DB().IsContactTemporary(hContact))
        {
            try
            {
                bstr_t uri  = psr->uri;
                bstr_t nick = psr->nick ? psr->nick : "";
                if(0 == nick.length())
                {
                    nick = ComposeNickByUri(uri);
                }

                m_client.AddBuddy(uri, nick, PALF_TEMPORARY & flags);

                //
                // TODO: It can take a lot of time before the buddy is really added.
                //
                Sleep(500);

                hContact = g_env.DB().FindOrAddContact(psr->uri, nick, PALF_TEMPORARY & flags);
                MTLASSERT(0 != hContact);
            }
            catch(_com_error& e)
            {
                g_env.Trace().WriteError(L"Failed to add contact: %X (%s)", e.Error(),
                    static_cast<const wchar_t*>(e.Description()));
            }
        }
    }

    return hContact;
}
//--------------------------------------------------------------------------------------------------

inline HANDLE CSipRtcPlugin::SvcAddToListByEvent(int flags, HANDLE hDbEvent)
{
    MTLASSERT(hDbEvent);

    HANDLE hContact = 0;

    DBEVENTINFO dbei = { 0 };
    dbei.cbSize = sizeof(dbei);

    if((dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0)) > 0)
    {
        dbei.pBlob = (PBYTE)_alloca(dbei.cbBlob + 1);
        dbei.pBlob[dbei.cbBlob] = 0;

        if(S_OK == CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei) &&
            0 == lstrcmpA(dbei.szModule, GetProtocolName()) &&
            (EVENTTYPE_AUTHREQUEST == dbei.eventType || EVENTTYPE_ADDED == dbei.eventType))
        {
            CAuthRequestBlob blob(dbei.pBlob, dbei.cbBlob);
            hContact = blob.GetHContact();
            bstr_t nick = blob.GetNick();

            bstr_t uri = g_env.DB().GetContactSettingWString(hContact, "uri");

            MTLASSERT(uri == blob.GetURI());

            if(uri.length() > 0)
            {
                SIPRTC_SEARCH_RESULT sr;
                ZeroMemory(&sr, sizeof(sr));
                sr.cbSize = sizeof(sr);
                StringCbCopyW(sr.uri, sizeof(sr.uri), uri);
                sr.nick = nick;

                hContact = SvcAddToListFromSearch(flags, &sr);
            }
        }
    }

    return hContact;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcOnContactAdded(WPARAM, CCSDATA* ccs)
{
    g_env.Trace().WriteVerbose(L"PSS_ADDED event");

    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(ccs);

    MTLASSERT(ccs->hContact);

    if(g_env.DB().IsContactTemporary(ccs->hContact))
    {
        bstr_t uri = g_env.DB().GetContactSettingWString(ccs->hContact, "uri");

        MTLASSERT(uri.length() > 0);
        if(uri.length() > 0)
        {
            try
            {
                m_client.AddBuddy(uri, ComposeNickByUri(uri));
            }
            catch(_com_error& e)
            {
                g_env.Trace().WriteError(L"PSS_ADDED failed: %X (%s)", e.Error(),
                    static_cast<const wchar_t*>(e.Description()));
            }
        }
        else
            g_env.Trace().WriteVerbose(L"The contact doesn't have URI");
    }
    else
        g_env.Trace().WriteVerbose(L"The contact is not temporary - skip");

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::EvtOnContactDeleted(WPARAM wparam, LPARAM lparam)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(wparam);

    HANDLE hContact = (HANDLE)wparam;

    const char* proto = reinterpret_cast<const char*>(CallService(MS_PROTO_GETCONTACTBASEPROTO, wparam, 0));
    if(!proto || lstrcmpA(proto, GetProtocolName()))
    {
        return 0;
    }

    bstr_t uri = g_env.DB().GetContactSettingWString(hContact, "uri");

    MTLASSERT(uri.length() > 0);

    if(uri.length() > 0)
    {
        try
        {
            m_client.RemoveBuddy(uri);
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to remove contact: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }

    return 0;
}
//--------------------------------------------------------------------------------------------------


inline int CSipRtcPlugin::SvcGetContactInfo(WPARAM, CCSDATA* ccs)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(ccs);

    MTLASSERT(ccs->hContact);

    int res = E_FAIL;

    bstr_t uri = g_env.DB().GetContactSettingWString(ccs->hContact, "uri");

    MTLASSERT(uri.length() > 0);
    if(0 == uri.length())
    {
        return 1;
    }

    try
    {
        m_client.RefreshBuddy(uri);

        res = S_OK;
    }
    catch(_com_error& e)
    {
        g_env.Trace().WriteError(L"GetInfo failed: %X (%s)", e.Error(),
            static_cast<const wchar_t*>(e.Description()));
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline bool CSipRtcPlugin::AuthorizeWatcher(HANDLE hDbEvent, bool allow)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(hDbEvent);

    if(!m_client.IsLoggedOn())
        return false;

    bool res = false;

    DBEVENTINFO dbei = { 0 };
    dbei.cbSize = sizeof(dbei);

    if((dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0)) > 0)
    {
        dbei.pBlob = (PBYTE)_alloca(dbei.cbBlob + 1);
        dbei.pBlob[dbei.cbBlob] = 0;

        if(S_OK == CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei) &&
            0 == lstrcmpA(dbei.szModule, GetProtocolName()) &&
            EVENTTYPE_AUTHREQUEST == dbei.eventType)
        {
            CAuthRequestBlob blob(dbei.pBlob, dbei.cbBlob);

            bstr_t uri = g_env.DB().GetContactSettingWString(blob.GetHContact(), "uri");

            MTLASSERT(uri == blob.GetURI());

            if(uri.length() > 0)
            {
                try
                {
                    m_client.AuthorizeWatcher(uri, allow);
                    res = true;
                }
                catch(_com_error& e)
                {
                    g_env.Trace().WriteError(L"AuthorizeBuddy failed: %X (%s)", e.Error(),
                        static_cast<const wchar_t*>(e.Description()));
                }
            }
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcAuthAllow(HANDLE hDbEvent, WPARAM)
{
    return AuthorizeWatcher(hDbEvent, true) ? S_OK : E_FAIL;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcAuthDeny(HANDLE hDbEvent, const char*)
{
    return AuthorizeWatcher(hDbEvent, false) ? S_OK : E_FAIL;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcSetApparentMode(WPARAM, CCSDATA* ccs)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(ccs);

    int res = E_FAIL;

    int statusMode = (int)ccs->wParam;

    MTLASSERT(0 == statusMode || ID_STATUS_OFFLINE == statusMode);
    if(0 != statusMode && ID_STATUS_OFFLINE != statusMode) return 1;

    int oldMode = g_env.DB().GetContactSettingInt(ccs->hContact, "ApparentMode", 0);
    if(statusMode == oldMode)
    {
        res = S_OK;
    }
    else
    {
        g_env.DB().WriteContactSettingInt(ccs->hContact, "ApparentMode", statusMode);

        if(m_client.IsLoggedOn())
        {
            bstr_t uri = g_env.DB().GetContactSettingWString(ccs->hContact, "uri");
            MTLASSERT(uri.length() > 0);

            try
            {
                m_client.AuthorizeWatcher(uri, ID_STATUS_OFFLINE != statusMode);
                res = S_OK;
            }
            catch(_com_error& e)
            {
                res = e.Error();
                g_env.Trace().WriteError(L"Failed to change appatent mode: %X (%s)", e.Error(),
                    static_cast<const wchar_t*>(e.Description()));
            }
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcStartAppSharing(HANDLE hContact, HWND hCListWnd)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(hContact);

    if(m_client.IsLoggedOn())
    {
        bstr_t uri = g_env.DB().GetContactSettingWString(hContact, "uri");
        MTLASSERT(uri.length() > 0);
        bstr_t nick = g_env.DB().GetContactSettingWString(hContact, "Nick");

        try
        {
            ISessionController* controller =
                new CT120SessionController(RTCTA_APPSHARING);

            m_client.EstablishPcToPcSession(controller, uri, nick);
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to start AppSharing: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcStartWhiteboard(HANDLE hContact, HWND hCListWnd)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(hContact);

    if(m_client.IsLoggedOn())
    {
        bstr_t uri = g_env.DB().GetContactSettingWString(hContact, "uri");
        MTLASSERT(uri.length() > 0);
        bstr_t nick = g_env.DB().GetContactSettingWString(hContact, "Nick");

        try
        {
            ISessionController* controller =
                new CT120SessionController(RTCTA_WHITEBOARD);

            m_client.EstablishPcToPcSession(controller, uri, nick);
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to start Whiteboard: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcCallBuddy(HANDLE hContact, HWND hCListWnd)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());
    MTLASSERT(hContact);

    SIPRTC_CALL call = { 0 };
    call.cbSize = sizeof(call);
    call.hContact = hContact;
    CallProtoService(GetProtocolName(), MS_SIPRTC_CALL, 0, (LPARAM)&call);

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline int CSipRtcPlugin::SvcHangUp(HANDLE hContact, HWND hCListWnd)
{
    MTLASSERT(GetCurrentThreadId() == g_pluginLink.GetMainThreadId());

    CallProtoService(GetProtocolName(), MS_SIPRTC_TERMINATE_CALL, 0, 0);

    return 0;
}
//--------------------------------------------------------------------------------------------------


inline void CSipRtcPlugin::RegisterInUpdater(void)
{
    const int fileId = 3455;
    CallService(MS_UPDATE_REGISTERFL, fileId, (LPARAM)GetPluginInfo());

    /*
    Update update = { 0 };

    update.cbSize = sizeof(Update);
    update.szComponentName = GetPluginInfo()->shortName;

    char szVersion[16];
    update.pbVersion = (BYTE*)CreateVersionStringPlugin(GetPluginInfo(), szVersion);
    update.cpbVersion = lstrlenA((const char*)update.pbVersion);

    update.szBetaUpdateURL    = "http://potapochkin.ru/files/paul/siprtc.zip";
    update.szBetaVersionURL   = "http://potapochkin.ru/files/paul/siprtc_version.txt";
    update.szBetaChangelogURL = "http://potapochkin.ru/files/paul/siprtc_changelog.txt";
    update.pbBetaVersionPrefix = (BYTE*)"SIP version ";
    update.cpbBetaVersionPrefix = lstrlenA((const char*)update.pbBetaVersionPrefix);

    CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
    */
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcPlugin::RegisterContactMenu(void)
{
    //
    // Contact menu
    //
    char serviceFunction[100];
    unsigned bufSize = sizeof(serviceFunction) / sizeof(serviceFunction[0]);
    StringCchCopyA(serviceFunction, bufSize, GetProtocolName());
    char* serviceName = serviceFunction + lstrlenA(serviceFunction);
    bufSize -= lstrlenA(serviceFunction);
    CLISTMENUITEM mi;

    StringCchCopyA(serviceName, bufSize, MS_SIP_START_APPSHARING);

    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    mi.flags = 0;
    mi.position = -500050000;
    mi.hIcon = (HICON)LoadImage(Instance(), MAKEINTRESOURCE(IDI_ICON_NETMEETING), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0); // TODO: AppSharing icon
    mi.pszContactOwner = const_cast<char*>(GetProtocolName());
    mi.pszName = Translate("Start &Application Sharing");
    mi.pszService = serviceFunction;
    CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

    StringCchCopyA(serviceName, bufSize, MS_SIP_START_WHITEBOARD);

    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    mi.flags = 0;
    mi.position = -500050001;
    mi.hIcon = (HICON)LoadImage(Instance(), MAKEINTRESOURCE(IDI_ICON_NETMEETING), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0); // TODO: Whiteboard icon
    mi.pszContactOwner = const_cast<char*>(GetProtocolName());
    mi.pszName = Translate("Start &Whiteboard");
    mi.pszService = serviceFunction;
    CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

    StringCchCopyA(serviceName, bufSize, MS_SIP_HANGUP);

    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    mi.flags = 0;
    mi.position = -500050002;
    mi.hIcon = (HICON)LoadImage(Instance(), MAKEINTRESOURCE(IDI_ICON_HANGUP), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    mi.pszContactOwner = const_cast<char*>(GetProtocolName());
    mi.pszName = Translate("&Hang up");
    mi.pszService = serviceFunction;
    CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

    StringCchCopyA(serviceName, bufSize, MS_SIP_CALL);

    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    mi.flags = 0;
    mi.position = -500050003;
    mi.hIcon = (HICON)LoadImage(Instance(), MAKEINTRESOURCE(IDI_ICON_CALL), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    mi.pszContactOwner = const_cast<char*>(GetProtocolName());
    mi.pszName = Translate("&Call");
    mi.pszService = serviceFunction;
    CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
}
//--------------------------------------------------------------------------------------------------

inline void CSipRtcPlugin::RegisterInChatApi(void)
{
    if(ServiceExists(MS_GC_REGISTER))
    {
        GCREGISTER reg = { 0 };
        reg.cbSize = sizeof(reg);
        reg.dwFlags = GC_ACKMSG | GC_TYPNOTIF | GC_UNICODE;
        reg.pszModule = GetProtocolName();
        reg.pszModuleDispName = GetPluginInfo()->shortName;
        reg.iMaxText = SvcGetCaps(PFLAG_MAXLENOFMESSAGE, 0);
        CallService(MS_GC_REGISTER, 0, (LPARAM)&reg);
    }
}
//--------------------------------------------------------------------------------------------------
