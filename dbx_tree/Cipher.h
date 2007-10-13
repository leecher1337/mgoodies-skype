#pragma once

class CCipher
{
public:
	CCipher();
	virtual ~CCipher();

	virtual int BlockSizeBytes() = 0;
	virtual void SetKey(unsigned char* Key, int KeyLength) = 0;
	virtual void Encrypt(unsigned char* Data, int DataLength) = 0;  // DataLength % BlockSize === 0
	virtual void Decrypt(unsigned char* Data, int DataLength) = 0;  // DataLength % BlockSize === 0
};
