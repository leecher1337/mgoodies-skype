#include "Cipher.h"

class CCAST128 : public CCipher
{
private:
	static const int cBlockSize = 8;

	unsigned int Km[16], Kr[16];

	void EncryptBlock(unsigned char *Block);
	void DecryptBlock(unsigned char *Block);
	void CreateSubKeys(unsigned char* Key);

public:
	CCAST128();
	virtual ~CCAST128();

	virtual int BlockSizeBytes();
	virtual void SetKey(unsigned char* Key, int KeyLength);
	virtual void Encrypt(unsigned char* Data, int DataLength);
	virtual void Decrypt(unsigned char* Data, int DataLength);	
};