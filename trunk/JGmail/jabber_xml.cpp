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
	// Note: cannot use JabberXmlFreeNode() to free xmlState->root
	// because it will do free( xmlState->root ) which is not freeable.
	node = &( xmlState->root );
	// Free all children first
	for ( i=0; i<node->numChild; i++ )
		JabberXmlFreeNode( node->child[i] );
	if ( node->child ) free( node->child );
	// Free all attributes
	for ( i=0; i<node->numAttr; i++ ) {
		if ( node->attr[i]->name ) free( node->attr[i]->name );
		if ( node->attr[i]->value ) free( node->attr[i]->value );
		free( node->attr[i] );
	}
	if ( node->attr ) free( node->attr );
	// Free string field
	if ( node->text ) free( node->text );
	if ( node->name ) free( node->name );
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
				str = ( char* )malloc( q-p+1 );
				strncpy( str, p, q-p );
				str[q-p] = '\0';
				JabberXmlProcessElem( xmlState, ELEM_TEXT, str, NULL );
				free( str );
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
			node->attr = ( XmlAttr ** ) realloc( node->attr, node->maxNumAttr*sizeof( XmlAttr * ));
		}
		a = node->attr[node->numAttr] = ( XmlAttr * ) malloc( sizeof( XmlAttr ));
		node->numAttr++;

		// Skip possible whitespaces between key and '='
		for ( ;*p!='\0' && ( *p==' ' || *p=='\t' ); p++ );

		if ( *p == '\0' ) {
			a->name = ( char* )malloc( klen+1 );
			strncpy( a->name, kstart, klen );
			a->name[klen] = '\0';
			a->value = _strdup( "" );
			break;
		}

		if ( *p != '=' ) {
			a->name = ( char* )malloc( klen+1 );
			strncpy( a->name, kstart, klen );
			a->name[klen] = '\0';
			a->value = _strdup( "" );
			continue;
		}

		// Found '='
		p++;

		// Skip possible whitespaces between '=' and value
		for ( ;*p!='\0' && ( *p==' ' || *p=='\t' ); p++ );

		if ( *p == '\0' ) {
			a->name = ( char* )malloc( klen+1 );
			strncpy( a->name, kstart, klen );
			a->name[klen] = '\0';
			a->value = _strdup( "" );
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

		a->name = ( char* )malloc( klen+1 );
		strncpy( a->name, kstart, klen );
		a->name[klen] = '\0';
		a->value = ( char* )malloc( vlen+1 );
		strncpy( a->value, vstart, vlen );
		a->value[vlen] = '\0';
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

	text = _strdup( elemText );

	if ( elemAttr )
		attr = _strdup( elemAttr );
	else
		attr = NULL;

	switch ( elemType ) {
	case ELEM_OPEN:
		if ( node->numChild >= node->maxNumChild ) {
			node->maxNumChild = node->numChild + 20;
			node->child = ( XmlNode ** ) realloc( node->child, node->maxNumChild*sizeof( XmlNode * ));
		}
		n = node->child[node->numChild] = ( XmlNode * ) malloc( sizeof( XmlNode ));
		node->numChild++;
		n->name = text;
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
			node->child = ( XmlNode ** ) realloc( node->child, node->maxNumChild*sizeof( XmlNode * ));
		}
		n = node->child[node->numChild] = ( XmlNode * ) malloc( sizeof( XmlNode ));
		node->numChild++;
		n->name = text;
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
			}
			free( text );
		}
		else {
			JabberLog( "XML: Closing </%s> without opening tag", text );
			free( text );
			if ( attr ) free( attr );
			return FALSE;
		}
		break;
	case ELEM_TEXT:
		node->text = text;
		break;
	default:
		free( text );
		if ( attr ) free( attr );
		return FALSE;
	}

	if ( attr ) free( attr );

	return TRUE;
}

