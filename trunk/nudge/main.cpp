#include "headers.h"
#include "main.h"
#include "shake.h"
#include "include\m_msg_buttonsbar.h"


static INT_PTR CALLBACK DlgProcOptsTrigger(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK NudgePopUpProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

int nProtocol = 0;
static HANDLE g_hEventModulesLoaded = NULL, g_hEventIconsChanged = NULL,hEventOptionsInitialize, g_hIcon = NULL, g_hEventDbWindowEvent = NULL, g_hEventToolbarLoaded = NULL, g_hEventButtonPressed = NULL;
HINSTANCE hInst;
PLUGINLINK *pluginLink;
NudgeElementList *NudgeList;
CNudgeElement DefaultNudge;
CShake shake;
CNudge GlobalNudge;

MM_INTERFACE mmi;
int hLangpack = 0;

BOOL     (WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD) = 0;
HMODULE hUxTheme = 0;

// function pointers, use typedefs for casting to shut up the compiler when using GetProcAddress()

typedef BOOL (WINAPI *PITA)();
typedef HANDLE (WINAPI *POTD)(HWND, LPCWSTR);
typedef UINT (WINAPI *PDTB)(HANDLE, HDC, int, int, RECT *, RECT *);
typedef UINT (WINAPI *PCTD)(HANDLE);
typedef UINT (WINAPI *PDTT)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, RECT *);

PITA pfnIsThemeActive = 0;
POTD pfnOpenThemeData = 0;
PDTB pfnDrawThemeBackground = 0;
PCTD pfnCloseThemeData = 0;
PDTT pfnDrawThemeText = 0;

#define FIXED_TAB_SIZE 100                  // default value for fixed width tabs

/*
 * visual styles support (XP+)
 * returns 0 on failure
 */

int InitVSApi()
{
    if((hUxTheme = LoadLibraryA("uxtheme.dll")) == 0)
        return 0;

    pfnIsThemeActive = (PITA)GetProcAddress(hUxTheme, "IsThemeActive");
    pfnOpenThemeData = (POTD)GetProcAddress(hUxTheme, "OpenThemeData");
    pfnDrawThemeBackground = (PDTB)GetProcAddress(hUxTheme, "DrawThemeBackground");
    pfnCloseThemeData = (PCTD)GetProcAddress(hUxTheme, "CloseThemeData");
    pfnDrawThemeText = (PDTT)GetProcAddress(hUxTheme, "DrawThemeText");
    
    MyEnableThemeDialogTexture = (BOOL (WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
    if(pfnIsThemeActive != 0 && pfnOpenThemeData != 0 && pfnDrawThemeBackground != 0 && pfnCloseThemeData != 0 && pfnDrawThemeText != 0) {
        return 1;
    }
    return 0;
}

/*
 * unload uxtheme.dll
 */

int FreeVSApi()
{
    if(hUxTheme != 0)
        FreeLibrary(hUxTheme);
    return 0;
}

DWORD MirVer;


//========================
//  MirandaPluginInfo
//========================
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Nudge",
	PLUGIN_MAKE_VERSION(0,0,1,19),
	"Plugin to shake the clist and chat window",
	"Tweety/GouZ",
	"francois.mean@skynet.be / Sylvain.gougouzian@gmail.com ",
	"copyright to the miranda community",
	"http://addons.miranda-im.org/details.php?action=viewfile&id=2708",		// www
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
	{ 0x9ceee701, 0x35cd, 0x4ff7, { 0x8c, 0xc4, 0xef, 0x7d, 0xd2, 0xac, 0x53, 0x5c } }	// {9CEEE701-35CD-4ff7-8CC4-EF7DD2AC535C}
};


 
void RegisterToUpdate(void)
{
	//Use for the Updater plugin
	if(ServiceExists(MS_UPDATE_REGISTER)) 
	{
		Update update = {0};
		char szVersion[16];

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionStringPluginEx(&pluginInfo, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);
		update.szUpdateURL = UPDATER_AUTOREGISTER;
		update.szVersionURL = "http://addons.miranda-im.org/details.php?action=viewfile&id=2708";
		update.pbVersionPrefix = (BYTE *)"<span class=\"fileNameHeader\">Nudge ";
	    update.szBetaUpdateURL = "http://www.miranda-fr.net/tweety/Nudge/Nudge.zip";
		update.szBetaVersionURL = "http://www.miranda-fr.net/tweety/Nudge/Nudge_beta.html";
		update.pbBetaVersionPrefix = (BYTE *)"Nudge version ";

		update.cpbVersionPrefix = strlen((char *)update.pbVersionPrefix);
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);

	}
}

