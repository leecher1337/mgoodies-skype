/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005     George Hazan

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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_vcard.cpp,v $
Revision       : $Revision: 1.18 $
Last change on : $Date: 2006/03/16 21:54:39 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <commctrl.h>
#include "jabber_iq.h"
#include "resource.h"

extern char* jabberVcardPhotoFileName;
extern char* jabberVcardPhotoType;

/////////////////////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK JabberVcardDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );

int JabberMenuHandleVcard( WPARAM wParam, LPARAM lParam )
{
	if ( IsWindow( hwndJabberVcard ))
		SetForegroundWindow( hwndJabberVcard );
	else {
		hwndJabberVcard = CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_VCARD ), NULL, JabberVcardDlgProc, ( LPARAM )NULL );
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int JabberSendGetVcard( const char* jid )
{
	int iqId = JabberSerialNext();
	JabberIqAdd( iqId, ( jid == jabberJID ) ? IQ_PROC_GETVCARD : IQ_PROC_NONE, JabberIqResultGetVcard );
	JabberSend( jabberThreadInfo->s,
        "<iq type=\"get\" id=\""JABBER_IQID"%d\" to=\"%s\"><vCard xmlns=\"vcard-temp\" prodid=\"-//HandGen//NONSGML vGen v1.0//EN\" version=\"2.0\" /></iq>",
		iqId, jid );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	HWND hwnd;
	int dlgId;
	DLGPROC dlgProc;
} VcardPage;

typedef struct {
	int pageCount;
	int currentPage;
	RECT rectTab;
	VcardPage *page;
	BOOL changed;
	int updateAnimFrame;
	TCHAR* szUpdating;
	BOOL animating;
} VcardTab;

static void SetDialogField( HWND hwndDlg, int nDlgItem, char* key )
{
	DBVARIANT dbv;

	if ( !DBGetContactSetting( NULL, jabberProtoName, key, &dbv )) {
		char* str = JabberUnixToDos( dbv.pszVal );
		SetDlgItemTextA( hwndDlg, nDlgItem, str );
		free( str );
		JFreeVariant( &dbv );
	}
	else SetDlgItemTextA( hwndDlg, nDlgItem, "" );
}

static BOOL CALLBACK PersonalDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SendMessage( GetDlgItem( hwndDlg, IDC_GENDER ), CB_ADDSTRING, 0, ( LPARAM )TranslateT( "Male" ));
		SendMessage( GetDlgItem( hwndDlg, IDC_GENDER ), CB_ADDSTRING, 0, ( LPARAM )TranslateT( "Female" ));
		SendMessage( hwndDlg, WM_JABBER_REFRESH, 0, 0 );
		return TRUE;
	case WM_JABBER_REFRESH:
		SetDialogField( hwndDlg, IDC_FULLNAME, "FullName" );
		SetDialogField( hwndDlg, IDC_NICKNAME, "Nick" );
		SetDialogField( hwndDlg, IDC_FIRSTNAME, "FirstName" );
		SetDialogField( hwndDlg, IDC_MIDDLE, "MiddleName" );
		SetDialogField( hwndDlg, IDC_LASTNAME, "LastName" );
		SetDialogField( hwndDlg, IDC_BIRTH, "BirthDate" );
		SetDialogField( hwndDlg, IDC_GENDER, "GenderString" );
		SetDialogField( hwndDlg, IDC_OCCUPATION, "Role" );
		SetDialogField( hwndDlg, IDC_HOMEPAGE, "Homepage" );
		break;
	case WM_COMMAND:
		if (( ( HWND )lParam==GetFocus() && HIWORD( wParam )==EN_CHANGE ) ||
			(( HWND )lParam==GetDlgItem( hwndDlg, IDC_GENDER ) && ( HIWORD( wParam )==CBN_EDITCHANGE||HIWORD( wParam )==CBN_SELCHANGE )) )
			SendMessage( GetParent( hwndDlg ), WM_JABBER_CHANGED, 0, 0 );
		break;
	}
	return FALSE;
}

static BOOL CALLBACK HomeDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SendMessage( hwndDlg, WM_JABBER_REFRESH, 0, 0 );
		return TRUE;
	case WM_JABBER_REFRESH:
		SetDialogField( hwndDlg, IDC_ADDRESS1, "Street" );
		SetDialogField( hwndDlg, IDC_ADDRESS2, "Street2" );
		SetDialogField( hwndDlg, IDC_CITY, "City" );
		SetDialogField( hwndDlg, IDC_STATE, "State" );
		SetDialogField( hwndDlg, IDC_ZIP, "ZIP" );
		SetDialogField( hwndDlg, IDC_COUNTRY, "CountryName" );
		break;
	case WM_COMMAND:
		if (( HWND )lParam==GetFocus() && HIWORD( wParam )==EN_CHANGE )
			SendMessage( GetParent( hwndDlg ), WM_JABBER_CHANGED, 0, 0 );
		break;
	}
	return FALSE;
}

static BOOL CALLBACK WorkDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SendMessage( hwndDlg, WM_JABBER_REFRESH, 0, 0 );
		return TRUE;
	case WM_JABBER_REFRESH:
		SetDialogField( hwndDlg, IDC_COMPANY, "Company" );
		SetDialogField( hwndDlg, IDC_DEPARTMENT, "CompanyDepartment" );
		SetDialogField( hwndDlg, IDC_TITLE, "CompanyPosition" );
		SetDialogField( hwndDlg, IDC_ADDRESS1, "CompanyStreet" );
		SetDialogField( hwndDlg, IDC_ADDRESS2, "CompanyStreet2" );
		SetDialogField( hwndDlg, IDC_CITY, "CompanyCity" );
		SetDialogField( hwndDlg, IDC_STATE, "CompanyState" );
		SetDialogField( hwndDlg, IDC_ZIP, "CompanyZIP" );
		SetDialogField( hwndDlg, IDC_COUNTRY, "CompanyCountryName" );
		break;
	case WM_COMMAND:
		if (( HWND )lParam==GetFocus() && HIWORD( wParam )==EN_CHANGE )
			SendMessage( GetParent( hwndDlg ), WM_JABBER_CHANGED, 0, 0 );
		break;
	}
	return FALSE;
}

static char szPhotoFileName[MAX_PATH];
static char szPhotoType[33];
static BOOL bPhotoChanged;

