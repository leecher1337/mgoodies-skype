#pragma once

#include <windows.h>
#include "FileAccess.h"

class CMappedMemory : public CFileAccess
{
private:
	uint8_t* m_Base;
	uint32_t m_AllocSize;
	uint32_t m_AllocGranularity;

	HANDLE m_DirectFile;
	HANDLE m_FileMapping;
protected:
	
	uint32_t mRead(void* Buf, uint32_t Source, uint32_t Size);
  uint32_t mWrite(void* Buf, uint32_t Dest, uint32_t Size);	
public:
	CMappedMemory(const char* FileName);
	virtual ~CMappedMemory();

	uint32_t SetAllocationSize(uint32_t Size);
};
