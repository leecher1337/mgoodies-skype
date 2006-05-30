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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_xml.h,v $
Revision       : $Revision: 1.9 $
Last change on : $Date: 2006/05/12 20:13:35 $
Last change by : $Author: ghazan $

*/

#ifndef _JABBER_XML_H_
#define _JABBER_XML_H_

#define NOID (-1)

typedef enum { ELEM_OPEN, ELEM_CLOSE, ELEM_OPENCLOSE, ELEM_TEXT } XmlElemType;
typedef enum { NODE_OPEN, NODE_CLOSE } XmlNodeType;

struct XmlAttr
{
	XmlAttr();
	XmlAttr( const char* pszName, const TCHAR* ptszValue );
	#if defined( _UNICODE )
		XmlAttr( const char* pszName, const char* ptszValue );
	#endif
	~XmlAttr();

	char* name;
	union {
		TCHAR* value;
		char*  sendValue;
	};
};

struct XmlNode
{
	XmlNode( const char* name );
	XmlNode( const char* pszName, const TCHAR* ptszText );
	#if defined( _UNICODE )
		XmlNode( const char* pszName, const char* ptszText );
	#endif
	~XmlNode();

	XmlAttr* addAttr( XmlAttr* );
	XmlAttr* addAttr( const char* pszName, const TCHAR* ptszValue );
	#if defined( _UNICODE )
		XmlAttr* addAttr( const char* pszName, const char* pszValue );
	#endif
	XmlAttr* addAttr( const char* pszName, int value );
	XmlAttr* addAttrID( int id );

	XmlNode* addChild( XmlNode* );
	XmlNode* addChild( const char* pszName );
	XmlNode* addChild( const char* pszName, const TCHAR* ptszValue );
	#if defined( _UNICODE )
		XmlNode* addChild( const char* pszName, const char* pszValue );
	#endif

	XmlNode* addQuery( const char* szNameSpace );

	int   getTextLen() const;
	char* getText() const;

	int depth;									// depth of the current node ( 1=root )
	char* name;									// tag name of the current node
	union {
		TCHAR* text;
		char*  sendText;
	};
	int numAttr;								// number of attributes
	int maxNumAttr;							// internal use ( num of slots currently allocated to attr )
	XmlAttr **attr;							// attribute list
	int numChild;								// number of direct child nodes
	int maxNumChild;							// internal use ( num of slots currently allocated to child )
	XmlNode **child;							// child node list
	XmlNodeType state;						// internal use by parser
	char* props;
	BOOL dirtyHack;						// to allow generator to issue the unclosed tag
};

struct XmlNodeIq : public XmlNode
{
	XmlNodeIq( const char* type, int id = NOID, const TCHAR* to = NULL );
	XmlNodeIq( const char* type, const TCHAR* idStr, const TCHAR* to );
	#if defined( _UNICODE )
		XmlNodeIq( const char* type, int id, const char* to );
	#endif
};

typedef void ( *JABBER_XML_CALLBACK )( XmlNode*, void* );

struct XmlState
{
	XmlState() : root(NULL) {}

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
TCHAR* JabberXmlGetAttrValue( XmlNode *node, char* key );
XmlNode *JabberXmlGetChild( XmlNode *node, char* tag );
XmlNode *JabberXmlGetNthChild( XmlNode *node, char* tag, int nth );
XmlNode *JabberXmlGetChildWithGivenAttrValue( XmlNode *node, char* tag, char* attrKey, TCHAR* attrValue );
void JabberXmlDumpAll( XmlState *xmlState );
void JabberXmlDumpNode( XmlNode *node );
XmlNode *JabberXmlCopyNode( XmlNode *node );
BOOL JabberXmlSetCallback( XmlState *xmlState, int depth, XmlElemType type, JABBER_XML_CALLBACK callback, void *userdata );

XmlNode *JabberXmlCreateNode( char* name );
void JabberXmlAddAttr( XmlNode *n, char* name, char* value );
XmlNode *JabberXmlAddChild( XmlNode *n, char* name );

inline XmlNode& operator+( XmlNode& n1, XmlNode& n2 )
{	n1.addChild( &n2 );
	return n1;
}

inline XmlNode& operator+( XmlNode& n, XmlAttr& a )
{	n.addAttr( &a );
	return n;
}

#endif
