/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2007  Michael Stepura, George Hazan

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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_groupchat.cpp,v $
Revision       : $Revision: 4256 $
Last change on : $Date: 2006-11-28 23:34:13 +1000 (Вт, 28 ноя 2006) $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include <commctrl.h>
#include "resource.h"
#include "jabber_iq.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Bookmarks editor window

static BOOL CALLBACK JabberAddBookmarkDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	TCHAR text[128];
	JABBER_LIST_ITEM *item;
	TCHAR* roomJID=0;
	TCHAR* currJID=0;

	switch ( msg ) {
	case WM_INITDIALOG:
		// lParam is the JABBER_BOOKMARK_ITEM*
		hwndJabberAddBookmark= hwndDlg;
		TranslateDialogDefault( hwndDlg );
		if ( item=(JABBER_LIST_ITEM* )lParam ){
			if (!lstrcmp( item->type, _T("conference") )) SendDlgItemMessage(hwndDlg, IDC_ROOM_RADIO, BM_SETCHECK, BST_CHECKED, 0);
			else {
				SendDlgItemMessage(hwndDlg, IDC_URL_RADIO, BM_SETCHECK, BST_CHECKED, 0);
				EnableWindow( GetDlgItem( hwndDlg, IDC_NICK ), FALSE );
				EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORD ), FALSE );
			}

			EnableWindow( GetDlgItem( hwndDlg, IDC_ROOM_RADIO), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_URL_RADIO), FALSE );
			
			currJID = mir_tstrdup(item->jid);
			SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) currJID );
			if (item->jid) SetDlgItemText( hwndDlg, IDC_ROOM_JID, mir_tstrdup(item->jid));
			if (item->name) SetDlgItemText( hwndDlg, IDC_NAME, mir_tstrdup (item->name) );
			if (item->nick) SetDlgItemText( hwndDlg, IDC_NICK, mir_tstrdup (item->nick) );
			if (item->password) SetDlgItemText( hwndDlg, IDC_PASSWORD, mir_tstrdup (item->password) );
			
		}
		else EnableWindow( GetDlgItem( hwndDlg, IDOK ), FALSE );
		return TRUE;
	case WM_COMMAND:
		switch ( HIWORD(wParam) ) {
			case BN_CLICKED:
				switch (LOWORD (wParam) ) {
					case IDC_ROOM_RADIO:
						EnableWindow( GetDlgItem( hwndDlg, IDC_NICK ), TRUE );
						EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORD ), TRUE );
						break;
					case IDC_URL_RADIO:
						EnableWindow( GetDlgItem( hwndDlg, IDC_NICK ), FALSE );
						EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORD ), FALSE );
						break;
				}
		}
//		break;	
		switch ( LOWORD( wParam )) {
		case IDC_ROOM_JID:
			if (( HWND )lParam==GetFocus() && HIWORD( wParam )==EN_CHANGE ) {
				GetDlgItemText( hwndDlg, IDC_ROOM_JID, text, SIZEOF( text ));
				roomJID = mir_tstrdup( text );

				if (roomJID == NULL || roomJID[0] == _T('\0')) EnableWindow( GetDlgItem( hwndDlg, IDOK ), FALSE );
				else EnableWindow( GetDlgItem( hwndDlg, IDOK ), TRUE );
			}

			break;

		case IDOK:
			GetDlgItemText( hwndDlg, IDC_ROOM_JID, text, SIZEOF( text ));
			roomJID = mir_tstrdup( text );

			currJID = ( TCHAR* )GetWindowLong( hwndDlg, GWL_USERDATA );
			
			if ( currJID) {
				JabberListRemove(LIST_BOOKMARK, currJID);
				mir_free( currJID );
				SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) NULL );
			}
			item=JabberListAdd(LIST_BOOKMARK, roomJID);

			if (SendDlgItemMessage(hwndDlg, IDC_URL_RADIO, BM_GETSTATE,0, 0) == BST_CHECKED ) {
				item->type=_T("url");
			}
			else item->type=_T("conference");
			
			GetDlgItemText( hwndDlg, IDC_NICK, text, SIZEOF( text ));
			item->nick = mir_tstrdup( text );

			GetDlgItemText( hwndDlg, IDC_PASSWORD, text, SIZEOF( text ));
			item->password = mir_tstrdup( text );

			GetDlgItemText( hwndDlg, IDC_NAME, text, SIZEOF( text ));
			item->name = mir_tstrdup(( text[0] == 0 ) ? roomJID : text );
			{
				int iqId = JabberSerialNext();
				JabberIqAdd( iqId, IQ_PROC_SETBOOKMARKS, JabberIqResultSetBookmarks);

				XmlNodeIq iq( "set", iqId);
				JabberSetBookmarkRequest(iq);
				JabberSend( jabberThreadInfo->s, iq );
			}
			// fall through
		case IDCANCEL:
			EndDialog( hwndDlg, 0 );
			break;
		}
		break;

	case WM_JABBER_CHECK_ONLINE:
		if ( !jabberOnline )
			EndDialog( hwndDlg, 0 );
		break;

	case WM_DESTROY:
		char* str = ( char* )GetWindowLong( hwndDlg, GWL_USERDATA );
		if ( str != NULL )
			mir_free( str );
		hwndJabberAddBookmark = NULL;
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Bookmarks manager window

