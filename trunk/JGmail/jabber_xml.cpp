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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_xml.cpp,v $
Revision       : $Revision: 1.8 $
Last change on : $Date: 2005/11/20 20:30:20 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"

static BOOL JabberXmlProcessElem( XmlState *xmlState, XmlElemType elemType, char* elemText, char* elemAttr );
static void JabberXmlRemoveChild( XmlNode *node, XmlNode *child );

void JabberXmlInitState( XmlState *xmlState )
{
	if ( xmlState == NULL ) return;
	xmlState->root.name = NULL;
	xmlState->root.depth = 0;
	xmlState->root.numAttr = 0;
	xmlState->root.maxNumAttr = 0;
	xmlState->root.attr = NULL;
	xmlState->root.numChild = 0;
	xmlState->root.maxNumChild = 0;
	xmlState->root.child = NULL;
	xmlState->root.text = NULL;
	xmlState->root.state = NODE_OPEN;
	xmlState->callback1_open = NULL;
	xmlState->callback1_close = NULL;
	xmlState->callback2_open = NULL;
	xmlState->callback2_close = NULL;
	xmlState->userdata1_open = NULL;
	xmlState->userdata1_close = NULL;
	xmlState->userdata2_open = NULL;
	xmlState->userdata2_close = NULL;
}

void JabberXmlDestroyState( XmlState *xmlState )
{
	int i;
	XmlNode *node;

	if ( xmlState == NULL ) return;
	// Note: cannot use JabberXmlFreeNode() to mir_free xmlState->root
	// because it will do mir_free( xmlState->root ) which is not freeable.
	node = &( xmlState->root );

	// Free all children first
	for ( i=0; i<node->numChild; i++ )
		delete node->child[i];
	if ( node->child ) mir_free( node->child );

	// Free all attributes
	for ( i=0; i<node->numAttr; i++ )
		delete node->attr[i];
	if ( node->attr ) mir_free( node->attr );

	// Free string field
	if ( node->text ) mir_free( node->text );
	if ( node->name ) mir_free( node->name );

	memset( xmlState, 0, sizeof( XmlState ));
}

BOOL JabberXmlSetCallback( XmlState *xmlState, int depth, XmlElemType type, JABBER_XML_CALLBACK callback, void *userdata )
{
	if ( depth==1 && type==ELEM_OPEN ) {
		xmlState->callback1_open = callback;
		xmlState->userdata1_open = userdata;
	}
	else if ( depth==1 && type==ELEM_CLOSE ) {
		xmlState->callback1_close = callback;
		xmlState->userdata1_close = userdata;
	}
	else if ( depth==2 && type==ELEM_OPEN ) {
		xmlState->callback2_open = callback;
		xmlState->userdata2_open = userdata;
	}
	else if ( depth==2 && type==ELEM_CLOSE ) {
		xmlState->callback2_close = callback;
		xmlState->userdata2_close = userdata;
	}
	else
		return FALSE;

	return TRUE;
}

