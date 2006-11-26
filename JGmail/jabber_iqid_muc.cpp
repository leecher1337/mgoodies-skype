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
#include "resource.h"
#include "jabber_list.h"
#include "jabber_iq.h"
#include <commctrl.h>
#ifdef __GNUC__
	#define _WIN32_IE 0x0501
#endif

void JabberAddMucListItem( JABBER_MUC_JIDLIST_INFO* jidListInfo, TCHAR* str );
void JabberDeleteMucListItem( JABBER_MUC_JIDLIST_INFO* jidListInfo, TCHAR* str );
BOOL JabberEnterString( TCHAR* result, size_t resultLen );

void JabberIqResultBrowseRooms( XmlNode *iqNode, void *userdata )
{
	ThreadData* info = ( ThreadData* ) userdata;
	XmlNode *confNode, *roomNode;
	TCHAR* type, *category, *jid, *str;
	JABBER_LIST_ITEM *item;
	int i, j;

	// RECVED: room list
	// ACTION: refresh groupchat room list dialog
	JabberLog( "<iq/> iqIdBrowseRooms" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result"))) {
		JabberListRemoveList( LIST_ROOM );
		for ( i=0; i<iqNode->numChild; i++ ) {
			if (( confNode=iqNode->child[i] )!=NULL && confNode->name!=NULL ) {
				if ( !strcmp( confNode->name, "item" )) {
					if (( category=JabberXmlGetAttrValue( confNode, "category" ))!=NULL && !lstrcmp( category, _T("conference"))) {
						for ( j=0; j<confNode->numChild; j++ ) {
							if (( roomNode=confNode->child[j] )!=NULL && !strcmp( roomNode->name, "item" )) {
								if (( category=JabberXmlGetAttrValue( roomNode, "category" ))!=NULL && !lstrcmp( category, _T("conference"))) {
									if (( jid=JabberXmlGetAttrValue( roomNode, "jid" )) != NULL ) {
										item = JabberListAdd( LIST_ROOM, jid );
										if (( str=JabberXmlGetAttrValue( roomNode, "name" )) != NULL )
											item->name = mir_tstrdup( str );
										if (( str=JabberXmlGetAttrValue( roomNode, "type" )) != NULL )
											item->type = mir_tstrdup( str );
					}	}	}	}	}
				}
				else if ( !strcmp( confNode->name, "conference" )) {
					for ( j=0; j<confNode->numChild; j++ ) {
						if (( roomNode=confNode->child[j] )!=NULL && !strcmp( roomNode->name, "conference" )) {
							if (( jid=JabberXmlGetAttrValue( roomNode, "jid" )) != NULL ) {
								item = JabberListAdd( LIST_ROOM, jid );
								if (( str=JabberXmlGetAttrValue( roomNode, "name" )) != NULL )
									item->name = mir_tstrdup( str );
								if (( str=JabberXmlGetAttrValue( roomNode, "type" )) != NULL )
									item->type = mir_tstrdup( str );
		}	}	}	}	}	}

		if ( hwndJabberGroupchat != NULL ) {
			if (( jid=JabberXmlGetAttrValue( iqNode, "from" )) != NULL )
				SendMessage( hwndJabberGroupchat, WM_JABBER_REFRESH, 0, ( LPARAM )jid );
			else
				SendMessage( hwndJabberGroupchat, WM_JABBER_REFRESH, 0, ( LPARAM )info->server );
		}
	}
}

void JabberSetMucConfig( XmlNode* node, void *from )
{
	if ( jabberThreadInfo && from ) {
		XmlNodeIq iq( "set", NOID, ( TCHAR* )from );
		XmlNode* query = iq.addQuery( xmlnsOwner );
		query->addChild( node );
		JabberSend( jabberThreadInfo->s, iq );
}	}

