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

class CEnumURIs : public IEnumString
{
public:
                        CEnumURIs(void);

    static IEnumString* CreateInstance(void);

    // IUnknown
    IFACEMETHODIMP      QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef(void);
    IFACEMETHODIMP_(ULONG) Release(void);

    // IEnumString
    IFACEMETHODIMP      Next(ULONG celt, LPWSTR* rgelt, ULONG* pceltFetched);
    IFACEMETHODIMP      Skip(ULONG skip);
    IFACEMETHODIMP      Reset(void);
    IFACEMETHODIMP      Clone(IEnumString** ppenum);

private:
                        ~CEnumURIs(void);

private:
    LONG                m_refs;
    unsigned            m_current;
};
//--------------------------------------------------------------------------------------------------

IEnumString* CEnumURIs::CreateInstance(void)
{
    IEnumString* res = 0;

    CEnumURIs* penum = new CEnumURIs();
    penum->QueryInterface(IID_IEnumString, &res);
    penum->Release();

    return res;

}
//--------------------------------------------------------------------------------------------------


CEnumURIs::CEnumURIs(void) :
    m_refs(1),
    m_current(0),
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

CEnumURIs::~CEnumURIs(void)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

STDMETHODIMP CEnumURIs::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = E_NOINTERFACE;

    *ppv = 0;

    if(IID_IUnknown == riid || IID_IEnumString == riid)
    {
        *ppv = static_cast<IEnumString*>(this);
    }

    if(0 != *ppv)
    {
        AddRef();
        hr = S_OK;
    }

    return hr;

}
//--------------------------------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CEnumURIs::AddRef(void)
{
    return InterlockedIncrement(&m_refs);
}
//--------------------------------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CEnumURIs::Release(void)
{
    ULONG refs = InterlockedDecrement(&m_refs);
    if(0 == refs)
    {
        delete this;
    }

    return refs;
}
//--------------------------------------------------------------------------------------------------

STDMETHODIMP CEnumURIs::Next(ULONG celt, LPWSTR* rgelt, ULONG* pceltFetched)
{
    HRESULT hr = S_OK;
    unsigned fetched = 0;

    // Load handler IDs if they haven't been loaded already.
    if(_pHandlerCollection->_pDevices == NULL)
    {
        hr = _pHandlerCollection->_LoadHandlerIDs();
    }
    if(SUCCEEDED(hr))
    {
        SYNCDEVICEINFO *pDevices = _pHandlerCollection->_pDevices;
        while((fetched < celt) && (m_current < _pHandlerCollection->_cDevices) && SUCCEEDED(hr))
        {
            if (pDevices[m_current].szHandlerID[0] != L'\0')
            {
                hr = SHStrDupW(pDevices[m_current].szHandlerID, &rgelt[fetched]);
                fetched++;
            }
            ++m_current;
        }

        if(FAILED(hr))
        {
            while(fetched > 0)
            {
                --fetched;
                CoTaskMemFree(rgelt[fetched]);
                rgelt[fetched] = 0;
            }
        }
        else
        {
            hr = (fetched == celt) ? S_OK : S_FALSE;
        }
    }

    if(0 != pceltFetched)
    {
        *pceltFetched = fetched;
    }

    return hr;
}
//--------------------------------------------------------------------------------------------------

STDMETHODIMP CEnumURIs::Skip(ULONG)
{
    return E_NOTIMPL;
}
//--------------------------------------------------------------------------------------------------

STDMETHODIMP CEnumURIs::Reset(void)
{
    m_current = 0;
    return S_OK;
}
//--------------------------------------------------------------------------------------------------

STDMETHODIMP CEnumURIs::Clone(IEnumString** ppenum)
{
    *ppenum = 0;
    return E_NOTIMPL;
}
//--------------------------------------------------------------------------------------------------
