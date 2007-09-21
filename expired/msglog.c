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
#include <ctype.h>
#include <mbstring.h>
#include "tom.h"

#define MIRANDA_0_5

#define LOGICON_MSG_IN      0
#define LOGICON_MSG_OUT     1
#define LOGICON_MSG_NOTICE  2

#if defined ( _UNICODE )
extern int RTL_Detect(WCHAR *pszwText);
#endif
extern HINSTANCE g_hInst;
static int logPixelSY;
static PBYTE pLogIconBmpBits[3];
static int logIconBmpSize[sizeof(pLogIconBmpBits) / sizeof(pLogIconBmpBits[0])];
static HIMAGELIST g_hImageList;

#define STREAMSTAGE_HEADER  0
#define STREAMSTAGE_EVENTS  1
#define STREAMSTAGE_TAIL    2
#define STREAMSTAGE_STOP    3
struct LogStreamData {
	int stage;
	HANDLE hContact;
	HANDLE hDbEvent, hDbEventLast;
	char *buffer;
	int bufferOffset, bufferLen;
	int eventsToInsert;
	int isFirst;
	struct MessageWindowData *dlgDat;
};

struct EventData {
	int		cbSize;
	int		iType;
	DWORD	dwFlags;
	const char *fontName;
	int			fontSize;
	int         fontStyle;
	COLORREF	color;
	union {
		char *pszNick;		// Nick, usage depends on type of event
		wchar_t *pszNickW;    // Nick - Unicode
	};
	union {
		char *pszText;			// Text, usage depends on type of event
		wchar_t *pszTextW;			// Text - Unicode
	};
	union {
		char *pszText2;			// Text, usage depends on type of event
		wchar_t *pszText2W;			// Text - Unicode
	};
	DWORD	time;
	DWORD	eventType;
	HANDLE	hContact;
};

TCHAR *GetNickname(HANDLE hContact, const char* szProto) {
	char * szBaseNick;
	TCHAR *szName = NULL;
	CONTACTINFO ci;
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
    ci.szProto = (char *)szProto;
	ci.dwFlag = CNF_DISPLAY;
#if defined ( _UNICODE )
	if(IsUnicodeMIM()) {
		ci.dwFlag |= CNF_UNICODE;
    }
#endif
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (ci.pszVal) {
#if defined ( _UNICODE )
				if(IsUnicodeMIM()) {
					if(!_tcscmp((TCHAR *)ci.pszVal, TranslateW(_T("'(Unknown Contact)'")))) {
						ci.dwFlag &= ~CNF_UNICODE;
						if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
							szName = a2t((char *)ci.pszVal);
						}
					} else {
						szName = mir_tstrdup((TCHAR *)ci.pszVal);
					}
				} else {
					szName = a2t((char *)ci.pszVal);
				}
#else
				szName = mir_tstrdup((TCHAR *)ci.pszVal);
#endif
				miranda_sys_free(ci.pszVal);
				if (szName != NULL) {
					return szName;
				}
			}
		}
	}
	szBaseNick = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	if (szBaseNick != NULL) {
#if defined ( _UNICODE )
		int len;
		len = strlen(szBaseNick) + 1;
		szName = (TCHAR *) mir_alloc(len * 2);
	    MultiByteToWideChar(CP_ACP, 0, szBaseNick, -1, szName, len);
		szName[len - 1] = 0;
	    return szName;
#else
	    return mir_tstrdup(szBaseNick);
#endif
	}
    return mir_tstrdup(TranslateT("Unknown Contact"));
}

int DbEventIsShown(DBEVENTINFO * dbei, struct MessageWindowData *dat)
{
	switch (dbei->eventType) {
		case EVENTTYPE_MESSAGE:
			return 1;
		case EVENTTYPE_STATUSCHANGE:
			if (!DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSTATUSCH, SRMSGDEFSET_SHOWSTATUSCH)) {
//			if (dbei->flags & DBEF_READ)
				return 0;
			}
			return 1;
		case EVENTTYPE_FILE:
		case EVENTTYPE_URL:
//			if (dat->hwndLog != NULL)
				return 1;
	}
	return 0;
}


struct EventData *getEventFromDB(struct MessageWindowData *dat, HANDLE hContact, HANDLE hDbEvent) {
	DBEVENTINFO dbei = { 0 };
	struct EventData *event;
	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM) hDbEvent, 0);
	if (dbei.cbBlob == -1) return NULL;
	dbei.pBlob = (PBYTE) mir_alloc(dbei.cbBlob);
	CallService(MS_DB_EVENT_GET, (WPARAM) hDbEvent, (LPARAM) & dbei);
	if (!DbEventIsShown(&dbei, dat)) {
		mir_free(dbei.pBlob);
		return NULL;
	}
	if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_URL)) {
		CallService(MS_DB_EVENT_MARKREAD, (WPARAM) hContact, (LPARAM) hDbEvent);
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM) hContact, (LPARAM) hDbEvent);
	}
	else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
		CallService(MS_DB_EVENT_MARKREAD, (WPARAM) hContact, (LPARAM) hDbEvent);
	}
	event = (struct EventData *) mir_alloc(sizeof(struct EventData));
	memset(event, 0, sizeof(struct EventData));
	event->hContact = hContact;
	event->eventType = dbei.eventType;
	event->dwFlags = (dbei.flags & DBEF_READ ? IEEDF_READ : 0) | (dbei.flags & DBEF_SENT ? IEEDF_SENT : 0) | (dbei.flags & DBEF_RTL ? IEEDF_RTL : 0);
	event->time = dbei.timestamp;
	event->pszNick = NULL;
