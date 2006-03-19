#include "status_msg.h"


HANDLE hProtoAck = NULL;

int ProtoAck(WPARAM wParam, LPARAM lParam);




void InitStatusMsgs()
{
	hProtoAck = HookEvent(ME_PROTO_ACK, ProtoAck);
}


void FreeStatusMsgs()
{
	UnhookEvent(hProtoAck);
}


void ClearStatusMessage(HANDLE hContact)
{
	DBWriteContactSettingString(hContact, "CList", "StatusMsg", "");
}


void SetStatusMessage(HANDLE hContact, const TCHAR *msg)
{
	DBWriteContactSettingString(hContact, "CList", "StatusMsg", msg);
}



int ProtoAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (ack->type == ACKTYPE_STATUS)
	{
		WORD status = (WORD)ack->lParam;
		char *proto = (char*)ack->szModule;

		if (PoolCheckProtocol(proto) && status > ID_STATUS_OFFLINE)
			PoolAddAllContacts(ONLINE_TIMER, proto, FALSE);

	}
	else if (ack->type == ACKTYPE_AWAYMSG)
	{
		PoolRemoveContact(ack->hContact);

		if (ack->result == ACKRESULT_SUCCESS)
		{
logC(MODULE_NAME, "ProtoAck", ack->hContact, "Status msg changed");

			SetStatusMessage(ack->hContact, (const TCHAR *) ack->lParam);
		}
	}

	return 0; //The protocol changed in a way we don't care.
}
