#pragma once

#include "MREWSync.h"

class CFileAccess
{
protected:
	CMultiReadExclusiveWriteSynchronizer m_Sync;
public:
	CFileAccess();
	~CFileAccess();

	virtual unsigned int Read(void* Buf, unsigned int Source, unsigned int Size) = 0;
  virtual unsigned int Write(void* Buf, unsigned int Dest, unsigned int Size) = 0;
	virtual unsigned int Move(unsigned int Source, unsigned int Dest, unsigned int Size) = 0;

	virtual unsigned int Alloc(unsigned int Size) = 0;
	virtual void Free(unsigned int Dest, unsigned int Count) = 0;

	virtual void BeginRead(); 
	virtual void EndRead();
	virtual bool BeginWrite();
	virtual void EndWrite();
};
