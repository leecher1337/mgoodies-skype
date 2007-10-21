/*
 * Jingle call example
 * Copyright 2004--2005, Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



#include "commons.h"




CallClient::CallClient(DATA *aData)
{
	data = aData;
	phone_client_ = NULL;
	signaling = false;
	needSignal = false;
}

CallClient::~CallClient() 
{
}

void CallClient::StartSignaling()
{
	if (!signaling && needSignal)
		data->session_manager_->OnSignalingReady();
	
	signaling = true;
}

void CallClient::StopSignaling()
{
	signaling = false;
	needSignal = false;
}

void CallClient::OnSignalingReady() 
{
	if (signaling)
		data->session_manager_->OnSignalingReady();
	else
		needSignal = true;
}

void CallClient::OnConnect()
{
	std::string jid = ToString(data->fullJID);
	cricket::InitRandom(jid.c_str(), jid.size());

	phone_client_ = new cricket::PhoneSessionClient(buzz::Jid(jid), data->session_manager_);
	phone_client_->SignalCallCreate.connect(this, &CallClient::OnCallCreate);
}

void CallClient::OnDisconnect()
{
	if (phone_client_ != NULL)
	{
		delete phone_client_;
		phone_client_ = NULL;
	}
}

void CallClient::OnCallCreate(cricket::Call* call) 
{
	call->SignalSessionState.connect(this, &CallClient::OnSessionState);
}

void CallClient::OnCallDestroy(cricket::Call* call) 
{
}

#ifdef UNICODE

static WCHAR *mir_dupToUnicode(const char *ptr)
{
	if (ptr == NULL)
		return NULL;

	size_t size = strlen(ptr) + 1;
	WCHAR *tmp = (WCHAR *) mir_alloc(size * sizeof(WCHAR));

	MultiByteToWideChar(CP_ACP, 0, ptr, -1, tmp, size * sizeof(WCHAR));

	return tmp;
}

#endif

void CallClient::NofifyState(cricket::Call* call,
							 cricket::Session* session,
							 int state) 
{
	char id[20];
	VOICE_CALL vc = {0};
	vc.cbSize = sizeof(vc);
	vc.szModule = data->jabber->protocolName;
	vc.id = itoa(call->id(), id, 10);
	vc.flags = VOICE_CALL_CONTACT;
	vc.state = state;
#ifdef UNICODE
	TCHAR *jid = mir_dupToUnicode(session->remote_name().c_str());
	vc.hContact = data->jabber->pfHContactFromJID(jid);
	mir_free(jid);
#else
	vc.hContact = data->jabber->pfHContactFromJID(session->remote_name().c_str());
#endif
	NotifyEventHooks(data->hVoiceNotify, (WPARAM) &vc, 0);
}

void CallClient::OnSessionState(cricket::Call* call,
                                cricket::Session* session,
                                cricket::Session::State state) 
{
	if (state == cricket::Session::STATE_RECEIVEDINITIATE) {
		NofifyState(call, session, VOICE_STATE_RINGING);

	} else if (state == cricket::Session::STATE_SENTINITIATE) {
		NofifyState(call, session, VOICE_STATE_CALLING);

	} else if (state == cricket::Session::STATE_SENTACCEPT
				|| state == cricket::Session::STATE_RECEIVEDACCEPT) {
		StartSignaling();

	} else if (state == cricket::Session::STATE_INPROGRESS) {
		NofifyState(call, session, VOICE_STATE_TALKING);

	} else if (state == cricket::Session::STATE_DEINIT) {
		NofifyState(call, session, VOICE_STATE_ENDED);
		StopSignaling();
	}
}

void CallClient::OnMessage(talk_base::Message *pmsg)
{
	switch(pmsg->message_id)
	{
		case MSG_TIMER:
		{
			EnterCriticalSection(&data->csPendingIqMap);

			time_t expire = time(NULL) - 3;
			for(PendingIqMap::iterator it = data->pendingIqs.begin(); it != data->pendingIqs.end(); )
			{
				PendingIq &piq = it->second;
				if (piq.sent < expire)
				{
					buzz::XmlElement *stanza = (buzz::XmlElement *) piq.param;
					data->session_manager_->OnFailedSend(stanza, NULL);
					delete stanza;

					it = data->pendingIqs.erase(it);
				}
				else
					++it;
			}

			LeaveCriticalSection(&data->csPendingIqMap);
			break;
		}
		case MSG_REPLY_MSG:
		{
			ReplyMessageData *rmd = (ReplyMessageData *) pmsg->pdata;
			
			if (rmd->newStanza->Attr(buzz::QN_TYPE) == buzz::STR_RESULT) {
				data->session_manager_->OnIncomingResponse(rmd->oldStanza, rmd->newStanza);
			} else {
				data->session_manager_->OnFailedSend(rmd->oldStanza, rmd->newStanza);
			}

			delete rmd->oldStanza;
			delete rmd->newStanza;
			delete rmd;

			break;
		}
		case MSG_INCOMING_MSG:
		{
			buzz::XmlElement *el = (buzz::XmlElement *) pmsg->pdata;
			if (el == NULL)
				return;

			if (data->session_manager_->IsSessionMessage(el))
				data->session_manager_->OnIncomingMessage(el);

			delete el;
			break;
		}
		case MSG_MAKE_CALL_TO:
		{
			HANDLE hContact = (HANDLE) pmsg->pdata;
			if ( hContact == NULL )
				return;

			DBVARIANT dbv;
			if (DBGetContactSettingTString(hContact, data->jabber->protocolName, "jid", &dbv))
				return;

			TCHAR jid[512];
			_tcsncpy(jid, dbv.ptszVal, MAX_REGS(jid));
			DBFreeVariant(&dbv);

			if (DBGetContactSettingWord(hContact, data->jabber->protocolName, "Status", ID_STATUS_OFFLINE) <= ID_STATUS_OFFLINE)
				return;

			TCHAR szJid[512];
			const TCHAR *bestResName = data->jabber->pfGetBestClientResourceNamePtr(jid);
			mir_sntprintf(szJid, MAX_REGS(szJid), bestResName ? _T("%s/%s") : _T("%s"), jid, bestResName);

			phone_client_->SignalCallDestroy.connect(this, &CallClient::OnCallDestroy);
			cricket::Call * call = phone_client_->CreateCall();
			call->InitiateSession(buzz::Jid(ToString(szJid)), NULL);
			phone_client_->SetFocus(call);
			break;
		}
		case MSG_ANSWER_CALL:
		{
			uint32 id = (uint32) pmsg->pdata;
			cricket::Call * call = phone_client_->GetCall(id);
			if (call == NULL)
				return;

			call->AcceptSession(call->sessions()[0]);
			phone_client_->SetFocus(call);
			break;
		}
		case MSG_DROP_CALL:
		{
			uint32 id = (uint32) pmsg->pdata;
			cricket::Call * call = phone_client_->GetCall(id);
			if (call == NULL)
				return;

			if (call->sessions()[0]->state() == cricket::Session::STATE_RECEIVEDINITIATE 
				|| call->sessions()[0]->state() == cricket::Session::STATE_RECEIVEDMODIFY)
				call->RejectSession(call->sessions()[0]);
			else 
				call->Terminate();
			break;
		}
		case TERMINATE_ALL:
		{
			data->session_manager_->TerminateAll();
			break;
		}
	}
}

void CallClient::MakeCallTo(HANDLE hContact) 
{
	if ( hContact == NULL )
		return;

	data->signaling_thread_->Post(this, MSG_MAKE_CALL_TO, (talk_base::MessageData *) hContact);
}

void CallClient::AnswerCall(int id)
{
	data->signaling_thread_->Post(this, MSG_ANSWER_CALL, (talk_base::MessageData *) id);
}

void CallClient::DropCall(int id) 
{
	data->signaling_thread_->Post(this, MSG_DROP_CALL, (talk_base::MessageData *) id);
}

