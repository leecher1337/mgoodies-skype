/*

Jabber Protocol Plugin (GMail mod) for Miranda IM
Copyright ( C ) 2006  Y.B.

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
#include "jabber.h"
#include "sdk/m_icolib.h"
#include <commctrl.h>
#include "resource.h"
extern HINSTANCE hInst;

/*
#define IDI_JABBER                      102
#define IDI_ADDROSTER                   108
#define IDI_USER2ROOM                   109
#define IDI_ADDCONTACT                  122
#define IDI_DELETE                      123
#define IDI_EDIT                        124
#define IDI_OPEN                        131
#define IDI_REQUEST                     141
#define IDI_GRANT                       142
#define IDI_AGENTS                      154
#define IDI_VCARD                       155
#define IDI_SAVE                        166
#define IDI_MAIL_NEW                    186
#define IDI_MAIL_STOP                   187
#define IDI_MAIL_INFO                   188
#define IDI_MAIL_CLOCK                  189
#define IDI_MAIL_GMAIL                  190
*/
static char *iconNames[NUMICONSSMALL]={NULL,"vcard","Agents",
	"Add","Delete","Rename","Request","Grant","Open","Save",
	"mail-new","mail-stop","mail-info","mail-clock","mail-gmail","addroster","convert",
	"trlogonoff"};
static char *iconDescs[NUMICONSSMALL]={NULL,"VCard Menu",iconNames[2],
	iconNames[3],iconNames[4],iconNames[5],iconNames[6],iconNames[7],iconNames[8],iconNames[9],
	"New E-Mail","E-Mail Error","E-Mail Info","E-Mail Clock","Visit GMail","Convert Chat / Contact","Add to roster",
	"Transport Logon/Off"};
static int iconInd[NUMICONSSMALL]={0,155,154,122,123,124,141,142,131,166,186,187,188,189,190,109,108,191};
HICON iconList[NUMICONSSMALL];

static int iconBigInd[NUMICONSBIG]={147,144,IDC_LOGO};
static char *iconBigNames[NUMICONSBIG]={"group","key","write"};
static char *iconBigDescs[NUMICONSBIG]={"Group Chat","Password","VCard"};
HICON iconBigList[NUMICONSBIG];

	extern HANDLE hMenuRequestAuth;
	extern HANDLE hMenuGrantAuth;
	extern HANDLE hMenuJoinLeave;
	extern HANDLE hMenuConvert;
	extern HANDLE hMenuRosterAdd;
	extern HANDLE hMenuLogin;

	extern HANDLE hMenuVisitGMail;

	extern HANDLE hMenuAgent;
	extern HANDLE hMenuChangePassword;
	extern HANDLE hMenuGroupchat;
	extern HANDLE hMenuVCard;

static void IcoLibUpdateMenus(){
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIM_ICON;
	mi.hIcon = iconList[6];// IDI_REQUEST;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuRequestAuth, ( LPARAM )&mi );
	mi.hIcon = iconList[7];// IDI_GRANT;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuGrantAuth, ( LPARAM )&mi );
	mi.hIcon = iconBigList[0];
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuJoinLeave, ( LPARAM )&mi );
	mi.hIcon = iconList[16];// IDI_ADDROSTER;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuRosterAdd, ( LPARAM )&mi );
	mi.hIcon = iconList[15];// IDI_USER2ROOM;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuConvert, ( LPARAM )&mi );
	mi.hIcon = iconList[14];// IDI_MAIL_GMAIL;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuVisitGMail, ( LPARAM )&mi );
	mi.hIcon = iconList[17];// IDI_LOGIN;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuLogin, ( LPARAM )&mi );

	mi.hIcon = iconList[2];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_AGENTS ));
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuAgent, ( LPARAM )&mi );
	mi.hIcon = iconBigList[1];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_KEYS ));
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuChangePassword, ( LPARAM )&mi );
	mi.hIcon = iconBigList[0];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_GROUP ));
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuGroupchat, ( LPARAM )&mi );
	mi.hIcon = iconList[1];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_VCARD ))
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuVCard, ( LPARAM )&mi );
}

int IcoLibIconsChanged(WPARAM wParam, LPARAM lParam)
{
	HICON temp;
	char szTemp[MAX_PATH + 128];
	unsigned int i;
	for (i=0;i<NUMICONSBIG;i++){ //BigIcons
		mir_snprintf(szTemp, sizeof(szTemp), "%s_%s", jabberProtoName, iconBigNames[i]);
		if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) szTemp))iconBigList[i]=temp; 
	}
	for (i=1;i<NUMICONSSMALL;i++){ //BigIcons
		mir_snprintf(szTemp, sizeof(szTemp), "%s_%s", jabberProtoName, iconNames[i]);
		if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) szTemp))iconList[i]=temp; 
	}
