#include "commonheaders.h"
#include "virtdb.h"
#ifdef SECUREDB
	#include "SecureDB.h"
#endif

#include "../../protocols/IcqOscarJ/m_icolib.h"

extern HINSTANCE g_hInst;
static char *iconNames[NUMICONS]={"DB_Virt","DB_Real","DB_Save"
#ifdef SECUREDB
	,"DB_SetPw", "DB_DelPw"
#endif
};
static char *iconDescs[NUMICONS]={"Virtualize","Realize","Save DB..."
#ifdef SECUREDB
	,"Set Password", "Remove Password"
#endif
};
HICON iconList[NUMICONS];
HICON mainIcon;
void xModifyMenu(HANDLE hMenu,long flags,const char* name, HICON icon);
extern HANDLE hVirtualizeMenu;
extern HANDLE hSaveFileMenu;

int IcoLibIconsChanged(WPARAM wParam, LPARAM lParam)
{
	HICON temp;
	unsigned int i;
	for (i=0;i<NUMICONS;i++){
		if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconNames[i]))iconList[i]=temp; 
	}
	if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "DB_Main"))mainIcon=temp; 
	xModifyMenu(hVirtualizeMenu,0,NULL,iconList[isDBvirtual?1:0]);
	xModifyMenu(hSaveFileMenu,isDBvirtual?0:CMIF_GRAYED,NULL,iconList[2]);
#ifdef SECUREDB
	xModifyMenu(hDelPwdMenu,g_secured?0:CMIF_GRAYED,NULL,iconList[4]);
	xModifyMenu(hSetPwdMenu,0,NULL,iconList[3]);
#endif
	return 0;
}


void setupIcons(){
	unsigned int i;
	HIMAGELIST CSImages = ImageList_Create(16, 16, ILC_COLOR8|ILC_MASK, 0, NUMICONS);
	{// workarround of 4bit forced images
		HBITMAP hScrBM   = (HBITMAP)LoadImage(g_hInst,MAKEINTRESOURCE(IDB_ICONS), IMAGE_BITMAP, 0, 0,LR_SHARED);
		ImageList_AddMasked(CSImages, hScrBM, RGB( 255, 0, 255 ));
		DeleteObject(hScrBM);    
	}
	for (i=0; i<NUMICONS; i++){
		iconList[i] = ImageList_ExtractIcon(NULL, CSImages, i);
	}
	ImageList_Destroy(CSImages);
	mainIcon = LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_ICON1));
	if (ServiceExists(MS_SKIN2_GETICON)){
		HICON temp;
		SKINICONDESC sid = {0};
		unsigned int i;

        HookEvent(ME_SKIN2_ICONSCHANGED, IcoLibIconsChanged);

		sid.cbSize = SKINICONDESC_SIZE_V2;
		sid.pszSection =MENUNAME;
		sid.pszDefaultFile = NULL;
		sid.iDefaultIndex = 0;
		for (i=0;i<NUMICONS;i++) {
			sid.pszDescription = iconDescs[i];
			sid.pszName = iconNames[i];
			sid.iDefaultIndex = -((int)(IDI_ICONV1+i));
			sid.hDefaultIcon = iconList[i];;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconNames[i]))iconList[i]=temp; 
		}
		sid.pszDescription = MENUNAME;
		sid.pszName = "DB_Main";
		sid.iDefaultIndex = -IDI_ICON1;
		sid.hDefaultIcon = mainIcon;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "DB_Main"))mainIcon=temp; 
	}
}

