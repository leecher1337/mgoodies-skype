

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Mon Jul 31 21:11:35 2006
 */
/* Compiler settings for .\foobar2000.idl:
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


#ifndef __foobar2000_h_h__
#define __foobar2000_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IVBApplication_FWD_DEFINED__
#define __IVBApplication_FWD_DEFINED__
typedef interface IVBApplication IVBApplication;
#endif 	/* __IVBApplication_FWD_DEFINED__ */


#ifndef __IVBMediaLibrary_FWD_DEFINED__
#define __IVBMediaLibrary_FWD_DEFINED__
typedef interface IVBMediaLibrary IVBMediaLibrary;
#endif 	/* __IVBMediaLibrary_FWD_DEFINED__ */


#ifndef __IVBPlayback_FWD_DEFINED__
#define __IVBPlayback_FWD_DEFINED__
typedef interface IVBPlayback IVBPlayback;
#endif 	/* __IVBPlayback_FWD_DEFINED__ */


#ifndef __IVBPlaybackOrders_FWD_DEFINED__
#define __IVBPlaybackOrders_FWD_DEFINED__
typedef interface IVBPlaybackOrders IVBPlaybackOrders;
#endif 	/* __IVBPlaybackOrders_FWD_DEFINED__ */


#ifndef __IVBPlaybackSettings_FWD_DEFINED__
#define __IVBPlaybackSettings_FWD_DEFINED__
typedef interface IVBPlaybackSettings IVBPlaybackSettings;
#endif 	/* __IVBPlaybackSettings_FWD_DEFINED__ */


#ifndef __IVBPlaylist_FWD_DEFINED__
#define __IVBPlaylist_FWD_DEFINED__
typedef interface IVBPlaylist IVBPlaylist;
#endif 	/* __IVBPlaylist_FWD_DEFINED__ */


#ifndef __IVBPlaylists_FWD_DEFINED__
#define __IVBPlaylists_FWD_DEFINED__
typedef interface IVBPlaylists IVBPlaylists;
#endif 	/* __IVBPlaylists_FWD_DEFINED__ */


#ifndef __IVBReplaygainSettings_FWD_DEFINED__
#define __IVBReplaygainSettings_FWD_DEFINED__
typedef interface IVBReplaygainSettings IVBReplaygainSettings;
#endif 	/* __IVBReplaygainSettings_FWD_DEFINED__ */


#ifndef __IVBTrack_FWD_DEFINED__
#define __IVBTrack_FWD_DEFINED__
typedef interface IVBTrack IVBTrack;
#endif 	/* __IVBTrack_FWD_DEFINED__ */


#ifndef __IVBTracks_FWD_DEFINED__
#define __IVBTracks_FWD_DEFINED__
typedef interface IVBTracks IVBTracks;
#endif 	/* __IVBTracks_FWD_DEFINED__ */


#ifndef ___IVBPlaybackEvents_FWD_DEFINED__
#define ___IVBPlaybackEvents_FWD_DEFINED__
typedef interface _IVBPlaybackEvents _IVBPlaybackEvents;
#endif 	/* ___IVBPlaybackEvents_FWD_DEFINED__ */


#ifndef ___IVBPlaybackSettingsEvents_FWD_DEFINED__
#define ___IVBPlaybackSettingsEvents_FWD_DEFINED__
typedef interface _IVBPlaybackSettingsEvents _IVBPlaybackSettingsEvents;
#endif 	/* ___IVBPlaybackSettingsEvents_FWD_DEFINED__ */


#ifndef ___IVBPlaylistEvents_FWD_DEFINED__
#define ___IVBPlaylistEvents_FWD_DEFINED__
typedef interface _IVBPlaylistEvents _IVBPlaylistEvents;
#endif 	/* ___IVBPlaylistEvents_FWD_DEFINED__ */


#ifndef ___IVBPlaylistsEvents_FWD_DEFINED__
#define ___IVBPlaylistsEvents_FWD_DEFINED__
typedef interface _IVBPlaylistsEvents _IVBPlaylistsEvents;
#endif 	/* ___IVBPlaylistsEvents_FWD_DEFINED__ */


#ifndef __Application07_FWD_DEFINED__
#define __Application07_FWD_DEFINED__

#ifdef __cplusplus
typedef class Application07 Application07;
#else
typedef struct Application07 Application07;
#endif /* __cplusplus */

#endif 	/* __Application07_FWD_DEFINED__ */


#ifndef __MediaLibrary07_FWD_DEFINED__
#define __MediaLibrary07_FWD_DEFINED__

#ifdef __cplusplus
typedef class MediaLibrary07 MediaLibrary07;
#else
typedef struct MediaLibrary07 MediaLibrary07;
#endif /* __cplusplus */

#endif 	/* __MediaLibrary07_FWD_DEFINED__ */


#ifndef __Playback07_FWD_DEFINED__
#define __Playback07_FWD_DEFINED__

#ifdef __cplusplus
typedef class Playback07 Playback07;
#else
typedef struct Playback07 Playback07;
#endif /* __cplusplus */

#endif 	/* __Playback07_FWD_DEFINED__ */


#ifndef __PlaybackOrders07_FWD_DEFINED__
#define __PlaybackOrders07_FWD_DEFINED__

#ifdef __cplusplus
typedef class PlaybackOrders07 PlaybackOrders07;
#else
typedef struct PlaybackOrders07 PlaybackOrders07;
#endif /* __cplusplus */

#endif 	/* __PlaybackOrders07_FWD_DEFINED__ */


#ifndef __PlaybackSettings07_FWD_DEFINED__
#define __PlaybackSettings07_FWD_DEFINED__

#ifdef __cplusplus
typedef class PlaybackSettings07 PlaybackSettings07;
#else
typedef struct PlaybackSettings07 PlaybackSettings07;
#endif /* __cplusplus */

#endif 	/* __PlaybackSettings07_FWD_DEFINED__ */


#ifndef __Playlist07_FWD_DEFINED__
#define __Playlist07_FWD_DEFINED__

#ifdef __cplusplus
typedef class Playlist07 Playlist07;
#else
typedef struct Playlist07 Playlist07;
#endif /* __cplusplus */

#endif 	/* __Playlist07_FWD_DEFINED__ */


#ifndef __Playlists07_FWD_DEFINED__
#define __Playlists07_FWD_DEFINED__

#ifdef __cplusplus
typedef class Playlists07 Playlists07;
#else
typedef struct Playlists07 Playlists07;
#endif /* __cplusplus */

#endif 	/* __Playlists07_FWD_DEFINED__ */


#ifndef __ReplaygainSettings07_FWD_DEFINED__
#define __ReplaygainSettings07_FWD_DEFINED__

#ifdef __cplusplus
typedef class ReplaygainSettings07 ReplaygainSettings07;
#else
typedef struct ReplaygainSettings07 ReplaygainSettings07;
#endif /* __cplusplus */

#endif 	/* __ReplaygainSettings07_FWD_DEFINED__ */


#ifndef __Track07_FWD_DEFINED__
#define __Track07_FWD_DEFINED__

#ifdef __cplusplus
typedef class Track07 Track07;
#else
typedef struct Track07 Track07;
#endif /* __cplusplus */

#endif 	/* __Track07_FWD_DEFINED__ */


#ifndef __Tracks07_FWD_DEFINED__
#define __Tracks07_FWD_DEFINED__

#ifdef __cplusplus
typedef class Tracks07 Tracks07;
#else
typedef struct Tracks07 Tracks07;
#endif /* __cplusplus */

#endif 	/* __Tracks07_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __Foobar2000_LIBRARY_DEFINED__
#define __Foobar2000_LIBRARY_DEFINED__

/* library Foobar2000 */
/* [version][helpstring][uuid] */ 






























/* [helpstring] */ 
enum fbDisplayLevel
    {	fbDisplayLevelNone	= 0,
	fbDisplayLevelBasic	= 1,
	fbDisplayLevelTitles	= 2,
	fbDisplayLevelAll	= 3
    } ;
/* [helpstring] */ 
enum fbStopReason
    {	fbStopReasonUser	= 0,
	fbStopReasonEOF	= fbStopReasonUser + 1,
	fbStopReasonStartingAnother	= fbStopReasonEOF + 1
    } ;
/* [helpstring] */ 
enum fbPosition
    {	fbPositionFirst	= 0,
	fbPositionLast	= fbPositionFirst + 1,
	fbPositionBefore	= fbPositionLast + 1,
	fbPositionAfter	= fbPositionBefore + 1
    } ;
/* [helpstring] */ 
enum fbReplaygainMode
    {	fbReplaygainModeNone	= 0,
	fbReplaygainModeTrack	= 1,
	fbReplaygainModeAlbum	= 2
    } ;

