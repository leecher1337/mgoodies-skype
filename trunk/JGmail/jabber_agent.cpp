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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_agent.cpp,v $
Revision       : $Revision: 1.14 $
Last change on : $Date: 2006/02/01 20:15:20 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include <commctrl.h>
#include "resource.h"
#include "jabber_iq.h"

static BOOL CALLBACK JabberAgentsDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK JabberAgentRegInputDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );
static BOOL CALLBACK JabberAgentRegDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );
static BOOL CALLBACK JabberAgentManualRegDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );

int JabberMenuHandleAgents( WPARAM wParam, LPARAM lParam )
{
	if ( IsWindow( hwndJabberAgents ))
		SetForegroundWindow( hwndJabberAgents );
	else {
		CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_AGENTS ), NULL, JabberAgentsDlgProc, ( LPARAM )NULL );
	}

	return 0;
}

static void JabberRegisterAgent( HWND hwndDlg, char* jid )
{
	int iqId = JabberSerialNext();
	JabberIqAdd( iqId, IQ_PROC_GETREGISTER, JabberIqResultGetRegister );
	JabberSend( jabberThreadInfo->s, "<iq type='get' id='"JABBER_IQID"%d' to='%s'><query xmlns='jabber:iq:register'/></iq>", iqId, jid );
	hwndAgentRegInput = CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_FORM ), hwndDlg, JabberAgentRegInputDlgProc, 0 );
}

