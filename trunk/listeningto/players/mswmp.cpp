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


#include "..\commons.h"

extern "C"
{
#include ".\wmp\wmp.h"
#include ".\wmp\wmpids.h"
}

/////////////////////////////////////////////////////////////////////////////
// WLM Windows Live Messanger  forward declaration

/*
#define WMP_WINDOWCLASS _T("MsnMsgrUIManager")

static LRESULT CALLBACK WLM_ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static VOID    CALLBACK WLM_SendTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

static UINT_PTR hSendTimer = NULL;
static WindowsMediaPlayer *singleton = NULL;
*/
static TCHAR *wcs[] = {
		_T("WMPlayerApp"),		//WMP latest
		_T("WMP Skin Host"),	//WMP latest (Skin Host window)
		_T("Media Player 2")	//WMP old
};

//static LRESULT CALLBACK WMP_OCXreceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
//static WindowsMediaPlayer *singleton = NULL;

/////////////////////////////////////////////////////////////////////////////
// common

WindowsMediaPlayer::WindowsMediaPlayer(int index)
: Player(index)
{
	m_name					= _T("WindowsMediaPlayer");
//	m_window_classes		= wcs;
//	m_window_classes_num	= MAX_REGS(wcs);

	m_comAppH				= NULL;
	m_comApp				= NULL;
	m_comAppEventSink		= NULL;
	m_comRet				= NULL;
//	m_vbServerState			= VARIANT_FALSE;
	m_state					= PL_OFFLINE;
	m_pView					= NULL;
	m_bFullPlayer			= FALSE;
//	_Module.Init(NULL, hInst, &LIBID_ATLLib);
}

WindowsMediaPlayer::~WindowsMediaPlayer()
{
	switch(m_enabled){
		case 1:		//COM
			COM_Stop();
		//	_Module.Term();
			break;
/*		case 2:		//WLM
			WLM_Stop();
			break;   */
	}
	FreeData();
}

void 
WindowsMediaPlayer::EnableDisable()
{
	static BOOL old = FALSE;
	if(m_enabled == old) return;

	switch(old){				//stop ...
		case 0:					break;
		case 1:	COM_Stop();		break;		//COM
//		case 2:	WLM_Stop();		break;		//WLM
	}
	switch(m_enabled){			//start ...
		case 0:	{
			if(loaded)
				NotifyInfoChanged();
		}		break;
		case 1:	COM_Start();	break;		//COM
//		case 2:	WLM_Start();	break;		//WLM
	}
	old = m_enabled;
}

BOOL
WindowsMediaPlayer::CreateWnd()
{
	// Create windows class
	WNDCLASS wc				= {0};
	wc.lpfnWndProc			= DefWindowProc /*WMP_OCXreceiverWndProc TestWndProc*/;
	wc.hInstance			= hInst;
	wc.lpszClassName		= _T("MIMlisteningToWMP");

	cWndclass = RegisterClass(&wc);
	DWORD err = GetLastError();

	if (!cWndclass) {
		TCHAR msg[1024];
		wsprintf(msg, TranslateT("Failed to register %s class."),wc.lpszClassName);
		MessageBox(NULL, msg, _T(MODULE_NAME), MB_ICONSTOP|MB_OK);

//		singleton = NULL;
		return FALSE;
	}

	// Create a window.
/*	if (WLM) {
		m_received[0]			= L'\0';
		m_last_received[0]		= L'\0';
		singleton				= this;
		m_hwndclass = CreateWindow(WMP_WINDOWCLASS, _T("Miranda ListeningTo WMP receiver"), 
									0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
	}
	else {   */
		m_hwndclass = CreateWindow(_T("MIMlisteningToWMP"), _T("Miranda ListeningTo WMP OCX receiver"),
									WS_OVERLAPPEDWINDOW | WS_DISABLED, CW_USEDEFAULT, CW_USEDEFAULT,
									CW_USEDEFAULT, CW_USEDEFAULT, NULL/*HWND_MESSAGE*/, NULL, hInst, NULL);
//		singleton = m_hwndclass ? this : NULL;  
//	}

	return m_hwndclass == NULL ? FALSE : TRUE;
}

