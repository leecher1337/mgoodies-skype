#pragma once

#include "Cipher.h"

class CAST128 : public CCipher
{
private:
	uint32_t Km[16], Kr[16];

	void EncryptBlock(uint8_t *Block);
	void DecryptBlock(uint8_t *Block);
	void CreateSubKeys(uint8_t* Key);

public:
	static  inline const char * Name()           {return "Cast128";};
	static  inline const char * Description()    {return "Blockcipher - 64bit block, fast and secure, Carlisle Adams and Stafford Tavares 1996";};
	virtual inline uint32_t     BlockSizeBytes() {return 8;};
	virtual inline bool         IsStreamCipher() {return false;};

	CAST128();
	virtual ~CAST128();
	static CCipher* Create();

	virtual void SetKey(void* Key, uint32_t KeyLength);
	virtual void Encrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte);
	virtual void Decrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte);
};

