#include "headers.h"
#include "main.h"
#include "shake.h"


LRESULT CALLBACK NudgePopUpProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

COLORREF colorBack = 250, colorText = 0;
int popupTime = 4;
bool bShowPopup = true;
bool bUseWindowColor = true;
bool bShakeClist = true;
bool bShakeChat = true;
static HANDLE hEventOptionsInitialize;
HINSTANCE hInst;
PLUGINLINK *pluginLink;
CNudgeElement *NudgeList;

//========================
//  MirandaPluginInfo
//========================
PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"Nudge",
	PLUGIN_MAKE_VERSION(0,0,0,3),
	"Plugin to shake the clist and chat window",
	"Tweety/GouZ",
	"francois.mean@skynet.be / Sylvain.gougouzian@gmail.com ",
	"copywright to the miranda community",
	"http://www.miranda-fr.net/",		// www
	0,		//not transient
	0		//doesn't replace anything built-in
};


 
void RegisterToUpdate(void)
{
	//Use for the Updater plugin
	if(ServiceExists(MS_UPDATE_REGISTER)) 
	{
		Update update = {0};
		char szVersion[16];

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionStringPlugin(&pluginInfo, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);
		update.szUpdateURL = NULL;//"http://miranda-im.org/download/feed.php?dlfile=2165";
		update.szVersionURL = NULL;//"http://www.miranda-im.org/download/details.php?action=viewfile&id=2165";
		update.pbVersionPrefix = (BYTE*) "";//(BYTE *)"<span class=\"fileNameHeader\">Updater ";
	    update.szBetaUpdateURL = "http://www.miranda-fr.net/tweety/Nudge/Nudge.zip";
		update.szBetaVersionURL = "http://www.miranda-fr.net/tweety/Nudge/Nudge_beta.html";
		update.pbBetaVersionPrefix = (BYTE *)"Nudge version ";

		update.cpbVersionPrefix = strlen((char *)update.pbVersionPrefix);
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);

	}
}


int NudgeRecieved(WPARAM wParam,LPARAM lParam)
{
	if(bShakeClist)
		ShakeClist(wParam,lParam);
	if(bShakeChat)
		ShakeChat(wParam,lParam);

	char *protoName = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);

	CNudgeElement *n;
	for(n = NudgeList;n != NULL; n = n->next)
	{
		if(!strcmp(protoName,n->ProtocolName))
		{
			SkinPlaySound( n->NudgeSoundname );
			if(n->showPopup)
				Nudge_ShowPopup(n, (HANDLE) wParam);
			/*if(n->showEvent)
			{
				char sMsg[250];
				CLISTEVENT cEvent;
				cEvent.cbSize = sizeof(CLISTEVENT);
				cEvent.hContact = (HCONTACT) wParam;
				cEvent.hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_NUDGE ));
				cEvent.hDbEvent = (HANDLE)"yamn new mail";
				cEvent.lParam = (LPARAM) ActualAccount->hContact;
				cEvent.pszService = MS_YAMN_CLISTDBLCLICK;
				cEvent.pszTooltip = new char[250];

				sprintf(cEvent.pszTooltip,Translate("%s : %d new mail(s), %d total"),ActualAccount->Name,MN->Real.PopUpNC+MN->Virtual.PopUpNC,MN->Real.PopUpTC+MN->Virtual.PopUpTC);
				CallServiceSync(MS_CLIST_ADDEVENT, 0,(LPARAM)&cEvent);
				
				sprintf(sMsg,Translate("%d new mail(s), %d total"),MN->Real.PopUpNC+MN->Virtual.PopUpNC,MN->Real.PopUpTC+MN->Virtual.PopUpTC);
				DBWriteContactSettingString(ActualAccount->hContact, "CList", "StatusMsg", sMsg);
				
				if(nflags & YAMN_ACC_CONTNICK)
				{
					DBWriteContactSettingString(ActualAccount->hContact, ProtoName, "Nick", cEvent.pszTooltip);
				}
			}*/
			//MessageBox(NULL,n->ProtocolName,n->NudgeSoundname,0);
		}		
	}
	return 0;
}


extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

int MainInit(WPARAM wParam,LPARAM lParam)
{
	return 0;
}

