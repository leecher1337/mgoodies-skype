/* 
Copyright (C) 2007 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __COMMONS_H__
# define __COMMONS_H__

#include "talk/p2p/base/session.h"
#include "talk/p2p/base/sessionmanager.h"
#include "talk/p2p/client/httpportallocator.h"
#include "talk/xmllite/xmlconstants.h"
#include "talk/xmllite/xmlelement.h"
#include "talk/xmllite/xmlnsstack.h"
#include "talk/xmpp/constants.h"
#include "talk/xmpp/jid.h"
#include "talk/base/helpers.h"
#include "talk/base/thread.h"
#include "talk/base/network.h"
#include "talk/base/socketaddress.h"
#include "talk/session/phone/phonesessionclient.h"

#include <map>
#include <string>
#include <vector>

using namespace buzz;


#include <windows.h>
#include <time.h>


// Miranda headers
#define MIRANDA_VER 0x0700
#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_updater.h>
#include <m_popup.h>
#include <m_message.h>
#include "../../protocols/JabberG/m_jabber_plugin.h"
#include "sdk/m_voice.h"
#include "sdk/m_voiceservice.h"


#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;

class DATA;

class SessionManagerTask : public sigslot::has_slots<> {
	DATA *data;

public:

	SessionManagerTask(DATA *in) : data(in) {}
	void OnOutgoingMessage(const XmlElement* stanza);
};

class CallClient;

struct PendingIq {
	time_t sent;

	void (*callback)(DATA *data, void *param, IXmlNode *node);
	void *param;
};

typedef std::map<int, PendingIq> PendingIqMap;

class DATA
{
public:

	JABBER_DATA *jabber;
	TCHAR fullJID[512];
	
	HANDLE hVoiceNotify;

	talk_base::NetworkManager *network_manager_;
	talk_base::Thread *worker_thread_;
	talk_base::Thread *signaling_thread_;
	cricket::HttpPortAllocator *port_allocator_;
	SessionManagerTask *session_manager_task_;
	cricket::SessionManager *session_manager_;
	CallClient *call_client_;
	buzz::XmlnsStack xmlnsstack_;

	CRITICAL_SECTION csPendingIqMap;
	PendingIqMap pendingIqs;

	bool started;
};

#include "libjingle_callclient.h"



#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


#define MIID_JINGLE { 0x9b9f6b3e, 0xd167, 0x4682, { 0x9b, 0x80, 0xef, 0xd0, 0x54, 0xa4, 0x85, 0xb1 } }


#define JABBER_CAPS_MIRANDA_NODE                "http://miranda-im.org/caps"
#define JABBER_CAPS_GOOGLETALK_NODE             "http://www.google.com/xmpp/client/caps"

#define JABBER_EXT_JINGLE_VOICE                 "voice-v1"
#define JABBER_EXT_JINGLE_SHARE                 "share-v1"

#define JABBER_FEAT_JINGLE_VOICE                "http://www.google.com/xmpp/protocol/voice/v1"
#define JABBER_CAPS_JINGLE_VOICE                ((JabberCapsBits)1<<33)
#define JABBER_FEAT_JINGLE_SHARE                "http://www.google.com/xmpp/protocol/share/v1"
#define JABBER_CAPS_JINGLE_SHARE                ((JabberCapsBits)1<<34)


std::string ToString(const TCHAR *str);

#endif // __COMMONS_H__
