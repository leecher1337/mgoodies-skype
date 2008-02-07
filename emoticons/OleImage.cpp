#include "commons.h"

#define GIF_DISPOSAL_UNSPECIFIED	0
#define GIF_DISPOSAL_LEAVE			1
#define GIF_DISPOSAL_BACKGROUND		2
#define GIF_DISPOSAL_PREVIOUS		3


void PreMultiply(HBITMAP hBitmap);

typedef map<UINT_PTR, OleImage *> ImageTimerMapType;

static ImageTimerMapType timers;



OleImage::OleImage(const TCHAR *aFilename, const TCHAR *aText, const TCHAR *aTooltip)
{
	memset(&si, 0, sizeof(si));
	memset(&ag, 0, sizeof(ag));

	refCount = 1;
	filename = mir_t2a(aFilename);
	originalFilename = mir_tstrdup(aFilename);
	text = mir_tstrdup(aText);
	closed = FALSE;

	if (aTooltip == NULL)
		tooltip = NULL;
	else
	{
#ifdef UNICODE
		tooltip = SysAllocString(aTooltip);
#else
		WCHAR *wtext = mir_t2u(aTooltip);
		tooltip = SysAllocString(wtext);
		mir_free(wtext);
#endif
	}

	clientSite = NULL;
	oleAdviseHolder = NULL;
	viewAdviseSink = NULL;

	if (!LoadImages())
		return;

	CalcSize();
}


OleImage::~OleImage()
{
	KillTimer();

	if (clientSite != NULL)
	{
		clientSite->Release();
		clientSite = NULL;
	}

	if (oleAdviseHolder != NULL)
	{
		oleAdviseHolder->Release();
		oleAdviseHolder = NULL;
	}

	if (viewAdviseSink != NULL)
	{
		viewAdviseSink->Release();
		viewAdviseSink = NULL;
	}

	DestroyImages();

	if (tooltip != NULL) 
	{
		SysFreeString(tooltip);
		tooltip = NULL;
	}
	mir_free(filename);
	mir_free(originalFilename);
	mir_free(text);
}


BOOL OleImage::ShowDownloadingIcon(BOOL show)
{
	DestroyImages();

	if (show)
	{
		mir_free(filename);

		size_t len = lstrlen(protocolsFolder) + 20;
		filename = (char *) mir_alloc(len * sizeof(char));
		mir_snprintf(filename, len, TCHAR_STR_PARAM "\\downloading.gif", protocolsFolder);
	}
	else
	{
		mir_free(filename);

		filename = mir_t2a(originalFilename);
	}

	if (!LoadImages())
		return FALSE;

	CalcSize();

	OnImageChange();

	return TRUE;
}


BOOL OleImage::LoadImages()
{
	animated = LoadAnimatedGif();
	if (!animated)
		if (!LoadStaticImage())
			return FALSE;

	return TRUE;
}


void OleImage::DestroyImages()
{
	if (animated)
	{
		DestroyAnimatedGif();
	}
	else if (si.hBmp != NULL)
	{
		DeleteObject(si.hBmp);
		si.hBmp = NULL;
	}

	memset(&si, 0, sizeof(si));
	memset(&ag, 0, sizeof(ag));
}


const TCHAR * OleImage::GetFilename() const
{
	return originalFilename;
}


const TCHAR * OleImage::GetText() const
{
	return text;
}


BOOL OleImage::isValid() const
{
	return animated || si.hBmp != NULL;
}


void OleImage::CalcSize()
{
	HDC hdc = GetDC(NULL);
	int ppiX = GetDeviceCaps(hdc, LOGPIXELSX);
	int ppiY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);

	sizel.cx = (2540 * (width + 1) + ppiX / 2) / ppiX;
	sizel.cy = (2540 * (height + 1) + ppiY / 2) / ppiY;
}


// IUnknown ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


