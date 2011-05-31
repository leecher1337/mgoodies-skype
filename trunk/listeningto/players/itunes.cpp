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
#include ".\iTunes\iTunesCOMInterface_i.c"
}


/////////////////////////////////////////////////////////////////////////////
// main

ITunes::ITunes(int index)
: Player(index)
{
	m_name					= _T("iTunes");

	m_filename[0]			= L'\0';

	m_comApp				= NULL;
	m_comAppEventSink		= NULL;
	m_comTrack				= NULL;
//	m_comFile				= NULL;
	m_comRet				= NULL;
	m_vbServerState			= VARIANT_FALSE;
	m_state					= PL_OFFLINE;
}

ITunes::~ITunes()
{
	COM_Stop();
	FreeData();
}

HWND 
ITunes::FindWindow()
{
	HWND hwnd = ::FindWindow(_T("iTunes"), _T("iTunes"));
	if (hwnd != m_hwnd) {
		m_hwnd = hwnd;
		if(m_comApp) COM_ReleaseServer();
		if(m_hwnd)   COM_ConnectServer();
	}
	return m_hwnd;
}

void 
ITunes::EnableDisable()
{
	static BOOL old = FALSE;
	if(m_enabled == old) {
		return;
	}
	else if(m_enabled){
		COM_Start();
	}
	else {
		COM_Stop();
	}
	old = m_enabled;
}

BYTE 
ITunes::GetStatus()
{
	if(m_needPoll)
		FindWindow();
	return m_state;
}

/////////////////////////////////////////////////////////////////////////////
// COM ...

BOOL 
ITunes::COM_Start()
{
	m_needPoll = TRUE;
	COM_ConnectServer();
	return TRUE;
}

BOOL 
ITunes::COM_Stop()
{
	COM_ReleaseServer();
	m_needPoll = FALSE;
	return TRUE;
}

BOOL 
ITunes::COM_ConnectServer()
{
	HRESULT hr;
	//***** Attach to the running COM instance... *****/
	//if (SUCCEEDED(ObjGet(CLSID_iTunesApp, __uuidof(m_comApp), (void**)&m_comApp))) {

	// ***** Start new COM instance... *****
	hr = CoCreateInstance(	CLSID_iTunesApp, 
							NULL, 
							CLSCTX_LOCAL_SERVER, 
							__uuidof(m_comApp), 
							(void **)&m_comApp );
	if(SUCCEEDED(hr)) {
		m_vbServerState = VARIANT_TRUE;
		DEBUGOUT("iTunes: \tcom Server ","on");
		// ***** get the IITTrack interface...
		CALL( m_comApp->get_CurrentTrack(&m_comTrack) );
		if (m_comTrack == NULL)
			goto FAILURE;
		// ***** Instantiate an IMIM_iTunesEventH object. *****
		m_comAppEventSink = new IMIM_iTunesEventH(*this, m_comApp, &ITunes::COM_OnEventInvoke);
		if(!m_comAppEventSink)
			goto FAILURE;
		m_state = PL_STOPPED;
		SetActivePlayer(m_index, m_index);

		BOOL isPause = COM_IsPause();
		ITPlayerState bState;
		if(SUCCEEDED(m_comApp->get_PlayerState(&bState)) && isPause != -1)
			switch(bState) {
				//TODO: check result for NotifyInfoChanged() needet
				case ITPlayerStatePlaying:
					m_state = isPause ? PL_PAUSED : PL_PLAYING;
					DEBUGOUT("iTune_Evt:\tPlayerState = ", isPause ? "Playing (Pause)" : "Playing");
					NotifyInfoChanged();
					break;
				case ITPlayerStateStopped:
					m_state = isPause ? PL_PAUSED : PL_STOPPED;
					DEBUGOUT("iTune_Evt:\tPlayerState = ", isPause ? "Stopped (Pause)" : "Stopped");
					m_state = PL_STOPPED;
					break;
				case ITPlayerStateFastForward:
					m_state = PL_FORWARD;
					DEBUGOUT("iTune_Evt:\tPlayerState = ","FastForward");
					break;
				case ITPlayerStateRewind:
					m_state = PL_REWIND;
					DEBUGOUT("iTune_Evt:\tPlayerState = ","Rewind");
					break;
			}//end switch(bState)
	}	
	else {
		DEBUGOUT("iTunes: \tcom Server ","off");
		m_state = PL_OFFLINE;
		return FALSE;
	}
	return TRUE;

FAILURE:
	COM_ReleaseServer();
	return FALSE;
}

