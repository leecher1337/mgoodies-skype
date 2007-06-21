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
//--------------------------------------------------------------------------------------------------

// Class to sink RTC events
class CRtcEvents : public IRTCEventNotification
{
public:
                        CRtcEvents(void);

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

    HRESULT             Advise(IRTCClient* pClient, HWND hWnd);
    HRESULT             Unadvise(IRTCClient* pClient);

    HRESULT STDMETHODCALLTYPE Event(RTC_EVENT enEvent, IDispatch* pDisp);

private:
    DWORD               m_refCount;
    DWORD               m_cookie;
    HWND                m_hWnd;
};
//--------------------------------------------------------------------------------------------------

inline CRtcEvents::CRtcEvents() :
    m_refCount(0),
    m_cookie(0),
    m_hWnd(0)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline HRESULT STDMETHODCALLTYPE CRtcEvents::QueryInterface(REFIID iid, void **ppvObject)
{
    if(__uuidof(IRTCEventNotification) == iid || IID_IUnknown == iid)
    {
        *ppvObject = (void*)this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}
//--------------------------------------------------------------------------------------------------

inline ULONG STDMETHODCALLTYPE CRtcEvents::AddRef(void)
{
    ++m_refCount;
    return m_refCount;
}
//--------------------------------------------------------------------------------------------------

inline ULONG STDMETHODCALLTYPE CRtcEvents::Release(void)
{
    --m_refCount;

    if(0 == m_refCount)
    {
        delete this;
    }

    return 1;
}
//--------------------------------------------------------------------------------------------------

inline HRESULT CRtcEvents::Advise(IRTCClient* pClient, HWND hWnd)
{
    IConnectionPointContainerPtr pCPC;
    IConnectionPointPtr pCP;

    // Find the connection point container
    HRESULT hr = pClient->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);

    if(SUCCEEDED(hr))
    {
        // Find the connection point
        hr = pCPC->FindConnectionPoint(__uuidof(IRTCEventNotification), &pCP);

        if(SUCCEEDED(hr))
        {
            // Advise the connection
            hr = pCP->Advise(this, &m_cookie);
        }
    }

    // Store the window handle of the application so we
    // can post messages to it when events are fired
    m_hWnd = hWnd;

    return hr;
}
//--------------------------------------------------------------------------------------------------

inline HRESULT CRtcEvents::Unadvise(IRTCClient* pClient)
{
    IConnectionPointContainerPtr pCPC;
    IConnectionPointPtr pCP;
    HRESULT hr = S_OK;

    if(m_cookie)
    {
        // Find the connection point container
        hr = pClient->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);

        if(SUCCEEDED(hr))
        {
            // Find the connection point
            hr = pCPC->FindConnectionPoint(__uuidof(IRTCEventNotification), &pCP);

            if(SUCCEEDED(hr))
            {
                // Unadvise this connection
                hr = pCP->Unadvise(m_cookie);
            }
        }
    }

    return hr;
}
//--------------------------------------------------------------------------------------------------

inline HRESULT STDMETHODCALLTYPE CRtcEvents::Event(RTC_EVENT enEvent, IDispatch* pDisp)
{
    // We will post a message containing the event to the
    // application window.

    // Add a reference to the event so we can hold onto it while
    // the event is in the message queue.
    pDisp->AddRef();

    // Post the message
    PostMessage(m_hWnd, WM_RTC_EVENT, (WPARAM)enEvent, (LPARAM)pDisp);

    return S_OK;
}
//--------------------------------------------------------------------------------------------------
