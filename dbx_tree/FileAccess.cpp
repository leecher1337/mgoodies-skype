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
#include <vector>
#ifndef _MSC_VER
#include "savestrings_gcc.h"
#define _time32 time
#endif


CFileAccess::CFileAccess(const TCHAR* FileName)
{
	m_FileName = new TCHAR[_tcslen(FileName) + 1];
	m_JournalFileName = new TCHAR[_tcslen(FileName) + 5];
	_tcscpy_s(m_FileName, _tcslen(FileName) + 1, FileName);
	_tcscpy_s(m_JournalFileName, _tcslen(FileName) + 5, FileName);
	_tcscat_s(m_JournalFileName, _tcslen(FileName) + 5, _T(".jrn"));

	m_ReadOnly = false;
	m_LastSize = 0;
	m_Size = 0;

	m_LastAllocTime = _time32(NULL);
}

CFileAccess::~CFileAccess()
{
	CloseHandle(m_Journal);
	DeleteFile(m_JournalFileName);

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
	if (m_Size != m_LastSize)
	{
		m_sigFileSizeChanged.emit(this, m_Size);
		m_LastSize = m_Size;
	}
}
void CFileAccess::CloseTransaction()
{
	TJournalEntry e = {'fini', 0, m_Size};
	DWORD written;
	WriteFile(m_Journal, &e, sizeof(e), &written, NULL);
//	FlushFileBuffers(m_Journal);
}

void CFileAccess::FlushJournal()
{
	FlushFileBuffers(m_Journal);

	uint32_t filesize = GetFileSize(m_Journal, NULL) - sizeof(cJournalSignature);
	SetFilePointer(m_Journal, sizeof(cJournalSignature), NULL, FILE_BEGIN);

	uint8_t* buf = (uint8_t*)malloc(filesize);
	TJournalEntry* e = (TJournalEntry*)buf;
	DWORD read = 0;
	if (!ReadFile(m_Journal, buf, filesize, &read, NULL) || (read != filesize))
	{
		free(buf);
		throwException(_T("Couldn't flush the journal because ReadFile failed!"));
	}

	std::vector<TJournalEntry*> currentops;

	while (filesize >= sizeof(TJournalEntry))
	{
		switch (e->Signature)
		{
			case 'fini': 
			{
				e->Size = (e->Size + m_AllocGranularity - 1) & ~(m_AllocGranularity - 1);

				if (e->Size == 0)
					e->Size = m_AllocGranularity;

				if (e->Size != m_AllocSize)
				{
					m_AllocSize = mSetSize(e->Size);

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

				std::vector<TJournalEntry*>::iterator i = currentops.begin();
				while (i != currentops.end())
				{
					switch ((*i)->Signature)
					{
						case 'writ':
						{
							if ((*i)->Address + (*i)->Size <= m_Size)
							{
								mWrite(*i + 1, (*i)->Address, (*i)->Size);
							} else if ((*i)->Address < m_Size) 
							{
								mWrite(*i + 1, (*i)->Address, m_Size - (*i)->Address);
							}
						} break;
						case 'inva':
						{
							if ((*i)->Address + (*i)->Size <= m_Size)
							{
								mInvalidate((*i)->Address, (*i)->Size);
							} else if ((*i)->Address < m_Size) 
							{
								mInvalidate((*i)->Address, m_Size - (*i)->Address);
							}
						} break;
					}
					++i;
				}
				currentops.clear();

				e++;
				filesize = filesize - sizeof(TJournalEntry);
			} break;
			case 'writ':
			{
				if (filesize < sizeof(e) + e->Size)
				{
					filesize = 0;
				} else {
					currentops.push_back(e);
					filesize = filesize - sizeof(TJournalEntry) - e->Size;
					e = (TJournalEntry*)((uint8_t*)e + sizeof(TJournalEntry) + e->Size);					
				}
			} break;
			case 'inva':
			{
				if (filesize < sizeof(e))
				{
					filesize = 0;
				} else {
					currentops.push_back(e);
					e++;
					filesize = filesize - sizeof(TJournalEntry);
				}
			} break;
			default: throwException(_T("Jounral corrupt!"));
		}
	}

	mFlush();

	SetFilePointer(m_Journal, sizeof(cJournalSignature), NULL, FILE_BEGIN);
	SetEndOfFile(m_Journal);

	free(buf);
}

void CFileAccess::InitJournal()
{
	m_Journal = CreateFile(m_JournalFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
	assertThrow(m_Journal != INVALID_HANDLE_VALUE,
		          _T("CreateFile failed on Journal %s"), m_JournalFileName);

	uint8_t h[sizeof(cJournalSignature)];
	DWORD read;
	if (ReadFile(m_Journal, &h, sizeof(h), &read, NULL) && (read == sizeof(h)) && (0 == memcmp(h, cJournalSignature, sizeof(h))))
		FlushJournal();

	SetFilePointer(m_Journal, 0, NULL, FILE_BEGIN);
	SetEndOfFile(m_Journal);
	DWORD written;
	WriteFile(m_Journal, cJournalSignature, sizeof(cJournalSignature), &written, NULL);
}

void CFileAccess::Invalidate(uint32_t Dest, uint32_t Size)
{
	DWORD written;
	TJournalEntry e = {'inva', Dest, Size};
	WriteFile(m_Journal, &e, sizeof(e), &written, NULL);
}

uint32_t CFileAccess::Size(uint32_t Size)
{
	m_Size = Size;

	return Size;
}
