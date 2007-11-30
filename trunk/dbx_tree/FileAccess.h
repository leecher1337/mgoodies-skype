#pragma once

#include "Cipher.h"
#include <windows.h>

class CFileAccess
{
protected:
	char * m_FileName;
	CCipher * m_Cipher;
	unsigned int m_EncryptionStart;

	virtual unsigned int mRead(void* Buf, unsigned int Source, unsigned int Size) = 0;
  virtual unsigned int mWrite(void* Buf, unsigned int Dest, unsigned int Size) = 0;
	virtual unsigned int mAlloc(unsigned int Size) = 0;
	virtual void mFree(unsigned int Dest, unsigned int Size) = 0;


public:
	CFileAccess(const char* FileName);
	virtual ~CFileAccess();

	void SetCipher(CCipher * Cipher, unsigned int EncryptionStart);

	unsigned int Read(void* Buf, unsigned int Source, unsigned int Size);
  unsigned int Write(void* Buf, unsigned int Dest, unsigned int Size);
	//unsigned int Move(unsigned int Source, unsigned int Dest, unsigned int Size);

	unsigned int Alloc(unsigned int Size);
	void Free(unsigned int Dest, unsigned int Count);
};
