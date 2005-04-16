/*
SRMM

Copyright 2000-2003 Miranda ICQ/IM project,
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
// IEVIew MOD Begin
#include <m_ieview.h>
// IEVIew MOD End
#pragma hdrstop
#include <ctype.h>
#include <malloc.h>
#include <mbstring.h>
#include "m_smileyadd.h"

extern HINSTANCE g_hInst;

static int logPixelSY;
#define LOGICON_MSG_IN      0
#define LOGICON_MSG_OUT     1
#define LOGICON_MSG_NOTICE  2
static PBYTE pLogIconBmpBits[3];
static int logIconBmpSize[sizeof(pLogIconBmpBits) / sizeof(pLogIconBmpBits[0])];
static HIMAGELIST g_hImageList;

#define STREAMSTAGE_HEADER  0
#define STREAMSTAGE_EVENTS  1
#define STREAMSTAGE_TAIL    2
#define STREAMSTAGE_STOP    3
struct LogStreamData
{
	int stage;
	HANDLE hContact;
	HANDLE hDbEvent, hDbEventLast;
	char *buffer;
	int bufferOffset, bufferLen;
	int eventsToInsert;
	int isEmpty;
	struct MessageWindowData *dlgDat;
};


int safe_wcslen(wchar_t *msg, int maxLen) {
    int i;
	for (i = 0; i < maxLen; i++) {
		if (msg[i] == (wchar_t)0)
			return i;
	}
	return 0;
}

static void AppendToBuffer(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, const char *fmt, ...)
{
	va_list va;
	int charsDone;

	va_start(va, fmt);
	for (;;) {
		charsDone = _vsnprintf(*buffer + *cbBufferEnd, *cbBufferAlloced - *cbBufferEnd, fmt, va);
		if (charsDone >= 0)
			break;
		*cbBufferAlloced += 1024;
		*buffer = (char *) realloc(*buffer, *cbBufferAlloced);
	}
	va_end(va);
	*cbBufferEnd += charsDone;
}

#if defined( _UNICODE )
static int AppendUnicodeToBuffer(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, TCHAR * line)
{
	DWORD textCharsCount = 0;
	char *d;

	int lineLen = wcslen(line) * 9 + 8;
	if (*cbBufferEnd + lineLen > *cbBufferAlloced) {
		cbBufferAlloced[0] += (lineLen + 1024 - lineLen % 1024);
		*buffer = (char *) realloc(*buffer, *cbBufferAlloced);
	}

	d = *buffer + *cbBufferEnd;
	strcpy(d, "{\\uc1 ");
	d += 6;

	for (; *line; line++, textCharsCount++) {
		if (*line == '\r' && line[1] == '\n') {
			CopyMemory(d, "\\par ", 5);
			line++;
			d += 5;
		}
		else if (*line == '\n') {
			CopyMemory(d, "\\par ", 5);
			d += 5;
		}
		else if (*line == '\t') {
			CopyMemory(d, "\\tab ", 5);
			d += 5;
		}
		else if (*line == '\\' || *line == '{' || *line == '}') {
			*d++ = '\\';
			*d++ = (char) *line;
		}
		else if (*line < 128) {
			*d++ = (char) *line;
		}
		else
			d += sprintf(d, "\\u%d ?", *line);
	}

	strcpy(d, "}");
	d++;

	*cbBufferEnd = (int) (d - *buffer);
	return textCharsCount;
}
#endif

//same as above but does "\r\n"->"\\par " and "\t"->"\\tab " too
static int AppendToBufferWithRTF(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, const char *fmt, ...)
{
	va_list va;
	int charsDone, i;

	va_start(va, fmt);
	for (;;) {
		charsDone = _vsnprintf(*buffer + *cbBufferEnd, *cbBufferAlloced - *cbBufferEnd, fmt, va);
		if (charsDone >= 0)
			break;
		*cbBufferAlloced += 1024;
		*buffer = (char *) realloc(*buffer, *cbBufferAlloced);
	}
	va_end(va);
	*cbBufferEnd += charsDone;
	for (i = *cbBufferEnd - charsDone; (*buffer)[i]; i++) {
		if ((*buffer)[i] == '\r' && (*buffer)[i + 1] == '\n') {
			if (*cbBufferEnd + 4 > *cbBufferAlloced) {
				*cbBufferAlloced += 1024;
				*buffer = (char *) realloc(*buffer, *cbBufferAlloced);
			}
			MoveMemory(*buffer + i + 5, *buffer + i + 2, *cbBufferEnd - i - 1);
			CopyMemory(*buffer + i, "\\par ", 5);
			*cbBufferEnd += 3;
		}
		else if ((*buffer)[i] == '\n') {
			if (*cbBufferEnd + 5 > *cbBufferAlloced) {
				*cbBufferAlloced += 1024;
				*buffer = (char *) realloc(*buffer, *cbBufferAlloced);
			}
			MoveMemory(*buffer + i + 5, *buffer + i + 1, *cbBufferEnd - i);
			CopyMemory(*buffer + i, "\\par ", 5);
			*cbBufferEnd += 4;
		}
		else if ((*buffer)[i] == '\t') {
			if (*cbBufferEnd + 5 > *cbBufferAlloced) {
				*cbBufferAlloced += 1024;
				*buffer = (char *) realloc(*buffer, *cbBufferAlloced);
			}
			MoveMemory(*buffer + i + 5, *buffer + i + 1, *cbBufferEnd - i);
			CopyMemory(*buffer + i, "\\tab ", 5);
			*cbBufferEnd += 4;
		}
		else if ((*buffer)[i] == '\\' || (*buffer)[i] == '{' || (*buffer)[i] == '}') {
			if (*cbBufferEnd + 2 > *cbBufferAlloced) {
				*cbBufferAlloced += 1024;
				*buffer = (char *) realloc(*buffer, *cbBufferAlloced);
			}
			MoveMemory(*buffer + i + 1, *buffer + i, *cbBufferEnd - i + 1);
			(*buffer)[i] = '\\';
			++*cbBufferEnd;
			i++;
		}
	}
	return _mbslen(*buffer + *cbBufferEnd);
}

//free() the return value
static char *CreateRTFHeader(struct MessageWindowData *dat)
{
	char *buffer;
	int bufferAlloced, bufferEnd;
	int i;
	LOGFONTA lf;
	COLORREF colour;
	HDC hdc;

	hdc = GetDC(NULL);
	logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);
	bufferEnd = 0;
	bufferAlloced = 1024;
	buffer = (char *) malloc(bufferAlloced);
	buffer[0] = '\0';
	if (dat->flags & SMF_RTL)
		AppendToBuffer(&buffer,&bufferEnd,&bufferAlloced,"{\\rtf1\\ansi\\deff0\\rtldoc{\\fonttbl");
	else
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "{\\rtf1\\ansi\\deff0{\\fonttbl");
	for (i = 0; i < msgDlgFontCount; i++) {
		LoadMsgDlgFont(i, &lf, NULL);
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "{\\f%u\\fnil\\fcharset%u %s;}", i, lf.lfCharSet, lf.lfFaceName);
	}
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "}{\\colortbl ");
	for (i = 0; i < msgDlgFontCount; i++) {
		LoadMsgDlgFont(i, NULL, &colour);
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	}
	if (GetSysColorBrush(COLOR_HOTLIGHT) == NULL)
		colour = RGB(0, 0, 255);
	else
		colour = GetSysColor(COLOR_HOTLIGHT);
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	if (dat->flags & SMF_RTL)
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "}\\rtlpar");
	else
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "}\\pard");
	return buffer;
}

//free() the return value
static char *CreateRTFTail(struct MessageWindowData *dat)
{
	char *buffer;
	int bufferAlloced, bufferEnd;

	bufferEnd = 0;
	bufferAlloced = 1024;
	buffer = (char *) malloc(bufferAlloced);
	buffer[0] = '\0';
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "}");
	return buffer;
}

//return value is static
static char *SetToStyle(int style)
{
	static char szStyle[128];
	LOGFONTA lf;

	LoadMsgDlgFont(style, &lf, NULL);
	wsprintfA(szStyle, "\\f%u\\cf%u\\b%d\\i%d\\fs%u", style, style, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / logPixelSY);
	return szStyle;
}

int DbEventIsShown(DBEVENTINFO * dbei, struct MessageWindowData *dat)
{
	switch (dbei->eventType) {
		case EVENTTYPE_MESSAGE:
			return 1;
		case EVENTTYPE_STATUSCHANGE:
			if (dbei->flags & DBEF_READ)
				return 0;
			return 1;
	}
	return 0;
}


char *TimestampToString(DWORD dwFlags, time_t check, int groupStart)
{
    static char szResult[512];
    char str[80];

    DBTIMETOSTRING dbtts;

    dbtts.cbDest = 70;;
    dbtts.szDest = str;

    if(!groupStart || !(dwFlags & SMF_SHOWDATE)) {
        dbtts.szFormat = (dwFlags & SMF_SHOWSECONDS) ? (char *)"s" : (char *)"t";
        szResult[0] = '\0';
    }
    else {
		struct tm tm_now, tm_today;
		time_t now = time(NULL);
		time_t today;
        tm_now = *localtime(&now);
        tm_today = tm_now;
        tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
        today = mktime(&tm_today);

        if(dwFlags & SMF_RELATIVEDATE && check >= today) {
            dbtts.szFormat = (dwFlags & SMF_SHOWSECONDS) ? (char *)"s" : (char *)"t";
            strcpy(szResult, Translate("Today"));
	        strcat(szResult, ", ");
        }
        else if(dwFlags & SMF_RELATIVEDATE && check > (today - 86400)) {
            dbtts.szFormat = (dwFlags & SMF_SHOWSECONDS) ? (char *)"s" : (char *)"t";
            strcpy(szResult, Translate("Yesterday"));
	        strcat(szResult, ", ");
        }
        else {
            if(dwFlags & SMF_LONGDATE)
                dbtts.szFormat = (dwFlags & SMF_SHOWSECONDS) ? (char *)"D s" : (char *)"D t";
            else
                dbtts.szFormat = (dwFlags & SMF_SHOWSECONDS) ? (char *)"d s" : (char *)"d t";
            szResult[0] = '\0';
        }
    }
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
    strncat(szResult, str, 500);
    return szResult;
}

//free() the return value
static char *CreateRTFFromDbEvent(struct MessageWindowData *dat, HANDLE hContact, HANDLE hDbEvent, int prefixParaBreak, struct LogStreamData *streamData)
{
	char *buffer;
	int bufferAlloced, bufferEnd;
	DBEVENTINFO dbei = { 0 };
	int showColon = 0;
	int isGroupBreak = TRUE;

	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM) hDbEvent, 0);
	if (dbei.cbBlob == -1)
		return NULL;
	dbei.pBlob = (PBYTE) malloc(dbei.cbBlob);
	CallService(MS_DB_EVENT_GET, (WPARAM) hDbEvent, (LPARAM) & dbei);
	if (!DbEventIsShown(&dbei, dat)) {
		free(dbei.pBlob);
		return NULL;
	}
	if (!(dbei.flags & DBEF_SENT) && dbei.eventType == EVENTTYPE_MESSAGE) {
		CallService(MS_DB_EVENT_MARKREAD, (WPARAM) hContact, (LPARAM) hDbEvent);
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM) hContact, (LPARAM) hDbEvent);
	}
	else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
		CallService(MS_DB_EVENT_MARKREAD, (WPARAM) hContact, (LPARAM) hDbEvent);
	}
	bufferEnd = 0;
	bufferAlloced = 1024;
	buffer = (char *) malloc(bufferAlloced);
	buffer[0] = '\0';
	if (prefixParaBreak) {
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\par");
	}
 	if ((g_dat->flags & SMF_GROUPMESSAGES) && dbei.flags == LOWORD(dat->lastEventType)
	  && dbei.eventType == EVENTTYPE_MESSAGE && HIWORD(dat->lastEventType) == EVENTTYPE_MESSAGE
	  && ((dbei.timestamp - dat->lastEventTime) < 86400)) {
		isGroupBreak = FALSE;
	}

	if (g_dat->flags&SMF_SHOWICONS && isGroupBreak) {
		int i = LOGICON_MSG_NOTICE;

		switch (dbei.eventType) {
			case EVENTTYPE_MESSAGE:
				if (dbei.flags & DBEF_SENT) {
					i = LOGICON_MSG_OUT;
				}
				else {
					i = LOGICON_MSG_IN;
				}
				break;
			case EVENTTYPE_STATUSCHANGE:
				i = LOGICON_MSG_NOTICE;
				break;
		}
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\f0\\fs14");
		while (bufferAlloced - bufferEnd < logIconBmpSize[i])
			bufferAlloced += 1024;
		buffer = (char *) realloc(buffer, bufferAlloced);
		CopyMemory(buffer + bufferEnd, pLogIconBmpBits[i], logIconBmpSize[i]);
		bufferEnd += logIconBmpSize[i];
	}

	if (g_dat->flags&SMF_SHOWTIME && (isGroupBreak || (g_dat->flags & SMF_MARKFOLLOWUPS))) {
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, " %s ", SetToStyle(dbei.flags & DBEF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
		AppendToBufferWithRTF(&buffer, &bufferEnd, &bufferAlloced, "%s", TimestampToString(g_dat->flags, dbei.timestamp, isGroupBreak));
		showColon = 1;
	}
	if (!(g_dat->flags&SMF_HIDENAMES) && dbei.eventType != EVENTTYPE_STATUSCHANGE  && isGroupBreak) {
		char *szName = "";
		CONTACTINFO ci;
		ZeroMemory(&ci, sizeof(ci));

		if (dbei.flags & DBEF_SENT) {
			ci.cbSize = sizeof(ci);
			ci.hContact = NULL;
			ci.szProto = dbei.szModule;
			ci.dwFlag = CNF_DISPLAY;
			if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
				// CNF_DISPLAY always returns a string type
				szName = ci.pszVal;
			}
		}
		else
			szName = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, 0);
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, " %s ", SetToStyle(dbei.flags & DBEF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
		AppendToBufferWithRTF(&buffer, &bufferEnd, &bufferAlloced, "%s", szName);
		showColon = 1;
		if (ci.pszVal)
			miranda_sys_free(ci.pszVal);
	}
	if (showColon) {
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s :", SetToStyle(dbei.flags & DBEF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
	}
	switch (dbei.eventType) {
		case EVENTTYPE_MESSAGE:
		if (isGroupBreak && g_dat->flags & SMF_MSGONNEWLINE) {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\line");
		}
		{
#if defined( _UNICODE )
			wchar_t *msg;
#else
			BYTE *msg;
#endif

			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, " %s ", SetToStyle(dbei.flags & DBEF_SENT ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG));
#if defined( _UNICODE )
			{
				int msglen = strlen((char *) dbei.pBlob) + 1;
				if (msglen != (int) dbei.cbBlob && !(dat->flags & SMF_DISABLE_UNICODE)) {
					int wlen;
					msg = (TCHAR *) &dbei.pBlob[msglen];
					wlen = safe_wcslen(msg, (dbei.cbBlob - msglen) / 2);
					if (wlen > 0 && wlen < msglen) {
						AppendUnicodeToBuffer(&buffer, &bufferEnd, &bufferAlloced, msg);
					} else {
						msg = (TCHAR *) malloc(sizeof(TCHAR) * msglen);
						MultiByteToWideChar(dat->codePage, 0, (char *) dbei.pBlob, -1, msg, msglen);
						AppendUnicodeToBuffer(&buffer, &bufferEnd, &bufferAlloced, msg);
						free(msg);
					}
				} else {
					msg = (TCHAR *) malloc(sizeof(TCHAR) * msglen);
					MultiByteToWideChar(dat->codePage, 0, (char *) dbei.pBlob, -1, msg, msglen);
					AppendUnicodeToBuffer(&buffer, &bufferEnd, &bufferAlloced, msg);
					free(msg);
				}
			}
#else
			msg = (BYTE *) dbei.pBlob;
			AppendToBufferWithRTF(&buffer, &bufferEnd, &bufferAlloced, "%s", msg);
#endif
			break;
		}
		case EVENTTYPE_STATUSCHANGE:
		{
			BYTE *msg;
			char *szName = "";
			CONTACTINFO ci;
			ZeroMemory(&ci, sizeof(ci));

			if (dbei.flags & DBEF_SENT) {
				ci.cbSize = sizeof(ci);
				ci.hContact = NULL;
				ci.szProto = dbei.szModule;
				ci.dwFlag = CNF_DISPLAY;
				if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
					// CNF_DISPLAY always returns a string type
					szName = ci.pszVal;
				}
			}
			else
				szName = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, 0);

			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, " %s ", SetToStyle(MSGFONTID_NOTICE));
			msg = (BYTE *) dbei.pBlob;
			AppendToBufferWithRTF(&buffer, &bufferEnd, &bufferAlloced, "%s %s", szName, msg);
			if (ci.pszVal)
				miranda_sys_free(ci.pszVal);
			break;
		}
	}
	dat->lastEventTime = dbei.timestamp;
	dat->lastEventType = MAKELONG(dbei.flags, dbei.eventType);
	free(dbei.pBlob);
	return buffer;
}

static DWORD CALLBACK LogStreamInEvents(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	struct LogStreamData *dat = (struct LogStreamData *) dwCookie;

	if (dat->buffer == NULL) {
		dat->bufferOffset = 0;
		switch (dat->stage) {
			case STREAMSTAGE_HEADER:
				dat->buffer = CreateRTFHeader(dat->dlgDat);
				dat->stage = STREAMSTAGE_EVENTS;
				break;
			case STREAMSTAGE_EVENTS:
				if (dat->eventsToInsert) {
					do {
						dat->buffer = CreateRTFFromDbEvent(dat->dlgDat, dat->hContact, dat->hDbEvent, !dat->isEmpty, dat);
						if (dat->buffer)
							dat->hDbEventLast = dat->hDbEvent;
						dat->hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) dat->hDbEvent, 0);
						if (--dat->eventsToInsert == 0)
							break;
					} while (dat->buffer == NULL && dat->hDbEvent);
					if (dat->buffer) {
						dat->isEmpty = 0;
						break;
					}
				}
				dat->stage = STREAMSTAGE_TAIL;
				//fall through
			case STREAMSTAGE_TAIL:
				dat->buffer = CreateRTFTail(dat->dlgDat);
				dat->stage = STREAMSTAGE_STOP;
				break;
			case STREAMSTAGE_STOP:
				*pcb = 0;
				return 0;
		}
		dat->bufferLen = lstrlenA(dat->buffer);
	}
	*pcb = min(cb, dat->bufferLen - dat->bufferOffset);
	CopyMemory(pbBuff, dat->buffer + dat->bufferOffset, *pcb);
	dat->bufferOffset += *pcb;
	if (dat->bufferOffset == dat->bufferLen) {
		free(dat->buffer);
		dat->buffer = NULL;
	}
	return 0;
}

void StreamInEvents(HWND hwndDlg, HANDLE hDbEventFirst, int count, int fAppend)
{
	EDITSTREAM stream = { 0 };
	struct LogStreamData streamData = { 0 };
	struct MessageWindowData *dat = (struct MessageWindowData *) GetWindowLong(hwndDlg, GWL_USERDATA);
	CHARRANGE oldSel, sel;

// IEVIew MOD Begin
	if (dat->flags & SMF_USEIEVIEW) {
		IEVIEWEVENT event;
		event.cbSize = sizeof(IEVIEWEVENT);
		event.dwFlags = ((dat->flags & SMF_RTL) ? IEEF_RTL : 0) | ((dat->flags & SMF_DISABLE_UNICODE) ? IEEF_NO_UNICODE : 0);
		event.hwnd = dat->hwndLog;
		event.hContact = dat->hContact;
		if (!fAppend) {
			event.iType = IEE_CLEAR_LOG;
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
		}
		event.iType = IEE_LOG_EVENTS;
		event.codepage = dat->codePage;
		event.hDbEventFirst = hDbEventFirst;
		event.count = count;
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
		dat->hDbEventLast = event.hDbEventFirst != NULL ? event.hDbEventFirst : dat->hDbEventLast;
		return;
	}
// IEVIew MOD End

	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_HIDESELECTION, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXGETSEL, 0, (LPARAM) & oldSel);
	streamData.hContact = dat->hContact;
	streamData.hDbEvent = hDbEventFirst;
	streamData.hDbEventLast = dat->hDbEventLast;
	streamData.dlgDat = dat;
	streamData.eventsToInsert = count;
	streamData.isEmpty = fAppend ? GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOG)) == 0 : 1;
	stream.pfnCallback = LogStreamInEvents;
	stream.dwCookie = (DWORD_PTR) & streamData;
	if (fAppend) {
		sel.cpMin = sel.cpMax = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOG));
		SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXSETSEL, 0, (LPARAM) & sel);
	}
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_STREAMIN, fAppend ? SFF_SELECTION | SF_RTF : SF_RTF, (LPARAM) & stream);
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXSETSEL, 0, (LPARAM) & oldSel);
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_HIDESELECTION, FALSE, 0);
	if (ServiceExists(MS_SMILEYADD_REPLACESMILEYS)) {
		SMADD_RICHEDIT2 smre;
		smre.cbSize = sizeof(SMADD_RICHEDIT2);
		smre.hwndRichEditControl = GetDlgItem(hwndDlg, IDC_LOG);
		smre.Protocolname = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) dat->hContact, 0);
		smre.rangeToReplace = NULL;
		smre.useSounds = FALSE;
		smre.disableRedraw = FALSE;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM) &smre);
	}
	dat->hDbEventLast = streamData.hDbEventLast;
	if (GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG), GWL_STYLE) & WS_VSCROLL)
		PostMessage(hwndDlg, DM_SCROLLLOGTOBOTTOM, 0, 0);
}

#define RTFPICTHEADERMAXSIZE   78
void LoadMsgLogIcons(void)
{
	HICON hIcon = NULL;
	HBITMAP hBmp, hoBmp;
	HDC hdc, hdcMem;
	BITMAPINFOHEADER bih = { 0 };
	int widthBytes, i;
	RECT rc;
	HBRUSH hBkgBrush;
	int rtfHeaderSize;
	PBYTE pBmpBits;

	g_hImageList = ImageList_Create(10, 10, IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, sizeof(pLogIconBmpBits) / sizeof(pLogIconBmpBits[0]), 0);
	hBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));
	bih.biSize = sizeof(bih);
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biHeight = 10;
	bih.biPlanes = 1;
	bih.biWidth = 10;
	widthBytes = ((bih.biWidth * bih.biBitCount + 31) >> 5) * 4;
	rc.top = rc.left = 0;
	rc.right = bih.biWidth;
	rc.bottom = bih.biHeight;
	hdc = GetDC(NULL);
	hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	hdcMem = CreateCompatibleDC(hdc);
	pBmpBits = (PBYTE) malloc(widthBytes * bih.biHeight);
	for (i = 0; i < sizeof(pLogIconBmpBits) / sizeof(pLogIconBmpBits[0]); i++) {
		switch (i) {
			case LOGICON_MSG_IN:
				ImageList_AddIcon(g_hImageList, g_dat->hIcons[SMF_ICON_INCOMING]);
				hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_IN, ILD_NORMAL);
				break;
			case LOGICON_MSG_OUT:
				ImageList_AddIcon(g_hImageList, g_dat->hIcons[SMF_ICON_OUTGOING]);
				hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_OUT, ILD_NORMAL);
				break;
			case LOGICON_MSG_NOTICE:
				ImageList_AddIcon(g_hImageList, g_dat->hIcons[SMF_ICON_NOTICE]);
				hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_NOTICE, ILD_NORMAL);
				break;
		}
		pLogIconBmpBits[i] = (PBYTE) malloc(RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2);
		//I can't seem to get binary mode working. No matter.
		rtfHeaderSize = sprintf(pLogIconBmpBits[i], "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, (UINT) bih.biWidth, (UINT)bih.biHeight);
		hoBmp = (HBITMAP) SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBkgBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO *) & bih, DIB_RGB_COLORS);
		{
			int n;
			for (n = 0; n < sizeof(BITMAPINFOHEADER); n++)
				sprintf(pLogIconBmpBits[i] + rtfHeaderSize + n * 2, "%02X", ((PBYTE) & bih)[n]);
			for (n = 0; n < widthBytes * bih.biHeight; n += 4)
				sprintf(pLogIconBmpBits[i] + rtfHeaderSize + (bih.biSize + n) * 2, "%02X%02X%02X%02X", pBmpBits[n], pBmpBits[n + 1], pBmpBits[n + 2], pBmpBits[n + 3]);
		}
		logIconBmpSize[i] = rtfHeaderSize + (bih.biSize + widthBytes * bih.biHeight) * 2 + 1;
		pLogIconBmpBits[i][logIconBmpSize[i] - 1] = '}';
	}
	free(pBmpBits);
	DeleteDC(hdcMem);
	DeleteObject(hBmp);
	ReleaseDC(NULL, hdc);
	DeleteObject(hBkgBrush);
}

void FreeMsgLogIcons(void)
{
	int i;
	for (i = 0; i < sizeof(pLogIconBmpBits) / sizeof(pLogIconBmpBits[0]); i++)
		free(pLogIconBmpBits[i]);
	ImageList_RemoveAll(g_hImageList);
	ImageList_Destroy(g_hImageList);
}
