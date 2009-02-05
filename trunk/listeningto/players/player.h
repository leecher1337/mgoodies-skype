/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


class Player
{
protected:
	LISTENINGTOINFO listening_info;

	void NotifyInfoChanged();

public:
	BOOL enabled;
	BOOL needPoll;
	TCHAR *name; 

	Player();
	virtual ~Player();

	// Return:
	// < 0 removed
	// 0 not changed
	// > 0 changed
	virtual int ChangedListeningInfo() = 0;

	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);

	virtual void FreeData();

	// Called everytime options change
	virtual void EnableDisable() {}
};

class PollPlayer : public Player
{
public:
	PollPlayer();
};

class CallbackPlayer : public Player
{
protected:
	BOOL changed;
	BOOL csFreed;

public:
	CRITICAL_SECTION cs;

	CallbackPlayer();
	virtual ~CallbackPlayer();

	virtual int ChangedListeningInfo();

	virtual void FreeData();
};

class CodeInjectionPlayer : public PollPlayer
{
protected:
	char *dll_name;
	TCHAR *window_class;
	TCHAR *window_name;
	TCHAR *message_window_class;
	DWORD next_request_time;
	BOOL found_window;

public:
	CodeInjectionPlayer();
	virtual ~CodeInjectionPlayer();

	virtual int ChangedListeningInfo();
	
	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);
};

class ExternalPlayer : public PollPlayer
{
protected:
	TCHAR *window_class;
	TCHAR *window_name;
	DWORD next_request_time;
	BOOL found_window;

public:
	ExternalPlayer();
	virtual ~ExternalPlayer();

	virtual int ChangedListeningInfo();
	
	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);
};
