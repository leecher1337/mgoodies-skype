/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-06  George Hazan

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

#include "sha1.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Jabber_StretchBitmap - rescales a bitmap to 64x64 pixels and creates a DIB from it

HBITMAP __stdcall JabberStretchBitmap( HBITMAP hBitmap )
{
	BITMAPINFO bmStretch = { 0 };
	bmStretch.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmStretch.bmiHeader.biWidth = 64;
	bmStretch.bmiHeader.biHeight = 64;
	bmStretch.bmiHeader.biPlanes = 1;
	bmStretch.bmiHeader.biBitCount = 32;

	UINT* ptPixels;
	HBITMAP hStretchedBitmap = CreateDIBSection( NULL, &bmStretch, DIB_RGB_COLORS, ( void** )&ptPixels, NULL, 0);
	if ( hStretchedBitmap == NULL ) {
		JabberLog( "Bitmap creation failed with error %d", GetLastError() );
		return NULL;
	}

	BITMAP bmp;
	HDC hDC = CreateCompatibleDC( NULL );
	HBITMAP hOldBitmap1 = ( HBITMAP )SelectObject( hDC, hBitmap );
	GetObject( hBitmap, sizeof( BITMAP ), &bmp );
	if (max(bmp.bmWidth,bmp.bmHeight)!=64){// bitmap is not standard size
		TCHAR temp[25];
		mir_sntprintf(temp,24,_T(TCHAR_STR_PARAM)_T(" %s"),jabberProtoName,TranslateT( "avatar" ));
		if (IDYES == MessageBox( NULL,TranslateT( "Bitmap is not suggested size\nResize?" ),temp, MB_YESNO|MB_ICONEXCLAMATION|MB_SETFOREGROUND) ){
			HDC hBmpDC = CreateCompatibleDC( hDC );
			HBITMAP hOldBitmap2 = ( HBITMAP )SelectObject( hBmpDC, hStretchedBitmap );
			int side, dx, dy;
			if ( bmp.bmWidth > bmp.bmHeight ) {
				side = bmp.bmHeight;
				dx = ( bmp.bmWidth - bmp.bmHeight )/2;
				dy = 0;
			}
			else {
				side = bmp.bmWidth;
				dx = 0;
				dy = ( bmp.bmHeight - bmp.bmWidth )/2;
			}

			SetStretchBltMode( hBmpDC, HALFTONE );
			StretchBlt( hBmpDC, 0, 0, 64, 64, hDC, dx, dy, side, side, SRCCOPY );

			SelectObject( hBmpDC, hOldBitmap2 );
			DeleteDC( hBmpDC );
		} else hStretchedBitmap = hBitmap;
	} else hStretchedBitmap = hBitmap;
	SelectObject( hDC, hOldBitmap1 );
	if (hStretchedBitmap != hBitmap) DeleteObject( hBitmap );
	DeleteDC( hDC );

	return hStretchedBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberBitmapToAvatar - updates the avatar database settins and file from a bitmap

int __stdcall JabberBitmapToAvatar( HBITMAP hBitmap )
{
	if ( !ServiceExists( MS_DIB2PNG ))
		return 1;

	HDC hdc = CreateCompatibleDC( NULL );
	HBITMAP hOldBitmap = ( HBITMAP )SelectObject( hdc, hBitmap );

	BITMAPINFO* bmi = ( BITMAPINFO* )alloca( sizeof( BITMAPINFO ) + sizeof( RGBQUAD )*256 );
	memset( bmi, 0, sizeof( BITMAPINFO ));
	bmi->bmiHeader.biSize = 0x28;
	if ( GetDIBits( hdc, hBitmap, 0, 64, NULL, bmi, DIB_RGB_COLORS ) == 0 ) {
		DWORD tErrorCode = GetLastError();
		JabberLog( "Unable to get the bitmap: error %d", tErrorCode );
		return 2;
	}

	BITMAPINFOHEADER* pDib;
	BYTE* pDibBits;
	pDib = ( BITMAPINFOHEADER* )GlobalAlloc( LPTR, sizeof( BITMAPINFO ) + sizeof( RGBQUAD )*256 + bmi->bmiHeader.biSizeImage );
	if ( pDib == NULL )
		return 3;

	memcpy( pDib, bmi, sizeof( BITMAPINFO ) + sizeof( RGBQUAD )*256 );
	pDibBits = (( BYTE* )pDib ) + sizeof( BITMAPINFO ) + sizeof( RGBQUAD )*256;

	GetDIBits( hdc, hBitmap, 0, pDib->biHeight, pDibBits, ( BITMAPINFO* )pDib, DIB_RGB_COLORS );
	SelectObject( hdc, hOldBitmap );
	DeleteDC( hdc );

	long dwPngSize = 0;
	DIB2PNG convertor;
	convertor.pbmi = ( BITMAPINFO* )pDib;
	convertor.pDiData = pDibBits;
	convertor.pResult = NULL;
	convertor.pResultLen = &dwPngSize;
	if ( !CallService( MS_DIB2PNG, 0, (LPARAM)&convertor )) {
		GlobalFree( pDib );
		return 2;
	}

	convertor.pResult = new BYTE[ dwPngSize ];
	CallService( MS_DIB2PNG, 0, (LPARAM)&convertor );
	GlobalFree( pDib );

	uint8_t digest[SHA1HashSize];
	SHA1Context sha1ctx;
	SHA1Reset( &sha1ctx );
	SHA1Input( &sha1ctx, (uint8_t*)convertor.pResult, dwPngSize );
	SHA1Result( &sha1ctx, digest );

	char tFileName[ MAX_PATH ];
	JabberGetAvatarFileName( NULL, tFileName, MAX_PATH );
	DeleteFileA( tFileName );

	char buf[SHA1HashSize*2+1];
	for ( int i=0; i<SHA1HashSize; i++ )
		sprintf( buf+( i<<1 ), "%02x", digest[i] );
   JSetString( NULL, "AvatarHash", buf );
	JSetByte( "AvatarType", PA_FORMAT_PNG );

	JabberGetAvatarFileName( NULL, tFileName, MAX_PATH );
	FILE* out = fopen( tFileName, "wb" );
	if ( out != NULL ) {
		fwrite( convertor.pResult, dwPngSize, 1, out );
		fclose( out );
	}
	delete convertor.pResult;
	return ERROR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberEnterBitmapName - enters the picture filename

int __stdcall JabberEnterBitmapName( char* szDest )
{
	*szDest = 0;

	char szFilter[ 512 ];
	JCallService( MS_UTILS_GETBITMAPFILTERSTRINGS, sizeof szFilter, ( LPARAM )szFilter );

	char str[ MAX_PATH ]; str[0] = 0;
	OPENFILENAMEA ofn = {0};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szDest;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "bmp";
	if ( !GetOpenFileNameA( &ofn ))
		return 1;

	return ERROR_SUCCESS;
}
