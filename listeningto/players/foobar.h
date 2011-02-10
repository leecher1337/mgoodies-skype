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
#include "foo_comserver2\foobar2000_h.h"
#include "foo_comserver2\COMServer2Helper.h"
}

#include "TEventHandler.h"
using namespace TEventHandlerNamespace;

// ***** Make a forward declaration so that our TEventHandler template class can use it. *****
class Foobar;

// ***** Declare an event handling class using the TEventHandler template. *****
typedef TEventHandler<Foobar, IVBPlayback,		_IVBPlaybackEvents		> IMIM_fooEventS;
typedef TEventHandler<Foobar, IClientHelper07,	_IClientHelperEvents07	> IMIM_fooEventH;

class Foobar : public Player
{
protected:		//only foo:mlt
	TCHAR **m_window_classes;
	int		m_window_classes_num;
	HWND	FindWindow();			//find Player Window
	
protected:
	void EnableDisable();

	//com (foo_comserver2)
	/* ***** Declare an instance of a COMApplication smart pointer. ***** */
	IVBApplication		*m_comApp;
	IClientHelper07		*m_comAppH;
	/* ***** Declare an instance of a IVBPlayback smart pointer. ***** */
	IVBPlayback			*m_comPlayback;

	/* ***** Declare a pointer to a IMIM_fooEventS class which is specially tailored
	/  ***** to receiving events from the _IVBPlaybackEvents events of an
	/  ***** IVBPlayback object (It is designed to be a sink object) *****/
	IMIM_fooEventH*		m_comClientHelper07Sink;	//host
	IMIM_fooEventS*		m_comPlaybackEventSink;		//server

	VARIANT_BOOL		m_vbServerState;			//hold the server state VARIANT_TRUE == server is running
	BSTR				m_comRet;					//global BSTR for free use
	BOOL	COM_Start();
	BOOL	COM_Stop();
	BOOL	COM_ConnectServer();					//start the COM instance
	void	COM_ReleaseServer();					//stop  the COM instance, disconect and free all
	BOOL	COM_infoCache();						//get the listeningTo info

	/* ***** common function that handle events fired from the COM object. *****/
	HRESULT	COM_OnEventInvoke
	(
		void*			pEventHandler,			//client sync  Interface (m_comClientHelper07Sink or m_comPlaybackEventSink)
		DISPID			dispIdMember,			//server event Interface member
		REFIID			riid,					//always NULL (see MSDN)
		LCID			lcid,
		WORD			wFlags,
		DISPPARAMS FAR*	pDispParams,
		VARIANT FAR*	pVarResult,
		EXCEPINFO FAR*	pExcepInfo,
		UINT FAR*		puArgErr
	);

	~Foobar();

public:
	//common
	Foobar(int index);

	//COM ...
	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);

	//MLT
	BYTE GetStatus();
};
