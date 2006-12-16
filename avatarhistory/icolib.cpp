#include "AvatarHistory.h"
#include <commctrl.h> //for ImageList_*

//#include "m_icolib.h" We don't need the full header. Will define the needed things here.
#define SKINICONDESC_SIZE_V2  0x1C
#define MS_SKIN2_GETICON "Skin2/Icons/GetIcon"
#define MS_SKIN2_ADDICON "Skin2/Icons/AddIcon"
#define ME_SKIN2_ICONSCHANGED "Skin2/IconsChanged"
typedef struct {
  int cbSize;
  char *pszSection;         // section name used to group icons
  char *pszDescription;     // description for options dialog
  char *pszName;              // name to refer to icon when playing and in db
  char *pszDefaultFile;       // default icon file to use
  int  iDefaultIndex;         // index of icon in default file
  HICON hDefaultIcon;         // handle to default icon
  int cx,cy;                  // dimensions of icon
  int flags; 
} SKINICONDESC;
//end of icolib defines

static char *iconNames[NUMICONS]={"History","Overlay"};
static char *iconDescs[NUMICONS]={iconNames[0],"Avatar Overlay"};
static int iconInd[NUMICONS]={IDI_AVATARHIST,IDI_AVATAROVERLAY};
HICON iconList[NUMICONS];
HICON overlayedIcon = NULL;
HICON overlayedBigIcon = NULL;

static void IcoLibUpdateMenus(){
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIM_ICON;
	mi.hIcon = overlayedIcon;
	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenu, ( LPARAM )&mi );
}

int IcoLibIconsChanged(WPARAM wParam, LPARAM lParam)
{
	HICON temp;
	unsigned int i;
	for (i=0;i<NUMICONS;i++){ 
		if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconNames[i]))iconList[i]=temp; 
	}
	if (overlayedIcon) DestroyIcon(overlayedIcon);
	if (overlayedBigIcon) DestroyIcon(overlayedBigIcon);
	overlayedIcon = getOverlayedIcon(iconList[0],iconList[1],FALSE);
	overlayedBigIcon= getOverlayedIcon(iconList[0],iconList[1],TRUE);
	IcoLibUpdateMenus();
	return 0;
}

void SetupIcoLib(){
	iconList[0] = LoadIcon(hInst, MAKEINTRESOURCE(IDI_AVATARHIST));
	iconList[1] = LoadIcon(hInst, MAKEINTRESOURCE(IDI_AVATAROVERLAY));
	if (ServiceExists(MS_SKIN2_GETICON)){
		HICON temp;
		SKINICONDESC sid = {0};
		unsigned int i;

        HookEvent(ME_SKIN2_ICONSCHANGED, IcoLibIconsChanged);

		sid.cbSize = SKINICONDESC_SIZE_V2;
		sid.pszSection = Translate("Avatar History");
		sid.pszDefaultFile = NULL;
		sid.iDefaultIndex = 0;
		for (i=0;i<NUMICONS;i++) if (iconInd[i]){
			sid.pszDescription = iconDescs[i];
			sid.pszName = iconNames[i];
			sid.iDefaultIndex = -iconInd[i];
			sid.hDefaultIcon = iconList[i];;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconNames[i]))iconList[i]=temp; 
		}
	}
	overlayedIcon = getOverlayedIcon(iconList[0],iconList[1],FALSE);
	overlayedBigIcon= getOverlayedIcon(iconList[0],iconList[1],TRUE);
	IcoLibUpdateMenus();
}

HICON getOverlayedIcon(HICON icon, HICON overlay, BOOL big){
	HIMAGELIST il = ImageList_Create(
		GetSystemMetrics(big?SM_CXICON:SM_CXSMICON),
		GetSystemMetrics(big?SM_CYICON:SM_CYSMICON),
		ILC_COLOR32|ILC_MASK, 2, 2);
	ImageList_AddIcon(il, icon);
	ImageList_AddIcon(il, overlay);
	HIMAGELIST newImage = ImageList_Merge(il,0,il,1,0,0);
	ImageList_Destroy(il);
	HICON hIcon = ImageList_GetIcon(newImage, 0, 0);
	ImageList_Destroy(newImage);
	return hIcon; // the result should be destroyed by DestroyIcon()
}