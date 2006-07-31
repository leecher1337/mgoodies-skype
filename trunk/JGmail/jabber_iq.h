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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_iq.h,v $
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

*/

#ifndef _JABBER_IQ_H_
#define _JABBER_IQ_H_

#include "jabber_xml.h"

typedef enum {
	IQ_PROC_NONE,
	IQ_PROC_GETAGENTS,
	IQ_PROC_GETREGISTER,
	IQ_PROC_SETREGISTER,
	IQ_PROC_GETVCARD,
	IQ_PROC_SETVCARD,
	IQ_PROC_GETSEARCH,
	IQ_PROC_BROWSEROOMS,
	IQ_PROC_DISCOROOMSERVER,
	IQ_PROC_DISCOAGENTS
} JABBER_IQ_PROCID;

typedef void ( *JABBER_IQ_PFUNC )( XmlNode *iqNode, void *usedata );

typedef struct {
	TCHAR* xmlns;
	JABBER_IQ_PFUNC func;
	BOOL allowSubNs;		// e.g. #info in disco#info
} JABBER_IQ_XMLNS_FUNC;

void JabberIqInit();
void JabberIqUninit();
JABBER_IQ_PFUNC JabberIqFetchFunc( int iqId );
void JabberIqAdd( unsigned int iqId, JABBER_IQ_PROCID procId, JABBER_IQ_PFUNC func );
JABBER_IQ_PFUNC JabberIqFetchXmlnsFunc( TCHAR* xmlns );

void JabberIqResultExtSearch( XmlNode *iqNode, void *userdata );
void JabberIqResultGetAvatar( XmlNode *iqNode, void *userdata );
//void JabberIqResultGetAuth( XmlNode *iqNode, void *userdata );
//void JabberIqResultSetAuth( XmlNode *iqNode, void *userdata );
void JabberIqResultGetRoster( XmlNode *iqNode, void *userdata );
void JabberIqResultGetAgents( XmlNode *iqNode, void *userdata );
void JabberIqResultGetRegister( XmlNode *iqNode, void *userdata );
void JabberIqResultSetRegister( XmlNode *iqNode, void *userdata );
void JabberIqResultGetVcard( XmlNode *iqNode, void *userdata );
void JabberIqResultSetVcard( XmlNode *iqNode, void *userdata );
void JabberIqResultSetSearch( XmlNode *iqNode, void *userdata );
void JabberIqResultSetPassword( XmlNode *iqNode, void *userdata );
void JabberIqResultDiscoAgentItems( XmlNode *iqNode, void *userdata );
void JabberIqResultDiscoAgentInfo( XmlNode *iqNode, void *userdata );
void JabberIqResultDiscoClientInfo( XmlNode *iqNode, void *userdata );
void JabberIqResultBrowseRooms( XmlNode *iqNode, void *userdata );
void JabberIqResultGetMuc( XmlNode *iqNode, void *userdata );
void JabberIqResultDiscoRoomItems( XmlNode *iqNode, void *userdata );
void JabberIqResultMucGetVoiceList( XmlNode *iqNode, void *userdata );
void JabberIqResultMucGetMemberList( XmlNode *iqNode, void *userdata );
void JabberIqResultMucGetModeratorList( XmlNode *iqNode, void *userdata );
void JabberIqResultMucGetBanList( XmlNode *iqNode, void *userdata );
void JabberIqResultMucGetAdminList( XmlNode *iqNode, void *userdata );
void JabberIqResultMucGetOwnerList( XmlNode *iqNode, void *userdata );
void JabberIqResultBind( XmlNode *iqNode, void *userdata );
void JabberIqResultMailNotify( XmlNode *iqNode, void *userdata );
void JabberRequestMailBox(HANDLE hConn);
void JabberEnableNotifications(ThreadData *info);

#endif
