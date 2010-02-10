/*

CountMe Plugin for Miranda IM
Copyright (C) 2005  Piotr Piastucki

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

#include "countme.h"

HINSTANCE hInstance;
HANDLE hNetlibUser;
PLUGINLINK *pluginLink;
static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int PreShutdown(WPARAM wParam, LPARAM lParam);
static char *countmeModuleName;
static int stopThread;

static int 	version = 1031;

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"CountMe Plugin",
	PLUGIN_MAKE_VERSION(1,0,3,2),
	"CountMe Plugin (1.0.3.2 "__DATE__")",
	"Piotr Piastucki",
	"the_leech@users.berlios.de",
	"(c) 2005-2010 Piotr Piastucki",
	"http://developer.berlios.de/projects/mgoodies",
	0,
	0
};


struct FORK_ARG {
	HANDLE hEvent;
	void (__cdecl *threadcode)(void*);
	void *arg;
};

static void __cdecl forkthread_r(struct FORK_ARG *fa)
{
	void (*callercode)(void*) = fa->threadcode;
	void *arg = fa->arg;
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
	SetEvent(fa->hEvent);
	callercode(arg);
	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	return;
}

unsigned long ForkThread(
	void (__cdecl *threadcode)(void*),
	unsigned long stacksize,
	void *arg
)
{
	unsigned long rc;
	struct FORK_ARG fa;

	fa.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	fa.threadcode = threadcode;
	fa.arg = arg;
	rc = _beginthread((void (__cdecl *)(void*))forkthread_r, stacksize, &fa);
	if ((unsigned long) -1L != rc) {
		WaitForSingleObject(fa.hEvent, INFINITE);
	}
	CloseHandle(fa.hEvent);
	return rc;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFO *MirandaPluginInfo(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0,3,1,0)) {
		MessageBox(NULL, "The CountMe plugin cannot be loaded. It requires Miranda IM 0.3.1 or later.", "CountMe Plugin", MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return NULL;
	}
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	char text[_MAX_PATH];
	char *p, *q;

	GetModuleFileName(hInstance, text, sizeof(text));
	p = strrchr(text, '\\');
	p++;
	q = strrchr(p, '.');
	*q = '\0';
	countmeModuleName = _strdup("CountMe");//_strdup(p);
	pluginLink = link;
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	return 0;
}

static void trim(char *str) {
	int i, l, j;
	l = strlen(str);
	for (i=0;i<l;i++) {
		if (str[i]!=' ' && str[i]!='\t') break;
	}
	for (j=l-1;j>=i;j--) {
		if (str[j]!=' ' && str[j]!='\t') break;
	}
	memcpy(str, &str[i], j-i+1);
	str[j-i+1]='\0';
}

static int getTokens(const char *src, int len, char **tokens, int maxTokens) {
	int i,j,k;
	for (j=k=i=0;j<maxTokens && i<len;i++) {
		if (src[i]==',') {
			tokens[j] = new char[i-k+1];
			memcpy(tokens[j], &src[k], i-k);
			tokens[j][i-k]='\0';
			trim(tokens[j]);
			j++;
			k = i+1;
		}
	}
	while (j<maxTokens) {
		tokens[j] = new char[i-k+1];
		if (k<len) {
			memcpy(tokens[j], &src[k], i-k);
			k = i;
		}
		tokens[j][i-k]='\0';
		trim(tokens[j]);
		j++;
	}
	return 1;
}

static void __cdecl CountThread(void* ptr) {
    DBVARIANT dbv;
    char *countLink;
    char *xid;
    int mode;
    int sleepTime;
	if (DBGetContactSetting(NULL, countmeModuleName, "CountURL", &dbv)) {
		DBWriteContactSettingString(NULL, countmeModuleName, "CountURL", "http://miranda.kom.pl/countme.php");
		countLink = _strdup("http://miranda.kom.pl/countme.php");
	} else {
        countLink = _strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (DBGetContactSetting(NULL, countmeModuleName, "ExtID", &dbv)) {
		xid = _strdup("");
	} else {
        xid = _strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	sleepTime = 10;
	if (DBGetContactSetting(NULL, countmeModuleName, "PingInterval", &dbv)) {
        DBWriteContactSettingDword(NULL, countmeModuleName, "PingInterval", sleepTime);
	} else {
		if (dbv.type != DBVT_DWORD) {
			sleepTime = atoi(dbv.pszVal);
			DBDeleteContactSetting(NULL, countmeModuleName, "PingInterval");
		} else {
			sleepTime = dbv.dVal;
		}
		DBFreeVariant(&dbv);
        DBWriteContactSettingDword(NULL, countmeModuleName, "PingInterval", sleepTime);
	}
	mode = DBGetContactSettingDword(NULL, countmeModuleName, "Mode", 0);
	while (!stopThread) {
        NETLIBHTTPREQUEST nlhr;
        memset(&nlhr, 0, sizeof(NETLIBHTTPREQUEST));
        nlhr.cbSize = sizeof(NETLIBHTTPREQUEST);
		if (DBGetContactSetting(NULL, countmeModuleName, "CountID", &dbv)) {
			char url[2048];
			sleepTime *= 2;
			if (mode == 0) {
				sprintf(url, "%s?ver=%d&xid=%s", countLink, version, xid);
			} else {
				sprintf(url, "%s?mode=%d&ver=%d&xid=%s", countLink, mode, version, xid);
			}
	        nlhr.requestType = REQUEST_GET;
	        nlhr.flags = NLHRF_GENERATEHOST;
	        nlhr.headers = NULL;
	        nlhr.headersCount = 0;
	        nlhr.pData = NULL;
	        nlhr.dataLength = 0;
	        nlhr.szUrl = url;
	        NETLIBHTTPREQUEST *nlhr2 = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM) hNetlibUser, (LPARAM)&nlhr);
	        if (nlhr2!=NULL && nlhr2->resultCode/100 == 2) {
				if (mode == 0) {
					int headerRead = 0;
					for (int i=0; i<nlhr2->headersCount;i ++) {
	                    NETLIBHTTPHEADER *header = &nlhr2->headers[i];
	                    if (!strcmp(header->szName, "ID")) {
							headerRead |= 1;
	                        DBWriteContactSettingString(NULL, countmeModuleName, "CountID", header->szValue);
						}
	                    if (!strcmp(header->szName, "WAIT")) {
							headerRead |= 2;
							sleepTime = atoi(header->szValue);
						}
	                    if (!strcmp(header->szName, "MODE")) {
							headerRead |= 4;
							mode = atoi(header->szValue);
						}
					}
					if (headerRead == 0) {
						mode = 1;
					}
				} else {
					char *tokens[3];
					getTokens(nlhr2->pData, nlhr2->dataLength, tokens, 3);
					if (strlen(tokens[0]) > 0) {
	                	DBWriteContactSettingString(NULL, countmeModuleName, "CountID", tokens[0]);
					}
					if (strlen(tokens[1]) > 0) {
						sleepTime = atoi(tokens[1]);
					}
					if (strlen(tokens[2]) > 0) {
						mode = atoi(tokens[2]);
					}
					delete tokens[0];
					delete tokens[1];
					delete tokens[2];
				}
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM) nlhr2);
			}
		}
		if (!DBGetContactSetting(NULL, countmeModuleName, "CountID", &dbv)) {
			char url[2048];
			if (mode == 0) {
				sprintf(url, "%s?ver=%d&id=%s&xid=%s", countLink, version, dbv.pszVal, xid);
			} else {
				sprintf(url, "%s?mode=%d&ver=%d&id=%s&xid=%s", countLink, mode, version, dbv.pszVal, xid);
			}
            memset(&nlhr, 0, sizeof(NETLIBHTTPREQUEST));
	        nlhr.requestType = REQUEST_GET;
	        nlhr.flags = NLHRF_GENERATEHOST;
	        nlhr.headers = NULL;
	        nlhr.headersCount = 0;
	        nlhr.pData = NULL;
	        nlhr.dataLength = 0;
	        nlhr.szUrl = url;
	        NETLIBHTTPREQUEST *nlhr2 = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM) hNetlibUser, (LPARAM)&nlhr);
	        if (nlhr2!=NULL && nlhr2->resultCode/100 == 2) {
				if (mode == 0) {
					int headerRead = 0;
					for (int i=0; i<nlhr2->headersCount;i ++) {
	                    NETLIBHTTPHEADER *header = &nlhr2->headers[i];
	                    if (!strcmp(header->szName, "ID")) {
							headerRead |= 1;
	                        DBWriteContactSettingString(NULL, countmeModuleName, "CountID", header->szValue);
						}
	                    if (!strcmp(header->szName, "WAIT")) {
							headerRead |= 2;
							sleepTime = atoi(header->szValue);
						}
	                    if (!strcmp(header->szName, "MODE")) {
							headerRead |= 4;
							mode = atoi(header->szValue);
						}
					}
					if (headerRead == 0) {
						mode = 1;
					}
				} else {
					char *tokens[3];
					getTokens(nlhr2->pData, nlhr2->dataLength, tokens, 3);
					if (strlen(tokens[0]) > 0) {
	                	DBWriteContactSettingString(NULL, countmeModuleName, "CountID", tokens[0]);
					}
					if (strlen(tokens[1]) > 0) {
						sleepTime = atoi(tokens[1]);
					}
					if (strlen(tokens[2]) > 0) {
						mode = atoi(tokens[2]);
					}
					delete tokens[0];
					delete tokens[1];
					delete tokens[2];
				}
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM) nlhr2);
			}
			DBFreeVariant(&dbv);
		}
		if (sleepTime < 1) sleepTime = 10;
		if (sleepTime > 1440) sleepTime = 1440;
		DBWriteContactSettingDword(NULL, countmeModuleName, "Mode", mode);
		DBWriteContactSettingDword(NULL, countmeModuleName, "PingInterval", sleepTime);
		for (int i =0; !stopThread && i< 60 *sleepTime; i++) {
			 Sleep(1000);
		}
	}
	free(xid);
	free(countLink);
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	NETLIBUSER nlu = {0};
	char name[128];

	sprintf(name, "%s %s", countmeModuleName, Translate("connection"));

	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS;	// | NUF_HTTPGATEWAY;
	nlu.szDescriptiveName = name;
	nlu.szSettingsModule = countmeModuleName;
	hNetlibUser = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) &nlu);
	ForkThread((void (__cdecl *)(void*))CountThread, 0, NULL);
	return 0;
}

static int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	stopThread = 1;
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
    free(countmeModuleName);
	return 0;
}

