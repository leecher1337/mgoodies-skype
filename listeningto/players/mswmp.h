/* 
ListeningTo plugin for Miranda IM
==========================================================================
Copyright	(C) 2005-2011 Ricardo Pescuma Domenecci
			(C) 2010-2011 Merlin_de

PRE-CONDITION to use this code under the GNU General Public License:
 1. you do not build another Miranda IM plugin with the code without written permission
    of the autor (peace for the project).
 2. you do not publish copies of the code in other Miranda IM-related code repositories.
    This project is already hosted in a SVN and you are welcome to become a contributing member.
 3. you do not create listeningTo-derivatives based on this code for the Miranda IM project.
    (feel free to do this for another project e.g. foobar)
 4. you do not distribute any kind of self-compiled binary of this plugin (we want continuity
    for the plugin users, who should know that they use the original) you can compile this plugin
    for your own needs, friends, but not for a whole branch of people (e.g. miranda plugin pack).
 5. This isn't free beer. If your jurisdiction (country) does not accept
    GNU General Public License, as a whole, you have no rights to the software
    until you sign a private contract with its author. !!!
 6. you always put these notes and copyright notice at the beginning of your code.
==========================================================================

in case you accept the pre-condition,
this is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
