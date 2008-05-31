#pragma once

#include "stdint.h"

class CCipher
{
public:
	CCipher();
	virtual ~CCipher();

	virtual uint32_t     BlockSizeBytes() = 0;
	virtual bool         IsStreamCipher() = 0;

	virtual void SetKey(void* Key, uint32_t KeyLength) = 0;
	virtual void Encrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte) = 0;
	virtual void Decrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte) = 0;
};
