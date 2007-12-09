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

class CAuthDialog : public CDialogImpl<CAuthDialog>
{
public:
    enum { IDD = IDD_AUTH };

                        CAuthDialog(void);

    void                Init(const bstr_t& realm, const bstr_t& uri, const bstr_t& account,
                            const bstr_t& password, bool savePassword);

    bstr_t              GetUri(void) const;
    bstr_t              GetAccount(void) const;
    bstr_t              GetPassword(void) const;
    bool                GetSavePassword(void) const;

private:
    BEGIN_MSG_MAP(CAuthDialog)
        MESSAGE_HANDLER(WM_INITDIALOG,       OnInitDialog)
        COMMAND_ID_HANDLER(IDOK,             OnOK)
        COMMAND_ID_HANDLER(IDCANCEL,         OnCancel)
        COMMAND_HANDLER(IDC_NAME, EN_CHANGE, OnNameChanged)
    END_MSG_MAP()

    LRESULT             OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT             OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnNameChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    void                UpdateControls(void);

private:
    bstr_t              m_realm;
    bstr_t              m_uri;
    bstr_t              m_account;
    bstr_t              m_password;
    bool                m_savePassword;
};
//--------------------------------------------------------------------------------------------------

inline CAuthDialog::CAuthDialog(void) :
    m_savePassword(false)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline void CAuthDialog::Init(const bstr_t& realm, const bstr_t& uri, const bstr_t& account,
    const bstr_t& password, bool savePassword)
{
    m_realm =        realm;
    m_uri =          uri;
    m_account =      account;
    m_password =     password;
    m_savePassword = savePassword;
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CAuthDialog::GetUri(void) const
{
    return m_uri.length() ? m_uri : L"";
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CAuthDialog::GetAccount(void) const
{
    return m_account.length() ? m_account : L"";
}
//--------------------------------------------------------------------------------------------------

inline bstr_t CAuthDialog::GetPassword(void) const
{
    return m_password.length() ? m_password : L"";
}
//--------------------------------------------------------------------------------------------------

inline bool CAuthDialog::GetSavePassword(void) const
{
    return m_savePassword;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CAuthDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    TranslateDialogDefault(m_hWnd);

    // center the dialog on the screen
    CenterWindow();

    SetDlgItemText(IDC_NAME,     m_uri);
    SetDlgItemText(IDC_ACCOUNT,  m_account);
    SetDlgItemText(IDC_PASSWORD, m_password);
    SetDlgItemText(IDC_REALM,    m_realm);
    CheckDlgButton(IDC_SAVE_PASSWORD, m_savePassword ? BST_CHECKED : BST_UNCHECKED);

    UpdateControls();

    return TRUE;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CAuthDialog::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    GetDlgItemText(IDC_NAME,     m_uri.GetBSTR());
    GetDlgItemText(IDC_ACCOUNT,  m_account.GetBSTR());
    GetDlgItemText(IDC_PASSWORD, m_password.GetBSTR());
    m_savePassword = BST_CHECKED == IsDlgButtonChecked(IDC_SAVE_PASSWORD);

    EndDialog(wID);
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CAuthDialog::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CAuthDialog::OnNameChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    UpdateControls();
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline void CAuthDialog::UpdateControls(void)
{
    bool nameNotEmpty = ::GetWindowTextLength(GetDlgItem(IDC_NAME)) > 0;

    ::EnableWindow(GetDlgItem(IDOK), nameNotEmpty);
}
//--------------------------------------------------------------------------------------------------