void JabberIqResultGetMuc( XmlNode *iqNode, void *userdata )
{
	XmlNode *queryNode, *xNode;
	TCHAR *type, *from, *str;

	// RECVED: room config form
	// ACTION: show the form
	JabberLog( "<iq/> iqIdGetMuc" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( from=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;

	if ( !_tcscmp( type, _T("result"))) {
		if (( queryNode=JabberXmlGetChild( iqNode, "query" )) != NULL ) {
			str = JabberXmlGetAttrValue( queryNode, "xmlns" );
			if ( !lstrcmp( str, _T("http://jabber.org/protocol/muc#owner" ))) {
				if (( xNode=JabberXmlGetChild( queryNode, "x" )) != NULL ) {
					str = JabberXmlGetAttrValue( xNode, "xmlns" );
					if ( !lstrcmp( str, _T("jabber:x:data" )))
						JabberFormCreateDialog( xNode, _T("Jabber Conference Room Configuration"), JabberSetMucConfig, mir_tstrdup( from ));
}	}	}	}	}

void JabberIqResultDiscoRoomItems( XmlNode *iqNode, void *userdata )
{
	ThreadData* info = ( ThreadData* ) userdata;
	XmlNode *queryNode, *itemNode;
	TCHAR* type, *jid, *from;
	JABBER_LIST_ITEM *item;
	int i;
	int iqId;

	// RECVED: room list
	// ACTION: refresh groupchat room list dialog
	JabberLog( "<iq/> iqIdDiscoRoomItems" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( from=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result"))) {
		if (( queryNode=JabberXmlGetChild( iqNode, "query" )) != NULL ) {
			JabberListRemoveList( LIST_ROOM );
			for ( i=0; i<queryNode->numChild; i++ ) {
				if (( itemNode=queryNode->child[i] )!=NULL && itemNode->name!=NULL && !strcmp( itemNode->name, "item" )) {
					if (( jid=JabberXmlGetAttrValue( itemNode, "jid" )) != NULL ) {
						item = JabberListAdd( LIST_ROOM, jid );
						item->name = mir_tstrdup( JabberXmlGetAttrValue( itemNode, "name" ));
		}	}	}	}

		if ( hwndJabberGroupchat != NULL ) {
			if (( jid=JabberXmlGetAttrValue( iqNode, "from" )) != NULL )
				SendMessage( hwndJabberGroupchat, WM_JABBER_REFRESH, 0, ( LPARAM )jid );
			else
				SendMessage( hwndJabberGroupchat, WM_JABBER_REFRESH, 0, ( LPARAM )info->server );
		}
	}
	else if ( !_tcscmp( type, _T("error"))) {
		// disco is not supported, try browse
		iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_BROWSEROOMS, JabberIqResultBrowseRooms );

		XmlNodeIq iq( "get", iqId, from );
		XmlNode* query = iq.addQuery( "jabber:iq:browse" );
		JabberSend( jabberThreadInfo->s, iq );
}	}

static BOOL CALLBACK JabberMucJidListDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg ) {
	case WM_INITDIALOG:
		{
			// lParam is ( JABBER_MUC_JIDLIST_INFO * )
			LVCOLUMN lvc;
			RECT rc;
			HWND hwndList;

			TranslateDialogDefault( hwndDlg );
			hwndList = GetDlgItem( hwndDlg, IDC_LIST );
			GetClientRect( hwndList, &rc );
			rc.right -= GetSystemMetrics( SM_CXVSCROLL );
			lvc.mask = LVCF_WIDTH;
			lvc.cx = rc.right - 20;
			ListView_InsertColumn( hwndList, 0, &lvc );
			lvc.cx = 20;
			ListView_InsertColumn( hwndList, 1, &lvc );
			SendMessage( hwndDlg, WM_JABBER_REFRESH, 0, lParam );
		}
		return TRUE;
	case WM_JABBER_REFRESH:
		{
			// lParam is ( JABBER_MUC_JIDLIST_INFO * )
			JABBER_MUC_JIDLIST_INFO *jidListInfo;
			XmlNode *iqNode, *queryNode, *itemNode;
			TCHAR* from, *jid, *localFrom;
			LVITEM lvi;
			HWND hwndList;
			int count, i;
			TCHAR title[256];

			// Clear current GWL_USERDATA, if any
			jidListInfo = ( JABBER_MUC_JIDLIST_INFO * ) GetWindowLong( hwndDlg, GWL_USERDATA );
			if ( jidListInfo != NULL ) {
				if ( jidListInfo->roomJid != NULL )
					mir_free( jidListInfo->roomJid );
				if ( jidListInfo->iqNode != NULL )
					delete jidListInfo->iqNode;
				mir_free( jidListInfo );
			}

			// Clear current displayed list
			hwndList = GetDlgItem( hwndDlg, IDC_LIST );
			count = ListView_GetItemCount( hwndList );
			lvi.mask = LVIF_PARAM;
			lvi.iSubItem = 0;
			for ( i=0; i<count; i++ ) {
				lvi.iItem = i;
				if ( ListView_GetItem( hwndList, &lvi ) == TRUE ) {
					if ( lvi.lParam!=( LPARAM )( -1 ) && lvi.lParam!=( LPARAM )( NULL )) {
						mir_free(( void * ) lvi.lParam );
					}
				}
			}
			ListView_DeleteAllItems( hwndList );

			// Set new GWL_USERDATA
			jidListInfo = ( JABBER_MUC_JIDLIST_INFO * ) lParam;
			SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) jidListInfo );

			// Populate displayed list from iqNode
			lstrcpyn( title, TranslateT( "JID List" ), SIZEOF( title ));
			if (( jidListInfo=( JABBER_MUC_JIDLIST_INFO * ) lParam ) != NULL ) {
				if (( iqNode = jidListInfo->iqNode ) != NULL ) {
					if (( from = JabberXmlGetAttrValue( iqNode, "from" )) != NULL ) {
						jidListInfo->roomJid = mir_tstrdup( from );
						localFrom = mir_tstrdup( from );
						mir_sntprintf( title, SIZEOF( title ), _T("%s ( %s )"),
							( jidListInfo->type==MUC_VOICELIST ) ? TranslateT( "Voice List" ) :
							( jidListInfo->type==MUC_MEMBERLIST ) ? TranslateT( "Member List" ) :
							( jidListInfo->type==MUC_MODERATORLIST ) ? TranslateT( "Moderator List" ) :
							( jidListInfo->type==MUC_BANLIST ) ? TranslateT( "Ban List" ) :
							( jidListInfo->type==MUC_ADMINLIST ) ? TranslateT( "Admin List" ) :
							( jidListInfo->type==MUC_OWNERLIST ) ? TranslateT( "Owner List" ) :
							TranslateT( "JID List" ),
							localFrom );
						mir_free( localFrom );
						if (( queryNode=JabberXmlGetChild( iqNode, "query" )) != NULL ) {
							lvi.mask = LVIF_TEXT | LVIF_PARAM;
							lvi.iSubItem = 0;
							lvi.iItem = 0;
							for ( i=0; i<queryNode->numChild; i++ ) {
								if (( itemNode=queryNode->child[i] ) != NULL ) {
									if (( jid=JabberXmlGetAttrValue( itemNode, "jid" )) != NULL ) {
										lvi.pszText = jid;
										lvi.lParam = ( LPARAM )mir_tstrdup( jid );
										ListView_InsertItem( hwndList, &lvi );
										lvi.iItem++;
				}	}	}	}	}	}

				lvi.mask = LVIF_PARAM;
				lvi.lParam = ( LPARAM )( -1 );
				ListView_InsertItem( hwndList, &lvi );
			}
			SetWindowText( hwndDlg, title );
		}
		break;
	case WM_NOTIFY:
		if (( ( LPNMHDR )lParam )->idFrom == IDC_LIST ) {
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
							if ( nm->iSubItem == 1 ) {
								if( nm->nmcd.lItemlParam == ( LPARAM )( -1 ))
									hIcon = iconList[3]; //IDI_ADDCONTACT
								else
									hIcon = iconList[4]; //IDI_DELETE
								DrawIconEx( nm->nmcd.hdc, ( rc.left+rc.right-GetSystemMetrics( SM_CXSMICON ))/2, ( rc.top+rc.bottom-GetSystemMetrics( SM_CYSMICON ))/2,hIcon, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ), 0, NULL, DI_NORMAL );
								DestroyIcon( hIcon );
								SetWindowLong( hwndDlg, DWL_MSGRESULT, CDRF_SKIPDEFAULT );
								return TRUE;
				}	}	}	}
				break;
			case NM_CLICK:
				{
					JABBER_MUC_JIDLIST_INFO *jidListInfo;
					NMLISTVIEW *nm = ( NMLISTVIEW * ) lParam;
					LVITEM lvi;
					LVHITTESTINFO hti;
					TCHAR text[128];

					if ( nm->iSubItem < 1 ) break;
					jidListInfo = ( JABBER_MUC_JIDLIST_INFO * ) GetWindowLong( hwndDlg, GWL_USERDATA );
					hti.pt.x = ( short ) LOWORD( GetMessagePos());
					hti.pt.y = ( short ) HIWORD( GetMessagePos());
					ScreenToClient( nm->hdr.hwndFrom, &hti.pt );
					if ( ListView_SubItemHitTest( nm->hdr.hwndFrom, &hti ) == -1 )
						break;

					if ( hti.iSubItem != 1 )
						break;

					lvi.mask = LVIF_PARAM | LVIF_TEXT;
					lvi.iItem = hti.iItem;
					lvi.iSubItem = 0;
					lvi.pszText = text;
					lvi.cchTextMax = sizeof( text );
					ListView_GetItem( nm->hdr.hwndFrom, &lvi );
					if ( lvi.lParam == ( LPARAM )( -1 )) {
						TCHAR szBuffer[ 1024 ];
						_tcscpy( szBuffer, jidListInfo->type2str());
						if ( !JabberEnterString( szBuffer, SIZEOF(szBuffer)))
							break;

						// Trim leading and trailing whitespaces
						TCHAR *p = szBuffer, *q;
						for ( p = szBuffer; *p!='\0' && isspace( BYTE( *p )); p++);
						for ( q = p; *q!='\0' && !isspace( BYTE( *q )); q++);
						if (*q != '\0') *q = '\0';
						if (*p == '\0')
							break;

						JabberAddMucListItem( jidListInfo, p );
					}
					else {
						//delete
						TCHAR msgText[128];

						mir_sntprintf( msgText, sizeof( msgText ), _T("%s %s?"), TranslateT( "Removing" ), text );
						if ( MessageBox( hwndDlg, msgText, jidListInfo->type2str(), MB_YESNO|MB_SETFOREGROUND ) == IDYES ) {
							JabberDeleteMucListItem( jidListInfo, ( TCHAR* )lvi.lParam );
							ListView_DeleteItem( nm->hdr.hwndFrom, hti.iItem );
				}	}	}
				break;
			}
			break;
		}
		break;
