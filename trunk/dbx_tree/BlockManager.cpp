#include "BlockManager.h"


CBlockManager::CBlockManager(CFileAccess & FileAccess)
: m_FileAccess(FileAccess),
	m_FreeBlocks(),
	m_FreeIDs()
{
	m_BlockTable = (TBlockTableEntry*)malloc(sizeof(TBlockTableEntry));
	m_BlockTable[0].Addr = 0;
//	m_BlockTable[0].Flags = 0;
	m_TableSize = 1;

	m_Granularity = 4;

}
CBlockManager::~CBlockManager()
{
	if (m_BlockTable)
		free(m_BlockTable);
}

void CBlockManager::SetCipher(CCipher *Cipher)
{
	m_Cipher = Cipher;
	if (Cipher)
	{
		int i = 1;
		while (m_Granularity % Cipher->BlockSizeBytes())
		{
			m_Granularity = m_Granularity / i * (i+1);
			++i;
		}
	}
}

uint32_t CBlockManager::ScanFile(uint32_t FirstBlockStart, uint32_t HeaderSignature)
{
	TBlockHeadOcc h;
	uint32_t p;
	uint32_t res = 0;
	p = FirstBlockStart;
	m_FirstBlockStart = FirstBlockStart;

	while (p < m_FileAccess.GetSize())
	{
		m_FileAccess.Read(&h, p, sizeof(h));
		if (h.ID == 0)
		{
			m_FreeBlocks.insert(std::make_pair(h.Size, p));
		} else {
			uint32_t oldsize = m_TableSize;
			while ((h.ID >> 2) >= m_TableSize)
				m_TableSize = m_TableSize << 1;

			if (oldsize != m_TableSize)
			{
				m_BlockTable = (TBlockTableEntry*) realloc(m_BlockTable, sizeof(TBlockTableEntry) * m_TableSize);
				memset(&m_BlockTable[oldsize], 0, sizeof(TBlockTableEntry) * (m_TableSize - oldsize));
			}			

			m_BlockTable[h.ID >> 2].Addr = p;

			if (h.Signature == HeaderSignature)
				res = h.ID;
		}

		p = p + h.Size;
	}

	for (uint32_t i = 1; i < m_TableSize; ++i)
	{
		if (m_BlockTable[i].Addr == 0)
			m_FreeIDs.push_back(i);
	}

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

inline void CBlockManager::Zero(uint32_t Addr, bool IsVirtual, uint32_t Size)
{
	if (IsVirtual)
	{
	memset((void*)Addr, 0, Size);
	} else {
		void *buf = malloc(Size);
		memset(buf,0,Size);
		Write(Addr, false, buf, Size);
		free(buf);
	}
}


inline bool CBlockManager::InitOperation(uint32_t BlockID, uint32_t & Addr, bool & IsVirtual, TBlockHeadOcc & Header)
{
	BlockID = BlockID >> 2;
	if (BlockID >= m_TableSize)
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
	uint32_t res = (uint32_t)malloc(ContentSize + sizeof(TBlockHeadOcc));
	m_BlockTable[BlockID].Addr = res | cVirtualBlockFlag;
	return res;
}

inline void CBlockManager::InsertFreeBlock(uint32_t Addr, uint32_t Size)
{
	TBlockTailFree ft;
	TBlockHeadFree fh;

	if (Addr > m_FirstBlockStart) // don't go before our start.
	{
		Read(Addr - sizeof(ft), false, &ft, sizeof(ft));
		if (ft.ID == 0)
		{ // free block in front of ours
			Addr = Addr - ft.Size;
			Size = Size + ft.Size;

			RemoveFreeBlock(Addr, Size);
		}
	}

	if (Addr < m_FileAccess.GetSize())
	{
		Read(Addr + Size, false, &fh, sizeof(fh));
		if (fh.ID == 0) 
		{ // free block beyond our block				
			RemoveFreeBlock(Addr + Size, fh.Size);

			Size = Size + fh.Size;
		}
	}

	fh.ID = 0;
	fh.Size = Size;
	ft.ID = 0;
	ft.Size = Size;

	Zero(Addr + sizeof(fh), false, Size - sizeof(fh) - sizeof(ft));
	
	Write(Addr, false, &fh, sizeof(fh));
	Write(Addr + Size - sizeof(ft), false, &ft, sizeof(ft));

	m_FreeBlocks.insert(std::make_pair(Size, Addr));

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
	TFreeBlockMap::iterator f = m_FreeBlocks.end();

	if (m_FreeBlocks.rbegin()->first >= Size) // check if we can fulfill the request
	{
		int i = 0;
		while ((i < 5) && (m_FreeBlocks.end() == f)) // try to find perfect fit
		{
			f = m_FreeBlocks.find(Size * i);
			++i;		
		}

		if (m_FreeBlocks.end() == f) // try worst fit strategy. take the biggest block available that has at least the double size
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
		if (f->first != Size)
			m_FreeBlocks.insert(std::make_pair(f->first - Size, f->second + Size)); // we have some space left.

		m_FreeBlocks.erase(f);
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

	if (!InitOperation(BlockID, a, isvirtual, h))
		return false;
	
	if ((Signature != 0) && (Signature != h.Signature))
	{
		Signature = h.Signature;
		return false;
	}
	Signature = h.Signature;

	if ((Size != 0) && (Size != h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
	{
		Size = h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc);
		return false;
	} 

	if (Buffer == NULL)
		Buffer = malloc(Size);
	
	Read(a + sizeof(h), isvirtual, Buffer, Size);
	
	if ((m_Cipher) && !isvirtual)
	{
		m_Cipher->Decrypt(Buffer, Size, BlockID);
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

	if (!InitOperation(BlockID, a, isvirtual, h))
		return false;

	if (Size != h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc))
		return false;

	if ((!isvirtual) && m_FileAccess.GetReadOnly())
	{
		a = CreateVirtualBlock(BlockID, Size);
		isvirtual = true;
	}

	h.Signature = Signature;

	if ((m_Cipher) && !isvirtual)
	{
		m_Cipher->Encrypt(Buffer, Size, BlockID);
	}
	Write(a, isvirtual, &h, sizeof(h));
	Write(a + sizeof(h), isvirtual, Buffer, Size);

	return true;
}
bool CBlockManager::WriteBlockCheck(uint32_t BlockID, void * Buffer, size_t Size, uint32_t & Signature)
{
	if ((Buffer == NULL) || (Size == 0)) // must be something usefull...
		return false;

	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	if (!InitOperation(BlockID, a, isvirtual, h))
		return false;

	if ((Signature != 0) && (h.Signature != Signature))
	{
		Signature = h.Signature;
		return false;
	}
	Signature = h.Signature;

	if (Size != h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc))
		return false;


	if ((!isvirtual) && m_FileAccess.GetReadOnly())
	{
		a = CreateVirtualBlock(BlockID, Size);
		isvirtual = true;
	}

	if ((m_Cipher) && !isvirtual)
	{
		m_Cipher->Encrypt(Buffer, Size, BlockID);
	}
	Write(a, isvirtual, &h, sizeof(h));
	Write(a + sizeof(h), isvirtual, Buffer, Size);

	return true;
}

bool CBlockManager::ReadPart(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t & Signature)
{
	//if ((Size == 0) || ((Buffer == NULL) ^ (Size == 0))) // if the one is specified the other must be too
	if ((Size == 0) || (Buffer == NULL)) // same as above, simplified
		return false;

	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	if (!InitOperation(BlockID, a, isvirtual, h))
		return false;
	
	if ((Signature != 0) && (Signature != h.Signature))
	{
		Signature = h.Signature;
		return false;
	}
	Signature = h.Signature;

	if ((Size + Offset > h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
		return false;

	if ((m_Cipher) && !isvirtual)
	{
		uint32_t cryptoffset;
		uint32_t cryptsize;
		void* cryptbuf;

		cryptoffset = Offset - (Offset % m_Cipher->BlockSizeBytes());
		cryptsize = Size + (Offset % m_Cipher->BlockSizeBytes());
		if (cryptsize % m_Cipher->BlockSizeBytes() != 0)
			cryptsize = cryptsize + m_Granularity - (cryptsize % m_Cipher->BlockSizeBytes());

		cryptbuf = malloc(cryptsize);
		Read(a + sizeof(h) + cryptoffset, false, cryptbuf, cryptsize);
		m_Cipher->Decrypt(cryptbuf, cryptsize, BlockID + cryptoffset);
		
		memcpy(Buffer, (uint8_t*)cryptbuf + (Offset - cryptoffset), Size);
		free(cryptbuf);
	} else {
		Read(a + sizeof(h) + Offset, isvirtual, Buffer, Size);
	}
		
	return true;
}

void CBlockManager::PartWriteEncrypt(uint32_t BlockID, uint32_t Offset, uint32_t Size, void * Buffer, uint32_t Addr)
{
	uint32_t cryptoffset;
	uint32_t cryptsize;
	void* cryptbuf;
	bool loadlast = false;

	cryptoffset = Offset - (Offset % m_Cipher->BlockSizeBytes());
	cryptsize = Size + (Offset % m_Cipher->BlockSizeBytes());
	if (cryptsize % m_Cipher->BlockSizeBytes() != 0)
	{
		cryptsize = cryptsize + m_Granularity - (cryptsize % m_Cipher->BlockSizeBytes());
		loadlast = true;
	}

	cryptbuf = malloc(cryptsize);
	
	if (loadlast)
	{
		Read(Addr + sizeof(TBlockHeadOcc) + cryptoffset + cryptsize - m_Cipher->BlockSizeBytes(), false, (uint8_t*)cryptbuf + cryptsize - m_Cipher->BlockSizeBytes(), m_Cipher->BlockSizeBytes());
		m_Cipher->Decrypt((uint8_t*)cryptbuf + cryptsize - m_Cipher->BlockSizeBytes(), m_Cipher->BlockSizeBytes(), BlockID + cryptsize - m_Cipher->BlockSizeBytes());
	}

	if ((cryptoffset != Offset) && ((!loadlast) || (cryptsize > m_Cipher->BlockSizeBytes())))
	{
		Read(Addr + sizeof(TBlockHeadOcc), false, cryptbuf, m_Cipher->BlockSizeBytes());
		m_Cipher->Decrypt(cryptbuf, m_Cipher->BlockSizeBytes(), BlockID + cryptoffset);
	}

	memcpy((uint8_t*)cryptbuf + (Offset - cryptoffset), Buffer, Size);

	m_Cipher->Encrypt(cryptbuf, cryptsize, BlockID + cryptoffset);
	Write(Addr + sizeof(TBlockHeadOcc) + cryptoffset, false, cryptbuf, cryptsize);
	free(cryptbuf);
}

bool CBlockManager::WritePart(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t Signature)
{
	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	if (!InitOperation(BlockID, a, isvirtual, h))
		return false;
	
	if ((Size + Offset > h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
		return false;

	if ((!isvirtual) && m_FileAccess.GetReadOnly())
	{
		int b = a;
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
		if ((m_Cipher) && !isvirtual)
		{
			PartWriteEncrypt(BlockID, Offset, Size, Buffer, a);
		} else {
			Write(a + sizeof(h) + Offset, isvirtual, Buffer, Size);
		}
	}
		
	return true;
}
bool CBlockManager::WritePartCheck(uint32_t BlockID, void * Buffer, uint32_t Offset, size_t Size, uint32_t & Signature)
{
	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	if (!InitOperation(BlockID, a, isvirtual, h))
		return false;
	
	if ((Size + Offset > h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
		return false;

	if ((Signature != 0) && (Signature != h.Signature))
	{
		Signature = h.Signature;
		return false;
	}
	Signature = h.Signature;

	if ((!isvirtual) && m_FileAccess.GetReadOnly())
	{
		int b = a;
		a = CreateVirtualBlock(BlockID, h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc));
		Read(b, false, (void*)a, h.Size - sizeof(TBlockTailOcc)); // copy block.
		isvirtual = true;
	}

	if ((Size != 0) && (Buffer != NULL))
	{
		if ((m_Cipher) && !isvirtual)
		{
			PartWriteEncrypt(BlockID, Offset, Size, Buffer, a);
		} else {
			Write(a + sizeof(h) + Offset, isvirtual, Buffer, Size);
		}
	}

	return true;
}

uint32_t CBlockManager::CreateBlock(size_t Size, uint32_t Signature)
{
	if (Size % m_Granularity > 0)
		Size = Size - Size % m_Granularity + m_Granularity;

	uint32_t id = 0;
	if (m_FreeIDs.empty())
	{
		id = m_TableSize;

		m_TableSize = m_TableSize << 1;
		m_BlockTable = (TBlockTableEntry*) realloc(m_BlockTable, sizeof(TBlockTableEntry) * m_TableSize);
		memset(&m_BlockTable[id], 0, id);
		
		for (uint32_t i = 1 + id; i < m_TableSize; ++i)
			m_FreeIDs.push_back(i);
	} else {
		id = m_FreeIDs.front();
		m_FreeIDs.pop_front();
	}

	TBlockHeadOcc h;
	h.ID = id << 2;
	h.Signature = Signature;
	h.Size = Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc);
	if (m_FileAccess.GetReadOnly())
	{
		uint32_t a = CreateVirtualBlock(h.ID, Size);
		Zero(a, true, Size + sizeof(TBlockHeadOcc));
		
	} else {

		uint32_t addr = FindFreePosition(h.Size);
		
		TBlockTailOcc t;
		t.ID = h.ID;

		Zero(addr + sizeof(h), false, Size);
		
		Write(addr, false, &h, sizeof(h));
		Write(addr + sizeof(h) + Size, false, &t, sizeof(t));
		m_BlockTable[id].Addr = addr;
		
	}

	return h.ID;
}
bool CBlockManager::DeleteBlock(uint32_t BlockID)
{
	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;

	if (!InitOperation(BlockID, a, isvirtual, h))
		return false;

	BlockID = BlockID >> 2;

	m_BlockTable[BlockID].Addr = 0;
	m_FreeIDs.push_back(BlockID);
	
	if (isvirtual)
	{
		free((void*)a);		

	} else if (!m_FileAccess.GetReadOnly()) {
		InsertFreeBlock(a, h.Size);
	}

	return true;
}
uint32_t CBlockManager::ResizeBlock(uint32_t BlockID, uint32_t Size, bool SaveData)
{
	if (Size == 0) return 0;

	if (Size % m_Granularity > 0)
		Size = Size - Size % m_Granularity + m_Granularity;
	

	uint32_t a;
	bool isvirtual;
	TBlockHeadOcc h;
	if (!InitOperation(BlockID, a, isvirtual, h))
		return false;

	if (Size == h.Size - sizeof(h) - sizeof(TBlockTailOcc))
		return Size;

	if (isvirtual)
	{
		a = (uint32_t) realloc((void*)a, Size + sizeof(h));
		Zero(a + h.Size, true, Size + sizeof(h) - h.Size);

		h.Size = Size + sizeof(h);		
		memcpy((void*)a, &h, sizeof(h));
		m_BlockTable[BlockID >> 2].Addr = a | cVirtualBlockFlag;
	} else if (m_FileAccess.GetReadOnly())
	{
		void* buf = NULL;
		if (SaveData)
		{
			buf = malloc(h.Size - sizeof(TBlockTailOcc));
			Read(a, false, buf, h.Size - sizeof(TBlockTailOcc));
		}
		a = CreateVirtualBlock(BlockID, Size);

		if (buf)
		{
			memcpy((void*)a, buf, h.Size - sizeof(TBlockTailOcc));
			free(buf);
		}

		h.Size = Size + sizeof(h);		
		memcpy((void*)a, &h, sizeof(h));
	} else { // resize in file	
		TBlockTailOcc t;
		t.ID = BlockID;

		if (h.Size - sizeof(h) - sizeof(t) < Size) // enlarge
		{
			if (a + h.Size >= m_FileAccess.GetSize()) // fileend
			{
				m_FileAccess.SetSize(a + Size + sizeof(h) + sizeof(t));

				Zero(a + h.Size - sizeof(t), false, Size - (h.Size - sizeof(h) - sizeof(t)));
				
				h.Size = Size + sizeof(h) + sizeof(t);
				Write(a, false, &h, sizeof(h));
				Write(a + h.Size - sizeof(t), false, &t, sizeof(t));

			} else { // normal enlarge
				TBlockHeadFree fh;
				
				bool newalloc = true;
				bool split = false;

				Read(a + h.Size, false, &fh, sizeof(fh));
				if (fh.ID == 0) // we have an empty block behind.
				{
					if (fh.Size < Size - (h.Size - sizeof(h) - sizeof(t)))
					{ // block too small :-(
						newalloc = true;
					} else if (fh.Size - sizeof(TBlockHeadFree) - sizeof(TBlockTailFree) < Size - sizeof(h) - sizeof(t)) // following block nearly used complete (we cannot write the boundmarkers again)
					{
						newalloc = false;
					} else if (fh.Size < Size + h.Size) // cannot use the free block, because we don't know if enough will left to use it again.
					{
						newalloc = true;
					} else // huge free block, use it
					{
						newalloc = false;
						split = true;
					}
				}

				if (newalloc)
				{
					uint32_t na = FindFreePosition(Size + sizeof(h) + sizeof(t));

					Zero(na + h.Size - sizeof(t), false, Size - h.Size);

					void* buf = malloc(h.Size - sizeof(h) - sizeof(t));
					Read(a + sizeof(h), false, buf, h.Size - sizeof(h) - sizeof(t));
					Write(na + sizeof(h), false, buf, Size);
					free(buf);					

					InsertFreeBlock(a, h.Size);
					
					h.Size = Size + sizeof(h) + sizeof(t);
					Write(na, false, &h, sizeof(h));
					Write(na + h.Size - sizeof(t), false, &t, sizeof(t));
				} else {
					RemoveFreeBlock(a + h.Size, fh.Size);

					if (split)
					{
						Zero(a + h.Size - sizeof(t), false, sizeof(t) + sizeof(fh));

						InsertFreeBlock(a + Size + sizeof(h) + sizeof(t), fh.Size - (Size - (h.Size - sizeof(h) - sizeof(t))));

						h.Size = Size + sizeof(h) + sizeof(t);
						Write(a, false, &h, sizeof(h));
						Write(a + h.Size - sizeof(t), false, &t, sizeof(t));
					} else {
						Zero(a + h.Size - sizeof(t), false, fh.Size + sizeof(t));

						h.Size = h.Size + fh.Size;
						Write(a, false, &h, sizeof(h));
						Write(a + h.Size - sizeof(t), false, &t, sizeof(t));
						Size = h.Size - sizeof(h) - sizeof(t);
					}
				}
			}

		} else { // shrink block.
			if (h.Size - sizeof(h) - sizeof(t) - Size > sizeof(TBlockHeadFree) + sizeof(TBlockTailFree))
			{ // produce new free block
				uint32_t fs = h.Size - sizeof(h) - sizeof(t) - Size;

				h.Size = Size + sizeof(h) + sizeof(t);
				Write(a, false, &h, sizeof(h));
				Write(a + h.Size - sizeof(t), false, &t, sizeof(t));
				
				InsertFreeBlock(a + h.Size, fs);
			} else { // do nothing.
				Size = h.Size - sizeof(h) - sizeof(t);
			}
		}
	}


	return Size;

}
