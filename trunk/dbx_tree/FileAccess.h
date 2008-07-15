#pragma once

#include <windows.h>
#include <time.h>
#include "stdint.h"
#include "EncryptionManager.h"
#include "Exception.h"
#include "sigslot.h"

class CFileAccess
{
public:
	CFileAccess(const char* FileName, CEncryptionManager & EncryptionManager, uint32_t EncryptionStart);
	virtual ~CFileAccess();

	uint32_t Read(void* Buf, uint32_t Source, uint32_t Size);
  uint32_t Write(void* Buf, uint32_t Dest, uint32_t Size);
	virtual uint32_t SetSize(uint32_t Size);
	virtual uint32_t GetSize();
	void SetReadOnly(bool ReadOnly);
	bool GetReadOnly();

	typedef sigslot::signal2<CFileAccess *, uint32_t> TOnFileSizeChanged;

	TOnFileSizeChanged & sigFileSizeChanged();

protected:
	char * m_FileName;
	CEncryptionManager & m_EncryptionManager;
	
	uint32_t m_Size;
	uint32_t m_AllocSize;
	uint32_t m_AllocGranularity;
	uint32_t m_MinAllocGranularity;
	uint32_t m_MaxAllocGranularity;
	uint32_t m_LastAllocTime;
	uint32_t m_EncryptionStart;
	bool m_ReadOnly;

	TOnFileSizeChanged m_sigFileSizeChanged;
	virtual uint32_t mRead(void* Buf, uint32_t Source, uint32_t Size) = 0;
  virtual uint32_t mWrite(void* Buf, uint32_t Dest, uint32_t Size) = 0;
	virtual uint32_t mSetSize(uint32_t Size) = 0;




};
