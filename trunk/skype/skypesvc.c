#include "skype.h"
#include "skypesvc.h"
#include "skypeapi.h"
#include "skypeopt.h"
#include "contacts.h"
#include "m_toptoolbar.h"

//From skype.c
extern char protocol;
extern HINSTANCE hInst;
extern DWORD mirandaVersion;
static HANDLE m_hPrebuildCMenu=NULL, m_hStatusHookContact=NULL, m_hContactDeleted=NULL, 
	m_hHookModulesLoaded=NULL, m_hHookOkToExit=NULL, m_hOptHook=NULL, m_hHookMirandaExit=NULL,
	m_hTTBModuleLoadedHook = NULL, m_hHookOnUserInfoInit = NULL;

void CreateServices(void)
{

	CreateServiceFunction(SKYPE_CALL, SkypeCall);
	CreateServiceFunction(SKYPE_CALLHANGUP, SkypeCallHangup);
	CreateServiceFunction(SKYPEOUT_CALL, SkypeOutCall);
	CreateServiceFunction(SKYPE_HOLDCALL, SkypeHoldCall);
	CreateServiceFunction(SKYPE_ADDUSER, SkypeAdduserDlg);
	CreateServiceFunction(SKYPE_IMPORTHISTORY, ImportHistory);
	CreateServiceFunction(SKYPE_ANSWERCALL, SkypeAnswerCall);
	CreateServiceFunction(SKYPE_SENDFILE, SkypeSendFile);
	CreateServiceFunction(SKYPE_SETAVATAR, SkypeSetAvatar);

	CreateServiceFunction(SKYPE_PROTONAME PS_GETCAPS, SkypeGetCaps);
	CreateServiceFunction(SKYPE_PROTONAME PS_GETNAME, SkypeGetName);
	CreateServiceFunction(SKYPE_PROTONAME PS_LOADICON, SkypeLoadIcon);
	CreateServiceFunction(SKYPE_PROTONAME PS_SETSTATUS, SkypeSetStatus);
	CreateServiceFunction(SKYPE_PROTONAME PS_GETSTATUS, SkypeGetStatus);
	CreateServiceFunction(SKYPE_PROTONAME PS_ADDTOLIST, SkypeAddToList);
	CreateServiceFunction(SKYPE_PROTONAME PS_ADDTOLISTBYEVENT, SkypeAddToListByEvent);
	CreateServiceFunction(SKYPE_PROTONAME PS_BASICSEARCH, SkypeBasicSearch);

	CreateServiceFunction(SKYPE_PROTONAME PSS_GETINFO, SkypeGetInfo);
	CreateServiceFunction(SKYPE_PROTONAME PSS_MESSAGE, SkypeSendMessage);
	CreateServiceFunction(SKYPE_PROTONAME PSR_MESSAGE, SkypeRecvMessage);
	CreateServiceFunction(SKYPE_PROTONAME PSS_AUTHREQUEST, SkypeSendAuthRequest);
	CreateServiceFunction(SKYPE_PROTONAME PSR_AUTH, SkypeRecvAuth);
	CreateServiceFunction(SKYPE_PROTONAME PS_AUTHALLOW, SkypeAuthAllow);
	CreateServiceFunction(SKYPE_PROTONAME PS_AUTHDENY, SkypeAuthDeny);

	CreateServiceFunction(SKYPE_PROTONAME PS_GETAVATARINFO, SkypeGetAvatarInfo);
	CreateServiceFunction(SKYPE_PROTONAME PS_GETAVATARCAPS, SkypeGetAvatarCaps);
	CreateServiceFunction(SKYPE_PROTONAME PS_GETMYAVATAR, SkypeGetAvatar);
	CreateServiceFunction(SKYPE_PROTONAME PS_SETMYAVATAR, SkypeSetAvatar);

	CreateServiceFunction(SKYPE_PROTONAME PS_SETAWAYMSG, SkypeSetAwayMessage);
	CreateServiceFunction(SKYPE_PROTONAME PS_SETAWAYMSGW, SkypeSetAwayMessageW);
	CreateServiceFunction(SKYPE_PROTONAME PSS_GETAWAYMSG, SkypeGetAwayMessage);
	CreateServiceFunction(SKYPE_PROTONAME PS_SETMYNICKNAME, SkypeSetNick);

	CreateServiceFunction(SKYPE_PROTONAME PSS_SKYPEAPIMSG, SkypeReceivedAPIMessage);
	CreateServiceFunction(SKYPE_PROTONAME SKYPE_REGPROXY, SkypeRegisterProxy);
}

