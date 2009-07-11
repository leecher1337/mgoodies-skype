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

#include "BlockManager.h"

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic (_InterlockedExchange)
#pragma intrinsic (_InterlockedCompareExchange64)
#pragma intrinsic (_interlockedbittestandset)
#else
#include "intrin_gcc.h"
#endif

const uint32_t CBlockManager::cCacheFactor[16] = {557,709,797,977,1109,1249,1297,1657,2297,2309,2377,2417,3049,3229,3709,3761};


CBlockManager::CBlockManager(
	CFileAccess & FileAccess,
	CEncryptionManager & EncryptionManager
	)
:	m_BlockSync(),
	m_FileAccess(FileAccess),
	m_EncryptionManager(EncryptionManager),
	m_BlockTable(1024),
	m_FreeBlocks(),
	m_FreeIDs()
{
	m_BlockTable[0].Addr = 0;
	m_OptimizeThread = NULL;
	m_Optimize = 0;
	m_CacheLastPurge = time(NULL);
	m_CacheOverflow = NULL;
	memset(m_Cache, 0, sizeof(m_Cache));
}
CBlockManager::~CBlockManager()
{
	TransactionBeginWrite();
	if (m_OptimizeThread)
	{
		m_OptimizeThread->FreeOnTerminate(false);
		m_OptimizeThread->Terminate();
		m_CacheLastPurge = 0;
		TransactionEndWrite();
		m_OptimizeThread->WaitFor();

		delete m_OptimizeThread;
	} else {
		m_CacheLastPurge = 0;
		TransactionEndWrite();
	}

	TCacheItem* i = m_Cache;
	while (i != m_Cache + sizeof(m_Cache) / sizeof(m_Cache[0]))
	{
		if (i->Buffer)
			free((void*)(i->Buffer & ~(cCacheChangedFlag | cCacheUsedFlag)));
		++i;
	}
}

