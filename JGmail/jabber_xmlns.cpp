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

/////////////////////////////////////////////////////////////////////////////////////////
// JabberXmlnsBrowse

void JabberXmlnsBrowse( XmlNode *iqNode, void *userdata )
{
	XmlNode *queryNode;
	TCHAR *xmlns, *iqFrom, *iqType;

	if ( iqNode == NULL ) return;
	if (( iqFrom=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;
	if (( iqType=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;
	if (( xmlns=JabberXmlGetAttrValue( queryNode, "xmlns" )) == NULL ) return;

	if ( !_tcscmp( iqType, _T("get"))) {
		XmlNodeIq iq( "result", JabberGetPacketID( iqNode ), iqFrom );
		XmlNode* user = iq.addChild( "user" ); user->addAttr( "jid", jabberJID ); user->addAttr( "type", "client" ); user->addAttr( "xmlns", xmlns );
		user->addChild( "ns", "http://jabber.org/protocol/disco#info" );
		user->addChild( "ns", "http://jabber.org/protocol/muc" );
		user->addChild( "ns", "jabber:iq:agents" );
		user->addChild( "ns", "jabber:iq:browse" );
		user->addChild( "ns", "jabber:iq:oob" );
		user->addChild( "ns", "jabber:iq:version" );
		user->addChild( "ns", "jabber:x:data" );
		user->addChild( "ns", "jabber:x:event" );
		user->addChild( "ns", "vcard-temp" );
		jabberThreadInfo->send( iq );
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberXmlnsDisco

static void sttAddFeature( XmlNode* n, char* text )
{
	XmlNode* f = n->addChild( "feature" ); f->addAttr( "var", text );
}

void JabberXmlnsDisco( XmlNode *iqNode, void *userdata )
{
	XmlNode *queryNode;
	TCHAR *xmlns, *p, *discoType;
	TCHAR *iqFrom, *iqType;

	if ( iqNode == NULL ) return;
	if (( iqFrom = JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;
	if (( iqType = JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( queryNode = JabberXmlGetChild( iqNode, "query" )) == NULL ) return;
	if (( xmlns = JabberXmlGetAttrValue( queryNode, "xmlns" )) == NULL ) return;

	p = _tcsrchr( xmlns, '/' );
	discoType = _tcsrchr( xmlns, '#' );

	if ( p==NULL || discoType==NULL || discoType < p )
		return;

	if ( !_tcscmp( iqType, _T("get"))) {
		XmlNodeIq iq( "result", JabberGetPacketID( iqNode ), iqFrom );

		if ( !_tcscmp( discoType, _T("#info"))) {
			XmlNode* query = iq.addChild( "query" ); query->addAttr( "xmlns", xmlns );
			XmlNode* ident = query->addChild( "identity" ); ident->addAttr( "category", "user" );
			ident->addAttr( "type", "client" ); ident->addAttr( "name", "Miranda" );
			sttAddFeature( query, "http://jabber.org/protocol/disco#info" );
			sttAddFeature( query, "http://jabber.org/protocol/muc" );
			sttAddFeature( query, "http://jabber.org/protocol/si" );
			sttAddFeature( query, "http://jabber.org/protocol/si/profile/file-transfer" );
			sttAddFeature( query, "http://jabber.org/protocol/bytestreams" );
			sttAddFeature( query, "http://jabber.org/protocol/chatstates" );
			sttAddFeature( query, "jabber:iq:agents" );
			sttAddFeature( query, "jabber:iq:browse" );
			sttAddFeature( query, "jabber:iq:oob" );
			sttAddFeature( query, "jabber:iq:version" );
			sttAddFeature( query, "jabber:x:data" );
			sttAddFeature( query, "jabber:x:event" );
			sttAddFeature( query, "vcard-temp" );
		}
		jabberThreadInfo->send( iq );
}	}
