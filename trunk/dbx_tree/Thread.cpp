#include "Thread.h"

#include <intrin.h>

#pragma intrinsic (_InterlockedExchange)

unsigned int __stdcall ThreadDistributor(void* Param)
{
	CThread * thread = static_cast<CThread *>(Param);
	DWORD result = thread->Wrapper();
	_endthreadex(result);
	return result;
}

CThread::CThread(bool CreateSuspended)
{
	m_Handle = NULL;
	m_Terminated = 0;
	m_FreeOnTerminate = false;
	m_Finished = false;
	m_Suspended = CreateSuspended;
	m_ReturnValue = 0;
	unsigned int flags = 0;
	if (CreateSuspended)
		flags = CREATE_SUSPENDED;
	
	m_Handle = reinterpret_cast<HANDLE> (_beginthreadex(NULL, 0, &ThreadDistributor, this, flags, &m_ThreadID));
}
CThread::~CThread()
{
	if (!m_Finished && !m_Suspended)
	{
		Terminate();
		WaitFor();
	}
	if (m_Handle)
		CloseHandle(m_Handle);
}
DWORD CThread::Wrapper()
{
	__try {
    Execute();
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		ReturnValue(GetExceptionCode());
	}
  bool dofree = FreeOnTerminate();
  DWORD result = ReturnValue();
	m_Finished = true;

	m_sigTerminate(this);
  if (dofree)
    delete this;
	
	return result;
}

void CThread::Resume()
{
	if (ResumeThread(m_Handle) == 1)
		_InterlockedExchange(&m_Suspended, 0);
}
void CThread::Suspend()
{
	SuspendThread(m_Handle);
	_InterlockedExchange(&m_Suspended, 1);
}
void CThread::Terminate()
{
	_InterlockedExchange(&m_Terminated, 1);
}
DWORD CThread::WaitFor()
{
	HANDLE tmp = m_Handle;
	DWORD result = WAIT_FAILED;

	if (WaitForSingleObject(m_Handle, INFINITE) != WAIT_FAILED)
		GetExitCodeThread(tmp, &result);

	return result;
}

void CThread::FreeOnTerminate(bool Terminate)
{
	_InterlockedExchange(&m_FreeOnTerminate, Terminate);
}
void CThread::ReturnValue(DWORD Value)
{
	_InterlockedExchange(&m_ReturnValue, Value);
}

void CThread::Priority(TPriority NewPriority)
{
	SetThreadPriority(m_Handle, NewPriority);
}
CThread::TPriority CThread::Priority()
{
	return static_cast<TPriority> (GetThreadPriority(m_Handle) & 0xffff);
}