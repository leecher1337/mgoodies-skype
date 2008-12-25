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

#include "BlockManager.h"


CBlockManager::CBlockManager(
	CFileAccess & FileAccess,
	CEncryptionManager & EncryptionManager
	)
:	m_FileAccess(FileAccess),
	m_EncryptionManager(EncryptionManager),
	m_BlockTable(1),
	m_FreeBlocks(),
	m_FreeIDs(),
	m_BlockSync()
{
	m_BlockTable[0].Addr = 0;
	m_OptimizeThread = NULL;
	m_Optimize = 0;
}
CBlockManager::~CBlockManager()
{
	SYNC_BEGINWRITE(m_BlockSync);
	if (m_OptimizeThread)
	{
		m_OptimizeThread->FreeOnTerminate(false);
		m_OptimizeThread->Terminate();
		m_OptimizeThread->Resume();
		m_BlockSync.EndWrite();
		m_OptimizeThread->WaitFor();

		delete m_OptimizeThread;
	} else {
		m_BlockSync.EndWrite();
	}
}

// Optimize File Size
void CBlockManager::ExecuteOptimize()
{
	TBlockHeadFree h = {0,0};
	uint8_t * buf = NULL;
	uint32_t bufsize = 0;
	uint32_t nextbufpos = 0;
	uint32_t freebytes = 0;
	struct {
		uint32_t BlockID;
		uint32_t NewPos;
	} moveblocks[32];
	uint32_t blocks = 0;
	uint32_t flushblocks = 8;

	{
		int i = 0;
		while (!m_OptimizeThread->Terminated() && (i < 600))
		{
			++i;
			Sleep(100); // wait for Miranda to start
		}
	}

	SYNC_BEGINWRITE(m_BlockSync);

	if (m_FileAccess.GetReadOnly())
	{
		SYNC_ENDWRITE(m_BlockSync);
		return;
	}

	m_Optimize = m_FirstBlockStart;

	while (!m_OptimizeThread->Terminated() && (m_Optimize < m_FileAccess.GetSize()) && (h.ID != cFreeBlockID))
	{
		m_Optimize += h.Size;
		Read(m_Optimize, false, &h, sizeof(h));
	}

	while (!m_OptimizeThread->Terminated() && (m_Optimize < m_FileAccess.GetSize()))
	{
		Read(m_Optimize, false, &h, sizeof(h));
		if (h.ID == cFreeBlockID)
		{
			RemoveFreeBlock(m_Optimize, h.Size);
			freebytes += h.Size;
		} else if (freebytes > 0) {
			moveblocks[blocks].BlockID = h.ID;
			moveblocks[blocks].NewPos = m_Optimize - freebytes;
			blocks++;

			if (bufsize < nextbufpos + freebytes + h.Size)
			{
				bufsize = nextbufpos + freebytes + h.Size;
				buf = (uint8_t*) realloc(buf, bufsize);
			}
			Read(m_Optimize, false, buf + nextbufpos, h.Size);
			nextbufpos += h.Size;

		}
		m_Optimize += h.Size;
	
		if (blocks == flushblocks)
		{
			memset(buf + nextbufpos, 0, freebytes);
			((TBlockHeadFree*)(buf + nextbufpos))->ID = cFreeBlockID;
			((TBlockHeadFree*)(buf + nextbufpos))->Size = freebytes;
			((TBlockTailFree*)(buf + (nextbufpos + freebytes - sizeof(TBlockTailFree))))->ID = cFreeBlockID;
			((TBlockTailFree*)(buf + (nextbufpos + freebytes - sizeof(TBlockTailFree))))->Size = freebytes;
			Write(m_Optimize - nextbufpos - freebytes, false, buf, nextbufpos + freebytes);
			
			for (unsigned int i = 0; i < blocks; i++)
			{
				if ((m_BlockTable[moveblocks[i].BlockID >> 2].Addr & cVirtualBlockFlag) == 0)
					m_BlockTable[moveblocks[i].BlockID >> 2].Addr = moveblocks[i].NewPos;
			}
			
			blocks = 0;
			nextbufpos = 0;
			m_Optimize -= freebytes;
			freebytes = 0;			

			flushblocks += 3 - m_BlockSync.ReadWaiting();
			if (flushblocks < 4)
				flushblocks = 4;
			if (flushblocks > 32)
				flushblocks = 32;

			if (m_BlockSync.ReadWaiting() > 0)
			{
				SYNC_ENDWRITE(m_BlockSync);
				Sleep(m_BlockSync.ReadWaiting() * 50 + 1);
				SYNC_BEGINWRITE(m_BlockSync);
			}
		}
	}

	if (m_Optimize >= m_FileAccess.GetSize())
	{
		if (blocks)
			Write(m_Optimize - nextbufpos - freebytes, false, buf, nextbufpos);

		for (unsigned int i = 0; i < blocks; i++)
		{
			if ((m_BlockTable[moveblocks[i].BlockID >> 2].Addr & cVirtualBlockFlag) == 0)
				m_BlockTable[moveblocks[i].BlockID >> 2].Addr = moveblocks[i].NewPos;
		}

		m_FileAccess.SetSize(m_Optimize - freebytes);
	}

	m_OptimizeThread = NULL;
	m_Optimize = 0;
	SYNC_ENDWRITE(m_BlockSync);

	if (buf)
		free(buf);
}


