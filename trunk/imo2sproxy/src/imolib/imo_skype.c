/* Module:  imo_skype.c
   Purpose: Communication layer for imo.im Skype 
   Author:  leecher
   Date:    30.08.2009
*/
#include <stdlib.h>
#include <string.h>
#include "imo_request.h"
#include "imo_skype.h"

#define PROTO "prpl-skype"

struct _tagIMOSKYPE
{
	IMORQ *hRq;
	IMORQ *hPoll;
	char *pszUser;
	IMOSTATCB StatusCb;
	char *pszLastRes;
	void *pUser;
};

static int CheckReturn (IMOSKYPE *hSkype, char *pszMsg, char *pszExpected);
static int ManageBuddy(IMOSKYPE *hSkype, char *pszAction, char *pszBuddy, char *pszGroup);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

IMOSKYPE *ImoSkype_Init(IMOSTATCB StatusCb, void *pUser)
{
	IMOSKYPE *hSkype = calloc(1, sizeof(IMOSKYPE));

	if (!hSkype) return NULL;
	if (!(hSkype->hRq = ImoRq_Init()) || !(hSkype->hPoll = ImoRq_Clone(hSkype->hRq)))
	{
		ImoSkype_Exit(hSkype);
		return NULL;
	}
	hSkype->StatusCb = StatusCb;
	hSkype->pUser = pUser;
	return hSkype;
}

// -----------------------------------------------------------------------------

void ImoSkype_Exit(IMOSKYPE *hSkype)
{
	if (hSkype->hRq) ImoRq_Exit(hSkype->hRq);
	if (hSkype->hPoll) ImoRq_Exit(hSkype->hPoll);
	if (hSkype->pszUser) free(hSkype->pszUser);
	free (hSkype);
}

// -----------------------------------------------------------------------------

char *ImoSkype_GetLastError(IMOSKYPE *hSkype)
{
	char *pszRet = ImoRq_GetLastError(hSkype->hRq);

	if (!pszRet || !*pszRet) return hSkype->pszLastRes;
	return pszRet;
}

// -----------------------------------------------------------------------------

char *ImoSkype_GetUserHandle(IMOSKYPE *hSkype)
{
	return hSkype->pszUser;
}

// -----------------------------------------------------------------------------

// -1	-	Error
// 0 	-	Login failed
// 1	-	Login successful
int ImoSkype_Login(IMOSKYPE *hSkype, char *pszUser, char *pszPass)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	if (pszRet = ImoRq_PostToSys(hSkype->hRq, "cookie_login", "session", root, 1))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;

	if (!(root=cJSON_CreateObject())) return 0;
	if (hSkype->pszUser) free (hSkype->pszUser);
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser = strdup(pszUser));
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "passwd", pszPass);
	cJSON_AddNullToObject(root, "captcha");	// Uh-oh, thay may get annoying in the future! :(
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "account_login", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);

	return iRet;
}

// -----------------------------------------------------------------------------

// -1	-	Error
// 0 	-	Logout failed
// 1	-	Logout successful
int ImoSkype_Logout(IMOSKYPE *hSkype)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	if (pszRet = ImoRq_PostToSys(hSkype->hRq, "signoff_all", "session", root, 1))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	return iRet;
}

// -----------------------------------------------------------------------------

// -1	-	Error
// 0	-	Received unknown answer
// 1	-	Got back information, called notification callback
// 2	-	Received PING [deprecated]
int ImoSkype_Poll(IMOSKYPE *hSkype)
{
	char *pszRet;

	pszRet = ImoRq_PostSystem(hSkype->hPoll, "forward_to_server", NULL, NULL, NULL, 1);
	if (!pszRet) return -1;
	return CheckReturn (hSkype, pszRet, "ping");
}

// -----------------------------------------------------------------------------

int ImoSkype_KeepAlive(IMOSKYPE *hSkype)
{
	char *pszRet;

	/* In case we want to receive Promo-Infos...
	{
		cJSON *edata = cJSON_CreateObject(), *root;

		root=cJSON_CreateObject();
		cJSON_AddStringToObject(edata, "kind", "web");
		cJSON_AddNumberToObject(edata, "quantity", 1);
		cJSON_AddItemToObject(root, "edata", edata);
		if (pszRet = ImoRq_PostToSys(hSkype->hRq, "get_promos", "promo", root, 1))
			CheckReturn(hSkype, pszRet, "ok");
	}
	*/

	pszRet = ImoRq_UserActivity(hSkype->hRq);
	if (!pszRet) return -1;
	return CheckReturn (hSkype, pszRet, "ok");
}

// -----------------------------------------------------------------------------

// pszStatus:
// Valid states:
//	typing
//	typed
//	not_typing
//
// -1	-	Error
// 0 	-	Typing notification failed
// 1	-	Typing notification  successful
int ImoSkype_Typing(IMOSKYPE *hSkype, char *pszBuddy, char *pszStatus)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "buid", pszBuddy);
	cJSON_AddStringToObject(root, "status", pszStatus);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "im_typing", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

// -1	-	Error
// 0 	-	Sending failed
// 1	-	Send pending
int ImoSkype_SendMessage(IMOSKYPE *hSkype, char *pszBuddy, char *pszMessage)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "buid", pszBuddy);
	cJSON_AddStringToObject(root, "msg", pszMessage);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "send_im", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

