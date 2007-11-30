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
protected:
	unsigned int mRead(void* Buf, unsigned int Source, unsigned int Size);
  unsigned int mWrite(void* Buf, unsigned int Dest, unsigned int Size);	
	unsigned int mAlloc(unsigned int Size);
	void mFree(unsigned int Dest, unsigned int Count);
public:
	CDirectAccess(const char* FileName);
	virtual ~CDirectAccess();
};