uint32_t CBlockManager::ScanFile(uint32_t FirstBlockStart, uint32_t HeaderSignature, uint32_t FileSize)
{
	TBlockHeadOcc h, lasth = {0, 0, 0};
	uint32_t p;
	uint32_t res = 0;
	bool merge = false;

	p = FirstBlockStart;
	m_FirstBlockStart = FirstBlockStart;

	while (p < FileSize)
	{
		m_FileAccess.Read(&h, p, sizeof(h));
		if (h.Size == 0)
		{
			throwException("File Corrupt!");
		}

 		if (h.ID == cFreeBlockID)
		{
			if (lasth.ID == cFreeBlockID)
			{				
				lasth.Size += h.Size;
				merge = true;
			} else {				
				lasth = h;
			}
		
		} else {
			
			if (lasth.ID == cFreeBlockID)
			{
				if (merge)
					InsertFreeBlock(p - lasth.Size, lasth.Size, false, false);
				else
					m_FreeBlocks.insert(std::make_pair(lasth.Size, p - lasth.Size));
			}
			lasth = h;
			merge = false;

			while ((h.ID >> 2) >= m_BlockTable.size())
				m_BlockTable.resize(m_BlockTable.size() << 1);

			m_BlockTable[h.ID >> 2].Addr = p;

			if (h.Signature == HeaderSignature)
				res = h.ID;
		}

		p = p + h.Size;
	}

	for (uint32_t i = m_BlockTable.size() - 1; i > 0; --i)
	{
		if (m_BlockTable[i].Addr == 0)
			m_FreeIDs.push_back(i);
	}

	m_OptimizeThread = new COptimizeThread(*this);
	m_OptimizeThread->Priority(CThread::tpLowest);
	m_OptimizeThread->FreeOnTerminate(true);
	m_OptimizeThread->Resume();

	return res;
}


inline void CBlockManager::Read(uint32_t Addr, bool IsVirtual, void* Buffer, uint32_t Size)
{
	if (IsVirtual)
		memcpy(Buffer, (void*)Addr, Size);
	else
		m_FileAccess.Read(Buffer, Addr, Size);
}

inline void CBlockManager::Write(uint32_t Addr, bool IsVirtual, void* Buffer, uint32_t Size)
{
	if (IsVirtual)
		memcpy((void*)Addr, Buffer, Size);
	else
		m_FileAccess.Write(Buffer, Addr, Size);
}