static BOOL CALLBACK PhotoDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	static HBITMAP hBitmap;
	char szTempPath[MAX_PATH], szTempFileName[MAX_PATH];

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		hBitmap = NULL;
		SendMessage( GetDlgItem( hwndDlg, IDC_LOAD ), BM_SETIMAGE, IMAGE_ICON, ( LPARAM )iconList[8]);
		SendMessage( GetDlgItem( hwndDlg, IDC_DELETE ), BM_SETIMAGE, IMAGE_ICON, ( LPARAM )iconList[4]);
		ShowWindow( GetDlgItem( hwndDlg, IDC_SAVE ), SW_HIDE );
		SendMessage( hwndDlg, WM_JABBER_REFRESH, 0, 0 );
		return TRUE;
	case WM_JABBER_REFRESH:
		if ( hBitmap ) {
			DeleteObject( hBitmap );
			hBitmap = NULL;
			DeleteFileA( szPhotoFileName );
			szPhotoFileName[0] = '\0';
		}
		EnableWindow( GetDlgItem( hwndDlg, IDC_DELETE ), FALSE );
		if ( jabberVcardPhotoFileName ) {
			if ( GetTempPathA( sizeof( szTempPath ), szTempPath ) <= 0 )
				strcpy( szTempPath, ".\\" );
			if ( GetTempFileNameA( szTempPath, jabberProtoName, GetTickCount(), szTempFileName ) > 0 ) {
				//put correct extension to make MS_UTILS_LOADBITMAP happy
				szTempFileName[strlen(szTempFileName)-3]='\0';
				strcat(szTempFileName,strrchr(jabberVcardPhotoType,'/')+1);
				JabberLog( "Temp file = %s", szTempFileName );
				if ( CopyFileA( jabberVcardPhotoFileName, szTempFileName, FALSE ) == TRUE ) {
					if (( hBitmap=( HBITMAP ) JCallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )szTempFileName )) != NULL ) {
						strcpy( szPhotoFileName, szTempFileName );
						EnableWindow( GetDlgItem( hwndDlg, IDC_DELETE ), TRUE );
					}
					else DeleteFileA( szTempFileName );
				}
				else DeleteFileA( szTempFileName );
			}
		}
		bPhotoChanged = FALSE;
		InvalidateRect( hwndDlg, NULL, TRUE );
		UpdateWindow( hwndDlg );
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_LOAD:
			{
				OPENFILENAMEA ofn = {0};
				static char szFilter[512];
				char szFileName[_MAX_PATH];
				char* p;
				int n;

//				JCallService( MS_UTILS_GETBITMAPFILTERSTRINGS, ( WPARAM ) sizeof( szFilter ), ( LPARAM )szFilter );
				p = szFilter;
				n = sizeof( szFilter );
				strncpy( p, JTranslate( "All Bitmaps" ), n ); n = sizeof( szFilter )-strlen( szFilter );
				strncat( p, " ( *.bmp;*.png;*.jpg;*.jpeg;*.gif )", n ); n = sizeof( szFilter )-strlen( szFilter );
				p += strlen( p )+1; n = sizeof( szFilter )-( p-szFilter );
				strncpy( p, "*.BMP;*.PNG;*.JPG;*.JPEG;*.GIF", n );
				szFilter[512-1] = '\0';


#ifndef OPENFILENAME_SIZE_VERSION_400
				ofn.lStructSize = sizeof( OPENFILENAME );
#else
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#endif
				ofn.hwndOwner = hwndDlg;
				ofn.lpstrFilter = szFilter;
				ofn.lpstrCustomFilter = NULL;
				ofn.lpstrFile = szFileName;
				ofn.nMaxFile = _MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST;
				szFileName[0] = '\0';
				if ( GetOpenFileNameA( &ofn )) {
					struct _stat st;
					HBITMAP hNewBitmap;

					JabberLog( "File selected is %s", szFileName );
					if ( _stat( szFileName, &st )<0 || st.st_size>40*1024 ) {
						MessageBox( hwndDlg, TranslateT( "Only JPG, GIF, and BMP image files smaller than 40 KB are supported." ), TranslateT( "Jabber vCard" ), MB_OK|MB_SETFOREGROUND );
						break;
					}
					if ( GetTempPathA( sizeof( szTempPath ), szTempPath ) <= 0 )
						strcpy( szTempPath, ".\\" );
					if ( GetTempFileNameA( szTempPath, jabberProtoName, GetTickCount(), szTempFileName ) > 0 ) {
						//put correct extension to make MS_UTILS_LOADBITMAP happy
						szTempFileName[strlen(szTempFileName)-3]='\0';
						strcat(szTempFileName,strrchr(szFileName,'.')+1);
						JabberLog( "Temp file = %s", szTempFileName );
						if ( CopyFileA( szFileName, szTempFileName, FALSE ) == TRUE ) {
							if (( hNewBitmap=( HBITMAP ) JCallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )szTempFileName )) != NULL ) {
								if ( hBitmap ) {
									DeleteObject( hBitmap );
									DeleteFileA( szPhotoFileName );
								}
								if (( p=strrchr( szFileName, '.' )) != NULL ) {
									if ( !stricmp( p, ".bmp" ))
										strcpy( szPhotoType, "image/bmp" );
									else if ( !stricmp( p, ".png" ))
										strcpy( szPhotoType, "image/png" );
									else if ( !stricmp( p, ".gif" ))
										strcpy( szPhotoType, "image/gif" );
									else
										strcpy( szPhotoType, "image/jpeg" );
								}
								else
									strcpy( szPhotoType, "image/jpeg" );
								hBitmap = hNewBitmap;
								strcpy( szPhotoFileName, szTempFileName );
								bPhotoChanged = TRUE;
								EnableWindow( GetDlgItem( hwndDlg, IDC_DELETE ), TRUE );
								InvalidateRect( hwndDlg, NULL, TRUE );
								UpdateWindow( hwndDlg );
								SendMessage( GetParent( hwndDlg ), WM_JABBER_CHANGED, 0, 0 );
							}
							else {
								DeleteFileA( szTempFileName );
							}
						}
						else
							DeleteFileA( szTempFileName );
					}
				}
			}
			break;
		case IDC_DELETE:
			if ( hBitmap ) {
				DeleteObject( hBitmap );
				hBitmap = NULL;
				DeleteFileA( szPhotoFileName );
				szPhotoFileName[0] = '\0';
				bPhotoChanged = TRUE;
				EnableWindow( GetDlgItem( hwndDlg, IDC_DELETE ), FALSE );
				InvalidateRect( hwndDlg, NULL, TRUE );
				UpdateWindow( hwndDlg );
				SendMessage( GetParent( hwndDlg ), WM_JABBER_CHANGED, 0, 0 );
			}
			break;
		}
		break;
	case WM_PAINT:
		if ( hBitmap ) {
			BITMAP bm;
			HDC hdcMem;
			HWND hwndCanvas;
			HDC hdcCanvas;
			POINT ptSize, ptOrg, pt, ptFitSize;
			RECT rect;

			hwndCanvas = GetDlgItem( hwndDlg, IDC_CANVAS );
			hdcCanvas = GetDC( hwndCanvas );
			hdcMem = CreateCompatibleDC( hdcCanvas );
			SelectObject( hdcMem, hBitmap );
			SetMapMode( hdcMem, GetMapMode( hdcCanvas ));
			GetObject( hBitmap, sizeof( BITMAP ), ( LPVOID ) &bm );
			ptSize.x = bm.bmWidth;
			ptSize.y = bm.bmHeight;
			DPtoLP( hdcCanvas, &ptSize, 1 );
			ptOrg.x = ptOrg.y = 0;
			DPtoLP( hdcMem, &ptOrg, 1 );
			GetClientRect( hwndCanvas, &rect );
			InvalidateRect( hwndCanvas, NULL, TRUE );
			UpdateWindow( hwndCanvas );
			if ( ptSize.x<=rect.right && ptSize.y<=rect.bottom ) {
				pt.x = ( rect.right - ptSize.x )/2;
				pt.y = ( rect.bottom - ptSize.y )/2;
				BitBlt( hdcCanvas, pt.x, pt.y, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y, SRCCOPY );
			}
			else {
				if (( ( float )( ptSize.x-rect.right ))/ptSize.x > (( float )( ptSize.y-rect.bottom ))/ptSize.y ) {
					ptFitSize.x = rect.right;
					ptFitSize.y = ( ptSize.y*rect.right )/ptSize.x;
					pt.x = 0;
					pt.y = ( rect.bottom - ptFitSize.y )/2;
				}
				else {
					ptFitSize.x = ( ptSize.x*rect.bottom )/ptSize.y;
					ptFitSize.y = rect.bottom;
					pt.x = ( rect.right - ptFitSize.x )/2;
					pt.y = 0;
				}
				SetStretchBltMode( hdcCanvas, COLORONCOLOR );
				StretchBlt( hdcCanvas, pt.x, pt.y, ptFitSize.x, ptFitSize.y, hdcMem, ptOrg.x, ptOrg.y, ptSize.x, ptSize.y, SRCCOPY );
			}
			DeleteDC( hdcMem );
		}
		break;
	case WM_DESTROY:
		if ( hBitmap ) {
			JabberLog( "Delete bitmap" );
			DeleteObject( hBitmap );
			DeleteFileA( szPhotoFileName );
			szPhotoFileName[0] = '\0';
		}
		break;
	}
	return FALSE;
}