static BOOL CALLBACK JabberAgentsDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HWND lv;
	LVCOLUMN lvCol;
	LVITEMA lvItem;
	JABBER_LIST_ITEM *item;
	int i;
	char text[128];
	char* p;
	int iqId;

	switch ( msg ) {
	case WM_INITDIALOG:
		hwndJabberAgents = hwndDlg;
		SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )LoadIcon( hInst, MAKEINTRESOURCE( IDI_AGENTS )) );
		TranslateDialogDefault( hwndDlg );
		// Add columns to the top list
		lv = GetDlgItem( hwndDlg, IDC_AGENT_LIST );
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvCol.pszText = TranslateT( "JID" );
		lvCol.cx = 120;
		lvCol.iSubItem = 0;
		ListView_InsertColumn( lv, 0, &lvCol );
		lvCol.pszText = TranslateT( "Description" );
		lvCol.cx = 250;
		lvCol.iSubItem = 1;
		ListView_InsertColumn( lv, 1, &lvCol );
		// Add columns to the bottom list
		lv = GetDlgItem( hwndDlg, IDC_AGENT_TRANSPORT );
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvCol.pszText = TranslateT( "JID" );
		lvCol.cx = 120;
		lvCol.iSubItem = 0;
		ListView_InsertColumn( lv, 0, &lvCol );
		lvCol.pszText = TranslateT( "Status" );
		lvCol.cx = 80;
		lvCol.iSubItem = 1;
		ListView_InsertColumn( lv, 1, &lvCol );
		if ( jabberOnline ) {
			SetDlgItemTextA( hwndDlg, IDC_AGENT_SERVER, jabberThreadInfo->server );
			JabberListRemoveList( LIST_AGENT );
			iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_DISCOAGENTS, JabberIqResultDiscoAgentItems );
			JabberSend( jabberThreadInfo->s, "<iq type='get' id='"JABBER_IQID"%d' to='%s'><query xmlns='http://jabber.org/protocol/disco#items'/></iq>", iqId, TXT(jabberThreadInfo->server));

			SendMessage( hwndDlg, WM_JABBER_TRANSPORT_REFRESH, 0, 0 );
		}
		return TRUE;
	case WM_NOTIFY:
		switch (( ( LPNMHDR ) lParam )->code ) {
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW lpnm;

				lpnm = ( LPNMLISTVIEW ) lParam;
				if ( lpnm->hdr.idFrom == IDC_AGENT_LIST ) {
					lv = GetDlgItem( hwndDlg, IDC_AGENT_LIST );
					if ( lpnm->uChanged & LVIF_STATE ) {
						if ( lpnm->uNewState & LVIS_SELECTED ) {
							lvItem.iItem = lpnm->iItem;
							lvItem.iSubItem = 0;
							lvItem.mask = LVIF_TEXT;
							lvItem.pszText = text;
							lvItem.cchTextMax = sizeof( text );
							SendMessageA( lv, LVM_GETITEMA, 0, (LPARAM)&lvItem );
							if (( item=JabberListGetItemPtr( LIST_AGENT, lvItem.pszText )) != NULL ) {
								if ( item->cap & AGENT_CAP_REGISTER )
									EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_REGISTER ), TRUE );
								//if ( item->canSearch ) EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_SEARCH ), TRUE );
								if ( item->cap & AGENT_CAP_GROUPCHAT )
									EnableWindow( GetDlgItem( hwndDlg, IDC_JOIN ), TRUE );
							}
						}
						else {
							EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_REGISTER ), FALSE );
							//EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_SEARCH ), FALSE );
							EnableWindow( GetDlgItem( hwndDlg, IDC_JOIN ), FALSE );
						}
						return TRUE;
					}
				}
				else if ( lpnm->hdr.idFrom == IDC_AGENT_TRANSPORT ) {
					lv = GetDlgItem( hwndDlg, IDC_AGENT_TRANSPORT );
					if ( lpnm->uChanged & LVIF_STATE ) {
						EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_LOGON ), FALSE );
						EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_LOGOFF ), FALSE );
						EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_UNREGISTER ), FALSE );
						if ( lpnm->uNewState & LVIS_SELECTED ) {
							lvItem.iItem = lpnm->iItem;
							lvItem.iSubItem = 0;
							lvItem.mask = LVIF_TEXT;
							lvItem.pszText = text;
							lvItem.cchTextMax = sizeof( text );
							SendMessageA( lv, LVM_GETITEMA, 0, (LPARAM)&lvItem );
							if (( item=JabberListGetItemPtr( LIST_ROSTER, lvItem.pszText )) != NULL ) {
								if ( item->status == ID_STATUS_OFFLINE )
									EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_LOGON ), TRUE );
								else
									EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_LOGOFF ), TRUE );
								EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_UNREGISTER ), TRUE );
						}	}
						return TRUE;
			}	}	}
			break;
		}
		break;
	case WM_JABBER_AGENT_REFRESH:
		// lParam = server from which agent information is obtained
		if ( lParam )
			SetDlgItemTextA( hwndDlg, IDC_AGENT_SERVER, ( char* )lParam );
		EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_REGISTER ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_SEARCH ), FALSE );
		i = 0;
		lv = GetDlgItem( hwndDlg, IDC_AGENT_LIST );
		ListView_DeleteAllItems( lv );
		lvItem.iItem = 0;
		while (( i=JabberListFindNext( LIST_AGENT, i )) >= 0 ) {
			if (( item=JabberListGetItemPtrFromIndex( i )) != NULL ) {
				lvItem.mask = LVIF_TEXT;
				lvItem.iSubItem = 0;
				lvItem.pszText = item->jid;
				SendMessageA( lv, LVM_INSERTITEMA, 0, (LPARAM)&lvItem );
				lvItem.iSubItem = 1;
				lvItem.pszText = item->name;
				SendMessageA( lv, LVM_SETITEMA, 0, (LPARAM)&lvItem );
				lvItem.iItem++;
			}
			i++;
		}
		EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_SERVER ), TRUE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_BROWSE ), TRUE );
		return TRUE;
	case WM_JABBER_TRANSPORT_REFRESH:
		EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_LOGON ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_LOGOFF ), FALSE );
		i = 0;
		lv = GetDlgItem( hwndDlg, IDC_AGENT_TRANSPORT );
		ListView_DeleteAllItems( lv );
		lvItem.iItem = 0;
		while (( i=JabberListFindNext( LIST_ROSTER, i )) >= 0 ) {
			if (( item=JabberListGetItemPtrFromIndex( i )) != NULL ) {
				if ( strchr( item->jid, '@' )==NULL && item->subscription!=SUB_NONE ) {
					strcpy( text, item->jid );
					if (( p=strchr( text, '/' )) != NULL )
						*p = '\0';
					lvItem.mask = LVIF_TEXT;
					lvItem.iSubItem = 0;
					lvItem.pszText = text;
					SendMessageA( lv, LVM_INSERTITEMA, 0, (LPARAM)&lvItem );
					lvItem.iSubItem = 1;
					if ( item->status != ID_STATUS_OFFLINE )
						lvItem.pszText = JTranslate( "Online" );
					else
						lvItem.pszText = JTranslate( "Offline" );
					SendMessageA( lv, LVM_SETITEMA, 0, (LPARAM)&lvItem );
					lvItem.iItem++;
			}	}
			i++;
		}
		return TRUE;
	case WM_JABBER_CHECK_ONLINE:
		if ( !jabberOnline ) {
			if ( hwndAgentRegInput )
				DestroyWindow( hwndAgentRegInput );
			if ( hwndAgentManualReg )
				DestroyWindow( hwndAgentManualReg );
		}
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_MANUAL_REGISTER:
			hwndAgentManualReg = CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_AGENT_MANUAL_REGISTER ), hwndDlg, JabberAgentManualRegDlgProc, 0 );
			return TRUE;
		case IDC_AGENT_REGISTER:
			lv = GetDlgItem( hwndDlg, IDC_AGENT_LIST );
			if (( lvItem.iItem=ListView_GetNextItem( lv, -1, LVNI_SELECTED )) >= 0 ) {
				lvItem.iSubItem = 0;
				lvItem.mask = LVIF_TEXT;
				lvItem.pszText = text;
				lvItem.cchTextMax = sizeof( text );
				SendMessageA( lv, LVM_GETITEMA, 0, (LPARAM)&lvItem );
				ListView_SetItemState( lv, lvItem.iItem, 0, LVIS_SELECTED ); // Unselect the item
				if (( item=JabberListGetItemPtr( LIST_AGENT, lvItem.pszText )) != NULL )
					JabberRegisterAgent( hwndDlg, item->jid );
			}
			return TRUE;
		case IDC_JOIN:
			lv = GetDlgItem( hwndDlg, IDC_AGENT_LIST );
			if (( lvItem.iItem=ListView_GetNextItem( lv, -1, LVNI_SELECTED )) >= 0 ) {
				lvItem.iSubItem = 0;
				lvItem.mask = LVIF_TEXT;
				lvItem.pszText = text;
				lvItem.cchTextMax = sizeof( text );
				SendMessageA( lv, LVM_GETITEMA, 0, (LPARAM)&lvItem );
				ListView_SetItemState( lv, lvItem.iItem, 0, LVIS_SELECTED ); // Unselect the item
				if (( item=JabberListGetItemPtr( LIST_AGENT, lvItem.pszText )) != NULL )
					JabberMenuHandleGroupchat( 0, ( LPARAM )item->jid );
			}
			return TRUE;
		case IDC_AGENT_SERVER:
			GetDlgItemTextA( hwndDlg, IDC_AGENT_SERVER, text, sizeof( text ));
			if ( jabberOnline && text[0] )
				EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_BROWSE ), TRUE );
			else
				EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_BROWSE ), FALSE );
			break;
		case IDC_AGENT_BROWSE:
			GetDlgItemTextA( hwndDlg, IDC_AGENT_SERVER, text, sizeof( text ));
			EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_BROWSE ), FALSE );
			ListView_DeleteAllItems( GetDlgItem( hwndDlg, IDC_AGENT_LIST ));
			JabberListRemoveList( LIST_AGENT );
			iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_DISCOAGENTS, JabberIqResultDiscoAgentItems );
			JabberSend( jabberThreadInfo->s, "<iq type='get' id='"JABBER_IQID"%d' to='%s'><query xmlns='http://jabber.org/protocol/disco#items'/></iq>", iqId, TXT(text));
			return TRUE;
		case IDC_AGENT_LOGON:
		case IDC_AGENT_LOGOFF:
			EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_UNREGISTER ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_LOGON ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_LOGOFF ), FALSE );
			lv = GetDlgItem( hwndDlg, IDC_AGENT_TRANSPORT );
			if (( lvItem.iItem=ListView_GetNextItem( lv, -1, LVNI_SELECTED )) >= 0 ) {
				lvItem.iSubItem = 0;
				lvItem.mask = LVIF_TEXT;
				lvItem.pszText = text;
				lvItem.cchTextMax = sizeof( text );
				SendMessageA( lv, LVM_GETITEMA, 0, (LPARAM)&lvItem );
				if (( item=JabberListGetItemPtr( LIST_ROSTER, lvItem.pszText )) != NULL ) {
					if ( LOWORD( wParam )==IDC_AGENT_LOGON )
						JabberSend( jabberThreadInfo->s, "<presence to='%s'/>", item->jid );
					else
						JabberSend( jabberThreadInfo->s, "<presence to='%s' type='unavailable'/>", item->jid );
			}	}
			return TRUE;
		case IDC_AGENT_UNREGISTER:
			EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_UNREGISTER ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_LOGON ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_AGENT_LOGOFF ), FALSE );
			lv = GetDlgItem( hwndDlg, IDC_AGENT_TRANSPORT );
			if (( lvItem.iItem=ListView_GetNextItem( lv, -1, LVNI_SELECTED )) >= 0 ) {
				lvItem.iSubItem = 0;
				lvItem.mask = LVIF_TEXT;
				lvItem.pszText = text;
				lvItem.cchTextMax = sizeof( text );
				SendMessageA( lv, LVM_GETITEMA, 0, (LPARAM)&lvItem );
				if (( item=JabberListGetItemPtr( LIST_ROSTER, lvItem.pszText )) != NULL ) {
					// strip off resource name if any
					strcpy( text, item->jid );
					//if (( p=strchr( text, '/' )) != NULL ) *p = '\0';
					if (( p=JabberTextEncode( text )) != NULL ) {
						JabberSend( jabberThreadInfo->s, "<iq type='set' to='%s'><query xmlns='jabber:iq:register'><remove/></query></iq>", p );
						JabberSend( jabberThreadInfo->s, "<iq type='set'><query xmlns='jabber:iq:roster'><item jid='%s' subscription='remove'></item></query></iq>", p );
						free( p );
			}	}	}
			return TRUE;

		case IDCLOSE:
			DestroyWindow( hwndDlg );
			return TRUE;
		}
		break;
	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;
	case WM_DESTROY:
		hwndJabberAgents = NULL;
		break;
	}
	return FALSE;
}