inline bool CBlockManager::InitOperation(uint32_t BlockID, uint32_t & Addr, bool & IsVirtual, TBlockHeadOcc & Header)
{
	if (BlockID & 3)
		return false;

	BlockID = BlockID >> 2;
	if (BlockID >= m_BlockTable.size())
		return false;

	Addr = m_BlockTable[BlockID].Addr;

	if (Addr == 0)
		return false;

	IsVirtual = false;
	if (Addr & cVirtualBlockFlag)
	{
		Addr = Addr & ~cVirtualBlockFlag;
		IsVirtual = true;
	}

	Read(Addr, IsVirtual, &Header, sizeof(Header));

	return true;
}

inline uint32_t CBlockManager::CreateVirtualBlock(uint32_t BlockID, uint32_t ContentSize)
{
	BlockID = BlockID >> 2;
	uint32_t res = (uint32_t)malloc(ContentSize + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc));
	m_BlockTable[BlockID].Addr = res | cVirtualBlockFlag;
	return res;
}

inline void CBlockManager::InsertFreeBlock(uint32_t Addr, uint32_t Size, bool LookLeft, bool LookRight)
{
	TBlockTailFree ft;
	TBlockHeadFree fh;

	if (LookLeft && (Addr > m_FirstBlockStart + sizeof(TBlockTailFree))) // don't go before our start.
	{
		Read(Addr - sizeof(ft), false, &ft, sizeof(ft));
		if (ft.ID == cFreeBlockID)
		{ // free block in front of ours
			if (m_Optimize != Addr - ft.Size)
			{
				Addr = Addr - ft.Size;
				Size = Size + ft.Size;

				RemoveFreeBlock(Addr, ft.Size);
			}
		}
	}

	if (LookRight && (Addr + Size + sizeof(TBlockHeadFree) < m_FileAccess.GetSize()))
	{
		Read(Addr + Size, false, &fh, sizeof(fh));
		if (fh.ID == cFreeBlockID)
		{ // free block beyond our block
			if (m_Optimize == Addr + Size)
			{
				m_Optimize = Addr;
			} else {
				RemoveFreeBlock(Addr + Size, fh.Size);

				Size = Size + fh.Size;
			}
		}
	}

	if (Addr + Size == m_FileAccess.GetSize())
	{
		m_FileAccess.SetSize(Addr);
	} else {
		uint8_t * buf = (uint8_t*) malloc(Size);
		memset(buf, 0, Size);
		((TBlockHeadFree *)buf)->ID = cFreeBlockID;
		((TBlockHeadFree *)buf)->Size = Size;
		((TBlockTailFree *)(buf + Size - sizeof(TBlockTailFree)))->ID = cFreeBlockID;
		((TBlockTailFree *)(buf + Size - sizeof(TBlockTailFree)))->Size = Size;

		Write(Addr, false, buf, Size);
		free(buf);

		m_FreeBlocks.insert(std::make_pair(Size, Addr));
	}
}

inline void CBlockManager::RemoveFreeBlock(uint32_t Addr, uint32_t Size)
{
	TFreeBlockMap::iterator i = m_FreeBlocks.find(Size);
	while ((i != m_FreeBlocks.end()) && (i->first == Size))
	{
		if (i->second == Addr)
		{
			m_FreeBlocks.erase(i);
			i = m_FreeBlocks.end();
		} else {
			++i;
		}
	}
}


