#include "seen.h"

HANDLE hmenuitem=NULL;

void ShowHistory(HANDLE hContact, BYTE isAlert);
void InitHistoryDialog(void);

/*
Handles the messages sent by clicking the contact's menu item
*/
int MenuitemClicked(WPARAM wparam,LPARAM lparam)
{
	ShowHistory((HANDLE)wparam, 0);
	return 0;
}



int BuildContactMenu(WPARAM wparam,LPARAM lparam)
{
	CLISTMENUITEM cmi;
	DBVARIANT dbv;
	int id=-1,isetting;
	HANDLE hContact;
	char *szProto;

	hContact = (HANDLE)wparam;
	szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);


	ZeroMemory(&cmi,sizeof(cmi));
	cmi.cbSize=sizeof(cmi);
	if(!IsWatchedProtocol(szProto) || !DBGetContactSettingByte(NULL,S_MOD,"MenuItem",1))
	{
		cmi.flags=CMIM_FLAGS|CMIF_HIDDEN;
	}
	else
	{
		cmi.flags=CMIM_NAME|CMIM_FLAGS|CMIM_ICON;
		cmi.hIcon=NULL;
		cmi.pszName=ParseString(!DBGetContactSetting(NULL,S_MOD,"MenuStamp",&dbv)?dbv.pszVal:DEFAULT_MENUSTAMP,(HANDLE)wparam,0);
		
		if(!strcmp(cmi.pszName,Translate("<unknown>")))
		{	
			if (IsWatchedProtocol(szProto))
				cmi.flags|=CMIF_GRAYED;
			else
				cmi.flags|=CMIF_HIDDEN;	
		} 
		else if(DBGetContactSettingByte(NULL,S_MOD,"ShowIcon",1))
		{
			isetting=DBGetContactSettingWord((HANDLE)hContact,S_MOD,"Status",-1);
			cmi.hIcon=LoadSkinnedProtoIcon(szProto,isetting);
			
		}
	}

	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuitem,(LPARAM)&cmi);
	DBFreeVariant(&dbv);


	return 0;
}



void InitMenuitem(void)
{
	CLISTMENUITEM cmi;

	CreateServiceFunction("LastSeenUserDetails",MenuitemClicked);

	ZeroMemory(&cmi,sizeof(cmi));
	cmi.cbSize=sizeof(cmi);
	cmi.flags=0;
	cmi.hIcon=NULL;
	cmi.hotKey=0;
	cmi.position=-0x7FFFFFFF;
	cmi.pszContactOwner=NULL;
	cmi.pszName="<none>";
	cmi.pszService="LastSeenUserDetails";
	
	hmenuitem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cmi);
	
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU,BuildContactMenu);

	InitHistoryDialog();
}
