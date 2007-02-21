/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
Copyright ( c ) 2003-5 George Hazan.
Copyright ( c ) 2002-3 Richard Hughes ( original version ).

Miranda IM: the free icq client for MS Windows
Copyright ( C ) 2000-2002 Richard Hughes, Roland Rabien & Tristan Van de Vreede

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL$
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

*/

#include "jabber.h"
#ifdef __GNUC__
	#define __try
	#define __except(x) if (0) /* don't execute handler */
	#define __finally
	#define _try __try
	#define _except __except
	#define _finally __finally
#endif

extern UINT jabberCodePage;

static void __stdcall sttUtf8Decode( const BYTE* str, wchar_t* tempBuf )
{
	wchar_t* d = tempBuf;
	BYTE* s = ( BYTE* )str;

	while( *s )
	{
		if (( *s & 0x80 ) == 0 ) {
			*d++ = *s++;
			continue;
		}

		if (( s[0] & 0xE0 ) == 0xE0 && ( s[1] & 0xC0 ) == 0x80 && ( s[2] & 0xC0 ) == 0x80 ) {
			*d++ = (( WORD )( s[0] & 0x0F ) << 12 ) + ( WORD )(( s[1] & 0x3F ) << 6 ) + ( WORD )( s[2] & 0x3F );
			s += 3;
			continue;
		}

		if (( s[0] & 0xE0 ) == 0xC0 && ( s[1] & 0xC0 ) == 0x80 ) {
			*d++ = ( WORD )(( s[0] & 0x1F ) << 6 ) + ( WORD )( s[1] & 0x3F );
			s += 2;
			continue;
		}

		*d++ = *s++;
	}

	*d = 0;
}


char* deprecatedUtf8Decode( char* str, WCHAR** ucs2 )
{
	if ( str == NULL )
		return NULL;

	int len = strlen( str );
	if ( len < 2 ) {
		if ( ucs2 != NULL ) {
			*ucs2 = ( wchar_t* )mir_alloc(( len+1 )*sizeof( wchar_t ));
			MultiByteToWideChar( CP_ACP, 0, str, len, *ucs2, len );
			( *ucs2 )[ len ] = 0;
		}
		return str;
	}

	wchar_t* tempBuf = ( wchar_t* )alloca(( len+1 )*sizeof( wchar_t ));
	sttUtf8Decode(( BYTE* )str, tempBuf );

	if ( ucs2 != NULL ) {
		int fullLen = ( len+1 )*sizeof( wchar_t );
		*ucs2 = ( wchar_t* )mir_alloc( fullLen );
		memcpy( *ucs2, tempBuf, fullLen );
	}

   WideCharToMultiByte( CP_ACP, 0, tempBuf, -1, str, len, NULL, NULL );
	return str;
}

char* deprecatedUtf8EncodeW( const WCHAR* wstr )
{
	const WCHAR* w;

	// Convert unicode to utf8
	int len = 0;
	for ( w = wstr; *w; w++ ) {
		if ( *w < 0x0080 ) len++;
		else if ( *w < 0x0800 ) len += 2;
		else len += 3;
	}

	unsigned char* szOut = ( unsigned char* )mir_alloc( len+1 );
	if ( szOut == NULL )
		return NULL;

	int i = 0;
	for ( w = wstr; *w; w++ ) {
		if ( *w < 0x0080 )
			szOut[i++] = ( unsigned char ) *w;
		else if ( *w < 0x0800 ) {
			szOut[i++] = 0xc0 | (( *w ) >> 6 );
			szOut[i++] = 0x80 | (( *w ) & 0x3f );
		}
		else {
			szOut[i++] = 0xe0 | (( *w ) >> 12 );
			szOut[i++] = 0x80 | (( ( *w ) >> 6 ) & 0x3f );
			szOut[i++] = 0x80 | (( *w ) & 0x3f );
	}	}

	szOut[ i ] = '\0';
	return ( char* )szOut;
}

char* deprecatedUtf8Encode( const char* str )
{
	if ( str == NULL )
		return NULL;

	// Convert local codepage to unicode
	int len = strlen( str );
	WCHAR* wszTemp = ( WCHAR* )alloca( sizeof( WCHAR )*( len+1 ));
	MultiByteToWideChar( jabberCodePage, 0, str, -1, wszTemp, len+1 );

	return deprecatedUtf8EncodeW( wszTemp );
}

