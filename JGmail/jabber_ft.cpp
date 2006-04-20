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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_ft.cpp,v $
Revision       : $Revision: 1.13 $
Last change on : $Date: 2006/01/29 16:56:22 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "jabber_iq.h"
#include "jabber_byte.h"

void JabberFtCancel( filetransfer* ft )
{
	JABBER_LIST_ITEM *item;
	JABBER_BYTE_TRANSFER *jbt;
	int i;

	JabberLog( "Invoking JabberFtCancel()" );

	// For file sending session that is still in si negotiation phase
	for ( i=0; ( i=JabberListFindNext( LIST_FTSEND, i ))>=0; i++ ) {
		item = JabberListGetItemPtrFromIndex( i );
		if ( item->ft == ft ) {
			JabberLog( "Canceling file sending session while in si negotiation" );
			JabberListRemoveByIndex( i );
			JSendBroadcast( ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0 );
			delete ft;
			return;
		}
	}
	// For file receiving session that is still in si negotiation phase
	for ( i=0; ( i=JabberListFindNext( LIST_FTRECV, i ))>=0; i++ ) {
		item = JabberListGetItemPtrFromIndex( i );
		if ( item->ft == ft ) {
			JabberLog( "Canceling file receiving session while in si negotiation" );
			JabberListRemoveByIndex( i );
			JSendBroadcast( ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0 );
			delete ft;
			return;
		}
	}
	// For file transfer through bytestream
	if (( jbt=ft->jbt ) != NULL ) {
		JabberLog( "Canceling bytestream session" );
		jbt->state = JBT_ERROR;
		if ( jbt->hConn ) {
			JabberLog( "Force closing bytestream session" );
			Netlib_CloseHandle( jbt->hConn );
			jbt->hConn = NULL;
		}
		if ( jbt->hEvent ) SetEvent( jbt->hEvent );
	}
}

///////////////// File sending using stream initiation /////////////////////////

static void JabberFtSiResult( XmlNode *iqNode, void *userdata );
static BOOL JabberFtSend( HANDLE hConn, void *userdata );
static void JabberFtSendFinal( BOOL success, void *userdata );

void JabberFtInitiate( char* jid, filetransfer* ft )
{
	int iqId;
	char* rs, *filename, *p;
	char idStr[32];
	JABBER_LIST_ITEM *item;
	int i;
	char sid[9];

	if ( jid==NULL || ft==NULL || !jabberOnline ||
		( rs=JabberListGetBestClientResourceNamePtr( jid ))==NULL ) {
		JSendBroadcast( ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0 );
		delete ft;
		return;
	}
	iqId = JabberSerialNext();
	JabberIqAdd( iqId, IQ_PROC_NONE, JabberFtSiResult );
	mir_snprintf( idStr, sizeof( idStr ), JABBER_IQID"%d", iqId );
	if (( item=JabberListAdd( LIST_FTSEND, idStr )) == NULL ) {
		JSendBroadcast( ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0 );
		delete ft;
		return;
	}
	item->ft = ft;
	ft->type = FT_SI;
	for ( i=0; i<8; i++ )
		sid[i] = ( rand()%10 ) + '0';
	sid[8] = '\0';
	if ( ft->sid != NULL ) free( ft->sid );
	ft->sid = _strdup( sid );
	filename = ft->std.files[ ft->std.currentFileNumber ];
	if (( p=strrchr( filename, '\\' )) != NULL )
		filename = p+1;

	JabberSend( jabberThreadInfo->s,
        "<iq type=\"set\" id=\""JABBER_IQID"%d\" to=\"%s/%s\">"
            "<si xmlns=\"http://jabber.org/protocol/si\" id=\"%s\" mime-type=\"binary/octet-stream\" profile=\"http://jabber.org/protocol/si/profile/file-transfer\">"
                "<file xmlns=\"http://jabber.org/protocol/si/profile/file-transfer\" name=\"%s\" size=\"%d\">"
					"<desc>%s</desc>"
					//"<range/>"
				"</file>"
                "<feature xmlns=\"http://jabber.org/protocol/feature-neg\">"
                    "<x xmlns=\"jabber:x:data\" type=\"form\">"
                        "<field var=\"stream-method\" type=\"list-single\">"
							"<option><value>http://jabber.org/protocol/bytestreams</value></option>"
						"</field>"
					"</x>"
				"</feature>"
			"</si>"
		"</iq>",
		iqId, jid, rs, sid, UTF8(filename), ft->fileSize[ ft->std.currentFileNumber ], UTF8(ft->szDescription));
}

