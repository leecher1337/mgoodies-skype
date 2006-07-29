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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_password.cpp,v $
Revision       : $Revision: 1.9 $
Last change on : $Date: 2006/05/12 20:13:35 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_iq.h"
#include "resource.h"

static BOOL CALLBACK JabberChangePasswordDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );

int JabberMenuHandleChangePassword( WPARAM wParam, LPARAM lParam )
{
	if ( IsWindow( hwndJabberChangePassword ))
		SetForegroundWindow( hwndJabberChangePassword );
	else {
		hwndJabberChangePassword = CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_CHANGEPASSWORD ), NULL, JabberChangePasswordDlgProc, 0 );
	}

	return 0;
}

static BOOL CALLBACK JabberChangePasswordDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )iconBigList[1] );
		TranslateDialogDefault( hwndDlg );
		if ( jabberOnline && jabberThreadInfo!=NULL ) {
			TCHAR text[128];
			mir_sntprintf( text, SIZEOF( text ), _T("%s %s@") _T(TCHAR_STR_PARAM), TranslateT( "Set New Password for" ), jabberThreadInfo->username, jabberThreadInfo->server );
			SetWindowText( hwndDlg, text );
		}
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
			if ( jabberOnline && jabberThreadInfo!=NULL ) {
				char newPasswd[128], text[128];
				GetDlgItemTextA( hwndDlg, IDC_NEWPASSWD, newPasswd, SIZEOF( newPasswd ));
				GetDlgItemTextA( hwndDlg, IDC_NEWPASSWD2, text, SIZEOF( text ));
				if ( strcmp( newPasswd, text )) {
					MessageBox( hwndDlg, TranslateT( "New password does not match." ), TranslateT( "Change Password" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
					break;
				}
				GetDlgItemTextA( hwndDlg, IDC_OLDPASSWD, text, SIZEOF( text ));
				if ( strcmp( text, jabberThreadInfo->password )) {
					MessageBox( hwndDlg, TranslateT( "Current password is incorrect." ), TranslateT( "Change Password" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
					break;
				}
				strncpy( jabberThreadInfo->newPassword, newPasswd, SIZEOF( jabberThreadInfo->newPassword ));

				int iqId = JabberSerialNext();
				JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultSetPassword );

				XmlNodeIq iq( "set", iqId, jabberThreadInfo->server );
				XmlNode* q = iq.addQuery( "jabber:iq:register" );
				q->addChild( "username", jabberThreadInfo->username );
				q->addChild( "password", newPasswd );
				JabberSend( jabberThreadInfo->s, iq );
			}
			DestroyWindow( hwndDlg );
			break;
		case IDCANCEL:
			DestroyWindow( hwndDlg );
			break;
		}
		break;
	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;
	case WM_DESTROY:
		hwndJabberChangePassword = NULL;
		break;
	}

	return FALSE;
}
