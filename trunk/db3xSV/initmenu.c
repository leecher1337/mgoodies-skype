#include "commonheaders.h"
#include "virtdb.h"
#include "SecureDB.h"

HANDLE hVirtualizeMenu = NULL;
HANDLE hRealizeMenu = NULL;
HANDLE hSaveFileMenu = NULL;

HANDLE hOnLoadHook = NULL;

 void xModifyMenu(HANDLE hMenu,long flags,const char* name)
{
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | ((name)?CMIM_NAME:0);
	mi.flags |= flags;
	mi.pszName = (char*)name;

	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenu,(LPARAM)&mi);
}

 void updateMenus(){
	   xModifyMenu(hVirtualizeMenu,0,isDBvirtual?"Realize":"Virtualize");
	   xModifyMenu(hSaveFileMenu,isDBvirtual?0:CMIF_GRAYED,NULL);
}

static int extraInitMenus(WPARAM wParam, LPARAM lParam)
{
	extern HINSTANCE g_hInst;
	CLISTMENUITEM mi = {0};

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
		mi.hIcon = NULL;
#ifdef SECUREDB
		mi.pszPopupName ="DB extras";
#else
		mi.pszPopupName ="Virtual DB";
#endif

		mi.pszName="Virtualize";
		mi.pszService=MS_DBV_VIRTUALIZE;
		hVirtualizeMenu = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

		mi.pszName="Save DB...";
		mi.pszService=MS_DBV_SAVEFILE;
		hSaveFileMenu = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
		updateMenus();

#ifdef SECUREDB
		mi.hIcon = LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_ICON3));
		mi.pszName=(g_secured)?"Change Password":"Set Password";
		mi.pszService="DB3XS/SetPassword";
		if(!(hSetPwdMenu = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi)))return 1;

		mi.pszName="Remove Password";
		mi.pszService="DB3XS/RemovePassword";
		mi.hIcon = LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_ICON2));
		if(!(hDelPwdMenu = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi)))return 1;

		if(!g_secured){
			xModifyMenu(hDelPwdMenu,CMIF_GRAYED,NULL);
		}
#endif
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

