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

#if !defined(__MTL_MTL_H__)
#define __MTL_MTL_H__

#pragma once

#define NODEFINEDLINKFUNCTIONS
//--------------------------------------------------------------------------------------------------

#include <windows.h>
#include <newpluginapi.h>
#include <vector>
#define STRSAFE_LIB
#include <strsafe.h>
//--------------------------------------------------------------------------------------------------

#if defined(MTL_DISABLE_NO_VTABLE)
#define MTL_NO_VTABLE
#else
#define MTL_NO_VTABLE __declspec(novtable)
#endif // MTL_DISABLE_NO_VTABLE
//--------------------------------------------------------------------------------------------------

#if defined(_DEBUG)

#if !defined(_CRT_STRINGIZE)
#define _CRT_STRINGIZE2(x) #x
#define _CRT_STRINGIZE(x) _CRT_STRINGIZE2(x)
#endif

#if !defined(MTLASSERT)

#define MTLASSERT_BASE(expr, msg, file, lineno) \
    if(!(expr)) \
    { \
        MessageBoxA(0, "Assertion failed: " msg "\nFile: " file ", Line: " _CRT_STRINGIZE(lineno), "Error", \
            MB_ICONERROR | MB_OK);\
        DebugBreak(); \
    }

#define MTLASSERT(expr)  MTLASSERT_BASE((expr), #expr, __FILE__, __LINE__)

#endif // MTLASSERT

#if !defined(MTLVERIFY)
#define MTLVERIFY(expr)  MTLASSERT_BASE((expr), #expr, __FILE__, __LINE__)
#endif // MTLVERIFY

#else

#if !defined(MTLASSERT)
#define MTLASSERT(expr)
#endif // MTLASSERT

#if !defined(MTLVERIFY)
#define MTLVERIFY(expr) (expr)
#endif // MTLVERIFY

#endif // _DEBUG
//--------------------------------------------------------------------------------------------------


//
// Call thunks - allow class members to be used as Miranda service and hook functions
//
// TODO: 64bit support
//

#if defined(_M_IX86)

#pragma pack(push,1)

struct thiscallthunk
{
                        thiscallthunk();
                        ~thiscallthunk();

    template<class T, class W, class L, class Ret>
    void                InitWL(Ret (T::*func)(W, L), T* pObject);

    template<class T, class W, class Ret>
    void                InitW(Ret (T::*func)(W), T* pObject);

    template<class T, class L, class Ret>
    void                InitL(Ret (T::*func)(L), T* pObject);

    template<class T, class Ret>
    void                InitNo(Ret (T::*func)(void), T* pObject);

    MIRANDASERVICE      GetCodeAddress() const;

private:
    void                FlushInstructionCache();

private:
    struct TwoParams
    {
        DWORD           push1;            // push        d,[esp][08] ; LPARAM
        DWORD           push2;            // push        d,[esp][08] ; WPARAM
        BYTE            mov;              // mov         ecx, pThis
        DWORD           pThis;            //
        BYTE            call;             // call        proc
        DWORD           relProc;          //
        BYTE            retn;             // retn
    };

    struct OneWParam
    {
        DWORD           push;             // push        d,[esp][04] ; WPARAM
        BYTE            mov;              // mov         ecx, pThis
        DWORD           pThis;            //
        BYTE            call;             // call        proc
        DWORD           relProc;          //
        BYTE            retn;             // retn
    };

    struct OneLParam
    {
        DWORD           push;             // push        d,[esp][08] ; LPARAM
        BYTE            mov;              // mov         ecx, pThis
        DWORD           pThis;            //
        BYTE            call;             // call        proc
        DWORD           relProc;          //
        BYTE            retn;             // retn
    };

    struct NoParams
    {
        BYTE            mov;              // mov         ecx, pThis
        DWORD           pThis;            //
        BYTE            jmp;              // jmp         proc
        DWORD           relProc;          //
    };

    union code_t
    {
        TwoParams       two;
        OneWParam       oneW;
        OneWParam       oneL;
        NoParams        no;
        bool            initialized;
    };

    code_t              m_code;
};

#pragma pack(pop)
//--------------------------------------------------------------------------------------------------

inline thiscallthunk::thiscallthunk()
{
    ZeroMemory(this, sizeof(*this));
}
//--------------------------------------------------------------------------------------------------

inline thiscallthunk::~thiscallthunk()
{
    ZeroMemory(this, sizeof(*this));
}
//--------------------------------------------------------------------------------------------------

