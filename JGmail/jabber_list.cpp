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
#include "jabber_list.h"

void JabberMenuUpdateSrmmIcon(JABBER_LIST_ITEM *item);

static int compareListItems( const JABBER_LIST_ITEM* p1, const JABBER_LIST_ITEM* p2 )
{
	if ( p1->list != p2->list )
		return p1->list - p2->list;

	// don't strip text after "/" for Bookmarks because JID contains URL
	if ( p1->list == LIST_BOOKMARK || p1->list == LIST_VCARD_TEMP )
		return lstrcmpi( p1->jid, p2->jid );

	TCHAR szp1[ JABBER_MAX_JID_LEN ], szp2[ JABBER_MAX_JID_LEN ];
	JabberStripJid( p1->jid, szp1, sizeof( szp1 ));
	JabberStripJid( p2->jid, szp2, sizeof( szp2 ));
	return lstrcmpi( szp1, szp2 );
}

static LIST<JABBER_LIST_ITEM> roster( 50, compareListItems );
static CRITICAL_SECTION csLists;

void JabberListInit( void )
{
	InitializeCriticalSection( &csLists );
}

void JabberListUninit( void )
{
	JabberListWipe();
	DeleteCriticalSection( &csLists );
}

/////////////////////////////////////////////////////////////////////////////////////////
// List item freeing

static void JabberListFreeItemInternal( JABBER_LIST_ITEM *item )
{
	if ( item == NULL )
		return;

	if ( item->jid ) mir_free( item->jid );
	if ( item->nick ) mir_free( item->nick );

	JABBER_RESOURCE_STATUS* r = item->resource;
	for ( int i=0; i < item->resourceCount; i++, r++ ) {
		if ( r->resourceName ) mir_free( r->resourceName );
		if ( r->statusMessage ) mir_free( r->statusMessage );
		if ( r->software ) mir_free( r->software );
		if ( r->version ) mir_free( r->version );
		if ( r->system ) mir_free( r->system );
	}
	if ( item->resource ) mir_free( item->resource );
	if ( item->statusMessage ) mir_free( item->statusMessage );
	if ( item->group ) mir_free( item->group );
	if ( item->photoFileName ) {
		DeleteFileA( item->photoFileName );
		mir_free( item->photoFileName );
	}
	if ( item->messageEventIdStr ) mir_free( item->messageEventIdStr );
	if ( item->name ) mir_free( item->name );
	if ( item->type ) mir_free( item->type );
	if ( item->service ) mir_free( item->service );
	if ( item->password ) mir_free( item->password );
	if ( item->list==LIST_ROSTER && item->ft ) delete item->ft;
	mir_free( item );
}

void JabberListWipe( void )
{
	int i;

	EnterCriticalSection( &csLists );
	for( i=0; i < roster.getCount(); i++ )
		JabberListFreeItemInternal( roster[i] );

	roster.destroy();
	LeaveCriticalSection( &csLists );
}

int JabberListExist( JABBER_LIST list, const TCHAR* jid )
{
	JABBER_LIST_ITEM tmp;
	tmp.list = list;
	tmp.jid  = (TCHAR*)jid;

	EnterCriticalSection( &csLists );

	int idx = roster.getIndex( &tmp );
	if ( idx == -1 ) {
		LeaveCriticalSection( &csLists );
		return 0;
	}

	LeaveCriticalSection( &csLists );
	return idx+1;
}

