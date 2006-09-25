/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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
#include "chat.h"
#include <shlwapi.h>

extern HICON		hIcons[30];
extern BOOL			PopUpInstalled;
extern HINSTANCE	g_hInst;
extern FONTINFO		aFonts[OPTIONS_FONTCOUNT];
extern HMENU		g_hMenu;
extern HANDLE		hBuildMenuEvent ;
extern HANDLE		hSendEvent;
extern SESSION_INFO g_TabSession;

int GetRichTextLength(HWND hwnd)
{
	GETTEXTLENGTHEX gtl;

	gtl.flags = GTL_PRECISE;
	gtl.codepage = CP_ACP ;
	return (int) SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
}

char * RemoveFormatting(char * pszWord)
{
	static char szTemp[10000];
	int i = 0;
	int j = 0;

	if(pszWord == 0 || lstrlenA(pszWord) == 0)
		return NULL;

	while(j < 9999 && i <= lstrlenA(pszWord))
	{
		if(pszWord[i] == '%')
		{
			switch (pszWord[i+1])
			{
			case '%':
				szTemp[j] = '%';
				j++;
				i++; i++;
				break;
			case 'b':
			case 'u':
			case 'i':
			case 'B':
			case 'U':
			case 'I':
			case 'r':
			case 'C':
			case 'F':
				i++; i++;
				break;

			case 'c':
			case 'f':
				i += 4;
				break;

			default:
				szTemp[j] = pszWord[i];
				j++;
				i++;
				break;
			}

		}
		else
		{
			szTemp[j] = pszWord[i];
			j++;
			i++;
		}
	}

	return (char *) &szTemp;


}
static void __stdcall ShowRoomFromPopup(void * pi)
{
	SESSION_INFO * si = (SESSION_INFO *) pi;
	ShowRoom(si, WINDOW_VISIBLE, TRUE);
}

static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_COMMAND:
			if (HIWORD(wParam) == STN_CLICKED)
			{
				SESSION_INFO * si = (SESSION_INFO *)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)0);;

				CallFunctionAsync(ShowRoomFromPopup, si);

				PUDeletePopUp(hWnd);
				return TRUE;
			}
			break;
		case WM_CONTEXTMENU:
			{
				SESSION_INFO * si = (SESSION_INFO *)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)0);
				if(si->hContact)
				{
					if(CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
						CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)"chaticon");
				}
				PUDeletePopUp( hWnd );
			}
			break;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static int ShowPopup (HANDLE hContact, SESSION_INFO * si, HICON hIcon,  char * pszProtoName,  char * pszRoomName, COLORREF crBkg, char * fmt, ...)
{
	POPUPDATAEX pd = {0};
	va_list marker;
	static char szBuf[4*1024];

	if (!fmt || lstrlenA(fmt) == 0 || lstrlenA(fmt) > 2000)
		return 0;

	va_start(marker, fmt);
	vsprintf(szBuf, fmt, marker);
	va_end(marker);

	pd.lchContact = hContact;

	if (hIcon)
		pd.lchIcon = hIcon ;
	else
		pd.lchIcon = LoadIconEx(IDI_CHANMGR, "window", 0, 0 );

	mir_snprintf(pd.lpzContactName, MAX_CONTACTNAME-1, "%s - %s", pszProtoName, (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0));

	lstrcpynA(pd.lpzText, Translate(szBuf), MAX_SECONDLINE-1);

	pd.iSeconds = g_Settings.iPopupTimeout;

	if (g_Settings.iPopupStyle == 2)
	{
		pd.colorBack = 0;
		pd.colorText = 0;
	}
	else if (g_Settings.iPopupStyle == 3)
	{
		pd.colorBack = g_Settings.crPUBkgColour;
		pd.colorText = g_Settings.crPUTextColour;
	}
	else
	{
		pd.colorBack = g_Settings.crLogBackground;
		pd.colorText = crBkg;

	}

	pd.PluginWindowProc = PopupDlgProc;

	pd.PluginData = si;

	return PUAddPopUpEx (&pd);

}