template<class T, class W, class L, class Ret>
inline void thiscallthunk::InitWL(Ret (T::*proc)(W, L), T* pObject)
{
    MTLASSERT(pObject);
    MTLASSERT(proc);

    // Static assert ensuring that sizeof of the member pointer is 4 bytes.
    // Currently larger pointers are not supported.
    // HINT: Multiple inheritance tends to increase pointer size.
    typedef char ASSERT_Member_ptr_size_must_be_4_bytes[sizeof(proc) == sizeof(DWORD) ? 1 : 0];
    typedef char ASSERT_First_param_size_must_be_less_or_eq_to_WPARAM[sizeof(W) <= sizeof(WPARAM) ? 1 : 0];
    typedef char ASSERT_Second_param_size_must_be_less_or_eq_to_LPARAM[sizeof(L) <= sizeof(LPARAM) ? 1 : 0];
    typedef char ASSERT_Ret_type_size_must_be_the_same_as_int[sizeof(Ret) <= sizeof(int) ? 1 : 0];

    union
    {
        Ret (T::*proc)(W, L);
        INT_PTR ptr;
    } cast;

    cast.proc = proc;

    m_code.two.push1 = 0x082474FF;               // push        d,[esp][08]
    m_code.two.push2 = 0x082474FF;               // push        d,[esp][08]
    m_code.two.mov   = 0xB9;                     // mov         ecx, pThis
    m_code.two.pThis = PtrToUlong(pObject);
    m_code.two.call  = 0xE8;                     // call        proc
    m_code.two.relProc = DWORD(cast.ptr - ((INT_PTR)this + sizeof(m_code.two) - sizeof(m_code.two.retn)));
    m_code.two.retn  = 0xC3;                     // retn

    FlushInstructionCache();
}
//--------------------------------------------------------------------------------------------------

template<class T, class W, class Ret>
inline void thiscallthunk::InitW(Ret (T::*proc)(W), T* pObject)
{
    MTLASSERT(pObject);
    MTLASSERT(proc);

    // Static assert ensuring that sizeof of the member pointer is 4 bytes.
    // Currently larger pointers are not supported.
    // HINT: Multiple inheritance tends to increase pointer size.
    typedef char ASSERT_Member_ptr_size_must_be_4_bytes[sizeof(proc) == sizeof(DWORD) ? 1 : 0];
    typedef char ASSERT_Param_size_must_be_less_or_eq_to_WPARAM[sizeof(W) <= sizeof(WPARAM) ? 1 : 0];
    typedef char ASSERT_Ret_type_size_must_be_the_same_as_int[sizeof(Ret) <= sizeof(int) ? 1 : 0];

    union
    {
        Ret (T::*proc)(W);
        INT_PTR ptr;
    } cast;

    cast.proc = proc;

    m_code.oneW.push  = 0x042474FF;               // push        d,[esp][04] ; WPARAM
    m_code.oneW.mov   = 0xB9;                     // mov         ecx, pThis
    m_code.oneW.pThis = PtrToUlong(pObject);
    m_code.oneW.call  = 0xE8;                     // call        proc
    m_code.oneW.relProc = DWORD(cast.ptr - ((INT_PTR)this + sizeof(m_code.oneW) - sizeof(m_code.oneW.retn)));
    m_code.oneW.retn  = 0xC3;                     // retn

    FlushInstructionCache();
}
//--------------------------------------------------------------------------------------------------

template<class T, class L, class Ret>
inline void thiscallthunk::InitL(Ret (T::*proc)(L), T* pObject)
{
    MTLASSERT(pObject);
    MTLASSERT(proc);

    // Static assert ensuring that sizeof of the member pointer is 4 bytes.
    // Currently larger pointers are not supported.
    // HINT: Multiple inheritance tends to increase pointer size.
    typedef char ASSERT_Member_ptr_size_must_be_4_bytes[sizeof(proc) == sizeof(DWORD) ? 1 : 0];
    typedef char ASSERT_Param_size_must_be_less_or_eq_to_LPARAM[sizeof(L) <= sizeof(LPARAM) ? 1 : 0];
    typedef char ASSERT_Ret_type_size_must_be_the_same_as_int[sizeof(Ret) <= sizeof(int) ? 1 : 0];

    union
    {
        Ret (T::*proc)(L);
        INT_PTR ptr;
    } cast;

    cast.proc = proc;

    m_code.oneL.push  = 0x082474FF;               // push        d,[esp][08] ; LPARAM
    m_code.oneL.mov   = 0xB9;                     // mov         ecx, pThis
    m_code.oneL.pThis = PtrToUlong(pObject);
    m_code.oneL.call  = 0xE8;                     // call        proc
    m_code.oneL.relProc = DWORD(cast.ptr - ((INT_PTR)this + sizeof(m_code.oneL) - sizeof(m_code.oneL.retn)));
    m_code.oneL.retn  = 0xC3;                     // retn

    FlushInstructionCache();
}
//--------------------------------------------------------------------------------------------------