// Optimize File Size
void CBlockManager::ExecuteOptimize()
{
	TBlockHeadFree h = {0,0};
	void * buf = NULL;
	uint32_t bufsize = 0;
	uint32_t lastflush = 0;

	{
		int i = 0;
		while (!m_OptimizeThread->Terminated() && (i < 600))
		{
			++i;
			Sleep(100); // wait for Miranda to start
		}
	}

	SYNC_BEGINWRITE(m_BlockSync);
	CacheFlush();
	m_FileAccess.FlushJournal();

	while (!m_OptimizeThread->Terminated() && (m_Optimize < m_FileAccess.Size()))
	{
		m_FileAccess.Read(&h, m_Optimize, sizeof(h));
		if (h.ID == cFreeBlockID)
		{
			m_Optimize += h.Size;
			RemoveFreeBlock(m_Optimize, h.Size);

		} else if (m_Optimize != m_OptimizeDest) {
			if (bufsize < h.Size)
			{
				buf = realloc(buf, h.Size);
				bufsize = h.Size;
			}
			m_FileAccess.Read(buf, m_Optimize, h.Size);
			m_FileAccess.Write(buf, m_OptimizeDest, h.Size);

			switch (m_BlockTable[h.ID >> 2].Addr & (cBlockIsCachedFlag | cBlockIsVirtualOnlyFlag))
			{
				case 0:                  m_BlockTable[h.ID >> 2].Addr = m_OptimizeDest; break;
				case cBlockIsCachedFlag: m_BlockTable[h.ID >> 2].Addr = m_OptimizeDest | cBlockIsCachedFlag; break;
			}

			m_OptimizeDest += h.Size;
			m_Optimize += h.Size;
		} else {
			m_OptimizeDest += h.Size;
			m_Optimize += h.Size;
		}
		
		if ((m_BlockSync.Waiting() > 0) || (m_OptimizeDest - lastflush > (1 << 20)))
		{
			TBlockHeadFree fh = {cFreeBlockID, m_Optimize - m_OptimizeDest};
			TBlockTailFree ft = {m_Optimize - m_OptimizeDest, cFreeBlockID};

			if (m_OptimizeDest != m_Optimize)
				InsertFreeBlock(m_OptimizeDest, m_Optimize - m_OptimizeDest, true, false);				

			//m_FileAccess.CompleteTransaction();
			m_FileAccess.CloseTransaction();
			m_FileAccess.FlushJournal();
			lastflush = m_OptimizeDest;
			
			if (m_BlockSync.Waiting() > 0)
			{
				SYNC_ENDWRITE(m_BlockSync);
				Sleep(m_BlockSync.Waiting() * 50 + 1);
				SYNC_BEGINWRITE(m_BlockSync);
				//CacheFlush();
				m_FileAccess.FlushJournal();
			}
		}
	}

	if (m_Optimize >= m_FileAccess.Size())
		m_FileAccess.Size(m_OptimizeDest);
	
	m_OptimizeThread = NULL;
	m_Optimize = 0;
	m_FileAccess.CompleteTransaction();
	m_FileAccess.CloseTransaction();
	m_FileAccess.FlushJournal();
	SYNC_ENDWRITE(m_BlockSync);

	if (buf)
		free(buf);
}
CBlockManager::TBlockHeadOcc* CBlockManager::CacheInsert(uint32_t ID, TBlockHeadOcc * Buffer, bool Modify)
{
	{
	
		uint32_t cachefactor = cCacheFactor[ID & 0xf];
		uint32_t index = ID & 0xfff;
		uint32_t count = 0;
		TCacheItem newitem, olditem, tmpitem;

		newitem.BlockID = ID;
		newitem.Buffer = (uint32_t)Buffer | cCacheUsedFlag;

		while (count < cCacheDepth)
		{
			olditem.Complete = _InterlockedCompareExchange64(&m_Cache[index].Complete, newitem.Complete, 0);
			
			if (olditem.Complete == 0)
			{
				if (Modify)
					_interlockedbittestandset((volatile long *)&m_Cache[index].Buffer, 0); // set cChacheChangedFlag	
				return Buffer;
			}

			if (olditem.BlockID == ID)
			{			
				if (!(olditem.Buffer & cCacheUsedFlag))
				{
					newitem.Buffer = olditem.Buffer | cCacheUsedFlag;
					tmpitem.Complete = _InterlockedCompareExchange64(&m_Cache[index].Complete, newitem.Complete, olditem.Complete);
				} else {
					tmpitem = olditem;
				}
				
				if (tmpitem.BlockID == ID)
				{
					if (Modify)
						_interlockedbittestandset((volatile long *)&m_Cache[index].Buffer, 0); // set cChacheChangedFlag	

					free(Buffer);
					return (TBlockHeadOcc*)(olditem.Buffer & ~(cCacheUsedFlag | cCacheChangedFlag));
				}
			
			} else if ((olditem.Buffer & cCacheUsedFlag) == 0)
			{
				tmpitem.Complete = _InterlockedCompareExchange64(&m_Cache[index].Complete, newitem.Complete, olditem.Complete);
				if (tmpitem.Complete == olditem.Complete)
				{				
					free((void*)(tmpitem.Buffer & ~(cCacheUsedFlag | cCacheChangedFlag)));
					if (Modify)
						_interlockedbittestandset((volatile long *)&m_Cache[index].Buffer, 0); // set cChacheChangedFlag	

					return Buffer;
				}
			}

			count++;
			index = (index * cachefactor + 3697) & 0xfff;
		}
	}


	{
		// insert into overflow stack
		TCacheOverflowItem * newstack = (TCacheOverflowItem*) malloc(sizeof(TCacheOverflowItem));
		volatile TCacheOverflowItem * oldstack;
		volatile TCacheOverflowItem * lastcheck = NULL;
		newstack->Item.BlockID = ID;
		newstack->Item.Buffer = (uint32_t)Buffer | cCacheUsedFlag;
		if (Modify)
			newstack->Item.Buffer = newstack->Item.Buffer | cCacheChangedFlag;

		do
		{
			oldstack = m_CacheOverflow;
			TCacheOverflowItem * i = (TCacheOverflowItem*) oldstack;
			while (i != lastcheck)	// check if item was added to the stack during this function
			{
				if (i->Item.BlockID == ID)
				{
					free(newstack);
					free(Buffer);
					return (TBlockHeadOcc*) (i->Item.Buffer & ~(cCacheChangedFlag | cCacheUsedFlag));
				}
				i = (TCacheOverflowItem*) i->Next;
			}
			lastcheck = oldstack;

			newstack->Next = oldstack;

		} while ((long)oldstack != _InterlockedCompareExchange((volatile long *)&m_CacheOverflow, (long)newstack, (long)oldstack));
	}

	return Buffer;
}