JABBER_LIST_ITEM *JabberListAdd( JABBER_LIST list, const TCHAR* jid )
{
	JABBER_LIST_ITEM* item;

	EnterCriticalSection( &csLists );
	if (( item = JabberListGetItemPtr( list, jid )) != NULL ) {
		LeaveCriticalSection( &csLists );
		return item;
	}

	TCHAR *s = mir_tstrdup( jid );
	// strip resource name if any
	if ( list != LIST_VCARD_TEMP ) {
		TCHAR *p, *q;
		if (( p = _tcschr( s, '@' )) != NULL )
			if (( q = _tcschr( p, '/' )) != NULL )
				*q = '\0';
	}

	item = ( JABBER_LIST_ITEM* )mir_alloc( sizeof( JABBER_LIST_ITEM ));
	ZeroMemory( item, sizeof( JABBER_LIST_ITEM ));
	item->list = list;
	item->jid = s;
	item->status = ID_STATUS_OFFLINE;
	item->resource = NULL;
	item->resourceMode = RSMODE_LASTSEEN;
	item->lastSeenResource = -1;
	item->manualResource = -1;
	if ( list == LIST_ROSTER )
		item->cap = CLIENT_CAP_CHATSTAT;

	roster.insert( item );
	LeaveCriticalSection( &csLists );

	JabberMenuUpdateSrmmIcon(item);
	return item;
}

void JabberListRemove( JABBER_LIST list, const TCHAR* jid )
{
	EnterCriticalSection( &csLists );
	int i = JabberListExist( list, jid );
	if ( i != 0 ) {
		JabberListFreeItemInternal( roster[ --i ] );
		roster.remove( i );
	}
	LeaveCriticalSection( &csLists );
}

void JabberListRemoveList( JABBER_LIST list )
{
	int i = 0;
	while (( i=JabberListFindNext( list, i )) >= 0 )
		JabberListRemoveByIndex( i );
}

void JabberListRemoveByIndex( int index )
{
	EnterCriticalSection( &csLists );
	if ( index >= 0 && index < roster.getCount() ) {
		JabberListFreeItemInternal( roster[index] );
		roster.remove( index );
	}
	LeaveCriticalSection( &csLists );
}

int JabberListAddResource( JABBER_LIST list, const TCHAR* jid, int status, const TCHAR* statusMessage )
{
	int j;
	const TCHAR* p, *q;

	EnterCriticalSection( &csLists );
	int i = JabberListExist( list, jid );
	if ( !i ) {
		LeaveCriticalSection( &csLists );
		return 0;
	}
	JABBER_LIST_ITEM* LI = roster[i-1];

	int bIsNewResource = false;

	if (( p = _tcschr( jid, '@' )) != NULL ) {
		if (( q = _tcschr( p, '/' )) != NULL ) {
			const TCHAR* resource = q+1;
			if ( resource[0] ) {
				JABBER_RESOURCE_STATUS* r = LI->resource;
				for ( j=0; j < LI->resourceCount; j++, r++ ) {
					if ( !_tcscmp( r->resourceName, resource )) {
						// Already exist, update status and statusMessage
						r->status = status;
						replaceStr( r->statusMessage, statusMessage );
						break;
				}	}

				if ( j >= LI->resourceCount ) {
					// Not already exist, add new resource
					LI->resource = ( JABBER_RESOURCE_STATUS * ) mir_realloc( LI->resource, ( LI->resourceCount+1 )*sizeof( JABBER_RESOURCE_STATUS ));
					bIsNewResource = true;
					r = LI->resource + LI->resourceCount++;
					memset( r, 0, sizeof( JABBER_RESOURCE_STATUS ));
					r->status = status;
					r->affiliation = AFFILIATION_NONE;
					r->role = ROLE_NONE;
					r->resourceName = mir_tstrdup( resource );
					if ( statusMessage )
						r->statusMessage = mir_tstrdup( statusMessage );
			}	}
		}
		// No resource, update the main statusMessage
		else replaceStr( LI->statusMessage, statusMessage );
	}

	LeaveCriticalSection( &csLists );

	JabberMenuUpdateSrmmIcon(LI);

	return bIsNewResource;
}

