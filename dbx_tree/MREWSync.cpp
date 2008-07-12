#include "MREWSync.h"
#include <assert.h>
#include <stdio.h>

#include <intrin.h>

#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)
#pragma intrinsic (_InterlockedExchange)
#pragma intrinsic (_InterlockedExchangeAdd)




#define mrWRITEREQUEST 0x10000
#define ALIVE          0x7FFFFFFF


CThreadLocalCounter::CThreadLocalCounter()
{
	int i;
	for (i = 0; i < cTLCHASHTABLESIZE; i++)
		m_HashTable[i] = NULL;

}
CThreadLocalCounter::~CThreadLocalCounter()
{
	PThreadInfo p, q;
	int i;

	for (i = 0; i < cTLCHASHTABLESIZE; i++)
	{
		p = m_HashTable[i];
		m_HashTable[i] = NULL;

		while (p)
		{
			q = p;
			p = p->Next;
			delete q;
		}
	}
}

unsigned char CThreadLocalCounter::HashIndex()
{
	unsigned long h = GetCurrentThreadId();	
	return (cTLCHASHTABLESIZE - 1) & (h ^ (h >> 8));
}
CThreadLocalCounter::PThreadInfo CThreadLocalCounter::Recycle()
{
	int gen;
	PThreadInfo res = m_HashTable[HashIndex()];
	while (res)
	{
		gen = _InterlockedExchange((long*)&res->Active, (long)ALIVE);

		if (gen != ALIVE)
		{
			res->ThreadID = GetCurrentThreadId();
			return res;
		} else {
			res = res->Next;
		}
	}

	return res;
}

void CThreadLocalCounter::Open(PThreadInfo & Thread)
{
	unsigned char h = HashIndex();
	PThreadInfo p = m_HashTable[h];
	unsigned long curthread = GetCurrentThreadId();

	while ((p) && (p->ThreadID != curthread))
		p = p->Next;

	if (!p)
	{
		p = Recycle();
		if (!p)
		{
			p = new TThreadInfo;
			
			p->ThreadID = curthread;
			p->Active = ALIVE;
			p->RecursionCount = 0;
			p->Next = p;
			p->Next = (PThreadInfo)_InterlockedExchange((long*)&m_HashTable[h], (long)p);
		}
	}

	Thread = p;
}

void CThreadLocalCounter::Delete(PThreadInfo & Thread)
{
	Thread->ThreadID = 0;
	Thread->Active = 0;
}



CMultiReadExclusiveWriteSynchronizer::CMultiReadExclusiveWriteSynchronizer(void)
: tls()
{
	m_Sentinel = mrWRITEREQUEST;
	m_ReadSignal = CreateEvent(NULL, true, true, NULL);
	m_WriteSignal = CreateEvent(NULL, false, false, NULL);
	m_WaitRecycle = INFINITE;
	m_WriteRecursionCount = 0;
	m_WriterID = 0;
	m_RevisionLevel = 0;
#if defined(MREW_DO_DEBUG_LOGGING) && (defined(DEBUG) || defined(_DEBUG))
	m_Log = CreateFileA("dbx_treeSync.log", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
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
	CThreadLocalCounter::PThreadInfo thread;
	bool wasrecursive;
	int sentvalue;

	tls.Open(thread);
	thread->RecursionCount++;
	wasrecursive = thread->RecursionCount > 1;

	if (m_WriterID != GetCurrentThreadId())
	{
		if (!wasrecursive)
		{
			WaitForReadSignal();

			while (_InterlockedDecrement((long*)&m_Sentinel) <= 0)
			{
				sentvalue = _InterlockedIncrement((long*)&m_Sentinel);

				if (sentvalue == mrWRITEREQUEST)
					UnblockOneWriter();

				Sleep(0);

				WaitForReadSignal();
			}
		}
	}
}
void CMultiReadExclusiveWriteSynchronizer::EndRead()
{
	CThreadLocalCounter::PThreadInfo thread;
	int test;

	tls.Open(thread);
	thread->RecursionCount--;

	if (thread->RecursionCount == 0)
	{
		tls.Delete(thread);
		
		if (m_WriterID != GetCurrentThreadId())
		{
			test = _InterlockedIncrement((long*)&m_Sentinel);

			if ((test & (mrWRITEREQUEST - 1)) == 0)
			{
				UnblockOneWriter();
			}
		}
	}

}
bool CMultiReadExclusiveWriteSynchronizer::BeginWrite()
{
	bool res = true;
	CThreadLocalCounter::PThreadInfo thread;
	bool hasreadlock;
	DWORD threadid = GetCurrentThreadId();
	int test;
	long oldrevisionlevel;

	if (m_WriterID != threadid) 
	{
		BlockReaders();

		oldrevisionlevel = m_RevisionLevel;

		tls.Open(thread);

		hasreadlock = thread->RecursionCount > 0;
		if (hasreadlock)
			_InterlockedIncrement(&m_Sentinel);

		while (_InterlockedExchangeAdd(&m_Sentinel, -mrWRITEREQUEST) != mrWRITEREQUEST)
		{
			test = _InterlockedExchangeAdd(&m_Sentinel, mrWRITEREQUEST);
			if (test > 0)
				WaitForWriteSignal();
		}

		BlockReaders();

		if (hasreadlock)
			_InterlockedDecrement(&m_Sentinel);

		m_WriterID = threadid;

		res = (oldrevisionlevel == (_InterlockedIncrement(&m_RevisionLevel) - 1));
	}

	m_WriteRecursionCount++;

	return res;
}
void CMultiReadExclusiveWriteSynchronizer::EndWrite()
{
	CThreadLocalCounter::PThreadInfo thread;

	assert(m_WriterID == GetCurrentThreadId());

	tls.Open(thread);
	m_WriteRecursionCount--;
	if (m_WriteRecursionCount == 0)
	{
		m_WriterID = 0;
		_InterlockedExchangeAdd(&m_Sentinel, mrWRITEREQUEST);
		UnblockOneWriter();
		
		UnblockReaders();
	}

	if (thread->RecursionCount == 0)
		tls.Delete(thread);

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
	WaitForSingleObject(m_ReadSignal, m_WaitRecycle);
}
void CMultiReadExclusiveWriteSynchronizer::WaitForWriteSignal()
{
	WaitForSingleObject(m_WriteSignal, m_WaitRecycle);
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
void CMultiReadExclusiveWriteSynchronizer::EndWrite  (char * File, int Line, char * Function)
{
	DoLog("EndWrite  ", File, Line, Function);
	EndWrite();
}


void CMultiReadExclusiveWriteSynchronizer::DoLog(char * Desc, char * File, int Line, char * Function)
{
	char buf [1024];
	int l = sprintf_s(buf, "%08x - %s from \"%s\" %d (%s)\n", GetCurrentThreadId(), Desc, File, Line, Function);
	DWORD read = 0;

	WriteFile(m_Log, buf, l, &read, NULL);
}
#endif
