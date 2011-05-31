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


class mRadio : public Player
{
protected:
	virtual void EnableDisable();

	char*	m_proto;
	DWORD	m_version;

	LPTSTR	m_ptszURL;
	HANDLE	m_hContact;									//CurrentStation
	HANDLE	FindContact(LPTSTR ptszURL = NULL);			//Fill

	BOOL	COM_Start();
	BOOL	COM_Stop();
	BOOL	COM_infoCache();							//get the listeningTo info

	HANDLE			m_hRadioStatus;						//HookEventObj on ME_RADIO_STATUS
	int __cdecl		EVT_RadioStatus(WPARAM wParam, LPARAM lParam);

	virtual ~mRadio();

/////////////////////////////////////////////////////////////////////////////
// only for mradio + 0.0.1.x (never need for + 0.0.2.x)

	BOOL			COM_ConnectServer();				//InitmRadio
	void			COM_ReleaseServer();				//ClearmRadio

	int __cdecl		EVT_RadioStatus_1x(WPARAM wParam, LPARAM lParam);

	HANDLE			m_hProtoAck;						//HookEventObj on ME_PROTO_ACK
	int __cdecl		EVT_ProtoAck(WPARAM wParam,LPARAM lParam);

	HANDLE			m_hDbSettingChanged;				//HookEventObj on ME_DB_CONTACT_SETTINGCHANGED
	int __cdecl		EVT_DbSettingChanged(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////

public:
	//common
	mRadio (int index);

	//Plugin ...
	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);
};
