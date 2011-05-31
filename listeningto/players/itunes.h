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
