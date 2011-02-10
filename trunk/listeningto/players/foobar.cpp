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


#include "..\commons.h"
#include <comdef.h>

extern "C"
{
#include "foo_comserver2\COMServer2Helper_i.c"
#include "foo_comserver2\foobar2000_i.c"
}

static TCHAR *wcs[] = {
		_T("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}"),	// Foobar Columns UI
		_T("{97E27FAA-C0B3-4b8e-A693-ED7881E99FC1}"),	// Foobar 0.9.5.3 +
		_T("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}/1"),	// Foobar 0.9.1
		_T("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}"),
};

/////////////////////////////////////////////////////////////////////////////
// common

Foobar::Foobar(int index)
: Player(index)
{
	m_name					= _T("foobar2000");
	m_window_classes		= wcs;
	m_window_classes_num	= MAX_REGS(wcs);

	m_comApp				= NULL;
	m_comAppH				= NULL;
	m_comPlayback			= NULL;
	m_comPlaybackEventSink	= NULL;
	m_comClientHelper07Sink	= NULL;
	m_comRet				= NULL;
	m_vbServerState			= VARIANT_FALSE;
	m_state					= PL_OFFLINE;
}

Foobar::~Foobar()
{
	COM_Stop();
	FreeData();
}

void 
Foobar::EnableDisable()
{
	static BOOL old = FALSE;
	if(m_enabled == old) return;

	switch(old){				//stop ...
		case 0:						break;
		case 1:	COM_Stop();			break;		//COM
		case 2:	m_needPoll = FALSE;	break;		//MLT
	}
	switch(m_enabled){			//start ...
		case 0:	{
			if(loaded)
				NotifyInfoChanged();
		}		break;
		case 1:	{
			CLSID clsid;
			if(	(SUCCEEDED(CLSIDFromProgID(L"Foobar2000.ApplicationHelper.0.7", &clsid))) &&
				(COM_Start()) ) break;	//COM is full install  (host and server)
			else
			if(	(SUCCEEDED(CLSIDFromProgID(L"Foobar2000.Application0.7", &clsid))) ) {
				m_needPoll = COM_ConnectServer() ? FALSE : TRUE;
				break;					//COM is install  (only server)
			}
		}
		//fall through (no com)
		case 2:	{
			m_enabled  = 2;
			m_needPoll = FALSE;
			//MLT_Start();
			}	break;		//MLT
	}
	old = m_enabled;
}

/////////////////////////////////////////////////////////////////////////////
// COM ...foo_comserver2

BOOL 
Foobar::COM_Start()
{
	HRESULT hr;
	// ***** Start new Foobar COM Helper instance... *****
	hr = CoCreateInstance(	CLSID_ApplicationHelper07, 
							NULL, 
							CLSCTX_LOCAL_SERVER, 
							__uuidof(m_comAppH), 
							(void **)&m_comAppH );
	if(SUCCEEDED(hr)) {
		// ***** Instantiate an IMIM_fooEventH object. *****
		if(m_comClientHelper07Sink = new IMIM_fooEventH(*this, m_comAppH, &Foobar::COM_OnEventInvoke)) {
			//get the server state
			if(SUCCEEDED(m_comAppH->get_Running(&m_vbServerState))){
				DISPPARAMS	dispparams;
				ZeroMemory(&dispparams, sizeof dispparams);
				dispparams.cArgs = 1;
				VARIANTARG* pvarg = new VARIANTARG[dispparams.cArgs];
				if(pvarg == NULL)
					return FALSE;	 //TODO: error handeling
				dispparams.rgvarg = pvarg;
				dispparams.rgvarg[0].boolVal = m_vbServerState;

				COM_OnEventInvoke(m_comClientHelper07Sink, 1, IID_NULL,0,0,&dispparams,NULL,NULL,NULL);
				delete pvarg;
			}
		}
	}
	else {
		DEBUGOUT("Foobar:\t\tServer = ","off");
		CLSID clsid;
		if(S_OK != CLSIDFromProgID(L"Foobar2000.ApplicationHelper.0.7", &clsid)) {
			DEBUGOUT("Foobar:\t\tServer = ","helper not install !!");
			return FALSE;
		}
		DEBUGOUT("Foobar:\t\tServer = ","Reg ok, helper fail !!");

		return COM_ConnectServer();
	}
	return TRUE;
}