static BOOL DoTrayIcon(SESSION_INFO * si, GCEVENT * gce)
{
	int iEvent = gce->pDest->iType;

	if (iEvent&g_Settings.dwTrayIconFlags)
	{
		switch (iEvent)
		{
		case GC_EVENT_MESSAGE|GC_EVENT_HIGHLIGHT :
		case GC_EVENT_ACTION|GC_EVENT_HIGHLIGHT :
			CList_AddEvent(si->hContact, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), "chaticon", 0, Translate("%s wants your attention in %s"), gce->pszNick, si->pszName);
			break;
		case GC_EVENT_MESSAGE :
			CList_AddEvent(si->hContact, hIcons[ICON_MESSAGE], "chaticon", CLEF_ONLYAFEW, Translate("%s speaks in %s"), gce->pszNick, si->pszName);
			break;
		case GC_EVENT_ACTION:
			CList_AddEvent(si->hContact, hIcons[ICON_ACTION], "chaticon", CLEF_ONLYAFEW, Translate("%s speaks in %s"), gce->pszNick, si->pszName);
			break;
		case GC_EVENT_JOIN:
			CList_AddEvent(si->hContact, hIcons[ICON_JOIN], "chaticon", CLEF_ONLYAFEW, Translate("%s has joined %s"), gce->pszNick, si->pszName);
			break;
		case GC_EVENT_PART:
			CList_AddEvent(si->hContact, hIcons[ICON_PART], "chaticon", CLEF_ONLYAFEW, Translate("%s has left %s"), gce->pszNick, si->pszName);
			break;
		case GC_EVENT_QUIT:
			CList_AddEvent(si->hContact, hIcons[ICON_QUIT], "chaticon", CLEF_ONLYAFEW, Translate("%s has disconnected"), gce->pszNick);
			break;
		case GC_EVENT_NICK:
			CList_AddEvent(si->hContact, hIcons[ICON_NICK], "chaticon", CLEF_ONLYAFEW, Translate("%s is now known as %s"), gce->pszNick, gce->pszText);
			break;
		case GC_EVENT_KICK:
			CList_AddEvent(si->hContact, hIcons[ICON_KICK], "chaticon", CLEF_ONLYAFEW, Translate("%s kicked %s from %s"), gce->pszStatus, gce->pszNick, si->pszName);
			break;
		case GC_EVENT_NOTICE:
			CList_AddEvent(si->hContact, hIcons[ICON_NOTICE], "chaticon", CLEF_ONLYAFEW, Translate("Notice from %s"), gce->pszNick);
			break;
		case GC_EVENT_TOPIC:
			CList_AddEvent(si->hContact, hIcons[ICON_TOPIC], "chaticon", CLEF_ONLYAFEW, Translate("Topic change in %s"), si->pszName);
			break;
		case GC_EVENT_INFORMATION:
			CList_AddEvent(si->hContact, hIcons[ICON_INFO], "chaticon", CLEF_ONLYAFEW, Translate("Information in %s"), si->pszName);
			break;
		case GC_EVENT_ADDSTATUS:
			CList_AddEvent(si->hContact, hIcons[ICON_ADDSTATUS], "chaticon", CLEF_ONLYAFEW, Translate("%s enables \'%s\' status for %s in %s"), gce->pszText, gce->pszStatus, gce->pszNick, si->pszName);
			break;
		case GC_EVENT_REMOVESTATUS:
			CList_AddEvent(si->hContact, hIcons[ICON_REMSTATUS], "chaticon", CLEF_ONLYAFEW, Translate("%s disables \'%s\' status for %s in %s"), gce->pszText, gce->pszStatus, gce->pszNick, si->pszName);
			break;

		default:break;
		}

	}

	return TRUE;
}
static BOOL DoPopup(SESSION_INFO * si, GCEVENT * gce)
{
	int iEvent = gce->pDest->iType;

	if (iEvent&g_Settings.dwPopupFlags)
	{
		switch (iEvent)
		{
		case GC_EVENT_MESSAGE|GC_EVENT_HIGHLIGHT :
			ShowPopup(si->hContact, si, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), si->pszModule, si->pszName, aFonts[16].color, Translate("%s says: %s"), (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_ACTION|GC_EVENT_HIGHLIGHT :
			ShowPopup(si->hContact, si, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), si->pszModule, si->pszName, aFonts[16].color, "%s %s", (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_MESSAGE :
			ShowPopup(si->hContact, si, hIcons[ICON_MESSAGE], si->pszModule, si->pszName, aFonts[9].color, Translate("%s says: %s"), (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_ACTION:
			ShowPopup(si->hContact, si, hIcons[ICON_ACTION], si->pszModule, si->pszName, aFonts[15].color, "%s %s", (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_JOIN:
			ShowPopup(si->hContact, si, hIcons[ICON_JOIN], si->pszModule, si->pszName, aFonts[3].color, Translate("%s has joined"), (char *)gce->pszNick);
			break;
		case GC_EVENT_PART:
			if(!gce->pszText)
				ShowPopup(si->hContact, si, hIcons[ICON_PART], si->pszModule, si->pszName, aFonts[4].color, Translate("%s has left"), (char *)gce->pszNick);
			else
				ShowPopup(si->hContact, si, hIcons[ICON_PART], si->pszModule, si->pszName, aFonts[4].color, Translate("%s has left (%s)"), (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
				break;
		case GC_EVENT_QUIT:
			if(!gce->pszText)
				ShowPopup(si->hContact, si, hIcons[ICON_QUIT], si->pszModule, si->pszName, aFonts[5].color, Translate("%s has disconnected"), (char *)gce->pszNick);
			else
				ShowPopup(si->hContact, si, hIcons[ICON_QUIT], si->pszModule, si->pszName, aFonts[5].color, Translate("%s has disconnected (%s)"), (char *)gce->pszNick,RemoveFormatting((char *)gce->pszText));
				break;
		case GC_EVENT_NICK:
			ShowPopup(si->hContact, si, hIcons[ICON_NICK], si->pszModule, si->pszName, aFonts[7].color, Translate("%s is now known as %s"), (char *)gce->pszNick, (char *)gce->pszText);
			break;
		case GC_EVENT_KICK:
			if(!gce->pszText)
				ShowPopup(si->hContact, si, hIcons[ICON_KICK], si->pszModule, si->pszName, aFonts[6].color, Translate("%s kicked %s"), (char *)gce->pszStatus, (char *)gce->pszNick);
			else
				ShowPopup(si->hContact, si, hIcons[ICON_KICK], si->pszModule, si->pszName, aFonts[6].color, Translate("%s kicked %s (%s)"), (char *)gce->pszStatus, (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_NOTICE:
			ShowPopup(si->hContact, si, hIcons[ICON_NOTICE], si->pszModule, si->pszName, aFonts[8].color, Translate("Notice from %s: %s"), (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_TOPIC:
			if(!gce->pszNick)
				ShowPopup(si->hContact, si, hIcons[ICON_TOPIC], si->pszModule, si->pszName, aFonts[11].color, Translate("The topic is \'%s\'"), RemoveFormatting((char *)gce->pszText));
			else
				ShowPopup(si->hContact, si, hIcons[ICON_TOPIC], si->pszModule, si->pszName, aFonts[11].color, Translate("The topic is \'%s\' (set by %s)"), RemoveFormatting((char *)gce->pszText), (char *)gce->pszNick);
			break;
		case GC_EVENT_INFORMATION:
			ShowPopup(si->hContact, si, hIcons[ICON_INFO], si->pszModule, si->pszName, aFonts[12].color, "%s", RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_ADDSTATUS:
			ShowPopup(si->hContact, si, hIcons[ICON_ADDSTATUS], si->pszModule, si->pszName, aFonts[13].color, Translate("%s enables \'%s\' status for %s"), (char *)gce->pszText, (char *)gce->pszStatus, (char *)gce->pszNick);
			break;
		case GC_EVENT_REMOVESTATUS:
			ShowPopup(si->hContact, si, hIcons[ICON_REMSTATUS], si->pszModule, si->pszName, aFonts[14].color, Translate("%s disables \'%s\' status for %s"), (char *)gce->pszText, (char *)gce->pszStatus, (char *)gce->pszNick);
			break;

		default:break;
		}

	}

	return TRUE;
}

BOOL DoSoundsFlashPopupTrayStuff(SESSION_INFO * si, GCEVENT * gce, BOOL bHighlight, int bManyFix)
{
	BOOL bInactive;
	int iEvent;

	if(!gce || !si ||  gce->bIsMe || si->iType == GCW_SERVER)
		return FALSE;

	bInactive = si->hWnd == NULL || GetForegroundWindow() != GetParent(si->hWnd);
	// bInactive |=  GetActiveWindow() != si->hWnd; // Removed this, because it seemed to be FALSE, even when window was focused, causing incorrect notifications

	iEvent = gce->pDest->iType;

	if(bHighlight)
	{
		gce->pDest->iType |= GC_EVENT_HIGHLIGHT;
		if(bInactive || !g_Settings.SoundsFocus)
			SkinPlaySound("ChatHighlight");
		if(bInactive && si->hWnd && DBGetContactSettingByte(NULL, "Chat", "FlashWindowHighlight", 0) != 0) //!g_Settings.TabsEnable &&
			SendMessage(GetParent(si->hWnd), CM_STARTFLASHING, 0, 0);
		if(DBGetContactSettingByte(si->hContact, "CList", "Hidden", 0) != 0)
			DBDeleteContactSetting(si->hContact, "CList", "Hidden");
		if(bInactive)
			DoTrayIcon(si, gce);
		if(bInactive || !g_Settings.PopUpInactiveOnly)
			DoPopup(si, gce);
		if(bInactive && si->hWnd)
			SendMessage(si->hWnd, GC_SETMESSAGEHIGHLIGHT, 0, 0);
		return TRUE;
	}

	// do blinking icons in tray
	if(bInactive || !g_Settings.TrayIconInactiveOnly)
		DoTrayIcon(si, gce);

	// stupid thing to not create multiple popups for a QUIT event for instance
	if(bManyFix == 0)
	{
		// do popups
		if(bInactive || !g_Settings.PopUpInactiveOnly)
			DoPopup(si, gce);

		// do sounds and flashing
		switch (iEvent)
		{
		case GC_EVENT_JOIN:
			if(bInactive || !g_Settings.SoundsFocus)
				SkinPlaySound("ChatJoin");
			break;
		case GC_EVENT_PART:
			if(bInactive || !g_Settings.SoundsFocus)
				SkinPlaySound("ChatPart");
			break;
		case GC_EVENT_QUIT:
			if(bInactive || !g_Settings.SoundsFocus)
				SkinPlaySound("ChatQuit");
			break;
		case GC_EVENT_ADDSTATUS:
		case GC_EVENT_REMOVESTATUS:
			if(bInactive || !g_Settings.SoundsFocus)
				SkinPlaySound("ChatMode");
			break;
		case GC_EVENT_KICK:
			if(bInactive || !g_Settings.SoundsFocus)
				SkinPlaySound("ChatKick");
			break;
		case GC_EVENT_MESSAGE:
			if(bInactive || !g_Settings.SoundsFocus)
				SkinPlaySound("ChatMessage");
			if(bInactive && g_Settings.FlashWindow && si->hWnd) // !g_Settings.TabsEnable &&
				SendMessage(GetParent(si->hWnd), CM_STARTFLASHING, 0, 0);
			if(bInactive && !(si->wState&STATE_TALK))
			{
				si->wState |= STATE_TALK;
				DBWriteContactSettingWord(si->hContact, si->pszModule,"ApparentMode",(LPARAM)(WORD) 40071);
			}
			if(bInactive && si->hWnd)
				SendMessage(si->hWnd, GC_SETTABHIGHLIGHT, 0, 0);
			break;
		case GC_EVENT_ACTION:
			if(bInactive || !g_Settings.SoundsFocus)
				SkinPlaySound("ChatAction");
			break;
		case GC_EVENT_NICK:
			if(bInactive || !g_Settings.SoundsFocus)
				SkinPlaySound("ChatNick");
			break;
		case GC_EVENT_NOTICE:
			if(bInactive || !g_Settings.SoundsFocus)
				SkinPlaySound("ChatNotice");
			break;
		case GC_EVENT_TOPIC:
			if(bInactive || !g_Settings.SoundsFocus)
				SkinPlaySound("ChatTopic");
			break;
		default:break;
		}
	}

	return TRUE;
}
int GetColorIndex(char * pszModule, COLORREF cr)
{
	MODULEINFO * pMod = MM_FindModule(pszModule);
	int i = 0;

	if(!pMod || pMod->nColorCount == 0)
		return -1;

	for (i = 0; i < pMod->nColorCount; i++)
	{
		if (pMod->crColors[i] == cr)
			return i;
	}

	return -1;


}

// obscure function that is used to make sure that any of the colors
// passed by the protocol is used as fore- or background color
// in the messagebox. THis is to vvercome limitations in the richedit
// that I do not know currently how to fix
void CheckColorsInModule(char * pszModule)
{
	MODULEINFO * pMod = MM_FindModule(pszModule);
	int i = 0;
	COLORREF crFG;
	COLORREF crBG = (COLORREF)DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW));

	Chat_LoadMsgDlgFont(17, NULL, &crFG);

	if(!pMod)
		return;

	for (i = 0; i < pMod->nColorCount; i++)
	{
		if (pMod->crColors[i] == crFG || pMod->crColors[i] == crBG)
		{
			if(pMod->crColors[i] == RGB(255,255,255))
				pMod->crColors[i]--;
			else
				pMod->crColors[i]++;

		}
	}

	return;
}

char* my_strstri(char *s1, char *s2)
{
	int i,j,k;
	for(i=0;s1[i];i++)
		for(j=i,k=0;tolower(s1[j])==tolower(s2[k]);j++,k++)
			if(!s2[k+1])
				return (s1+i);
	return NULL;
}

BOOL IsHighlighted(SESSION_INFO * si, char * pszText)
{
	if(g_Settings.HighlightEnabled && g_Settings.pszHighlightWords && pszText && si->pMe)
	{
		char* p1 = g_Settings.pszHighlightWords;
		char* p2 = NULL;
		char* p3 = pszText;
		static char szWord1[1000];
		static char szWord2[1000];
		static char szTrimString[] = ":,.!?;\'>)";

		// compare word for word
		while (*p1 != '\0')
		{
			// find the next/first word in the highlight word string
			// skip 'spaces' be4 the word
			while(*p1 == ' ' && *p1 != '\0')
				p1 += 1;

			//find the end of the word
			p2 = strchr(p1, ' ');
			if (!p2)
				p2 = strchr(p1, '\0');
			if (p1 == p2)
				return FALSE;

			// copy the word into szWord1
			lstrcpynA(szWord1, p1, p2-p1>998?999:p2-p1+1);
			p1 = p2;

			// replace %m with the users nickname
			p2 = strchr(szWord1, '%');
			if (p2 && p2[1] == 'm')
			{
				char szTemp[50];

				p2[1] = 's';
				lstrcpynA(szTemp, szWord1, 999);
				mir_snprintf(szWord1, sizeof(szWord1), szTemp, si->pMe->pszNick);
			}

			// time to get the next/first word in the incoming text string
			while(*p3 != '\0')
			{
				// skip 'spaces' be4 the word
				while(*p3 == ' ' && *p3 != '\0')
					p3 += 1;

				//find the end of the word
				p2 = strchr(p3, ' ');
				if (!p2)
					p2 = strchr(p3, '\0');


				if (p3 != p2)
				{
					// eliminate ending character if needed
					if(p2-p3 > 1 && strchr(szTrimString, p2[-1]))
						p2 -= 1;

					// copy the word into szWord2 and remove formatting
					lstrcpynA(szWord2, p3, p2-p3>998?999:p2-p3+1);

					// reset the pointer if it was touched because of an ending character
					if(*p2 != '\0' && *p2 != ' ')
						p2 += 1;
					p3 = p2;

					CharLowerA(szWord1);
					CharLowerA(szWord2);

					// compare the words, using wildcards
					if (WCCmp(szWord1, RemoveFormatting(szWord2)))
						return TRUE;
				}
			}
			p3 = pszText;
		}

	}
	return FALSE;
}

BOOL LogToFile(SESSION_INFO * si, GCEVENT * gce)
{

	MODULEINFO * mi = NULL;
	static char szBuffer[4096];
	static char szLine[4096];
	static char szTime[100];
	FILE *hFile = NULL;
	char szFile[MAX_PATH];
	char szName[MAX_PATH];
	char szFolder[MAX_PATH];
	char p = '\0';
	szBuffer[0] = '\0';

	if (!si || !gce)
		return FALSE;

	mi = MM_FindModule(si->pszModule);

	if(!mi)
		return FALSE;

 	mir_snprintf(szName, MAX_PATH,"%s",mi->pszModDispName);
	ValidateFilename(szName);
	mir_snprintf(szFolder, MAX_PATH,"%s\\%s", g_Settings.pszLogDir, szName );

	if(!PathIsDirectoryA(szFolder))
		CreateDirectoryA(szFolder, NULL);

	mir_snprintf(szName, MAX_PATH,"%s.log",si->pszID);
	ValidateFilename(szName);

	mir_snprintf(szFile, MAX_PATH,"%s\\%s", szFolder, szName );
	lstrcpynA(szTime, MakeTimeStamp(g_Settings.pszTimeStampLog, gce->time), 99);

	hFile = fopen(szFile,"at+");
	if(hFile)
	{
		char szTemp[512];
		char szTemp2[512];
		char * pszNick = NULL;
		if(gce->pszNick)
		{
			if(g_Settings.LogLimitNames && lstrlenA(gce->pszNick) > 20)
			{
				lstrcpynA(szTemp2, gce->pszNick, 20);
				lstrcpynA(szTemp2+20, "...", 4);
			}
			else
				lstrcpynA(szTemp2, gce->pszNick, 511);

			if(gce->pszUserInfo)
				mir_snprintf(szTemp, sizeof(szTemp), "%s (%s)", szTemp2, gce->pszUserInfo);
			else
				mir_snprintf(szTemp, sizeof(szTemp), "%s", szTemp2);
			pszNick = szTemp;
		}
		switch (gce->pDest->iType)
		{
		case GC_EVENT_MESSAGE:
		case GC_EVENT_MESSAGE|GC_EVENT_HIGHLIGHT:
			p = '*';
			mir_snprintf(szBuffer, 4000, "%s * %s", (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_ACTION:
			p = '*';
			mir_snprintf(szBuffer, 4000, "%s %s", (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_JOIN:
			p = '>';
			mir_snprintf(szBuffer, 4000, Translate("%s has joined"), (char *)pszNick);
			break;
		case GC_EVENT_PART:
			p = '<';
			if(!gce->pszText)
				mir_snprintf(szBuffer, 4000, Translate("%s has left"), (char *)pszNick);
			else
				mir_snprintf(szBuffer, 4000, Translate("%s has left (%s)"), (char *)pszNick, RemoveFormatting((char *)gce->pszText));
				break;
		case GC_EVENT_QUIT:
			p = '<';
			if(!gce->pszText)
				mir_snprintf(szBuffer, 4000, Translate("%s has disconnected"), (char *)pszNick);
			else
				mir_snprintf(szBuffer, 4000, Translate("%s has disconnected (%s)"), (char *)pszNick,RemoveFormatting((char *)gce->pszText));
				break;
		case GC_EVENT_NICK:
			p = '^';
			mir_snprintf(szBuffer, 4000, Translate("%s is now known as %s"), (char *)gce->pszNick, (char *)gce->pszText);
			break;
		case GC_EVENT_KICK:
			p = '~';
			if(!gce->pszText)
				mir_snprintf(szBuffer, 4000, Translate("%s kicked %s"), (char *)gce->pszStatus, (char *)gce->pszNick);
			else
				mir_snprintf(szBuffer, 4000, Translate("%s kicked %s (%s)"), (char *)gce->pszStatus, (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_NOTICE:
			p = '¤';
			mir_snprintf(szBuffer, 4000, Translate("Notice from %s: %s"), (char *)gce->pszNick, RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_TOPIC:
			p = '#';
			if(!gce->pszNick)
				mir_snprintf(szBuffer, 4000, Translate("The topic is \'%s\'"), RemoveFormatting((char *)gce->pszText));
			else
				mir_snprintf(szBuffer, 4000, Translate("The topic is \'%s\' (set by %s)"), RemoveFormatting((char *)gce->pszText), (char *)gce->pszNick);
			break;
		case GC_EVENT_INFORMATION:
			p = '!';
			mir_snprintf(szBuffer, 4000, "%s", RemoveFormatting((char *)gce->pszText));
			break;
		case GC_EVENT_ADDSTATUS:
			p = '+';
			mir_snprintf(szBuffer, 4000, Translate("%s enables \'%s\' status for %s"), (char *)gce->pszText, (char *)gce->pszStatus, (char *)gce->pszNick);
			break;
		case GC_EVENT_REMOVESTATUS:
			p = '-';
			mir_snprintf(szBuffer, 4000, Translate("%s disables \'%s\' status for %s"), (char *)gce->pszText, (char *)gce->pszStatus, (char *)gce->pszNick);
			break;
		default:break;
		}
		if(p)
			mir_snprintf(szLine, 4000, Translate("%s %c %s\n"), szTime, p, szBuffer);
		else
			mir_snprintf(szLine, 4000, Translate("%s %s\n"), szTime, szBuffer);

		if(szLine[0])
		{
			fputs(szLine, hFile);
			if(g_Settings.LoggingLimit > 0)
			{
				DWORD dwSize;
				DWORD trimlimit;

				fseek(hFile,0,SEEK_END);
				dwSize = ftell(hFile);
				rewind (hFile);
				trimlimit = g_Settings.LoggingLimit*1024+ 1024*10;
				if (dwSize > trimlimit)
				{
					BYTE * pBuffer = 0;
					BYTE * pBufferTemp = 0;
					int read = 0;

					pBuffer = (BYTE *)malloc(g_Settings.LoggingLimit*1024+1);
					pBuffer[g_Settings.LoggingLimit*1024] = '\0';
					fseek(hFile,-g_Settings.LoggingLimit*1024,SEEK_END);
					read = fread(pBuffer, 1, g_Settings.LoggingLimit*1024, hFile);
					fclose(hFile);
					hFile = NULL;

					// trim to whole lines, should help with broken log files I hope.
					pBufferTemp = strchr(pBuffer, '\n');
					if(pBufferTemp)
					{
						pBufferTemp++;
						read = read - (pBufferTemp - pBuffer);
					}
					else
						pBufferTemp = pBuffer;

					if(read > 0)
					{
						hFile = fopen(szFile,"wt");
						if(hFile )
						{
							fwrite(pBufferTemp, 1, read, hFile);
							fclose(hFile); hFile = NULL;
						}
					}
					if(pBuffer)
						free(pBuffer);
				}
			}
		}
		if (hFile)
			fclose(hFile); hFile = NULL;
		return TRUE;
	}

	return FALSE;
}






UINT CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO * si, char * pszUID, char * pszWordText)
{
	GCMENUITEMS gcmi = {0};
	int i;
	HMENU hSubMenu = 0;

	*hMenu = GetSubMenu(g_hMenu, iIndex);
	CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) *hMenu, 0);
	gcmi.pszID = si->pszID;
	gcmi.pszModule = si->pszModule;
	gcmi.pszUID = pszUID;

	if(iIndex == 1)
	{
		int i = GetRichTextLength(GetDlgItem(hwndDlg, IDC_CHAT_LOG));

		EnableMenuItem(*hMenu, ID_CLEARLOG, MF_ENABLED);
		EnableMenuItem(*hMenu, ID_COPYALL, MF_ENABLED);
//		EnableMenuItem(*hMenu, 4, MF_BYPOSITION|MF_GRAYED);
				ModifyMenuA(*hMenu, 4, MF_GRAYED|MF_BYPOSITION, 4, NULL);
		if (!i)
		{
			EnableMenuItem(*hMenu, ID_COPYALL, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(*hMenu, ID_CLEARLOG, MF_BYCOMMAND | MF_GRAYED);
			if(pszWordText && pszWordText[0])
			{
				ModifyMenuA(*hMenu, 4, MF_ENABLED|MF_BYPOSITION, 4, NULL);
				//EnableMenuItem(*hMenu, 4, MF_BYPOSITION|MF_ENABLED);
			}
		}
		if(pszWordText && pszWordText[0])
		{
			char szMenuText[4096];
			mir_snprintf(szMenuText, 4096, "Look up \'%s\':", pszWordText);
			ModifyMenuA(*hMenu, 4, MF_STRING|MF_BYPOSITION, 4, szMenuText);
		}
		else
			ModifyMenuA(*hMenu, 4, MF_STRING|MF_GRAYED|MF_BYPOSITION, 4, "No word to look up");
		gcmi.Type = MENU_ON_LOG;
	}
	else if(iIndex == 0)
	{
		char szTemp[30];
		char szTemp2[30];
		lstrcpynA(szTemp, Translate("&Message"), 24);
		if(pszUID)
			mir_snprintf(szTemp2, 29, "%s %s", szTemp, pszUID);
		else
			lstrcpynA(szTemp2, szTemp, 24);
		if(lstrlenA(szTemp2) > 22)
			lstrcpynA(szTemp2+22, "...", 4);
		ModifyMenuA(*hMenu, ID_MESS, MF_STRING|MF_BYCOMMAND, ID_MESS, szTemp2);
		gcmi.Type = MENU_ON_NICKLIST;
	}
	NotifyEventHooks(hBuildMenuEvent, 0, (WPARAM)&gcmi);
	if (gcmi.nItems > 0)
		AppendMenu(*hMenu, MF_SEPARATOR, 0, 0);
	for(i = 0; i < gcmi.nItems; i++)
	{
		if(gcmi.Item[i].uType == MENU_NEWPOPUP)
		{
			hSubMenu = CreateMenu();
			AppendMenuA(*hMenu, gcmi.Item[i].bDisabled?MF_POPUP|MF_GRAYED:MF_POPUP, (UINT)hSubMenu, gcmi.Item[i].pszDesc);
		}
		else if(gcmi.Item[i].uType == MENU_POPUPITEM)
			AppendMenuA(hSubMenu==0?*hMenu:hSubMenu, gcmi.Item[i].bDisabled?MF_STRING|MF_GRAYED:MF_STRING, gcmi.Item[i].dwID, gcmi.Item[i].pszDesc);
		else if(gcmi.Item[i].uType == MENU_POPUPSEPARATOR)
			AppendMenuA(hSubMenu==0?*hMenu:hSubMenu, MF_SEPARATOR, 0, gcmi.Item[i].pszDesc);
		else if(gcmi.Item[i].uType == MENU_SEPARATOR)
			AppendMenuA(*hMenu, MF_SEPARATOR, 0, gcmi.Item[i].pszDesc);
		else if(gcmi.Item[i].uType == MENU_ITEM)
			AppendMenuA(*hMenu, gcmi.Item[i].bDisabled?MF_STRING|MF_GRAYED:MF_STRING, gcmi.Item[i].dwID, gcmi.Item[i].pszDesc);
	}
	return TrackPopupMenu(*hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);

}





void DestroyGCMenu(HMENU *hMenu, int iIndex)
{
	MENUITEMINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_SUBMENU;
	while(GetMenuItemInfo(*hMenu, iIndex, TRUE, &mi))
	{
		if(mi.hSubMenu != NULL)
			DestroyMenu(mi.hSubMenu);
		RemoveMenu(*hMenu, iIndex, MF_BYPOSITION);
	}
}






BOOL DoEventHookAsync(HWND hwnd, char * pszID, char * pszModule, int iType, char * pszUID, char * pszText, DWORD dwItem)
{
	GCHOOK * gch = malloc(sizeof(GCHOOK));
	GCDEST * gcd = malloc(sizeof(GCDEST));

	if (pszID)
	{
		gcd->pszID = malloc(lstrlenA(pszID)+1);
		lstrcpyA(gcd->pszID, pszID);
	} else gcd->pszID = NULL;
	if (pszModule)
	{
		gcd->pszModule = malloc(lstrlenA(pszModule)+1);
		lstrcpyA(gcd->pszModule, pszModule);
	}else gcd->pszModule = NULL;
	if (pszUID)
	{
		gch->pszUID = malloc(lstrlenA(pszUID)+1);
		lstrcpyA(gch->pszUID, pszUID);
	}else gch->pszUID = NULL;
	if (pszText)
	{
		gch->pszText = malloc(lstrlenA(pszText)+1);
		lstrcpyA(gch->pszText, pszText);
	}else gch->pszText = NULL;

	gcd->iType = iType;
	gch->dwData = dwItem;
	gch->pDest = gcd;
	PostMessage(hwnd, GC_FIREHOOK, 0, (LPARAM) gch);
	return TRUE;
}




BOOL DoEventHook(char * pszID, char * pszModule, int iType, char * pszUID, char * pszText, DWORD dwItem)
{
	GCHOOK gch = {0};
	GCDEST gcd = {0};

	gcd.pszID = pszID;
	gcd.pszModule = pszModule;
	gcd.iType = iType;
	gch.pDest = &gcd;
	gch.pszText = pszText;
	gch.pszUID = pszUID;
	gch.dwData = dwItem;
	NotifyEventHooks(hSendEvent,0,(WPARAM)&gch);
	return TRUE;
}

void ValidateFilename (char * filename)
{
	char *p1 = filename;
	char szForbidden[] = "\\/:*?\"<>|";
	while(*p1 != '\0')
	{
		if(strchr(szForbidden, *p1))
			*p1 = '_';
		p1 +=1;
	}
}
