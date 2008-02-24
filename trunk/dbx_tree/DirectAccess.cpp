#include "DirectAccess.h"

CDirectAccess::CDirectAccess(const char* FileName)
: CFileAccess(FileName)
{
	m_File = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
	if (m_File == INVALID_HANDLE_VALUE) 
		throw "CreateFile failed";

	m_AllocGranularity = 65536; // 64kb to avoid heavy fragmentation
}

CDirectAccess::~CDirectAccess()
{
	if (m_File)
	{
		if (INVALID_SET_FILE_POINTER != SetFilePointer(m_File, m_Size, NULL, FILE_BEGIN))
			SetEndOfFile(m_File);

		CloseHandle(m_File);
	}
}

uint32_t CDirectAccess::mRead(void* Buf, uint32_t Source, uint32_t Size)
{
	DWORD read = 0;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_File, Source, NULL, FILE_BEGIN))
		throw "Cannot set file position";

	if (0 == ReadFile(m_File, Buf, Size, &read, NULL))
		throw "Cannot read";

	return read;
}
uint32_t CDirectAccess::mWrite(void* Buf, uint32_t Dest, uint32_t Size)
{
	DWORD read = 0;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_File, Dest, NULL, FILE_BEGIN))
		throw "Cannot set file position";

	if (0 == WriteFile(m_File, Buf, Size, &read, NULL))
		throw "Cannot write";

	return read;
}

uint32_t CDirectAccess::mSetSize(uint32_t Size)
{
	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_File, Size, NULL, FILE_BEGIN))
		throw "Cannot set file position";

	if (!SetEndOfFile(m_File))
		throw "Cannot set end of file";

	return Size;		
}