inline uint32_t CBlockManager::FindFreePosition(uint32_t & Size)
{
	// try to find free block
	TFreeBlockMap::iterator f = m_FreeBlocks.end();

	if ((!m_FreeBlocks.empty()) && (m_FreeBlocks.rbegin()->first >= Size)) // check if we can fulfill the request
	{
		int i = 1;
		while ((i < 5) && (m_FreeBlocks.end() == f)) // try to find a good fit
		{
			f = m_FreeBlocks.find(Size * i);
			++i;
		}

		if (m_FreeBlocks.end() == f) // try worst fit strategy. take the biggest block available (double size of request to catch remaining part later)
		{
			f = m_FreeBlocks.end();
			--f;
			if (f->first < 2 * Size)
				f = m_FreeBlocks.end();
		}
	}

	uint32_t addr = 0;

	if (f == m_FreeBlocks.end()) // no block found - expand file
	{
		addr = m_FileAccess.GetSize();
		m_FileAccess.SetSize(addr + Size);
	} else {
		addr = f->second;
		if (f->first <= Size + sizeof(TBlockHeadFree) + sizeof(TBlockTailFree))
		{
			Size = f->first;
			m_FreeBlocks.erase(f);
		} else { // we have some space left.
			uint8_t * buf = (uint8_t*) malloc(f->first);
			memset(buf, 0, f->first);
			((TBlockHeadFree*)buf)->ID = cFreeBlockID;
			((TBlockHeadFree*)buf)->Size = Size;
			((TBlockTailFree*)(buf + Size - sizeof(TBlockTailFree)))->ID = cFreeBlockID;
			((TBlockTailFree*)(buf + Size - sizeof(TBlockTailFree)))->Size = Size;
			((TBlockHeadFree*)(buf + Size))->ID = cFreeBlockID;
			((TBlockHeadFree*)(buf + Size))->Size = f->first - Size;
			((TBlockTailFree*)(buf + f->first - sizeof(TBlockTailFree)))->ID = cFreeBlockID;
			((TBlockTailFree*)(buf + f->first - sizeof(TBlockTailFree)))->Size = f->first - Size;
			Write(addr, false, buf, f->first);
			free(buf);
			m_FreeBlocks.insert(std::make_pair(f->first - Size, addr + Size));
			m_FreeBlocks.erase(f);
		}
	}

	return addr;
}

