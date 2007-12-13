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


#include "../stdafx.h"

#include "annoyuser.h"
#include "../SDK/m_popup.h"
#include "../SDK/m_popupw.h"
#include "m_langpack.h"
#include "../resource.h"
//--------------------------------------------------------------------------------------------------

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_COMMAND:
            if(STN_CLICKED == HIWORD(wParam))
            {  //It was a click on the Popup.
                PUDeletePopUp(hWnd);
                return TRUE;
            }
            break;

        case WM_CONTEXTMENU:
            {
                wchar_t* message = (wchar_t*)PUGetPluginData(hWnd);
                if(message > 0 && !IsBadStringPtr(message, MAX_SECONDLINE))
                {
                    if(OpenClipboard(hWnd))
                    {
                        EmptyClipboard();

                        HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, (lstrlenW(message) + 1) * sizeof(wchar_t));
                        if(hglb)
                        {
                            wchar_t* data = (wchar_t*)GlobalLock(hglb);
                            if(data)
                            {
                                lstrcpyW(data, message);
                                GlobalUnlock(hglb);

                                MTLVERIFY(0 != SetClipboardData(CF_UNICODETEXT, hglb));
                            }
                        }

                        MTLVERIFY(CloseClipboard());
                    }
                }

                PUDeletePopUp(hWnd);
                return TRUE;
            }
            break;

        case UM_FREEPLUGINDATA:
            {
                wchar_t* message = (wchar_t*)PUGetPluginData(hWnd);
                if(message > 0 && !IsBadStringPtr(message, MAX_SECONDLINE))
                    delete[] message;
                return TRUE;
            }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
//--------------------------------------------------------------------------------------------------

void AnnoyUser(const wchar_t* message, const wchar_t* title)
{
    MTLASSERT(message);

    if(message && message[0])
    {
        if(!title)
            title = TranslateW(L"SIP Protocol");

        bool userWasAnnoyed = false;
        //
        // Use popups if any
        //
        if(ServiceExists(MS_POPUP_ADDPOPUPW) && CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) != 0)
        {
            const wchar_t* fmt  = L"%s\n\n%s";
            const wchar_t* footnote = TranslateW(L"Right-click to copy this information");

            unsigned bufSize = lstrlenW(message) + lstrlenW(fmt) + lstrlenW(footnote) + 10;
            wchar_t* buffer = (wchar_t*)_alloca(sizeof(wchar_t) * bufSize);

            MTLVERIFY(S_OK == StringCchPrintf(buffer, bufSize, fmt, message, footnote));

            if(!PUIsSecondLineShown())
                title = buffer;

            POPUPDATAW popup = { 0 };
            popup.lchIcon = (HICON)LoadImage(g_env.Instance(), MAKEINTRESOURCE(IDI_SIPRTC), IMAGE_ICON,
                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
            MTLVERIFY(S_OK == StringCbCopy(popup.lpwzContactName, sizeof(popup.lpwzContactName), title));
            MTLVERIFY(S_OK == StringCbCopy(popup.lpwzText, sizeof(popup.lpwzText), buffer));
            popup.colorBack = RGB(255, 0, 0);
            popup.colorText = RGB(255, 255, 255);
            popup.iSeconds = 20;
            popup.PluginWindowProc = PopupDlgProc;
            popup.PluginData = new wchar_t[lstrlenW(message) + 1];
            lstrcpyW((wchar_t*)popup.PluginData, message);

            userWasAnnoyed = PUAddPopUpW(&popup) >= 0;
            MTLASSERT(userWasAnnoyed);
        }

        if(!userWasAnnoyed)
        {
            const wchar_t* fmt  = L"%s\n\n%s";
            const wchar_t* footnote = TranslateW(L"Press Ctrl+C to copy this information");

            unsigned bufSize = lstrlenW(message) + lstrlenW(fmt) + lstrlenW(footnote) + 10;
            wchar_t* buffer = (wchar_t*)_alloca(sizeof(wchar_t) * bufSize);

            MTLVERIFY(S_OK == StringCchPrintf(buffer, bufSize, fmt, message, footnote));

            MessageBoxW(0, buffer, title, MB_ICONERROR);
        }
    }
}
//--------------------------------------------------------------------------------------------------