/*	case WM_SETCURSOR:
		if ( LOWORD( LPARAM )!= HTCLIENT ) break;
		if ( GetForegroundWindow() == GetParent( hwndDlg )) {
			POINT pt;
			GetCursorPos( &pt );
			ScreenToClient( hwndDlg,&pt );
			SetFocus( ChildWindowFromPoint( hwndDlg,pt ));	  //ugly hack because listviews ignore their first click
		}
		break;
*/	case WM_CLOSE:
		{
			JABBER_MUC_JIDLIST_INFO *jidListInfo;
			HWND hwndList;
			int count, i;
			LVITEM lvi;

			// Free lParam of the displayed list items
			hwndList = GetDlgItem( hwndDlg, IDC_LIST );
			count = ListView_GetItemCount( hwndList );
			lvi.mask = LVIF_PARAM;
			lvi.iSubItem = 0;
			for ( i=0; i<count; i++ ) {
				lvi.iItem = i;
				if ( ListView_GetItem( hwndList, &lvi ) == TRUE ) {
					if ( lvi.lParam!=( LPARAM )( -1 ) && lvi.lParam!=( LPARAM )( NULL )) {
						mir_free(( void * ) lvi.lParam );
					}
				}
			}
			ListView_DeleteAllItems( hwndList );

			jidListInfo = ( JABBER_MUC_JIDLIST_INFO * ) GetWindowLong( hwndDlg, GWL_USERDATA );
			switch ( jidListInfo->type ) {
			case MUC_VOICELIST:
				hwndMucVoiceList = NULL;
				break;
			case MUC_MEMBERLIST:
				hwndMucMemberList = NULL;
				break;
			case MUC_MODERATORLIST:
				hwndMucModeratorList = NULL;
				break;
			case MUC_BANLIST:
				hwndMucBanList = NULL;
				break;
			case MUC_ADMINLIST:
				hwndMucAdminList = NULL;
				break;
			case MUC_OWNERLIST:
				hwndMucOwnerList = NULL;
				break;
			}

			DestroyWindow( hwndDlg );
		}
		break;
	case WM_DESTROY:
		{
			JABBER_MUC_JIDLIST_INFO *jidListInfo;

			// Clear GWL_USERDATA
			jidListInfo = ( JABBER_MUC_JIDLIST_INFO * ) GetWindowLong( hwndDlg, GWL_USERDATA );
			if ( jidListInfo != NULL ) {
				if ( jidListInfo->iqNode != NULL )
					delete jidListInfo->iqNode;
				if ( jidListInfo->roomJid != NULL )
					mir_free( jidListInfo->roomJid );
				mir_free( jidListInfo );
			}
		}
		break;
	}
	return FALSE;
}

