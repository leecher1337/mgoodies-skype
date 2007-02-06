/*
Scriver

Copyright 2000-2003 Miranda ICQ/IM project,
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
#include "m_metacontacts.h"
#include <commctrl.h>

#define TIMERID_MSGSEND      0
#define TIMERID_FLASHWND     1
#define TIMERID_TYPE         2
#define TIMEOUT_FLASHWND     900
#define TIMEOUT_ANTIBOMB     4000       //multiple-send bombproofing: send max 3 messages every 4 seconds
#define ANTIBOMB_COUNT       3
#define TIMEOUT_TYPEOFF      10000      //send type off after 10 seconds of inactivity
#define VALID_AVATAR(x)      (x==PA_FORMAT_PNG||x==PA_FORMAT_JPEG||x==PA_FORMAT_ICON||x==PA_FORMAT_BMP||x==PA_FORMAT_GIF)

#define ENTERCLICKTIME   1000   //max time in ms during which a double-tap on enter will cause a send

#if defined(_UNICODE)
	#define SEND_FLAGS PREF_UNICODE
#else
	#define SEND_FLAGS 0
#endif

extern HCURSOR hCurSplitNS, hCurSplitWE, hCurHyperlinkHand, hDragCursor;
extern HANDLE hHookWinEvt;
extern HANDLE hHookWinPopup;
extern struct CREOleCallback reOleCallback, reOleCallback2;
extern HINSTANCE g_hInst;


static void UpdateReadChars(HWND hwndDlg, struct MessageWindowData * dat);

static WNDPROC OldMessageEditProc, OldSplitterProc, OldLogEditProc;
static TCHAR *buttonNames[] = {_T("User Menu"), _T("User Details"), _T("Smiley"), _T("Add Contact"), _T("History"), _T("Quote"), _T("Close"), _T("Send")};
static const UINT buttonLineControls[] = { IDC_USERMENU, IDC_DETAILS, IDC_SMILEYS, IDC_ADD, IDC_HISTORY, IDC_QUOTE, IDCANCEL, IDOK};
static char buttonAlignment[] = { 0, 0, 0, 1, 1, 1, 1, 1};
static UINT buttonSpacing[] = { 0, 0, 12, 0, 0, 0, 0, 0};
static UINT buttonWidth[] = { 24, 24, 24, 24, 24, 24, 24, 38};

/*
static DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
    char *szFilename = (char *)dwCookie;
    FILE *file;
	file = fopen(szFilename, "ab");
	if (file != NULL) {
		*pcb = fwrite(pbBuff, cb, 1, file);
		fclose(file);
		return 0;
	}
    return 1;
}
*/
static DWORD CALLBACK StreamOutCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	struct MessageSendInfo * msi = (struct MessageSendInfo *) dwCookie;
	msi->sendBuffer = (char *)mir_realloc(msi->sendBuffer, msi->sendBufferSize + cb + 2);
	memcpy (msi->sendBuffer + msi->sendBufferSize, pbBuff, cb);
	msi->sendBufferSize += cb;
	*((TCHAR *)(msi->sendBuffer+msi->sendBufferSize)) = '\0';
	*pcb = cb;
    return 0;
}

TCHAR *GetRichEditSelection(HWND hwnd) {
	CHARRANGE sel;
	SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin!=sel.cpMax) {
		struct MessageSendInfo msi;
		EDITSTREAM stream;
		DWORD dwFlags = 0;
		ZeroMemory(&stream, sizeof(stream));
		stream.pfnCallback = StreamOutCallback;
		stream.dwCookie = (DWORD) &msi;
#if defined( _UNICODE )
		dwFlags = SF_TEXT|SF_UNICODE|SFF_SELECTION;
#else
		dwFlags = SF_TEXT|SFF_SELECTION;
#endif
		msi.sendBuffer = NULL;
		msi.sendBufferSize = 0;
		SendMessage(hwnd, EM_STREAMOUT, (WPARAM)dwFlags, (LPARAM) & stream);
		return (TCHAR *)msi.sendBuffer;
	}
	return NULL;
}

static TCHAR *GetIEViewSelection(struct MessageWindowData *dat) {
	IEVIEWEVENT event;
	ZeroMemory(&event, sizeof(event));
	event.cbSize = sizeof(event);
#ifdef _UNICODE
	event.dwFlags = 0;
#else
	event.dwFlags = IEEF_NO_UNICODE;
#endif
	event.codepage = dat->codePage;
	event.hwnd = dat->hwndLog;
	event.hContact = dat->hContact;
	event.iType = IEE_GET_SELECTION;
	return mir_tstrdup((TCHAR *)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event));
}

static TCHAR *GetQuotedTextW(TCHAR * text) {
	int i, j, l, newLine, wasCR;
	TCHAR *out;
#ifdef _UNICODE
	l = wcslen(text);
#else
	l = strlen(text);
#endif
	newLine = 1;
	wasCR = 0;
	for (i=j=0; i<l; i++) {
		if (text[i]=='\r') {
			wasCR = 1;
			newLine = 1;
			j += text[i+1]!='\n' ? 2 : 1;
		} else if (text[i]=='\n') {
			newLine = 1;
			j += wasCR ? 1 : 2;
			wasCR = 0;
		} else {
			j++;
			if (newLine) {
				//for (;i<l && text[i]=='>';i++) j--;
				j+=2;
			}
			newLine = 0;
			wasCR = 0;
		}
	}
	j+=3;
	out = (TCHAR *)mir_alloc(sizeof(TCHAR) * j);
	newLine = 1;
	wasCR = 0;
	for (i=j=0; i<l; i++) {
		if (text[i]=='\r') {
			wasCR = 1;
			newLine = 1;
			out[j++] = '\r';
			if (text[i+1]!='\n') {
				out[j++]='\n';
			}
		} else if (text[i]=='\n') {
			newLine = 1;
			if (!wasCR) {
				out[j++]='\r';
			}
			out[j++]='\n';
			wasCR = 0;
		} else {
			if (newLine) {
				out[j++]='>';
				out[j++]=' ';
				//for (;i<l && text[i]=='>';i++) j--;
			}
			newLine = 0;
			wasCR = 0;
			out[j++]=text[i];
		}
	}
	out[j++]='\r';
	out[j++]='\n';
	out[j++]='\0';
	return out;
}

static void saveDraftMessage(struct MessageWindowData *dat) {
	TCHAR *textBuffer;
	int textBufferSize;
	textBufferSize = (GetWindowTextLengthA(GetDlgItem(dat->hwnd, IDC_MESSAGE)) + 1);
	if (textBufferSize > 1) {
		textBufferSize *= sizeof(TCHAR);
		textBuffer = (TCHAR *) mir_alloc(textBufferSize);
#if defined( _UNICODE )
		{
			GETTEXTEX  gt;
			gt.cb = textBufferSize;
			gt.flags = GT_USECRLF;
			gt.codepage = 1200;
			SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_GETTEXTEX, (WPARAM) &gt, (LPARAM) textBuffer);
		}
#else
		GetDlgItemTextA(dat->hwnd, IDC_MESSAGE, textBuffer, textBufferSize);
#endif
		g_dat->draftList = tcmdlist_append2(g_dat->draftList, dat->hContact, (TCHAR *) textBuffer);
		mir_free(textBuffer);
	} else {
		g_dat->draftList = tcmdlist_remove2(g_dat->draftList, dat->hContact);
	}
}


static void RemoveSendBuffer(struct MessageWindowData *dat, int i) {
	if (dat->sendInfo[i].sendBuffer) {
 		mir_free(dat->sendInfo[i].sendBuffer);
		dat->sendInfo[i].sendBuffer = NULL;
		dat->sendInfo[i].hSendId = NULL;
		for (i = 0; i < dat->sendCount; i++)
			if (dat->sendInfo[i].sendBuffer)
				break;
		if (i == dat->sendCount) {
			//all messages sent
			dat->sendCount = 0;
			mir_free(dat->sendInfo);
			dat->sendInfo = NULL;
			KillTimer(dat->hwnd, TIMERID_MSGSEND);
		}
	}
}

void NotifyLocalWinEvent(HANDLE hContact, HWND hwnd, unsigned int type) {
	MessageWindowEventData mwe = { 0 };
	BOOL bChat = FALSE;
	if (hContact==NULL || hwnd==NULL) return;
	mwe.cbSize = sizeof(mwe);
	mwe.hContact = hContact;
	mwe.hwndWindow = hwnd;
	mwe.szModule = SRMMMOD;
	mwe.uType = type;
	mwe.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
	bChat = (WindowList_Find(g_dat->hMessageWindowList, hContact) == NULL);
	mwe.hwndInput = GetDlgItem(hwnd, bChat ? IDC_CHAT_MESSAGE : IDC_MESSAGE);
	mwe.hwndLog = GetDlgItem(hwnd, bChat ? IDC_CHAT_LOG : IDC_LOG);
	NotifyEventHooks(hHookWinEvt, 0, (LPARAM)&mwe);
}

static char *MsgServiceName(HANDLE hContact)
{
#ifdef _UNICODE
    char szServiceName[100];
    char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
    if (szProto == NULL)
        return PSS_MESSAGE;

    mir_snprintf(szServiceName, sizeof(szServiceName), "%s%sW", szProto, PSS_MESSAGE);
    if (ServiceExists(szServiceName))
        return PSS_MESSAGE "W";
#endif
    return PSS_MESSAGE;
}

#if defined(_UNICODE)
int RTL_Detect(WCHAR *pszwText)
{
    WORD *infoTypeC2;
    int i;
    int iLen = lstrlenW(pszwText);

    infoTypeC2 = (WORD *)mir_alloc(sizeof(WORD) * (iLen + 2));

    if(infoTypeC2) {
        ZeroMemory(infoTypeC2, sizeof(WORD) * (iLen + 2));

        GetStringTypeW(CT_CTYPE2, pszwText, iLen, infoTypeC2);

        for(i = 0; i < iLen; i++) {
            if(infoTypeC2[i] == C2_RIGHTTOLEFT) {
                mir_free(infoTypeC2);
                return 1;
            }
        }
        mir_free(infoTypeC2);
    }
    return 0;
}
#endif

static void AddToFileList(char ***pppFiles,int *totalCount,const char *szFilename) {
	*pppFiles=(char**)mir_realloc(*pppFiles,(++*totalCount+1)*sizeof(char*));
	(*pppFiles)[*totalCount]=NULL;
	(*pppFiles)[*totalCount-1]=mir_strdup(szFilename);
	if(GetFileAttributesA(szFilename)&FILE_ATTRIBUTE_DIRECTORY) {
		WIN32_FIND_DATAA fd;
		HANDLE hFind;
		char szPath[MAX_PATH];
		lstrcpyA(szPath,szFilename);
		lstrcatA(szPath,"\\*");
		if((hFind=FindFirstFileA(szPath,&fd))) {
			do {
				if(!lstrcmpA(fd.cFileName,".") || !lstrcmpA(fd.cFileName,"..")) continue;
				lstrcpyA(szPath,szFilename);
				lstrcatA(szPath,"\\");
				lstrcatA(szPath,fd.cFileName);
				AddToFileList(pppFiles,totalCount,szPath);
			} while(FindNextFileA(hFind,&fd));
			FindClose(hFind);
		}
	}
}

static int GetToolbarWidth()
{
	int i, w = 0;
	for (i = 0; i < sizeof(buttonLineControls) / sizeof(buttonLineControls[0]); i++) {
//		if (g_dat->buttonVisibility & (1 << i)) {
			if (buttonLineControls[i] != IDC_SMILEYS || g_dat->smileyServiceExists) {
				w += buttonWidth[i] + buttonSpacing[i];
			}
//		}
	}
	return w;
}

static void ShowMultipleControls(HWND hwndDlg, const UINT * controls, int cControls, int state)
{
	int i;
	for (i = 0; i < cControls; i++)
		ShowWindow(GetDlgItem(hwndDlg, controls[i]), (g_dat->buttonVisibility & (1 << i)) ? state : SW_HIDE);
}

static void SetDialogToType(HWND hwndDlg)
{
	struct MessageWindowData *dat;
	ParentWindowData *pdat;

	dat = (struct MessageWindowData *) GetWindowLong(hwndDlg, GWL_USERDATA);
	pdat = (ParentWindowData *) GetWindowLong(GetParent(hwndDlg), GWL_USERDATA);
	if (dat->hContact) {
		ShowMultipleControls(hwndDlg, buttonLineControls, sizeof(buttonLineControls) / sizeof(buttonLineControls[0]), (pdat->flags2&SMF2_SHOWTOOLBAR) ? SW_SHOW : SW_HIDE);
		if (!DBGetContactSettingByte(dat->hContact, "CList", "NotOnList", 0)) {
			ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);
		}
		if (!g_dat->smileyServiceExists) {
			ShowWindow(GetDlgItem(hwndDlg, IDC_SMILEYS), SW_HIDE);
		}
	} else {
		ShowMultipleControls(hwndDlg, buttonLineControls, sizeof(buttonLineControls) / sizeof(buttonLineControls[0]), SW_HIDE);
	}
	ShowWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), SW_SHOW);
	if (dat->hwndLog != NULL) {
		ShowWindow (GetDlgItem(hwndDlg, IDC_LOG), SW_HIDE);
	} else {
		ShowWindow (GetDlgItem(hwndDlg, IDC_LOG), SW_SHOW);
	}
	UpdateReadChars(hwndDlg, dat);
	ShowWindow(GetDlgItem(hwndDlg, IDC_SPLITTER), SW_SHOW);
	EnableWindow(GetDlgItem(hwndDlg, IDOK), GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE))?TRUE:FALSE);
	SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
	SendMessage(hwndDlg, WM_SIZE, 0, 0);
}

struct SavedMessageData
{
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	DWORD keyStates;            //use MOD_ defines from RegisterHotKey()
};

struct MsgEditSubclassData
{
	DWORD lastEnterTime;
	struct SavedMessageData *keyboardMsgQueue;
	int	msgQueueCount;
};

static LRESULT CALLBACK LogEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
/*	switch (msg) {

		case WM_CHAR:
			if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
				SetFocus(GetDlgItem(GetParent(hwnd), IDC_MESSAGE));
				SendMessage(GetDlgItem(GetParent(hwnd), IDC_MESSAGE), msg, wParam, lParam);
				return 0;
			}
			break;

		case WM_KEYDOWN:
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				if (wParam == VK_TAB) {	// ctrl-(shift) tab
					if (GetKeyState(VK_SHIFT) & 0x8000) {
						SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEPREV, 0, (LPARAM)GetParent(hwnd));
						return 0;
					} else {
						SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATENEXT, 0, (LPARAM)GetParent(hwnd));
						return 0;
					}
				}
			}
			break;

	}*/
	return CallWindowProc(OldLogEditProc, hwnd, msg, wParam, lParam);
}

                                                  //todo: decide if this should be set or not
static LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct MsgEditSubclassData *dat;
	struct MessageWindowData *pdat;
	SETTEXTEX  st;
	st.flags = ST_DEFAULT;
	#ifdef _UNICODE
		st.codepage = 1200;
	#else
		st.codepage = CP_ACP;
	#endif

	pdat=(struct MessageWindowData *)GetWindowLong(GetParent(hwnd),GWL_USERDATA);
	dat = (struct MsgEditSubclassData *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (msg) {
	case EM_SUBCLASSED:
		dat = (struct MsgEditSubclassData *) mir_alloc(sizeof(struct MsgEditSubclassData));
		SetWindowLong(hwnd, GWL_USERDATA, (LONG) dat);
		dat->lastEnterTime = 0;
		dat->keyboardMsgQueue = NULL;
		dat->msgQueueCount = 0;
		return 0;

	case WM_CHAR:
		{
			BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
			BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
			BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
			if (GetWindowLong(hwnd, GWL_STYLE) & ES_READONLY) {
				break;
			}
			if (wParam == 1 && isCtrl) {      //ctrl-a; select all
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				return 0;
			}
			if (wParam == 12 && isCtrl) {     // ctrl-l; clear log
				SendMessage(GetParent(hwnd), DM_CLEARLOG, 0, 0);
				return 0;
			}
			if (wParam == 23 && isCtrl) {     // ctrl-w; close
				SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
				return 0;
			}
			if (wParam == 20 && isCtrl && isShift) {     // ctrl-shift-t
				SendMessage(GetParent(GetParent(hwnd)), DM_SWITCHTOOLBAR, 0, 0);
				return 0;
			}
			if (wParam == 18 && isCtrl && isShift) {     // ctrl-shift-r
				SendMessage(GetParent(hwnd), DM_SWITCHRTL, 0, 0);
				return 0;
			}
			if (wParam == 19 && isCtrl && isShift) {     // ctrl-shift-s
				SendMessage(GetParent(GetParent(hwnd)), DM_SWITCHSTATUSBAR, 0, 0);
				return 0;
			}
			if (wParam == 13 && isCtrl && isShift) {     // ctrl-shift-m
				SendMessage(GetParent(GetParent(hwnd)), DM_SWITCHTITLEBAR, 0, 0);
				return 0;
			}
		}
		break;
	case WM_KEYUP:
		break;
	case WM_KEYDOWN:
		{
			BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
			BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
			BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
			if (wParam == VK_UP && isCtrl && (g_dat->flags & SMF_CTRLSUPPORT) && !DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AUTOCLOSE, SRMSGDEFSET_AUTOCLOSE)) {
				if (pdat->cmdList) {
					if (!pdat->cmdListCurrent) {
						saveDraftMessage(pdat);
						pdat->cmdListCurrent = pdat->cmdListNew = tcmdlist_last(pdat->cmdList);
					//	SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
						SendMessage(hwnd, EM_SETTEXTEX, (WPARAM) &st, (LPARAM)pdat->cmdListCurrent->szCmd);
						SendMessage(hwnd, EM_SCROLLCARET, 0,0);
					//	SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
						//SendMessage(hwnd, EM_SETSEL, 0, -1);
					}
					else if (pdat->cmdListCurrent->prev) {
						pdat->cmdListCurrent = pdat->cmdListNew = pdat->cmdListCurrent->prev;
					//	SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
						SendMessage(hwnd, EM_SETTEXTEX, (WPARAM) &st, (LPARAM)pdat->cmdListCurrent->szCmd);
						SendMessage(hwnd, EM_SCROLLCARET, 0,0);
					//	SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
						//SendMessage(hwnd, EM_SETSEL, 0, -1);
					}
				}
				EnableWindow(GetDlgItem(GetParent(hwnd), IDOK), GetWindowTextLength(GetDlgItem(GetParent(hwnd), IDC_MESSAGE)) != 0);
				UpdateReadChars(GetParent(hwnd), pdat);
				return 0;
			}
			else if (wParam == VK_DOWN && isCtrl && (g_dat->flags & SMF_CTRLSUPPORT) && !DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AUTOCLOSE, SRMSGDEFSET_AUTOCLOSE)) {
				if (pdat->cmdList) {
					if (pdat->cmdListCurrent) {
						pdat->cmdListCurrent = pdat->cmdListNew = pdat->cmdListCurrent->next;
						if (!pdat->cmdListCurrent) {
							pdat->cmdListCurrent = tcmdlist_get2(g_dat->draftList, pdat->hContact);
						}
						if (pdat->cmdListCurrent) {
							//SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
							SendMessage(hwnd, EM_SETTEXTEX, (WPARAM) &st, (LPARAM)pdat->cmdListCurrent->szCmd);
							SendMessage(hwnd, EM_SCROLLCARET, 0,0);
							//SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
							//SendMessage(hwnd, EM_SETSEL, 0, -1);
						} else {
							pdat->cmdListCurrent = 0;
							SetWindowText(hwnd, _T(""));
						}
					}
				}
				EnableWindow(GetDlgItem(GetParent(hwnd), IDOK), GetWindowTextLength(GetDlgItem(GetParent(hwnd), IDC_MESSAGE)) != 0);
				UpdateReadChars(GetParent(hwnd), pdat);
				return 0;
			}
			/*
			if(wParam == VK_INSERT && isShift) {
				SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0); // shift insert
				return 0;
			}*/
			if (wParam == VK_TAB && isCtrl && isShift) { // ctrl-shift tab
				SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEPREV, 0, (LPARAM)GetParent(hwnd));
				return 0;
			}
			if (isCtrl && !isAlt) {
	/*
				if (wParam == 'V') {    // ctrl v
					SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
					return 0;
				}
				*/
				if (wParam == VK_TAB) { // ctrl tab
					SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATENEXT, 0, (LPARAM)GetParent(hwnd));
					return 0;
				}
				if (wParam == VK_PRIOR) { // page up
					SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEPREV, 0, (LPARAM)GetParent(hwnd));
					return 0;
				}
				if (wParam == VK_NEXT) { // page down
					SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATENEXT, 0, (LPARAM)GetParent(hwnd));
					return 0;
				}
			}
			if (wParam == VK_F4 && isCtrl && !isShift) {
				SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
				return 0;
			}
			if(wParam == VK_ESCAPE && isShift) {
				ShowWindow(GetParent(GetParent(hwnd)), SW_MINIMIZE);
				return 0;
			}
			if (wParam == VK_RETURN) {
				if (isCtrl && isShift) {
					PostMessage(GetParent(hwnd), WM_COMMAND, IDC_SENDALL, 0);
					return 0;
				}
				if ((isCtrl != 0) ^ (0 != (g_dat->flags & SMF_SENDONENTER))) {
					PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
					return 0;
				}
				if (g_dat->flags & SMF_SENDONDBLENTER) {
					if (dat->lastEnterTime + ENTERCLICKTIME < GetTickCount())
						dat->lastEnterTime = GetTickCount();
					else {
						SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
						SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
						PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
						return 0;
					}
				}
			}
			else
				dat->lastEnterTime = 0;
		}
		break;
		//fall through
	case WM_MOUSEWHEEL:
		if ((GetWindowLong(hwnd, GWL_STYLE) & WS_VSCROLL) == 0) {
			SendMessage(GetDlgItem(GetParent(hwnd), IDC_LOG), WM_MOUSEWHEEL, wParam, lParam);
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_KILLFOCUS:
		dat->lastEnterTime = 0;
		break;
	case WM_SYSKEYDOWN:
		if ((wParam == VK_LEFT) && GetKeyState(VK_MENU) & 0x8000) {
			SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEPREV, 0, (LPARAM)GetParent(hwnd));
			return 0;
		}
		if ((wParam == VK_RIGHT) && GetKeyState(VK_MENU) & 0x8000) {
			SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATENEXT, 0, (LPARAM)GetParent(hwnd));
			return 0;
		}
		break;
	case WM_SYSKEYUP:
		if ((wParam == VK_LEFT) && GetKeyState(VK_MENU) & 0x8000) {
			return 0;
		}
		if ((wParam == VK_RIGHT) && GetKeyState(VK_MENU) & 0x8000) {
			return 0;
		}
		break;
	case WM_SYSCHAR:
		dat->lastEnterTime = 0;
		if ((wParam == 's' || wParam == 'S') && GetKeyState(VK_MENU) & 0x8000) {
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		break;
	case WM_DROPFILES:
		SendMessage(GetParent(hwnd), WM_DROPFILES, wParam, lParam);
		return 0;
	case WM_CONTEXTMENU:
		{
			HMENU hMenu, hSubMenu;
			POINT pt;
			CHARRANGE sel, all = { 0, -1 };
			MessageWindowPopupData mwpd;
			int selection;

			hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
			hSubMenu = GetSubMenu(hMenu, 2);
			CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) & sel);
			if (sel.cpMin == sel.cpMax) {
				EnableMenuItem(hSubMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hSubMenu, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);
			}
			if (!SendMessage(hwnd, EM_CANUNDO, 0, 0)) {
				EnableMenuItem(hSubMenu, IDM_UNDO, MF_BYCOMMAND | MF_GRAYED);
			}
			if (!SendMessage(hwnd, EM_CANREDO, 0, 0)) {
				EnableMenuItem(hSubMenu, IDM_REDO, MF_BYCOMMAND | MF_GRAYED);
			}
			if (!SendMessage(hwnd, EM_CANPASTE, 0, 0)) {
				EnableMenuItem(hSubMenu, IDM_PASTE, MF_BYCOMMAND | MF_GRAYED);
			}
			if (lParam == 0xFFFFFFFF) {
				SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM) & pt, (LPARAM) sel.cpMax);
				ClientToScreen(hwnd, &pt);
			}
			else {
				pt.x = (short) LOWORD(lParam);
				pt.y = (short) HIWORD(lParam);
			}

			// First notification
			mwpd.cbSize = sizeof(mwpd);
			mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
			mwpd.uFlags = MSG_WINDOWPOPUP_INPUT;
			mwpd.hContact = pdat->hContact;
			mwpd.hwnd = hwnd;
			mwpd.hMenu = hSubMenu;
			mwpd.selection = 0;
			mwpd.pt = pt;
			NotifyEventHooks(hHookWinPopup, 0, (LPARAM)&mwpd);

			selection = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, GetParent(hwnd), NULL);

			// Second notification
			mwpd.selection = selection;
			mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
			NotifyEventHooks(hHookWinPopup, 0, (LPARAM)&mwpd);

			switch (selection) {
			case IDM_UNDO:
				SendMessage(hwnd, WM_UNDO, 0, 0);
				break;
			case IDM_REDO:
				SendMessage(hwnd, EM_REDO, 0, 0);
				break;
			case IDM_CUT:
				SendMessage(hwnd, WM_CUT, 0, 0);
				break;
			case IDM_COPY:
				SendMessage(hwnd, WM_COPY, 0, 0);
				break;
			case IDM_PASTE:
				SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
				break;
			case IDM_DELETE:
				SendMessage(hwnd, EM_REPLACESEL, TRUE, 0);
				break;
			case IDM_SELECTALL:
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & all);
				break;
			}

			DestroyMenu(hMenu);
			return TRUE;
		}
	case EM_UNSUBCLASSED:
		if (dat->keyboardMsgQueue)
			mir_free(dat->keyboardMsgQueue);
		mir_free(dat);
		return 0;
	}
	return CallWindowProc(OldMessageEditProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_NCHITTEST:
		  return HTCLIENT;
		case WM_SETCURSOR:
		{
			RECT rc;
				GetClientRect(hwnd, &rc);
				SetCursor(rc.right > rc.bottom ? hCurSplitNS : hCurSplitWE);
				return TRUE;
		}
		case WM_LBUTTONDOWN:
			SetCapture(hwnd);
			return 0;
		case WM_MOUSEMOVE:
			if (GetCapture() == hwnd) {
				RECT rc;
				GetClientRect(hwnd, &rc);
				SendMessage(GetParent(hwnd), DM_SPLITTERMOVED, rc.right > rc.bottom ? (short) HIWORD(GetMessagePos()) + rc.bottom / 2 : (short) LOWORD(GetMessagePos()) + rc.right / 2, (LPARAM) hwnd);
			}
			return 0;
		case WM_LBUTTONUP:
			ReleaseCapture();
			return 0;
	}
	return CallWindowProc(OldSplitterProc, hwnd, msg, wParam, lParam);
}

static void SubclassMessageEdit(HWND hwnd) {
	OldMessageEditProc = (WNDPROC) SetWindowLong(hwnd, GWL_WNDPROC, (LONG) MessageEditSubclassProc);
	SendMessage(hwnd, EM_SUBCLASSED, 0, 0);
}

static void UnsubclassMessageEdit(HWND hwnd) {
	SendMessage(hwnd, EM_UNSUBCLASSED, 0, 0);
	SetWindowLong(hwnd, GWL_WNDPROC, (LONG) OldMessageEditProc);
}

static void SubclassLogEdit(HWND hwnd) {
	OldLogEditProc = (WNDPROC) SetWindowLong(hwnd, GWL_WNDPROC, (LONG) LogEditSubclassProc);
	SendMessage(hwnd, EM_SUBCLASSED, 0, 0);
}

static void UnsubclassLogEdit(HWND hwnd) {
	SendMessage(hwnd, EM_UNSUBCLASSED, 0, 0);
	SetWindowLong(hwnd, GWL_WNDPROC, (LONG) OldLogEditProc);
}