HRESULT STDMETHODCALLTYPE OleImage::QueryInterface(/* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL) 
		return E_POINTER;

	if (riid == IID_IOleImage)
	{
		*ppvObject = (OleImage *) this;
	}
	else if (riid == IID_IViewObject)
	{
		*ppvObject = (IViewObject *) this;
	}
	else if (riid == IID_IOleObject)
	{
		*ppvObject = (IOleObject *) this;
	}
	else if (riid == __uuidof(ITooltipData))
	{
		*ppvObject = (ITooltipData *) this;
	}
	else if (riid == IID_IUnknown) 
	{
		*ppvObject = (OleImage *) this;
	}
	else 
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


ULONG STDMETHODCALLTYPE OleImage::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG STDMETHODCALLTYPE OleImage::Release(void)
{
	LONG ret = InterlockedDecrement(&refCount);
	if (ret <= 0)
		delete this;
	return ret;
}



// IOleObject //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


HRESULT STDMETHODCALLTYPE OleImage::SetClientSite(/* [unique][in] */ IOleClientSite *pClientSite)
{
	if (clientSite != NULL)
		clientSite->Release();

	clientSite = pClientSite;

	if (clientSite != NULL)
		clientSite->AddRef();

	return S_OK;
}


HRESULT STDMETHODCALLTYPE OleImage::GetClientSite(/* [out] */ IOleClientSite **ppClientSite)
{
	*ppClientSite = clientSite;
	return S_OK;
}


HRESULT STDMETHODCALLTYPE OleImage::SetHostNames(/* [in] */ LPCOLESTR szContainerApp, /* [unique][in] */ LPCOLESTR szContainerObj)
{
	return S_OK;
}


HRESULT STDMETHODCALLTYPE OleImage::Close(/* [in] */ DWORD dwSaveOption)
{
	KillTimer();

	if (animated)
		ag.started = FALSE;

	closed = TRUE;

//	if (viewAdviseSink != NULL)
//	{
//		viewAdviseSink->OnClose();
//		//viewAdviseSink = NULL;
//	}

	return S_OK;
}


HRESULT STDMETHODCALLTYPE OleImage::SetMoniker(/* [in] */ DWORD dwWhichMoniker, /* [unique][in] */ IMoniker *pmk)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::GetMoniker(/* [in] */ DWORD dwAssign, /* [in] */ DWORD dwWhichMoniker, /* [out] */ IMoniker **ppmk)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::InitFromData(/* [unique][in] */ IDataObject *pDataObject, /* [in] */ BOOL fCreation, /* [in] */ DWORD dwReserved)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::GetClipboardData(/* [in] */ DWORD dwReserved, /* [out] */ IDataObject **ppDataObject)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::DoVerb(/* [in] */ LONG iVerb, /* [unique][in] */ LPMSG lpmsg, /* [unique][in] */ IOleClientSite *pActiveSite, /* [in] */ LONG lindex, /* [in] */ HWND hwndParent, /* [unique][in] */ LPCRECT lprcPosRect)
{
	return OLEOBJ_E_NOVERBS;
}


HRESULT STDMETHODCALLTYPE OleImage::EnumVerbs(/* [out] */ IEnumOLEVERB **ppEnumOleVerb)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::Update(void)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::IsUpToDate(void)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::GetUserClassID(/* [out] */ CLSID *pClsid)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::GetUserType(/* [in] */ DWORD dwFormOfType, /* [out] */ LPOLESTR *pszUserType)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::SetExtent(/* [in] */ DWORD dwDrawAspect, /* [in] */ SIZEL *psizel)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::GetExtent(/* [in] */ DWORD dwDrawAspect, /* [out] */ SIZEL *psizel)
{
	if (dwDrawAspect != DVASPECT_CONTENT) 
		return DV_E_DVASPECT;
	if (psizel == NULL)
		return E_POINTER;

	*psizel = sizel;

	return S_OK;
}


HRESULT STDMETHODCALLTYPE OleImage::Advise(/* [unique][in] */ IAdviseSink *pAdvSink, /* [out] */ DWORD *pdwConnection)
{
	HRESULT hr = S_OK;
	if (oleAdviseHolder == NULL)
		hr = CreateOleAdviseHolder(&oleAdviseHolder);
	if (SUCCEEDED(hr))
		hr = oleAdviseHolder->Advise(pAdvSink, pdwConnection);
	return hr;
}


HRESULT STDMETHODCALLTYPE OleImage::Unadvise(/* [in] */ DWORD dwConnection)
{
	if (oleAdviseHolder != NULL)
		return oleAdviseHolder->Unadvise(dwConnection);
	else
		return E_FAIL;
}


