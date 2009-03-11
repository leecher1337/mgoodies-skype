/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2009 Michael "Protogenes" Kunz,

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

//#define MREW_DO_DEBUG_LOGGING 1

#define cTLCHASHTABLESIZE 16

#if defined(MREW_DO_DEBUG_LOGGING) && (defined(DEBUG) || defined(_DEBUG))
	#define SYNC_BEGINREAD(sync)  sync.BeginRead (__FILE__, __LINE__, __FUNCTION__)
	#define SYNC_ENDREAD(sync)    sync.EndRead   (__FILE__, __LINE__, __FUNCTION__)
	#define SYNC_BEGINWRITE(sync) sync.BeginWrite(__FILE__, __LINE__, __FUNCTION__)
	#define SYNC_ENDWRITE(sync)   sync.EndWrite  (__FILE__, __LINE__, __FUNCTION__)
#else
	#define SYNC_BEGINREAD(sync)  sync.BeginRead ()
	#define SYNC_ENDREAD(sync)    sync.EndRead   ()
	#define SYNC_BEGINWRITE(sync) sync.BeginWrite()
	#define SYNC_ENDWRITE(sync)   sync.EndWrite  ()
#endif



class CThreadLocalCounter
{
public:
	typedef struct TThreadInfo {
			TThreadInfo* Next;
			unsigned int ThreadID;
			int          Active;
			unsigned int RecursionCount;
		}	TThreadInfo, *PThreadInfo;
private:
	PThreadInfo m_HashTable[cTLCHASHTABLESIZE];

	unsigned char HashIndex();
	PThreadInfo Recycle();

public:
	CThreadLocalCounter();
	virtual ~CThreadLocalCounter();

	void Open(PThreadInfo & Thread);
	void Delete(PThreadInfo & Thread);


};

class CMultiReadExclusiveWriteSynchronizer
{
private:
	long m_Sentinel;
	HANDLE m_ReadSignal;
	HANDLE m_WriteSignal;
	unsigned int m_WaitRecycle;
	unsigned int m_WriteRecursionCount;

	CThreadLocalCounter tls;

	unsigned int m_WriterID;
	long m_RevisionLevel;

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
	void BeginRead (char * File, int Line, char * Function);
	void EndRead   (char * File, int Line, char * Function);
	bool BeginWrite(char * File, int Line, char * Function);
	void EndWrite  (char * File, int Line, char * Function);
#endif
	void BeginRead();
	void EndRead();
	bool BeginWrite();
	void EndWrite();

};

class CSmallMREWSynchronizer 
{
private:
	volatile long m_Sentinel;
	volatile long m_RevisionLevel;
	volatile long m_ReadWaiting;
	
	HANDLE m_ReadSignal;
	HANDLE m_WriteSignal;
	unsigned int m_WaitRecycle;
	unsigned int m_WriterID;

	void BlockReaders();
	void UnblockReaders();
	void UnblockOneWriter();
	void WaitForReadSignal();
	void WaitForWriteSignal();
public:
	CSmallMREWSynchronizer();
	~CSmallMREWSynchronizer();

	void BeginRead();
	void EndRead();
	bool BeginWrite();
	void EndWrite();
	long ReadWaiting() {return m_ReadWaiting;};
};
