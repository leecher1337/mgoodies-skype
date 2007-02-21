/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-07  George Hazan

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
#include "resource.h"

static BOOL CALLBACK JabberFormMultiLineWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	//case WM_GETDLGCODE:
	//	return DLGC_WANTARROWS|DLGC_WANTCHARS|DLGC_HASSETSEL|DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		if ( wParam == VK_TAB ) {
			SetFocus( GetNextDlgTabItem( GetParent( GetParent( hwnd )), hwnd, GetKeyState( VK_SHIFT )<0?TRUE:FALSE ));
			return TRUE;
		};
		break;
	}
	return CallWindowProc(( WNDPROC ) GetWindowLong( hwnd, GWL_USERDATA ), hwnd, msg, wParam, lParam );
}

void JabberFormCreateUI( HWND hwndStatic, XmlNode *xNode, int *formHeight )
{
	HWND hFrame, hCtrl;
	HFONT hFont;
	XmlNode *n, *v, *o, *vs;
	int id, i, j, k, ypos, index, size;
	TCHAR* label, *type, *labelStr, *valueStr, *varStr, *str, *p, *valueText;
	int labelOffset, labelWidth, labelHeight;
	int ctrlOffset, ctrlWidth;
	RECT frameRect, strRect;

	if ( xNode==NULL || xNode->name==NULL || strcmp( xNode->name, "x" ) || hwndStatic==NULL ) return;
	hFrame = hwndStatic;
	hFont = ( HFONT ) SendMessage( hFrame, WM_GETFONT, 0, 0 );

	GetClientRect( hwndStatic, &frameRect );
	labelOffset = 10;
	labelWidth = ( frameRect.right - frameRect.left )/2 - 20 - 20;
	ctrlOffset = labelWidth + 20;
	ctrlWidth = frameRect.right - frameRect.left - labelWidth - 20 - 20;

	id = 0;
	ypos = 14;
	for ( i=0; i<xNode->numChild; i++ ) {
		n = xNode->child[i];
		if ( n->name ) {
			if ( !strcmp( n->name, "field" )) {
				if (( varStr=JabberXmlGetAttrValue( n, "var" )) != NULL &&
					( type=JabberXmlGetAttrValue( n, "type" )) != NULL ) {

					if (( label=JabberXmlGetAttrValue( n, "label" )) != NULL )
						labelStr = mir_tstrdup( label );
					else
						labelStr = mir_tstrdup( varStr );
					strRect.top = strRect.left = 0; strRect.right = labelWidth; strRect.bottom = 1;
					labelHeight = DrawText( GetDC( hFrame ), labelStr, -1, &strRect, DT_CALCRECT|DT_RIGHT|DT_WORDBREAK );
					//labelHeight = labelHeight * 6 / 7;
					if ( labelHeight < 18 ) labelHeight = 18;

					if (( v=JabberXmlGetChild( n, "value" )) != NULL ) {
						valueStr = mir_tstrdup( v->text );
						valueText = v->text;
					}
					else valueStr = valueText = NULL;

					if ( !_tcscmp( type, _T("text-private"))) {
						if ( labelStr ) {
							hCtrl = CreateWindow( _T("static"), labelStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						}
						hCtrl = CreateWindowEx( WS_EX_CLIENTEDGE, _T("edit"), valueStr, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|ES_LEFT|ES_AUTOHSCROLL|ES_PASSWORD, ctrlOffset, ypos, ctrlWidth, 24, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						id++;
						ypos += (( labelHeight>24 )?labelHeight:24 );
					}
					else if ( !_tcscmp( type, _T("text-multi")) || !_tcscmp( type, _T("jid-multi"))) {
						WNDPROC oldWndProc;

						if ( labelStr ) {
							hCtrl = CreateWindow( _T("static"), labelStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						}
						size = 1;
						for ( j=0; j<n->numChild; j++ ) {
							v = n->child[j];
							if ( v->name && !strcmp( v->name, "value" ) && v->text )
								size += _tcslen( v->text ) + 2;
						}
						str = ( TCHAR* )mir_alloc( sizeof(TCHAR)*size );
						str[0] = '\0';
						for ( j=0; j<n->numChild; j++ ) {
							v = n->child[j];
							if ( v->name && !strcmp( v->name, "value" ) && v->text ) {
								if ( str[0] )	_tcscat( str, _T("\r\n"));
								_tcscat( str, v->text );
						}	}

						hCtrl = CreateWindowEx( WS_EX_CLIENTEDGE, _T("edit"), str, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|WS_VSCROLL|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN, ctrlOffset, ypos, ctrlWidth, 24*3, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						oldWndProc = ( WNDPROC ) SetWindowLong( hCtrl, GWL_WNDPROC, ( LPARAM )JabberFormMultiLineWndProc );
						SetWindowLong( hCtrl, GWL_USERDATA, ( LONG ) oldWndProc );
						id++;
						ypos += (( labelHeight>24*3 )?labelHeight:24*3 );
						mir_free( str );
					}
					else if ( !_tcscmp( type, _T("boolean"))) {
						strRect.top = strRect.left = 0;
						strRect.right = ctrlWidth-20; strRect.bottom = 1;
						labelHeight = DrawText( GetDC( hFrame ), labelStr, -1, &strRect, DT_CALCRECT|DT_WORDBREAK );
						if ( labelHeight < 24 ) labelHeight = 24;
						hCtrl = CreateWindowEx( 0, _T("button"), labelStr, WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX|BS_MULTILINE, ctrlOffset, ypos, ctrlWidth, ( labelHeight>24 )?labelHeight:24, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						if ( valueStr && !_tcscmp( valueStr, _T("1")))
							SendMessage( hCtrl, BM_SETCHECK, 1, 0 );
						id++;
						ypos += (( labelHeight>24 )?labelHeight:24 );
					}
					else if ( !_tcscmp( type, _T("list-single"))) {
						if ( labelStr ) {
							hCtrl = CreateWindow( _T("static"), labelStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						}
						hCtrl = CreateWindowExA( WS_EX_CLIENTEDGE, "combobox", NULL, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|CBS_DROPDOWNLIST, ctrlOffset, ypos, ctrlWidth, 24*4, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						for ( j=0; j<n->numChild; j++ ) {
							o = n->child[j];
							if ( o->name && !strcmp( o->name, "option" )) {
								if (( v=JabberXmlGetChild( o, "value" ))!=NULL && v->text ) {
									if (( str=JabberXmlGetAttrValue( o, "label" )) == NULL )
										str = v->text;
									if (( p = mir_tstrdup( str )) != NULL ) {
										index = SendMessage( hCtrl, CB_ADDSTRING, 0, ( LPARAM )p );
										mir_free( p );
										if ( valueText!=NULL && !_tcscmp( valueText, v->text )) {
											SendMessage( hCtrl, CB_SETCURSEL, index, 0 );
						}	}	}	}	}
						id++;
						ypos += (( labelHeight>24 )?labelHeight:24 );
					}
					else if ( !_tcscmp( type, _T("list-multi"))) {
						if ( labelStr ) {
							hCtrl = CreateWindow( _T("static"), labelStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						}
						hCtrl = CreateWindowExA( WS_EX_CLIENTEDGE, "listbox", NULL, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|LBS_MULTIPLESEL, ctrlOffset, ypos, ctrlWidth, 24*3, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						for ( j=0; j<n->numChild; j++ ) {
							o = n->child[j];
							if ( o->name && !strcmp( o->name, "option" )) {
								if (( v = JabberXmlGetChild( o, "value" ))!=NULL && v->text ) {
									if (( str=JabberXmlGetAttrValue( o, "label" )) == NULL )
										str = v->text;
									if (( p = mir_tstrdup( str )) != NULL ) {
										index = SendMessage( hCtrl, LB_ADDSTRING, 0, ( LPARAM )p );
										mir_free( p );
										for ( k=0; k<n->numChild; k++ ) {
											vs = n->child[k];
											if ( vs->name && !strcmp( vs->name, "value" ) && vs->text && !_tcscmp( vs->text, v->text ))
												SendMessage( hCtrl, LB_SETSEL, TRUE, index );
						}	}	}	}	}
						id++;
						ypos += (( labelHeight>24*3 )?labelHeight:24*3 );
					}
					else if ( !_tcscmp( type, _T("fixed"))) {
						if ( valueStr ) {
							strRect.top = strRect.left = 0;
							strRect.right = ctrlWidth; strRect.bottom = 1;
							labelHeight = DrawText( GetDC( hFrame ), labelStr, -1, &strRect, DT_CALCRECT|DT_WORDBREAK );
							labelHeight = labelHeight * 6 / 7;
							if ( labelHeight < 24 ) labelHeight = 24;
							hCtrl = CreateWindow( _T("static"), valueStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth+ctrlWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
							ypos += labelHeight;
						}
					}
					else if ( !_tcscmp( type, _T("hidden"))) {
						// skip
					}
					else { // everything else is considered "text-single"
						if ( labelStr ) {
							hCtrl = CreateWindow( _T("static"), labelStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						}
						hCtrl = CreateWindowEx( WS_EX_CLIENTEDGE, _T("edit"), valueStr, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|ES_LEFT|ES_AUTOHSCROLL, ctrlOffset, ypos, ctrlWidth, 24, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						id++;
						ypos += (( labelHeight>24 )?labelHeight:24 );
					}
					mir_free( labelStr );
					if ( valueStr ) mir_free( valueStr );
	}	}	}	}

	*formHeight = ypos;
}

XmlNode* JabberFormGetData( HWND hwndStatic, XmlNode* xNode )
{
	HWND hFrame, hCtrl;
	XmlNode *n, *v, *o, *x;
	int id, j, k, len;
	TCHAR *varName, *type, *fieldStr, *str, *str2, *p, *q, *labelText;

	if ( xNode==NULL || xNode->name==NULL || strcmp( xNode->name, "x" ) || hwndStatic==NULL )
		return NULL;

	hFrame = hwndStatic;
	id = 0;
	x = new XmlNode( "x" ); x->addAttr( "xmlns", "jabber:x:data" ); x->addAttr( "type", "submit" );
	for ( int i=0; i<xNode->numChild; i++ ) {
		n = xNode->child[i];
		fieldStr = NULL;
		if ( lstrcmpA( n->name, "field" ))
			continue;

		if (( varName=JabberXmlGetAttrValue( n, "var" )) == NULL || ( type=JabberXmlGetAttrValue( n, "type" )) == NULL )
			continue;

		hCtrl = GetDlgItem( hFrame, id );
		XmlNode* field = x->addChild( "field" ); field->addAttr( "var", varName );

		if ( !_tcscmp( type, _T("text-multi")) || !_tcscmp( type, _T("jid-multi"))) {
			len = GetWindowTextLength( GetDlgItem( hFrame, id ));
			str = ( TCHAR* )mir_alloc( sizeof(TCHAR)*( len+1 ));
			GetDlgItemText( hFrame, id, str, len+1 );
			p = str;
			while ( p != NULL ) {
				if (( q = _tcsstr( p, _T("\r\n"))) != NULL )
					*q = '\0';
				field->addChild( "value", p );
				p = q ? q+2 : NULL;
			}
			mir_free( str );
			id++;
		}
		else if ( !_tcscmp( type, _T("boolean"))) {
			TCHAR buf[ 10 ];
			_itot( IsDlgButtonChecked( hFrame, id ) == BST_CHECKED ? 1 : 0, buf, 10 );
			field->addChild( "value", buf );
			id++;
		}
		else if ( !_tcscmp( type, _T("list-single"))) {
			len = GetWindowTextLength( GetDlgItem( hFrame, id ));
			str = ( TCHAR* )mir_alloc( sizeof( TCHAR )*( len+1 ));
			GetDlgItemText( hFrame, id, str, len+1 );
			for ( j=0; j < n->numChild; j++ ) {
				o = n->child[j];
				if ( o && o->name && !strcmp( o->name, "option" )) {
					if (( v=JabberXmlGetChild( o, "value" ))!=NULL && v->text ) {
						if (( str2=JabberXmlGetAttrValue( o, "label" )) == NULL )
							str2 = v->text;
						if ( !lstrcmp( str2, str ))
							break;
			}	}	}

			if ( j < n->numChild )
				field->addChild( "value", v->text );

			mir_free( str );
			id++;
		}
		else if ( !_tcscmp( type, _T("list-multi"))) {
			int count = SendMessage( hCtrl, LB_GETCOUNT, 0, 0 );
			for ( j=0; j<count; j++ ) {
				if ( SendMessage( hCtrl, LB_GETSEL, j, 0 ) > 0 ) {
					// an entry is selected
					len = SendMessage( hCtrl, LB_GETTEXTLEN, 0, 0 );
					if (( str = ( TCHAR* )mir_alloc(( len+1 )*sizeof( TCHAR ))) != NULL ) {
						SendMessage( hCtrl, LB_GETTEXT, j, ( LPARAM )str );
						for ( k=0; k < n->numChild; k++ ) {
							o = n->child[k];
							if ( o && o->name && !strcmp( o->name, "option" )) {
								if (( v=JabberXmlGetChild( o, "value" ))!=NULL && v->text ) {
									if (( labelText=JabberXmlGetAttrValue( o, "label" )) == NULL )
										labelText = v->text;

									if ( !lstrcmp( labelText, str ))
										field->addChild( "value", v->text );
						}	}	}
						mir_free( str );
			}	}	}
			id++;
		}
		else if ( !_tcscmp( type, _T("fixed")) || !_tcscmp( type, _T("hidden"))) {
			v = JabberXmlGetChild( n, "value" );
			if ( v != NULL && v->text != NULL )
				field->addChild( "value", v->text );
		}
		else { // everything else is considered "text-single" or "text-private"
			len = GetWindowTextLength( GetDlgItem( hFrame, id ));
			str = ( TCHAR* )mir_alloc( sizeof(TCHAR)*( len+1 ));
			GetDlgItemText( hFrame, id, str, len+1 );
			field->addChild( "value", str );
			mir_free( str );
			id++;
	}	}

	return x;
}

typedef struct {
	XmlNode *xNode;
	TCHAR defTitle[128];	// Default title if no <title/> in xNode
	RECT frameRect;		// Clipping region of the frame to scroll
	int frameHeight;	// Height of the frame ( can be eliminated, redundant to frameRect )
	int formHeight;		// Actual height of the form
	int curPos;			// Current scroll position
	JABBER_FORM_SUBMIT_FUNC pfnSubmit;
	void *userdata;
} JABBER_FORM_INFO;

static BOOL CALLBACK JabberFormDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	JABBER_FORM_INFO *jfi;

	switch ( msg ) {
	case WM_INITDIALOG:
		{
			XmlNode *n;
			LONG frameExStyle;

			// lParam is ( JABBER_FORM_INFO * )
			TranslateDialogDefault( hwndDlg );
			ShowWindow( GetDlgItem( hwndDlg, IDC_FRAME_TEXT ), SW_HIDE );
			jfi = ( JABBER_FORM_INFO * ) lParam;
			if ( jfi != NULL ) {
				// Set dialog title
				if ( jfi->xNode!=NULL && ( n=JabberXmlGetChild( jfi->xNode, "title" ))!=NULL && n->text!=NULL )
					SetWindowText( hwndDlg, n->text );
				else if ( jfi->defTitle != NULL )
					SetWindowText( hwndDlg, TranslateTS( jfi->defTitle ));
				// Set instruction field
				if ( jfi->xNode!=NULL && ( n=JabberXmlGetChild( jfi->xNode, "instructions" ))!=NULL && n->text!=NULL )
					SetDlgItemText( hwndDlg, IDC_INSTRUCTION, n->text );

				// Create form
				if ( jfi->xNode != NULL ) {
					RECT rect;

					GetClientRect( GetDlgItem( hwndDlg, IDC_FRAME ), &( jfi->frameRect ));
					GetClientRect( GetDlgItem( hwndDlg, IDC_VSCROLL ), &rect );
					jfi->frameRect.right -= ( rect.right - rect.left );
					GetClientRect( GetDlgItem( hwndDlg, IDC_FRAME ), &rect );
					jfi->frameHeight = rect.bottom - rect.top;
					JabberFormCreateUI( GetDlgItem( hwndDlg, IDC_FRAME ), jfi->xNode, &( jfi->formHeight ));
				}
			}

			if ( jfi->formHeight > jfi->frameHeight ) {
				HWND hwndScroll;

				hwndScroll = GetDlgItem( hwndDlg, IDC_VSCROLL );
				EnableWindow( hwndScroll, TRUE );
				SetScrollRange( hwndScroll, SB_CTL, 0, jfi->formHeight - jfi->frameHeight, FALSE );
				jfi->curPos = 0;
			}

			// Enable WS_EX_CONTROLPARENT on IDC_FRAME ( so tab stop goes through all its children )
			frameExStyle = GetWindowLong( GetDlgItem( hwndDlg, IDC_FRAME ), GWL_EXSTYLE );
			frameExStyle |= WS_EX_CONTROLPARENT;
			SetWindowLong( GetDlgItem( hwndDlg, IDC_FRAME ), GWL_EXSTYLE, frameExStyle );

			SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) jfi );
			if ( jfi->pfnSubmit != NULL )
				EnableWindow( GetDlgItem( hwndDlg, IDC_SUBMIT ), TRUE );
		}
		return TRUE;
	case WM_VSCROLL:
		{
			int pos;

			jfi = ( JABBER_FORM_INFO * ) GetWindowLong( hwndDlg, GWL_USERDATA );
			if ( jfi != NULL ) {
				pos = jfi->curPos;
				switch ( LOWORD( wParam )) {
				case SB_LINEDOWN:
					pos += 10;
					break;
				case SB_LINEUP:
					pos -= 10;
					break;
				case SB_PAGEDOWN:
					pos += ( jfi->frameHeight - 10 );
					break;
				case SB_PAGEUP:
					pos -= ( jfi->frameHeight - 10 );
					break;
				case SB_THUMBTRACK:
					pos = HIWORD( wParam );
					break;
				}
				if ( pos > ( jfi->formHeight - jfi->frameHeight ))
					pos = jfi->formHeight - jfi->frameHeight;
				if ( pos < 0 )
					pos = 0;
				if ( jfi->curPos != pos ) {
					ScrollWindow( GetDlgItem( hwndDlg, IDC_FRAME ), 0, jfi->curPos - pos, NULL, &( jfi->frameRect ));
					SetScrollPos( GetDlgItem( hwndDlg, IDC_VSCROLL ), SB_CTL, pos, TRUE );
					jfi->curPos = pos;
				}
			}
		}
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_SUBMIT:
			jfi = ( JABBER_FORM_INFO * ) GetWindowLong( hwndDlg, GWL_USERDATA );
			if ( jfi != NULL ) {
				XmlNode* n = JabberFormGetData( GetDlgItem( hwndDlg, IDC_FRAME ), jfi->xNode );
				( jfi->pfnSubmit )( n, jfi->userdata );
			}
			// fall through
		case IDCANCEL:
			DestroyWindow( hwndDlg );
			return TRUE;
		}
		break;
	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;
	case WM_DESTROY:
		jfi = ( JABBER_FORM_INFO * ) GetWindowLong( hwndDlg, GWL_USERDATA );
		if ( jfi != NULL ) {
			if ( jfi->xNode != NULL )
				delete jfi->xNode;
			mir_free( jfi );
		}
		break;
	}

	return FALSE;
}

static VOID CALLBACK JabberFormCreateDialogApcProc( DWORD param )
{
	CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_FORM ), NULL, JabberFormDlgProc, ( LPARAM )param );
}

void JabberFormCreateDialog( XmlNode *xNode, TCHAR* defTitle, JABBER_FORM_SUBMIT_FUNC pfnSubmit, void *userdata )
{
	JABBER_FORM_INFO *jfi;

	jfi = ( JABBER_FORM_INFO * ) mir_alloc( sizeof( JABBER_FORM_INFO ));
	memset( jfi, 0, sizeof( JABBER_FORM_INFO ));
	jfi->xNode = JabberXmlCopyNode( xNode );
	if ( defTitle )
		_tcsncpy( jfi->defTitle, defTitle, SIZEOF( jfi->defTitle ));
	jfi->pfnSubmit = pfnSubmit;
	jfi->userdata = userdata;

	if ( GetCurrentThreadId() != jabberMainThreadId )
		QueueUserAPC( JabberFormCreateDialogApcProc, hMainThread, ( DWORD )jfi );
	else
		JabberFormCreateDialogApcProc(( DWORD )jfi );
}
