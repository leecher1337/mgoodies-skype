#pragma once

#include <windows.h>
#include <map>
#include <list>

#include "stdint.h"
#include "FileAccess.h"
#include "Cipher.h"


class CBlockManager
{
protected:
	static const uint32_t cVirtualBlockFlag = 0x00000001; // coded into addressfield of blocktable !!! malloc needs to align memory correctly !!!

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

	typedef std::map<uint32_t, uint32_t> TFreeBlockMap;

	typedef struct TBlockTableEntry {
		uint32_t Addr;
		//uint32_t Flags;
	} TBlockTableEntry;
	TBlockTableEntry* m_BlockTable;
	uint32_t m_TableSize;

	CFileAccess & m_FileAccess;
	CCipher * m_Cipher;

	uint32_t m_FirstBlockStart;
	uint32_t m_Granularity;
	TFreeBlockMap m_FreeBlocks;
	std::list<uint32_t> m_FreeIDs;

	void Read(uint32_t Addr, bool IsVirtual, void* Buffer, uint32_t Size);
	void Write(uint32_t Addr, bool IsVirtual, void* Buffer, uint32_t Size);
	void Zero(uint32_t Addr, bool IsVirtual, uint32_t Size);

	bool InitOperation(uint32_t BlockID, uint32_t & Addr, bool & IsVirtual, TBlockHeadOcc & Header);
	uint32_t CreateVirtualBlock(uint32_t BlockID, uint32_t ContentSize);

	void InsertFreeBlock(uint32_t Addr, uint32_t Size);	
	uint32_t FindFreePosition(uint32_t Size);
	void RemoveFreeBlock(uint32_t Addr, uint32_t Size);

	void PartWriteEncrypt(uint32_t BlockID, uint32_t Offset, uint32_t Size, void * Buffer, uint32_t Addr);
public:
	CBlockManager(CFileAccess & FileAccess);
	~CBlockManager();

	void SetCipher(CCipher *Cipher);
	uint32_t ScanFile(uint32_t FirstBlockStart, uint32_t HeaderSignature);

	bool ReadBlock(uint32_t BlockID, void * & Buffer, size_t & Size, uint32_t & Signature);
	bool WriteBlock(uint32_t BlockID, void * Buffer, size_t Size, uint32_t Signature);
	bool WriteBlockCheck(uint32_t BlockID, void * Buffer, size_t Size, uint32_t & Signature);

	bool ReadPart(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t & Signature);
	bool WritePart(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t Signature = 0);
	bool WritePartCheck(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t & Signature);

	uint32_t CreateBlock(size_t Size, uint32_t Signature);
	bool DeleteBlock(uint32_t BlockID);
	uint32_t ResizeBlock(uint32_t BlockID, uint32_t Size, bool SaveData = true);


};