HRESULT STDMETHODCALLTYPE OleImage::EnumAdvise(/* [out] */ IEnumSTATDATA **ppenumAdvise)
{
	if (ppenumAdvise == NULL)
		return E_POINTER;
	*ppenumAdvise = NULL;

	if (oleAdviseHolder != NULL)
		return oleAdviseHolder->EnumAdvise(ppenumAdvise);
	else
		return E_FAIL;
}


HRESULT STDMETHODCALLTYPE OleImage::GetMiscStatus(/* [in] */ DWORD dwAspect, /* [out] */ DWORD *pdwStatus)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::SetColorScheme(/* [in] */ LOGPALETTE *pLogpal)
{
	return E_NOTIMPL;
}



// IViewObject /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


HRESULT STDMETHODCALLTYPE OleImage::Draw(/* [in] */ DWORD dwDrawAspect, /* [in] */ LONG lindex, /* [unique][in] */ void *pvAspect, /* [unique][in] */ DVTARGETDEVICE *ptd, /* [in] */ HDC hdcTargetDev, /* [in] */ HDC hdcDraw, /* [in] */ LPCRECTL lprcBounds, /* [unique][in] */ LPCRECTL lprcWBounds, /* [in] */ BOOL ( STDMETHODCALLTYPE *pfnContinue )(ULONG_PTR dwContinue), /* [in] */ ULONG_PTR dwContinue)
{
	if (dwDrawAspect != DVASPECT_CONTENT) 
		return DV_E_DVASPECT;
	if (hdcDraw == NULL)
		return E_INVALIDARG;
	if (lprcBounds == NULL)
		return E_INVALIDARG;

	if (closed)
		closed = FALSE;

	int oldBkMode = SetBkMode(hdcDraw, TRANSPARENT);

	int w = min(lprcBounds->right - lprcBounds->left, width);
	int h = min(lprcBounds->bottom - lprcBounds->top, height);

	if (animated)
	{
		if (!ag.started)
			AnimatedGifMountFrame();
        
		HDC hdcImg = CreateCompatibleDC(hdcDraw);
		HBITMAP oldBmp = (HBITMAP) SelectObject(hdcImg, ag.hbms[ag.frame.num]);

		BLENDFUNCTION bf = {0};
		bf.SourceConstantAlpha = 255;
		bf.AlphaFormat = AC_SRC_ALPHA;
		AlphaBlend(hdcDraw, lprcBounds->left, lprcBounds->top, w, h, hdcImg, 0, 0, w, h, bf);

		SelectObject(hdcImg, oldBmp);
		DeleteDC(hdcImg);
		
		if (!ag.started)
		{
			SetTimer(ag.times[ag.frame.num]);
			ag.started = TRUE;
		}
	}
	else if (si.hBmp != NULL)
	{
		HDC hdcImg = CreateCompatibleDC(hdcDraw);
		HBITMAP oldBmp = (HBITMAP) SelectObject(hdcImg, si.hBmp);

		if (si.transparent)
		{
			BLENDFUNCTION bf = {0};
			bf.SourceConstantAlpha = 255;
			bf.AlphaFormat = AC_SRC_ALPHA;
			AlphaBlend(hdcDraw, lprcBounds->left, lprcBounds->top, w, h, hdcImg, 0, 0, w, h, bf);
		}
		else
		{
			BitBlt(hdcDraw, lprcBounds->left, lprcBounds->top, w, h, hdcImg, 0, 0, SRCCOPY);
		}

		SelectObject(hdcImg, oldBmp);
		DeleteDC(hdcImg);
	}

	SetBkMode(hdcDraw, oldBkMode);

	return S_OK;
}


HRESULT STDMETHODCALLTYPE OleImage::GetColorSet(/* [in] */ DWORD dwDrawAspect, /* [in] */ LONG lindex, /* [unique][in] */ void *pvAspect, /* [unique][in] */ DVTARGETDEVICE *ptd, /* [in] */ HDC hicTargetDev, /* [out] */ LOGPALETTE **ppColorSet)
{
	return S_FALSE;
}


