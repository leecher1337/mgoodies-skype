#pragma once

#include <windows.h>
#include "FileAccess.h"

class CDirectAccess :	public CFileAccess
{
private:
	unsigned int m_Size;
	
	HANDLE m_File;
protected:
	uint32_t mRead(void* Buf, uint32_t Source, uint32_t Size);
  uint32_t mWrite(void* Buf, uint32_t Dest, uint32_t Size);	
public:
	CDirectAccess(const char* FileName);
	virtual ~CDirectAccess();

	uint32_t SetAllocationSize(uint32_t Size);
};
