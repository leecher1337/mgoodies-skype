/* 
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

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

#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

//TODO: change to non-ATL code
#define _ATL_DLL
#define _ATL_APARTMENT_THREADED
#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>

extern "C"
{
#include ".\wmp\wmp.h"
#include ".\wmp\wmpids.h"
}

#include "TEventHandler.h"
using namespace TEventHandlerNamespace;

// ***** Make a forward declaration so that our TEventHandler template class can use it. *****
class WindowsMediaPlayer;

// ***** Declare an event handling class using the TEventHandler template. *****
typedef TEventHandler<WindowsMediaPlayer, IWMPPlayer4, _WMPOCXEvents> IMIM_wmpEventH;

class WindowsMediaPlayer : public Player
{
protected:		//only WLM
//	WCHAR	m_received[1024];
//	WCHAR	m_last_received[1024];

//	TCHAR **m_window_classes;
//	int		m_window_classes_num;
//	HWND	FindWindow();			//find Player Window

//	BOOL	WLM_Start();			//start
//	BOOL	WLM_Stop();				//stop

protected:
	ATOM	cWndclass;
	HWND	m_hwndclass;
	BOOL	m_bFullPlayer;

	void EnableDisable();

	//com (Remoting the Windows Media Player Control)
	class	CRemoteHost :
		public CComObjectRootEx<CComSingleThreadModel>,
		public IServiceProvider,
		public IWMPRemoteMediaServices
	{
	public:
		CRemoteHost();
		virtual ~CRemoteHost();

		//DECLARE_PROTECT_FINAL_CONSTRUCT()
		BEGIN_COM_MAP(CRemoteHost)
			COM_INTERFACE_ENTRY(IServiceProvider)
			COM_INTERFACE_ENTRY(IWMPRemoteMediaServices)
		END_COM_MAP()

		// IServiceProvider
		STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void ** ppv);
		// IWMPRemoteMediaServices
		STDMETHOD(GetServiceType)(BSTR * pbstrType);
		STDMETHOD(GetApplicationName)(BSTR * pbstrName);
		STDMETHOD(GetScriptableObject)(BSTR * pbstrName, IDispatch ** ppDispatch);
		STDMETHOD(GetCustomUIMode)(BSTR * pbstrFile);
	};

	CComModule				_Module;
	/* ***** Declare an instance of a COMApplication smart pointer. ***** */
	IWMPPlayer4				*m_comAppH;		//Host (include IWMPCore and IWMPPlayer1-4)
	CAxWindow				*m_pView;		//ActiveX control container
	/* ***** interface provides methods for switching between a remoted
	   ***** Windows Media Player control and the full mode of the Player. ***** */
	IWMPPlayerApplication	*m_comApp;
	/* ***** Declare a pointer to a TEventHandler class which is specially tailored
	   ***** to receiving events from the _WMPOCXEvents events of an
	   ***** IWMPPlayer4 object (It is designed to be a sink object) *****/
	IMIM_wmpEventH*			m_comAppEventSink;

	BSTR					m_comRet;				//global BSTR for free use

	BOOL	COM_Start();							//start the Host instance
	BOOL	COM_Stop();								//stop  the Host instance, disconect and free all
	BOOL	COM_ConnectServer();					//start the COM instance
	void	COM_ReleaseServer();
	BOOL	COM_infoCache();
	//helper
	BOOL	COM_IsPlayerDocked();
	void	COM_PlayState(long NewState);
	BOOL	CreateWnd();

	/* ***** common function that handle events fired from the COM object. *****/
	HRESULT	COM_OnEventInvoke
	(
		void*			pEventHandler,			//client sync  Interface (m_comAppEventSink)
		DISPID			dispIdMember,			//server event Interface member
		REFIID			riid,					//always NULL (see MSDN)
		LCID			lcid,
		WORD			wFlags,
		DISPPARAMS FAR*	pDispParams,
		VARIANT FAR*	pVarResult,
		EXCEPINFO FAR*	pExcepInfo,
		UINT FAR*		puArgErr
	);

	virtual ~WindowsMediaPlayer();

public:
	//common
	WindowsMediaPlayer(int index);

	//COM ...Remoting the Windows Media Player Control
	BOOL GetListeningInfo(LISTENINGTOINFO *lti);

/*	//WLM ...Windows Live Messanger
	BYTE GetStatus();
	void WLM_ProcessReceived();
	void WLM_NewData(const WCHAR *data, size_t len);
 */
};