BOOL 
Foobar::COM_Stop()
{
	if (m_vbServerState == VARIANT_TRUE) {
		COM_ReleaseServer();
	}
	if (m_comClientHelper07Sink) {
		m_comClientHelper07Sink -> ShutdownConnectionPoint();
		m_comClientHelper07Sink -> Release();
		m_comClientHelper07Sink = NULL;
	}
	RELEASE(m_comAppH,TRUE);
	return TRUE;
}

BOOL 
Foobar::COM_ConnectServer()
{
	HRESULT hr;
	/***** Attach to the running COM instance... *****/
	IDispatch	*pDisp = NULL;
	if(m_comAppH) {
		//try Foobar COMServer2Helper (fast way)
		CALL( m_comAppH->get_Server(&pDisp));
		hr = pDisp->QueryInterface(__uuidof(m_comApp), (void**)&m_comApp);
		RELEASE(pDisp,TRUE);
	}
	else {
		//try GetActive Foobar Object (COMServer2Helper.exe not install right)
		CALL( ObjGet(CLSID_Application07, __uuidof(m_comApp), (void**)&m_comApp));
	}
	if(SUCCEEDED(hr)) {
		DEBUGOUT("Foobar:\t\tServer = ","on");
		// ***** get the Fooar Playback interface...    *****
		CALL( m_comApp->get_Playback(&m_comPlayback) );
		if (m_comPlayback == NULL)
			goto FAILURE;
		// ***** Instantiate an IMIM_fooEventS object. *****
		if(m_comPlaybackEventSink = new IMIM_fooEventS(*this, m_comPlayback, &Foobar::COM_OnEventInvoke)) {

			VARIANT_BOOL bState = VARIANT_FALSE;
			m_comPlayback->get_IsPlaying(&bState);

			m_state = (bState == VARIANT_TRUE) ? PL_PLAYING : PL_STOPPED;
			if(SetActivePlayer(m_index, m_index))
				NotifyInfoChanged();
		}
	}
	else {
		DEBUGOUT("Foobar:\t\tServer = ","off");
		RELEASE(pDisp,TRUE);
		m_state = PL_OFFLINE;
		return FALSE;
	}
	return TRUE;

FAILURE:
  #if defined(_DEBUG) || defined(DEBUG)
	_com_error err(hr);
	OutputDebugString(_T("Foobar:\t\tERROR = COM_ConnectServer()\n\t\t\t"));
	OutputDebugString(err.ErrorMessage());
	OutputDebugString(_T("\n)"));
  #endif
	RELEASE(pDisp,TRUE);
	COM_ReleaseServer();
	return FALSE;
}

void 
Foobar::COM_ReleaseServer()
{
	BOOL test = m_comApp ? OleIsRunning((LPOLEOBJECT)m_comApp) : FALSE;

	// Stop listening to events
	/* When the program is terminating, make sure that we instruct our */
	/* Event Handler to disconnect from the connection point of the */
	/* object which implemented the IEventFiringObject interface. */
	/* We also needs to Release() it (instead of deleting it). */
	if (m_comPlaybackEventSink) {
		m_comPlaybackEventSink -> ShutdownConnectionPoint();
		m_comPlaybackEventSink -> Release();
		m_comPlaybackEventSink = NULL;
	}
	RELEASE(m_comPlayback, m_vbServerState);
	RELEASE(m_comApp, m_vbServerState);
	BSTRFREE(m_comRet);

	m_state = PL_OFFLINE;
	if(loaded)
		NotifyInfoChanged();
}

