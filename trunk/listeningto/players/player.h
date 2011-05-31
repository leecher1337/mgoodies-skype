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

#define PL_OFFLINE	0
#define PL_STOPPED	1
#define PL_STARTET	2
#define PL_PLAYING	3
#define PL_PAUSED	4
#define PL_FORWARD	5
#define PL_REWIND	6

#define CALL(_F_)			hr = _F_; if (FAILED(hr)) goto FAILURE
#define RELEASE(_x_,_y_)	{ if (_x_ != NULL && _y_ != 0) _x_->Release(); _x_ = NULL; }
#define BSTRFREE(_x_)		{ SysFreeString(_x_); _x_ = NULL; }
#define mir_bstr2t(_X_)		( SysStringLen(_X_)>0 ? mir_u2t(_X_) : NULL )

extern UINT_PTR hTimer;
extern void HasNewListeningInfo(int ID);
extern BOOL SetActivePlayer(int ID, int newVal);

class Player
{
protected:
	LISTENINGTOINFO		m_listening_info;
	CRITICAL_SECTION	cs;
	HWND	m_hwnd;

	void	NotifyInfoChanged(int ID = -1);
	virtual HRESULT ObjGet(REFCLSID rclsid, REFIID riid, void** pDispatch);
	void	EVT_Unhook(HANDLE *h){if(*h) {UnhookEvent(*h); *h = NULL;}}

private:

public:
	BOOL	m_enabled;
	BOOL	m_needPoll;
	BYTE	m_state;
	TCHAR*	m_name;
	int		m_index;

	Player(int index);
	virtual ~Player();

	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);

	virtual void FreeData();
	virtual BYTE GetStatus() {return m_state;};

	// Helpers to write to this object's listening info
	virtual LISTENINGTOINFO * LockListeningInfo();
	virtual void ReleaseListeningInfo();

	// Called everytime options change
	virtual void EnableDisable() {}
};


class ExternalPlayer : public Player
{
protected:
	TCHAR **m_window_classes;
	int		m_window_classes_num;

	virtual HWND FindWindow();

public:
	ExternalPlayer(int index);
	virtual ~ExternalPlayer();

	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);
};


class CodeInjectionPlayer : public ExternalPlayer
{
protected:
	char*	m_dll_name;
	TCHAR*	m_message_window_class;
	DWORD	m_next_request_time;

	virtual void InjectCode();

public:
	CodeInjectionPlayer(int index);
	virtual ~CodeInjectionPlayer();

	virtual BYTE GetStatus();
	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);
};
