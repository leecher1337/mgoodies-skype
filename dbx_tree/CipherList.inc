/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2008 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

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