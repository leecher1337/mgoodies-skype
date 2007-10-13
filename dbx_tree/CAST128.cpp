#include "CAST128.h"
#include "CAST128.inc"


CCAST128::CCAST128()
{

}
CCAST128::~CCAST128()
{

}

int CCAST128::BlockSizeBytes()
{
	return cBlockSize;
}
void CCAST128::SetKey(unsigned char* Key, int KeyLength)
{
	unsigned char k128[16] = {0};
	int i = 0;
	while (KeyLength > 0)
	{
		k128[i] = k128[i] ^ (*Key);
		i = (i + 1) % 16;
		Key++;
		KeyLength--;
	}
	CreateSubKeys(k128);
}
void CCAST128::Encrypt(unsigned char* Data, int DataLength)
{
	for (int i = 0; i <= DataLength - cBlockSize; i += cBlockSize)
	{
		EncryptBlock(Data + i);
	}
}
void CCAST128::Decrypt(unsigned char* Data, int DataLength)
{
	for (int i = 0; i <= DataLength - cBlockSize; i += cBlockSize)
	{
		DecryptBlock(Data + i);
	}
}

void CCAST128::CreateSubKeys(unsigned char* Key)
{
	union {
		unsigned char z[16];
		unsigned int i[4];
	} t;

	unsigned int* k;
	k = (unsigned int*) Key;
	
	t.i[0] = k[0] ^ S5[Key[0xD]] ^ S6[Key[0xF]] ^ S7[Key[0xC]] ^ S8[Key[0xE]] ^ S7[Key[0x8]];
	t.i[1] = k[2] ^ S5[t.z[0x0]] ^ S6[t.z[0x2]] ^ S7[t.z[0x1]] ^ S8[t.z[0x3]] ^ S8[Key[0xA]];
	t.i[2] = k[3] ^ S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S5[Key[0x9]];
	t.i[3] = k[1] ^ S5[t.z[0xA]] ^ S6[t.z[0x9]] ^ S7[t.z[0xB]] ^ S8[t.z[0x8]] ^ S6[Key[0xB]];

	Km[0x0] = S5[t.z[0x8]] ^ S6[t.z[0x9]] ^ S7[t.z[0x7]] ^ S8[t.z[0x6]] ^ S5[t.z[0x2]];
	Km[0x1] = S5[t.z[0xA]] ^ S6[t.z[0xB]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S6[t.z[0x6]];
	Km[0x2] = S5[t.z[0xC]] ^ S6[t.z[0xD]] ^ S7[t.z[0x3]] ^ S8[t.z[0x2]] ^ S7[t.z[0x9]];
	Km[0x3] = S5[t.z[0xE]] ^ S6[t.z[0xF]] ^ S7[t.z[0x1]] ^ S8[t.z[0x0]] ^ S8[t.z[0xC]];

	k[0] = t.i[2] ^ S5[t.z[0x5]] ^ S6[t.z[0x7]] ^ S7[t.z[0x4]] ^ S8[t.z[0x6]] ^ S7[t.z[0x0]];
	k[1] = t.i[0] ^ S5[Key[0x0]] ^ S6[Key[0x2]] ^ S7[Key[0x1]] ^ S8[Key[0x3]] ^ S8[t.z[0x2]];
	k[2] = t.i[1] ^ S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S5[t.z[0x1]];
	k[3] = t.i[3] ^ S5[Key[0xA]] ^ S6[Key[0x9]] ^ S7[Key[0xB]] ^ S8[Key[0x8]] ^ S6[t.z[0x3]];

	Km[0x4] = S5[Key[0x3]] ^ S6[Key[0x2]] ^ S7[Key[0xC]] ^ S8[Key[0xD]] ^ S5[Key[0x8]];
	Km[0x5] = S5[Key[0x1]] ^ S6[Key[0x0]] ^ S7[Key[0xE]] ^ S8[Key[0xF]] ^ S6[Key[0xD]];
	Km[0x6] = S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x8]] ^ S8[Key[0x9]] ^ S7[Key[0x3]];
	Km[0x7] = S5[Key[0x5]] ^ S6[Key[0x4]] ^ S7[Key[0xA]] ^ S8[Key[0xB]] ^ S8[Key[0x7]];

	t.i[0] = k[0] ^ S5[Key[0xD]] ^ S6[Key[0xF]] ^ S7[Key[0xC]] ^ S8[Key[0xE]] ^ S7[Key[0x8]];
	t.i[1] = k[2] ^ S5[t.z[0x0]] ^ S6[t.z[0x2]] ^ S7[t.z[0x1]] ^ S8[t.z[0x3]] ^ S8[Key[0xA]];
	t.i[2] = k[3] ^ S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S5[Key[0x9]];
	t.i[3] = k[1] ^ S5[t.z[0xA]] ^ S6[t.z[0x9]] ^ S7[t.z[0xB]] ^ S8[t.z[0x8]] ^ S6[Key[0xB]];

	Km[0x8] = S5[t.z[0x3]] ^ S6[t.z[0x2]] ^ S7[t.z[0xC]] ^ S8[t.z[0xD]] ^ S5[t.z[0x9]];
	Km[0x9] = S5[t.z[0x1]] ^ S6[t.z[0x0]] ^ S7[t.z[0xE]] ^ S8[t.z[0xF]] ^ S6[t.z[0xC]];
	Km[0xa] = S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x8]] ^ S8[t.z[0x9]] ^ S7[t.z[0x2]];
	Km[0xb] = S5[t.z[0x5]] ^ S6[t.z[0x4]] ^ S7[t.z[0xA]] ^ S8[t.z[0xB]] ^ S8[t.z[0x6]];

	k[0] = t.i[2] ^ S5[t.z[0x5]] ^ S6[t.z[0x7]] ^ S7[t.z[0x4]] ^ S8[t.z[0x6]] ^ S7[t.z[0x0]];
	k[1] = t.i[0] ^ S5[Key[0x0]] ^ S6[Key[0x2]] ^ S7[Key[0x1]] ^ S8[Key[0x3]] ^ S8[t.z[0x2]];
	k[2] = t.i[1] ^ S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S5[t.z[0x1]];
	k[3] = t.i[3] ^ S5[Key[0xA]] ^ S6[Key[0x9]] ^ S7[Key[0xB]] ^ S8[Key[0x8]] ^ S6[t.z[0x3]];

	Km[0xc] = S5[Key[0x8]] ^ S6[Key[0x9]] ^ S7[Key[0x7]] ^ S8[Key[0x6]] ^ S5[Key[0x3]];
	Km[0xd] = S5[Key[0xA]] ^ S6[Key[0xB]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S6[Key[0x7]];
	Km[0xe] = S5[Key[0xC]] ^ S6[Key[0xD]] ^ S7[Key[0x3]] ^ S8[Key[0x2]] ^ S7[Key[0x8]];
	Km[0xf] = S5[Key[0xE]] ^ S6[Key[0xF]] ^ S7[Key[0x1]] ^ S8[Key[0x0]] ^ S8[Key[0xD]];



	t.i[0] = k[0] ^ S5[Key[0xD]] ^ S6[Key[0xF]] ^ S7[Key[0xC]] ^ S8[Key[0xE]] ^ S7[Key[0x8]];
	t.i[1] = k[2] ^ S5[t.z[0x0]] ^ S6[t.z[0x2]] ^ S7[t.z[0x1]] ^ S8[t.z[0x3]] ^ S8[Key[0xA]];
	t.i[2] = k[3] ^ S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S5[Key[0x9]];
	t.i[3] = k[1] ^ S5[t.z[0xA]] ^ S6[t.z[0x9]] ^ S7[t.z[0xB]] ^ S8[t.z[0x8]] ^ S6[Key[0xB]];

	Kr[0x0] = S5[t.z[0x8]] ^ S6[t.z[0x9]] ^ S7[t.z[0x7]] ^ S8[t.z[0x6]] ^ S5[t.z[0x2]];
	Kr[0x1] = S5[t.z[0xA]] ^ S6[t.z[0xB]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S6[t.z[0x6]];
	Kr[0x2] = S5[t.z[0xC]] ^ S6[t.z[0xD]] ^ S7[t.z[0x3]] ^ S8[t.z[0x2]] ^ S7[t.z[0x9]];
	Kr[0x3] = S5[t.z[0xE]] ^ S6[t.z[0xF]] ^ S7[t.z[0x1]] ^ S8[t.z[0x0]] ^ S8[t.z[0xC]];

	k[0] = t.i[2] ^ S5[t.z[0x5]] ^ S6[t.z[0x7]] ^ S7[t.z[0x4]] ^ S8[t.z[0x6]] ^ S7[t.z[0x0]];
	k[1] = t.i[0] ^ S5[Key[0x0]] ^ S6[Key[0x2]] ^ S7[Key[0x1]] ^ S8[Key[0x3]] ^ S8[t.z[0x2]];
	k[2] = t.i[1] ^ S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S5[t.z[0x1]];
	k[3] = t.i[3] ^ S5[Key[0xA]] ^ S6[Key[0x9]] ^ S7[Key[0xB]] ^ S8[Key[0x8]] ^ S6[t.z[0x3]];

	Kr[0x4] = S5[Key[0x3]] ^ S6[Key[0x2]] ^ S7[Key[0xC]] ^ S8[Key[0xD]] ^ S5[Key[0x8]];
	Kr[0x5] = S5[Key[0x1]] ^ S6[Key[0x0]] ^ S7[Key[0xE]] ^ S8[Key[0xF]] ^ S6[Key[0xD]];
	Kr[0x6] = S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x8]] ^ S8[Key[0x9]] ^ S7[Key[0x3]];
	Kr[0x7] = S5[Key[0x5]] ^ S6[Key[0x4]] ^ S7[Key[0xA]] ^ S8[Key[0xB]] ^ S8[Key[0x7]];

	t.i[0] = k[0] ^ S5[Key[0xD]] ^ S6[Key[0xF]] ^ S7[Key[0xC]] ^ S8[Key[0xE]] ^ S7[Key[0x8]];
	t.i[1] = k[2] ^ S5[t.z[0x0]] ^ S6[t.z[0x2]] ^ S7[t.z[0x1]] ^ S8[t.z[0x3]] ^ S8[Key[0xA]];
	t.i[2] = k[3] ^ S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S5[Key[0x9]];
	t.i[3] = k[1] ^ S5[t.z[0xA]] ^ S6[t.z[0x9]] ^ S7[t.z[0xB]] ^ S8[t.z[0x8]] ^ S6[Key[0xB]];

	Kr[0x8] = S5[t.z[0x3]] ^ S6[t.z[0x2]] ^ S7[t.z[0xC]] ^ S8[t.z[0xD]] ^ S5[t.z[0x9]];
	Kr[0x9] = S5[t.z[0x1]] ^ S6[t.z[0x0]] ^ S7[t.z[0xE]] ^ S8[t.z[0xF]] ^ S6[t.z[0xC]];
	Kr[0xa] = S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x8]] ^ S8[t.z[0x9]] ^ S7[t.z[0x2]];
	Kr[0xb] = S5[t.z[0x5]] ^ S6[t.z[0x4]] ^ S7[t.z[0xA]] ^ S8[t.z[0xB]] ^ S8[t.z[0x6]];

	k[0] = t.i[2] ^ S5[t.z[0x5]] ^ S6[t.z[0x7]] ^ S7[t.z[0x4]] ^ S8[t.z[0x6]] ^ S7[t.z[0x0]];
	k[1] = t.i[0] ^ S5[Key[0x0]] ^ S6[Key[0x2]] ^ S7[Key[0x1]] ^ S8[Key[0x3]] ^ S8[t.z[0x2]];
	k[2] = t.i[1] ^ S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S5[t.z[0x1]];
	k[3] = t.i[3] ^ S5[Key[0xA]] ^ S6[Key[0x9]] ^ S7[Key[0xB]] ^ S8[Key[0x8]] ^ S6[t.z[0x3]];

	Kr[0xc] = S5[Key[0x8]] ^ S6[Key[0x9]] ^ S7[Key[0x7]] ^ S8[Key[0x6]] ^ S5[Key[0x3]];
	Kr[0xd] = S5[Key[0xA]] ^ S6[Key[0xB]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S6[Key[0x7]];
	Kr[0xe] = S5[Key[0xC]] ^ S6[Key[0xD]] ^ S7[Key[0x3]] ^ S8[Key[0x2]] ^ S7[Key[0x8]];
	Kr[0xf] = S5[Key[0xE]] ^ S6[Key[0xF]] ^ S7[Key[0x1]] ^ S8[Key[0x0]] ^ S8[Key[0xD]];


	for (int i = 0; i < 16; i++)
		Kr[i] = Kr[i] & 0x0000001F;
}


