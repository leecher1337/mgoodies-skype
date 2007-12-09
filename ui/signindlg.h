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
//--------------------------------------------------------------------------------------------------

class CSignInDialog : public CDialogImpl<CSignInDialog>
{
public:
    enum { IDD = IDD_SIGNIN };

                        CSignInDialog(void);

    void                Init(const bstr_t& uri, bool autoConnect, const bstr_t& server, const bstr_t& transport);

    bstr_t              GetUri(void) const;
    bool                GetAutoConnect(void) const;
    bstr_t              GetServer(void) const;
    bstr_t              GetTransport(void) const;

private:
    BEGIN_MSG_MAP(CSignInDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_AUTODETECT, OnAutoDetectChanged)
        COMMAND_HANDLER(IDC_NAME, EN_CHANGE, OnNameChanged)
    END_MSG_MAP()

    LRESULT             OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT             OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnAutoDetectChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnNameChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    void                UpdateControls(void);

private:
    bstr_t              m_uri;
    bool                m_autoConnect;
    bstr_t              m_server;
    bstr_t              m_transport;
};
//--------------------------------------------------------------------------------------------------

inline CSignInDialog::CSignInDialog(void) :
    m_autoConnect(true)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline void CSignInDialog::Init(const bstr_t& uri, bool autoConnect, const bstr_t& server,
    const bstr_t& transport)
{
    m_uri = uri;
    m_autoConnect = autoConnect;
    m_server = server;
    m_transport = transport.length() > 0 ? transport : L"TCP";
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CSignInDialog::GetUri(void) const
{
    return m_uri.length() > 0 ? m_uri : L"";
}
//--------------------------------------------------------------------------------------------------

inline bool CSignInDialog::GetAutoConnect(void) const
{
    return m_autoConnect;
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CSignInDialog::GetServer(void) const
{
    return m_server.length() > 0 ? m_uri : L"";
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CSignInDialog::GetTransport(void) const
{
    return m_transport.length() > 0 ? m_transport : L"TCP";
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CSignInDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    // center the dialog on the screen
    CenterWindow();

    TranslateDialogDefault(m_hWnd);

    SetDlgItemText(IDC_NAME, m_uri);
    SetDlgItemText(IDC_SERVER, m_server);

    CComboBox transport(GetDlgItem(IDC_TRANSPORT));
    transport.AddString(L"TCP");
    transport.AddString(L"TLS");
    transport.AddString(L"UDP");
    if(0 == lstrcmpW(L"UDP", m_transport))
        transport.SetCurSel(2);
    else if(0 == lstrcmpW(L"TLS", m_transport))
        transport.SetCurSel(1);
    else
        transport.SetCurSel(0);

    CheckDlgButton(IDC_AUTODETECT, m_autoConnect ? BST_CHECKED : BST_UNCHECKED);

    UpdateControls();

    return TRUE;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CSignInDialog::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_autoConnect = BST_CHECKED == IsDlgButtonChecked(IDC_AUTODETECT);

    GetDlgItemText(IDC_NAME, m_uri.GetBSTR());
    GetDlgItemText(IDC_SERVER, m_server.GetBSTR());
    GetDlgItemText(IDC_TRANSPORT, m_transport.GetBSTR());

    EndDialog(wID);
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CSignInDialog::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CSignInDialog::OnAutoDetectChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    UpdateControls();
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CSignInDialog::OnNameChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    UpdateControls();
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline void CSignInDialog::UpdateControls(void)
{
    bool autoDetect = BST_CHECKED == IsDlgButtonChecked(IDC_AUTODETECT);

    ::EnableWindow(GetDlgItem(IDC_SERVER), !autoDetect);
    ::EnableWindow(GetDlgItem(IDC_TRANSPORT), !autoDetect);

    bool nameNotEmpty = ::GetWindowTextLength(GetDlgItem(IDC_NAME)) > 0;

    ::EnableWindow(GetDlgItem(IDOK), nameNotEmpty);
}
//--------------------------------------------------------------------------------------------------