BOOL CALLBACK JabberAgentRegInputDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	static XmlNode *agentRegIqNode;

	XmlNode *queryNode, *xNode, *n;
	HWND hFrame, hCtrl;
	HFONT hFont;
	int id, ypos, i;
	char* from;
	char* str, *str2, *regStr;
	LONG frameExStyle;
	int iqId;
	char* p;

	switch ( msg ) {
	case WM_INITDIALOG:
		EnableWindow( GetParent( hwndDlg ), FALSE );
		TranslateDialogDefault( hwndDlg );
		agentRegIqNode = NULL;
		SetWindowText( hwndDlg, TranslateT( "Jabber Agent Registration" ));
		SetDlgItemText( hwndDlg, IDC_SUBMIT, TranslateT( "Register" ));
		SetDlgItemText( hwndDlg, IDC_FRAME_TEXT, TranslateT( "Please wait..." ));

		// Enable WS_EX_CONTROLPARENT on IDC_FRAME ( so tab stop goes through all its children )
		frameExStyle = GetWindowLong( GetDlgItem( hwndDlg, IDC_FRAME ), GWL_EXSTYLE );
		frameExStyle |= WS_EX_CONTROLPARENT;
		SetWindowLong( GetDlgItem( hwndDlg, IDC_FRAME ), GWL_EXSTYLE, frameExStyle );

		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_SUBMIT:
			if ( agentRegIqNode == NULL ) return TRUE;
			if (( from=JabberXmlGetAttrValue( agentRegIqNode, "from" )) == NULL ) return TRUE;
			if (( queryNode=JabberXmlGetChild( agentRegIqNode, "query" )) == NULL ) return TRUE;
			hFrame = GetDlgItem( hwndDlg, IDC_FRAME );
			regStr = ( char* )malloc( 512 );
			regStr[0] = '\0';
			str = ( char* )malloc( 128 );
			str2 = ( char* )malloc( 128 );
			id = 0;
			if (( xNode=JabberXmlGetChild( queryNode, "x" )) != NULL ) {
				// use new jabber:x:data form
				regStr = JabberFormGetData( hFrame, xNode );
			}
			else {
				// use old registration information form
				for ( i=0; i<queryNode->numChild; i++ ) {
					n = queryNode->child[i];
					if ( n->name ) {
						if ( !strcmp( n->name, "key" )) {
							// field that must be passed along with the registration
							if ( n->text ) {
								if (( p=JabberTextEncode( n->text )) != NULL ) {
									wsprintfA( str, "<%s>%s</%s>", n->name, p, n->name );
									free( p );
								}
							}
							else
								wsprintfA( str, "<%s/>", n->name );
							strcat( regStr, str );
						}
						else if ( !strcmp( n->name, "registered" ) || !strcmp( n->name, "instructions" )) {
							// do nothing, we will skip these
						}
						else {
							GetDlgItemTextA( hFrame, id, str2, 128 );
							if (( p=JabberTextEncode( str2 )) != NULL ) {
								wsprintfA( str, "<%s>%s</%s>", n->name, p, n->name );
								strcat( regStr, str );
								free( p );
							}
							id++;
			}	}	}	}

			iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_SETREGISTER, JabberIqResultSetRegister );
			JabberSend( jabberThreadInfo->s, "<iq type='set' id='"JABBER_IQID"%d' to='%s'><query xmlns='jabber:iq:register'>%s</query></iq>", iqId, from, regStr );
			free( str );
			free( str2 );
			free( regStr );
			DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_OPT_REGISTER ), hwndDlg, JabberAgentRegDlgProc, 0 );
			// Fall through to IDCANCEL
		case IDCANCEL:
			if ( agentRegIqNode )
				JabberXmlFreeNode( agentRegIqNode );
			DestroyWindow( hwndDlg );
			return TRUE;
		}
		break;
	case WM_JABBER_REGINPUT_ACTIVATE:
		if ( wParam == 1 ) { // success
			// lParam = <iq/> node from agent JID as a result of "get jabber:iq:register"
			hFrame = GetDlgItem( hwndDlg, IDC_FRAME );
			hFont = ( HFONT ) SendMessage( hFrame, WM_GETFONT, 0, 0 );
			ShowWindow( GetDlgItem( hwndDlg, IDC_FRAME_TEXT ), SW_HIDE );
			if (( agentRegIqNode=( XmlNode * ) lParam ) == NULL ) return TRUE;
			if (( queryNode=JabberXmlGetChild( agentRegIqNode, "query" )) == NULL ) return TRUE;
			id = 0;
			ypos = 14;
			if (( xNode=JabberXmlGetChild( queryNode, "x" )) != NULL ) {
				// use new jabber:x:data form
				if (( n=JabberXmlGetChild( xNode, "instructions" ))!=NULL && n->text!=NULL ) {
					char* str = JabberTextDecode( n->text );
					SetDlgItemTextA( hwndDlg, IDC_INSTRUCTION, str );
					free( str );
				}
				JabberFormCreateUI( hFrame, xNode, &i /*dummy*/ );
			}
			else {
				// use old registration information form
				for ( i=0; i<queryNode->numChild; i++ ) {
					n = queryNode->child[i];
					if ( n->name ) {
						str = JabberTextDecode( n->text );
						if ( !strcmp( n->name, "instructions" )) {
							if ( str ) SetDlgItemTextA( hwndDlg, IDC_INSTRUCTION, str );
						}
						else if ( !strcmp( n->name, "key" ) || !strcmp( n->name, "registered" )) {
							// do nothing
						}
						else if ( !strcmp( n->name, "password" )) {
							hCtrl = CreateWindowA( "static", n->name, WS_CHILD|WS_VISIBLE|SS_RIGHT, 10, ypos+4, 100, 18, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
							hCtrl = CreateWindowExA( WS_EX_CLIENTEDGE, "edit", str, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|ES_LEFT|ES_AUTOHSCROLL|ES_PASSWORD, 120, ypos, 128, 24, hFrame, ( HMENU ) id, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
							id++;
							ypos += 24;
						}
						else {	// everything else is a normal text field
							hCtrl = CreateWindowA( "static", n->name, WS_CHILD|WS_VISIBLE|SS_RIGHT, 10, ypos+4, 100, 18, hFrame, ( HMENU ) IDC_STATIC, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
							hCtrl = CreateWindowExA( WS_EX_CLIENTEDGE, "edit", str, WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|ES_LEFT|ES_AUTOHSCROLL, 120, ypos, 128, 24, hFrame, ( HMENU ) id, hInst, NULL );
							SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) hFont, 0 );
							id++;
							ypos += 24;
						}
						if ( str ) free( str );
			}	}	}

			EnableWindow( GetDlgItem( hwndDlg, IDC_SUBMIT ), TRUE );
		}
		else if ( wParam == 0 ) {
			// lParam = error message
			SetDlgItemTextA( hwndDlg, IDC_FRAME_TEXT, ( LPCSTR ) lParam );
		}
		return TRUE;
	case WM_DESTROY:
		hwndAgentRegInput = NULL;
		EnableWindow( GetParent( hwndDlg ), TRUE );
		SetActiveWindow( GetParent( hwndDlg ));
		break;
	}

	return FALSE;
}