CBlockManager::TBlockHeadOcc* CBlockManager::CacheFind(uint32_t ID, bool Modify)
{
	uint32_t cachefactor = cCacheFactor[ID & 0xf];
	uint32_t index = ID & 0xfff;
	uint32_t count = 0;
	
	while (count < cCacheDepth)
	{
		if (m_Cache[index].BlockID == ID)
		{
			_interlockedbittestandset((volatile long*)&m_Cache[index].Buffer, 1); // set used flag
			if (m_Cache[index].BlockID == ID)
			{
				if (Modify)
					_interlockedbittestandset((volatile long*)&m_Cache[index].Buffer, 0); // set changed flag

				return (TBlockHeadOcc*)(m_Cache[index].Buffer & ~(cCacheUsedFlag | cCacheChangedFlag));
			} else {
				return NULL;
			}
		}

		count++;
		index = (index * cachefactor + 3697) & 0xfff;		
	}

	volatile TCacheOverflowItem * overflow = (TCacheOverflowItem*) m_CacheOverflow;
	while (overflow && (overflow->Item.BlockID != ID))
		overflow = overflow->Next;

	if (overflow)
	{
		if (Modify)
			_interlockedbittestandset((volatile long*)&overflow->Item.Buffer, 0);

		return (TBlockHeadOcc*) (overflow->Item.Buffer & ~(cCacheUsedFlag | cCacheChangedFlag));
	}

	return NULL;
}

void CBlockManager::CacheUpdate(uint32_t ID, TBlockHeadOcc * Buffer)
{
	uint32_t cachefactor = cCacheFactor[ID & 0xf];
	uint32_t index = ID & 0xfff;
	uint32_t count = 0;

	while (count < cCacheDepth)
	{
		if (m_Cache[index].BlockID == ID)
		{
			m_Cache[index].Buffer = (uint32_t)Buffer | cCacheUsedFlag | cCacheChangedFlag;

			return;
		}
		count++;
		index = (index * cachefactor + 3697) & 0xfff;		
	}

	volatile TCacheOverflowItem * overflow = m_CacheOverflow;
	while (overflow && (overflow->Item.BlockID != ID))
		overflow = overflow->Next;

	if (overflow)
	{
		overflow->Item.Buffer = (uint32_t)Buffer | cCacheUsedFlag | cCacheChangedFlag;
	}
}
void CBlockManager::CacheErase(uint32_t ID)
{
	uint32_t cachefactor = cCacheFactor[ID & 0xf];
	uint32_t index = ID & 0xfff;
	uint32_t count = 0;

	while (count < cCacheDepth)
	{
		if (m_Cache[index].BlockID == ID)
		{			
			free((TBlockHeadOcc*) (m_Cache[index].Buffer & ~(cCacheUsedFlag | cCacheChangedFlag)));
			m_Cache[index].Complete = 0;

			return;
		}
		count++;
		index = (index * cachefactor + 3697) & 0xfff;		
	}

	volatile TCacheOverflowItem * overflow = m_CacheOverflow;
	volatile TCacheOverflowItem * last = NULL;
	while (overflow && (overflow->Item.BlockID != ID))
	{
		last = overflow;
		overflow = overflow->Next;
	}

	if (overflow)
	{		
		if (last)
			last->Next = overflow->Next;
		else 
			m_CacheOverflow = overflow->Next;
		free((TBlockHeadOcc*) (overflow->Item.Buffer & ~(cCacheUsedFlag | cCacheChangedFlag)));
		free((TCacheOverflowItem*) overflow);
	}
}

