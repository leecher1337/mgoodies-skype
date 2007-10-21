/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci

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

#include "commons.h"



// Prototypes ///////////////////////////////////////////////////////////////////////////


PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
#ifdef UNICODE
	"Jingle (Unicode)",
#else
	"Jingle",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,1),
	"Jingle support for Jabber protocol",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2007 Ricardo Pescuma Domenecci",
	"http://pescuma.mirandaim.ru/miranda/jingle",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
#if defined( _UNICODE )
	{ 0xf34e6d64, 0xc12e, 0x4621, { 0x98, 0x87, 0xbe, 0xa2, 0x42, 0xed, 0xea, 0x8a } } // {F34E6D64-C12E-4621-9887-BEA242EDEA8A}
#else
	{ 0xaee557dd, 0xbc7b, 0x46d9, { 0xad, 0xcc, 0x82, 0x2a, 0x3f, 0xa3, 0xa, 0x90 } } // {AEE557DD-BC7B-46d9-ADCC-822A3FA30A90}
#endif
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;
LIST_INTERFACE li;

HANDLE hHooks[2] = {0};

LIST<DATA> jabbers(2);

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);

void RegisterJabberPlugin(const char *proto);
__inline static int ProtoServiceExists(const char *szModule,const char *szService);

static XmlElement *ConvertToXmlElement(DATA *data, IXmlNode *jabberNode);



// Functions ////////////////////////////////////////////////////////////////////////////


extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	hInst = hinstDLL;
	return TRUE;
}


extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) 
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*) &pluginInfo;
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFOEX);
	return &pluginInfo;
}


static const MUUID interfaces[] = { MIID_JINGLE, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link) 
{
	pluginLink = link;
	
	init_mir_malloc();

	li.cbSize = sizeof(li);
	CallService(MS_SYSTEM_GET_LI, 0, (LPARAM) &li);

	// hooks
	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hHooks[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	for (int i = 0; i < jabbers.getCount(); i++)
	{
		DATA *data = jabbers[i];

		if (data->hVoiceNotify != NULL)
			DestroyHookableEvent(data->hVoiceNotify);

		if (data->call_client_ != NULL)
			delete data->call_client_;
		if (data->network_manager_ != NULL)
			delete data->network_manager_;
		if (data->port_allocator_ != NULL)
			delete data->port_allocator_;
		if (data->session_manager_ != NULL)
			delete data->session_manager_;
		if (data->session_manager_task_ != NULL)
			delete data->session_manager_task_;
		if (data->signaling_thread_ != NULL)
			delete data->signaling_thread_;
		if (data->worker_thread_ != NULL)
			delete data->worker_thread_;

		DeleteCriticalSection(&data->csPendingIqMap);

		free(data);
	}
	return 0;
}

// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];
		
		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;
		
		upd.szUpdateURL = UPDATER_AUTOREGISTER;
		
		upd.szBetaVersionURL = "http://pescuma.mirandaim.ru/miranda/jingle_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.mirandaim.ru/miranda/jingle#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"Jingle ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/jingleW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.mirandaim.ru/miranda/jingle.zip";
#endif
		
		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);
		
        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	
	if (!ServiceExists(MS_VOICESERVICE_REGISTER))
	{
		MessageBox(NULL, _T("Jingle needs Voice Service plugin to work."), _T("Jingle - Jabber plugin"), MB_OK | MB_ICONERROR);
		return 0;
	}

	PROTOCOLDESCRIPTOR **protos;
	int count;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);
	for (int i = 0; i < count; i++)
	{
		if (protos[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0')
			continue;

		// Found a protocol
		RegisterJabberPlugin(protos[i]->szName);
	}

	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < MAX_REGS(hHooks); ++i)
		if (hHooks[i] != NULL)
			UnhookEvent(hHooks[i]);

	for (int i = 0; i < jabbers.getCount(); i++)
	{
		DATA *data = jabbers[i];

		if (data->hVoiceNotify != NULL)
			DestroyHookableEvent(data->hVoiceNotify);
	}

	return 0;
}


static void replace(std::string &str, char *find, char *replace)
{
	int pos = str.find(find);
	if (pos >= 0) 
		str.replace(pos, strlen(find), replace);
}