__forceinline void CCAST128::EncryptBlock(unsigned char *Block)
{
	unsigned int l, r, tmp;
	union {
		unsigned char byte[4];
		unsigned int block;
	} t;

	l = ((unsigned int*)Block)[0];
	r = ((unsigned int*)Block)[1];

	for (int i = 0; i < 16; i++)
	{
		if ((i % 3) == 0)
		{
			t.block = r + Km[i];			
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] ^ S2[t.byte[1]]) - S3[t.byte[2]]) + S4[t.byte[3]];
		} else if ((i % 3) == 1)
		{			
			t.block = r ^ Km[i];			
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] - S2[t.byte[1]]) + S3[t.byte[2]]) ^ S4[t.byte[3]];
		} else {
			t.block = r - Km[i];			
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] + S2[t.byte[1]]) ^ S3[t.byte[2]]) - S4[t.byte[3]];
		}

		tmp = r;
		r = l ^ t.block;
		l = tmp;
	}

	((unsigned int*)Block)[0] = l;
	((unsigned int*)Block)[1] = r;
}



__forceinline void CCAST128::DecryptBlock(unsigned char *Block)
{
	unsigned int l, r, tmp;
	union {
		unsigned char byte[4];
		unsigned int block;
	} t;

	r = ((unsigned int*)Block)[0];
	l = ((unsigned int*)Block)[1];

	for (int i = 15; i >= 0; i--)
	{
		if ((i % 3) == 0)
		{
			t.block = r + Km[i];			
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] ^ S2[t.byte[1]]) - S3[t.byte[2]]) + S4[t.byte[3]];
		} else if ((i % 3) == 1)
		{			
			t.block = r ^ Km[i];			
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] - S2[t.byte[1]]) + S3[t.byte[2]]) ^ S4[t.byte[3]];
		} else {
			t.block = r - Km[i];			
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] + S2[t.byte[1]]) ^ S3[t.byte[2]]) - S4[t.byte[3]];
		}

		tmp = r;
		r = l ^ t.block;
		l = tmp;
	}

	((unsigned int*)Block)[0] = r;
	((unsigned int*)Block)[1] = l;
}
