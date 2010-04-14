/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#pragma once

#include <windows.h>
#include "lockfree_hashmap.h"

//#define MREW_DO_DEBUG_LOGGING 1

#if defined(MREW_DO_DEBUG_LOGGING) && (defined(DEBUG) || defined(_DEBUG))
	#define SYNC_BEGINREAD(sync)     sync.BeginRead (__FILE__, __LINE__, __FUNCTION__)
	#define SYNC_ENDREAD(sync)       sync.EndRead   (__FILE__, __LINE__, __FUNCTION__)
	#define SYNC_BEGINWRITE(sync)    sync.BeginWrite(__FILE__, __LINE__, __FUNCTION__)
	#define SYNC_TRYBEGINWRITE(sync) sync.TryBeginWrite(__FILE__, __LINE__, __FUNCTION__)
	#define SYNC_ENDWRITE(sync)      sync.EndWrite  (__FILE__, __LINE__, __FUNCTION__)
#else
	#define SYNC_BEGINREAD(sync)     sync.BeginRead ()
	#define SYNC_ENDREAD(sync)       sync.EndRead   ()
	#define SYNC_BEGINWRITE(sync)    sync.BeginWrite()
	#define SYNC_TRYBEGINWRITE(sync) sync.TryBeginWrite()
	#define SYNC_ENDWRITE(sync)      sync.EndWrite  ()
#endif


class CMultiReadExclusiveWriteSynchronizer
{
private:
	volatile long m_Sentinel;
	HANDLE m_ReadSignal;
	HANDLE m_WriteSignal;
	unsigned int m_WriteRecursion;

	lockfree::hash_map<DWORD, unsigned int> tls;
	typedef lockfree::hash_map<DWORD, unsigned int>::iterator TThreadStorage;

	unsigned long m_WriterID;
	volatile long m_Revision;
	volatile long m_Waiting;

	void BlockReaders();
	void UnblockReaders();
	void UnblockOneWriter();
	void WaitForReadSignal();
	void WaitForWriteSignal();

#if defined(MREW_DO_DEBUG_LOGGING) && (defined(DEBUG) || defined(_DEBUG))
	HANDLE m_Log;
	void  DoLog(char * Desc, char * File, int Line, char * Function);

#endif


public:
	CMultiReadExclusiveWriteSynchronizer();
	virtual ~CMultiReadExclusiveWriteSynchronizer();

#if defined(MREW_DO_DEBUG_LOGGING) && (defined(DEBUG) || defined(_DEBUG))
	void BeginRead    (char * File, int Line, char * Function);
	void EndRead      (char * File, int Line, char * Function);
	bool BeginWrite   (char * File, int Line, char * Function);
	bool TryBeginWrite(char * File, int Line, char * Function);
	void EndWrite     (char * File, int Line, char * Function);
#endif
	void BeginRead();
	void EndRead();
	bool BeginWrite();
	bool TryBeginWrite();
	bool EndWrite();

	long Waiting() {return m_Waiting;};
	unsigned int WriteRecursionCount() {return m_WriteRecursion;};

};
