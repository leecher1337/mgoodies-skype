/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#pragma once

#include <windows.h>
#include <time.h>
#include "stdint.h"
#include "sigslot.h"

class CFileAccess
{
public:
	static const uint8_t cJournalSignature[20];
	static const uint32_t cJournalSizeReserve = 1 << 20; // 1MB

	CFileAccess(const TCHAR* FileName);
	virtual ~CFileAccess();


	uint32_t Read(void* Buf, uint32_t Source, uint32_t Size)
		{
			return _Read(Buf, Source, Size);
		};

	bool Write(void* Buf, uint32_t Dest, uint32_t Size)
		{
			if (m_UseJournal)
			{
				DWORD written;
				uint32_t data[3] = {'writ', Dest, Size};
				WriteFile(m_Journal, &data, sizeof(data), &written, NULL);
				WriteFile(m_Journal, Buf, Size, &written, NULL);
			} else {
				_Write(Buf, Dest, Size);
			}

			return true;
		};

	void Invalidate(uint32_t Dest, uint32_t Size)
		{
			if (m_UseJournal)
			{
				DWORD written;
				TJournalEntry e = {'inva', Dest, Size};
				WriteFile(m_Journal, &e, sizeof(e), &written, NULL);
			} else {
				_Invalidate(Dest, Size);
			}
		};

	void Flush()
		{
			if (m_UseJournal)
			{
				FlushFileBuffers(m_Journal);
			} else {
				_Flush();
			}
		};

	void UseJournal(bool UseIt)
		{
			m_UseJournal = UseIt;
		};

	void CompleteTransaction()
		{
			if (m_Size != m_LastSize)
			{
				m_sigFileSizeChanged.emit(this, m_Size);
				m_LastSize = m_Size;
			}
		};
	void CloseTransaction()
		{
			if (m_UseJournal)
			{
				TJournalEntry e = {'fini', 0, m_Size};
				DWORD written;
				WriteFile(m_Journal, &e, sizeof(e), &written, NULL);
			}
		};

	void CleanJournal();

	uint32_t Size(uint32_t NewSize);
	uint32_t Size()
		{	return m_Size; };
	void ReadOnly(bool ReadOnly)
		{	m_ReadOnly = ReadOnly; };
	bool ReadOnly()
		{	return m_ReadOnly; };

	typedef sigslot::signal2<CFileAccess *, uint32_t> TOnFileSizeChanged;

	TOnFileSizeChanged & sigFileSizeChanged()
	{	return m_sigFileSizeChanged; };

protected:
	TCHAR* m_FileName;
	TCHAR* m_JournalFileName;
	HANDLE m_Journal;

	bool   m_UseJournal;
	
	uint32_t m_Size;
	uint32_t m_AllocSize;
	uint32_t m_AllocGranularity;
	uint32_t m_MinAllocGranularity;
	uint32_t m_MaxAllocGranularity;
	uint32_t m_LastAllocTime;
	bool     m_ReadOnly;
	uint32_t m_LastSize;

	TOnFileSizeChanged m_sigFileSizeChanged;
	virtual uint32_t _Read(void* Buf, uint32_t Source, uint32_t Size) = 0;
  virtual uint32_t _Write(void* Buf, uint32_t Dest, uint32_t Size) = 0;
	virtual void     _Invalidate(uint32_t Dest, uint32_t Size) = 0;
	virtual uint32_t _SetSize(uint32_t Size) = 0;
	virtual void     _Flush() = 0;

#pragma pack (push, 1)
	typedef struct TJournalEntry
	{
		uint32_t Signature;
		uint32_t Address;
		uint32_t Size;
	} TJournalEntry;
#pragma pack (pop)

	void InitJournal();
	void ProcessJournal();
};