int ModulesLoaded(WPARAM,LPARAM)
{
	RegisterToUpdate();
	int numberOfProtocols,ret;
	char str[MAXMODULELABELLENGTH + 10];
	HANDLE NudgeEvent = NULL;
	PROTOCOLDESCRIPTOR ** ppProtocolDescriptors;
	ret = CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM) &numberOfProtocols,(LPARAM)&ppProtocolDescriptors);
	if(ret == 0)
	{
		for(int i = 0; i < numberOfProtocols ; i++)
		{
			if(ppProtocolDescriptors[i]->type == PROTOTYPE_PROTOCOL)
			{
				sprintf(str,"%s\\Nudge",ppProtocolDescriptors[i]->szName);
				NudgeEvent = HookEvent(str, NudgeRecieved);
				if(NudgeEvent != NULL)
					Nudge_AddElement(ppProtocolDescriptors[i]->szName);
				
				NudgeEvent = NULL;
			}
		}
		
	}
	/*CNudgeElement *n;
	for(n = NudgeList;n != NULL; n = n->next)
	{
		MessageBox(NULL,n->ProtocolName,n->NudgeSoundname,0);
	}*/

	if( ServiceExists(MS_TRIGGER_REGISTERACTION))
	{
		ACTIONREGISTER ar;
		ZeroMemory(&ar, sizeof(ar));
		ar.cbSize = sizeof(ar);
		ar.hInstance = NULL;
		ar.pfnDlgProc = NULL;
		ar.pszTemplate = NULL;

		ar.pszName = Translate("ShakeClist");
		ar.pszService = MS_SHAKE_CLIST;

		/* register the action */
		CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);

		ar.pszName = Translate("ShakeChat");
		ar.pszService = MS_TRIGGER_SHAKE_CHAT;
		/* register the action */
		CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);
	}
	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{ 	
	pluginLink = link;
	NudgeList = NULL;
	HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	
	InitOptions();

	//Create function for plugins
	CreateServiceFunction(MS_SHAKE_CLIST,ShakeClist);
	CreateServiceFunction(MS_SHAKE_CHAT,ShakeChat);
	CreateServiceFunction(MS_TRIGGER_SHAKE_CHAT,TriggerShakeChat);
	return 0; 
}

extern "C" int __declspec(dllexport) Unload(void) 
{ 
	return 0;
}

LRESULT CALLBACK NudgePopUpProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	switch(msg)
	{
		case WM_COMMAND:
			break;

		case WM_CONTEXTMENU:
			SendMessage(hWnd,UM_DESTROYPOPUP,0,0);
			break;			
		case UM_FREEPLUGINDATA:
			//Here we'd free our own data, if we had it.
			return FALSE;
		case UM_INITPOPUP:
			break;
		case UM_DESTROYPOPUP:
			break;
		case WM_NOTIFY:
		default:
			break;
	}
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

int Preview()
{
	if(bShakeClist)
		ShakeClist(0,0);

	CNudgeElement *n;
	for(n = NudgeList;n != NULL; n = n->next)
	{
		if(n->showPopup)
			Nudge_ShowPopup(n, NULL);
	}
	return 0;
}
void Nudge_ShowPopup(CNudgeElement *n, HANDLE hCont)
{
	POPUPDATAEX NudgePopUp;
	HANDLE hContact;

	hContact = Nudge_GethContact(hCont);

	if(hContact == NULL) //no contact at all
		NudgePopUp.lchContact = (HANDLE) n;

	NudgePopUp.lchContact = hContact;
	NudgePopUp.lchIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_NUDGE ));
	NudgePopUp.colorBack = ! n->popupWindowColor ? n->popupBackColor : GetSysColor(COLOR_BTNFACE);
	NudgePopUp.colorText = ! n->popupWindowColor ? n->popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
	NudgePopUp.iSeconds = n->popupTimeSec;
	NudgePopUp.PluginWindowProc = (WNDPROC)NudgePopUpProc;
	NudgePopUp.PluginData = (void *)1;

	char * lpzContactName = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0);
	
	sprintf(NudgePopUp.lpzText, Translate("You recieved a nudge"));
	lstrcpy(NudgePopUp.lpzContactName, lpzContactName);

	CallService(MS_POPUP_ADDPOPUPEX,(WPARAM)&NudgePopUp,0);
}

int Nudge_AddElement(char *protoName)
{
	//Add contact menu entry
	char servicefunction[ 100 ];
	CLISTMENUITEM mi;

	sprintf(servicefunction, "%s\\SendNudge", protoName);
	memset( &mi, 0, sizeof( mi ));
	mi.popupPosition = 500085000;
	mi.pszContactOwner = protoName;
	mi.pszPopupName = protoName;
	mi.cbSize = sizeof( mi );
	mi.flags = CMIF_NOTOFFLINE;
	mi.position = -500050004;
	mi.hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_NUDGE ));
	mi.pszName = Translate( "Send &Nudge" );
	mi.pszService = servicefunction;
	CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );
	
	//Add a specific sound per protocol
	char nudgesoundtemp[ 64 ];
	CNudgeElement *newNudge;
	newNudge = (CNudgeElement*) malloc(sizeof(CNudgeElement));
	strcpy( nudgesoundtemp, protoName );
	strcat( nudgesoundtemp, ": " );
	strcat( nudgesoundtemp,  Translate( "Nudge" ));

	strcpy( newNudge->NudgeSoundname , nudgesoundtemp );
	strcpy( newNudge->ProtocolName, protoName );
	newNudge->showPopup = bShowPopup;
	newNudge->popupBackColor = colorBack;
	newNudge->popupTextColor = colorText;
	newNudge->popupTimeSec = popupTime ;
	newNudge->popupWindowColor = bUseWindowColor;
	
	SkinAddNewSound( newNudge->NudgeSoundname, nudgesoundtemp, "nudge.wav" );
	
	newNudge->next = NudgeList;
	NudgeList = newNudge;

	return 0;
}

HANDLE Nudge_GethContact(HANDLE hCont)
{
	HANDLE hContact;
	hContact = hCont;

	if(ServiceExists(MS_MC_GETMETACONTACT)) //try to retrieve the metacontact if some
		hContact = (HANDLE) CallService( MS_MC_GETMETACONTACT, (WPARAM)hContact, 0 );
	
	if(hContact == NULL) //no metacontact
		hContact = hCont;
	
	return hContact;
}