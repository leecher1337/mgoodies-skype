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
#include "jabber_iq.h"
#include "resource.h"

extern HANDLE hInitChat;

/////////////////////////////////////////////////////////////////////////////////////////
// One string entry dialog

struct JabberEnterStringParams
{	TCHAR* result;
	size_t resultLen;
};

BOOL CALLBACK JabberEnterStringDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault( hwndDlg );
		JabberEnterStringParams* params = ( JabberEnterStringParams* )lParam;
		SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG )params );
		SetWindowText( hwndDlg, params->result );
		return TRUE;
	}

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
		{	JabberEnterStringParams* params = ( JabberEnterStringParams* )GetWindowLong( hwndDlg, GWL_USERDATA );
			GetDlgItemText( hwndDlg, IDC_TOPIC, params->result, params->resultLen );
			params->result[ params->resultLen-1 ] = 0;
			EndDialog( hwndDlg, 1 );
			break;
		}
		case IDCANCEL:
			EndDialog( hwndDlg, 0 );
			break;
	}	}

	return FALSE;
}

BOOL JabberEnterString( TCHAR* result, size_t resultLen )
{
	JabberEnterStringParams params = { result, resultLen };
	return DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_GROUPCHAT_INPUT ), NULL, JabberEnterStringDlgProc, LPARAM( &params ));
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGcInit - initializes the new chat

static TCHAR* sttRoles[] = { _T("Other"), _T("Visitors"), _T("Participants"), _T("Moderators") };

