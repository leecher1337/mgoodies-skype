#include "TabSRMMHTMLBuilder.h"
#include "Options.h"
#include "Utils.h"

// tabsrmm stuff

#define MWF_LOG_SHOWNICK 512
#define MWF_LOG_SHOWTIME 1024
#define MWF_LOG_SHOWSECONDS 2048
#define MWF_LOG_SHOWDATES 4096
#define MWF_LOG_NEWLINE 8192
#define MWF_LOG_INDENT 16384
#define MWF_LOG_RTL 32768
#define MWF_LOG_UNDERLINE 65536
#define MWF_LOG_SWAPNICK 131072
#define MWF_LOG_SHOWICONS 262144

#define MWF_LOG_INDENTWITHTABS 1048576
#define MWF_LOG_SYMBOLS 0x200000
#define MWF_LOG_TEXTFORMAT 0x2000000
#define MWF_LOG_GRID 0x4000000
#define MWF_LOG_INDIVIDUALBKG 0x8000000

#define MWF_DIVIDERWANTED 0x40000000
#define MWF_LOG_GROUPMODE 0x80000000
#define MWF_LOG_LONGDATES 64
#define MWF_LOG_USERELATIVEDATES 1

#define MWF_SHOW_URLEVENTS 1
#define MWF_SHOW_FILEEVENTS 2
#define MWF_SHOW_INOUTICONS 4
#define MWF_SHOW_EMPTYLINEFIX 8
#define MWF_SHOW_MICROLF 16
#define MWF_SHOW_MARKFOLLOWUPTS 32

#define SRMSGMOD "SRMsg"
#define SRMSGMOD_T "Tab_SRMsg"

#define EVENTTYPE_STATUSCHANGE 25368
#define EVENTTYPE_DIVIDER 25367
#define EVENTTYPE_ERRMSG 25366

#define SRMSGSET_SHOWURLS          "ShowURLs"
#define SRMSGSET_SHOWFILES         "ShowFiles"
#define SRMSGSET_SHOWSTATUSCHANGES "ShowFiles"

#define MWF_LOG_DEFAULT (MWF_LOG_SHOWTIME | MWF_LOG_SHOWNICK | MWF_LOG_SHOWDATES)

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 19

static const char *classNames[] = {
	".messageOut", ".miscOut", ".messageIn", ".miscIn", ".nameOut", ".timeOut", ".nameIn", ".timeIn",
	".hMessageOut", ".hMiscOut", ".hMessageIn", ".hMiscIn", ".hNameOut", ".hTimeOut", ".hNameIn", ".hTimeIn",
	".inputArea", ".statusChange", ".dividers"
};

TabSRMMHTMLBuilder::TabSRMMHTMLBuilder() {
	iLastEventType = -1;
	startedTime = time(NULL);
	lastEventTime = time(NULL);
}    

bool TabSRMMHTMLBuilder::isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei)
{
    switch (dbei->eventType) {
        case EVENTTYPE_MESSAGE:
            return 1;
            break;
        case EVENTTYPE_STATUSCHANGE:
            return 1;
            break;
        case EVENTTYPE_URL:
            if(dwFlags & MWF_SHOW_URLEVENTS) return 1;
            break;
        case EVENTTYPE_FILE:
            if(dwFlags & MWF_SHOW_FILEEVENTS) return 1;
            break;
    }
    return 0;
}