HRESULT STDMETHODCALLTYPE OleImage::Freeze(/* [in] */ DWORD dwDrawAspect, /* [in] */ LONG lindex, /* [unique][in] */ void *pvAspect, /* [out] */ DWORD *pdwFreeze)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::Unfreeze(/* [in] */ DWORD dwFreeze)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE OleImage::SetAdvise(/* [in] */ DWORD aspects, /* [in] */ DWORD advf, /* [unique][in] */ IAdviseSink *pAdvSink)
{
	if (aspects != DVASPECT_CONTENT) 
		return DV_E_DVASPECT;

	if (viewAdviseSink != NULL)
		viewAdviseSink->Release();

	viewAdviseSink = pAdvSink;
	viewAdvf = advf;
	
	if (viewAdviseSink != NULL)
		viewAdviseSink->AddRef();

	if (viewAdviseSink != NULL && viewAdvf & ADVF_PRIMEFIRST)
		SendOnViewChage();

	return S_OK;
}


/* [local] */ HRESULT STDMETHODCALLTYPE OleImage::GetAdvise(/* [unique][out] */ DWORD *pAspects, /* [unique][out] */ DWORD *pAdvf, /* [out] */ IAdviseSink **ppAdvSink)
{
	*pAspects = DVASPECT_CONTENT;
	*pAdvf = viewAdvf;
	*ppAdvSink = viewAdviseSink;

	return S_OK;
}



// ITooltipData /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


HRESULT OleImage::SetTooltip(/* [in] */ BSTR aTooltip)
{
	if (tooltip != NULL) 
	{
		SysFreeString(tooltip);
		tooltip = NULL;
	}
	if (aTooltip != NULL)
		tooltip = SysAllocString(aTooltip);

	return S_OK;
}


HRESULT OleImage::GetTooltip(/* [out, retval] */ BSTR * aTooltip)
{
	if (tooltip == NULL) 
		*aTooltip = NULL;
	else
		*aTooltip = SysAllocString(tooltip);

	return S_OK;
}




void OleImage::OnImageChange()
{
	if (closed)
		return;

	SendOnViewChage();
	NotifyHpp();
}


void OleImage::NotifyHpp()
{
	if (clientSite == NULL)
		return;

	IOleWindow *oleWindow = NULL;
	clientSite->QueryInterface(IID_IOleWindow, (void **) &oleWindow);
	if (oleWindow != NULL)
	{
		HWND hwnd = NULL;
		oleWindow->GetWindow(&hwnd);
		if (hwnd != NULL)
		{
			HWND hwndGrand = GetParent(hwnd);
			if (hwndGrand != NULL)
			{
				FVCNDATA_NMHDR nmhdr = {0};
				nmhdr.cbSize = sizeof(FVCNDATA_NMHDR);
				nmhdr.hwndFrom = hwnd;
				nmhdr.code = NM_FIREVIEWCHANGE;
				nmhdr.bAction = FVCA_SENDVIEWCHANGE;
				nmhdr.bEvent = FVCN_POSTFIRE;
				SendMessage(hwndGrand, WM_NOTIFY, (WPARAM) nmhdr.hwndFrom, (LPARAM) &nmhdr);
			}
		}
		oleWindow->Release();
	}
}


void OleImage::Invalidate()
{
	if (clientSite == NULL)
		return;
	
	IOleWindow *oleWindow = NULL;
	clientSite->QueryInterface(IID_IOleWindow, (void **) &oleWindow);
	if (oleWindow != NULL)
	{
		HWND hwnd = NULL;
		oleWindow->GetWindow(&hwnd);
		if (hwnd != NULL)
		{
			InvalidateRect(hwnd, NULL, FALSE);
		}
		oleWindow->Release();
	}
}


void OleImage::SendOnViewChage()
{
	if (viewAdviseSink == NULL)
		return;

	viewAdviseSink->OnViewChange(DVASPECT_CONTENT, -1);
	if (viewAdvf & ADVF_ONLYONCE)
	{
		viewAdviseSink->Release();
		viewAdviseSink = NULL;
	}
}


BOOL OleImage::LoadStaticImage()
{
	// Load static image
	DWORD transp;
	si.hBmp = (HBITMAP) CallService(MS_AV_LOADBITMAP32, (WPARAM) &transp, (LPARAM) filename);
	if (si.hBmp == NULL)
		return FALSE;

	BITMAP bmp;
	if (!GetObject(si.hBmp, sizeof(bmp), &bmp))
	{
		DeleteObject(si.hBmp);
		si.hBmp = NULL;
		return FALSE;
	}

	si.transparent = (bmp.bmBitsPixel == 32 && transp);
	if (si.transparent)
		PreMultiply(si.hBmp);

	width = bmp.bmWidth;
	height = bmp.bmHeight;

	return TRUE;
}


