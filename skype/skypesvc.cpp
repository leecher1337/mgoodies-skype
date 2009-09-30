#include "skype.h"
#include "skypesvc.h"
#include "skypeapi.h"
#include "skypeopt.h"
#include "contacts.h"

//From skype.cpp
extern char pszSkypeProtoName[MAX_PATH+30],protocol;
extern HINSTANCE hInst;
extern HANDLE hPrebuildCMenu, hStatusHookContact, hContactDeleted, hHookModulesLoaded, hHookOkToExit, hOptHook, hHookMirandaExit;

void CreateServices(void)
{

	char pszServiceName[MAX_PATH+30];

	CreateServiceFunction(SKYPE_CALL, SkypeCall);
	CreateServiceFunction(SKYPE_CALLHANGUP, SkypeCallHangup);
	CreateServiceFunction(SKYPEOUT_CALL, SkypeOutCall);
	CreateServiceFunction(SKYPE_HOLDCALL, SkypeHoldCall);
	CreateServiceFunction(SKYPE_ADDUSER, SkypeAdduserDlg);
	CreateServiceFunction(SKYPE_IMPORTHISTORY, ImportHistory);
	CreateServiceFunction(SKYPE_ANSWERCALL, SkypeAnswerCall);
	CreateServiceFunction(SKYPE_SENDFILE, SkypeSendFile);
	CreateServiceFunction(SKYPE_SETAVATAR, SkypeSetAvatar);

	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETCAPS);
	CreateServiceFunction(pszServiceName , SkypeGetCaps);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETNAME);
	CreateServiceFunction(pszServiceName , SkypeGetName);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_LOADICON);
	CreateServiceFunction(pszServiceName , SkypeLoadIcon);

	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_SETSTATUS);
	CreateServiceFunction(pszServiceName , SkypeSetStatus);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETSTATUS);
	CreateServiceFunction(pszServiceName , SkypeGetStatus);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_ADDTOLIST);
	CreateServiceFunction(pszServiceName , SkypeAddToList);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_ADDTOLISTBYEVENT);
	CreateServiceFunction(pszServiceName , SkypeAddToListByEvent);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_BASICSEARCH);
	CreateServiceFunction(pszServiceName , SkypeBasicSearch);

	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSS_GETINFO);
	CreateServiceFunction(pszServiceName , SkypeGetInfo);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSS_MESSAGE);
	CreateServiceFunction(pszServiceName , SkypeSendMessage);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSR_MESSAGE);
	CreateServiceFunction(pszServiceName , SkypeRecvMessage);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSS_AUTHREQUEST);
	CreateServiceFunction(pszServiceName , SkypeSendAuthRequest);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSR_AUTH);
	CreateServiceFunction(pszServiceName , SkypeRecvAuth);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_AUTHALLOW);
	CreateServiceFunction(pszServiceName , SkypeAuthAllow);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_AUTHDENY);
	CreateServiceFunction(pszServiceName , SkypeAuthDeny);

	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETAVATARINFO);
	CreateServiceFunction(pszServiceName , SkypeGetAvatarInfo);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETAVATARCAPS);
	CreateServiceFunction(pszServiceName , SkypeGetAvatarCaps);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_GETMYAVATAR);
	CreateServiceFunction(pszServiceName , SkypeGetAvatar);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_SETMYAVATAR);
	CreateServiceFunction(pszServiceName , SkypeSetAvatar);

	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_SETAWAYMSG);
	CreateServiceFunction(pszServiceName , SkypeSetAwayMessage);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PSS_GETAWAYMSG);
	CreateServiceFunction(pszServiceName , SkypeGetAwayMessage);
	strcpy(pszServiceName, pszSkypeProtoName); strcat(pszServiceName, PS_SETMYNICKNAME);
	CreateServiceFunction(pszServiceName , SkypeSetNick);

}

void HookEvents(void)
{
	hPrebuildCMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	//HookEvent(ME_CLIST_DOUBLECLICKED, ClistDblClick);
	hOptHook = HookEvent(ME_OPT_INITIALISE, RegisterOptions);
	hStatusHookContact = HookEvent(ME_DB_CONTACT_ADDED,HookContactAdded);
	hContactDeleted = HookEvent( ME_DB_CONTACT_DELETED, HookContactDeleted );
	hHookModulesLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hHookMirandaExit = HookEvent(ME_SYSTEM_OKTOEXIT, MirandaExit);
	hHookOkToExit = HookEvent(ME_SYSTEM_PRESHUTDOWN, OkToExit);
}

int SkypeGetCaps(WPARAM wParam, LPARAM lParam) {
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
            break;
        case PFLAG_UNIQUEIDTEXT:
            ret = (int) "NAME";
            break;
        case PFLAG_UNIQUEIDSETTING:
            ret = (int) SKYPE_NAME;
            break;
    }
    return ret;
		
}

int SkypeGetName(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
	{
		lstrcpyn((char *)lParam, pszSkypeProtoName, wParam);
		return 0; // Success
	}
	return 1; // Failure
}


int SkypeLoadIcon(WPARAM wParam,LPARAM lParam)
{
	UINT id;

	switch(wParam&0xFFFF) {
		case PLI_PROTOCOL: id=IDI_SKYPE; break; // IDI_MAIN is the main icon for the protocol
		default: return (int)(HICON)NULL;	
	}
	return (int)LoadImage(hInst,MAKEINTRESOURCE(id),IMAGE_ICON,GetSystemMetrics(wParam&PLIF_SMALL?SM_CXSMICON:SM_CXICON),GetSystemMetrics(wParam&PLIF_SMALL?SM_CYSMICON:SM_CYICON),0);
}

int SkypeGetAvatar(WPARAM wParam,LPARAM lParam)
{	DBVARIANT dbv;
	if (!DBGetContactSetting(NULL,pszSkypeProtoName, "AvatarFile", &dbv)){
		lstrcpynA((char*)wParam, dbv.pszVal, (int)lParam);
		DBFreeVariant(&dbv);
	}
	return 0;
}