static VOID CALLBACK JabberMucJidListCreateDialogApcProc( DWORD param )
{
	XmlNode *iqNode, *queryNode;
	TCHAR* from;
	JABBER_MUC_JIDLIST_INFO *jidListInfo;
	HWND *pHwndJidList;

	if (( jidListInfo=( JABBER_MUC_JIDLIST_INFO * ) param ) == NULL ) return;
	if (( iqNode=jidListInfo->iqNode ) == NULL )                      return;
	if (( from=JabberXmlGetAttrValue( iqNode, "from" )) == NULL )     return;
	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL )   return;

	switch ( jidListInfo->type ) {
	case MUC_VOICELIST:
		pHwndJidList = &hwndMucVoiceList;
		break;
	case MUC_MEMBERLIST:
		pHwndJidList = &hwndMucMemberList;
		break;
	case MUC_MODERATORLIST:
		pHwndJidList = &hwndMucModeratorList;
		break;
	case MUC_BANLIST:
		pHwndJidList = &hwndMucBanList;
		break;
	case MUC_ADMINLIST:
		pHwndJidList = &hwndMucAdminList;
		break;
	case MUC_OWNERLIST:
		pHwndJidList = &hwndMucOwnerList;
		break;
	default:
		mir_free( jidListInfo );
		return;
	}

	if ( *pHwndJidList!=NULL && IsWindow( *pHwndJidList )) {
		SetForegroundWindow( *pHwndJidList );
		SendMessage( *pHwndJidList, WM_JABBER_REFRESH, 0, ( LPARAM )jidListInfo );
	}
	else *pHwndJidList = CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_JIDLIST ), NULL, JabberMucJidListDlgProc, ( LPARAM )jidListInfo );
}

