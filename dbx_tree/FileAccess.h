#pragma once

#include <windows.h>
#include "stdint.h"
#include "Cipher.h"

class CFileAccess
{
protected:
	char * m_FileName;
	CCipher * m_Cipher;
	unsigned int m_EncryptionStart;

	virtual uint32_t mRead(void* Buf, uint32_t Source, uint32_t Size) = 0;
  virtual uint32_t mWrite(void* Buf, uint32_t Dest, uint32_t Size) = 0;

public:
	CFileAccess(const char* FileName);
	virtual ~CFileAccess();

	void SetCipher(CCipher * Cipher);
	void SetEncryptionStart(uint32_t EncryptionStart);

	uint32_t Read(void* Buf, uint32_t Source, uint32_t Size);
  uint32_t Write(void* Buf, uint32_t Dest, uint32_t Size);
	virtual uint32_t SetAllocationSize(uint32_t Size) = 0;
};