#if defined( _UNICODE )
	event->dwFlags |= IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK | IEEDF_UNICODE_TEXT2;
	if (event->dwFlags & IEEDF_SENT) {
		event->pszNickW = GetNickname(NULL, dat->szProto);
	} else {
		event->pszNickW = GetNickname(event->hContact, dat->szProto);
	}
	if (event->eventType == EVENTTYPE_FILE) {
		char* filename = ((char *)dbei.pBlob) + sizeof(DWORD);
		char* descr = filename + lstrlenA( filename ) + 1;
		event->pszTextW = a2t(filename);//dat->codePage);
		if ( *descr != 0 ) {
			event->pszText2W = a2t(descr);//dat->codePage);
		}
	} else { //if (event->eventType == EVENTTYPE_MESSAGE) {
		int msglen = strlen((char *) dbei.pBlob) + 1;
		if (msglen != (int) dbei.cbBlob && !(dat->flags & SMF_DISABLE_UNICODE)) {
			int wlen;
			wlen = safe_wcslen((wchar_t*) &dbei.pBlob[msglen], (dbei.cbBlob - msglen) / 2);
			if (wlen > 0 && wlen < msglen) {
				event->pszTextW = mir_wstrdup((wchar_t*) &dbei.pBlob[msglen]);
			} else {
				event->pszTextW = a2tcp((char *) dbei.pBlob, dat->codePage);
			}
		} else {
			event->pszTextW = a2tcp((char *) dbei.pBlob, dat->codePage);
		}
	}
	if ( dat->flags & SMF_RTL) {
		event->dwFlags |= IEEDF_RTL;
	} else if ( RTL_Detect(event->pszTextW)) {
		event->dwFlags |= IEEDF_RTL;
	}
#else
	if (event->dwFlags & IEEDF_SENT) {
		event->pszNick = GetNickname(NULL, dat->szProto);
	} else {
		event->pszNick = GetNickname(event->hContact, dat->szProto);
	}
	if (event->eventType == EVENTTYPE_FILE) {
		char* filename = ((char *)dbei.pBlob) + sizeof(DWORD);
		char* descr = filename + lstrlenA( filename ) + 1;
		event->pszText = mir_strdup(filename);
		if ( *descr != 0 ) {
			event->pszText2 = mir_strdup(descr);
		}
	} else {
		event->pszText = mir_strdup((char *) dbei.pBlob);
	}
#endif
	mir_free(dbei.pBlob);
	return event;
}

static void freeEvent(struct EventData *event) {
#if defined( _UNICODE )
	if (event->pszNickW != NULL) mir_free (event->pszNickW);
	if (event->pszTextW != NULL) mir_free (event->pszTextW);
	if (event->pszText2W != NULL) mir_free (event->pszText2W);
#else
	if (event->pszNick != NULL) mir_free (event->pszNick);
	if (event->pszText != NULL) mir_free (event->pszText);
	if (event->pszText2 != NULL) mir_free (event->pszText2);
#endif
	mir_free(event);
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
		*buffer = (char *) mir_realloc(*buffer, *cbBufferAlloced);
	}
	va_end(va);
	*cbBufferEnd += charsDone;
}