void HandleStanzaReturn(DATA *data, void *param, IXmlNode *jabberNode) 
{
	XmlElement *newStanza = ConvertToXmlElement(data, jabberNode);

//	OutputDebugStringA("<IN>\n\t");
//	OutputDebugStringA(newStanza->Str().c_str());
//	OutputDebugStringA("\n</IN>\n");

	ReplyMessageData *rmd = new ReplyMessageData();
	rmd->oldStanza = (buzz::XmlElement *) param;
	rmd->newStanza = newStanza;
	data->signaling_thread_->Post(data->call_client_, MSG_REPLY_MSG, rmd);
}


void SessionManagerTask ::OnOutgoingMessage(const XmlElement* stanza) {
	if (!data->started)
		return;

	if (stanza->Attr(buzz::QN_TYPE) == "set") 
	{
		buzz::XmlElement *newOne = new buzz::XmlElement(*stanza);

		int iqId;
		if (!newOne->HasAttr(buzz::QN_ID))
		{
			char tmp[100];
			iqId = data->jabber->pfIqSerialNext();
			mir_snprintf(tmp, MAX_REGS(tmp), "mir_%d", iqId);
			newOne->SetAttr(buzz::QN_ID, std::string(tmp));
		}
		else
		{
			iqId = atoi(newOne->Attr(buzz::QN_ID).c_str());
		}

		EnterCriticalSection(&data->csPendingIqMap);
		PendingIq &piq = data->pendingIqs[iqId];
		piq.sent = time(NULL);
		piq.callback = HandleStanzaReturn;
		piq.param = newOne;
		LeaveCriticalSection(&data->csPendingIqMap);

		std::string str = newOne->Str();

//		OutputDebugStringA("<OUT>\n\t");
//		OutputDebugStringA(str.c_str());
//		OutputDebugStringA("\n</OUT>\n");

		data->jabber->pfSendString(str.c_str());
	}
	else
	{
		std::string str = stanza->Str();

//		OutputDebugStringA("<OUT>\n\t");
//		OutputDebugStringA(str.c_str());
//		OutputDebugStringA("\n</OUT>\n");

		data->jabber->pfSendString(str.c_str());
	}
}


static bool StartsWithXmlns(const char *name) {
	return name[0] == 'x' &&
		name[1] == 'm' &&
		name[2] == 'l' &&
		name[3] == 'n' &&
		name[4] == 's';
}

static QName ResolveQName(buzz::XmlnsStack &xmlnsstack_, const char *qname, bool isAttr) {
	const char *c;
	for (c = qname; *c; ++c) {
		if (*c == ':') {
			const std::string * result;
			result = xmlnsstack_.NsForPrefix(std::string(qname, c - qname));
			if (result == NULL)
				return QN_EMPTY;
			const char * localname = c + 1;
			return QName(*result, localname); 
		}
	}
	if (isAttr) {
		return QName(STR_EMPTY, qname);
	}

	const std::string * result;
	result = xmlnsstack_.NsForPrefix(STR_EMPTY);
	if (result == NULL)
		return QN_EMPTY;

	return QName(*result, qname);
}


std::string ToString(const TCHAR *str) 
{
	if (str == NULL)
		return std::string();

#ifdef UNICODE

	size_t size = lstrlenW(str) + 1;
	char *tmp = (char *) mir_alloc(size);

	WideCharToMultiByte(CP_UTF8, 0, str, -1, tmp, size, NULL, NULL);

	std::string ret(tmp);

	mir_free(tmp);

	return ret;

#else

	return std::string(str);

#endif
}


static XmlElement *ConvertToXmlElement(DATA *data, buzz::XmlnsStack &xmlnsstack_, IXmlNode *jabberNode) 
{
	xmlnsstack_.PushFrame();

	for (int i=0; i < data->jabber->pfXmlGetNumAttr(jabberNode); i++) 
	{
		const char *name = data->jabber->pfXmlGetAttrName(jabberNode, i);
		if (StartsWithXmlns(name))
		{
			const TCHAR *value = data->jabber->pfXmlGetAttrValue(jabberNode, i);
			if (name[5] == '\0') 
			{
				xmlnsstack_.AddXmlns(STR_EMPTY, ToString(value));
			}
			else if (name[5] == ':') 
			{
				xmlnsstack_.AddXmlns(std::string(name + 6), ToString(value));
			}
		}
	}

	QName tagName(ResolveQName(xmlnsstack_, data->jabber->pfXmlGetNodeName(jabberNode), false));
	if (tagName == QN_EMPTY) 
	{
		xmlnsstack_.PopFrame();
		return NULL;
	}

	XmlElement * pelNew = new XmlElement(tagName);

	for ( int i=0; i < data->jabber->pfXmlGetNumAttr(jabberNode); i++ ) 
	{
		QName attName(ResolveQName(xmlnsstack_, data->jabber->pfXmlGetAttrName(jabberNode, i), true));
		if (attName == QN_EMPTY)
			continue;
		
		pelNew->AddAttr(attName, ToString(data->jabber->pfXmlGetAttrValue(jabberNode, i)));
	}

	const TCHAR *text = data->jabber->pfXmlGetNodeText(jabberNode);
	if (text != NULL) 
		pelNew->AddText(ToString(text));

	for ( int i=0; i < data->jabber->pfXmlGetNumChild(jabberNode); i++ )
	{
		XmlElement *tmp = ConvertToXmlElement(data, data->jabber->pfXmlGetChild(jabberNode, i));
		if (tmp != NULL)
			pelNew->AddElement(tmp);
	}

	xmlnsstack_.PopFrame();
	return pelNew;
}

