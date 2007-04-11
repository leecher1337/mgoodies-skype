#include "commonheaders.h"
#include "virtdb.h"
#include "SecureDB.h"

HANDLE hVirtualizeMenu = NULL;
HANDLE hSaveFileMenu = NULL;

HANDLE hOnLoadHook = NULL;

void setupIcons();

void xModifyMenu(HANDLE hMenu,long flags,const char* name, HICON icon)
{
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | (name?CMIM_NAME:0) | (icon?CMIM_ICON:0);
	mi.flags |= flags;
	mi.hIcon = icon;
	mi.pszName = (char*)name;

	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenu,(LPARAM)&mi);
}

void updateMenus(){
	xModifyMenu(hVirtualizeMenu,0,isDBvirtual?"Realize":"Virtualize",iconList[isDBvirtual?1:0]);
	xModifyMenu(hSaveFileMenu,isDBvirtual?0:CMIF_GRAYED,NULL,iconList[2]);
#ifdef SECUREDB
	xModifyMenu(hDelPwdMenu,g_secured?0:CMIF_GRAYED,NULL,iconList[4]);
#endif
}

static int extraInitMenus(WPARAM wParam, LPARAM lParam)
{
	extern HINSTANCE g_hInst;
	CLISTMENUITEM mi = {0};
	//Load the icons. Later will change the menus if IcoLib is available
	setupIcons();

#ifdef SECUREDB
	CreateServiceFunction("DB3XS/SetPassword",DB3XSSetPassword);
	CreateServiceFunction("DB3XS/RemovePassword",DB3XSRemovePassword);
#endif
	CreateServiceFunction(MS_DBV_VIRTUALIZE,virtualizeService);
	CreateServiceFunction(MS_DBV_SAVEFILE,saveFileService);
	if (!disableMenu){
		mi.cbSize = sizeof(mi);
		mi.position = 0xFFffFFff;
		mi.flags = 0;
		mi.hIcon = mainIcon; // this one clist_nicer will put in the main menu
		mi.pszPopupName =MENUNAME;
		mi.pszName="Virtualize";
		mi.pszService=MS_DBV_VIRTUALIZE;
		hVirtualizeMenu = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

		mi.pszName="Save DB...";
		mi.pszService=MS_DBV_SAVEFILE;
		hSaveFileMenu = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

#ifdef SECUREDB
		mi.hIcon = iconList[3];//LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_ICON3));
		mi.pszName=(g_secured)?"Change Password":"Set Password";
		mi.pszService="DB3XS/SetPassword";
		if(!(hSetPwdMenu = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi)))return 1;

		mi.pszName="Remove Password";
		mi.pszService="DB3XS/RemovePassword";
		mi.hIcon = iconList[4];//LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_ICON2));
		if(!(hDelPwdMenu = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi)))return 1;
#endif
		updateMenus();
	}
	UnhookEvent(hOnLoadHook);
	hOnLoadHook = NULL;
	return 0;
}


 int extraOnLoad()
{
	hOnLoadHook = HookEvent(ME_SYSTEM_MODULESLOADED,extraInitMenus);
	hOnExitHook = HookEvent(ME_SYSTEM_SHUTDOWN,realizeDBonExit);
#ifdef SECUREDB
	CreateServiceFunction("DB3XS/MakeBackup",DB3XSMakeBackup);
#endif
	return hOnLoadHook == NULL;
}

