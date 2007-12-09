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

#include "resource.h"
//--------------------------------------------------------------------------------------------------

class CActivationContext
{
public:
    explicit             CActivationContext(HMODULE module);
                        ~CActivationContext(void);

private:
                        CActivationContext(const CActivationContext&);
    CActivationContext& operator= (const CActivationContext&);

    bool                InitAPI(void);

private:
    typedef HANDLE (__stdcall *CreateActCtxW_t)(PCACTCTXW);
    typedef BOOL (__stdcall *ActivateActCtx_t)(HANDLE, ULONG_PTR*);
    typedef BOOL (__stdcall *DeactivateActCtx_t)(DWORD, ULONG_PTR);
    typedef void (__stdcall *ReleaseActCtx_t)(HANDLE);

    HANDLE              m_ctx;
    ULONG_PTR           m_cookie;
    CreateActCtxW_t     m_CreateActCtxW;
    ActivateActCtx_t    m_ActivateActCtx;
    DeactivateActCtx_t  m_DeactivateActCtx;
    ReleaseActCtx_t     m_ReleaseActCtx;
};
//--------------------------------------------------------------------------------------------------

inline CActivationContext::CActivationContext(HMODULE module) :
    m_ctx(INVALID_HANDLE_VALUE),
    m_cookie(0),
    m_CreateActCtxW(0),
    m_ActivateActCtx(0),
    m_DeactivateActCtx(0),
    m_ReleaseActCtx(0)
{
    if(InitAPI())
    {
        WCHAR moduleFileName[MAX_PATH];
        if(GetModuleFileName(module, moduleFileName, sizeof(moduleFileName) / sizeof(moduleFileName[0])))
        {
            ACTCTXW ctx = { 0 };
            ctx.cbSize = sizeof(ctx);
            ctx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
            ctx.lpSource = moduleFileName;
            ctx.lpResourceName = MAKEINTRESOURCE(IDR_RT_MANIFEST);

            m_ctx = m_CreateActCtxW(&ctx);

            if(INVALID_HANDLE_VALUE != m_ctx)
            {
                m_ActivateActCtx(m_ctx, &m_cookie);
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------

inline CActivationContext::~CActivationContext(void)
{
    if(INVALID_HANDLE_VALUE != m_ctx)
    {
        m_DeactivateActCtx(0, m_cookie);
        m_ReleaseActCtx(m_ctx);
        m_ctx = INVALID_HANDLE_VALUE;
    }
}
//--------------------------------------------------------------------------------------------------

inline bool CActivationContext::InitAPI(void)
{
    HMODULE hK32 = GetModuleHandleW(L"kernel32.dll");
    if(hK32)
    {
        m_CreateActCtxW = (CreateActCtxW_t)GetProcAddress(hK32, "CreateActCtxW");
        m_ActivateActCtx = (ActivateActCtx_t)GetProcAddress(hK32, "ActivateActCtx");
        m_DeactivateActCtx = (DeactivateActCtx_t)GetProcAddress(hK32, "DeactivateActCtx");
        m_ReleaseActCtx = (ReleaseActCtx_t)GetProcAddress(hK32, "ReleaseActCtx");
    }

    return 0 != m_CreateActCtxW && 0 != m_ActivateActCtx && 0 != m_DeactivateActCtx &&
        0 != m_ReleaseActCtx;
}
//--------------------------------------------------------------------------------------------------
