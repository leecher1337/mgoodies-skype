#pragma once

#include <windows.h>


#define cTLCHASHTABLESIZE 16

class CThreadLocalCounter
{
public:
	typedef struct TThreadInfo {
			TThreadInfo* Next;  //must be first elem - look at CThreadLocalCounter::Delete
			TThreadInfo* NextDead;
			unsigned int ThreadID;
			unsigned int RecursionCount;
		}	TThreadInfo, *PThreadInfo;
private:
	PThreadInfo m_HashTable[cTLCHASHTABLESIZE];
	PThreadInfo m_Purgatory;
	long m_OpenCount;

	int HashIndex();
	PThreadInfo Recycle();
	void Reattach(PThreadInfo List);
protected:
	unsigned int m_HoldTime;

public:
	CThreadLocalCounter();
	~CThreadLocalCounter();

	void Open(PThreadInfo & Thread);
	void Delete(PThreadInfo & Thread);
	void Close(PThreadInfo & Thread);


};

class CMultiReadExclusiveWriteSynchronizer
{
private:
	CThreadLocalCounter tls;

	long m_Sentinel;
	HANDLE m_ReadSignal;
	HANDLE m_WriteSignal;
	unsigned int m_WaitRecycle;
	unsigned int m_WriteRecursionCount;

	unsigned int m_WriterID;
	long m_RevisionLevel;

	void BlockReaders();
	void UnblockReaders();
	void UnblockOneWriter();
	void WaitForReadSignal();
	void WaitForWriteSignal();
public:
	CMultiReadExclusiveWriteSynchronizer();
	~CMultiReadExclusiveWriteSynchronizer();

	void BeginRead();
	void EndRead();
	bool BeginWrite();
	void EndWrite();
};