#define TAG_MAX_LEN 50
#define ATTR_MAX_LEN 1024
int JabberXmlParse( XmlState *xmlState, char* buffer, int datalen )
{
	char* p, *q, *r, *eob;
	char* str;
	int num;
	char tag[TAG_MAX_LEN];
	char attr[ATTR_MAX_LEN];
	XmlElemType elemType;

	eob = buffer + datalen;
	num = 0;
	// Skip leading whitespaces
	for ( p=buffer; p<eob && isspace( BYTE( *p )); p++,num++ );
	while ( num < datalen ) {
		if ( *p == '<' ) {	// found starting bracket
			for ( q=p+1; q<eob && *q!='>'; q++ );
			if ( q < eob ) {	// found closing bracket
				for ( r=p+1; *r!='>' && *r!=' ' && *r!='\t'; r++ );
				if ( r-( p+1 ) > TAG_MAX_LEN ) {
					JabberLog( "TAG_MAX_LEN too small, ignore current tag" );
				}
				else {
					if ( *( p+1 ) == '/' ) {	// closing tag
						strncpy( tag, p+2, r-( p+2 ));
						tag[r-( p+2 )] = '\0';
						elemType = ELEM_CLOSE;
					}
					else {
						if ( *( r-1 ) == '/' ) {	// single open/close tag
							strncpy( tag, p+1, r-( p+1 )-1 );
							tag[r-( p+1 )-1] = '\0';
							elemType = ELEM_OPENCLOSE;
						}
						else {
							strncpy( tag, p+1, r-( p+1 ));
							tag[r-( p+1 )] = '\0';
							elemType = ELEM_OPEN;
						}
					}
					for ( ;r<q && ( *r==' ' || *r=='\t' ); r++ );
					if ( q-r > ATTR_MAX_LEN ) {
						JabberLog( "ATTR_MAX_LEN too small, ignore current tag" );
					}
					else {
						strncpy( attr, r, q-r );
						if (( q-r )>0 && attr[q-r-1]=='/' ) {
							attr[q-r-1] = '\0';
							elemType = ELEM_OPENCLOSE;
						}
						else
							attr[q-r] = '\0';
						JabberXmlProcessElem( xmlState, elemType, tag, attr );
					}
				}
				num += ( q-p+1 );
				p = q + 1;
				if ( elemType==ELEM_CLOSE || elemType==ELEM_OPENCLOSE ) {
					// Skip whitespaces after end tags
					for ( ; p<eob && isspace( BYTE( *p )); p++,num++ );
				}
			}
			else
				break;
		}
		else {	// found inner text
			for ( q=p+1; q<eob && *q!='<'; q++ );
			if ( q < eob ) {	// found starting bracket of the next element
				str = ( char* )mir_alloc( q-p+1 );
				strncpy( str, p, q-p );
				str[q-p] = '\0';
				JabberXmlProcessElem( xmlState, ELEM_TEXT, str, NULL );
				mir_free( str );
				num += ( q-p );
				p = q;
			}
			else
				break;
		}
	}

	return num;
}

static void JabberXmlParseAttr( XmlNode *node, char* text )
{
	char* kstart, *vstart;
	int klen, vlen;
	char* p;
	XmlAttr *a;

	if ( node==NULL || text==NULL || strlen( text )<=0 )
		return;

	for ( p=text;; ) {

		// Skip leading whitespaces
		for ( ;*p!='\0' && ( *p==' ' || *p=='\t' ); p++ );
		if ( *p == '\0' )
			break;

		// Fetch key
		kstart = p;
		for ( ;*p!='\0' && *p!='=' && *p!=' ' && *p!='\t'; p++ );
		klen = p-kstart;

		if ( node->numAttr >= node->maxNumAttr ) {
			node->maxNumAttr = node->numAttr + 20;
			node->attr = ( XmlAttr ** ) mir_realloc( node->attr, node->maxNumAttr*sizeof( XmlAttr * ));
		}
		a = node->attr[node->numAttr] = new XmlAttr();
		node->numAttr++;

		// Skip possible whitespaces between key and '='
		for ( ;*p!='\0' && ( *p==' ' || *p=='\t' ); p++ );

		if ( *p == '\0' ) {
			a->name = ( char* )mir_alloc( klen+1 );
			strncpy( a->name, kstart, klen );
			a->name[klen] = '\0';
			a->value = mir_tstrdup( _T(""));
			break;
		}

		if ( *p != '=' ) {
			a->name = ( char* )mir_alloc( klen+1 );
			strncpy( a->name, kstart, klen );
			a->name[klen] = '\0';
			a->value = mir_tstrdup( _T(""));
			continue;
		}

		// Found '='
		p++;

		// Skip possible whitespaces between '=' and value
		for ( ;*p!='\0' && ( *p==' ' || *p=='\t' ); p++ );

		if ( *p == '\0' ) {
			a->name = ( char* )mir_alloc( klen+1 );
			strncpy( a->name, kstart, klen );
			a->name[klen] = '\0';
			a->value = mir_tstrdup( _T(""));
			break;
		}

		// Fetch value
		if ( *p=='\'' || *p=='"' ) {
			p++;
			vstart = p;
			for ( ;*p!='\0' && *p!=*( vstart-1 ); p++ );
			vlen = p-vstart;
			if ( *p != '\0' ) p++;
		}
		else {
			vstart = p;
			for ( ;*p!='\0' && *p!=' ' && *p!='\t'; p++ );
			vlen = p-vstart;
		}

		a->name = ( char* )mir_alloc( klen+1 );
		strncpy( a->name, kstart, klen );
		a->name[klen] = '\0';

		JabberUtfToTchar( vstart, vlen, a->value );
	}
}

