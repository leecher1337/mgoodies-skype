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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_xmlns.cpp,v $
Revision       : $Revision: 1.9 $
Last change on : $Date: 2005/11/20 20:30:20 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"

void JabberXmlnsBrowse( XmlNode *iqNode, void *userdata )
{
	XmlNode *queryNode;
	char* xmlns, *iqFrom, *iqType, *iqId;
	char idStr[64];

	if ( iqNode == NULL ) return;
	if (( iqFrom=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;
	if (( iqType=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( queryNode=JabberXmlGetChild( iqNode, "query" )) == NULL ) return;
	if (( xmlns=JabberXmlGetAttrValue( queryNode, "xmlns" )) == NULL ) return;

	if (( iqId=JabberXmlGetAttrValue( iqNode, "id" )) == NULL )
		idStr[0] = '\0';
	else
        mir_snprintf( idStr, sizeof( idStr ), " id=\"%s\"", iqId );

	if ( !strcmp( iqType, "get" )) {
        JabberSend( jabberThreadInfo->s, "<iq type=\"result\" to=\"%s\"%s>"
            "<user jid=\"%s\" type=\"client\" name=\"Miranda\" xmlns=\"%s\">"
			"<ns>http://jabber.org/protocol/disco#info</ns>"
			"<ns>http://jabber.org/protocol/muc</ns>"
			"<ns>jabber:iq:agents</ns>"
			"<ns>jabber:iq:browse</ns>"
			"<ns>jabber:iq:oob</ns>"
			"<ns>jabber:iq:version</ns>"
			"<ns>jabber:x:data</ns>"
			"<ns>jabber:x:event</ns>"
			"<ns>vcard-temp</ns>"
			"</user></iq>", iqFrom, idStr, jabberJID, xmlns );
}	}

void JabberXmlnsDisco( XmlNode *iqNode, void *userdata )
{
	XmlNode *queryNode;
	char* xmlns, *p, *discoType;
	char* iqFrom, *iqType, *iqId;
	char idStr[64];

	if ( iqNode == NULL ) return;
	if (( iqFrom = JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;
	if (( iqType = JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( queryNode = JabberXmlGetChild( iqNode, "query" )) == NULL ) return;
	if (( xmlns = JabberXmlGetAttrValue( queryNode, "xmlns" )) == NULL ) return;

	//JabberUrlDecode( iqFrom ); this fsck-up the output xml stream

    p = strrchr( xmlns, '/' );
	discoType = strrchr( xmlns, '#' );

	if ( p==NULL || discoType==NULL || discoType<p ) return;

	if (( iqId=JabberXmlGetAttrValue( iqNode, "id" )) == NULL )
		idStr[0] = '\0';
	else
        mir_snprintf( idStr, sizeof( idStr ), " id=\"%s\"", iqId );

	if ( !strcmp( iqType, "get" )) {
		if ( !strcmp( discoType, "#info" )) {
            JabberSend( jabberThreadInfo->s, "<iq type=\"result\" to=\"%s\"%s><query xmlns=\"%s\">"
                "<identity category=\"user\" type=\"client\" name=\"Miranda\"/>"
                "<feature var=\"http://jabber.org/protocol/disco#info\"/>"
                "<feature var=\"http://jabber.org/protocol/muc\"/>"
                "<feature var=\"http://jabber.org/protocol/si\"/>"
                "<feature var=\"http://jabber.org/protocol/si/profile/file-transfer\"/>"
                "<feature var=\"http://jabber.org/protocol/bytestreams\"/>"
                "<feature var=\"jabber:iq:agents\"/>"
                "<feature var=\"jabber:iq:browse\"/>"
                "<feature var=\"jabber:iq:oob\"/>"
                "<feature var=\"jabber:iq:version\"/>"
                "<feature var=\"jabber:x:data\"/>"
                "<feature var=\"jabber:x:event\"/>"
                "<feature var=\"vcard-temp\"/>"
				"</query></iq>", iqFrom, idStr, xmlns );
		}
        else JabberSend( jabberThreadInfo->s, "<iq type=\"result\"%s><query xmlns=\"%s\"/></iq>", idStr, xmlns );
}	}