BOOL OleImage::LoadAnimatedGif()
{
	int x, y;

	FREE_IMAGE_FORMAT fif = fei->FI_GetFileType(filename, 0);
	if(fif == FIF_UNKNOWN)
		fif = fei->FI_GetFIFFromFilename(filename);

	ag.multi = fei->FI_OpenMultiBitmap(fif, filename, FALSE, TRUE, FALSE, GIF_LOAD256);
	if (ag.multi == NULL)
		return FALSE;

	ag.frameCount = fei->FI_GetPageCount(ag.multi);
	if (ag.frameCount <= 1)
		goto ERR;

	if (!AnimatedGifGetData())
		goto ERR;

	//allocate entire logical area
	ag.dib = fei->FI_Allocate(width, height, 32, 0, 0, 0);
	if (ag.dib == NULL)
		goto ERR;

	//fill with background color to start
	for (y = 0; y < height; y++) 
	{
		RGBQUAD *scanline = (RGBQUAD *) fei->FI_GetScanLine(ag.dib, y);
		for (x = 0; x < width; x++)
			*scanline++ = ag.background;
	}

	ag.hbms = (HBITMAP *) malloc(sizeof(HBITMAP) * ag.frameCount);
	memset(ag.hbms, 0, sizeof(HBITMAP) * ag.frameCount);

	ag.times = (int *) malloc(sizeof(int) * ag.frameCount);
	memset(ag.times, 0, sizeof(int) * ag.frameCount);

	ag.frame.num = 0;

	return TRUE;
ERR:
	fei->FI_CloseMultiBitmap(ag.multi, 0);
	ag.multi = NULL;

	return FALSE;
}


BOOL OleImage::AnimatedGifGetData()
{
	FIBITMAP *page = fei->FI_LockPage(ag.multi, 0);
	if (page == NULL)
		return FALSE;
	
	// Get info
	FITAG *tag = NULL;
	if (!fei->FI_GetMetadata(FIMD_ANIMATION, page, "LogicalWidth", &tag))
		goto ERR;
	width = *(WORD *)fei->FI_GetTagValue(tag);
	
	if (!fei->FI_GetMetadata(FIMD_ANIMATION, page, "LogicalHeight", &tag))
		goto ERR;
	height = *(WORD *)fei->FI_GetTagValue(tag);
	
	if (!fei->FI_GetMetadata(FIMD_ANIMATION, page, "Loop", &tag))
		goto ERR;
	ag.loop = (*(LONG *)fei->FI_GetTagValue(tag) > 0);
	
	if (fei->FI_HasBackgroundColor(page))
		fei->FI_GetBackgroundColor(page, &ag.background);

	fei->FI_UnlockPage(ag.multi, page, FALSE);
	return TRUE;

ERR:
	fei->FI_UnlockPage(ag.multi, page, FALSE);
	return FALSE;
}


