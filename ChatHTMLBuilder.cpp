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
#include "ChatHTMLBuilder.h"

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
#define CHATMOD 			"Chat"
#define CHATFONTMOD 		"ChatFonts"
#define MUCCSET_OPTIONS     "ChatWindowOptions"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 17

static const char *classNames[] = {
	".timestamp", ".nameIn", ".nameOut", ".userJoined", ".userLeft", ".userDisconnected",
	".userKicked", ".nickChange", ".notice",
	".messageIn", ".messageOut", ".topicChange", ".information", ".statusEnable", ".statusDisable",
    ".action", ".highlight"
};

ChatHTMLBuilder::ChatHTMLBuilder() {
	setLastEventType(-1);
	setLastEventTime(time(NULL));
}

void ChatHTMLBuilder::loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour) {
    char str[32];
    int style;
    DBVARIANT dbv;
    if (colour) {
        wsprintfA(str, "Font%dCol", i);
        *colour = DBGetContactSettingDword(NULL, CHATFONTMOD, str, 0x000000);
    }
    if (lf) {
        wsprintfA(str, "Font%dSize", i);
        lf->lfHeight = (char) DBGetContactSettingByte(NULL, CHATFONTMOD, str, 10);
        lf->lfHeight = abs(lf->lfHeight);
        lf->lfWidth = 0;
        lf->lfEscapement = 0;
        lf->lfOrientation = 0;
        wsprintfA(str, "Font%dSty", i);
        style = DBGetContactSettingByte(NULL, CHATFONTMOD, str, 0);
        lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
        lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
        lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
        lf->lfStrikeOut = 0;
        wsprintfA(str, "Font%dSet", i);
        lf->lfCharSet = DBGetContactSettingByte(NULL, CHATFONTMOD, str, DEFAULT_CHARSET);
        lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf->lfQuality = DEFAULT_QUALITY;
        lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        wsprintfA(str, "Font%d", i);
        if (DBGetContactSetting(NULL, CHATFONTMOD, str, &dbv))
            lstrcpyA(lf->lfFaceName, "Verdana");
        else {
            lstrcpynA(lf->lfFaceName, dbv.pszVal, sizeof(lf->lfFaceName));
            DBFreeVariant(&dbv);
        }
    }
}

char *ChatHTMLBuilder::timestampToString(DWORD dwFlags, time_t check)
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

void ChatHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event) {
	LOGFONTA lf;
	COLORREF color;
	char *output = NULL;
	int outputSize;
	ProtocolSettings *protoSettings = getProtocolSettings(event->pszProto);
	if (protoSettings == NULL) {
		return;
	}
 	if (protoSettings->getChatMode() == Options::MODE_TEMPLATE) {
//		buildHeadTemplate(view, event);
		return;
	}
 	if (protoSettings->getChatMode() == Options::MODE_CSS) {
	 	const char *externalCSS = (event->dwFlags & IEEF_RTL) ? protoSettings->getChatCssFilenameRtl() : protoSettings->getChatCssFilename();
        Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n", externalCSS);
	} else {
		HDC hdc = GetDC(NULL);
	    int logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
		Utils::appendText(&output, &outputSize, "<html><head>");
		Utils::appendText(&output, &outputSize, "<style type=\"text/css\">\n");
		COLORREF bkgColor = DBGetContactSettingDword(NULL, CHATMOD, "BackgroundLog", 0xFFFFFF);
		COLORREF inColor, outColor;
	    bkgColor= (((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16));
		inColor = outColor = bkgColor;
		if (protoSettings->getChatFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".body {padding: 2px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); overflow: auto;}\n",
			protoSettings->getChatFlags() & Options::LOG_IMAGE_SCROLL ? "scroll" : "fixed", (int) bkgColor, protoSettings->getChatBackgroundFilename());
		} else {
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-color: #%06X; overflow: auto;}\n",
				 	     (int) bkgColor);
		}
		Utils::appendText(&output, &outputSize, ".link {color: #0000FF; text-decoration: underline;}\n");
		Utils::appendText(&output, &outputSize, ".img {vertical-align: middle;}\n");
		if (protoSettings->getChatFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		} else {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) outColor);
		}
	 	for(int i = 0; i < FONT_NUM; i++) {
			loadMsgDlgFont(i, &lf, &color);
			Utils::appendText(&output, &outputSize, "%s {font-family: %s; font-size: %dpt; font-weight: %s; color: #%06X; %s }\n",
			classNames[i],
			lf.lfFaceName,
			abs((signed char)lf.lfHeight) *  74 /logPixelSY ,
			lf.lfWeight >= FW_BOLD ? "bold" : "normal",
			(int)(((color & 0xFF) << 16) | (color & 0xFF00) | ((color & 0xFF0000) >> 16)),
			lf.lfItalic ? "font-style: italic;" : "");
		}
		Utils::appendText(&output, &outputSize, "</style></head><body class=\"body\">\n");
	}
	if (output != NULL) {
        view->write(output);
		free(output);
	}
	setLastEventType(-1);
}