static int AppendAnsiToBufferL(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, unsigned char * line, int maxLen) 
{
	int textCharsCount = 0;
	char *d;
	int wasEOL = 0;
	unsigned char *maxLine = line + maxLen;
	int lineLen = strlen(line) * 9 + 8;
	if (*cbBufferEnd + lineLen > *cbBufferAlloced) {
		cbBufferAlloced[0] += (lineLen + 1024 - lineLen % 1024);
		*buffer = (char *) mir_realloc(*buffer, *cbBufferAlloced);
	}

	d = *buffer + *cbBufferEnd;
	strcpy(d, "{");
	d += 1;

	for (; *line && (maxLen < 0 || line < maxLine); line++, textCharsCount++) {
		wasEOL = 0;
		if (*line == '\r' && line[1] == '\n') {
			CopyMemory(d, "\\par ", 5);
			wasEOL = 1;
			d += 5;
			line++;
		}
		else if (*line == '\n') {
			CopyMemory(d, "\\par ", 5);
			wasEOL = 1;
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
			d += sprintf(d, "\\'%x", *line);
	}
	if (wasEOL) {
		CopyMemory(d, " ", 1);
		d++;
	}
	strcpy(d, "}");
	d++;

	*cbBufferEnd = (int) (d - *buffer);
	return textCharsCount;
}


static int AppendUnicodeToBufferL(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, WCHAR * line, int maxLen) 
{
	int textCharsCount = 0;
	char *d;
	int wasEOL = 0;
	WCHAR *maxLine = line + maxLen;
	int lineLen = wcslen(line) * 9 + 8;
	if (*cbBufferEnd + lineLen > *cbBufferAlloced) {
		cbBufferAlloced[0] += (lineLen + 1024 - lineLen % 1024);
		*buffer = (char *) mir_realloc(*buffer, *cbBufferAlloced);
	}

	d = *buffer + *cbBufferEnd;
	strcpy(d, "{\\uc1 ");
	d += 6;

	for (; *line && (maxLen < 0 || line < maxLine); line++, textCharsCount++) {
		wasEOL = 0;
		if (*line == '\r' && line[1] == '\n') {
			CopyMemory(d, "\\par ", 5);
			wasEOL = 1;
			d += 5;
			line++;
		}
		else if (*line == '\n') {
			CopyMemory(d, "\\par ", 5);
			wasEOL = 1;
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
	if (wasEOL) {
		CopyMemory(d, " ", 1);
		d++;
	}
	strcpy(d, "}");
	d++;

	*cbBufferEnd = (int) (d - *buffer);
	return textCharsCount;
}

static int AppendAnsiToBuffer(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, unsigned char * line) 
{
	return AppendAnsiToBufferL(buffer, cbBufferEnd, cbBufferAlloced, line, -1);
}

static int AppendUnicodeToBuffer(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, WCHAR * line) 
{
	return AppendUnicodeToBufferL(buffer, cbBufferEnd, cbBufferAlloced, line, -1);
}

static int AppendTToBuffer(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, TCHAR * line)
{
#if defined ( _UNICODE )
	return AppendUnicodeToBuffer(buffer, cbBufferEnd, cbBufferAlloced, line);
#else
	return AppendAnsiToBuffer(buffer, cbBufferEnd, cbBufferAlloced, line);
#endif
}

//mir_free() the return value
static char *CreateRTFHeader(struct MessageWindowData *dat)
{
	char *buffer;
	int bufferAlloced, bufferEnd;
	int i;
	LOGFONT lf;
	COLORREF colour;
	HDC hdc;
	int charset = 0;
	BOOL forceCharset = FALSE;
#if !defined ( _UNICODE )
		if (dat->codePage != CP_ACP) {
			CHARSETINFO csi;
 			if(TranslateCharsetInfo((DWORD*)dat->codePage, &csi, TCI_SRCCODEPAGE)) {
				forceCharset = TRUE;
				charset = csi.ciCharset;
			}
		}
#endif

	hdc = GetDC(NULL);
	logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);
	bufferEnd = 0;
	bufferAlloced = 1024;
	buffer = (char *) mir_alloc(bufferAlloced);
	buffer[0] = '\0';
	if (dat->flags & SMF_RTL)
		AppendToBuffer(&buffer,&bufferEnd,&bufferAlloced,"{\\rtf1\\ansi\\deff0{\\fonttbl");
	else
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "{\\rtf1\\ansi\\deff0{\\fonttbl");
	for (i = 0; i < fontOptionsListSize; i++) {
		LoadMsgDlgFont(i, &lf, NULL);
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "{\\f%u\\fnil\\fcharset%u " TCHAR_STR_PARAM ";}", i, 
			(!forceCharset) ? lf.lfCharSet : charset, lf.lfFaceName);
	}
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "}{\\colortbl ");
	for (i = 0; i < fontOptionsListSize; i++) {
		LoadMsgDlgFont(i, NULL, &colour);
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	}
	if (GetSysColorBrush(COLOR_HOTLIGHT) == NULL)
		colour = RGB(0, 0, 255);
	else
		colour = GetSysColor(COLOR_HOTLIGHT);
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, RGB(224,224,224));
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_INCOMINGBKGCOLOUR, RGB(224,224,224));
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_OUTGOINGBKGCOLOUR, RGB(224,224,224));
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_LINECOLOUR, RGB(224,224,224));
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "}");
//	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\li30\\ri30\\fi0\\tx0");
	return buffer;
}

//mir_free() the return value
static char *CreateRTFTail(struct MessageWindowData *dat)
{
	char *buffer;
	int bufferAlloced, bufferEnd;

	bufferEnd = 0;
	bufferAlloced = 1024;
	buffer = (char *) mir_alloc(bufferAlloced);
	buffer[0] = '\0';
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "}");
	return buffer;
}

//return value is static
static char *SetToStyle(int style)
{
	static char szStyle[128];
	LOGFONT lf;

	LoadMsgDlgFont(style, &lf, NULL);
	wsprintfA(szStyle, "\\f%u\\cf%u\\b%d\\i%d\\fs%u", style, style, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / logPixelSY);
	return szStyle;
}

TCHAR *TimestampToString(DWORD dwFlags, time_t check, int groupStart)
{
    static TCHAR szResult[512];
    TCHAR str[80];

    DBTIMETOSTRINGT dbtts;

    dbtts.cbDest = 70;;
    dbtts.szDest = str;

    if(!groupStart || !(dwFlags & SMF_SHOWDATE)) {
        dbtts.szFormat = (dwFlags & SMF_SHOWSECONDS) ? _T("s") : _T("t");
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
            dbtts.szFormat = (dwFlags & SMF_SHOWSECONDS) ? _T("s") : _T("t");
            lstrcpy(szResult, TranslateT("Today"));
	        lstrcat(szResult, _T(", "));
        }
        else if(dwFlags & SMF_RELATIVEDATE && check > (today - 86400)) {
            dbtts.szFormat = (dwFlags & SMF_SHOWSECONDS) ? _T("s") : _T("t");
            lstrcpy(szResult, TranslateT("Yesterday"));
	        lstrcat(szResult, _T(", "));
        }
        else {
            if(dwFlags & SMF_LONGDATE)
                dbtts.szFormat = (dwFlags & SMF_SHOWSECONDS) ? _T("D s") : _T("D t");
            else
                dbtts.szFormat = (dwFlags & SMF_SHOWSECONDS) ? _T("d s") : _T("d t");
            szResult[0] = '\0';
        }
    }