static BOOL CALLBACK NoteDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SendMessage( hwndDlg, WM_JABBER_REFRESH, 0, 0 );
		return TRUE;
	case WM_JABBER_REFRESH:
		SetDialogField( hwndDlg, IDC_DESC, "About" );
		break;
	case WM_COMMAND:
		if (( HWND )lParam==GetFocus() && HIWORD( wParam )==EN_CHANGE )
			SendMessage( GetParent( hwndDlg ), WM_JABBER_CHANGED, 0, 0 );
		break;
	}
	return FALSE;
}

static BOOL CALLBACK EditEmailDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SetWindowLong( hwndDlg, GWL_USERDATA, lParam );
		if ( lParam >= 0 ) {
			DBVARIANT dbv;
			char idstr[33];
			WORD nFlag;

			SetWindowText( hwndDlg, TranslateT( "Jabber vCard: Edit Email Address" ));
			wsprintfA( idstr, "e-mail%d", lParam );
			if ( !DBGetContactSetting( NULL, jabberProtoName, idstr, &dbv )) {
				SetDlgItemTextA( hwndDlg, IDC_EMAIL, dbv.pszVal );
				JFreeVariant( &dbv );
				wsprintfA( idstr, "e-mailFlag%d", lParam );
				nFlag = DBGetContactSettingWord( NULL, jabberProtoName, idstr, 0 );
				if ( nFlag & JABBER_VCEMAIL_HOME ) CheckDlgButton( hwndDlg, IDC_HOME, TRUE );
				if ( nFlag & JABBER_VCEMAIL_WORK ) CheckDlgButton( hwndDlg, IDC_WORK, TRUE );
				if ( nFlag & JABBER_VCEMAIL_INTERNET ) CheckDlgButton( hwndDlg, IDC_INTERNET, TRUE );
				if ( nFlag & JABBER_VCEMAIL_X400 ) CheckDlgButton( hwndDlg, IDC_X400, TRUE );
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
			{
				char text[128];
				char idstr[33];
				int id = ( int ) GetWindowLong( hwndDlg, GWL_USERDATA );
				DBVARIANT dbv;
				WORD nFlag;

				if ( id < 0 ) {
					for ( id=0;;id++ ) {
						wsprintfA( idstr, "e-mail%d", id );
						if ( DBGetContactSetting( NULL, jabberProtoName, idstr, &dbv )) break;
						JFreeVariant( &dbv );
					}
				}
				GetDlgItemTextA( hwndDlg, IDC_EMAIL, text, sizeof( text ));
				wsprintfA( idstr, "e-mail%d", id );
				JSetString( NULL, idstr, text );
				nFlag = 0;
				if ( IsDlgButtonChecked( hwndDlg, IDC_HOME )) nFlag |= JABBER_VCEMAIL_HOME;
				if ( IsDlgButtonChecked( hwndDlg, IDC_WORK )) nFlag |= JABBER_VCEMAIL_WORK;
				if ( IsDlgButtonChecked( hwndDlg, IDC_INTERNET )) nFlag |= JABBER_VCEMAIL_INTERNET;
				if ( IsDlgButtonChecked( hwndDlg, IDC_X400 )) nFlag |= JABBER_VCEMAIL_X400;
				wsprintfA( idstr, "e-mailFlag%d", id );
				JSetWord( NULL, idstr, nFlag );
			}
			// fall through
		case IDCANCEL:
			EndDialog( hwndDlg, wParam );
			break;
		}
	}
	return FALSE;
}

static BOOL CALLBACK EditPhoneDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SetWindowLong( hwndDlg, GWL_USERDATA, lParam );
		if ( lParam >= 0 ) {
			DBVARIANT dbv;
			char idstr[33];
			WORD nFlag;

			SetWindowText( hwndDlg, TranslateT( "Jabber vCard: Edit Phone Number" ));
			wsprintfA( idstr, "Phone%d", lParam );
			if ( !DBGetContactSetting( NULL, jabberProtoName, idstr, &dbv )) {
				SetDlgItemTextA( hwndDlg, IDC_PHONE, dbv.pszVal );
				JFreeVariant( &dbv );
				wsprintfA( idstr, "PhoneFlag%d", lParam );
				nFlag = JGetWord( NULL, idstr, 0 );
				if ( nFlag & JABBER_VCTEL_HOME ) CheckDlgButton( hwndDlg, IDC_HOME, TRUE );
				if ( nFlag & JABBER_VCTEL_WORK ) CheckDlgButton( hwndDlg, IDC_WORK, TRUE );
				if ( nFlag & JABBER_VCTEL_VOICE ) CheckDlgButton( hwndDlg, IDC_VOICE, TRUE );
				if ( nFlag & JABBER_VCTEL_FAX ) CheckDlgButton( hwndDlg, IDC_FAX, TRUE );
				if ( nFlag & JABBER_VCTEL_PAGER ) CheckDlgButton( hwndDlg, IDC_PAGER, TRUE );
				if ( nFlag & JABBER_VCTEL_MSG ) CheckDlgButton( hwndDlg, IDC_MSG, TRUE );
				if ( nFlag & JABBER_VCTEL_CELL ) CheckDlgButton( hwndDlg, IDC_CELL, TRUE );
				if ( nFlag & JABBER_VCTEL_VIDEO ) CheckDlgButton( hwndDlg, IDC_VIDEO, TRUE );
				if ( nFlag & JABBER_VCTEL_BBS ) CheckDlgButton( hwndDlg, IDC_BBS, TRUE );
				if ( nFlag & JABBER_VCTEL_MODEM ) CheckDlgButton( hwndDlg, IDC_MODEM, TRUE );
				if ( nFlag & JABBER_VCTEL_ISDN ) CheckDlgButton( hwndDlg, IDC_ISDN, TRUE );
				if ( nFlag & JABBER_VCTEL_PCS ) CheckDlgButton( hwndDlg, IDC_PCS, TRUE );
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
			{
				char text[128];
				char idstr[33];
				int id = ( int ) GetWindowLong( hwndDlg, GWL_USERDATA );
				DBVARIANT dbv;
				WORD nFlag;

				if ( id < 0 ) {
					for ( id=0;;id++ ) {
						wsprintfA( idstr, "Phone%d", id );
						if ( DBGetContactSetting( NULL, jabberProtoName, idstr, &dbv )) break;
						JFreeVariant( &dbv );
					}
				}
				GetDlgItemTextA( hwndDlg, IDC_PHONE, text, sizeof( text ));
				wsprintfA( idstr, "Phone%d", id );
				JSetString( NULL, idstr, text );
				nFlag = 0;
				if ( IsDlgButtonChecked( hwndDlg, IDC_HOME )) nFlag |= JABBER_VCTEL_HOME;
				if ( IsDlgButtonChecked( hwndDlg, IDC_WORK )) nFlag |= JABBER_VCTEL_WORK;
				if ( IsDlgButtonChecked( hwndDlg, IDC_VOICE )) nFlag |= JABBER_VCTEL_VOICE;
				if ( IsDlgButtonChecked( hwndDlg, IDC_FAX )) nFlag |= JABBER_VCTEL_FAX;
				if ( IsDlgButtonChecked( hwndDlg, IDC_PAGER )) nFlag |= JABBER_VCTEL_PAGER;
				if ( IsDlgButtonChecked( hwndDlg, IDC_MSG )) nFlag |= JABBER_VCTEL_MSG;
				if ( IsDlgButtonChecked( hwndDlg, IDC_CELL )) nFlag |= JABBER_VCTEL_CELL;
				if ( IsDlgButtonChecked( hwndDlg, IDC_VIDEO )) nFlag |= JABBER_VCTEL_VIDEO;
				if ( IsDlgButtonChecked( hwndDlg, IDC_BBS )) nFlag |= JABBER_VCTEL_BBS;
				if ( IsDlgButtonChecked( hwndDlg, IDC_MODEM )) nFlag |= JABBER_VCTEL_MODEM;
				if ( IsDlgButtonChecked( hwndDlg, IDC_ISDN )) nFlag |= JABBER_VCTEL_ISDN;
				if ( IsDlgButtonChecked( hwndDlg, IDC_PCS )) nFlag |= JABBER_VCTEL_PCS;
				wsprintfA( idstr, "PhoneFlag%d", id );
				JSetWord( NULL, idstr, nFlag );
			}
			// fall through
		case IDCANCEL:
			EndDialog( hwndDlg, wParam );
			break;
		}
	}
	return FALSE;
}

