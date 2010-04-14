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

#include "MREWSync.h"
#include <assert.h>

#if defined(MREW_DO_DEBUG_LOGGING) && (defined(DEBUG) || defined(_DEBUG))
	#include <stdio.h>
#endif

#ifdef _MSC_VER
#include <intrin.h>

#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)
#pragma intrinsic (_InterlockedExchangeAdd)
#pragma intrinsic (_InterlockedCompareExchange)
#else
#include "intrin_gcc.h"
#endif

#define WRITEREQUEST 0x10000
#define SOMEONEHASLOCK(Sentinel) ((Sentinel & (WRITEREQUEST - 1)) != 0)

CMultiReadExclusiveWriteSynchronizer::CMultiReadExclusiveWriteSynchronizer(void)
: tls()
{
	m_Sentinel = WRITEREQUEST;
	m_ReadSignal = CreateEvent(NULL, true, true, NULL);
	m_WriteSignal = CreateEvent(NULL, false, true, NULL);
	m_WriteRecursion = 0;
	m_WriterID = 0;
	m_Revision = 0;
	m_Waiting = 0;
#if defined(MREW_DO_DEBUG_LOGGING) && (defined(DEBUG) || defined(_DEBUG))
	char fn[MAX_PATH];
	sprintf_s(fn, "dbx_treeSync%08x.log", this);
	m_Log = CreateFileA(fn, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
#endif

}

CMultiReadExclusiveWriteSynchronizer::~CMultiReadExclusiveWriteSynchronizer(void)
{
	BeginWrite();
	CloseHandle(m_ReadSignal);
	CloseHandle(m_WriteSignal);
#if defined(MREW_DO_DEBUG_LOGGING) && (defined(DEBUG) || defined(_DEBUG))
	CloseHandle(m_Log);
#endif

}

void CMultiReadExclusiveWriteSynchronizer::BeginRead()
{
	unsigned long id = GetCurrentThreadId();
	TThreadStorage data = tls.insert(std::make_pair(id, 0)).first;

	data->second++;
	if ((m_WriterID != id) && (data->second == 1))
	{
		_InterlockedIncrement(&m_Waiting);
		while (_InterlockedDecrement(&m_Sentinel) <= 0)
		{
			if (!SOMEONEHASLOCK(_InterlockedIncrement(&m_Sentinel)))
				UnblockOneWriter();
			WaitForReadSignal();
		}

		_InterlockedDecrement(&m_Waiting);
	}
}
void CMultiReadExclusiveWriteSynchronizer::EndRead()
{
	unsigned long id = GetCurrentThreadId();
	TThreadStorage data = tls.insert(std::make_pair(id, 0)).first;

	data->second--;
	if ((data->second == 0) && (m_WriterID != id))
	{
		if (!SOMEONEHASLOCK(_InterlockedIncrement(&m_Sentinel)))
			UnblockOneWriter();
		tls.erase(data);
	}
}
bool CMultiReadExclusiveWriteSynchronizer::BeginWrite()
{
	unsigned long id = GetCurrentThreadId();
	bool res = true;
	
	if (m_WriterID != id)
	{
		TThreadStorage & data = tls.insert(std::make_pair(id, 0)).first;

		long oldrevision = m_Revision;
		bool hasreadlock = data->second > 0;
		
		if (! ((!hasreadlock && (_InterlockedCompareExchange(&m_Sentinel, -1, WRITEREQUEST) == WRITEREQUEST))
			  || ( hasreadlock && (_InterlockedCompareExchange(&m_Sentinel, -1, WRITEREQUEST - 1) == WRITEREQUEST - 1))))
		{

			_InterlockedIncrement(&m_Waiting);
			if (!hasreadlock || (hasreadlock && SOMEONEHASLOCK(_InterlockedIncrement(&m_Sentinel))))
				WaitForWriteSignal();
			
			if (SOMEONEHASLOCK(_InterlockedExchangeAdd(&m_Sentinel, -WRITEREQUEST - 1)))
			{
				do
				{
					BlockReaders();
					if (SOMEONEHASLOCK(_InterlockedIncrement(&m_Sentinel)))
						WaitForWriteSignal();

				} while (SOMEONEHASLOCK(_InterlockedDecrement(&m_Sentinel) + 1));
			}

			_InterlockedDecrement(&m_Waiting);
		}

		BlockReaders();
		m_WriterID = id;

		res = (oldrevision == (_InterlockedIncrement(&m_Revision) - 1));
	}
	m_WriteRecursion++;
	return res;
}

bool CMultiReadExclusiveWriteSynchronizer::TryBeginWrite()
{
	unsigned long id = GetCurrentThreadId();

	if (m_WriterID != id)
	{
		TThreadStorage data = tls.insert(std::make_pair(id, 0)).first;

		bool hasreadlock = data->second > 0;

		if (! ((!hasreadlock && (_InterlockedCompareExchange(&m_Sentinel, -1, WRITEREQUEST) == WRITEREQUEST))
			  || ( hasreadlock && (_InterlockedCompareExchange(&m_Sentinel, -1, WRITEREQUEST - 1) == WRITEREQUEST - 1))))
		{
			return false;
		}

		BlockReaders();
		m_WriterID = id;

		_InterlockedIncrement(&m_Revision);
	}
	m_WriteRecursion++;
	
	return true;
}
bool CMultiReadExclusiveWriteSynchronizer::EndWrite()
{
	unsigned long id = GetCurrentThreadId();
	assert(m_WriterID == id);

	m_WriteRecursion--;
	if (m_WriteRecursion == 0)
	{
		TThreadStorage data = tls.insert(std::make_pair(id, 0)).first;

		m_WriterID = 0;
		if (data->second == 0)
		{
			_InterlockedExchangeAdd(&m_Sentinel, WRITEREQUEST + 1);
			UnblockReaders();
			UnblockOneWriter();

			tls.erase(data);
		} else {
			_InterlockedExchangeAdd(&m_Sentinel, WRITEREQUEST);
			UnblockReaders();

		}

		return true;
	}

	return false;
}


void CMultiReadExclusiveWriteSynchronizer::BlockReaders()
{
	ResetEvent(m_ReadSignal);
}
void CMultiReadExclusiveWriteSynchronizer::UnblockReaders()
{
	SetEvent(m_ReadSignal);
}
void CMultiReadExclusiveWriteSynchronizer::UnblockOneWriter()
{
	SetEvent(m_WriteSignal);
}
void CMultiReadExclusiveWriteSynchronizer::WaitForReadSignal()
{
	WaitForSingleObject(m_ReadSignal, INFINITE);
}
void CMultiReadExclusiveWriteSynchronizer::WaitForWriteSignal()
{
	WaitForSingleObject(m_WriteSignal, INFINITE);
}

#if defined(MREW_DO_DEBUG_LOGGING) && (defined(DEBUG) || defined(_DEBUG))

void CMultiReadExclusiveWriteSynchronizer::BeginRead (char * File, int Line, char * Function)
{
	//DoLog("BeginRead ", File, Line, Function);
	BeginRead();
}
void CMultiReadExclusiveWriteSynchronizer::EndRead   (char * File, int Line, char * Function)
{
	//DoLog("EndRead   ", File, Line, Function);
	EndRead();
}
bool CMultiReadExclusiveWriteSynchronizer::BeginWrite(char * File, int Line, char * Function)
{
	bool res = BeginWrite();
	DoLog("BeginWrite", File, Line, Function);
	return res;
}
bool CMultiReadExclusiveWriteSynchronizer::TryBeginWrite(char * File, int Line, char * Function)
{
	bool res = TryBeginWrite();
	DoLog("TryBeginWrite", File, Line, Function);
	return res;
}
void CMultiReadExclusiveWriteSynchronizer::EndWrite  (char * File, int Line, char * Function)
{
	DoLog("EndWrite  ", File, Line, Function);
	EndWrite();
}


void CMultiReadExclusiveWriteSynchronizer::DoLog(char * Desc, char * File, int Line, char * Function)
{
	char buf [1024];
	int l = sprintf_s(buf, "%08x %08x - %s from \"%s\" %d (%s)\n", this, GetCurrentThreadId(), Desc, File, Line, Function);
	DWORD read = 0;

	WriteFile(m_Log, buf, l, &read, NULL);
}
#endif
