#pragma once

#include "Cipher.h"

class ARC4 : public CCipher
{
private:	
	uint8_t x;
	uint8_t y;
	uint8_t State[256];

	uint8_t Backx;
	uint8_t Backy;
	uint8_t BackState[256];

	uint8_t Stream();
public:
	static  inline const char * Name()           {return "ARC4";};
	static  inline const char * Description()    {return "Streamcipher - 8bit step, fast, Ron Rivest 1987";};
	virtual inline uint32_t     BlockSizeBytes() {return 1;};
	virtual inline bool         IsStreamCipher() {return true;};

	ARC4();
	virtual ~ARC4();
	static CCipher* Create();

	virtual void SetKey(void* Key, uint32_t KeyLength);
	virtual void Encrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte);
	virtual void Decrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte);
};
