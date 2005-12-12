
#include "main.h"
#include "yamn.h"
// External icon var for icolib support
extern HICON hYamnIcon;
extern HICON hNeutralIcon;
extern HICON hNewMailIcon;
extern HICON hConnectFailIcon;
extern HICON hTopToolBarUp;
extern HICON hTopToolBarDown;

extern char *ProtoName;
extern int YAMN_STATUS;

extern PYAMN_VARIABLES pYAMNVar;
extern HYAMNPROTOPLUGIN POP3Plugin;

static int Service_GetCaps(WPARAM wParam, LPARAM lParam)
{
	/*if(wParam==PFLAGNUM_2)
		return PF2_ONLINE;*/
	if(wParam==PFLAGNUM_4)
		return PF4_NOCUSTOMAUTH;
	if(wParam==PFLAG_UNIQUEIDTEXT)
        return (int) Translate("Nick");
	if(wParam==PFLAG_MAXLENOFMESSAGE)
        return 400;
	if(wParam==PFLAG_UNIQUEIDSETTING)
        return (int) "Id";
	return 0;
}

static int Service_GetStatus(WPARAM wParam, LPARAM lParam)
{
	return YAMN_STATUS;	
}

static int Service_SetStatus(WPARAM wParam,LPARAM lParam)
{	
	switch(wParam)
	{
	case ID_STATUS_ONLINE:
		YAMN_STATUS = ID_STATUS_ONLINE;
		RefreshContact();
		break;
	case ID_STATUS_OFFLINE:
		YAMN_STATUS = ID_STATUS_OFFLINE;
		RefreshContact();
		break;
	default:
		break;
	}

	char t[150];
	sprintf(t,"%i",wParam);
	//MessageBox(NULL,t,"Test",0);
	return 0;

}
static int Service_GetName(WPARAM wParam, LPARAM lParam)
{
	lstrcpyn((char *) lParam, ProtoName, wParam);;
	return 0;
}
static int Service_LoadIcon(WPARAM wParam,LPARAM lParam)
{

	switch(wParam&0xFFFF) 
	{
		case PLI_PROTOCOL: 
			return (int)(HICON)hNeutralIcon;
		default:	
			return (int)(HICON)hNeutralIcon;	
	}
	return 0;
}

static int Service_ContactDoubleclicked(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	char *szProto;
	WPARAM mwParam;

	szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if(szProto != NULL && strcmp(szProto, ProtoName)==0)
	{
		if(!DBGetContactSetting((HANDLE) wParam,ProtoName,"Id",&dbv)) 
		{
			mwParam = CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)dbv.pszVal);
			if(mwParam != NULL)
			{
				YAMN_MAILBROWSERPARAM Param={(HANDLE)0,(CAccount*)mwParam,YAMN_ACC_MSGP,YAMN_ACC_MSGP,NULL};

				Param.nnflags=Param.nnflags | (YAMN_ACC_POP | YAMN_ACC_MSGP);
				CallService(MS_YAMN_MAILBROWSER,(WPARAM)&Param,(LPARAM)YAMN_MAILBROWSERVERSION);

			}
			DBFreeVariant(&dbv);
		}

	}

	return 0;
}


static int IcoLibIconsChanged(WPARAM wParam, LPARAM lParam)
{
	hNeutralIcon= (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "YAMN_Neutral");
	hYamnIcon= (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "YAMN_Yamn");
	hNewMailIcon= (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "YAMN_NewMail");
	hConnectFailIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "YAMN_ConnectFail");
	hTopToolBarUp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "YAMN_TopToolBarUp");
	hTopToolBarDown = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "YAMN_TopToolBarDown");
	return 0;
}

void HookEvents(void)
{
	HookEvent(ME_OPT_INITIALISE,YAMNOptInitSvc);

	HookEvent(ME_PLUGINUNINSTALLER_UNINSTALL,UninstallQuestionSvc);

	HookEvent(ME_SYSTEM_PRESHUTDOWN,Shutdown);

	HookEvent(ME_CLIST_DOUBLECLICKED, Service_ContactDoubleclicked);

		//Check if icolib is there
	if(ServiceExists(MS_SKIN2_ADDICON))
        HookEvent(ME_SKIN2_ICONSCHANGED, IcoLibIconsChanged);
}