// Convert an XML element from our representation to libjingle's one
static XmlElement *ConvertToXmlElement(DATA *data, IXmlNode *jabberNode) 
{
	return ConvertToXmlElement(data, data->xmlnsstack_, jabberNode);
}

static int VoiceGetInfo(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	DATA *data = jabbers[(int) param];
	if (data == NULL)
		return 0;

	return VOICE_SUPPORTED | VOICE_CALL_CONTACT | VOICE_CALL_CONTACT_NEED_TEST;
}

static int ContactValidForVoice(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	DATA *data = jabbers[(int) param];
	if (data == NULL)
		return FALSE;

	if (lParam == FALSE)
		return TRUE;

	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return FALSE;

	DBVARIANT dbv;
	if (DBGetContactSettingTString(hContact, data->jabber->protocolName, "jid", &dbv))
		return FALSE;

	TCHAR jid[512];
	_tcsncpy(jid, dbv.ptszVal, MAX_REGS(jid));
	DBFreeVariant(&dbv);

	if (DBGetContactSettingWord(hContact, data->jabber->protocolName, "Status", ID_STATUS_OFFLINE) <= ID_STATUS_OFFLINE)
		return FALSE;

	return (data->jabber->pfGetResourceCapabilites(jid) & JABBER_CAPS_JINGLE_VOICE) == JABBER_CAPS_JINGLE_VOICE;
}

int VoiceCall(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	if (wParam == NULL)
		return -1;

	DATA *data = jabbers[(int) param];
	if (data == NULL)
		return FALSE;

	if (data->call_client_ != NULL)
		data->call_client_->MakeCallTo((HANDLE) wParam);
	return 0;
}

static int VoiceAnswer(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	if (wParam == NULL)
		return -1;

	DATA *data = jabbers[(int) param];
	if (data == NULL)
		return FALSE;

	if (data->call_client_ != NULL)
		data->call_client_->AnswerCall(atoi((const char *) wParam));
	return 0;
}

static int VoiceDrop(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	if (wParam == NULL)
		return -1;

	DATA *data = jabbers[(int) param];
	if (data == NULL)
		return FALSE;

	if (data->call_client_ != NULL)
		data->call_client_->DropCall(atoi((const char *) wParam));
	return 0;
}


VOID CALLBACK ExpireMessagesTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	DATA *data = jabbers[(int) idEvent];
	if (data == NULL)
		return;

	data->signaling_thread_->Post(data->call_client_, MSG_TIMER);
}


void AssertLibjingleStarted(int id, DATA *data) 
{
	if (data->started)
		return;

	// Libjingle stuff
	data->network_manager_ = new talk_base::NetworkManager();
	data->worker_thread_ = new talk_base::Thread();
	data->signaling_thread_ = new talk_base::Thread();
	data->port_allocator_ = new cricket::HttpPortAllocator(data->network_manager_, "call");
	data->session_manager_task_ = new SessionManagerTask(data);
	data->session_manager_ = new cricket::SessionManager(data->port_allocator_, data->worker_thread_, data->signaling_thread_);
	data->session_manager_->SignalOutgoingMessage.connect(data->session_manager_task_, &SessionManagerTask::OnOutgoingMessage);

	data->worker_thread_->Start();
	data->signaling_thread_->Start();

	//talk_base::ThreadManager::SetCurrent(signaling_thread_);

	data->call_client_ = new CallClient(data);
	data->session_manager_->SignalRequestSignaling.connect(data->call_client_, &CallClient::OnSignalingReady);

	SetTimer(NULL, id, 1000, ExpireMessagesTimer);

	data->started = true;
}