void JabberListRemoveResource( JABBER_LIST list, const TCHAR* jid )
{
	int j;
	const TCHAR* p, *q;

	EnterCriticalSection( &csLists );
	int i = JabberListExist( list, jid );
	JABBER_LIST_ITEM* LI = roster[i-1];
	if ( !i || LI == NULL ) {
		LeaveCriticalSection( &csLists );
		return;
	}

	if (( p = _tcschr( jid, '@' )) != NULL ) {
		if (( q = _tcschr( p, '/' )) != NULL ) {
			const TCHAR* resource = q+1;
			if ( resource[0] ) {
				JABBER_RESOURCE_STATUS* r = LI->resource;
				for ( j=0; j < LI->resourceCount; j++, r++ ) {
					if ( !_tcsicmp( r->resourceName, resource ))
						break;
				}
				if ( j < LI->resourceCount ) {
					// Found last seen resource ID to be removed
					if ( LI->lastSeenResource == j )
						LI->lastSeenResource = -1;
					else if ( LI->lastSeenResource > j )
						LI->lastSeenResource--;
					// update manually selected resource ID
					if (LI->resourceMode == RSMODE_MANUAL)
					{
						if ( LI->manualResource == j )
						{
							LI->resourceMode = RSMODE_LASTSEEN;
							LI->manualResource = -1;
						} else if ( LI->manualResource > j )
							LI->manualResource--;
					}

					// Update MirVer due to possible resource changes
					JabberUpdateMirVer(LI);

					if ( r->resourceName ) mir_free( r->resourceName );
					if ( r->statusMessage ) mir_free( r->statusMessage );
					if ( r->software ) mir_free( r->software );
					if ( r->version ) mir_free( r->version );
					if ( r->system ) mir_free( r->system );
					if ( LI->resourceCount-- == 1 ) {
						mir_free( r );
						LI->resource = NULL;
					}
					else {
						memmove( r, r+1, ( LI->resourceCount-j )*sizeof( JABBER_RESOURCE_STATUS ));
						LI->resource = ( JABBER_RESOURCE_STATUS * )mir_realloc( LI->resource, LI->resourceCount*sizeof( JABBER_RESOURCE_STATUS ));
	}	}	}	}	}

	LeaveCriticalSection( &csLists );

	JabberMenuUpdateSrmmIcon(LI);
}

TCHAR* JabberListGetBestResourceNamePtr( const TCHAR* jid )
{
	TCHAR* res;

	EnterCriticalSection( &csLists );
	int i = JabberListExist( LIST_ROSTER, jid );
	if ( !i ) {
		LeaveCriticalSection( &csLists );
		return NULL;
	}

	JABBER_LIST_ITEM* LI = roster[i-1];
	if ( LI->resourceCount == 1 )
		res = LI->resource[0].resourceName;
	else {
		res = NULL;
		if (LI->resourceMode == RSMODE_LASTSEEN && LI->lastSeenResource>=0 && LI->lastSeenResource < LI->resourceCount)
		{
			res = LI->resource[ LI->lastSeenResource ].resourceName;
		} else
		if (LI->resourceMode == RSMODE_MANUAL && LI->manualResource>=0 && LI->manualResource < LI->resourceCount)
		{
			res = LI->resource[ LI->manualResource ].resourceName;
		}
	}

	LeaveCriticalSection( &csLists );
	return res;
}

