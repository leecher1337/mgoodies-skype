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

#pragma once

#include <windows.h>
#include <time.h>
#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif
#include "EncryptionManager.h"
#include "Exception.h"
#include "sigslot.h"

class CFileAccess
{
public:
	CFileAccess(const char* FileName, CEncryptionManager & EncryptionManager, uint32_t EncryptionStart);
	virtual ~CFileAccess();

	uint32_t Read(void* Buf, uint32_t Source, uint32_t Size);
  uint32_t Write(void* Buf, uint32_t Dest, uint32_t Size);
	virtual uint32_t SetSize(uint32_t Size);
	virtual uint32_t GetSize();
	void SetReadOnly(bool ReadOnly);
	bool GetReadOnly();

	typedef sigslot::signal2<CFileAccess *, uint32_t> TOnFileSizeChanged;

	TOnFileSizeChanged & sigFileSizeChanged();

protected:
	char * m_FileName;
	CEncryptionManager & m_EncryptionManager;
	
	uint32_t m_Size;
	uint32_t m_AllocSize;
	uint32_t m_AllocGranularity;
	uint32_t m_MinAllocGranularity;
	uint32_t m_MaxAllocGranularity;
	uint32_t m_LastAllocTime;
	uint32_t m_EncryptionStart;
	bool m_ReadOnly;

	TOnFileSizeChanged m_sigFileSizeChanged;
	virtual uint32_t mRead(void* Buf, uint32_t Source, uint32_t Size) = 0;
  virtual uint32_t mWrite(void* Buf, uint32_t Dest, uint32_t Size) = 0;
	virtual uint32_t mSetSize(uint32_t Size) = 0;




};
