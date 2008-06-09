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
	virtual ~CThreadLocalCounter();

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