TCHAR* JabberListGetBestClientResourceNamePtr( const TCHAR* jid )
{
	EnterCriticalSection( &csLists );
	int i = JabberListExist( LIST_ROSTER, jid );
	if ( !i ) {
		LeaveCriticalSection( &csLists );
		return NULL;
	}

	JABBER_LIST_ITEM* LI = roster[i-1];
	TCHAR* res = JabberListGetBestResourceNamePtr( jid );
	if ( res == NULL ) {
		JABBER_RESOURCE_STATUS* r = LI->resource;
		int status = ID_STATUS_OFFLINE;
		res = NULL;
		for ( i=0; i < LI->resourceCount; i++ ) {
			int s = r[i].status;
			BOOL foundBetter = FALSE;
			switch ( s ) {
			case ID_STATUS_FREECHAT:
				foundBetter = TRUE;
				break;
			case ID_STATUS_ONLINE:
				if ( status != ID_STATUS_FREECHAT )
					foundBetter = TRUE;
				break;
			case ID_STATUS_DND:
				if ( status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE )
					foundBetter = TRUE;
				break;
			case ID_STATUS_AWAY:
				if ( status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE && status != ID_STATUS_DND )
					foundBetter = TRUE;
				break;
			case ID_STATUS_NA:
				if ( status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE && status != ID_STATUS_DND && status != ID_STATUS_AWAY )
					foundBetter = TRUE;
				break;
			}
			if ( foundBetter ) {
				res = r[i].resourceName;
#ifndef ORGINALRESOURCEMANAGMENT
				//LI->defaultResource = i; // this line might be buggy!!!
#endif
				status = s;
	}	}	}

	LeaveCriticalSection( &csLists );
	return res;
}

int JabberListFindNext( JABBER_LIST list, int fromOffset )
{
	EnterCriticalSection( &csLists );
	int i = ( fromOffset >= 0 ) ? fromOffset : 0;
	for( ; i<roster.getCount(); i++ )
		if ( roster[i]->list == list ) {
		  	LeaveCriticalSection( &csLists );
			return i;
		}
	LeaveCriticalSection( &csLists );
	return -1;
}

JABBER_LIST_ITEM *JabberListGetItemPtr( JABBER_LIST list, const TCHAR* jid )
{
	EnterCriticalSection( &csLists );
	int i = JabberListExist( list, jid );
	if ( !i ) {
		LeaveCriticalSection( &csLists );
		return NULL;
	}
	i--;
	LeaveCriticalSection( &csLists );
	return roster[i];
}

JABBER_LIST_ITEM *JabberListGetItemPtrFromIndex( int index )
{
	EnterCriticalSection( &csLists );
	if ( index >= 0 && index < roster.getCount() ) {
		LeaveCriticalSection( &csLists );
		return roster[index];
	}
	LeaveCriticalSection( &csLists );
	return NULL;
}

void putResUserSett(HANDLE hContact, JABBER_RESOURCE_STATUS *r){
#define LOG_PRUS 1
#ifdef LOG_PRUS
	DBVARIANT dbv;
	int res = JGetStringT( hContact, "jid", &dbv );
	JabberLog(
		"Updating contact "TCHAR_STR_PARAM":\nResource: "TCHAR_STR_PARAM"\nSoftware: "TCHAR_STR_PARAM"\nVersion: "TCHAR_STR_PARAM"\nSystem: "TCHAR_STR_PARAM,
		res?_T("buggy_jid"):dbv.ptszVal, r->resourceName, r->software, r->version, r->system);
	if (!res) JFreeVariant(&dbv);
#endif
	if (!hContact) {
		JabberLog(
			"Trying to update NULL contact with resource "TCHAR_STR_PARAM,
			r->resourceName);
		return;
	}
	TCHAR mirver[256];
	int pos=0;
	JSetStringT( hContact, "Resource", r->resourceName );
	TCHAR *p=r->software?_tcsstr( r->software, _T("Miranda IM") ):NULL;
	if (r->software){
		pos = mir_sntprintf(mirver,255,
			_T("%s"),
			p?p:r->software);
		if ((p!=NULL) && ( (mirver[pos-1] == _T(')')) || (mirver[pos-1] == _T(' ')) ) ) pos--;
		mirver[pos]='\0';
	}
	if (r->version) pos += mir_sntprintf(mirver+pos,255-pos,
		_T(" (%s)"),
		r->version);
	if (pos) {
		JSetStringT( hContact, "MirVer", mirver );
	} else JSetStringT( hContact, "MirVer", r->resourceName );
	if (r->system){
		JSetStringT( hContact, "System", r->system );
	} else JDeleteSetting( hContact, "System" );
}
