/* 
Copyright (C) 2008 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#ifndef __OLEIMAGE_H__
# define __OLEIMAGE_H__

#include <windows.h>

// {2FD9449B-7EBB-476a-A9DD-AE61382CCE08}
static const GUID IID_IOleImage = { 0x2fd9449b, 0x7ebb, 0x476a, { 0xa9, 0xdd, 0xae, 0x61, 0x38, 0x2c, 0xce, 0x8 } };


class OleImage : public IOleObject, IViewObject
{
public:
	OleImage(HWND aParent, const TCHAR *aFilename, const TCHAR *aText);
	virtual ~OleImage();


	BOOL isValid() const;
	const TCHAR * GetText() const;
	void OnTimer();


	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(/* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

	// IOleObject
    virtual HRESULT STDMETHODCALLTYPE SetClientSite(/* [unique][in] */ IOleClientSite *pClientSite);
    virtual HRESULT STDMETHODCALLTYPE GetClientSite(/* [out] */ IOleClientSite **ppClientSite);
    virtual HRESULT STDMETHODCALLTYPE SetHostNames(/* [in] */ LPCOLESTR szContainerApp, /* [unique][in] */ LPCOLESTR szContainerObj);
    virtual HRESULT STDMETHODCALLTYPE Close(/* [in] */ DWORD dwSaveOption);
    virtual HRESULT STDMETHODCALLTYPE SetMoniker(/* [in] */ DWORD dwWhichMoniker, /* [unique][in] */ IMoniker *pmk);
    virtual HRESULT STDMETHODCALLTYPE GetMoniker(/* [in] */ DWORD dwAssign, /* [in] */ DWORD dwWhichMoniker, /* [out] */ IMoniker **ppmk);
    virtual HRESULT STDMETHODCALLTYPE InitFromData(/* [unique][in] */ IDataObject *pDataObject, /* [in] */ BOOL fCreation, /* [in] */ DWORD dwReserved);
    virtual HRESULT STDMETHODCALLTYPE GetClipboardData(/* [in] */ DWORD dwReserved, /* [out] */ IDataObject **ppDataObject);
    virtual HRESULT STDMETHODCALLTYPE DoVerb(/* [in] */ LONG iVerb, /* [unique][in] */ LPMSG lpmsg, /* [unique][in] */ IOleClientSite *pActiveSite, /* [in] */ LONG lindex, /* [in] */ HWND hwndParent, /* [unique][in] */ LPCRECT lprcPosRect);
    virtual HRESULT STDMETHODCALLTYPE EnumVerbs(/* [out] */ IEnumOLEVERB **ppEnumOleVerb);
    virtual HRESULT STDMETHODCALLTYPE Update(void);
    virtual HRESULT STDMETHODCALLTYPE IsUpToDate(void);
    virtual HRESULT STDMETHODCALLTYPE GetUserClassID(/* [out] */ CLSID *pClsid);
    virtual HRESULT STDMETHODCALLTYPE GetUserType(/* [in] */ DWORD dwFormOfType, /* [out] */ LPOLESTR *pszUserType);
    virtual HRESULT STDMETHODCALLTYPE SetExtent(/* [in] */ DWORD dwDrawAspect, /* [in] */ SIZEL *psizel);
    virtual HRESULT STDMETHODCALLTYPE GetExtent(/* [in] */ DWORD dwDrawAspect, /* [out] */ SIZEL *psizel);
    virtual HRESULT STDMETHODCALLTYPE Advise(/* [unique][in] */ IAdviseSink *pAdvSink, /* [out] */ DWORD *pdwConnection);
    virtual HRESULT STDMETHODCALLTYPE Unadvise(/* [in] */ DWORD dwConnection);
    virtual HRESULT STDMETHODCALLTYPE EnumAdvise(/* [out] */ IEnumSTATDATA **ppenumAdvise);
    virtual HRESULT STDMETHODCALLTYPE GetMiscStatus(/* [in] */ DWORD dwAspect, /* [out] */ DWORD *pdwStatus);
    virtual HRESULT STDMETHODCALLTYPE SetColorScheme(/* [in] */ LOGPALETTE *pLogpal);

	// IViewObject
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE Draw(/* [in] */ DWORD dwDrawAspect, /* [in] */ LONG lindex, /* [unique][in] */ void *pvAspect, /* [unique][in] */ DVTARGETDEVICE *ptd, /* [in] */ HDC hdcTargetDev, /* [in] */ HDC hdcDraw, /* [in] */ LPCRECTL lprcBounds, /* [unique][in] */ LPCRECTL lprcWBounds, /* [in] */ BOOL ( STDMETHODCALLTYPE *pfnContinue )(ULONG_PTR dwContinue), /* [in] */ ULONG_PTR dwContinue);
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetColorSet(/* [in] */ DWORD dwDrawAspect, /* [in] */ LONG lindex, /* [unique][in] */ void *pvAspect, /* [unique][in] */ DVTARGETDEVICE *ptd, /* [in] */ HDC hicTargetDev, /* [out] */ LOGPALETTE **ppColorSet);
	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Freeze(/* [in] */ DWORD dwDrawAspect, /* [in] */ LONG lindex, /* [unique][in] */ void *pvAspect, /* [out] */ DWORD *pdwFreeze);
	virtual HRESULT STDMETHODCALLTYPE Unfreeze(/* [in] */ DWORD dwFreeze);
	virtual HRESULT STDMETHODCALLTYPE SetAdvise(/* [in] */ DWORD aspects, /* [in] */ DWORD advf, /* [unique][in] */ IAdviseSink *pAdvSink);
	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetAdvise(/* [unique][out] */ DWORD *pAspects, /* [unique][out] */ DWORD *pAdvf, /* [out] */ IAdviseSink **ppAdvSink);

protected: 
	HWND hwndParent;
	char *filename;
	TCHAR *text;
	int width;
	int height;
	BOOL animated;

	LONG refCount;

	IOleAdviseHolder *oleAdviseHolder;
	IAdviseSink *viewAdviseSink;
	DWORD viewAdvf;
	SIZEL sizel;

	void SetTimer(int time);
	void KillTimer();
	void SendOnViewChage();

	BOOL LoadStaticImage();
	BOOL LoadAnimatedGif();
	BOOL AnimatedGifGetData();
	void AnimatedGifMountFrame();
	void AnimatedGifDispodeFrame();
	void AnimatedGifDeleteTmpValues();
	void DestroyAnimatedGif();

	struct 
	{
		HBITMAP hBmp;
		BOOL transparent;
	} si;

	struct 
	{
		UINT timer;
		HBITMAP *hbms;
		int *times;

		FIMULTIBITMAP *multi;
		FIBITMAP *dib;
		int frameCount;
		BOOL loop;
		RGBQUAD background;
		BOOL started;

		struct {
			int num;
			int top;
			int left;
			int width;
			int height;
			int disposal_method;
		} frame;
	} ag;
};



#endif // __OLEIMAGE_H__