BOOL 
Foobar::COM_infoCache()
{
	if(	(m_state <= PL_STOPPED) ||
		(m_comPlaybackEventSink && m_comPlaybackEventSink->m_vbServerState == VARIANT_FALSE) ||
		(!m_comPlayback) )
		return FALSE;

	struct info_t {
		TCHAR* &ptszValue;
		WCHAR* ptszFormat;
	} static info[] = {
	{m_listening_info.ptszAlbum,	L"[%album%]"		},
	{m_listening_info.ptszArtist,	L"[%artist%]"		},
	{m_listening_info.ptszGenre,	L"[%genre%]"		},
	{m_listening_info.ptszLength,	L"[%length%]"		},	//Length of the track, formatted as [HH:]MM:SS.
	{m_listening_info.ptszTitle,	L"[%title%]"		},
	{m_listening_info.ptszTrack,	L"[%tracknumber%]"	},
	{m_listening_info.ptszYear,		L"[$year(%date%)]"	}
	};

	HRESULT hr;
	VARIANT_BOOL IsPlaying = 0;			/* 0 == FALSE, -1 == TRUE */
	VARIANT_BOOL IsPaused  = 0;
	m_comPlayback->get_IsPlaying(&IsPlaying);
	m_comPlayback->get_IsPaused(&IsPaused);
	if(	IsPlaying == VARIANT_FALSE || 
		IsPlaying == VARIANT_TRUE && IsPaused == VARIANT_TRUE)
		return FALSE;

	m_listening_info.cbSize		= sizeof(m_listening_info);
	m_listening_info.ptszPlayer	= mir_tstrdup(m_name);
	m_listening_info.ptszType	= mir_tstrdup(_T("Music"));			//TODO:support foobar radio
	m_listening_info.dwFlags	= LTI_TCHAR;

	BSTR strResult = SysAllocString( L"");
	BSTR strFormat = SysAllocString( L"");

	for(int i = 0; i < SIZEOF(info); i++) {
		SysReAllocString(&strFormat,info[i].ptszFormat);
		if(S_OK!=(hr = m_comPlayback->FormatTitle(strFormat, &strResult)))
			break;
		info[i].ptszValue = mir_bstr2t(strResult);
	}
	BSTRFREE(strResult);
	BSTRFREE(strFormat);
	if(SUCCEEDED(hr)) {
		return TRUE;
	}
	FreeData();
	return FALSE;
}

BOOL 
Foobar::GetListeningInfo(LISTENINGTOINFO *lti)
{
	switch(m_enabled){
		case 1:		//COM
			FreeData();
			return COM_infoCache() ? Player::GetListeningInfo(lti) : FALSE;
		case 2:		//MLT
			if(m_needPoll && FindWindow() == NULL)
				return FALSE;
			return Player::GetListeningInfo(lti);
	}
	return FALSE;
}