struct FORK_ARG {
	HANDLE hEvent;
	void ( __cdecl *threadcode )( void* );
	void *arg;
};

static void __cdecl forkthread_r( struct FORK_ARG *fa )
{
	void ( *callercode )( void* ) = fa->threadcode;
	void *arg = fa->arg;
	JabberLog( "Thread started: %08X %d", callercode, GetCurrentThreadId());
	JCallService( MS_SYSTEM_THREAD_PUSH, 0, 0 );
	SetEvent( fa->hEvent );
	__try {
		callercode( arg );
	} __finally {
		JCallService( MS_SYSTEM_THREAD_POP, 0, 0 );
	}
	return;
}

BOOL hasForkThreadService =0;
ULONG deprecatedForkThread( void ( __cdecl *threadcode )( void* ), unsigned long stacksize, void *arg )
{
	struct FORK_ARG fa;
	fa.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	fa.threadcode = threadcode;
	fa.arg = arg;

	ULONG rc = _beginthread(( pThreadFunc )forkthread_r, 0, &fa );
	if (( unsigned long ) -1L != rc )
		WaitForSingleObject( fa.hEvent, INFINITE );

	CloseHandle( fa.hEvent );
	return rc;
}


/*
MD5 implementation - to be used with older Miranda core 
The original and principal author of md5.c is L. Peter Deutsch
<ghost@aladdin.com>.
*/


#define T_MASK ((mir_md5_word_t)~0)
#define T1 /* 0xd76aa478 */ (T_MASK ^ 0x28955b87)
#define T2 /* 0xe8c7b756 */ (T_MASK ^ 0x173848a9)
#define T3    0x242070db
#define T4 /* 0xc1bdceee */ (T_MASK ^ 0x3e423111)
#define T5 /* 0xf57c0faf */ (T_MASK ^ 0x0a83f050)
#define T6    0x4787c62a
#define T7 /* 0xa8304613 */ (T_MASK ^ 0x57cfb9ec)
#define T8 /* 0xfd469501 */ (T_MASK ^ 0x02b96afe)
#define T9    0x698098d8
#define T10 /* 0x8b44f7af */ (T_MASK ^ 0x74bb0850)
#define T11 /* 0xffff5bb1 */ (T_MASK ^ 0x0000a44e)
#define T12 /* 0x895cd7be */ (T_MASK ^ 0x76a32841)
#define T13    0x6b901122
#define T14 /* 0xfd987193 */ (T_MASK ^ 0x02678e6c)
#define T15 /* 0xa679438e */ (T_MASK ^ 0x5986bc71)
#define T16    0x49b40821
#define T17 /* 0xf61e2562 */ (T_MASK ^ 0x09e1da9d)
#define T18 /* 0xc040b340 */ (T_MASK ^ 0x3fbf4cbf)
#define T19    0x265e5a51
#define T20 /* 0xe9b6c7aa */ (T_MASK ^ 0x16493855)
#define T21 /* 0xd62f105d */ (T_MASK ^ 0x29d0efa2)
#define T22    0x02441453
#define T23 /* 0xd8a1e681 */ (T_MASK ^ 0x275e197e)
#define T24 /* 0xe7d3fbc8 */ (T_MASK ^ 0x182c0437)
#define T25    0x21e1cde6
#define T26 /* 0xc33707d6 */ (T_MASK ^ 0x3cc8f829)
#define T27 /* 0xf4d50d87 */ (T_MASK ^ 0x0b2af278)
#define T28    0x455a14ed
#define T29 /* 0xa9e3e905 */ (T_MASK ^ 0x561c16fa)
#define T30 /* 0xfcefa3f8 */ (T_MASK ^ 0x03105c07)
#define T31    0x676f02d9
#define T32 /* 0x8d2a4c8a */ (T_MASK ^ 0x72d5b375)
#define T33 /* 0xfffa3942 */ (T_MASK ^ 0x0005c6bd)
#define T34 /* 0x8771f681 */ (T_MASK ^ 0x788e097e)
#define T35    0x6d9d6122
#define T36 /* 0xfde5380c */ (T_MASK ^ 0x021ac7f3)
#define T37 /* 0xa4beea44 */ (T_MASK ^ 0x5b4115bb)
#define T38    0x4bdecfa9
#define T39 /* 0xf6bb4b60 */ (T_MASK ^ 0x0944b49f)
#define T40 /* 0xbebfbc70 */ (T_MASK ^ 0x4140438f)
#define T41    0x289b7ec6
#define T42 /* 0xeaa127fa */ (T_MASK ^ 0x155ed805)
#define T43 /* 0xd4ef3085 */ (T_MASK ^ 0x2b10cf7a)
#define T44    0x04881d05
#define T45 /* 0xd9d4d039 */ (T_MASK ^ 0x262b2fc6)
#define T46 /* 0xe6db99e5 */ (T_MASK ^ 0x1924661a)
#define T47    0x1fa27cf8
#define T48 /* 0xc4ac5665 */ (T_MASK ^ 0x3b53a99a)
#define T49 /* 0xf4292244 */ (T_MASK ^ 0x0bd6ddbb)
#define T50    0x432aff97
#define T51 /* 0xab9423a7 */ (T_MASK ^ 0x546bdc58)
#define T52 /* 0xfc93a039 */ (T_MASK ^ 0x036c5fc6)
#define T53    0x655b59c3
#define T54 /* 0x8f0ccc92 */ (T_MASK ^ 0x70f3336d)
#define T55 /* 0xffeff47d */ (T_MASK ^ 0x00100b82)
#define T56 /* 0x85845dd1 */ (T_MASK ^ 0x7a7ba22e)
#define T57    0x6fa87e4f
#define T58 /* 0xfe2ce6e0 */ (T_MASK ^ 0x01d3191f)
#define T59 /* 0xa3014314 */ (T_MASK ^ 0x5cfebceb)
#define T60    0x4e0811a1
#define T61 /* 0xf7537e82 */ (T_MASK ^ 0x08ac817d)
#define T62 /* 0xbd3af235 */ (T_MASK ^ 0x42c50dca)
#define T63    0x2ad7d2bb
#define T64 /* 0xeb86d391 */ (T_MASK ^ 0x14792c6e)