bool CBlockManager::ReadBlock(uint32_t BlockID, void * & Buffer, size_t & Size, uint32_t & Signature)
{
	if ((Buffer == NULL) ^ (Size == 0)) // if the one is specified the other must be too
		return false;

	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	SYNC_BEGINREAD(m_BlockSync);
	if (!InitOperation(BlockID, a, isvirtual, h))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if ((Signature != 0) && (Signature != h.Signature))
	{
		SYNC_ENDREAD(m_BlockSync);
		Signature = h.Signature;
		return false;
	}
	Signature = h.Signature;

	if ((Size != 0) && (Size != h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
	{
		SYNC_ENDREAD(m_BlockSync);
		Size = h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc);
		return false;
	}
	Size = h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc);

	if (Buffer == NULL)
		Buffer = malloc(Size);

	Read(a + sizeof(h), isvirtual, Buffer, Size);
	SYNC_ENDREAD(m_BlockSync);

	if (!isvirtual && m_EncryptionManager.IsEncrypted(BlockID, ET_BLOCK))
	{
		m_EncryptionManager.Decrypt(Buffer, Size, ET_BLOCK, BlockID, 0);
	}
	return true;
}
bool CBlockManager::WriteBlock(uint32_t BlockID, void * Buffer, size_t Size, uint32_t Signature)
{
	if ((Buffer == NULL) || (Size == 0)) // must be something usefull...
		return false;

	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	SYNC_BEGINREAD(m_BlockSync);
	if (!InitOperation(BlockID, a, isvirtual, h))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if (Size > h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if ((!isvirtual) && m_FileAccess.GetReadOnly())
	{
		a = CreateVirtualBlock(BlockID, Size);
		isvirtual = true;
	}

	h.Signature = Signature;

	if (!isvirtual && m_EncryptionManager.IsEncrypted(BlockID, ET_BLOCK))
	{
		m_EncryptionManager.Encrypt(Buffer, Size, ET_BLOCK, BlockID, 0);
	}
	Write(a, isvirtual, &h, sizeof(h));
	Write(a + sizeof(h), isvirtual, Buffer, Size);

	SYNC_ENDREAD(m_BlockSync);

	return true;
}
bool CBlockManager::WriteBlockCheck(uint32_t BlockID, void * Buffer, size_t Size, uint32_t & Signature)
{
	if ((Buffer == NULL) || (Size == 0)) // data must be something usefull...
		return false;

	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	SYNC_BEGINREAD(m_BlockSync);
	if (!InitOperation(BlockID, a, isvirtual, h))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if ((Signature != 0) && (h.Signature != Signature))
	{
		SYNC_ENDREAD(m_BlockSync);
		Signature = h.Signature;
		return false;
	}
	Signature = h.Signature;

	if (Size != h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}


	if ((!isvirtual) && m_FileAccess.GetReadOnly())
	{
		a = CreateVirtualBlock(BlockID, Size);
		isvirtual = true;
	}

	if (!isvirtual && m_EncryptionManager.IsEncrypted(BlockID, ET_BLOCK))
	{
		m_EncryptionManager.Encrypt(Buffer, Size, ET_BLOCK, BlockID, 0);
	}
	Write(a, isvirtual, &h, sizeof(h));
	Write(a + sizeof(h), isvirtual, Buffer, Size);

	SYNC_ENDREAD(m_BlockSync);
	return true;
}

bool CBlockManager::ReadPart(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t & Signature)
{
	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	SYNC_BEGINREAD(m_BlockSync);
	if (!InitOperation(BlockID, a, isvirtual, h))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if ((Signature != 0) && (Signature != h.Signature))
	{
		SYNC_ENDREAD(m_BlockSync);
		Signature = h.Signature;
		return false;
	}
	Signature = h.Signature;

	//if ((Size == 0) || ((Buffer == NULL) ^ (Size == 0))) // if the one is specified the other must be too
	if ((Size == 0) || (Buffer == NULL)) // same as above, simplified
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if ((Size + Offset > h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
		}

	if (!isvirtual && m_EncryptionManager.IsEncrypted(BlockID, ET_BLOCK))
	{
		uint32_t estart = Offset;
		uint32_t eend = Offset + Size;
		void* cryptbuf;

		m_EncryptionManager.AlignData(BlockID, ET_BLOCK, estart, eend);

		cryptbuf = malloc(eend - estart);

		Read(a + sizeof(h) + estart, false, cryptbuf, eend - estart);
		m_EncryptionManager.Decrypt(cryptbuf, eend - estart, ET_BLOCK, BlockID, estart);

		memcpy(Buffer, (uint8_t*)cryptbuf + (Offset - estart), Size);
		free(cryptbuf);

	} else {
		Read(a + sizeof(h) + Offset, isvirtual, Buffer, Size);
	}

	SYNC_ENDREAD(m_BlockSync);
	return true;
}

void CBlockManager::PartWriteEncrypt(uint32_t BlockID, uint32_t Offset, uint32_t Size, void * Buffer, uint32_t Addr)
{
	uint32_t estart1 = Offset;
	uint32_t eend1 = Offset;
	uint32_t estart2 = Offset + Size;
	uint32_t eend2 = Offset + Size;

	m_EncryptionManager.AlignData(BlockID, ET_BLOCK, estart1, eend1);
	m_EncryptionManager.AlignData(BlockID, ET_BLOCK, estart2, eend2);

	uint8_t * cryptbuf;

	cryptbuf = (uint8_t*) malloc(eend2 - estart1);

	if (estart1 != estart2) // two different blocks
	{
		if (estart1 < Offset) // load leading block
		{
			Read(Addr + estart1 + sizeof(TBlockHeadOcc), false, cryptbuf, eend1 - estart1);
			m_EncryptionManager.Decrypt(cryptbuf, eend1 - estart1, ET_BLOCK, BlockID, estart1);
		}
		if (eend2 > Offset + Size) // load trailing block
		{
			Read(Addr + estart2 + sizeof(TBlockHeadOcc), false, cryptbuf + (estart2 - estart1), eend2 - estart2);
			m_EncryptionManager.Decrypt(cryptbuf + (estart2 - estart1), eend2 - estart2, ET_BLOCK, BlockID, estart2);
		}
	} else if ((estart2 != Offset) || (eend2 != Offset + Size))
	{ // only one block
		Read(Addr + estart2 + sizeof(TBlockHeadOcc), false, cryptbuf, eend2 - estart2);
		m_EncryptionManager.Decrypt(cryptbuf, eend2 - estart2, ET_BLOCK, BlockID, estart2);
	}

	memcpy(cryptbuf + (Offset - estart1), Buffer, Size);

	m_EncryptionManager.Encrypt(cryptbuf, eend2 - estart1, ET_BLOCK, BlockID, estart1);
	Write(Addr + sizeof(TBlockHeadOcc) + estart1, false, cryptbuf, eend2 - estart1);

	free(cryptbuf);

}

bool CBlockManager::WritePart(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t Signature)
{
	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	SYNC_BEGINREAD(m_BlockSync);
	if (!InitOperation(BlockID, a, isvirtual, h))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if ((Size + Offset > h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if ((!isvirtual) && m_FileAccess.GetReadOnly())
	{
		uint32_t b = a;
		a = CreateVirtualBlock(BlockID, h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc));
		Read(b, false, (void*)a, h.Size - sizeof(TBlockTailOcc)); // copy block.
		isvirtual = true;
	}

	if ((Signature != 0) && (Signature != h.Signature))
	{
		h.Signature = Signature;
		Write(a, isvirtual, &h, sizeof(h));
	}

	if ((Size != 0) && (Buffer != NULL))
	{
		if (!isvirtual && m_EncryptionManager.IsEncrypted(BlockID, ET_BLOCK))
		{
			PartWriteEncrypt(BlockID, Offset, Size, Buffer, a);
		} else {
			Write(a + sizeof(h) + Offset, isvirtual, Buffer, Size);
		}
	}

	SYNC_ENDREAD(m_BlockSync);
	return true;
}
bool CBlockManager::WritePartCheck(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t & Signature)
{
	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	SYNC_BEGINREAD(m_BlockSync);
	if (!InitOperation(BlockID, a, isvirtual, h))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if ((Size + Offset > h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if ((Signature != 0) && (Signature != h.Signature))
	{
		SYNC_ENDREAD(m_BlockSync);
		Signature = h.Signature;
		return false;
	}
	Signature = h.Signature;

	if ((!isvirtual) && m_FileAccess.GetReadOnly())
	{
		uint32_t b = a;
		a = CreateVirtualBlock(BlockID, h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc));
		Read(b, false, (void*)a, h.Size - sizeof(TBlockTailOcc)); // copy block.
		isvirtual = true;
	}

	if ((Size != 0) && (Buffer != NULL))
	{
		if (!isvirtual && m_EncryptionManager.IsEncrypted(BlockID, ET_BLOCK))
		{
			PartWriteEncrypt(BlockID, Offset, Size, Buffer, a);
		} else {
			Write(a + sizeof(h) + Offset, isvirtual, Buffer, Size);
		}
	}

	SYNC_ENDREAD(m_BlockSync);
	return true;
}

uint32_t CBlockManager::CreateBlock(uint32_t Size, uint32_t Signature)
{
	uint32_t id = 0;

	SYNC_BEGINREAD(m_BlockSync);
	if (m_FreeIDs.empty())
	{
		id = m_BlockTable.size();

		m_BlockTable.resize(m_BlockTable.size() << 1);

		for (uint32_t i = m_BlockTable.size() - 1; i > id; --i)
			m_FreeIDs.push_back(i);
	} else {
		id = m_FreeIDs.back();
		m_FreeIDs.pop_back();
	}

	Size = m_EncryptionManager.AlignSize(id << 2, ET_BLOCK, (Size + 3) & 0xfffffffc); // align on cipher after we aligned on 4 bytes

	TBlockHeadOcc h;
	h.ID = id << 2;
	h.Signature = Signature;
	h.Size = Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc);
	if (m_FileAccess.GetReadOnly())
	{
		uint32_t a = CreateVirtualBlock(h.ID, Size);
		memset((void*)a, 0, Size + sizeof(TBlockHeadOcc));
		memcpy((void*)a, &h, sizeof(h));
		((TBlockTailOcc*)(a + Size + sizeof(TBlockHeadOcc)))->ID = h.ID;
	} else {
		uint32_t addr = FindFreePosition(h.Size);

		TBlockTailOcc t;
		t.ID = h.ID;

		Write(addr + h.Size - sizeof(t), false, &t, sizeof(t));
		Write(addr, false, &h, sizeof(h));
		m_BlockTable[id].Addr = addr;
	}

	SYNC_ENDREAD(m_BlockSync);
	return h.ID;
}

uint32_t CBlockManager::CreateBlockVirtual(uint32_t Size, uint32_t Signature)
{
	uint32_t id = 0;

	SYNC_BEGINREAD(m_BlockSync);
	if (m_FreeIDs.empty())
	{
		id = m_BlockTable.size();

		m_BlockTable.resize(m_BlockTable.size() << 1);

		for (uint32_t i = m_BlockTable.size() - 1; i > id; --i)
			m_FreeIDs.push_back(i);
	} else {
		id = m_FreeIDs.back();
		m_FreeIDs.pop_back();
	}

	Size = m_EncryptionManager.AlignSize(id << 2, ET_BLOCK, (Size + 3) & 0xfffffffc); // align on cipher after we aligned on 4 bytes

	uint32_t a = CreateVirtualBlock(id << 2, Size);
	SYNC_ENDREAD(m_BlockSync);

	memset((void*)(a + sizeof(TBlockHeadOcc)), 0, Size);
	((TBlockHeadOcc*)a)->ID = id << 2;
	((TBlockHeadOcc*)a)->Signature = Signature;
	((TBlockHeadOcc*)a)->Size = Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc);
	((TBlockTailOcc*)(a + Size + sizeof(TBlockHeadOcc)))->ID = id << 2;

	return id << 2;
}
bool CBlockManager::DeleteBlock(uint32_t BlockID)
{
	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	SYNC_BEGINREAD(m_BlockSync);
	if (!InitOperation(BlockID, a, isvirtual, h))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	BlockID = BlockID >> 2;

	m_BlockTable[BlockID].Addr = 0;
	// m_FreeIDs.push_back(BlockID); don't reuse blockids during a session to avoid side effects

	if (isvirtual)
	{
		free((void*)a);

	} else if (!m_FileAccess.GetReadOnly()) {
		InsertFreeBlock(a, h.Size);
	}

	SYNC_ENDREAD(m_BlockSync);
	return true;
}
uint32_t CBlockManager::ResizeBlock(uint32_t BlockID, uint32_t Size, bool SaveData)
{
	if (Size == 0) return 0;

	Size = m_EncryptionManager.AlignSize(BlockID, ET_BLOCK, (Size + 3) & 0xfffffffc); // align on cipher after we aligned on 4 bytes

	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	SYNC_BEGINREAD(m_BlockSync);
	if (!InitOperation(BlockID, a, isvirtual, h))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if (Size + sizeof(h) + sizeof(TBlockTailOcc) == h.Size)
	{
		SYNC_ENDREAD(m_BlockSync);
		return Size;
	}

	if (isvirtual)
	{
		a = (uint32_t) realloc((void*)a, Size + sizeof(h) + sizeof(TBlockTailOcc));
		memset((void*)(a + h.Size), 0, Size + sizeof(h) - h.Size);

		h.Size = Size + sizeof(h);
		memcpy((void*)a, &h, sizeof(h));
		m_BlockTable[BlockID >> 2].Addr = a | cVirtualBlockFlag;
	} else if (m_FileAccess.GetReadOnly())
	{
		uint32_t na = CreateVirtualBlock(BlockID, Size);
		uint32_t ns = Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc);

		if (SaveData)
		{
			if (ns > h.Size)
			{
				Read(a, false, (void*) na, h.Size);
			} else {
				Read(a, false, (void*) na, ns);
			}
		}

		h.Size = ns;
		memcpy((void*)na, &h, sizeof(h));
		((TBlockTailOcc*)(na + ns - sizeof(TBlockTailOcc)))->ID = h.ID;
	} else { // resize in file
		uint32_t ns = Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc);
		uint32_t na = FindFreePosition(ns);
		uint8_t * buf = (uint8_t*) malloc(ns);

		if (SaveData)
		{
			if (ns > h.Size)
			{
				Read(a, false, buf, h.Size);
			} else {
				Read(a, false, buf, ns);
			}
		}

		((TBlockHeadOcc*)buf)->ID = h.ID;
		((TBlockHeadOcc*)buf)->Size = ns;
		((TBlockHeadOcc*)buf)->Signature = h.Signature;
		((TBlockTailOcc*)(buf + ns - sizeof(TBlockTailOcc)))->ID = h.ID;
		Write(na, false, buf, ns);
		InsertFreeBlock(a, h.Size, true, true);
		m_BlockTable[BlockID >> 2].Addr = na;

		free(buf);
	}

	SYNC_ENDREAD(m_BlockSync);
	return Size;
}

