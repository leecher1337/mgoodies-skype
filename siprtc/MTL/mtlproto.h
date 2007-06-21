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

#if !defined(__MTL_MTLPROTO_H__)
#define __MTL_MTLPROTO_H__

#pragma once

#include "mtl.h"
#include "m_protocols.h"
#include "m_protomod.h"
#include "m_protosvc.h"
//--------------------------------------------------------------------------------------------------

//
// Protocol implementation
//

#define BEGIN_CAPS_MAP(theClass) \
    int GetCaps(int flag, LPARAM) \
    { \
        int res = 0; \
        switch(flag) \
        { \

// Generic flag
#define CAPS_FLAG(flag, caps) \
        case (flag): res = (caps); break;

#define CAPS_BASE(caps) \
        case PFLAGNUM_1: res = (caps); break;

#define CAPS_SUPPORTED_STATUSES(statuses) \
        case PFLAGNUM_2: res = (statuses); break;

#define CAPS_SUPPORTED_AWAY_STATUSES(statuses) \
        case PFLAGNUM_3: res = (statuses); break;

#define CAPS_MISC(caps) \
        case PFLAGNUM_4: res = (caps); break;

#define CAPS_UNIQUEIDTEXT(text) \
        case PFLAG_UNIQUEIDTEXT: res = PtrToInt(text); break;

#define CAPS_MAXCONTACTSPERPACKET(size) \
        case PFLAG_MAXCONTACTSPERPACKET: res = (size); break;

#define CAPS_UNIQUEIDSETTING(value) \
        case PFLAG_UNIQUEIDSETTING: res = PtrToInt(value); break;

#define CAPS_MAXLENOFMESSAGE(length) \
        case PFLAG_MAXLENOFMESSAGE : res = (length); break;

#define END_CAPS_MAP() \
        } \
        return res; \
    }
//--------------------------------------------------------------------------------------------------

#define BEGIN_PROTO_SERVICES_MAP(theClass) \
    void RegisterProtoServicesMap(const char* protocol) \
    { \
        typedef theClass TClass; \
        char serviceName[300];

#define PROTO_SERVICE(id, func) \
        mir_snprintf(serviceName, sizeof(serviceName), "%s%s", protocol, id); \
        ::CreateServiceFunction(serviceName, static_cast<TClass*>(this), &TClass::func);

#define PROTO_SERVICE_MTHREAD(id, func) \
        mir_snprintf(serviceName, sizeof(serviceName), "%s%s", protocol, id); \
        ::CreateMainThreadServiceFunction(serviceName, static_cast<TClass*>(this), &TClass::func);

#define CHAIN_PROTO_SERVICES_MAP(member) \
        member.RegisterProtoServicesMap(protocol);

#define END_PROTO_SERVICES_MAP() \
    }
//--------------------------------------------------------------------------------------------------


/*
//
// Async broadcasting
//
inline void __stdcall AsyncBroadcastAckWorker(void* param)
{
    MTLASSERT(param);

    if(param)
    {
        std::auto_ptr<ACKDATA> ack(reinterpret_cast<ACKDATA*>(param));
        CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)ack.get());
    }
}
//--------------------------------------------------------------------------------------------------

//
// WARNING! DO NOT PASS POINTERS AS THE LAST PARAMETERS!
//
inline void ProtoAsyncBroadcastAck(const char* proto, HANDLE hContact, int type, int result,
    HANDLE hProcess, LPARAM param)
{
    MTLASSERT(proto);
    MTLASSERT(IsBadReadPtr((const void*)param, sizeof(LPARAM))); // We don't expect pointers here!

    ACKDATA* ack = new ACKDATA;

    ZeroMemory(ack, sizeof(ACKDATA));
    ack->cbSize   = sizeof(ACKDATA);
    ack->szModule = proto;
    ack->hContact = hContact;
    ack->type     = type;
    ack->result   = result;
    ack->hProcess = hProcess;
    ack->lParam   = param;

    CallFunctionAsync(AsyncBroadcastAckWorker, ack);
}
//--------------------------------------------------------------------------------------------------
*/

template<class T>
class MTL_NO_VTABLE CMirandaProtocolPlugin : public CMirandaPlugin<T>
{
public:
                        CMirandaProtocolPlugin(void);

    void                RegisterProtoServices(void);

    const char*         GetProtocolName(void) const;

protected:
    bool                GenerateProtocolName(char* name, unsigned nameSize);

private:
    char                m_protocolName[32];
};
//--------------------------------------------------------------------------------------------------

template<class T>
inline CMirandaProtocolPlugin<T>::CMirandaProtocolPlugin(void)
{
    m_protocolName[0] = 0;
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline void CMirandaProtocolPlugin<T>::RegisterProtoServices(void)
{
    T* pT = static_cast<T*>(this);

    bool res = pT->GenerateProtocolName(m_protocolName, sizeof(m_protocolName) / sizeof(m_protocolName[0]));
    MTLASSERT(res);
    MTLASSERT(m_protocolName[0]);

    PROTOCOLDESCRIPTOR pd;
    pd.cbSize = sizeof(pd);
    pd.szName = const_cast<char*>(pT->GetProtocolName());
    pd.type = PROTOTYPE_PROTOCOL;

    CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

    pT->RegisterProtoServicesMap(pT->GetProtocolName());
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline const char* CMirandaProtocolPlugin<T>::GetProtocolName(void) const
{
    MTLASSERT(m_protocolName[0]);
    return m_protocolName;
}
//--------------------------------------------------------------------------------------------------

template<class T>
inline bool CMirandaProtocolPlugin<T>::GenerateProtocolName(char* name, unsigned nameSize)
{
    MTLASSERT(name);
    MTLASSERT(nameSize > 0);

    bool res = false;
    name[0] = 0;

    char fileName[MAX_PATH];
    if(GetModuleFileNameA(Instance(), fileName, sizeof(fileName) / sizeof(fileName[0])) > 0)
    {
        char* slash = strrchr(fileName, '\\');
        char* dot = strrchr(fileName, '.');
        if(slash && dot && slash < dot)
        {
            *dot = 0;
            MTLVERIFY(S_OK == StringCchCopyA(name, nameSize, slash + 1));
            MTLASSERT(lstrlenA(name) > 0);
            CharUpperBuffA(name, lstrlenA(name));
            res = true;
        }
    }

    return res;
}
//--------------------------------------------------------------------------------------------------

#endif // __MTL_MTLPROTO_H__
//--------------------------------------------------------------------------------------------------
