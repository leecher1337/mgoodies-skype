

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __COMServer2Helper_h__
#define __COMServer2Helper_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IClientHelper07_FWD_DEFINED__
#define __IClientHelper07_FWD_DEFINED__
typedef interface IClientHelper07 IClientHelper07;
#endif 	/* __IClientHelper07_FWD_DEFINED__ */


#ifndef __IServerHelper07_FWD_DEFINED__
#define __IServerHelper07_FWD_DEFINED__
typedef interface IServerHelper07 IServerHelper07;
#endif 	/* __IServerHelper07_FWD_DEFINED__ */


#ifndef ___IClientHelperEvents07_FWD_DEFINED__
#define ___IClientHelperEvents07_FWD_DEFINED__
typedef interface _IClientHelperEvents07 _IClientHelperEvents07;
#endif 	/* ___IClientHelperEvents07_FWD_DEFINED__ */


#ifndef __ApplicationHelper07_FWD_DEFINED__
#define __ApplicationHelper07_FWD_DEFINED__

#ifdef __cplusplus
typedef class ApplicationHelper07 ApplicationHelper07;
#else
typedef struct ApplicationHelper07 ApplicationHelper07;
#endif /* __cplusplus */

#endif 	/* __ApplicationHelper07_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IClientHelper07_INTERFACE_DEFINED__
#define __IClientHelper07_INTERFACE_DEFINED__

/* interface IClientHelper07 */
/* [unique][helpstring][uuid][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IClientHelper07;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D63D265A-88DB-4D95-8B1C-797B871CD6D1")
    IClientHelper07 : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Running( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Server( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IClientHelper07Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClientHelper07 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClientHelper07 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClientHelper07 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IClientHelper07 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IClientHelper07 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IClientHelper07 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IClientHelper07 * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Running )( 
            IClientHelper07 * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Server )( 
            IClientHelper07 * This,
            /* [retval][out] */ IDispatch **pVal);
        
        END_INTERFACE
    } IClientHelper07Vtbl;

    interface IClientHelper07
    {
        CONST_VTBL struct IClientHelper07Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClientHelper07_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClientHelper07_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClientHelper07_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClientHelper07_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IClientHelper07_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IClientHelper07_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IClientHelper07_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IClientHelper07_get_Running(This,pVal)	\
    (This)->lpVtbl -> get_Running(This,pVal)

#define IClientHelper07_get_Server(This,pVal)	\
    (This)->lpVtbl -> get_Server(This,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IClientHelper07_get_Running_Proxy( 
    IClientHelper07 * This,
    /* [retval][out] */ VARIANT_BOOL *pVal);


void __RPC_STUB IClientHelper07_get_Running_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IClientHelper07_get_Server_Proxy( 
    IClientHelper07 * This,
    /* [retval][out] */ IDispatch **pVal);


void __RPC_STUB IClientHelper07_get_Server_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IClientHelper07_INTERFACE_DEFINED__ */


#ifndef __IServerHelper07_INTERFACE_DEFINED__
#define __IServerHelper07_INTERFACE_DEFINED__

/* interface IServerHelper07 */
/* [unique][helpstring][uuid][hidden][oleautomation][object] */ 


EXTERN_C const IID IID_IServerHelper07;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("623D88DD-8B7E-4ce7-A54A-C5FBD8A7031F")
    IServerHelper07 : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RegisterServerObject( 
            /* [in] */ IUnknown *pObj,
            /* [retval][out] */ DWORD *pdwCookie) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RevokeServerObject( 
            /* [in] */ DWORD dwCookie) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IServerHelper07Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServerHelper07 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServerHelper07 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServerHelper07 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RegisterServerObject )( 
            IServerHelper07 * This,
            /* [in] */ IUnknown *pObj,
            /* [retval][out] */ DWORD *pdwCookie);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RevokeServerObject )( 
            IServerHelper07 * This,
            /* [in] */ DWORD dwCookie);
        
        END_INTERFACE
    } IServerHelper07Vtbl;

    interface IServerHelper07
    {
        CONST_VTBL struct IServerHelper07Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServerHelper07_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServerHelper07_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServerHelper07_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServerHelper07_RegisterServerObject(This,pObj,pdwCookie)	\
    (This)->lpVtbl -> RegisterServerObject(This,pObj,pdwCookie)

#define IServerHelper07_RevokeServerObject(This,dwCookie)	\
    (This)->lpVtbl -> RevokeServerObject(This,dwCookie)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IServerHelper07_RegisterServerObject_Proxy( 
    IServerHelper07 * This,
    /* [in] */ IUnknown *pObj,
    /* [retval][out] */ DWORD *pdwCookie);


void __RPC_STUB IServerHelper07_RegisterServerObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IServerHelper07_RevokeServerObject_Proxy( 
    IServerHelper07 * This,
    /* [in] */ DWORD dwCookie);


void __RPC_STUB IServerHelper07_RevokeServerObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IServerHelper07_INTERFACE_DEFINED__ */



#ifndef __Foobar2000Helper_LIBRARY_DEFINED__
#define __Foobar2000Helper_LIBRARY_DEFINED__

/* library Foobar2000Helper */
/* [helpstring][uuid][version] */ 


EXTERN_C const IID LIBID_Foobar2000Helper;

#ifndef ___IClientHelperEvents07_DISPINTERFACE_DEFINED__
#define ___IClientHelperEvents07_DISPINTERFACE_DEFINED__

/* dispinterface _IClientHelperEvents07 */
/* [helpstring][uuid][hidden] */ 


EXTERN_C const IID DIID__IClientHelperEvents07;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("112498DE-9039-4138-91E9-4B9386309966")
    _IClientHelperEvents07 : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IClientHelperEvents07Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IClientHelperEvents07 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IClientHelperEvents07 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IClientHelperEvents07 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IClientHelperEvents07 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IClientHelperEvents07 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IClientHelperEvents07 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IClientHelperEvents07 * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IClientHelperEvents07Vtbl;

    interface _IClientHelperEvents07
    {
        CONST_VTBL struct _IClientHelperEvents07Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IClientHelperEvents07_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IClientHelperEvents07_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IClientHelperEvents07_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IClientHelperEvents07_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IClientHelperEvents07_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IClientHelperEvents07_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IClientHelperEvents07_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IClientHelperEvents07_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_ApplicationHelper07;

#ifdef __cplusplus

class DECLSPEC_UUID("23D75BD0-096D-49F7-83F8-EE21ACFC6A11")
ApplicationHelper07;
#endif
#endif /* __Foobar2000Helper_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


