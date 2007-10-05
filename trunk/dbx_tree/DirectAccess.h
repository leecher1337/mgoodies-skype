#pragma once

#include <windows.h>
#include <map>
#include "FileAccess.h"

class CDirectAccess :	public CFileAccess
{
private:
	typedef std::multimap<unsigned int, unsigned int> TFreeSpaceMap;	
	TFreeSpaceMap m_FreeSpace;

	unsigned int m_Size;
	
	HANDLE m_File;
public:
	CDirectAccess(const char* FileName);
	~CDirectAccess();
	
	
	unsigned int Read(void* Buf, unsigned int Source, unsigned int Size);
  unsigned int Write(void* Buf, unsigned int Dest, unsigned int Size);
	unsigned int Move(unsigned int Source, unsigned int Dest, unsigned int Size);

	unsigned int Alloc(unsigned int Size);
	void Free(unsigned int Dest, unsigned int Count);
};
