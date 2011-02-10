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

class WindowsLiveMessanger : public Player
{
protected:		//only WLM
	WCHAR	m_received[1024];
	WCHAR	m_last_received[1024];

	HWND	FindWindow();			//find Player Window

	BOOL	WLM_Start();			//start
	BOOL	WLM_Stop();				//stop

protected:
	ATOM	cWndclass;
	HWND	m_hwndclass;

	void EnableDisable();

	virtual ~WindowsLiveMessanger();

public:
	//common
	TCHAR **m_window_classes;
	int		m_window_classes_num;
	TCHAR	m_window_class[128];
	WindowsLiveMessanger(int index);

	//COM ...Remoting the Windows Media Player Control
	BOOL GetListeningInfo(LISTENINGTOINFO *lti);

	//WLM ...Windows Live Messanger
	BYTE GetStatus();
	void WLM_ProcessReceived();
	void WLM_NewData(const WCHAR *data, size_t len);

};