static void MessageDialogResize(HWND hwndDlg, struct MessageWindowData *dat, int w, int h) {
	HDWP hdwp;
	ParentWindowData *pdat = dat->parent;
	int i, lPos, rPos, vPos;
	int vSplitterPos = 0, hSplitterPos = dat->splitterPos, toolbarHeight = pdat->flags2&SMF2_SHOWTOOLBAR ? dat->toolbarSize.cy : 0;
	int hSplitterMinTop = toolbarHeight + dat->minLogBoxHeight, hSplitterMinBottom = dat->minEditBoxHeight;

	if (h-hSplitterPos < hSplitterMinTop) {
		hSplitterPos = h - hSplitterMinTop;
	}
	if (hSplitterPos < hSplitterMinBottom) {
		hSplitterPos = hSplitterMinBottom;
	}
	dat->splitterPos = hSplitterPos;
	SendMessage(hwndDlg, DM_AVATARCALCSIZE, 0, 0);
	if (hSplitterPos + toolbarHeight < dat->avatarHeight) {
		hSplitterPos = dat->avatarHeight - toolbarHeight;
	}
	dat->splitterPos = hSplitterPos;
	vPos = h - hSplitterPos - toolbarHeight + 1;
	hdwp = BeginDeferWindowPos(12);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_LOG), 0, 0, 0, w-vSplitterPos, h-hSplitterPos-toolbarHeight-1, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_MESSAGE), 0, 0, h-hSplitterPos+2, w-(dat->avatarWidth ? dat->avatarWidth+1 : 0), hSplitterPos-2, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_SPLITTER), 0, 0, h - hSplitterPos-1, w-dat->avatarWidth, 3, SWP_NOZORDER);
	lPos = 0;
	if (h - dat->avatarHeight > vPos + 24) {
		rPos = w;
	} else {
		rPos = w - dat->avatarWidth;
	}
	for (i = 0; i < sizeof(buttonLineControls) / sizeof(buttonLineControls[0]); i++) {
		if (!buttonAlignment[i] && (g_dat->buttonVisibility & (1 << i))) {
			lPos += buttonSpacing[i];
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, buttonLineControls[i]), 0, lPos, vPos, buttonWidth[i], 24, SWP_NOZORDER);
			lPos += buttonWidth[i];
		}
	}
	for (i = sizeof(buttonLineControls) / sizeof(buttonLineControls[0]) - 1; i >=0; i--) {
		if (buttonAlignment[i] && (g_dat->buttonVisibility & (1 << i))) {
			rPos -= buttonSpacing[i] + buttonWidth[i];
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, buttonLineControls[i]), 0, rPos, vPos, buttonWidth[i], 24, SWP_NOZORDER);
		}
	}
	/*
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_USERMENU), 0, 0, vPos, 24, 24, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_DETAILS), 0, 24, vPos, 24, 24, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_SMILEYS), 0, 60, vPos, 24, 24, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_ADD), 0, rPos-4*24-38, vPos, 24, 24, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_HISTORY), 0, rPos-3*24-38, vPos, 24, 24, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_QUOTE), 0, rPos-2*24-38, vPos, 24, 24, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDCANCEL), 0, rPos-24-38, vPos, 24, 24, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDOK), 0, rPos-38, vPos, 38, 24, SWP_NOZORDER);
*/
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_AVATAR), 0, w-dat->avatarWidth, h - dat->avatarHeight, dat->avatarWidth, dat->avatarHeight, SWP_NOZORDER);
//	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_AVATAR), 0, w-dat->avatarWidth, h - (hSplitterPos + toolbarHeight + dat->avatarHeight)/2, dat->avatarWidth, dat->avatarHeight, SWP_NOZORDER);
	EndDeferWindowPos(hdwp);
	if (dat->hwndLog != NULL) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_SETPOS;
		ieWindow.parent = hwndDlg;
		ieWindow.hwnd = dat->hwndLog;
        ieWindow.x = 0;
        ieWindow.y = 0;
        ieWindow.cx = w-vSplitterPos;
        ieWindow.cy = h-hSplitterPos - toolbarHeight - 1;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
	} else {
		RedrawWindow(GetDlgItem(hwndDlg, IDC_LOG), NULL, NULL, RDW_INVALIDATE);
	}
	RedrawWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), NULL, NULL, RDW_INVALIDATE);
	if (g_dat->flags&SMF_AVATAR && IsWindowVisible(GetDlgItem(hwndDlg, IDC_AVATAR))) {
		RedrawWindow(GetDlgItem(hwndDlg, IDC_AVATAR), NULL, NULL, RDW_INVALIDATE);
	}
}

static void UpdateReadChars(HWND hwndDlg, struct MessageWindowData * dat)
{
	if (dat->parent->hwndActive == hwndDlg) {
		TCHAR szText[256];
		StatusBarData sbd;
		int len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE));
		sbd.iItem = 1;
		sbd.iFlags = SBDF_TEXT | SBDF_ICON;
		sbd.hIcon = NULL;
		sbd.pszText = szText;
		mir_sntprintf(szText, SIZEOF(szText), _T("%d"), len);
		SendMessage(dat->hwndParent, CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)hwndDlg);
	}
}

void ShowAvatar(HWND hwndDlg, struct MessageWindowData *dat) {
	DBVARIANT dbv;

	if (g_dat->avatarServiceExists) {
		if (dat->ace != NULL) {
			dat->avatarPic = (dat->ace->dwFlags & AVS_HIDEONCLIST) ? NULL : dat->ace->hbmPic;
		} else {
			dat->avatarPic = NULL;
		}
	} else {
		if (dat->avatarPic)  {
			DeleteObject(dat->avatarPic);
			dat->avatarPic=0;
		}
		if (!DBGetContactSetting(dat->hContact, SRMMMOD, SRMSGSET_AVATAR, &dbv)) {
			HANDLE hFile;
			char tmpPath[MAX_PATH];
			/* relative to absolute here */
			strcpy(tmpPath, dbv.pszVal);
			if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)) {
				CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
			}
			DBFreeVariant(&dbv);
			if((hFile = CreateFileA(tmpPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE) {
				dat->avatarPic=(HBITMAP)CallService(MS_UTILS_LOADBITMAP,0,(LPARAM)tmpPath);
				CloseHandle(hFile);
			}
		}
	}
	SendMessage(hwndDlg, DM_AVATARCALCSIZE, 0, 0);
	SendMessage(hwndDlg, DM_SCROLLLOGTOBOTTOM, 0, 0);
	SendMessage(hwndDlg, WM_SIZE, 0, 0);
}

static void NotifyTyping(struct MessageWindowData *dat, int mode)
{
	DWORD protoStatus;
	DWORD protoCaps;
	DWORD typeCaps;

	if (!dat->hContact)
		return;
	// Don't send to protocols who don't support typing
	// Don't send to users who are unchecked in the typing notification options
	// Don't send to protocols that are offline
	// Don't send to users who are not visible and
	// Don't send to users who are not on the visible list when you are in invisible mode.
	if (!DBGetContactSettingByte(dat->hContact, SRMMMOD, SRMSGSET_TYPING, DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)))
		return;
	if (!dat->szProto)
		return;
	protoStatus = CallProtoService(dat->szProto, PS_GETSTATUS, 0, 0);
	protoCaps = CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	typeCaps = CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_4, 0);

	if (!(typeCaps & PF4_SUPPORTTYPING))
		return;
	if (protoStatus < ID_STATUS_ONLINE)
		return;
	if (protoCaps & PF1_VISLIST && DBGetContactSettingWord(dat->hContact, dat->szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
		return;
	if (protoCaps & PF1_INVISLIST && protoStatus == ID_STATUS_INVISIBLE && DBGetContactSettingWord(dat->hContact, dat->szProto, "ApparentMode", 0) != ID_STATUS_ONLINE)
		return;
	if (DBGetContactSettingByte(dat->hContact, "CList", "NotOnList", 0)
		&& !DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
		return;
	// End user check
	dat->nTypeMode = mode;
	CallService(MS_PROTO_SELFISTYPING, (WPARAM) dat->hContact, dat->nTypeMode);
}


static int MeasureMenuItem(WPARAM wParam, LPARAM lParam)
{
	LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT) lParam;
	if (mis->itemData != 0xDEAD) {
		return FALSE;
	}
	mis->itemWidth = max(0, GetSystemMetrics(SM_CXSMICON) - GetSystemMetrics(SM_CXMENUCHECK) + 4);
	mis->itemHeight = GetSystemMetrics(SM_CYSMICON) + 2;
	return TRUE;
}


static int DrawMenuItem(WPARAM wParam, LPARAM lParam)
{
	int y;
	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;
	if (dis->itemData != 0xDEAD) {
		return FALSE;
	}
	y = (dis->rcItem.bottom - dis->rcItem.top - GetSystemMetrics(SM_CYSMICON)) / 2 + 1;
	if (dis->itemState & ODS_SELECTED) {
		if (dis->itemState & ODS_CHECKED) {
			RECT rc;
			rc.left = 2;
			rc.right = GetSystemMetrics(SM_CXSMICON) + 2;
			rc.top = y;
			rc.bottom = rc.top + GetSystemMetrics(SM_CYSMICON) + 2;
			FillRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
			ImageList_DrawEx(g_dat->hButtonIconList, dis->itemID, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_DEFAULT, ILD_SELECTED);
		} else
			ImageList_DrawEx(g_dat->hButtonIconList, dis->itemID, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_DEFAULT, ILD_FOCUS);
	} else {
		if (dis->itemState & ODS_CHECKED) {
			HBRUSH hBrush;
			RECT rc;
			COLORREF menuCol, hiliteCol;
			rc.left = 0;
			rc.right = GetSystemMetrics(SM_CXSMICON) + 4;
			rc.top = y - 2;
			rc.bottom = rc.top + GetSystemMetrics(SM_CYSMICON) + 4;
			DrawEdge(dis->hDC, &rc, BDR_SUNKENOUTER, BF_RECT);
			InflateRect(&rc, -1, -1);
			menuCol = GetSysColor(COLOR_MENU);
			hiliteCol = GetSysColor(COLOR_3DHIGHLIGHT);
			hBrush = CreateSolidBrush(RGB
				((GetRValue(menuCol) + GetRValue(hiliteCol)) / 2, (GetGValue(menuCol) + GetGValue(hiliteCol)) / 2,
				(GetBValue(menuCol) + GetBValue(hiliteCol)) / 2));
			FillRect(dis->hDC, &rc, hBrush);
			DeleteObject(hBrush);
			ImageList_DrawEx(g_dat->hButtonIconList, dis->itemID, dis->hDC, 2, y, 0, 0, CLR_NONE, GetSysColor(COLOR_MENU), ILD_BLEND25);
		} else
			ImageList_DrawEx(g_dat->hButtonIconList, dis->itemID, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
	}
	return TRUE;
}

static BOOL CALLBACK ConfirmSendAllDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			RECT rcParent, rcChild;
			GetWindowRect(GetParent(hwndDlg), &rcParent);
			GetWindowRect(hwndDlg, &rcChild);
			rcChild.bottom -= rcChild.top;
			rcChild.right -= rcChild.left;
			rcParent.bottom -= rcParent.top;
			rcParent.right -= rcParent.left;
			rcChild.left = rcParent.left + (rcParent.right - rcChild.right) / 2;
			rcChild.top = rcParent.top + (rcParent.bottom - rcChild.bottom) / 2;
			MoveWindow(hwndDlg, rcChild.left, rcChild.top, rcChild.right, rcChild.bottom, FALSE);
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDYES:
		case IDNO:
			{
				int result = LOWORD(wParam);
				if (IsDlgButtonChecked(hwndDlg, IDC_REMEMBER)) {
					result |= 0x10000;
				}
				EndDialog(hwndDlg, result);
			}
			return TRUE;
		}
		break;
	}

	return FALSE;
}


BOOL CALLBACK DlgProcMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HMENU hToolbarMenu;
	struct MessageWindowData *dat;
	dat = (struct MessageWindowData *) GetWindowLong(hwndDlg, GWL_USERDATA);
	if (!dat && msg!=WM_INITDIALOG) return FALSE;
	switch (msg) {
	case WM_INITDIALOG:
		{
			int len;
			int notifyUnread = 0;
			NewMessageWindowLParam *newData = (NewMessageWindowLParam *) lParam;
			//TranslateDialogDefault(hwndDlg);
			dat = (struct MessageWindowData *) mir_alloc(sizeof(struct MessageWindowData));
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) dat);
			dat->hContact = newData->hContact;
			NotifyLocalWinEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_OPENING);
//			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETTEXTMODE, TM_PLAINTEXT, 0);

			if (newData->szInitialText) {
	#if defined(_UNICODE)
				if(newData->isWchar)
					SetDlgItemText(hwndDlg, IDC_MESSAGE, (TCHAR *)newData->szInitialText);
				else
					SetDlgItemTextA(hwndDlg, IDC_MESSAGE, newData->szInitialText);
	#else
				SetDlgItemTextA(hwndDlg, IDC_MESSAGE, newData->szInitialText);
	#endif
			} else if (g_dat->flags & SMF_SAVEDRAFTS) {
				TCmdList *draft = tcmdlist_get2(g_dat->draftList, dat->hContact);
				if (draft != NULL) {
					SetDlgItemText(hwndDlg, IDC_MESSAGE, draft->szCmd);
				}
				len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE));
				PostMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETSEL, len, len);
			}
			dat->hwnd = hwndDlg;
			dat->hwndParent = GetParent(hwndDlg);
			dat->parent = (ParentWindowData *) GetWindowLong(dat->hwndParent, GWL_USERDATA);
			dat->hwndLog = NULL;
			dat->szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) dat->hContact, 0);
			dat->avatarPic = 0;
			if (dat->hContact && dat->szProto != NULL)
				dat->wStatus = DBGetContactSettingWord(dat->hContact, dat->szProto, "Status", ID_STATUS_OFFLINE);
			else
				dat->wStatus = ID_STATUS_OFFLINE;
			dat->wOldStatus = dat->wStatus;
			dat->sendInfo = NULL;
			dat->hDbEventFirst = NULL;
			dat->hDbEventLast = NULL;
//			dat->sendBuffer = NULL;
			dat->sendCount = 0;
			dat->messagesInProgress = 0;
//			dat->nFlash = 0;
			dat->nTypeSecs = 0;
			dat->nLastTyping = 0;
			dat->showTyping = 0;
			dat->showUnread = 0;
			dat->cmdList = 0;
			dat->cmdListCurrent = 0;
			dat->cmdListNew = 0;
			dat->sendAllConfirm = 0;
			dat->messagesInProgress = 0;
			dat->nTypeMode = PROTOTYPE_SELFTYPING_OFF;
			SetTimer(hwndDlg, TIMERID_TYPE, 1000, NULL);
			dat->lastMessage = 0;
			dat->lastEventType = -1;
			dat->lastEventTime = time(NULL);
			dat->startTime = time(NULL);
			dat->userMenuIcon = NULL;
			dat->flags = 0;
			if (DBGetContactSettingByte(dat->hContact, SRMMMOD, "UseRTL", (BYTE) 0)) {
				dat->flags |= SMF_RTL;
			}
			if (DBGetContactSettingByte(dat->hContact, SRMMMOD, "DisableUnicode", (BYTE) 0)) {
				dat->flags |= SMF_DISABLE_UNICODE;
			}
			dat->flags |= ServiceExists(MS_IEVIEW_WINDOW) ? g_dat->flags & SMF_USEIEVIEW : 0;
			{
				PARAFORMAT2 pf2;
				ZeroMemory((void *)&pf2, sizeof(pf2));
				pf2.cbSize = sizeof(pf2);
				pf2.dwMask = PFM_RTLPARA;
				pf2.wEffects = 0;
				if (!(dat->flags & SMF_RTL)) {
//					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
					SetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE) & ~(WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));
				} else {
					pf2.wEffects = PFE_RTLPARA;
					SetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
				}
				SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
				/* Workaround to make Richedit display RTL messages correctly */
				ZeroMemory((void *)&pf2, sizeof(pf2));
				pf2.cbSize = sizeof(pf2);
				pf2.dwMask = PFM_RTLPARA | PFM_OFFSET | PFM_OFFSETINDENT ;
                pf2.wEffects = PFE_RTLPARA;
                pf2.dxOffset = 30;
                pf2.dxStartIndent = 30;
                SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
				pf2.wEffects = 0;
				SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
				if (dat->flags & SMF_RTL) {
					SetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE) | WS_EX_LEFTSCROLLBAR);
				} else {
					SetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE) & ~WS_EX_LEFTSCROLLBAR);
				}
			}
			dat->codePage = DBGetContactSettingWord(dat->hContact, SRMMMOD, "CodePage", (WORD) CP_ACP);
			dat->ace = NULL;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_MESSAGE), &dat->minEditInit);
			dat->minEditBoxHeight = dat->minEditInit.bottom - dat->minEditInit.top;
			dat->minLogBoxHeight = dat->minEditBoxHeight;
			dat->splitterPos = (int) DBGetContactSettingDword((g_dat->flags & SMF_SAVESPLITTERPERCONTACT) ? dat->hContact : NULL, SRMMMOD, "splitterPos", (DWORD) - 1);