#if defined ( _UNICODE )
	CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, check, (LPARAM) & dbtts);
#else
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
#endif
    _tcsncat(szResult, str, 500);
    return szResult;
}

int isSameDate(time_t time1, time_t time2)
{
    struct tm tm_t1, tm_t2;
    tm_t1 = *localtime((time_t *)(&time1));
    tm_t2 = *localtime((time_t *)(&time2));
    if (tm_t1.tm_year == tm_t2.tm_year && tm_t1.tm_mon == tm_t2.tm_mon
		&& tm_t1.tm_mday == tm_t2.tm_mday) {
		return 1;
	}
	return 0;
}

static int DetectURLW(wchar_t *text, BOOL firstChar) {
	wchar_t c;
	struct prefix_s {
		wchar_t *text;
		int length;
	} prefixes[12] = {
		{L"http:", 5},
		{L"file:", 5},
		{L"mailto:", 7},
		{L"ftp:", 4},
		{L"https:", 6},
		{L"gopher:", 7},
		{L"nntp:", 5},
		{L"prospero:", 9},
		{L"telnet:", 7},
		{L"news:", 5},
		{L"wais:", 5},
		{L"www.", 4}
	};
	c = firstChar ? ' ' : text[-1];
	if (!((c >= '0' && c<='9') || (c >= 'A' && c<='Z') || (c >= 'a' && c<='z'))) {
		int found = 0;
		int i, len = 0;
		int prefixlen = SIZEOF(prefixes);
		for (i = 0; i < prefixlen; i++) {
			if (!wcsncmp(text, prefixes[i].text, prefixes[i].length)) {
				len = prefixes[i].length;
				found = 1;
				break;
			}
		}
		if (found) {
			for (; text[len]!='\n' && text[len]!='\r' && text[len]!='\t' && text[len]!=' ' && text[len]!='\0';  len++);
			for (; len > 0; len --) {
				if ((text[len-1] >= '0' && text[len-1]<='9') || (text[len-1] >= 'A' && text[len-1]<='Z') || (text[len-1] >= 'a' && text[len-1]<='z')) {
					break;
				}
			}
			return len;
		}
	}
	return 0;
}

static int DetectURL(const char* text, BOOL firstChar) {
	char c;
	struct prefix_s {
		char *text;
		int length;
	} prefixes[12] = {
		{"http:", 5},
		{"file:", 5},
		{"mailto:", 7},
		{"ftp:", 4},
		{"https:", 6},
		{"gopher:", 7},
		{"nntp:", 5},
		{"prospero:", 9},
		{"telnet:", 7},
		{"news:", 5},
		{"wais:", 5},
		{"www.", 4}
	};	
	c = firstChar ? ' ' : text[-1];
	if (!((c >= '0' && c<='9') || (c >= 'A' && c<='Z') || (c >= 'a' && c<='z'))) {
		int found = 0;
		int i, len = 0;
		int prefixlen = SIZEOF(prefixes);
		for (i = 0; i < prefixlen; i++) {
			if (!strncmp(text, prefixes[i].text, prefixes[i].length)) {
				len = prefixes[i].length;
				found = 1;
				break;
			}
		}
		if (found) {
			for (; text[len]!='\n' && text[len]!='\r' && text[len]!='\t' && text[len]!=' ' && text[len]!='\0';  len++);
			for (; len > 0; len --) {
				if ((text[len-1] >= '0' && text[len-1]<='9') || (text[len-1] >= 'A' && text[len-1]<='Z') || (text[len-1] >= 'a' && text[len-1]<='z')) {
					break;
				}
			}
			return len;
		}
	}
	return 0;
}

static void AppendWithCustomLinks(struct EventData *event, int style, char **buffer, int *bufferEnd, int *bufferAlloced) {
	int lasttoken = 0, newtoken = 0;
	int laststart = 0, newlen = 0;
	int j, len;
	if (event->dwFlags & IEEDF_UNICODE_TEXT) {
		len = wcslen(event->pszTextW);
	} else {
		len = strlen(event->pszText);
	}
	for (j = 0; j < len ; j+=newlen) {
		int l;
		newtoken = 0;
		newlen = 1;
		if (event->dwFlags & IEEDF_UNICODE_TEXT) {
			l = DetectURLW(event->pszTextW+j, j==0);
		} else {
			l = DetectURL(event->pszText+j, j==0);
		}
		if (l > 0) {
			newtoken = 1;
			newlen = l;
		}
		if (j == 0) {
			lasttoken = newtoken;
		}
		if (newtoken != lasttoken) {
			if (lasttoken == 0) {
				AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(style));
			} else {
				AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(event->dwFlags & IEEDF_SENT ? MSGFONTID_MYURL : MSGFONTID_YOURURL));
			}
			if (event->dwFlags & IEEDF_UNICODE_TEXT) {
				AppendUnicodeToBufferL(buffer, bufferEnd, bufferAlloced, event->pszTextW + laststart, j - laststart);
			} else {
				AppendAnsiToBufferL(buffer, bufferEnd, bufferAlloced, event->pszText + laststart, j - laststart);
			}
			laststart = j;
			lasttoken = newtoken;
		}
	}
	if (len - laststart > 0) {
		if (lasttoken == 0) {
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(style));
		} else {
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(event->dwFlags & IEEDF_SENT ? MSGFONTID_MYURL : MSGFONTID_YOURURL));
		}
		if (event->dwFlags & IEEDF_UNICODE_TEXT) {
			AppendUnicodeToBufferL(buffer, bufferEnd, bufferAlloced, event->pszTextW + laststart, len - laststart);
		} else {
			AppendAnsiToBufferL(buffer, bufferEnd, bufferAlloced, event->pszText + laststart, len - laststart);
		}
	}
}

