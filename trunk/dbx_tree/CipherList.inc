#ifndef __CIPHERLIST_INC
#define __CIPHERLIST_INC

#include "Cipher.h"
#include "Cast128.h"
#include "stdint.h"

typedef CCipher* (*TCipherCreate)();

typedef struct TCipherListContact {
	uint32_t ID;
	TCipherCreate Create;
	char * Name;
} TCipherListContact, *PCipherListContact;


static const TCipherListContact cCipherList[1] = {
		{0x01000000, CCAST128::Create, "CAST128"}
	};
	

#endif