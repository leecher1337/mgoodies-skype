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

extern "C"
{
#include ".\iTunes\iTunesCOMInterface.h"
}

#include "TEventHandler.h"
using namespace TEventHandlerNamespace;

// ***** Make a forward declaration so that our TEventHandler template class can use it. *****
class ITunes;

// ***** Declare an event handling class using the TEventHandler template. *****
typedef TEventHandler<ITunes, IiTunes, _IiTunesEvents> IMIM_iTunesEventH;

class ITunes : public Player
{
protected:
	HWND FindWindow();			//find Player Window
	virtual void EnableDisable();

	//com object
	/* ***** Declare an instance of a COMApplication smart pointer. ***** */
	IiTunes				*m_comApp;
	/* ***** Declare an instance of a IITTrack smart pointer. ***** */
	IITTrack			*m_comTrack;

	/* ***** Declare a pointer to a TEventHandler class which is specially tailored
	/  ***** to receiving events from the _IiTunesEvents events of an
	/  ***** IiTunes object (It is designed to be a sink object) *****/
	IMIM_iTunesEventH*	m_comAppEventSink;

	WCHAR				m_filename[1024];
	VARIANT_BOOL		m_vbServerState;			//hold the server state VARIANT_TRUE == server is running
	BSTR				m_comRet;					//global BSTR for free use
	BOOL	COM_Start();
	BOOL	COM_Stop();
	BOOL	COM_ConnectServer();					//start the COM instance
	void	COM_ReleaseServer();					//stop  the COM instance, disconect and free all
	BOOL	COM_infoCache();						//get the listeningTo info
	BOOL	COM_IsPause();

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

	virtual ~ITunes();

public:
	ITunes(int index);
	BYTE GetStatus();

	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);
};