static BOOL JabberXmlProcessElem( XmlState *xmlState, XmlElemType elemType, char* elemText, char* elemAttr )
{
	XmlNode *node, *parentNode, *n;
	BOOL activateCallback = FALSE;
	char* text, *attr;

	if ( elemText == NULL ) return FALSE;

	if ( elemType==ELEM_OPEN && !strcmp( elemText, "?xml" )) {
		JabberLog( "XML: skip <?xml> tag" );
		return TRUE;
	}

	// Find active node
	node = &( xmlState->root );
	parentNode = NULL;
	while ( node->numChild>0 && node->child[node->numChild-1]->state==NODE_OPEN ) {
		parentNode = node;
		node = node->child[node->numChild-1];
	}

	if ( node->state != NODE_OPEN ) return FALSE;

	text = NEWSTR_ALLOCA( elemText );

	if ( elemAttr )
		attr = mir_strdup( elemAttr );
	else
		attr = NULL;

	switch ( elemType ) {
	case ELEM_OPEN:
		if ( node->numChild >= node->maxNumChild ) {
			node->maxNumChild = node->numChild + 20;
			node->child = ( XmlNode ** ) mir_realloc( node->child, node->maxNumChild*sizeof( XmlNode * ));
		}
		n = node->child[node->numChild] = new XmlNode(text);
		node->numChild++;
		n->depth = node->depth + 1;
		n->state = NODE_OPEN;
		n->numChild = n->maxNumChild = 0;
		n->child = NULL;
		n->numAttr = n->maxNumAttr = 0;
		n->attr = NULL;
		JabberXmlParseAttr( n, attr );
		n->text = NULL;
		if ( n->depth==1 && xmlState->callback1_open!=NULL )
			( *( xmlState->callback1_open ))( n, xmlState->userdata1_open );
		if ( n->depth==2 && xmlState->callback2_open!=NULL )
			( *xmlState->callback2_open )( n, xmlState->userdata2_open );
		break;
	case ELEM_OPENCLOSE:
		if ( node->numChild >= node->maxNumChild ) {
			node->maxNumChild = node->numChild + 20;
			node->child = ( XmlNode ** ) mir_realloc( node->child, node->maxNumChild*sizeof( XmlNode * ));
		}
		n = node->child[node->numChild] = new XmlNode( text );
		node->numChild++;
		n->depth = node->depth + 1;
		n->state = NODE_CLOSE;
		n->numChild = n->maxNumAttr = 0;
		n->child = NULL;
		n->numAttr = n->maxNumAttr = 0;
		n->attr = NULL;
		JabberXmlParseAttr( n, attr );
		n->text = NULL;
		if ( n->depth==1 && xmlState->callback1_close!=NULL ) {
			( *( xmlState->callback1_close ))( n, xmlState->userdata1_close );
			JabberXmlRemoveChild( node, n );
		}
		if ( n->depth==2 && xmlState->callback2_close!=NULL ) {
			( *xmlState->callback2_close )( n, xmlState->userdata2_close );
			JabberXmlRemoveChild( node, n );
		}
		break;
	case ELEM_CLOSE:
		if ( node->name!=NULL && !strcmp( node->name, text )) {
			node->state = NODE_CLOSE;
			if ( node->depth==1 && xmlState->callback1_close!=NULL ) {
				( *( xmlState->callback1_close ))( node, xmlState->userdata1_close );
				JabberXmlRemoveChild( parentNode, node );
			}
			else if ( node->depth==2 && xmlState->callback2_close!=NULL ) {
				( *xmlState->callback2_close )( node, xmlState->userdata2_close );
				JabberXmlRemoveChild( parentNode, node );
		}	}
		else {
			JabberLog( "XML: Closing </%s> without opening tag", text );
			if ( attr ) mir_free( attr );
			return FALSE;
		}
		break;
	case ELEM_TEXT:
		JabberUtfToTchar( text, strlen( text ), node->text );
		break;
	default:
		if ( attr ) mir_free( attr );
		return FALSE;
	}

	if ( attr ) mir_free( attr );

	return TRUE;
}

