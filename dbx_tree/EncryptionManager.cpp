#include "EncryptionManager.h"
#include "CipherList.inc"

static const uint32_t cFileBlockMask = 0xfffff000;

CEncryptionManager::CEncryptionManager()
{
	m_Cipher = NULL;
	m_OldCipher = NULL;
	m_Type = ET_NONE;
	m_OldType = ET_NONE;
	m_Changing = false;
	m_ChangingProcess = 0;
}

bool CEncryptionManager::InitEncryption(TFileEncryption & Enc)
{
	m_Type = (TEncryptionType)(Enc.AccessType & ET_MASK);
		
	if (Enc.AccessType & cEncryptionChangingFlag)
	{
		m_Changing = true;
		m_ChangingProcess = Enc.ConversionProcess;
		m_OldType = (TEncryptionType)((Enc.AccessType >> 8) & ET_MASK);
	}

	uint32_t i = 0;
	while ((i < sizeof(cCipherList) / sizeof(cCipherList[0])) && (((m_Type != ET_NONE) && (m_Cipher == NULL)) || ((m_OldType != ET_NONE) && (m_OldCipher == NULL))))
	{
		if ((m_Type != ET_NONE) && (m_Cipher == NULL) && (cCipherList[i].ID == Enc.CipherID))
			m_Cipher = cCipherList[i].Create();

		if ((m_OldType != ET_NONE) && (m_OldCipher == NULL) && (cCipherList[i].ID == Enc.CipherOldID))
			m_OldCipher = cCipherList[i].Create();

		++i;
	}

	return true;
}
CEncryptionManager::~CEncryptionManager()
{
	if (m_Cipher)
		delete m_Cipher;
	m_Cipher = NULL;
	if (m_OldCipher)
		delete m_OldCipher;
	m_OldCipher = NULL;
}

bool CEncryptionManager::AlignData(uint32_t ID, TEncryptionType Type, uint32_t & Start, uint32_t & End)
{
	if (Type == ET_FILE)
		ID = ID & cFileBlockMask;

	if (m_Cipher && (Type == m_Type) && (!m_Changing || (ID < m_ChangingProcess)))
	{
		if (m_Cipher->IsStreamCipher())
		{
			if (Type == ET_FILE)
			{
				Start = Start & cFileBlockMask; // Start - Start % 4096;
				End = End - End % m_Cipher->BlockSizeBytes() + m_Cipher->BlockSizeBytes();
			} else {
				Start = 0;
				End = End - End % m_Cipher->BlockSizeBytes() + m_Cipher->BlockSizeBytes();
			}
		} else {
			Start = Start - Start % m_Cipher->BlockSizeBytes();
			if (End % m_Cipher->BlockSizeBytes())
				End = End - End % m_Cipher->BlockSizeBytes() + m_Cipher->BlockSizeBytes();
		}

		return true;
	} else if (m_OldCipher && m_Changing && (Type == m_OldType) && (ID >= m_ChangingProcess))
	{
		if (m_OldCipher->IsStreamCipher())
		{
			Start = 0;
			End = End - End % m_OldCipher->BlockSizeBytes() + m_OldCipher->BlockSizeBytes();
		} else {
			Start = Start - Start % m_OldCipher->BlockSizeBytes();
			if (End % m_OldCipher->BlockSizeBytes())
				End = End - End % m_OldCipher->BlockSizeBytes() + m_OldCipher->BlockSizeBytes();
		}

		return true;
	}

	return false;
}
uint32_t CEncryptionManager::AlignSize(uint32_t ID, TEncryptionType Type, uint32_t Size)
{
	if (Type == ET_FILE)
		ID = ID & cFileBlockMask;
	
	if (m_Cipher && (Type == m_Type) && (!m_Changing || (ID < m_ChangingProcess)))
	{
		if (Size % m_Cipher->BlockSizeBytes())
			return Size - Size % m_Cipher->BlockSizeBytes() + m_Cipher->BlockSizeBytes();

	} else if (m_OldCipher && m_Changing && (Type == m_OldType) && (ID >= m_ChangingProcess))
	{
		if (Size % m_OldCipher->BlockSizeBytes())
			return Size - Size % m_OldCipher->BlockSizeBytes() + m_OldCipher->BlockSizeBytes();

	}

	return Size;
}

bool CEncryptionManager::IsEncrypted(uint32_t ID, TEncryptionType Type)
{
	if (Type == ET_FILE)
		ID = ID & cFileBlockMask;

	return (m_Cipher && (Type == m_Type) && (!m_Changing || (ID < m_ChangingProcess))) ||
	       (m_OldCipher && m_Changing && (Type == m_OldType) && (ID >= m_ChangingProcess));
}

