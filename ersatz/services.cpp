/*
Translator plugin for Miranda IM

Copyright © 2006 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "services.h"


// Prototypes /////////////////////////////////////////////////////////////////////////////////////

typedef int (*CALLSERVICEFUNCTION) (const char *, WPARAM, LPARAM);
typedef HANDLE (*CREATESERVICEFUNCTION)(const char *,MIRANDASERVICE);

#define MAX_PROTOS 20

typedef struct{
	char name[64];
	BOOL initialized;
	WORD flags;
	HANDLE handleGetAwayMsg;
	MIRANDASERVICE setAwayMsg;
	char *msgs[ID_STATUS_OUTTOLUNCH - ID_STATUS_ONLINE + 1];

} PROTOCOL_INFO;

typedef struct{
	unsigned int count;
	PROTOCOL_INFO info[MAX_PROTOS];
} PROTOCOL_LIST;

static PROTOCOL_LIST protos;

static CREATESERVICEFUNCTION realCreateServiceFunction = NULL;

static int SetAwayMsg(int pos, WPARAM wParam, LPARAM lParam);
static int GetAwayMsg(int pos, WPARAM wParam, LPARAM lParam);


// Just forget that you read this ;)
static int SA00(WPARAM w, LPARAM l) { return SetAwayMsg( 0, w, l); }
static int SA01(WPARAM w, LPARAM l) { return SetAwayMsg( 1, w, l); }
static int SA02(WPARAM w, LPARAM l) { return SetAwayMsg( 2, w, l); }
static int SA03(WPARAM w, LPARAM l) { return SetAwayMsg( 3, w, l); }
static int SA04(WPARAM w, LPARAM l) { return SetAwayMsg( 4, w, l); }
static int SA05(WPARAM w, LPARAM l) { return SetAwayMsg( 5, w, l); }
static int SA06(WPARAM w, LPARAM l) { return SetAwayMsg( 6, w, l); }
static int SA07(WPARAM w, LPARAM l) { return SetAwayMsg( 7, w, l); }
static int SA08(WPARAM w, LPARAM l) { return SetAwayMsg( 8, w, l); }
static int SA09(WPARAM w, LPARAM l) { return SetAwayMsg( 9, w, l); }
static int SA10(WPARAM w, LPARAM l) { return SetAwayMsg(10, w, l); }
static int SA11(WPARAM w, LPARAM l) { return SetAwayMsg(11, w, l); }
static int SA12(WPARAM w, LPARAM l) { return SetAwayMsg(12, w, l); }
static int SA13(WPARAM w, LPARAM l) { return SetAwayMsg(13, w, l); }
static int SA14(WPARAM w, LPARAM l) { return SetAwayMsg(14, w, l); }
static int SA15(WPARAM w, LPARAM l) { return SetAwayMsg(15, w, l); }
static int SA16(WPARAM w, LPARAM l) { return SetAwayMsg(16, w, l); }
static int SA17(WPARAM w, LPARAM l) { return SetAwayMsg(17, w, l); }
static int SA18(WPARAM w, LPARAM l) { return SetAwayMsg(18, w, l); }
static int SA19(WPARAM w, LPARAM l) { return SetAwayMsg(19, w, l); }

static MIRANDASERVICE setAwayFuncs[MAX_PROTOS] = { 
	SA00, SA01, SA02, SA03, SA04, SA05, SA06, SA07, SA08, SA09, 
	SA10, SA11, SA12, SA13, SA14, SA15, SA16, SA17, SA18, SA19 };


static int GA00(WPARAM w, LPARAM l) { return GetAwayMsg( 0, w, l); }
static int GA01(WPARAM w, LPARAM l) { return GetAwayMsg( 1, w, l); }
static int GA02(WPARAM w, LPARAM l) { return GetAwayMsg( 2, w, l); }
static int GA03(WPARAM w, LPARAM l) { return GetAwayMsg( 3, w, l); }
static int GA04(WPARAM w, LPARAM l) { return GetAwayMsg( 4, w, l); }
static int GA05(WPARAM w, LPARAM l) { return GetAwayMsg( 5, w, l); }
static int GA06(WPARAM w, LPARAM l) { return GetAwayMsg( 6, w, l); }
static int GA07(WPARAM w, LPARAM l) { return GetAwayMsg( 7, w, l); }
static int GA08(WPARAM w, LPARAM l) { return GetAwayMsg( 8, w, l); }
static int GA09(WPARAM w, LPARAM l) { return GetAwayMsg( 9, w, l); }
static int GA10(WPARAM w, LPARAM l) { return GetAwayMsg(10, w, l); }
static int GA11(WPARAM w, LPARAM l) { return GetAwayMsg(11, w, l); }
static int GA12(WPARAM w, LPARAM l) { return GetAwayMsg(12, w, l); }
static int GA13(WPARAM w, LPARAM l) { return GetAwayMsg(13, w, l); }
static int GA14(WPARAM w, LPARAM l) { return GetAwayMsg(14, w, l); }
static int GA15(WPARAM w, LPARAM l) { return GetAwayMsg(15, w, l); }
static int GA16(WPARAM w, LPARAM l) { return GetAwayMsg(16, w, l); }
static int GA17(WPARAM w, LPARAM l) { return GetAwayMsg(17, w, l); }
static int GA18(WPARAM w, LPARAM l) { return GetAwayMsg(18, w, l); }
static int GA19(WPARAM w, LPARAM l) { return GetAwayMsg(19, w, l); }

static MIRANDASERVICE getAwayFuncs[MAX_PROTOS] = { 
	GA00, GA01, GA02, GA03, GA04, GA05, GA06, GA07, GA08, GA09, 
	GA10, GA11, GA12, GA13, GA14, GA15, GA16, GA17, GA18, GA19 };



// Functions //////////////////////////////////////////////////////////////////////////////////////


void replaceStr(char*& dest, const char* src)
{
	if (dest != NULL)
	{
		free(dest);
		dest = NULL;
	}

	if (src != NULL) 
	{
		dest = strdup(src);
	}
}


void DestroyProtoList()
{
	for(unsigned int i = 0; i < protos.count; i++) 
	{
		for(int j = 0; j < ID_STATUS_OUTTOLUNCH - ID_STATUS_ONLINE + 1; j++)
			if (protos.info[i].msgs[j] != NULL)
				free(protos.info[i].msgs[j]);
		
		if(protos.info[i].handleGetAwayMsg)
			DestroyServiceFunction(protos.info[i].handleGetAwayMsg);
	}
}


int GetAwayMsg(int pos, WPARAM wParam, LPARAM lParam)
{
	WORD status = (WORD) wParam;

	if (status == 0)
		status = CallProtoService(protos.info[pos].name, PS_GETSTATUS, 0, 0);

	if (status < ID_STATUS_ONLINE || status > ID_STATUS_OUTTOLUNCH)
		return NULL;

	return (int) mir_strdup(protos.info[pos].msgs[status - ID_STATUS_ONLINE]);
}


int SetAwayMsg(int pos, WPARAM wParam, LPARAM lParam)
{
	WORD status = (WORD) wParam;
	char *msg = (char *) lParam;

	if (protos.info[pos].initialized 
			&& status >= ID_STATUS_ONLINE && status <= ID_STATUS_OUTTOLUNCH
			&& (Proto_Status2Flag(status) & protos.info[pos].flags))
	{
		replaceStr(protos.info[pos].msgs[status - ID_STATUS_ONLINE], msg);
	}

	return protos.info[pos].setAwayMsg(wParam, lParam);
}


HANDLE HackCreateServiceFunction(const char *name, MIRANDASERVICE service)
{
	if (name == NULL || service == NULL || protos.count >= MAX_REGS(protos.info))
		return realCreateServiceFunction(name, service);

	// Check if is a set away msg service
	const char *pos = strstr(name, PS_SETAWAYMSG);
	if (pos == NULL || *(pos + strlen(PS_SETAWAYMSG)) != '\0')
		return realCreateServiceFunction(name, service);

	// Get protocol name
	char proto_name[128];
	lstrcpyn(proto_name, name, min(sizeof(proto_name), pos - name + 1));

	// Check if it is realy a protocol
	PROTOCOLDESCRIPTOR **proto;
	unsigned int count;

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&proto);

	// First count usefull protos
	for (unsigned int i = 0; i < count; i++)
	{
		if (proto[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (proto[i]->szName == NULL || proto[i]->szName[0] == '\0')
			continue;

		if (strcmp(proto[i]->szName, proto_name) != 0)
			continue;

		// Found a protocol
		int pos = protos.count;
		protos.count++;

		protos.info[pos].initialized = FALSE;
		strcpy(protos.info[pos].name, proto_name);
		protos.info[pos].setAwayMsg = service;

		return realCreateServiceFunction(name, setAwayFuncs[pos]);
	}

	return realCreateServiceFunction(name, service);
}

void HookRealServices()
{
	memset(&protos, 0, sizeof(protos));

	realCreateServiceFunction = pluginLink->CreateServiceFunction;
	pluginLink->CreateServiceFunction = HackCreateServiceFunction;
}

void UnhookRealServices()
{
	if (realCreateServiceFunction)
		pluginLink->CreateServiceFunction = realCreateServiceFunction;
}

void InitServices()
{
	HookRealServices();
}

void ModulesLoadedServices()
{
	// Now initialize things
	for(unsigned int i = 0; i < protos.count; i++) 
	{
		if (!(CallProtoService(protos.info[i].name, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
			continue;

		if (ProtoServiceExists(protos.info[i].name, PS_GETMYAWAYMSG))
			continue;

		protos.info[i].flags = CallProtoService(protos.info[i].name, PS_GETCAPS, PFLAGNUM_3, 0);

		if (protos.info[i].flags == 0)
			continue;

		// Is a valid proto
		protos.info[i].initialized = TRUE;
		protos.info[i].handleGetAwayMsg = (HANDLE) CreateProtoServiceFunction(protos.info[i].name, 
														PS_GETMYAWAYMSG, getAwayFuncs[i]);
	}
}

void DestroyServices()
{
	UnhookRealServices();
	DestroyProtoList();
}






































/*
static CRITICAL_SECTION cs;



#if __GNUC__
#define NOINLINEASM
#endif

DWORD NameHashFunction(const char *szStr)
{
#if defined _M_IX86 && !defined _NUMEGA_BC_FINALCHECK && !defined NOINLINEASM
	__asm {		   //this breaks if szStr is empty
		xor  edx,edx
		xor  eax,eax
		mov  esi,szStr
		mov  al,[esi]
		xor  cl,cl
	lph_top:	 //only 4 of 9 instructions in here don't use AL, so optimal pipe use is impossible
		xor  edx,eax
		inc  esi
		xor  eax,eax
		and  cl,31
		mov  al,[esi]
		add  cl,5
		test al,al
		rol  eax,cl		 //rol is u-pipe only, but pairable
		                 //rol doesn't touch z-flag
		jnz  lph_top  //5 clock tick loop. not bad.

		xor  eax,edx
	}
#else
	DWORD hash=0;
	int i;
	int shift=0;
	for(i=0;szStr[i];i++) {
		hash^=szStr[i]<<shift;
		if(shift>24) hash^=(szStr[i]>>(32-shift))&0x7F;
		shift=(shift+5)&0x1F;
	}
	return hash;
#endif
}


int DummyService(WPARAM wParam, LPARAM lParam)
{
	return 0;
}


BOOL CreateProtoList()
{
	char szServiceName[MAXMODULELABELLENGTH+1];
	PROTOCOLDESCRIPTOR **proto;
	unsigned int count, i;

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&proto);

	// First count usefull protos
	protos.count = 0;
	for (i = 0; i < count; i++)
	{
		if (proto[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (proto[i]->szName == NULL || proto[i]->szName[0] == '\0')
			continue;

		if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
			continue;

		if (CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0) == 0)
			continue;

		if (ProtoServiceExists(proto[i]->szName, PS_GETMYAWAYMSG))
			continue;

		// Found a protocol
		protos.count++;
	}

	if (protos.count <=0)
	{
		protos.info = NULL;
		return FALSE;
	}

	// Now lets init our array
	protos.info = (PROTOCOL_INFO *) mir_alloc0(protos.count * sizeof(PROTOCOL_INFO));
	if (protos.info == NULL) 
		return FALSE;

	protos.count = 0;
	for (i = 0; i < count; i++)
	{
		if (proto[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (proto[i]->szName == NULL || proto[i]->szName[0] == '\0')
			continue;

		if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
			continue;

		if (CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0) == 0)
			continue;

		if (ProtoServiceExists(proto[i]->szName, PS_GETMYAWAYMSG))
			continue;

		// Found a protocol
		lstrcpyn(protos.info[protos.count].name, proto[i]->szName, sizeof(protos.info[protos.count].name));
		protos.info[protos.count].flags = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0);
		protos.info[protos.count].handle = (HANDLE) CreateProtoServiceFunction(proto[i]->szName, PS_GETMYAWAYMSG, DummyService);

		mir_snprintf(szServiceName, sizeof(szServiceName), "%s%s", proto[i]->szName, PS_GETMYAWAYMSG);
		protos.info[protos.count].hashGetAwayMsg = NameHashFunction(szServiceName);

		mir_snprintf(szServiceName, sizeof(szServiceName), "%s%s", proto[i]->szName, PS_SETAWAYMSG);
		protos.info[protos.count].hashSetAwayMsg = NameHashFunction(szServiceName);

		protos.count++;
	}

	return TRUE;
}

void DestroyProtoList()
{
	for(unsigned int i = 0; i < protos.count; i++) 
	{
		for(int j = 0; j < ID_STATUS_OUTTOLUNCH - ID_STATUS_ONLINE + 1; j++)
			if (protos.info[i].msgs[j] != NULL)
				free(protos.info[i].msgs[j]);
		
		if(protos.info[i].handle)
			DestroyServiceFunction(protos.info[i].handle);
	}

	protos.count = 0;
	if(protos.info)
		mir_free(protos.info);
}


char *GetProtoAwayMsg(PROTOCOL_INFO *proto)
{
	int status = CallProtoService(proto->name, PS_GETSTATUS, 0, 0);

	if (status < ID_STATUS_ONLINE || status > ID_STATUS_OUTTOLUNCH)
		return NULL;

	if (!(Proto_Status2Flag(status) & proto->flags))
		return NULL;

	return mir_strdup(proto->msgs[status - ID_STATUS_ONLINE]);
}

void StoreProtoAwayMsg(PROTOCOL_INFO *proto, char *msg, WORD status)
{
	if (status < ID_STATUS_ONLINE || status > ID_STATUS_OUTTOLUNCH)
		return;

	replaceStr(proto->msgs[status - ID_STATUS_ONLINE], msg);
}

int HackCallServiceFunction(const char *name, WPARAM wParam, LPARAM lParam)
{
	DWORD hash = NameHashFunction(name);

	for(unsigned int i = 0; i < protos.count; i++)
	{
		if (hash == protos.info[i].hashGetAwayMsg) 
		{
			return (int) GetProtoAwayMsg(&protos.info[i]);
		}
		else if (hash == protos.info[i].hashSetAwayMsg)
		{
			int res = realCallServiceFunction(name, wParam, lParam);
			if(!res)
				StoreProtoAwayMsg(&protos.info[i], (char *)lParam, (WORD)wParam);
			return res;
		}
	}

	return realCallServiceFunction(name, wParam, lParam);
}

void HookRealServices()
{
	EnterCriticalSection(&cs);
	realCallServiceFunction = pluginLink->CallService;
	pluginLink->CallService = HackCallServiceFunction;
	LeaveCriticalSection(&cs);
}

void UnhookRealServices()
{
	EnterCriticalSection(&cs);
	if (realCallServiceFunction)
		pluginLink->CallService = realCallServiceFunction;
	LeaveCriticalSection(&cs);
}

void InitServices()
{
	InitializeCriticalSection(&cs);
	if (CreateProtoList())
		HookRealServices();
}

void DestroyServices()
{
	UnhookRealServices();
	DeleteCriticalSection(&cs);
}
*/