//mir_free() the return value
static char *CreateRTFFromDbEvent2(struct MessageWindowData *dat, struct EventData *event, struct LogStreamData *streamData)
{
	char *buffer;
	int bufferAlloced, bufferEnd;
	int style, showColon = 0;
	int isGroupBreak = TRUE;
	int highlight = 0;
	bufferEnd = 0;
	bufferAlloced = 1024;
	buffer = (char *) mir_alloc(bufferAlloced);
	buffer[0] = '\0';

 	if ((g_dat->flags & SMF_GROUPMESSAGES) && event->dwFlags == LOWORD(dat->lastEventType)
	  && event->eventType == EVENTTYPE_MESSAGE && HIWORD(dat->lastEventType) == EVENTTYPE_MESSAGE
	  && (isSameDate(event->time, dat->lastEventTime))
//	  && ((dbei.timestamp - dat->lastEventTime) < 86400)
	  && ((((int)event->time < dat->startTime) == (dat->lastEventTime < dat->startTime)) || !(event->dwFlags & IEEDF_READ))) {
		isGroupBreak = FALSE;
	}

	if (!streamData->isFirst && !dat->isMixed) {
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\par");
	}
	if (event->dwFlags & IEEDF_RTL) {
		dat->isMixed = 1;
	}
	if (!streamData->isFirst && isGroupBreak && (g_dat->flags & SMF_DRAWLINES)) {
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\sl-1\\slmult0\\highlight%d\\cf%d\\fs1  \\par\\sl0", fontOptionsListSize + 4, fontOptionsListSize + 4);
	}
	if ( streamData->isFirst ) {
		if (event->dwFlags & IEEDF_RTL) {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\rtlpar");
		} else {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\ltrpar");
		}
	} else {
		if (event->dwFlags & IEEDF_RTL) {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\rtlpar");
		} else {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\ltrpar");
		}
	}
	if (event->eventType == EVENTTYPE_MESSAGE) {
		highlight = fontOptionsListSize + 2 + ((event->dwFlags & IEEDF_SENT) ? 1 : 0);
	} else {
		highlight = fontOptionsListSize + 1;
	}
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\highlight%d\\cf%d", highlight , highlight );
	streamData->isFirst = FALSE;
	if (dat->isMixed) {
		if (event->dwFlags & IEEDF_RTL) {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\ltrch\\rtlch");
		} else {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\rtlch\\ltrch");
		}
	}
	if (g_dat->flags&SMF_SHOWICONS && isGroupBreak) {
		int i = LOGICON_MSG_NOTICE;

		switch (event->eventType) {
			case EVENTTYPE_MESSAGE:
				if (event->dwFlags & IEEDF_SENT) {
					i = LOGICON_MSG_OUT;
				}
				else {
					i = LOGICON_MSG_IN;
				}
				break;
			case EVENTTYPE_STATUSCHANGE:
			case EVENTTYPE_URL:
			case EVENTTYPE_FILE:
				i = LOGICON_MSG_NOTICE;
				break;
		}
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\fs1  ");
		while (bufferAlloced - bufferEnd < logIconBmpSize[i])
			bufferAlloced += 1024;
		buffer = (char *) mir_realloc(buffer, bufferAlloced);
		CopyMemory(buffer + bufferEnd, pLogIconBmpBits[i], logIconBmpSize[i]);
		bufferEnd += logIconBmpSize[i];
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, " ");
	}
	if (g_dat->flags&SMF_SHOWTIME &&
		(event->eventType != EVENTTYPE_MESSAGE ||
		!(g_dat->flags & SMF_GROUPMESSAGES) ||
		(isGroupBreak && !(g_dat->flags & SMF_MARKFOLLOWUPS)) ||  (!isGroupBreak && (g_dat->flags & SMF_MARKFOLLOWUPS))))
	{
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s ", SetToStyle(event->dwFlags & IEEDF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
		AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TimestampToString(g_dat->flags, event->time, isGroupBreak));
		if (event->eventType != EVENTTYPE_MESSAGE) {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s: ", SetToStyle(event->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
		}
		showColon = 1;
	}
	if ((!(g_dat->flags&SMF_HIDENAMES) && event->eventType == EVENTTYPE_MESSAGE && isGroupBreak) || event->eventType == EVENTTYPE_STATUSCHANGE) {
		if (event->eventType == EVENTTYPE_MESSAGE) {
			if (showColon) {
				AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, " %s ", SetToStyle(event->dwFlags & IEEDF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
			} else {
				AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s ", SetToStyle(event->dwFlags & IEEDF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
			}
		} else {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s ", SetToStyle(MSGFONTID_NOTICE));
		}
#if defined( _UNICODE )
		if (event->dwFlags & IEEDF_UNICODE_NICK) {
			AppendUnicodeToBuffer(&buffer, &bufferEnd, &bufferAlloced, event->pszNickW);
		} else {
			AppendAnsiToBuffer(&buffer, &bufferEnd, &bufferAlloced, event->pszNick);
		}
#else
		AppendAnsiToBuffer(&buffer, &bufferEnd, &bufferAlloced, event->pszNick);
#endif
		showColon = 1;
		if (event->eventType == EVENTTYPE_MESSAGE && g_dat->flags & SMF_GROUPMESSAGES) {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\par");
			showColon = 0;
		}
	}

	if (g_dat->flags&SMF_SHOWTIME && g_dat->flags & SMF_GROUPMESSAGES && g_dat->flags & SMF_MARKFOLLOWUPS
		&& event->eventType == EVENTTYPE_MESSAGE && isGroupBreak) {
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, " %s ", SetToStyle(event->dwFlags & IEEDF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
		AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TimestampToString(g_dat->flags, event->time, isGroupBreak));
		showColon = 1;
	}
	if (showColon && event->eventType == EVENTTYPE_MESSAGE) {
		if (event->dwFlags & IEEDF_RTL) {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\~%s: ", SetToStyle(event->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
		} else {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s: ", SetToStyle(event->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
		}
	}
	switch (event->eventType) {
		case EVENTTYPE_MESSAGE:
		if (g_dat->flags & SMF_MSGONNEWLINE && showColon) {
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\par");
		}
		style = event->dwFlags & IEEDF_SENT ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG;
		AppendWithCustomLinks(event, style, &buffer, &bufferEnd, &bufferAlloced);
		/*		
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s ", SetToStyle(event->dwFlags & IEEDF_SENT ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG));
		if (event->dwFlags & IEEDF_UNICODE_TEXT) {
			AppendUnicodeToBuffer(&buffer, &bufferEnd, &bufferAlloced, event->pszTextW);
		} else {
			AppendAnsiToBuffer(&buffer, &bufferEnd, &bufferAlloced, event->pszText);
		}
		*/
		break;
		case EVENTTYPE_STATUSCHANGE:
		case EVENTTYPE_URL:
		case EVENTTYPE_FILE:
		{
			style = MSGFONTID_NOTICE;
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s ", SetToStyle(style));
			if (event->eventType == EVENTTYPE_FILE) {
				if (event->dwFlags & IEEDF_SENT) {
					AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TranslateT("File sent"));
				} else {
					AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TranslateT("File received"));
				}
				AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, _T(":"));
			} else if (event->eventType == EVENTTYPE_URL) {
				if (event->dwFlags & IEEDF_SENT) {
					AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TranslateT("URL sent"));
				} else {
					AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TranslateT("URL received"));
				}
				AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, _T(":"));
			}
			AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, _T(" "));
			
			if (event->dwFlags & IEEDF_UNICODE_TEXT) {
				AppendUnicodeToBuffer(&buffer, &bufferEnd, &bufferAlloced, event->pszTextW);
			} else {
				AppendAnsiToBuffer(&buffer, &bufferEnd, &bufferAlloced, event->pszText);
			}
			if (event->pszText2W != NULL) {
				AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, _T(" ("));
				if (event->dwFlags & IEEDF_UNICODE_TEXT2) {
					AppendUnicodeToBuffer(&buffer, &bufferEnd, &bufferAlloced, event->pszText2W);
				} else {
					AppendAnsiToBuffer(&buffer, &bufferEnd, &bufferAlloced, event->pszText2);
				}
				AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, _T(")"));
			}
			break;
		}
	}
	if (dat->isMixed) {
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\par");
	}

	dat->lastEventTime = event->time;
	dat->lastEventType = MAKELONG(event->dwFlags, event->eventType);
	dat->lastEventContact = event->hContact;
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
#ifdef MIRANDA_0_5
						struct EventData *event = getEventFromDB(dat->dlgDat, dat->hContact, dat->hDbEvent);
						dat->buffer = NULL;
						if (event != NULL) {
							dat->buffer = CreateRTFFromDbEvent2(dat->dlgDat, event, dat);
							freeEvent(event);
						}