int JabberGcInit( WPARAM wParam, LPARAM lParam )
{
	JABBER_LIST_ITEM* item = ( JABBER_LIST_ITEM* )wParam;
	GCSESSION gcw = {0};
	GCEVENT gce = {0};

	TCHAR* szNick = JabberNickFromJID( item->jid );
	gcw.cbSize = sizeof(GCSESSION);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = jabberProtoName;
	gcw.ptszName = szNick;
	gcw.ptszID = item->jid;
	gcw.dwFlags = GC_TCHAR;
	JCallService( MS_GC_NEWSESSION, NULL, (LPARAM)&gcw );

	HANDLE hContact = JabberHContactFromJID( item->jid );
	if ( hContact != NULL ) {
		DBVARIANT dbv;
		if ( !DBGetContactSettingTString( hContact, jabberProtoName, "MyNick", &dbv )) {
			if ( !lstrcmp( dbv.ptszVal, szNick ))
				JDeleteSetting( hContact, "MyNick" );
			else
				JSetStringT( hContact, "MyNick", item->nick );
			JFreeVariant( &dbv );
		}
		else JSetStringT( hContact, "MyNick", item->nick );
	}
	mir_free( szNick );

	item->bChatActive = TRUE;

	GCDEST gcd = { jabberProtoName, NULL, GC_EVENT_ADDGROUP };
	gcd.ptszID = item->jid;
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.dwFlags = GC_TCHAR;
	for ( int i = SIZEOF(sttRoles)-1; i >= 0; i-- ) {
		gce.ptszStatus = TranslateTS( sttRoles[i] );
		JCallService(MS_GC_EVENT, NULL, ( LPARAM )&gce );
	}

	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gcd.iType = GC_EVENT_CONTROL;
	JCallService(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	JCallService(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
	return 0;
}

void JabberGcLogCreate( JABBER_LIST_ITEM* item )
{
	if ( item->bChatActive )
		return;

	NotifyEventHooks( hInitChat, (WPARAM)item, 0 );
}

void JabberGcLogUpdateMemberStatus( JABBER_LIST_ITEM* item, TCHAR* nick, TCHAR* jid, int action, XmlNode* reason )
{
	int statusToSet = 0;
	TCHAR* szReason = NULL;
	if ( reason != NULL && reason->text != NULL )
		szReason = reason->text;

	TCHAR* myNick = (item->nick == NULL) ? NULL : mir_tstrdup( item->nick );
	if ( myNick == NULL )
		myNick = JabberNickFromJID( jabberJID );

	GCDEST gcd = { jabberProtoName, 0, 0 };
	gcd.ptszID = item->jid;
	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.ptszNick = nick;
	gce.ptszUID = nick;
	if (jid != NULL)
		gce.ptszUserInfo = jid;
	gce.ptszText = szReason;
	gce.dwFlags = GC_TCHAR;
	gce.pDest = &gcd;
 	if ( item->bChatActive == 2 ) {
		gce.dwFlags |= GCEF_ADDTOLOG;
		gce.time = time(0);
	}

	switch( gcd.iType = action ) {
	case GC_EVENT_PART:  break;
	case GC_EVENT_KICK:  gce.ptszStatus = TranslateT( "Moderator" );  break;
	default:
		for ( int i=0; i < item->resourceCount; i++ ) {
			JABBER_RESOURCE_STATUS& JS = item->resource[i];
			if ( !lstrcmp( nick, JS.resourceName )) {
				if ( action != GC_EVENT_JOIN ) {
					switch( action ) {
					case 0:
						gcd.iType = GC_EVENT_ADDSTATUS;
					case GC_EVENT_REMOVESTATUS:
						gce.dwFlags &= ~GCEF_ADDTOLOG;
					}
					gce.ptszText = TranslateT( "Moderator" );
				}
				gce.ptszStatus = TranslateTS( sttRoles[ JS.role ] );
				gce.bIsMe = ( lstrcmp( nick, myNick ) == 0 );
				statusToSet = JS.status;
				break;
	}	}	}

	JCallService( MS_GC_EVENT, NULL, ( LPARAM )&gce );

	if ( statusToSet != 0 ) {
		gce.ptszText = nick;
		if ( statusToSet == ID_STATUS_AWAY || statusToSet == ID_STATUS_NA || statusToSet == ID_STATUS_DND )
			gce.dwItemData = 3;
		else
			gce.dwItemData = 1;
		gcd.iType = GC_EVENT_SETSTATUSEX;
		JCallService( MS_GC_EVENT, NULL, ( LPARAM )&gce );
	}

	mir_free( myNick );
}

void JabberGcQuit( JABBER_LIST_ITEM* item, int code, XmlNode* reason )
{
	TCHAR* szReason = NULL;
	if ( reason != NULL && reason->text != NULL )
		szReason = reason->text;

	GCDEST gcd = { jabberProtoName, NULL, GC_EVENT_CONTROL };
	gcd.ptszID = item->jid;
	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.ptszUID = item->jid;
	gce.ptszText = szReason;
	gce.dwFlags = GC_TCHAR;
	gce.pDest = &gcd;

	if ( code != 307 ) {
		JCallService( MS_GC_EVENT, SESSION_TERMINATE, ( LPARAM )&gce );
		JCallService( MS_GC_EVENT, WINDOW_CLEARLOG, ( LPARAM )&gce );
	}
	else {
		TCHAR* myNick = JabberNickFromJID( jabberJID );
		JabberGcLogUpdateMemberStatus( item, myNick, NULL, GC_EVENT_KICK, reason );
		mir_free( myNick );
		JCallService( MS_GC_EVENT, SESSION_OFFLINE, ( LPARAM )&gce );
	}

	DBDeleteContactSetting( JabberHContactFromJID( item->jid ), "CList", "Hidden" );
	item->bChatActive = FALSE;

	if ( jabberOnline ) {
		TCHAR text[ 1024 ];
		mir_sntprintf( text, SIZEOF( text ), _T("%s/%s"), item->jid, item->nick );
		XmlNode p( "presence" ); p.addAttr( "to", text ); p.addAttr( "type", "unavailable" );
		JabberSend( jabberThreadInfo->s, p );
		JabberListRemove( LIST_CHATROOM, item->jid );
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu hooks

#define IDM_LEAVE       10
#define IDM_TOPIC       12

int JabberGcMenuHook( WPARAM wParam, LPARAM lParam )
{
	GCMENUITEMS* gcmi= ( GCMENUITEMS* )lParam;
	if ( gcmi == NULL )
		return 0;

	if ( lstrcmpiA( gcmi->pszModule, jabberProtoName ))
		return 0;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, gcmi->pszID );
	if ( item == NULL )
		return 0;

	JABBER_RESOURCE_STATUS *me = NULL, *him = NULL;
	for ( int i=0; i < item->resourceCount; i++ ) {
		JABBER_RESOURCE_STATUS& p = item->resource[i];
		if ( !lstrcmp( p.resourceName, item->nick   ))  me = &p;
		if ( !lstrcmp( p.resourceName, gcmi->pszUID ))  him = &p;
	}

	if ( gcmi->Type == MENU_ON_LOG ) {
		static struct gc_item sttLogListItems[] = {
			{ TranslateT( "&Leave chat session" ),    IDM_LEAVE,     MENU_ITEM, FALSE },
			{ TranslateT( "Add to Bookmarks" ),       IDM_BOOKMARKS, MENU_ITEM, TRUE },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "&Voice List..." ),         IDM_VOICE,     MENU_ITEM, TRUE  },
			{ TranslateT( "&Ban List..." ),           IDM_BAN,       MENU_ITEM, TRUE  },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "&Member List..." ),        IDM_MEMBER,    MENU_ITEM, TRUE  },
			{ TranslateT( "Mo&derator List..." ),     IDM_MODERATOR, MENU_ITEM, TRUE  },
			{ TranslateT( "&Admin List..." ),         IDM_ADMIN,     MENU_ITEM, TRUE  },
			{ TranslateT( "&Owner List..." ),         IDM_OWNER,     MENU_ITEM, TRUE  },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "Change &Nickname..." ),    IDM_NICK,      MENU_ITEM, FALSE },
			{ TranslateT( "Set &Topic..." ),          IDM_TOPIC,     MENU_ITEM, FALSE },
			{ TranslateT( "&Invite a User..." ),      IDM_INVITE,    MENU_ITEM, FALSE },
			{ TranslateT( "Room Con&figuration..." ), IDM_CONFIG,    MENU_ITEM, TRUE  },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "Destroy Room..." ),        IDM_DESTROY,   MENU_ITEM, TRUE  }};

		gcmi->nItems = sizeof( sttLogListItems ) / sizeof( sttLogListItems[0] );
		gcmi->Item = sttLogListItems;

		if ( me != NULL ) {
			if ( me->role == ROLE_MODERATOR )
				sttLogListItems[3].bDisabled = FALSE;

			if ( me->affiliation == AFFILIATION_ADMIN )
				sttLogListItems[4].bDisabled = sttLogListItems[6].bDisabled = sttLogListItems[7].bDisabled = FALSE;
			else if ( me->affiliation == AFFILIATION_OWNER )
				sttLogListItems[4].bDisabled = sttLogListItems[6].bDisabled =
				sttLogListItems[7].bDisabled = sttLogListItems[8].bDisabled =
				sttLogListItems[9].bDisabled = sttLogListItems[14].bDisabled =
				sttLogListItems[16].bDisabled = FALSE;
		}
		if ( jabberThreadInfo->caps & CAPS_BOOKMARK ) sttLogListItems[1].bDisabled = FALSE;
	}
	else if ( gcmi->Type == MENU_ON_NICKLIST ) {
		static struct gc_item sttListItems[] = {
			{ TranslateT( "&User Details" ),          IDM_VCARD,     MENU_ITEM, FALSE },
			{ TranslateT( "&Leave chat session" ),    IDM_LEAVE,     MENU_ITEM, FALSE },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "Kick" ),                   IDM_KICK,      MENU_ITEM, TRUE },
			{ TranslateT( "Ban" ),                    IDM_BAN,       MENU_ITEM, TRUE },
			{ NULL, 0, MENU_SEPARATOR, FALSE },
			{ TranslateT( "Toggle &Voice" ),          IDM_VOICE,     MENU_ITEM, TRUE },
			{ TranslateT( "Toggle Moderator" ),       IDM_MODERATOR, MENU_ITEM, TRUE },
			{ TranslateT( "Toggle Admin" ),           IDM_ADMIN,     MENU_ITEM, TRUE },
			{ TranslateT( "Toggle Owner" ),           IDM_OWNER,     MENU_ITEM, TRUE }};

		gcmi->nItems = sizeof( sttListItems )/sizeof( sttListItems[0] );
		gcmi->Item = sttListItems;

		if ( me != NULL && him != NULL ) {
			if ( me->role == ROLE_MODERATOR )
				if ( him->affiliation != AFFILIATION_ADMIN && him->affiliation != AFFILIATION_OWNER )
					sttListItems[3].bDisabled = sttListItems[4].bDisabled = FALSE;

			if ( me->affiliation == AFFILIATION_ADMIN ) {
				if ( him->affiliation != AFFILIATION_ADMIN && him->affiliation != AFFILIATION_OWNER )
					sttListItems[6].bDisabled = sttListItems[7].bDisabled = FALSE;
			}
			else if ( me->affiliation == AFFILIATION_OWNER )
				sttListItems[6].bDisabled = sttListItems[7].bDisabled =
				sttListItems[8].bDisabled = sttListItems[9].bDisabled = FALSE;
	}	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Conference invitation dialog

