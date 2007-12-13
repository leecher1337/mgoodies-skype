/*

SIP RTC Plugin for Miranda IM

Copyright 2006 Paul Shmakov

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

#define WIN32_LEAN_AND_MEAN

#define UNICODE
#define _UNICODE

#define _WIN32_WINNT 0x0501
#define _WIN32_DCOM

//#define ISOLATION_AWARE_ENABLED 1
//--------------------------------------------------------------------------------------------------

#include <windows.h>
#include <atlbase.h>
#include <objbase.h>
#include <comdef.h>
#include <comip.h>
#include <tchar.h>
#define STRSAFE_LIB
#include <strsafe.h>
#include <time.h>
#include "SDK/rtccore.h"
#include "SDK/rtcerr.h"

// ATL/WTL headers
#include <oleauto.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include "dbg.h"
#include "env.h"
//--------------------------------------------------------------------------------------------------

#if defined(_DEBUG)

#define MTLASSERT_BASE(expr, msg, file, lineno) \
    if(!(expr)) \
    { \
        g_env.Trace().WriteError(bstr_t("Assertion failed: " msg "\nFile: " file ", Line: " _CRT_STRINGIZE(lineno))); \
        MessageBoxA(0, "Assertion failed: " msg "\nFile: " file ", Line: " _CRT_STRINGIZE(lineno), "SIP RTC Protocol Error", \
            MB_ICONERROR | MB_OK);\
    }

#else

#define MTLASSERT_BASE(expr, msg, file, lineno) \
    if(!(expr)) \
    { \
        g_env.Trace().WriteError(bstr_t("Assertion failed: " msg "\nFile: " file ", Line: " _CRT_STRINGIZE(lineno))); \
    }

#endif
//--------------------------------------------------------------------------------------------------

#define MTLASSERT(expr)  MTLASSERT_BASE((expr), #expr, __FILE__, __LINE__)
//--------------------------------------------------------------------------------------------------

#define MTLVERIFY(expr)  MTLASSERT_BASE((expr), #expr, __FILE__, __LINE__)
//--------------------------------------------------------------------------------------------------

#include "MTL/mtl.h"
#include "MTL/mtlproto.h"

#include "m_utils.h"
#include "m_database.h"
//--------------------------------------------------------------------------------------------------

_COM_SMARTPTR_TYPEDEF(IRTCClient, __uuidof(IRTCClient));
_COM_SMARTPTR_TYPEDEF(IRTCClient2, __uuidof(IRTCClient2));
_COM_SMARTPTR_TYPEDEF(IRTCProfile, __uuidof(IRTCProfile));
_COM_SMARTPTR_TYPEDEF(IRTCProfile2, __uuidof(IRTCProfile2));
_COM_SMARTPTR_TYPEDEF(IRTCProfile3, __uuidof(IRTCProfile3));
_COM_SMARTPTR_TYPEDEF(IConnectionPointContainer, __uuidof(IConnectionPointContainer));
_COM_SMARTPTR_TYPEDEF(IConnectionPoint, __uuidof(IConnectionPoint));
_COM_SMARTPTR_TYPEDEF(IRTCRegistrationStateChangeEvent, __uuidof(IRTCRegistrationStateChangeEvent));
_COM_SMARTPTR_TYPEDEF(IRTCSessionStateChangeEvent, __uuidof(IRTCSessionStateChangeEvent));
_COM_SMARTPTR_TYPEDEF(IRTCParticipantStateChangeEvent, __uuidof(IRTCParticipantStateChangeEvent));
_COM_SMARTPTR_TYPEDEF(IRTCMessagingEvent, __uuidof(IRTCMessagingEvent));
_COM_SMARTPTR_TYPEDEF(IRTCMediaEvent, __uuidof(IRTCMediaEvent));
_COM_SMARTPTR_TYPEDEF(IRTCMediaRequestEvent, __uuidof(IRTCMediaRequestEvent));
_COM_SMARTPTR_TYPEDEF(IRTCIntensityEvent, __uuidof(IRTCIntensityEvent));
_COM_SMARTPTR_TYPEDEF(IRTCClientEvent, __uuidof(IRTCClientEvent));
_COM_SMARTPTR_TYPEDEF(IRTCBuddyEvent2, __uuidof(IRTCBuddyEvent2));
_COM_SMARTPTR_TYPEDEF(IRTCWatcherEvent2, __uuidof(IRTCWatcherEvent2));
_COM_SMARTPTR_TYPEDEF(IRTCBuddyGroupEvent, __uuidof(IRTCBuddyGroupEvent));
_COM_SMARTPTR_TYPEDEF(IRTCUserSearchResultsEvent, __uuidof(IRTCUserSearchResultsEvent));
_COM_SMARTPTR_TYPEDEF(IRTCRoamingEvent, __uuidof(IRTCRoamingEvent));
_COM_SMARTPTR_TYPEDEF(IRTCProfileEvent2, __uuidof(IRTCProfileEvent2));
_COM_SMARTPTR_TYPEDEF(IRTCPresencePropertyEvent, __uuidof(IRTCPresencePropertyEvent));
_COM_SMARTPTR_TYPEDEF(IRTCPresenceDataEvent, __uuidof(IRTCPresenceDataEvent));
_COM_SMARTPTR_TYPEDEF(IRTCPresenceStatusEvent, __uuidof(IRTCPresenceStatusEvent));
_COM_SMARTPTR_TYPEDEF(IRTCClientPresence, __uuidof(IRTCClientPresence));
_COM_SMARTPTR_TYPEDEF(IRTCClientPresence2, __uuidof(IRTCClientPresence2));
_COM_SMARTPTR_TYPEDEF(IRTCClientProvisioning2, __uuidof(IRTCClientProvisioning2));
_COM_SMARTPTR_TYPEDEF(IRTCBuddy, __uuidof(IRTCBuddy));
_COM_SMARTPTR_TYPEDEF(IRTCBuddy2, __uuidof(IRTCBuddy2));
_COM_SMARTPTR_TYPEDEF(IRTCEnumBuddies, __uuidof(IRTCEnumBuddies));
_COM_SMARTPTR_TYPEDEF(IRTCSession, __uuidof(IRTCSession));
_COM_SMARTPTR_TYPEDEF(IRTCParticipant, __uuidof(IRTCParticipant));
_COM_SMARTPTR_TYPEDEF(IRTCSessionOperationCompleteEvent, __uuidof(IRTCSessionOperationCompleteEvent));
_COM_SMARTPTR_TYPEDEF(IRTCUserSearch, __uuidof(IRTCUserSearch));
_COM_SMARTPTR_TYPEDEF(IRTCUserSearchQuery, __uuidof(IRTCUserSearchQuery));
_COM_SMARTPTR_TYPEDEF(IRTCUserSearchResult, __uuidof(IRTCUserSearchResult));
_COM_SMARTPTR_TYPEDEF(IRTCEnumUserSearchResults, __uuidof(IRTCEnumUserSearchResults));
_COM_SMARTPTR_TYPEDEF(IRTCUserSearchResultsEvent, __uuidof(IRTCUserSearchResultsEvent));
_COM_SMARTPTR_TYPEDEF(IRTCEnumParticipants, __uuidof(IRTCEnumParticipants));
_COM_SMARTPTR_TYPEDEF(IRTCBuddyGroup, __uuidof(IRTCBuddyGroup));
_COM_SMARTPTR_TYPEDEF(IRTCEnumGroups, __uuidof(IRTCEnumGroups));
_COM_SMARTPTR_TYPEDEF(IRTCWatcher, __uuidof(IRTCWatcher));
_COM_SMARTPTR_TYPEDEF(IRTCEnumWatchers, __uuidof(IRTCEnumWatchers));
_COM_SMARTPTR_TYPEDEF(IRTCEnumPresenceDevices, __uuidof(IRTCEnumPresenceDevices));
_COM_SMARTPTR_TYPEDEF(IRTCPresenceDevice, __uuidof(IRTCPresenceDevice));
//--------------------------------------------------------------------------------------------------

#define WM_RTC_EVENT (WM_USER + 16)
//--------------------------------------------------------------------------------------------------

inline void ComCheck(HRESULT hr)
{
    if(FAILED(hr))
    {
        // TODO: Log error
        _com_issue_error(hr);
    }
}
//--------------------------------------------------------------------------------------------------

#define MAX_COMPUTER_NAME_LENGTH 300
//--------------------------------------------------------------------------------------------------

#define RTC_TRY \
    try \
    {
//--------------------------------------------------------------------------------------------------

#define RTC_CATCH                       \
    }                                   \
    catch(_com_error& e)                \
    {                                   \
        m_clientObserver.OnError(e, __FUNCTION__); \
    }
//--------------------------------------------------------------------------------------------------