//gfd*
static void deprecated_md5_process(mir_md5_state_t *pms, const mir_md5_byte_t *data /*[64]*/)
{
  mir_md5_word_t
    a = pms->abcd[0], b = pms->abcd[1],
    c = pms->abcd[2], d = pms->abcd[3];
  mir_md5_word_t t;
  /* Define storage for little-endian or both types of CPUs. */
  mir_md5_word_t xbuf[16];
  const mir_md5_word_t *X;

  {
    /*
     * Determine dynamically whether this is a big-endian or
     * little-endian machine, since we can use a more efficient
     * algorithm on the latter.
     */
    static const int w = 1;

  if (*((const mir_md5_byte_t *)&w)) /* dynamic little-endian */
  {
      /*
       * On little-endian machines, we can process properly aligned
       * data without copying it.
       */
      if (!((data - (const mir_md5_byte_t *)0) & 3)) {
    /* data are properly aligned */
    X = (const mir_md5_word_t *)data;
      } else {
    /* not aligned */
    memcpy(xbuf, data, 64);
    X = xbuf;
      }
  }
  else      /* dynamic big-endian */
  {
      /*
       * On big-endian machines, we must arrange the bytes in the
       * right order.
       */
      const mir_md5_byte_t *xp = data;
      int i;

      X = xbuf;    /* (dynamic only) */
      for (i = 0; i < 16; ++i, xp += 4)
    xbuf[i] = xp[0] + (xp[1] << 8) + (xp[2] << 16) + (xp[3] << 24);
  }
    }

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

    /* Round 1. */
    /* Let [abcd k s i] denote the operation
       a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define SET1(a, b, c, d, k, s, Ti)\
  t = a + F(b,c,d) + X[k] + Ti;\
  a = ROTATE_LEFT(t, s) + b
    /* Do the following 16 operations. */
    SET1(a, b, c, d,  0,  7,  T1);
    SET1(d, a, b, c,  1, 12,  T2);
    SET1(c, d, a, b,  2, 17,  T3);
    SET1(b, c, d, a,  3, 22,  T4);
    SET1(a, b, c, d,  4,  7,  T5);
    SET1(d, a, b, c,  5, 12,  T6);
    SET1(c, d, a, b,  6, 17,  T7);
    SET1(b, c, d, a,  7, 22,  T8);
    SET1(a, b, c, d,  8,  7,  T9);
    SET1(d, a, b, c,  9, 12, T10);
    SET1(c, d, a, b, 10, 17, T11);
    SET1(b, c, d, a, 11, 22, T12);
    SET1(a, b, c, d, 12,  7, T13);
    SET1(d, a, b, c, 13, 12, T14);
    SET1(c, d, a, b, 14, 17, T15);
    SET1(b, c, d, a, 15, 22, T16);

     /* Round 2. */
     /* Let [abcd k s i] denote the operation
          a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s). */
#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define SET2(a, b, c, d, k, s, Ti)\
  t = a + G(b,c,d) + X[k] + Ti;\
  a = ROTATE_LEFT(t, s) + b
     /* Do the following 16 operations. */
    SET2(a, b, c, d,  1,  5, T17);
    SET2(d, a, b, c,  6,  9, T18);
    SET2(c, d, a, b, 11, 14, T19);
    SET2(b, c, d, a,  0, 20, T20);
    SET2(a, b, c, d,  5,  5, T21);
    SET2(d, a, b, c, 10,  9, T22);
    SET2(c, d, a, b, 15, 14, T23);
    SET2(b, c, d, a,  4, 20, T24);
    SET2(a, b, c, d,  9,  5, T25);
    SET2(d, a, b, c, 14,  9, T26);
    SET2(c, d, a, b,  3, 14, T27);
    SET2(b, c, d, a,  8, 20, T28);
    SET2(a, b, c, d, 13,  5, T29);
    SET2(d, a, b, c,  2,  9, T30);
    SET2(c, d, a, b,  7, 14, T31);
    SET2(b, c, d, a, 12, 20, T32);

     /* Round 3. */
     /* Let [abcd k s t] denote the operation
          a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s). */
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define SET3(a, b, c, d, k, s, Ti)\
  t = a + H(b,c,d) + X[k] + Ti;\
  a = ROTATE_LEFT(t, s) + b
     /* Do the following 16 operations. */
    SET3(a, b, c, d,  5,  4, T33);
    SET3(d, a, b, c,  8, 11, T34);
    SET3(c, d, a, b, 11, 16, T35);
    SET3(b, c, d, a, 14, 23, T36);
    SET3(a, b, c, d,  1,  4, T37);
    SET3(d, a, b, c,  4, 11, T38);
    SET3(c, d, a, b,  7, 16, T39);
    SET3(b, c, d, a, 10, 23, T40);
    SET3(a, b, c, d, 13,  4, T41);
    SET3(d, a, b, c,  0, 11, T42);
    SET3(c, d, a, b,  3, 16, T43);
    SET3(b, c, d, a,  6, 23, T44);
    SET3(a, b, c, d,  9,  4, T45);
    SET3(d, a, b, c, 12, 11, T46);
    SET3(c, d, a, b, 15, 16, T47);
    SET3(b, c, d, a,  2, 23, T48);

     /* Round 4. */
     /* Let [abcd k s t] denote the operation
          a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s). */
#define I(x, y, z) ((y) ^ ((x) | ~(z)))
#define SET4(a, b, c, d, k, s, Ti)\
  t = a + I(b,c,d) + X[k] + Ti;\
  a = ROTATE_LEFT(t, s) + b
     /* Do the following 16 operations. */
    SET4(a, b, c, d,  0,  6, T49);
    SET4(d, a, b, c,  7, 10, T50);
    SET4(c, d, a, b, 14, 15, T51);
    SET4(b, c, d, a,  5, 21, T52);
    SET4(a, b, c, d, 12,  6, T53);
    SET4(d, a, b, c,  3, 10, T54);
    SET4(c, d, a, b, 10, 15, T55);
    SET4(b, c, d, a,  1, 21, T56);
    SET4(a, b, c, d,  8,  6, T57);
    SET4(d, a, b, c, 15, 10, T58);
    SET4(c, d, a, b,  6, 15, T59);
    SET4(b, c, d, a, 13, 21, T60);
    SET4(a, b, c, d,  4,  6, T61);
    SET4(d, a, b, c, 11, 10, T62);
    SET4(c, d, a, b,  2, 15, T63);
    SET4(b, c, d, a,  9, 21, T64);

     /* Then perform the following additions. (That is increment each
        of the four registers by the value it had before this block
        was started.) */
    pms->abcd[0] += a;
    pms->abcd[1] += b;
    pms->abcd[2] += c;
    pms->abcd[3] += d;
}

