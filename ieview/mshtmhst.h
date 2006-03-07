#define DISPID_AMBIENT_DLCONTROL                (-5512)
#define DISPID_AMBIENT_USERAGENT                (-5513)
#define DISPID_SECURITYDOMAIN                   (-5514)
#define DLCTL_DLIMAGES                          0x00000010
#define DLCTL_VIDEOS                            0x00000020
#define DLCTL_BGSOUNDS                          0x00000040
#define DLCTL_NO_SCRIPTS                        0x00000080
#define DLCTL_NO_JAVA                           0x00000100
#define DLCTL_NO_RUNACTIVEXCTLS                 0x00000200
#define DLCTL_NO_DLACTIVEXCTLS                  0x00000400
#define DLCTL_DOWNLOADONLY                      0x00000800
#define DLCTL_NO_FRAMEDOWNLOAD                  0x00001000
#define DLCTL_RESYNCHRONIZE                     0x00002000
#define DLCTL_PRAGMA_NO_CACHE                   0x00004000
#define DLCTL_FORCEOFFLINE                      0x10000000
#define DLCTL_NO_CLIENTPULL                     0x20000000
#define DLCTL_SILENT                            0x40000000
#define DLCTL_OFFLINEIFNOTCONNECTED             0x80000000
#define DLCTL_OFFLINE                           DLCTL_OFFLINEIFNOTCONNECTED

#ifndef _MSC_VER

/*****************************************************************************
 * IDocHostUIHandler interface
 */

const IID IID_IHTMLAnchorElement ={0x3050f1da,0x98b5,0x11cf,{0xbb, 0x82, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b}};

DECLARE_INTERFACE_(IHTMLAnchorElement,IDispatch)
{

	STDMETHOD(put_href)(BSTR) PURE;
	STDMETHOD(get_href)(BSTR *) PURE;
	STDMETHOD(put_target)(BSTR) PURE;
	STDMETHOD(get_target)(BSTR *) PURE;
	STDMETHOD(put_rel)(BSTR) PURE;
	STDMETHOD(get_rel)(BSTR *) PURE;
	STDMETHOD(put_rev)(BSTR) PURE;
	STDMETHOD(get_rev)(BSTR *) PURE;
	STDMETHOD(put_urn)(BSTR) PURE;
	STDMETHOD(get_urn)(BSTR *) PURE;
	STDMETHOD(put_Methods)(BSTR) PURE;
	STDMETHOD(get_Methods)(BSTR *) PURE;
	STDMETHOD(put_name)(BSTR) PURE;
	STDMETHOD(get_name)(BSTR *) PURE;
	STDMETHOD(put_host)(BSTR) PURE;
	STDMETHOD(get_host)(BSTR *) PURE;
	STDMETHOD(put_hostname)(BSTR) PURE;
	STDMETHOD(get_hostname)(BSTR *) PURE;
	STDMETHOD(put_pathname)(BSTR) PURE;
	STDMETHOD(get_pathname)(BSTR *) PURE;
	STDMETHOD(put_port)(BSTR) PURE;
	STDMETHOD(get_port)(BSTR *) PURE;
	STDMETHOD(put_protocol)(BSTR) PURE;
	STDMETHOD(get_protocol)(BSTR *) PURE;
	STDMETHOD(put_search)(BSTR) PURE;
	STDMETHOD(get_search)(BSTR *) PURE;
	STDMETHOD(put_hash)(BSTR) PURE;
	STDMETHOD(get_hash)(BSTR *) PURE;
	STDMETHOD(put_onblur)(VARIANT) PURE;
	STDMETHOD(get_onblur)(VARIANT *) PURE;
	STDMETHOD(put_onfocus)(VARIANT) PURE;
	STDMETHOD(get_onfocus)(VARIANT *) PURE;
	STDMETHOD(put_accessKey)(BSTR) PURE;
	STDMETHOD(get_accessKey)(BSTR *) PURE;
	STDMETHOD(get_protocolLong)(BSTR *) PURE;
	STDMETHOD(get_mimeType)(BSTR *) PURE;
	STDMETHOD(get_nameProp)(BSTR *) PURE;
	STDMETHOD(put_tabIndex)(short) PURE;
	STDMETHOD(get_tabIndex)(short *) PURE;
	STDMETHOD(focus)(void) PURE;
	STDMETHOD(blur)(void) PURE;
};
#endif


