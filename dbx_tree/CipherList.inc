#ifndef __CIPHERLIST_INC
#define __CIPHERLIST_INC

#include "Cipher.h"
#include "Cast128.h"
#include "ARC4.h"
#include "HC256.h"
#include "stdint.h"

typedef CCipher* (*TCipherCreate)();
typedef const char* (*TCipherName)();
typedef const char* (*TCipherDescription)();

typedef struct TCipherListItem {
	const uint32_t ID;
	TCipherCreate Create;
	TCipherName Name;
	TCipherDescription Description;
} TCipherListItem, *PCipherListItem;


static const TCipherListItem cCipherList[3] = {
		{1, ARC4::Create, ARC4::Name, ARC4::Description},
		{2, CAST128::Create, CAST128::Name, CAST128::Description},
		{3, HC256::Create, HC256::Name, HC256::Description},
	};
	

#endif