INT_PTR NudgeShowMenu(WPARAM wParam,LPARAM lParam)
{	

	NudgeElementList *n;
	for(n = NudgeList;n != NULL; n = n->next)
	{
		if(!strcmp((char *) wParam,n->item.ProtocolName))
		{
			return n->item.ShowContactMenu(lParam != 0);
		}		
	}
	return 0;
}

INT_PTR NudgeSend(WPARAM wParam,LPARAM lParam)
{

	char *protoName = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);	
	int diff = time(NULL) - DBGetContactSettingDword((HANDLE) wParam, "Nudge", "LastSent", time(NULL)-30);

	if(diff < GlobalNudge.sendTimeSec)
	{
		TCHAR msg[500];
		mir_sntprintf(msg,500, TranslateT("You are not allowed to send too much nudge (only 1 each %d sec, %d sec left)"),GlobalNudge.sendTimeSec, 30 - diff);
		//MessageBox(NULL,msg,NULL,0);
		if(GlobalNudge.useByProtocol)
		{
			NudgeElementList *n;
			for(n = NudgeList;n != NULL; n = n->next)
			{
				if(!strcmp(protoName,n->item.ProtocolName))
				{
					Nudge_ShowPopup(n->item, (HANDLE) wParam, msg);
				}		
			}	
		}
		else
		{
			Nudge_ShowPopup(DefaultNudge, (HANDLE) wParam, msg);
		}
		return 0;
	}
	
	DBWriteContactSettingDword((HANDLE) wParam, "Nudge", "LastSent", time(NULL));

	if(GlobalNudge.useByProtocol)
	{
		NudgeElementList *n;
		for(n = NudgeList;n != NULL; n = n->next)
		{
			if(!strcmp(protoName,n->item.ProtocolName))
			{
				if(n->item.showPopup)
					Nudge_ShowPopup(n->item, (HANDLE) wParam, n->item.senText);
				if(n->item.showEvent)
					Nudge_SentEvent(n->item, (HANDLE) wParam);
				if(n->item.showStatus)
					Nudge_SentStatus(n->item, (HANDLE) wParam);
			}		
		}
	}
	else
	{
		if(DefaultNudge.showPopup)
			Nudge_ShowPopup(DefaultNudge, (HANDLE) wParam, DefaultNudge.senText);
		if(DefaultNudge.showEvent)
			Nudge_SentEvent(DefaultNudge, (HANDLE) wParam);
		if(DefaultNudge.showStatus)
			Nudge_SentStatus(DefaultNudge, (HANDLE) wParam);
	}

	char servicefunction[ 100 ];
	sprintf(servicefunction, "%s/SendNudge", protoName);

	CallService(servicefunction, wParam, lParam);

	return 0;
}