#define M_REMAKELISTS  ( WM_USER+1 )
static BOOL CALLBACK ContactDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg ) {
	case WM_INITDIALOG:
		{
			LVCOLUMN lvc;
			RECT rc;

			TranslateDialogDefault( hwndDlg );
			GetClientRect( GetDlgItem( hwndDlg,IDC_EMAILS ), &rc );
			rc.right -= GetSystemMetrics( SM_CXVSCROLL );
			lvc.mask = LVCF_WIDTH;
			lvc.cx = 30;
			ListView_InsertColumn( GetDlgItem( hwndDlg,IDC_EMAILS ), 0, &lvc );
			ListView_InsertColumn( GetDlgItem( hwndDlg,IDC_PHONES ), 0, &lvc );
			lvc.cx = rc.right - 30 - 40;
			ListView_InsertColumn( GetDlgItem( hwndDlg,IDC_EMAILS ), 1, &lvc );
			ListView_InsertColumn( GetDlgItem( hwndDlg,IDC_PHONES ), 1, &lvc );
			lvc.cx = 20;
			ListView_InsertColumn( GetDlgItem( hwndDlg,IDC_EMAILS ), 2, &lvc );
			ListView_InsertColumn( GetDlgItem( hwndDlg,IDC_EMAILS ), 3, &lvc );
			ListView_InsertColumn( GetDlgItem( hwndDlg,IDC_PHONES ), 2, &lvc );
			ListView_InsertColumn( GetDlgItem( hwndDlg,IDC_PHONES ), 3, &lvc );
			SendMessage( hwndDlg, M_REMAKELISTS, 0, 0 );
		}
		return TRUE;
	case M_REMAKELISTS:
		{
			LVITEM lvi;
			int i;
			char idstr[33];
			TCHAR number[20];
			DBVARIANT dbv;

			//e-mails
			ListView_DeleteAllItems( GetDlgItem( hwndDlg, IDC_EMAILS ));
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.iItem = 0;
			for ( i=0;;i++ ) {
				wsprintfA( idstr, "e-mail%d", i );
				if ( DBGetContactSettingTString( NULL, jabberProtoName, idstr, &dbv )) break;
				wsprintf( number, _T("%d"), i+1 );
				lvi.pszText = number;
				lvi.lParam = ( LPARAM )i;
				ListView_InsertItem( GetDlgItem( hwndDlg, IDC_EMAILS ), &lvi );
				ListView_SetItemText( GetDlgItem( hwndDlg, IDC_EMAILS ), lvi.iItem, 1, dbv.ptszVal );
				JFreeVariant( &dbv );
				lvi.iItem++;
			}
			lvi.mask = LVIF_PARAM;
			lvi.lParam = ( LPARAM )( -1 );
			ListView_InsertItem( GetDlgItem( hwndDlg, IDC_EMAILS ), &lvi );
			//phones
			ListView_DeleteAllItems( GetDlgItem( hwndDlg, IDC_PHONES ));
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.iItem = 0;
			for ( i=0;;i++ ) {
				wsprintfA( idstr, "Phone%d", i );
				if ( DBGetContactSettingTString( NULL, jabberProtoName, idstr, &dbv )) break;
				wsprintf( number, _T("%d"), i+1 );
				lvi.pszText = number;
				lvi.lParam = ( LPARAM )i;
				ListView_InsertItem( GetDlgItem( hwndDlg, IDC_PHONES ), &lvi );
				ListView_SetItemText( GetDlgItem( hwndDlg, IDC_PHONES ), lvi.iItem, 1, dbv.ptszVal );
				JFreeVariant( &dbv );
				lvi.iItem++;
			}
			lvi.mask = LVIF_PARAM;
			lvi.lParam = ( LPARAM )( -1 );
			ListView_InsertItem( GetDlgItem( hwndDlg, IDC_PHONES ), &lvi );
		}
		break;
	case WM_NOTIFY:
		switch (( ( LPNMHDR )lParam )->idFrom ) {
		case IDC_EMAILS:
		case IDC_PHONES:
			switch (( ( LPNMHDR )lParam )->code ) {
			case NM_CUSTOMDRAW:
				{
					NMLVCUSTOMDRAW *nm = ( NMLVCUSTOMDRAW * ) lParam;

					switch ( nm->nmcd.dwDrawStage ) {
					case CDDS_PREPAINT:
					case CDDS_ITEMPREPAINT:
						SetWindowLong( hwndDlg, DWL_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW );
						return TRUE;
					case CDDS_SUBITEM|CDDS_ITEMPREPAINT:
						{
							RECT rc;
							HICON hIcon;

							ListView_GetSubItemRect( nm->nmcd.hdr.hwndFrom, nm->nmcd.dwItemSpec, nm->iSubItem, LVIR_LABEL, &rc );
							if( nm->nmcd.lItemlParam==( LPARAM )( -1 ) && nm->iSubItem==3 )
								hIcon = iconList[3];
							else if ( nm->iSubItem==2 && nm->nmcd.lItemlParam!=( LPARAM )( -1 ))
								hIcon = iconList[5];
							else if ( nm->iSubItem==3 && nm->nmcd.lItemlParam!=( LPARAM )( -1 ))
								hIcon = iconList[4];
							else break;
							DrawIconEx( nm->nmcd.hdc, ( rc.left+rc.right-GetSystemMetrics( SM_CXSMICON ))/2, ( rc.top+rc.bottom-GetSystemMetrics( SM_CYSMICON ))/2,hIcon, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ), 0, NULL, DI_NORMAL );
							DestroyIcon( hIcon );
							SetWindowLong( hwndDlg, DWL_MSGRESULT, CDRF_SKIPDEFAULT );
						}
						return TRUE;
					}
				}
				break;
			case NM_CLICK:
				{
					NMLISTVIEW *nm = ( NMLISTVIEW * ) lParam;
					LVITEM lvi;
					const char* szIdTemplate = nm->hdr.idFrom==IDC_PHONES?"Phone%d":"e-mail%d";
					const char* szFlagTemplate = nm->hdr.idFrom==IDC_PHONES?"PhoneFlag%d":"e-mailFlag%d";
					LVHITTESTINFO hti;

					if ( nm->iSubItem < 2 ) break;
					hti.pt.x = ( short ) LOWORD( GetMessagePos());
					hti.pt.y = ( short ) HIWORD( GetMessagePos());
					ScreenToClient( nm->hdr.hwndFrom, &hti.pt );
					if ( ListView_SubItemHitTest( nm->hdr.hwndFrom, &hti ) == -1 ) break;
					lvi.mask = LVIF_PARAM;
					lvi.iItem = hti.iItem;
					lvi.iSubItem = 0;
					ListView_GetItem( nm->hdr.hwndFrom, &lvi );
					if ( lvi.lParam == ( LPARAM )( -1 )) {
						if ( hti.iSubItem == 3 ) {
							//add
							if ( DialogBoxParam( hInst, MAKEINTRESOURCE( nm->hdr.idFrom==IDC_PHONES?IDD_VCARD_ADDPHONE:IDD_VCARD_ADDEMAIL ), hwndDlg, nm->hdr.idFrom==IDC_PHONES?EditPhoneDlgProc:EditEmailDlgProc, ( LPARAM )( -1 )) != IDOK )
								break;
							SendMessage( hwndDlg, M_REMAKELISTS, 0, 0 );
							SendMessage( GetParent( hwndDlg ), WM_JABBER_CHANGED, 0, 0 );
						}
					}
					else {
						if ( hti.iSubItem == 3 ) {
							//delete
							int i;
							char idstr[33];
							DBVARIANT dbv;

							for( i=lvi.lParam;;i++ ) {
								WORD nFlag;

								wsprintfA( idstr, szIdTemplate, i+1 );
								if ( DBGetContactSetting( NULL, jabberProtoName, idstr, &dbv )) break;
								wsprintfA( idstr,szIdTemplate,i );
								JSetString( NULL, idstr, dbv.pszVal );
								wsprintfA( idstr, szFlagTemplate, i+1 );
								JFreeVariant( &dbv );
								nFlag = JGetWord( NULL, idstr, 0 );
								wsprintfA( idstr, szFlagTemplate, i );
								JSetWord( NULL, idstr, nFlag );
							}
							wsprintfA( idstr, szIdTemplate, i );
							JDeleteSetting( NULL, idstr );
							wsprintfA( idstr, szFlagTemplate, i );
							JDeleteSetting( NULL, idstr );
							SendMessage( hwndDlg, M_REMAKELISTS, 0, 0 );
							SendMessage( GetParent( hwndDlg ), WM_JABBER_CHANGED, 0, 0 );
						}
						else if ( hti.iSubItem == 2 ) {
							//edit
							if ( DialogBoxParam( hInst, MAKEINTRESOURCE( nm->hdr.idFrom==IDC_PHONES?IDD_VCARD_ADDPHONE:IDD_VCARD_ADDEMAIL ), hwndDlg, nm->hdr.idFrom==IDC_PHONES?EditPhoneDlgProc:EditEmailDlgProc, lvi.lParam ) != IDOK )
								break;
							SendMessage( hwndDlg,M_REMAKELISTS,0,0 );
							SendMessage( GetParent( hwndDlg ), WM_JABBER_CHANGED, 0, 0 );
						}
					}
				}
				break;
			}
			break;
		}
		break;
	case WM_SETCURSOR:
		if ( LOWORD( lParam ) != HTCLIENT ) break;
		if ( GetForegroundWindow() == GetParent( hwndDlg )) {
			POINT pt;
			GetCursorPos( &pt );
			ScreenToClient( hwndDlg,&pt );
			SetFocus( ChildWindowFromPoint( hwndDlg,pt ));	  //ugly hack because listviews ignore their first click
		}
		break;
	}
	return FALSE;
}

