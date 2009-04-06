/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2009 Michael "Protogenes" Kunz,

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

#include "FileAccess.h"
#ifndef _MSC_VER
#include "savestrings_gcc.h"
#define _time32 time
#endif


CFileAccess::CFileAccess(const char* FileName)
{
	m_FileName = new char[strlen(FileName) + 1];
	m_JournalFileName = new char[strlen(FileName) + 5];
	strcpy_s(m_FileName, strlen(FileName) + 1, FileName);
	strcpy_s(m_JournalFileName, strlen(FileName) + 5, FileName);
	strcat_s(m_JournalFileName, strlen(FileName) + 5, ".jrn");

	m_ReadOnly = false;

	m_LastAllocTime = _time32(NULL);
}

CFileAccess::~CFileAccess()
{
	CloseHandle(m_Journal);
	DeleteFileA(m_JournalFileName);

	delete [] m_FileName;
	delete [] m_JournalFileName;
}

bool CFileAccess::AppendJournal(void* Buf, uint32_t DestAddr, uint32_t Size)
{
	DWORD written;
	uint32_t data[3] = {'writ', DestAddr, Size};
	WriteFile(m_Journal, &data, sizeof(data), &written, NULL);
	WriteFile(m_Journal, Buf, Size, &written, NULL);

	return true;
}

void CFileAccess::CompleteTransaction()
{
	uint32_t mark = 'fini';
	DWORD written;
	WriteFile(m_Journal, &mark, sizeof(mark), &written, NULL);
	FlushFileBuffers(m_Journal);
}

void CFileAccess::FlushJournal()
{
	FlushFileBuffers(m_Journal);

	SetFilePointer(m_Journal, sizeof(cJournalSignature), NULL, FILE_BEGIN);
	uint32_t type;
	uint32_t addr;
	uint32_t size;
	void* buf = NULL;
	uint32_t bufsize = 0;
	DWORD read;
	bool b = true;
	
	while (b)
	{
		b = b && ReadFile(m_Journal, &type, sizeof(type), &read, NULL)
			    && (read == sizeof(type));

		if (b)
		{
			switch (type)
			{
				case 'fini': break;
				case 'writ':
				{
					b = b && ReadFile(m_Journal, &addr, sizeof(addr), &read, NULL)
						&& (read == sizeof(addr));
					b = b && ReadFile(m_Journal, &size, sizeof(size), &read, NULL)
						&& (read == sizeof(size));

					if (b && (bufsize < size))
					{
						buf = realloc(buf, size);
						bufsize = size;
					}

					b = b && ReadFile(m_Journal, buf, size, &read, NULL)
								&& (read == size);

					if (b)
					{
						if (addr + size <= m_Size)
							mWrite(buf, addr, size);
						else if (addr < m_Size)
							mWrite(buf, addr, m_Size - addr);
					}
				} break;
				case 'inva':
				{
					b = b && ReadFile(m_Journal, &addr, sizeof(addr), &read, NULL)
						&& (read == sizeof(addr));
					b = b && ReadFile(m_Journal, &size, sizeof(size), &read, NULL)
						&& (read == sizeof(size));

					if (b)
					{
						if (addr + size <= m_Size)
							mInvalidate(addr, size);
						else if (addr < m_Size)
							mInvalidate(addr, m_Size - addr);
					}
				} break;
				default: throwException("Jounral corrupt!");
			}
		}
	}

	mFlush();

	SetFilePointer(m_Journal, sizeof(cJournalSignature), NULL, FILE_BEGIN);
	SetEndOfFile(m_Journal);

	if (buf)
		free(buf);
}

void CFileAccess::CheckJournal()
{
	DWORD read = 0;
	uint8_t header[20];
	uint32_t fileend = sizeof(header);
	uint32_t currentpos = sizeof(header);
	uint32_t type;
	uint32_t addr;
	uint32_t size;
	if (ReadFile(m_Journal, header, sizeof(header), &read, NULL) && (read == sizeof(header)) && (memcmp(header, cJournalSignature, sizeof(header)) == 0))
	{
		bool b = true;

		while (b)
		{
			b = b && ReadFile(m_Journal, &type, sizeof(type), &read, NULL)
				&& (read == sizeof(type));
			currentpos += read;

			if (b)
			{
				switch (type)
				{
				case 'fini':
					{
						fileend = currentpos;
					} break;
				case 'writ':
					{
						b = b && ReadFile(m_Journal, &addr, sizeof(addr), &read, NULL)
							&& (read == sizeof(addr));
						currentpos += read;
						b = b && ReadFile(m_Journal, &size, sizeof(size), &read, NULL)
							&& (read == sizeof(size));
						currentpos += read;

						SetFilePointer(m_Journal, size, NULL, FILE_CURRENT);
						currentpos += size;

					} break;
				case 'inva':
					{
						b = b && ReadFile(m_Journal, &addr, sizeof(addr), &read, NULL)
							&& (read == sizeof(addr));
						currentpos += read;
						b = b && ReadFile(m_Journal, &size, sizeof(size), &read, NULL)
							&& (read == sizeof(size));
						currentpos += read;

					} break;
				}
			}
		}

		SetFilePointer(m_Journal, fileend, NULL, FILE_BEGIN);
		SetEndOfFile(m_Journal);

		FlushJournal();
	}
}

void CFileAccess::InitJournal()
{
	m_Journal = CreateFileA(m_JournalFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if (m_Journal == INVALID_HANDLE_VALUE) 
		throwException("CreateFile failed on Journal");

	CheckJournal();

	SetFilePointer(m_Journal, 0, NULL, FILE_BEGIN);
	SetEndOfFile(m_Journal);
	DWORD written;
	WriteFile(m_Journal, cJournalSignature, sizeof(cJournalSignature), &written, NULL);
}

void CFileAccess::Invalidate(uint32_t Dest, uint32_t Size)
{
	DWORD written;
	uint32_t data[3] = {'inva', Dest, Size};
	WriteFile(m_Journal, &data, sizeof(data), &written, NULL);
}

uint32_t CFileAccess::Size(uint32_t Size)
{
	m_sigFileSizeChanged.emit(this, Size);

	m_Size = Size;

	Size = (Size + m_AllocGranularity - 1) & ~(m_AllocGranularity - 1);

	if (Size == 0)
		Size = m_AllocGranularity;

	if (Size != m_AllocSize)
	{
		m_AllocSize = mSetSize(Size);

		// adapt Alloc Granularity
		uint32_t t = _time32(NULL);
		uint32_t d = t - m_LastAllocTime;
		m_LastAllocTime = t;

		if (d < 30) // increase alloc stepping
		{
			if (m_AllocGranularity < m_MaxAllocGranularity)
				m_AllocGranularity = m_AllocGranularity << 1;
		} else if (d > 120) // decrease alloc stepping
		{
			if (m_AllocGranularity > m_MinAllocGranularity)
				m_AllocGranularity = m_AllocGranularity >> 1;
		}
	}

	return Size;
}