int NudgeRecieved(WPARAM wParam,LPARAM lParam)
{
	
	char *protoName = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);

	DWORD currentTimestamp = time(NULL);
	DWORD nudgeSentTimestamp = lParam ? (DWORD)lParam : currentTimestamp;

	int diff = currentTimestamp - DBGetContactSettingDword((HANDLE) wParam, "Nudge", "LastReceived", currentTimestamp-30);
	int diff2 = nudgeSentTimestamp - DBGetContactSettingDword((HANDLE) wParam, "Nudge", "LastReceived2", nudgeSentTimestamp-30);
	
	if(diff >= GlobalNudge.recvTimeSec)
		DBWriteContactSettingDword((HANDLE) wParam, "Nudge", "LastReceived", currentTimestamp);
	if(diff2 >= GlobalNudge.recvTimeSec)
		DBWriteContactSettingDword((HANDLE) wParam, "Nudge", "LastReceived2", nudgeSentTimestamp);

	if(GlobalNudge.useByProtocol)
	{
		NudgeElementList *n;
		for(n = NudgeList;n != NULL; n = n->next)
		{
			if(!strcmp(protoName,n->item.ProtocolName))
			{
				
				if(n->item.enabled)
				{
					DWORD Status = CallProtoService(protoName,PS_GETSTATUS,0,0);

					if( ((n->item.statusFlags & NUDGE_ACC_ST0) && (Status<=ID_STATUS_OFFLINE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST1) && (Status==ID_STATUS_ONLINE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST2) && (Status==ID_STATUS_AWAY)) ||
						((n->item.statusFlags & NUDGE_ACC_ST3) && (Status==ID_STATUS_DND)) ||
						((n->item.statusFlags & NUDGE_ACC_ST4) && (Status==ID_STATUS_NA)) ||
						((n->item.statusFlags & NUDGE_ACC_ST5) && (Status==ID_STATUS_OCCUPIED)) ||
						((n->item.statusFlags & NUDGE_ACC_ST6) && (Status==ID_STATUS_FREECHAT)) ||
						((n->item.statusFlags & NUDGE_ACC_ST7) && (Status==ID_STATUS_INVISIBLE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST8) && (Status==ID_STATUS_ONTHEPHONE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST9) && (Status==ID_STATUS_OUTTOLUNCH)))
					{
						if(diff >= GlobalNudge.recvTimeSec)
						{
							if(n->item.showPopup)
								Nudge_ShowPopup(n->item, (HANDLE) wParam, n->item.recText);
							if(n->item.shakeClist)
								ShakeClist(wParam,lParam);
							if(n->item.shakeChat)
								ShakeChat(wParam,lParam);
							if(n->item.autoResend)
								mir_forkthread(AutoResendNudge,(void *)wParam);

							SkinPlaySound( n->item.NudgeSoundname );
						}
					}
					if(diff2 >= GlobalNudge.recvTimeSec)
					{
						if(n->item.showEvent)
							Nudge_ShowEvent(n->item, (HANDLE) wParam, nudgeSentTimestamp);
						if(n->item.showStatus)
							Nudge_ShowStatus(n->item, (HANDLE) wParam, nudgeSentTimestamp);
					}
					
				}
			}		
		}
	}
	else
	{
		if(DefaultNudge.enabled)
		{
			DWORD Status = CallService(MS_CLIST_GETSTATUSMODE,0,0);
			if( ((DefaultNudge.statusFlags & NUDGE_ACC_ST0) && (Status<=ID_STATUS_OFFLINE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST1) && (Status==ID_STATUS_ONLINE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST2) && (Status==ID_STATUS_AWAY)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST3) && (Status==ID_STATUS_DND)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST4) && (Status==ID_STATUS_NA)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST5) && (Status==ID_STATUS_OCCUPIED)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST6) && (Status==ID_STATUS_FREECHAT)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST7) && (Status==ID_STATUS_INVISIBLE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST8) && (Status==ID_STATUS_ONTHEPHONE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST9) && (Status==ID_STATUS_OUTTOLUNCH)))
			{
				if(diff >= GlobalNudge.recvTimeSec)
				{
					if(DefaultNudge.showPopup)
						Nudge_ShowPopup(DefaultNudge, (HANDLE) wParam, DefaultNudge.recText);
					if(DefaultNudge.shakeClist)
						ShakeClist(wParam,lParam);
					if(DefaultNudge.shakeChat)
						ShakeChat(wParam,lParam);
					if(DefaultNudge.autoResend)
						mir_forkthread(AutoResendNudge,(void *)wParam);

					SkinPlaySound( DefaultNudge.NudgeSoundname );
				}
			}
			if(diff2 >= GlobalNudge.recvTimeSec)
			{
				if(DefaultNudge.showEvent)
					Nudge_ShowEvent(DefaultNudge, (HANDLE) wParam, nudgeSentTimestamp);
				if(DefaultNudge.showStatus)
					Nudge_ShowStatus(DefaultNudge, (HANDLE) wParam, nudgeSentTimestamp);
			}
		}
	}
	return 0;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	MirVer = mirandaVersion;
	return &pluginInfo;
}