void 
ITunes::COM_ReleaseServer()
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
	RELEASE(m_comTrack, m_vbServerState);
	RELEASE(m_comApp, m_vbServerState);
	BSTRFREE(m_comRet);

	m_state		= PL_OFFLINE;
	m_needPoll	= TRUE;
	if(loaded)
		NotifyInfoChanged();
}

BOOL 
ITunes::COM_infoCache()
{
	if(	(m_state <= PL_STOPPED) ||
		(m_comAppEventSink && m_comAppEventSink->m_vbServerState == VARIANT_FALSE) ||
		(!m_comTrack) )
		return FALSE;

	HRESULT hr;
	long lret;
	IITFileOrCDTrack *comFile = NULL;

/* TODO: cleanup comment (now we get ITPlayerState from event   see m_state member)
	ITPlayerState state;
	CALL( m_comApp->get_PlayerState(&state) );
	if (state == ITPlayerStateStopped ) return FALSE;
*/

	//set m_listening_info.cbSize to make FreeData() posibil
	m_listening_info.cbSize		= sizeof(m_listening_info);

	if(SUCCEEDED(m_comTrack->get_Name(&m_comRet)))
		m_listening_info.ptszTitle = mir_bstr2t(m_comRet);

	if (!m_listening_info.ptszTitle) {
		// Get from filename
		// ***** get the IITFileOrCDTrack interface...
		CALL( m_comTrack->QueryInterface(__uuidof(comFile), (void **)&comFile) );
		if (comFile == NULL)
			goto FAILURE;
		CALL( comFile->get_Location(&m_comRet) );
		if(SysStringLen(m_comRet)==0)
			goto FAILURE;
		RELEASE(comFile, m_vbServerState);

		wcscpy(m_filename, m_comRet);
		WCHAR *p = wcsrchr(m_filename, '\\');
		if (p != NULL)
			p++;
		else
			p = m_filename;
		
		m_listening_info.ptszTitle = mir_u2t(p);

		TCHAR *pt = _tcsrchr(m_listening_info.ptszTitle, '.');
		if (pt != NULL)
			*p = _T('\0');
	}

	m_listening_info.ptszPlayer	= mir_tstrdup(m_name);
	m_listening_info.ptszType	= mir_tstrdup(_T("Music"));
	m_listening_info.dwFlags	= LTI_TCHAR;

	if(SUCCEEDED(m_comTrack->get_Album(&m_comRet)))
		m_listening_info.ptszAlbum = mir_bstr2t(m_comRet);

	if(SUCCEEDED(m_comTrack->get_Artist(&m_comRet)))
		m_listening_info.ptszArtist = mir_bstr2t(m_comRet);

	if(SUCCEEDED(m_comTrack->get_Year(&lret)))
		if (lret > 0)
		{
			m_listening_info.ptszYear = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));
			_i64tot(lret, m_listening_info.ptszYear, 10);
		}

	if(SUCCEEDED(m_comTrack->get_TrackNumber(&lret)))
		if (lret > 0)
		{
			m_listening_info.ptszTrack = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));
			_i64tot(lret, m_listening_info.ptszTrack, 10);
		}

	if(SUCCEEDED(m_comTrack->get_Genre(&m_comRet)))
		m_listening_info.ptszGenre = mir_bstr2t(m_comRet);

	if(SUCCEEDED(m_comTrack->get_Duration(&lret)))
		if (lret > 0)
		{
			m_listening_info.ptszLength = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));

			int s = lret % 60;
			int m = (lret / 60) % 60;
			int h = (lret / 60) / 60;

			if (h > 0)
				mir_sntprintf(m_listening_info.ptszLength, 9, _T("%d:%02d:%02d"), h, m, s);
			else
				mir_sntprintf(m_listening_info.ptszLength, 9, _T("%d:%02d"), m, s);
		}
	return TRUE;

