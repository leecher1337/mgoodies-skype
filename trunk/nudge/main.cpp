#include "headers.h"
#include "main.h"
#include "shake.h"


LRESULT CALLBACK NudgePopUpProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
int nProtocol = 0;
static HANDLE hEventOptionsInitialize;
HINSTANCE hInst;
PLUGINLINK *pluginLink;
NudgeElementList *NudgeList;
CNudgeElement DefaultNudge;
CShake shake;
CNudge GlobalNudge;

//========================
//  MirandaPluginInfo
//========================
PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"Nudge",
	PLUGIN_MAKE_VERSION(0,0,0,10),
	"Plugin to shake the clist and chat window",
	"Tweety/GouZ",
	"francois.mean@skynet.be / Sylvain.gougouzian@gmail.com ",
	"copyright to the miranda community",
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

int NudgeSend(WPARAM wParam,LPARAM lParam)
{

	char *protoName = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);

	if(GlobalNudge.useByProtocol)
	{
		NudgeElementList *n;
		for(n = NudgeList;n != NULL; n = n->next)
		{
			if(!strcmp(protoName,n->item.ProtocolName))
			{
				if(n->item.showEvent)
					Nudge_SentEvent(n->item, (HANDLE) wParam);
			}		
		}
	}
	else
	{
		if(DefaultNudge.showEvent)
			Nudge_SentEvent(DefaultNudge, (HANDLE) wParam);
	}

	char servicefunction[ 100 ];
	sprintf(servicefunction, "%s/SendNudge", protoName);

	CallService(servicefunction, wParam, lParam);

	return 0;
}

int NudgeRecieved(WPARAM wParam,LPARAM lParam)
{

	char *protoName = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);

	if(GlobalNudge.useByProtocol)
	{
		NudgeElementList *n;
		for(n = NudgeList;n != NULL; n = n->next)
		{
			if(!strcmp(protoName,n->item.ProtocolName))
			{
				if(n->item.enabled)
				{
					SkinPlaySound( n->item.NudgeSoundname );
					if(n->item.showPopup)
						Nudge_ShowPopup(n->item, (HANDLE) wParam);
					if(n->item.shakeClist)
						ShakeClist(wParam,lParam);
					if(n->item.shakeChat)
						ShakeChat(wParam,lParam);
					if(n->item.showEvent)
						Nudge_ShowEvent(n->item, (HANDLE) wParam);
				}
			}		
		}
	}
	else
	{
		if(DefaultNudge.enabled)
		{
			SkinPlaySound( DefaultNudge.NudgeSoundname );
			if(DefaultNudge.showPopup)
				Nudge_ShowPopup(DefaultNudge, (HANDLE) wParam);
			if(DefaultNudge.shakeClist)
				ShakeClist(wParam,lParam);
			if(DefaultNudge.shakeChat)
				ShakeChat(wParam,lParam);
			if(DefaultNudge.showEvent)
				Nudge_ShowEvent(DefaultNudge, (HANDLE) wParam);
		}
	}
	return 0;
}


extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

int MainInit(WPARAM wParam,LPARAM lParam)
{
	return 0;
}

void LoadProtocols(void)
{
	//Load the default nudge
	sprintf(DefaultNudge.ProtocolName,"Default");
	sprintf(DefaultNudge.NudgeSoundname,"Nudge : Default");
	SkinAddNewSound( DefaultNudge.NudgeSoundname, DefaultNudge.NudgeSoundname, "nudge.wav" );
	DefaultNudge.Load();

	GlobalNudge.Load();

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
				sprintf(str,"%s/Nudge",ppProtocolDescriptors[i]->szName);
				NudgeEvent = HookEvent(str, NudgeRecieved);
				if(NudgeEvent != NULL)
					Nudge_AddElement(ppProtocolDescriptors[i]->szName);
				
				NudgeEvent = NULL;
			}
		}
		
	}

	shake.Load();

	/*CNudgeElement *n;
	for(n = NudgeList;n != NULL; n = n->next)
	{
		MessageBox(NULL,n->ProtocolName,n->NudgeSoundname,0);
	}*/
}

void RegisterToTrigger(void)
{
	if( ServiceExists(MS_TRIGGER_REGISTERACTION))
	{
		ACTIONREGISTER ar;
		ZeroMemory(&ar, sizeof(ar));
		ar.cbSize = sizeof(ar);
		ar.hInstance = NULL;
		ar.pfnDlgProc = NULL;
		ar.pszTemplate = NULL;

		ar.pszName = Translate("Nudge : Shake contact list");
		ar.pszService = MS_SHAKE_CLIST_TRIGGER;

		/* register the action */
		CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);

		ar.pszName = Translate("Nudge : Shake message window");
		ar.pszService = MS_SHAKE_CHAT_TRIGGER;
		/* register the action */
		CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);
	}
}

void RegisterToDbeditorpp(void)
{
    // known modules list
    if (ServiceExists("DBEditorpp/RegisterSingleModule"))
        CallService("DBEditorpp/RegisterSingleModule", (WPARAM)"Nudge", 0);
}

