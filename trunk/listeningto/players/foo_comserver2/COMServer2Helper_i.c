

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Mon Jul 31 12:23:07 2006
 */
/* Compiler settings for .\COMServer2Helper.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IClientHelper07,0xD63D265A,0x88DB,0x4D95,0x8B,0x1C,0x79,0x7B,0x87,0x1C,0xD6,0xD1);


MIDL_DEFINE_GUID(IID, IID_IServerHelper07,0x623D88DD,0x8B7E,0x4ce7,0xA5,0x4A,0xC5,0xFB,0xD8,0xA7,0x03,0x1F);


MIDL_DEFINE_GUID(IID, LIBID_Foobar2000Helper,0x332869AA,0x4769,0x46C5,0xBE,0x50,0x88,0x86,0x69,0x27,0xA2,0x3D);


MIDL_DEFINE_GUID(IID, DIID__IClientHelperEvents07,0x112498DE,0x9039,0x4138,0x91,0xE9,0x4B,0x93,0x86,0x30,0x99,0x66);


MIDL_DEFINE_GUID(CLSID, CLSID_ApplicationHelper07,0x23D75BD0,0x096D,0x49F7,0x83,0xF8,0xEE,0x21,0xAC,0xFC,0x6A,0x11);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