//			dat->nFlashMax = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT);
				dat->toolbarSize.cy = 24 + 2;//rc.bottom - rc.top + 3;
			dat->toolbarSize.cx = GetToolbarWidth();
			if (dat->splitterPos == -1) {
				dat->splitterPos = dat->minEditBoxHeight;
			}
			WindowList_Add(g_dat->hMessageWindowList, hwndDlg, dat->hContact);

			SendMessage(hwndDlg, DM_CHANGEICONS, 0, 0);
			// Make them flat buttons
			{
				int i;
				for (i = 0; i < sizeof(buttonLineControls) / sizeof(buttonLineControls[0]); i++)
					SendMessage(GetDlgItem(hwndDlg, buttonLineControls[i]), BUTTONSETASFLATBTN, 0, 0);
			}
			SendMessage(GetDlgItem(hwndDlg, IDC_ADD), BUTTONADDTOOLTIP, (WPARAM) Translate("Add Contact Permanently to List"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_USERMENU), BUTTONADDTOOLTIP, (WPARAM) Translate("User Menu"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_DETAILS), BUTTONADDTOOLTIP, (WPARAM) Translate("View User's Details"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_HISTORY), BUTTONADDTOOLTIP, (WPARAM) Translate("View User's History"), 0);

			SendMessage(GetDlgItem(hwndDlg, IDC_QUOTE), BUTTONADDTOOLTIP, (WPARAM) Translate("Quote Text"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SMILEYS), BUTTONADDTOOLTIP, (WPARAM) Translate("Insert Emoticon"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDOK), BUTTONADDTOOLTIP, (WPARAM) Translate("Send Message"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDCANCEL), BUTTONADDTOOLTIP, (WPARAM) Translate("Close Session"), 0);

			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETOLECALLBACK, 0, (LPARAM) & reOleCallback);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_KEYEVENTS);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETLANGOPTIONS, 0, (LPARAM) SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETOLECALLBACK, 0, (LPARAM) & reOleCallback2);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETLANGOPTIONS, 0, (LPARAM) SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETLANGOPTIONS, 0, 0) & ~(IMF_AUTOKEYBOARD | IMF_AUTOFONTSIZEADJUST));
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0,0));
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_CHANGE);
			if (dat->flags & SMF_USEIEVIEW) {
				IEVIEWWINDOW ieWindow;
				ieWindow.cbSize = sizeof(IEVIEWWINDOW);
				ieWindow.iType = IEW_CREATE;
				ieWindow.dwFlags = 0;
				ieWindow.dwMode = IEWM_SCRIVER;
				ieWindow.parent = hwndDlg;
				ieWindow.x = 0;
				ieWindow.y = 0;
				ieWindow.cx = 200;
				ieWindow.cy = 300;
				CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
				dat->hwndLog = ieWindow.hwnd;
				if (dat->hwndLog == NULL) {
					dat->flags ^= SMF_USEIEVIEW;
				}
			}
			/* duh, how come we didnt use this from the start? */
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_AUTOURLDETECT, (WPARAM) TRUE, 0);
			if (dat->hContact) {
				if (dat->szProto) {
					int nMax;
					nMax = CallProtoService(dat->szProto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, (LPARAM) dat->hContact);
					if (nMax)
						SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_LIMITTEXT, (WPARAM) nMax, 0);
				}
			}
			/* get around a lame bug in the Windows template resource code where richedits are limited to 0x7FFF */
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_LIMITTEXT, (WPARAM) sizeof(TCHAR) * 0x7FFFFFFF, 0);
			SubclassLogEdit(GetDlgItem(hwndDlg, IDC_LOG));
			SubclassMessageEdit(GetDlgItem(hwndDlg, IDC_MESSAGE));
			OldSplitterProc = (WNDPROC) SetWindowLong(GetDlgItem(hwndDlg, IDC_SPLITTER), GWL_WNDPROC, (LONG) SplitterSubclassProc);
			if (dat->hContact) {
				int historyMode = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY);
				// This finds the first message to display, it works like shit
				dat->hDbEventFirst = (HANDLE) CallService(MS_DB_EVENT_FINDFIRSTUNREAD, (WPARAM) dat->hContact, 0);
				if (dat->hDbEventFirst != NULL) {
					DBEVENTINFO dbei = { 0 };
					dbei.cbSize = sizeof(dbei);
					CallService(MS_DB_EVENT_GET, (WPARAM) dat->hDbEventFirst, (LPARAM) & dbei);
					if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_READ) && !(dbei.flags & DBEF_SENT)) {
						notifyUnread = 1;
					}
				}
				switch (historyMode) {
				case LOADHISTORY_COUNT:
					{
						int i;
						HANDLE hPrevEvent;
						DBEVENTINFO dbei = { 0 };
						dbei.cbSize = sizeof(dbei);
						for (i = DBGetContactSettingWord(NULL, SRMMMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT); i > 0; i--) {
							if (dat->hDbEventFirst == NULL)
								hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDLAST, (WPARAM) dat->hContact, 0);
							else
								hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) dat->hDbEventFirst, 0);
							if (hPrevEvent == NULL)
								break;
							dbei.cbBlob = 0;
							dat->hDbEventFirst = hPrevEvent;
							CallService(MS_DB_EVENT_GET, (WPARAM) dat->hDbEventFirst, (LPARAM) & dbei);
							if (!DbEventIsShown(&dbei, dat))
								i++;
						}
						break;
					}
				case LOADHISTORY_TIME:
					{
						HANDLE hPrevEvent;
						DBEVENTINFO dbei = { 0 };
						DWORD firstTime;

						dbei.cbSize = sizeof(dbei);
						if (dat->hDbEventFirst == NULL) {
							dbei.timestamp = time(NULL);
							hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDLAST, (WPARAM) dat->hContact, 0);
						} else {
							CallService(MS_DB_EVENT_GET, (WPARAM) dat->hDbEventFirst, (LPARAM) & dbei);
							hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) dat->hDbEventFirst, 0);
						}
						firstTime = dbei.timestamp - 60 * DBGetContactSettingWord(NULL, SRMMMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME);
						for (;;) {
							if (hPrevEvent == NULL)
								break;
							dbei.cbBlob = 0;
							CallService(MS_DB_EVENT_GET, (WPARAM) hPrevEvent, (LPARAM) & dbei);
							if (dbei.timestamp < firstTime)
								break;
							if (DbEventIsShown(&dbei, dat))
								dat->hDbEventFirst = hPrevEvent;
							hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) hPrevEvent, 0);
						}
						break;
					}
				}
			}
			SendMessage(dat->hwndParent, CM_ADDCHILD, (WPARAM) hwndDlg, (LPARAM) dat->hContact);
			SendMessage(hwndDlg, DM_OPTIONSAPPLIED, 0, 0);
			{
				DBEVENTINFO dbei = { 0 };
				HANDLE hdbEvent;

				dbei.cbSize = sizeof(dbei);
				hdbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDLAST, (WPARAM) dat->hContact, 0);
				if (hdbEvent) {
					do {
						ZeroMemory(&dbei, sizeof(dbei));
						dbei.cbSize = sizeof(dbei);
						CallService(MS_DB_EVENT_GET, (WPARAM) hdbEvent, (LPARAM) & dbei);
						if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
							dat->lastMessage = dbei.timestamp;
							SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
							break;
						}
					}
					while ((hdbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) hdbEvent, 0)));
				}
			}
			if (newData->flags & NMWLP_INCOMING) {
				if (dat->parent->childrenCount == 1 && g_dat->flags & SMF_STAYMINIMIZED) {
					SendMessage(dat->hwndParent, CM_ACTIVATECHILD, 0, (LPARAM) hwndDlg);
					SendMessage(dat->hwndParent, DM_DEACTIVATE, 0, 0);
					ShowWindow(dat->hwndParent, SW_SHOWMINNOACTIVE);
				} else {
					ShowWindow(dat->hwndParent, SW_SHOWNA);
					if (dat->parent->childrenCount == 1 ||
						((g_dat->flags2 & SMF2_SWITCHTOACTIVE) && (IsIconic(dat->hwndParent) || GetForegroundWindow() != dat->hwndParent))) {
						SendMessage(dat->hwndParent, CM_ACTIVATECHILD, 0, (LPARAM) hwndDlg);
						if (dat->parent->childrenCount == 1) {
							SetForegroundWindow(dat->hwndParent);
							SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
						}
					}
				}
			} else {
				if (IsIconic(dat->hwndParent)) {
					ShowWindow(dat->hwndParent, SW_SHOWNORMAL);
				} else {
					ShowWindow(dat->hwndParent, SW_SHOW);
				}
				SendMessage(dat->hwndParent, CM_ACTIVATECHILD, 0, (LPARAM) hwndDlg);
				SetForegroundWindow(dat->hwndParent);
				SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
			}
			NotifyLocalWinEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_OPEN);
			if (notifyUnread) {
				SendMessage(dat->hwndParent, CM_STARTFLASHING, 0, 0);
//				if (GetActiveWindow() != dat->hwndParent || GetForegroundWindow() != dat->hwndParent || dat->parent->hwndActive != hwndDlg) {
				if (GetForegroundWindow() != dat->hwndParent || dat->parent->hwndActive != hwndDlg) {
					dat->showUnread = 0;
					SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
				}
			}
			return TRUE;
		}
	case DM_GETCONTEXTMENU:
		{
			HMENU hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM) dat->hContact, 0);
			SetWindowLong(hwndDlg, DWL_MSGRESULT, (LONG)hMenu);
			return TRUE;
		}
	case WM_CONTEXTMENU:
		if (dat->hwndParent == (HWND) wParam) {
			POINT pt;
			HMENU hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM) dat->hContact, 0);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hwndDlg, NULL);
			DestroyMenu(hMenu);
		}
		break;
	case WM_LBUTTONDBLCLK:
		SendMessage(dat->hwndParent, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		break;
	case WM_RBUTTONUP:
		{
			int i;
			POINT pt;
			MENUITEMINFO mii;
			hToolbarMenu = CreatePopupMenu();
			for (i = 0; i < sizeof(buttonLineControls) / sizeof(buttonLineControls[0]); i++) {
				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE | MIIM_DATA | MIIM_BITMAP;
				mii.fType = MFT_STRING;
				mii.fState = (g_dat->buttonVisibility & (1<< i)) ? MFS_CHECKED : MFS_UNCHECKED;
				mii.wID = i + 1;
				mii.dwItemData = 0xDEAD;
				mii.hbmpItem = HBMMENU_CALLBACK;
				mii.dwTypeData = TranslateTS((buttonNames[i]));
				InsertMenuItem(hToolbarMenu, i, TRUE, &mii);
			}
//			CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hToolbarMenu, 0);
			pt.x = (short) LOWORD(GetMessagePos());
			pt.y = (short) HIWORD(GetMessagePos());
			i = TrackPopupMenu(hToolbarMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
			if (i > 0) {
				g_dat->buttonVisibility ^= (1 << (i - 1));
				DBWriteContactSettingDword(NULL, SRMMMOD, SRMSGSET_BUTTONVISIBILITY, g_dat->buttonVisibility);
				WindowList_Broadcast(g_dat->hMessageWindowList, DM_OPTIONSAPPLIED, 0, 0);
			}
			DestroyMenu(hToolbarMenu);
			return TRUE;
		}
	case WM_DROPFILES:
	{
		if (dat->szProto==NULL) break;
		if (!(CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_1,0)&PF1_FILESEND)) break;
		if (dat->wStatus==ID_STATUS_OFFLINE) break;
		if (dat->hContact!=NULL) {
			HDROP hDrop;
			char **ppFiles=NULL;
			char szFilename[MAX_PATH];
			int fileCount,totalCount=0,i;

			hDrop=(HDROP)wParam;
			fileCount=DragQueryFile(hDrop,-1,NULL,0);
			ppFiles=NULL;
			for(i=0;i<fileCount;i++) {
				DragQueryFileA(hDrop, i, szFilename, sizeof(szFilename));
				AddToFileList(&ppFiles, &totalCount, szFilename);
			}
			CallServiceSync(MS_FILE_SENDSPECIFICFILES, (WPARAM)dat->hContact, (LPARAM)ppFiles);
			for(i=0;ppFiles[i];i++) mir_free(ppFiles[i]);
			mir_free(ppFiles);
		}
		break;
	}
	case HM_AVATARACK:
	{
		ACKDATA *pAck = (ACKDATA *)lParam;
		PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)pAck->hProcess;
		if (pAck->hContact!=dat->hContact)
			return 0;
		if (pAck->type != ACKTYPE_AVATAR)
			return 0;
		if (pAck->result == ACKRESULT_STATUS) {
			SendMessage(hwndDlg, DM_GETAVATAR, 0, 0);
			return 0;
		}
		if (pai==NULL)
			return 0;
		if (pAck->result == ACKRESULT_SUCCESS) {
			if (pai->filename&&strlen(pai->filename)&&VALID_AVATAR(pai->format)) {
				DBWriteContactSettingString(dat->hContact, SRMMMOD, SRMSGSET_AVATAR, pai->filename);
				ShowAvatar(hwndDlg, dat);
			}
		} else if (pAck->result == ACKRESULT_FAILED) {
			DBDeleteContactSetting(dat->hContact, SRMMMOD, SRMSGSET_AVATAR);
			SendMessage(hwndDlg, DM_GETAVATAR, 0, 0);
		}
		break;
	}
	case DM_CHANGEICONS:
		SendDlgItemMessage(hwndDlg, IDC_ADD, BM_SETIMAGE, IMAGE_ICON, (LPARAM) g_dat->hIcons[SMF_ICON_ADD]);
		SendDlgItemMessage(hwndDlg, IDC_DETAILS, BM_SETIMAGE, IMAGE_ICON, (LPARAM) g_dat->hIcons[SMF_ICON_USERDETAILS]);
		SendDlgItemMessage(hwndDlg, IDC_HISTORY, BM_SETIMAGE, IMAGE_ICON, (LPARAM) g_dat->hIcons[SMF_ICON_HISTORY]);
		SendDlgItemMessage(hwndDlg, IDC_QUOTE, BM_SETIMAGE, IMAGE_ICON, (LPARAM) g_dat->hIcons[SMF_ICON_QUOTE]);
		SendDlgItemMessage(hwndDlg, IDC_SMILEYS, BM_SETIMAGE, IMAGE_ICON, (LPARAM) g_dat->hIcons[SMF_ICON_SMILEY]);
		SendDlgItemMessage(hwndDlg, IDOK, BM_SETIMAGE, IMAGE_ICON, (LPARAM) g_dat->hIcons[SMF_ICON_SEND]);
		SendDlgItemMessage(hwndDlg, IDCANCEL, BM_SETIMAGE, IMAGE_ICON, (LPARAM) g_dat->hIcons[SMF_ICON_CANCEL]);
		SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
		break;
	case DM_AVATARCALCSIZE:
	{
		BITMAP bminfo;
		int avatarH;
		ParentWindowData *pdat;
		pdat = dat->parent;
		dat->avatarWidth = 0;
		dat->avatarHeight = 0;
		if (dat->avatarPic==0||!(g_dat->flags&SMF_AVATAR)) {
			ShowWindow(GetDlgItem(hwndDlg, IDC_AVATAR), SW_HIDE);
			return 0;
		}
		GetObject(dat->avatarPic, sizeof(bminfo), &bminfo);
		dat->avatarHeight = avatarH = dat->splitterPos + ((pdat->flags2&SMF2_SHOWTOOLBAR) ? dat->toolbarSize.cy : 0);//- 3;
		if (g_dat->flags & SMF_LIMITAVATARH) {
			if (avatarH < g_dat->limitAvatarMinH) {
				avatarH = g_dat->limitAvatarMinH;
			}
			if (avatarH > g_dat->limitAvatarMaxH) {
				avatarH = g_dat->limitAvatarMaxH;
			}
		}
		{
			RECT rc;
			double aspect = 0;
			GetClientRect(hwndDlg, &rc);
			dat->avatarHeight = avatarH;
			aspect = (double)dat->avatarHeight / (double)bminfo.bmHeight;
			dat->avatarWidth = (int)(bminfo.bmWidth * aspect);
			// if edit box width < min then adjust avatarWidth
			if (rc.right - dat->avatarWidth < dat->toolbarSize.cx) {
				dat->avatarWidth = rc.right - dat->toolbarSize.cx;
				if (dat->avatarWidth < 0) dat->avatarWidth = 0;
				aspect = (double)dat->avatarWidth / (double)bminfo.bmWidth;
				dat->avatarHeight = (int)(bminfo.bmHeight * aspect);

			}
			if (rc.bottom - dat->avatarHeight < dat->minLogBoxHeight) {
				dat->avatarHeight = rc.bottom - dat->minLogBoxHeight;
				if (dat->avatarHeight < 0) dat->avatarHeight = 0;
				aspect = (double)dat->avatarHeight / (double)bminfo.bmHeight;
				dat->avatarWidth = (int)(bminfo.bmWidth * aspect);

			}
		}
		ShowWindow(GetDlgItem(hwndDlg, IDC_AVATAR), SW_SHOW);
		break;
	}
	case DM_AVATARCHANGED:
		if ((HANDLE) wParam == NULL) {
			dat->ace = (struct avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)dat->hContact, 0);
			ShowAvatar(hwndDlg, dat);
		} else if (dat->hContact == (HANDLE) wParam) {
			dat->ace = (struct avatarCacheEntry *) lParam;
			ShowAvatar(hwndDlg, dat);
		}
		break;
	case DM_GETAVATAR:
	{
		PROTO_AVATAR_INFORMATION pai;
		int result;
		//Disable avatars
        if (!(g_dat->flags&SMF_AVATAR)) {
			SendMessage(hwndDlg, DM_AVATARCALCSIZE, 0, 0);
			break;
		}
		//Use contact photo
        if (!(CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_4, 0)&PF4_AVATARS)) {
			DBVARIANT dbv;
			if (!DBGetContactSetting(dat->hContact, "ContactPhoto", "File", &dbv)) {
				DBWriteContactSettingString(dat->hContact, SRMMMOD, SRMSGSET_AVATAR, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			ShowAvatar(hwndDlg, dat);
			break;
		}
		if(DBGetContactSettingWord(dat->hContact, dat->szProto, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) {
			ShowAvatar(hwndDlg, dat);
			break;
		}
		ZeroMemory((void *)&pai, sizeof(pai));
		pai.cbSize = sizeof(pai);
		pai.hContact = dat->hContact;
		pai.format = PA_FORMAT_UNKNOWN;
		strcpy(pai.filename, "");
		result = CallProtoService(dat->szProto, PS_GETAVATARINFO, GAIF_FORCE, (LPARAM)&pai);
		if (result==GAIR_SUCCESS) {
			if (VALID_AVATAR(pai.format)) {
				DBVARIANT dbv;
				DBWriteContactSettingString(dat->hContact, SRMMMOD, SRMSGSET_AVATAR, pai.filename);
				if (DBGetContactSetting(dat->hContact, "ContactPhoto", "File", &dbv)) {
					DBWriteContactSettingString(dat->hContact, "ContactPhoto", "File", pai.filename);
				} else {
					DBFreeVariant(&dbv);
				}
			} else DBDeleteContactSetting(dat->hContact, SRMMMOD, SRMSGSET_AVATAR);
			ShowAvatar(hwndDlg, dat);
		} else if (result==GAIR_NOAVATAR) {
			DBVARIANT dbv;
			if (!DBGetContactSetting(dat->hContact, "ContactPhoto", "File", &dbv)) {
				DBWriteContactSettingString(dat->hContact, SRMMMOD, SRMSGSET_AVATAR, dbv.pszVal);
				DBFreeVariant(&dbv);
			} else {
				DBDeleteContactSetting(dat->hContact, SRMMMOD, SRMSGSET_AVATAR);
			}
			ShowAvatar(hwndDlg, dat);
		}
		break;
	}
	case DM_TYPING:
		{
			dat->nTypeSecs = (int) lParam > 0 ? (int) lParam : 0;
			break;
		}
	case DM_UPDATEICON:
		if (dat->szProto) {
			TitleBarData tbd;
			TabControlData tcd;
			HICON hIcon = NULL;
			int i, icoIdx = 0;
			char *szProto = dat->szProto;
			HANDLE hContact = dat->hContact;
			if (strcmp(dat->szProto, "MetaContacts") == 0 && DBGetContactSettingByte(NULL,"CLC","Meta",0) == 0) {
				hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT,(UINT)dat->hContact, 0);
				if (hContact != NULL) {
					szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(UINT)hContact,0);
				} else {
					hContact = dat->hContact;
				}
			}
			dat->wStatus = DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			CallService(MS_SKIN2_RELEASEICON,(WPARAM)dat->userMenuIcon, 0);
			dat->userMenuIcon = LoadSkinnedProtoIcon(szProto, dat->wStatus);
			SendDlgItemMessage(hwndDlg, IDC_USERMENU, BM_SETIMAGE, IMAGE_ICON, (LPARAM)dat->userMenuIcon);
			if (g_dat->flags & SMF_STATUSICON) {
				if (dat->showTyping && (g_dat->flags2&SMF2_SHOWTYPINGWIN)) {
					hIcon = g_dat->hIcons[SMF_ICON_TYPING];
				} else if (dat->showUnread && (GetActiveWindow() != dat->hwndParent || GetForegroundWindow() != dat->hwndParent)) {
					hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
				} else {
					hIcon = LoadSkinnedProtoIcon(szProto, dat->wStatus);
				}
			} else {
				hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
			}
			icoIdx = 0;
			for (i = 0; i < g_dat->protoNum; i++) {
				if (!strcmp(g_dat->protoNames[i], szProto)) {
					icoIdx = dat->wStatus - ID_STATUS_OFFLINE + (ID_STATUS_OUTTOLUNCH - ID_STATUS_OFFLINE + 1) * (i +1) + 2;
					break;
				}
			}
			if (hwndDlg != dat->parent->hwndActive) {
				if (dat->showTyping) {
					icoIdx = 1;
				} else if (dat->showUnread & 1) {
					icoIdx = 0;
				}
			}
			tbd.iFlags = TBDF_ICON;
			tbd.hIcon = hIcon;
			SendMessage(dat->hwndParent, CM_UPDATETITLEBAR, (WPARAM)&tbd, (LPARAM)hwndDlg);
			tcd.iFlags = TCDF_ICON;
			tcd.iconIdx = icoIdx;
			SendMessage(dat->hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)hwndDlg);
		}
		break;
    case DM_USERNAMETOCLIP:
		{
			CONTACTINFO ci;
			char buf[128];
			HGLOBAL hData;

			buf[0] = 0;
			if(dat->hContact) {
				ZeroMemory(&ci, sizeof(ci));
				ci.cbSize = sizeof(ci);
				ci.hContact = dat->hContact;
				ci.szProto = dat->szProto;
				ci.dwFlag = CNF_UNIQUEID;
				if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
					switch (ci.type) {
						case CNFT_ASCIIZ:
							mir_snprintf(buf, sizeof(buf), "%s", ci.pszVal);
							miranda_sys_free(ci.pszVal);
							break;
						case CNFT_DWORD:
							mir_snprintf(buf, sizeof(buf), "%u", ci.dVal);
							break;
					}
				}
				if (!OpenClipboard(hwndDlg) || !lstrlenA(buf)) break;
				EmptyClipboard();
				hData = GlobalAlloc(GMEM_MOVEABLE, lstrlenA(buf) + 1);
				lstrcpyA(GlobalLock(hData), buf);
				GlobalUnlock(hData);
				SetClipboardData(CF_TEXT, hData);
				CloseClipboard();
			}
			break;
		}
	case DM_OPTIONSAPPLIED:
		{
			CHARFORMAT2 cf2 = {0};
			LOGFONT lf;
			COLORREF colour;
			dat->flags &= ~SMF_USEIEVIEW;
			dat->flags |= ServiceExists(MS_IEVIEW_WINDOW) ? g_dat->flags & SMF_USEIEVIEW : 0;
			if (dat->flags & SMF_USEIEVIEW && dat->hwndLog == NULL) {
				IEVIEWWINDOW ieWindow;
				ieWindow.cbSize = sizeof(IEVIEWWINDOW);
				ieWindow.iType = IEW_CREATE;
				ieWindow.dwFlags = 0;
				ieWindow.dwMode = IEWM_SCRIVER;
				ieWindow.parent = hwndDlg;
				ieWindow.x = 0;
				ieWindow.y = 0;
				ieWindow.cx = 200;
				ieWindow.cy = 300;
				CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
				dat->hwndLog = ieWindow.hwnd;
				if (dat->hwndLog == NULL) {
					dat->flags ^= SMF_USEIEVIEW;
				}
			} else if (!(dat->flags & SMF_USEIEVIEW) && dat->hwndLog != NULL) {
				if (dat->hwndLog != NULL) {
					IEVIEWWINDOW ieWindow;
					ieWindow.cbSize = sizeof(IEVIEWWINDOW);
					ieWindow.iType = IEW_DESTROY;
					ieWindow.hwnd = dat->hwndLog;
					CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
				}
				dat->hwndLog = NULL;
			}
			if(g_dat->avatarServiceExists) {
				dat->ace = (struct avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)dat->hContact, 0);
			}
			SendMessage(hwndDlg, DM_GETAVATAR, 0, 0);
			SetDialogToType(hwndDlg);

			colour = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETBKGNDCOLOR, 0, colour);
			colour = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETBKGNDCOLOR, 0, colour);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_MESSAGE), NULL, FALSE);
			LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, &colour);
			cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
			cf2.cbSize = sizeof(cf2);
			cf2.crTextColor = colour;
			cf2.bCharSet = lf.lfCharSet;
			_tcsncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
			cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
			cf2.wWeight = (WORD)lf.lfWeight;
			cf2.bPitchAndFamily = lf.lfPitchAndFamily;
			cf2.yHeight = abs(lf.lfHeight) * 15;
			SendDlgItemMessageA(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, 0, (LPARAM)&cf2);

			SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
			SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
			break;
		}
	case DM_UPDATETITLEBAR:
		{
			DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) wParam;
			if (dat->hContact) {
				if (dat->szProto) {
					TitleBarData tbd;
					TabControlData tcd;
					CONTACTINFO ci;
					char buf[128];
					buf[0] = 0;
					ZeroMemory(&ci, sizeof(ci));
					ci.cbSize = sizeof(ci);
					ci.hContact = dat->hContact;
					ci.szProto = dat->szProto;
					ci.dwFlag = CNF_UNIQUEID;
					if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
						switch (ci.type) {
						case CNFT_ASCIIZ:
							mir_snprintf(buf, sizeof(buf), Translate("User Menu - %s"), ci.pszVal);
							miranda_sys_free(ci.pszVal);
							break;
						case CNFT_DWORD:
							mir_snprintf(buf, sizeof(buf), Translate("User Menu - %u"), ci.dVal);
							break;
						}
					}
					SendMessage(GetDlgItem(hwndDlg, IDC_USERMENU), BUTTONADDTOOLTIP, (WPARAM) buf, 0);
		//			SetDlgItemTextA(hwndDlg, IDC_NAME, buf[0] ? buf : contactName);

					if (!cws || (!strcmp(cws->szModule, dat->szProto) && !strcmp(cws->szSetting, "Status"))) {
						SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
					}
					// log status change
					if ((dat->wStatus != dat->wOldStatus || lParam != 0)
						&& DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSTATUSCH, SRMSGDEFSET_SHOWSTATUSCH)) {
						DBEVENTINFO dbei;
						TCHAR buffer[512];
						char blob[2048];
						HANDLE hNewEvent;
						int iLen;
						TCHAR *szOldStatus = mir_tstrdup((TCHAR *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM) dat->wOldStatus, GCMDF_TCHAR));
						TCHAR *szNewStatus = mir_tstrdup((TCHAR *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM) dat->wStatus, GCMDF_TCHAR));

						if (dat->wStatus == ID_STATUS_OFFLINE) {
							iLen = mir_sntprintf(buffer, SIZEOF(buffer), TranslateT("signed off (was %s)"), szOldStatus);
							SendMessage(hwndDlg, DM_TYPING, 0, 0);
						}
						else if (dat->wOldStatus == ID_STATUS_OFFLINE) {
							iLen = mir_sntprintf(buffer, SIZEOF(buffer), TranslateT("signed on (%s)"), szNewStatus);
						}
						else {
							iLen = mir_sntprintf(buffer, SIZEOF(buffer), TranslateT("is now %s (was %s)"), szNewStatus, szOldStatus);
						}
					#if defined( _UNICODE )
						{
							int ansiLen = WideCharToMultiByte(CP_ACP, 0, buffer, -1, blob, sizeof(blob), 0, 0);
							memcpy( blob+ansiLen, buffer, sizeof(TCHAR)*(iLen+1));
							dbei.cbBlob = ansiLen + sizeof(TCHAR)*(iLen+1);
						}
					#else
						{
							int wLen = MultiByteToWideChar(CP_ACP, 0, buffer, -1, NULL, 0 );
							memcpy( blob, buffer, iLen+1 );
							MultiByteToWideChar(CP_ACP, 0, buffer, -1, (WCHAR*)&blob[iLen+1], wLen+1 );
							dbei.cbBlob = iLen+1 + sizeof(WCHAR)*wLen;
						}
					#endif
						//iLen = strlen(buffer) + 1;
						//MultiByteToWideChar(CP_ACP, 0, buffer, iLen, (LPWSTR) & buffer[iLen], iLen);
						dbei.cbSize = sizeof(dbei);
						dbei.pBlob = (PBYTE) blob;
					//	dbei.cbBlob = (strlen(buffer) + 1) * (sizeof(TCHAR) + 1);
						dbei.eventType = EVENTTYPE_STATUSCHANGE;
						dbei.flags = 0;
						dbei.timestamp = time(NULL);
						dbei.szModule = dat->szProto;
						hNewEvent = (HANDLE) CallService(MS_DB_EVENT_ADD, (WPARAM) dat->hContact, (LPARAM) & dbei);
						if (dat->hDbEventFirst == NULL) {
							dat->hDbEventFirst = hNewEvent;
							SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
						}
						dat->wOldStatus = dat->wStatus;
						mir_free(szOldStatus);
						mir_free(szNewStatus);
					}
					tbd.iFlags = TBDF_TEXT;
					tbd.pszText = GetWindowTitle(dat->hContact, dat->szProto);
					SendMessage(dat->hwndParent, CM_UPDATETITLEBAR, (WPARAM)&tbd, (LPARAM)hwndDlg);
					mir_free(tbd.pszText);
					tcd.iFlags = TCDF_TEXT;
					tcd.pszText = GetTabName(dat->hContact);
					SendMessage(dat->hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)hwndDlg);
					mir_free(tcd.pszText);
				}
			}

			break;
		}
	case DM_SWITCHTOOLBAR:
		SetDialogToType(hwndDlg);
