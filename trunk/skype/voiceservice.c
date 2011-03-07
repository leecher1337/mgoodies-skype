#include "skype.h"
#include "skypeapi.h"
#include "voiceservice.h"
#include "sdk/m_voiceservice.h"
#include "../../include/m_utils.h"


HANDLE hVoiceNotify = NULL;
BOOL has_voice_service = FALSE;


BOOL HasVoiceService()
{
	return has_voice_service;
}

void NofifyVoiceService(HANDLE hContact, char *callId, int state) 
{
	VOICE_CALL vc = {0};
	vc.cbSize = sizeof(vc);
	vc.szModule = SKYPE_PROTONAME;
	vc.id = callId;
	vc.flags = VOICE_CALL_CONTACT;
	vc.state = state;
	vc.hContact = hContact;
	NotifyEventHooks(hVoiceNotify, (WPARAM) &vc, 0);
}

static INT_PTR VoiceGetInfo(WPARAM wParam, LPARAM lParam)
{
	return VOICE_SUPPORTED | VOICE_CALL_CONTACT | VOICE_CAN_HOLD;
}

static HANDLE FindContactByCallId(char *callId)
{
	HANDLE hContact;
	for (hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
			hContact != NULL;
			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) 
	{
		char *szProto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

		DBVARIANT dbv;
		if (szProto != NULL 
			&& !strcmp(szProto, SKYPE_PROTONAME) 
			&& DBGetContactSettingByte(hContact, SKYPE_PROTONAME, "ChatRoom", 0) == 0 
			&& !DBGetContactSetting(hContact, SKYPE_PROTONAME, "CallId", &dbv)) 
		{
			if (!strcmp(callId, dbv.pszVal))
				return hContact;
		}
	}

	return NULL;
}

static INT_PTR VoiceCall(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	char msg[512];

	if (!wParam) return -1;

	if (DBGetContactSetting((HANDLE)wParam, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) 
		return -1;

	mir_snprintf(msg, sizeof(msg), "CALL %s", dbv.pszVal);
	SkypeSend(msg);

	return 0;
}

static INT_PTR VoiceAnswer(WPARAM wParam, LPARAM lParam)
{
	char *callId = (char *) wParam;
	char msg[512];

	if (!wParam) return -1;
	
	if (FindContactByCallId(callId) == NULL)
		return -1;

	mir_snprintf(msg, sizeof(msg), "SET %s STATUS INPROGRESS", callId);
	SkypeSend(msg);
	testfor("ERROR", 200);

	return 0;
}

static INT_PTR VoiceDrop(WPARAM wParam, LPARAM lParam)
{
	char *callId = (char *) wParam;
	char msg[512];

	if (!wParam) return -1;

	if (FindContactByCallId(callId) == NULL)
		return -1;

	mir_snprintf(msg, sizeof(msg), "SET %s STATUS FINISHED", callId);
	SkypeSend(msg);

	return 0;
}

static INT_PTR VoiceHold(WPARAM wParam, LPARAM lParam)
{
	char *callId = (char *) wParam;
	char msg[512];

	if (!wParam) return -1;

	if (FindContactByCallId(callId) == NULL)
		return -1;

	mir_snprintf(msg, sizeof(msg), "SET %s STATUS ONHOLD", callId);
	SkypeSend(msg);

	return 0;
}

void VoiceServiceInit() 
{
	char szTmp[ 200 ];
	mir_snprintf( szTmp, sizeof(szTmp), "%s" PE_VOICE_CALL_STATE, SKYPE_PROTONAME );
	hVoiceNotify = CreateHookableEvent( szTmp );

	mir_snprintf( szTmp, sizeof(szTmp), "%s" PS_VOICE_GETINFO, SKYPE_PROTONAME );
	CreateServiceFunction( szTmp, VoiceGetInfo );

	mir_snprintf( szTmp, sizeof(szTmp), "%s" PS_VOICE_CALL, SKYPE_PROTONAME );
	CreateServiceFunction( szTmp, VoiceCall );

	mir_snprintf( szTmp, sizeof(szTmp), "%s" PS_VOICE_ANSWERCALL, SKYPE_PROTONAME );
	CreateServiceFunction( szTmp, VoiceAnswer );

	mir_snprintf( szTmp, sizeof(szTmp), "%s" PS_VOICE_DROPCALL, SKYPE_PROTONAME );
	CreateServiceFunction( szTmp, VoiceDrop );

	mir_snprintf( szTmp, sizeof(szTmp), "%s" PS_VOICE_HOLDCALL, SKYPE_PROTONAME );
	CreateServiceFunction( szTmp, VoiceHold );
}

void VoiceServiceModulesLoaded() 
{
	has_voice_service = ServiceExists(MS_VOICESERVICE_REGISTER);
}