void CEncryptionManager::Encrypt(void* Data, uint32_t DataLength, TEncryptionType Type, uint32_t ID, uint32_t StartByte)
{
	if (Type == ET_FILE)
		ID = ID & cFileBlockMask;

	if (Type == ET_FILE)
	{
		if ((m_Type == ET_FILE) || (m_OldType == ET_FILE))
		{
			if (StartByte & cFileBlockMask) // handle leading partial block
			{
				uint32_t d = (~cFileBlockMask) + 1 - (StartByte & cFileBlockMask);

				if (m_Cipher && (Type == m_Type) && (!m_Changing || (ID < m_ChangingProcess)))
				{
					m_Cipher->Encrypt(Data, d, ID, StartByte);
				} else if (m_OldCipher && m_Changing && (Type == m_OldType) && (ID >= m_ChangingProcess))
				{
					m_OldCipher->Encrypt(Data, d, ID, StartByte);
				}

				DataLength -= d;
				ID += (~cFileBlockMask) + 1;
				StartByte += d;
				Data = (uint8_t *)Data + d;
			}

			while (DataLength >= (~cFileBlockMask) + 1)
			{
				if (m_Cipher && (Type == m_Type) && (!m_Changing || (ID < m_ChangingProcess)))
				{
					m_Cipher->Encrypt(Data, (~cFileBlockMask) + 1, ID, StartByte);
				} else if (m_OldCipher && m_Changing && (Type == m_OldType) && (ID >= m_ChangingProcess))
				{
					m_OldCipher->Encrypt(Data, (~cFileBlockMask) + 1, ID, StartByte);
				}

				DataLength -= (~cFileBlockMask) + 1;
				ID += (~cFileBlockMask) + 1;
				StartByte += (~cFileBlockMask) + 1;
				Data = (uint8_t *)Data + (~cFileBlockMask) + 1;
			}
		}
	} 
	
	if (DataLength > 0) // last partial file block handled here, also all other 
	{
		if (m_Cipher && (Type == m_Type) && (!m_Changing || (ID < m_ChangingProcess)))
		{
			m_Cipher->Encrypt(Data, DataLength, ID, StartByte);
		} else if (m_OldCipher && m_Changing && (Type == m_OldType) && (ID >= m_ChangingProcess))
		{
			m_OldCipher->Encrypt(Data, DataLength, ID, StartByte);
		}
	}
}
void CEncryptionManager::Decrypt(void* Data, uint32_t DataLength, TEncryptionType Type, uint32_t ID, uint32_t StartByte)
{	
	if (Type == ET_FILE)
		ID = ID & cFileBlockMask;

	if (Type == ET_FILE)
	{
		if ((m_Type == ET_FILE) || (m_OldType == ET_FILE))
		{
			if (StartByte & cFileBlockMask) // handle leading partial block
			{
				uint32_t d = (~cFileBlockMask) + 1 - (StartByte & cFileBlockMask);

				if (m_Cipher && (Type == m_Type) && (!m_Changing || (ID < m_ChangingProcess)))
				{
					m_Cipher->Decrypt(Data, d, ID, StartByte);
				} else if (m_OldCipher && m_Changing && (Type == m_OldType) && (ID >= m_ChangingProcess))
				{
					m_OldCipher->Decrypt(Data, d, ID, StartByte);
				}

				DataLength -= d;
				ID += (~cFileBlockMask) + 1;
				StartByte += d;
				Data = (uint8_t *)Data + d;
			}

			while (DataLength >= (~cFileBlockMask) + 1)
			{
				if (m_Cipher && (Type == m_Type) && (!m_Changing || (ID < m_ChangingProcess)))
				{
					m_Cipher->Decrypt(Data, (~cFileBlockMask) + 1, ID, StartByte);
				} else if (m_OldCipher && m_Changing && (Type == m_OldType) && (ID >= m_ChangingProcess))
				{
					m_OldCipher->Decrypt(Data, (~cFileBlockMask) + 1, ID, StartByte);
				}

				DataLength -= (~cFileBlockMask) + 1;
				ID += (~cFileBlockMask) + 1;
				StartByte += (~cFileBlockMask) + 1;
				Data = (uint8_t *)Data + (~cFileBlockMask) + 1;
			}
		}
	} 
	
	if (DataLength > 0) // last partial file block handled here, also all other 
	{
		if (m_Cipher && (Type == m_Type) && (!m_Changing || (ID < m_ChangingProcess)))
		{
			m_Cipher->Decrypt(Data, DataLength, ID, StartByte);
		} else if (m_OldCipher && m_Changing && (Type == m_OldType) && (ID >= m_ChangingProcess))
		{
			m_OldCipher->Decrypt(Data, DataLength, ID, StartByte);
		}
	}
}

bool CEncryptionManager::CanChangeCipher()
{
	return false;
}
bool CEncryptionManager::ChangeCipher(TEncryption & Encryption)
{
	return false;
}