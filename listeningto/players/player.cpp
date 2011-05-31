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

Player::Player(int index) : m_index (index), m_name(_T("Player")), m_enabled(FALSE), m_needPoll(FALSE), m_state(PL_OFFLINE)
{
	m_hwnd	= NULL;
	ZeroMemory(&m_listening_info, sizeof(m_listening_info));
	InitializeCriticalSection(&cs);
}

Player::~Player()
{
	FreeData();
	DeleteCriticalSection(&cs);
}

void 
Player::NotifyInfoChanged(int ID)
{
	//check if prev event set a timer (handle doubble events)
	if(hTimer)
		return;
	HasNewListeningInfo((ID<0)? m_index : ID);
}

BOOL 
Player::GetListeningInfo(LISTENINGTOINFO *lti)
{
	EnterCriticalSection(&cs);

	BOOL ret;
	if (m_listening_info.cbSize == 0)
	{
		ret = FALSE;
	}
	else 
	{
		if (lti != NULL)
			CopyListeningInfo(lti, &m_listening_info);
		ret = TRUE;
	}

	LeaveCriticalSection(&cs);

	return ret;
}

void 
Player::FreeData()
{
	EnterCriticalSection(&cs);

	if (m_listening_info.cbSize != 0)
		FreeListeningInfo(&m_listening_info);

	LeaveCriticalSection(&cs);
}

LISTENINGTOINFO*
Player::LockListeningInfo()
{
	EnterCriticalSection(&cs);

	return &m_listening_info;
}

void 
Player::ReleaseListeningInfo()
{
	LeaveCriticalSection(&cs);
}

//Retrieves a reference to a COM object from an existing process
HRESULT 
Player::ObjGet(REFCLSID rclsid, REFIID riid, void** pDispatch)
{
	//based on c++ example http://support.microsoft.com/kb/238610
	HRESULT hr;
	IUnknown	*pUnk  = NULL;

	for(int i=1;i<=5;i++) { //try attaching for up to 5 attempts
		hr = GetActiveObject(rclsid, NULL, (IUnknown**)&pUnk);
		if(SUCCEEDED(hr)) {
			hr = pUnk->QueryInterface(riid, pDispatch);
			break;
		}
		::Sleep(200);
	}
	//Release the no-longer-needed IUnknown...
	RELEASE(pUnk, TRUE);

	return hr;
}

//--------------------------------------------------------

ExternalPlayer::ExternalPlayer(int index)
: Player(index)
{
	m_name					= _T("ExternalPlayer");
	m_needPoll				= TRUE;

	m_window_classes		= NULL;
	m_window_classes_num	= 0;
}

ExternalPlayer::~ExternalPlayer()
{
}

HWND 
ExternalPlayer::FindWindow()
{
	m_hwnd = NULL;
	for(int i = 0; i < m_window_classes_num; i++)
	{
		m_hwnd = ::FindWindow(m_window_classes[i], NULL);
		if (m_hwnd != NULL)
			break;
	}
	return m_hwnd;
}

BOOL 
ExternalPlayer::GetListeningInfo(LISTENINGTOINFO *lti)
{
	if (FindWindow() == NULL)
		return FALSE;

	return Player::GetListeningInfo(lti);
}

//--------------------------------------------------------

CodeInjectionPlayer::CodeInjectionPlayer(int index)
: ExternalPlayer(index)
{
	m_name					= _T("CodeInjectionPlayer");
	m_dll_name				= NULL;
	m_message_window_class	= NULL;
	m_next_request_time		= 0;
}

CodeInjectionPlayer::~CodeInjectionPlayer()
{
}

BYTE 
CodeInjectionPlayer::GetStatus()
{
	if(FindWindow() == 0)
		m_state = PL_OFFLINE;
	return m_state;
}

void 
CodeInjectionPlayer::InjectCode()
{
	if (!opts.enable_code_injection)
		return;
	else if (m_next_request_time > GetTickCount())
		return;

	// Window is opened?
	m_hwnd = FindWindow();
	if (m_hwnd == NULL) {
		m_state = PL_OFFLINE;
		return;
	}

	// Msg Window is registered? (aka plugin is running?)
	HWND msgHwnd = ::FindWindow(m_message_window_class, NULL);
	if (msgHwnd != NULL)
		return;

	m_next_request_time = GetTickCount() + 30000;

	// Get the dll path
	char dll_path[1024] = {0};
	if (!GetModuleFileNameA(hInst, dll_path, MAX_REGS(dll_path)))
		return;

	char *p = strrchr(dll_path, '\\');
	if (p == NULL)
		return;

	p++;
	*p = '\0';

	size_t len = p - dll_path;

	mir_snprintf(p, 1024 - len, "listeningto\\%s.dll", m_dll_name);

	len = strlen(dll_path);

	// File exists?
	DWORD attribs = GetFileAttributesA(dll_path);
	if (attribs == 0xFFFFFFFF || !(attribs & FILE_ATTRIBUTE_ARCHIVE))
		return;

	// Do the code injection
	unsigned long pid;
	GetWindowThreadProcessId(m_hwnd, &pid);
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION 
									| PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
	if (hProcess == NULL)
		return;

	char *_dll = (char *) VirtualAllocEx(hProcess, NULL, len+1, MEM_COMMIT, PAGE_READWRITE );
	if (_dll == NULL)
	{
		CloseHandle(hProcess);
		return;
	}
	WriteProcessMemory(hProcess, _dll, dll_path, len+1, NULL);

	HMODULE hKernel32 = GetModuleHandleA("kernel32");
	HANDLE hLoadLibraryA = GetProcAddress(hKernel32, "LoadLibraryA");
	DWORD threadId;
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) hLoadLibraryA, 
										_dll, 0, &threadId);
	if (hThread == NULL)
	{
		VirtualFreeEx(hProcess, _dll, len+1, MEM_RELEASE);
		CloseHandle(hProcess);
		return;
	}
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	VirtualFreeEx(hProcess, _dll, len+1, MEM_RELEASE);
	CloseHandle(hProcess);
}

BOOL 
CodeInjectionPlayer::GetListeningInfo(LISTENINGTOINFO *lti)
{
	if (m_enabled)
		InjectCode();

	return ExternalPlayer::GetListeningInfo(lti);
}