static void FilterList(HWND hwndList)
{
	for	(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
			hContact;
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
	{
		char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (!proto || lstrcmpA(proto, jabberProtoName) || DBGetContactSettingByte(hContact, proto, "ChatRoom", 0))
			if (int hItem = SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0))
				SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
}	}
 
static void ResetListOptions(HWND hwndList)
{
	int i;
	SendMessage(hwndList,CLM_SETBKBITMAP,0,(LPARAM)(HBITMAP)NULL);
	SendMessage(hwndList,CLM_SETBKCOLOR,GetSysColor(COLOR_WINDOW),0);
	SendMessage(hwndList,CLM_SETGREYOUTFLAGS,0,0);
	SendMessage(hwndList,CLM_SETLEFTMARGIN,4,0);
	SendMessage(hwndList,CLM_SETINDENT,10,0);
	SendMessage(hwndList,CLM_SETHIDEEMPTYGROUPS,1,0);
	SendMessage(hwndList,CLM_SETHIDEOFFLINEROOT,1,0);
	for ( i=0; i <= FONTID_MAX; i++ )
		SendMessage( hwndList, CLM_SETTEXTCOLOR, i, GetSysColor( COLOR_WINDOWTEXT ));
}

static void InviteUser(TCHAR *room, TCHAR *pUser, TCHAR *text)
{
	int iqId = JabberSerialNext();

	XmlNode m( "message" ); m.addAttr( "from", jabberJID ); m.addAttr( "to", room ); m.addAttrID( iqId );
	XmlNode* x = m.addChild( "x" ); x->addAttr( "xmlns", _T("http://jabber.org/protocol/muc#user"));
	XmlNode* i = x->addChild( "invite" ); i->addAttr( "to", pUser ); 
	if ( text[0] != 0 )
		i->addChild( "reason", text );
	JabberSend( jabberThreadInfo->s, m );
}

