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

#pragma once

#include <windows.h>
#include <map>
#include <vector>

#include "Exception.h"
#ifdef _MSC_VER
#include "stdint.h"
#include <hash_map>
#else
#include <stdint.h>
#include <ext/hash_map>
#endif

#include "FileAccess.h"
#include "EncryptionManager.h"
#include "MREWSync.h"
#include "Thread.h"

class CBlockManager
{
protected:
	static const uint32_t cFreeBlockID = 0xFFFFFFFF;
	static const uint32_t cBlockIsCachedFlag = 0x00000001; // coded into addressfield of blocktable !!! malloc needs to align memory !!!
	static const uint32_t cBlockIsVirtualOnlyFlag = 0x00000002; // coded into addressfield of blocktable !!! malloc needs to align memory !!!
	static const uint32_t cCacheChangedFlag = 0x00000001; // coded into addressfield of blocktable !!! malloc needs to align memory !!!
	static const uint32_t cCacheUsedFlag = 0x00000002; // coded into addressfield of blocktable !!! malloc needs to align memory !!!


	#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary

	typedef struct TBlockHeadFree {
		uint32_t ID;
		uint32_t Size;
	} TBlockHeadFree;
	typedef struct TBlockHeadOcc {
		uint32_t ID;
		uint32_t Size;
		uint32_t Signature; /// if occupied block
	}	TBlockHeadOcc;

	typedef struct TBlockTailOcc {
		uint32_t ID;
	} TBlockTailOcc;

	typedef struct TBlockTailFree {
		uint32_t Size; /// if free block
		uint32_t ID;
	} TBlockTailFree;

	#pragma pack(pop)

	typedef std::multimap<uint32_t, uint32_t> TFreeBlockMap;

	class COptimizeThread : public CThread
	{
	protected:
		CBlockManager & m_Owner;
		void Execute() {m_Owner.ExecuteOptimize();};
	public:
		COptimizeThread(CBlockManager & Owner) : CThread(true), m_Owner(Owner) {};
		~COptimizeThread() {};
	};
	
	typedef struct TBlockTableEntry {
		uint32_t Addr;
	} TBlockTableEntry;
	std::vector<TBlockTableEntry> m_BlockTable;

#pragma pack(push, 1)
	typedef union TCacheItem 
	{
		volatile int64_t Complete;
		struct {
			volatile uint32_t BlockID;
			volatile uint32_t Buffer;
		};
	} TCacheItem;
#pragma pack(pop)

	TCacheItem m_Cache[4096];
	uint32_t m_CacheLastPurge;

	static const uint32_t cCacheAddend = 3697;
	static const uint32_t cCacheFactor[16];

	typedef struct TCacheOverflowItem
	{		
		TCacheItem Item;
		volatile TCacheOverflowItem * Next;
	} TCacheOverflowItem;

	volatile TCacheOverflowItem* m_CacheOverflow;
	
	CFileAccess & m_FileAccess;
	CEncryptionManager & m_EncryptionManager;
	CMultiReadExclusiveWriteSynchronizer m_BlockSync;

	uint32_t m_FirstBlockStart;
	TFreeBlockMap m_FreeBlocks;
	std::vector<uint32_t> m_FreeIDs;

	uint32_t m_Optimize;
	uint32_t m_OptimizeDest;
	COptimizeThread * m_OptimizeThread;
	void ExecuteOptimize();
	
	TBlockHeadOcc* CacheInsert(uint32_t ID, TBlockHeadOcc * Buffer, bool Modify);
	TBlockHeadOcc* CacheFind(uint32_t ID, bool Modify);
	void CacheUpdate(uint32_t ID, TBlockHeadOcc * Buffer);
	void CacheErase(uint32_t ID);
	void CacheFlush();
	void CachePurge();

	bool InitOperation(uint32_t BlockID, uint32_t & Addr, TBlockHeadOcc* & Cache, bool Modify);
	uint32_t GetAvailableID();
	void InsertFreeBlock(uint32_t Addr, uint32_t Size, bool InvalidateData, bool Reuse);
	void RemoveFreeBlock(uint32_t Addr, uint32_t Size);
	uint32_t FindFreePosition(uint32_t Size);
public:
	CBlockManager(
		CFileAccess & FileAccess,
		CEncryptionManager & EncryptionManager
		);
	~CBlockManager();

	void TransactionBeginRead();
	void TransactionBeginWrite();
	void TransactionEndRead();
	void TransactionEndWrite();

	uint32_t ScanFile(uint32_t FirstBlockStart, uint32_t HeaderSignature, uint32_t FileSize);

	bool ReadBlock(uint32_t BlockID, void * & Buffer, size_t & Size, uint32_t & Signature);
	bool WriteBlock(uint32_t BlockID, void * Buffer, size_t Size, uint32_t Signature);
	bool WriteBlockCheck(uint32_t BlockID, void * Buffer, size_t Size, uint32_t & Signature);

	bool ReadPart(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t & Signature);
	bool WritePart(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t Signature = 0);
	bool WritePartCheck(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t & Signature);

	uint32_t CreateBlock(uint32_t Size, uint32_t Signature);
	uint32_t CreateBlockVirtual(uint32_t Size, uint32_t Signature);
	bool     DeleteBlock(uint32_t BlockID);
	uint32_t ResizeBlock(uint32_t BlockID, uint32_t Size, bool SaveData = true);

	bool IsForcedVirtual(uint32_t BlockID);
	bool WriteBlockToDisk(uint32_t BlockID);
	bool MakeBlockVirtual(uint32_t BlockID);
};
