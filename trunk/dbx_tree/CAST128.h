#pragma once

#include "Cipher.h"

class CCAST128 : public CCipher
{
private:
	static const uint32_t cBlockSize = 8;

	uint32_t Km[16], Kr[16];

	void EncryptBlock(uint8_t *Block, uint32_t BlockIndex);
	void DecryptBlock(uint8_t *Block, uint32_t BlockIndex);
	void CreateSubKeys(uint8_t* Key);

public:
	CCAST128();
	virtual ~CCAST128();
	static CCipher* Create();

	virtual uint32_t BlockSizeBytes();
	virtual void SetKey(void* Key, uint32_t KeyLength);
	virtual void Encrypt(void* Data, uint32_t DataLength, uint32_t StartBlockIndex);
	virtual void Decrypt(void* Data, uint32_t DataLength, uint32_t StartBlockIndex);	
};