FAILURE:
	FreeData();
	RELEASE(comFile, m_vbServerState);
	BSTRFREE(m_comRet);
	return FALSE;
}

// ***** COM_OnEventInvoke() is inoked by the TEventHandler based class object *****
// ***** when an event is fired from the COM object that implements .FiringObject.     *****
HRESULT 
ITunes::COM_OnEventInvoke(
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

	HRESULT hr = S_OK;
	//TODO: check if all events work
	if (pEventHandler==m_comAppEventSink) {
		switch (dispIdMember) {
			case ITEventDatabaseChanged:			//= 1,
			{/*	not needet*/
				DEBUGOUT("iTune_Evt:\tDatabase","Changed");
			}	break;
			case ITEventPlayerPlay:					//= 2,
			{/*	Parameters:(ITTrack iTrack)
				iTrack - An ITTrack object corresponding to the track that has started playing.
				The ITEventPlayerPlay event is fired when a track begins playing.*/
				DEBUGOUT("iTune_Evt:\tPlayer","Play");
				m_state = PL_PLAYING;
				NotifyInfoChanged();
			}	break;
			case ITEventPlayerStop:					//= 3,
			{/*	Parameters:(ITTrack iTrack)
				iTrack - An ITTrack object corresponding to the track that has stopped playing.
				The ITEventPlayerStop event is fired when a track stops playing.*/
				DEBUGOUT("iTune_Evt:\tPlayer","Stop");
				m_state = PL_STOPPED;
				NotifyInfoChanged();
			}	break;
			case ITEventPlayerPlayingTrackChanged:	//= 4,
			{/*	Parameters:(ITTrack iTrack)
				iTrack - An ITTrack object corresponding to the track that is now playing.
				The ITEventPlayerPlayingTrackChanged event is fired when information about
				the currently playing track has changed. This event is fired when the user
				changes information about the currently playing track (e.g. the name of the track).
				This event is also fired when iTunes plays the next joined CD track in a CD playlist, 
				since joined CD tracks are treated as a single track.*/
			//RELEASE(m_comTrack, TRUE);
			//m_comTrack = pDispParams->rgvarg[0].pdispVal;
				DEBUGOUT("iTune_Evt:\tPlayer","PlayingTrackChanged");
				m_state = PL_PLAYING;
				NotifyInfoChanged();
			}	break;
			case ITEventUserInterfaceEnabled:		//= 5,
			{/*	no info, dont know what to do with this event*/
				DEBUGOUT("iTune_Evt:\tUserInterfaceEnabled","");
			}	break;
			case ITEventCOMCallsDisabled:			//= 6,
			{/*	Parameters:(ITCOMDisabledReason reason)
				reason - The reason the COM interface is being disabled. This is typically ITCOMDisabledReasonDialog.
				The ITEventCOMCallsDisabled event is fired when calls to the iTunes COM interface will be deferred. 
				Typically, iTunes will defer COM calls when any modal dialog is being displayed. 
				When the user dismisses the last modal dialog, COM calls will be enabled again, 
				and any deferred COM calls will be executed. 
				You can use this event to avoid making a COM call which will be deferred.*/
				switch (pDispParams->rgvarg[0].uintVal){
					case ITCOMDisabledReasonOther:
						DEBUGOUT("iTune_Evt:\tCOMCallsDisabled = ","ReasonOther");
						break;
					case ITCOMDisabledReasonDialog:
						DEBUGOUT("iTune_Evt:\tCOMCallsDisabled = ","ReasonDialog");
						break;
					case ITCOMDisabledReasonQuitting:
						DEBUGOUT("iTune_Evt:\tCOMCallsDisabled = ","ReasonQuitting");
						break;
				}
				if(m_comAppEventSink)
					m_comAppEventSink->m_vbServerState = VARIANT_FALSE;
			}	break;
			case ITEventCOMCallsEnabled:			//= 7,
			{/*	Parameters:()
				The ITEventCOMCallsEnabled event is fired when calls to the 
				iTunes COM interface will no longer be deferred. Typically, iTunes will defer 
				COM calls when any modal dialog is being displayed. When the user dismisses 
				the last modal dialog, COM calls will be enabled again, 
				and any deferred COM calls will be executed. */
				DEBUGOUT("iTune_Evt:\tCOMCallsEnabled","");
				if(m_comAppEventSink)
					m_comAppEventSink->m_vbServerState = VARIANT_TRUE;
			}	break;
			case ITEventQuitting:					//= 8,
			{/*	Parameters:()
				The ITEventQuitting event is fired when iTunes is about to quit. 
				If the user attempts to quit iTunes while a client still has outstanding 
				iTunes COM objects instantiated, iTunes will display a warning dialog. 
				The user can still choose to quit iTunes anyway, in which case this event will be fired. 
				After this event is fired, any existing iTunes COM objects will no longer be valid. 
				This event is only used to notify clients that iTunes is quitting, 
				clients cannot prevent this from happening.*/
				DEBUGOUT("iTune_Evt:\tQuitting","");
				m_state = PL_OFFLINE;
				m_vbServerState = VARIANT_FALSE;
				COM_ReleaseServer();
			}	break;
			case ITEventAboutToPromptUserToQuit:	//= 9,
			{/*	Parameters:()
				The ITEventAboutToPromptUserToQuit event is fired when iTunes is about prompt the user to quit. 
				This event gives clients the opportunity to prevent the warning dialog prompt from occurring. 
				If the user attempts to quit iTunes while a client still has outstanding iTunes COM objects 
				instantiated, iTunes will display a warning dialog. 
				This event is fired just before the warning dialog is shown. iTunes will then wait up to 5 seconds 
				for clients to release any outstanding iTunes COM objects. 
				If all objects are released during this time, the warning dialog will not be shown and iTunes 
				will quit immediately. Otherwise, the warning dialog will be shown. If the user chooses to quit 
				iTunes anyway, the ITEventQuitting event is fired. 
				See iTunesEventsInterface.onQuittingEvent() for more details. */
				DEBUGOUT("iTune_Evt:\tAboutToPromptUserToQuit","");
			}	break;
			case ITEventSoundVolumeChanged:			//= 10
			{/*	Parameters:(int newVolume)
				newVolume - The new sound output volume (0 = minimum, 100 = maximum).
				The ITEventSoundVolumeChanged event is fired when the sound output volume has changed.*/
			#ifdef DEBUG
				char temp[5];
				_i64toa(pDispParams->rgvarg[0].intVal,temp,10);
				DEBUGOUT("iTune_Evt:\tSoundVolumeChanged = ",temp);
			#endif
			}	break;

			case 0x00009999:
			{//RPCoff() - this is not a interface member (it is only a event from sink object if count m_cRef == 1);
				DEBUGOUT("iTune_Evt:\t","ServerDisconect");
				//m_state = PL_OFFLINE;
			}	break;
		}
	} //end if (pEventHandler==m_comPlaybackEventSink)

	return( hr );
}