TCHAR* JabberXmlGetAttrValue( XmlNode *node, char* key )
{
	if ( node==NULL || node->numAttr<=0 || key==NULL || strlen( key )<=0 )
		return NULL;

	for ( int i=0; i<node->numAttr; i++ )
		if ( !lstrcmpA( key, node->attr[i]->name ))
			return node->attr[i]->value;

	return NULL;
}

XmlNode *JabberXmlGetChild( XmlNode *node, char* tag )
{
	return JabberXmlGetNthChild( node, tag, 1 );
}

XmlNode *JabberXmlGetNthChild( XmlNode *node, char* tag, int nth )
{
	int i, num;

	if ( node==NULL || node->numChild<=0 || tag==NULL || strlen( tag )<=0 || nth<1 )
		return NULL;
	num = 1;
	for ( i=0; i<node->numChild; i++ ) {
		if ( node->child[i]->name && !strcmp( tag, node->child[i]->name )) {
			if ( num == nth ) {
				return node->child[i];
			}
			num++;
		}
	}
	return NULL;
}

XmlNode *JabberXmlGetChildWithGivenAttrValue( XmlNode *node, char* tag, char* attrKey, TCHAR* attrValue )
{
	if ( node==NULL || node->numChild<=0 || tag==NULL || strlen( tag )<=0 || attrKey==NULL || strlen( attrKey )<=0 || attrValue==NULL || lstrlen( attrValue )<=0 )
		return NULL;

	TCHAR* str;
	for ( int i=0; i<node->numChild; i++ )
		if ( node->child[i]->name && !strcmp( tag, node->child[i]->name ))
			if (( str=JabberXmlGetAttrValue( node->child[i], attrKey )) != NULL )
				if ( !lstrcmp( str, attrValue ))
					return node->child[i];

	return NULL;
}

static void JabberXmlRemoveChild( XmlNode *node, XmlNode *child )
{
	int i;

	if ( node==NULL || child==NULL || node->numChild<=0 ) return;
	for ( i=0; i<node->numChild; i++ ) {
		if ( node->child[i] == child )
			break;
	}
	if ( i < node->numChild ) {
		for ( ++i; i<node->numChild; i++ )
			node->child[i-1] = node->child[i];
		node->numChild--;
		delete child;
	}
}