static void JabberIqResultMucGetJidList( XmlNode *iqNode, JABBER_MUC_JIDLIST_TYPE listType )
{
	TCHAR* type;
	JABBER_MUC_JIDLIST_INFO *jidListInfo;

	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;

	if ( !lstrcmp( type, _T("result" ))) {
		if (( jidListInfo=( JABBER_MUC_JIDLIST_INFO * ) mir_alloc( sizeof( JABBER_MUC_JIDLIST_INFO )) ) != NULL ) {
			jidListInfo->type = listType;
			jidListInfo->roomJid = NULL;	// Set in the dialog procedure
			if (( ( jidListInfo->iqNode )=JabberXmlCopyNode( iqNode )) != NULL ) {
				if ( GetCurrentThreadId() != jabberMainThreadId )
					QueueUserAPC( JabberMucJidListCreateDialogApcProc, hMainThread, ( DWORD )jidListInfo );
				else
					JabberMucJidListCreateDialogApcProc(( DWORD )jidListInfo );
			}
			else mir_free( jidListInfo );
}	}	}

void JabberIqResultMucGetVoiceList( XmlNode *iqNode, void *userdata )
{
	JabberLog( "<iq/> iqResultMucGetVoiceList" );
	JabberIqResultMucGetJidList( iqNode, MUC_VOICELIST );
}

void JabberIqResultMucGetMemberList( XmlNode *iqNode, void *userdata )
{
	JabberLog( "<iq/> iqResultMucGetMemberList" );
	JabberIqResultMucGetJidList( iqNode, MUC_MEMBERLIST );
}

void JabberIqResultMucGetModeratorList( XmlNode *iqNode, void *userdata )
{
	JabberLog( "<iq/> iqResultMucGetModeratorList" );
	JabberIqResultMucGetJidList( iqNode, MUC_MODERATORLIST );
}

void JabberIqResultMucGetBanList( XmlNode *iqNode, void *userdata )
{
	JabberLog( "<iq/> iqResultMucGetBanList" );
	JabberIqResultMucGetJidList( iqNode, MUC_BANLIST );
}

void JabberIqResultMucGetAdminList( XmlNode *iqNode, void *userdata )
{
	JabberLog( "<iq/> iqResultMucGetAdminList" );
	JabberIqResultMucGetJidList( iqNode, MUC_ADMINLIST );
}

void JabberIqResultMucGetOwnerList( XmlNode *iqNode, void *userdata )
{
	JabberLog( "<iq/> iqResultMucGetOwnerList" );
	JabberIqResultMucGetJidList( iqNode, MUC_OWNERLIST );
}

/////////////////////////////////////////////////////////////////////////////////////////

TCHAR* JABBER_MUC_JIDLIST_INFO::type2str() const
{
	switch( type ) {
		case MUC_VOICELIST:     return TranslateT( "Voice List" );
		case MUC_MEMBERLIST:    return TranslateT( "Member List" );
		case MUC_MODERATORLIST: return TranslateT( "Moderator List" );
		case MUC_BANLIST:       return TranslateT( "Ban List" );
		case MUC_ADMINLIST:     return TranslateT( "Admin List" );
		case MUC_OWNERLIST:     return TranslateT( "Owner List" );
	}

	return TranslateT( "JID List" );
}
