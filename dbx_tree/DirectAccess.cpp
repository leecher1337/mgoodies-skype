/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2008 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "DirectAccess.h"

CDirectAccess::CDirectAccess(const char* FileName, CEncryptionManager & EncryptionManager, uint32_t EncryptionStart)
: CFileAccess(FileName, EncryptionManager, EncryptionStart)
{
	m_File = CreateFileA(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
	if (m_File == INVALID_HANDLE_VALUE) 
		throwException("CreateFile failed");

	m_MinAllocGranularity = 0x00001000;  // 4kb   to avoid heavy fragmentation
	m_AllocGranularity    = 0x00008000;  // 32kb
	m_MaxAllocGranularity = 0x00100000;  // 1mb   for fast increasing

	SetSize(GetFileSize(m_File, NULL));
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
		throwException("Cannot set file position");

	if (0 == ReadFile(m_File, Buf, Size, &read, NULL))
		throwException("Cannot read");

	return read;
}
uint32_t CDirectAccess::mWrite(void* Buf, uint32_t Dest, uint32_t Size)
{
	DWORD read = 0;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_File, Dest, NULL, FILE_BEGIN))
		throwException("Cannot set file position");

	if (0 == WriteFile(m_File, Buf, Size, &read, NULL))
		throwException("Cannot write");

	return read;
}

uint32_t CDirectAccess::mSetSize(uint32_t Size)
{
	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_File, Size, NULL, FILE_BEGIN))
		throwException("Cannot set file position");

	if (!SetEndOfFile(m_File))
		throwException("Cannot set end of file");

	return Size;		
}
