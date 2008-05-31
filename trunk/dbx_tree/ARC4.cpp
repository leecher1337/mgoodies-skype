#include "ARC4.h"
#include <string.h>

ARC4::ARC4()
{

}
ARC4::~ARC4()
{

}

CCipher* ARC4::Create() 
{
	return new ARC4();
}

void ARC4::SetKey(void* Key, uint32_t KeyLength)
{	
	uint8_t swapbyte;
	uint8_t index1;
	uint8_t index2;
	unsigned int i;

	uint8_t * k = (uint8_t *) Key;

	for (i = 0; i < 256; ++i)
		State[i] = i;
	
	x = 0;
	y = 0;
	index1 = 0;
	index2 = 0;
	for (i = 0; i < 256; ++i)
	{
		index2 = (k[index1] + State[i] + index2) & 0xff;
		swapbyte = State[i];
		State[i] = State[index2];
		State[index2] = swapbyte;
		index1 = (index1 + 1) % KeyLength;
	}

	for (i = 0; i < 3742; ++i)
		Stream();

	Backx = x;
	Backy = y;
	memcpy(BackState, State, sizeof(State));
}

inline uint8_t ARC4::Stream()
{
	uint8_t swapbyte;

	x = (x + 1) & 0xff;
	y = (State[x] + y) & 0xff;

  swapbyte = State[x];
	State[x] = State[y];
	State[y] = swapbyte;

	return State[ (State[x] + State[y]) & 0xff ];
}

void ARC4::Encrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte)
{	
	uint8_t * dat = (uint8_t *) Data;
	x = Backx;
	y = Backy;
	memcpy(State, BackState, sizeof(State));

	for (unsigned int i = (Nonce + (Nonce >> 8) + (Nonce >> 16) + (Nonce >> 24)) & 0x1ff; i > 0; --i)
		Stream();

	while (Size > 0)
	{
		(*dat) = (*dat) ^ Stream();
		--Size;
		++dat;
	}
}
void ARC4::Decrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte)
{
	Encrypt(Data, Size, Nonce, StartByte);
}