inline void CBlockManager::CacheFlush()
{
	TCacheItem* i = m_Cache;
	TCacheOverflowItem* o = NULL;
	void * cryptbuf = NULL;
	uint32_t cryptbufsize = 0;

	while (i != NULL)
	{
		if (i->Buffer & cCacheChangedFlag)
		{
			TBlockHeadOcc * cache = (TBlockHeadOcc*)(i->Buffer & ~(cCacheChangedFlag | cCacheUsedFlag));
			if (m_EncryptionManager.IsEncrypted(i->BlockID << 2, ET_BLOCK))
			{
				if (cryptbufsize < cache->Size)
				{
					cryptbuf = realloc(cryptbuf, cache->Size);
					cryptbufsize = cache->Size;
				}
				memcpy(cryptbuf, cache, cache->Size);
				m_EncryptionManager.Encrypt((uint8_t*)cryptbuf + sizeof(TBlockHeadOcc), 
					cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc),
					ET_BLOCK, 
					i->BlockID << 2, 
					0);

				m_FileAccess.Write(cryptbuf, 
					m_BlockTable[i->BlockID].Addr & ~(cBlockIsCachedFlag | cBlockIsVirtualOnlyFlag),
					cache->Size);

			} else {
				m_FileAccess.Write(cache, 
					m_BlockTable[i->BlockID].Addr & ~(cBlockIsCachedFlag | cBlockIsVirtualOnlyFlag),
					cache->Size);
			}

			i->Buffer = i->Buffer & ~cCacheChangedFlag;
		}

		++i;
		if (i == m_Cache + sizeof(m_Cache) / sizeof(m_Cache[0]))
		{
			o = (TCacheOverflowItem*) m_CacheOverflow;
			i = &o->Item;
		} else if (o != NULL)
		{
			o = (TCacheOverflowItem*) o->Next;
			i = &o->Item;
		}
	}

	if (cryptbuf)
		free(cryptbuf);
}
inline void CBlockManager::CachePurge()
{
	if (((time(NULL) - m_CacheLastPurge > 60) || (m_CacheOverflow != NULL)) && SYNC_TRYBEGINWRITE(m_BlockSync))
	{
		
		m_FileAccess.FlushJournal();

		m_CacheLastPurge = time(NULL);

		TCacheItem* i = m_Cache;
		TCacheOverflowItem* o = NULL;

		while (i != NULL)
		{
			if (i->BlockID)
			{
				if (((i->Buffer & (cCacheUsedFlag | cCacheChangedFlag)) == 0) || (o != NULL))
				{
					free((void*)(i->Buffer & ~(cCacheUsedFlag | cCacheChangedFlag)));
					m_BlockTable[i->BlockID].Addr = m_BlockTable[i->BlockID].Addr & ~cBlockIsCachedFlag;
					i->Complete = 0;
				} else {
					i->Buffer = i->Buffer & ~cCacheUsedFlag;
				}
			}

			++i;
			if (i == m_Cache + sizeof(m_Cache) / sizeof(m_Cache[0]))
			{
				o = (TCacheOverflowItem*) m_CacheOverflow;				
				i = &o->Item;
			} else if (o != NULL)
			{
				TCacheOverflowItem* tmp = o;
				o = (TCacheOverflowItem*) o->Next;
				i = &o->Item;

				free(tmp);
			}
		}

		m_CacheOverflow = NULL;
		SYNC_ENDWRITE(m_BlockSync);
	}
	
}
uint32_t CBlockManager::ScanFile(uint32_t FirstBlockStart, uint32_t HeaderSignature, uint32_t FileSize)
{
	TBlockHeadOcc h, lasth = {0, 0, 0};
	uint32_t p;
	uint32_t res = 0;
	bool merge = false;

	p = FirstBlockStart;
	m_FirstBlockStart = FirstBlockStart;
	m_Optimize = 0;
	m_OptimizeDest = 0;

	while (p < FileSize)
	{
		m_FileAccess.Read(&h, p, sizeof(h));
		assertThrow(h.Size, _T("File Corrupt!"));

 		if (h.ID == cFreeBlockID)
		{
			if (m_OptimizeDest == 0)
				m_OptimizeDest = p;

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
				if (m_Optimize == 0)
					m_Optimize = p;

				InsertFreeBlock(p - lasth.Size, lasth.Size, merge, (p - lasth.Size) != m_OptimizeDest);
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

	m_FileAccess.CompleteTransaction();
	CacheFlush();
	m_FileAccess.CloseTransaction();
	m_FileAccess.FlushJournal();

	if (m_Optimize && !m_FileAccess.ReadOnly())
	{
		m_OptimizeThread = new COptimizeThread(*this);
		m_OptimizeThread->Priority(CThread::tpLowest);
		m_OptimizeThread->FreeOnTerminate(true);
		m_OptimizeThread->Resume();
	}

	return res;
}

inline uint32_t CBlockManager::GetAvailableID()
{
	uint32_t id;
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
	return id;
}

inline void CBlockManager::InsertFreeBlock(uint32_t Addr, uint32_t Size, bool InvalidateData, bool Reuse)
{
	if (Addr + Size == m_FileAccess.Size())
	{
		if (Reuse)
			m_FileAccess.Size(Addr);
	} else {
		TBlockHeadFree h = {cFreeBlockID, Size};
		TBlockTailFree t = {Size, cFreeBlockID};

		m_FileAccess.Write(&h, Addr, sizeof(h));
		if (InvalidateData)
			m_FileAccess.Invalidate(Addr + sizeof(h), Size - sizeof(h) - sizeof(t));
		m_FileAccess.Write(&t, Addr + Size - sizeof(t), sizeof(t));

//		if (Reuse)
//			m_FreeBlocks.insert(std::make_pair(Size, Addr));
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

inline uint32_t CBlockManager::FindFreePosition(uint32_t Size)
{
	// try to find free block
	TFreeBlockMap::iterator f;

	f = m_FreeBlocks.lower_bound(Size);
	while ((f != m_FreeBlocks.end()) && (f->first > Size) && (f->first < Size + sizeof(TBlockHeadFree) + sizeof(TBlockTailFree)))
		++f;

	uint32_t addr = 0;

	if (f == m_FreeBlocks.end()) // no block found - expand file
	{
		addr = m_FileAccess.Size();
		m_FileAccess.Size(addr + Size);
		
	} else {
		addr = f->second;

		if (f->first != Size)
		{
			InsertFreeBlock(addr + Size, f->first - Size, false, true);
		}
		InsertFreeBlock(addr, Size, false, false);
		
		m_FreeBlocks.erase(f);
	}

	return addr;
}

inline bool CBlockManager::InitOperation(uint32_t BlockID, uint32_t & Addr, TBlockHeadOcc* & Cache, bool Modify)
{
	if (BlockID & 3)
		return false;

	BlockID = BlockID >> 2;
	if (BlockID >= m_BlockTable.size())
		return false;

	Addr = m_BlockTable[BlockID].Addr;

	if (Addr == 0)
	{
		return false;
	}


	// 0 -> only in file
	// cBlockIsCachedFlag -> in cache and file
	// cBlockIsCachedFlag | cBlockIsVirtualOnly -> cached only, but not forced
	// cBlockIsVirtualOnlyFlag -> cached only, forced
	switch (Addr & (cBlockIsVirtualOnlyFlag | cBlockIsCachedFlag))
	{
		case cBlockIsVirtualOnlyFlag: case cBlockIsVirtualOnlyFlag | cBlockIsCachedFlag:
		{
			Cache = (TBlockHeadOcc*)(Addr & ~(cBlockIsVirtualOnlyFlag | cBlockIsCachedFlag));
			Addr = 0;

		} break;
		case cBlockIsCachedFlag:
		{			
			Cache = CacheFind(BlockID, Modify);
			Addr = Addr & ~(cBlockIsVirtualOnlyFlag | cBlockIsCachedFlag);

			if (Cache)
				break;
		}
		case 0:
		{
			TBlockHeadOcc h;
			m_FileAccess.Read(&h, Addr, sizeof(h));
			Cache = (TBlockHeadOcc*)malloc(h.Size);
			m_FileAccess.Read(Cache, Addr, h.Size);

			if (m_EncryptionManager.IsEncrypted(BlockID << 2, ET_BLOCK))
				m_EncryptionManager.Decrypt(Cache + 1, h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc), ET_BLOCK, BlockID << 2, 0);
			
			if (m_FileAccess.ReadOnly())
			{
				Addr = 0;
				_InterlockedExchange((volatile long *)&m_BlockTable[BlockID].Addr, (long)Cache | cBlockIsVirtualOnlyFlag | cBlockIsCachedFlag);
			} else {
				Cache = CacheInsert(BlockID, Cache, Modify);
				_interlockedbittestandset((volatile long *)&m_BlockTable[BlockID].Addr, 0 /*cBlockIsCachedFlag*/);
			}
		} break;
	}

	return true;
}

uint32_t CBlockManager::CreateBlock(uint32_t Size, uint32_t Signature)
{
	uint32_t id = GetAvailableID();

	Size = m_EncryptionManager.AlignSize(id << 2, ET_BLOCK, (Size + 3) & 0xfffffffc); // align on cipher after we aligned on 4 bytes

	TBlockHeadOcc h = {id << 2, Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc), Signature};
	TBlockTailOcc t = {id << 2};

	TBlockHeadOcc* cache = (TBlockHeadOcc*)malloc(Size + sizeof(h) + sizeof(t));
	memset(cache, 0, Size + sizeof(h) + sizeof(t));
	*cache = h;
	*(TBlockTailOcc*)(((uint8_t*)(cache + 1)) + Size) = t;

	if (m_FileAccess.ReadOnly())
	{
		m_BlockTable[id].Addr = (uint32_t)cache | cBlockIsVirtualOnlyFlag | cBlockIsCachedFlag;
	} else {
		uint32_t addr = FindFreePosition(Size + sizeof(h) + sizeof(t));
		CacheInsert(id, cache, true);
		m_BlockTable[id].Addr = addr | cBlockIsCachedFlag;		
	}

	return h.ID;
}

uint32_t CBlockManager::CreateBlockVirtual(uint32_t Size, uint32_t Signature)
{
	uint32_t id = GetAvailableID();

	Size = m_EncryptionManager.AlignSize(id << 2, ET_BLOCK, (Size + 3) & 0xfffffffc); // align on cipher after we aligned on 4 bytes

	TBlockHeadOcc h = {id << 2, Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc), Signature};
	TBlockTailOcc t = {id << 2};

	TBlockHeadOcc* cache = (TBlockHeadOcc*) malloc(Size + sizeof(h) + sizeof(t));
	memset(cache, 0, Size + sizeof(h) + sizeof(t));
	*cache = h;
	*(TBlockTailOcc*)(((uint8_t*)(cache + 1)) + Size) = t;

	m_BlockTable[id].Addr = (uint32_t)cache | cBlockIsVirtualOnlyFlag;

	return h.ID;
}
bool CBlockManager::DeleteBlock(uint32_t BlockID)
{
	uint32_t addr;
	TBlockHeadOcc* cache;

	if (!InitOperation(BlockID, addr, cache, false))
		return false;

	if (addr)
	{
		InsertFreeBlock(addr, cache->Size, true, true);
	}

	CacheErase(BlockID >> 2);
	m_BlockTable[BlockID >> 2].Addr = 0;
	// m_FreeIDs.push_back(BlockID); don't reuse blockids during a session to avoid side effects

	return true;
}
uint32_t CBlockManager::ResizeBlock(uint32_t BlockID, uint32_t Size, bool SaveData)
{
	if (Size == 0) return 0;

	Size = m_EncryptionManager.AlignSize(BlockID, ET_BLOCK, (Size + 3) & 0xfffffffc); // align on cipher after we aligned on 4 bytes

	uint32_t addr;
	TBlockHeadOcc* cache;

	if (!InitOperation(BlockID, addr, cache, true))
		return false;

	uint32_t os = cache->Size;
	uint32_t ns = Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc);
	if (ns == cache->Size)
		return Size;

	cache = (TBlockHeadOcc*)realloc(cache, ns);
	cache->Size = ns;
	TBlockTailOcc t = {BlockID};
	*(TBlockTailOcc*)(((uint8_t*)(cache + 1)) + Size) = t;

	if (m_FileAccess.ReadOnly())
	{
		m_BlockTable[BlockID >> 2].Addr = (uint32_t)cache | (m_BlockTable[BlockID >> 2].Addr & (cBlockIsCachedFlag | cBlockIsVirtualOnlyFlag));
	} else {
		InsertFreeBlock(addr, os, true, true);
		addr = FindFreePosition(ns);
		m_BlockTable[BlockID >> 2].Addr = addr | (m_BlockTable[BlockID >> 2].Addr & (cBlockIsCachedFlag | cBlockIsVirtualOnlyFlag));
		CacheUpdate(BlockID >> 2, cache);
	}

	return Size;
}

