#include "seen.h"


HINSTANCE hInstance;
HANDLE ehdb=NULL,ehproto=NULL,ehmissed=NULL,ehuserinfo=NULL,ehmissed_proto=NULL;
PLUGINLINK *pluginLink;
PLUGININFO pluginInfo={
		sizeof(PLUGININFO),
#ifndef PERMITNSN
		"Last seen plugin mod",
#else	
		"Last seen plugin mod (NSNCompat)",
#endif
		PLUGIN_MAKE_VERSION(5,0,4,3),
		"Log when a user was last seen online and which users were online while you were away",
		"Heiko Schillinger, YB",
		"",
		"© 2001-2002 Heiko Schillinger, 2003 modified by Bruno Rino, 2005 Modified by YB",
		"http://forums.miranda-im.org/showthread.php?t=2822",
		0,
#ifndef PERMITNSN
		DEFMOD_RNDUSERONLINE
#else	
		0
#endif
};



int OptionsInit(WPARAM,LPARAM);
int UserinfoInit(WPARAM,LPARAM);
int InitFileOutput(void);
void InitMenuitem(void);
int UpdateValues(WPARAM,LPARAM);
int ModeChange(WPARAM,LPARAM);
//int GetInfoAck(WPARAM,LPARAM);
void SetOffline(void);
int ModeChange_mo(WPARAM,LPARAM);
int CheckIfOnline(void);

BOOL includeIdle;
logthread_info **contactQueue = NULL;
int contactQueueSize = 0;


int MainInit(WPARAM wparam,LPARAM lparam)
{
	contactQueueSize = 16*sizeof(logthread_info *);
	contactQueue = (logthread_info **)malloc(contactQueueSize);
	memset(&contactQueue[0], 0, contactQueueSize);
	contactQueueSize = 16;
	includeIdle = (BOOL )DBGetContactSettingByte(NULL,S_MOD,"IdleSupport",1);
	HookEvent(ME_OPT_INITIALISE,OptionsInit);
	
	if(DBGetContactSettingByte(NULL,S_MOD,"MenuItem",1)) {
		InitMenuitem();
	}
	
	if(DBGetContactSettingByte(NULL,S_MOD,"UserinfoTab",1))
		ehuserinfo=HookEvent(ME_USERINFO_INITIALISE,UserinfoInit);

	if(DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0))
		InitFileOutput();

	if(DBGetContactSettingByte(NULL,S_MOD,"MissedOnes",0))
		ehmissed_proto=HookEvent(ME_PROTO_ACK,ModeChange_mo);

//	SetOffline();

	ehdb=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,UpdateValues);
	ehproto=HookEvent(ME_PROTO_ACK,ModeChange);

	SkinAddNewSoundEx("LastSeenTrackedStatusChange",Translate("LastSeen"),Translate("User status change"));
	SkinAddNewSoundEx("LastSeenTrackedStatusOnline",Translate("LastSeen"),Translate("Changed to Online"));
	SkinAddNewSoundEx("LastSeenTrackedStatusOffline",Translate("LastSeen"),Translate("Changed to Offline"));
	// known modules list
	if (ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule", (WPARAM)S_MOD, 0);
	DBWriteContactSettingString(NULL,"Uninstall",Translate("Last seen"),S_MOD);

#ifndef PERMITNSN
	SkinAddNewSoundEx("UserOnline",Translate("Alerts"),Translate("Online"));
#endif
	return 0;
}



__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}



__declspec(dllexport)int Unload(void)
{
	UnhookEvent(ehdb);
	if(ehmissed!=NULL) UnhookEvent(ehmissed);
	UnhookEvent(ehproto);
	if(ehmissed_proto)UnhookEvent(ehmissed_proto);
//	free(contactQueue);
	return 0;
}



BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved)
{
	hInstance=hinst;
	return 1;
}



int __declspec(dllexport)Load(PLUGINLINK *link)
{
	pluginLink=link;
	// this isn't required for most events
	// but the ME_USERINFO_INITIALISE
	// I decided to hook all events after
	// everything is loaded because it seems
	// to be safer in my opinion
	HookEvent(ME_SYSTEM_MODULESLOADED,MainInit);
	return 0;
}