/*****************************************************************************
 * IDocHostUIHandler interface
 */

const IID IID_IDocHostUIHandler ={0xbd3f23c0,0xd43e,0x11CF,{0x89, 0x3b, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x1a}};

typedef
enum tagDOCHOSTUITYPE {
        DOCHOSTUITYPE_BROWSE    = 0,
        DOCHOSTUITYPE_AUTHOR    = 1,
} DOCHOSTUITYPE;

typedef enum tagDOCHOSTUIDBLCLK {
        DOCHOSTUIDBLCLK_DEFAULT         = 0,
        DOCHOSTUIDBLCLK_SHOWPROPERTIES  = 1,
        DOCHOSTUIDBLCLK_SHOWCODE        = 2,
} DOCHOSTUIDBLCLK ;

typedef enum tagDOCHOSTUIFLAG {
        DOCHOSTUIFLAG_DIALOG            = 1,
        DOCHOSTUIFLAG_DISABLE_HELP_MENU = 2,
        DOCHOSTUIFLAG_NO3DBORDER        = 4,
        DOCHOSTUIFLAG_SCROLL_NO         = 8,
        DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE = 16,
        DOCHOSTUIFLAG_OPENNEWWIN        = 32,
        DOCHOSTUIFLAG_DISABLE_OFFSCREEN = 64,
        DOCHOSTUIFLAG_FLAT_SCROLLBAR = 128,
        DOCHOSTUIFLAG_DIV_BLOCKDEFAULT = 256,
        DOCHOSTUIFLAG_ACTIVATE_CLIENTHIT_ONLY = 512,
        DOCHOSTUIFLAG_DISABLE_COOKIE = 1024,
} DOCHOSTUIFLAG ;

DECLARE_INTERFACE_(IDocHostUIHandler,IUnknown)
{

    typedef struct _DOCHOSTUIINFO
    {
        ULONG cbSize;
        DWORD dwFlags;
        DWORD dwDoubleClick;
    } DOCHOSTUIINFO;


	STDMETHOD(ShowContextMenu)(THIS_ DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved) PURE;
	STDMETHOD(GetHostInfo)(THIS_ DOCHOSTUIINFO *pInfo) PURE;
    STDMETHOD(ShowUI)(THIS_ DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget,
    				IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc) PURE;
    STDMETHOD(HideUI)(THIS) PURE;
    STDMETHOD(UpdateUI)(THIS) PURE;
    STDMETHOD(EnableModeless)(THIS_ BOOL fEnable) PURE;
    STDMETHOD(OnDocWindowActivate)(THIS_ BOOL fEnable) PURE;
    STDMETHOD(OnFrameWindowActivate)(THIS_ BOOL fEnable) PURE;
    STDMETHOD(ResizeBorder)(THIS_ LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow) PURE;
    STDMETHOD(TranslateAccelerator)(THIS_ LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID) PURE;
    STDMETHOD(GetOptionKeyPath)(THIS_ LPOLESTR *pchKey, DWORD dw) PURE;
    STDMETHOD(GetDropTarget)(THIS_ IDropTarget *pDropTarget, IDropTarget **ppDropTarget) PURE;
    STDMETHOD(GetExternal)(THIS_ IDispatch **ppDispatch) PURE;
    STDMETHOD(TranslateUrl)(THIS_ DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut) PURE;
    STDMETHOD(FilterDataObject)(THIS_ IDataObject *pDO, IDataObject **ppDORet) PURE;
};