// pszStatus:
// Valid states:
//	available
//	away
//	busy
//	invisible
//
// -1	-	Error
// 0 	-	Failed
// 1	-	OK
int ImoSkype_SetStatus(IMOSKYPE *hSkype, char *pszStatus, char *pszStatusMsg)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	/*
	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "ad", "");
	cJSON_AddStringToObject(root, "primitive", pszStatus);
	cJSON_AddStringToObject(root, "status", pszStatusMsg);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "set_status", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	*/

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "ad", "");
	cJSON_AddStringToObject(root, "primitive", pszStatus);
	cJSON_AddStringToObject(root, "status", pszStatusMsg);
	if (pszRet = ImoRq_PostToSys(hSkype->hRq, "set_status", "session", root, 0))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;

	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

// -1	-	Error
// 0 	-	Failed
// 1	-	OK
int ImoSkype_AddBuddy(IMOSKYPE *hSkype, char *pszBuddy)
{
	return ManageBuddy (hSkype, "add_buddy", pszBuddy, "Offline");
}

// -----------------------------------------------------------------------------

// pszGroup = "Offline" if the user if offline, otherwise "Skype" or "Buddies"
// -1	-	Error
// 0 	-	Failed
// 1	-	OK
int ImoSkype_DelBuddy(IMOSKYPE *hSkype, char *pszBuddy, char *pszGroup)
{
	int iRet = ManageBuddy (hSkype, "del_buddy", pszBuddy, pszGroup);

	if (iRet<1 && strcmp(pszGroup, "Skype")==0)
		return ManageBuddy (hSkype, "del_buddy", pszBuddy, "Buddies");
	return iRet;
}
// -----------------------------------------------------------------------------

int ImoSkype_BlockBuddy(IMOSKYPE *hSkype, char *pszBuddy)
{
	return ManageBuddy (hSkype, "block_buddy", pszBuddy, NULL);
}

// -----------------------------------------------------------------------------

int ImoSkype_UnblockBuddy(IMOSKYPE *hSkype, char *pszBuddy)
{
	return ManageBuddy (hSkype, "unblock_buddy", pszBuddy, NULL);
}

// -----------------------------------------------------------------------------

int ImoSkype_ChangeAlias(IMOSKYPE *hSkype, char *pszBuddy, char *pszNewAlias)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "buid", pszBuddy);
	cJSON_AddStringToObject(root, "alias", pszNewAlias);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "change_buddy_alias", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

int ImoSkype_StartVoiceCall(IMOSKYPE *hSkype, char *pszBuddy)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "buid", pszBuddy);
	if (pszRet = ImoRq_PostToSys (hSkype->hRq, "start_audio_chat", "av", root, 1))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	return iRet;
}

// -----------------------------------------------------------------------------

int ImoSkype_Ping(IMOSKYPE *hSkype)
{
	char *pszRet;
	int iRet = -1;

	if (pszRet = ImoRq_Echo(hSkype->hRq))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	return iRet;
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------
// 0	-	Unexpected answer
// 1	-	Got back JSON data, notified callback
// 2	-	Received expected message pszExpected [deprecated]
static int CheckReturn (IMOSKYPE *hSkype, char *pszMsg, char *pszExpected)
{
	cJSON *root, *data, *msgs, *msg, *sys, *arr;
	char *pszMethod, *pszSys;

	hSkype->pszLastRes = pszMsg;
	if (root = cJSON_Parse(pszMsg))
	{
		// Now let's see if this one is interesting for our system
		if ((data = cJSON_GetObjectItem(root,"method")) &&
			(pszMethod = data->valuestring) &&
			strcmp(pszMethod, "forward_to_client") == 0 &&
			(data = cJSON_GetObjectItem(root,"data")) &&
			(msgs = cJSON_GetObjectItem(data,"messages")))
		{
			int i, iCount = cJSON_GetArraySize(msgs);

			if (!iCount && pszExpected && strcmp(pszExpected, "ok") == 0) return 2;	// Empty msg = ok?
			for (i=0; i<iCount; i++)
			{
				if (msg = cJSON_GetArrayItem(msgs, i))
				{
					// Is this for me?
					if ((sys = cJSON_GetObjectItem(msg,"to")) &&
						(pszSys = cJSON_GetObjectItem(sys, "system")->valuestring) &&
						strcmp (pszSys, "client") == 0)
					{
						if (sys = cJSON_GetObjectItem(msg,"seq"))
						{
							ImoRq_UpdateAck(hSkype->hRq, sys->valueint+1);
							ImoRq_UpdateAck(hSkype->hPoll, sys->valueint+1);
						}

						// Callback is only called for system IM
						if ((sys = cJSON_GetObjectItem(msg,"from")) &&
						(pszSys = cJSON_GetObjectItem(sys, "system")->valuestring) &&
						(strcmp (pszSys, "im") == 0 || strcmp (pszSys, "av") == 0) &&

						(data = cJSON_GetObjectItem(msg,"data")) &&
						(arr = cJSON_CreateArray()))
						{
							// Pack data into array for Callback backwards
							// compatibility
							cJSON *next;

							next = data->next;
							data->next = NULL;
							cJSON_AddItemToArray (arr, data);
							hSkype->StatusCb(arr, hSkype->pUser);
							data->next = next;
							free(arr);
						}
					}
				}
			}
		}
		cJSON_Delete(root);
		return 1;
	}
	else
	{
		if (pszExpected && strcmp(pszMsg, pszExpected)==0)
			return 2;
	}
	return 0;
}

// -----------------------------------------------------------------------------

static int ManageBuddy(IMOSKYPE *hSkype, char *pszAction, char *pszBuddy, char *pszGroup)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "buid", pszBuddy);
	if (pszGroup) cJSON_AddStringToObject(root, "group", pszGroup);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, pszAction, root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);
	return iRet;
}
