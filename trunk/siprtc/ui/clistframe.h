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
#include "../m_siprtc.h"
#include "../rtchelper.h"
#include "../SDK/m_cluiframes.h"
#include "m_clui.h"
#include "m_button.h"
#include "atlframe.h"
//--------------------------------------------------------------------------------------------------

class CListFrameDialog : public CDialogImpl<CListFrameDialog>,
                         public CDialogResize<CListFrameDialog>
{
public:
    enum { IDD = IDD_CLIST_FRAME };

                        CListFrameDialog(void);

    void                Initialize(void);

private:
    enum
    {
        WM_ME_SIPRTC_CALL_STATE_CHANGED = WM_USER + 100,
    };


    typedef CDialogResize<CListFrameDialog> resizeBase_t;

    BEGIN_MSG_MAP(CListFrameDialog)
        MESSAGE_HANDLER(WM_INITDIALOG,           OnInitDialog)
        MESSAGE_HANDLER(WM_ERASEBKGND,           OnEraseBackground)
        MESSAGE_HANDLER(WM_CTLCOLORDLG,          OnCtlColorDlg)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC,       OnCtlColorStatic)

//        MESSAGE_HANDLER(WM_WINDOWPOSCHANGED,     OnWindowPosChanged)
//        MESSAGE_HANDLER(WM_TIMER,                OnTimer)
        //MESSAGE_HANDLER(WM_DESTROY,              OnDestroy)
        COMMAND_ID_HANDLER(IDC_CALL,             OnCall)
        COMMAND_ID_HANDLER(IDC_HANGUP,           OnHangUp)
        COMMAND_ID_HANDLER(IDC_OPEN_SESSION_DLG, OnOpenSessionDialog)
        COMMAND_ID_HANDLER(IDC_POPUP_MENU,       OnPopupMenu)
        COMMAND_HANDLER(IDC_URICOMBO, CBN_EDITCHANGE, OnUriChanged)

        MESSAGE_HANDLER(WM_ME_SIPRTC_CALL_STATE_CHANGED, OnCallStateChanged)

        CHAIN_MSG_MAP(resizeBase_t)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CListFrameDialog)
        DLGRESIZE_CONTROL(IDC_STATUS,   DLSZ_SIZE_X)
        DLGRESIZE_CONTROL(IDC_TIME,     DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_URICOMBO, DLSZ_SIZE_X)
        DLGRESIZE_CONTROL(IDC_CALL,     DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_HANGUP,   DLSZ_MOVE_X)
    END_DLGRESIZE_MAP()

    LRESULT             OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT             OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT             OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT             OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT             OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT             OnCall(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnHangUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnOpenSessionDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnPopupMenu(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT             OnUriChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT             OnCallStateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    void                UpdateControls(void);

private:
    int                 m_callState;
    bstr_t              m_uri;
    HANDLE              m_hContact;
    int frameId;
};
//--------------------------------------------------------------------------------------------------

inline CListFrameDialog::CListFrameDialog(void) :
    m_callState(SIPRTC_CALL_DISCONNECTED),
    m_uri(L""),
    m_hContact(0)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline void CListFrameDialog::Initialize(void)
{
    Create((HWND)CallService(MS_CLUI_GETHWND, 0, 0));
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    TranslateDialogDefault(m_hWnd);
    DlgResize_Init(false);

    ModifyStyle(WS_CLIPCHILDREN, 0);
    CComboBoxEx(GetDlgItem(IDC_URICOMBO)).GetEditCtrl().SetCueBannerText(TranslateW(L"Type in phone number or SIP Address"));

    char eventName[300];
    mir_snprintf(eventName, sizeof(eventName), "%s%s", g_env.ProtocolName(), ME_SIPRTC_CALL_STATE_CHANGED);
    HookEventMessage(eventName, m_hWnd, WM_ME_SIPRTC_CALL_STATE_CHANGED);

    m_callState = CallProtoService(g_env.ProtocolName(), MS_SIPRTC_GET_CALL_STATE, 0, 0);

    CLISTFrame frame = { 0 };
    frame.cbSize = sizeof(frame);
    frame.hWnd = m_hWnd;
    frame.align = alBottom;
    frame.hIcon = LoadIcon(g_env.Instance(), MAKEINTRESOURCE(IDI_SIPRTC));
    frame.Flags = F_VISIBLE | F_NOBORDER/* | F_NO_SUBCONTAINER*/;
    RECT rc = { 0 };
    GetWindowRect(&rc);
    int h = rc.bottom - rc.top;
    frame.height = (0 == h) ? 20 : h;
    frame.name = Translate("SIP Call");
    frame.TBname = Translate("SIP Call");
    frameId = CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&frame, 0);

    struct
    {
        int         buttonId;
        const char* tooltip;
        int         icon;
    }
    buttons[] =
    {
        { IDC_OPEN_SESSION_DLG,     "Open Call window",     IDI_ICON_OPEN_SESSION },
        { IDC_POPUP_MENU,           "More...",              IDI_ICON_DOWN_ARROW },
        { IDC_CALL,                 "Call",                 IDI_ICON_CALL },
        { IDC_HANGUP,               "Hang Up",              IDI_ICON_HANGUP }
    };

    for(unsigned i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i)
    {
        SendDlgItemMessage(buttons[i].buttonId, BUTTONSETASFLATBTN, 0, 0);
        SendDlgItemMessage(buttons[i].buttonId, BUTTONADDTOOLTIP,
            (WPARAM)Translate(buttons[i].tooltip), 0);
        SendDlgItemMessage(buttons[i].buttonId, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM)LoadImage(g_env.Instance(), MAKEINTRESOURCE(buttons[i].icon), IMAGE_ICON,
                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));
    }

    SendDlgItemMessage(IDC_POPUP_MENU, BUTTONSETARROW, 1, 0);

    UpdateControls();

    CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frameId, FU_FMPOS);

    return TRUE;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    HDC hdc = (HDC)wParam;
    RECT r;
    GetClientRect(&r);
    HBRUSH hB = CreateSolidBrush((COLORREF)DBGetContactSettingDword(NULL, "MyDetails", "BackgroundColor", GetSysColor(COLOR_BTNFACE)));
    FillRect(hdc, &r, hB);
    DeleteObject(hB);
    return TRUE;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnCtlColorDlg(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    return (LRESULT)CreateSolidBrush((COLORREF)DBGetContactSettingDword(NULL, "MyDetails", "BackgroundColor", GetSysColor(COLOR_BTNFACE)));
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    static HBRUSH hB = 0;
    if(hB == 0)
    {
        hB = CreateSolidBrush((COLORREF)DBGetContactSettingDword(NULL, "MyDetails", "BackgroundColor", GetSysColor(COLOR_BTNFACE)));
    }
    SetBkMode((HDC)wParam, TRANSPARENT);
    return (LRESULT)hB;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnWindowPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = false;
    CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frameId, FU_FMREDRAW);

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnCall(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    MTLASSERT(SIPRTC_CALL_DISCONNECTED == m_callState);

    if(SIPRTC_CALL_DISCONNECTED == m_callState)
    {
        if(::GetWindowTextLength(GetDlgItem(IDC_URICOMBO)) > 0)
        {
            bstr_t uri(L"");
            GetDlgItemText(IDC_URICOMBO, uri.GetBSTR());
            if(uri.length() > 0)
            {
                SIPRTC_CALL sc = { 0 };
                sc.cbSize = sizeof(sc);
                sc.uri = uri;

                CallProtoService(g_env.ProtocolName(), MS_SIPRTC_CALL, 0, (LPARAM)&sc);
            }
        }
    }

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnHangUp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    MTLASSERT(SIPRTC_CALL_DISCONNECTED != m_callState);

    if(SIPRTC_CALL_DISCONNECTED != m_callState)
    {
        CallProtoService(g_env.ProtocolName(), MS_SIPRTC_TERMINATE_CALL, 0, 0);
    }

    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnOpenSessionDialog(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnPopupMenu(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnUriChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    UpdateControls();
    return 0;
}
//--------------------------------------------------------------------------------------------------

inline void CListFrameDialog::UpdateControls(void)
{
    bool uriReadOnly = true;
    bool timerVisible = false;
    bool popupMenuVisible = true;
    bool callEnabled = false;
    bool hangUpEnabled = true;

    const wchar_t* status = L"";
    switch(m_callState)
    {
    case SIPRTC_CALL_IDLE:
        status = L"Idle";
        break;
    case SIPRTC_CALL_INCOMING:
        status = L"Incoming call";
        callEnabled = true;
        break;
    case SIPRTC_CALL_ANSWERING:
        status = L"Answering";
        break;
    case SIPRTC_CALL_INPROGRESS:
        status = L"In progress";
        break;
    case SIPRTC_CALL_CONNECTED:
        status = L"Connected";
        timerVisible = true;
        break;
    case SIPRTC_CALL_DISCONNECTED:
        status = L"Not in a call";
        uriReadOnly = false;
        hangUpEnabled = false;
        callEnabled = ::GetWindowTextLength(GetDlgItem(IDC_URICOMBO)) > 0;
        break;
    case SIPRTC_CALL_HOLD:
        status = L"On hold";
        break;
    case SIPRTC_CALL_REFER:
        status = L"Call referred";
        break;
    }

    SetDlgItemText(IDC_STATUS, TranslateW(status));

    ::EnableWindow(GetDlgItem(IDC_URICOMBO),    !uriReadOnly);
    ::EnableWindow(GetDlgItem(IDC_CALL),        callEnabled);
    ::EnableWindow(GetDlgItem(IDC_HANGUP),      hangUpEnabled);
    //::EnableWindow(GetDlgItem(IDC_POPUP_MENU),  popupMenuVisible);
    ::ShowWindow(GetDlgItem(IDC_TIME),          timerVisible ? SW_SHOW : SW_HIDE);
}
//--------------------------------------------------------------------------------------------------

inline LRESULT CListFrameDialog::OnCallStateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_callState = (int)wParam;
    SIPRTC_CALL* call = (SIPRTC_CALL*)lParam;
    m_uri = call->uri ? call->uri : L"";
    m_hContact = call->hContact;

    UpdateControls();

    return 0;
}
//--------------------------------------------------------------------------------------------------