struct JabberGcLogInviteDlgJidData
{
	int hItem;
	TCHAR jid[JABBER_MAX_JID_LEN];
};

struct JabberGcLogInviteDlgData 
{
	JabberGcLogInviteDlgData(const TCHAR *room):
		newJids(1), room(mir_tstrdup(room)) {}
	~JabberGcLogInviteDlgData()
	{
		for (int i = 0; i < newJids.getCount(); ++i)
			mir_free(newJids[i]);
		mir_free(room);
	}

	LIST<JabberGcLogInviteDlgJidData> newJids;
	TCHAR *room;
};

static BOOL CALLBACK JabberGcLogInviteDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		{
			RECT dlgRect, scrRect;
			GetWindowRect( hwndDlg, &dlgRect );
			SystemParametersInfo( SPI_GETWORKAREA, 0, &scrRect, 0 );
			SetWindowPos( hwndDlg, HWND_TOPMOST, (scrRect.right/2)-(dlgRect.right/2), (scrRect.bottom/2)-(dlgRect.bottom/2), 0, 0, SWP_NOSIZE );
			TranslateDialogDefault( hwndDlg );
			SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )iconBigList[0]);
			SetDlgItemText( hwndDlg, IDC_ROOM, ( TCHAR* )lParam );

			SetWindowLong(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE,
				GetWindowLong(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE)|CLS_HIDEOFFLINE|CLS_CHECKBOXES|CLS_HIDEEMPTYGROUPS|CLS_USEGROUPS|CLS_GREYALTERNATE|CLS_GROUPCHECKBOXES);
			SendMessage(GetDlgItem(hwndDlg, IDC_CLIST), CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP|CLS_EX_TRACKSELECT, 0);
			ResetListOptions(GetDlgItem(hwndDlg, IDC_CLIST));
			FilterList(GetDlgItem(hwndDlg, IDC_CLIST));

			SendDlgItemMessage(hwndDlg, IDC_ADDJID, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_ADDJID, BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconList[17]);//LoadIconEx("addroster"));

			// use new operator to properly construct LIST object
			JabberGcLogInviteDlgData *data = new JabberGcLogInviteDlgData((TCHAR *)lParam);
			data->room = mir_tstrdup((TCHAR *)lParam);
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)data);
		}
		return TRUE;

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_ADDJID:
			{
				TCHAR buf[JABBER_MAX_JID_LEN];
				GetWindowText(GetDlgItem(hwndDlg, IDC_NEWJID), buf, SIZEOF(buf));
				SetWindowText(GetDlgItem(hwndDlg, IDC_NEWJID), _T(""));

				if (JabberHContactFromJID(buf))
					break;

				JabberGcLogInviteDlgData *data = (JabberGcLogInviteDlgData *)GetWindowLong(hwndDlg, GWL_USERDATA);

				int i;
				for (i = 0; i < data->newJids.getCount(); ++i)
					if (!lstrcmp(data->newJids[i]->jid, buf))
						break;
				if (i != data->newJids.getCount())
					break;

				JabberGcLogInviteDlgJidData *jidData = (JabberGcLogInviteDlgJidData *)mir_alloc(sizeof(JabberGcLogInviteDlgJidData));
				lstrcpy(jidData->jid, buf);
				CLCINFOITEM cii = {0};
				cii.cbSize = sizeof(cii);
				cii.flags = CLCIIF_CHECKBOX;
				mir_sntprintf(buf, SIZEOF(buf), _T("%s (%s)"), jidData->jid, TranslateT("not on roster"));
				cii.pszText = buf;
				jidData->hItem = SendDlgItemMessage(hwndDlg,IDC_CLIST,CLM_ADDINFOITEM,0,(LPARAM)&cii);
				SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, jidData->hItem, 1);
				data->newJids.insert(jidData);
			}
			break;

		case IDC_INVITE:
			{
				JabberGcLogInviteDlgData *data = (JabberGcLogInviteDlgData *)GetWindowLong(hwndDlg, GWL_USERDATA);
				TCHAR* room = data->room;
				if ( room != NULL ) {
					TCHAR text[256];
					GetDlgItemText( hwndDlg, IDC_REASON, text, SIZEOF( text ));
					HWND hwndList = GetDlgItem(hwndDlg, IDC_CLIST);

					// invite users from roster
					for	(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
							hContact;
							hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
						char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
						if ( !lstrcmpA(proto, jabberProtoName) && !DBGetContactSettingByte(hContact, proto, "ChatRoom", 0)) {
							if (int hItem = SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0)) {
								if ( SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0 )) {
									DBVARIANT dbv={0};
									JGetStringT(hContact, "jid", &dbv);
									if (dbv.ptszVal && ( dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_WCHAR ))
										InviteUser(room, dbv.ptszVal, text);
									JFreeVariant(&dbv);
					}	}	}	}

					// invite others
					for (int i = 0; i < data->newJids.getCount(); ++i)
						if (SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)data->newJids[i]->hItem, 0))
							InviteUser(room, data->newJids[i]->jid, text);
			}	}
			// Fall through
		case IDCANCEL:
		case IDCLOSE:
			DestroyWindow( hwndDlg );
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_CLIST) {
			switch (((LPNMHDR)lParam)->code) {
			case CLN_NEWCONTACT:
				FilterList(GetDlgItem(hwndDlg,IDC_CLIST));
				break;
			case CLN_LISTREBUILT:
				FilterList(GetDlgItem(hwndDlg,IDC_CLIST));
				break;
			case CLN_OPTIONSCHANGED:
				ResetListOptions(GetDlgItem(hwndDlg,IDC_CLIST));
				break;
		}	}
		break;

	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;

	case WM_DESTROY:
		JabberGcLogInviteDlgData *data = (JabberGcLogInviteDlgData *)GetWindowLong(hwndDlg, GWL_USERDATA);
		delete data;
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu processing