void LoadIcons(void)
{
	//Load icons
	if(ServiceExists(MS_SKIN2_ADDICON))
	{
		NudgeElementList *n;
		for(n = NudgeList;n != NULL; n = n->next)
		{
			SKINICONDESC sid;
			char szFilename[MAX_PATH];
			char iconName[MAXMODULELABELLENGTH + 10];
			char iconDesc[MAXMODULELABELLENGTH + 10];
			strncpy(szFilename, "plugins\\nudge.dll", MAX_PATH);

			sid.cbSize = sizeof(SKINICONDESC);
			sid.pszSection = Translate("Nudge");
			sid.pszDefaultFile = szFilename;
			sprintf(iconName,"Nudge_%s",n->item.ProtocolName);
			sid.pszName = iconName;
			sprintf(iconDesc,"Nudge for %s",n->item.ProtocolName);
			sid.pszDescription = Translate(iconDesc);
			sid.iDefaultIndex = -IDI_NUDGE;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

			n->item.hIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconName);
		}
		SKINICONDESC sid;
		char szFilename[MAX_PATH];
		char iconName[MAXMODULELABELLENGTH + 10];
		char iconDesc[MAXMODULELABELLENGTH + 10];
		strncpy(szFilename, "plugins\\nudge.dll", MAX_PATH);
		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszSection = Translate("Nudge");
		sid.pszDefaultFile = szFilename;
		sprintf(iconName,"Nudge_Default");
		sid.pszName = iconName;
		sprintf(iconDesc,"Nudge as Default");
		sid.pszDescription = Translate(iconDesc);
		sid.iDefaultIndex = -IDI_NUDGE;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		DefaultNudge.hIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconName);
	}
	else // Do not forget people not using IcoLib!!!!
	{
		NudgeElementList *n;
		for(n = NudgeList;n != NULL; n = n->next)
		{
			n->item.hIcon = (HICON)CallProtoService(n->item.ProtocolName, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);
			if(n->item.hIcon == NULL || (int)n->item.hIcon == CALLSERVICE_NOTFOUND)
				n->item.hIcon = (HICON)CallProtoService(n->item.ProtocolName, PS_LOADICON, PLI_PROTOCOL, 0);
 			if(n->item.hIcon == NULL || (int)n->item.hIcon == CALLSERVICE_NOTFOUND)
				n->item.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NUDGE));
		}
		DefaultNudge.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NUDGE));
	}
}

static int LoadChangedIcons(WPARAM wParam, LPARAM lParam)
{
	//Load icons
	if(ServiceExists(MS_SKIN2_ADDICON))
	{
		NudgeElementList *n;
		for(n = NudgeList;n != NULL; n = n->next)
		{
			char iconName[MAXMODULELABELLENGTH + 10];
			sprintf(iconName,"Nudge_%s",n->item.ProtocolName);
			n->item.hIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconName);
		}
	}
	return 0;
}

int ModulesLoaded(WPARAM,LPARAM)
{
	RegisterToUpdate();
	RegisterToTrigger();
	RegisterToDbeditorpp();
	LoadProtocols();
	LoadIcons();
	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{ 	
	pluginLink = link;
	NudgeList = NULL;
	HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	if(ServiceExists(MS_SKIN2_ADDICON))
        HookEvent(ME_SKIN2_ICONSCHANGED, LoadChangedIcons);
	
	InitOptions();

	//Create function for plugins
	CreateServiceFunction(MS_SHAKE_CLIST,ShakeClist);
	CreateServiceFunction(MS_SHAKE_CHAT,ShakeChat);
	CreateServiceFunction(MS_SHAKE_CHAT_TRIGGER,TriggerShakeChat);
	CreateServiceFunction(MS_SHAKE_CLIST_TRIGGER,TriggerShakeClist);
	CreateServiceFunction(MS_NUDGE_SEND,NudgeSend);
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
	if( GlobalNudge.useByProtocol )
	{
		NudgeElementList *n;
		for(n = NudgeList;n != NULL; n = n->next)
		{
			if(n->item.enabled)
			{
				SkinPlaySound( n->item.NudgeSoundname );
				if(n->item.showPopup)
					Nudge_ShowPopup(n->item, NULL);
				if(n->item.shakeClist)
					ShakeClist(0,0);
			}
		}
	}
	else
	{
		if(DefaultNudge.enabled)
		{
			SkinPlaySound( DefaultNudge.NudgeSoundname );
			if(DefaultNudge.showPopup)
				Nudge_ShowPopup(DefaultNudge, NULL);
			if(DefaultNudge.shakeClist)
				ShakeClist(0,0);
		}
	}
	return 0;
}

void Nudge_ShowPopup(CNudgeElement n, HANDLE hCont)
{
	POPUPDATAEX NudgePopUp;
	HANDLE hContact;

	hContact = Nudge_GethContact(hCont);

	if(hContact == NULL) //no contact at all
		NudgePopUp.lchContact = (HANDLE) &n;

	NudgePopUp.lchContact = hContact;
	NudgePopUp.lchIcon = n.hIcon;
	NudgePopUp.colorBack = ! n.popupWindowColor ? n.popupBackColor : GetSysColor(COLOR_BTNFACE);
	NudgePopUp.colorText = ! n.popupWindowColor ? n.popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
	NudgePopUp.iSeconds = n.popupTimeSec;
	NudgePopUp.PluginWindowProc = (WNDPROC)NudgePopUpProc;
	NudgePopUp.PluginData = (void *)1;

	char * lpzContactName = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0);
	
	sprintf(NudgePopUp.lpzText, Translate("You received a nudge"));
	lstrcpy(NudgePopUp.lpzContactName, lpzContactName);

	CallService(MS_POPUP_ADDPOPUPEX,(WPARAM)&NudgePopUp,0);
}

