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