static void JabberAdminSet( const TCHAR* to, const char* ns, const char* szItem, const TCHAR* itemVal, const char* var, const TCHAR* varVal )
{
	XmlNodeIq iq( "set", NOID, to );
	XmlNode* query = iq.addQuery( ns );
	XmlNode* item = query->addChild( "item" ); item->addAttr( szItem, itemVal ); item->addAttr( var, varVal );
	JabberSend( jabberThreadInfo->s, iq );
}

static void JabberAdminGet( const TCHAR* to, const char* ns, const char* var, const TCHAR* varVal, JABBER_IQ_PFUNC foo )
{
	int id = JabberSerialNext();
	JabberIqAdd( id, IQ_PROC_NONE, foo );

	XmlNodeIq iq( "get", id, to );
	XmlNode* query = iq.addQuery( ns );
	XmlNode* item = query->addChild( "item" ); item->addAttr( var, varVal );
	JabberSend( jabberThreadInfo->s, iq );
}

static void sttNickListHook( JABBER_LIST_ITEM* item, GCHOOK* gch )
{
	JABBER_RESOURCE_STATUS *me = NULL, *him = NULL;
	for ( int i=0; i < item->resourceCount; i++ ) {
		JABBER_RESOURCE_STATUS& p = item->resource[i];
		if ( !lstrcmp( p.resourceName, item->nick  )) me = &p;
		if ( !lstrcmp( p.resourceName, gch->ptszUID )) him = &p;
	}

	if ( him == NULL || me == NULL )
		return;

	TCHAR szBuffer[ 1024 ];

	switch( gch->dwData ) {
	case IDM_LEAVE:
		JabberGcQuit( item, 0, 0 );
		break;
	case IDM_VCARD:
	{
		HANDLE hContact;
		JABBER_SEARCH_RESULT jsr;
//		_tcsncpy(jsr.jid, him->jid, SIZEOF(jsr.jid));
		mir_sntprintf(jsr.jid, SIZEOF(jsr.jid), _T("%s/%s"), item->jid, him->resourceName );
		jsr.hdr.cbSize = sizeof(JABBER_SEARCH_RESULT);
		
		JABBER_LIST_ITEM* item = JabberListAdd( LIST_VCARD_TEMP, jsr.jid );
		JabberListAddResource( LIST_VCARD_TEMP, jsr.jid, him->status, him->statusMessage);

		hContact=(HANDLE)CallProtoService(jabberProtoName, PS_ADDTOLIST, PALF_TEMPORARY,(LPARAM)&jsr);

		int iqId = JabberSerialNext(); // Requesto for version
		XmlNodeIq iq( "get", iqId, jsr.jid );
		XmlNode* query = iq.addQuery( "jabber:iq:version" );
		JabberSend( jabberThreadInfo->s, iq );
		
		CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)hContact,0);
		break;
	}
	case IDM_KICK:
	{
		mir_sntprintf( szBuffer, SIZEOF(szBuffer), _T("%s %s"), TranslateT( "Reason to kick" ), him->resourceName );
		if ( JabberEnterString( szBuffer, SIZEOF(szBuffer))) {
			XmlNodeIq iq( "set", NOID, item->jid );
			XmlNode* query = iq.addQuery( xmlnsAdmin );
			XmlNode* item = query->addChild( "item" ); item->addAttr( "nick", him->resourceName ); item->addAttr( "role", "none" );
			item->addChild( "reason", szBuffer );
			JabberSend( jabberThreadInfo->s, iq );
		}
		break;
	}

	case IDM_BAN:
		mir_sntprintf( szBuffer, SIZEOF(szBuffer), _T("%s %s"), TranslateT( "Reason to ban" ), him->resourceName );
		if ( JabberEnterString( szBuffer, SIZEOF(szBuffer))) {
			XmlNodeIq iq( "set", NOID, item->jid );
			XmlNode* query = iq.addQuery( xmlnsAdmin );
			XmlNode* item = query->addChild( "item" ); item->addAttr( "nick", him->resourceName ); item->addAttr( "affiliation", "outcast" );
			item->addChild( "reason", szBuffer );
			JabberSend( jabberThreadInfo->s, iq );
		}
		break;

	case IDM_VOICE:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", him->resourceName,
			"role", ( him->role == ROLE_PARTICIPANT ) ? _T("visitor") : _T("participant"));
		break;

	case IDM_MODERATOR:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", him->resourceName,
			"role", ( him->role == ROLE_MODERATOR ) ? _T("participant") : _T("moderator"));
		break;

	case IDM_ADMIN:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", him->resourceName,
			"affiliation", ( him->affiliation==AFFILIATION_ADMIN )? _T("member") : _T("admin"));
		break;

	case IDM_OWNER:
		JabberAdminSet( item->jid, xmlnsAdmin, "nick", him->resourceName,
			"affiliation", ( him->affiliation==AFFILIATION_OWNER ) ? _T("admin") : _T("owner"));
		break;
}	}

