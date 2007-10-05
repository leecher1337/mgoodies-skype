#include "DirectAccess.h"

CDirectAccess::CDirectAccess(const char* FileName)
{

	BeginWrite();

	m_File = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS, 0);
	if (m_File == INVALID_HANDLE_VALUE) 
		throw "CreateFile failed";

	m_Size = GetFileSize(m_File, NULL);

	EndWrite();
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

unsigned int CDirectAccess::Read(void* Buf, unsigned int Source, unsigned int Size)
{
	unsigned long read = 0;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_File, Source, NULL, FILE_BEGIN))
		throw "Cannot set file position";

	if (0 == ReadFile(m_File, Buf, Size, &read, NULL))
		throw "Cannot read";

	return read;
}
unsigned int CDirectAccess::Write(void* Buf, unsigned int Dest, unsigned int Size)
{
	unsigned long read = 0;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_File, Dest, NULL, FILE_BEGIN))
		throw "Cannot set file position";

	if (0 == WriteFile(m_File, Buf, Size, &read, NULL))
		throw "Cannot write";

	return read;
}
unsigned int CDirectAccess::Move(unsigned int Source, unsigned int Dest, unsigned int Size)
{
	unsigned long read = 0;
	char* buf;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_File, Source, NULL, FILE_BEGIN))
		throw "Cannot set file position";

	buf = new char[Size];

	if (0 == ReadFile(m_File, buf, Size, &read, NULL))
		throw "Cannot read";

	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_File, Dest, NULL, FILE_BEGIN))
		throw "Cannot set file position";

	if (0 == WriteFile(m_File, buf, Size, &read, NULL))
		throw "Cannot write";

	delete [] buf;
	return read;
}

unsigned int CDirectAccess::Alloc(unsigned int Size)
{
	unsigned int res = 0xFFFFFFFF;	

	if (Size == 0) return res;

	BeginWrite();

	TFreeSpaceMap::iterator i = m_FreeSpace.lower_bound(Size);
	
	if (i == m_FreeSpace.end())
	{
		res = m_Size;
		m_Size += Size;

	} else {
		res = i->second;

		if (i->first > Size)
			Free(res + Size, i->first - Size);

		m_FreeSpace.erase(i);
	}
	
	EndWrite();

	return res;
}
void CDirectAccess::Free(unsigned int Dest, unsigned int Count)
{
	//needs improvements
	BeginWrite();
	if (Dest + Count == m_Size)
	{
		m_Size -= Count;
	} else {
		TFreeSpaceMap::iterator i = m_FreeSpace.insert(std::make_pair(Count, Dest));
	}
	
	EndWrite();
}