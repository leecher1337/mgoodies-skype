#include "commonheaders.h"

struct GlobalMessageData *g_dat=NULL;
extern HINSTANCE g_hInst;
static HANDLE g_hDbEvent = 0, g_hAck = 0;
static int dbaddedevent(WPARAM wParam, LPARAM lParam);
static int ackevent(WPARAM wParam, LPARAM lParam);

void LoadProtocolIcons() {
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

void SetIcoLibIcons() {
	if (ServiceExists(MS_SKIN2_ADDICON)) {
		SKINICONDESC sid;
		char path[MAX_PATH];
		GetModuleFileNameA(g_hInst, path, MAX_PATH);
		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszSection = "Scriver";
		sid.pszDefaultFile = path;
		sid.pszName = (char *) "scriver_ADD";
		sid.iDefaultIndex = 0; //IDI_ADDCONTACT;
		sid.pszDescription = Translate("Add contact");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_USERDETAILS";
		sid.iDefaultIndex = 1;//IDI_USERDETAILS;
		sid.pszDescription = Translate("User's details");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_HISTORY";
		sid.iDefaultIndex = 2;//IDI_USERDETAILS;
		sid.pszDescription = Translate("User's history");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_SEND";
		sid.iDefaultIndex = 3;//IDI_USERDETAILS;
		sid.pszDescription = Translate("Send message");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_CANCEL";
		sid.iDefaultIndex = 4;//IDI_CANCEL;
		sid.pszDescription = Translate("Close session");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_SMILEY";
		sid.iDefaultIndex = 5;//IDI_SMILEY;
		sid.pszDescription = Translate("Smiley button");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_TYPING";
		sid.iDefaultIndex = 6;//IDI_TYPING;
		sid.pszDescription = Translate("User is typing");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszName = (char *) "scriver_UNICODEON";
		sid.iDefaultIndex = 7;//IDI_UNICODEON;
		sid.pszDescription = Translate("Unicode is on");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_UNICODEOFF";
		sid.iDefaultIndex = 8;//IDI_UNICODEOFF;
		sid.pszDescription = Translate("Unicode is off");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszName = (char *) "scriver_DELIVERING";
		sid.iDefaultIndex = 9;//IDI_UNICODEOFF;
		sid.pszDescription = Translate("Sending");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszName = (char *) "scriver_INCOMING";
		sid.iDefaultIndex = 7;//IDI_INCOMING;
		sid.pszDescription = Translate("Incoming message");
//		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_OUTGOING";
		sid.iDefaultIndex = 8;//IDI_OUTGOING;
		sid.pszDescription = Translate("Outgoing message");
//		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_NOTICE";
		sid.iDefaultIndex = 9;//IDI_NOTICE;
		sid.pszDescription = Translate("Notice");
//		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}
}

void LoadGlobalIcons() {

	if (ServiceExists(MS_SKIN2_ADDICON)) {
		g_dat->hIcons[SMF_ICON_ADD] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_ADD");
		g_dat->hIcons[SMF_ICON_USERDETAILS] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_USERDETAILS");
		g_dat->hIcons[SMF_ICON_HISTORY] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_HISTORY");
		g_dat->hIcons[SMF_ICON_TYPING] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_TYPING");
		g_dat->hIcons[SMF_ICON_SEND] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_SEND");
		g_dat->hIcons[SMF_ICON_CANCEL] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_CANCEL");
		g_dat->hIcons[SMF_ICON_SMILEY] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_SMILEY");
//		g_dat->hIcons[SMF_ICON_INCOMING] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_INCOMING");
//		g_dat->hIcons[SMF_ICON_OUTGOING] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_OUTGOING");
//		g_dat->hIcons[SMF_ICON_NOTICE] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_NOTICE");
		g_dat->hIcons[SMF_ICON_UNICODEON] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_UNICODEON");
		g_dat->hIcons[SMF_ICON_UNICODEOFF] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_UNICODEOFF");
		g_dat->hIcons[SMF_ICON_DELIVERING] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_DELIVERING");

		g_dat->hIcons[SMF_ICON_INCOMING] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_INCOMING));
		g_dat->hIcons[SMF_ICON_OUTGOING] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_OUTGOING));
		g_dat->hIcons[SMF_ICON_NOTICE] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NOTICE));
	} else {
		g_dat->hIcons[SMF_ICON_ADD] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ADDCONTACT), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_USERDETAILS] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_USERDETAILS32 : IDI_USERDETAILS), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_HISTORY] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_HISTORY32 : IDI_HISTORY), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_SEND] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_SEND : IDI_SEND), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_CANCEL] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_CANCEL : IDI_CANCEL), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_SMILEY] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_SMILEY : IDI_SMILEY), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_TYPING] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_TYPING32 : IDI_TYPING), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_UNICODEON] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_UNICODEON : IDI_UNICODEON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_UNICODEOFF] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_UNICODEOFF : IDI_UNICODEOFF), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_DELIVERING] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_TIMESTAMP : IDI_TIMESTAMP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

		g_dat->hIcons[SMF_ICON_INCOMING] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_INCOMING));
		g_dat->hIcons[SMF_ICON_OUTGOING] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_OUTGOING));
		g_dat->hIcons[SMF_ICON_NOTICE] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NOTICE));
	}
}

void InitGlobals() {
	g_dat = (struct GlobalMessageData *)malloc(sizeof(struct GlobalMessageData));
	g_dat->hMessageWindowList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	g_dat->hParentWindowList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	g_hAck = HookEvent(ME_PROTO_ACK, ackevent);
	ReloadGlobals();
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
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTITLEBAR, SRMSGDEFSET_SHOWTITLEBAR))
		g_dat->flags |= SMF_SHOWTITLEBAR;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWPROGRESS, SRMSGDEFSET_SHOWPROGRESS))
		g_dat->flags |= SMF_SHOWPROGRESS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USETABS, SRMSGDEFSET_USETABS))
		g_dat->flags |= SMF_USETABS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_TABSATBOTTOM, SRMSGDEFSET_TABSATBOTTOM))
		g_dat->flags |= SMF_TABSATBOTTOM;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_LIMITNAMES, SRMSGDEFSET_LIMITNAMES))
		g_dat->flags |= SMF_LIMITNAMES;

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
