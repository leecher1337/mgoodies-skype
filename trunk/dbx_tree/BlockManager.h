#pragma once

#include <windows.h>
#include "stdint.h"
#include "FileAccess.h"
#include "Cipher.h"


class CBlockManager
{
protected:
	typedef struct TBlockEntry {
		uint32_t Position;
		size_t Size;
		uint32_t Signature;
	} TBlockEntry, *PBlockEntry;

	TBlockEntry* m_BlockTable;
	uint32_t m_TableSize;


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