static BOOL sortAscending;
static int sortColumn;

static int CALLBACK BookmarkCompare( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	JABBER_LIST_ITEM *item1, *item2;
	int res = 0;
	item1 = JabberListGetItemPtr( LIST_BOOKMARK, ( TCHAR* )lParam1 );
	item2 = JabberListGetItemPtr( LIST_BOOKMARK, ( TCHAR* )lParam2 );
	if ( item1!=NULL && item2!=NULL ) {
		switch ( lParamSort ) {
		case 1:	// sort by JID column
			res = lstrcmp( item1->jid, item2->jid );
			break;
		case 0: // sort by Name column
			res = lstrcmp( item1->name, item2->name );
			break;
		case 2:
			res = lstrcmp( item1->nick, item2->nick );
			break;
	}	}

	if ( !sortAscending )
		res *= -1;

	return res;
}

static BOOL CALLBACK JabberBookmarksDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HWND lv;
	LVCOLUMN lvCol;
	LVITEM lvItem;
	JABBER_LIST_ITEM *item;
	HIMAGELIST hIml;    // A handle to the image list.
	TCHAR room[256], *server, *p, *ItemNick;
	TCHAR text[128];

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )iconList[20]);//LoadIconEx( "bookmarks" ));
		hwndJabberBookmarks = hwndDlg;

		//EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_ADD ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), FALSE );
		TranslateDialogDefault( hwndDlg );
		sortColumn = -1;

		lv = GetDlgItem( hwndDlg, IDC_BM_LIST );

		ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP );
		hIml = ImageList_Create(16, 16,	 IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, 2, 1);
		if (hIml) {
			ImageList_AddIcon(hIml, (HICON)JCallService(MS_SKIN_LOADPROTOICON,(WPARAM)jabberProtoName,(LPARAM)ID_STATUS_ONLINE) );
			ImageList_AddIcon(hIml, (HICON)JCallService(MS_SKIN_LOADICON, SKINICON_EVENT_URL ,0));
			ListView_SetImageList (lv, hIml, LVSIL_SMALL);
		}

		// Add columns
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvCol.pszText = TranslateT( "Bookmark Name" );
		lvCol.cx = 120;
		lvCol.iSubItem = 0;
		ListView_InsertColumn( lv, 0, &lvCol );


		lvCol.pszText = TranslateT( "Room JID / URL" );
		lvCol.cx = 210;
		lvCol.iSubItem = 1;
		ListView_InsertColumn( lv, 1, &lvCol );

		lvCol.pszText = TranslateT( "Nick" );
		lvCol.cx = 90;
		lvCol.iSubItem = 2;
		ListView_InsertColumn( lv, 2, &lvCol );
		if ( jabberOnline ) {
			int iqId = JabberSerialNext();
			JabberIqAdd( iqId, IQ_PROC_DISCOBOOKMARKS, JabberIqResultDiscoBookmarks);

			XmlNodeIq iq( "get", iqId);
			XmlNode* query = iq.addQuery( "jabber:iq:private" );
			XmlNode* storage = query->addChild("storage");
			storage->addAttr("xmlns","storage:bookmarks");

			JabberSend( jabberThreadInfo->s, iq );
		}
		return TRUE;

	case WM_JABBER_ACTIVATE:
		ListView_DeleteAllItems( GetDlgItem( hwndDlg, IDC_BM_LIST));
		//EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_ADD ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), FALSE );
		return TRUE;

	case WM_JABBER_REFRESH:
		{
			lv = GetDlgItem( hwndDlg, IDC_BM_LIST);
			ListView_DeleteAllItems( lv );

			LVITEM lvItem;
			lvItem.iItem = 0;
			for ( int i=0; ( i = JabberListFindNext( LIST_BOOKMARK, i )) >= 0; i++ ) {
				if (( item = JabberListGetItemPtrFromIndex( i )) != NULL ) {
					TCHAR szBuffer[256];
					lvItem.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
					lvItem.iSubItem = 0;

					if(!lstrcmpi(item->type, _T("conference") )) lvItem.iImage = 0;
					else lvItem.iImage = 1;

					lvItem.lParam = ( LPARAM )item->jid;
					lvItem.pszText = item->name;
					ListView_InsertItem( lv, &lvItem );

					lvItem.mask = LVIF_TEXT;
					_tcsncpy( szBuffer, item->jid, SIZEOF(szBuffer));
					szBuffer[ SIZEOF(szBuffer)-1 ] = 0;
					lvItem.iSubItem = 1;
					lvItem.pszText = szBuffer;
					ListView_SetItem( lv, &lvItem );

					if(!lstrcmpi(item->type, _T("conference") )){

						if(item->nick){lvItem.mask = LVIF_TEXT;
						lvItem.iSubItem = 2;
						_tcsncpy( szBuffer, item->nick, SIZEOF(szBuffer));
						szBuffer[ SIZEOF(szBuffer)-1 ] = 0;
						lvItem.pszText = szBuffer;}
						ListView_SetItem( lv, &lvItem );
					}

					lvItem.iItem++;
			}	}

			if ( lvItem.iItem > 0 ) {
				EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT ), TRUE );
				EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), TRUE );
			}
			//EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), TRUE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_ADD ), TRUE );
		}
		return TRUE;

	case WM_JABBER_CHECK_ONLINE:
		if ( !jabberOnline ) {
			//EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_ADD ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), FALSE );

			lv = GetDlgItem( hwndDlg, IDC_BM_LIST);
			ListView_DeleteAllItems( lv );
		}
		else //EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), TRUE );
			SendMessage (hwndDlg, WM_COMMAND, IDC_BROWSE, 0);
		break;

	case WM_NOTIFY:
		switch ( wParam ) {
		case IDC_BM_LIST:
			switch (( ( LPNMHDR )lParam )->code ) {
			case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW pnmlv = ( LPNMLISTVIEW ) lParam;

					if ( pnmlv->iSubItem>=0 && pnmlv->iSubItem<=2 ) {
						if ( pnmlv->iSubItem == sortColumn )
							sortAscending = !sortAscending;
						else {
							sortAscending = TRUE;
							sortColumn = pnmlv->iSubItem;
						}
						ListView_SortItems( GetDlgItem( hwndDlg, IDC_BM_LIST), BookmarkCompare, sortColumn );
					}
				}
				break;
			case NM_DBLCLK:
				lv = GetDlgItem( hwndDlg, IDC_BM_LIST);
				if (( lvItem.iItem=ListView_GetNextItem( lv, -1, LVNI_SELECTED )) >= 0 ) {

					lvItem.iSubItem = 0;
					lvItem.mask = LVIF_PARAM;
					ListView_GetItem( lv, &lvItem );

					item = JabberListGetItemPtr(LIST_BOOKMARK, ( TCHAR* )lvItem.lParam);

					if(!lstrcmpi(item->type, _T("conference") )){
						if ( jabberChatDllPresent ) {
							_tcsncpy( text, ( TCHAR* )lvItem.lParam, SIZEOF( text ));
							_tcsncpy( room, text, SIZEOF( room ));

							p = _tcstok( room, _T( "@" ));
							server = _tcstok( NULL, _T( "@" ));

							lvItem.iSubItem = 2;
							lvItem.mask = LVIF_TEXT;
							lvItem.cchTextMax = SIZEOF(text);
							lvItem.pszText = text;

							ListView_GetItem( lv, &lvItem );
							ItemNick = mir_tstrdup(text);

							ListView_SetItemState( lv, lvItem.iItem, 0, LVIS_SELECTED ); // Unselect the item

							TCHAR *pass;
							if (item->password && item->password[0]!=_T('\0')) {pass = mir_tstrdup(item->password);}
							else pass = _T("");
							if (ItemNick && ItemNick[0]!=_T('\0')) {JabberGroupchatJoinRoom( server, p, ItemNick, pass );}
							else JabberGroupchatJoinRoom( server, p, JabberNickFromJID(jabberJID), pass );

						}
						else JabberChatDllError();
					}
					else {
						JCallService(MS_UTILS_OPENURL, 1, (LPARAM)t2a((TCHAR*) lvItem.lParam));
					}
				}
				return TRUE;
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_BROWSE:
			if ( jabberOnline) {
				//EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), FALSE );
				EnableWindow( GetDlgItem( hwndDlg, IDC_ADD ), FALSE );
				EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT ), FALSE );
				EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), FALSE );

				ListView_DeleteAllItems( GetDlgItem( hwndDlg, IDC_BM_LIST));

				int iqId = JabberSerialNext();
				JabberIqAdd( iqId, IQ_PROC_DISCOBOOKMARKS, JabberIqResultDiscoBookmarks);

				XmlNodeIq iq( "get", iqId);

				XmlNode* query = iq.addQuery( "jabber:iq:private" );
				XmlNode* storage = query->addChild("storage");
				storage->addAttr("xmlns","storage:bookmarks");

				JabberSend( jabberThreadInfo->s, iq );
			}
			return TRUE;

		case IDC_ADD:
			if ( jabberOnline)
				DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_BOOKMARK_ADD ), hwndDlg, JabberAddBookmarkDlgProc, NULL );
			return TRUE;

		case IDC_EDIT:
			if ( jabberOnline) {
				lv = GetDlgItem( hwndDlg, IDC_BM_LIST);
				if (( lvItem.iItem=ListView_GetNextItem( lv, -1, LVNI_SELECTED )) >= 0 ) {
					lvItem.iSubItem = 0;
					lvItem.mask = LVIF_PARAM;
					ListView_GetItem( lv, &lvItem );

					item = JabberListGetItemPtr(LIST_BOOKMARK, ( TCHAR* )lvItem.lParam);

					if (item) DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_BOOKMARK_ADD ), hwndDlg, JabberAddBookmarkDlgProc, (LPARAM) item);

					ListView_SetItemState( lv, lvItem.iItem, 0, LVIS_SELECTED ); // Unselect the item
			}	}
			return TRUE;

		case IDC_REMOVE:
			if ( jabberOnline) {
				lv = GetDlgItem( hwndDlg, IDC_BM_LIST);
				if (( lvItem.iItem=ListView_GetNextItem( lv, -1, LVNI_SELECTED )) >= 0 ) {

					//EnableWindow( GetDlgItem( hwndDlg, IDC_BROWSE ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, IDC_ADD ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), FALSE );

					lvItem.iSubItem = 0;
					lvItem.mask = LVIF_PARAM;
					ListView_GetItem( lv, &lvItem );

					JabberListRemove(LIST_BOOKMARK, ( TCHAR* )lvItem.lParam);

					ListView_SetItemState( lv, lvItem.iItem, 0, LVIS_SELECTED ); // Unselect the item

					int iqId = JabberSerialNext();
					JabberIqAdd( iqId, IQ_PROC_SETBOOKMARKS, JabberIqResultSetBookmarks);

					XmlNodeIq iq( "set", iqId );
					JabberSetBookmarkRequest( iq );
					JabberSend( jabberThreadInfo->s, iq );
			}	}
			return TRUE;

		case IDCLOSE:
		case IDCANCEL:
			DestroyWindow( hwndDlg );
			return TRUE;
		}
		break;

	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;

	case WM_DESTROY:
		hwndJabberBookmarks= NULL;
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Launches the Bookmarks manager window

int JabberMenuHandleBookmarks( WPARAM wParam, LPARAM lParam )
{
	int iqId;

	if ( IsWindow( hwndJabberBookmarks )) {
		SetForegroundWindow( hwndJabberBookmarks );

		SendMessage( hwndJabberBookmarks, WM_JABBER_ACTIVATE, 0, 0 );	// Just to clear the list
		iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_DISCOBOOKMARKS, JabberIqResultDiscoBookmarks);

		XmlNodeIq iq( "get", iqId);

		XmlNode* query = iq.addQuery( "jabber:iq:private" );
		XmlNode* storage = query->addChild("storage");
		storage->addAttr("xmlns","storage:bookmarks");

		// <iq/> result will send WM_JABBER_REFRESH to update the list with real data
		JabberSend( jabberThreadInfo->s, iq );
	}
	else CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_BOOKMARKS), NULL, JabberBookmarksDlgProc, lParam );

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Launches the Bookmark details window, lParam is JABBER_BOOKMARK_ITEM*
int JabberAddEditBookmark( WPARAM wParam, LPARAM lParam )
{
	if ( jabberOnline)
		DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_BOOKMARK_ADD ), NULL, JabberAddBookmarkDlgProc, lParam);
	return 0;
}