void OleImage::AnimatedGifMountFrame()
{
	int page = ag.frame.num;
	if (ag.hbms[page] != NULL)
	{
		ag.frame.disposal_method = GIF_DISPOSAL_LEAVE;
		return;
	}

	FIBITMAP *dib = fei->FI_LockPage(ag.multi, page);
	if (dib == NULL)
		return;

	FITAG *tag = NULL;
	if (fei->FI_GetMetadata(FIMD_ANIMATION, dib, "FrameLeft", &tag))
		ag.frame.left = *(WORD *)fei->FI_GetTagValue(tag);
	else
		ag.frame.left = 0;

	if (fei->FI_GetMetadata(FIMD_ANIMATION, dib, "FrameTop", &tag))
		ag.frame.top = *(WORD *)fei->FI_GetTagValue(tag);
	else
		ag.frame.top = 0;

	if (fei->FI_GetMetadata(FIMD_ANIMATION, dib, "FrameTime", &tag))
		ag.times[page] = *(LONG *)fei->FI_GetTagValue(tag);
	else
		ag.times[page] = 0;

	if (fei->FI_GetMetadata(FIMD_ANIMATION, dib, "DisposalMethod", &tag))
		ag.frame.disposal_method = *(BYTE *)fei->FI_GetTagValue(tag);
	else
		ag.frame.disposal_method = 0;

	ag.frame.width  = fei->FI_GetWidth(dib);
	ag.frame.height = fei->FI_GetHeight(dib);


	//decode page
	int palSize = fei->FI_GetColorsUsed(dib);
	RGBQUAD *pal = fei->FI_GetPalette(dib);
	BOOL have_transparent = FALSE;
	int transparent_color = -1;
	if( fei->FI_IsTransparent(dib) ) {
		int count = fei->FI_GetTransparencyCount(dib);
		BYTE *table = fei->FI_GetTransparencyTable(dib);
		for( int i = 0; i < count; i++ ) {
			if( table[i] == 0 ) {
				have_transparent = TRUE;
				transparent_color = i;
				break;
			}
		}
	}

	//copy page data into logical buffer, with full alpha opaqueness
	for( int y = 0; y < ag.frame.height; y++ ) {
		RGBQUAD *scanline = (RGBQUAD *)fei->FI_GetScanLine(ag.dib, height - (y + ag.frame.top) - 1) + ag.frame.left;
		BYTE *pageline = fei->FI_GetScanLine(dib, ag.frame.height - y - 1);
		for( int x = 0; x < ag.frame.width; x++ ) {
			if( !have_transparent || *pageline != transparent_color ) {
				*scanline = pal[*pageline];
				scanline->rgbReserved = 255;
			}
			scanline++;
			pageline++;
		}
	}

	ag.hbms[page] = fei->FI_CreateHBITMAPFromDIB(ag.dib);

	if (transparent_color)
		PreMultiply(ag.hbms[page]);

	fei->FI_UnlockPage(ag.multi, dib, FALSE);
}


void OleImage::AnimatedGifDispodeFrame()
{
	if (ag.frame.disposal_method == GIF_DISPOSAL_PREVIOUS) 
	{
		// TODO
	} 
	else if (ag.frame.disposal_method == GIF_DISPOSAL_BACKGROUND) 
	{
		for (int y = 0; y < ag.frame.height; y++) 
		{
			RGBQUAD *scanline = (RGBQUAD *) fei->FI_GetScanLine(ag.dib, height - (y + ag.frame.top) - 1) + ag.frame.left;
			for (int x = 0; x < ag.frame.width; x++)
				*scanline++ = ag.background;
		}
	}
}


void OleImage::AnimatedGifDeleteTmpValues()
{
	if (ag.multi != NULL)
	{
		fei->FI_CloseMultiBitmap(ag.multi, 0);
		ag.multi = NULL;
	}

	if (ag.dib != NULL)
	{
		fei->FI_Unload(ag.dib);
		ag.dib = NULL;
	}
}


void OleImage::DestroyAnimatedGif()
{
	AnimatedGifDeleteTmpValues();

	if (ag.hbms != NULL)
	{
		for (int i = 0; i < ag.frameCount; i++)
			if (ag.hbms[i] != NULL)
				DeleteObject(ag.hbms[i]);

		free(ag.hbms);
		ag.hbms = NULL;
	}

	if (ag.times != NULL)
	{
		free(ag.times);
		ag.times = NULL;
	}
}


void OleImage::OnTimer()
{
	KillTimer();

	// Move to next frame
	AnimatedGifDispodeFrame();

	int frame = ag.frame.num + 1;
	if (frame >= ag.frameCount)
	{
		// Don't need fi data no more
		AnimatedGifDeleteTmpValues();
		frame = 0;
	}

	ag.frame.num = frame;
	ag.started = FALSE;

	OnImageChange();
}


static VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);

	ImageTimerMapType::iterator it = timers.find(idEvent);
	if (it == timers.end())
		return;

	OleImage *oimg = it->second;
	if (oimg == NULL)
		return;

	oimg->OnTimer();
}


void OleImage::SetTimer(int time)
{
	KillTimer();
	ag.timer = ::SetTimer(0, 0, time, TimerProc);
	timers[ag.timer] = this;
}


void OleImage::KillTimer()
{
	if (ag.timer != NULL)
	{
		::KillTimer(NULL, ag.timer);
		timers.erase(ag.timer);
		ag.timer = NULL;
	}
}