void HookEvents(void)
{
	m_hPrebuildCMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	//HookEvent(ME_CLIST_DOUBLECLICKED, ClistDblClick);
	m_hOptHook = HookEvent(ME_OPT_INITIALISE, RegisterOptions);
	m_hStatusHookContact = HookEvent(ME_DB_CONTACT_ADDED,HookContactAdded);
	m_hContactDeleted = HookEvent( ME_DB_CONTACT_DELETED, HookContactDeleted );
	m_hHookModulesLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	m_hHookMirandaExit = HookEvent(ME_SYSTEM_OKTOEXIT, MirandaExit);
	m_hHookOkToExit = HookEvent(ME_SYSTEM_PRESHUTDOWN, OkToExit);
}

void HookEventsLoaded(void)
{
	// We cannot check for the TTB-service before this event gets fired... :-/
	m_hTTBModuleLoadedHook = HookEvent(ME_TTB_MODULELOADED, CreateTopToolbarButton);
	m_hHookOnUserInfoInit = HookEvent( ME_USERINFO_INITIALISE, OnDetailsInit );
}

void UnhookEvents(void)
{
	UnhookEvent(m_hOptHook);
	UnhookEvent(m_hTTBModuleLoadedHook);
	UnhookEvent(m_hHookOnUserInfoInit);
	UnhookEvent(m_hStatusHookContact);
	UnhookEvent(m_hContactDeleted);
	UnhookEvent(m_hHookModulesLoaded);
	UnhookEvent(m_hPrebuildCMenu);
	UnhookEvent(m_hHookOkToExit);
	UnhookEvent(m_hHookMirandaExit);
	//UnhookEvent(ClistDblClick);
}


INT_PTR SkypeGetCaps(WPARAM wParam, LPARAM lParam) {
    int ret = 0;
    switch (wParam) {        
        case PFLAGNUM_1:
			ret = PF1_BASICSEARCH | PF1_IM | PF1_MODEMSG; // | PF1_AUTHREQ;
			if (protocol>=5) ret |= PF1_ADDSEARCHRES;
            break;

        case PFLAGNUM_2:
            ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_INVISIBLE | PF2_HEAVYDND | PF2_FREECHAT; 
#ifdef MAPDND
	ret |= PF2_LIGHTDND | PF2_HEAVYDND;
#endif		
            break;

        case PFLAGNUM_3:
			ret = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE | PF2_IDLE;
            break;
            
        case PFLAGNUM_4:
            ret = PF4_FORCEAUTH | PF4_FORCEADDED | PF4_AVATARS;
			if (mirandaVersion >= 0x070000) ret |= PF4_IMSENDUTF;
            break;
        case PFLAG_UNIQUEIDTEXT:
            ret = (INT_PTR) "NAME";
            break;
        case PFLAG_UNIQUEIDSETTING:
            ret = (INT_PTR) SKYPE_NAME;
            break;
    }
    return ret;
		
}

INT_PTR SkypeGetName(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
	{
		strncpy((char *)lParam, SKYPE_PROTONAME, wParam);
		return 0; // Success
	}
	return 1; // Failure
}


INT_PTR SkypeLoadIcon(WPARAM wParam,LPARAM lParam)
{
	UINT id;

	switch(wParam&0xFFFF) {
		case PLI_PROTOCOL: id=IDI_SKYPE; break; // IDI_MAIN is the main icon for the protocol
		default: return (int)(HICON)NULL;	
	}
	return (int)LoadImage(hInst,MAKEINTRESOURCE(id),IMAGE_ICON,GetSystemMetrics(wParam&PLIF_SMALL?SM_CXSMICON:SM_CXICON),GetSystemMetrics(wParam&PLIF_SMALL?SM_CYSMICON:SM_CYICON),0);
}

INT_PTR SkypeGetAvatar(WPARAM wParam,LPARAM lParam)
{	DBVARIANT dbv;
	if (!DBGetContactSettingString(NULL,SKYPE_PROTONAME, "AvatarFile", &dbv)){
		lstrcpynA((char*)wParam, dbv.pszVal, (int)lParam);
		DBFreeVariant(&dbv);
	}
	return 0;
}
