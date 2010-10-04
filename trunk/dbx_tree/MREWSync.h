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
#include "TLS.h"
#include "stdint.h"

class CMultiReadExclusiveWriteSynchronizer
{
private:
	uint64_t volatile m_Sentinel;
	HANDLE m_ReadSignal[2];
	HANDLE m_WriteSignal;

	volatile uint32_t m_Revision;
	unsigned int m_WriterID;
	unsigned int m_WriteRecursion;

	CThreadLocalStorage<CMultiReadExclusiveWriteSynchronizer, unsigned> tls;

public:
	CMultiReadExclusiveWriteSynchronizer();
	virtual ~CMultiReadExclusiveWriteSynchronizer();

	void BeginRead();
	void EndRead();
	bool BeginWrite();
	bool TryBeginWrite();
	bool EndWrite();

	unsigned int Waiting();
	unsigned int WriteRecursionCount() {return m_WriteRecursion;};

};
