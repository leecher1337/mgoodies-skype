#include <windows.h>
#include <objbase.h>
#include <exdisp.h>
#include <stdio.h>

#include <initguid.h>
#include <exdisp.h>
#include <shlguid.h>
#include <memory.h>
#include <shlobj.h>
#include <mshtml.h>
#include <oleauto.h>

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
char szClassName[ ] = "WindowsApp";

class WebBrowserEvents:public DWebBrowserEvents2
{

};

class SmallFucker:public IOleClientSite, public IOleInPlaceSite
{
public:
	SmallFucker(HWND hwnd)
	{
		m_cRef=0;
		this->hwnd=hwnd;
	}
private:
	HWND	hwnd;
	int		m_cRef;
	BOOL  m_bInPlaceActive;

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv)
	{
		*ppv=NULL;
		if (IID_IUnknown==riid)
			*ppv=this;
		if (IID_IOleClientSite==riid)
			*ppv=(IOleClientSite*)this;//Unknown)m_pIOleClientSite;
		if (IID_IOleWindow==riid || IID_IOleInPlaceSite==riid)
			*ppv=(IOleInPlaceSite*)this;//m_pIOleIPSite;
		if (NULL!=*ppv) {
			((LPUNKNOWN)*ppv)->AddRef();
			return NOERROR;
		}
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef(void)
	{
		++m_cRef;
  		return m_cRef;
	}

	STDMETHODIMP_(ULONG) Release(void)
	{
		--m_cRef;
		return m_cRef;
	}

	// IOleWindow
	STDMETHOD(GetWindow)(HWND *phwnd)
	{
	   *phwnd = hwnd;
		return S_OK;
	}

	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

	// IOleInPlace
   STDMETHOD(CanInPlaceActivate)(void)
	{
	   return S_OK;
	}

   STDMETHOD(OnInPlaceActivate)(void)
	{
		m_bInPlaceActive = TRUE;
		return S_OK;
	}

   STDMETHOD(OnUIActivate)(void)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(GetWindowContext)(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc,
	                            LPRECT lprcPosRect, LPRECT lprcClipRect,
                               LPOLEINPLACEFRAMEINFO lpFrameInfo)
	{
		GetClientRect(hwnd, lprcPosRect);
		GetClientRect(hwnd, lprcClipRect);
		return S_OK;
	}

   STDMETHOD(Scroll)(SIZE scrollExtant)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(OnUIDeactivate)(BOOL fUndoable)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(OnInPlaceDeactivate)( void)
	{
		m_bInPlaceActive = FALSE;
		return S_OK;
	}

   STDMETHOD(DiscardUndoState)( void)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(DeactivateAndUndo)( void)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

	// IOleClientSite
   STDMETHOD(SaveObject)(void)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(GetContainer)(IOleContainer **ppContainer)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(ShowObject)(void)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(OnShowWindow)(BOOL fShow)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(RequestNewObjectLayout)(void)
	{
		return E_NOTIMPL;//ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

};


IHTMLDocument2 *GetDocument(IWebBrowser2 * webBrowser) {
	HRESULT         hr       = S_OK;

	IHTMLDocument2 *document = NULL;
	while ((document == NULL) && (hr == S_OK)) {
		Sleep(0);
		IDispatch *dispatch = NULL;
		hr = webBrowser->get_Document(&dispatch);
		if (SUCCEEDED(hr) && (dispatch != NULL)) {
	    	hr = dispatch->QueryInterface(IID_IHTMLDocument2, (void **)&document);
			dispatch->Release();
		}
	}
		/*
		if (document != NULL) {
		  document->Release();
		}

		
		if (webBrowser != NULL) {
			// get browser document's dispatch interface
			IDispatch *document_dispatch = NULL;
			HRESULT hr = webBrowser->get_Document(&document_dispatch);
			if (SUCCEEDED(hr) && (document_dispatch != NULL)) {
				// get the actual document interface
				hr = document_dispatch->QueryInterface(IID_IHTMLDocument2,
			                                       (void **)&document);

				// release dispatch interface
				document_dispatch->Release();
			}
		}*/
	return document;
}

int WINAPI
WinMain (HINSTANCE hThisInstance,
         HINSTANCE hPrevInstance,
         LPSTR lpszArgument,
         int nFunsterStil)

{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "Windows App",       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nFunsterStil);



/*
   		hr = pWebBrowser->put_Visible(VARIANT_TRUE);
*/

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
	CoUninitialize();
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK
WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SmallFucker *sf=new SmallFucker(hwnd);
	static HRESULT hr;
	static IWebBrowser2* pWebBrowser = NULL;
	static IOleObject*   pOleObject = NULL;
	
    switch (message)                  /* handle the messages */
    {
		case WM_CREATE:
			{
	   			if (FAILED(OleInitialize(NULL))) {
					MessageBox(NULL,"OleInitialize fiailed.","RESULT",MB_OK);
				}
	   			if (FAILED(hr = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_INPROC, IID_IWebBrowser2, (LPVOID*)&pWebBrowser))) {
					MessageBox(NULL,"CoCreateInstance fiailed.","RESULT",MB_OK);
		   		}
				else {
					pWebBrowser->QueryInterface(IID_IOleObject, (void**)&pOleObject);
					pOleObject->SetClientSite(sf);
			   		RECT rcClient;
					GetClientRect(hwnd, &rcClient);
					MSG msg;
					hr=pOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, &msg, sf, 0, hwnd, &rcClient);
					pWebBrowser->Navigate(L"about:blank", NULL, NULL, NULL, NULL);
///					pWebBrowser->Navigate(L"res://webhost.exe/startpage.htm", NULL, NULL, NULL, NULL);
					IHTMLDocument2 *document = GetDocument(pWebBrowser);
					if (document != NULL) {
			            SAFEARRAY *safe_array = SafeArrayCreateVector(VT_VARIANT,0,1);
						VARIANT	*variant;
						SafeArrayAccessData(safe_array,(LPVOID *)&variant);
						variant->vt      = VT_BSTR;
						variant->bstrVal = SysAllocString(OLESTR("<htm><h1>Stream Test</h1><p>This HTML content is being loaded from a stream.</html>"));
						SafeArrayUnaccessData(safe_array);
						document->write(safe_array);
						document->Release();
						document = NULL;
					}

				static char *szHTMLText= "<htm><h1>Stream Test</h1><p>This HTML content is being loaded from a stream.</html>";
		        IUnknown* pUnkBrowser = NULL;
				IUnknown* pUnkDisp = NULL;
				IStream* pStream = NULL;
				if (SUCCEEDED(pWebBrowser->QueryInterface(IID_IUnknown, (void **)&pUnkBrowser))) {
					if (SUCCEEDED(pWebBrowser->QueryInterface(IID_IUnknown, (void **)&pUnkBrowser))) {
					}

				}
				}
			}
			break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