template<class T, class Ret>
inline void thiscallthunk::InitNo(Ret (T::*proc)(void), T* pObject)
{
    MTLASSERT(pObject);
    MTLASSERT(proc);

    // Static assert ensuring that sizeof of the member pointer is 4 bytes.
    // Currently larger pointers are not supported.
    // HINT: Multiple inheritance tends to increase pointer size.
    typedef char ASSERT_Member_ptr_size_must_be_4_bytes[sizeof(proc) == sizeof(DWORD) ? 1 : 0];
    typedef char ASSERT_Ret_type_size_must_be_the_same_as_int[sizeof(Ret) <= sizeof(int) ? 1 : 0];

    union
    {
        Ret (T::*proc)(void);
        INT_PTR ptr;
    } cast;

    cast.proc = proc;

    m_code.no.mov   = 0xB9;                     // mov         ecx, pThis
    m_code.no.pThis = PtrToUlong(pObject);
    m_code.no.jmp   = 0xE9;                     // jmp         proc
    m_code.no.relProc = DWORD(cast.ptr - ((INT_PTR)this + sizeof(m_code.no)));

    FlushInstructionCache();
}
//--------------------------------------------------------------------------------------------------

inline MIRANDASERVICE thiscallthunk::GetCodeAddress() const
{
    MTLASSERT(m_code.initialized);
    return reinterpret_cast<MIRANDASERVICE>(this);
}
//--------------------------------------------------------------------------------------------------

inline void thiscallthunk::FlushInstructionCache(void)
{
    ::FlushInstructionCache(GetCurrentProcess(), this, sizeof(*this));
}
//--------------------------------------------------------------------------------------------------

#else
#error Only X86 supported!
#endif
//--------------------------------------------------------------------------------------------------

typedef thiscallthunk CCallThunk;
//--------------------------------------------------------------------------------------------------

/*
    CLock
*/
class CLock
{
public:
                        CLock(void);
                        ~CLock(void);

    void                Lock(void);
    void                Unlock(void);

private:
                        CLock(const CLock&);
    CLock&              operator= (const CLock&);

private:
    CRITICAL_SECTION    m_cs;
};
//---------------------------------------------------------------------------

/*
    CLockPtr
*/
class CLockPtr
{
public:
    explicit            CLockPtr(CLock& lock);
                        ~CLockPtr(void);

private:
                        CLockPtr(const CLockPtr&);
    CLockPtr&           operator= (const CLockPtr&);

private:
    CLock&              m_lock;
};
//---------------------------------------------------------------------------

/*
    CLock
*/
inline CLock::CLock(void)
{
    ::InitializeCriticalSection(&m_cs);
}
//---------------------------------------------------------------------------

inline CLock::~CLock(void)
{
    ::DeleteCriticalSection(&m_cs);
}
//---------------------------------------------------------------------------

inline void CLock::Lock(void)
{
    ::EnterCriticalSection(&m_cs);
}
//---------------------------------------------------------------------------

inline void CLock::Unlock(void)
{
    ::LeaveCriticalSection(&m_cs);
}
//---------------------------------------------------------------------------

/*
    CLockPtr
*/
inline CLockPtr::CLockPtr(CLock& lock) :
    m_lock(lock)
{
    m_lock.Lock();
}
//---------------------------------------------------------------------------

inline CLockPtr::~CLockPtr(void)
{
    m_lock.Unlock();
}
//---------------------------------------------------------------------------

//
// PLUGINLINK wrapper allowing to use class members as hooks and services
//
class CMirandaPluginLink
{
public:
                        CMirandaPluginLink(void);
                        ~CMirandaPluginLink(void);

    bool                IsInitialized(void) const;
    void                Init(PLUGINLINK* pluginLink);
    void                Terminate(void);

    DWORD               GetMainThreadId(void) const;

    friend HANDLE       CreateHookableEvent(const char* name);
    friend int          DestroyHookableEvent(HANDLE hHook);
    friend int          NotifyEventHooks(HANDLE hHook, WPARAM wparam, LPARAM lparam);

    friend HANDLE       HookEvent(const char* name, MIRANDAHOOK func);
    template<class U, class W, class L, class Ret>
    friend HANDLE       HookEvent(const char* name, U* pObject, Ret (U::*func)(W, L));
    friend HANDLE       HookEventMessage(const char* name, HWND hWnd, UINT message);

    friend int          SetHookDefaultForHookableEvent(HANDLE hHook, MIRANDAHOOK func);
    template<class U>
    friend int          SetHookDefaultForHookableEvent(HANDLE hHook, U* pObject, int (U::*func)(WPARAM, LPARAM));

    friend int          UnhookEvent(HANDLE hHook);

    friend HANDLE       CreateServiceFunction(const char* name, MIRANDASERVICE func);
    template<class U, class W, class L, class Ret>
    friend HANDLE       CreateServiceFunction(const char* name, U* pObject, Ret (U::*func)(W, L));

