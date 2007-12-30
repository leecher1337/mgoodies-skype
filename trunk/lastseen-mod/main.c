/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL$
Revision       : $Rev$
Last change on : $Date$
Last change by : $Author$
*/
#include "seen.h"


HINSTANCE hInstance;
HANDLE ehdb=NULL,ehproto=NULL,ehmissed=NULL,ehuserinfo=NULL,ehmissed_proto=NULL;
PLUGINLINK *pluginLink;
char authemail[] = "fscking@spammer.oip.info";//the correct e-mail shall be constructed in DllMain
PLUGININFOEX pluginInfo={
		sizeof(PLUGININFOEX),
#ifndef PERMITNSN
		"Last seen plugin mod",
#else	
		"Last seen plugin mod (NSNCompat)",
#endif
		PLUGIN_MAKE_VERSION(5,0,4,7),
		"Log when a user was last seen online and which users were online while you were away",
		"Heiko Schillinger, YB",
		authemail,
		"© 2001-2002 Heiko Schillinger, 2003 modified by Bruno Rino, 2005-7 Modified by YB",
		"http://forums.miranda-im.org/showthread.php?t=2822",
		0,
#ifndef PERMITNSN
		DEFMOD_RNDUSERONLINE,
		{ 0x0beac488,0x578d,0x458d,{0xbb, 0x93, 0x8f, 0x2f, 0x53, 0x9b, 0x2a, 0xe4}}/* 0beac488-578d-458d-bb93-8f2f539b2ae4 */
#else	
		0,
		{ 0x2d506d46,0xc94e,0x4ef8,{0x85, 0x37, 0xf1, 0x12, 0x33, 0xa8, 0x03, 0x81}}/* 2d506d46-c94e-4ef8-8537-f11233a80381 */
#endif
};

#define TRANSNUMBER 2
DBVTranslation idleTr[TRANSNUMBER]={
	{(TranslateFunc*)any_to_IdleNotidleUnknown, L"Any to Idle/Not Idle/Unknown",0},
	{(TranslateFunc*)any_to_Idle, L"Any to /Idle or empty",0}
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
	SkinAddNewSoundEx("LastSeenTrackedStatusOffline",Translate("LastSeen"),Translate("User Logged Off"));
	SkinAddNewSoundEx("LastSeenTrackedStatusFromOffline",Translate("LastSeen"),Translate("User Logged In"));
	// known modules list
	if (ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule", (WPARAM)S_MOD, 0);
	DBWriteContactSettingString(NULL,"Uninstall",Translate("Last seen"),S_MOD);

#ifndef PERMITNSN
	SkinAddNewSoundEx("UserOnline",Translate("Alerts"),Translate("Online"));
#endif
	if (ServiceExists(MS_TIPPER_ADDTRANSLATION)){
		int i=0;
		for (i=0;i<TRANSNUMBER;i++){
			CallService(MS_TIPPER_ADDTRANSLATION,0,(LPARAM)&idleTr[i]);
		}
	}

	return 0;
}

__declspec(dllexport) PLUGININFOEX * MirandaPluginInfo(DWORD mirandaVersion)
{
	if ( mirandaVersion < PLUGIN_MAKE_VERSION(0,5,2,0)) {
		MessageBox( NULL, _T("The LastSeen-mod plugin cannot be loaded. Your Miranda is too old."), _T("SeenPlugin"), MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
		return NULL;
	}
	if ( mirandaVersion < PLUGIN_MAKE_VERSION( 0,7,0,17 )) pluginInfo.cbSize = sizeof( PLUGININFO );
	return &pluginInfo;
}

__declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof( PLUGININFOEX );
	return &pluginInfo;
}

#define MIID_LASTSEEN     {0x0df23e71, 0x7950, 0x43d5, {0xb9, 0x86, 0x7a, 0xbf, 0xf5, 0xa5, 0x40, 0x18}}
static const MUUID interfaces[] = {MIID_LASTSEEN,
#ifndef PERMITNSN
MIID_USERONLINE, 
#endif
MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
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
	memcpy(pluginInfo.authorEmail,"y_b@saaplugin.no-",17);
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





