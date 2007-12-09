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

#include "../resource.h"
#include "rtcsession.h"
//--------------------------------------------------------------------------------------------------

class CAcceptT120Dialog : public CDialogImpl<CAcceptT120Dialog>
{
public:
    enum { IDD = IDD_ACCEPT_T120 };

    explicit            CAcceptT120Dialog(void);

    void                Show(const bstr_t& participant, CRtcSession* session);
    void                Close(void);

private:
    BEGIN_MSG_MAP(CAcceptT120Dialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDC_ACCEPT, OnAccept)
        COMMAND_ID_HANDLER(IDCANCEL,   OnCancel)
    END_MSG_MAP()

    LRESULT             OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT             OnAccept(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    bstr_t              m_participant;
    CRtcSession*        m_session;
};
//--------------------------------------------------------------------------------------------------

inline CAcceptT120Dialog::CAcceptT120Dialog(void) :
    m_session(0)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline void CAcceptT120Dialog::Show(const bstr_t& participant, CRtcSession* session)
{
    MTLASSERT(participant.length() > 0);
    MTLASSERT(session);

    m_participant = participant;
    m_session = session;

    if(!IsWindow())
        Create(0);
}
//--------------------------------------------------------------------------------------------------

inline void CAcceptT120Dialog::Close(void)
{
    m_session = 0;

    if(IsWindow())
        EndDialog(IDCANCEL);
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CAcceptT120Dialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    // center the dialog on the screen
    CenterWindow();

    TranslateDialogDefault(m_hWnd);

    const wchar_t* fmt  = TranslateW(L"%s would like to have a session using Whiteboard or Application Sharing with you.\nDo you want to accept the invitation?");

    unsigned bufSize = lstrlenW(fmt) + m_participant.length() + 10;
    wchar_t* buffer = (wchar_t*)_alloca(sizeof(wchar_t) * bufSize);

    MTLVERIFY(S_OK == StringCchPrintf(buffer, bufSize, fmt, static_cast<const wchar_t*>(m_participant)));

    SetDlgItemText(IDC_INVITATION, buffer);

    return TRUE;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CAcceptT120Dialog::OnAccept(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    MTLASSERT(m_session);

    if(m_session)
    {
        try
        {
            m_session->Accept();
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to accept the session: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }

    m_session = 0;
    EndDialog(wID);
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CAcceptT120Dialog::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    MTLASSERT(m_session);

    if(m_session)
    {
        try
        {
            m_session->Reject();
        }
        catch(_com_error& e)
        {
            g_env.Trace().WriteError(L"Failed to reject the session: %X (%s)", e.Error(),
                static_cast<const wchar_t*>(e.Description()));
        }
    }

    m_session = 0;
    EndDialog(wID);
    return 0;
}
//--------------------------------------------------------------------------------------------------
