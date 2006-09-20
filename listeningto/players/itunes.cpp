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


#include "..\\commons.h"


extern "C"
{
#include "iTunesCOMInterface_i.c"
}



ITunes::ITunes()
{
	filename[0] = L'\0';

	hwnd = NULL;
	iTunesApp = NULL;
	track = NULL;
	file = NULL;
	ret = NULL;
}


void ITunes::FindWindow()
{
	hwnd = ::FindWindow(_T("iTunes"), _T("iTunes"));
}


void ITunes::FreeTempData()
{
#define RELEASE(_x_) if (_x_ != NULL) { _x_->Release(); _x_ = NULL; }

	RELEASE(file);
	RELEASE(track);
	RELEASE(iTunesApp);

	if (ret != NULL)
	{
		SysFreeString(ret);
		ret = NULL;
	}
}


void ITunes::FreeData()
{
	PollPlayer::FreeData();
	FreeTempData();
}


#define CALL(_F_) hr = _F_; if (FAILED(hr)) return FALSE

// Init data and put filename playing in ret and ->fi.filename
BOOL ITunes::InitAndGetFilename()
{
	HRESULT hr;

	// Find window
	FindWindow();
	if (hwnd == NULL)
		return FALSE;

	CALL( CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, __uuidof(iTunesApp), (void **)&iTunesApp) );

	ITPlayerState state;
	CALL( iTunesApp->get_PlayerState(&state) );
	if (state == ITPlayerStateStopped)
		return FALSE;

	CALL( iTunesApp->get_CurrentTrack(&track) );
	if (track == NULL)
		return FALSE;

	CALL( track->QueryInterface(__uuidof(file), (void **)&file) );

	CALL( file->get_Location(&ret) );

	return TRUE;
}


BOOL ITunes::FillCache()
{
	HRESULT hr;
	long lret;

	CALL( track->get_Album(&ret) );
	if (ret != NULL && ret[0] != L'\0')
		listening_info.szAlbum = mir_dupTW(ret);

	CALL( track->get_Artist(&ret) );
	if (ret != NULL && ret[0] != L'\0')
		listening_info.szArtist = mir_dupTW(ret);

	CALL( track->get_Name(&ret) );
	if (ret != NULL && ret[0] != L'\0')
		listening_info.szTitle = mir_dupTW(ret);

	CALL( track->get_Year(&lret) );
	if (lret > 0)
	{
		listening_info.szYear = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));
		_itot(lret, listening_info.szYear, 10);
	}

	CALL( track->get_TrackNumber(&lret) );
	if (lret > 0)
	{
		listening_info.szTrack = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));
		_itot(lret, listening_info.szTrack, 10);
	}

	CALL( track->get_Genre(&ret) );
	if (ret != NULL && ret[0] != L'\0')
		listening_info.szGenre = mir_dupTW(ret);

	CALL( track->get_Duration(&lret) );
	if (lret > 0)
	{
		listening_info.szLength = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));

		int s = lret % 60;
		int m = (lret / 60) % 60;
		int h = (lret / 60) / 60;

		if (h > 0)
			mir_sntprintf(listening_info.szLength, 9, _T("%d:%02d:%02d"), h, m, s);
		else
			mir_sntprintf(listening_info.szLength, 9, _T("%d:%02d"), m, s);
	}

	listening_info.szType = mir_dupT(_T("Music"));

	if (listening_info.szTitle == NULL)
	{
		// Get from filename
		TCHAR *p = wcsrchr(filename, '\\');
		if (p != NULL)
			p++;
		else
			p = filename;
		
		listening_info.szTitle = mir_dupTW(p);

		TCHAR *pt = _tcsrchr(listening_info.szTitle, '.');
		if (pt != NULL)
			*p = _T('\0');
	}

	listening_info.szPlayer = mir_dupT(_T("iTunes"));

	listening_info.cbSize = sizeof(listening_info);

	return TRUE;
}


// < 0 removed
// 0 not changed
// > 0 changed
int ITunes::ChangedListeningInfo()
{
	if (!enabled || !InitAndGetFilename() || ret == NULL || ret[0] == L'\0')
	{
		FreeData();
		if (filename[0] != L'\0')
		{
			filename[0] = L'\0';
			return -1;
		}
		else
			return 0;
	}
	
	if (strcmpnullW(filename, ret) == 0)
	{
		FreeTempData();
		return 0;
	}

	// Fill the data cache
	FreeListeningInfo(&listening_info);

	lstrcpyW(filename, ret);

	if (!FillCache())
	{
		FreeData();
		if (filename[0] != L'\0')
		{
			filename[0] = L'\0';
			return -1;
		}
		else
			return 0;
	}
	else
	{
		FreeTempData();
		return 1;
	}
}