bool CBlockManager::IsForcedVirtual(uint32_t BlockID)
{
	BlockID = BlockID >> 2;

	if (BlockID >= m_BlockTable.size())
		return true;

	uint32_t addr = m_BlockTable[BlockID].Addr;

	if (addr == 0)
		return false;

	if ((addr & (cBlockIsVirtualOnlyFlag | cBlockIsCachedFlag)) == cBlockIsVirtualOnlyFlag)
		return true;

	return false;
}

bool CBlockManager::WriteBlockToDisk(uint32_t BlockID)
{
	uint32_t addr;
	TBlockHeadOcc* cache;

	if (!IsForcedVirtual(BlockID))
		return true;

	if (!InitOperation(BlockID, addr, cache, true))
		return false;
	
	if (m_FileAccess.ReadOnly())
	{
		m_BlockTable[BlockID >> 2].Addr = m_BlockTable[BlockID >> 2].Addr | cBlockIsCachedFlag | cBlockIsVirtualOnlyFlag;
	} else {
		m_BlockTable[BlockID >> 2].Addr = FindFreePosition(cache->Size) | cBlockIsCachedFlag;
		CacheInsert(BlockID >> 2, cache, true);
	}
	return true;
}

bool CBlockManager::MakeBlockVirtual(uint32_t BlockID)
{
	uint32_t addr;
	TBlockHeadOcc* cache;

	if (!InitOperation(BlockID, addr, cache, false))
		return false;

	if ((m_BlockTable[BlockID >> 2].Addr & (cBlockIsCachedFlag | cBlockIsVirtualOnlyFlag)) == cBlockIsCachedFlag)
		CacheErase(BlockID >> 2);
	m_BlockTable[BlockID >> 2].Addr = (uint32_t)cache | cBlockIsVirtualOnlyFlag;

	if (addr)
	{
		InsertFreeBlock(addr, cache->Size, true, true);
	}
	
	return true;
}