static const MUUID interfaces[] = {MUUID_NUDGE_SEND, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

int MainInit(WPARAM wParam,LPARAM lParam)
{
	return 0;
}

static INT_PTR CALLBACK DlgProcOptsTrigger(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
	case WM_INITDIALOG: {
		// lParam = (LPARAM)(DWORD)actionID or 0 if this is a new trigger entry
		DWORD actionID;
		BOOL bshakeClist,bshakeChat;
		
		actionID = (DWORD)lParam;
        TranslateDialogDefault(hwnd);
		// Initialize the dialog according to the action ID
		bshakeClist = DBGetActionSettingByte(actionID, NULL, "Nudge", "ShakeClist",FALSE);
		bshakeChat = DBGetActionSettingByte(actionID, NULL, "Nudge", "ShakeChat",FALSE);
		CheckDlgButton(hwnd, IDC_TRIGGER_SHAKECLIST, bshakeClist ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_TRIGGER_SHAKECHAT, bshakeChat ? BST_CHECKED : BST_UNCHECKED);
        break;
						}

	case TM_ADDACTION: {
		// save your settings
		// wParam = (WPARAM)(DWORD)actionID
		DWORD actionID;
		bool bshakeClist,bshakeChat;

		actionID = (DWORD)wParam;
		bshakeClist = (IsDlgButtonChecked(hwnd,IDC_TRIGGER_SHAKECLIST)==BST_CHECKED);
		bshakeChat = (IsDlgButtonChecked(hwnd,IDC_TRIGGER_SHAKECHAT)==BST_CHECKED);
		DBWriteActionSettingByte(actionID, NULL, "Nudge", "ShakeClist",bshakeClist);
		DBWriteActionSettingByte(actionID, NULL, "Nudge", "ShakeChat",bshakeChat);
		break;
					   }
	}

    return FALSE;
}

int TriggerActionRecv( DWORD actionID, REPORTINFO *ri)
{
	// check how to process this call
	if (ri->flags&ACT_PERFORM) {
		BOOL bshakeClist,bshakeChat;
		HANDLE hContact = ((ri->td!=NULL)&&(ri->td->dFlags&DF_CONTACT))?ri->td->hContact:NULL;
		bshakeClist = DBGetActionSettingByte(actionID, NULL, "Nudge", "ShakeClist",FALSE);
		bshakeChat = DBGetActionSettingByte(actionID, NULL, "Nudge", "ShakeChat",FALSE);

		if(bshakeClist)
			ShakeClist(NULL,NULL);
		if(bshakeChat && (hContact != NULL))
			ShakeChat((WPARAM)hContact,NULL);

	/*	// Actually show the message box
		DBVARIANT dbv;
		TCHAR *tszMsg;
		
		// Retrieve the correct settings for this action ID
		if (!DBGetActionSettingTString(actionID, NULL, MODULENAME, SETTING_TEXT, &dbv)) {
			// Parse by Variables, if available (notice extratext and subject are given).
			tszMsg = variables_parsedup(dbv.ptszVal, ((ri->td!=NULL)&&(ri->td->dFlags&DF_TEXT))?ri->td->tszText:NULL, ((ri->td!=NULL)&&(ri->td->dFlags&DF_CONTACT))?ri->td->hContact:NULL);
			if (tszMsg != NULL) {
				// Show the message box
				MessageBox(NULL, tszMsg, TranslateT("ExampleAction"), MB_OK);
				free(tszMsg);
			}
			DBFreeVariant(&dbv);
		}
		*/
	}
	if (ri->flags&ACT_CLEANUP) { // request to delete all associated settings
		RemoveAllActionSettings(actionID, "Nudge");
	}
	return FALSE;
}

