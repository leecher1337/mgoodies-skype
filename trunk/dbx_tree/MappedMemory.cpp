#include "MappedMemory.h"

CMappedMemory::CMappedMemory(const char* FileName)
:	CFileAccess(FileName),
	m_FreeSpace()
{
	SYSTEM_INFO sysinfo;

	m_FreeFileEnd = m_FreeSpace.end();
	m_AllocSize = 0;
	m_Size = 0;
	m_DirectFile = 0;
	m_FileMapping = 0;
	
	GetSystemInfo(&sysinfo);
	m_AllocGranularity = sysinfo.dwAllocationGranularity;

	m_DirectFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
	if (m_DirectFile == INVALID_HANDLE_VALUE) 
		throw "CreateFile failed";

	m_AllocSize = GetFileSize(m_DirectFile, NULL);

	if (m_AllocSize % m_AllocGranularity > 0)
		m_AllocSize = m_AllocSize - m_AllocSize % m_AllocGranularity + m_AllocGranularity;

	if (m_AllocSize == 0)
		m_AllocSize = m_AllocGranularity;

	Map();
}

CMappedMemory::~CMappedMemory()
{
	if (m_Base)
		UnmapViewOfFile(m_Base);
	if (m_FileMapping)
		CloseHandle(m_FileMapping);
	if (m_DirectFile)
		CloseHandle(m_DirectFile);
}

void CMappedMemory::Map()
{	
  m_FileMapping = CreateFileMapping(m_DirectFile, NULL, PAGE_READWRITE, 0, m_AllocSize, NULL);

	if (m_FileMapping == 0)
		throw "CreateFileMapping failed";

	m_Base = (__int8*) MapViewOfFile(m_FileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (m_Base == NULL)
		throw "MapViewOfFile failed";
}

unsigned int CMappedMemory::mRead(void* Buf, unsigned int Source, unsigned int Size)
{
	memcpy(Buf, m_Base + Source, Size);
	return Size;
}
unsigned int CMappedMemory::mWrite(void* Buf, unsigned int Dest, unsigned int Size)
{
	memcpy(m_Base + Dest, Buf, Size);
	return Size;
}

/*
unsigned int CMappedMemory::Move(unsigned int Source, unsigned int Dest, unsigned int Size)
{
	memcpy(m_Base + Dest, m_Base + Source, Size);
	return Size;
}
*/

unsigned int CMappedMemory::mAlloc(unsigned int Size)
{
	unsigned int res = 0xFFFFFFFF;	

	TFreeSpaceMap::iterator i = m_FreeSpace.lower_bound(Size);
	
	if (i == m_FreeSpace.end())
	{
		// need new space -> remap file and take care of m_FreeFileEnd
		unsigned int freestart;

		UnmapViewOfFile(m_Base);
		m_Base = NULL;
		CloseHandle(m_FileMapping);
		
		if (m_FreeFileEnd != m_FreeSpace.end())
		{
			freestart = m_FreeFileEnd->second;
			m_FreeSpace.erase(m_FreeFileEnd);
		}	else {
			freestart = m_AllocSize;
		}

		m_AllocSize += Size;
		if (m_AllocSize % m_AllocGranularity > 0)
			m_AllocSize = m_AllocSize - m_AllocSize % m_AllocGranularity + m_AllocGranularity;
		
		Map();

		Free(freestart, m_AllocSize - freestart);

		res = Alloc(Size);
	} else {
		res = i->second;

		if (i->first > Size)
			Free(res + Size, i->first - Size); // if allocating from m_FreeFileEnd it will be set to the left part, if something left...

		if (i == m_FreeFileEnd)
			m_FreeFileEnd = m_FreeSpace.end();  // ...or to end() if nothing left

		m_FreeSpace.erase(i);
	}
	
	memset(m_Base + res, 0, Size);

	return res;
}
void CMappedMemory::mFree(unsigned int Dest, unsigned int Count)
{
	char * zero = new char [Count];
	memset(zero, 0, Count);
	Write(zero, Dest, Count);
	delete [] zero;

	TFreeSpaceMap::iterator i = m_FreeSpace.insert(std::make_pair(Count, Dest));
	if (Dest + Count == m_AllocSize)
	{
		m_FreeFileEnd = i;
	}	
}

