#include "commonheaders.h"

struct GlobalMessageData *g_dat=NULL;
extern HINSTANCE g_hInst;
static HANDLE g_hDbEvent = 0, g_hAck = 0;
static int dbaddedevent(WPARAM wParam, LPARAM lParam);
static int ackevent(WPARAM wParam, LPARAM lParam);

void CreateProtocolIcons() {
	PROTOCOLDESCRIPTOR **pProtos;
	int i, j, allProtoNum, k;
	HICON hIcon;
	int iCurIcon = 0;
	
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM) &allProtoNum, (LPARAM) &pProtos);
	g_dat->protoNum  = 0;
	for(i = 0; i < allProtoNum; i++) {
        if (pProtos[i]->type == PROTOTYPE_PROTOCOL) g_dat->protoNum++;
	}

	if (g_dat->protoNames != NULL) {
	}
	g_dat->protoNames = (char **) malloc(sizeof(char **) * g_dat->protoNum);
	if (g_dat->hIconList == NULL) {
		g_dat->hIconList = ImageList_Create(16, 16, IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, (g_dat->protoNum + 1) * 12 + 8, 0);
	} else {
		ImageList_RemoveAll(g_dat->hIconList);
	}

    ImageList_AddIcon(g_dat->hIconList, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
    ImageList_AddIcon(g_dat->hIconList, g_dat->hIcons[SMF_ICON_TYPING]);

	for (i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
		ImageList_AddIcon(g_dat->hIconList, LoadSkinnedProtoIcon(NULL, i));
	}

	for(i = j = 0; i < allProtoNum; i++) {
        if (pProtos[i]->type != PROTOTYPE_PROTOCOL) continue;
		//MessageBox(NULL, g_dat->protoNames[j], g_dat->protoNames[j], MB_OK);
		g_dat->protoNames[j] = _strdup(pProtos[i]->szName);
        for (k = ID_STATUS_OFFLINE; k <= ID_STATUS_OUTTOLUNCH; k++) {
            hIcon = LoadSkinnedProtoIcon(pProtos[i]->szName, k);
            if (hIcon != NULL) {
				ImageList_AddIcon(g_dat->hIconList, hIcon);
            } else {
				ImageList_AddIcon(g_dat->hIconList, LoadSkinnedProtoIcon(NULL, ID_STATUS_OFFLINE));
			}
        }
		j++;
    }
}

void InitGlobals() {
	g_dat = (struct GlobalMessageData *)malloc(sizeof(struct GlobalMessageData));
	g_dat->hMessageWindowList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	g_dat->hParentWindowList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	g_hAck = HookEvent(ME_PROTO_ACK, ackevent);
	ReloadGlobals();
	g_dat->hIcons[SMF_ICON_ADD] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ADDCONTACT), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	g_dat->hIcons[SMF_ICON_USERDETAIL] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_USERDETAILS32 : IDI_USERDETAILS), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	g_dat->hIcons[SMF_ICON_HISTORY] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_HISTORY32 : IDI_HISTORY), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	g_dat->hIcons[SMF_ICON_ARROW] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_DOWNARROW32 : IDI_DOWNARROW), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	g_dat->hIcons[SMF_ICON_TYPING] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_TYPING32 : IDI_TYPING), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	g_dat->hIcons[SMF_ICON_SEND] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_SEND : IDI_SEND), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	g_dat->hIcons[SMF_ICON_CANCEL] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_CANCEL : IDI_CANCEL), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	g_dat->hIcons[SMF_ICON_SMILEY] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_SMILEY : IDI_SMILEY), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	g_dat->hParent = NULL;
	g_dat->protoNum = 0;
	g_dat->protoNames = NULL;
	g_dat->hIconList = NULL;
}

void FreeGlobals() {
	int i;

	if (g_dat) {
		for (i=0;i<sizeof(g_dat->hIcons)/sizeof(g_dat->hIcons[0]);i++)
			DestroyIcon(g_dat->hIcons[i]);
		free(g_dat);
	}
	if (g_hDbEvent) UnhookEvent(g_hDbEvent);
	if (g_hAck) UnhookEvent(g_hAck);
}

void ReloadGlobals() {
	g_dat->flags = 0;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWBUTTONLINE, SRMSGDEFSET_SHOWBUTTONLINE))
		g_dat->flags |= SMF_SHOWBTNS;
//	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SENDBUTTON, SRMSGDEFSET_SENDBUTTON))
//		g_dat->flags |= SMF_SENDBTN;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AVATARENABLE, SRMSGDEFSET_AVATARENABLE))
		g_dat->flags |= SMF_AVATAR;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSTATUSBAR, SRMSGDEFSET_SHOWSTATUSBAR))
		g_dat->flags |= SMF_SHOWSTATUSBAR;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USETABS, SRMSGDEFSET_USETABS))
		g_dat->flags |= SMF_USETABS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_TABSATBOTTOM, SRMSGDEFSET_TABSATBOTTOM))
		g_dat->flags |= SMF_TABSATBOTTOM;

	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, SRMSGDEFSET_SHOWLOGICONS))
		g_dat->flags |= SMF_SHOWICONS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTIME, SRMSGDEFSET_SHOWTIME))
		g_dat->flags |= SMF_SHOWTIME;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSECONDS, SRMSGDEFSET_SHOWSECONDS))
		g_dat->flags |= SMF_SHOWSECONDS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWDATE, SRMSGDEFSET_SHOWDATE))
		g_dat->flags |= SMF_SHOWDATE;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USELONGDATE, SRMSGDEFSET_USELONGDATE))
		g_dat->flags |= SMF_LONGDATE;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USERELATIVEDATE, SRMSGDEFSET_USERELATIVEDATE))
		g_dat->flags |= SMF_RELATIVEDATE;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_GROUPMESSAGES, SRMSGDEFSET_GROUPMESSAGES))
		g_dat->flags |= SMF_GROUPMESSAGES;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_MARKFOLLOWUPS, SRMSGDEFSET_MARKFOLLOWUPS))
		g_dat->flags |= SMF_MARKFOLLOWUPS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES))
		g_dat->flags |= SMF_HIDENAMES;

	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING))
		g_dat->flags |= SMF_SHOWTYPING;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN))
		g_dat->flags |= SMF_SHOWTYPINGWIN;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN))
		g_dat->flags |= SMF_SHOWTYPINGTRAY;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST))
		g_dat->flags |= SMF_SHOWTYPINGCLIST;

}

static int ackevent(WPARAM wParam, LPARAM lParam) {
	ACKDATA *pAck = (ACKDATA *)lParam;
	
	if (!pAck) return 0;
	else if (pAck->type==ACKTYPE_AVATAR) {
		HWND h = WindowList_Find(g_dat->hMessageWindowList, (HANDLE)pAck->hContact);
		if(h) SendMessage(h, HM_AVATARACK, wParam, lParam);
	}
	else if (pAck->type==ACKTYPE_MESSAGE) {
		HWND h = WindowList_Find(g_dat->hMessageWindowList, (HANDLE)pAck->hContact);
		if(h) SendMessage(h, HM_EVENTSENT, wParam, lParam);
	}
	return 0;
}