void TabSRMMHTMLBuilder::loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour) {
    char str[32];
    int style;
    DBVARIANT dbv;
    if (colour) {
        wsprintfA(str, "Font%dCol", i);
        *colour = DBGetContactSettingDword(NULL, SRMSGMOD_T, str, 0x000000);
    }
    if (lf) {
        HDC hdc = GetDC(NULL);
        wsprintfA(str, "Font%dSize", i);
//        if(i == H_MSGFONTID_DIVIDERS)
  //          lf->lfHeight = 5;
     //   else {
            lf->lfHeight = (char) DBGetContactSettingByte(NULL, SRMSGMOD_T, str, 10);
            lf->lfHeight= MulDiv(lf->lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 74);
       // }
        ReleaseDC(NULL,hdc);
        lf->lfWidth = 0;
        lf->lfEscapement = 0;
        lf->lfOrientation = 0;
        wsprintfA(str, "Font%dSty", i);
        style = DBGetContactSettingByte(NULL, SRMSGMOD_T, str, 0);
        lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
        lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
        lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
        lf->lfStrikeOut = 0;
        wsprintfA(str, "Font%dSet", i);
        lf->lfCharSet = DBGetContactSettingByte(NULL, SRMSGMOD_T, str, DEFAULT_CHARSET);
        lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf->lfQuality = DEFAULT_QUALITY;
        lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        wsprintfA(str, "Font%d", i);
        if (DBGetContactSetting(NULL, SRMSGMOD_T, str, &dbv))
            lstrcpyA(lf->lfFaceName, "Verdana");
        else {
            lstrcpynA(lf->lfFaceName, dbv.pszVal, sizeof(lf->lfFaceName));
            DBFreeVariant(&dbv);
        }
    }
}

char *TabSRMMHTMLBuilder::timestampToString(DWORD dwFlags, time_t check, int isGroupBreak)
{
    static char szResult[512];
    char str[80];

    DBTIMETOSTRING dbtts;

    struct tm tm_now, tm_today;
    time_t now = time(NULL);
    time_t today;

    dbtts.cbDest = 70;;
    dbtts.szDest = str;

    if(!isGroupBreak || !(dwFlags & MWF_LOG_SHOWDATES)) {
        dbtts.szFormat = (dwFlags & MWF_LOG_SHOWSECONDS) ? (char *)"s" : (char *)"t";
        szResult[0] = '\0';
    }
    else {
        tm_now = *localtime(&now);
        tm_today = tm_now;
        tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
        today = mktime(&tm_today);

        if(dwFlags & MWF_LOG_USERELATIVEDATES && check >= today) {
            dbtts.szFormat = (dwFlags & MWF_LOG_SHOWSECONDS) ? (char *)"s" : (char *)"t";
            strcpy(szResult, Translate("Today"));
	        strcat(szResult, ", ");
        }
        else if(dwFlags & MWF_LOG_USERELATIVEDATES && check > (today - 86400)) {
            dbtts.szFormat = (dwFlags & MWF_LOG_SHOWSECONDS) ? (char *)"s" : (char *)"t";
            strcpy(szResult, Translate("Yesterday"));
	        strcat(szResult, ", ");
        }
        else {
            if(dwFlags & MWF_LOG_LONGDATES)
                dbtts.szFormat = (dwFlags & MWF_LOG_SHOWSECONDS) ? (char *)"D s" : (char *)"D t";
            else
                dbtts.szFormat = (dwFlags & MWF_LOG_SHOWSECONDS) ? (char *)"d s" : (char *)"d t";
            szResult[0] = '\0';
        }
    }
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
    strncat(szResult, str, 500);
	Utils::UTF8Encode(szResult, szResult, 500);
    return szResult;
}



void TabSRMMHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event) {
	LOGFONTA lf;
	COLORREF color;
	char *output = NULL;
	int outputSize;
 	if (Options::getExternalCSSFlags() & Options::EXTERNALCSS_ENABLED) {
	 	const char *externalCSS = (event->dwFlags & IEEF_RTL) ? Options::getExternalCSSFileRTL() : Options::getExternalCSSFile();
        Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n",externalCSS);
	} else {
		HDC hdc = GetDC(NULL);
	    int logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
	 	DWORD dwFlags = DBGetContactSettingDword(NULL, SRMSGMOD_T, "mwflags", MWF_LOG_DEFAULT);
		Utils::appendText(&output, &outputSize, "<html><head><style type=\"text/css\">\n");
		COLORREF inColor, outColor;
		COLORREF bkgColor = DBGetContactSettingDword(NULL, SRMSGMOD, "BkgColour", 0xFFFFFF);
	    bkgColor= (((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16));
		COLORREF gridColor = DBGetContactSettingDword(NULL, SRMSGMOD_T, "hgrid", 0xFFFFFF);
	    gridColor= (((gridColor & 0xFF) << 16) | (gridColor & 0xFF00) | ((gridColor & 0xFF0000) >> 16));
	    if (dwFlags & MWF_LOG_INDIVIDUALBKG) {
			inColor = DBGetContactSettingDword(NULL, SRMSGMOD_T, "inbg", RGB(224,224,224));
		    outColor = DBGetContactSettingDword(NULL, SRMSGMOD_T, "outbg", RGB(224,224,224));
		    inColor= (((inColor & 0xFF) << 16) | (inColor & 0xFF00) | ((inColor & 0xFF0000) >> 16));
		    outColor= (((outColor & 0xFF) << 16) | (outColor & 0xFF00) | ((outColor & 0xFF0000) >> 16));
		} else {
			inColor = outColor = bkgColor;
		}
		if (Options::getBkgImageFlags() & Options::BKGIMAGE_ENABLED) {
			const char *bkgImageFilename = Options::getBkgImageFile();
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); }\n",
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
			Utils::appendText(&output, &outputSize, ".divInGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) gridColor);
			Utils::appendText(&output, &outputSize, ".divOutGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) gridColor);
		} else {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) outColor);
			Utils::appendText(&output, &outputSize, ".divInGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
		        (int) gridColor, (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOutGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
		        (int) gridColor, (int) outColor);
		}
	 	for(int i = 0; i < FONT_NUM; i++) {
			loadMsgDlgFont(i, &lf, &color);
			Utils::appendText(&output, &outputSize, "%s {font-family: %s; font-size: %dpt; font-weight: %d; color: #%06X; %s}\n",
			classNames[i],
			lf.lfFaceName,
			abs((signed char)lf.lfHeight) *  74 /logPixelSY ,
			lf.lfWeight >= FW_BOLD ? 900 : 300,
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

void TabSRMMHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event) {

//	int	  indentLeft = DBGetContactSettingDword(NULL, SRMSGMOD_T, "IndentAmount", 0);
//	int	  indentRight = DBGetContactSettingDword(NULL, SRMSGMOD_T, "RightIndent", 0);
 	DWORD dwFlags = DBGetContactSettingDword(NULL, SRMSGMOD_T, "mwflags", MWF_LOG_DEFAULT);
	DWORD dwFlags2 = DBGetContactSettingByte(NULL, SRMSGMOD, SRMSGSET_SHOWURLS, 0) ? MWF_SHOW_URLEVENTS : 0;
    dwFlags2 |= DBGetContactSettingByte(NULL, SRMSGMOD, SRMSGSET_SHOWFILES, 0) ? MWF_SHOW_FILEEVENTS : 0;
    dwFlags2 |= DBGetContactSettingByte(NULL, SRMSGMOD_T, "in_out_icons", 0) ? MWF_SHOW_INOUTICONS : 0;
    dwFlags2 |= DBGetContactSettingByte(NULL, SRMSGMOD_T, "emptylinefix", 1) ? MWF_SHOW_EMPTYLINEFIX : 0;
	dwFlags2 |= MWF_SHOW_MICROLF;
    dwFlags2 |= DBGetContactSettingByte(NULL, SRMSGMOD_T, "followupts", 1) ? MWF_SHOW_MARKFOLLOWUPTS : 0;

	char *szProto = _strdup((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) event->hContact, 0));
	HANDLE hDbEvent = event->hDbEventFirst;
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

		if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_URL || dbei.eventType == EVENTTYPE_STATUSCHANGE)) {
			CallService(MS_DB_EVENT_MARKREAD, (WPARAM) event->hContact, (LPARAM) hDbEvent);
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM) event->hContact, (LPARAM) hDbEvent);
		} 
        hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
		if (!isDbEventShown(dwFlags2, &dbei)) {
            free(dbei.pBlob);
	        continue;
    	}
		output = NULL;
		if (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_FILE || dbei.eventType == EVENTTYPE_URL || dbei.eventType == EVENTTYPE_STATUSCHANGE) {
			int isGroupBreak = TRUE;
			int isSent = (dbei.flags & DBEF_SENT);
			int isHistory = (dbei.timestamp < (DWORD)getStartedTime() && (dbei.flags & DBEF_READ || dbei.flags & DBEF_SENT));
		  	if (dwFlags & MWF_LOG_GROUPMODE && dbei.flags == LOWORD(getLastEventType())
			  && dbei.eventType == EVENTTYPE_MESSAGE && HIWORD(getLastEventType()) == EVENTTYPE_MESSAGE
			  && ((dbei.timestamp - getLastEventTime()) < 86400)) {
		        isGroupBreak = FALSE;
		    }
			char *szName = NULL;
			char *szText = NULL;
			if (isSent) {
                CONTACTINFO ci;
				ZeroMemory(&ci, sizeof(ci));
			    ci.cbSize = sizeof(ci);
			    ci.hContact = NULL;
			    ci.szProto = dbei.szModule;
			    ci.dwFlag = CNF_DISPLAY;
				if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
			        szName = encodeUTF8(ci.pszVal, NULL, false);
    			}
   			} else {
                szName = encodeUTF8((char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) event->hContact, 0), NULL, false);
			}
			if (dbei.eventType == EVENTTYPE_MESSAGE) {
				DWORD aLen = strlen((char *)dbei.pBlob)+1;
				if (dbei.cbBlob > aLen && !(event->dwFlags & IEEF_NO_UNICODE)) {
					DWORD wlen = Utils::safe_wcslen((wchar_t *)&dbei.pBlob[aLen], (dbei.cbBlob - aLen) / 2);
					if (wlen > 0 && wlen < aLen) {
                        szText = encodeUTF8((wchar_t *)&dbei.pBlob[aLen], szProto, true);
					} else {
                        szText = encodeUTF8((char *)dbei.pBlob, szProto, true);
					}
				} else {
                	szText = encodeUTF8((char *)dbei.pBlob, szProto, true);
				}
			} else if (dbei.eventType == EVENTTYPE_FILE) {
                szText = encodeUTF8((char *)dbei.pBlob + sizeof(DWORD), NULL, false);
			} else if (dbei.eventType == EVENTTYPE_URL) {
                szText = encodeUTF8((char *)dbei.pBlob, NULL, false);
			} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
                szText = encodeUTF8((char *)dbei.pBlob, NULL, false);
			}
			/* TabSRMM-specific formatting */
			if (dwFlags & MWF_LOG_GRID && isGroupBreak) {
				Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divOutGrid" : "divInGrid");
			} else {
				Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divOut" : "divIn");
			}
			if (dwFlags & MWF_LOG_SHOWICONS && isGroupBreak) {
				const char *iconFile = "";
				if (dbei.eventType == EVENTTYPE_MESSAGE) {
					if (dwFlags2 & MWF_SHOW_INOUTICONS) iconFile = isSent ? "message_out.gif" : "message_in.gif";
					else iconFile = "message.gif";
				} else if (dbei.eventType == EVENTTYPE_FILE) {
					iconFile = "file.gif";
				} else if (dbei.eventType == EVENTTYPE_URL) {
					iconFile = "url.gif";
				} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
					iconFile = "status.gif";
				}
				Utils::appendText(&output, &outputSize, "<img class=\"img\" src=\"%s/plugins/ieview/%s\"/>",
								workingDir, iconFile);
			}
			if ((dwFlags & MWF_LOG_SWAPNICK) && (dwFlags & MWF_LOG_SHOWNICK) && isGroupBreak && (dbei.eventType != EVENTTYPE_STATUSCHANGE)) {
				const char *className = "";
				if (!isHistory)	className = isSent ? "nameOut" : "nameIn";
				else className = isSent ? "hNameOut" : "hNameIn";
				if (dwFlags & MWF_LOG_UNDERLINE) {
					Utils::appendText(&output, &outputSize, "<span class=\"%s\"><u>%s%s</span>",
								className, szName, (dwFlags & MWF_LOG_SHOWTIME) ? " </u>" :"</u>: ");
				} else {
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s%s</span>",
								className, szName, (dwFlags & MWF_LOG_SHOWTIME) ? " " :": ");
				}
			}
			if (dwFlags & MWF_LOG_SHOWTIME && (isGroupBreak || dwFlags2 & MWF_SHOW_MARKFOLLOWUPTS)) {
				const char *className = "";
				if (!isHistory)	className = isSent ? "timeOut" : "timeIn";
				else className = isSent ? "hTimeOut" : "hTimeIn";
				if (dwFlags & MWF_LOG_UNDERLINE) {
					Utils::appendText(&output, &outputSize, "<span class=\"%s\"><u>%s%s</span>",
								className, timestampToString(dwFlags, dbei.timestamp, isGroupBreak),
								(!isGroupBreak || (dbei.eventType == EVENTTYPE_STATUSCHANGE) || (dwFlags & MWF_LOG_SWAPNICK) || !(dwFlags & MWF_LOG_SHOWNICK)) ? "</u>: " : " </u>");
				} else {
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s%s</span>",
								className, timestampToString(dwFlags, dbei.timestamp, isGroupBreak),
								(!isGroupBreak || (dbei.eventType == EVENTTYPE_STATUSCHANGE) || (dwFlags & MWF_LOG_SWAPNICK) || !(dwFlags & MWF_LOG_SHOWNICK)) ? ": " : " ");
				}
			}
			if ((dbei.eventType == EVENTTYPE_STATUSCHANGE) || ((dwFlags & MWF_LOG_SHOWNICK) && !(dwFlags & MWF_LOG_SWAPNICK) && isGroupBreak)) {
				if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
					Utils::appendText(&output, &outputSize, "<span class=\"statusChange\">%s </span>", szName);
				} else {
					const char *className = "";
					if (!isHistory) className = isSent ? "nameOut" : "nameIn";
					else className = isSent ? "hNameOut" : "hNameIn";
					if (dwFlags & MWF_LOG_UNDERLINE) {
						Utils::appendText(&output, &outputSize, "<span class=\"%s\"><u>%s</u>: </span>",
									className, szName);
					} else {
						Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: </span>",
									className, szName);
					}
				}
			}
			if (dwFlags & MWF_LOG_NEWLINE && dbei.eventType != EVENTTYPE_STATUSCHANGE && dbei.eventType != EVENTTYPE_ERRMSG && isGroupBreak) {
				Utils::appendText(&output, &outputSize, "<br>");
			}
    		const char *className = "";
			if (dbei.eventType == EVENTTYPE_MESSAGE) {
				if (!isHistory) className = isSent ? "messageOut" : "messageIn";
				else className = isSent ? "hMessageOut" : "hMessageIn";
			} else if (dbei.eventType == EVENTTYPE_FILE) {
				className = isHistory ? "hMiscIn" : "miscIn";
			} else if (dbei.eventType == EVENTTYPE_URL) {
				className = isHistory ? "hMiscIn" : "miscIn";
			} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
				className = "statusChange";
			}
            Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>", className, szText);
            Utils::appendText(&output, &outputSize, "</div>\n");
			setLastEventType(MAKELONG(dbei.flags, dbei.eventType));
			setLastEventTime(dbei.timestamp);
			free (szName);
			free (szText);
		}
		if (output != NULL) {
            view->write(output);
			free(output);
		}
        free(dbei.pBlob);
    }
    free (szProto);
	view->scrollToBottom();
}

time_t TabSRMMHTMLBuilder::getStartedTime() {
	return startedTime;
}

int TabSRMMHTMLBuilder::getLastEventType() {
	return iLastEventType;
}

void TabSRMMHTMLBuilder::setLastEventType(int t) {
	iLastEventType = t;
}

time_t TabSRMMHTMLBuilder::getLastEventTime() {
	return lastEventTime;
}

void TabSRMMHTMLBuilder::setLastEventTime(time_t t) {
	lastEventTime = t;
}