static BOOL CALLBACK JabberAgentManualRegDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	static BOOL dontEnableParent;	// Very ugly hack

	switch ( msg ) {
	case WM_INITDIALOG:
		EnableWindow( GetParent( hwndDlg ), FALSE );
		dontEnableParent = FALSE;
		SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )LoadIcon( hInst, MAKEINTRESOURCE( IDI_AGENTS )) );
		TranslateDialogDefault( hwndDlg );
		return TRUE;
	case WM_COMMAND:
		{
			char jid[256];

			switch ( LOWORD( wParam )) {
			case IDC_JID:
				GetDlgItemTextA( hwndDlg, IDC_JID, jid, sizeof( jid ));
				EnableWindow( GetDlgItem( hwndDlg, IDOK ), ( jid[0]=='\0' )?FALSE:TRUE );
				break;
			case IDOK:
				GetDlgItemTextA( hwndDlg, IDC_JID, jid, sizeof( jid ));
				JabberRegisterAgent( GetParent( hwndDlg ), jid );
				dontEnableParent = TRUE;
				// Fall through
			case IDCANCEL:
			case IDCLOSE:
				DestroyWindow( hwndDlg );
				return TRUE;
		}	}
		break;
	case WM_DESTROY:
		hwndAgentManualReg = NULL;
		if ( dontEnableParent == FALSE ) {
			EnableWindow( GetParent( hwndDlg ), TRUE );
			SetActiveWindow( GetParent( hwndDlg ));
		}
		break;
	}

	return FALSE;
}