bool CBlockManager::IsForcedVirtual(uint32_t BlockID)
{
	BlockID = BlockID >> 2;

	SYNC_BEGINREAD(m_BlockSync);
	if (BlockID >= m_BlockTable.size())
	{
		SYNC_ENDREAD(m_BlockSync);
		return true;
	}

	uint32_t addr = m_BlockTable[BlockID].Addr;
	SYNC_ENDREAD(m_BlockSync);

	if (addr == 0)
		return true;

	if (addr & cForcedVirtualBlockFlag)
		return true;

	return false;
}

bool CBlockManager::WriteBlockToDisk(uint32_t BlockID)
{
	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	SYNC_BEGINREAD(m_BlockSync);
	if (!InitOperation(BlockID, a, isvirtual, h))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if (!isvirtual)
	{
		SYNC_ENDREAD(m_BlockSync);
		return true;
	}

	if (m_FileAccess.GetReadOnly())
	{
		m_BlockTable[BlockID >> 2].Addr = m_BlockTable[BlockID >> 2].Addr & ~cForcedVirtualBlockFlag;
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	uint32_t os = h.Size;
	uint32_t oa = a;
	a = FindFreePosition(h.Size);

	((TBlockTailOcc*)(oa + os - sizeof(TBlockTailOcc)))->ID = h.ID;
	Write(a, false, (void*)oa, os);

	m_BlockTable[BlockID >> 2].Addr = a;

	SYNC_ENDREAD(m_BlockSync);
	free((void*)oa);

	return true;
}

bool CBlockManager::MakeBlockVirtual(uint32_t BlockID)
{
	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	SYNC_BEGINREAD(m_BlockSync);
	if (!InitOperation(BlockID, a, isvirtual, h))
	{
		SYNC_ENDREAD(m_BlockSync);
		return false;
	}

	if (isvirtual)
	{
		m_BlockTable[BlockID >> 2].Addr = m_BlockTable[BlockID >> 2].Addr | cForcedVirtualBlockFlag;
		SYNC_ENDREAD(m_BlockSync);
		return true;
	}

	uint32_t b = CreateVirtualBlock(BlockID, h.Size - sizeof(h) - sizeof(TBlockTailOcc));
	m_BlockTable[BlockID >> 2].Addr = m_BlockTable[BlockID >> 2].Addr | cForcedVirtualBlockFlag;

	Read(a, false, (void*)b, h.Size - sizeof(TBlockTailOcc));

	InsertFreeBlock(a, h.Size);

	SYNC_ENDREAD(m_BlockSync);
	return true;
}
