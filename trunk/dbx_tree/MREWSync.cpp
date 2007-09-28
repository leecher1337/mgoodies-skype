#include "MREWSync.h"
#include <assert.h>

#define mrWRITEREQUEST 0xFFFF


CThreadLocalCounter::CThreadLocalCounter()
{
	int i;
	for (i = 0; i < cTLCHASHTABLESIZE; i++)
		m_HashTable[i] = NULL;

	m_Purgatory = NULL;
	m_OpenCount = 0;

	m_HoldTime = 60 * 1000; //1 minute
}
CThreadLocalCounter::~CThreadLocalCounter()
{
	PThreadInfo p, q;
	int i;

	assert(m_OpenCount == 0);

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

	p = m_Purgatory;
	m_Purgatory = NULL;
	
	while (p)
	{
		q = p;
		p = p->NextDead;
		delete q;
	}
}

int CThreadLocalCounter::HashIndex()
{
	unsigned long h = GetCurrentThreadId();	
	return (cTLCHASHTABLESIZE - 1) & (h ^ (h >> 8));
}
CThreadLocalCounter::PThreadInfo CThreadLocalCounter::Recycle()
{
	PThreadInfo head, p, q;
	head = (PThreadInfo) InterlockedExchange((long*)&m_Purgatory, 0);
	p = head;
	q = head;
	while ((p) && (p->ThreadID != 0))
	{
		q = p;
		p = p->NextDead;
	}

	if (p) 
	{
		if (p == head)
		{
			head = p->NextDead;
		} else {
			q->NextDead = p->NextDead;
		}
	}

	Reattach(head);
	return p;
}
void CThreadLocalCounter::Reattach(PThreadInfo List)
{
	PThreadInfo p;
	if (List)
	{
		p = List;
		while (p->NextDead) p = p->NextDead;
		p->NextDead = List;
		p->NextDead = (PThreadInfo) InterlockedExchange((long*)&m_Purgatory, (long)List);
	}
}

void CThreadLocalCounter::Open(PThreadInfo & Thread)
{
	PThreadInfo p;
	int h;
	unsigned long curthread;

	InterlockedIncrement(&m_OpenCount);
	h = HashIndex();
	p = m_HashTable[h];
	curthread = GetCurrentThreadId();

	while ((p) && (p->ThreadID != curthread))
		p = p->Next;

	if (!p)
	{
		p = Recycle();
		if (!p)
			p = new TThreadInfo;
			
		memset(p, 0, sizeof(TThreadInfo));
		p->ThreadID = curthread;
		p->Next = p;
		p->Next = (PThreadInfo)InterlockedExchange((long*)&m_HashTable[h], (long)p);
	}

	Thread = p;
}
void CThreadLocalCounter::Delete(PThreadInfo & Thread)
{
	PThreadInfo p;

	assert(m_OpenCount > 0);
	assert(Thread);

	p = (PThreadInfo) &(m_HashTable[HashIndex()]); //cheating around
	while ((p->Next) && (p->Next != Thread))
		p = p->Next;

	assert(p->Next == Thread);

	InterlockedExchange((long*)&p->Next, (long)Thread->Next);
	Thread->NextDead = Thread;
	Thread->NextDead = (PThreadInfo) InterlockedExchange((long*)&m_Purgatory, (long)Thread);
}
void CThreadLocalCounter::Close(PThreadInfo & Thread)
{
	PThreadInfo p, q, head;
	PThreadInfo* trail;
	unsigned long timestamp;

	assert(m_OpenCount > 0);

	head = (PThreadInfo)InterlockedExchange((long*)&m_Purgatory,0);
	if (InterlockedDecrement(&m_OpenCount) == 0)
	{
		p = head;
		trail = &head;
		timestamp = GetTickCount();

		while (p)
		{
			q = p;
			p = p->NextDead;
			
			if (q->ThreadID = 0)
			{
				q->ThreadID = 0;
				q->RecursionCount = timestamp;

				trail = &(q->NextDead);
			}
			else if ((timestamp < q->RecursionCount) || (timestamp - q->RecursionCount > m_HoldTime))
			{
				delete q;
				(*trail) = p;
			} else {
				trail = &(q->NextDead);
			}
		}
	}

	Reattach(head);
	Thread = NULL;
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
}

CMultiReadExclusiveWriteSynchronizer::~CMultiReadExclusiveWriteSynchronizer(void)
{
	BeginWrite();
	CloseHandle(m_ReadSignal);
	CloseHandle(m_WriteSignal);
}

void CMultiReadExclusiveWriteSynchronizer::BeginRead()
{
	CThreadLocalCounter::PThreadInfo thread;
	bool diddec = false;

	if (m_WriterID != GetCurrentThreadId())
	{
		while (InterlockedDecrement(&m_Sentinel) <= 0)
		{
			InterlockedIncrement(&m_Sentinel);

			WaitForReadSignal();
		}

		diddec = true;
	}

	tls.Open(thread);
	thread->RecursionCount++;

	if ((thread->RecursionCount > 1) && diddec)
		InterlockedIncrement(&m_Sentinel);

	tls.Close(thread);	
}
void CMultiReadExclusiveWriteSynchronizer::EndRead()
{
	CThreadLocalCounter::PThreadInfo thread;

	tls.Open(thread);
	thread->RecursionCount--;

	if (thread->RecursionCount == 0)
	{
		tls.Delete(thread);
		if ((m_WriterID != GetCurrentThreadId()) && (InterlockedIncrement(&m_Sentinel) == mrWRITEREQUEST))
			UnblockOneWriter();
	}

	tls.Close(thread);
}
bool CMultiReadExclusiveWriteSynchronizer::BeginWrite()
{
	bool res = true;
	CThreadLocalCounter::PThreadInfo thread;
	bool hasreadlock;
	DWORD threadid = GetCurrentThreadId();
	int test;
	unsigned int oldrevisionlevel;

	if (m_WriterID != threadid)
	{
		BlockReaders();

		oldrevisionlevel = m_RevisionLevel;

		tls.Open(thread);

		hasreadlock = thread->RecursionCount > 0;
		if (hasreadlock)
			InterlockedIncrement(&m_Sentinel);

		tls.Close(thread);

		while ((InterlockedExchangeAdd(&m_Sentinel, -mrWRITEREQUEST) - mrWRITEREQUEST) != 0)
		{
			test = InterlockedExchangeAdd(&m_Sentinel, mrWRITEREQUEST) + mrWRITEREQUEST;
			if (test > 0)
				WaitForWriteSignal();
		}

		if (hasreadlock)
			InterlockedDecrement(&m_Sentinel);

		m_WriterID = threadid;

		res = (oldrevisionlevel == InterlockedIncrement(&m_RevisionLevel) - 1);

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
		InterlockedExchangeAdd(&m_Sentinel, mrWRITEREQUEST);
		UnblockOneWriter();
		Sleep(0); //prefer Writers: give the chance to sneak in, before readers can
		UnblockReaders();
	}

	if (thread->RecursionCount == 0)
		tls.Delete(thread);

	tls.Close(thread);
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