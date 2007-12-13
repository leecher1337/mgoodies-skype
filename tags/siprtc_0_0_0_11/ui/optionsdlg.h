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

#include "MTL/mtlopt.h"
#include "../resource.h"
//--------------------------------------------------------------------------------------------------

class COptionsDialog : public CMirandaOptionsPage<COptionsDialog>
{
public:
    enum { IDD = IDD_OPTIONS };

private:

    typedef CMirandaOptionsPage<COptionsDialog> baseClass_t;

    BEGIN_MSG_MAP(COptionsDialog)
        MESSAGE_HANDLER(WM_INITDIALOG,      OnInitDialog)
        COMMAND_ID_HANDLER(IDC_AUTOAUTH,    OnAutoChanged)
        COMMAND_ID_HANDLER(IDC_AUTOCONNECT, OnAutoChanged)
		COMMAND_ID_HANDLER(IDC_ENABLELOG,	OnDataChanged)
        COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnDataChanged)
        COMMAND_CODE_HANDLER(EN_CHANGE,     OnDataChanged)
        CHAIN_MSG_MAP(baseClass_t)
    END_MSG_MAP()

    LRESULT             OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT             OnAutoChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnDataChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    void                UpdateControls(void);

    BOOL                OnApply(void);
};
//--------------------------------------------------------------------------------------------------

inline LRESULT COptionsDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    TranslateDialogDefault(m_hWnd);

    bstr_t uri       = g_env.DB().GetMySettingWString("URI");
    bool autoAuth    = g_env.DB().GetMySettingBool("AutoAuth", true);
    bstr_t account   = g_env.DB().GetMySettingWString("Account");
    bstr_t password  = g_env.DB().GetMyEncryptedSettingWString("Password");
    bool autoConnect = g_env.DB().GetMySettingBool("AutoConnect", true);
    bstr_t server    = g_env.DB().GetMySettingWString("Server");
    bstr_t transport = g_env.DB().GetMySettingWString("Transport");
    if(0 == transport.length())
       transport = L"TCP";
	bool enableLog   = g_env.DB().GetMySettingBool("EnableLogFile", false);

    SetDlgItemText(IDC_NAME,        uri);
    SetDlgItemText(IDC_ACCOUNT,     account);
    SetDlgItemText(IDC_PASSWORD,    password);
    SetDlgItemText(IDC_SERVER,      server);

    CComboBox transportComboBox(GetDlgItem(IDC_TRANSPORT));
    transportComboBox.AddString(L"TCP");
    transportComboBox.AddString(L"TLS");
    transportComboBox.AddString(L"UDP");

    if(0 == lstrcmpiW(transport, L"UDP"))
        transportComboBox.SetCurSel(2);
    else if(0 == lstrcmpiW(transport, L"TLS"))
        transportComboBox.SetCurSel(1);
    else
        transportComboBox.SetCurSel(0);

    CheckDlgButton(IDC_AUTOAUTH,    autoAuth ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_AUTOCONNECT, autoConnect ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_ENABLELOG,	enableLog ? BST_CHECKED : BST_UNCHECKED);

    UpdateControls();

    return TRUE;
}
//--------------------------------------------------------------------------------------------------

inline BOOL COptionsDialog::OnApply(void)
{
    bstr_t uri(L"");
    bstr_t account(L"");
    bstr_t password(L"");
    bstr_t server(L"");
    bstr_t transport(L"");

    GetDlgItemText(IDC_NAME,        uri.GetBSTR());
    GetDlgItemText(IDC_ACCOUNT,     account.GetBSTR());
    GetDlgItemText(IDC_PASSWORD,    password.GetBSTR());
    GetDlgItemText(IDC_SERVER,      server.GetBSTR());
    GetDlgItemText(IDC_TRANSPORT,   transport.GetBSTR());

    bool autoConnect = BST_CHECKED == IsDlgButtonChecked(IDC_AUTOCONNECT);
    bool autoAuth    = BST_CHECKED == IsDlgButtonChecked(IDC_AUTOAUTH);
	bool enableLog   = BST_CHECKED == IsDlgButtonChecked(IDC_ENABLELOG);

    g_env.DB().WriteMySettingWString("URI",       uri.length() > 0 ? uri : L"");
    g_env.DB().WriteMySettingBool("AutoConnect",  autoConnect);
    g_env.DB().WriteMySettingWString("Server",    server.length() > 0 ? server : L"");
    g_env.DB().WriteMySettingWString("Transport", transport.length() > 0 ? transport : L"TCP");
    g_env.DB().WriteMySettingBool("AutoAuth",     autoAuth);
    g_env.DB().WriteMySettingWString("Account",   account.length() > 0 ? account : L"");
    g_env.DB().WriteMyEncryptedSettingWString("Password", password.length() > 0 ? password : L"");
	g_env.DB().WriteMySettingBool("EnableLogFile", enableLog);

    bstr_t nick = g_env.DB().GetMySettingWString("Nick");
    if(0 == nick.length() && uri.length() > 0)
    {
        g_env.DB().WriteMySettingWString("Nick", ComposeNickByUri(uri));
    }

    g_env.Trace().WriteVerbose(L"All the settings have been saved");

    return TRUE;
}
//--------------------------------------------------------------------------------------------------

inline void COptionsDialog::UpdateControls(void)
{
    bool autoConnect = BST_CHECKED == IsDlgButtonChecked(IDC_AUTOCONNECT);
    bool autoAuth    = BST_CHECKED == IsDlgButtonChecked(IDC_AUTOAUTH);

    ::EnableWindow(GetDlgItem(IDC_SERVER), !autoConnect);
    ::EnableWindow(GetDlgItem(IDC_TRANSPORT), !autoConnect);

    ::EnableWindow(GetDlgItem(IDC_ACCOUNT), !autoAuth);
    ::EnableWindow(GetDlgItem(IDC_PASSWORD), !autoAuth);
}
//--------------------------------------------------------------------------------------------------

inline LRESULT COptionsDialog::OnAutoChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    UpdateControls();
    SetModified();
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT COptionsDialog::OnDataChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SetModified();
    return 0;
}
//--------------------------------------------------------------------------------------------------