void deprecated_md5_init(mir_md5_state_t *pms)
{
  pms->count[0] = pms->count[1] = 0;
  pms->abcd[0] = 0x67452301;
  pms->abcd[1] = /*0xefcdab89*/ T_MASK ^ 0x10325476;
  pms->abcd[2] = /*0x98badcfe*/ T_MASK ^ 0x67452301;
  pms->abcd[3] = 0x10325476;
}

void deprecated_md5_append(mir_md5_state_t *pms, const mir_md5_byte_t *data, int nbytes)
{
  const mir_md5_byte_t *p = data;
  int left = nbytes;
  int offset = (pms->count[0] >> 3) & 63;
  mir_md5_word_t nbits = (mir_md5_word_t)(nbytes << 3);

  if (nbytes <= 0)
    return;

  /* Update the message length. */
  pms->count[1] += nbytes >> 29;
  pms->count[0] += nbits;
  if (pms->count[0] < nbits)
    pms->count[1]++;

  /* Process an initial partial block. */
  if (offset) 
  {
    int copy = (offset + nbytes > 64 ? 64 - offset : nbytes);

    memcpy(pms->buf + offset, p, copy);
    if (offset + copy < 64)
      return;
    p += copy;
    left -= copy;
    deprecated_md5_process(pms, pms->buf);
  }

  /* Process full blocks. */
  for (; left >= 64; p += 64, left -= 64)
    deprecated_md5_process(pms, p);

  /* Process a final partial block. */
  if (left)
    memcpy(pms->buf, p, left);
}

