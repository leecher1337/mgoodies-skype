#include "MappedMemory.h"

CMappedMemory::CMappedMemory(const char* FileName, CEncryptionManager & EncryptionManager, uint32_t EncryptionStart)
:	CFileAccess(FileName, EncryptionManager, EncryptionStart)
{
	SYSTEM_INFO sysinfo;

	m_AllocSize = 0;
	m_DirectFile = 0;
	m_FileMapping = 0;
	m_Base = NULL;
	
	GetSystemInfo(&sysinfo);
	m_AllocGranularity = sysinfo.dwAllocationGranularity; // usually 64kb
	m_MinAllocGranularity = m_AllocGranularity;           // must be at least one segment
	m_MaxAllocGranularity = m_AllocGranularity << 4;      // usually 1mb for fast increasing

	m_DirectFile = CreateFileA(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
	if (m_DirectFile == INVALID_HANDLE_VALUE) 
		throwException("CreateFile failed");

	SetSize(GetFileSize(m_DirectFile, NULL));
}

CMappedMemory::~CMappedMemory()
{
	if (m_Base)
		UnmapViewOfFile(m_Base);
	if (m_FileMapping)
		CloseHandle(m_FileMapping);

	if (INVALID_SET_FILE_POINTER != SetFilePointer(m_DirectFile, m_Size, NULL, FILE_BEGIN))
		SetEndOfFile(m_DirectFile);	

	if (m_DirectFile)
		CloseHandle(m_DirectFile);
}


uint32_t CMappedMemory::mRead(void* Buf, uint32_t Source, uint32_t Size)
{
	memcpy(Buf, m_Base + Source, Size);
	return Size;
}
uint32_t CMappedMemory::mWrite(void* Buf, uint32_t Dest, uint32_t Size)
{
	memcpy(m_Base + Dest, Buf, Size);
	return Size;
}

uint32_t CMappedMemory::mSetSize(uint32_t Size)
{
	if (m_Base)
	{
		FlushViewOfFile(m_Base, 0);
		UnmapViewOfFile(m_Base);
	}
	if (m_FileMapping)
		CloseHandle(m_FileMapping);

	m_Base = NULL;
	m_FileMapping = NULL;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_DirectFile, Size, NULL, FILE_BEGIN))
		throwException("Cannot set file position");

	if (!SetEndOfFile(m_DirectFile))
		throwException("Cannot set end of file");

	m_FileMapping = CreateFileMapping(m_DirectFile, NULL, PAGE_READWRITE, 0, Size, NULL);

	if (m_FileMapping == 0)
		throwException("CreateFileMapping failed");

	m_Base = (uint8_t*) MapViewOfFile(m_FileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (m_Base == NULL)
		throwException("MapViewOfFile failed");

	return Size;
}