int TriggerActionSend( DWORD actionID, REPORTINFO *ri)
{
	if (ri->flags&ACT_PERFORM) {
		HANDLE hContact = ((ri->td!=NULL)&&(ri->td->dFlags&DF_CONTACT))?ri->td->hContact:NULL;
		if(hContact != NULL)
			NudgeSend((WPARAM)hContact,NULL);
	}
	
	return FALSE;
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
					Nudge_AddElement(ppProtocolDescriptors[i]->szName, NudgeEvent);
				
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
		ar.hInstance = hInst;
		ar.flags = ARF_TCHAR|ARF_FUNCTION;
		ar.actionFunction = TriggerActionRecv;
		ar.pfnDlgProc = DlgProcOptsTrigger;
		ar.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TRIGGER);
		ar.pszName = Translate("Nudge : Shake contact list/chat window");

		// register the action 
		CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);

		ar.actionFunction = TriggerActionSend;
		ar.pszName = Translate("Nudge : Send a nudge");
		ar.pfnDlgProc = NULL;
		ar.pszTemplate = NULL;

		// register the action
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
		SKINICONDESC sid = {0};
		TCHAR szFilename[MAX_PATH];
		char iconName[MAXMODULELABELLENGTH + 10];
		char iconDesc[MAXMODULELABELLENGTH + 10];
		GetModuleFileName(hInst,szFilename,MAX_PATH);

		sid.cbSize = SKINICONDESC_SIZE;
		sid.flags = SIDF_PATH_TCHAR;
		sid.pszSection = "Nudge";
		sid.ptszDefaultFile = szFilename;

		for(NudgeElementList *n = NudgeList;n != NULL; n = n->next)
		{			
			sprintf(iconName,"Nudge_%s",n->item.ProtocolName);
			sid.pszName = iconName;
			sprintf(iconDesc,"%s %s",Translate("Nudge for"),n->item.ProtocolName);
			sid.pszDescription = iconDesc;
			sid.iDefaultIndex = -IDI_NUDGE;
			sid.hDefaultIcon =  LoadIcon(hInst,MAKEINTRESOURCE(IDI_NUDGE));
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

			n->item.hIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconName);
		}

		sprintf(iconName,"Nudge_Default");
		sid.pszName = iconName;
		sprintf(iconDesc,Translate("Nudge as Default"));
		sid.pszDescription = iconDesc;
		sid.iDefaultIndex = -IDI_NUDGE;
		sid.hDefaultIcon =  LoadIcon(hInst,MAKEINTRESOURCE(IDI_NUDGE));
		g_hIcon = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		DefaultNudge.hIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconName);
	}
	else // Do not forget people not using IcoLib!!!!
	{
		for(NudgeElementList *n = NudgeList;n != NULL; n = n->next)
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
		char iconName[MAXMODULELABELLENGTH + 10];

		for(n = NudgeList;n != NULL; n = n->next)
		{
			sprintf(iconName,"Nudge_%s",n->item.ProtocolName);
			n->item.hIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconName);
		}
		sprintf(iconName,"Nudge_Default");
		DefaultNudge.hIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconName);
	}
	return 0;
}

// Nudge support
static int TabsrmmButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd=(CustomButtonClickData *)lParam;

	if(!strcmp(cbcd->pszModule,"Nudge"))
		NudgeSend(wParam, 0);

	return 0;
}

static int TabsrmmButtonInit(WPARAM wParam, LPARAM lParam)
{
	BBButton bbd = {0};
 
	bbd.cbSize = sizeof(BBButton);
	bbd.pszModuleName = "Nudge";
	bbd.ptszTooltip = LPGENT("Nudge");
	bbd.dwDefPos = 300;
	bbd.bbbFlags = BBBF_ISIMBUTTON|BBBF_ISLSIDEBUTTON|BBBF_CANBEHIDDEN;
	bbd.hIcon = g_hIcon;
	bbd.dwButtonID = 6000;
	bbd.iButtonWidth = 0;
	CallService (MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

	return 0;
} 

void HideNudgeButton(HANDLE hContact) 
{ 
	char str[MAXMODULELABELLENGTH + 12] = {0};
	CONTACTINFO ci = { 0 };
					
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;

	CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci);
	mir_snprintf(str,MAXMODULELABELLENGTH + 12,"%s/SendNudge", ci.szProto);

	if (!ServiceExists(str)) 
    { 
      BBButton bbd={0}; 
      bbd.cbSize=sizeof(BBButton); 
      bbd.bbbFlags=BBSF_HIDDEN|BBSF_DISABLED; 
      bbd.pszModuleName="Nudge";
	  bbd.dwButtonID = 6000;
      CallService(MS_BB_SETBUTTONSTATE, (WPARAM)hContact, (LPARAM)&bbd); 
    } 
} 

