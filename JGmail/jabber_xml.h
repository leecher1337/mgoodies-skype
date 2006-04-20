/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-2004  Santithorn Bunchua

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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_xml.h,v $
Revision       : $Revision: 1.7 $
Last change on : $Date: 2005/11/20 20:30:20 $
Last change by : $Author: ghazan $

*/

#ifndef _JABBER_XML_H_
#define _JABBER_XML_H_

typedef enum { ELEM_OPEN, ELEM_CLOSE, ELEM_OPENCLOSE, ELEM_TEXT } XmlElemType;
typedef enum { NODE_OPEN, NODE_CLOSE } XmlNodeType;

struct XmlAttr
{
	char* name;
	char* value;
};

struct  XmlNode
{
	int depth;									// depth of the current node ( 1=root )
	char* name;									// tag name of the current node
	int numAttr;								// number of attributes
	int maxNumAttr;							// internal use ( num of slots currently allocated to attr )
	XmlAttr **attr;							// attribute list
	int numChild;								// number of direct child nodes
	int maxNumChild;							// internal use ( num of slots currently allocated to child )
	XmlNode **child;							// child node list
	char* text;
	XmlNodeType state;						// internal use by parser
};

typedef void ( *JABBER_XML_CALLBACK )( XmlNode*, void* );

struct XmlState
{
	XmlNode root;			// root is the document ( depth = 0 );
	// callback for depth=n element on opening/closing
	JABBER_XML_CALLBACK callback1_open;
	JABBER_XML_CALLBACK callback1_close;
	JABBER_XML_CALLBACK callback2_open;
	JABBER_XML_CALLBACK callback2_close;
	void *userdata1_open;
	void *userdata1_close;
	void *userdata2_open;
	void *userdata2_close;
};

void JabberXmlInitState( XmlState *xmlState );
void JabberXmlDestroyState( XmlState *xmlState );
BOOL JabberXmlSetCallback( XmlState *xmlState, int depth, XmlElemType type, void ( *callback )(), void *userdata );
int JabberXmlParse( XmlState *xmlState, char* buffer, int datalen );
char* JabberXmlGetAttrValue( XmlNode *node, char* key );
XmlNode *JabberXmlGetChild( XmlNode *node, char* tag );
XmlNode *JabberXmlGetNthChild( XmlNode *node, char* tag, int nth );
XmlNode *JabberXmlGetChildWithGivenAttrValue( XmlNode *node, char* tag, char* attrKey, char* attrValue );
void JabberXmlFreeNode( XmlNode *node );
void JabberXmlDumpAll( XmlState *xmlState );
void JabberXmlDumpNode( XmlNode *node );
XmlNode *JabberXmlCopyNode( XmlNode *node );
BOOL JabberXmlSetCallback( XmlState *xmlState, int depth, XmlElemType type, JABBER_XML_CALLBACK callback, void *userdata );

#ifdef _DEBUG
XmlNode *JabberXmlCreateNode( char* name );
void JabberXmlAddAttr( XmlNode *n, char* name, char* value );
XmlNode *JabberXmlAddChild( XmlNode *n, char* name );
void JabberXmlAddText( XmlNode *n, char* text );
#endif

#endif