// ***** COM_OnEventInvoke() is inoked by the TEventHandler based class object     *****
// ***** when an event is fired from the COM object that implements .FiringObject. *****
HRESULT 
Foobar::COM_OnEventInvoke(
				void*			pEventHandler,
				DISPID			dispIdMember,
				REFIID			riid,
				LCID			lcid,
				WORD			wFlags,
				DISPPARAMS FAR*	pDispParams,
				VARIANT FAR*	pVarResult,
				EXCEPINFO FAR*	pExcepInfo,
				UINT FAR*		puArgErr )
{
	if (!pDispParams) 
		return E_POINTER;
	if (pDispParams->cNamedArgs != 0)
		return DISP_E_NONAMEDARGS;

	HRESULT hr = S_OK;
	if (pEventHandler==m_comPlaybackEventSink) {
		switch (dispIdMember) {
			case 0x00000001:
			{//HRESULT Started([in] VARIANT_BOOL bPaused);
				DEBUGOUT("foo_Evt:\tStartet = ", pDispParams->rgvarg[0].boolVal !=0 ? "FALSE" : "TRUE");
				m_state = pDispParams->rgvarg[0].boolVal !=0 ? PL_STOPPED : PL_PLAYING;
				NotifyInfoChanged();
			}	break;
			case 0x00000002:
			{//HRESULT Stopped([in] fbStopReason lStopReason);
				switch (pDispParams->rgvarg[0].lVal) {
					case fbStopReasonUser:
						DEBUGOUT("foo_Evt:\tStopped = ","ReasonUser");
						m_state = PL_STOPPED;
						NotifyInfoChanged();
						break;
					case fbStopReasonEOF:
						DEBUGOUT("foo_Evt:\tStopped = ","ReasonEOF");
						m_state = PL_STOPPED;
						NotifyInfoChanged();
						break;
					case fbStopReasonStartingAnother:
						DEBUGOUT("foo_Evt:\tStopped = ","StartingAnother");
						m_state = PL_PLAYING;
						break;
					default:
						DEBUGOUT("foo_Evt:\tStopped = ","#NF");
						break;
				}
			}	break;
			case 0x00000003:
			{//HRESULT Paused([in] VARIANT_BOOL bPaused);
				DEBUGOUT("foo_Evt:\tPaused = ", pDispParams->rgvarg[0].boolVal !=0 ? "TRUE" : "FALSE");
				m_state = pDispParams->rgvarg[0].boolVal !=0 ? PL_PAUSED : PL_PLAYING;
				NotifyInfoChanged();
			}	break;
			case 0x00000004:
			{//HRESULT TrackChanged([in] VARIANT_BOOL bLocationChanged);
				DEBUGOUT("foo_Evt:\tTrackChanged = ", pDispParams->rgvarg[0].boolVal !=0 ? "TRUE" : "FALSE");
				m_state = PL_PLAYING;
				NotifyInfoChanged();
			}	break;
			case 0x00000005:
			{//HRESULT InfoChanged();
				DEBUGOUT("foo_Evt:\tInfoChanged = ","TRUE");
			}	break;
			case 0x00009999:
			{//RPCoff() - this is not a interface member (it is only a event from sink object if count m_cRef == 1);
				DEBUGOUT("foo_Evt:\t","ServerDisconect");
				//m_state = PL_OFFLINE;
			}	break;
		}
	}
	else if (pEventHandler==m_comClientHelper07Sink) {
		switch (dispIdMember) {
			case 0x00000001:
			{//HRESULT ServerStateChanged([in] VARIANT_BOOL bServerState);
				m_vbServerState = pDispParams->rgvarg[0].boolVal;
				if(m_comPlaybackEventSink)
					m_comPlaybackEventSink->m_vbServerState = pDispParams->rgvarg[0].boolVal;
				DEBUGOUT("foo_Evt:\tServerState = ", m_vbServerState !=0 ? "TRUE" : "FALSE");
				m_vbServerState !=0 ? COM_ConnectServer() : COM_ReleaseServer();
			}	break;
			case 0x00009999:
			{//RPCoff() - this is not a interface member (it is only a event from sink object if count m_cRef == 1);
				DEBUGOUT("foo_Evt:\t","HelperDisconect");
			}	break;
		}
	}

	return( hr );
}

/////////////////////////////////////////////////////////////////////////////
// MLT ...foo_mlt

HWND
Foobar::FindWindow()
{
	HWND hwnd = NULL;
	for(int i = 0; i < m_window_classes_num; i++)
	{
		hwnd = ::FindWindow(m_window_classes[i], NULL);
		if (hwnd != NULL)
			break;
	}
	if (hwnd != m_hwnd) {
		m_hwnd = hwnd;
	}
	return m_hwnd;
}


BYTE 
Foobar::GetStatus()
{
	if(!m_comApp && FindWindow() == 0)
		m_state = PL_OFFLINE;
	return m_state;
}

