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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_iq.cpp,v $
Revision       : $Revision: 1.7 $
Last change on : $Date: 2006/05/12 20:13:35 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_iq.h"
#include "jabber_xmlns.h"

static JABBER_IQ_XMLNS_FUNC jabberXmlns[] = {
	{ _T("http://jabber.org/protocol/disco"), JabberXmlnsDisco, TRUE },
	{ _T("jabber:iq:browse"), JabberXmlnsBrowse, FALSE }
};

typedef struct {
	int iqId;					// id to match IQ get/set with IQ result
	JABBER_IQ_PROCID procId;	// must be unique in the list, except for IQ_PROC_NONE which can have multiple entries
	JABBER_IQ_PFUNC func;		// callback function
	time_t requestTime;			// time the request was sent, used to remove relinquent entries
} JABBER_IQ_FUNC;

static CRITICAL_SECTION csIqList;
static JABBER_IQ_FUNC *iqList;
static int iqCount;
static int iqAlloced;

void JabberIqInit()
{
	InitializeCriticalSection( &csIqList );
	iqList = NULL;
	iqCount = 0;
	iqAlloced = 0;
}

void JabberIqUninit()
{
	if ( iqList ) mir_free( iqList );
	iqList = NULL;
	iqCount = 0;
	iqAlloced = 0;
	DeleteCriticalSection( &csIqList );
}

static void JabberIqRemove( int index )
{
	EnterCriticalSection( &csIqList );
	if ( index>=0 && index<iqCount ) {
		memmove( iqList+index, iqList+index+1, sizeof( JABBER_IQ_FUNC )*( iqCount-index-1 ));
		iqCount--;
	}
	LeaveCriticalSection( &csIqList );
}

static void JabberIqExpire()
{
	int i;
	time_t expire;

	EnterCriticalSection( &csIqList );
	expire = time( NULL ) - 120;	// 2 minute
	i = 0;
	while ( i < iqCount ) {
		if ( iqList[i].requestTime < expire )
			JabberIqRemove( i );
		else
			i++;
	}
	LeaveCriticalSection( &csIqList );
}

JABBER_IQ_PFUNC JabberIqFetchFunc( int iqId )
{
	int i;
	JABBER_IQ_PFUNC res;

	EnterCriticalSection( &csIqList );
	JabberIqExpire();
#ifdef _DEBUG
	for ( i=0; i<iqCount; i++ )
		JabberLog( "  %04d : %02d : 0x%x", iqList[i].iqId, iqList[i].procId, iqList[i].func );
#endif
	for ( i=0; i<iqCount && iqList[i].iqId!=iqId; i++ );
	if ( i < iqCount ) {
		res = iqList[i].func;
		JabberIqRemove( i );
	}
	else {
		res = ( JABBER_IQ_PFUNC ) NULL;
	}
	LeaveCriticalSection( &csIqList );
	return res;
}

void JabberIqAdd( unsigned int iqId, JABBER_IQ_PROCID procId, JABBER_IQ_PFUNC func )
{
	int i;

	EnterCriticalSection( &csIqList );
	JabberLog( "IqAdd id=%d, proc=%d, func=0x%x", iqId, procId, func );
	if ( procId == IQ_PROC_NONE )
		i = iqCount;
	else
		for ( i=0; i<iqCount && iqList[i].procId!=procId; i++ );

	if ( i>=iqCount && iqCount>=iqAlloced ) {
		iqAlloced = iqCount + 8;
		iqList = ( JABBER_IQ_FUNC * )mir_realloc( iqList, sizeof( JABBER_IQ_FUNC )*iqAlloced );
	}

	if ( iqList != NULL ) {
		iqList[i].iqId = iqId;
		iqList[i].procId = procId;
		iqList[i].func = func;
		iqList[i].requestTime = time( NULL );
		if ( i == iqCount ) iqCount++;
	}
	LeaveCriticalSection( &csIqList );
}

JABBER_IQ_PFUNC JabberIqFetchXmlnsFunc( TCHAR* xmlns )
{
	unsigned int len, count, i;
	TCHAR* p, *q;

	if ( xmlns == NULL )
		return NULL;

	p = _tcsrchr( xmlns, '/' );
	q = _tcsrchr( xmlns, '#' );
	if ( p!=NULL && q!=NULL && q>p )
		len = q - xmlns;
	else
		len = _tcslen( xmlns );

	count = sizeof( jabberXmlns ) / sizeof( jabberXmlns[0] );
	for ( i=0; i<count; i++ ) {
		if ( jabberXmlns[i].allowSubNs ) {
			if ( _tcslen( jabberXmlns[i].xmlns ) == len && !_tcsncmp( jabberXmlns[i].xmlns, xmlns, len ))
				break;
		}
		else {
			if ( !_tcscmp( jabberXmlns[i].xmlns, xmlns ))
				break;
		}
	}

	if ( i < count )
		return jabberXmlns[i].func;

	return NULL;
}