static void JabberFtSiResult( XmlNode *iqNode, void *userdata )
{
	char* type, *from, *to;
	char* idStr, *str;
	XmlNode *siNode, *fileNode, *rangeNode, *featureNode, *xNode, *fieldNode, *valueNode;
	JABBER_LIST_ITEM *item;
	int offset, length;
	JABBER_BYTE_TRANSFER *jbt;

	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;
	if (( from=JabberXmlGetAttrValue( iqNode, "from" )) == NULL ) return;
	if (( to=JabberXmlGetAttrValue( iqNode, "to" )) == NULL ) return;
	idStr = JabberXmlGetAttrValue( iqNode, "id" );
	if (( item=JabberListGetItemPtr( LIST_FTSEND, idStr )) == NULL ) return;

	if ( !strcmp( type, "result" )) {
		if (( siNode=JabberXmlGetChild( iqNode, "si" )) != NULL ) {
			if (( fileNode=JabberXmlGetChild( siNode, "file" )) != NULL ) {
				if (( rangeNode=JabberXmlGetChild( fileNode, "range" )) != NULL ) {
// ************** Need to store offset/length in ft structure **********************
// but at this tiem, we should not get <range/> tag since we don't sent <range/> on our request
					if (( str=JabberXmlGetAttrValue( rangeNode, "offset" )) != NULL )
						offset = atoi( str );
					if (( str=JabberXmlGetAttrValue( rangeNode, "length" )) != NULL )
						length = atoi( str );
				}
			}
			if (( featureNode=JabberXmlGetChild( siNode, "feature" )) != NULL ) {
				if (( xNode=JabberXmlGetChildWithGivenAttrValue( featureNode, "x", "xmlns", "jabber:x:data" )) != NULL ) {
					if (( fieldNode=JabberXmlGetChildWithGivenAttrValue( xNode, "field", "var", "stream-method" )) != NULL ) {
						if (( valueNode=JabberXmlGetChild( fieldNode, "value" ))!=NULL && valueNode->text!=NULL ) {
							if ( !strcmp( valueNode->text, "http://jabber.org/protocol/bytestreams" )) {
								// Start Bytestream session
								jbt = ( JABBER_BYTE_TRANSFER * ) malloc( sizeof( JABBER_BYTE_TRANSFER ));
								ZeroMemory( jbt, sizeof( JABBER_BYTE_TRANSFER ));
								jbt->srcJID = JabberUrlDecodeNew( to );
								jbt->dstJID = JabberUrlDecodeNew( from );
								jbt->sid = _strdup( item->ft->sid );
								jbt->pfnSend = JabberFtSend;
								jbt->pfnFinal = JabberFtSendFinal;
								jbt->userdata = item->ft;
								item->ft->type = FT_BYTESTREAM;
								item->ft->jbt = jbt;
								JabberForkThread(( JABBER_THREAD_FUNC )JabberByteSendThread, 0, jbt );
							}
						}
					}
				}
			}
		}
	}
	else if ( !strcmp( type, "error" )) {
		JabberLog( "File transfer stream initiation request denied" );
		JSendBroadcast( item->ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, item->ft, 0 );
		delete item->ft;
		item->ft = NULL;
	}

	JabberListRemove( LIST_FTSEND, idStr );
}

static BOOL JabberFtSend( HANDLE hConn, void *userdata )
{
	filetransfer* ft = ( filetransfer* ) userdata;

	struct _stat statbuf;
	int fd;
	char* buffer;
	int numRead;

	JabberLog( "Sending [%s]", ft->std.files[ ft->std.currentFileNumber ] );
	_stat( ft->std.files[ ft->std.currentFileNumber ], &statbuf );	// file size in statbuf.st_size
	if (( fd=_open( ft->std.files[ ft->std.currentFileNumber ], _O_BINARY|_O_RDONLY )) < 0 ) {
		JabberLog( "File cannot be opened" );
		return FALSE;
	}

	ft->std.sending = TRUE;
	ft->std.currentFileSize = statbuf.st_size;
	ft->std.currentFileProgress = 0;

	if (( buffer=( char* )malloc( 2048 )) != NULL ) {
		while (( numRead=_read( fd, buffer, 2048 )) > 0 ) {
			if ( Netlib_Send( hConn, buffer, numRead, 0 ) != numRead ) {
				free( buffer );
				_close( fd );
				return FALSE;
			}
			ft->std.currentFileProgress += numRead;
			ft->std.totalProgress += numRead;
			JSendBroadcast( ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, ( LPARAM )&ft->std );
		}
		free( buffer );
	}
	_close( fd );
	return TRUE;
}

static void JabberFtSendFinal( BOOL success, void *userdata )
{
	filetransfer* ft = ( filetransfer* )userdata;

	if ( !success ) {
		JabberLog( "File transfer complete with error" );
		JSendBroadcast( ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0 );
	}
	else {
		if ( ft->std.currentFileNumber < ft->std.totalFiles-1 ) {
			ft->std.currentFileNumber++;
			replaceStr( ft->std.currentFile, ft->std.files[ ft->std.currentFileNumber ] );
			JSendBroadcast( ft->std.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0 );
			JabberFtInitiate( ft->jid, ft );
			return;
		}

		JSendBroadcast( ft->std.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0 );
	}

	delete ft;
}

