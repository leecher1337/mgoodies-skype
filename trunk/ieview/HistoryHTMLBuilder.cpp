#include "HistoryHTMLBuilder.h"

#include "Options.h"
#include "Utils.h"

// srmm stuff
#define SMF_LOG_SHOWNICK 1
#define SMF_LOG_SHOWTIME 2
#define SMF_LOG_SHOWDATES 4
#define SMF_LOG_SHOWICONS 8
#define SMF_LOG_SHOWSTATUSCHANGES 16
#define SRMMMOD "HistoryPlusPlus"

#define SRMSGSET_SHOWLOGICONS      "ShowLogIcon"
#define SRMSGSET_HIDENAMES         "HideNames"
#define SRMSGSET_SHOWTIME          "ShowTime"
#define SRMSGSET_SHOWDATE          "ShowDate"
#define SRMSGSET_SHOWSTATUSCHANGES "ShowStatusChanges"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 11

static const char *classNames[] = {
	".messageOut", ".messageIn", 
	".fileOut", ".fileIn", 
	".urlOut", ".urlIn", 
	".system"
	".nameOut", ".nameIn",
	".timeOut", ".timeIn", 
};

static const char *dbSettingNames[] = {
	"Font.OutMes", "Font.IncMes", 
	"Font.OutFil", "Font.IncFil", 
	"Font.OutUrl", "Font.IncUrl", 
	".system"
	".nameOut", "Font.Contact",
	".timeOut", "Font.ContactDate", 
};

bool HistoryHTMLBuilder::isDbEventShown(DBEVENTINFO * dbei)
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

void HistoryHTMLBuilder::loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour) {
    char str[128];
    int style;
    DBVARIANT dbv;
    if (colour) {
        wsprintfA(str, "%s.Color", dbSettingNames[i]);
        *colour = DBGetContactSettingDword(NULL, SRMMMOD, str, 0x000000);
    }
    if (lf) {
//        HDC hdc = GetDC(NULL);
        wsprintfA(str, "%s.Size", dbSettingNames[i]);
//        if(i == H_MSGFONTID_DIVIDERS)
  //          lf->lfHeight = 5;
     //   else {
            lf->lfHeight = (char) DBGetContactSettingByte(NULL, SRMMMOD, str, 10);
//            lf->lfHeight=-MulDiv(lf->lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
            lf->lfHeight = abs(lf->lfHeight);
       // }
//        ReleaseDC(NULL,hdc);
        lf->lfWidth = 0;
        lf->lfEscapement = 0;
        lf->lfOrientation = 0;
        wsprintfA(str, "%s.Style.Bold", dbSettingNames[i]);
        style = DBGetContactSettingByte(NULL, SRMMMOD, str, 0);
        wsprintfA(str, "%s.Style.Italic", dbSettingNames[i]);
        style = DBGetContactSettingByte(NULL, SRMMMOD, str, 0) << 1;
        lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
        lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
        lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
        lf->lfStrikeOut = 0;
        wsprintfA(str, "%s.Charset", dbSettingNames[i]);
        lf->lfCharSet = DBGetContactSettingByte(NULL, SRMMMOD, str, DEFAULT_CHARSET);
        lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf->lfQuality = DEFAULT_QUALITY;
        lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        wsprintfA(str, "%s.Name", dbSettingNames[i]);
        if (DBGetContactSetting(NULL, SRMMMOD, str, &dbv))
            lstrcpyA(lf->lfFaceName, "Verdana");
        else {
            lstrcpynA(lf->lfFaceName, dbv.pszVal, sizeof(lf->lfFaceName));
            DBFreeVariant(&dbv);
        }
    }
}

char *HistoryHTMLBuilder::timestampToString(DWORD dwFlags, time_t check) {
    static char szResult[512];
    char str[80];
    DBTIMETOSTRING dbtts;
    dbtts.cbDest = 70;;
    dbtts.szDest = str;
    szResult[0] = '\0';
    dbtts.szFormat = (char *)"d t";
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
    strncat(szResult, str, 500);
	Utils::UTF8Encode(szResult, szResult, 500);
    return szResult;
}


void HistoryHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event) {
 	if (Options::getHistoryFlags() & Options::TEMPLATES_ENABLED) {
		buildHeadTemplate(view, event);
		return;
	}
	LOGFONTA lf;
	COLORREF color;
	char *output = NULL;
	int outputSize;
 	if (Options::getHistoryFlags() & Options::CSS_ENABLED) {
	 	const char *externalCSS = (event->dwFlags & IEEF_RTL) ? Options::getHistoryCSSFileRTL() : Options::getHistoryCSSFile();
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
		if (Options::getHistoryFlags() & Options::IMAGE_ENABLED) {
			const char *bkgImageFilename = Options::getBkgImageFile();
			Utils::appendText(&output, &outputSize, ".body {padding: 2px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); overflow: auto;}\n",
			Options::getSRMMFlags() & Options::IMAGE_ENABLED ? "scroll" : "fixed", (int) bkgColor, bkgImageFilename);
		} else {
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-color: #%06X; overflow: auto;}\n",
				 	     (int) bkgColor);
		}
		Utils::appendText(&output, &outputSize, ".link {color: #0000FF; text-decoration: underline;}\n");
		Utils::appendText(&output, &outputSize, ".img {vertical-align: middle;}\n");
		if (Options::getHistoryFlags() & Options::IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".divMessageIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
			Utils::appendText(&output, &outputSize, ".divMessageOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
			Utils::appendText(&output, &outputSize, ".divFileIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
			Utils::appendText(&output, &outputSize, ".divFileOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
			Utils::appendText(&output, &outputSize, ".divUrlIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
			Utils::appendText(&output, &outputSize, ".divUrlOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
			Utils::appendText(&output, &outputSize, ".divSystem {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int) lineColor);
		} else {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
		        (int) lineColor, (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
		        (int) lineColor, (int) outColor);
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
	iLastEventType = -1;
}

void HistoryHTMLBuilder::appendEventNonTemplate(IEView *view, IEVIEWEVENT *event) {

	DWORD dwFlags = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTIME, 0) ? SMF_LOG_SHOWTIME : 0;
    dwFlags |= !DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_HIDENAMES, 0) ? SMF_LOG_SHOWNICK : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWDATE, 0) ? SMF_LOG_SHOWDATES : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, 0) ? SMF_LOG_SHOWICONS : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSTATUSCHANGES, 0) ? SMF_LOG_SHOWSTATUSCHANGES : 0;
	int cp = CP_ACP;
	if (event->cbSize >= IEVIEWEVENT_SIZE_V2) {
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

		if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE )) {
			CallService(MS_DB_EVENT_MARKREAD, (WPARAM) event->hContact, (LPARAM) hDbEvent);
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM) event->hContact, (LPARAM) hDbEvent);
		} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
			CallService(MS_DB_EVENT_MARKREAD, (WPARAM) event->hContact, (LPARAM) hDbEvent);
		}
		HANDLE hCurDbEvent = hDbEvent;
        hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
		if (!isDbEventShown(&dbei)) {
            free(dbei.pBlob);
	        continue;
    	}
		output = NULL;
		if (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_STATUSCHANGE) {
			int isSent = (dbei.flags & DBEF_SENT);
			char *szName = NULL;
			char *szText = NULL;
			if (isSent) {
				szName = getEncodedContactName(NULL, szProto, szRealProto);
   			} else {
                szName = getEncodedContactName(event->hContact, szProto, szRealProto);
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
			} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
                szText = encodeUTF8((char *)dbei.pBlob, szRealProto, ENF_NONE);
			}
			/* SRMM-specific formatting */
			Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divOut" : "divIn");
			if (dwFlags & SMF_LOG_SHOWICONS) {
				const char *iconFile = "";
				if (dbei.eventType == EVENTTYPE_MESSAGE) {
					iconFile = isSent ? "message_out.gif" : "message_in.gif";
				} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
					iconFile = "status.gif";
				}
				Utils::appendText(&output, &outputSize, "<img class=\"img\" src=\"%s/plugins/ieview/%s\"/>",
								workingDir, iconFile);
			}
			if (dwFlags & SMF_LOG_SHOWTIME) {
				const char *className = "";
				className = isSent ? "timeOut" : "timeIn";
				if (!(dwFlags & SMF_LOG_SHOWNICK) || (dbei.eventType == EVENTTYPE_STATUSCHANGE)) {
					const char *className2 = "";
					className2 = isSent ? "colonOut" : "colonIn";
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span><span class=\"%s\">: </span>",
							className, timestampToString(dwFlags, dbei.timestamp), className2);
				} else {
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s </span>",
							className, timestampToString(dwFlags, dbei.timestamp));
				}
			}
			if (dwFlags & SMF_LOG_SHOWNICK) {
                if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
					Utils::appendText(&output, &outputSize, "<span class=\"notices\">%s </span>", szName);
				} else {
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span><span class=\"%s\">: </span>",
								isSent ? "nameOut" : "nameIn", szName, isSent ? "colonOut" : "colonIn");
				}
			}
			const char *className = "";
			if (dbei.eventType == EVENTTYPE_MESSAGE) {
				className = isSent ? "messageOut" : "messageIn";
			} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
                className = "notices";
			}
            Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>", className, szText);
            Utils::appendText(&output, &outputSize, "</div>\n");
			event->hDbEventFirst = hCurDbEvent;
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
  
void HistoryHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event) {
 	if (Options::getHistoryFlags() & Options::TEMPLATES_ENABLED) {
		appendEventTemplate(view, event);
	} else {
		appendEventNonTemplate(view, event);
	}
}
