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
			isetting=DBGetContactSettingWord((HANDLE)hContact,S_MOD,"StatusTriger",-1);
			cmi.hIcon=LoadSkinnedProtoIcon(szProto,isetting|0x8000);
			
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
