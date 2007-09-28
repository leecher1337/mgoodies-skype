#include "MappedMemory.h"

CMappedMemory::CMappedMemory(const char* FileName)
: m_Sync(),
	m_FreeSpace()
{
	SYSTEM_INFO sysinfo;

	m_FreeFileEnd = NULL;
	m_AllocSize = 0;
	m_Size = 0;
	m_DirectFile = 0;
	m_FileMapping = 0;

	m_Sync.BeginWrite();
	
	GetSystemInfo(&sysinfo);
	m_AllocGranularity = sysinfo.dwAllocationGranularity;

	m_DirectFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS, 0);
	if (m_DirectFile == INVALID_HANDLE_VALUE) 
		throw "CreateFile failed";

	m_AllocSize = GetFileSize(m_DirectFile, NULL);

	if (m_AllocSize % m_AllocGranularity > 0)
		m_AllocSize = m_AllocSize - m_AllocSize % m_AllocGranularity + m_AllocGranularity;

	if (m_AllocSize == 0)
		m_AllocSize = m_AllocGranularity;

	Map();

  m_Sync.EndWrite();  
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
	m_Sync.BeginWrite();
	
  m_FileMapping = CreateFileMapping(m_DirectFile, NULL, PAGE_READWRITE, 0, m_AllocSize, NULL);

	if (m_FileMapping == 0)
		throw "CreateFileMapping failed";

	m_Base = (__int8*) MapViewOfFile(m_FileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (m_Base == NULL)
		throw "MapViewOfFile failed";

  m_Sync.EndWrite();
}

unsigned int CMappedMemory::Read(void* Buf, unsigned int Source, unsigned int Size)
{
	m_Sync.BeginRead();
	memcpy(Buf, m_Base + Source, Size);
	m_Sync.EndRead();	

	return Size;
}
unsigned int CMappedMemory::Write(void* Buf, unsigned int Dest, unsigned int Size)
{
	m_Sync.BeginWrite();
	memcpy(m_Base + Dest, Buf, Size);
	m_Sync.EndWrite();	
	return Size;
}

unsigned int CMappedMemory::Move(unsigned int Source, unsigned int Dest, unsigned int Size)
{
	m_Sync.BeginWrite();
	memcpy(m_Base + Dest, m_Base + Source, Size);
	m_Sync.EndWrite();
	return Size;
}

void* CMappedMemory::MakePointer(unsigned int Source)
{
	return m_Base + Source;
}


unsigned int CMappedMemory::Alloc(unsigned int Size)
{
	unsigned int res = 0xFFFFFFFF;	

	if (Size == 0) return res;

	m_Sync.BeginWrite();

	TFreeSpaceMap::iterator i = m_FreeSpace.lower_bound(Size);
	

	if (i == m_FreeSpace.end())
	{
		// need new space -> remap file and take care of m_FreeFileEnd
		unsigned int freestart;

		UnmapViewOfFile(m_Base);
		m_Base = NULL;
		CloseHandle(m_FileMapping);
		
		if (m_FreeFileEnd != NULL)
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
			m_FreeFileEnd = NULL;  // ...or to NULL if nothing left

		m_FreeSpace.erase(i);
	}
	
	m_Sync.EndWrite();

	return res;
}
void CMappedMemory::Free(unsigned int Dest, unsigned int Count)
{
	//needs improvements
	m_Sync.BeginWrite();
	TFreeSpaceMap::iterator i = m_FreeSpace.insert(std::make_pair(Count, Dest));
	if (Dest + Count == m_AllocSize)
	{
		m_FreeFileEnd = i;
	}
	
	m_Sync.EndWrite();
}

void CMappedMemory::BeginRead()
{
	m_Sync.BeginRead();
}
void CMappedMemory::EndRead()
{
	m_Sync.EndRead();
}
bool CMappedMemory::BeginWrite()
{
	return m_Sync.BeginWrite();
}
void CMappedMemory::EndWrite()
{
	m_Sync.EndWrite();
}