static int ContactWindowOpen(WPARAM wparam,LPARAM lParam) 
{ 
   MessageWindowEventData *MWeventdata = (MessageWindowEventData*)lParam; 

   if(MWeventdata->uType == MSG_WINDOW_EVT_OPENING&&MWeventdata->hContact) 
   { 
      HideNudgeButton(MWeventdata->hContact); 
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
	g_hEventToolbarLoaded = HookEvent(ME_MSG_TOOLBARLOADED, TabsrmmButtonInit);
	if (g_hEventToolbarLoaded)
	{
		g_hEventButtonPressed = HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
		g_hEventDbWindowEvent = HookEvent(ME_MSG_WINDOWEVENT,ContactWindowOpen);
	}
	return 0;
}

HANDLE hShakeClist=NULL,hShakeChat=NULL,hNudgeSend=NULL,hNudgeShowMenu=NULL;
extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{ 	
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getLP(&pluginInfo);
	NudgeList = NULL;
	g_hEventModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	if(ServiceExists(MS_SKIN2_ADDICON))
        g_hEventIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, LoadChangedIcons);
	
	InitOptions();
	InitVSApi();

	//Create function for plugins
	hShakeClist=CreateServiceFunction(MS_SHAKE_CLIST,ShakeClist);
	hShakeChat=CreateServiceFunction(MS_SHAKE_CHAT,ShakeChat);
	hNudgeSend=CreateServiceFunction(MS_NUDGE_SEND,NudgeSend);
	hNudgeShowMenu=CreateServiceFunction(MS_NUDGE_SHOWMENU,NudgeShowMenu);
	return 0; 
}

extern "C" int __declspec(dllexport) Unload(void) 
{ 
	if(g_hEventToolbarLoaded) UnhookEvent(g_hEventToolbarLoaded);
	if(g_hEventDbWindowEvent) UnhookEvent(g_hEventButtonPressed);
	if(g_hEventDbWindowEvent) UnhookEvent(g_hEventDbWindowEvent);

	if(g_hEventModulesLoaded) UnhookEvent(g_hEventModulesLoaded);
	if(g_hEventIconsChanged) UnhookEvent(g_hEventIconsChanged);

	DestroyServiceFunction(hShakeClist);
	DestroyServiceFunction(hShakeChat);
	DestroyServiceFunction(hNudgeSend);
	DestroyServiceFunction(hNudgeShowMenu);

	UninitOptions();

	FreeVSApi();
	NudgeElementList* p = NudgeList;
	while ( p != NULL ) 
	{
		if(p->item.hEvent) UnhookEvent(p->item.hEvent);
		NudgeElementList* p1 = p->next;
		//free( p );
		delete p;
		p = p1;
	}
	return 0; 
}

LRESULT CALLBACK NudgePopUpProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	switch(msg)
	{
		case WM_COMMAND:
		{
			HANDLE hContact = PUGetContact(hWnd);
			CallService(MS_MSG_SENDMESSAGET, (WPARAM)hContact, 0);
			PUDeletePopUp(hWnd);
			break;
		}

		case WM_CONTEXTMENU:
			PUDeletePopUp(hWnd);
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
		HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST,0,0);

		for(NudgeElementList *n = NudgeList;n != NULL; n = n->next)
		{
			if(n->item.enabled)
			{
				SkinPlaySound( n->item.NudgeSoundname );
				if(n->item.showPopup)
					Nudge_ShowPopup(n->item, hContact, n->item.recText);
				if(n->item.shakeClist)
					ShakeClist(0,0);
				if(n->item.shakeChat)
				{
					CallService(MS_MSG_SENDMESSAGET,(WPARAM)hContact,NULL);
					ShakeChat((WPARAM)hContact,(LPARAM)time(NULL));
				}
			}
		}
	}
	else
	{
		if(DefaultNudge.enabled)
		{
			HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST,0,0);

			SkinPlaySound( DefaultNudge.NudgeSoundname );
			if(DefaultNudge.showPopup)
				Nudge_ShowPopup(DefaultNudge, hContact, DefaultNudge.recText);
			if(DefaultNudge.shakeClist)
				ShakeClist(0,0);
			if(DefaultNudge.shakeChat)
			{
				CallService(MS_MSG_SENDMESSAGET,(WPARAM)hContact,NULL);
				ShakeChat((WPARAM)hContact,(LPARAM)time(NULL));
			}
		}
	}
	return 0;
}