static BOOL CALLBACK JabberAgentRegDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		hwndRegProgress = hwndDlg;
		SetWindowTextA( hwndDlg, "Jabber Agent Registration" );
		TranslateDialogDefault( hwndDlg );
		ShowWindow( GetDlgItem( hwndDlg, IDOK ), SW_HIDE );
		ShowWindow( GetDlgItem( hwndDlg, IDCANCEL ), SW_HIDE );
		ShowWindow( GetDlgItem( hwndDlg, IDC_PROGRESS_REG ), SW_SHOW );
		ShowWindow( GetDlgItem( hwndDlg, IDCANCEL2 ), SW_SHOW );
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDCANCEL2:
		case IDOK2:
			hwndRegProgress = NULL;
			EndDialog( hwndDlg, 0 );
			return TRUE;
		}
		break;
	case WM_JABBER_REGDLG_UPDATE:	// wParam=progress ( 0-100 ), lparam=status string
		if (( char* )lParam == NULL )
			SetDlgItemText( hwndDlg, IDC_REG_STATUS, TranslateT( "No message" ));
		else
			SetDlgItemTextA( hwndDlg, IDC_REG_STATUS, ( char* )lParam );
		if ( wParam >= 0 )
			SendMessage( GetDlgItem( hwndDlg, IDC_PROGRESS_REG ), PBM_SETPOS, wParam, 0 );
		if ( wParam >= 100 ) {
			ShowWindow( GetDlgItem( hwndDlg, IDCANCEL2 ), SW_HIDE );
			ShowWindow( GetDlgItem( hwndDlg, IDOK2 ), SW_SHOW );
			SetFocus( GetDlgItem( hwndDlg, IDOK2 ));
		}
		else
			SetFocus( GetDlgItem( hwndDlg, IDCANCEL2 ));
		return TRUE;
	}

	return FALSE;
}