#else
						dat->buffer = CreateRTFFromDbEvent(dat->dlgDat, dat->hContact, dat->hDbEvent, !dat->isFirst, dat);
#endif
						if (dat->buffer)
							dat->hDbEventLast = dat->hDbEvent;
						dat->hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) dat->hDbEvent, 0);
						if (--dat->eventsToInsert == 0)
							break;
					} while (dat->buffer == NULL && dat->hDbEvent);
					if (dat->buffer) {
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
		mir_free(dat->buffer);
		dat->buffer = NULL;
	}
	return 0;
}
/*
#ifndef CFE_LINK
#define CFE_LINK 32
#endif


static const CLSID IID_ITextDocument=
{ 0x8CC497C0,0xA1DF,0x11CE,
    { 0x80,0x98, 0x00,0xAA,
      0x00,0x47,0xBE,0x5D} };

void AutoURLDetect(HWND hwnd, CHARRANGE* sel) {
	CHARFORMAT2 cf;
	long cnt;
	BSTR btxt = 0;
	CHARRANGE oldSel;
	LOGFONT lf;
	COLORREF colour;

	IRichEditOle* RichEditOle;
	ITextDocument* TextDocument;
	ITextRange* TextRange;
	ITextSelection* TextSelection;

	LoadMsgDlgFont(MSGFONTID_MYMSG, &lf, &colour);

	SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle);
	if (RichEditOle->lpVtbl->QueryInterface(RichEditOle, &IID_ITextDocument, (void**)&TextDocument) != S_OK)
	{
		RichEditOle->lpVtbl->Release(RichEditOle);
		return;
	}
	// retrieve text range
	if (TextDocument->lpVtbl->Range(TextDocument,sel->cpMin, sel->cpMax, &TextRange) != S_OK) 
	{
		TextDocument->lpVtbl->Release(TextDocument);
		RichEditOle->lpVtbl->Release(RichEditOle);
		return;
	}
	
	// retrieve text to parse for URLs 
	if (TextRange->lpVtbl->GetText(TextRange, &btxt) != S_OK)
	{
		TextRange->lpVtbl->Release(TextRange);
		TextDocument->lpVtbl->Release(TextDocument);
		RichEditOle->lpVtbl->Release(RichEditOle);
		return;
	}

	TextRange->lpVtbl->Release(TextRange);
	
	// disable screen updates
	
	TextDocument->lpVtbl->Freeze(TextDocument, &cnt);
	
	TextDocument->lpVtbl->GetSelection(TextDocument, &TextSelection);

	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_LINK | CFM_COLOR | CFM_UNDERLINE | CFM_BOLD | CFM_ITALIC | CFM_FACE | CFM_SIZE;
	cf.dwEffects = CFE_UNDERLINE | (lf.lfWeight >= FW_BOLD ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
	_tcsncpy(cf.szFaceName, lf.lfFaceName, SIZEOF(cf.szFaceName));
	cf.crTextColor = RGB(255,255,255);//colour;
	cf.yHeight = 20 * lf.lfHeight;
	
	//text = GetRichEditSelection(hwnd);
	if (btxt!=NULL) {
		int cpMin = sel->cpMin;
		int cpMax = sel->cpMax;
		int i, j, len = _tcslen(btxt);
		for (j = 0; j < len ; j++) {
			int l = DetectURL(btxt+j);
			if (l > 0) {
				sel->cpMin = cpMin + j;
				sel->cpMax = cpMin + j + l;
				TextSelection->lpVtbl->SetRange(TextSelection, cpMin + j, cpMin + j + l);
				SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				j+= l-1;
			}
		}
	} 	
	TextSelection->lpVtbl->SetRange(TextSelection,oldSel.cpMin, oldSel.cpMax);
	TextSelection->lpVtbl->Release(TextSelection);
	TextDocument->lpVtbl->Unfreeze(TextDocument,&cnt);
	SysFreeString(btxt);
	TextDocument->lpVtbl->Release(TextDocument);
	RichEditOle->lpVtbl->Release(RichEditOle);
	UpdateWindow(hwnd);
}
*/
void StreamInEvents(HWND hwndDlg, HANDLE hDbEventFirst, int count, int fAppend)
{
	FINDTEXTEXA fi;
	EDITSTREAM stream = { 0 };
	struct LogStreamData streamData = { 0 };
	struct MessageWindowData *dat = (struct MessageWindowData *) GetWindowLong(hwndDlg, GWL_USERDATA);
	CHARRANGE oldSel, sel;

// IEVIew MOD Begin
	if (dat->hwndLog != NULL) {
		IEVIEWEVENT event;
		IEVIEWWINDOW ieWindow;
		ZeroMemory(&event, sizeof(event));
		event.cbSize = sizeof(event);
		event.dwFlags = ((dat->flags & SMF_RTL) ? IEEF_RTL : 0) | ((dat->flags & SMF_DISABLE_UNICODE) ? IEEF_NO_UNICODE : 0);
		event.hwnd = dat->hwndLog;
		event.hContact = dat->hContact;
		event.codepage = dat->codePage;
		event.pszProto = dat->szProto;
		if (!fAppend) {
			event.iType = IEE_CLEAR_LOG;
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
		}
		event.iType = IEE_LOG_DB_EVENTS;
		event.hDbEventFirst = hDbEventFirst;
		event.count = count;
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
		dat->hDbEventLast = event.hDbEventFirst != NULL ? event.hDbEventFirst : dat->hDbEventLast;

		ZeroMemory(&ieWindow, sizeof(ieWindow));
		ieWindow.cbSize = sizeof(ieWindow);
		ieWindow.iType = IEW_SCROLLBOTTOM;
		ieWindow.hwnd = dat->hwndLog;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
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
	streamData.isFirst = fAppend ? GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOG)) == 0 : 1;
	stream.pfnCallback = LogStreamInEvents;
	stream.dwCookie = (DWORD_PTR) & streamData;
	sel.cpMin = 0;
	if (fAppend) {
        GETTEXTLENGTHEX gtxl = {0};
#if defined( _UNICODE )
        gtxl.codepage = 1200;
#else
        gtxl.codepage = CP_ACP;
#endif
        gtxl.codepage = 1200;
        gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMCHARS;
        fi.chrg.cpMin = SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
        sel.cpMin = sel.cpMax = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOG));
        SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXSETSEL, 0, (LPARAM) & sel);
    } else {
		SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
        sel.cpMin = 0;
		sel.cpMax = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOG));
        SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXSETSEL, 0, (LPARAM) & sel);
        fi.chrg.cpMin = 0;
		dat->isMixed = 0;
	}