void Nudge_ShowPopup(CNudgeElement n, HANDLE hCont, TCHAR * Message)
{
	HANDLE hContact;

	hContact = Nudge_GethContact(hCont);
	TCHAR * lpzContactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,GCDNF_TCHAR);
	
	if(ServiceExists(MS_POPUP_ADDPOPUPT)) 
	{
		POPUPDATAT NudgePopUp;
		
		if(hContact == NULL) //no contact at all
			NudgePopUp.lchContact = (HANDLE) &n;

		NudgePopUp.lchContact = hContact;
		NudgePopUp.lchIcon = n.hIcon;
		NudgePopUp.colorBack = ! n.popupWindowColor ? n.popupBackColor : GetSysColor(COLOR_BTNFACE);
		NudgePopUp.colorText = ! n.popupWindowColor ? n.popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
		NudgePopUp.iSeconds = n.popupTimeSec;
		NudgePopUp.PluginWindowProc = (WNDPROC)NudgePopUpProc;
		NudgePopUp.PluginData = (void *)1;
		
		//lstrcpy(NudgePopUp.lpzText, Translate(Message));
		lstrcpy(NudgePopUp.lptzText, Message);

		lstrcpy(NudgePopUp.lptzContactName, lpzContactName);

		CallService(MS_POPUP_ADDPOPUPT,(WPARAM)&NudgePopUp,0);
	}
	else
	{
		MessageBox(NULL,Message,lpzContactName,0);
	}
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

	hContact = hCont;

	NudgeEvent.cbSize = sizeof(NudgeEvent);
	NudgeEvent.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	NudgeEvent.flags = DBEF_SENT;
	NudgeEvent.timestamp = ( DWORD )time(NULL);
	NudgeEvent.eventType = EVENTTYPE_MESSAGE;
	#if defined( _UNICODE )
		char buff[TEXT_LEN];
		WideCharToMultiByte(code_page, 0, n.senText, -1, buff, TEXT_LEN, 0, 0);
		buff[TEXT_LEN] = 0;
		NudgeEvent.cbBlob = strlen(buff) + 1;
		NudgeEvent.pBlob = ( PBYTE ) buff;
	#else
		NudgeEvent.cbBlob = _tcsclen(n.senText) + 1;
		NudgeEvent.pBlob = ( PBYTE ) n.senText;
	#endif

	if(ServiceExists(MS_MC_GETMETACONTACT)) //try to retrieve the metacontact if some
		hMetaContact = (HANDLE) CallService( MS_MC_GETMETACONTACT, (WPARAM)hContact, 0 );
	
	if(hMetaContact != NULL) //metacontact
		CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&NudgeEvent);
	
	CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&NudgeEvent);
}

void Nudge_SentStatus(CNudgeElement n, HANDLE hCont)
{
	DBEVENTINFO NudgeEvent = { 0 };;
	HANDLE hContact;
	HANDLE hMetaContact = NULL;

	hContact = hCont;

	NudgeEvent.cbSize = sizeof(NudgeEvent);
	NudgeEvent.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	NudgeEvent.flags = 0;
	NudgeEvent.timestamp = ( DWORD )time(NULL);
	NudgeEvent.eventType = EVENTTYPE_STATUSCHANGE;
	#if defined( _UNICODE )
		char buff[TEXT_LEN];
		WideCharToMultiByte(code_page, 0, n.senText, -1, buff, TEXT_LEN, 0, 0);
		buff[TEXT_LEN] = 0;
		NudgeEvent.cbBlob = strlen(buff) + 1;
		NudgeEvent.pBlob = ( PBYTE ) buff;
	#else
		NudgeEvent.cbBlob = _tcsclen(n.senText) + 1;
		NudgeEvent.pBlob = ( PBYTE ) n.senText;
	#endif

	if(ServiceExists(MS_MC_GETMETACONTACT)) //try to retrieve the metacontact if some
		hMetaContact = (HANDLE) CallService( MS_MC_GETMETACONTACT, (WPARAM)hContact, 0 );
	
	if(hMetaContact != NULL) //metacontact
		CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&NudgeEvent);
	
	CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&NudgeEvent);
}

