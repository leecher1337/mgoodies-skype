/*

MTL - Miranda Template Library for Miranda IM

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

#if !defined(__MTL_MTLOPT_H__)
#define __MTL_MTLOPT_H__

#pragma once

#define _WTL_NEW_PAGE_NOTIFY_HANDLERS

#include <atldlgs.h>
//--------------------------------------------------------------------------------------------------

struct CDlgProcThunk : public thiscallthunk
{
    template<class T>
    void                InitDlgProc(INT_PTR (T::*proc)(HWND, UINT, WPARAM, LPARAM),
                            T* pObject);

    DLGPROC             GetDLGPROC(void) const;
};
//--------------------------------------------------------------------------------------------------

template<class T>
inline void CDlgProcThunk::InitDlgProc(INT_PTR (T::*proc)(HWND, UINT, WPARAM, LPARAM),
    T* pObject)
{
    MTLASSERT(pObject);
    MTLASSERT(proc);

    // Static assert ensuring that sizeof of the member pointer is 4 bytes.
    // Currently larger pointers are not supported.
    // HINT: Multiple inheritance tends to increase pointer size.
    typedef char ASSERT_Member_ptr_size_must_be_4_bytes[sizeof(proc) == sizeof(DWORD) ? 1 : 0];

    union
    {
        INT_PTR (T::*source)(HWND, UINT, WPARAM, LPARAM);
        INT_PTR (T::*target)(void);
    } cast;

    cast.source = proc;

    InitNo(cast.target, pObject);
}
//--------------------------------------------------------------------------------------------------

inline DLGPROC CDlgProcThunk::GetDLGPROC(void) const
{
    return reinterpret_cast<DLGPROC>(GetCodeAddress());
}
//--------------------------------------------------------------------------------------------------


template <class T, class TBase = CPropertyPageWindow>
class ATL_NO_VTABLE CMirandaOptionsPage : public CPropertyPageImpl<T, TBase>
{
public:
                        CMirandaOptionsPage(void);

    DLGPROC             GetDLGPROC(void) const;

    /*
        Implement in your code

    int OnApply()
    {
        // PSNRET_NOERROR = apply OK
        // PSNRET_INVALID = apply not OK, return to this page
        // PSNRET_INVALID_NOCHANGEPAGE = apply not OK, don't change focus
        return PSNRET_NOERROR;
    }
    */

private:
    CDlgProcThunk       m_startThunk;
    struct Stub
    {
        T* pThis;
        INT_PTR         StartDialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    };
    Stub                m_startStub;
};
//--------------------------------------------------------------------------------------------------

template <class T, class TBase>
inline CMirandaOptionsPage<T, TBase>::CMirandaOptionsPage(void)
{
    m_startStub.pThis = static_cast<T*>(this);
    m_startThunk.InitDlgProc(&CMirandaOptionsPage::Stub::StartDialogProc, &m_startStub);
}
//--------------------------------------------------------------------------------------------------

template <class T, class TBase>
inline DLGPROC CMirandaOptionsPage<T, TBase>::GetDLGPROC(void) const
{
    return m_startThunk.GetDLGPROC();
}
//--------------------------------------------------------------------------------------------------

template <class T, class TBase>
inline INT_PTR CMirandaOptionsPage<T, TBase>::Stub::StartDialogProc(HWND hwnd, UINT msg,
    WPARAM wparam, LPARAM lparam)
{
    MTLASSERT(pThis != NULL);

    pThis->m_hWnd = hwnd;

    pThis->m_thunk.Init((WNDPROC)pThis->GetDialogProc(), pThis);
    DLGPROC pProc = (DLGPROC)pThis->m_thunk.GetWNDPROC();
    MTLASSERT(pProc);
    ::SetWindowLongPtr(hwnd, DWLP_DLGPROC, (LONG_PTR)pProc);

    return pProc(hwnd, msg, wparam, lparam);
}
//--------------------------------------------------------------------------------------------------

#endif // __MTL_MTLOPT_H__