static void sttLogListHook( JABBER_LIST_ITEM* item, GCHOOK* gch )
{
	TCHAR szBuffer[ 1024 ];

	switch( gch->dwData ) {
	case IDM_VOICE:
		JabberAdminGet( gch->pDest->ptszID, xmlnsAdmin, "role", _T("participant"), JabberIqResultMucGetVoiceList );
		break;

	case IDM_MEMBER:
		JabberAdminGet( gch->pDest->ptszID, xmlnsAdmin, "affiliation", _T("member"), JabberIqResultMucGetMemberList );
		break;

	case IDM_MODERATOR:
		JabberAdminGet( gch->pDest->ptszID, xmlnsAdmin, "role", _T("moderator"), JabberIqResultMucGetModeratorList );
		break;

	case IDM_BAN:
		JabberAdminGet( gch->pDest->ptszID, xmlnsAdmin, "affiliation", _T("outcast"), JabberIqResultMucGetBanList );
		break;

	case IDM_ADMIN:
		JabberAdminGet( gch->pDest->ptszID, xmlnsOwner, "affiliation", _T("admin"), JabberIqResultMucGetAdminList );
		break;

	case IDM_OWNER:
		JabberAdminGet( gch->pDest->ptszID, xmlnsOwner, "affiliation", _T("owner"), JabberIqResultMucGetOwnerList );
		break;

	case IDM_TOPIC:
		mir_sntprintf( szBuffer, SIZEOF(szBuffer), _T("%s %s"), TranslateT( "Set topic for" ), gch->pDest->ptszID );
		if ( JabberEnterString( szBuffer, SIZEOF(szBuffer))) {
			XmlNode msg( "message" ); msg.addAttr( "to", gch->pDest->ptszID ); msg.addAttr( "type", "groupchat" );
			msg.addChild( "subject", szBuffer );
			JabberSend( jabberThreadInfo->s, msg );
		}
		break;

	case IDM_NICK:
		mir_sntprintf( szBuffer, SIZEOF(szBuffer), _T("%s %s"), TranslateT( "Change nickname in" ), gch->pDest->ptszID );
		if ( JabberEnterString( szBuffer, SIZEOF(szBuffer))) {
			JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, gch->pDest->ptszID );
			if ( item != NULL ) {
				TCHAR text[ 1024 ];
				mir_sntprintf( text, SIZEOF( text ), _T("%s/%s"), gch->pDest->ptszID, szBuffer );
				JabberSendPresenceTo( jabberStatus, text, NULL );
		}	}
		break;

	case IDM_INVITE:
		CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_GROUPCHAT_INVITE ), NULL, JabberGcLogInviteDlgProc, ( LPARAM )gch->pDest->pszID );
		break;

	case IDM_CONFIG:
	{
		int iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetMuc );

		XmlNodeIq iq( "get", iqId, gch->pDest->ptszID );
		XmlNode* query = iq.addQuery( xmlnsOwner );
		JabberSend( jabberThreadInfo->s, iq );
		break;
	}
	case IDM_BOOKMARKS:
	{
		JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_BOOKMARK, gch->pDest->ptszID );
		if ( item == NULL ) {
			item = JabberListGetItemPtr( LIST_CHATROOM, gch->pDest->ptszID );
			if (item != NULL) {
				item->type = _T("conference");
				HANDLE hContact = JabberHContactFromJID( item->jid );
				item->name = ( TCHAR* )JCallService( MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, GCDNF_TCHAR );
				JabberAddEditBookmark(NULL, (LPARAM) item);
			}
		}
		break;
	}
	case IDM_DESTROY:
		mir_sntprintf( szBuffer, SIZEOF(szBuffer), _T("%s %s"), TranslateT( "Reason to destroy" ), gch->pDest->ptszID );
		if ( !JabberEnterString( szBuffer, SIZEOF(szBuffer)))
			break;

		{	XmlNodeIq iq( "set", NOID, gch->pDest->ptszID );
			XmlNode* query = iq.addQuery( xmlnsOwner );
			query->addChild( "destroy" )->addChild( "reason", szBuffer );
			JabberSend( jabberThreadInfo->s, iq );
		}

	case IDM_LEAVE:
		JabberGcQuit( item, 0, 0 );
		break;
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// Sends a private message to a chat user