/*	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIM_ICON;
	mi.hIcon = iconList[6];// IDI_REQUEST;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuRequestAuth, ( LPARAM )&mi );
	mi.hIcon = iconList[7];// IDI_GRANT;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuGrantAuth, ( LPARAM )&mi );
	mi.hIcon = iconBigList[0];
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuJoinLeave, ( LPARAM )&mi );
	mi.hIcon = iconList[16];// IDI_ADDROSTER;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuRosterAdd, ( LPARAM )&mi );
	mi.hIcon = iconList[15];// IDI_USER2ROOM;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuConvert, ( LPARAM )&mi );
	mi.hIcon = iconList[14];// IDI_MAIL_GMAIL;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuVisitGMail, ( LPARAM )&mi );

	mi.hIcon = iconList[2];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_AGENTS ));
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuAgent, ( LPARAM )&mi );
	mi.hIcon = iconBigList[1];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_KEYS ));
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuChangePassword, ( LPARAM )&mi );
	mi.hIcon = iconBigList[0];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_GROUP ));
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuGroupchat, ( LPARAM )&mi );
	mi.hIcon = iconList[1];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_VCARD ))
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuVCard, ( LPARAM )&mi );
*/	IcoLibUpdateMenus();
	ReloadIconsEventHook(wParam,lParam);
	return 0;
}

void JGmailSetupIcons(){
	HIMAGELIST CSImages = ImageList_Create(32, 32, ILC_COLOR8|ILC_MASK, 0, 3);
	{// workarround of 4bit forced images
		HBITMAP hScrBM   = (HBITMAP)LoadImage(hInst,MAKEINTRESOURCE(IDB_ICONSBIG), IMAGE_BITMAP, 0, 0,LR_SHARED);
		ImageList_AddMasked(CSImages, hScrBM, RGB( 255, 0, 255 ));
		DeleteObject(hScrBM);    
	}
	unsigned int i;
	for (i=0; i<NUMICONSBIG; i++){
		iconBigList[i] = ImageList_ExtractIcon(NULL, CSImages, i);
	}
	ImageList_Destroy(CSImages);
	CSImages = ImageList_Create(16, 16, ILC_COLOR8|ILC_MASK, 0, 14);
	{// workarround of 4bit forced images
		HBITMAP hScrBM   = (HBITMAP)LoadImage(hInst,MAKEINTRESOURCE(IDB_ICONSSMALL), IMAGE_BITMAP, 0, 0,LR_SHARED);
		ImageList_AddMasked(CSImages, hScrBM, RGB( 255, 0, 255 ));
		DeleteObject(hScrBM);    
	}
	for (i=0; i<NUMICONSSMALL; i++){
		iconList[i] = ImageList_ExtractIcon(NULL, CSImages, i);
	}
	ImageList_Destroy(CSImages);
}
extern HANDLE hReloadIcons; // is defined in Jabber.cpp
void JGmailSetupIcoLib(){
	if (ServiceExists(MS_SKIN2_GETICON)){
		HICON temp;
		SKINICONDESC sid = {0};
		unsigned int i;
		char szTemp[MAX_PATH + 128];

        hReloadIcons = HookEvent(ME_SKIN2_ICONSCHANGED, IcoLibIconsChanged);

		sid.cbSize = SKINICONDESC_SIZE_V2;
		sid.pszSection = jabberProtoName;
		sid.pszDefaultFile = NULL;
		sid.iDefaultIndex = 0;
		for (i=0;i<NUMICONSSMALL;i++) if (iconInd[i]){
			sid.pszDescription = iconDescs[i];
			mir_snprintf(szTemp, sizeof(szTemp), "%s_%s", jabberProtoName, iconNames[i]);
			sid.pszName = szTemp;
			sid.iDefaultIndex = -iconInd[i];
			sid.hDefaultIcon = iconList[i];;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) szTemp))iconList[i]=temp; 
		}
		sid.cbSize = SKINICONDESC_SIZE_V3;
		sid.cx=sid.cy=32;
		for (i=0;i<NUMICONSBIG;i++){
			sid.pszDescription = Translate(iconBigDescs[i]);
			mir_snprintf(szTemp, sizeof(szTemp), "%s_%s", jabberProtoName, iconBigNames[i]);
			sid.pszName = szTemp;
			sid.iDefaultIndex = -iconBigInd[i];
			sid.hDefaultIcon = iconBigList[i];
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) szTemp))iconBigList[i]=temp; 
		}
	}
	IcoLibUpdateMenus();
}