EXTERN_C const IID LIBID_Foobar2000;

#ifndef __IVBApplication_INTERFACE_DEFINED__
#define __IVBApplication_INTERFACE_DEFINED__

/* interface IVBApplication */
/* [uuid][helpstring][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IVBApplication;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a3177b54-b826-40ec-996d-a30b9a57a3bf")
    IVBApplication : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *strName) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ApplicationPath( 
            /* [retval][out] */ BSTR *strPath) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProfilePath( 
            /* [retval][out] */ BSTR *strPath) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Minimized( 
            /* [retval][out] */ VARIANT_BOOL *bFlag) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Minimized( 
            /* [in] */ VARIANT_BOOL bFlag) = 0;
        
        virtual /* [helpstring][source][id][propget] */ HRESULT STDMETHODCALLTYPE get_Playback( 
            /* [retval][out] */ IVBPlayback **oPlaylist) = 0;
        
        virtual /* [helpstring][source][id][propget] */ HRESULT STDMETHODCALLTYPE get_Playlists( 
            /* [retval][out] */ IVBPlaylists **oPlaylists) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MediaLibrary( 
            /* [retval][out] */ IVBMediaLibrary **oMediaLibrary) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVBApplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVBApplication * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVBApplication * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVBApplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVBApplication * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVBApplication * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVBApplication * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVBApplication * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVBApplication * This,
            /* [retval][out] */ BSTR *strName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ApplicationPath )( 
            IVBApplication * This,
            /* [retval][out] */ BSTR *strPath);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ProfilePath )( 
            IVBApplication * This,
            /* [retval][out] */ BSTR *strPath);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Minimized )( 
            IVBApplication * This,
            /* [retval][out] */ VARIANT_BOOL *bFlag);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Minimized )( 
            IVBApplication * This,
            /* [in] */ VARIANT_BOOL bFlag);
        
        /* [helpstring][source][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Playback )( 
            IVBApplication * This,
            /* [retval][out] */ IVBPlayback **oPlaylist);
        
        /* [helpstring][source][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Playlists )( 
            IVBApplication * This,
            /* [retval][out] */ IVBPlaylists **oPlaylists);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MediaLibrary )( 
            IVBApplication * This,
            /* [retval][out] */ IVBMediaLibrary **oMediaLibrary);
        
        END_INTERFACE
    } IVBApplicationVtbl;

    interface IVBApplication
    {
        CONST_VTBL struct IVBApplicationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVBApplication_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVBApplication_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVBApplication_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVBApplication_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVBApplication_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVBApplication_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVBApplication_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVBApplication_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#define IVBApplication_get_ApplicationPath(This,strPath)	\
    (This)->lpVtbl -> get_ApplicationPath(This,strPath)

#define IVBApplication_get_ProfilePath(This,strPath)	\
    (This)->lpVtbl -> get_ProfilePath(This,strPath)

#define IVBApplication_get_Minimized(This,bFlag)	\
    (This)->lpVtbl -> get_Minimized(This,bFlag)

#define IVBApplication_put_Minimized(This,bFlag)	\
    (This)->lpVtbl -> put_Minimized(This,bFlag)

#define IVBApplication_get_Playback(This,oPlaylist)	\
    (This)->lpVtbl -> get_Playback(This,oPlaylist)

#define IVBApplication_get_Playlists(This,oPlaylists)	\
    (This)->lpVtbl -> get_Playlists(This,oPlaylists)

#define IVBApplication_get_MediaLibrary(This,oMediaLibrary)	\
    (This)->lpVtbl -> get_MediaLibrary(This,oMediaLibrary)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBApplication_get_Name_Proxy( 
    IVBApplication * This,
    /* [retval][out] */ BSTR *strName);


void __RPC_STUB IVBApplication_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBApplication_get_ApplicationPath_Proxy( 
    IVBApplication * This,
    /* [retval][out] */ BSTR *strPath);


void __RPC_STUB IVBApplication_get_ApplicationPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBApplication_get_ProfilePath_Proxy( 
    IVBApplication * This,
    /* [retval][out] */ BSTR *strPath);


void __RPC_STUB IVBApplication_get_ProfilePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBApplication_get_Minimized_Proxy( 
    IVBApplication * This,
    /* [retval][out] */ VARIANT_BOOL *bFlag);


void __RPC_STUB IVBApplication_get_Minimized_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBApplication_put_Minimized_Proxy( 
    IVBApplication * This,
    /* [in] */ VARIANT_BOOL bFlag);


void __RPC_STUB IVBApplication_put_Minimized_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][source][id][propget] */ HRESULT STDMETHODCALLTYPE IVBApplication_get_Playback_Proxy( 
    IVBApplication * This,
    /* [retval][out] */ IVBPlayback **oPlaylist);


void __RPC_STUB IVBApplication_get_Playback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][source][id][propget] */ HRESULT STDMETHODCALLTYPE IVBApplication_get_Playlists_Proxy( 
    IVBApplication * This,
    /* [retval][out] */ IVBPlaylists **oPlaylists);


void __RPC_STUB IVBApplication_get_Playlists_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBApplication_get_MediaLibrary_Proxy( 
    IVBApplication * This,
    /* [retval][out] */ IVBMediaLibrary **oMediaLibrary);


void __RPC_STUB IVBApplication_get_MediaLibrary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVBApplication_INTERFACE_DEFINED__ */


#ifndef __IVBMediaLibrary_INTERFACE_DEFINED__
#define __IVBMediaLibrary_INTERFACE_DEFINED__