BOOL HandleIqReturn(DATA *data, IXmlNode *node, const TCHAR *id)
{
	//TCHAR *from = data->jabber->pfXmlGetAttrValueStr(node, "from");
	//if (from == NULL)
	//	return FALSE;

	if (lstrlen(id) < 5)
		return FALSE;

	int iqId = _ttoi(&id[4]);

	EnterCriticalSection(&data->csPendingIqMap);

	PendingIqMap::iterator it = data->pendingIqs.find(iqId);
	if (it == data->pendingIqs.end())
	{
		LeaveCriticalSection(&data->csPendingIqMap);
		return FALSE;
	}

	//XmlElement *oldStanza = it->second.stanza;
	//if (oldStanza->Attr(QN_TO) != ToString(from))
	//{
	//	LeaveCriticalSection( &csPendingIqMap );
	//	return FALSE;
	//}

	PendingIq iq = it->second;
	data->pendingIqs.erase(it);

	LeaveCriticalSection(&data->csPendingIqMap);

	iq.callback(data, iq.param, node);
	return TRUE;
}


BOOL HandleSet(DATA *data, IXmlNode *node, const TCHAR *id) 
{
	IXmlNode *sessionNode = data->jabber->pfXmlGetChildWithGivenAttrValue(node, "session", "xmlns", _T("http://www.google.com/session"));
	if (sessionNode == NULL)
		return FALSE;

	data->xmlnsstack_.Reset();
	XmlElement *el = ConvertToXmlElement(data, node);
	if (el == NULL)
		return FALSE;

//	OutputDebugStringA("<IN>\n\t");
//	OutputDebugStringA(el->Str().c_str());
//	OutputDebugStringA("\n</IN>\n");

	data->signaling_thread_->Post(data->call_client_, MSG_INCOMING_MSG, (talk_base::MessageData *) el);
	return TRUE;
}


BOOL HandleGet(DATA *data, IXmlNode *node, const TCHAR *id) 
{
	IXmlNode *query = data->jabber->pfXmlGetChildWithGivenAttrValue(node, "query", "xmlns", _T("http://jabber.org/protocol/disco#info"));
	if (query == NULL)
		return FALSE;

	const TCHAR *feat = data->jabber->pfXmlGetAttrValueStr(query, "node");
	if (feat == NULL)
		return FALSE;

	if (lstrcmp(feat, _T(JABBER_CAPS_MIRANDA_NODE) _T("#") _T(JABBER_EXT_JINGLE_VOICE)) != 0)
		return FALSE;

	const TCHAR *from = data->jabber->pfXmlGetAttrValueStr(node, "from");
	if (from == NULL)
		return FALSE;

	IXmlNode *iq = data->jabber->pfXmlCreateNode("iq");
	data->jabber->pfXmlAddAttr(iq, "type", _T("result"));
	data->jabber->pfXmlAddAttr(iq, "to", from);
	data->jabber->pfXmlAddAttr(iq, "id", id);

	IXmlNode *q = data->jabber->pfXmlAddChild(iq, "query");
	data->jabber->pfXmlAddAttr(q, "xmlns", _T("http://jabber.org/protocol/disco#info"));

	char *ver = data->jabber->pfGetVersionText();
	TCHAR capsVer[512];
	mir_sntprintf(capsVer, MAX_REGS(capsVer), _T(JABBER_CAPS_MIRANDA_NODE) _T("#") _T(TCHAR_STR_PARAM), ver);
	data->jabber->pfXmlAddAttr(q, "node", capsVer);

	IXmlNode *f = data->jabber->pfXmlAddChild(q, "feature");
	data->jabber->pfXmlAddAttr(f, "var", _T(JABBER_FEAT_JINGLE_VOICE));

	data->jabber->pfSendNode(iq);

	mir_free(ver);
	return TRUE;
}