XmlNode *JabberXmlCopyNode( XmlNode *node )
{
	XmlNode *n;
	int i;

	if ( node == NULL ) return NULL;
	n = ( XmlNode * ) mir_alloc( sizeof( XmlNode ));
	// Copy attributes
	if ( node->numAttr > 0 ) {
		n->attr = ( XmlAttr ** ) mir_alloc( node->numAttr*sizeof( XmlAttr * ));
		for ( i=0; i<node->numAttr; i++ ) {
			n->attr[i] = ( XmlAttr * ) mir_alloc( sizeof( XmlAttr ));
			if ( node->attr[i]->name ) n->attr[i]->name = mir_strdup( node->attr[i]->name );
			else n->attr[i]->name = NULL;
			if ( node->attr[i]->value ) n->attr[i]->value = mir_tstrdup( node->attr[i]->value );
			else n->attr[i]->value = NULL;
		}
	}
	else
		n->attr = NULL;
	// Recursively copy children
	if ( node->numChild > 0 ) {
		n->child = ( XmlNode ** ) mir_alloc( node->numChild*sizeof( XmlNode * ));
		for ( i=0; i<node->numChild; i++ )
			n->child[i] = JabberXmlCopyNode( node->child[i] );
	}
	else
		n->child = NULL;
	// Copy other fields
	n->numAttr = node->numAttr;
	n->maxNumAttr = node->numAttr;
	n->numChild = node->numChild;
	n->maxNumChild = node->numChild;
	n->depth = node->depth;
	n->state = node->state;
	n->name = ( node->name )?mir_strdup( node->name ):NULL;
	n->text = ( node->text )?mir_tstrdup( node->text ):NULL;

	return n;
}

XmlNode *JabberXmlAddChild( XmlNode *n, char* name )
{
	if ( n==NULL || name==NULL )
		return NULL;

	XmlNode* result = new XmlNode( name );
	if ( result == NULL )
		return NULL;

	return n->addChild( result );
}

//==================================================================================

XmlNode::XmlNode( const char* pszName )
{
	memset( this, 0, sizeof( XmlNode ));
	name = mir_strdup( pszName );
}

XmlNode::XmlNode( const char* pszName, const TCHAR* ptszText )
{
	memset( this, 0, sizeof( XmlNode ));
	name = mir_strdup( pszName );
	#if defined( _UNICODE )
		sendText = JabberTextEncodeW( ptszText );
	#else
		sendText = JabberTextEncode( ptszText );
	#endif
}

#if defined( _UNICODE )
XmlNode::XmlNode( const char* pszName, const char* ptszText )
{
	memset( this, 0, sizeof( XmlNode ));
	name = mir_strdup( pszName );
	sendText = JabberTextEncode( ptszText );
}
#endif

XmlNode::~XmlNode()
{
	if ( this == NULL ) return;

	// Free all children first
	int i;
	for ( i=0; i < numChild; i++ )
		delete child[i];
	if ( child ) mir_free( child );

	// Free all attributes
	for ( i=0; i < numAttr; i++ )
		delete attr[i];
	if ( attr ) mir_free( attr );

	// Free string field
	if ( text ) mir_free( text );
	if ( name ) mir_free( name );
}

XmlAttr* XmlNode::addAttr( XmlAttr* a )
{
	if ( this == NULL || a == NULL )
		return NULL;

	int i = numAttr++;
	attr = ( XmlAttr ** ) mir_realloc( attr, sizeof( XmlAttr * ) * numAttr );
	attr[i] = a;
	return a;
}

XmlAttr* XmlNode::addAttr( const char* pszName, const TCHAR* ptszValue )
{
	return addAttr( new XmlAttr( pszName, ptszValue ));
}

#if defined( _UNICODE )
XmlAttr* XmlNode::addAttr( const char* pszName, const char* pszValue )
{
	return addAttr( new XmlAttr( pszName, pszValue ));
}
#endif

XmlAttr* XmlNode::addAttr( const char* pszName, int value )
{
	if ( this == NULL )
		return NULL;

	TCHAR buf[ 40 ];
	_itot( value, buf, 10 );
	return addAttr( new XmlAttr( pszName, buf ));
}

XmlAttr* XmlNode::addAttrID( int id )
{
	if ( this == NULL )
		return NULL;

	TCHAR text[ 100 ];
	mir_sntprintf( text, SIZEOF(text), _T("mir_%d"), id );
	return addAttr( new XmlAttr( "id", text ));
}