char* JabberXmlGetAttrValue( XmlNode *node, char* key )
{
	int i;

	if ( node==NULL || node->numAttr<=0 || key==NULL || strlen( key )<=0 )
		return NULL;
	for ( i=0; i<node->numAttr; i++ ) {
		if ( node->attr[i]->name && !strcmp( key, node->attr[i]->name ))
			return node->attr[i]->value;
	}
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

XmlNode *JabberXmlGetChildWithGivenAttrValue( XmlNode *node, char* tag, char* attrKey, char* attrValue )
{
	int i;
	char* str;

	if ( node==NULL || node->numChild<=0 || tag==NULL || strlen( tag )<=0 || attrKey==NULL || strlen( attrKey )<=0 || attrValue==NULL || strlen( attrValue )<=0 )
		return NULL;
	for ( i=0; i<node->numChild; i++ ) {
		if ( node->child[i]->name && !strcmp( tag, node->child[i]->name )) {
			if (( str=JabberXmlGetAttrValue( node->child[i], attrKey )) != NULL )
				if ( !strcmp( str, attrValue ))
					return node->child[i];
		}
	}
	return NULL;
}

void JabberXmlDumpAll( XmlState *xmlState )
{
	XmlNode *root;
	int i;

	JabberLog( "XML: DUMPALL: ------------------------" );
	root = &( xmlState->root );
	if ( root->numChild <= 0 )
		JabberLog( "XML: DUMPALL: NULL" );
	else {
		for ( i=0; i<root->numChild; i++ )
			JabberXmlDumpNode( root->child[i] );
	}
	JabberLog( "XML: DUMPALL: ------------------------" );
}

void JabberXmlDumpNode( XmlNode *node )
{
	char* str;
	int i;

	if ( node == NULL ) {
		JabberLog( "XML: DUMP: NULL" );
		return;
	}

	str = ( char* )malloc( 256 );
	str[0] = '\0';
	for ( i=0; i<node->depth; i++ )
		strcat( str, "  " );
	strcat( str, "<" );
	if ( node->name )
		strcat( str, node->name );
	else
		strcat( str, "( NULL )" );
	for ( i=0; i<node->numAttr; i++ ) {
		strcat( str, " " );
		if ( node->attr[i]->name )
			strcat( str, node->attr[i]->name );
		else
			strcat( str, "( NULL )" );
		strcat( str, "=" );
		if ( node->attr[i]->value ) {
			strcat( str, "'" );
			strcat( str, node->attr[i]->value );
			strcat( str, "'" );
		}
	}
	if ( node->text || node->numChild>0 ) {
		strcat( str, ">" );
		if ( node->text )
			strcat( str, node->text );
		if ( node->numChild > 0 ) {
			JabberLog( "XML: DUMP: %s", str );
			for ( i=0; i<node->numChild; i++ )
				JabberXmlDumpNode( node->child[i] );
			str[0] = '\0';
			for ( i=0; i<node->depth; i++ )
				strcat( str, "  " );
		}
		if ( node->state != NODE_OPEN ) {
			strcat( str, "</" );
			strcat( str, node->name );
			strcat( str, ">" );
		}
	}
	else {
		if ( node->state != NODE_OPEN )
			strcat( str, "/>" );
	}

	JabberLog( "XML: DUMP: %s", str );
	free( str );
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
		JabberXmlFreeNode( child );
	}
}

void JabberXmlFreeNode( XmlNode *node )
{
	int i;

	if ( node == NULL ) return;
	// Free all children first
	for ( i=0; i<node->numChild; i++ )
		JabberXmlFreeNode( node->child[i] );
	if ( node->child ) free( node->child );
	// Free all attributes
	for ( i=0; i<node->numAttr; i++ ) {
		if ( node->attr[i]->name ) free( node->attr[i]->name );
		if ( node->attr[i]->value ) free( node->attr[i]->value );
		free( node->attr[i] );
	}
	if ( node->attr ) free( node->attr );
	// Free string field
	if ( node->text ) free( node->text );
	if ( node->name ) free( node->name );
	// Free the node itself
	free( node );
}

XmlNode *JabberXmlCopyNode( XmlNode *node )
{
	XmlNode *n;
	int i;

	if ( node == NULL ) return NULL;
	n = ( XmlNode * ) malloc( sizeof( XmlNode ));
	// Copy attributes
	if ( node->numAttr > 0 ) {
		n->attr = ( XmlAttr ** ) malloc( node->numAttr*sizeof( XmlAttr * ));
		for ( i=0; i<node->numAttr; i++ ) {
			n->attr[i] = ( XmlAttr * ) malloc( sizeof( XmlAttr ));
			if ( node->attr[i]->name ) n->attr[i]->name = _strdup( node->attr[i]->name );
			else n->attr[i]->name = NULL;
			if ( node->attr[i]->value ) n->attr[i]->value = _strdup( node->attr[i]->value );
			else n->attr[i]->value = NULL;
		}
	}
	else
		n->attr = NULL;
	// Recursively copy children
	if ( node->numChild > 0 ) {
		n->child = ( XmlNode ** ) malloc( node->numChild*sizeof( XmlNode * ));
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
	n->name = ( node->name )?_strdup( node->name ):NULL;
	n->text = ( node->text )?_strdup( node->text ):NULL;

	return n;
}

#ifdef _DEBUG
XmlNode *JabberXmlCreateNode( char* name )
{
	XmlNode *n;

	if ( name == NULL )
		return NULL;

	n = ( XmlNode * ) malloc( sizeof( XmlNode ));
	memset( n, 0, sizeof( XmlNode ));
	n->name = _strdup( name );
	return n;
}

void JabberXmlAddAttr( XmlNode *n, char* name, char* value )
{
	int i;

	if ( n==NULL || name==NULL || value==NULL )
		return;

	i = n->numAttr;
	( n->numAttr )++;
	n->attr = ( XmlAttr ** ) realloc( n->attr, sizeof( XmlAttr * ) * n->numAttr );
	n->attr[i] = ( XmlAttr * ) malloc( sizeof( XmlAttr ));
	n->attr[i]->name = _strdup( name );
	n->attr[i]->value = _strdup( value );
}

XmlNode *JabberXmlAddChild( XmlNode *n, char* name )
{
	int i;

	if ( n==NULL || name==NULL )
		return NULL;

	i = n->numChild;
	n->numChild++;
	n->child = ( XmlNode ** ) realloc( n->child, sizeof( XmlNode * ) * n->numChild );
	n->child[i] = ( XmlNode * ) malloc( sizeof( XmlNode ));
	memset( n->child[i], 0, sizeof( XmlNode ));
	n->child[i]->name = _strdup( name );
	return n->child[i];
}

void JabberXmlAddText( XmlNode *n, char* text )
{
	if ( n!=NULL && text!=NULL ) {
		if ( n->text ) free( n->text );
		n->text = _strdup( text );
	}
}
#endif