BOOL OnXmlReceived(void *param, IXmlNode *node)
{
	DATA *data = jabbers[(int) param];
	if (data == NULL)
		return FALSE;

	if (!data->started)
		return FALSE;

	const char *name = data->jabber->pfXmlGetNodeName(node);
	if (name == NULL || strcmp(name, "iq") != 0)
		return FALSE;

	const TCHAR *type = data->jabber->pfXmlGetAttrValueStr(node, "type");
	if (type == NULL)
		return FALSE;

	const TCHAR * id = data->jabber->pfXmlGetAttrValueStr(node, "id");
	if (id == NULL)
		return FALSE;

	if (_tcscmp(type, _T("result")) == 0 || _tcscmp(type, _T("error")) == 0)
		return HandleIqReturn(data, node, id);

	else if (_tcscmp(type, _T("set")) == 0)
		return HandleSet(data, node, id);

	else if (_tcscmp(type, _T("get")) == 0)
		return HandleGet(data, node, id);

	else
		return FALSE;
}

void JingleInfoResult(DATA *data, void *param, IXmlNode *iqNode)
{
	const TCHAR *type = data->jabber->pfXmlGetAttrValueStr(iqNode, "type");
	if (_tcscmp( type, _T("error")) == 0) 
		return;

	IXmlNode *query = data->jabber->pfXmlGetChildWithGivenAttrValue(iqNode, "query", "xmlns", _T("google:jingleinfo"));
	if (query == NULL)
		return;

	std::vector<std::string> relay_hosts;
	std::vector<talk_base::SocketAddress> stun_hosts;
	std::string relay_token;

	IXmlNode *stun = data->jabber->pfXmlGetChildByName(query, "stun");
	if (stun != NULL)
	{
		for (int i = 0; i < data->jabber->pfXmlGetNumChild(stun); i++)
		{
			IXmlNode *child = data->jabber->pfXmlGetChild(stun, i);
			const char *name = data->jabber->pfXmlGetNodeName(child);
			if (name != NULL && !strcmp("server", name))
			{
				std::string host = ToString(data->jabber->pfXmlGetAttrValueStr(child, "host"));
				std::string port = ToString(data->jabber->pfXmlGetAttrValueStr(child, "udp"));
				if (host.length() > 0 && port.length() > 0)
					stun_hosts.push_back(talk_base::SocketAddress(host, atoi(port.c_str())));
			}
		}
	}

	IXmlNode *relay = data->jabber->pfXmlGetChildByName(query, "relay");
	if (relay != NULL)
	{
		for (int i = 0; i < data->jabber->pfXmlGetNumChild(relay); i++)
		{
			IXmlNode *child = data->jabber->pfXmlGetChild(relay, i);
			const char *name = data->jabber->pfXmlGetNodeName(child);

			if (name == NULL)
				continue;

			if (strcmp("server", name) == 0)
			{
				std::string host = ToString(data->jabber->pfXmlGetAttrValueStr(child, "host"));
				if (host.length() > 0)
					relay_hosts.push_back(host);
			}
			else if (strcmp("token", name) == 0)
			{
				const TCHAR *text = data->jabber->pfXmlGetNodeText(child);
				if (text != NULL && text[0] != '\0')
					relay_token = ToString(text);
			}
		}
	}

	data->port_allocator_->SetStunHosts(stun_hosts);
	data->port_allocator_->SetRelayHosts(relay_hosts);
	data->port_allocator_->SetRelayToken(relay_token);
}


void OnConnect(void *param, TCHAR *fullJID)
{
	DATA *data = jabbers[(int) param];
	if (data == NULL)
		return;

	AssertLibjingleStarted((int) param, data);

	_tcsncpy(data->fullJID, fullJID, MAX_REGS(data->fullJID));
	data->call_client_->OnConnect();

	int iqId = data->jabber->pfIqSerialNext();

	IXmlNode *iq = data->jabber->pfXmlCreateNode("iq"); 
	data->jabber->pfXmlAddAttr(iq, "type", _T("get")); 
	data->jabber->pfXmlAddAttrID(iq, iqId); 
	IXmlNode *query = data->jabber->pfXmlAddChild(iq, "query"); 
	data->jabber->pfXmlAddAttr(query, "xmlns", _T("google:jingleinfo")); 

	data->jabber->pfSendNode(iq);
	data->jabber->pfXmlDeleteNode(iq);

	EnterCriticalSection(&data->csPendingIqMap);
	PendingIq &piq = data->pendingIqs[iqId];
	piq.sent = time(NULL);
	piq.callback = JingleInfoResult;
	piq.param = 0;
	LeaveCriticalSection(&data->csPendingIqMap);
}

void OnDisconnect(void *param)
{
	DATA *data = jabbers[(int) param];
	if (data == NULL)
		return;

	if (!data->started)
		return;
	
	data->call_client_->OnDisconnect();
	data->signaling_thread_->Post(data->call_client_, TERMINATE_ALL, NULL);
}