bool CBlockManager::ReadBlock(uint32_t BlockID, void * & Buffer, size_t & Size, uint32_t & Signature)
{
	uint32_t addr;
	TBlockHeadOcc* cache;
	
	if (!InitOperation(BlockID, addr, cache, false))
		return false;

	if ((Signature != 0) && (Signature != cache->Signature))
	{
		Signature = cache->Signature;
		return false;
	}
	Signature = cache->Signature;

	if ((Size != 0) && (Size != cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
	{
		Size = cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc);
		return false;
	}
	Size = cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc);

	if (Buffer == NULL)
		Buffer = malloc(Size);

	memcpy(Buffer, cache + 1, Size);
	return true;
}

bool CBlockManager::WriteBlock(uint32_t BlockID, void * Buffer, size_t Size, uint32_t Signature)
{
	uint32_t addr;
	TBlockHeadOcc* cache;
	
	if (!InitOperation(BlockID, addr, cache, true))
		return false;

	if (Size > cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc))
		return false;

	memcpy(cache + 1, Buffer, Size);
	cache->Signature = Signature;

	return true;
}
bool CBlockManager::WriteBlockCheck(uint32_t BlockID, void * Buffer, size_t Size, uint32_t & Signature)
{
	uint32_t addr;
	TBlockHeadOcc* cache;
	
	if (!InitOperation(BlockID, addr, cache, true))
		return false;

	if (Size > cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc))
		return false;

	if ((Signature != 0) && (cache->Signature != Signature))
	{
		Signature = cache->Signature;
		return false;
	}
	Signature = cache->Signature;

	memcpy(cache + 1, Buffer, Size);
	cache->Signature = Signature;

	return true;
}