static void sttSendPrivateMessage( JABBER_LIST_ITEM* item, const TCHAR* nick )
{
	TCHAR szFullJid[ 256 ];
	mir_sntprintf( szFullJid, SIZEOF(szFullJid), _T("%s/%s"), item->jid, nick );
	HANDLE hContact = JabberDBCreateContact( szFullJid, NULL, TRUE, FALSE );
	if ( hContact != NULL ) {
		for ( int i=0; i < item->resourceCount; i++ ) {
			if ( _tcsicmp( item->resource[i].resourceName, nick ) == 0 ) {
				JSetWord( hContact, "Status", item->resource[i].status );
				break;
		}	}

		DBWriteContactSettingByte( hContact, "CList", "Hidden", 1 );
		JSetStringT( hContact, "Nick", nick );
		DBWriteContactSettingDword( hContact, "Ignore", "Mask1", 0 );
		JCallService( MS_MSG_SENDMESSAGE, ( WPARAM )hContact, 0 );
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// General chat event processing hook

int JabberGcEventHook(WPARAM wParam,LPARAM lParam)
{
	GCHOOK* gch = ( GCHOOK* )lParam;
	if ( gch == NULL )
		return 0;

	if ( lstrcmpiA( gch->pDest->pszModule, jabberProtoName ))
		return 0;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, gch->pDest->ptszID );
	if ( item == NULL )
		return 0;

	switch ( gch->pDest->iType ) {
	case GC_USER_MESSAGE:
		if ( gch->pszText && lstrlen( gch->ptszText) > 0 ) {
			rtrim( gch->ptszText );

			if ( jabberOnline ) {
				XmlNode m( "message" ); m.addAttr( "to", item->jid ); m.addAttr( "type", "groupchat" );
				XmlNode* b = m.addChild( "body", gch->ptszText );
				if ( b->sendText != NULL )
					UnEscapeChatTags( b->sendText );
				JabberSend( jabberThreadInfo->s, m );
		}	}
		break;

	case GC_USER_PRIVMESS:
		sttSendPrivateMessage( item, gch->ptszUID );
		break;

	case GC_USER_LOGMENU:
		sttLogListHook( item, gch );
		break;

	case GC_USER_NICKLISTMENU:
		sttNickListHook( item, gch );
		break;

	case GC_USER_CHANMGR:
		int iqId = JabberSerialNext();
		JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetMuc );

		XmlNodeIq iq( "get", iqId, item->jid );
		XmlNode* query = iq.addQuery( xmlnsOwner );
		JabberSend( jabberThreadInfo->s, iq );
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void JabberAddMucListItem( JABBER_MUC_JIDLIST_INFO* jidListInfo, TCHAR* str )
{
	const char* field = _tcschr(str,'@') ? "jid" : "nick";
	TCHAR* roomJid = jidListInfo->roomJid;

	switch (jidListInfo->type) {
	case MUC_VOICELIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "role", _T("participant"));
		JabberAdminGet( roomJid, xmlnsAdmin, "role", _T("participant"), JabberIqResultMucGetVoiceList);
		break;
	case MUC_MEMBERLIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "affiliation", _T("member"));
		JabberAdminGet( roomJid, xmlnsAdmin, "affiliation", _T("member"), JabberIqResultMucGetMemberList);
		break;
	case MUC_MODERATORLIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "role", _T("moderator"));
		JabberAdminGet( roomJid, xmlnsAdmin, "role", _T("moderator"), JabberIqResultMucGetModeratorList);
		break;
	case MUC_BANLIST:
		JabberAdminSet( roomJid, xmlnsAdmin, field, str, "affiliation", _T("outcast"));
		JabberAdminGet( roomJid, xmlnsAdmin, "affiliation", _T("outcast"), JabberIqResultMucGetBanList);
		break;
	case MUC_ADMINLIST:
		JabberAdminSet( roomJid, xmlnsOwner, field, str, "affiliation", _T("admin"));
		JabberAdminGet( roomJid, xmlnsOwner, "affiliation", _T("admin"), JabberIqResultMucGetAdminList);
		break;
	case MUC_OWNERLIST:
		JabberAdminSet( roomJid, xmlnsOwner, field, str, "affiliation", _T("owner"));
		JabberAdminGet( roomJid, xmlnsOwner, "affiliation", _T("owner"), JabberIqResultMucGetOwnerList);
		break;
}	}

