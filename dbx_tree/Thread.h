#pragma once

#include <process.h>
#include <windows.h>
#include "sigslot.h"

class CThread
{
private:
	long         m_Terminated;
	long         m_FreeOnTerminate;
	long         m_Finished;
	long         m_Suspended;
	long         m_ReturnValue;
protected:
	HANDLE       m_Handle;
	unsigned int m_ThreadID;

	void ReturnValue(DWORD Value);
	virtual void Execute() = 0;
public:
	CThread(bool CreateSuspended);
	virtual ~CThread();

	DWORD Wrapper();

	void Resume();
	void Suspend();
	void Terminate();
	DWORD WaitFor();
	
	bool Suspended() {return m_Suspended != 0;};
	bool Terminated() {return m_Terminated != 0;};
	bool FreeOnTerminate() {return m_FreeOnTerminate != 0;};
	void FreeOnTerminate(bool Terminate);
	DWORD ReturnValue() {return m_ReturnValue;};

	typedef enum TPriority {
		tpIdle = THREAD_PRIORITY_IDLE,
		tpLowest = THREAD_PRIORITY_LOWEST,
		tpLower = THREAD_PRIORITY_BELOW_NORMAL,
		tpNormal = THREAD_PRIORITY_NORMAL,
		tpHigher = THREAD_PRIORITY_ABOVE_NORMAL,
		tpHighest = THREAD_PRIORITY_HIGHEST,
		tpRealTime = THREAD_PRIORITY_TIME_CRITICAL
	} TPriority;

	void Priority(TPriority NewPriority);
	TPriority Priority();

	typedef sigslot::signal1<CThread *> TOnTerminate;
	TOnTerminate m_sigTerminate;
	TOnTerminate & sigTerminate() {return m_sigTerminate;};
};