void ChatHTMLBuilder::appendEventNonTemplate(IEView *view, IEVIEWEVENT *event) {

	int cp = CP_ACP;
	if (event->cbSize == sizeof(IEVIEWEVENT)) {
		cp = event->codepage;
	}
    IEVIEWEVENTDATA* eventData = (IEVIEWEVENTDATA *) event->hDbEventFirst;
	for (int eventIdx = 0; eventData!=NULL && (eventIdx < event->count || event->count==-1); eventData = eventData->next, eventIdx++) {
		DWORD dwFlags = eventData->dwData;
		char *style = NULL;
		int styleSize;
		int isSent = eventData->bIsMe;
		int outputSize;
		char *output = NULL;
		char *szName = NULL, *szText = NULL;
		if (eventData->iType == IEED_GC_EVENT_MESSAGE) {
			if (eventData->dwFlags & IEEDF_UNICODE) {
				szText = encodeUTF8((wchar_t *)eventData->pszText, event->pszProto, ENF_ALL);
			} else {
				szText = encodeUTF8((char *)eventData->pszText, event->pszProto, ENF_ALL);
			}
			szName = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NAMESMILEYS);
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
                Utils::appendText(&style, &styleSize, "font-weight: %s;", eventData->fontStyle & IE_FONT_BOLD ? "bold" : "normal");
                Utils::appendText(&style, &styleSize, "font-style: %s;", eventData->fontStyle & IE_FONT_ITALIC ? "italic" : "normal");
                Utils::appendText(&style, &styleSize, "text-decoration: %s;", eventData->fontStyle & IE_FONT_UNDERLINE ? "underline" : "none");
			}
			Utils::appendText(&output, &outputSize, "<span class=\"%s\"><span style=\"%s\">%s</span></span>", className, style!=NULL ? style : "", szText);
            Utils::appendText(&output, &outputSize, "</div>\n");
			if (style!=NULL) free(style);
		} else if (eventData->iType == IEED_GC_EVENT_JOIN || eventData->iType == IEED_GC_EVENT_PART || eventData->iType == IEED_GC_EVENT_TOPIC) {
			Utils::appendText(&output, &outputSize, "<div class=\"%s\">", "divIn");
			if (dwFlags & FLAG_SHOW_TIMESTAMP || dwFlags & FLAG_SHOW_DATE) {
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s </span>",
							isSent ? "timestamp" : "timestamp", timestampToString(dwFlags, eventData->time));
			}
			const char *className;
			const char *eventText;
			if (eventData->iType == IEED_GC_EVENT_JOIN) {
                className = "userJoined";
				eventText = "%s has joined";
				szText = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NONE);
			} else if (eventData->iType == IEED_GC_EVENT_PART) {
                className = "userLeft";
				eventText = "%s has left";
				szText = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NONE);
			} else if (eventData->iType == IEED_GC_EVENT_QUIT) {
                className = "userLeft";
				eventText = "%s has disconnected";
				szText = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NONE);
			} else if (eventData->iType == IEED_GC_EVENT_NICK) {
                className = "userLeft";
				eventText = "%s is now known as %s";
				szText = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NONE);
			} else if (eventData->iType == IEED_GC_EVENT_ACTION) {
                className = "userLeft";
				eventText = "%s has left.";
				szText = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NONE);
			} else if (eventData->iType == IEED_GC_EVENT_KICK) {
                className = "userLeft";
				eventText = "%s kicked %s";
				szText = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NONE);
			} else if (eventData->iType == IEED_GC_EVENT_NOTICE) {
                className = "userLeft";
				eventText = "Notice from %s: %s";
				szText = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NONE);
			} else if (eventData->iType == IEED_GC_EVENT_INFORMATION) {
                className = "userLeft";
				eventText = "";
				szText = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NONE);
			} else if (eventData->iType == IEED_GC_EVENT_ADDSTATUS) {
                className = "userLeft";
				eventText = "%s has left.";
				szText = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NONE);
			} else if (eventData->iType == IEED_GC_EVENT_REMOVESTATUS) {
                className = "userLeft";
				eventText = "%s has left.";
				szText = encodeUTF8(eventData->pszNick, event->pszProto, ENF_NONE);
			} else {
                className = "topicChange";
				eventText = "The topic is \'%s%s\'";
				szText = encodeUTF8(eventData->pszText, event->pszProto, ENF_NONE);
			}
			Utils::appendText(&output, &outputSize, "<span class=\"%s\">", className);
			Utils::appendText(&output, &outputSize, Translate(eventText), szText);
			Utils::appendText(&output, &outputSize, "</span>");
            Utils::appendText(&output, &outputSize, "</div>\n");
		} else if (eventData->iType == IEED_GC_EVENT_NOTICE) {
            const char *className = "error";
			szText = encodeUTF8(eventData->pszText, event->pszProto, ENF_NONE);
			Utils::appendText(&output, &outputSize, "<div class=\"%s\">", "divIn");
			Utils::appendText(&output, &outputSize, "<span class=\"%s\"> %s: %s</span>", className, Translate("Error"), szText);
            Utils::appendText(&output, &outputSize, "</div>\n");
		}
		if (szName!=NULL) delete szName;
		if (szText!=NULL) delete szText;
		if (output != NULL) {
            view->write(output);
			free(output);
		}
    }
}

void ChatHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event) {
	ProtocolSettings *protoSettings = getProtocolSettings(event->pszProto);
	if (protoSettings == NULL) {
		return;
	}
// 	if (protoSettings->getSRMMMode() == Options::MODE_TEMPLATE) {
	//	appendEventTemplate(view, event);
//	} else {
		appendEventNonTemplate(view, event);
//	}
}

bool ChatHTMLBuilder::isDbEventShown(DBEVENTINFO * dbei) {
	return true;
}
