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
		WORD newStatus = (WORD)ack->lParam;
		WORD oldStatus = (WORD)ack->hProcess;
		char *proto = (char*)ack->szModule;

		if ((oldStatus <= ID_STATUS_OFFLINE || oldStatus == ID_STATUS_INVISIBLE) 
				&& newStatus > ID_STATUS_OFFLINE && newStatus != ID_STATUS_INVISIBLE
				&& PollCheckProtocol(proto))
		{
			if (opts.poll_check_on_status_change)
				PollAddAllContactsProtoOnline(ONLINE_TIMER, proto);
		}
	}
	else if (ack->type == ACKTYPE_AWAYMSG)
	{
		char *proto = (char*)ack->szModule;

		if (PollCheckProtocol(proto))
		{
			if (ack->result == ACKRESULT_SUCCESS)
			{
logC(MODULE_NAME, "ProtoAck", ack->hContact, "Status msg changed");
				PollReceivedContactMessage(ack->hContact, TRUE);
				SetStatusMessage(ack->hContact, (const TCHAR *) ack->lParam);
			}
			else if (ack->result == ACKRESULT_FAILED)
			{
log(MODULE_NAME, "ProtoAck", "ERROR, pausing for a while");
				PollPause();
			}
		}
	}

	return 0; //The protocol changed in a way we don't care.
}