BOOL OnXmlSendNode(void *param, IXmlNode *node)
{
	DATA *data = jabbers[(int) param];
	if (data == NULL)
		return TRUE;

	const char *name = data->jabber->pfXmlGetNodeName(node);
	if (name == NULL || strcmp(name, "presence") != 0)
		return TRUE;

	IXmlNode *c = data->jabber->pfXmlGetChildByName(node, "c");
	if (c == NULL)
		return TRUE;

	const TCHAR *ext = data->jabber->pfXmlGetAttrValueStr(c, "ext");
	if (ext == NULL)
		data->jabber->pfXmlAddAttr(c, "ext", _T(JABBER_EXT_JINGLE_VOICE));
	else
	{
		TCHAR tmp[512];
		mir_sntprintf(tmp, MAX_REGS(tmp), _T("%s %s"), ext, _T(JABBER_EXT_JINGLE_VOICE));
		data->jabber->pfXmlSetAttrValueStr(c, "ext", tmp);
	}

	return TRUE;
}


void RegisterJabberPlugin(const char *proto)
{
	if (!ProtoServiceExists(proto, PS_REGISTER_JABBER_PLUGIN))
		return;

	int id = jabbers.getCount();
	JABBER_PLUGIN_DATA info = {
		sizeof(JABBER_PLUGIN_DATA),
		"Jingle",
		"Enable voice calls",
		TRUE,
		(void *) id,
		OnConnect,
		OnDisconnect,
		OnXmlReceived,
		NULL,
		OnXmlSendNode,
		NULL,
		NULL
	};
	
	
	JABBER_DATA *jabber = (JABBER_DATA *) CallProtoService(proto, PS_REGISTER_JABBER_PLUGIN, (WPARAM) &info, 1);
	if (jabber == NULL)
		// We are disabled / ignored
		return;

	DATA *data = new DATA();
	data->network_manager_ = NULL;
	data->worker_thread_ = NULL;
	data->signaling_thread_ = NULL;
	data->port_allocator_ = NULL;
	data->session_manager_task_ = NULL;
	data->session_manager_ = NULL;
	data->call_client_ = NULL;
	data->started = false;

	data->jabber = jabber;
	
	jabbers.insert(data, jabbers.getCount());

	InitializeCriticalSection(&data->csPendingIqMap);

	// Voice service support
	char tmp[256];

	mir_snprintf(tmp, MAX_REGS(tmp), "%s" PE_VOICE_CALL_STATE, data->jabber->protocolName);
	data->hVoiceNotify = CreateHookableEvent(tmp);

	mir_snprintf(tmp, MAX_REGS(tmp), "%s" PS_VOICE_GETINFO, data->jabber->protocolName);
	CreateServiceFunctionParam(tmp, VoiceGetInfo, id);

	mir_snprintf(tmp, MAX_REGS(tmp), "%s" PS_VOICE_CALL, data->jabber->protocolName);
	CreateServiceFunctionParam(tmp, VoiceCall, id);

	mir_snprintf(tmp, MAX_REGS(tmp), "%s" PS_VOICE_ANSWERCALL, data->jabber->protocolName);
	CreateServiceFunctionParam(tmp, VoiceAnswer, id);

	mir_snprintf(tmp, MAX_REGS(tmp), "%s" PS_VOICE_DROPCALL, data->jabber->protocolName);
	CreateServiceFunctionParam(tmp, VoiceDrop, id);

	mir_snprintf(tmp, MAX_REGS(tmp), "%s" PS_VOICE_CALL_CONTACT_VALID, data->jabber->protocolName);
	CreateServiceFunctionParam(tmp, ContactValidForVoice, id);

	data->jabber->pfSetClientCaps(_T(JABBER_CAPS_MIRANDA_NODE), _T(JABBER_EXT_JINGLE_VOICE), JABBER_CAPS_JINGLE_VOICE);
	data->jabber->pfSetClientCaps(_T(JABBER_CAPS_GOOGLETALK_NODE), _T(JABBER_EXT_JINGLE_VOICE), JABBER_CAPS_JINGLE_VOICE);
	data->jabber->pfSetClientCaps(_T(JABBER_CAPS_GOOGLETALK_NODE), _T(JABBER_EXT_JINGLE_SHARE), JABBER_CAPS_JINGLE_SHARE);
}


__inline static int ProtoServiceExists(const char *szModule,const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}
