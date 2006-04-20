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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_form.cpp,v $
Revision       : $Revision: 1.7 $
Last change on : $Date: 2006/01/22 20:05:00 $
Last change by : $Author: ghazan $

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
	char* label, *type, *labelStr, *valueStr, *varStr, *str, *p, *valueText;
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
						labelStr = JabberTextDecode( label );
					else
						labelStr = JabberTextDecode( varStr );
					strRect.top = strRect.left = 0; strRect.right = labelWidth; strRect.bottom = 1;
					labelHeight = DrawTextA( GetDC( hFrame ), labelStr, -1, &strRect, DT_CALCRECT|DT_RIGHT|DT_WORDBREAK );
					//labelHeight = labelHeight * 6 / 7;
					if ( labelHeight < 18 ) labelHeight = 18;

					if (( v=JabberXmlGetChild( n, "value" )) != NULL ) {
						valueStr = JabberTextDecode( v->text );
						valueText = v->text;
					}
					else {
						valueStr = valueText = NULL;
					}

					if ( !strcmp( type, "text-private" )) {
						if ( labelStr ) {
							hCtrl = CreateWindowA( "static", labelStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						}
						hCtrl = CreateWindowExA( WS_EX_CLIENTEDGE, "edit", valueStr, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|ES_LEFT|ES_AUTOHSCROLL|ES_PASSWORD, ctrlOffset, ypos, ctrlWidth, 24, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						id++;
						ypos += (( labelHeight>24 )?labelHeight:24 );
					}
					else if ( !strcmp( type, "text-multi" ) || !strcmp( type, "jid-multi" )) {
						WNDPROC oldWndProc;

						if ( labelStr ) {
							hCtrl = CreateWindowA( "static", labelStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						}
						size = 1;
						for ( j=0; j<n->numChild; j++ ) {
							v = n->child[j];
							if ( v->name && !strcmp( v->name, "value" ) && v->text )
								size += strlen( v->text ) + 2;
						}
						str = ( char* )malloc( size );
						str[0] = '\0';
						for ( j=0; j<n->numChild; j++ ) {
							v = n->child[j];
							if ( v->name && !strcmp( v->name, "value" ) && v->text ) {
								if ( str[0] )	strcat( str, "\r\n" );
								strcat( str, v->text );
							}
						}
						hCtrl = CreateWindowExA( WS_EX_CLIENTEDGE, "edit", str, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|WS_VSCROLL|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN, ctrlOffset, ypos, ctrlWidth, 24*3, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						oldWndProc = ( WNDPROC ) SetWindowLong( hCtrl, GWL_WNDPROC, ( LPARAM )JabberFormMultiLineWndProc );
						SetWindowLong( hCtrl, GWL_USERDATA, ( LONG ) oldWndProc );
						id++;
						ypos += (( labelHeight>24*3 )?labelHeight:24*3 );
						free( str );
					}
					else if ( !strcmp( type, "boolean" )) {
						strRect.top = strRect.left = 0;
						strRect.right = ctrlWidth-20; strRect.bottom = 1;
						labelHeight = DrawTextA( GetDC( hFrame ), labelStr, -1, &strRect, DT_CALCRECT|DT_WORDBREAK );
						//labelHeight = labelHeight * 6 / 7;
						if ( labelHeight < 24 ) labelHeight = 24;
						hCtrl = CreateWindowExA( 0, "button", labelStr, WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX|BS_MULTILINE, ctrlOffset, ypos, ctrlWidth, ( labelHeight>24 )?labelHeight:24, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						if ( valueStr && !strcmp( valueStr, "1" ))
							SendMessage( hCtrl, BM_SETCHECK, 1, 0 );
						id++;
						ypos += (( labelHeight>24 )?labelHeight:24 );
					}
					else if ( !strcmp( type, "list-single" )) {
						if ( labelStr ) {
							hCtrl = CreateWindowA( "static", labelStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
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
									if (( p=JabberTextDecode( str )) != NULL ) {
										index = SendMessageA( hCtrl, CB_ADDSTRING, 0, ( LPARAM )p );
										free( p );
										if ( valueText!=NULL && !strcmp( valueText, v->text )) {
											SendMessage( hCtrl, CB_SETCURSEL, index, 0 );
										}
									}
								}
							}
						}
						id++;
						ypos += (( labelHeight>24 )?labelHeight:24 );
					}
					else if ( !strcmp( type, "list-multi" )) {
						if ( labelStr ) {
							hCtrl = CreateWindowA( "static", labelStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						}
						hCtrl = CreateWindowExA( WS_EX_CLIENTEDGE, "listbox", NULL, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|LBS_MULTIPLESEL, ctrlOffset, ypos, ctrlWidth, 24*3, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						for ( j=0; j<n->numChild; j++ ) {
							o = n->child[j];
							if ( o->name && !strcmp( o->name, "option" )) {
								if (( v=JabberXmlGetChild( o, "value" ))!=NULL && v->text ) {
									if (( str=JabberXmlGetAttrValue( o, "label" )) == NULL )
										str = v->text;
									if (( p=JabberTextDecode( str )) != NULL ) {
										index = SendMessage( hCtrl, LB_ADDSTRING, 0, ( LPARAM )p );
										free( p );
										for ( k=0; k<n->numChild; k++ ) {
											vs = n->child[k];
											if ( vs->name && !strcmp( vs->name, "value" ) && vs->text && !strcmp( vs->text, v->text ))
												SendMessage( hCtrl, LB_SETSEL, TRUE, index );
										}
									}
								}
							}
						}
						id++;
						ypos += (( labelHeight>24*3 )?labelHeight:24*3 );
					}
					else if ( !strcmp( type, "fixed" )) {
						if ( valueStr ) {
							strRect.top = strRect.left = 0;
							strRect.right = ctrlWidth; strRect.bottom = 1;
							labelHeight = DrawTextA( GetDC( hFrame ), labelStr, -1, &strRect, DT_CALCRECT|DT_WORDBREAK );
							labelHeight = labelHeight * 6 / 7;
							if ( labelHeight < 24 ) labelHeight = 24;
							hCtrl = CreateWindowA( "static", valueStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth+ctrlWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
							ypos += labelHeight;
						}
					}
					else if ( !strcmp( type, "hidden" )) {
						// skip
					}
					else { // everything else is considered "text-single"
#ifdef _DEBUG
						if ( strcmp( type, "text-single" ))
							JabberLog( "Non-recognize field type='%s'", type );
#endif
						if ( labelStr ) {
							hCtrl = CreateWindowA( "static", labelStr, WS_CHILD|WS_VISIBLE|SS_RIGHT, labelOffset, ypos+4, labelWidth, labelHeight, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						}
						hCtrl = CreateWindowExA( WS_EX_CLIENTEDGE, "edit", valueStr, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|ES_LEFT|ES_AUTOHSCROLL, ctrlOffset, ypos, ctrlWidth, 24, hFrame, ( HMENU ) id, hInst, NULL );
						SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
						id++;
						ypos += (( labelHeight>24 )?labelHeight:24 );
					}
					free( labelStr );
					if ( valueStr ) free( valueStr );
				}
			}
		}
	}
	*formHeight = ypos;
}

char* JabberFormGetData( HWND hwndStatic, XmlNode *xNode )
{
	HWND hFrame, hCtrl;
	XmlNode *n, *v, *o;
	int id, i, j, k, len, count, size;
	char* varName, *type;
	char* regStr, *fieldStr, *str, *str2, *p, *q, *s, *labelStr, *labelText;
	unsigned int regStrSize;

	if ( xNode==NULL || xNode->name==NULL || strcmp( xNode->name, "x" ) || hwndStatic==NULL ) return NULL;
	hFrame = hwndStatic;

	regStrSize = 512;
	regStr = ( char* )malloc( regStrSize );
	regStr[0] = '\0';
	id = 0;
	strcpy( regStr, "<x xmlns='jabber:x:data' type='submit'>" );
	for ( i=0; i<xNode->numChild; i++ ) {
		n = xNode->child[i];
		fieldStr = NULL;
		if ( n->name && !strcmp( n->name, "field" )) {
			if (( varName=JabberXmlGetAttrValue( n, "var" )) != NULL &&
				( type=JabberXmlGetAttrValue( n, "type" )) != NULL ) {

				hCtrl = GetDlgItem( hFrame, id );

				if ( !strcmp( type, "text-multi" ) || !strcmp( type, "jid-multi" )) {
					len = GetWindowTextLength( GetDlgItem( hFrame, id ));
					str = ( char* )malloc( len+1 );
					GetDlgItemTextA( hFrame, id, str, len+1 );
					p = str; len = 0;
					while ( p != NULL ) {
						if (( q=strstr( p, "\r\n" )) != NULL )
							*q = '\0';
						s = JabberTextEncode( p );
						if ( q != NULL )
							*q = '\r';
						len += ( strlen( s ) + 15 );	// <value>...</value>
						free( s );
						p = q?q+2:NULL;
					}
					str2 = ( char* )malloc( len+1 );
					str2[0] = '\0';
					p = str;
					while ( p != NULL ) {
						if (( q=strstr( p, "\r\n" )) != NULL )
							*q = '\0';
						s = JabberTextEncode( p );
						strcat( str2, "<value>" );
						strcat( str2, s );
						strcat( str2, "</value>" );
						free( s );
						p = q?q+2:NULL;
					}
					if (( p=JabberTextEncode( varName )) != NULL ) {
						fieldStr = ( char* )malloc( strlen( p ) + strlen( str2 ) + 38 );
						sprintf( fieldStr, "<field var='%s'>%s</field>", p, str2 );
						free( p );
					}
					free( str2 );
					free( str );
					id++;
				}
				else if ( !strcmp( type, "boolean" )) {
					if (( p=JabberTextEncode( varName )) != NULL ) {
						fieldStr = ( char* )malloc( strlen( p ) + 40 );
						sprintf( fieldStr, "<field var='%s'><value>%d</value></field>", p, IsDlgButtonChecked( hFrame, id )==BST_CHECKED?1:0 );
						free( p );
					}
					id++;
				}
				else if ( !strcmp( type, "list-single" )) {
					len = GetWindowTextLength( GetDlgItem( hFrame, id ));
					str = ( char* )malloc( len+1 );
					GetDlgItemTextA( hFrame, id, str, len+1 );
					for ( j=0; j<n->numChild; j++ ) {
						o = n->child[j];
						if ( o && o->name && !strcmp( o->name, "option" )) {
							if (( v=JabberXmlGetChild( o, "value" ))!=NULL && v->text ) {
								if (( str2=JabberXmlGetAttrValue( o, "label" )) == NULL )
									str2 = v->text;
								if (( p=JabberTextEncode( str2 )) != NULL ) {
									if ( !strcmp( p, str )) {
										free( p );
										break;
									}
									free( p );
								}
							}
						}
					}
					if ( j < n->numChild ) {
						if (( p=JabberTextEncode( varName )) != NULL ) {
							if (( q=JabberTextEncode( v->text )) != NULL ) {
								fieldStr = ( char* )malloc( strlen( p ) + strlen( q ) + 38 );
								sprintf( fieldStr, "<field var='%s'><value>%s</value></field>", p, q );
								free( q );
							}
							free( p );
						}
					}
					free( str );
					id++;
				}
				else if ( !strcmp( type, "list-multi" )) {
					if (( p=JabberTextEncode( varName )) != NULL ) {
						size = strlen( p ) + 23;
						fieldStr = ( char* )malloc( size );	// <field var='xxx'></field>
						sprintf( fieldStr, "<field var='%s'>", p );
						free( p );

						count = SendMessage( hCtrl, LB_GETCOUNT, 0, 0 );
						for ( j=0; j<count; j++ ) {
							if ( SendMessage( hCtrl, LB_GETSEL, j, 0 ) > 0 ) {
								// an entry is selected
								len = SendMessage( hCtrl, LB_GETTEXTLEN, 0, 0 );
								if (( str=( char* )malloc(( len+1 )*sizeof( TCHAR )) ) != NULL ) {
									SendMessage( hCtrl, LB_GETTEXT, j, ( LPARAM )str );
									for ( k=0; k<n->numChild; k++ ) {
										o = n->child[k];
										if ( o && o->name && !strcmp( o->name, "option" )) {
											if (( v=JabberXmlGetChild( o, "value" ))!=NULL && v->text ) {
												if (( labelText=JabberXmlGetAttrValue( o, "label" )) == NULL )
													labelText = v->text;
												if (( labelStr=JabberTextEncode( labelText )) != NULL ) {
													if ( !strcmp( labelStr, str )) {
														if (( q=JabberTextEncode( v->text )) != NULL ) {
															size += strlen( q )+15;
															fieldStr = ( char* )realloc( fieldStr, size );
															sprintf( fieldStr+strlen( fieldStr ), "<value>%s</value>", q );
															free( q );
														}
													}
													free( labelStr );
												}
											}
										}
									}
									free( str );
								}
							}
						}
						sprintf( fieldStr+strlen( fieldStr ), "</field>" );
					}
					id++;
				}
				else if ( !strcmp( type, "fixed" ) || !strcmp( type, "hidden" )) {
					if (( p=JabberTextEncode( varName )) != NULL ) {
						v = JabberXmlGetChild( n, "value" );
						if ( v!=NULL && v->text!=NULL ) {
							if (( q=JabberTextEncode( v->text )) != NULL ) {
								fieldStr = ( char* )malloc( strlen( p ) + strlen( q ) + 38 );
								sprintf( fieldStr, "<field var='%s'><value>%s</value></field>", p, q );
								free( q );
							}
						}
						else {
							fieldStr = ( char* )malloc( strlen( p ) + 16 );
							sprintf( fieldStr, "<field var='%s'/>", p );
						}
						free( p );
					}
				}
				else { // everything else is considered "text-single" or "text-private"
					len = GetWindowTextLength( GetDlgItem( hFrame, id ));
					str = ( char* )malloc( len+1 );
					GetDlgItemTextA( hFrame, id, str, len+1 );
					if (( p=JabberTextEncode( varName )) != NULL ) {
						if (( q=JabberTextEncode( str )) != NULL ) {
							fieldStr = ( char* )malloc( strlen( p ) + strlen( q ) + 38 );
							sprintf( fieldStr, "<field var='%s'><value>%s</value></field>", p, q );
							free( q );
						}
						free( p );
					}
					free( str );
					id++;
				}

				if ( fieldStr != NULL ) {
					if ( strlen( regStr )+strlen( fieldStr )+5 > regStrSize ) {
						regStrSize = strlen( regStr )+strlen( fieldStr )+512;
						regStr = ( char* )realloc( regStr, regStrSize );
					}
					strcat( regStr, fieldStr );
					free( fieldStr );
				}
			}
		}
	}
	strcat( regStr, "</x>" );
	return regStr;
}

typedef struct {
	XmlNode *xNode;
	char defTitle[128];	// Default title if no <title/> in xNode
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
			char* str;

			// lParam is ( JABBER_FORM_INFO * )
			TranslateDialogDefault( hwndDlg );
			ShowWindow( GetDlgItem( hwndDlg, IDC_FRAME_TEXT ), SW_HIDE );
			jfi = ( JABBER_FORM_INFO * ) lParam;
			if ( jfi != NULL ) {
				// Set dialog title
				if ( jfi->xNode!=NULL && ( n=JabberXmlGetChild( jfi->xNode, "title" ))!=NULL && n->text!=NULL ) {
					if (( str=JabberTextDecode( n->text )) != NULL ) {
						SetWindowTextA( hwndDlg, str );
						free( str );
					}
				}
				else if ( jfi->defTitle != NULL )
					SetWindowTextA( hwndDlg, JTranslate( jfi->defTitle ));
				// Set instruction field
				if ( jfi->xNode!=NULL && ( n=JabberXmlGetChild( jfi->xNode, "instructions" ))!=NULL && n->text!=NULL ) {
					if (( str=JabberTextDecode( n->text )) != NULL ) {
						SetDlgItemTextA( hwndDlg, IDC_INSTRUCTION, str );
						free( str );
					}
				}
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
				char* str;

				str = JabberFormGetData( GetDlgItem( hwndDlg, IDC_FRAME ), jfi->xNode );
				( jfi->pfnSubmit )( str, jfi->userdata );
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
				JabberXmlFreeNode( jfi->xNode );
			free( jfi );
		}
		break;
	}

	return FALSE;
}

static VOID CALLBACK JabberFormCreateDialogApcProc( DWORD param )
{
	CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_FORM ), NULL, JabberFormDlgProc, ( LPARAM )param );
}

void JabberFormCreateDialog( XmlNode *xNode, char* defTitle, JABBER_FORM_SUBMIT_FUNC pfnSubmit, void *userdata )
{
	JABBER_FORM_INFO *jfi;

	jfi = ( JABBER_FORM_INFO * ) malloc( sizeof( JABBER_FORM_INFO ));
	memset( jfi, 0, sizeof( JABBER_FORM_INFO ));
	jfi->xNode = JabberXmlCopyNode( xNode );
	if ( defTitle )
		strncpy( jfi->defTitle, defTitle, sizeof( jfi->defTitle ));
	jfi->pfnSubmit = pfnSubmit;
	jfi->userdata = userdata;

	if ( GetCurrentThreadId() != jabberMainThreadId )
		QueueUserAPC( JabberFormCreateDialogApcProc, hMainThread, ( DWORD )jfi );
	else
		JabberFormCreateDialogApcProc(( DWORD )jfi );
}
