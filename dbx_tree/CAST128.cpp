#include "CAST128.h"
#include "CAST128.inc"


CCAST128::CCAST128()
{

}
CCAST128::~CCAST128()
{

}
CCipher* CCAST128::Create()
{
	return new CCAST128();
}

uint32_t CCAST128::BlockSizeBytes()
{
	return cBlockSize;
}
void CCAST128::SetKey(void* Key, uint32_t KeyLength)
{
	uint8_t k128[16] = "Hello MirandaIM";
	int i = 0;
	uint8_t* k = (uint8_t*) Key;
	while (KeyLength > 0)
	{
		k128[i] = k128[i] ^ (*k);
		i = (i + 1) % 16;
		k++;
		KeyLength--;
	}
	CreateSubKeys(k128);
}
void CCAST128::Encrypt(void* Data, uint32_t DataLength, uint32_t StartBlock)
{
	for (uint32_t i = 0; i <= DataLength - cBlockSize; i += cBlockSize)
	{
		EncryptBlock((uint8_t*)Data + i, StartBlock);
		StartBlock += cBlockSize;
	}
}
void CCAST128::Decrypt(void* Data, uint32_t DataLength, uint32_t StartBlock)
{
	for (uint32_t i = 0; i <= DataLength - cBlockSize; i += cBlockSize)
	{
		DecryptBlock((uint8_t*)Data + i, StartBlock);
		StartBlock += cBlockSize;
	}
}

void CCAST128::CreateSubKeys(uint8_t* Key)
{
	union {
		uint8_t z[16];
		uint32_t i[4];
	} t;

	uint32_t* k;
	k = (uint32_t*) Key;

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


inline void CCAST128::EncryptBlock(uint8_t *Block, uint32_t BlockIndex)
{
	uint32_t l, r, tmp;
	union {
		uint8_t byte[4];
		uint32_t block;
	} t;

	l = ((uint32_t*)Block)[0];
	r = ((uint32_t*)Block)[1];

	for (int i = 0; i < 16; i++)
	{
		if ((i % 3) == 0)
		{
			t.block = (r + Km[i]) ^ BlockIndex;
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] ^ S2[t.byte[1]]) - S3[t.byte[2]]) + S4[t.byte[3]];
		} else if ((i % 3) == 1)
		{
			t.block = (r ^ Km[i]) - (BlockIndex << 1);
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] - S2[t.byte[1]]) + S3[t.byte[2]]) ^ S4[t.byte[3]];
		} else {
			t.block = (r - Km[i]) + (BlockIndex << 3);
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] + S2[t.byte[1]]) ^ S3[t.byte[2]]) - S4[t.byte[3]];
		}

		tmp = r;
		r = l ^ t.block;
		l = tmp;
	}

	((uint32_t*)Block)[0] = l;
	((uint32_t*)Block)[1] = r;
}



inline void CCAST128::DecryptBlock(uint8_t *Block, uint32_t BlockIndex)
{
	uint32_t l, r, tmp;
	union {
		uint8_t byte[4];
		uint32_t block;
	} t;

	r = ((uint32_t*)Block)[0];
	l = ((uint32_t*)Block)[1];

	for (int i = 15; i >= 0; i--)
	{
		if ((i % 3) == 0)
		{
			t.block = (r + Km[i]) ^ BlockIndex;
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] ^ S2[t.byte[1]]) - S3[t.byte[2]]) + S4[t.byte[3]];
		} else if ((i % 3) == 1)
		{
			t.block = (r ^ Km[i]) - (BlockIndex << 1);
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] - S2[t.byte[1]]) + S3[t.byte[2]]) ^ S4[t.byte[3]];
		} else {
			t.block = (r - Km[i]) + (BlockIndex << 3);
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] + S2[t.byte[1]]) ^ S3[t.byte[2]]) - S4[t.byte[3]];
		}

		tmp = r;
		r = l ^ t.block;
		l = tmp;
	}

	((uint32_t*)Block)[0] = r;
	((uint32_t*)Block)[1] = l;
}
