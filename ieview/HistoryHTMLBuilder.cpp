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

HistoryHTMLBuilder::HistoryHTMLBuilder() {
	setLastEventType(-1);
	setLastEventTime(time(NULL));
	startedTime = time(NULL);
}

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
			Utils::appendText(&output, &outputSize, ".divMessageIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n", (int) lineColor, (int) inColor);
			Utils::appendText(&output, &outputSize, ".divMessageOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n", (int) lineColor, (int) outColor);
			Utils::appendText(&output, &outputSize, ".divFileIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n", (int) lineColor, (int) inColor);
			Utils::appendText(&output, &outputSize, ".divFileOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n", (int) lineColor, (int) outColor);
			Utils::appendText(&output, &outputSize, ".divUrlIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n", (int) lineColor, (int) inColor);
			Utils::appendText(&output, &outputSize, ".divUrlOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n", (int) lineColor, (int) outColor);
			Utils::appendText(&output, &outputSize, ".divSystem {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n", (int) lineColor, (int) inColor);
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

void HistoryHTMLBuilder::appendEventNonTemplate(IEView *view, IEVIEWEVENT *event) {

	DWORD dwFlags = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTIME, 0) ? SMF_LOG_SHOWTIME : 0;
    dwFlags |= !DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_HIDENAMES, 0) ? SMF_LOG_SHOWNICK : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWDATE, 0) ? SMF_LOG_SHOWDATES : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, 0) ? SMF_LOG_SHOWICONS : 0;
    dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSTATUSCHANGES, 0) ? SMF_LOG_SHOWSTATUSCHANGES : 0;
	char *szRealProto = getRealProto(event->hContact);
	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData!=NULL && (eventIdx < event->count || event->count==-1); eventData = eventData->next, eventIdx++) {
		int outputSize;
		char *output;
		output = NULL;
		int isSent = eventData->dwFlags & IEEDF_SENT;
		if (eventData->iType == IEED_EVENT_MESSAGE || eventData->iType == IEED_EVENT_STATUSCHANGE
			|| eventData->iType == IEED_EVENT_URL || eventData->iType == IEED_EVENT_FILE) {
			char *szName = NULL;
			char *szText = NULL;
			if (eventData->dwFlags & IEEDF_UNICODE_NICK) {
				szName = encodeUTF8(eventData->pszNickW, szRealProto, ENF_NAMESMILEYS);
   			} else {
                szName = encodeUTF8(eventData->pszNick, szRealProto, ENF_NAMESMILEYS);
			}
			if (eventData->dwFlags & IEEDF_UNICODE_TEXT) {
				szText = encodeUTF8(eventData->pszTextW, szRealProto, ENF_ALL);
   			} else {
                szText = encodeUTF8(eventData->pszText, event->codepage, szRealProto, ENF_ALL);
			}
			/* SRMM-specific formatting */
			const char *className = NULL;
			const char *iconFile = NULL;
			switch (eventData->iType) {
				case IEED_EVENT_MESSAGE:
					iconFile = "message.gif";
					Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divMessageOut" : "divMessageIn");
					break;
				case IEED_EVENT_FILE:
					iconFile = "file.gif";
					Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divFileOut" : "divFileIn");
					break;
				case IEED_EVENT_URL:
					iconFile = "url.gif";
					Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divUrlOut" : "divUrlIn");
					break;
				default:
					Utils::appendText(&output, &outputSize, "<div class=\"%s\">", "divSystem");
			}
			if (dwFlags & SMF_LOG_SHOWICONS && iconFile != NULL) {
				Utils::appendText(&output, &outputSize, "<img class=\"img\" src=\"%s/plugins/ieview/%s\"/> ",
								workingDir, iconFile);
			} else {
				Utils::appendText(&output, &outputSize, " ");
			}
			Utils::appendText(&output, &outputSize, "<span class=\"%s\"> %s:</span>",
									isSent ? "nameOut" : "nameIn", szName);

			Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>",
							isSent ? "timeOut" : "timeIn", timestampToString(dwFlags, eventData->time));

			if (eventData->iType == IEED_EVENT_MESSAGE) {
				Utils::appendText(&output, &outputSize, "<br>");
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
//	view->scrollToBottom();
}

void HistoryHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event) {
 	if (Options::getHistoryFlags() & Options::TEMPLATES_ENABLED) {
		appendEventTemplate(view, event);
	} else {
		appendEventNonTemplate(view, event);
	}
}
