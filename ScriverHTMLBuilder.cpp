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

#define EVENTTYPE_STATUSCHANGE 25368
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

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 10

static const char *classNames[] = {
	".messageOut", ".messageIn", ".nameOut", ".timeOut", ".colonOut", ".nameIn", ".timeIn", ".colonIn",
	".inputArea", ".notices"
};

ScriverHTMLBuilder::ScriverHTMLBuilder() {
	iLastEventType = -1;
	lastEventTime = time(NULL);
	startedTime = time(NULL);
}

bool ScriverHTMLBuilder::isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei)
{
    switch (dbei->eventType) {
        case EVENTTYPE_MESSAGE:
            return 1;
        case EVENTTYPE_STATUSCHANGE:
            if (dbei->flags & DBEF_READ) return 0;
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
 	if (Options::getSRMMFlags() & Options::CSS_ENABLED) {
	 	const char *externalCSS = (event->dwFlags & IEEF_RTL) ? Options::getExternalCSSFileRTL() : Options::getExternalCSSFile();
        Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n", externalCSS);
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
		if (Options::getSRMMFlags() & Options::IMAGE_ENABLED) {
			const char *bkgImageFilename = Options::getBkgImageFile();
			Utils::appendText(&output, &outputSize, ".body {padding: 2px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); overflow: auto;}\n",
			Options::getSRMMFlags() & Options::IMAGE_ENABLED ? "scroll" : "fixed", (int) bkgColor, bkgImageFilename);
		} else {
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-color: #%06X; overflow: auto;}\n",
				 	     (int) bkgColor);
		}
		Utils::appendText(&output, &outputSize, ".link {color: #0000FF; text-decoration: underline;}\n");
		Utils::appendText(&output, &outputSize, ".img {vertical-align: middle;}\n");
		if (Options::getSRMMFlags() & Options::IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divInGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
			Utils::appendText(&output, &outputSize, ".divOutGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
		} else {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) outColor);
			Utils::appendText(&output, &outputSize, ".divInGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
		        (int) lineColor, (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOutGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
		        (int) lineColor, (int) outColor);
		}
		Utils::appendText(&output, &outputSize, ".divNotice {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		Utils::appendText(&output, &outputSize, ".divNoticeGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
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
	iLastEventType = -1;
}

void ScriverHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event) {
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

	int cp = CP_ACP;
	if (event->cbSize == sizeof(IEVIEWEVENT)) {
		cp = event->codepage;
	}
	char *szProto = getProto(event->hContact);
	char *szRealProto = getRealProto(event->hContact);
	HANDLE hDbEvent = event->hDbEventFirst;
	event->hDbEventFirst = NULL;
	for (int eventIdx = 0; hDbEvent!=NULL && (eventIdx < event->count || event->count==-1); eventIdx++) {
		int outputSize;
		char *output;
		DBEVENTINFO dbei = { 0 };
        dbei.cbSize = sizeof(dbei);
        dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM) hDbEvent, 0);
        if (dbei.cbBlob == 0xFFFFFFFF) {
            return;
		}
        dbei.pBlob = (PBYTE) malloc(dbei.cbBlob);
        CallService(MS_DB_EVENT_GET, (WPARAM)  hDbEvent, (LPARAM) & dbei);

		if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_URL)) {
			CallService(MS_DB_EVENT_MARKREAD, (WPARAM) event->hContact, (LPARAM) hDbEvent);
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM) event->hContact, (LPARAM) hDbEvent);
		} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
			CallService(MS_DB_EVENT_MARKREAD, (WPARAM) event->hContact, (LPARAM) hDbEvent);
		}
		HANDLE hCurDbEvent = hDbEvent;
        hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
		if (!isDbEventShown(dwFlags, &dbei)) {
            free(dbei.pBlob);
	        continue;
    	}
		output = NULL;
		showColon = false;
		if (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_STATUSCHANGE
			|| dbei.eventType == EVENTTYPE_URL || dbei.eventType == EVENTTYPE_FILE) {
			int isSent = (dbei.flags & DBEF_SENT);
			int isGroupBreak = TRUE;
 		  	if ((dwFlags & SMF_LOG_GROUPMESSAGES) && dbei.flags == LOWORD(getLastEventType())
			  && dbei.eventType == EVENTTYPE_MESSAGE && HIWORD(getLastEventType()) == EVENTTYPE_MESSAGE
			  && (isSameDate(dbei.timestamp, getLastEventTime()))
			  && (((dbei.timestamp < (DWORD)startedTime) == (getLastEventTime() < (DWORD)startedTime)) || !(dbei.flags & DBEF_READ))) {
		        isGroupBreak = FALSE;
		    }
			char *szName = NULL;
			char *szText = NULL;
			if (isSent) {
				szName = getContactName(NULL, szProto, szRealProto);
   			} else {
                szName = getContactName(event->hContact, szProto, szRealProto);
			}
			if (dbei.eventType == EVENTTYPE_MESSAGE) {
				DWORD aLen = strlen((char *)dbei.pBlob)+1;
				if (dbei.cbBlob > aLen && !(event->dwFlags & IEEF_NO_UNICODE)) {
					DWORD wlen = Utils::safe_wcslen((wchar_t *)&dbei.pBlob[aLen], (dbei.cbBlob - aLen) / 2);
					if (wlen > 0 && wlen < aLen) {
                        szText = encodeUTF8((wchar_t *)&dbei.pBlob[aLen], szRealProto, ENF_ALL);
					} else {
                        szText = encodeUTF8((char *)dbei.pBlob, cp, szRealProto, ENF_ALL);
					}
				} else {
                	szText = encodeUTF8((char *)dbei.pBlob, cp, szRealProto, ENF_ALL);
				}
			} else if (dbei.eventType == EVENTTYPE_FILE) {
                szText = encodeUTF8(((char *)dbei.pBlob) + sizeof(DWORD), szRealProto, ENF_NONE);
			} else if (dbei.eventType == EVENTTYPE_URL) {
                szText = encodeUTF8((char *)dbei.pBlob, szRealProto, ENF_NONE);
			} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
                szText = encodeUTF8((char *)dbei.pBlob, szRealProto, ENF_NONE);
			}
			/* SRMM-specific formatting */
			if ((dwFlags & SMF_LOG_DRAWLINES) && isGroupBreak && getLastEventType()!=-1) {
				if (dbei.eventType == EVENTTYPE_MESSAGE) {
					Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divOutGrid" : "divInGrid");
				} else {
					Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divNoticeGrid" : "divNoticeGrid");
				}
			} else {
				if (dbei.eventType == EVENTTYPE_MESSAGE) {
					Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divOut" : "divIn");
				} else {
					Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divNotice" : "divNotice");
				}
			}
			if ((dwFlags & SMF_LOG_SHOWICONS) && isGroupBreak) {
				const char *iconFile = "";
				if (dbei.eventType == EVENTTYPE_MESSAGE) {
					iconFile = isSent ? "message_out.gif" : "message_in.gif";
				} else if (dbei.eventType == EVENTTYPE_FILE) {
					iconFile = "file.gif";
				} else if (dbei.eventType == EVENTTYPE_URL) {
					iconFile = "url.gif";
				} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
					iconFile = "status.gif";
				}
				Utils::appendText(&output, &outputSize, "<img class=\"img\" src=\"%s/plugins/ieview/%s\"/> ",
								workingDir, iconFile);
			}
			if (dwFlags & SMF_LOG_SHOWTIME &&
				(dbei.eventType != EVENTTYPE_MESSAGE ||
			    !(dwFlags & SMF_LOG_GROUPMESSAGES) || 
				(isGroupBreak && !(dwFlags & SMF_LOG_MARKFOLLOWUPS)) || (!isGroupBreak && (dwFlags & SMF_LOG_MARKFOLLOWUPS))))
				{
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>",
							isSent ? "timeOut" : "timeIn", timestampToString(dwFlags, dbei.timestamp, isGroupBreak));
				if (dbei.eventType != EVENTTYPE_MESSAGE) {
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">: </span>",
								isSent ? "colonOut" : "colonIn");
				}
				showColon = true;
			}
   			if ((dwFlags & SMF_LOG_SHOWNICK && dbei.eventType == EVENTTYPE_MESSAGE && isGroupBreak) || dbei.eventType == EVENTTYPE_STATUSCHANGE ) {
	            if (dbei.eventType == EVENTTYPE_MESSAGE) {
					if (showColon) {
						Utils::appendText(&output, &outputSize, "<span class=\"%s\"> %s</span>",
									isSent ? "nameOut" : "nameIn", szName);
					} else {
						Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>",
									isSent ? "nameOut" : "nameIn", szName);
					}
                    showColon = true;
					if (dwFlags & SMF_LOG_GROUPMESSAGES) {
						Utils::appendText(&output, &outputSize, "<br>");
						showColon = false;
					}
				} else {
					Utils::appendText(&output, &outputSize, "<span class=\"notices\">%s</span>", szName);
				}
			}
			if (dwFlags & SMF_LOG_SHOWTIME && dwFlags & SMF_LOG_GROUPMESSAGES && dwFlags & SMF_LOG_MARKFOLLOWUPS
				&& dbei.eventType == EVENTTYPE_MESSAGE && isGroupBreak) {
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>",
							isSent ? "timeOut" : "timeIn", timestampToString(dwFlags, dbei.timestamp, isGroupBreak));
				showColon = true;
			}
			if (showColon && dbei.eventType == EVENTTYPE_MESSAGE) {
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">: </span>",
							isSent ? "colonOut" : "colonIn");
			}
			const char *className = "";
			if (dbei.eventType == EVENTTYPE_MESSAGE) {
				if (dwFlags & SMF_LOG_MSGONNEWLINE && showColon) {
					Utils::appendText(&output, &outputSize, "<br>");
				}
				className = isSent ? "messageOut" : "messageIn";
			} else {
                className = "notices";
			}
			if (dbei.eventType == EVENTTYPE_FILE) {
				if (isSent) {
	            	Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: %s</span>", className, Translate("File sent"), szText);
				} else {
	            	Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: %s</span>", className, Translate("File received"), szText);
				}
			} else if (dbei.eventType == EVENTTYPE_URL) {
				if (isSent) {
	            	Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: %s</span>", className, Translate("URL sent"), szText);
				} else {
	            	Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: %s</span>", className, Translate("URL received"), szText);
				}
			} else {
            	Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>", className, szText);
			}
            Utils::appendText(&output, &outputSize, "</div>\n");
			event->hDbEventFirst = hCurDbEvent;
			setLastEventType(MAKELONG(dbei.flags, dbei.eventType));
			setLastEventTime(dbei.timestamp);
			if (szName!=NULL) delete szName;
			if (szText!=NULL) delete szText;
		}
		if (output != NULL) {
            view->write(output);
			free(output);
		}
        free(dbei.pBlob);
    }
    if (szProto!=NULL) delete szProto;
    if (szRealProto!=NULL) delete szRealProto;
//	view->scrollToBottom();
}