void Nudge_ShowStatus(CNudgeElement n, HANDLE hCont, DWORD timestamp)
{
	DBEVENTINFO NudgeEvent = { 0 };;
	HANDLE hContact;
	HANDLE hMetaContact = NULL;

	hContact = hCont;

	NudgeEvent.cbSize = sizeof(NudgeEvent);
	NudgeEvent.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	NudgeEvent.flags = 0;
	NudgeEvent.timestamp = timestamp;
	NudgeEvent.eventType = EVENTTYPE_STATUSCHANGE;
	#if defined( _UNICODE )
		char buff[TEXT_LEN];
		WideCharToMultiByte(code_page, 0, n.recText, -1, buff, TEXT_LEN, 0, 0);
		buff[TEXT_LEN] = 0;
		NudgeEvent.cbBlob = strlen(buff) + 1;
		NudgeEvent.pBlob = ( PBYTE ) buff;
	#else
		NudgeEvent.cbBlob = _tcsclen(n.recText) + 1;
		NudgeEvent.pBlob = ( PBYTE ) n.recText;
	#endif

	if(ServiceExists(MS_MC_GETMETACONTACT)) //try to retrieve the metacontact if some
		hMetaContact = (HANDLE) CallService( MS_MC_GETMETACONTACT, (WPARAM)hContact, 0 );
	
	if(hMetaContact != NULL) //metacontact
	{
		CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&NudgeEvent);
		NudgeEvent.flags = DBEF_READ;
	}
	
	CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&NudgeEvent);
}

void Nudge_ShowEvent(CNudgeElement n, HANDLE hCont, DWORD timestamp)
{
	DBEVENTINFO NudgeEvent = { 0 };
	HANDLE hContact;
	HANDLE hMetaContact = NULL;

	hContact = hCont;

	NudgeEvent.cbSize = sizeof(NudgeEvent);
	NudgeEvent.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	NudgeEvent.flags = CheckMsgWnd(hContact) ? 0 : DBEF_READ;
	NudgeEvent.timestamp = timestamp;
	NudgeEvent.eventType = EVENTTYPE_MESSAGE;
	#if defined( _UNICODE )
		char buff[TEXT_LEN];
		WideCharToMultiByte(code_page, 0, n.recText, -1, buff, TEXT_LEN, 0, 0);
		buff[TEXT_LEN] = 0;
		NudgeEvent.cbBlob = strlen(buff) + 1;
		NudgeEvent.pBlob = ( PBYTE ) buff;
	#else
		NudgeEvent.cbBlob = _tcsclen(n.recText) + 1;
		NudgeEvent.pBlob = ( PBYTE ) n.recText;
	#endif
	

	if(ServiceExists(MS_MC_GETMETACONTACT)) //try to retrieve the metacontact if some
		hMetaContact = (HANDLE) CallService( MS_MC_GETMETACONTACT, (WPARAM)hContact, 0 );
	
	if(hMetaContact != NULL) //metacontact
	{
		CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&NudgeEvent);
		NudgeEvent.flags = DBEF_READ;
	}
	
	CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&NudgeEvent);
}

int Nudge_AddElement(char *protoName, HANDLE hevent)
{
	nProtocol ++;
	//Add contact menu entry
	CLISTMENUITEM mi;

	memset( &mi, 0, sizeof( mi ));
	mi.popupPosition = 500085000;
	mi.pszContactOwner = protoName;
	mi.pszPopupName = protoName;
	mi.cbSize = sizeof( mi );
	mi.flags = (CMIF_NOTOFFLINE & CMIF_HIDDEN) | CMIF_TCHAR;
	mi.position = -500050004;
	mi.hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_NUDGE ));
	mi.ptszName = LPGENT( "Send &Nudge" );
	mi.pszService = MS_NUDGE_SEND;
	
	
	//Add a specific sound per protocol
	char nudgesoundtemp[ 512 ];
	NudgeElementList *newNudge;
	//newNudge = (NudgeElementList*) malloc(sizeof(NudgeElementList));
	newNudge = new NudgeElementList;
	strcpy( nudgesoundtemp, protoName );
	strcat( nudgesoundtemp, ": " );
	strcat( nudgesoundtemp,  Translate( "Nudge" ));
	strncpy( newNudge->item.NudgeSoundname, nudgesoundtemp, sizeof(newNudge->item.NudgeSoundname) ); 

	strcpy( newNudge->item.ProtocolName, protoName );

	newNudge->item.hContactMenu = (HANDLE) CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi );

	newNudge->item.Load();

	newNudge->item.hEvent = hevent;
	
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

void AutoResendNudge(void *wParam) 
{

	Sleep(GlobalNudge.resendDelaySec * 1000);
	NudgeSend((WPARAM) wParam,NULL);
}