//		SendMessage(dat->hwndParent, DM_SWITCHTOOLBAR, 0, 0);
		break;
	case DM_GETCODEPAGE:
		SetWindowLong(hwndDlg, DWL_MSGRESULT, dat->codePage);
		return TRUE;
	case DM_SETCODEPAGE:
		dat->codePage = (int) lParam;
		SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		break;
	case DM_SWITCHUNICODE:
#if defined( _UNICODE )
		{
			StatusIconData sid = {0};
			dat->flags ^= SMF_DISABLE_UNICODE;
			sid.cbSize = sizeof(sid);
			sid.szModule = SRMMMOD;
			sid.flags = (dat->flags & SMF_DISABLE_UNICODE) ? MBF_DISABLED : 0;
			CallService(MS_MSG_MODIFYICON, (WPARAM)dat->hContact, (LPARAM) &sid);
			SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		}
#endif
		break;
	case DM_SWITCHRTL:
		{
			PARAFORMAT2 pf2;
			ZeroMemory((void *)&pf2, sizeof(pf2));
			pf2.cbSize = sizeof(pf2);
			pf2.dwMask = PFM_RTLPARA;
			dat->flags ^= SMF_RTL;
			if (dat->flags&SMF_RTL) {
				pf2.wEffects = PFE_RTLPARA;
				SetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
				SetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE) | WS_EX_LEFTSCROLLBAR);
			} else {
				pf2.wEffects = 0;
				SetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE) &~ (WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));
				SetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE) &~ (WS_EX_LEFTSCROLLBAR));
			}
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		}
		SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		break;
	case DM_GETWINDOWSTATE:
		{
			UINT state = 0;

			state |= MSG_WINDOW_STATE_EXISTS;
			if (IsWindowVisible(hwndDlg))
				state |= MSG_WINDOW_STATE_VISIBLE;
			if (GetForegroundWindow()==dat->hwndParent)
				state |= MSG_WINDOW_STATE_FOCUS;
			if (IsIconic(dat->hwndParent))
				state |= MSG_WINDOW_STATE_ICONIC;
			SetWindowLong(hwndDlg, DWL_MSGRESULT, state);
			return TRUE;

		}
	case DM_ACTIVATE:

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;
		//fall through
	case WM_MOUSEACTIVATE:
		if (dat->showUnread) {
			dat->showUnread = 0;
			if (KillTimer(hwndDlg, TIMERID_FLASHWND)) {
	//			dat->nFlash = 0;
			}
			SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
		}
		break;
	case WM_LBUTTONDOWN:
		SendMessage(dat->hwndParent, WM_LBUTTONDOWN, wParam, lParam);
		return TRUE;
	case DM_SETFOCUS:
		if (lParam == WM_MOUSEACTIVATE) {
			HWND hLog;
			RECT rc;
			POINT pt;
			GetCursorPos(&pt);
			if (dat->hwndLog != NULL) {
				hLog = dat->hwndLog;
			} else {
				hLog = GetDlgItem(hwndDlg, IDC_LOG);
			}
			GetWindowRect(hLog, &rc);
			if (pt.x >= rc.left && pt.x <= rc.right && pt.y >= rc.top && pt.y <=rc.bottom) {
		//		SetFocus(hLog);
				return TRUE;
			}
		}
		SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
		return TRUE;
	case WM_SETFOCUS:
		SendMessage(dat->hwndParent, CM_ACTIVATECHILD, 0, (LPARAM)hwndDlg);
		PostMessage(hwndDlg, DM_SETFOCUS, 0, 0);
