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

#ifndef __CALLCLIENT_H__
#define __CALLCLIENT_H__


namespace talk_base {
	class Thread;
	class NetworkManager;
}

namespace cricket {
	class PortAllocator;
	class PhoneSessionClient;
	class Receiver;
	class Call;
	class SessionManagerTask;
}

#define MSG_MAKE_CALL_TO 11
#define MSG_ANSWER_CALL 12
#define MSG_DROP_CALL 13
#define MSG_INCOMING_MSG 14
#define MSG_TIMER 15
#define MSG_REPLY_MSG 16
#define TERMINATE_ALL 17


class ReplyMessageData : public talk_base::MessageData {
public:
	buzz::XmlElement *oldStanza;
	buzz::XmlElement *newStanza;
};



class CallClient : public sigslot::has_slots<>, public talk_base::MessageHandler {
public:
	CallClient(DATA *aData);
	virtual ~CallClient();

	void OnConnect();
	void OnDisconnect();

	void MakeCallTo(HANDLE hContact);
	void AnswerCall(int id);
	void DropCall(int id);
	void OnSignalingReady();

	virtual void OnMessage(talk_base::Message *pmsg);

private:
	DATA *data;

	bool signaling;
	bool needSignal;

	void StartSignaling();
	void StopSignaling();

	cricket::PhoneSessionClient *phone_client_;

	void NofifyState(cricket::Call* call, cricket::Session* session, int state);

	void OnCallCreate(cricket::Call* call);
	void OnCallDestroy(cricket::Call* call);
	void OnSessionState(cricket::Call* call, cricket::Session* session, cricket::Session::State state);
};


#endif // __CALLCLIENT_H__