BOOL CheckMsgWnd(HANDLE hContact)
{
	if (ServiceExists(MS_MSG_GETWINDOWDATA))	// use the new Window API
	{
		MessageWindowData mwd;
		MessageWindowInputData mwid;

		mwid.cbSize = sizeof(MessageWindowInputData); 
		mwid.hContact = Nudge_GethContact(hContact);
		mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
		mwd.cbSize = sizeof(MessageWindowData);
		mwd.hContact = Nudge_GethContact(hContact);
		if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd) && mwd.hwndWindow)
			return TRUE;
	}

	return FALSE;
}

void Nudge_SentEvent(CNudgeElement n, HANDLE hCont)
{
	DBEVENTINFO NudgeEvent = { 0 };;
	HANDLE hContact;
	HANDLE hMetaContact = NULL;
	char* EventLog;

	hContact = hCont;
	EventLog = Translate("You sent a nudge");

	NudgeEvent.cbSize = sizeof(NudgeEvent);
	NudgeEvent.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	NudgeEvent.flags = DBEF_SENT;
	NudgeEvent.timestamp = ( DWORD )time(NULL);
	NudgeEvent.eventType = EVENTTYPE_MESSAGE;
	NudgeEvent.cbBlob = strlen( EventLog )+1;
	NudgeEvent.pBlob = ( PBYTE )EventLog;

	if(ServiceExists(MS_MC_GETMETACONTACT)) //try to retrieve the metacontact if some
		hMetaContact = (HANDLE) CallService( MS_MC_GETMETACONTACT, (WPARAM)hContact, 0 );
	
	if(hMetaContact != NULL) //metacontact
		CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&NudgeEvent);
	
	CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&NudgeEvent);
}

void Nudge_ShowEvent(CNudgeElement n, HANDLE hCont)
{
	DBEVENTINFO NudgeEvent = { 0 };;
	HANDLE hContact;
	HANDLE hMetaContact = NULL;
	char* EventLog;

	hContact = hCont;
	EventLog = Translate("You received a nudge");

	NudgeEvent.cbSize = sizeof(NudgeEvent);
	NudgeEvent.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	NudgeEvent.flags = CheckMsgWnd(hContact) ? 0 : DBEF_READ;
	NudgeEvent.timestamp = ( DWORD )time(NULL);
	NudgeEvent.eventType = EVENTTYPE_MESSAGE;
	NudgeEvent.cbBlob = strlen( EventLog )+1;
	NudgeEvent.pBlob = ( PBYTE )EventLog;

	if(ServiceExists(MS_MC_GETMETACONTACT)) //try to retrieve the metacontact if some
		hMetaContact = (HANDLE) CallService( MS_MC_GETMETACONTACT, (WPARAM)hContact, 0 );
	
	if(hMetaContact != NULL) //metacontact
	{
		CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&NudgeEvent);
		NudgeEvent.flags = DBEF_READ;
	}
	
	CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&NudgeEvent);
}

int Nudge_AddElement(char *protoName)
{
	nProtocol ++;
	//Add contact menu entry
	CLISTMENUITEM mi;

	memset( &mi, 0, sizeof( mi ));
	mi.popupPosition = 500085000;
	mi.pszContactOwner = protoName;
	mi.pszPopupName = protoName;
	mi.cbSize = sizeof( mi );
	mi.flags = CMIF_NOTOFFLINE;
	mi.position = -500050004;
	mi.hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_NUDGE ));
	mi.pszName = Translate( "Send &Nudge" );
	mi.pszService = MS_NUDGE_SEND;
	CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );
	
	//Add a specific sound per protocol
	char nudgesoundtemp[ 64 ];
	NudgeElementList *newNudge;
	newNudge = (NudgeElementList*) malloc(sizeof(NudgeElementList));
	strcpy( nudgesoundtemp, protoName );
	strcat( nudgesoundtemp, ": " );
	strcat( nudgesoundtemp,  Translate( "Nudge" ));
	strcpy( newNudge->item.NudgeSoundname, nudgesoundtemp ); 

	strcpy( newNudge->item.ProtocolName, protoName );

	newNudge->item.Load();
	
	SkinAddNewSound( newNudge->item.NudgeSoundname, newNudge->item.NudgeSoundname, "nudge.wav" );
	
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
