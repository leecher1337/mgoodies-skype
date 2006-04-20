#include "jabber.h"
#include "sdk/m_icolib.h"
#include <commctrl.h>
#include "resource.h"
extern HINSTANCE hInst;

/*
#define IDI_JABBER                      102
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
*/
#define NUMICONSSMALL 14
#define NUMICONSBIG 3
static char *iconNames[NUMICONSSMALL]={NULL,"vcard","Agents",
	"Add","Delete","Rename","Request","Grant","Open","Save",
	"mail-new","mail-stop","mail-info","mail-clock"};
static char *iconDescs[NUMICONSSMALL]={NULL,"VCard Menu",iconNames[2],
	iconNames[3],iconNames[4],iconNames[5],iconNames[6],iconNames[7],iconNames[8],iconNames[9],
	"New E-Mail","E-Mail Error","E-Mail Info","E-Mail Clock"};
static int iconInd[NUMICONSSMALL]={0,155,154,122,123,124,141,142,131,166,186,187,188,189};
HICON iconList[NUMICONSSMALL];

static int iconBigInd[NUMICONSBIG]={147,144,IDC_LOGO};
static char *iconBigNames[NUMICONSBIG]={"group","key","write"};
static char *iconBigDescs[NUMICONSBIG]={"Group Chat","Password","VCard"};
HICON iconBigList[NUMICONSBIG];

	extern HANDLE hMenuRequestAuth;
	extern HANDLE hMenuGrantAuth;
	extern HANDLE hMenuJoinLeave;

	extern HANDLE hMenuAgent;
	extern HANDLE hMenuChangePassword;
	extern HANDLE hMenuGroupchat;
	extern HANDLE hMenuVCard;

void IcoLibUpdateMenus(){
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIM_ICON;
	mi.hIcon = iconList[6];// IDI_REQUEST;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuRequestAuth, ( LPARAM )&mi );
	mi.hIcon = iconList[7];// IDI_GRANT;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuGrantAuth, ( LPARAM )&mi );
	mi.hIcon = iconBigList[0];
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuJoinLeave, ( LPARAM )&mi );

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

	for (int i=0;i<3;i++){ //BigIcons
		mir_snprintf(szTemp, sizeof(szTemp), "%s_%s", jabberProtoName, iconBigNames[i]);
		if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) szTemp))iconBigList[i]=temp; 
	}
	for (i=1;i<14;i++){ //BigIcons
		mir_snprintf(szTemp, sizeof(szTemp), "%s_%s", jabberProtoName, iconNames[i]);
		if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) szTemp))iconList[i]=temp; 
	}
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIM_ICON;
	mi.hIcon = iconList[6];// IDI_REQUEST;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuRequestAuth, ( LPARAM )&mi );
	mi.hIcon = iconList[7];// IDI_GRANT;
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuGrantAuth, ( LPARAM )&mi );
	mi.hIcon = iconBigList[0];
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuJoinLeave, ( LPARAM )&mi );

	mi.hIcon = iconList[2];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_AGENTS ));
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuAgent, ( LPARAM )&mi );
	mi.hIcon = iconBigList[1];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_KEYS ));
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuChangePassword, ( LPARAM )&mi );
	mi.hIcon = iconBigList[0];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_GROUP ));
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuGroupchat, ( LPARAM )&mi );
	mi.hIcon = iconList[1];//LoadIcon( hInst, MAKEINTRESOURCE( IDI_VCARD ))
	JCallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuVCard, ( LPARAM )&mi );
	return 0;
}

void JGmailSetupIcons(){
	HIMAGELIST CSImages = ImageList_Create(32, 32, ILC_COLOR8|ILC_MASK, 0, 3);
	{// workarround of 4bit forced images
		HBITMAP hScrBM   = (HBITMAP)LoadImage(hInst,MAKEINTRESOURCE(IDB_ICONSBIG), IMAGE_BITMAP, 0, 0,LR_SHARED);
		ImageList_AddMasked(CSImages, hScrBM, RGB( 255, 0, 255 ));
		DeleteObject(hScrBM);    
	}
	for (int i=0; i<3; i++){
		iconBigList[i] = ImageList_ExtractIcon(NULL, CSImages, i);
	}
	ImageList_Destroy(CSImages);
	CSImages = ImageList_Create(16, 16, ILC_COLOR8|ILC_MASK, 0, 14);
	{// workarround of 4bit forced images
		HBITMAP hScrBM   = (HBITMAP)LoadImage(hInst,MAKEINTRESOURCE(IDB_ICONSSMALL), IMAGE_BITMAP, 0, 0,LR_SHARED);
		ImageList_AddMasked(CSImages, hScrBM, RGB( 255, 0, 255 ));
		DeleteObject(hScrBM);    
	}
	for (i=0; i<14; i++){
		iconList[i] = ImageList_ExtractIcon(NULL, CSImages, i);
	}
	ImageList_Destroy(CSImages);
}
void JGmailSetupIcoLib(){
	if (ServiceExists(MS_SKIN2_GETICON)){
		HICON temp;
		SKINICONDESC sid = {0};
		char szTemp[MAX_PATH + 128];

        HookEvent(ME_SKIN2_ICONSCHANGED, IcoLibIconsChanged);

		sid.cbSize = SKINICONDESC_SIZE_V2;
		sid.pszSection = jabberProtoName;
		sid.pszDefaultFile = NULL;
		sid.iDefaultIndex = 0;
		for (int i=0;i<14;i++) if (iconInd[i]){
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
		for (i=0;i<3;i++){
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