bool CBlockManager::ReadPart(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t & Signature)
{
	uint32_t addr;
	TBlockHeadOcc* cache;
	
	if (!InitOperation(BlockID, addr, cache, false))
		return false;

	if ((Signature != 0) && (Signature != cache->Signature))
	{
		Signature = cache->Signature;
		return false;
	}
	Signature = cache->Signature;

	//if ((Size == 0) || ((Buffer == NULL) ^ (Size == 0))) // if the one is specified the other must be too
	if ((Size == 0) || (Buffer == NULL)) // same as above, simplified
		return false;

	if ((Size + Offset > cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
		return false;

	memcpy(Buffer, ((uint8_t*)(cache + 1)) + Offset, Size);
	return true;
}
bool CBlockManager::WritePart(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t Signature)
{
	uint32_t addr;
	TBlockHeadOcc* cache;
	
	if (!InitOperation(BlockID, addr, cache, true))
		return false;

	if ((Size + Offset > cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
		return false;

	if (Signature != 0)
	{
		cache->Signature = Signature;
	}

	if ((Size != 0) && (Buffer != NULL))
	{
		memcpy(((uint8_t*)(cache + 1)) + Offset, Buffer, Size);
	}

	return true;
}
bool CBlockManager::WritePartCheck(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t & Signature)
{
	uint32_t addr;
	TBlockHeadOcc* cache;
	
	if (!InitOperation(BlockID, addr, cache, true))
		return false;

	if ((Size + Offset > cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
		return false;

	if ((Signature != 0) && (Signature != cache->Signature))
	{
		Signature = cache->Signature;
		return false;
	}
	Signature = cache->Signature;

	if ((Size != 0) && (Buffer != NULL))
	{
		memcpy(((uint8_t*)(cache + 1)) + Offset, Buffer, Size);
	}

	return true;
}