//		SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
		return TRUE;
	case DM_SETPARENT:
		dat->hwndParent = (HWND) lParam;
		dat->parent = (ParentWindowData *) GetWindowLong(dat->hwndParent, GWL_USERDATA);
		SetParent(hwndDlg, dat->hwndParent);
		return TRUE;
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *) lParam;
			int minBottomHeight = dat->toolbarSize.cy + dat->minEditBoxHeight;
			if (minBottomHeight < g_dat->limitAvatarMinH) {
			}
			mmi->ptMinTrackSize.x = dat->toolbarSize.cx;// + dat->avatarWidth;
			mmi->ptMinTrackSize.y = dat->minLogBoxHeight + minBottomHeight;

			return 0;
		}
	case WM_SIZE:
		{
			if (wParam==SIZE_RESTORED || wParam==SIZE_MAXIMIZED) {
				RECT rc;
				int dlgWidth, dlgHeight;
				dlgWidth = LOWORD(lParam);
				dlgHeight = HIWORD(lParam);
				/*if (dlgWidth == 0 && dlgHeight ==0) */{
					GetClientRect(hwndDlg, &rc);
					dlgWidth = rc.right - rc.left;
					dlgHeight = rc.bottom - rc.top;
				}
				MessageDialogResize(hwndDlg, dat, dlgWidth, dlgHeight);
			}
			return TRUE;
		}
	case DM_SPLITTERMOVED:
		{
			POINT pt;
			RECT rc;
			RECT rcLog;
			if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_SPLITTER)) {
				int oldSplitterY;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_LOG), &rcLog);
				GetClientRect(hwndDlg, &rc);
				pt.x = 0;
				pt.y = wParam;
				ScreenToClient(hwndDlg, &pt);

				oldSplitterY = dat->splitterPos;
				dat->splitterPos = rc.bottom - pt.y;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_MESSAGE), &rc);
			/*
				if (rc.bottom - rc.top + (dat->splitterPos - oldSplitterY) < dat->minEditBoxSize.cy)
					dat->splitterPos = oldSplitterY + dat->minEditBoxSize.cy - (rc.bottom - rc.top);
				if (rcLog.bottom - rcLog.top - (dat->splitterPos - oldSplitterY) < dat->minEditBoxSize.cy)
					dat->splitterPos = oldSplitterY - dat->minEditBoxSize.cy + (rcLog.bottom - rcLog.top);
					*/
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
			}
			break;
		}
	case DM_REMAKELOG:
		dat->lastEventType = -1;
		if (wParam == 0 || (HANDLE) wParam == dat->hContact) {
			//StreamInEvents(hwndDlg, dat->hDbEventFirst, 0, 0);
			StreamInEvents(hwndDlg, dat->hDbEventFirst, -1, 0);
		}
		break;
	case DM_APPENDTOLOG:   //takes wParam=hDbEvent
		StreamInEvents(hwndDlg, (HANDLE) wParam, 1, 1);
		break;
	case DM_SCROLLLOGTOBOTTOM:
		if (dat->hwndLog == NULL) {
			/*
			int	nMin, nMax;
			HWND hwndLog = GetDlgItem(hwndDlg, IDC_LOG);
			GetScrollRange(hwndLog, SB_VERT, &nMin, &nMax);
			SetScrollPos(hwndLog, SB_VERT, nMax, TRUE);
			PostMessage(hwndLog, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nMax), (LPARAM) NULL);
			*/
			SCROLLINFO si = { 0 };
			if ((GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG), GWL_STYLE) & WS_VSCROLL) == 0)
				break;
			si.cbSize = sizeof(si);
			si.fMask = SIF_PAGE | SIF_RANGE;
			if (GetScrollInfo(GetDlgItem(hwndDlg, IDC_LOG), SB_VERT, &si)) {
				si.fMask = SIF_POS;
				si.nPos = si.nMax - si.nPage + 1;
				SetScrollInfo(GetDlgItem(hwndDlg, IDC_LOG), SB_VERT, &si, TRUE);
				PostMessage(GetDlgItem(hwndDlg, IDC_LOG), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
			}
			RedrawWindow(GetDlgItem(hwndDlg, IDC_LOG), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
		} else {
			IEVIEWWINDOW ieWindow;
			ieWindow.cbSize = sizeof(IEVIEWWINDOW);
			ieWindow.iType = IEW_SCROLLBOTTOM;
			ieWindow.hwnd = dat->hwndLog;
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		}
	case HM_DBEVENTADDED:
		if ((HANDLE) wParam == dat->hContact)
		{
			DBEVENTINFO dbei = { 0 };

			dbei.cbSize = sizeof(dbei);
			dbei.cbBlob = 0;
			CallService(MS_DB_EVENT_GET, lParam, (LPARAM) & dbei);
			if (dat->hDbEventFirst == NULL)
				dat->hDbEventFirst = (HANDLE) lParam;
			if (dbei.eventType == EVENTTYPE_MESSAGE && (dbei.flags & DBEF_READ))
				break;
			if (DbEventIsShown(&dbei, dat)) {
				if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & (DBEF_SENT))) {
					dat->lastMessage = dbei.timestamp;
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
					if (GetForegroundWindow()==dat->hwndParent && dat->parent->hwndActive == hwndDlg)
						SkinPlaySound("RecvMsgActive");
					else SkinPlaySound("RecvMsgInactive");
					if ((g_dat->flags2 & SMF2_SWITCHTOACTIVE) && (IsIconic(dat->hwndParent) || GetActiveWindow() != dat->hwndParent)) {
						SendMessage(dat->hwndParent, CM_ACTIVATECHILD, 0, (LPARAM) hwndDlg);
					}
				}
				if ((HANDLE) lParam != dat->hDbEventFirst && (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, lParam, 0) == NULL)
					SendMessage(hwndDlg, DM_APPENDTOLOG, lParam, 0);
				else
					SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
				if (!(dbei.flags & DBEF_SENT) && dbei.eventType != EVENTTYPE_STATUSCHANGE) {
//					dat->nFlash = dat->nFlashMax;
					SendMessage(dat->hwndParent, CM_STARTFLASHING, 0, 0);
					if (GetActiveWindow() != dat->hwndParent || GetForegroundWindow() != dat->hwndParent || dat->parent->hwndActive != hwndDlg) {
						dat->showUnread = 0;
						SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
					}
				}
			}
		}
		break;
	case DM_UPDATESTATUSBAR:
		if (dat->parent->hwndActive == hwndDlg) {
			TCHAR szText[256];
			StatusBarData sbd= {0};
			StatusIconData sid = {0};
			sbd.iFlags = SBDF_TEXT | SBDF_ICON;
			if (dat->messagesInProgress && (g_dat->flags & SMF_SHOWPROGRESS)) {
				sbd.hIcon = g_dat->hIcons[SMF_ICON_DELIVERING];
				sbd.pszText = szText;
				mir_sntprintf(szText, SIZEOF(szText), TranslateT("Sending in progress: %d message(s) left..."), dat->messagesInProgress);
			} else if (dat->nTypeSecs) {
				TCHAR *szContactName = GetNickname(dat->hContact, dat->szProto);
				sbd.hIcon = g_dat->hIcons[SMF_ICON_TYPING];
				sbd.pszText = szText;
				mir_sntprintf(szText, SIZEOF(szText), TranslateT("%s is typing a message..."), szContactName);
				mir_free(szContactName);
				dat->nTypeSecs--;
			} else if (dat->lastMessage) {
				DBTIMETOSTRINGT dbtts;
				TCHAR date[64], time[64];
				dbtts.szFormat = _T("d");
				dbtts.cbDest = SIZEOF(date);
				dbtts.szDest = date;
#if defined ( _UNICODE )
				CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, dat->lastMessage, (LPARAM) & dbtts);
#else
				CallService(MS_DB_TIME_TIMESTAMPTOSTRING, dat->lastMessage, (LPARAM) & dbtts);
#endif
				dbtts.szFormat = _T("t");
				dbtts.cbDest = SIZEOF(time);
				dbtts.szDest = time;
#if defined ( _UNICODE )
				CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, dat->lastMessage, (LPARAM) & dbtts);
#else
				CallService(MS_DB_TIME_TIMESTAMPTOSTRING, dat->lastMessage, (LPARAM) & dbtts);