//SFF_SELECTION |
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_STREAMIN, fAppend ? SFF_SELECTION | SF_RTF : SFF_SELECTION |  SF_RTF, (LPARAM) & stream);
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXSETSEL, 0, (LPARAM) & oldSel);
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_HIDESELECTION, FALSE, 0);
	/*
	if (fi.chrg.cpMin > 0) {
		sel.cpMin = fi.chrg.cpMin;
	} else {
		sel.cpMin = 0;
	}
	{
		int len;
		len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOG));
		sel.cpMax = len;//100;//-1;
		//AutoURLDetect(GetDlgItem(hwndDlg, IDC_LOG), &sel);
	}
	*/
	if (ServiceExists(MS_SMILEYADD_REPLACESMILEYS)) {
		SMADD_RICHEDIT3 smre;
		smre.cbSize = sizeof(SMADD_RICHEDIT3);
		smre.hwndRichEditControl = GetDlgItem(hwndDlg, IDC_LOG);
		smre.Protocolname = dat->szProto;
        if (dat->szProto!=NULL && strcmp(dat->szProto,"MetaContacts")==0) {
            HANDLE hContact = (HANDLE) CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM) dat->hContact, 0);
            if (hContact!=NULL) {
                smre.Protocolname = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
            }
        }
		if (fi.chrg.cpMin > 0) {
			sel.cpMin = fi.chrg.cpMin;
			sel.cpMax = -1;
			smre.rangeToReplace = &sel;
		} else {
			smre.rangeToReplace = NULL;
		}
		//smre.rangeToReplace = NULL;
		smre.disableRedraw = TRUE;
		smre.hContact = dat->hContact;
		smre.flags = 0;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM) &smre);
	}