BOOL 
ITunes::COM_IsPause()
{
	BOOL res = -1;
	VARIANT_BOOL		previousEnabled		= VARIANT_FALSE;
	VARIANT_BOOL		nextEnabled			= VARIANT_FALSE;
	ITPlayButtonState	playPauseStopState	= ITPlayButtonStatePlayDisabled;
	if(SUCCEEDED(m_comApp->GetPlayerButtonsState(&previousEnabled, &playPauseStopState, &nextEnabled)))
	{//TODO: check if switch is realy needet
		res = FALSE;
		switch(playPauseStopState){
			case ITPlayButtonStatePlayDisabled:
				/*m_state = PL_?*/;	break;
			case ITPlayButtonStatePlayEnabled:
				/*m_state = PL_?*/;	break;
			case ITPlayButtonStatePauseEnabled:
				res = TRUE;	break;
			case ITPlayButtonStatePauseDisabled:
				/*m_state = PL_?*/;	break;
			case ITPlayButtonStateStopEnabled:
				/*m_state = PL_?*/;	break;
			case ITPlayButtonStateStopDisabled:
				/*m_state = PL_?*/;	break;
		}
	}
	return res;
}

BOOL 
ITunes::GetListeningInfo(LISTENINGTOINFO *lti)
{
	FreeData();
	if(m_needPoll && FindWindow() == NULL)
		return FALSE;
	return COM_infoCache() ? Player::GetListeningInfo(lti) : FALSE;
}
