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
#include "ScriverHTMLBuilder.h"

#include "Options.h"
#include "Utils.h"

// srmm stuff
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
#define SMF_LOG_DRAWLINES	   2048

#define SRMMMOD "SRMM"

#define SRMSGSET_SHOWLOGICONS      "ShowLogIcon"
#define SRMSGSET_HIDENAMES         "HideNames"
#define SRMSGSET_SHOWTIME          "ShowTime"
#define SRMSGSET_SHOWDATE          "ShowDate"
#define SRMSGSET_SHOWSTATUSCHANGES "ShowStatusChanges"
#define SRMSGSET_SHOWSECONDS       "ShowSeconds"
#define SRMSGSET_USERELATIVEDATE   "UseRelativeDate"
#define SRMSGSET_USELONGDATE  	   "UseLongDate"
#define SRMSGSET_GROUPMESSAGES     "GroupMessages"
#define SRMSGSET_MARKFOLLOWUPS	   "MarkFollowUps"
#define SRMSGSET_MESSAGEONNEWLINE  "MessageOnNewLine"
#define SRMSGSET_DRAWLINES		   "DrawLines"
#define SRMSGSET_USERTL		   	   "UseRTL"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 10

static const char *classNames[] = {
	".messageOut", ".messageIn", ".nameOut", ".timeOut", ".colonOut", ".nameIn", ".timeIn", ".colonIn",
	".inputArea", ".notices"
};

ScriverHTMLBuilder::ScriverHTMLBuilder() {
	setLastEventType(-1);
	setLastEventTime(time(NULL));
	startedTime = time(NULL);
}

bool ScriverHTMLBuilder::isDbEventShown(DBEVENTINFO * dbei)
{
    switch (dbei->eventType) {
        case EVENTTYPE_MESSAGE:
            return 1;
        case EVENTTYPE_STATUSCHANGE:
          //  if (dbei->flags & DBEF_READ) return 0;
            return 1;
        case EVENTTYPE_URL:
            return 1;
        case EVENTTYPE_FILE:
			return 1;
    }
    return 0;
}

void ScriverHTMLBuilder::loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour) {
    char str[32];
    int style;
    DBVARIANT dbv;
    if (colour) {
        wsprintfA(str, "SRMFont%dCol", i);
        *colour = DBGetContactSettingDword(NULL, SRMMMOD, str, 0x000000);
    }
    if (lf) {
        wsprintfA(str, "SRMFont%dSize", i);
        lf->lfHeight = (char) DBGetContactSettingByte(NULL, SRMMMOD, str, 10);
        lf->lfHeight = abs(lf->lfHeight);
        lf->lfWidth = 0;
        lf->lfEscapement = 0;
        lf->lfOrientation = 0;
        wsprintfA(str, "SRMFont%dSty", i);
        style = DBGetContactSettingByte(NULL, SRMMMOD, str, 0);
        lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
        lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
        lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
        lf->lfStrikeOut = 0;
        wsprintfA(str, "SRMFont%dSet", i);
        lf->lfCharSet = DBGetContactSettingByte(NULL, SRMMMOD, str, DEFAULT_CHARSET);
        lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf->lfQuality = DEFAULT_QUALITY;
        lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        wsprintfA(str, "SRMFont%d", i);
        if (DBGetContactSetting(NULL, SRMMMOD, str, &dbv))
            lstrcpyA(lf->lfFaceName, "Verdana");
        else {
            lstrcpynA(lf->lfFaceName, dbv.pszVal, sizeof(lf->lfFaceName));
            DBFreeVariant(&dbv);
        }
    }
}

char *ScriverHTMLBuilder::timestampToString(DWORD dwFlags, time_t check, int groupStart)
{
    static char szResult[512];
    char str[80];

    DBTIMETOSTRING dbtts;

    dbtts.cbDest = 70;;
    dbtts.szDest = str;

    if(!groupStart || !(dwFlags & SMF_LOG_SHOWDATE)) {
        dbtts.szFormat = (dwFlags & SMF_LOG_SHOWSECONDS) ? (char *)"s" : (char *)"t";
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

        if(dwFlags & SMF_LOG_USERELATIVEDATE && check >= today) {
            dbtts.szFormat = (dwFlags & SMF_LOG_SHOWSECONDS) ? (char *)"s" : (char *)"t";
            strcpy(szResult, Translate("Today"));
	        strcat(szResult, ", ");
        }
        else if(dwFlags & SMF_LOG_USERELATIVEDATE && check > (today - 86400)) {
            dbtts.szFormat = (dwFlags & SMF_LOG_SHOWSECONDS) ? (char *)"s" : (char *)"t";
            strcpy(szResult, Translate("Yesterday"));
	        strcat(szResult, ", ");
        }
        else {
            if(dwFlags & SMF_LOG_USELONGDATE)
                dbtts.szFormat = (dwFlags & SMF_LOG_SHOWSECONDS) ? (char *)"D s" : (char *)"D t";
            else
                dbtts.szFormat = (dwFlags & SMF_LOG_SHOWSECONDS) ? (char *)"d s" : (char *)"d t";
            szResult[0] = '\0';
        }
    }
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
    strncat(szResult, str, 500);
	Utils::UTF8Encode(szResult, szResult, 500);
    return szResult;
}


void ScriverHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event) {
	LOGFONTA lf;
	COLORREF color;
	char *output = NULL;
	int outputSize;
	ProtocolSettings *protoSettings = getSRMMProtocolSettings(event->hContact);
	if (protoSettings == NULL) {
		return;
	}
 	if (protoSettings->getSRMMMode() == Options::MODE_TEMPLATE) {
		buildHeadTemplate(view, event, protoSettings);
		return;
	}
 	if (protoSettings->getSRMMMode() == Options::MODE_CSS) {
	 	const char *externalCSS = protoSettings->getSRMMCssFilename();
		if (strncmp(externalCSS, "http://", 7)) {
			Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"file://%s\"/></head><body class=\"body\">\n", externalCSS);
		} else {
			Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n", externalCSS);
		}
	} else {
		HDC hdc = GetDC(NULL);
	    int logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
		Utils::appendText(&output, &outputSize, "<html><head>");
		Utils::appendText(&output, &outputSize, "<style type=\"text/css\">\n");
		COLORREF bkgColor = DBGetContactSettingDword(NULL, SRMMMOD, "BkgColour", 0xFFFFFF);
		COLORREF inColor = DBGetContactSettingDword(NULL, SRMMMOD, "IncomingBkgColour", 0xFFFFFF);
		COLORREF outColor = DBGetContactSettingDword(NULL, SRMMMOD, "OutgoingBkgColour", 0xFFFFFF);
		COLORREF lineColor = DBGetContactSettingDword(NULL, SRMMMOD, "LineColour", 0xFFFFFF);
	    bkgColor= (((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16));
		inColor= (((inColor & 0xFF) << 16) | (inColor & 0xFF00) | ((inColor & 0xFF0000) >> 16));
		outColor= (((outColor & 0xFF) << 16) | (outColor & 0xFF00) | ((outColor & 0xFF0000) >> 16));
	    lineColor= (((lineColor & 0xFF) << 16) | (lineColor & 0xFF00) | ((lineColor & 0xFF0000) >> 16));
		if (protoSettings->getSRMMFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".body {padding: 2px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); overflow: auto;}\n",
			protoSettings->getSRMMFlags() & Options::LOG_IMAGE_SCROLL ? "scroll" : "fixed", (int) bkgColor, protoSettings->getSRMMBackgroundFilename());
		} else {
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-color: #%06X; overflow: auto;}\n",
				 	     (int) bkgColor);
		}
		Utils::appendText(&output, &outputSize, ".link {color: #0000FF; text-decoration: underline;}\n");
		Utils::appendText(&output, &outputSize, ".img {}\n");
		if (protoSettings->getSRMMFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divInGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
			Utils::appendText(&output, &outputSize, ".divOutGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
			Utils::appendText(&output, &outputSize, ".divInRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divOutRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divInGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
			Utils::appendText(&output, &outputSize, ".divOutGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
		} else {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) outColor);
			Utils::appendText(&output, &outputSize, ".divInGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
		        (int) lineColor, (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOutGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
		        (int) lineColor, (int) outColor);
			Utils::appendText(&output, &outputSize, ".divInRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOutRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) outColor);
			Utils::appendText(&output, &outputSize, ".divInGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
		        (int) lineColor, (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOutGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
		        (int) lineColor, (int) outColor);
		}
		Utils::appendText(&output, &outputSize, ".divNotice {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		Utils::appendText(&output, &outputSize, ".divNoticeGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
		Utils::appendText(&output, &outputSize, ".divNoticeRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		Utils::appendText(&output, &outputSize, ".divNoticeGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
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

void ScriverHTMLBuilder::appendEventNonTemplate(IEView *view, IEVIEWEVENT *event) {
	bool showColon;
	DWORD dwFlags = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTIME, 0) ? SMF_LOG_SHOWTIME : 0;
    dwFlags |= !DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_HIDENAMES, 0) ? SMF_LOG_SHOWNICK : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWDATE, 0) ? SMF_LOG_SHOWDATE : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, 0) ? SMF_LOG_SHOWICONS : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSTATUSCHANGES, 0) ? SMF_LOG_SHOWSTATUSCHANGES : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSECONDS, 0) ? SMF_LOG_SHOWSECONDS : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USERELATIVEDATE, 0) ? SMF_LOG_USERELATIVEDATE : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USELONGDATE, 0) ? SMF_LOG_USELONGDATE : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_GROUPMESSAGES, 0) ? SMF_LOG_GROUPMESSAGES : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_MARKFOLLOWUPS, 0) ? SMF_LOG_MARKFOLLOWUPS : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_MESSAGEONNEWLINE, 0) ? SMF_LOG_MSGONNEWLINE : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_DRAWLINES, 0) ? SMF_LOG_DRAWLINES : 0;

	char *szRealProto = getRealProto(event->hContact);
	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData!=NULL && (eventIdx < event->count || event->count==-1); eventData = eventData->next, eventIdx++) {
		const char *className = "";
		int outputSize;
		char *output;
		output = NULL;
		int isSent = eventData->dwFlags & IEEDF_SENT;
		int isRTL = eventData->dwFlags & IEEDF_RTL;
		showColon = false;
		if (eventData->iType == IEED_EVENT_MESSAGE || eventData->iType == IEED_EVENT_STATUSCHANGE
			|| eventData->iType == IEED_EVENT_URL || eventData->iType == IEED_EVENT_FILE) {
			int isGroupBreak = TRUE;
 		  	if ((dwFlags & SMF_LOG_GROUPMESSAGES) && eventData->dwFlags == LOWORD(getLastEventType())
			  && eventData->iType == IEED_EVENT_MESSAGE && HIWORD(getLastEventType()) == IEED_EVENT_MESSAGE
			  && (isSameDate(eventData->time, getLastEventTime()))
			  && (((eventData->time < startedTime) == (getLastEventTime() < startedTime)) || !(eventData->dwFlags & IEEDF_READ))) {
		        isGroupBreak = FALSE;
		    }
			char *szName = NULL;
			char *szText = NULL;
			if (eventData->dwFlags & IEEDF_UNICODE_NICK) {
				szName = encodeUTF8(event->hContact, szRealProto, eventData->pszNickW, ENF_NAMESMILEYS);
   			} else {
                szName = encodeUTF8(event->hContact, szRealProto, eventData->pszNick, ENF_NAMESMILEYS);
			}
			if (eventData->dwFlags & IEEDF_UNICODE_TEXT) {
				szText = encodeUTF8(event->hContact, szRealProto, eventData->pszTextW, eventData->iType == IEED_EVENT_MESSAGE ? ENF_ALL : 0);
   			} else {
                szText = encodeUTF8(event->hContact, szRealProto, eventData->pszText, event->codepage, eventData->iType == IEED_EVENT_MESSAGE ? ENF_ALL : 0);
			}
			/* Scriver-specific formatting */
			if ((dwFlags & SMF_LOG_DRAWLINES) && isGroupBreak && getLastEventType()!=-1) {
				if (eventData->iType == IEED_EVENT_MESSAGE) {
					className = isRTL ? isSent ? "divOutGridRTL" : "divInGridRTL" : isSent ? "divOutGrid" : "divInGrid";
				} else {
					className = isRTL ? isSent ? "divNoticeGridRTL" : "divNoticeGridRTL" : isSent ? "divNoticeGrid" : "divNoticeGrid";
				}
			} else {
				if (eventData->iType == IEED_EVENT_MESSAGE) {
					className = isRTL ? isSent ? "divOutRTL" : "divInRTL" : isSent ? "divOut" : "divIn";
				} else {
					className = isRTL ? isSent ? "divNoticeRTL" : "divNoticeRTL" : isSent ? "divNotice" : "divNotice";
				}
			}
			Utils::appendText(&output, &outputSize, "<div class=\"%s\">", className);
			if ((dwFlags & SMF_LOG_SHOWICONS) && isGroupBreak) {
				const char *iconFile = "";
				if (eventData->iType == IEED_EVENT_MESSAGE) {
					iconFile = isSent ? "message_out.gif" : "message_in.gif";
				} else if (eventData->iType == IEED_EVENT_FILE) {
					iconFile = "file.gif";
				} else if (eventData->iType == IEED_EVENT_URL) {
					iconFile = "url.gif";
				} else if (eventData->iType == IEED_EVENT_STATUSCHANGE) {
					iconFile = "status.gif";
				}
				Utils::appendText(&output, &outputSize, "<img class=\"img\" src=\"file://%s/plugins/ieview/%s\"/> ",
								workingDir, iconFile);
			}
			if (dwFlags & SMF_LOG_SHOWTIME &&
				(eventData->iType != IEED_EVENT_MESSAGE ||
			    !(dwFlags & SMF_LOG_GROUPMESSAGES) ||
				(isGroupBreak && !(dwFlags & SMF_LOG_MARKFOLLOWUPS)) || (!isGroupBreak && (dwFlags & SMF_LOG_MARKFOLLOWUPS))))
				{
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>",
							isSent ? "timeOut" : "timeIn",
							timestampToString(dwFlags, eventData->time, isGroupBreak));
				if (eventData->iType != IEED_EVENT_MESSAGE) {
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">: </span>",
							isSent ? "colonOut" : "colonIn");
				}
				showColon = true;
			}
   			if ((dwFlags & SMF_LOG_SHOWNICK && eventData->iType == IEED_EVENT_MESSAGE && isGroupBreak) || eventData->iType == IEED_EVENT_STATUSCHANGE ) {
	            if (eventData->iType == IEED_EVENT_MESSAGE) {
					if (showColon) {
						Utils::appendText(&output, &outputSize, "<span class=\"%s\"> %s</span>",
									isSent ? "nameOut" : "nameIn",
									szName);
					} else {
						Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>",
									isSent ? "nameOut" : "nameIn",
									szName);
					}
                    showColon = true;
					if (dwFlags & SMF_LOG_GROUPMESSAGES) {
						Utils::appendText(&output, &outputSize, "<br>");
						showColon = false;
					}
				} else {
					Utils::appendText(&output, &outputSize, "<span class=\"notices\">%s </span>", szName);
				}
			}
			if (dwFlags & SMF_LOG_SHOWTIME && dwFlags & SMF_LOG_GROUPMESSAGES && dwFlags & SMF_LOG_MARKFOLLOWUPS
				&& eventData->iType == IEED_EVENT_MESSAGE && isGroupBreak) {
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>",
							isSent ? "timeOut" : "timeIn",
							timestampToString(dwFlags, eventData->time, isGroupBreak));
				showColon = true;
			}
			if (showColon && eventData->iType == IEED_EVENT_MESSAGE) {
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">: </span>",
							isSent ? "colonOut" : "colonIn");
			}
			if (eventData->iType == IEED_EVENT_MESSAGE) {
				if (dwFlags & SMF_LOG_MSGONNEWLINE && showColon) {
					Utils::appendText(&output, &outputSize, "<br>");
				}
				className = isSent ? "messageOut" : "messageIn";
			} else {
                className = "notices";
			}
			if (eventData->iType == IEED_EVENT_FILE) {
				if (isSent) {
	            	Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: %s</span>", className, Translate("File sent"), szText);
				} else {
	            	Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: %s</span>", className, Translate("File received"), szText);
				}
			} else if (eventData->iType == IEED_EVENT_URL) {
				if (isSent) {
	            	Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: %s</span>", className, Translate("URL sent"), szText);
				} else {
	            	Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: %s</span>", className, Translate("URL received"), szText);
				}
			} else {
            	Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>", className, szText);
			}
            Utils::appendText(&output, &outputSize, "</div>\n");
			setLastEventType(MAKELONG(eventData->dwFlags, eventData->iType));
			setLastEventTime(eventData->time);
			if (szName!=NULL) delete szName;
			if (szText!=NULL) delete szText;
		}
		if (output != NULL) {
            view->write(output);
			free(output);
		}
    }
    if (szRealProto!=NULL) delete szRealProto;
    view->documentClose();
//	view->scrollToBottom();
}

void ScriverHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event) {
	ProtocolSettings *protoSettings = getSRMMProtocolSettings(event->hContact);
	if (protoSettings == NULL) {
		return;
	}
 	if (protoSettings->getSRMMMode() == Options::MODE_TEMPLATE) {
		appendEventTemplate(view, event, protoSettings);
	} else {
		appendEventNonTemplate(view, event);
	}
}