/* interface IVBMediaLibrary */
/* [uuid][helpstring][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IVBMediaLibrary;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0386897e-eb1a-4e20-810d-7bdca4f2ff17")
    IVBMediaLibrary : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Rescan( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetTracks( 
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSortedTracks( 
            /* [in] */ BSTR strSortFormat,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVBMediaLibraryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVBMediaLibrary * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVBMediaLibrary * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVBMediaLibrary * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVBMediaLibrary * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVBMediaLibrary * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVBMediaLibrary * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVBMediaLibrary * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Rescan )( 
            IVBMediaLibrary * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetTracks )( 
            IVBMediaLibrary * This,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSortedTracks )( 
            IVBMediaLibrary * This,
            /* [in] */ BSTR strSortFormat,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks);
        
        END_INTERFACE
    } IVBMediaLibraryVtbl;

    interface IVBMediaLibrary
    {
        CONST_VTBL struct IVBMediaLibraryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVBMediaLibrary_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVBMediaLibrary_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVBMediaLibrary_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVBMediaLibrary_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVBMediaLibrary_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVBMediaLibrary_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVBMediaLibrary_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVBMediaLibrary_Rescan(This)	\
    (This)->lpVtbl -> Rescan(This)

#define IVBMediaLibrary_GetTracks(This,strQuery,oTracks)	\
    (This)->lpVtbl -> GetTracks(This,strQuery,oTracks)

#define IVBMediaLibrary_GetSortedTracks(This,strSortFormat,strQuery,oTracks)	\
    (This)->lpVtbl -> GetSortedTracks(This,strSortFormat,strQuery,oTracks)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBMediaLibrary_Rescan_Proxy( 
    IVBMediaLibrary * This);


void __RPC_STUB IVBMediaLibrary_Rescan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBMediaLibrary_GetTracks_Proxy( 
    IVBMediaLibrary * This,
    /* [optional][in] */ VARIANT strQuery,
    /* [retval][out] */ IVBTracks **oTracks);


void __RPC_STUB IVBMediaLibrary_GetTracks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBMediaLibrary_GetSortedTracks_Proxy( 
    IVBMediaLibrary * This,
    /* [in] */ BSTR strSortFormat,
    /* [optional][in] */ VARIANT strQuery,
    /* [retval][out] */ IVBTracks **oTracks);


void __RPC_STUB IVBMediaLibrary_GetSortedTracks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVBMediaLibrary_INTERFACE_DEFINED__ */


#ifndef __IVBPlayback_INTERFACE_DEFINED__
#define __IVBPlayback_INTERFACE_DEFINED__

/* interface IVBPlayback */
/* [uuid][helpstring][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IVBPlayback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ea5b418a-70a5-4962-b578-17f31a3536c2")
    IVBPlayback : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( 
            /* [in] */ VARIANT bPaused) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Next( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Previous( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Random( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Seek( 
            /* [in] */ DOUBLE dSecondsFromStart) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SeekRelative( 
            /* [in] */ DOUBLE dSecondsFromCurrentPosition) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FormatTitle( 
            /* [in] */ BSTR strFormat,
            /* [retval][out] */ BSTR *strText) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FormatTitleEx( 
            /* [in] */ BSTR strFormat,
            /* [in] */ enum fbDisplayLevel lDisplayLevel,
            /* [retval][out] */ BSTR *strText) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsPlaying( 
            /* [retval][out] */ VARIANT_BOOL *bState) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsPaused( 
            /* [retval][out] */ VARIANT_BOOL *bState) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Length( 
            /* [retval][out] */ DOUBLE *dLengthInSeconds) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CanSeek( 
            /* [retval][out] */ VARIANT_BOOL *bFlag) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Position( 
            /* [retval][out] */ DOUBLE *dSecondsFromStart) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Settings( 
            /* [retval][out] */ IVBPlaybackSettings **oPlaybackSettings) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Replaygain( 
            /* [retval][out] */ IVBReplaygainSettings **oReplaygainSettings) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVBPlaybackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVBPlayback * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVBPlayback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVBPlayback * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVBPlayback * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVBPlayback * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVBPlayback * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVBPlayback * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IVBPlayback * This,
            /* [in] */ VARIANT bPaused);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IVBPlayback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IVBPlayback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Next )( 
            IVBPlayback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Previous )( 
            IVBPlayback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Random )( 
            IVBPlayback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IVBPlayback * This,
            /* [in] */ DOUBLE dSecondsFromStart);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SeekRelative )( 
            IVBPlayback * This,
            /* [in] */ DOUBLE dSecondsFromCurrentPosition);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FormatTitle )( 
            IVBPlayback * This,
            /* [in] */ BSTR strFormat,
            /* [retval][out] */ BSTR *strText);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FormatTitleEx )( 
            IVBPlayback * This,
            /* [in] */ BSTR strFormat,
            /* [in] */ enum fbDisplayLevel lDisplayLevel,
            /* [retval][out] */ BSTR *strText);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsPlaying )( 
            IVBPlayback * This,
            /* [retval][out] */ VARIANT_BOOL *bState);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsPaused )( 
            IVBPlayback * This,
            /* [retval][out] */ VARIANT_BOOL *bState);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            IVBPlayback * This,
            /* [retval][out] */ DOUBLE *dLengthInSeconds);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CanSeek )( 
            IVBPlayback * This,
            /* [retval][out] */ VARIANT_BOOL *bFlag);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Position )( 
            IVBPlayback * This,
            /* [retval][out] */ DOUBLE *dSecondsFromStart);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Settings )( 
            IVBPlayback * This,
            /* [retval][out] */ IVBPlaybackSettings **oPlaybackSettings);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Replaygain )( 
            IVBPlayback * This,
            /* [retval][out] */ IVBReplaygainSettings **oReplaygainSettings);
        
        END_INTERFACE
    } IVBPlaybackVtbl;

    interface IVBPlayback
    {
        CONST_VTBL struct IVBPlaybackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVBPlayback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVBPlayback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVBPlayback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVBPlayback_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVBPlayback_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVBPlayback_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVBPlayback_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVBPlayback_Start(This,bPaused)	\
    (This)->lpVtbl -> Start(This,bPaused)

#define IVBPlayback_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IVBPlayback_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IVBPlayback_Next(This)	\
    (This)->lpVtbl -> Next(This)

#define IVBPlayback_Previous(This)	\
    (This)->lpVtbl -> Previous(This)

#define IVBPlayback_Random(This)	\
    (This)->lpVtbl -> Random(This)

#define IVBPlayback_Seek(This,dSecondsFromStart)	\
    (This)->lpVtbl -> Seek(This,dSecondsFromStart)

#define IVBPlayback_SeekRelative(This,dSecondsFromCurrentPosition)	\
    (This)->lpVtbl -> SeekRelative(This,dSecondsFromCurrentPosition)

#define IVBPlayback_FormatTitle(This,strFormat,strText)	\
    (This)->lpVtbl -> FormatTitle(This,strFormat,strText)

#define IVBPlayback_FormatTitleEx(This,strFormat,lDisplayLevel,strText)	\
    (This)->lpVtbl -> FormatTitleEx(This,strFormat,lDisplayLevel,strText)

#define IVBPlayback_get_IsPlaying(This,bState)	\
    (This)->lpVtbl -> get_IsPlaying(This,bState)

#define IVBPlayback_get_IsPaused(This,bState)	\
    (This)->lpVtbl -> get_IsPaused(This,bState)

#define IVBPlayback_get_Length(This,dLengthInSeconds)	\
    (This)->lpVtbl -> get_Length(This,dLengthInSeconds)

#define IVBPlayback_get_CanSeek(This,bFlag)	\
    (This)->lpVtbl -> get_CanSeek(This,bFlag)

#define IVBPlayback_get_Position(This,dSecondsFromStart)	\
    (This)->lpVtbl -> get_Position(This,dSecondsFromStart)

#define IVBPlayback_get_Settings(This,oPlaybackSettings)	\
    (This)->lpVtbl -> get_Settings(This,oPlaybackSettings)

#define IVBPlayback_get_Replaygain(This,oReplaygainSettings)	\
    (This)->lpVtbl -> get_Replaygain(This,oReplaygainSettings)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlayback_Start_Proxy( 
    IVBPlayback * This,
    /* [in] */ VARIANT bPaused);


void __RPC_STUB IVBPlayback_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlayback_Stop_Proxy( 
    IVBPlayback * This);


void __RPC_STUB IVBPlayback_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlayback_Pause_Proxy( 
    IVBPlayback * This);


void __RPC_STUB IVBPlayback_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlayback_Next_Proxy( 
    IVBPlayback * This);


void __RPC_STUB IVBPlayback_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlayback_Previous_Proxy( 
    IVBPlayback * This);


void __RPC_STUB IVBPlayback_Previous_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlayback_Random_Proxy( 
    IVBPlayback * This);


void __RPC_STUB IVBPlayback_Random_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlayback_Seek_Proxy( 
    IVBPlayback * This,
    /* [in] */ DOUBLE dSecondsFromStart);


void __RPC_STUB IVBPlayback_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlayback_SeekRelative_Proxy( 
    IVBPlayback * This,
    /* [in] */ DOUBLE dSecondsFromCurrentPosition);


void __RPC_STUB IVBPlayback_SeekRelative_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlayback_FormatTitle_Proxy( 
    IVBPlayback * This,
    /* [in] */ BSTR strFormat,
    /* [retval][out] */ BSTR *strText);


void __RPC_STUB IVBPlayback_FormatTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlayback_FormatTitleEx_Proxy( 
    IVBPlayback * This,
    /* [in] */ BSTR strFormat,
    /* [in] */ enum fbDisplayLevel lDisplayLevel,
    /* [retval][out] */ BSTR *strText);


void __RPC_STUB IVBPlayback_FormatTitleEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlayback_get_IsPlaying_Proxy( 
    IVBPlayback * This,
    /* [retval][out] */ VARIANT_BOOL *bState);


void __RPC_STUB IVBPlayback_get_IsPlaying_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlayback_get_IsPaused_Proxy( 
    IVBPlayback * This,
    /* [retval][out] */ VARIANT_BOOL *bState);


void __RPC_STUB IVBPlayback_get_IsPaused_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlayback_get_Length_Proxy( 
    IVBPlayback * This,
    /* [retval][out] */ DOUBLE *dLengthInSeconds);


void __RPC_STUB IVBPlayback_get_Length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlayback_get_CanSeek_Proxy( 
    IVBPlayback * This,
    /* [retval][out] */ VARIANT_BOOL *bFlag);


void __RPC_STUB IVBPlayback_get_CanSeek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlayback_get_Position_Proxy( 
    IVBPlayback * This,
    /* [retval][out] */ DOUBLE *dSecondsFromStart);


void __RPC_STUB IVBPlayback_get_Position_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlayback_get_Settings_Proxy( 
    IVBPlayback * This,
    /* [retval][out] */ IVBPlaybackSettings **oPlaybackSettings);


void __RPC_STUB IVBPlayback_get_Settings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlayback_get_Replaygain_Proxy( 
    IVBPlayback * This,
    /* [retval][out] */ IVBReplaygainSettings **oReplaygainSettings);


void __RPC_STUB IVBPlayback_get_Replaygain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVBPlayback_INTERFACE_DEFINED__ */


#ifndef __IVBPlaybackOrders_INTERFACE_DEFINED__
#define __IVBPlaybackOrders_INTERFACE_DEFINED__

/* interface IVBPlaybackOrders */
/* [uuid][helpstring][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IVBPlaybackOrders;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a94be3f4-ea45-4d4c-af02-7c7a398c224e")
    IVBPlaybackOrders : public IDispatch
    {
    public:
        virtual /* [restricted][hidden][helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **oEnum) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ LONG lIndex,
            /* [retval][out] */ BSTR *strName) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ LONG *lCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GuidFromName( 
            /* [in] */ BSTR strName,
            /* [retval][out] */ BSTR *strGuid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE NameFromGuid( 
            /* [in] */ BSTR strGuid,
            /* [retval][out] */ BSTR *strName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVBPlaybackOrdersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVBPlaybackOrders * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVBPlaybackOrders * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVBPlaybackOrders * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVBPlaybackOrders * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVBPlaybackOrders * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVBPlaybackOrders * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVBPlaybackOrders * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [restricted][hidden][helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IVBPlaybackOrders * This,
            /* [retval][out] */ IUnknown **oEnum);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IVBPlaybackOrders * This,
            /* [in] */ LONG lIndex,
            /* [retval][out] */ BSTR *strName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IVBPlaybackOrders * This,
            /* [retval][out] */ LONG *lCount);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GuidFromName )( 
            IVBPlaybackOrders * This,
            /* [in] */ BSTR strName,
            /* [retval][out] */ BSTR *strGuid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *NameFromGuid )( 
            IVBPlaybackOrders * This,
            /* [in] */ BSTR strGuid,
            /* [retval][out] */ BSTR *strName);
        
        END_INTERFACE
    } IVBPlaybackOrdersVtbl;

    interface IVBPlaybackOrders
    {
        CONST_VTBL struct IVBPlaybackOrdersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVBPlaybackOrders_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVBPlaybackOrders_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVBPlaybackOrders_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVBPlaybackOrders_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVBPlaybackOrders_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVBPlaybackOrders_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVBPlaybackOrders_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVBPlaybackOrders_get__NewEnum(This,oEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,oEnum)

#define IVBPlaybackOrders_get_Item(This,lIndex,strName)	\
    (This)->lpVtbl -> get_Item(This,lIndex,strName)

#define IVBPlaybackOrders_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define IVBPlaybackOrders_GuidFromName(This,strName,strGuid)	\
    (This)->lpVtbl -> GuidFromName(This,strName,strGuid)

#define IVBPlaybackOrders_NameFromGuid(This,strGuid,strName)	\
    (This)->lpVtbl -> NameFromGuid(This,strGuid,strName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][hidden][helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaybackOrders_get__NewEnum_Proxy( 
    IVBPlaybackOrders * This,
    /* [retval][out] */ IUnknown **oEnum);


void __RPC_STUB IVBPlaybackOrders_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaybackOrders_get_Item_Proxy( 
    IVBPlaybackOrders * This,
    /* [in] */ LONG lIndex,
    /* [retval][out] */ BSTR *strName);


void __RPC_STUB IVBPlaybackOrders_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaybackOrders_get_Count_Proxy( 
    IVBPlaybackOrders * This,
    /* [retval][out] */ LONG *lCount);


void __RPC_STUB IVBPlaybackOrders_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaybackOrders_GuidFromName_Proxy( 
    IVBPlaybackOrders * This,
    /* [in] */ BSTR strName,
    /* [retval][out] */ BSTR *strGuid);


void __RPC_STUB IVBPlaybackOrders_GuidFromName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaybackOrders_NameFromGuid_Proxy( 
    IVBPlaybackOrders * This,
    /* [in] */ BSTR strGuid,
    /* [retval][out] */ BSTR *strName);


void __RPC_STUB IVBPlaybackOrders_NameFromGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVBPlaybackOrders_INTERFACE_DEFINED__ */


#ifndef __IVBPlaybackSettings_INTERFACE_DEFINED__
#define __IVBPlaybackSettings_INTERFACE_DEFINED__

/* interface IVBPlaybackSettings */
/* [uuid][helpstring][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IVBPlaybackSettings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d25cec8d-2d0d-42e6-9a9f-0eee52d6f0c5")
    IVBPlaybackSettings : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Volume( 
            /* [retval][out] */ DOUBLE *dGainInDecibel) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Volume( 
            /* [in] */ DOUBLE dGainInDecibel) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlaybackFollowsCursor( 
            /* [retval][out] */ VARIANT_BOOL *bState) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PlaybackFollowsCursor( 
            /* [in] */ VARIANT_BOOL bState) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CursorFollowsPlayback( 
            /* [retval][out] */ VARIANT_BOOL *bState) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CursorFollowsPlayback( 
            /* [in] */ VARIANT_BOOL bState) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StopAfterCurrent( 
            /* [retval][out] */ VARIANT_BOOL *bState) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StopAfterCurrent( 
            /* [in] */ VARIANT_BOOL bState) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ActivePlaybackOrder( 
            /* [retval][out] */ BSTR *bState) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ActivePlaybackOrder( 
            /* [in] */ BSTR strName) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlaybackOrders( 
            /* [retval][out] */ IVBPlaybackOrders **oOrders) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVBPlaybackSettingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVBPlaybackSettings * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVBPlaybackSettings * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVBPlaybackSettings * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVBPlaybackSettings * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVBPlaybackSettings * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVBPlaybackSettings * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVBPlaybackSettings * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Volume )( 
            IVBPlaybackSettings * This,
            /* [retval][out] */ DOUBLE *dGainInDecibel);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Volume )( 
            IVBPlaybackSettings * This,
            /* [in] */ DOUBLE dGainInDecibel);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PlaybackFollowsCursor )( 
            IVBPlaybackSettings * This,
            /* [retval][out] */ VARIANT_BOOL *bState);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PlaybackFollowsCursor )( 
            IVBPlaybackSettings * This,
            /* [in] */ VARIANT_BOOL bState);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CursorFollowsPlayback )( 
            IVBPlaybackSettings * This,
            /* [retval][out] */ VARIANT_BOOL *bState);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CursorFollowsPlayback )( 
            IVBPlaybackSettings * This,
            /* [in] */ VARIANT_BOOL bState);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StopAfterCurrent )( 
            IVBPlaybackSettings * This,
            /* [retval][out] */ VARIANT_BOOL *bState);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StopAfterCurrent )( 
            IVBPlaybackSettings * This,
            /* [in] */ VARIANT_BOOL bState);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ActivePlaybackOrder )( 
            IVBPlaybackSettings * This,
            /* [retval][out] */ BSTR *bState);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ActivePlaybackOrder )( 
            IVBPlaybackSettings * This,
            /* [in] */ BSTR strName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PlaybackOrders )( 
            IVBPlaybackSettings * This,
            /* [retval][out] */ IVBPlaybackOrders **oOrders);
        
        END_INTERFACE
    } IVBPlaybackSettingsVtbl;

    interface IVBPlaybackSettings
    {
        CONST_VTBL struct IVBPlaybackSettingsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVBPlaybackSettings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVBPlaybackSettings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVBPlaybackSettings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVBPlaybackSettings_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVBPlaybackSettings_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVBPlaybackSettings_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVBPlaybackSettings_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVBPlaybackSettings_get_Volume(This,dGainInDecibel)	\
    (This)->lpVtbl -> get_Volume(This,dGainInDecibel)

#define IVBPlaybackSettings_put_Volume(This,dGainInDecibel)	\
    (This)->lpVtbl -> put_Volume(This,dGainInDecibel)

#define IVBPlaybackSettings_get_PlaybackFollowsCursor(This,bState)	\
    (This)->lpVtbl -> get_PlaybackFollowsCursor(This,bState)

#define IVBPlaybackSettings_put_PlaybackFollowsCursor(This,bState)	\
    (This)->lpVtbl -> put_PlaybackFollowsCursor(This,bState)

#define IVBPlaybackSettings_get_CursorFollowsPlayback(This,bState)	\
    (This)->lpVtbl -> get_CursorFollowsPlayback(This,bState)

#define IVBPlaybackSettings_put_CursorFollowsPlayback(This,bState)	\
    (This)->lpVtbl -> put_CursorFollowsPlayback(This,bState)

#define IVBPlaybackSettings_get_StopAfterCurrent(This,bState)	\
    (This)->lpVtbl -> get_StopAfterCurrent(This,bState)

#define IVBPlaybackSettings_put_StopAfterCurrent(This,bState)	\
    (This)->lpVtbl -> put_StopAfterCurrent(This,bState)

#define IVBPlaybackSettings_get_ActivePlaybackOrder(This,bState)	\
    (This)->lpVtbl -> get_ActivePlaybackOrder(This,bState)

#define IVBPlaybackSettings_put_ActivePlaybackOrder(This,strName)	\
    (This)->lpVtbl -> put_ActivePlaybackOrder(This,strName)

#define IVBPlaybackSettings_get_PlaybackOrders(This,oOrders)	\
    (This)->lpVtbl -> get_PlaybackOrders(This,oOrders)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_get_Volume_Proxy( 
    IVBPlaybackSettings * This,
    /* [retval][out] */ DOUBLE *dGainInDecibel);


void __RPC_STUB IVBPlaybackSettings_get_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_put_Volume_Proxy( 
    IVBPlaybackSettings * This,
    /* [in] */ DOUBLE dGainInDecibel);


void __RPC_STUB IVBPlaybackSettings_put_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_get_PlaybackFollowsCursor_Proxy( 
    IVBPlaybackSettings * This,
    /* [retval][out] */ VARIANT_BOOL *bState);


void __RPC_STUB IVBPlaybackSettings_get_PlaybackFollowsCursor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_put_PlaybackFollowsCursor_Proxy( 
    IVBPlaybackSettings * This,
    /* [in] */ VARIANT_BOOL bState);


void __RPC_STUB IVBPlaybackSettings_put_PlaybackFollowsCursor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_get_CursorFollowsPlayback_Proxy( 
    IVBPlaybackSettings * This,
    /* [retval][out] */ VARIANT_BOOL *bState);


void __RPC_STUB IVBPlaybackSettings_get_CursorFollowsPlayback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_put_CursorFollowsPlayback_Proxy( 
    IVBPlaybackSettings * This,
    /* [in] */ VARIANT_BOOL bState);


void __RPC_STUB IVBPlaybackSettings_put_CursorFollowsPlayback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_get_StopAfterCurrent_Proxy( 
    IVBPlaybackSettings * This,
    /* [retval][out] */ VARIANT_BOOL *bState);


void __RPC_STUB IVBPlaybackSettings_get_StopAfterCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_put_StopAfterCurrent_Proxy( 
    IVBPlaybackSettings * This,
    /* [in] */ VARIANT_BOOL bState);


void __RPC_STUB IVBPlaybackSettings_put_StopAfterCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_get_ActivePlaybackOrder_Proxy( 
    IVBPlaybackSettings * This,
    /* [retval][out] */ BSTR *bState);


void __RPC_STUB IVBPlaybackSettings_get_ActivePlaybackOrder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_put_ActivePlaybackOrder_Proxy( 
    IVBPlaybackSettings * This,
    /* [in] */ BSTR strName);


void __RPC_STUB IVBPlaybackSettings_put_ActivePlaybackOrder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaybackSettings_get_PlaybackOrders_Proxy( 
    IVBPlaybackSettings * This,
    /* [retval][out] */ IVBPlaybackOrders **oOrders);


void __RPC_STUB IVBPlaybackSettings_get_PlaybackOrders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVBPlaybackSettings_INTERFACE_DEFINED__ */


#ifndef __IVBPlaylist_INTERFACE_DEFINED__
#define __IVBPlaylist_INTERFACE_DEFINED__

/* interface IVBPlaylist */
/* [uuid][helpstring][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IVBPlaylist;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e2ab4023-2e5b-4d97-a8dd-823175cbf573")
    IVBPlaylist : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *strName) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR strName) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Index( 
            /* [retval][out] */ LONG *lIndex) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetTracks( 
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSortedTracks( 
            /* [in] */ BSTR strSortFormat,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DoDefaultAction( 
            /* [in] */ LONG lItemIndex,
            /* [retval][out] */ VARIANT_BOOL *bSuccess) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVBPlaylistVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVBPlaylist * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVBPlaylist * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVBPlaylist * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVBPlaylist * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVBPlaylist * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVBPlaylist * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVBPlaylist * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVBPlaylist * This,
            /* [retval][out] */ BSTR *strName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVBPlaylist * This,
            /* [in] */ BSTR strName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Index )( 
            IVBPlaylist * This,
            /* [retval][out] */ LONG *lIndex);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetTracks )( 
            IVBPlaylist * This,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSortedTracks )( 
            IVBPlaylist * This,
            /* [in] */ BSTR strSortFormat,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DoDefaultAction )( 
            IVBPlaylist * This,
            /* [in] */ LONG lItemIndex,
            /* [retval][out] */ VARIANT_BOOL *bSuccess);
        
        END_INTERFACE
    } IVBPlaylistVtbl;

    interface IVBPlaylist
    {
        CONST_VTBL struct IVBPlaylistVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVBPlaylist_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVBPlaylist_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVBPlaylist_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVBPlaylist_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVBPlaylist_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVBPlaylist_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVBPlaylist_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVBPlaylist_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#define IVBPlaylist_put_Name(This,strName)	\
    (This)->lpVtbl -> put_Name(This,strName)

#define IVBPlaylist_get_Index(This,lIndex)	\
    (This)->lpVtbl -> get_Index(This,lIndex)

#define IVBPlaylist_GetTracks(This,strQuery,oTracks)	\
    (This)->lpVtbl -> GetTracks(This,strQuery,oTracks)

#define IVBPlaylist_GetSortedTracks(This,strSortFormat,strQuery,oTracks)	\
    (This)->lpVtbl -> GetSortedTracks(This,strSortFormat,strQuery,oTracks)

#define IVBPlaylist_DoDefaultAction(This,lItemIndex,bSuccess)	\
    (This)->lpVtbl -> DoDefaultAction(This,lItemIndex,bSuccess)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaylist_get_Name_Proxy( 
    IVBPlaylist * This,
    /* [retval][out] */ BSTR *strName);


void __RPC_STUB IVBPlaylist_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBPlaylist_put_Name_Proxy( 
    IVBPlaylist * This,
    /* [in] */ BSTR strName);


void __RPC_STUB IVBPlaylist_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaylist_get_Index_Proxy( 
    IVBPlaylist * This,
    /* [retval][out] */ LONG *lIndex);


void __RPC_STUB IVBPlaylist_get_Index_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaylist_GetTracks_Proxy( 
    IVBPlaylist * This,
    /* [optional][in] */ VARIANT strQuery,
    /* [retval][out] */ IVBTracks **oTracks);


void __RPC_STUB IVBPlaylist_GetTracks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaylist_GetSortedTracks_Proxy( 
    IVBPlaylist * This,
    /* [in] */ BSTR strSortFormat,
    /* [optional][in] */ VARIANT strQuery,
    /* [retval][out] */ IVBTracks **oTracks);


void __RPC_STUB IVBPlaylist_GetSortedTracks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaylist_DoDefaultAction_Proxy( 
    IVBPlaylist * This,
    /* [in] */ LONG lItemIndex,
    /* [retval][out] */ VARIANT_BOOL *bSuccess);


void __RPC_STUB IVBPlaylist_DoDefaultAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVBPlaylist_INTERFACE_DEFINED__ */


#ifndef __IVBPlaylists_INTERFACE_DEFINED__
#define __IVBPlaylists_INTERFACE_DEFINED__

/* interface IVBPlaylists */
/* [uuid][helpstring][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IVBPlaylists;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e4c39463-646a-49d5-9ada-99eaec162ff6")
    IVBPlaylists : public IDispatch
    {
    public:
        virtual /* [restricted][hidden][helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **oEnum) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ LONG lIndex,
            /* [retval][out] */ IVBPlaylist **oPlaylist) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ LONG *lCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ BSTR strName,
            /* [defaultvalue][in] */ VARIANT_BOOL bActivate,
            /* [retval][out] */ IVBPlaylist **oPlaylist) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ IVBPlaylist *oPlaylist,
            /* [retval][out] */ VARIANT_BOOL *bSuccess) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Move( 
            /* [in] */ IVBPlaylist *oPlaylist,
            /* [in] */ enum fbPosition lPosition,
            /* [optional][in] */ VARIANT oReferencePlaylist) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Load( 
            /* [in] */ BSTR strName,
            /* [in] */ BSTR strFilename,
            /* [defaultvalue][in] */ VARIANT_BOOL bActivate,
            /* [retval][out] */ IVBPlaylist **oPlaylist) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Save( 
            /* [in] */ IVBPlaylist *oPlaylist,
            /* [in] */ BSTR strFilename) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ActivePlaylist( 
            /* [retval][out] */ IVBPlaylist **oPlaylist) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ActivePlaylist( 
            /* [in] */ IVBPlaylist *oPlaylist) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetTracks( 
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSortedTracks( 
            /* [in] */ BSTR strSortFormat,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVBPlaylistsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVBPlaylists * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVBPlaylists * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVBPlaylists * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVBPlaylists * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVBPlaylists * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVBPlaylists * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVBPlaylists * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [restricted][hidden][helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IVBPlaylists * This,
            /* [retval][out] */ IUnknown **oEnum);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IVBPlaylists * This,
            /* [in] */ LONG lIndex,
            /* [retval][out] */ IVBPlaylist **oPlaylist);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IVBPlaylists * This,
            /* [retval][out] */ LONG *lCount);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Add )( 
            IVBPlaylists * This,
            /* [in] */ BSTR strName,
            /* [defaultvalue][in] */ VARIANT_BOOL bActivate,
            /* [retval][out] */ IVBPlaylist **oPlaylist);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVBPlaylists * This,
            /* [in] */ IVBPlaylist *oPlaylist,
            /* [retval][out] */ VARIANT_BOOL *bSuccess);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Move )( 
            IVBPlaylists * This,
            /* [in] */ IVBPlaylist *oPlaylist,
            /* [in] */ enum fbPosition lPosition,
            /* [optional][in] */ VARIANT oReferencePlaylist);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Load )( 
            IVBPlaylists * This,
            /* [in] */ BSTR strName,
            /* [in] */ BSTR strFilename,
            /* [defaultvalue][in] */ VARIANT_BOOL bActivate,
            /* [retval][out] */ IVBPlaylist **oPlaylist);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Save )( 
            IVBPlaylists * This,
            /* [in] */ IVBPlaylist *oPlaylist,
            /* [in] */ BSTR strFilename);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ActivePlaylist )( 
            IVBPlaylists * This,
            /* [retval][out] */ IVBPlaylist **oPlaylist);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ActivePlaylist )( 
            IVBPlaylists * This,
            /* [in] */ IVBPlaylist *oPlaylist);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetTracks )( 
            IVBPlaylists * This,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSortedTracks )( 
            IVBPlaylists * This,
            /* [in] */ BSTR strSortFormat,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks);
        
        END_INTERFACE
    } IVBPlaylistsVtbl;

    interface IVBPlaylists
    {
        CONST_VTBL struct IVBPlaylistsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVBPlaylists_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVBPlaylists_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVBPlaylists_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVBPlaylists_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVBPlaylists_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVBPlaylists_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVBPlaylists_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVBPlaylists_get__NewEnum(This,oEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,oEnum)

#define IVBPlaylists_get_Item(This,lIndex,oPlaylist)	\
    (This)->lpVtbl -> get_Item(This,lIndex,oPlaylist)

#define IVBPlaylists_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define IVBPlaylists_Add(This,strName,bActivate,oPlaylist)	\
    (This)->lpVtbl -> Add(This,strName,bActivate,oPlaylist)

#define IVBPlaylists_Remove(This,oPlaylist,bSuccess)	\
    (This)->lpVtbl -> Remove(This,oPlaylist,bSuccess)

#define IVBPlaylists_Move(This,oPlaylist,lPosition,oReferencePlaylist)	\
    (This)->lpVtbl -> Move(This,oPlaylist,lPosition,oReferencePlaylist)

#define IVBPlaylists_Load(This,strName,strFilename,bActivate,oPlaylist)	\
    (This)->lpVtbl -> Load(This,strName,strFilename,bActivate,oPlaylist)

#define IVBPlaylists_Save(This,oPlaylist,strFilename)	\
    (This)->lpVtbl -> Save(This,oPlaylist,strFilename)

#define IVBPlaylists_get_ActivePlaylist(This,oPlaylist)	\
    (This)->lpVtbl -> get_ActivePlaylist(This,oPlaylist)

#define IVBPlaylists_put_ActivePlaylist(This,oPlaylist)	\
    (This)->lpVtbl -> put_ActivePlaylist(This,oPlaylist)

#define IVBPlaylists_GetTracks(This,strQuery,oTracks)	\
    (This)->lpVtbl -> GetTracks(This,strQuery,oTracks)

#define IVBPlaylists_GetSortedTracks(This,strSortFormat,strQuery,oTracks)	\
    (This)->lpVtbl -> GetSortedTracks(This,strSortFormat,strQuery,oTracks)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][hidden][helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_get__NewEnum_Proxy( 
    IVBPlaylists * This,
    /* [retval][out] */ IUnknown **oEnum);


void __RPC_STUB IVBPlaylists_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_get_Item_Proxy( 
    IVBPlaylists * This,
    /* [in] */ LONG lIndex,
    /* [retval][out] */ IVBPlaylist **oPlaylist);


void __RPC_STUB IVBPlaylists_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_get_Count_Proxy( 
    IVBPlaylists * This,
    /* [retval][out] */ LONG *lCount);


void __RPC_STUB IVBPlaylists_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_Add_Proxy( 
    IVBPlaylists * This,
    /* [in] */ BSTR strName,
    /* [defaultvalue][in] */ VARIANT_BOOL bActivate,
    /* [retval][out] */ IVBPlaylist **oPlaylist);


void __RPC_STUB IVBPlaylists_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_Remove_Proxy( 
    IVBPlaylists * This,
    /* [in] */ IVBPlaylist *oPlaylist,
    /* [retval][out] */ VARIANT_BOOL *bSuccess);


void __RPC_STUB IVBPlaylists_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_Move_Proxy( 
    IVBPlaylists * This,
    /* [in] */ IVBPlaylist *oPlaylist,
    /* [in] */ enum fbPosition lPosition,
    /* [optional][in] */ VARIANT oReferencePlaylist);


void __RPC_STUB IVBPlaylists_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_Load_Proxy( 
    IVBPlaylists * This,
    /* [in] */ BSTR strName,
    /* [in] */ BSTR strFilename,
    /* [defaultvalue][in] */ VARIANT_BOOL bActivate,
    /* [retval][out] */ IVBPlaylist **oPlaylist);


void __RPC_STUB IVBPlaylists_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_Save_Proxy( 
    IVBPlaylists * This,
    /* [in] */ IVBPlaylist *oPlaylist,
    /* [in] */ BSTR strFilename);


void __RPC_STUB IVBPlaylists_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_get_ActivePlaylist_Proxy( 
    IVBPlaylists * This,
    /* [retval][out] */ IVBPlaylist **oPlaylist);


void __RPC_STUB IVBPlaylists_get_ActivePlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_put_ActivePlaylist_Proxy( 
    IVBPlaylists * This,
    /* [in] */ IVBPlaylist *oPlaylist);


void __RPC_STUB IVBPlaylists_put_ActivePlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_GetTracks_Proxy( 
    IVBPlaylists * This,
    /* [optional][in] */ VARIANT strQuery,
    /* [retval][out] */ IVBTracks **oTracks);


void __RPC_STUB IVBPlaylists_GetTracks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBPlaylists_GetSortedTracks_Proxy( 
    IVBPlaylists * This,
    /* [in] */ BSTR strSortFormat,
    /* [optional][in] */ VARIANT strQuery,
    /* [retval][out] */ IVBTracks **oTracks);


void __RPC_STUB IVBPlaylists_GetSortedTracks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVBPlaylists_INTERFACE_DEFINED__ */


#ifndef __IVBReplaygainSettings_INTERFACE_DEFINED__
#define __IVBReplaygainSettings_INTERFACE_DEFINED__

/* interface IVBReplaygainSettings */
/* [uuid][helpstring][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IVBReplaygainSettings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6f873be6-ae3b-4c2f-828c-d5253a99805c")
    IVBReplaygainSettings : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Mode( 
            /* [retval][out] */ enum fbReplaygainMode *lReplaygainMode) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Mode( 
            /* [in] */ enum fbReplaygainMode lReplaygainMode) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ApplyGain( 
            /* [retval][out] */ VARIANT_BOOL *bFlag) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ApplyGain( 
            /* [in] */ VARIANT_BOOL bFlag) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PreventClipping( 
            /* [retval][out] */ VARIANT_BOOL *bFlag) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PreventClipping( 
            /* [in] */ VARIANT_BOOL bFlag) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PreampWithRGInfo( 
            /* [retval][out] */ DOUBLE *dGainInDecibel) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PreampWithRGInfo( 
            /* [in] */ DOUBLE dGainInDecibel) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PreampWithoutRGInfo( 
            /* [retval][out] */ DOUBLE *dGainInDecibel) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PreampWithoutRGInfo( 
            /* [in] */ DOUBLE dGainInDecibel) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVBReplaygainSettingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVBReplaygainSettings * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVBReplaygainSettings * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVBReplaygainSettings * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVBReplaygainSettings * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVBReplaygainSettings * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVBReplaygainSettings * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVBReplaygainSettings * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Mode )( 
            IVBReplaygainSettings * This,
            /* [retval][out] */ enum fbReplaygainMode *lReplaygainMode);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Mode )( 
            IVBReplaygainSettings * This,
            /* [in] */ enum fbReplaygainMode lReplaygainMode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ApplyGain )( 
            IVBReplaygainSettings * This,
            /* [retval][out] */ VARIANT_BOOL *bFlag);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ApplyGain )( 
            IVBReplaygainSettings * This,
            /* [in] */ VARIANT_BOOL bFlag);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PreventClipping )( 
            IVBReplaygainSettings * This,
            /* [retval][out] */ VARIANT_BOOL *bFlag);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PreventClipping )( 
            IVBReplaygainSettings * This,
            /* [in] */ VARIANT_BOOL bFlag);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PreampWithRGInfo )( 
            IVBReplaygainSettings * This,
            /* [retval][out] */ DOUBLE *dGainInDecibel);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PreampWithRGInfo )( 
            IVBReplaygainSettings * This,
            /* [in] */ DOUBLE dGainInDecibel);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PreampWithoutRGInfo )( 
            IVBReplaygainSettings * This,
            /* [retval][out] */ DOUBLE *dGainInDecibel);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PreampWithoutRGInfo )( 
            IVBReplaygainSettings * This,
            /* [in] */ DOUBLE dGainInDecibel);
        
        END_INTERFACE
    } IVBReplaygainSettingsVtbl;

    interface IVBReplaygainSettings
    {
        CONST_VTBL struct IVBReplaygainSettingsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVBReplaygainSettings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVBReplaygainSettings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVBReplaygainSettings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVBReplaygainSettings_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVBReplaygainSettings_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVBReplaygainSettings_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVBReplaygainSettings_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVBReplaygainSettings_get_Mode(This,lReplaygainMode)	\
    (This)->lpVtbl -> get_Mode(This,lReplaygainMode)

#define IVBReplaygainSettings_put_Mode(This,lReplaygainMode)	\
    (This)->lpVtbl -> put_Mode(This,lReplaygainMode)

#define IVBReplaygainSettings_get_ApplyGain(This,bFlag)	\
    (This)->lpVtbl -> get_ApplyGain(This,bFlag)

#define IVBReplaygainSettings_put_ApplyGain(This,bFlag)	\
    (This)->lpVtbl -> put_ApplyGain(This,bFlag)

#define IVBReplaygainSettings_get_PreventClipping(This,bFlag)	\
    (This)->lpVtbl -> get_PreventClipping(This,bFlag)

#define IVBReplaygainSettings_put_PreventClipping(This,bFlag)	\
    (This)->lpVtbl -> put_PreventClipping(This,bFlag)

#define IVBReplaygainSettings_get_PreampWithRGInfo(This,dGainInDecibel)	\
    (This)->lpVtbl -> get_PreampWithRGInfo(This,dGainInDecibel)

#define IVBReplaygainSettings_put_PreampWithRGInfo(This,dGainInDecibel)	\
    (This)->lpVtbl -> put_PreampWithRGInfo(This,dGainInDecibel)

#define IVBReplaygainSettings_get_PreampWithoutRGInfo(This,dGainInDecibel)	\
    (This)->lpVtbl -> get_PreampWithoutRGInfo(This,dGainInDecibel)

#define IVBReplaygainSettings_put_PreampWithoutRGInfo(This,dGainInDecibel)	\
    (This)->lpVtbl -> put_PreampWithoutRGInfo(This,dGainInDecibel)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBReplaygainSettings_get_Mode_Proxy( 
    IVBReplaygainSettings * This,
    /* [retval][out] */ enum fbReplaygainMode *lReplaygainMode);


void __RPC_STUB IVBReplaygainSettings_get_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBReplaygainSettings_put_Mode_Proxy( 
    IVBReplaygainSettings * This,
    /* [in] */ enum fbReplaygainMode lReplaygainMode);


void __RPC_STUB IVBReplaygainSettings_put_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBReplaygainSettings_get_ApplyGain_Proxy( 
    IVBReplaygainSettings * This,
    /* [retval][out] */ VARIANT_BOOL *bFlag);


void __RPC_STUB IVBReplaygainSettings_get_ApplyGain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBReplaygainSettings_put_ApplyGain_Proxy( 
    IVBReplaygainSettings * This,
    /* [in] */ VARIANT_BOOL bFlag);


void __RPC_STUB IVBReplaygainSettings_put_ApplyGain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBReplaygainSettings_get_PreventClipping_Proxy( 
    IVBReplaygainSettings * This,
    /* [retval][out] */ VARIANT_BOOL *bFlag);


void __RPC_STUB IVBReplaygainSettings_get_PreventClipping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBReplaygainSettings_put_PreventClipping_Proxy( 
    IVBReplaygainSettings * This,
    /* [in] */ VARIANT_BOOL bFlag);


void __RPC_STUB IVBReplaygainSettings_put_PreventClipping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBReplaygainSettings_get_PreampWithRGInfo_Proxy( 
    IVBReplaygainSettings * This,
    /* [retval][out] */ DOUBLE *dGainInDecibel);


void __RPC_STUB IVBReplaygainSettings_get_PreampWithRGInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBReplaygainSettings_put_PreampWithRGInfo_Proxy( 
    IVBReplaygainSettings * This,
    /* [in] */ DOUBLE dGainInDecibel);


void __RPC_STUB IVBReplaygainSettings_put_PreampWithRGInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBReplaygainSettings_get_PreampWithoutRGInfo_Proxy( 
    IVBReplaygainSettings * This,
    /* [retval][out] */ DOUBLE *dGainInDecibel);


void __RPC_STUB IVBReplaygainSettings_get_PreampWithoutRGInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IVBReplaygainSettings_put_PreampWithoutRGInfo_Proxy( 
    IVBReplaygainSettings * This,
    /* [in] */ DOUBLE dGainInDecibel);


void __RPC_STUB IVBReplaygainSettings_put_PreampWithoutRGInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVBReplaygainSettings_INTERFACE_DEFINED__ */


#ifndef __IVBTrack_INTERFACE_DEFINED__
#define __IVBTrack_INTERFACE_DEFINED__

/* interface IVBTrack */
/* [uuid][helpstring][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IVBTrack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6d590032-ba42-41ca-866c-321e2b511a62")
    IVBTrack : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Path( 
            /* [retval][out] */ BSTR *strPath) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Index( 
            /* [retval][out] */ LONG *lIndex) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FormatTitle( 
            /* [in] */ BSTR strFormat,
            /* [retval][out] */ BSTR *strText) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVBTrackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVBTrack * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVBTrack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVBTrack * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVBTrack * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVBTrack * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVBTrack * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVBTrack * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            IVBTrack * This,
            /* [retval][out] */ BSTR *strPath);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Index )( 
            IVBTrack * This,
            /* [retval][out] */ LONG *lIndex);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FormatTitle )( 
            IVBTrack * This,
            /* [in] */ BSTR strFormat,
            /* [retval][out] */ BSTR *strText);
        
        END_INTERFACE
    } IVBTrackVtbl;

    interface IVBTrack
    {
        CONST_VTBL struct IVBTrackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVBTrack_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVBTrack_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVBTrack_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVBTrack_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVBTrack_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVBTrack_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVBTrack_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVBTrack_get_Path(This,strPath)	\
    (This)->lpVtbl -> get_Path(This,strPath)

#define IVBTrack_get_Index(This,lIndex)	\
    (This)->lpVtbl -> get_Index(This,lIndex)

#define IVBTrack_FormatTitle(This,strFormat,strText)	\
    (This)->lpVtbl -> FormatTitle(This,strFormat,strText)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBTrack_get_Path_Proxy( 
    IVBTrack * This,
    /* [retval][out] */ BSTR *strPath);


void __RPC_STUB IVBTrack_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBTrack_get_Index_Proxy( 
    IVBTrack * This,
    /* [retval][out] */ LONG *lIndex);


void __RPC_STUB IVBTrack_get_Index_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBTrack_FormatTitle_Proxy( 
    IVBTrack * This,
    /* [in] */ BSTR strFormat,
    /* [retval][out] */ BSTR *strText);


void __RPC_STUB IVBTrack_FormatTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVBTrack_INTERFACE_DEFINED__ */


#ifndef __IVBTracks_INTERFACE_DEFINED__
#define __IVBTracks_INTERFACE_DEFINED__

/* interface IVBTracks */
/* [uuid][helpstring][nonextensible][dual][oleautomation][object] */ 


EXTERN_C const IID IID_IVBTracks;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0d5d4f8f-4132-48cb-9d6c-126843f78909")
    IVBTracks : public IDispatch
    {
    public:
        virtual /* [restricted][hidden][helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **oEnum) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ LONG lIndex,
            /* [retval][out] */ IVBTrack **oTrack) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ LONG *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetTracks( 
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSortedTracks( 
            /* [in] */ BSTR strSortFormat,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVBTracksVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVBTracks * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVBTracks * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVBTracks * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVBTracks * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVBTracks * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVBTracks * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVBTracks * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [restricted][hidden][helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IVBTracks * This,
            /* [retval][out] */ IUnknown **oEnum);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IVBTracks * This,
            /* [in] */ LONG lIndex,
            /* [retval][out] */ IVBTrack **oTrack);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IVBTracks * This,
            /* [retval][out] */ LONG *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetTracks )( 
            IVBTracks * This,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSortedTracks )( 
            IVBTracks * This,
            /* [in] */ BSTR strSortFormat,
            /* [optional][in] */ VARIANT strQuery,
            /* [retval][out] */ IVBTracks **oTracks);
        
        END_INTERFACE
    } IVBTracksVtbl;

    interface IVBTracks
    {
        CONST_VTBL struct IVBTracksVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVBTracks_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVBTracks_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVBTracks_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVBTracks_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVBTracks_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVBTracks_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVBTracks_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVBTracks_get__NewEnum(This,oEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,oEnum)

#define IVBTracks_get_Item(This,lIndex,oTrack)	\
    (This)->lpVtbl -> get_Item(This,lIndex,oTrack)

#define IVBTracks_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IVBTracks_GetTracks(This,strQuery,oTracks)	\
    (This)->lpVtbl -> GetTracks(This,strQuery,oTracks)

#define IVBTracks_GetSortedTracks(This,strSortFormat,strQuery,oTracks)	\
    (This)->lpVtbl -> GetSortedTracks(This,strSortFormat,strQuery,oTracks)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][hidden][helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBTracks_get__NewEnum_Proxy( 
    IVBTracks * This,
    /* [retval][out] */ IUnknown **oEnum);


void __RPC_STUB IVBTracks_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBTracks_get_Item_Proxy( 
    IVBTracks * This,
    /* [in] */ LONG lIndex,
    /* [retval][out] */ IVBTrack **oTrack);


void __RPC_STUB IVBTracks_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IVBTracks_get_Count_Proxy( 
    IVBTracks * This,
    /* [retval][out] */ LONG *pVal);


void __RPC_STUB IVBTracks_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBTracks_GetTracks_Proxy( 
    IVBTracks * This,
    /* [optional][in] */ VARIANT strQuery,
    /* [retval][out] */ IVBTracks **oTracks);


void __RPC_STUB IVBTracks_GetTracks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IVBTracks_GetSortedTracks_Proxy( 
    IVBTracks * This,
    /* [in] */ BSTR strSortFormat,
    /* [optional][in] */ VARIANT strQuery,
    /* [retval][out] */ IVBTracks **oTracks);


void __RPC_STUB IVBTracks_GetSortedTracks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVBTracks_INTERFACE_DEFINED__ */


#ifndef ___IVBPlaybackEvents_DISPINTERFACE_DEFINED__
#define ___IVBPlaybackEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IVBPlaybackEvents */
/* [uuid][helpstring][hidden] */ 


EXTERN_C const IID DIID__IVBPlaybackEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("7d6b3a92-4421-49d9-a111-4d16ef7144ca")
    _IVBPlaybackEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IVBPlaybackEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IVBPlaybackEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IVBPlaybackEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IVBPlaybackEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IVBPlaybackEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IVBPlaybackEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IVBPlaybackEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IVBPlaybackEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IVBPlaybackEventsVtbl;

    interface _IVBPlaybackEvents
    {
        CONST_VTBL struct _IVBPlaybackEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IVBPlaybackEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IVBPlaybackEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IVBPlaybackEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IVBPlaybackEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IVBPlaybackEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IVBPlaybackEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IVBPlaybackEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IVBPlaybackEvents_DISPINTERFACE_DEFINED__ */


#ifndef ___IVBPlaybackSettingsEvents_DISPINTERFACE_DEFINED__
#define ___IVBPlaybackSettingsEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IVBPlaybackSettingsEvents */
/* [uuid][helpstring][hidden] */ 


EXTERN_C const IID DIID__IVBPlaybackSettingsEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("831ae6d1-6ebf-488d-9274-a5523cbb29b7")
    _IVBPlaybackSettingsEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IVBPlaybackSettingsEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IVBPlaybackSettingsEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IVBPlaybackSettingsEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IVBPlaybackSettingsEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IVBPlaybackSettingsEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IVBPlaybackSettingsEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IVBPlaybackSettingsEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IVBPlaybackSettingsEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IVBPlaybackSettingsEventsVtbl;

    interface _IVBPlaybackSettingsEvents
    {
        CONST_VTBL struct _IVBPlaybackSettingsEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IVBPlaybackSettingsEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IVBPlaybackSettingsEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IVBPlaybackSettingsEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IVBPlaybackSettingsEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IVBPlaybackSettingsEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IVBPlaybackSettingsEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IVBPlaybackSettingsEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IVBPlaybackSettingsEvents_DISPINTERFACE_DEFINED__ */


#ifndef ___IVBPlaylistEvents_DISPINTERFACE_DEFINED__
#define ___IVBPlaylistEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IVBPlaylistEvents */
/* [uuid][helpstring][hidden] */ 


EXTERN_C const IID DIID__IVBPlaylistEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("b38e3232-b271-4072-ba59-47614658fd60")
    _IVBPlaylistEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IVBPlaylistEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IVBPlaylistEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IVBPlaylistEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IVBPlaylistEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IVBPlaylistEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IVBPlaylistEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IVBPlaylistEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IVBPlaylistEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IVBPlaylistEventsVtbl;

    interface _IVBPlaylistEvents
    {
        CONST_VTBL struct _IVBPlaylistEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IVBPlaylistEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IVBPlaylistEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IVBPlaylistEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IVBPlaylistEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IVBPlaylistEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IVBPlaylistEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IVBPlaylistEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IVBPlaylistEvents_DISPINTERFACE_DEFINED__ */


#ifndef ___IVBPlaylistsEvents_DISPINTERFACE_DEFINED__
#define ___IVBPlaylistsEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IVBPlaylistsEvents */
/* [uuid][helpstring][hidden] */ 


EXTERN_C const IID DIID__IVBPlaylistsEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("c9f6edd0-2bfd-46db-a580-4941eae782d0")
    _IVBPlaylistsEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IVBPlaylistsEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IVBPlaylistsEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IVBPlaylistsEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IVBPlaylistsEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IVBPlaylistsEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IVBPlaylistsEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IVBPlaylistsEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IVBPlaylistsEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IVBPlaylistsEventsVtbl;

    interface _IVBPlaylistsEvents
    {
        CONST_VTBL struct _IVBPlaylistsEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IVBPlaylistsEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IVBPlaylistsEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IVBPlaylistsEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IVBPlaylistsEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IVBPlaylistsEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IVBPlaylistsEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IVBPlaylistsEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IVBPlaylistsEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_Application07;

#ifdef __cplusplus

class DECLSPEC_UUID("1bffc1e4-21a6-45af-8831-5ee045281633")
Application07;
#endif

EXTERN_C const CLSID CLSID_MediaLibrary07;

#ifdef __cplusplus

class DECLSPEC_UUID("8F5F0CEF-4E98-470e-A62A-7A6BFA79457A")
MediaLibrary07;
#endif

EXTERN_C const CLSID CLSID_Playback07;

#ifdef __cplusplus

class DECLSPEC_UUID("6becd912-ace6-4d29-beb5-e835c5f39fd6")
Playback07;
#endif

EXTERN_C const CLSID CLSID_PlaybackOrders07;

#ifdef __cplusplus

class DECLSPEC_UUID("655bd1a9-22ec-47ab-ac84-0210e3da0fd5")
PlaybackOrders07;
#endif

EXTERN_C const CLSID CLSID_PlaybackSettings07;

#ifdef __cplusplus

class DECLSPEC_UUID("f022879b-544f-48bd-8911-c521406e6e40")
PlaybackSettings07;
#endif

EXTERN_C const CLSID CLSID_Playlist07;

#ifdef __cplusplus

class DECLSPEC_UUID("1ba62148-9f9a-4ca4-94a8-22b381a9c232")
Playlist07;
#endif

EXTERN_C const CLSID CLSID_Playlists07;

#ifdef __cplusplus

class DECLSPEC_UUID("81b34219-82bd-4651-805d-9cfe298a871b")
Playlists07;
#endif

EXTERN_C const CLSID CLSID_ReplaygainSettings07;

#ifdef __cplusplus

class DECLSPEC_UUID("a93cd01d-c851-4953-a2fa-839e4f720635")
ReplaygainSettings07;
#endif

EXTERN_C const CLSID CLSID_Track07;

#ifdef __cplusplus

class DECLSPEC_UUID("f4e5d447-3a78-40d4-8d3d-b311804f67b1")
Track07;
#endif

EXTERN_C const CLSID CLSID_Tracks07;

#ifdef __cplusplus

class DECLSPEC_UUID("942caf04-86a1-42d6-9e71-e62a24987ca3")
Tracks07;
#endif
#endif /* __Foobar2000_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