    friend HANDLE       CreateMainThreadServiceFunction(const char* name, MIRANDASERVICE func);
    template<class U, class W, class L, class Ret>
    friend HANDLE       CreateMainThreadServiceFunction(const char* name, U* pObject, Ret (U::*func)(W, L));

    friend HANDLE       CreateTransientServiceFunction(const char* name, MIRANDASERVICE func);
    template<class U>
    friend HANDLE       CreateTransientServiceFunction(const char* name, U* pObject, int (U::*func)(WPARAM, LPARAM));

    friend int          DestroyServiceFunction(HANDLE hService);

    friend int          CallService(const char* name, WPARAM wparam, LPARAM lparam);
    friend bool         ServiceExists(const char* name);
    friend int          CallServiceSync(const char* name, WPARAM wparam, LPARAM lparam);
    friend int          CallFunctionAsync(void (__stdcall *)(void*), void*);

private:
    enum HandleType
    {
        HOOKABLE_EVENT,
        HOOKED_EVENT,
        SERVICE,
        DEFAULT_HOOK
    };

    struct MainThreadProxy
    {
        char*               serviceName;
        CCallThunk          thunk;
        MIRANDASERVICE      plainFunc;

        MainThreadProxy(const char* name, MIRANDASERVICE func);
        template<class U, class W, class L, class Ret>
        MainThreadProxy(const char* name, U* pObject, Ret (U::*func)(W, L));
        ~MainThreadProxy(void);
        int Call(WPARAM w, LPARAM l);
    };

    struct ActiveHandle
    {

        HandleType          type;
        HANDLE              handle;
        CCallThunk*         pThunk;
        MainThreadProxy*       pProxy;

                            ActiveHandle(HandleType t, HANDLE h, CCallThunk* thunk,
                                MainThreadProxy* proxy) :
                                type(t), handle(h), pThunk(thunk), pProxy(proxy) {}
    };

    PLUGINLINK*         GetLink(void);

    template<class U, class W, class L, class Ret>
    CCallThunk*         CreateThunk(U* pObject, Ret (U::*func)(W, L)) const;

    void                RegisterHandle(HandleType type, HANDLE handle, CCallThunk* pThunk,
                            MainThreadProxy* pProxy = 0);
    void                UnregisterHandle(HandleType type, HANDLE handle);

private:
    PLUGINLINK*         m_pluginLink;
    CLock               m_handlesLock;
    std::vector<ActiveHandle> m_handles;
    const DWORD         m_mainThreadId;
};
//--------------------------------------------------------------------------------------------------

inline CMirandaPluginLink::CMirandaPluginLink(void) :
    m_pluginLink(0),
    m_mainThreadId(GetCurrentThreadId())
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

inline CMirandaPluginLink::~CMirandaPluginLink(void)
{
    //
    // Cleanup all leaked resources
    //
    Terminate();
}
//--------------------------------------------------------------------------------------------------

inline bool CMirandaPluginLink::IsInitialized(void) const
{
    return 0 != m_pluginLink;
}
//--------------------------------------------------------------------------------------------------

inline void CMirandaPluginLink::Init(PLUGINLINK* pluginLink)
{
    MTLASSERT(pluginLink);
    m_pluginLink = pluginLink;
}
//--------------------------------------------------------------------------------------------------

inline void CMirandaPluginLink::Terminate(void)
{
    CLockPtr lock(m_handlesLock);

    while(m_pluginLink && !m_handles.empty())
    {
        std::vector<ActiveHandle>::iterator i = m_handles.begin();

        MTLASSERT(i->handle);

        if(i->handle)
        {
            switch(i->type)
            {
            case HOOKABLE_EVENT:
                m_pluginLink->DestroyHookableEvent(i->handle);
                break;
            case HOOKED_EVENT:
                m_pluginLink->UnhookEvent(i->handle);
                break;
            case SERVICE:
                m_pluginLink->DestroyServiceFunction(i->handle);
                break;
            case DEFAULT_HOOK:
                break;
            }
        }

        if(i->pThunk)
        {
            delete i->pThunk;
        }

        if(i->pProxy)
        {
            delete i->pProxy;
        }

        m_handles.erase(i);
    }
}
//--------------------------------------------------------------------------------------------------

inline DWORD CMirandaPluginLink::GetMainThreadId(void) const
{
    return m_mainThreadId;
}
//--------------------------------------------------------------------------------------------------

inline PLUGINLINK* CMirandaPluginLink::GetLink(void)
{
    MTLASSERT(m_pluginLink);
    return m_pluginLink;
}
//--------------------------------------------------------------------------------------------------

