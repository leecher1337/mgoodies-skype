#include "FileAccess.h"

CFileAccess::CFileAccess(const char* FileName)
{
	m_FileName = new char[strlen(FileName) + 1];
	strcpy_s(m_FileName, strlen(FileName) + 1, FileName);
	m_Cipher = NULL;
	m_ReadOnly = false;
}

CFileAccess::~CFileAccess()
{
	delete [] m_FileName;
}

void CFileAccess::SetCipher(CCipher * Cipher)
{
	m_Cipher = Cipher;
	if (Cipher)
	{
		int i = 1;
		while (m_AllocGranularity % Cipher->BlockSizeBytes())
		{
			m_AllocGranularity = m_AllocGranularity / i * (i+1);
			++i;
		}
	}
}


void CFileAccess::SetEncryptionStart(uint32_t EncryptionStart)
{
	m_EncryptionStart = EncryptionStart;
}


uint32_t CFileAccess::Read(void* Buf, uint32_t Source, uint32_t Size)
{
	if (m_Cipher)
	{
		if (Source < m_EncryptionStart)
		{
			uint32_t cryptstart;
			uint32_t cryptsize;
			void* cryptbuf;

			if (Source + Size <= m_EncryptionStart)
			{
				mRead(Buf, Source, Size);
				return Size;

			} else {
				mRead(Buf, Source, m_EncryptionStart - Source);
				Source = m_EncryptionStart;
				Size = Size - (m_EncryptionStart - Source);
				Buf = (uint8_t*)Buf + (m_EncryptionStart - Source);
			}

			cryptstart = Source - (Source % m_Cipher->BlockSizeBytes());
			cryptsize = Size + (Source % m_Cipher->BlockSizeBytes());
			if (cryptsize % m_Cipher->BlockSizeBytes() != 0)
			{
				cryptsize = cryptsize + m_Cipher->BlockSizeBytes() - (cryptsize % m_Cipher->BlockSizeBytes());
			}

			cryptbuf = malloc(cryptsize);
			mRead(cryptbuf, cryptstart, cryptsize);
			m_Cipher->Decrypt(cryptbuf, cryptsize, cryptstart - m_EncryptionStart);
			memcpy(Buf, (uint8_t*)cryptbuf + (Source - cryptstart), Size);
			free(cryptbuf);

		}
	} else {
		mRead(Buf, Source, Size);
	}

	return Size;
}
uint32_t CFileAccess::Write(void* Buf, uint32_t Dest, uint32_t Size)
{
	if (m_Cipher)
	{
		if (Dest < m_EncryptionStart)
		{
			uint32_t cryptstart;
			uint32_t cryptsize;
			void* cryptbuf;
			bool loadlast = false;

			if (Dest + Size <= m_EncryptionStart)
			{
				mWrite(Buf, Dest, Size);
				return Size;
			} else {
				mWrite(Buf, Dest, m_EncryptionStart - Dest);
				Dest = m_EncryptionStart;
				Size = Size - (m_EncryptionStart - Dest);
				Buf = (uint8_t*)Buf + (m_EncryptionStart - Dest);	
			}

			cryptstart = Dest - (Dest % m_Cipher->BlockSizeBytes());
			cryptsize = Size + (Dest % m_Cipher->BlockSizeBytes());
			if (cryptsize % m_Cipher->BlockSizeBytes() != 0)
			{
				cryptsize = cryptsize + m_Cipher->BlockSizeBytes() - (cryptsize % m_Cipher->BlockSizeBytes());
				loadlast = true;
			}

			cryptbuf = malloc(cryptsize);		
			
			if (loadlast) 
			{
				mRead((uint8_t*)cryptbuf + (cryptsize - m_Cipher->BlockSizeBytes()), cryptstart + (cryptsize - m_Cipher->BlockSizeBytes()), m_Cipher->BlockSizeBytes());
				m_Cipher->Decrypt((uint8_t*)cryptbuf + (cryptsize - m_Cipher->BlockSizeBytes()), m_Cipher->BlockSizeBytes(), cryptstart + (cryptsize - m_Cipher->BlockSizeBytes()));
			}
			if ((cryptstart != Dest) && ((!loadlast) || (cryptsize > m_Cipher->BlockSizeBytes()))) // check if only one block at all
			{
				mRead(cryptbuf, cryptstart, m_Cipher->BlockSizeBytes());
				m_Cipher->Decrypt(cryptbuf, m_Cipher->BlockSizeBytes(), cryptstart);
			}

			memcpy((uint8_t*)cryptbuf + (Dest - cryptstart), Buf, Size);
			m_Cipher->Encrypt(cryptbuf, cryptsize, cryptstart);
			mWrite(cryptbuf, cryptstart, cryptsize);
			free(cryptbuf);
		}

	} else {
		mWrite(Buf, Dest, Size);
	}

	return Size;
}


uint32_t CFileAccess::SetSize(uint32_t Size)
{
	m_Size = Size;
	
	if (Size % m_AllocGranularity > 0)
		Size = Size - Size % m_AllocGranularity + m_AllocGranularity;

	if (Size == 0)
		Size = m_AllocGranularity;

	if (Size != m_AllocSize)
	{
		m_AllocSize = mSetSize(Size);
	}

	return Size;	
}
uint32_t CFileAccess::GetSize()
{
	return m_Size;
}

void CFileAccess::SetReadOnly(bool ReadOnly)
{
	m_ReadOnly = ReadOnly;
}
bool CFileAccess::GetReadOnly()
{
	return m_ReadOnly;
}
