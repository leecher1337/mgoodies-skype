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

extern "C"
{
#include "wa_ipc.h"
}


class Winamp : public PollPlayer
{
protected:
	char filename[1024];

	HWND hwnd;
	HANDLE process;

	extendedFileInfoStruct *_fi;
	char *_ret;
	char *_param;

	extendedFileInfoStruct fi;
	char ret[1024];

	void FindWindow();
	BOOL InitTempData();
	void FreeTempData();
	BOOL InitAndGetFilename();
	int GetMetadata(char *metadata, TCHAR **data);
	BOOL FillCache();

public:
	Winamp(int anId);

	virtual int ChangedListeningInfo();

	virtual void FreeData();
};
