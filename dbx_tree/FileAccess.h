#pragma once

#include <windows.h>
#include "stdint.h"
#include "Cipher.h"

class CFileAccess
{
protected:
	char * m_FileName;
	CCipher * m_Cipher;
	
	uint32_t m_Size;
	uint32_t m_AllocSize;
	uint32_t m_AllocGranularity;
	uint32_t m_EncryptionStart;
	bool m_ReadOnly;

	virtual uint32_t mRead(void* Buf, uint32_t Source, uint32_t Size) = 0;
  virtual uint32_t mWrite(void* Buf, uint32_t Dest, uint32_t Size) = 0;
	virtual uint32_t mSetSize(uint32_t Size) = 0;

public:
	CFileAccess(const char* FileName);
	virtual ~CFileAccess();

	void SetCipher(CCipher * Cipher);
	void SetEncryptionStart(uint32_t EncryptionStart);

	uint32_t Read(void* Buf, uint32_t Source, uint32_t Size);
  uint32_t Write(void* Buf, uint32_t Dest, uint32_t Size);
	virtual uint32_t SetSize(uint32_t Size);
	virtual uint32_t GetSize();
	void SetReadOnly(bool ReadOnly);
	bool GetReadOnly();
};