static void SaveVcardToDB( VcardTab *dat )
{
	HWND hwndPage;
	char text[2048];

	if ( dat==NULL || dat->page==NULL ) return;

	// Page 0: Personal
	if (( hwndPage=dat->page[0].hwnd ) != NULL ) {
		GetDlgItemTextA( hwndPage, IDC_FULLNAME, text, sizeof( text ));
		JSetString( NULL, "FullName", text );
		GetDlgItemTextA( hwndPage, IDC_NICKNAME, text, sizeof( text ));
		JSetString( NULL, "Nick", text );
		GetDlgItemTextA( hwndPage, IDC_FIRSTNAME, text, sizeof( text ));
		JSetString( NULL, "FirstName", text );
		GetDlgItemTextA( hwndPage, IDC_MIDDLE, text, sizeof( text ));
		JSetString( NULL, "MiddleName", text );
		GetDlgItemTextA( hwndPage, IDC_LASTNAME, text, sizeof( text ));
		JSetString( NULL, "LastName", text );
		GetDlgItemTextA( hwndPage, IDC_BIRTH, text, sizeof( text ));
		JSetString( NULL, "BirthDate", text );
		GetDlgItemTextA( hwndPage, IDC_GENDER, text, sizeof( text ));
		JSetString( NULL, "GenderString", text );
		GetDlgItemTextA( hwndPage, IDC_OCCUPATION, text, sizeof( text ));
		JSetString( NULL, "Role", text );
		GetDlgItemTextA( hwndPage, IDC_HOMEPAGE, text, sizeof( text ));
		JSetString( NULL, "Homepage", text );
	}
	// Page 1: Contacts
	//		no need to save, always in sync with the DB
	// Page 2: Home
	if (( hwndPage=dat->page[2].hwnd ) != NULL ) {
		GetDlgItemTextA( hwndPage, IDC_ADDRESS1, text, sizeof( text ));
		JSetString( NULL, "Street", text );
		GetDlgItemTextA( hwndPage, IDC_ADDRESS2, text, sizeof( text ));
		JSetString( NULL, "Street2", text );
		GetDlgItemTextA( hwndPage, IDC_CITY, text, sizeof( text ));
		JSetString( NULL, "City", text );
		GetDlgItemTextA( hwndPage, IDC_STATE, text, sizeof( text ));
		JSetString( NULL, "State", text );
		GetDlgItemTextA( hwndPage, IDC_ZIP, text, sizeof( text ));
		JSetString( NULL, "ZIP", text );
		GetDlgItemTextA( hwndPage, IDC_COUNTRY, text, sizeof( text ));
		JSetString( NULL, "CountryName", text );
	}
	// Page 3: Work
	if (( hwndPage=dat->page[3].hwnd ) != NULL ) {
		GetDlgItemTextA( hwndPage, IDC_COMPANY, text, sizeof( text ));
		JSetString( NULL, "Company", text );
		GetDlgItemTextA( hwndPage, IDC_DEPARTMENT, text, sizeof( text ));
		JSetString( NULL, "CompanyDepartment", text );
		GetDlgItemTextA( hwndPage, IDC_TITLE, text, sizeof( text ));
		JSetString( NULL, "CompanyPosition", text );
		GetDlgItemTextA( hwndPage, IDC_ADDRESS1, text, sizeof( text ));
		JSetString( NULL, "CompanyStreet", text );
		GetDlgItemTextA( hwndPage, IDC_ADDRESS2, text, sizeof( text ));
		JSetString( NULL, "CompanyStreet2", text );
		GetDlgItemTextA( hwndPage, IDC_CITY, text, sizeof( text ));
		JSetString( NULL, "CompanyCity", text );
		GetDlgItemTextA( hwndPage, IDC_STATE, text, sizeof( text ));
		JSetString( NULL, "CompanyState", text );
		GetDlgItemTextA( hwndPage, IDC_ZIP, text, sizeof( text ));
		JSetString( NULL, "CompanyZIP", text );
		GetDlgItemTextA( hwndPage, IDC_COUNTRY, text, sizeof( text ));
		JSetString( NULL, "CompanyCountryName", text );
	}
	// Page 4: Photo
	//		not saved in the database
	// Page 5: Note
	if (( hwndPage=dat->page[5].hwnd ) != NULL ) {
		GetDlgItemTextA( hwndPage, IDC_DESC, text, sizeof( text ));
		JSetString( NULL, "About", text );
	}
}

static void AppendVcardTextRaw( char* *buffer, int *bufferSize, char* text )
{
	int size;

	if ( buffer==NULL || *buffer==NULL || bufferSize==NULL || *bufferSize<=0 || text==NULL )
		return;
	size = strlen( *buffer ) + strlen( text ) + 1;
	if ( size > *bufferSize ) {
		*bufferSize = size + 1024;
		*buffer = ( char* )realloc( *buffer, *bufferSize );
		JabberLog( "Increase buffer size to %d", *bufferSize );
	}
	if ( *buffer ) strcat( *buffer, text );
}