template<class U, class W, class L, class Ret>
inline CCallThunk* CMirandaPluginLink::CreateThunk(U* pObject, Ret (U::*func)(W, L)) const
{
    CCallThunk* thunk = new CCallThunk;
    MTLASSERT(thunk);
    thunk->InitWL(func, pObject);
    return thunk;
}
//--------------------------------------------------------------------------------------------------

inline void CMirandaPluginLink::RegisterHandle(HandleType type, HANDLE handle, CCallThunk* pThunk,
    MainThreadProxy* pProxy)
{
    MTLASSERT(handle);

    CLockPtr lock(m_handlesLock);
    m_handles.push_back(ActiveHandle(type, handle, pThunk, pProxy));
}
//--------------------------------------------------------------------------------------------------

inline void CMirandaPluginLink::UnregisterHandle(HandleType type, HANDLE handle)
{
    MTLASSERT(handle);

    CLockPtr lock(m_handlesLock);

    std::vector<ActiveHandle>::iterator i = m_handles.begin();
    for(; i != m_handles.end(); ++i)
    {
        if(type == i->type && handle == i->handle)
        {
            if(i->pThunk)
            {
                delete i->pThunk;
            }

            if(i->pProxy)
            {
                delete i->pProxy;
            }

            m_handles.erase(i);
            break;
        }
    }
}
//--------------------------------------------------------------------------------------------------

//
// Global instance
//

extern CMirandaPluginLink g_pluginLink;
//--------------------------------------------------------------------------------------------------


inline CMirandaPluginLink::MainThreadProxy::MainThreadProxy(const char* name, MIRANDASERVICE func) :
    serviceName(0),
    plainFunc(func)
{
    MTLASSERT(name && lstrlenA(name) > 0);
    MTLASSERT(func);

    unsigned size = lstrlenA(name) + 1;
    serviceName = new char[size];
    MTLVERIFY(S_OK == StringCchCopyA(serviceName, size, name));
}
//--------------------------------------------------------------------------------------------------

template<class U, class W, class L, class Ret>
inline CMirandaPluginLink::MainThreadProxy::MainThreadProxy(const char* name, U* pObject,
    Ret (U::*func)(W, L)) :
    serviceName(0),
    plainFunc(0)
{
    MTLASSERT(name && lstrlenA(name) > 0);
    MTLASSERT(pObject);
    MTLASSERT(func);

    thunk.InitWL(func, pObject);

    unsigned size = lstrlenA(name) + 1;
    serviceName = new char[size];
    MTLVERIFY(S_OK == StringCchCopyA(serviceName, size, name));
}
//--------------------------------------------------------------------------------------------------

inline CMirandaPluginLink::MainThreadProxy::~MainThreadProxy(void)
{
    delete[] serviceName;
    serviceName = 0;
}
//--------------------------------------------------------------------------------------------------