BOOL 
WindowsMediaPlayer::GetListeningInfo(LISTENINGTOINFO *lti)
{
	switch(m_enabled){
		case 1:		//COM
			FreeData();
			return COM_infoCache() ? Player::GetListeningInfo(lti) : FALSE;
/*		case 2:		//WLM
			if(m_needPoll && FindWindow() == NULL)
				return FALSE;
			return Player::GetListeningInfo(lti); */
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// COM ...Remoting the Windows Media Player Control

BOOL 
WindowsMediaPlayer::COM_Start()
{
	m_pView		= 0;
	m_hwndclass	= 0;
	HRESULT hr	= S_OK;

	hr = _Module.Init(NULL, hInst, &LIBID_ATLLib);
	if(FAILED(hr))
		return FALSE;

	if(!CreateWnd()) {
		_Module.Term();
		return FALSE;
	}

	IObjectWithSite				*spHostObject	= NULL;
	IAxWinHostWindow			*spHost			= NULL;
	CComObject<CRemoteHost>		*pRemoteHost	= NULL;

	// Create an ActiveX control container
	m_pView = new CAxWindow();
	if(!m_pView) hr = E_OUTOFMEMORY;

	if(SUCCEEDED(hr)){
		m_pView->Create(m_hwndclass, NULL, NULL, WS_CHILD | WS_DISABLED);
		if(::IsWindow(m_pView->m_hWnd)) {
			hr = m_pView->QueryHost(__uuidof(spHostObject), (void **)&spHostObject);
			if(!spHostObject) {
				hr = E_POINTER;
			}
		}
	}

	// Create remote host which implements IServiceProvider and IWMPRemoteMediaServices
	if(SUCCEEDED(hr)) {
		hr = CComObject<CRemoteHost>::CreateInstance(&pRemoteHost);
		if(pRemoteHost) {
			pRemoteHost->AddRef();
		}
		else {
			hr = E_POINTER;
		}
	}

	// Set site to the remote host
	if(SUCCEEDED(hr)) {
		hr = spHostObject->SetSite((IWMPRemoteMediaServices *)pRemoteHost);
	}
	if(SUCCEEDED(hr)) {
		hr = m_pView->QueryHost(__uuidof(spHost), (void **)&spHost);
		if(!spHost) hr = E_NOINTERFACE;
	}

	// Create WMP Control here
	if(SUCCEEDED(hr)) {
		hr = spHost->CreateControl(CComBSTR(L"{6BF52A52-394A-11d3-B153-00C04F79FAA6}"), m_pView->m_hWnd, NULL);
	}
	// get a IWMPPlayer4 object
	if(SUCCEEDED(hr)) {
		hr = m_pView->QueryControl(&m_comAppH);
		if(!m_comAppH) hr = E_NOINTERFACE;
	}

	// Release object
	RELEASE(spHost,			TRUE);
	RELEASE(spHostObject,	TRUE);
	RELEASE(pRemoteHost,	TRUE);

	// get a IWMPPlayerApplication object
	if(SUCCEEDED(hr)) {
		hr = m_comAppH->get_playerApplication(&m_comApp);
	}
	// ***** Instantiate an IMIM_wmpEventH object. *****
	if(SUCCEEDED(hr)) {
		m_comAppEventSink = new IMIM_wmpEventH(*this, m_comAppH, &WindowsMediaPlayer::COM_OnEventInvoke);
		if(!m_comAppEventSink) hr = E_OUTOFMEMORY;
	}

	//check if wmp is docked / play
	if(SUCCEEDED(hr) && COM_IsPlayerDocked()) {
		COM_ConnectServer();
	}

	return TRUE;
}

BOOL 
WindowsMediaPlayer::COM_Stop()
{
	// Stop listening to events
	/* When the program is terminating, make sure that we instruct our */
	/* Event Handler to disconnect from the connection point of the */
	/* object which implemented the IEventFiringObject interface. */
	/* We also needs to Release() it (instead of deleting it). */
	if (m_comAppEventSink) {
		m_comAppEventSink -> ShutdownConnectionPoint();
		m_comAppEventSink -> Release();
		m_comAppEventSink = NULL;
	}

	m_state = PL_OFFLINE;
	DEBUGOUT("wmp_Player:\tServer = ","off");

//	if(m_state)
//		COM_ReleaseServer();

	RELEASE(m_comApp, TRUE);
	RELEASE(m_comAppH,TRUE);
	if (m_pView)
		m_pView->DestroyWindow();
	delete m_pView;
	BSTRFREE(m_comRet);

	if(DestroyWindow(m_hwndclass))
		m_hwndclass = NULL;
	if(UnregisterClass (MAKEINTATOM(cWndclass),hInst))
		cWndclass = 0;

	_Module.Term();

	return TRUE;
}

BOOL 
WindowsMediaPlayer::COM_ConnectServer()
{
	DEBUGOUT("wmp_Player:\tServer = ","on");
	if(SetActivePlayer(m_index, m_index))
		COM_PlayState(99);		//this start timer depend on status

	return TRUE;
}

void 
WindowsMediaPlayer::COM_ReleaseServer()
{
	DEBUGOUT("wmp_Player:\tServer = ","off");
	m_state = PL_OFFLINE;
	if(loaded)
		NotifyInfoChanged();
}

BOOL 
WindowsMediaPlayer::COM_infoCache()
{
	if(	(!m_comAppH) ||
		(m_comAppEventSink && m_comAppEventSink->m_vbServerState == VARIANT_FALSE) ||
		(m_state <= PL_STOPPED) ||
		(m_state == PL_PAUSED) )
		return FALSE;

	HRESULT		hr;
	//get current media object
	IWMPMedia	*spMedia;
	if(FAILED(m_comAppH->get_currentMedia(&spMedia)))
		return FALSE;

	struct info_t {
		TCHAR* &ptszValue;
		WCHAR* ptszFormat;
		WCHAR* ptszAlias;
	} static info[] = {
	{m_listening_info.ptszAlbum,	L"Album",			L"WM/AlbumTitle"},
	{m_listening_info.ptszArtist,	L"Artist",			L"AlbumArtist"	},	//WM/AlbumArtist
	{m_listening_info.ptszGenre,	L"Genre",			L"WM/Genre"		},
	{m_listening_info.ptszTitle,	L"Name",			L"Title"		},
	{m_listening_info.ptszTrack,	L"WM/TrackNumber",	L"WM/PartOfSet"	},
	{m_listening_info.ptszYear,		L"WM/Year",			NULL			}
	};

	m_listening_info.cbSize		= sizeof(m_listening_info);
	m_listening_info.ptszPlayer	= mir_tstrdup(m_name);
	m_listening_info.dwFlags	= LTI_TCHAR;

	BSTR strResult = SysAllocString( L"");
	BSTR strFormat = SysAllocString( L"MediaType");

	hr = spMedia->getItemInfo(strFormat, &strResult);
	switch (strResult[0]) {
		case 'a':
		case 'A':
			m_listening_info.ptszType	= mir_tstrdup(_T("Music"));
			break;
		case 'v':
		case 'V':
			m_listening_info.ptszType	= mir_tstrdup(_T("Video"));
			break;
		case 'r':
		case 'R':
			m_listening_info.ptszType	= mir_tstrdup(_T("Radio"));
			break;
		default:
			m_listening_info.ptszType	= mir_tstrdup(_T("Other"));
	}

	for(int i = 0; i < SIZEOF(info); i++) {
		//try main (ptszFormat)
		SysReAllocString(&strFormat,info[i].ptszFormat);
		if(FAILED(hr = spMedia->getItemInfo(strFormat, &strResult)))
			break;
		if(SysStringLen(strResult)>0) {
			info[i].ptszValue = mir_bstr2t(strResult);
			continue;
		}
		//try alias (ptszAlias)
		SysReAllocString(&strFormat,info[i].ptszAlias);
		if(FAILED(hr = spMedia->getItemInfo(strFormat, &strResult)))
			break;
		info[i].ptszValue = mir_bstr2t(strResult);
	}

	if(SUCCEEDED(hr)) {
		//Length of the track, formatted as [HH:]MM:SS.
		if(SUCCEEDED(hr = spMedia->get_durationString(&strResult)))
			m_listening_info.ptszLength = mir_bstr2t(strResult);
	}

	RELEASE(spMedia, TRUE)
	BSTRFREE(strResult);
	BSTRFREE(strFormat);
	if(SUCCEEDED(hr)) {
		return TRUE;
	}
	FreeData();
	return FALSE;
}

BOOL 
WindowsMediaPlayer::COM_IsPlayerDocked()
{
	HRESULT hr			= E_POINTER;
	VARIANT_BOOL bstate	= FALSE;

	hr = m_comApp->get_playerDocked(&bstate);
	if (SUCCEEDED(hr) && bstate == VARIANT_FALSE) {
		m_bFullPlayer = TRUE;
		DEBUGOUT("wmp_Player:\tDocked = ","on");
		return TRUE;		//Player is online (undocket = Full Player)
	}
	else					//Player change from undocket to docked 
	if(m_comAppH /*&& m_bFullPlayer*/){
//		m_bFullPlayer = FALSE;
		IWMPControls *spControls = NULL;
		hr = m_comAppH->get_controls(&spControls);
		//This method causes Windows Media Player to release any system resources it is using,
		//such as the audio device. The current media file, however, is not released.
		if(SUCCEEDED(hr) && spControls) spControls->stop();
		//This method closes the current digital media file, not the Player itself.
		hr = m_comAppH->close();
		RELEASE(spControls, TRUE);
	}
	DEBUGOUT("wmp_Player:\tDocked = ","off");
	m_bFullPlayer = FALSE;
	return FALSE;			//Player is offline or docked to a other Host
}

void 
WindowsMediaPlayer::COM_PlayState(long NewState)
{
	HRESULT hr = S_OK;
	WMPPlayState pwmpps;
	if(NewState > (long)wmppsLast)
		hr = m_comAppH->get_playState(&pwmpps);
	else
		pwmpps = (WMPPlayState)NewState;

	if(SUCCEEDED(hr)) {
		switch(pwmpps){
			case wmppsUndefined:		/*= 0*/
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","Undefined");
				m_state = PL_STOPPED;	//PL_OFFLINE;
				NotifyInfoChanged();
				break;
			case wmppsStopped:			/*= 1*/
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","Stopped");
				m_state = PL_STOPPED;
				NotifyInfoChanged();
				break;
			case wmppsPaused:			/*= 2*/
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","Paused");
				m_state = PL_PAUSED;
				NotifyInfoChanged();
				break;
			case wmppsPlaying:			/*= 3*/
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","Playing");
				m_state = PL_PLAYING;
				NotifyInfoChanged();
				break;
			case wmppsScanForward:		/*= 4*/
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","ScanForward");
				m_state = PL_FORWARD;
				NotifyInfoChanged();
				break;
			case wmppsScanReverse:		/*= 5*/
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","ScanReverse");
				m_state = PL_REWIND;
				NotifyInfoChanged();
				break;
			case wmppsBuffering:		/*= 6*/
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","Buffering");
				break;
			case wmppsWaiting:			/*= 7*/
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","Waiting");
				break;
			case wmppsMediaEnded:		/*= 8*/
//				do not use this event !!
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","MediaEnded");
				break;
			case wmppsTransitioning:	/*= 9*/
//				do not use this event !!
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","Transitioning");
				break;
			case wmppsReady:			/*= 10*/
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","Ready");
				m_state = PL_STOPPED;
				NotifyInfoChanged();
				break;
			case wmppsReconnecting:		/*= 11*/
//				do not use this event !!
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","Reconnecting");
				break;
			case wmppsLast:				/*= 12*/
//				do not use this event !!
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","Last");
				break;
			default:
				DEBUGOUT("wmp_Evt:\tPlayStateChange = ","#NV");
				break;
		}//end switch(pwmpps)
	}
}

// ***** COM_OnEventInvoke() is inoked by the TEventHandler based class object *****
// ***** when an event is fired from the COM object that implements .FiringObject.     *****
HRESULT 
WindowsMediaPlayer::COM_OnEventInvoke(
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
	//TODO: check if all events work
	if (pEventHandler==m_comAppEventSink) {
		switch (dispIdMember) {
			case DISPID_WMPCOREEVENT_PLAYSTATECHANGE:
			{/*	[id(0x000013ed), helpstring("Sent when the control changes PlayState")]
				void PlayStateChange([in] long NewState); */
				COM_PlayState(pDispParams->rgvarg[0].lVal);
			}	break;
			case DISPID_WMPCOREEVENT_OPENSTATECHANGE:
			{/*	[id(0x00001389), helpstring("Sent when the control changes OpenState")]
				void OpenStateChange([in] long NewState); */
			   #ifdef DEBUG
 				char* temp;
				switch(pDispParams->rgvarg[0].lVal){
					case wmposUndefined:
						temp="Undefined";			break;
					case wmposPlaylistChanging:
						temp="PlaylistChanging";	break;
					case wmposPlaylistLocating:
						temp="PlaylistLocating";	break;
					case wmposPlaylistConnecting:
						temp="PlaylistConnecting";	break;
					case wmposPlaylistLoading:
						temp="PlaylistLoading";		break;
					case wmposPlaylistOpening:
						temp="PlaylistOpening";		break;
					case wmposPlaylistOpenNoMedia:
						temp="PlaylistOpenNoMedia";	break;
					case wmposPlaylistChanged:
						temp="PlaylistChanged";		break;

					case wmposMediaChanging:
						temp="MediaChanging";		break;
					case wmposMediaLocating:
						temp="MediaLocating";		break;
					case wmposMediaConnecting:
						temp="MediaConnecting";		break;
					case wmposMediaLoading:
						temp="MediaLoading";		break;
					case wmposMediaOpening:
						temp="MediaOpening";		break;
					case wmposMediaOpen:
						temp="MediaOpen";			break;
					
					case wmposBeginCodecAcquisition:
						temp="BeginCodecAcquisition";	break;
					case wmposEndCodecAcquisition:
						temp="EndCodecAcquisition";		break;
					case wmposBeginLicenseAcquisition:
						temp="BeginLicenseAcquisition";	break;
					case wmposEndLicenseAcquisition:
						temp="EndLicenseAcquisition";	break;
					case wmposBeginIndividualization:
						temp="BeginIndividualization";	break;
					case wmposEndIndividualization:
						temp="EndIndividualization";	break;
					case wmposMediaWaiting:
						temp="MediaWaiting";			break;
					case wmposOpeningUnknownURL:
						temp="OpeningUnknownURL";		break;
				}//end switch(WMPOpenState)
				DEBUGOUT("wmp_Evt:\tOpenStateChange = ", temp);
			   #endif
			//ugly workaround for some wmp version on multimedia key press (if not docked)
			//TODO: find better way to disable multimedia key press 
			//COM_IsPlayerDocked() stop play immediately
				if(!m_bFullPlayer) COM_IsPlayerDocked();
			}	break;

		// DIID_WMPOCXEvents (These are the events that will be fired from OCX itself)
			case DISPID_WMPOCXEVENT_SWITCHEDTOPLAYERAPPLICATION:
			{/*	[id(0x00001965), helpstring("Sent when display switches to player application")]
				void SwitchedToPlayerApplication(); */
				DEBUGOUT("wmp_Evt:\tSwitchedToPlayerApplication","");
			}	break;
			case DISPID_WMPOCXEVENT_SWITCHEDTOCONTROL:
			{/*	[id(0x00001966), helpstring("Sent when display switches to control")]
				void SwitchedToControl(); */
				DEBUGOUT("wmp_Evt:\tSwitchedToControl","");
			}	break;
			case DISPID_WMPOCXEVENT_PLAYERDOCKEDSTATECHANGE:
			{/*	[id(0x00001967), helpstring("Sent when the player docks or undocks")]
				void PlayerDockedStateChange(); */
				DEBUGOUT("wmp_Evt:\tPlayerDockedStateChange","");
				COM_IsPlayerDocked() ? COM_ConnectServer() : COM_ReleaseServer();
			}	break;
			case DISPID_WMPOCXEVENT_PLAYERRECONNECT:
			{/*	[id(0x00001968), helpstring("Sent when the OCX reconnects to the player")]
				void PlayerReconnect(); */
				DEBUGOUT("wmp_Evt:\tPlayerReconnect","");
			}	break;

		// DIID_WMPCoreEvents
			//case DISPID_WMPCOREEVENT_STATUSCHANGE:
			//{/*	[id(0x0000138a), helpstring("Sent when the status string changes")]
			//	void StatusChange(); */
			//	m_comAppH->get_status(&m_comRet);
			//	OutputDebugStringW(L"wmp_Evt:\tStatusChange \n\t");
			//	OutputDebugStringW(m_comRet);
			//	OutputDebugStringW(L"\n");
			//}	break;

		// DIID_WMPCoreEvents CONTROL
			//case DISPID_WMPCOREEVENT_AUDIOLANGUAGECHANGE:
			//{/*	[id(0x000013ee), helpstring("Sent when the current audio language has changed")]
			//	void AudioLanguageChange([in] long LangID); */
			//	DEBUGOUT("wmp_Evt:\tAudioLanguageChange","");
			//}	break;

		// DIID_WMPCoreEvents SEEK
			case DISPID_WMPCOREEVENT_ENDOFSTREAM:
			{/*	[id(0x00001451), helpstring("Sent when the end of file is reached")]
				void EndOfStream([in] long Result); */
				DEBUGOUT("wmp_Evt:\tEndOfStream","");
			}	break;
			case DISPID_WMPCOREEVENT_POSITIONCHANGE:
			{/*	[id(0x00001452), helpstring("Indicates that the current position of the movie has changed")]
				void PositionChange( [in] double oldPosition, [in] double newPosition); */
				DEBUGOUT("wmp_Evt:\tPositionChange","");
			}	break;
			case DISPID_WMPCOREEVENT_MARKERHIT:
			{/*	[id(0x00001453), helpstring("Sent when a marker is reached")]
				void MarkerHit([in] long MarkerNum); */
				DEBUGOUT("wmp_Evt:\tMarkerHit","");
			}	break;
			case DISPID_WMPCOREEVENT_DURATIONUNITCHANGE:
			{/*	[id(0x00001454), helpstring("Indicates that the unit used to express duration and position has changed")]
				void DurationUnitChange([in] long NewDurationUnit); */
				DEBUGOUT("wmp_Evt:\tDurationUnitChange","");
			}	break;

		// DIID_WMPCoreEvents CONTENT
			//case DISPID_WMPCOREEVENT_SCRIPTCOMMAND:
			//{/*	[id(0x000014b5), helpstring("Sent when a synchronized command or URL is received")]
			//	void ScriptCommand([in] BSTR scType, [in] BSTR Param); */
			//	DEBUGOUT("wmp_Evt:\tScriptCommand","");
			//}	break;

		// DIID_WMPCoreEvents NETWORK
			//case DISPID_WMPCOREEVENT_DISCONNECT:
			//{/*	[id(0x00001519), helpstring("Sent when the control is disconnected from the server")]
			//	void Disconnect([in] long Result); */
			//	DEBUGOUT("wmp_Evt:\tDisconnect","");
			//}	break;
			case DISPID_WMPCOREEVENT_BUFFERING:
			{/*	[id(0x0000151a), helpstring("Sent when the control begins or ends buffering")]
				void Buffering([in] VARIANT_BOOL Start); */
				DEBUGOUT("wmp_Evt:\tBuffering","");
			}	break;
			case DISPID_WMPCOREEVENT_NEWSTREAM:
			{/*	[id(0x0000151b), helpstring("Sent when a new stream is started in a channel")]
				void NewStream(); */
				DEBUGOUT("wmp_Evt:\tNewStream","");
			}	break;

			// DIID_WMPCoreEvents ERROR
			case DISPID_WMPCOREEVENT_ERROR:
			{/*	[id(0x0000157d), helpstring("Sent when the control has an error condition")]
				void Error(); */
				DEBUGOUT("wmp_Evt:\tError","");
			}	break;

		// DIID_WMPCoreEvents WARNING
			case DISPID_WMPCOREEVENT_WARNING:
			{/*	[id(0x000015e1), helpstring("Sent when the control encounters a problem")]
				void Warning([in] long WarningType, [in] long Param, [in] BSTR Description); */
				DEBUGOUT("wmp_Evt:\tWarning","");
			}	break;

		// DIID_WMPCoreEvents CDROM
			case DISPID_WMPCOREEVENT_CDROMMEDIACHANGE:
			{/*	[id(0x00001645), helpstring("Indicates that the CD ROM media has changed")]
				void CdromMediaChange([in] long CdromNum); */
				DEBUGOUT("wmp_Evt:\tCdromMediaChange","");
			}	break;

		// DIID_WMPCoreEvents PLAYLIST
			//case DISPID_WMPCOREEVENT_PLAYLISTCHANGE:
			//{/*	[id(0x000016a9), helpstring("Sent when a playlist changes")]
			//	void PlaylistChange([in] IDispatch* Playlist, [in] WMPPlaylistChangeEventType change); */
			//	char temp[5];
			//	_i64toa(pDispParams->rgvarg[0].lVal, temp, 10);
			//	DEBUGOUT("wmp_Evt:\tPlaylistChange = ",temp);
			//}	break;
			//case DISPID_WMPCOREEVENT_MEDIACHANGE:
			//{/*	[id(0x000016aa), helpstring("Sent when a media object changes")]
			//	void MediaChange([in] IDispatch* Item); */
			//	DEBUGOUT("wmp_Evt:\tMediaChange","");
			//}	break;
			case DISPID_WMPCOREEVENT_CURRENTMEDIAITEMAVAILABLE:
			{/*	[id(0x000016ab), helpstring("Sent when a current media item becomes available")]
				void CurrentMediaItemAvailable([in] BSTR bstrItemName); */
				DEBUGOUT("wmp_Evt:\tCurrentMediaItemAvailable","");
			}	break;
			case DISPID_WMPCOREEVENT_CURRENTPLAYLISTCHANGE:
			{/*	[id(0x000016ac), helpstring("Sent when the current playlist changes")]
				void CurrentPlaylistChange([in] WMPPlaylistChangeEventType change); */
				DEBUGOUT("wmp_Evt:\tCurrentPlaylistChange","");
			}	break;
			case DISPID_WMPCOREEVENT_CURRENTPLAYLISTITEMAVAILABLE:
			{/*	[id(0x000016ad), helpstring("Sent when a current playlist item becomes available")]
				void CurrentPlaylistItemAvailable([in] BSTR bstrItemName); */
				DEBUGOUT("wmp_Evt:\tCurrentPlaylistItemAvailable","");
			}	break;
			//case DISPID_WMPCOREEVENT_CURRENTITEMCHANGE:
			//{/*	[id(0x000016ae), helpstring("Sent when the item selection on the current playlist changes")]
			//	void CurrentItemChange([in] IDispatch* pdispMedia); */
			//	DEBUGOUT("wmp_Evt:\tCurrentItemChange","");
			//}	break;
			case DISPID_WMPCOREEVENT_MEDIACOLLECTIONCHANGE:
			{/*	[id(0x000016af), helpstring("Sent when the media collection needs to be requeried")]
				void MediaCollectionChange(); */
				DEBUGOUT("wmp_Evt:\tMediaCollectionChange","");
			}	break;
			case DISPID_WMPCOREEVENT_MEDIACOLLECTIONATTRIBUTESTRINGADDED:
			{/*	[id(0x000016b0), helpstring("Sent when an attribute string is added in the media collection")]
				void MediaCollectionAttributeStringAdded([in] BSTR bstrAttribName, [in] BSTR bstrAttribVal); */
				DEBUGOUT("wmp_Evt:\tMediaCollectionAttributeStringAdded","");
			}	break;
			case DISPID_WMPCOREEVENT_MEDIACOLLECTIONATTRIBUTESTRINGREMOVED:
			{/*	[id(0x000016b1), helpstring("Sent when an attribute string is removed from the media collection")]
				void MediaCollectionAttributeStringRemoved( [in] BSTR bstrAttribName, [in] BSTR bstrAttribVal); */
				DEBUGOUT("wmp_Evt:\tMediaCollectionAttributeStringRemoved","");
			}	break;
			case DISPID_WMPCOREEVENT_PLAYLISTCOLLECTIONCHANGE:
			{/*	[id(0x000016b2), helpstring("Sent when playlist collection needs to be requeried")]
				void PlaylistCollectionChange(); */
				DEBUGOUT("wmp_Evt:\tPlaylistCollectionChange","");
			}	break;
			case DISPID_WMPCOREEVENT_PLAYLISTCOLLECTIONPLAYLISTADDED:
			{/*	[id(0x000016b3), helpstring("Sent when a playlist is added to the playlist collection")]
				void PlaylistCollectionPlaylistAdded([in] BSTR bstrPlaylistName); */
				DEBUGOUT("wmp_Evt:\tPlaylistCollectionPlaylistAdded","");
			}	break;
			case DISPID_WMPCOREEVENT_PLAYLISTCOLLECTIONPLAYLISTREMOVED:
			{/*	[id(0x000016b4), helpstring("Sent when a playlist is removed from the playlist collection")]
				void PlaylistCollectionPlaylistRemoved([in] BSTR bstrPlaylistName); */
				DEBUGOUT("wmp_Evt:\tPlaylistCollectionPlaylistRemoved","");
			}	break;
			case DISPID_WMPCOREEVENT_MEDIACOLLECTIONCONTENTSCANADDEDITEM:
			case DISPID_WMPCOREEVENT_MEDIACOLLECTIONCONTENTSCANPROGRESS:
			case DISPID_WMPCOREEVENT_MEDIACOLLECTIONSEARCHFOUNDITEM:
			case DISPID_WMPCOREEVENT_MEDIACOLLECTIONSEARCHPROGRESS:
			case DISPID_WMPCOREEVENT_MEDIACOLLECTIONSEARCHCOMPLETE:
			{/*	not found in lib, but present in wmpids.h*/
				DEBUGOUT("wmp_Evt:\tMediaCollection... ","check this event !!");
			}	break;
			case DISPID_WMPCOREEVENT_PLAYLISTCOLLECTIONPLAYLISTSETASDELETED:
			{/*	[id(0x000016ba), helpstring("Sent when a playlist has been set or reset as deleted")]
				void PlaylistCollectionPlaylistSetAsDeleted([in] BSTR bstrPlaylistName, [in] VARIANT_BOOL varfIsDeleted); */
				DEBUGOUT("wmp_Evt:\tPlaylistCollectionPlaylistSetAsDeleted","");
			}	break;
			//case DISPID_WMPCOREEVENT_MODECHANGE:
			//{/*	[id(0x000016bb), helpstring("Playlist playback mode has changed")]
			//	void ModeChange([in] BSTR ModeName, [in] VARIANT_BOOL NewValue); */
			//	DEBUGOUT("wmp_Evt:\tModeChange","");
			//}	break;
			case DISPID_WMPCOREEVENT_MEDIACOLLECTIONATTRIBUTESTRINGCHANGED:
			{/*	[id(0x000016bc), helpstring("Sent when an attribute string is changed in the media collection")]
				void MediaCollectionAttributeStringChanged([in] BSTR bstrAttribName, [in] BSTR bstrOldAttribVal, [in] BSTR bstrNewAttribVal); */
				DEBUGOUT("wmp_Evt:\tMediaCollectionAttributeStringChanged","");
			}	break;
			case DISPID_WMPCOREEVENT_MEDIAERROR:
			{/*	[id(0x000016bd), helpstring("Sent when the media object has an error condition")]
				void MediaError([in] IDispatch* pMediaObject); */
				DEBUGOUT("wmp_Evt:\tMediaError","");
			}	break;
			case DISPID_WMPCOREEVENT_DOMAINCHANGE:
			{/*	[id(0x000016be), helpstring("Send a current domain")]
				void DomainChange([in] BSTR strDomain); */
				DEBUGOUT("wmp_Evt:\tDomainChange","");
			}	break;
			case DISPID_WMPCOREEVENT_OPENPLAYLISTSWITCH:
			{/*	[id(0x000016bf), helpstring("Current playlist switch with no open state change")]
				void OpenPlaylistSwitch([in] IDispatch* pItem); */
				DEBUGOUT("wmp_Evt:\tOpenPlaylistSwitch","");
			}	break;

		// DIID_WMPOCXEvents Key / mouse
			case DISPID_WMPOCXEVENT_CLICK:
			{/*	[id(0x00001969), helpstring("Occurs when a user clicks the mouse")]
				void Click([in] short nButton, [in] short nShiftState, [in] long fX, [in] long fY); */
				DEBUGOUT("wmp_Evt:\tClick","");
			}	break;
			case DISPID_WMPOCXEVENT_DOUBLECLICK:
			{/*	[id(0x0000196a), helpstring("Occurs when a user double-clicks the mouse")]
				void DoubleClick([in] short nButton, [in] short nShiftState, [in] long fX, [in] long fY); */
				DEBUGOUT("wmp_Evt:\tDoubleClick","");
			}	break;
			case DISPID_WMPOCXEVENT_KEYDOWN:
			{/*	[id(0x0000196b), helpstring("Occurs when a key is pressed")]
				void KeyDown([in] short nKeyCode, [in] short nShiftState); */
				DEBUGOUT("wmp_Evt:\tKeyDown","");
			}	break;
			case DISPID_WMPOCXEVENT_KEYPRESS:
			{/*	[id(0x0000196c), helpstring("Occurs when a key is pressed and released")]
				void KeyPress([in] short nKeyAscii); */
				DEBUGOUT("wmp_Evt:\tKeyPress","");
			}	break;
			case DISPID_WMPOCXEVENT_KEYUP:
			{/*	[id(0x0000196d), helpstring("Occurs when a key is released")]
				void KeyUp([in] short nKeyCode, [in] short nShiftState); */
				DEBUGOUT("wmp_Evt:\tKeyUp","");
			}	break;
			case DISPID_WMPOCXEVENT_MOUSEDOWN:
			{/*	[id(0x0000196e), helpstring("Occurs when a mouse button is pressed")]
				void MouseDown([in] short nButton, [in] short nShiftState, [in] long fX, [in] long fY); */
				DEBUGOUT("wmp_Evt:\tMouseDown","");
			}	break;
			case DISPID_WMPOCXEVENT_MOUSEMOVE:
			{/*	[id(0x0000196f), helpstring("Occurs when a mouse pointer is moved")]
				void MouseMove([in] short nButton, [in] short nShiftState, [in] long fX, [in] long fY); */
				DEBUGOUT("wmp_Evt:\tMouseMove","");
			}	break;
			case DISPID_WMPOCXEVENT_MOUSEUP:
			{/*	[id(0x00001970), helpstring("Occurs when a mouse button is released")]
				void MouseUp([in] short nButton, [in] short nShiftState, [in] long fX, [in] long fY); */
				DEBUGOUT("wmp_Evt:\tMouseUp","");
			}	break;

		// OTHER Events
			case 0x00001971:
			{/*	[id(0x00001971), helpstring("Occurs when a device is connected")]
				void DeviceConnect([in] IWMPSyncDevice* pDevice); */
				DEBUGOUT("wmp_Evt:\tDeviceConnect","");
			}	break;
			case 0x00001972:
			{/*	[id(0x00001972), helpstring("Occurs when a device is disconnected")]
				void DeviceDisconnect([in] IWMPSyncDevice* pDevice); */
				DEBUGOUT("wmp_Evt:\tDeviceDisconnect","");
			}	break;
			case 0x00001973:
			{/*	[id(0x00001973), helpstring("Occurs when a device status changes")]
				void DeviceStatusChange([in] IWMPSyncDevice* pDevice, [in] WMPDeviceStatus NewStatus); */
				DEBUGOUT("wmp_Evt:\tDeviceStatusChange","");
			}	break;
			case 0x00001974:
			{/*	[id(0x00001974), helpstring("Occurs when a device sync state changes")]
				void DeviceSyncStateChange([in] IWMPSyncDevice* pDevice, [in] WMPSyncState NewState); */
				DEBUGOUT("wmp_Evt:\tDeviceSyncStateChange","");
			}	break;
			case 0x00001975:
			{/*	[id(0x00001975), helpstring("Occurs when a device's media has an error")]
				void DeviceSyncError([in] IWMPSyncDevice* pDevice, [in] IDispatch* pMedia); */
				DEBUGOUT("wmp_Evt:\tDeviceSyncError","");
			}	break;
			case 0x00001976:
			{/*	[id(0x00001976), helpstring("Occurs when createPartnership call completes")]
				void CreatePartnershipComplete([in] IWMPSyncDevice* pDevice, [in] HRESULT hrResult); */
				DEBUGOUT("wmp_Evt:\tCreatePartnershipComplete","");
			}	break;
			default:
				hr = DISP_E_MEMBERNOTFOUND;
				break;
		}
	} //end if (pEventHandler==m_comPlaybackEventSink)

	return( hr );
}

/////////////////////////////////////////////////////////////////////////////
// COM ...CRemoteHost  

WindowsMediaPlayer::CRemoteHost::CRemoteHost()
{
}

WindowsMediaPlayer::CRemoteHost::~CRemoteHost()
{
}

//***************************************************************************
// QueryService()
// API from IServiceProvider
//***************************************************************************
HRESULT 
WindowsMediaPlayer::CRemoteHost::QueryService(REFGUID guidService, REFIID riid, void ** ppv)
{
    return ppv? QueryInterface(riid, ppv) : E_POINTER;
}

//***************************************************************************
// GetServiceType()
// Always return Remote so that the player OCX runs at remote state
//***************************************************************************
HRESULT 
WindowsMediaPlayer::CRemoteHost::GetServiceType(BSTR * pbstrType)
{
    HRESULT hr = E_POINTER;
    if(pbstrType)
    {
        *pbstrType = ::SysAllocString(L"Remote");
        hr = *pbstrType? S_OK : E_POINTER;
    }
    return hr;
}

//***************************************************************************
// GetApplicationName()
// Return the application name. It will be shown in player's menu View >
// Switch to applications
//***************************************************************************
HRESULT 
WindowsMediaPlayer::CRemoteHost::GetApplicationName(BSTR * pbstrName)
{
    HRESULT     hr = E_POINTER;
    if(pbstrName)
    {
//        CComBSTR    bstrAppName = _T("");
//        bstrAppName.LoadString(IDS_PROJNAME);
//        *pbstrName = bstrAppName.Detach();
        *pbstrName = ::SysAllocString(L"Miranda ListeningTo");
        hr = *pbstrName? S_OK : E_POINTER;
    }
    return hr;
}

//***************************************************************************
// GetScriptableObject()
// There is no scriptable object in this application
//***************************************************************************
HRESULT 
WindowsMediaPlayer::CRemoteHost::GetScriptableObject(BSTR * pbstrName, IDispatch ** ppDispatch)
{
    if(pbstrName)
    {
        *pbstrName = NULL;
    }
    if(ppDispatch)
    {
        *ppDispatch = NULL;
    }
    return E_NOTIMPL;
}

//***************************************************************************
// GetCustomUIMode()
// When UI mode of the player OCX is set to custom, this function is called
// to give the skin file path that will be loaded to the player OCX.
// 
//***************************************************************************
HRESULT 
WindowsMediaPlayer::CRemoteHost::GetCustomUIMode(BSTR * pbstrFile)
{
    return E_NOTIMPL;
}


