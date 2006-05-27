/*
Scriver

Copyright 2000-2005 Miranda ICQ/IM project, 
Copyright 2005 Piotr Piastucki

all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "commonheaders.h"
#include "m_ieview.h"

struct GlobalMessageData *g_dat=NULL;
extern HINSTANCE g_hInst;
extern PSLWA pSetLayeredWindowAttributes;
extern HANDLE *hMsgMenuItem;
extern int hMsgMenuItemCount;

static HANDLE g_hAck = 0;
static int ackevent(WPARAM wParam, LPARAM lParam);


void LoadProtocolIcons() {
	PROTOCOLDESCRIPTOR **pProtos;
	int i, j, allProtoNum, k;
	HICON hIcon;

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM) &allProtoNum, (LPARAM) &pProtos);
	g_dat->protoNum  = 0;
	for(i = 0; i < allProtoNum; i++) {
        if (pProtos[i]->type == PROTOTYPE_PROTOCOL) g_dat->protoNum++;
	}

	if (g_dat->protoNames != NULL) {
		for(i = 0; i < g_dat->protoNum; i++) {
			if (g_dat->protoNames[i] != NULL) {
				free(g_dat->protoNames[i]);
			}
		}
		free(g_dat->protoNames);
	}
	g_dat->protoNames = (char **) malloc(sizeof(char **) * g_dat->protoNum);
	if (g_dat->hTabIconList == NULL) {
		g_dat->hTabIconList = ImageList_Create(16, 16, IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, (g_dat->protoNum + 1) * 12 + 8, 0);
	} else {
		ImageList_RemoveAll(g_dat->hTabIconList);
	}

    ImageList_AddIcon(g_dat->hTabIconList, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
	ImageList_AddIcon(g_dat->hTabIconList, g_dat->hIcons[SMF_ICON_TYPING]);
	for (i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
		ImageList_AddIcon(g_dat->hTabIconList, LoadSkinnedProtoIcon(NULL, i));
	}

	for(i = j = 0; i < allProtoNum; i++) {
        if (pProtos[i]->type != PROTOTYPE_PROTOCOL) continue;
		g_dat->protoNames[j] = _strdup(pProtos[i]->szName);
        for (k = ID_STATUS_OFFLINE; k <= ID_STATUS_OUTTOLUNCH; k++) {
            hIcon = LoadSkinnedProtoIcon(pProtos[i]->szName, k);
            if (hIcon != NULL) {
				ImageList_AddIcon(g_dat->hTabIconList, hIcon);
            } else {
				ImageList_AddIcon(g_dat->hTabIconList, LoadSkinnedProtoIcon(NULL, ID_STATUS_OFFLINE));
			}
        }
		j++;
    }
}

int IconsChanged(WPARAM wParam, LPARAM lParam)
{
	if (hMsgMenuItem) {
		int j;
		CLISTMENUITEM mi;
		mi.cbSize = sizeof(mi);
		mi.flags = CMIM_ICON;
		mi.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		for (j = 0; j < hMsgMenuItemCount; j++) {
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hMsgMenuItem[j], (LPARAM) & mi);
		}
	}
	FreeMsgLogIcons();
	LoadMsgLogIcons();
	LoadProtocolIcons();
	WindowList_Broadcast(g_dat->hMessageWindowList, DM_REMAKELOG, 0, 0);
	// change all the icons
	WindowList_Broadcast(g_dat->hMessageWindowList, DM_CHANGEICONS, 0, 0);
	WindowList_Broadcast(g_dat->hMessageWindowList, DM_UPDATETITLE, 0, 0);
	return 0;
}

int SmileySettingsChanged(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(g_dat->hMessageWindowList, DM_REMAKELOG, wParam, 0);
	return 0;
}

int IcoLibIconsChanged(WPARAM wParam, LPARAM lParam) 
{
	LoadGlobalIcons();
	return IconsChanged(wParam, lParam);
}

void RegisterIcoLibIcons() {
	if (ServiceExists(MS_SKIN2_ADDICON)) {
		SKINICONDESC sid;
		char path[MAX_PATH];
		GetModuleFileNameA(g_hInst, path, MAX_PATH);
		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszSection = "Scriver";
		sid.pszDefaultFile = path;
		sid.pszName = (char *) "scriver_ADD";
		sid.iDefaultIndex = -IDI_ADDCONTACT;
		sid.pszDescription = Translate("Add contact");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_USERDETAILS";
		sid.iDefaultIndex = -IDI_USERDETAILS;
		sid.pszDescription = Translate("User's details");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_HISTORY";
		sid.iDefaultIndex = -IDI_HISTORY;
		sid.pszDescription = Translate("User's history");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_SEND";
		sid.iDefaultIndex = -IDI_SEND;
		sid.pszDescription = Translate("Send message");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_CANCEL";
		sid.iDefaultIndex = -IDI_CANCEL;
		sid.pszDescription = Translate("Close session");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_SMILEY";
		sid.iDefaultIndex = -IDI_SMILEY;
		sid.pszDescription = Translate("Smiley button");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_TYPING";
		sid.iDefaultIndex = -IDI_TYPING;
		sid.pszDescription = Translate("User is typing");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszName = (char *) "scriver_UNICODEON";
		sid.iDefaultIndex = -IDI_UNICODEON;
		sid.pszDescription = Translate("Unicode is on");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		sid.pszName = (char *) "scriver_UNICODEOFF";
		sid.iDefaultIndex = -IDI_UNICODEOFF;
		sid.pszDescription = Translate("Unicode is off");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszName = (char *) "scriver_DELIVERING";
		sid.iDefaultIndex = -IDI_TIMESTAMP;
		sid.pszDescription = Translate("Sending");
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszName = (char *) "scriver_QUOTE";
		sid.iDefaultIndex = -IDI_QUOTE;
		sid.pszDescription = Translate("Quote button");
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

static int buttonIcons[] = {-1, SMF_ICON_USERDETAILS, SMF_ICON_SMILEY, SMF_ICON_ADD, SMF_ICON_HISTORY, SMF_ICON_QUOTE, SMF_ICON_CANCEL, SMF_ICON_SEND};

void LoadGlobalIcons() {
	int i;
	if (ServiceExists(MS_SKIN2_ADDICON)) {
		g_dat->hIcons[SMF_ICON_ADD] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_ADD");
		g_dat->hIcons[SMF_ICON_USERDETAILS] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_USERDETAILS");
		g_dat->hIcons[SMF_ICON_HISTORY] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_HISTORY");
		g_dat->hIcons[SMF_ICON_TYPING] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_TYPING");
		g_dat->hIcons[SMF_ICON_SEND] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_SEND");
		g_dat->hIcons[SMF_ICON_CANCEL] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_CANCEL");
		g_dat->hIcons[SMF_ICON_SMILEY] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_SMILEY");
		g_dat->hIcons[SMF_ICON_UNICODEON] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_UNICODEON");
		g_dat->hIcons[SMF_ICON_UNICODEOFF] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_UNICODEOFF");
		g_dat->hIcons[SMF_ICON_DELIVERING] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_DELIVERING");
		g_dat->hIcons[SMF_ICON_QUOTE] = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"scriver_QUOTE");

		g_dat->hIcons[SMF_ICON_INCOMING] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_INCOMING));
		g_dat->hIcons[SMF_ICON_OUTGOING] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_OUTGOING));
		g_dat->hIcons[SMF_ICON_NOTICE] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NOTICE));
	} else {
		g_dat->hIcons[SMF_ICON_ADD] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ADDCONTACT), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_USERDETAILS] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_USERDETAILS), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_HISTORY] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_HISTORY), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_SEND] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_SEND), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_CANCEL] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_CANCEL), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_SMILEY] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_SMILEY), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_TYPING] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_TYPING), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_UNICODEON] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_UNICODEON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_UNICODEOFF] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_UNICODEOFF), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_DELIVERING] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IsWinVerXPPlus()? IDI_TIMESTAMP : IDI_TIMESTAMP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		g_dat->hIcons[SMF_ICON_QUOTE] = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_QUOTE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

		g_dat->hIcons[SMF_ICON_INCOMING] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_INCOMING));
		g_dat->hIcons[SMF_ICON_OUTGOING] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_OUTGOING));
		g_dat->hIcons[SMF_ICON_NOTICE] = (HICON) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NOTICE));
	}
	if (g_dat->hButtonIconList == NULL) {
		g_dat->hButtonIconList = ImageList_Create(16, 16, IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, (g_dat->protoNum + 1) * 12 + 8, 0);
	} else {
		ImageList_RemoveAll(g_dat->hButtonIconList);
	}
	for (i=0; i<sizeof(buttonIcons)/sizeof(int); i++) {
		if (buttonIcons[i] == -1) {
			ImageList_AddIcon(g_dat->hButtonIconList, LoadSkinnedProtoIcon(NULL, ID_STATUS_OFFLINE));
		} else {
			ImageList_AddIcon(g_dat->hButtonIconList, g_dat->hIcons[buttonIcons[i]]);
		}
	}


}


static BOOL CALLBACK LangAddCallback(CHAR * str) {
	int i, count;
	UINT cp;
	static struct { UINT cpId; char *cpName; } cpTable[] = {
		{	874,	"Thai" },
		{	932,	"Japanese" },
		{	936,	"Simplified Chinese" },
		{	949,	"Korean" },
		{	950,	"Traditional Chinese" },
		{	1250,	"Central European" },
		{	1251,	"Cyrillic" },
		{	1252,	"Latin I" },
		{	1253,	"Greek" },
		{	1254,	"Turkish" },
		{	1255,	"Hebrew" },
		{	1256,	"Arabic" },
		{	1257,	"Baltic" },
		{	1258,	"Vietnamese" },
		{	1361,	"Korean (Johab)" }
	};
    cp = atoi(str);
	count = sizeof(cpTable)/sizeof(cpTable[0]);
	for (i=0; i<count && cpTable[i].cpId!=cp; i++);
	if (i < count) {
        AppendMenuA(g_dat->hMenuANSIEncoding, MF_STRING, cp, Translate(cpTable[i].cpName));
	}
	return TRUE;
}


void InitGlobals() {
	g_dat = (struct GlobalMessageData *)malloc(sizeof(struct GlobalMessageData));
	g_dat->hMessageWindowList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	g_dat->hParentWindowList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
    g_dat->hMenuANSIEncoding = CreatePopupMenu();
    AppendMenuA(g_dat->hMenuANSIEncoding, MF_STRING, 500, Translate("Default codepage"));
    AppendMenuA(g_dat->hMenuANSIEncoding, MF_SEPARATOR, 0, 0);
    EnumSystemCodePagesA(LangAddCallback, CP_INSTALLED);
	g_hAck = HookEvent(ME_PROTO_ACK, ackevent);
	ReloadGlobals();
	g_dat->lastParent = NULL;
	g_dat->protoNum = 0;
	g_dat->protoNames = NULL;
	g_dat->hTabIconList = NULL;
	g_dat->hButtonIconList = NULL;
	g_dat->draftList = NULL;
}

void FreeGlobals() {
	if (g_hAck) UnhookEvent(g_hAck);
	if (g_dat) {
		if (g_dat->draftList != NULL) tcmdlist_free(g_dat->draftList);
	//	for (i=0;i<sizeof(g_dat->hIcons)/sizeof(g_dat->hIcons[0]);i++)
	//		DestroyIcon(g_dat->hIcons[i]);
		free(g_dat);
	}
	if (g_dat->hTabIconList) {
		ImageList_Destroy(g_dat->hTabIconList);
	}
	if (g_dat->hButtonIconList) {
		ImageList_Destroy(g_dat->hButtonIconList);
	}
}

void ReloadGlobals() {
	g_dat->avatarServiceExists = ServiceExists(MS_AV_GETAVATARBITMAP);
	g_dat->smileyServiceExists =  ServiceExists(MS_SMILEYADD_SHOWSELECTION);
	g_dat->flags = 0;
	g_dat->flags2 = 0;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWBUTTONLINE, SRMSGDEFSET_SHOWBUTTONLINE))
		g_dat->flags |= SMF_SHOWBTNS;
//	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SENDBUTTON, SRMSGDEFSET_SENDBUTTON))
//		g_dat->flags |= SMF_SENDBTN;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AVATARENABLE, SRMSGDEFSET_AVATARENABLE)) {
		g_dat->flags |= SMF_AVATAR;
		if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, SRMSGDEFSET_LIMITAVHEIGHT))
			g_dat->flags |= SMF_LIMITAVATARH;
	}
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
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SWITCHTOACTIVE, SRMSGDEFSET_SWITCHTOACTIVE))
		g_dat->flags |= SMF_SWITCHTOACTIVE;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_LIMITNAMES, SRMSGDEFSET_LIMITNAMES))
		g_dat->flags |= SMF_LIMITNAMES;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_HIDEONETAB, SRMSGDEFSET_HIDEONETAB))
		g_dat->flags |= SMF_HIDEONETAB;

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
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_MESSAGEONNEWLINE, SRMSGDEFSET_MESSAGEONNEWLINE))
		g_dat->flags |= SMF_MSGONNEWLINE;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_DRAWLINES, SRMSGDEFSET_DRAWLINES))
		g_dat->flags |= SMF_DRAWLINES;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES))
		g_dat->flags |= SMF_HIDENAMES;
	g_dat->openFlags = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_POPFLAGS, SRMSGDEFSET_POPFLAGS);
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING))
		g_dat->flags |= SMF_SHOWTYPING;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN))
		g_dat->flags |= SMF_SHOWTYPINGWIN;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN))
		g_dat->flags |= SMF_SHOWTYPINGTRAY;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST))
		g_dat->flags |= SMF_SHOWTYPINGCLIST;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SAVEDRAFTS, SRMSGDEFSET_SAVEDRAFTS))
		g_dat->flags |= SMF_SAVEDRAFTS;

	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AUTOPOPUP, SRMSGDEFSET_AUTOPOPUP))
		g_dat->flags2 |= SMF2_AUTOPOPUP;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_STAYMINIMIZED, SRMSGDEFSET_STAYMINIMIZED))
		g_dat->flags2 |= SMF2_STAYMINIMIZED;
	
	g_dat->limitAvatarMaxH = 100000;
	g_dat->limitAvatarMinH = 0;
	if (g_dat->flags & SMF_LIMITAVATARH) {
		g_dat->limitAvatarMaxH = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_AVHEIGHT, SRMSGDEFSET_AVHEIGHT);
		g_dat->limitAvatarMinH = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_AVHEIGHTMIN, SRMSGDEFSET_AVHEIGHTMIN);
		if (g_dat->limitAvatarMinH > g_dat->limitAvatarMaxH) {
			g_dat->limitAvatarMinH = g_dat->limitAvatarMaxH;
		}
	}
	if (LOBYTE(LOWORD(GetVersion())) >= 5  && pSetLayeredWindowAttributes != NULL) {
		if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USETRANSPARENCY, SRMSGDEFSET_USETRANSPARENCY))
			g_dat->flags |= SMF_USETRANSPARENCY;
		g_dat->activeAlpha = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_ACTIVEALPHA, SRMSGDEFSET_ACTIVEALPHA);
		g_dat->inactiveAlpha = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_INACTIVEALPHA, SRMSGDEFSET_INACTIVEALPHA);
	}
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST))
		g_dat->flags |= SMF_SHOWTYPINGCLIST;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USEIEVIEW, SRMSGDEFSET_USEIEVIEW))
		g_dat->flags |= SMF_USEIEVIEW;

	g_dat->buttonVisibility = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_BUTTONVISIBILITY, SRMSGDEFSET_BUTTONVISIBILITY);

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