XmlNode* XmlNode::addChild( XmlNode* pNode )
{
	if ( this == NULL || pNode == NULL )
		return NULL;

	int i = numChild++;
	child = ( XmlNode ** ) mir_realloc( child, sizeof( XmlNode * ) * numChild );
	child[i] = pNode;
	pNode->depth = depth+1;
	return pNode;
}

XmlNode* XmlNode::addChild( const char* pszName )
{
	return addChild( new XmlNode( pszName ));
}

XmlNode* XmlNode::addChild( const char* pszName, const TCHAR* ptszValue )
{
	return addChild( new XmlNode( pszName, ptszValue ));
}

#if defined( _UNICODE )
XmlNode* XmlNode::addChild( const char* pszName, const char* pszValue )
{
	return addChild( new XmlNode( pszName, pszValue ));
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// text extraction routines

static char* sttCopyNode( const XmlNode* n, char* dest )
{
	if ( n->props ) {
		lstrcpyA( dest, n->props ); dest += lstrlenA( n->props );
	}

	*dest++ = '<';
	lstrcpyA( dest, n->name ); dest += lstrlenA( n->name );

	for ( int i=0; i < n->numAttr; i++ ) {
		*dest++ = ' ';
		lstrcpyA( dest, n->attr[i]->name ); dest += lstrlenA( n->attr[i]->name );
		*dest++ = '=';
		*dest++ = '\'';
		lstrcpyA( dest, n->attr[i]->sendValue ); dest += lstrlenA( n->attr[i]->sendValue );
		*dest++ = '\'';
	}

	if ( n->numChild != 0 || n->sendText != NULL )
		*dest++ = '>';

	if ( n->sendText != NULL ) {
		lstrcpyA( dest, n->sendText ); dest += lstrlenA( n->sendText );
	}

	if ( n->numChild != 0 )
		for ( int i=0; i < n->numChild; i++ )
			dest = sttCopyNode( n->child[i], dest );

	if ( n->numChild != 0 || n->sendText != NULL ) {
		*dest++ = '<';
		*dest++ = '/';
		lstrcpyA( dest, n->name ); dest += lstrlenA( n->name );
	}
	else if ( !n->dirtyHack ) *dest++ = '/';

	*dest++ = '>';
	*dest = 0;
	return dest;
}

char* XmlNode::getText() const
{
	int cbLen = getTextLen();
	char* result = ( char* )mir_alloc( cbLen+1 );
	if ( result == NULL )
		return NULL;

	sttCopyNode( this, result );
	return result;
}

int XmlNode::getTextLen() const
{
	int result = 10 + lstrlenA( props ) + lstrlenA( name )*2 + lstrlenA( sendText );

	for ( int i=0; i < numAttr; i++ )
		result += lstrlenA( attr[i]->name ) + lstrlenA( attr[i]->sendValue ) + 4;

	for ( int j=0; j < numChild; j++ )
		result += child[j]->getTextLen();

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// XmlAttr class members

XmlAttr::XmlAttr() :
	name( NULL ), value( NULL )
{
}

XmlAttr::XmlAttr( const char* pszName, const TCHAR* ptszValue )
{
	name = mir_strdup( pszName );
	#if defined( _UNICODE )
		sendValue = JabberTextEncodeW( ptszValue );
	#else
		sendValue = JabberTextEncode( ptszValue );
	#endif
}

#if defined( _UNICODE )
XmlAttr::XmlAttr( const char* pszName, const char* ptszValue )
{
	name = mir_strdup( pszName );
	sendValue = JabberTextEncode( ptszValue );
}
#endif

XmlAttr::~XmlAttr()
{
	if ( name != NULL ) mir_free( name );
	if ( value != NULL ) mir_free( value );
}