static void AppendVcardText( char* *buffer, int *bufferSize, char* text )
{
	char* str;
	int size;

	if ( buffer==NULL || *buffer==NULL || bufferSize==NULL || *bufferSize<=0 || text==NULL )
		return;
	if (( str=JabberTextEncode( text )) == NULL ) return;
	size = strlen( *buffer ) + strlen( str ) + 1;
	if ( size > *bufferSize ) {
		*bufferSize = size + 1024;
		*buffer = ( char* )realloc( *buffer, *bufferSize );
	}
	if ( *buffer ) strcat( *buffer, str );
	free( str );
}

static void AppendVcardFromDB( char* *buffer, int *bufferSize, char* tag, char* key )
{
	DBVARIANT dbv;
	char* text, *str;

	if ( buffer==NULL || *buffer==NULL || bufferSize==NULL || *bufferSize<=0 || tag==NULL || key==NULL )
		return;
	if ( DBGetContactSetting( NULL, jabberProtoName, key, &dbv )) {
		if (( text=( char* )malloc( strlen( tag ) + 4 )) == NULL ) return;
		sprintf( text, "<%s/>", tag );
	}
	else {
		str = JabberTextEncode( dbv.pszVal );
		if (( text=( char* )malloc( 2*strlen( tag ) + strlen( str ) + 6 )) == NULL ) {
			free( str );
			JFreeVariant( &dbv );
			return;
		}
		sprintf( text, "<%s>%s</%s>", tag, str, tag );
		free( str );
		JFreeVariant( &dbv );
	}
	AppendVcardTextRaw( buffer, bufferSize, text );
	free( text );
}

static void SetServerVcard()
{
	DBVARIANT dbv;
	int iqId;
	char* text, *szFileName, *szFileType;
	int textSize, i;
	char idstr[33];
	WORD nFlag;

	textSize = 4096;
	text = ( char* )malloc( textSize );
	text[0] = '\0';

	AppendVcardFromDB( &text, &textSize, "FN", "FullName" );

	AppendVcardTextRaw( &text, &textSize, "<N>" );
	AppendVcardFromDB( &text, &textSize, "GIVEN", "FirstName" );
	AppendVcardFromDB( &text, &textSize, "MIDDLE", "MiddleName" );
	AppendVcardFromDB( &text, &textSize, "FAMILY", "LastName" );
	AppendVcardTextRaw( &text, &textSize, "</N>" );

	AppendVcardFromDB( &text, &textSize, "NICKNAME", "Nick" );
	AppendVcardFromDB( &text, &textSize, "BDAY", "BirthDate" );
	AppendVcardFromDB( &text, &textSize, "GENDER", "GenderString" );

	for ( i=0;;i++ ) {
		wsprintfA( idstr, "e-mail%d", i );
		if ( DBGetContactSetting( NULL, jabberProtoName, idstr, &dbv )) break;
		AppendVcardTextRaw( &text, &textSize, "<EMAIL>" );
		AppendVcardText( &text, &textSize, dbv.pszVal );	// for compatibility with client using old vcard format
		JFreeVariant( &dbv );
		AppendVcardFromDB( &text, &textSize, "USERID", idstr );
		wsprintfA( idstr, "e-mailFlag%d", i );
		nFlag = DBGetContactSettingWord( NULL, jabberProtoName, idstr, 0 );
		if ( nFlag & JABBER_VCEMAIL_HOME ) AppendVcardTextRaw( &text, &textSize, "<HOME/>" );
		if ( nFlag & JABBER_VCEMAIL_WORK ) AppendVcardTextRaw( &text, &textSize, "<WORK/>" );
		if ( nFlag & JABBER_VCEMAIL_INTERNET ) AppendVcardTextRaw( &text, &textSize, "<INTERNET/>" );
		if ( nFlag & JABBER_VCEMAIL_X400 ) AppendVcardTextRaw( &text, &textSize, "<X400/>" );
		AppendVcardTextRaw( &text, &textSize, "</EMAIL>" );
	}

	AppendVcardTextRaw( &text, &textSize, "<ADR><HOME/>" );
	AppendVcardFromDB( &text, &textSize, "STREET", "Street" );
	AppendVcardFromDB( &text, &textSize, "EXTADR", "Street2" );
	AppendVcardFromDB( &text, &textSize, "EXTADD", "Street2" );	// for compatibility with client using old vcard format
	AppendVcardFromDB( &text, &textSize, "LOCALITY", "City" );
	AppendVcardFromDB( &text, &textSize, "REGION", "State" );
	AppendVcardFromDB( &text, &textSize, "PCODE", "ZIP" );
	AppendVcardFromDB( &text, &textSize, "CTRY", "CountryName" );
	AppendVcardFromDB( &text, &textSize, "COUNTRY", "CountryName" );	// for compatibility with client using old vcard format
	AppendVcardTextRaw( &text, &textSize, "</ADR>" );

	AppendVcardTextRaw( &text, &textSize, "<ADR><WORK/>" );
	AppendVcardFromDB( &text, &textSize, "STREET", "CompanyStreet" );
	AppendVcardFromDB( &text, &textSize, "EXTADR", "CompanyStreet2" );
	AppendVcardFromDB( &text, &textSize, "EXTADD", "CompanyStreet2" );	// for compatibility with client using old vcard format
	AppendVcardFromDB( &text, &textSize, "LOCALITY", "CompanyCity" );
	AppendVcardFromDB( &text, &textSize, "REGION", "CompanyState" );
	AppendVcardFromDB( &text, &textSize, "PCODE", "CompanyZIP" );
	AppendVcardFromDB( &text, &textSize, "CTRY", "CompanyCountryName" );
	AppendVcardFromDB( &text, &textSize, "COUNTRY", "CompanyCountryName" );	// for compatibility with client using old vcard format
	AppendVcardTextRaw( &text, &textSize, "</ADR>" );

	AppendVcardTextRaw( &text, &textSize, "<ORG>" );
	AppendVcardFromDB( &text, &textSize, "ORGNAME", "Company" );
	AppendVcardFromDB( &text, &textSize, "ORGUNIT", "CompanyDepartment" );
	AppendVcardTextRaw( &text, &textSize, "</ORG>" );

	AppendVcardFromDB( &text, &textSize, "TITLE", "CompanyPosition" );
	AppendVcardFromDB( &text, &textSize, "ROLE", "Role" );
	AppendVcardFromDB( &text, &textSize, "URL", "Homepage" );
	AppendVcardFromDB( &text, &textSize, "DESC", "About" );

	for ( i=0;;i++ ) {
		wsprintfA( idstr, "Phone%d", i );
		if ( DBGetContactSetting( NULL, jabberProtoName, idstr, &dbv )) break;
		JFreeVariant( &dbv );
		AppendVcardTextRaw( &text, &textSize, "<TEL>" );
		AppendVcardFromDB( &text, &textSize, "NUMBER", idstr );
		wsprintfA( idstr, "PhoneFlag%d", i );
		nFlag = JGetWord( NULL, idstr, 0 );
		if ( nFlag & JABBER_VCTEL_HOME ) AppendVcardTextRaw( &text, &textSize, "<HOME/>" );
		if ( nFlag & JABBER_VCTEL_WORK ) AppendVcardTextRaw( &text, &textSize, "<WORK/>" );
		if ( nFlag & JABBER_VCTEL_VOICE ) AppendVcardTextRaw( &text, &textSize, "<VOICE/>" );
		if ( nFlag & JABBER_VCTEL_FAX ) AppendVcardTextRaw( &text, &textSize, "<FAX/>" );
		if ( nFlag & JABBER_VCTEL_PAGER ) AppendVcardTextRaw( &text, &textSize, "<PAGER/>" );
		if ( nFlag & JABBER_VCTEL_MSG ) AppendVcardTextRaw( &text, &textSize, "<MSG/>" );
		if ( nFlag & JABBER_VCTEL_CELL ) AppendVcardTextRaw( &text, &textSize, "<CELL/>" );
		if ( nFlag & JABBER_VCTEL_VIDEO ) AppendVcardTextRaw( &text, &textSize, "<VIDEO/>" );
		if ( nFlag & JABBER_VCTEL_BBS ) AppendVcardTextRaw( &text, &textSize, "<BBS/>" );
		if ( nFlag & JABBER_VCTEL_MODEM ) AppendVcardTextRaw( &text, &textSize, "<MODEM/>" );
		if ( nFlag & JABBER_VCTEL_ISDN ) AppendVcardTextRaw( &text, &textSize, "<ISDN/>" );
		if ( nFlag & JABBER_VCTEL_PCS ) AppendVcardTextRaw( &text, &textSize, "<PCS/>" );
		AppendVcardTextRaw( &text, &textSize, "</TEL>" );
	}

	if ( bPhotoChanged ) {
		if ( szPhotoFileName[0] ) {
			szFileName = szPhotoFileName;
			szFileType = szPhotoType;
		}
		else szFileName = NULL;
	}
	else {
		szFileName = jabberVcardPhotoFileName;
		szFileType = jabberVcardPhotoType;
	}

	// Set photo element, also update the global jabberVcardPhotoFileName to reflect the update
	JabberLog( "Before update, jabberVcardPhotoFileName = %s", jabberVcardPhotoFileName );
	if ( szFileName == NULL ) {
		AppendVcardTextRaw( &text, &textSize, "<PHOTO/>" );
		if ( jabberVcardPhotoFileName ) {
			DeleteFileA( jabberVcardPhotoFileName );
			free( jabberVcardPhotoFileName );
			jabberVcardPhotoFileName = NULL;
	}	}
	else {
		char szTempPath[MAX_PATH], szTempFileName[MAX_PATH];
		HANDLE hFile;
		struct _stat st;
		char* buffer, *str;
		DWORD nRead;

		JabberLog( "Saving picture from %s", szFileName );
		if ( _stat( szFileName, &st ) >= 0 ) {
			// Note the FILE_SHARE_READ attribute so that the CopyFile can succeed
			if (( hFile=CreateFileA( szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL )) != INVALID_HANDLE_VALUE ) {
				if (( buffer=( char* )malloc( st.st_size )) != NULL ) {
					if ( ReadFile( hFile, buffer, st.st_size, &nRead, NULL )) {
						if (( str=JabberBase64Encode( buffer, nRead )) != NULL ) {
							AppendVcardTextRaw( &text, &textSize, "<PHOTO><TYPE>" );
							if ( szFileType )
								AppendVcardTextRaw( &text, &textSize, szFileType );
							else
								AppendVcardTextRaw( &text, &textSize, "image/jpeg" );
							if ( szFileType )
								JabberLog( "File type sent is %s", szFileType );
							else
								JabberLog( "File type sent is default to image/jpge" );
							AppendVcardTextRaw( &text, &textSize, "</TYPE><BINVAL>" );
							AppendVcardTextRaw( &text, &textSize, str );
							AppendVcardTextRaw( &text, &textSize, "</BINVAL></PHOTO>" );
							free( str );
							if ( szFileName != jabberVcardPhotoFileName ) {
								if ( jabberVcardPhotoFileName ) {
									DeleteFileA( jabberVcardPhotoFileName );
									free( jabberVcardPhotoFileName );
									jabberVcardPhotoFileName = NULL;
									if ( jabberVcardPhotoType ) {
										free( jabberVcardPhotoType );
										jabberVcardPhotoType = NULL;
								}	}

								if ( GetTempPathA( sizeof( szTempPath ), szTempPath ) <= 0 )
									strcpy( szTempPath, ".\\" );
								if ( GetTempFileNameA( szTempPath, jabberProtoName, GetTickCount(), szTempFileName ) > 0 ) {
									//put correct extension to make MS_UTILS_LOADBITMAP happy
									szTempFileName[strlen(szTempFileName)-3]='\0';
									strcat(szTempFileName,strrchr(szFileType,'/')+1);
									JabberLog( "New global file is %s", szTempFileName );
									if ( CopyFileA( szFileName, szTempFileName, FALSE ) == TRUE ) {
										jabberVcardPhotoFileName = _strdup( szTempFileName );
										if ( jabberVcardPhotoType ) free( jabberVcardPhotoType );
										if ( szFileType )
											jabberVcardPhotoType = _strdup( szFileType );
										else
											jabberVcardPhotoType = NULL;
									}
									else DeleteFileA( szTempFileName );
					}	}	}	}
					free( buffer );
				}
				CloseHandle( hFile );
	}	}	}

	if ( text != NULL ) {
		iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_SETVCARD, JabberIqResultSetVcard );
        JabberSend( jabberThreadInfo->s, "<iq type=\"set\" id=\""JABBER_IQID"%d\"><vCard xmlns=\"vcard-temp\">%s</vCard></iq>", iqId, text );
		free( text );
}	}