void deprecated_md5_finish(mir_md5_state_t *pms, mir_md5_byte_t digest[16])
{
  static const mir_md5_byte_t pad[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  mir_md5_byte_t data[8];
  int i;

  /* Save the length before padding. */
  for (i = 0; i < 8; ++i)
    data[i] = (mir_md5_byte_t)(pms->count[i >> 2] >> ((i & 3) << 3));
  /* Pad to 56 bytes mod 64. */
  deprecated_md5_append(pms, pad, ((55 - (pms->count[0] >> 3)) & 63) + 1);
  /* Append the length. */
  deprecated_md5_append(pms, data, 8);
  for (i = 0; i < 16; ++i)
    digest[i] = (mir_md5_byte_t)(pms->abcd[i >> 2] >> ((i & 3) << 3));
}

void deprecated_md5_hash_string(const mir_md5_byte_t *data, int len, mir_md5_byte_t digest[16])
{
    mir_md5_state_t state;
    deprecated_md5_init(&state);
	deprecated_md5_append(&state, data, len);
    deprecated_md5_finish(&state, digest);
}

/*
 *	End local MD5 implementaion
 */

/*
 *	Local SHA1 implementation
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * The Original Code is SHA 180-1 Reference Implementation (Compact version).
 *
 * The Initial Developer of the Original Code is
 * Paul Kocher of Cryptography Research.
 * Portions created by the Initial Developer are Copyright (C) 1995-9
 * the Initial Developer. All Rights Reserved.
 *
 * ***** END LICENSE BLOCK ***** */

static void deprecated_shaHashBlock(mir_sha1_ctx *ctx);

void deprecated_shaInit(mir_sha1_ctx *ctx) {
  int i;

  ctx->lenW = 0;
  ctx->sizeHi = ctx->sizeLo = 0;

  /* Initialize H with the magic constants (see FIPS180 for constants)
   */
  ctx->H[0] = 0x67452301L;
  ctx->H[1] = 0xefcdab89L;
  ctx->H[2] = 0x98badcfeL;
  ctx->H[3] = 0x10325476L;
  ctx->H[4] = 0xc3d2e1f0L;

  for (i = 0; i < 80; i++)
    ctx->W[i] = 0;
}


void deprecated_shaUpdate(mir_sha1_ctx *ctx, mir_sha1_byte_t *dataIn, int len) {
  int i;

  /* Read the data into W and process blocks as they get full
   */
  for (i = 0; i < len; i++) {
    ctx->W[ctx->lenW / 4] <<= 8;
    ctx->W[ctx->lenW / 4] |= (unsigned long)dataIn[i];
    if ((++ctx->lenW) % 64 == 0) {
      deprecated_shaHashBlock(ctx);
      ctx->lenW = 0;
    }
    ctx->sizeLo += 8;
    ctx->sizeHi += (ctx->sizeLo < 8);
  }
}


void deprecated_shaFinal(mir_sha1_ctx *ctx, mir_sha1_byte_t hashout[20]) {
  unsigned char pad0x80 = 0x80;
  unsigned char pad0x00 = 0x00;
  unsigned char padlen[8];
  int i;

  /* Pad with a binary 1 (e.g. 0x80), then zeroes, then length
   */
  padlen[0] = (unsigned char)((ctx->sizeHi >> 24) & 255);
  padlen[1] = (unsigned char)((ctx->sizeHi >> 16) & 255);
  padlen[2] = (unsigned char)((ctx->sizeHi >> 8) & 255);
  padlen[3] = (unsigned char)((ctx->sizeHi >> 0) & 255);
  padlen[4] = (unsigned char)((ctx->sizeLo >> 24) & 255);
  padlen[5] = (unsigned char)((ctx->sizeLo >> 16) & 255);
  padlen[6] = (unsigned char)((ctx->sizeLo >> 8) & 255);
  padlen[7] = (unsigned char)((ctx->sizeLo >> 0) & 255);
  deprecated_shaUpdate(ctx, &pad0x80, 1);
  while (ctx->lenW != 56)
    deprecated_shaUpdate(ctx, &pad0x00, 1);
  deprecated_shaUpdate(ctx, padlen, 8);

  /* Output hash
   */
  for (i = 0; i < 20; i++) {
    hashout[i] = (unsigned char)(ctx->H[i / 4] >> 24);
    ctx->H[i / 4] <<= 8;
  }

  /*
   *  Re-initialize the context (also zeroizes contents)
   */
  deprecated_shaInit(ctx); 
}


void deprecated_shaBlock(mir_sha1_byte_t *dataIn, int len, mir_sha1_byte_t hashout[20]) {
  mir_sha1_ctx ctx;

  deprecated_shaInit(&ctx);
  deprecated_shaUpdate(&ctx, dataIn, len);
  deprecated_shaFinal(&ctx, hashout);
}


#define SHA_ROTL(X,n) (((X) << (n)) | ((X) >> (32-(n))))

static void deprecated_shaHashBlock(mir_sha1_ctx *ctx) {
  int t;
  unsigned long A,B,C,D,E,TEMP;

  for (t = 16; t <= 79; t++)
    ctx->W[t] =
      SHA_ROTL(ctx->W[t-3] ^ ctx->W[t-8] ^ ctx->W[t-14] ^ ctx->W[t-16], 1);

  A = ctx->H[0];
  B = ctx->H[1];
  C = ctx->H[2];
  D = ctx->H[3];
  E = ctx->H[4];

  for (t = 0; t <= 19; t++) {
    TEMP = SHA_ROTL(A,5) + (((C^D)&B)^D)     + E + ctx->W[t] + 0x5a827999L;
    E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
  }
  for (t = 20; t <= 39; t++) {
    TEMP = SHA_ROTL(A,5) + (B^C^D)           + E + ctx->W[t] + 0x6ed9eba1L;
    E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
  }
  for (t = 40; t <= 59; t++) {
    TEMP = SHA_ROTL(A,5) + ((B&C)|(D&(B|C))) + E + ctx->W[t] + 0x8f1bbcdcL;
    E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
  }
  for (t = 60; t <= 79; t++) {
    TEMP = SHA_ROTL(A,5) + (B^C^D)           + E + ctx->W[t] + 0xca62c1d6L;
    E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
  }

  ctx->H[0] += A;
  ctx->H[1] += B;
  ctx->H[2] += C;
  ctx->H[3] += D;
  ctx->H[4] += E;
}

/*
 *	End local SHA1 implementation
 */