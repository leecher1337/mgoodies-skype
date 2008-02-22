#pragma once

#include "stdint.h"

class CCipher
{
public:
	CCipher();
	virtual ~CCipher();

	virtual uint32_t BlockSizeBytes() = 0;
	virtual void SetKey(void* Key, uint32_t KeyLength) = 0;
	virtual void Encrypt(void* Data, uint32_t DataLength, uint32_t StartBlock) = 0;  // DataLength % BlockSize === 0
	virtual void Decrypt(void* Data, uint32_t DataLength, uint32_t StartBlock) = 0;  // DataLength % BlockSize === 0
};