static void ThemeDialogBackground( HWND hwnd ) {
	if ( IsWinVerXPPlus()) {
		static HMODULE hThemeAPI = NULL;
		if ( !hThemeAPI ) hThemeAPI = GetModuleHandleA( "uxtheme" );
		if ( hThemeAPI ) {
			HRESULT ( STDAPICALLTYPE *MyEnableThemeDialogTexture )( HWND,DWORD ) = ( HRESULT ( STDAPICALLTYPE* )( HWND,DWORD ))GetProcAddress( hThemeAPI,"EnableThemeDialogTexture" );
			if ( MyEnableThemeDialogTexture )
				MyEnableThemeDialogTexture( hwnd,0x00000002|0x00000004 ); //0x00000002|0x00000004=ETDT_ENABLETAB
		}
	}
}

extern HICON iconBigList[];
static BOOL CALLBACK JabberVcardDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	VcardTab* dat = ( VcardTab* ) GetWindowLong( hwndDlg, GWL_USERDATA );

	switch ( msg ) {
	case WM_INITDIALOG:
	{
		SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )iconBigList[2] );
		SendDlgItemMessage(hwndDlg,  // dialog box window handle
               IDC_LOGO,              // icon identifier
               STM_SETIMAGE,          // message to send
               (WPARAM) IMAGE_ICON,   // image type
               (LPARAM) iconBigList[2]); // new icon handle

		TranslateDialogDefault( hwndDlg );
		EnableWindow( GetDlgItem( hwndDlg, IDC_UPDATE ), jabberOnline );

		dat = ( VcardTab * ) malloc( sizeof( VcardTab ));
		memset( dat, 0, sizeof( VcardTab ));
		dat->pageCount = 6;
		dat->currentPage = 0;
		dat->changed = FALSE;
		dat->updateAnimFrame = 0;
		dat->animating = FALSE;
		dat->page = ( VcardPage * ) malloc( dat->pageCount * sizeof( VcardPage ));
		memset( dat->page, 0, dat->pageCount * sizeof( VcardPage ));

		HWND hwndTabs = GetDlgItem( hwndDlg, IDC_TABS );

		TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT;
		// Page 0: Personal
		dat->page[0].dlgId = IDD_VCARD_PERSONAL;
		dat->page[0].dlgProc = PersonalDlgProc;
		tci.pszText = TranslateT( "Personal" );
		TabCtrl_InsertItem( hwndTabs, 0, &tci );
		// Page 1: Contacts
		dat->page[1].dlgId = IDD_VCARD_CONTACT;
		dat->page[1].dlgProc = ContactDlgProc;
		tci.pszText = TranslateT( "Contacts" );
		TabCtrl_InsertItem( hwndTabs, 1, &tci );
		// Page 2: Home
		dat->page[2].dlgId = IDD_VCARD_HOME;
		dat->page[2].dlgProc = HomeDlgProc;
		tci.pszText = TranslateT( "Home" );
		TabCtrl_InsertItem( hwndTabs, 2, &tci );
		// Page 3: Work
		dat->page[3].dlgId = IDD_VCARD_WORK;
		dat->page[3].dlgProc = WorkDlgProc;
		tci.pszText = TranslateT( "Work" );
		TabCtrl_InsertItem( hwndTabs, 3, &tci );
		// Page 4: Photo
		dat->page[4].dlgId = IDD_VCARD_PHOTO;
		dat->page[4].dlgProc = PhotoDlgProc;
		tci.pszText = TranslateT( "Photo" );
		TabCtrl_InsertItem( hwndTabs, 4, &tci );
		// Page 5: Note
		dat->page[5].dlgId = IDD_VCARD_NOTE;
		dat->page[5].dlgProc = NoteDlgProc;
		tci.pszText = TranslateT( "Note" );
		TabCtrl_InsertItem( hwndTabs, 5, &tci );

		GetWindowRect( hwndTabs, &( dat->rectTab ));
		TabCtrl_AdjustRect( hwndTabs, FALSE, &( dat->rectTab ));
		{	POINT pt = {0,0};
			ClientToScreen( hwndDlg, &pt );
			OffsetRect( &( dat->rectTab ), -pt.x, -pt.y );
		}

		TabCtrl_SetCurSel( hwndTabs, dat->currentPage );
		dat->page[dat->currentPage].hwnd = CreateDialogParam( hInst, MAKEINTRESOURCE( dat->page[dat->currentPage].dlgId ), hwndDlg, dat->page[dat->currentPage].dlgProc, 0 );
		ThemeDialogBackground( dat->page[dat->currentPage].hwnd );
		SetWindowPos( dat->page[dat->currentPage].hwnd, HWND_TOP, dat->rectTab.left, dat->rectTab.top, 0, 0, SWP_NOSIZE );
		ShowWindow( dat->page[dat->currentPage].hwnd, SW_SHOW );

		SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) dat );

		bPhotoChanged = FALSE;
		szPhotoFileName[0] = '\0';

		if ( jabberOnline ) SendMessage( hwndDlg, WM_COMMAND, IDC_UPDATE, 0 );
		return TRUE;
	}
	case WM_CTLCOLORSTATIC:
		switch (GetDlgCtrlID((HWND)lParam)) {
		case IDC_WHITERECT:
		case IDC_LOGO:
		case IDC_NAME:
		case IDC_DESCRIPTION:
			SetBkColor(( HDC ) wParam, RGB( 255, 255, 255 ));
			return ( BOOL ) GetStockObject( WHITE_BRUSH );

		case IDC_UPDATING:
			SetBkColor(( HDC )wParam, GetSysColor( COLOR_3DFACE ));
			return ( BOOL ) GetSysColorBrush( COLOR_3DFACE );
		}
		break;

	case WM_NOTIFY:
		switch ( wParam ) {
		case IDC_TABS:
			switch (( ( LPNMHDR ) lParam )->code ) {
			case TCN_SELCHANGE:
				if ( dat->currentPage>=0 && dat->page[dat->currentPage].hwnd!=NULL )
					ShowWindow( dat->page[dat->currentPage].hwnd, SW_HIDE );
				dat->currentPage = TabCtrl_GetCurSel( GetDlgItem( hwndDlg, IDC_TABS ));
				if ( dat->currentPage >= 0 ) {
					if ( dat->page[dat->currentPage].hwnd == NULL ) {
						dat->page[dat->currentPage].hwnd = CreateDialogParam( hInst, MAKEINTRESOURCE( dat->page[dat->currentPage].dlgId ), hwndDlg, dat->page[dat->currentPage].dlgProc, 0 );
						ThemeDialogBackground( dat->page[dat->currentPage].hwnd );
						SetWindowPos( dat->page[dat->currentPage].hwnd, HWND_TOP, dat->rectTab.left, dat->rectTab.top, 0, 0, SWP_NOSIZE );
					}
					ShowWindow( dat->page[dat->currentPage].hwnd, SW_SHOW );
				}
				break;
			}
			break;
		}
		break;
	case WM_JABBER_CHANGED:
		dat->changed = TRUE;
		EnableWindow( GetDlgItem( hwndDlg, IDC_SAVE ), jabberOnline );
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_UPDATE:
			EnableWindow( GetDlgItem( hwndDlg,IDC_UPDATE ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg,IDC_SAVE ), FALSE );
			dat->szUpdating = TranslateT( "Updating" );
			SetDlgItemText( hwndDlg, IDC_UPDATING, dat->szUpdating );
			ShowWindow( GetDlgItem( hwndDlg, IDC_UPDATING ), SW_SHOW );
			JabberSendGetVcard( jabberJID );
			dat->animating = TRUE;
			SetTimer( hwndDlg, 1, 200, NULL );
			break;
		case IDC_SAVE:
			EnableWindow( GetDlgItem( hwndDlg,IDC_UPDATE ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg,IDC_SAVE ), FALSE );
			dat->szUpdating = TranslateT( "Saving" );
			SetDlgItemText( hwndDlg, IDC_UPDATING, dat->szUpdating );
			ShowWindow( GetDlgItem( hwndDlg, IDC_UPDATING ), SW_SHOW );
			dat->animating = TRUE;
			SetTimer( hwndDlg, 1, 200, NULL );
			SaveVcardToDB( dat );
			SetServerVcard();
			break;
		case IDCLOSE:
			DestroyWindow( hwndDlg );
			break;
		}
		break;
	case WM_TIMER:
		{	TCHAR str[128];
			mir_sntprintf( str, SIZEOF(str), _T("%.*s%s%.*s"), dat->updateAnimFrame%5, _T("...."), dat->szUpdating, dat->updateAnimFrame%5, _T("...."));
			SetDlgItemText( hwndDlg, IDC_UPDATING, str );
			if (( ++dat->updateAnimFrame ) >= 5 ) dat->updateAnimFrame = 0;
		}
		break;
	case WM_JABBER_CHECK_ONLINE:
		EnableWindow( GetDlgItem( hwndDlg, IDC_UPDATE ), jabberOnline );
		if ( dat->changed )
			EnableWindow( GetDlgItem( hwndDlg, IDC_SAVE ), jabberOnline );
		break;
	case WM_JABBER_REFRESH:
		if ( dat->animating ) {
			KillTimer( hwndDlg, 1 );
			dat->animating = FALSE;
			ShowWindow( GetDlgItem( hwndDlg, IDC_UPDATING ), FALSE );
		}
		{	for ( int i=0; i<dat->pageCount; i++ )
				if ( dat->page[i].hwnd != NULL )
					SendMessage( dat->page[i].hwnd, WM_JABBER_REFRESH, 0, 0 );
		}
		dat->changed = FALSE;
		EnableWindow( GetDlgItem( hwndDlg, IDC_UPDATE ), TRUE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_SAVE ), FALSE );
		break;
	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;
	case WM_DESTROY:
		hwndJabberVcard = NULL;
		for ( int i=0; i<dat->pageCount; i++ )
			if ( dat->page[i].hwnd != NULL )
				DestroyWindow( dat->page[i].hwnd );

		if ( dat->page ) free( dat->page );
		if ( dat ) free( dat );
		break;
	}
	return FALSE;
}