void CreateServiceFunctions(void)
{
	//Create function for plugins
	char temp[MAXMODULELABELLENGTH];

	wsprintf(temp, "%s%s", ProtoName, PS_GETCAPS);
	CreateServiceFunction(temp,Service_GetCaps);
	
	wsprintf(temp, "%s%s", ProtoName, PS_GETSTATUS);
	CreateServiceFunction(temp,Service_GetStatus);

	wsprintf(temp, "%s%s", ProtoName, PS_SETSTATUS);
	//CreateServiceFunction(temp,Service_SetStatus);
	
	wsprintf(temp, "%s%s", ProtoName, PS_GETNAME);
	CreateServiceFunction(temp,Service_GetName);
	
	wsprintf(temp, "%s%s", ProtoName, PS_LOADICON);
	CreateServiceFunction(temp,Service_LoadIcon);
	
	//Function with which protocol plugin can register
	CreateServiceFunction(MS_YAMN_GETFCNPTR,GetFcnPtrSvc);
	
	//Function returns pointer to YAMN variables
	CreateServiceFunction(MS_YAMN_GETVARIABLES,GetVariablesSvc);
	
	//Function with which protocol plugin can register
	CreateServiceFunction(MS_YAMN_REGISTERPROTOPLUGIN,RegisterProtocolPluginSvc);
	
	//Function with which protocol plugin can unregister
	CreateServiceFunction(MS_YAMN_UNREGISTERPROTOPLUGIN,UnregisterProtocolPluginSvc);
	
	//Function creates an account for plugin
	CreateServiceFunction(MS_YAMN_CREATEPLUGINACCOUNT,CreatePluginAccountSvc);
	
	//Function deletes plugin account 
	CreateServiceFunction(MS_YAMN_DELETEPLUGINACCOUNT,DeletePluginAccountSvc);
	
	//Finds account for plugin by name
	CreateServiceFunction(MS_YAMN_FINDACCOUNTBYNAME,FindAccountByNameSvc);
	
	//Creates next account for plugin
	CreateServiceFunction(MS_YAMN_GETNEXTFREEACCOUNT,GetNextFreeAccountSvc);
	
	//Function removes account from YAMN queue. Does not delete it from memory
	CreateServiceFunction(MS_YAMN_DELETEACCOUNT,DeleteAccountSvc);
	
	//Function finds accounts for specified plugin
	CreateServiceFunction(MS_YAMN_READACCOUNTSA,AddAccountsFromFileASvc);
	
	//Function that reads all plugin mails from file
	CreateServiceFunction(MS_YAMN_READACCOUNTSW,AddAccountsFromFileWSvc);
	
	//Function that stores all plugin mails to one file 
	CreateServiceFunction(MS_YAMN_WRITEACCOUNTSA,WriteAccountsToFileASvc);
	
	//Function that stores all plugin mails to one file 
	CreateServiceFunction(MS_YAMN_WRITEACCOUNTSW,WriteAccountsToFileWSvc);
	
	//Function that returns user's filename
	CreateServiceFunction(MS_YAMN_GETFILENAMEA,GetFileNameASvc);
	
	//Function that returns user's filename (unicode input)
	CreateServiceFunction(MS_YAMN_GETFILENAMEW,GetFileNameWSvc);
	
	//Releases unicode string from memory
	CreateServiceFunction(MS_YAMN_DELETEFILENAME,DeleteFileNameSvc);
	
	//Checks mail
	CreateServiceFunction(MS_YAMN_FORCECHECK,ForceCheckSvc);
	
	//Runs YAMN's mail browser
	CreateServiceFunction(MS_YAMN_MAILBROWSER,RunMailBrowserSvc);
	
	//Runs YAMN's bad conenction window
	CreateServiceFunction(MS_YAMN_BADCONNECTION,RunBadConnectionSvc);
	
	//Function creates new mail for plugin
	CreateServiceFunction(MS_YAMN_CREATEACCOUNTMAIL,CreateAccountMailSvc);
	
	//Function deletes plugin account 
	CreateServiceFunction(MS_YAMN_DELETEACCOUNTMAIL,DeleteAccountMailSvc);
	
	//Function with which filter plugin can register
	CreateServiceFunction(MS_YAMN_REGISTERFILTERPLUGIN,RegisterFilterPluginSvc);
	
	//Function with which filter plugin can unregister
	CreateServiceFunction(MS_YAMN_UNREGISTERFILTERPLUGIN,UnregisterFilterPluginSvc);
	
	//Function filters mail
	CreateServiceFunction(MS_YAMN_FILTERMAIL,FilterMailSvc);

	return;
}

//Function to put all enabled contact with the current status
void RefreshContact(void)
{
	HACCOUNT Finder;

	for(Finder=POP3Plugin->FirstAccount;Finder!=NULL;Finder=Finder->Next)
	{
		if(Finder->Contact != NULL)
		{
			if((Finder->Flags & YAMN_ACC_ENA) && (Finder->NewMailN.Flags & YAMN_ACC_CONT))
			{
				DBWriteContactSettingWord(Finder->Contact, ProtoName, "Status", YAMN_STATUS);
				DBWriteContactSettingString(Finder->Contact, "CList", "StatusMsg", Translate("No new mail"));
			}
			else
			{
				CallService(MS_DB_CONTACT_DELETE,(WPARAM)(HANDLE) Finder->Contact, 0);
				Finder->Contact = NULL;
			}
		}
		else
		{
			if((Finder->Flags & YAMN_ACC_ENA) && (Finder->NewMailN.Flags & YAMN_ACC_CONT))
			{
				Finder->Contact =(HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0);
				CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)Finder->Contact,(LPARAM)ProtoName);
				DBWriteContactSettingString(Finder->Contact,ProtoName,"Id",Finder->Name);
				DBWriteContactSettingString(Finder->Contact,ProtoName,"Nick",Finder->Name);
				DBWriteContactSettingString(Finder->Contact,"Protocol","p",ProtoName);
				DBWriteContactSettingWord(Finder->Contact, ProtoName, "Status", YAMN_STATUS);
				DBWriteContactSettingString(Finder->Contact, "CList", "StatusMsg", Translate("No new mail"));
			}

		}
	}

}