void JabberDeleteMucListItem( JABBER_MUC_JIDLIST_INFO* jidListInfo, TCHAR* jid )
{
	TCHAR* roomJid = jidListInfo->roomJid;

	switch ( jidListInfo->type ) {
	case MUC_VOICELIST:		// change role to visitor ( from participant )
		JabberAdminSet( roomJid, xmlnsAdmin, "jid", jid, "role", _T("visitor"));
		break;
	case MUC_BANLIST:		// change affiliation to none ( from outcast )
	case MUC_MEMBERLIST:	// change affiliation to none ( from member )
		JabberAdminSet( roomJid, xmlnsAdmin, "jid", jid, "affiliation", _T("none"));
		break;
	case MUC_MODERATORLIST:	// change role to participant ( from moderator )
		JabberAdminSet( roomJid, xmlnsAdmin, "jid", jid, "role", _T("participant"));
		break;
	case MUC_ADMINLIST:		// change affiliation to member ( from admin )
		JabberAdminSet( roomJid, xmlnsOwner, "jid", jid, "affiliation", _T("member"));
		break;
	case MUC_OWNERLIST:		// change affiliation to admin ( from owner )
		JabberAdminSet( roomJid, xmlnsOwner, "jid", jid, "affiliation", _T("admin"));
		break;
}	}
