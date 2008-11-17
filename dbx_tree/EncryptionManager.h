#pragma once

#include "stdint.h"
#include "sigslot.h"
#include "Cipher.h"
#include "SHA256.h"


typedef enum TEncryptionType {
	ET_NONE = 0,
	ET_FILE = 1,
	ET_BLOCK = 2,
	ET_DATA = 3,
	ET_MASK = 3
} TEncryptionType;

static const uint32_t cEncryptionChangingFlag = 0x80000000;

#pragma pack(push, 1)

typedef struct TFileEncryption {
	uint32_t AccessType;
	uint32_t CipherID;
	uint32_t CipherOldID;
	uint32_t ConversionProcess;
	uint8_t  SHA[32];
	uint8_t  SHAOld[32];
	uint32_t Reserved[2];
} TFileEncryption, *PFileEncryption;

#pragma pack(pop)

typedef struct TEncryption {
	uint32_t CipherID;
	TEncryptionType Type;
	char * Password;
} TEncryption, *PEncryption;

class CEncryptionManager
{
public:
	CEncryptionManager();
	~CEncryptionManager();

	bool InitEncryption(TFileEncryption & Enc);

	bool AlignData(uint32_t ID, TEncryptionType Type, uint32_t & Start, uint32_t & End);
	uint32_t AlignSize(uint32_t ID, TEncryptionType Type, uint32_t Size);
	bool IsEncrypted(uint32_t ID, TEncryptionType Type);
	void Encrypt(void* Data, uint32_t DataLength, TEncryptionType Type, uint32_t ID, uint32_t StartByte);
	void Decrypt(void* Data, uint32_t DataLength, TEncryptionType Type, uint32_t ID, uint32_t StartByte);

	bool CanChangeCipher();
	bool ChangeCipher(TEncryption & Encryption);
private:
	bool m_Changing;
	uint32_t m_ChangingProcess;

	TEncryptionType m_OldType;
	CCipher * m_OldCipher;
	
	TEncryptionType m_Type;
	CCipher * m_Cipher;

};