#endif
				mir_sntprintf(szText, SIZEOF(szText), TranslateT("Last message received on %s at %s."), date, time);
				sbd.pszText = szText;
			} else {
				sbd.pszText =  _T("");
			}
			SendMessage(dat->hwndParent, CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)hwndDlg);
			UpdateReadChars(hwndDlg, dat);
			sid.cbSize = sizeof(sid);
			sid.szModule = SRMMMOD;
#if defined ( _UNICODE )
			sid.flags = (dat->flags & SMF_DISABLE_UNICODE) ? MBF_DISABLED : 0;
#else
			sid.flags = MBF_DISABLED;
#endif
			CallService(MS_MSG_MODIFYICON, (WPARAM)dat->hContact, (LPARAM) &sid);
		}
		break;
	case DM_CLEARLOG:
	// IEVIew MOD Begin
		if (dat->hwndLog != NULL) {
			IEVIEWEVENT event;
			ZeroMemory(&event, sizeof(event));
			event.cbSize = sizeof(event);
			event.iType = IEE_CLEAR_LOG;
			event.dwFlags = 0;
			event.hwnd = dat->hwndLog;
			event.hContact = dat->hContact;
			event.codepage = dat->codePage;
			event.pszProto = dat->szProto;
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
		}
	// IEVIew MOD End
		SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
		dat->hDbEventFirst = NULL;
		dat->lastEventType = -1;
		break;
	case WM_TIMER:
		if (wParam == TIMERID_MSGSEND) {
			int i;
			int timeout = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);
			for (i = 0; i < dat->sendCount; i++) {
				if (dat->sendInfo[i].sendBuffer) {
					if (dat->sendInfo[i].timeout < timeout) {
						dat->sendInfo[i].timeout+=1000;
						if (dat->sendInfo[i].timeout >= timeout) {
							ErrorWindowData *ewd = (ErrorWindowData *) mir_alloc(sizeof(ErrorWindowData));
							ewd->szName = GetNickname(dat->hContact, dat->szProto);
							ewd->szDescription = mir_tstrdup(TranslateT("The message send timed out."));
							ewd->textSize = dat->sendInfo[i].sendBufferSize;
							ewd->szText = (char *)mir_alloc(dat->sendInfo[i].sendBufferSize);
							memcpy(ewd->szText, dat->sendInfo[i].sendBuffer, dat->sendInfo[i].sendBufferSize);
							ewd->flags = dat->sendInfo[i].flags;
							ewd->hwndParent = hwndDlg;
							if (dat->messagesInProgress>0) {
								dat->messagesInProgress--;
								if (g_dat->flags & SMF_SHOWPROGRESS) {
									SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
								}
							}
							CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGSENDERROR), hwndDlg, ErrorDlgProc, (LPARAM) ewd);
							//RemoveSendBuffer(dat, i);
						}
					}
				}
			}
		}
		else if (wParam == TIMERID_FLASHWND) {
			dat->showUnread++;
			SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
		}
		else if (wParam == TIMERID_TYPE) {
			if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON && GetTickCount() - dat->nLastTyping > TIMEOUT_TYPEOFF) {
				NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);
			}
			if (dat->showTyping) {
				if (dat->nTypeSecs) {
					dat->nTypeSecs--;
				}
				else {
					dat->showTyping = 0;
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
					SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
				}
			}
			else {
				if (dat->nTypeSecs) {
					dat->showTyping = 1;
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
					SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
				}
			}
		}
		break;
	case DM_SENDMESSAGE:
			if (lParam) {
				HANDLE hSendId;
				struct MessageSendInfo *msi = (struct MessageSendInfo *)lParam;
				dat->sendCount ++;
				dat->sendInfo = (struct MessageSendInfo *) mir_realloc(dat->sendInfo, sizeof(struct MessageSendInfo) * dat->sendCount);
				dat->sendInfo[dat->sendCount-1].sendBufferSize = msi->sendBufferSize;
				dat->sendInfo[dat->sendCount-1].sendBuffer = (char *) mir_alloc(msi->sendBufferSize);
				dat->sendInfo[dat->sendCount-1].flags = msi->flags;
				dat->sendInfo[dat->sendCount-1].timeout=0;
				memcpy(dat->sendInfo[dat->sendCount-1].sendBuffer, msi->sendBuffer, dat->sendInfo[dat->sendCount-1].sendBufferSize);
				SetTimer(hwndDlg, TIMERID_MSGSEND, 1000, NULL);
				dat->messagesInProgress++;
				if (g_dat->flags & SMF_SHOWPROGRESS) {
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
				}
				hSendId = (HANDLE) CallContactService(dat->hContact, MsgServiceName(dat->hContact), msi->flags, (LPARAM) dat->sendInfo[dat->sendCount-1].sendBuffer);
				if (dat->sendCount>0) {
					dat->sendInfo[dat->sendCount-1].hSendId = hSendId;
				}
			}

		break;
	case DM_ERRORDECIDED:
		switch (wParam) {
		case MSGERROR_CANCEL:
			{
				SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
			}
			break;
		case MSGERROR_RETRY:
			if (lParam) {
				struct MessageSendInfo msi;
				ErrorWindowData *ewd = (ErrorWindowData *)lParam;
				msi.sendBufferSize = ewd->textSize;
				msi.sendBuffer = ewd->szText;
				msi.flags = ewd->flags;
				SendMessage(hwndDlg, DM_SENDMESSAGE, 0, (LPARAM)&msi);
			}
			break;
		}
		break;
	case WM_MEASUREITEM:
		if (!MeasureMenuItem(wParam, lParam)) {
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		}
		return TRUE;

	case WM_DRAWITEM:
			{
				LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;
				if (dis->hwndItem == (HWND)hToolbarMenu) {
					return DrawMenuItem(wParam, lParam);
				} else if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_AVATAR) && dat->avatarPic && (g_dat->flags&SMF_AVATAR)) {
					HDC hdcMem = CreateCompatibleDC(dis->hDC);
					HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0,0,0));
					HBITMAP hbmMem = CreateCompatibleBitmap(dis->hDC, dat->avatarWidth, dat->avatarHeight);
					hPen = (HPEN)SelectObject(hdcMem, hPen);
					hbmMem = (HBITMAP) SelectObject(hdcMem, hbmMem);
					Rectangle(hdcMem, 0, 0, dat->avatarWidth, dat->avatarHeight);
					if (!g_dat->avatarServiceExists) {
						BITMAP bminfo;
						HDC hdcTemp = CreateCompatibleDC(dis->hDC);
						HBITMAP hbmTemp = (HBITMAP)SelectObject(hdcTemp, dat->avatarPic);
						GetObject(dat->avatarPic, sizeof(bminfo), &bminfo);
						SetStretchBltMode(hdcMem, HALFTONE);
						StretchBlt(hdcMem, 1, 1, dat->avatarWidth-2, dat->avatarHeight-2, hdcTemp, 0, 0, bminfo.bmWidth, bminfo.bmHeight, SRCCOPY);
						hbmTemp = (HBITMAP) SelectObject(hdcTemp, hbmTemp);
						DeleteDC(hdcTemp);
					} else {
						AVATARDRAWREQUEST adr;
						ZeroMemory(&adr, sizeof(adr));
						adr.cbSize = sizeof (AVATARDRAWREQUEST);
						adr.hContact = dat->hContact;
						adr.hTargetDC = hdcMem;
						adr.rcDraw.left = 1;
						adr.rcDraw.top = 1;
						adr.rcDraw.right = dat->avatarWidth-1;
						adr.rcDraw.bottom = dat->avatarHeight -1;
						adr.dwFlags = 0;//AVDRQ_DRAWBORDER;
						adr.alpha = 0;
						CallService(MS_AV_DRAWAVATAR, (WPARAM)0, (LPARAM)&adr);
					}
					BitBlt(dis->hDC, 0, 0, dat->avatarWidth, dat->avatarHeight, hdcMem, 0, 0, SRCCOPY);
					hPen = (HPEN)SelectObject(hdcMem, hPen);
					hbmMem = (HBITMAP) SelectObject(hdcMem, hbmMem);
					DeleteObject(hPen);
					DeleteObject(hbmMem);
					DeleteDC(hdcMem);
					return TRUE;
				}
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
			}
	case WM_COMMAND:
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM) dat->hContact))
			break;
		switch (LOWORD(wParam)) {
		case IDC_SENDALL:
			{
				int result;
				if (dat->sendAllConfirm == 0) {
					result = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CONFIRM_SENDALL), hwndDlg, ConfirmSendAllDlgProc, (LPARAM)hwndDlg);
					if (result & 0x10000) {
						dat->sendAllConfirm = result;
					}
				} else {
					result = dat->sendAllConfirm;
				}
				if (LOWORD(result) == IDNO) {
					break;
				}

			}
		case IDOK:
			//this is a 'send' button
			if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDOK)))
				break;
			//if(GetKeyState(VK_CTRL) & 0x8000) {    // copy user name
					//SendMessage(hwndDlg, DM_USERNAMETOCLIP, 0, 0);
			//}
			if (dat->hContact !=NULL) {
				struct MessageSendInfo msi;
				int bufSize;
				bufSize = GetWindowTextLengthA(GetDlgItem(hwndDlg, IDC_MESSAGE)) + 1;
				msi.sendBufferSize = bufSize * (sizeof(TCHAR) + 1);
				msi.sendBuffer = (char *) mir_alloc(msi.sendBufferSize);
				msi.flags = SEND_FLAGS;
				GetDlgItemTextA(hwndDlg, IDC_MESSAGE, msi.sendBuffer, bufSize);
				{
					PARAFORMAT2 pf2;
					ZeroMemory((void *)&pf2, sizeof(pf2));
					pf2.cbSize = sizeof(pf2);
					pf2.dwMask = PFM_RTLPARA;
					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);
					if (pf2.wEffects & PFE_RTLPARA) {
						msi.flags |= PREF_RTL;
					}
				}

		#if defined( _UNICODE )
				{
					GETTEXTEX  gt;
					gt.cb = bufSize * sizeof(TCHAR);
					gt.flags = GT_USECRLF;
					gt.codepage = 1200;
					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_GETTEXTEX, (WPARAM) &gt, (LPARAM) &msi.sendBuffer[bufSize]);
				}

				if ( RTL_Detect((wchar_t *)&msi.sendBuffer[bufSize] )) {
					msi.flags |= PREF_RTL;
				}
		#endif
				if (msi.sendBuffer[0] == 0)
					break;
		#if defined( _UNICODE )
				dat->cmdList = tcmdlist_append(dat->cmdList, (TCHAR *) &msi.sendBuffer[bufSize]);
		#else
				dat->cmdList = tcmdlist_append(dat->cmdList, msi.sendBuffer);
		#endif
				dat->cmdListCurrent = 0;
				if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON) {
					NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);
				}
				SetDlgItemText(hwndDlg, IDC_MESSAGE, _T(""));
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AUTOMIN, SRMSGDEFSET_AUTOMIN))
					ShowWindow(dat->hwndParent, SW_MINIMIZE);
				if (LOWORD(wParam) == IDC_SENDALL) {
					SendMessage(dat->hwndParent, DM_SENDMESSAGE, 0, (LPARAM) &msi);
				} else {
					SendMessage(hwndDlg, DM_SENDMESSAGE, 0, (LPARAM) &msi);
				}
				mir_free (msi.sendBuffer);
			}
			return TRUE;
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;
		case IDC_USERMENU:
			{
				if(GetKeyState(VK_SHIFT) & 0x8000) {    // copy user name
					SendMessage(hwndDlg, DM_USERNAMETOCLIP, 0, 0);
				}
				else {
					RECT rc;
					HMENU hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM) dat->hContact, 0);
					GetWindowRect(GetDlgItem(hwndDlg, LOWORD(wParam)), &rc);
					TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
					DestroyMenu(hMenu);
				}
			}
			break;
		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM) dat->hContact, 0);
			break;
		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM) dat->hContact, 0);
			break;
		case IDC_SMILEYS:
			if (g_dat->smileyServiceExists) {
				SMADD_SHOWSEL3 smaddInfo;
				RECT rc;
				smaddInfo.cbSize = sizeof(SMADD_SHOWSEL3);
				smaddInfo.hwndParent = dat->hwndParent;
				smaddInfo.hwndTarget = GetDlgItem(hwndDlg, IDC_MESSAGE);
				smaddInfo.targetMessage = EM_REPLACESEL;
				smaddInfo.targetWParam = TRUE;
				smaddInfo.Protocolname = dat->szProto;
				if (dat->szProto!=NULL && strcmp(dat->szProto,"MetaContacts")==0) {
					HANDLE hContact = (HANDLE) CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM) dat->hContact, 0);
					if (hContact!=NULL) {
						smaddInfo.Protocolname = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
					}
				}
				GetWindowRect(GetDlgItem(hwndDlg, IDC_SMILEYS), &rc);
				smaddInfo.Direction = 0;
				smaddInfo.xPosition = rc.left;
				smaddInfo.yPosition = rc.bottom;
				smaddInfo.hContact = dat->hContact;
				CallService(MS_SMILEYADD_SHOWSELECTION, 0, (LPARAM) &smaddInfo);
			}
			break;
		case IDC_QUOTE:
			/*
			{
				char szFilename[MAX_PATH];
				OPENFILENAMEA ofn={0};
				strcpy(szFilename, "");
				ofn.lStructSize=sizeof(OPENFILENAME);
				ofn.hwndOwner=hwndDlg;
				ofn.lpstrFile = szFilename;
				ofn.lpstrFilter = "Rich Text File\0*.rtf\0\0";
				ofn.nMaxFile = MAX_PATH;
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.Flags = OFN_HIDEREADONLY;
				ofn.lpstrDefExt = "rtf";
				if (GetSaveFileNameA(&ofn)) {
					//remove(szFilename);
					EDITSTREAM stream = { 0 };
					stream.dwCookie = (DWORD_PTR)szFilename;
					stream.dwError = 0;
					stream.pfnCallback = EditStreamCallback;
					SendDlgItemMessage(hwndDlg, IDC_LOG, EM_STREAMOUT, SF_RTF | SF_USECODEPAGE, (LPARAM) & stream);
				}
			}
			*/
			{
				DBEVENTINFO dbei = { 0 };
				SETTEXTEX  st;
				TCHAR *buffer = NULL;
				st.flags = ST_SELECTION;
#ifdef _UNICODE
				st.codepage = 1200;
#else
				st.codepage = CP_ACP;
#endif
				if (dat->hDbEventLast==NULL) break;
				if (dat->hwndLog != NULL) {
					buffer = GetIEViewSelection(dat);
				} else {
					buffer = GetRichEditSelection(GetDlgItem(hwndDlg, IDC_LOG));
				}
				if (buffer!=NULL) {
					TCHAR *quotedBuffer = GetQuotedTextW(buffer);
					SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETTEXTEX, (WPARAM) &st, (LPARAM)quotedBuffer);
					mir_free(quotedBuffer);
					mir_free(buffer);
					SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
					break;
				}
				dbei.cbSize = sizeof(dbei);
				dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM) dat->hDbEventLast, 0);
				if (dbei.cbBlob == 0xFFFFFFFF) break;
				dbei.pBlob = (PBYTE) mir_alloc(dbei.cbBlob);
				CallService(MS_DB_EVENT_GET, (WPARAM)  dat->hDbEventLast, (LPARAM) & dbei);
				if (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_STATUSCHANGE) {
					TCHAR *buffer = NULL;
#ifdef _UNICODE
					DWORD aLen = strlen((char *)dbei.pBlob)+1;
					if (dbei.eventType == EVENTTYPE_MESSAGE) {
						if (dbei.cbBlob > aLen) {
							DWORD wlen = safe_wcslen((wchar_t *)&dbei.pBlob[aLen], (dbei.cbBlob - aLen) / 2);
							if (wlen > 0 && wlen < aLen) {
								buffer = (TCHAR *)&dbei.pBlob[aLen];
							}
						}
					}
					if (buffer == NULL) {
						buffer = a2t((char *) dbei.pBlob);
						mir_free(dbei.pBlob);
						dbei.pBlob = (char *)buffer;
					}
#else
					buffer = (TCHAR *)dbei.pBlob;
#endif
					if (buffer!=NULL) {
						TCHAR *quotedBuffer = GetQuotedTextW(buffer);
						SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETTEXTEX, (WPARAM) &st, (LPARAM)quotedBuffer);
						mir_free(quotedBuffer);
					}
				}
				mir_free(dbei.pBlob);
				SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
				break;
			}
		case IDC_ADD:
			{
				ADDCONTACTSTRUCT acs = { 0 };

				acs.handle = dat->hContact;
				acs.handleType = HANDLE_CONTACT;
				acs.szProto = 0;
				CallService(MS_ADDCONTACT_SHOW, (WPARAM) hwndDlg, (LPARAM) & acs);
			}
			if (!DBGetContactSettingByte(dat->hContact, "CList", "NotOnList", 0)) {
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
			}
		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE));
				dat->cmdListCurrent = dat->cmdListNew;
				dat->cmdListNew = 0;
				UpdateReadChars(hwndDlg, dat);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), len != 0);
				if (!(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000)) {
					dat->nLastTyping = GetTickCount();
					if (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE))) {
						if (dat->nTypeMode == PROTOTYPE_SELFTYPING_OFF) {
							NotifyTyping(dat, PROTOTYPE_SELFTYPING_ON);
						}
					}
					else {
						if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON) {
							NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);
						}
					}
				}
			}
			break;
		}
		break;
	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr;
			pNmhdr = (LPNMHDR)lParam;
			switch (pNmhdr->idFrom) {
			case IDC_LOG:
				switch (pNmhdr->code) {
				case EN_MSGFILTER:
					switch (((MSGFILTER *) lParam)->msg) {
					case WM_CHAR:
						if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
							SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
							SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), ((MSGFILTER *) lParam)->msg, ((MSGFILTER *) lParam)->wParam, ((MSGFILTER *) lParam)->lParam);
							SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
						}
						return TRUE;
					case WM_KEYDOWN:
						if (GetKeyState(VK_CONTROL) & 0x8000 && ((MSGFILTER *) lParam)->wParam== VK_TAB) {
							if (GetKeyState(VK_SHIFT) & 0x8000) {
								SendMessage(GetParent(hwndDlg), CM_ACTIVATEPREV, 0, (LPARAM)hwndDlg);
							} else {
								SendMessage(GetParent(hwndDlg), CM_ACTIVATENEXT, 0, (LPARAM)hwndDlg);
							}
							SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
						}
						return TRUE;
					case WM_LBUTTONDOWN:
						{
							HCURSOR hCur = GetCursor();
							if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE)
								|| hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE)) {
									SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
									return TRUE;
								}
								break;
						}
					case WM_MOUSEMOVE:
						{
							HCURSOR hCur = GetCursor();
							if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE)
								|| hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE))
								SetCursor(LoadCursor(NULL, IDC_ARROW));
							break;
						}
					case WM_RBUTTONUP:
						{
							HMENU hMenu, hSubMenu;
							POINT pt;
							CHARRANGE sel, all = { 0, -1 };

							hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
							hSubMenu = GetSubMenu(hMenu, 0);
							CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
							SendMessage(((NMHDR *) lParam)->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);
							if (sel.cpMin == sel.cpMax)
								EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
							pt.x = (short) LOWORD(((MSGFILTER *) lParam)->lParam);
							pt.y = (short) HIWORD(((MSGFILTER *) lParam)->lParam);
							ClientToScreen(((NMHDR *) lParam)->hwndFrom, &pt);
							switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL)) {
							case IDM_COPY:
								SendMessage(((NMHDR *) lParam)->hwndFrom, WM_COPY, 0, 0);
								break;
							case IDM_COPYALL:
								SendMessage(((NMHDR *) lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & all);
								SendMessage(((NMHDR *) lParam)->hwndFrom, WM_COPY, 0, 0);
								SendMessage(((NMHDR *) lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & sel);
								break;
							case IDM_SELECTALL:
								SendMessage(((NMHDR *) lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & all);
								break;
							case IDM_CLEAR:
								SendMessage(hwndDlg, DM_CLEARLOG, 0, 0);
							}
							DestroyMenu(hMenu);
							SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
							return TRUE;
						}
					}
					break;
				case EN_LINK:
					switch (((ENLINK *) lParam)->msg) {
					case WM_SETCURSOR:
						SetCursor(hCurHyperlinkHand);
						SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
						return TRUE;
					case WM_RBUTTONDOWN:
					case WM_LBUTTONUP:
						{
							TEXTRANGE tr;
							CHARRANGE sel;
							char* pszUrl;

							SendMessage(pNmhdr->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);
							if (sel.cpMin != sel.cpMax)
								break;
							tr.chrg = ((ENLINK *) lParam)->chrg;
							tr.lpstrText = mir_alloc(sizeof(TCHAR)*(tr.chrg.cpMax - tr.chrg.cpMin + 8));
							SendMessage(pNmhdr->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM) & tr);
							if (_tcschr(tr.lpstrText, _T('@')) != NULL && _tcschr(tr.lpstrText, _T(':')) == NULL && _tcschr(tr.lpstrText, _T('/')) == NULL) {
								MoveMemory(tr.lpstrText + sizeof(TCHAR) * 7, tr.lpstrText, sizeof(TCHAR)*(tr.chrg.cpMax - tr.chrg.cpMin + 1));
								CopyMemory(tr.lpstrText, _T("mailto:"), sizeof(TCHAR) * 7);
							}
							pszUrl = t2a( (const TCHAR *)tr.lpstrText );
							if (((ENLINK *) lParam)->msg == WM_RBUTTONDOWN) {
								HMENU hMenu, hSubMenu;
								POINT pt;

								hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
								hSubMenu = GetSubMenu(hMenu, 1);
								CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
								pt.x = (short) LOWORD(((ENLINK *) lParam)->lParam);
								pt.y = (short) HIWORD(((ENLINK *) lParam)->lParam);
								ClientToScreen(((NMHDR *) lParam)->hwndFrom, &pt);
								switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL)) {
								case IDM_OPENNEW:
									CallService(MS_UTILS_OPENURL, 1, (LPARAM) pszUrl);
									break;
								case IDM_OPENEXISTING:
									CallService(MS_UTILS_OPENURL, 0, (LPARAM) pszUrl);
									break;
								case IDM_COPYLINK:
									{
										HGLOBAL hData;
										if (!OpenClipboard(hwndDlg))
											break;
										EmptyClipboard();
										hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR)*(lstrlen(tr.lpstrText) + 1));
										lstrcpy(GlobalLock(hData), tr.lpstrText);
										GlobalUnlock(hData);
									#if defined( _UNICODE )
										SetClipboardData(CF_UNICODETEXT, hData);
									#else
										SetClipboardData(CF_TEXT, hData);
									 #endif
										CloseClipboard();
										break;
									}
								}
								DestroyMenu(hMenu);
								mir_free(tr.lpstrText);
								mir_free(pszUrl);
								SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
								return TRUE;
							}
							CallService(MS_UTILS_OPENURL, 1, (LPARAM) pszUrl);
							SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
							mir_free(tr.lpstrText);
							mir_free(pszUrl);
							break;
						}
					}
					break;
				}
				break;
			case IDC_MESSAGE:
				switch (((NMHDR *) lParam)->code) {
				case EN_MSGFILTER:
					switch (((MSGFILTER *) lParam)->msg) {
					case WM_RBUTTONUP:
						SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
						return TRUE;
					}
				}
			}
		}
		break;
	case HM_EVENTSENT:
		{
			ACKDATA *ack = (ACKDATA *) lParam;
			DBEVENTINFO dbei = { 0 };
			HANDLE hNewEvent;
			int i;

			if (ack->type != ACKTYPE_MESSAGE)
				break;
			if (dat->sendCount==0)
				break;
			for (i = 0; i < dat->sendCount; i++) {
				if (ack->hProcess == dat->sendInfo[i].hSendId && ack->hContact == dat->hContact)
					break;
			}
			if (dat->messagesInProgress>0) {
				dat->messagesInProgress--;
				if (g_dat->flags & SMF_SHOWPROGRESS) {
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
				}
			}
			if (ack->result == ACKRESULT_FAILED) {
				if (i == dat->sendCount) {
					for (i = 0; i < dat->sendCount; i++) {
						if (dat->sendInfo[i].sendBuffer) {
							break;
						}
					}
				}
				if (i < dat->sendCount) {
					ErrorWindowData *ewd = (ErrorWindowData *) mir_alloc(sizeof(ErrorWindowData));
					ewd->szName = GetNickname(dat->hContact, dat->szProto);
					ewd->szDescription = a2t((char *) ack->lParam);
					ewd->textSize = dat->sendInfo[i].sendBufferSize;
					ewd->szText = (char *)mir_alloc(dat->sendInfo[i].sendBufferSize);
					memcpy(ewd->szText, dat->sendInfo[i].sendBuffer, dat->sendInfo[i].sendBufferSize);
					ewd->flags = dat->sendInfo[i].flags;
					ewd->hwndParent = hwndDlg;
					CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGSENDERROR), hwndDlg, ErrorDlgProc, (LPARAM) ewd);//hwndDlg
					RemoveSendBuffer(dat, i);
				}
				return 0;
			}
			if (i == dat->sendCount)
				break;
			dbei.cbSize = sizeof(dbei);
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.flags = DBEF_SENT | (( dat->sendInfo[i].flags & PREF_RTL) ? DBEF_RTL : 0 );
			dbei.szModule = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) dat->hContact, 0);
			dbei.timestamp = time(NULL);
			dbei.cbBlob = lstrlenA(dat->sendInfo[i].sendBuffer) + 1;
	#if defined( _UNICODE )
			dbei.cbBlob *= sizeof(TCHAR) + 1;
	#endif
			dbei.pBlob = (PBYTE) dat->sendInfo[i].sendBuffer;
			hNewEvent = (HANDLE) CallService(MS_DB_EVENT_ADD, (WPARAM) dat->hContact, (LPARAM) & dbei);
			SkinPlaySound("SendMsg");
			if (dat->hDbEventFirst == NULL) {
				dat->hDbEventFirst = hNewEvent;
				SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
			}
			RemoveSendBuffer(dat, i);
			if (dat->sendCount == 0) {
				if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AUTOCLOSE, SRMSGDEFSET_AUTOCLOSE))
					DestroyWindow(hwndDlg);
			}
			break;
		}
	case WM_DESTROY:
		NotifyLocalWinEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSING);
		if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON) {
			NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);
		}
		CallService(MS_SKIN2_RELEASEICON,(WPARAM)dat->userMenuIcon, 0);
		dat->userMenuIcon = NULL;		
		if (dat->sendInfo) {
			int i;
			for (i = 0; i < dat->sendCount; i++) {
				RemoveSendBuffer(dat, i);
			}
			mir_free(dat->sendInfo);
		}
		if (g_dat->flags & SMF_SAVEDRAFTS) {
			saveDraftMessage(dat);
		} else {
			g_dat->draftList = tcmdlist_remove2(g_dat->draftList, dat->hContact);
		}
		tcmdlist_free(dat->cmdList);
		WindowList_Remove(g_dat->hMessageWindowList, hwndDlg);
		//if (!(g_dat->flags&SMF_AVATAR)||!dat->avatarPic)
		SetWindowLong(GetDlgItem(hwndDlg, IDC_SPLITTER), GWL_WNDPROC, (LONG) OldSplitterProc);
		UnsubclassMessageEdit(GetDlgItem(hwndDlg, IDC_MESSAGE));
		UnsubclassLogEdit(GetDlgItem(hwndDlg, IDC_LOG));
		{
			HFONT hFont;
			hFont = (HFONT) SendDlgItemMessage(hwndDlg, IDC_MESSAGE, WM_GETFONT, 0, 0);
			if (hFont != NULL && hFont != (HFONT) SendDlgItemMessage(hwndDlg, IDOK, WM_GETFONT, 0, 0))
				DeleteObject(hFont);
		}
		DBWriteContactSettingByte(dat->hContact, SRMMMOD, "UseRTL", (BYTE) ((dat->flags & SMF_RTL) ? 1 : 0));
		DBWriteContactSettingByte(dat->hContact, SRMMMOD, "DisableUnicode", (BYTE) ((dat->flags & SMF_DISABLE_UNICODE) ? 1 : 0));
		DBWriteContactSettingWord(dat->hContact, SRMMMOD, "CodePage", (WORD) dat->codePage);
		DBWriteContactSettingDword((g_dat->flags & SMF_SAVESPLITTERPERCONTACT) ? dat->hContact : NULL, SRMMMOD, "splitterPos", dat->splitterPos);
		if (dat->avatarPic && !g_dat->avatarServiceExists)
			DeleteObject(dat->avatarPic);
		NotifyLocalWinEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSE);
		if (dat->hContact && (g_dat->flags & SMF_DELTEMP)) {
			if (DBGetContactSettingByte(dat->hContact, "CList", "NotOnList", 0)) {
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)dat->hContact, 0);
			}
		}
		if (dat->hwndLog != NULL) {
			IEVIEWWINDOW ieWindow;
			ieWindow.cbSize = sizeof(IEVIEWWINDOW);
			ieWindow.iType = IEW_DESTROY;
			ieWindow.hwnd = dat->hwndLog;
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		}
		SetWindowLong(hwndDlg, GWL_USERDATA, 0);
		SendMessage(dat->hwndParent, CM_REMOVECHILD, 0, (LPARAM) hwndDlg);
		mir_free(dat);
		break;
	}
	return FALSE;
}