inline int CMirandaPluginLink::MainThreadProxy::Call(WPARAM w, LPARAM l)
{
    int res = 0;

    if(GetCurrentThreadId() != g_pluginLink.GetMainThreadId())
    {
        res = g_pluginLink.GetLink()->CallServiceSync(serviceName, w, l);
    }
    else
    {
        res = (0 != plainFunc ? plainFunc : thunk.GetCodeAddress())(w, l);
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

//
// Friends
//

inline HANDLE CreateHookableEvent(const char* name)
{
    MTLASSERT(name);

    HANDLE hEvent = g_pluginLink.GetLink()->CreateHookableEvent(name);
    if(hEvent)
    {
        g_pluginLink.RegisterHandle(CMirandaPluginLink::HOOKABLE_EVENT, hEvent, 0);
    }

    return hEvent;
}
//--------------------------------------------------------------------------------------------------

inline int DestroyHookableEvent(HANDLE hEvent)
{
    MTLASSERT(hEvent);

    int res = g_pluginLink.GetLink()->DestroyHookableEvent(hEvent);
    if(0 == res)
    {
        g_pluginLink.UnregisterHandle(CMirandaPluginLink::DEFAULT_HOOK, hEvent);
        g_pluginLink.UnregisterHandle(CMirandaPluginLink::HOOKABLE_EVENT, hEvent);
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int NotifyEventHooks(HANDLE hEvent, WPARAM wparam, LPARAM lparam)
{
    MTLASSERT(hEvent);

    return g_pluginLink.GetLink()->NotifyEventHooks(hEvent, wparam, lparam);
}
//--------------------------------------------------------------------------------------------------

inline HANDLE HookEvent(const char* name, MIRANDAHOOK func)
{
    MTLASSERT(name);
    MTLASSERT(func);

    HANDLE hHook = g_pluginLink.GetLink()->HookEvent(name, func);
    if(hHook)
    {
        g_pluginLink.RegisterHandle(CMirandaPluginLink::HOOKED_EVENT, hHook, 0);
    }

    return hHook;
}
//--------------------------------------------------------------------------------------------------

template<class U, class W, class L, class Ret>
inline HANDLE HookEvent(const char* name, U* pObject, Ret (U::*func)(W, L))
{
    MTLASSERT(name);
    MTLASSERT(pObject);
    MTLASSERT(func);

    CCallThunk* pThunk = g_pluginLink.CreateThunk(pObject, func);
    MTLASSERT(pThunk);

    HANDLE hEvent = g_pluginLink.GetLink()->HookEvent(name, pThunk->GetCodeAddress());
    MTLASSERT(hEvent);
    if(hEvent)
    {
        g_pluginLink.RegisterHandle(CMirandaPluginLink::HOOKED_EVENT, hEvent, pThunk);
    }
    else
    {
        delete pThunk;
    }

    return hEvent;
}
//--------------------------------------------------------------------------------------------------

inline HANDLE HookEventMessage(const char* name, HWND hWnd, UINT message)
{
    MTLASSERT(name);
    MTLASSERT(hWnd);

    HANDLE hHook = g_pluginLink.GetLink()->HookEventMessage(name, hWnd, message);
    if(hHook)
    {
        g_pluginLink.RegisterHandle(CMirandaPluginLink::HOOKED_EVENT, hHook, 0);
    }

    return hHook;
}
//--------------------------------------------------------------------------------------------------

inline int SetHookDefaultForHookableEvent(HANDLE hEvent, MIRANDAHOOK func)
{
    MTLASSERT(hEvent);
    MTLASSERT(func);

    return g_pluginLink.GetLink()->SetHookDefaultForHookableEvent(hEvent, func);
}
//--------------------------------------------------------------------------------------------------

template<class U>
inline int SetHookDefaultForHookableEvent(HANDLE hEvent, U* pObject,
    int (U::*func)(WPARAM, LPARAM))
{
    MTLASSERT(hEvent);
    MTLASSERT(pObject);
    MTLASSERT(func);

    CCallThunk* pThunk = g_pluginLink.CreateThunk(pObject, func);
    MTLASSERT(pThunk);

    int res = g_pluginLink.GetLink()->SetHookDefaultForHookableEvent(hHook, pThunk->GetCodeAddress());
    if(0 == res)
    {
        g_pluginLink.RegisterHandle(CMirandaPluginLink::DEFAULT_HOOK, hHook, pThunk);
    }
    else
    {
        delete pThunk;
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int UnhookEvent(HANDLE hHook)
{
    MTLASSERT(hHook);

    int res = g_pluginLink.GetLink()->UnhookEvent(hHook);

    if(0 == res)
    {
        g_pluginLink.UnregisterHandle(CMirandaPluginLink::HOOKED_EVENT, hHook);
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline HANDLE CreateServiceFunction(const char* name, MIRANDASERVICE func)
{
    MTLASSERT(name);
    MTLASSERT(func);

    HANDLE hService = g_pluginLink.GetLink()->CreateServiceFunction(name, func);
    if(hService)
    {
        g_pluginLink.RegisterHandle(CMirandaPluginLink::SERVICE, hService, 0);
    }

    return hService;
}
//--------------------------------------------------------------------------------------------------

template<class U, class W, class L, class Ret>
inline HANDLE CreateServiceFunction(const char* name, U* pObject,
    Ret (U::*func)(W, L))
{
    MTLASSERT(name);
    MTLASSERT(pObject);
    MTLASSERT(func);

    CCallThunk* pThunk = g_pluginLink.CreateThunk(pObject, func);
    MTLASSERT(pThunk);

    HANDLE hService = g_pluginLink.GetLink()->CreateServiceFunction(name, pThunk->GetCodeAddress());
    MTLASSERT(hService);
    if(hService)
    {
        g_pluginLink.RegisterHandle(CMirandaPluginLink::SERVICE, hService, pThunk);
    }
    else
    {
        delete pThunk;
    }

    return hService;
}
//--------------------------------------------------------------------------------------------------

inline HANDLE CreateMainThreadServiceFunction(const char* name, MIRANDASERVICE func)
{
    MTLASSERT(name);
    MTLASSERT(func);

    CMirandaPluginLink::MainThreadProxy* pProxy = new CMirandaPluginLink::MainThreadProxy(name, func);
    MTLASSERT(pProxy);
    CCallThunk* pThunk = g_pluginLink.CreateThunk(pProxy, &CMirandaPluginLink::MainThreadProxy::Call);
    MTLASSERT(pThunk);

    HANDLE hService = g_pluginLink.GetLink()->CreateServiceFunction(name, pThunk->GetCodeAddress());
    MTLASSERT(hService);
    if(hService)
    {
        g_pluginLink.RegisterHandle(CMirandaPluginLink::SERVICE, hService, pThunk, pProxy);
    }
    else
    {
        delete pThunk;
        delete pProxy;
    }

    return hService;
}
//--------------------------------------------------------------------------------------------------

template<class U, class W, class L, class Ret>
inline HANDLE CreateMainThreadServiceFunction(const char* name, U* pObject,
    Ret (U::*func)(W, L))
{
    MTLASSERT(name);
    MTLASSERT(pObject);
    MTLASSERT(func);

    CMirandaPluginLink::MainThreadProxy* pProxy = new CMirandaPluginLink::MainThreadProxy(name, pObject, func);
    MTLASSERT(pProxy);
    CCallThunk* pThunk = g_pluginLink.CreateThunk(pProxy, &CMirandaPluginLink::MainThreadProxy::Call);
    MTLASSERT(pThunk);

    HANDLE hService = g_pluginLink.GetLink()->CreateServiceFunction(name, pThunk->GetCodeAddress());
    MTLASSERT(hService);
    if(hService)
    {
        g_pluginLink.RegisterHandle(CMirandaPluginLink::SERVICE, hService, pThunk, pProxy);
    }
    else
    {
        delete pThunk;
        delete pProxy;
    }

    return hService;
}
//--------------------------------------------------------------------------------------------------

inline int DestroyServiceFunction(HANDLE hService)
{
    MTLASSERT(hService);

    int res = g_pluginLink.GetLink()->DestroyServiceFunction(hService);

    if(0 == res)
    {
        g_pluginLink.UnregisterHandle(CMirandaPluginLink::SERVICE, hService);
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

inline int CallService(const char* name, WPARAM wparam, LPARAM lparam)
{
    MTLASSERT(name);

    return g_pluginLink.GetLink()->CallService(name, wparam, lparam);
}
//--------------------------------------------------------------------------------------------------

inline bool ServiceExists(const char* name)
{
    MTLASSERT(name);

    return 0 != g_pluginLink.GetLink()->ServiceExists(name);
}
//--------------------------------------------------------------------------------------------------

inline int CallServiceSync(const char* name, WPARAM wparam, LPARAM lparam)
{
    MTLASSERT(name);

    return g_pluginLink.GetLink()->CallServiceSync(name, wparam, lparam);
}
//--------------------------------------------------------------------------------------------------

inline int CallFunctionAsync(void (__stdcall *func)(void*), void* param)
{
    MTLASSERT(func);

    return g_pluginLink.GetLink()->CallFunctionAsync(func, param);
}
//--------------------------------------------------------------------------------------------------




//
// PLUGININFO
//
#define BEGIN_PLUGIN_INFO(theClass) \
    static PLUGININFOEX* GetPluginInfoEx(void) \
    { \
        static PLUGININFOEX pluginInfo = { 0 }; \
        if(!pluginInfo.cbSize) \
        { \
            pluginInfo.cbSize = sizeof(pluginInfo);

#define PLUGIN_NAME(value)          pluginInfo.shortName = value;
#define PLUGIN_VERSION(value)       pluginInfo.version = value;
#define PLUGIN_DESCRIPTION(value)   pluginInfo.description = value;
#define PLUGIN_AUTHOR(value)        pluginInfo.author = value;
#define PLUGIN_EMAIL(value)         pluginInfo.authorEmail = value;
#define PLUGIN_COPYRIGHT(value)     pluginInfo.copyright = value;
#define PLUGIN_HOMEPAGE(value)      pluginInfo.homepage = value;
#define PLUGIN_FLAGS(value)         pluginInfo.flags = value;
#define PLUGIN_UUID(value)          MUUID muuid = value; pluginInfo.uuid = muuid;

#define END_PLUGIN_INFO() \
        } \
        return &pluginInfo; \
    } \
    static PLUGININFO* GetPluginInfo(void) \
    { \
        static PLUGININFO pluginInfo = { 0 }; \
        if(!pluginInfo.cbSize) \
        { \
            CopyMemory(&pluginInfo, GetPluginInfoEx(), sizeof(pluginInfo)); \
            pluginInfo.cbSize = sizeof(pluginInfo); \
        } \
        return &pluginInfo; \
    }
//--------------------------------------------------------------------------------------------------

//
// Plugin interfaces
//
#define BEGIN_PLUGIN_INTERFACES(theClass) \
    static const MUUID* GetPluginInterfaces(void) \
    { \
        static MUUID interfaces[] = { \

#define PLUGIN_INTERFACE(value) value,

#define END_PLUGIN_INTERFACES() \
            MIID_LAST \
            }; \
        return interfaces; \
    }
//--------------------------------------------------------------------------------------------------

//
// Derive your class from CMirandaPlugin
//
template<class T>
class MTL_NO_VTABLE CMirandaPlugin
{
public:
    typedef int (T::*HookFunction)(WPARAM, LPARAM);
    typedef int (T::*ServiceFunction)(WPARAM, LPARAM);

                        CMirandaPlugin(void);

    bool                DllMain(HINSTANCE hInst, DWORD reason);
    PLUGININFO*         MirandaPluginInfo(DWORD mirandaVersion);
    PLUGININFOEX*       MirandaPluginInfoEx(DWORD mirandaVersion);
    const MUUID*        MirandaPluginInterfaces();
    int                 Load(PLUGINLINK* link);
    int                 Unload(void);

    //
    // Helper functions allowing to leave out "this" pointer
    //
    template<class W, class L, class Ret>
    HANDLE              HookEvent(const char* name, Ret (T::*func)(W, L));
    int                 SetHookDefaultForHookableEvent(HANDLE hHook, HookFunction func);

    template<class W, class L, class Ret>
    HANDLE              CreateServiceFunction(const char* name, Ret (T::*func)(W, L));
    template<class W, class L, class Ret>
    HANDLE              CreateMainThreadServiceFunction(const char* name, Ret (T::*func)(W, L));
    HANDLE              CreateTransientServiceFunction(const char* name, ServiceFunction func);

    HINSTANCE           Instance(void) const;
    DWORD               MirandaVersion(void) const;

    //
    // Override any of these methods
    //
    int                 OnLoad(void);
    int                 OnUnload(void);

private:
    HINSTANCE           m_hInstance;
    DWORD               m_mirandaVersion;
};
//--------------------------------------------------------------------------------------------------

template<class T>
inline CMirandaPlugin<T>::CMirandaPlugin(void) :
    m_hInstance(0),
    m_mirandaVersion(0)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline bool CMirandaPlugin<T>::DllMain(HINSTANCE hInst, DWORD reason)
{
    if(DLL_PROCESS_ATTACH == reason)
    {
        m_hInstance = hInst;
    }

    return true;
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline PLUGININFO* CMirandaPlugin<T>::MirandaPluginInfo(DWORD mirandaVersion)
{
    m_mirandaVersion = mirandaVersion;
    return T::GetPluginInfo();
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline PLUGININFOEX* CMirandaPlugin<T>::MirandaPluginInfoEx(DWORD mirandaVersion)
{
    m_mirandaVersion = mirandaVersion;
    return T::GetPluginInfoEx();
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline const MUUID* CMirandaPlugin<T>::MirandaPluginInterfaces()
{
    return T::GetPluginInterfaces();
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline int CMirandaPlugin<T>::Load(PLUGINLINK* link)
{
    g_pluginLink.Init(link);

    T* pThis = static_cast<T*>(this);
    return pThis->OnLoad();
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline int CMirandaPlugin<T>::Unload(void)
{
    T* pThis = static_cast<T*>(this);
    int res = pThis->OnUnload();

    g_pluginLink.Terminate();

    return res;
}
//--------------------------------------------------------------------------------------------------

template<class T>
template<class W, class L, class Ret>
inline HANDLE CMirandaPlugin<T>::HookEvent(const char* name, Ret (T::*func)(W, L))
{
    MTLASSERT(name);
    MTLASSERT(func);

    return ::HookEvent(name, static_cast<T*>(this), func);
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline int CMirandaPlugin<T>::SetHookDefaultForHookableEvent(HANDLE hEvent, HookFunction func)
{
    MTLASSERT(hEvent);
    MTLASSERT(func);

    return ::SetHookDefaultForHookableEvent(hEvent, static_cast<T*>(this), func);
}
//--------------------------------------------------------------------------------------------------

template<class T>
template<class W, class L, class Ret>
inline HANDLE CMirandaPlugin<T>::CreateServiceFunction(const char* name, Ret (T::*func)(W, L))
{
    MTLASSERT(name);
    MTLASSERT(func);

    return ::CreateServiceFunction(name, static_cast<T*>(this), func);
}
//--------------------------------------------------------------------------------------------------

template<class T>
template<class W, class L, class Ret>
inline HANDLE CMirandaPlugin<T>::CreateMainThreadServiceFunction(const char* name, Ret (T::*func)(W, L))
{
    MTLASSERT(name);
    MTLASSERT(func);

    return ::CreateMainThreadServiceFunction(name, static_cast<T*>(this), func);
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline HINSTANCE CMirandaPlugin<T>::Instance(void) const
{
    return m_hInstance;
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline DWORD CMirandaPlugin<T>::MirandaVersion(void) const
{
    return m_mirandaVersion;
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline int CMirandaPlugin<T>::OnLoad(void)
{
    // Default implementation. Override in derived classes.
    return 0;
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline int CMirandaPlugin<T>::OnUnload(void)
{
    // Default implementation. Override in derived classes.
    return 0;
}
//--------------------------------------------------------------------------------------------------


#endif // __MTL_MTL_H__
//--------------------------------------------------------------------------------------------------
