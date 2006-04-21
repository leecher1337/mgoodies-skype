#include "seen.h"


HINSTANCE hInstance;
HANDLE ehdb,ehproto[2],ehmissed=NULL,ehuserinfo,ehmissed_proto=NULL;
PLUGINLINK *pluginLink;
PLUGININFO pluginInfo={
		sizeof(PLUGININFO),
		"Last seen plugin",
		PLUGIN_MAKE_VERSION(5,0,1,1),
		"Log when a user was last seen online and which users were online while you were away",
		"Heiko Schillinger",
		"micron@nexgo.de",
		"© 2001-2002 Heiko Schillinger, 2003 modified by Bruno Rino",
		"http://miranda-im.org/download/details.php?action=viewfile&id=202",
		0,
		0
};



int OptionsInit(WPARAM,LPARAM);
int UserinfoInit(WPARAM,LPARAM);
int InitFileOutput(void);
void InitMenuitem(void);
int UpdateValues(WPARAM,LPARAM);
int ModeChange(WPARAM,LPARAM);
int GetInfoAck(WPARAM,LPARAM);
void SetOffline(void);
int ModeChange_mo(WPARAM,LPARAM);
int CheckIfOnline(void);



int MainInit(WPARAM wparam,LPARAM lparam)
{
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

	SetOffline();

	ehdb=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,UpdateValues);
	ehproto[0]=HookEvent(ME_PROTO_ACK,ModeChange);
	ehproto[1]=HookEvent(ME_PROTO_ACK,GetInfoAck);

	SkinAddNewSound("LastSeenTrackedStatusChange",Translate("LastSeen: User status change"),"global.wav");
	DBWriteContactSettingString(NULL,"Uninstall",Translate("Last seen"),S_MOD);
	return 0;
}



__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}



__declspec(dllexport)int Unload(void)
{
	UnhookEvent(ehdb);
	if(ehmissed!=NULL)
		UnhookEvent(ehmissed);
	UnhookEvent(ehproto[0]);
	UnhookEvent(ehproto[1]);
	UnhookEvent(ehmissed_proto);

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