//	if (GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG), GWL_STYLE) & WS_VSCROLL)
	{
		int len;
		len = GetWindowTextLengthA(GetDlgItem(hwndDlg, IDC_LOG));
		SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETSEL, len - 1, len - 1);
	}
	dat->hDbEventLast = streamData.hDbEventLast;
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
	HBRUSH hBrush;
	HBRUSH hBkgBrush;
	HBRUSH hInBkgBrush;
	HBRUSH hOutBkgBrush;
	int rtfHeaderSize;
	PBYTE pBmpBits;

	g_hImageList = ImageList_Create(10, 10, IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, sizeof(pLogIconBmpBits) / sizeof(pLogIconBmpBits[0]), 0);
	hBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));
	hInBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_INCOMINGBKGCOLOUR, SRMSGDEFSET_INCOMINGBKGCOLOUR));
	hOutBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_OUTGOINGBKGCOLOUR, SRMSGDEFSET_OUTGOINGBKGCOLOUR));
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
	pBmpBits = (PBYTE) mir_alloc(widthBytes * bih.biHeight);
	hBrush = hBkgBrush;
	for (i = 0; i < sizeof(pLogIconBmpBits) / sizeof(pLogIconBmpBits[0]); i++) {
		switch (i) {
			case LOGICON_MSG_IN:
				ImageList_AddIcon(g_hImageList, g_dat->hIcons[SMF_ICON_INCOMING]);
				hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_IN, ILD_NORMAL);
				hBrush = hInBkgBrush;
				break;
			case LOGICON_MSG_OUT:
				ImageList_AddIcon(g_hImageList, g_dat->hIcons[SMF_ICON_OUTGOING]);
				hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_OUT, ILD_NORMAL);
				hBrush = hOutBkgBrush;
				break;
			case LOGICON_MSG_NOTICE:
				ImageList_AddIcon(g_hImageList, g_dat->hIcons[SMF_ICON_NOTICE]);
				hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_NOTICE, ILD_NORMAL);
				//hBrush = hInBkgBrush;
				hBrush = hBkgBrush;
				break;
		}
		pLogIconBmpBits[i] = (PBYTE) mir_alloc(RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2);
		//I can't seem to get binary mode working. No matter.
		rtfHeaderSize = sprintf(pLogIconBmpBits[i], "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, (UINT) bih.biWidth, (UINT)bih.biHeight);
		hoBmp = (HBITMAP) SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO *) & bih, DIB_RGB_COLORS);
		DestroyIcon(hIcon);
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
	mir_free(pBmpBits);
	DeleteDC(hdcMem);
	DeleteObject(hBmp);
	ReleaseDC(NULL, hdc);
	DeleteObject(hBkgBrush);
	DeleteObject(hInBkgBrush);
	DeleteObject(hOutBkgBrush);
}

void FreeMsgLogIcons(void)
{
	int i;
	for (i = 0; i < sizeof(pLogIconBmpBits) / sizeof(pLogIconBmpBits[0]); i++)
		mir_free(pLogIconBmpBits[i]);
	ImageList_RemoveAll(g_hImageList);
	ImageList_Destroy(g_hImageList);
}
