#include "TemplateHTMLBuilder.h"

#include "Options.h"
#include "Template.h"
#include "Utils.h"

// srmm stuff
#define EVENTTYPE_STATUSCHANGE 25368
#define SRMMMOD "SRMM"

#define SRMSGSET_SHOWLOGICONS      "ShowLogIcon"
#define SRMSGSET_HIDENAMES         "HideNames"
#define SRMSGSET_SHOWTIME          "ShowTime"
#define SRMSGSET_SHOWDATE          "ShowDate"
#define SRMSGSET_SHOWSTATUSCHANGES "ShowStatusChanges"

TemplateHTMLBuilder::TemplateHTMLBuilder() {
	isCleared = true;
	iLastEventType = -1;
	startedTime = time(NULL);
	lastEventTime = time(NULL);
	groupTemplate = NULL;
}    

bool TemplateHTMLBuilder::isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei)
{
    switch (dbei->eventType) {
        case EVENTTYPE_MESSAGE:
			return 1;
        case EVENTTYPE_FILE:
			if (Options::getTemplatesFlags() & Options::LOG_SHOW_FILE) return 1;
			return 0;
        case EVENTTYPE_URL:
			if (Options::getTemplatesFlags() & Options::LOG_SHOW_URL) return 1;
			return 0;
        case EVENTTYPE_STATUSCHANGE:
			if (Options::getTemplatesFlags() & Options::LOG_SHOW_STATUSCHANGE) return 1;
            return 0;
    }
    return 0;
}


char *TemplateHTMLBuilder::makeRelativeDate(time_t check, int mode)
{
    static char szResult[512];
    char str[80];
    DBTIMETOSTRING dbtts;
    dbtts.cbDest = 70;;
    dbtts.szDest = str;
    szResult[0] = '\0';
	if (mode) { //time
		dbtts.szFormat = (Options::getTemplatesFlags() & Options::LOG_SHOW_SECONDS) ? (char *)"s" : (char *)"t";
		CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
	    strncat(szResult, str, 500);
	} else {    //date
	    struct tm tm_now, tm_today;
	    time_t now = time(NULL);
	    time_t today;
        tm_now = *localtime(&now);
        tm_today = tm_now;
        tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
        today = mktime(&tm_today);
        if (Options::getTemplatesFlags() & Options::LOG_RELATIVE_DATE && check >= today) {
            strcpy(szResult, Translate("Today"));
        }
        else if(Options::getTemplatesFlags() & Options::LOG_RELATIVE_DATE && check > (today - 86400)) {
            strcpy(szResult, Translate("Yesterday"));
        }
        else {
			dbtts.szFormat = (Options::getTemplatesFlags() & Options::LOG_LONG_DATE) ? (char *)"D" : (char *)"d";
			CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
		    strncat(szResult, str, 500);
        }
	}
    return szResult;
}


void TemplateHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event) {
	groupTemplate = NULL;
	isCleared = true;
	isCleared = false;
	int outputSize;
	char *output;
    char szBase[1024];
	output = NULL;
	Template *tmplt = TemplateMap::getTemplate("default", "HTMLStart");
	TemplateMap *tmpm = TemplateMap::getTemplateMap("default");
	szBase[0]='\0';
	if (tmpm!=NULL) {
    	strcpy(szBase, tmpm->getFilename());
    	char* pathrun = szBase + strlen(szBase);
    	while ((*pathrun != '\\' && *pathrun != '/') && (pathrun > szBase)) pathrun--;
    	pathrun++;
    	*pathrun = '\0';
	}
	if (tmplt!=NULL) {
		for (Token *token = tmplt->getTokens();token!=NULL;token=token->getNext()) {
			const char *tokenVal;
			tokenVal = NULL;
			switch (token->getType()) {
				case Token::PLAIN:
					tokenVal = token->getText();
					break;
				case Token::BASE:
     				tokenVal = szBase;
                   break;
			}
			if (tokenVal != NULL) {
				Utils::appendText(&output, &outputSize, "%s", tokenVal);
			}
		}
	}
	if (output != NULL) {
        view->write(output);
		free(output);
	}
	groupTemplate = NULL;
}

void TemplateHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event) {
	char szBase[1024];
	char szNoAvatar[1024];
	const char *tmpltName[2];
	bool isGrouping = false;

   	TemplateMap *tmpm = TemplateMap::getTemplateMap("default");
	szBase[0]='\0';
	if (tmpm!=NULL) {
    	strcpy(szBase, tmpm->getFilename());
    	char* pathrun = szBase + strlen(szBase);
    	while ((*pathrun != '\\' && *pathrun != '/') && (pathrun > szBase)) pathrun--;
    	pathrun++;
    	*pathrun = '\0';
    	isGrouping = tmpm->isGrouping();
	}
	sprintf(szNoAvatar, "%snoavatar.jpg", szBase);

	char *szProto = _strdup((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) event->hContact, 0));
	if (isCleared) {
		groupTemplate = "HTMLStart";
	}
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

		if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE )) {
			CallService(MS_DB_EVENT_MARKREAD, (WPARAM) event->hContact, (LPARAM) hDbEvent);
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM) event->hContact, (LPARAM) hDbEvent);
		} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
			CallService(MS_DB_EVENT_MARKREAD, (WPARAM) event->hContact, (LPARAM) hDbEvent);
		}
        hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
		if (!isDbEventShown(0, &dbei)) {
            free(dbei.pBlob);
	        continue;
    	}
		output = NULL;
		if (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_STATUSCHANGE || dbei.eventType == EVENTTYPE_FILE || dbei.eventType == EVENTTYPE_URL) {
			int isSent = (dbei.flags & DBEF_SENT);
			int isHistory = (dbei.timestamp < (DWORD)getStartedTime() && (dbei.flags & DBEF_READ || dbei.flags & DBEF_SENT));
			int isGroupBreak = TRUE;
 		  	if ((Options::getTemplatesFlags() & Options::LOG_GROUP_MESSAGES) && dbei.flags == LOWORD(getLastEventType())
			  && dbei.eventType == EVENTTYPE_MESSAGE && HIWORD(getLastEventType()) == EVENTTYPE_MESSAGE
			  && ((dbei.timestamp - getLastEventTime()) < 86400)) {
		        isGroupBreak = FALSE;
		    }
			char *szName = "";
			char *szText = "";
			char *szAvatar = NULL;
			char szCID[32];
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
				sprintf(szCID, "%d", 0);
   			} else {
   				DBVARIANT dbv;
   				if (!DBGetContactSetting(event->hContact, "ContactPhoto", "File",&dbv)) {
   				    if (strlen(dbv.pszVal) > 0) {
			       		szAvatar = _strdup(dbv.pszVal);
			       		Utils::convertPath(szAvatar);
		       		}				    
           			DBFreeVariant(&dbv);
   				}    
                szName = encodeUTF8((char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) event->hContact, 0), NULL, false);
				sprintf(szCID, "%d", (int)event->hContact);
            }
			tmpltName[0] = groupTemplate;
			tmpltName[1] = NULL;
			groupTemplate = NULL;
			if (dbei.eventType == EVENTTYPE_MESSAGE) {
				DWORD aLen = strlen((char *)dbei.pBlob)+1;
				if (dbei.cbBlob > aLen) {
					DWORD wlen = Utils::safe_wcslen((wchar_t *)&dbei.pBlob[aLen], (dbei.cbBlob - aLen) / 2);
					if (wlen > 0 && wlen < aLen) {
                        szText = encodeUTF8((wchar_t *)&dbei.pBlob[aLen], szProto, true);
					} else {
                        szText = encodeUTF8((char *)dbei.pBlob, szProto, true);
					}
				} else {
                	szText = encodeUTF8((char *)dbei.pBlob, szProto, true);
				}
                if (isGrouping && (Options::getTemplatesFlags() & Options::LOG_GROUP_MESSAGES)) {
                    if (isGroupBreak || isCleared) {
              		    tmpltName[1] = isHistory ? isSent ? "hMessageOutGroupStart" : "hMessageInGroupStart" : isSent ? "MessageOutGroupStart" : "MessageInGroupStart";
                   	} else {
                   		tmpltName[0] = isHistory ? isSent ? "hMessageOutGroupInner" : "hMessageInGroupInner" : isSent ? "MessageOutGroupInner" : "MessageInGroupInner";
                   	}    
               		groupTemplate = isHistory ? isSent ? "hMessageOutGroupEnd" : "hMessageInGroupEnd" : isSent ? "MessageOutGroupEnd" : "MessageInGroupEnd";
               	} else {
               		tmpltName[1] = isHistory ? isSent ? "hMessageOut" : "hMessageIn" : isSent ? "MessageOut" : "MessageIn";
               	}    
			} else if (dbei.eventType == EVENTTYPE_FILE) {
                szText = encodeUTF8((char *)dbei.pBlob + sizeof(DWORD), NULL, false);
                tmpltName[1] = isHistory ? "hFile" : "File";
			} else if (dbei.eventType == EVENTTYPE_URL) {
                szText = encodeUTF8((char *)dbei.pBlob, NULL, false);
                tmpltName[1] = isHistory ? "hURL" : "URL";
			} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
                szText = encodeUTF8((char *)dbei.pBlob, NULL, false);
                tmpltName[1] = isHistory ? "hStatus" : "Status";
			}
			/* template-specific formatting */
			for (int i=0;i<2;i++) {
				Template *tmplt;
				if (tmpltName[i] == NULL) continue;
				tmplt = TemplateMap::getTemplate("default", tmpltName[i]);
				if (tmplt == NULL) continue;
				for (Token *token = tmplt->getTokens();token!=NULL;token=token->getNext()) {
					const char *tokenVal;
					tokenVal = NULL;
					switch (token->getType()) {
						case Token::PLAIN:
                            tokenVal = token->getText();
							break;
						case Token::NAME:
							if (Options::getTemplatesFlags() & Options::LOG_SHOW_NICKNAMES) {
	                            tokenVal = szName;
							} else {
                                tokenVal = "&nbsp;";
							}
							break;
						case Token::TIME:
							if (Options::getTemplatesFlags() & Options::LOG_SHOW_TIME) {
	                            tokenVal = makeRelativeDate(dbei.timestamp, 1);
							} else {
                                tokenVal = "&nbsp;";
							}
							break;
						case Token::DATE:
							if (Options::getTemplatesFlags() & Options::LOG_SHOW_DATE) {
	                            tokenVal = makeRelativeDate(dbei.timestamp, 0);
							} else {
                                tokenVal = "&nbsp;";
							}
							break;
	  					case Token::TEXT:
							tokenVal = szText;
							break;
	  					case Token::AVATAR:
	  					    if (szAvatar != NULL) {
	  					    	tokenVal = szAvatar;
 							} else {
                                tokenVal = szNoAvatar;
 							}    
							break;
	  					case Token::CID:
							tokenVal = szCID;
							break;
						case Token::BASE:
						    tokenVal = szBase;
						    break;
					}
					if (tokenVal != NULL) {
						Utils::appendText(&output, &outputSize, "%s", tokenVal);
					}
				}
				if (isCleared) {
					isCleared = false;
					view->write(output);
					free(output);
					output = NULL;
				}
			}
			setLastEventType(MAKELONG(dbei.flags, dbei.eventType));
			setLastEventTime(dbei.timestamp);
			if (szAvatar!=NULL) free (szAvatar);
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

time_t TemplateHTMLBuilder::getStartedTime() {
	return startedTime;
}

int TemplateHTMLBuilder::getLastEventType() {
	return iLastEventType;
}

void TemplateHTMLBuilder::setLastEventType(int t) {
	iLastEventType = t;
}

time_t TemplateHTMLBuilder::getLastEventTime() {
	return lastEventTime;
}

void TemplateHTMLBuilder::setLastEventTime(time_t t) {
	lastEventTime = t;
}

    