///////////////// File receiving through stream initiation /////////////////////////

static int JabberFtReceive( HANDLE hConn, void *userdata, char* buffer, int datalen );
static void JabberFtReceiveFinal( BOOL success, void *userdata );

void JabberFtHandleSiRequest( XmlNode *iqNode )
{
	char* from, *sid;
	XmlNode *siNode, *fileNode, *featureNode, *xNode, *fieldNode, *optionNode, *n;
	char* szId, *str, *filename, *localFilename;
	int filesize, i;
	JABBER_FT_TYPE ftType;

	if ( iqNode==NULL ||
		  ( from=JabberXmlGetAttrValue( iqNode, "from" ))==NULL ||
		  ( str=JabberXmlGetAttrValue( iqNode, "type" ))==NULL || strcmp( str, "set" ) ||
		  ( siNode=JabberXmlGetChildWithGivenAttrValue( iqNode, "si", "xmlns", "http://jabber.org/protocol/si" )) == NULL )
		return;

	szId = JabberXmlGetAttrValue( iqNode, "id" );
	if (( sid=JabberXmlGetAttrValue( siNode, "id" ))!=NULL &&
		( fileNode=JabberXmlGetChildWithGivenAttrValue( siNode, "file", "xmlns", "http://jabber.org/protocol/si/profile/file-transfer" ))!=NULL &&
		( filename=JabberXmlGetAttrValue( fileNode, "name" ))!=NULL &&
		( str=JabberXmlGetAttrValue( fileNode, "size" ))!=NULL ) {

		filesize = atoi( str );
		if (( featureNode=JabberXmlGetChildWithGivenAttrValue( siNode, "feature", "xmlns", "http://jabber.org/protocol/feature-neg" )) != NULL &&
			( xNode=JabberXmlGetChildWithGivenAttrValue( featureNode, "x", "xmlns", "jabber:x:data" ))!=NULL &&
			( fieldNode=JabberXmlGetChildWithGivenAttrValue( xNode, "field", "var", "stream-method" ))!=NULL ) {

			for ( i=0; i<fieldNode->numChild; i++ ) {
				optionNode = fieldNode->child[i];
				if ( optionNode->name && !strcmp( optionNode->name, "option" )) {
					if (( n=JabberXmlGetChild( optionNode, "value" ))!=NULL && n->text ) {
						if ( !strcmp( n->text, "http://jabber.org/protocol/bytestreams" )) {
							ftType = FT_BYTESTREAM;
							break;
			}	}	}	}

			if ( i < fieldNode->numChild ) {
				// Found known stream mechanism
				CCSDATA ccs;
				PROTORECVEVENT pre;
				char* szBlob, *desc;

				if (( n=JabberXmlGetChild( fileNode, "desc" ))!=NULL && n->text!=NULL )
					desc = JabberTextDecode( n->text );
				else
					desc = _strdup( "" );
				if ( desc != NULL ) {
					if (( localFilename=JabberTextDecode( filename )) != NULL ) {
						filetransfer* ft = new filetransfer;
						ft->jid = JabberUrlDecodeNew( from );
						ft->std.hContact = JabberHContactFromJID( from );
						ft->sid = _strdup( sid );
						ft->iqId = ( szId )?_strdup( szId ):NULL;
						ft->type = ftType;
						ft->std.totalFiles = 1;
						ft->std.currentFile = localFilename;
						ft->std.totalBytes = ft->std.currentFileSize = filesize;
						szBlob = ( char* )malloc( sizeof( DWORD )+ strlen( localFilename ) + strlen( desc ) + 2 );
						*(( PDWORD ) szBlob ) = ( DWORD )ft;
						strcpy( szBlob + sizeof( DWORD ), localFilename );
						strcpy( szBlob + sizeof( DWORD )+ strlen( localFilename ) + 1, desc );
						pre.flags = 0;
						pre.timestamp = time( NULL );
						pre.szMessage = szBlob;
						pre.lParam = 0;
						ccs.szProtoService = PSR_FILE;
						ccs.hContact = ft->std.hContact;
						ccs.wParam = 0;
						ccs.lParam = ( LPARAM )&pre;
						JCallService( MS_PROTO_CHAINRECV, 0, ( LPARAM )&ccs );
						free( szBlob );
					}
					free( desc );
					return;
				}
			}
			else {
				// No known stream mechanism
                JabberSend( jabberThreadInfo->s, "<iq type=\"error\" to=\"%s\"%s%s%s><error code=\"400\" type=\"cancel\"><bad-request xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/><no-valid-streams xmlns=\"http://jabber.org/protocol/si\"/></error></iq>", from, ( szId )?" id=\"":"", ( szId )?szId:"", ( szId )?"\"":"" );
				return;
	}	}	}

	// Bad stream initiation, reply with bad-profile
    JabberSend( jabberThreadInfo->s, "<iq type=\"error\" to=\"%s\"%s%s%s><error code=\"400\" type=\"cancel\"><bad-request xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/><bad-profile xmlns=\"http://jabber.org/protocol/si\"/></error></iq>", from, ( szId )?" id=\"":"", ( szId )?szId:"", ( szId )?"\"":"" );
}

