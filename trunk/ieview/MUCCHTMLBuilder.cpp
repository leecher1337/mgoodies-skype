/*

IEView Plugin for Miranda IM
Copyright (C) 2005  Piotr Piastucki

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
#include "MUCCHTMLBuilder.h"

#include "Options.h"
#include "Utils.h"

// srmm stuff
#define FLAG_SHOW_NICKNAMES	 0x00000001
#define FLAG_MSGONNEWLINE	 0x00000002
#define FLAG_OPT_SENDONENTER 0x00000004

#define FLAG_SHOW_DATE		 0x00000010
#define FLAG_SHOW_TIMESTAMP	 0x00000020
#define FLAG_SHOW_SECONDS	 0x00000040
#define FLAG_LONG_DATE		 0x00000080


#define SMF_LOG_SHOWNICK 1
#define SMF_LOG_SHOWTIME 2
#define SMF_LOG_SHOWDATE 4
#define SMF_LOG_SHOWICONS 8
#define SMF_LOG_SHOWSTATUSCHANGES 16
#define SMF_LOG_SHOWSECONDS 32
#define SMF_LOG_USERELATIVEDATE 64
#define SMF_LOG_USELONGDATE 128
#define SMF_LOG_GROUPMESSAGES	256
#define SMF_LOG_MARKFOLLOWUPS	512
#define SMF_LOG_MSGONNEWLINE 	1024

#define EVENTTYPE_STATUSCHANGE 25368
#define SRMMMOD "MUCC"
#define MUCCSET_OPTIONS      "ChatWindowOptions"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 9

static const char *classNames[] = {
	".timestamp", ".nameIn", ".nameOut", ".messageIn", ".messageOut", ".userJoined", ".userLeft", ".topicChange",
	".error"
};

MUCCHTMLBuilder::MUCCHTMLBuilder() {
	iLastEventType = -1;
	lastEventTime = time(NULL);
}

bool MUCCHTMLBuilder::isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei)
{
    switch (dbei->eventType) {
        case EVENTTYPE_MESSAGE:
            return 1;
        case EVENTTYPE_STATUSCHANGE:
            if (dbei->flags & DBEF_READ) return 0;
            return 1;
    }
    return 0;
}

void MUCCHTMLBuilder::loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour) {
    char str[32];
    int style;
    DBVARIANT dbv;
    if (colour) {
        wsprintfA(str, "Font%dCol", i);
        *colour = DBGetContactSettingDword(NULL, SRMMMOD, str, 0x000000);
    }
    if (lf) {
        wsprintfA(str, "Font%dSize", i);
        lf->lfHeight = (char) DBGetContactSettingByte(NULL, SRMMMOD, str, 10);
        lf->lfHeight = abs(lf->lfHeight);
        lf->lfWidth = 0;
        lf->lfEscapement = 0;
        lf->lfOrientation = 0;
        wsprintfA(str, "Font%dStyle", i);
        style = DBGetContactSettingByte(NULL, SRMMMOD, str, 0);
        lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
        lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
        lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
        lf->lfStrikeOut = 0;
        wsprintfA(str, "Font%dSet", i);
        lf->lfCharSet = DBGetContactSettingByte(NULL, SRMMMOD, str, DEFAULT_CHARSET);
        lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf->lfQuality = DEFAULT_QUALITY;
        lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        wsprintfA(str, "Font%dFace", i);
        if (DBGetContactSetting(NULL, SRMMMOD, str, &dbv))
            lstrcpyA(lf->lfFaceName, "Verdana");
        else {
            lstrcpynA(lf->lfFaceName, dbv.pszVal, sizeof(lf->lfFaceName));
            DBFreeVariant(&dbv);
        }
    }
}

char *MUCCHTMLBuilder::timestampToString(DWORD dwFlags, time_t check)
{
    static char szResult[512];
    char str[80];

    DBTIMETOSTRING dbtts;

    dbtts.cbDest = 70;;
    dbtts.szDest = str;

	szResult[0] = '\0';
	struct tm tm_now, tm_today;
	time_t now = time(NULL);
	time_t today;
    tm_now = *localtime(&now);
    tm_today = tm_now;
    tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
    today = mktime(&tm_today);
	if (dwFlags&FLAG_SHOW_DATE && dwFlags&FLAG_SHOW_TIMESTAMP) {
		if (dwFlags&FLAG_LONG_DATE) {
			dbtts.szFormat = dwFlags&FLAG_SHOW_SECONDS ? (char *)"D s" : (char *)"D t";
		} else {
			dbtts.szFormat = dwFlags&FLAG_SHOW_SECONDS ? (char *)"d s" : (char *)"d t";
		}
	} else if (dwFlags&FLAG_SHOW_DATE) {
		dbtts.szFormat = dwFlags&FLAG_LONG_DATE ? (char *)"D" : (char *)"d";
	} else if (dwFlags&FLAG_SHOW_TIMESTAMP) {
		dbtts.szFormat = dwFlags&FLAG_SHOW_SECONDS ? (char *)"s" : (char *)"t";
	} else {
		dbtts.szFormat = (char *)"";
	}
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
    strncat(szResult, str, 500);
	Utils::UTF8Encode(szResult, szResult, 500);
    return szResult;
}

void MUCCHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event) {
	LOGFONTA lf;
	COLORREF color;
	char *output = NULL;
	int outputSize;
/* 	if (Options::getExternalCSSFlags() & Options::EXTERNALCSS_ENABLED) {
	 	const char *externalCSS = (event->dwFlags & IEEF_RTL) ? Options::getExternalCSSFileRTL() : Options::getExternalCSSFile();
        Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n", externalCSS);
	} else */{
		HDC hdc = GetDC(NULL);
	    int logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
		Utils::appendText(&output, &outputSize, "<html><head>");
		Utils::appendText(&output, &outputSize, "<style type=\"text/css\">\n");
		COLORREF bkgColor = DBGetContactSettingDword(NULL, SRMMMOD, "BkgColour", 0xFFFFFF);
		COLORREF inColor, outColor;
	    bkgColor= (((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16));
		inColor = outColor = bkgColor;
		if (Options::getBkgImageFlags() & Options::BKGIMAGE_ENABLED) {
			const char *bkgImageFilename = Options::getBkgImageFile();
			Utils::appendText(&output, &outputSize, ".body {padding: 2px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); }\n",
			Options::getBkgImageFlags() & Options::BKGIMAGE_SCROLL ? "scroll" : "fixed", (int) bkgColor, bkgImageFilename);
		} else {
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-color: #%06X; }\n",
				 	     (int) bkgColor);
		}
		Utils::appendText(&output, &outputSize, ".link {color: #0000FF; text-decoration: underline;}\n");
		Utils::appendText(&output, &outputSize, ".img {vertical-align: middle;}\n");
		if (Options::getBkgImageFlags() & Options::BKGIMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		} else {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) outColor);
		}
	 	for(int i = 0; i < FONT_NUM; i++) {
			loadMsgDlgFont(i, &lf, &color);
			Utils::appendText(&output, &outputSize, "%s {font-family: %s; font-size: %dpt; font-weight: %d; color: #%06X; %s }\n",
			classNames[i],
			lf.lfFaceName,
			abs((signed char)lf.lfHeight) *  74 /logPixelSY ,
			lf.lfWeight >= FW_BOLD ? FW_BOLD : FW_NORMAL,
			(int)(((color & 0xFF) << 16) | (color & 0xFF00) | ((color & 0xFF0000) >> 16)),
			lf.lfItalic ? "font-style: italic;" : "");
		}
		Utils::appendText(&output, &outputSize, "</style></head><body class=\"body\">\n");
	}
	if (output != NULL) {
        view->write(output);
		free(output);
	}
	iLastEventType = -1;
}

void MUCCHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event) {

	DWORD dwFlags = DBGetContactSettingDword(NULL, SRMMMOD, MUCCSET_OPTIONS, 0);
	int cp = CP_ACP;
	if (event->cbSize == sizeof(IEVIEWEVENT)) {
		cp = event->codepage;
	}
    IEVIEWEVENTDATA* eventData = (IEVIEWEVENTDATA *) event->hDbEventFirst;
	for (int eventIdx = 0; eventData!=NULL && (eventIdx < event->count || event->count==-1); eventData = eventData->next, eventIdx++) {
		char *style = NULL;
		int styleSize;
		int isSent = eventData->bIsMe;
		int outputSize;
		char *output = NULL;
		char *szName = NULL, *szText = NULL;
		if (eventData->iType == IEED_EVENT_MESSAGE) {
			if (eventData->dwFlags & IEEDF_UNICODE) {
				szText = encodeUTF8((wchar_t *)eventData->pszText, eventData->pszProto, true);
			} else {
				szText = encodeUTF8((char *)eventData->pszText, eventData->pszProto, true);
			}
			szName = encodeUTF8(eventData->pszNick, NULL, false);
			Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divOut" : "divIn");
			if (dwFlags & FLAG_SHOW_TIMESTAMP || dwFlags & FLAG_SHOW_DATE) {
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s </span>",
							isSent ? "timestamp" : "timestamp", timestampToString(dwFlags, eventData->time));
			}
			if (dwFlags & SMF_LOG_SHOWNICK) {
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: </span>",
							isSent ? "nameOut" : "nameIn", szName);
			}
			if (dwFlags & FLAG_MSGONNEWLINE) {
				Utils::appendText(&output, &outputSize, "<br>");
			}
			const char *className = isSent ? "messageOut" : "messageIn";
			if (eventData->dwFlags & IEEDF_FORMAT_SIZE && eventData->fontSize > 0) {
                Utils::appendText(&style, &styleSize, "font-size:%dpt;", eventData->fontSize);
			}
			if (eventData->dwFlags & IEEDF_FORMAT_COLOR && eventData->color!=0xFFFFFFFF) {
                Utils::appendText(&style, &styleSize, "color:#%06X;", ((eventData->color & 0xFF) << 16) | (eventData->color & 0xFF00) | ((eventData->color & 0xFF0000) >> 16));
			}
			if (eventData->dwFlags & IEEDF_FORMAT_FONT) {
                Utils::appendText(&style, &styleSize, "font-family:%s;", eventData->fontName);
			}
			if (eventData->dwFlags & IEEDF_FORMAT_STYLE) {
                Utils::appendText(&style, &styleSize, "font-weight: %d;", eventData->fontStyle & IE_FONT_BOLD ? FW_BOLD : FW_NORMAL);
                Utils::appendText(&style, &styleSize, "font-style: %s;", eventData->fontStyle & IE_FONT_ITALIC ? "italic" : "normal");
                Utils::appendText(&style, &styleSize, "text-decoration: %s;", eventData->fontStyle & IE_FONT_UNDERLINE ? "underline" : "none");
			}
			Utils::appendText(&output, &outputSize, "<span class=\"%s\"><span style=\"%s\">%s</span></span>", className, style!=NULL ? style : "", szText);
            Utils::appendText(&output, &outputSize, "</div>\n");
			if (style!=NULL) free(style);
		} else if (eventData->iType == IEED_EVENT_JOIN || eventData->iType == IEED_EVENT_LEAVE || eventData->iType == IEED_EVENT_TOPIC) {
			Utils::appendText(&output, &outputSize, "<div class=\"%s\">", "divIn");
			if (dwFlags & FLAG_SHOW_TIMESTAMP || dwFlags & FLAG_SHOW_DATE) {
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s </span>",
							isSent ? "timestamp" : "timestamp", timestampToString(dwFlags, eventData->time));
			}
			const char *className;
			if (eventData->iType == IEED_EVENT_JOIN) {
                className = "userJoined";
				szText = encodeUTF8(eventData->pszNick, NULL, false);
			} else if (eventData->iType == IEED_EVENT_LEAVE) {
                className = "userLeft";
				szText = encodeUTF8(eventData->pszNick, NULL, false);
			} else {
                className = "topicChange";
				szText = encodeUTF8(eventData->pszText, NULL, false);
			}
			Utils::appendText(&output, &outputSize, "<span class=\"%s\">", className);
			Utils::appendText(&output, &outputSize, Translate("%s has joined."), szText);
			Utils::appendText(&output, &outputSize, "</span>");
            Utils::appendText(&output, &outputSize, "</div>\n");
		} else if (eventData->iType == IEED_EVENT_ERROR) {

		}
		if (szName!=NULL) delete szName;
		if (szText!=NULL) delete szText;
		if (output != NULL) {
            view->write(output);
			free(output);
		}
    }
	view->scrollToBottom();
}

int MUCCHTMLBuilder::getLastEventType() {
	return iLastEventType;
}

void MUCCHTMLBuilder::setLastEventType(int t) {
	iLastEventType = t;
}

time_t MUCCHTMLBuilder::getLastEventTime() {
	return lastEventTime;
}

void MUCCHTMLBuilder::setLastEventTime(time_t t) {
	lastEventTime = t;
}
