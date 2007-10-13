#pragma once

#include "Cipher.h"
#include <windows.h>

class CFileAccess
{
protected:
	char * m_FileName;
	CCipher * m_Cipher;

	virtual unsigned int mRead(void* Buf, unsigned int Source, unsigned int Size) = 0;
  virtual unsigned int mWrite(void* Buf, unsigned int Dest, unsigned int Size) = 0;

public:
	CFileAccess(const char* FileName);
	virtual ~CFileAccess();

	void SetCipher(CCipher * Cipher);

	unsigned int Read(void* Buf, unsigned int Source, unsigned int Size);
  unsigned int Write(void* Buf, unsigned int Dest, unsigned int Size);
	//unsigned int Move(unsigned int Source, unsigned int Dest, unsigned int Size);

	virtual unsigned int Alloc(unsigned int Size) = 0;
	virtual void Free(unsigned int Dest, unsigned int Count) = 0;
};