void JabberFtAcceptSiRequest( filetransfer* ft )
{
	JABBER_LIST_ITEM *item;
	char* szId;

	if ( !jabberOnline || ft==NULL || ft->jid==NULL || ft->sid==NULL ) return;

	if (( item=JabberListAdd( LIST_FTRECV, ft->sid )) != NULL ) {
		item->ft = ft;
		szId = ft->iqId;
		JabberSend( jabberThreadInfo->s,
            "<iq type=\"result\"  to=\"%s\"%s%s%s>"
                "<si xmlns=\"http://jabber.org/protocol/si\">"
                    //"<file xmlns=\"http://jabber.org/protocol/si/profile/file-transfer\" name=\"%s\" size=\"%d\">"
						//"<range/>"
					//"</file>"
                    "<feature xmlns=\"http://jabber.org/protocol/feature-neg\">"
                        "<x xmlns=\"jabber:x:data\" type=\"submit\">"
                            "<field var=\"stream-method\"><value>%s</value></field>"
						"</x>"
					"</feature>"
				"</si>"
			"</iq>",
            ft->jid, ( szId )?" id=\"":"", ( szId )?szId:"", ( szId )?"\"":"",
			/*( ft->type==FT_BYTESTREAM )?*/"http://jabber.org/protocol/bytestreams"
		 );
	}
}

BOOL JabberFtHandleBytestreamRequest( XmlNode *iqNode )
{
	XmlNode *queryNode;
	char* sid;
	JABBER_LIST_ITEM *item;
	JABBER_BYTE_TRANSFER *jbt;

	if ( iqNode == NULL ) return FALSE;
	if (( queryNode=JabberXmlGetChildWithGivenAttrValue( iqNode, "query", "xmlns", "http://jabber.org/protocol/bytestreams" ))!=NULL &&
		( sid=JabberXmlGetAttrValue( queryNode, "sid" ))!=NULL &&
		( item=JabberListGetItemPtr( LIST_FTRECV, sid ))!=NULL ) {
		// Start Bytestream session
		jbt = ( JABBER_BYTE_TRANSFER * ) malloc( sizeof( JABBER_BYTE_TRANSFER ));
		ZeroMemory( jbt, sizeof( JABBER_BYTE_TRANSFER ));
		jbt->iqNode = JabberXmlCopyNode( iqNode );
		jbt->pfnRecv = JabberFtReceive;
		jbt->pfnFinal = JabberFtReceiveFinal;
		jbt->userdata = item->ft;
		item->ft->jbt = jbt;
		JabberForkThread(( JABBER_THREAD_FUNC )JabberByteReceiveThread, 0, jbt );
		JabberListRemove( LIST_FTRECV, sid );
		return TRUE;
	}

	JabberLog( "File transfer invalid bytestream initiation request received" );
	return FALSE;
}

static int JabberFtReceive( HANDLE hConn, void *userdata, char* buffer, int datalen )
{
	filetransfer* ft = ( filetransfer* )userdata;
	if ( ft->create() == -1 )
		return -1;

	int remainingBytes = ft->std.currentFileSize - ft->std.currentFileProgress;
	if ( remainingBytes > 0 ) {
		int writeSize = ( remainingBytes<datalen ) ? remainingBytes : datalen;
		if ( _write( ft->fileId, buffer, writeSize ) != writeSize ) {
			JabberLog( "_write() error" );
			return -1;
		}

		ft->std.currentFileProgress += writeSize;
		ft->std.totalProgress += writeSize;
		JSendBroadcast( ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, ( LPARAM )&ft->std );
		return ( ft->std.currentFileSize == ft->std.currentFileProgress ) ? 0 : writeSize;
	}

	return 0;
}

static void JabberFtReceiveFinal( BOOL success, void *userdata )
{
	filetransfer* ft = ( filetransfer* )userdata;

	if ( success ) {
		JabberLog( "File transfer complete successfully" );
		ft->complete();
	}
	else JabberLog( "File transfer complete with error" );

	delete ft;
}
