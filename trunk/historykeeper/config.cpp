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


/*
 *   Service SameClients MS_FP_SAMECLIENTS
 *	 wParam - char * first MirVer value 
 *   lParam - char * second MirVer value 
 *	 return pointer to char string - client desription (DO NOT DESTROY) if clients are same otherwise NULL
 */
#define MS_FP_SAMECLIENTS "Fingerprint/SameClients"




// See if a protocol service exists
__inline static int ProtoServiceExists(const char *szModule,const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}


static TCHAR *GetTString(HANDLE hContact, char *module, char *setting) 
{
	TCHAR *ret = NULL;

	DBVARIANT db = {0};
	if (DBGetContactSettingTString(hContact, module, setting, &db) == 0)
	{
		if (db.ptszVal != NULL && db.ptszVal[0] != _T('\0'))
			ret = mir_tstrdup(db.ptszVal);
		DBFreeVariant(&db);
	}

	if (ret == NULL)
		ret = mir_tstrdup(TranslateT("<empty>"));

	return ret;
}


static TCHAR *GetCurrentTString(HANDLE hContact, char *module, char *setting) 
{
	char tmp[256];
	mir_snprintf(tmp, MAX_REGS(tmp), "%s_%s_Current", module, setting);

	TCHAR *ret = NULL;

	DBVARIANT db = {0};
	if (DBGetContactSettingTString(hContact, MODULE_NAME, tmp, &db) == 0)
	{
		if (db.ptszVal != NULL && db.ptszVal[0] != _T('\0'))
			ret = mir_tstrdup(db.ptszVal);
		DBFreeVariant(&db);
	}

	if (ret == NULL)
		ret = mir_tstrdup(TranslateT("<empty>"));

	return ret;
}


static void StatusAddVars(HANDLE hContact, TCHAR **vars, int i)
{
	vars[i++] = _T("%msg%");
	vars[i++] = GetTString(hContact, "CList", "StatusMsg");
}


static void XStatusAddVars(HANDLE hContact, TCHAR **vars, int i)
{
	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);

	vars[i++] = _T("%msg%");
	vars[i++] = GetTString(hContact, proto, "XStatusMsg");
}


static void ClientAddVars(HANDLE hContact, TCHAR **vars, int i)
{
	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);

	vars[i++] = _T("%MirVer_new%");
	vars[i++] = GetTString(hContact, proto, "MirVer");
	vars[i++] = _T("%MirVer_old%");
	vars[i++] = GetCurrentTString(hContact, proto, "MirVer");
}


BOOL SMHAllowProtocol(const char *proto)
{	
	return (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGRECV) != 0;
}


BOOL XStatusAllowProtocol(const char *proto)
{	
	return ProtoServiceExists(proto, PS_ICQ_GETCUSTOMSTATUS);
}


void StatusFormat(TCHAR *out, size_t out_size, void *val)
{
	lstrcpyn(out, (TCHAR *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (DWORD) val, GCMDF_TCHAR), out_size);
}


void ClientFormat(TCHAR *out, size_t out_size, void *val)
{
	char *name = NULL;

	if (ServiceExists(MS_FP_SAMECLIENTS))
	{
		char *tmp = mir_t2a((TCHAR *) val);
		name = (char *) CallService(MS_FP_SAMECLIENTS, (WPARAM) tmp, (LPARAM) tmp);
		mir_free(tmp);
	}

	if (name != NULL)
	{
		TCHAR *tmp = mir_a2t(name);
		lstrcpyn(out, tmp, out_size);
		mir_free(tmp);
	}
	else
	{
		lstrcpyn(out, (TCHAR *) val, out_size);
	}
}

BOOL ClientEquals(TCHAR *a, TCHAR *b)
{
	if (ServiceExists(MS_FP_SAMECLIENTS))
	{
#ifdef UNICODE
		char *ac = mir_t2a(a);
		char *bc = mir_t2a(b);
		char *ret = (char *) CallService(MS_FP_SAMECLIENTS, (WPARAM) ac, (LPARAM) bc);
		mir_free(ac);
		mir_free(bc);
#else
		char *ret = (char *) CallService(MS_FP_SAMECLIENTS, (WPARAM) a, (LPARAM) b);
#endif
		return ret != NULL;
	}
	else
		return lstrcmpi(a, b) == 0;
}


HISTORY_TYPE types[NUM_TYPES] = {
	{ "ClientHistory",	"Client",			IDI_CLIENT,		EVENTTYPE_CLIENT_CHANGE,			NULL,					ClientEquals,	ClientFormat,	0,									FALSE,	FALSE,	(char *) -1,	"MirVer",		TRUE,	NULL,				FALSE,	2,  ClientAddVars },
	{ "NickHistory",	"Nickname",			IDI_NICK,		EVENTTYPE_NICKNAME_CHANGE,			NULL,					NULL,			NULL,			0, 									FALSE,	FALSE,	(char *) -1,	"Nick",			TRUE,	NULL,				FALSE,	0,	NULL },
	{ "StatusHistory",	"Status",			IDI_STATUS,		EVENTTYPE_STATUSCHANGE,				NULL,					NULL,			StatusFormat,	HISTORYEVENTS_FLAG_KEEP_ONE_DAY,	FALSE,	TRUE,	(char *) -1,	"Status",		FALSE,	ID_STATUS_OFFLINE,	FALSE,	1,  StatusAddVars },
	{ "SMH",			"Status Message",	IDI_SMH,		EVENTTYPE_STATUSMESSAGE_CHANGE,		SMHAllowProtocol,		NULL,			NULL,			0, 									TRUE,	FALSE,	"CList",		"StatusMsg",	TRUE,	NULL,				TRUE,	0,	NULL },
	{ "XStatusHistory",	"XStatus",			IDI_XSTATUS,	EVENTTYPE_XSTATUS_CHANGE,			XStatusAllowProtocol,	NULL,			NULL,			HISTORYEVENTS_FLAG_KEEP_ONE_DAY,	TRUE,	FALSE,	(char *) -1,	"XStatusName",	TRUE,	NULL,				TRUE,	1,  XStatusAddVars },
};


