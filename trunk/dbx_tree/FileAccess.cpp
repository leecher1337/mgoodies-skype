#include "FileAccess.h"

CFileAccess::CFileAccess(const char* FileName, CEncryptionManager & EncryptionManager, uint32_t EncryptionStart)
: m_EncryptionManager(EncryptionManager)
{
	m_FileName = new char[strlen(FileName) + 1];
	strcpy_s(m_FileName, strlen(FileName) + 1, FileName);
	m_ReadOnly = false;
	m_EncryptionStart = EncryptionStart;

	m_LastAllocTime = _time32(NULL);
}

CFileAccess::~CFileAccess()
{
	delete [] m_FileName;
}

uint32_t CFileAccess::Read(void* Buf, uint32_t Source, uint32_t Size)
{
	uint32_t retsize = Size;

	if (((Source >= m_EncryptionStart) && m_EncryptionManager.IsEncrypted(Source - m_EncryptionStart, ET_FILE))
		|| ((Source + Size > m_EncryptionStart) && m_EncryptionManager.IsEncrypted(Source + Size - m_EncryptionStart, ET_FILE)))
	{
		if (Source < m_EncryptionStart)
		{
			mRead(Buf, Source, m_EncryptionStart - Source);
			Size -= m_EncryptionStart - Source;
			Buf = (uint8_t *)Buf + (m_EncryptionStart - Source);
			Source = m_EncryptionStart;			
		}

		uint32_t estart = Source - m_EncryptionStart;
		uint32_t eend = Source + Size - m_EncryptionStart;
		m_EncryptionManager.AlignData(Source, ET_FILE, estart, eend);
		
		if ((estart != Source - m_EncryptionStart) || (eend != Source + Size - m_EncryptionStart)) // we need an extra buffer
		{
			uint8_t * cryptbuf = (uint8_t*) malloc(eend - estart);
			__try {
				mRead(cryptbuf, estart + m_EncryptionStart, eend - estart);
				m_EncryptionManager.Decrypt(cryptbuf, eend - estart, ET_FILE, estart, estart);
				memcpy(Buf, cryptbuf + (Source - m_EncryptionStart - estart), Size);

			} __finally {
				free(cryptbuf);
			}

		} else {
			mRead(Buf, Source, Size);
			m_EncryptionManager.Decrypt(Buf, Size, ET_FILE, estart, estart);
		}

	} else { // whole unencrypted goes here
		mRead(Buf, Source, Size);
	}

	return retsize;
}
uint32_t CFileAccess::Write(void* Buf, uint32_t Dest, uint32_t Size)
{
	uint32_t retsize = Size;
	if (((Dest >= m_EncryptionStart) && m_EncryptionManager.IsEncrypted(Dest - m_EncryptionStart, ET_FILE))
		|| ((Dest + Size > m_EncryptionStart) && m_EncryptionManager.IsEncrypted(Dest + Size - m_EncryptionStart, ET_FILE)))
	{
		if (Dest < m_EncryptionStart)
		{
			if (Dest < m_EncryptionStart)
			{
				mRead(Buf, Dest, m_EncryptionStart - Dest);
				Size -= m_EncryptionStart - Dest;
				Buf = (uint8_t *)Buf + (m_EncryptionStart - Dest);
				Dest = m_EncryptionStart;			
			}

			uint32_t estart1 = Dest - m_EncryptionStart;
			uint32_t eend1 = estart1;
			uint32_t estart2 = Dest + Size - m_EncryptionStart;
			uint32_t eend2 = estart2;

			m_EncryptionManager.AlignData(estart1, ET_FILE, estart1, eend1);
			m_EncryptionManager.AlignData(estart2, ET_FILE, estart2, eend2);

			uint8_t * cryptbuf = (uint8_t *) malloc(eend2 - estart1);
			__try {
				if (estart1 != estart2) // two different blocks
				{
					if (estart1 < Dest - m_EncryptionStart) // load leading block
					{
						mRead(cryptbuf, estart1 + m_EncryptionStart, eend1 - estart1);
						m_EncryptionManager.Decrypt(cryptbuf, eend1 - estart1, ET_FILE, estart1, estart1);
					}
					if (eend2 > Dest - m_EncryptionStart + Size) // load trailing block
					{
						Read(cryptbuf + (estart2 - estart1), estart2 + m_EncryptionStart, eend2 - estart2);
						m_EncryptionManager.Decrypt(cryptbuf + (estart2 - estart1), eend2 - estart2, ET_FILE, estart2, estart2);
					}
				} else if ((estart2 != Dest - m_EncryptionStart) || (eend2 != Dest + Size - m_EncryptionStart)) 
				{ // only one block
					Read(cryptbuf, estart2 + m_EncryptionStart, eend2 - estart2);
					m_EncryptionManager.Decrypt(cryptbuf, eend2 - estart2, ET_FILE, estart2, estart2);
				}
				
				memcpy(cryptbuf + ((Dest - m_EncryptionStart) - estart1), Buf, Size);

				m_EncryptionManager.Encrypt(cryptbuf, eend2 - estart1, ET_FILE, estart1, estart1);
				mWrite(cryptbuf, estart1 + m_EncryptionStart, eend2 - estart1);
			} __finally {
				free(cryptbuf);
			}
		}

	} else {
		mWrite(Buf, Dest, Size);
	}

	return retsize;
}


uint32_t CFileAccess::SetSize(uint32_t Size)
{
	m_sigFileSizeChanged.emit(this, Size);

	m_Size = Size;

	Size = (Size + m_AllocGranularity - 1) & ~(m_AllocGranularity - 1);
	
	if (Size == 0)
		Size = m_AllocGranularity;

	if (Size >= m_EncryptionStart)
		Size = m_EncryptionManager.AlignSize(Size - m_EncryptionStart, ET_FILE, Size - m_EncryptionStart) + m_EncryptionStart;

	if (Size != m_AllocSize)
	{
		m_AllocSize = mSetSize(Size);

		// adapt Alloc Granularity
		uint32_t t = _time32(NULL);
		uint32_t d = t - m_LastAllocTime;
		m_LastAllocTime = t;
		
		if (d < 30) // increase alloc stepping
		{
			if (m_AllocGranularity < m_MaxAllocGranularity)
				m_AllocGranularity = m_AllocGranularity << 1;
		} else if (d > 120) // decrease alloc stepping
		{
			if (m_AllocGranularity > m_MinAllocGranularity)
				m_AllocGranularity = m_AllocGranularity >> 1;
		}
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

CFileAccess::TOnFileSizeChanged & CFileAccess::sigFileSizeChanged()
{
	return m_sigFileSizeChanged;
}