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
#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif
#include "Exception.h"
#include "sigslot.h"

static const uint8_t cJournalSignature[20] = "Miranda IM Journal!";

class CFileAccess
{
public:
	CFileAccess(const TCHAR* FileName);
	virtual ~CFileAccess();

	uint32_t Read(void* Buf, uint32_t Source, uint32_t Size) 
	{
		return mRead(Buf, Source, Size);
	};
  bool Write(void* Buf, uint32_t Dest, uint32_t Size)
	{
		return AppendJournal(Buf, Dest, Size);
	};
	void    Invalidate(uint32_t Dest, uint32_t Size);
	virtual uint32_t Size(uint32_t Size);
	virtual uint32_t Size()
	{
		return m_Size;
	};
	void ReadOnly(bool ReadOnly)
	{
		m_ReadOnly = ReadOnly;
	};
	bool ReadOnly()
	{
		return m_ReadOnly;
	};

	void CompleteTransaction();
	void CloseTransaction();
	void FlushJournal();

	typedef sigslot::signal2<CFileAccess *, uint32_t> TOnFileSizeChanged;

	TOnFileSizeChanged & sigFileSizeChanged()
	{
		return m_sigFileSizeChanged;
	};

protected:
	TCHAR* m_FileName;
	TCHAR* m_JournalFileName;
	HANDLE m_Journal;
	
	uint32_t m_Size;
	uint32_t m_AllocSize;
	uint32_t m_AllocGranularity;
	uint32_t m_MinAllocGranularity;
	uint32_t m_MaxAllocGranularity;
	uint32_t m_LastAllocTime;
	bool m_ReadOnly;
	uint32_t m_LastSize;

	TOnFileSizeChanged m_sigFileSizeChanged;
	virtual uint32_t mRead(void* Buf, uint32_t Source, uint32_t Size) = 0;
  virtual uint32_t mWrite(void* Buf, uint32_t Dest, uint32_t Size) = 0;
	virtual void     mInvalidate(uint32_t Dest, uint32_t Size) = 0;
	virtual uint32_t mSetSize(uint32_t Size) = 0;
	virtual void     mFlush() = 0;

#pragma pack (push, 1)
	typedef struct TJournalEntry
	{
		uint32_t Signature;
		uint32_t Address;
		uint32_t Size;
	} TJournalEntry;
#pragma pack (pop)

	bool AppendJournal(void* Buf, uint32_t DestAddr, uint32_t Size);
	void InitJournal();
};
