#include "TemplateHTMLBuilder.h"

#include "Template.h"
#include "Utils.h"
#include "m_avatars.h"

#define EVENTTYPE_STATUSCHANGE 25368

TemplateHTMLBuilder::TemplateHTMLBuilder() {
	iLastEventType = -1;
	startedTime = time(NULL);
	lastEventTime = time(NULL);
	groupTemplate = NULL;
}

const char *TemplateHTMLBuilder::getTemplateFilename(ProtocolSettings * protoSettings) {
	return protoSettings->getSRMMTemplateFilename();
}

const char *TemplateHTMLBuilder::getTemplateFilenameRtl(ProtocolSettings * protoSettings) {
	return protoSettings->getSRMMTemplateFilenameRtl();
}

int TemplateHTMLBuilder::getFlags(ProtocolSettings * protoSettings) {
	return protoSettings->getSRMMFlags();
}

char *TemplateHTMLBuilder::timestampToString(DWORD dwFlags, time_t check, int mode)
{
    static char szResult[512];
    char str[80];
    DBTIMETOSTRING dbtts;
    dbtts.cbDest = 70;;
    dbtts.szDest = str;
    szResult[0] = '\0';
	if (mode) { //time
		dbtts.szFormat = (dwFlags & Options::LOG_SHOW_SECONDS) ? (char *)"s" : (char *)"t";
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
        if (dwFlags & Options::LOG_RELATIVE_DATE && check >= today) {
            strcpy(szResult, Translate("Today"));
        }
        else if(dwFlags & Options::LOG_RELATIVE_DATE && check > (today - 86400)) {
            strcpy(szResult, Translate("Yesterday"));
        }
        else {
			dbtts.szFormat = (dwFlags & Options::LOG_LONG_DATE) ? (char *)"D" : (char *)"d";
			CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
		    strncat(szResult, str, 500);
        }
	}
	Utils::UTF8Encode(szResult, szResult, 500);
    return szResult;
}

void TemplateHTMLBuilder::buildHeadTemplate(IEView *view, IEVIEWEVENT *event, ProtocolSettings *protoSettings) {
	DBVARIANT dbv;
	CONTACTINFO ci;
	char tempBase[1024];
	char tempStr[1024];
	HANDLE hRealContact;
	char *szRealProto = NULL;
	char *szBase=NULL;
	char *szNoAvatar=NULL;
	char *szProto = NULL;
	char *szNameIn = NULL;
	char *szNameOut = NULL;
	char *szUINIn = NULL;
	char *szUINOut = NULL;
	char *szAvatarIn = NULL;
	char *szAvatarOut = NULL;
	char *szNickIn = NULL;
	char *szNickOut = NULL;
	char *szStatusMsg = NULL;
	int outputSize;
	char *output;

	output = NULL;
	hRealContact = getRealContact(event->hContact);
	szRealProto = getProto(hRealContact);
	szProto = getProto(event->pszProto, event->hContact);
	tempBase[0]='\0';
	if (protoSettings == NULL) {
		return;
	}

    TemplateMap *tmpm = TemplateMap::getTemplateMap((event->dwFlags & IEEF_RTL) ? getTemplateFilenameRtl(protoSettings) : getTemplateFilename(protoSettings));

	if (tmpm!=NULL) {
		strcpy(tempBase, "file://");
    	strcat(tempBase, tmpm->getFilename());
    	char* pathrun = tempBase + strlen(tempBase);
    	while ((*pathrun != '\\' && *pathrun != '/') && (pathrun > tempBase)) pathrun--;
    	pathrun++;
    	*pathrun = '\0';
	}
	szBase = Utils::UTF8Encode(tempBase);
	getUINs(event->hContact, szUINIn, szUINOut);
	if (getFlags(protoSettings) & Options::LOG_SHOW_NICKNAMES) {
		szNameOut = getEncodedContactName(NULL, szProto, szRealProto);
		szNameIn = getEncodedContactName(event->hContact, szProto, szRealProto);
	} else {
        szNameOut = Utils::dupString("&nbsp;");
        szNameIn = Utils::dupString("&nbsp;");
	}
	sprintf(tempStr, "%snoavatar.jpg", tempBase);
	szNoAvatar = Utils::UTF8Encode(tempStr);
	if (Options::getAvatarServiceFlags() & Options::AVATARSERVICE_PRESENT) {
		struct avatarCacheEntry *ace = (struct avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)event->hContact, 0);
		if (ace!=NULL) {
			szAvatarIn = Utils::UTF8Encode(ace->szFilename);
		}
	}
	if (szAvatarIn == NULL) {
		if (!DBGetContactSetting(event->hContact, "ContactPhoto", "File",&dbv)) {
			if (strlen(dbv.pszVal) > 0) {
				/* relative -> absolute */
				char tmpPath[MAX_PATH];
				strcpy (tmpPath, dbv.pszVal);
				if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)&& strncmp(tmpPath, "http://", 7)) {
					CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
				}
				szAvatarIn = Utils::UTF8Encode(tmpPath);
				Utils::convertPath(szAvatarIn);
			}
			DBFreeVariant(&dbv);
		}
	}
	if (szAvatarIn == NULL) {
        szAvatarIn = Utils::dupString(szNoAvatar);
	}
	if (Options::getAvatarServiceFlags() & Options::AVATARSERVICE_PRESENT) {
		struct avatarCacheEntry *ace = (struct avatarCacheEntry *)CallService(MS_AV_GETMYAVATAR, (WPARAM)0, (LPARAM)szRealProto);
		if (ace!=NULL) {
			szAvatarOut = Utils::UTF8Encode(ace->szFilename);
		}
	}
	if (szAvatarOut == NULL) {
		if (!DBGetContactSetting(NULL, "ContactPhoto", "File",&dbv)) {
		    if (strlen(dbv.pszVal) > 0) {
				/* relative -> absolute */
			    char tmpPath[MAX_PATH];
			    strcpy (tmpPath, dbv.pszVal);
			    if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)&& strncmp(tmpPath, "http://", 7)) {
	    			CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
			   	}
	            szAvatarOut = Utils::UTF8Encode(tmpPath);
			    Utils::convertPath(szAvatarOut);
		    }
	       	DBFreeVariant(&dbv);
		}
	}
	if (szAvatarOut == NULL) {
        szAvatarOut = Utils::dupString(szNoAvatar);
	}
	if (!DBGetContactSetting(event->hContact, "CList", "StatusMsg",&dbv)) {
	    if (strlen(dbv.pszVal) > 0) {
       		szStatusMsg = Utils::UTF8Encode(dbv.pszVal);
	    }
       	DBFreeVariant(&dbv);
	}
	ZeroMemory(&ci, sizeof(ci));
    ci.cbSize = sizeof(ci);
    ci.hContact = event->hContact;
    ci.szProto = szProto;
    ci.dwFlag = CNF_NICK;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
        szNickIn = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS);
	}
	ZeroMemory(&ci, sizeof(ci));
    ci.cbSize = sizeof(ci);
    ci.hContact = NULL;
    ci.szProto = szProto;
    ci.dwFlag = CNF_NICK;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
        szNickOut = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS);
	}

	Template *tmplt = TemplateMap::getTemplate((event->dwFlags & IEEF_RTL) ? getTemplateFilenameRtl(protoSettings) : getTemplateFilename(protoSettings), "HTMLStart");

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
				case Token::NAMEIN:
                    tokenVal = szNameIn;
					break;
				case Token::NAMEOUT:
                    tokenVal = szNameOut;
					break;
				case Token::AVATARIN:
			    	tokenVal = szAvatarIn;
					break;
				case Token::AVATAROUT:
			    	tokenVal = szAvatarOut;
					break;
				case Token::PROTO:
				    tokenVal = szRealProto;
				    break;
				case Token::UININ:
				    tokenVal = szUINIn;
				    break;
				case Token::UINOUT:
				    tokenVal = szUINOut;
				    break;
				case Token::STATUSMSG:
				    tokenVal = szStatusMsg;
				    break;
				case Token::NICKIN:
				    tokenVal = szNickIn;
				    break;
 				case Token::NICKOUT:
				    tokenVal = szNickOut;
 				    break;
			}
			if (tokenVal != NULL) {
				if (token->getEscape()) {
					char *escapedToken  = Utils::escapeString(tokenVal);
					Utils::appendText(&output, &outputSize, "%s", escapedToken);
					delete escapedToken;
				} else {
					Utils::appendText(&output, &outputSize, "%s", tokenVal);
				}
			}
		}
	}
	if (output != NULL) {
        view->write(output);
		free(output);
	}
	if (szBase!=NULL) delete szBase;
    if (szRealProto!=NULL) delete szRealProto;
    if (szProto!=NULL) delete szProto;
	if (szUINIn!=NULL) delete szUINIn;
	if (szUINOut!=NULL) delete szUINOut;
	if (szNoAvatar!=NULL) delete szNoAvatar;
	if (szAvatarIn!=NULL) delete szAvatarIn;
	if (szAvatarOut!=NULL) delete szAvatarOut;
	if (szNameIn!=NULL) delete szNameIn;
	if (szNameOut!=NULL) delete szNameOut;
	if (szNickIn!=NULL) delete szNickIn;
	if (szNickOut!=NULL) delete szNickOut;
	if (szStatusMsg!=NULL) delete szStatusMsg;
	view->scrollToBottom();
	groupTemplate = NULL;
	iLastEventType = -1;
}

void TemplateHTMLBuilder::appendEventTemplate(IEView *view, IEVIEWEVENT *event, ProtocolSettings* protoSettings) {
	DBVARIANT dbv;
	CONTACTINFO ci;
	HANDLE hRealContact;
	char *szRealProto = NULL;
	char tempBase[1024];
	char *szBase=NULL;
	char tempStr[1024];
	char *szNoAvatar=NULL;
	char szCID[32];
	char *szName = NULL;
	char *szNameIn = NULL;
	char *szNameOut = NULL;
	char *szUIN = NULL;
	char *szUINIn = NULL;
	char *szUINOut = NULL;
	char *szNickIn = NULL;
	char *szNickOut = NULL;
	char *szStatusMsg = NULL;
	char *szAvatar = NULL;
	char *szAvatarIn = NULL;
	char *szAvatarOut = NULL;
	char *szText = NULL;
	char *szProto = NULL;
	char *szFileDesc = NULL;
	const char *tmpltName[2];
	bool isGrouping = false;
//	DWORD today = (DWORD)time(NULL);
//	today = today - today % 86400;
	hRealContact = getRealContact(event->hContact);
	szRealProto = getProto(hRealContact);
	szProto = getProto(event->pszProto, event->hContact);
	tempBase[0]='\0';
	if (protoSettings == NULL) {
		return;
	}
	TemplateMap *tmpm = TemplateMap::getTemplateMap((event->dwFlags & IEEF_RTL) ? getTemplateFilenameRtl(protoSettings) : getTemplateFilename(protoSettings));
	if (tmpm!=NULL) {
		strcpy(tempBase, "file://");
    	strcat(tempBase, tmpm->getFilename());
    	char* pathrun = tempBase + strlen(tempBase);
    	while ((*pathrun != '\\' && *pathrun != '/') && (pathrun > tempBase)) pathrun--;
    	pathrun++;
    	*pathrun = '\0';
    	isGrouping = tmpm->isGrouping();
	}
	szBase = Utils::UTF8Encode(tempBase);
	getUINs(event->hContact, szUINIn, szUINOut);
	if (getFlags(protoSettings) & Options::LOG_SHOW_NICKNAMES) {
		szNameOut = getEncodedContactName(NULL, szProto, szRealProto);
		szNameIn = getEncodedContactName(event->hContact, szProto, szRealProto);
	} else {
        szNameOut = Utils::dupString("&nbsp;");
        szNameIn = Utils::dupString("&nbsp;");
	}
	sprintf(tempStr, "%snoavatar.jpg", tempBase);
	szNoAvatar = Utils::UTF8Encode(tempStr);
	if (Options::getAvatarServiceFlags() & Options::AVATARSERVICE_PRESENT) {
		struct avatarCacheEntry *ace = (struct avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)event->hContact, 0);
		if (ace!=NULL) {
			szAvatarIn = Utils::UTF8Encode(ace->szFilename);
		}
	} else {
		if(DBGetContactSettingWord(event->hContact, szProto, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
			if (!DBGetContactSetting(event->hContact, "ContactPhoto", "File",&dbv)) {
			    if (strlen(dbv.pszVal) > 0) {
					/* relative -> absolute */
				    char tmpPath[MAX_PATH];
				    strcpy (tmpPath, dbv.pszVal);
				    if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)&& strncmp(tmpPath, "http://", 7)) {
		    			CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
				   	}
		       		szAvatarIn = Utils::UTF8Encode(tmpPath);
				    Utils::convertPath(szAvatarIn);
			    }
		       	DBFreeVariant(&dbv);
			}
		}
	}
	if (szAvatarIn == NULL) {
        szAvatarIn = Utils::dupString(szNoAvatar);
	}
	if (Options::getAvatarServiceFlags() & Options::AVATARSERVICE_PRESENT) {
		struct avatarCacheEntry *ace = (struct avatarCacheEntry *)CallService(MS_AV_GETMYAVATAR, (WPARAM)0, (LPARAM)szRealProto);
		if (ace!=NULL) {
			szAvatarOut = Utils::UTF8Encode(ace->szFilename);
		}
	} else {
		if (!DBGetContactSetting(NULL, "ContactPhoto", "File",&dbv)) {
		    if (strlen(dbv.pszVal) > 0) {
				/* relative -> absolute */
			    char tmpPath[MAX_PATH];
			    strcpy (tmpPath, dbv.pszVal);
			    if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)&& strncmp(tmpPath, "http://", 7)) {
	    			CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)tmpPath);
			   	}
	       		szAvatarOut = Utils::UTF8Encode(tmpPath);
			    Utils::convertPath(szAvatarOut);
		    }
	       	DBFreeVariant(&dbv);
		}
	}
	if (szAvatarOut == NULL) {
        szAvatarOut = Utils::dupString(szNoAvatar);
	}
	if (!DBGetContactSetting(event->hContact, "CList", "StatusMsg",&dbv)) {
	    if (strlen(dbv.pszVal) > 0) {
       		szStatusMsg = Utils::UTF8Encode(dbv.pszVal);
	    }
       	DBFreeVariant(&dbv);
	}
	ZeroMemory(&ci, sizeof(ci));
    ci.cbSize = sizeof(ci);
    ci.hContact = event->hContact;
    ci.szProto = szProto;
    ci.dwFlag = CNF_NICK;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
        szNickIn = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS);
	}
	ZeroMemory(&ci, sizeof(ci));
    ci.cbSize = sizeof(ci);
    ci.hContact = NULL;
    ci.szProto = szProto;
    ci.dwFlag = CNF_NICK;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
        szNickOut = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS);
	}
//	char *szRealProto = getRealProto(event->hContact);
	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData!=NULL && (eventIdx < event->count || event->count==-1); eventData = eventData->next, eventIdx++) {
		int outputSize;
		char *output;
		output = NULL;
		if (eventData->iType == IEED_EVENT_MESSAGE || eventData->iType == IEED_EVENT_STATUSCHANGE || eventData->iType == IEED_EVENT_FILE || eventData->iType == IEED_EVENT_URL) {
			int isSent = (eventData->dwFlags & IEEDF_SENT);
			int isHistory = (eventData->time < (DWORD)getStartedTime() && (!(eventData->dwFlags & IEEDF_UNREAD) || eventData->dwFlags & IEEDF_SENT));
			int isGroupBreak = TRUE;
 		  	if ((getFlags(protoSettings) & Options::LOG_GROUP_MESSAGES) && eventData->dwFlags == LOWORD(getLastEventType())
			  && eventData->iType == IEED_EVENT_MESSAGE && HIWORD(getLastEventType()) == IEED_EVENT_MESSAGE
			  && (isSameDate(eventData->time, getLastEventTime()))
//			  && ((eventData->time < today) == (getLastEventTime() < today))
			  && (((eventData->time < (DWORD)startedTime) == (getLastEventTime() < (DWORD)startedTime)) || eventData->dwFlags & IEEDF_UNREAD)) {
		        isGroupBreak = FALSE;
		    }
			if (isSent) {
				szName = szNameOut;
				szAvatar = szAvatarOut;
				szUIN = szUINOut;
				sprintf(szCID, "%d", 0);
			} else {
				szName = szNameIn;
				szAvatar = szAvatarIn;
				szUIN = szUINIn;
				sprintf(szCID, "%d", (int)event->hContact);
			}
			tmpltName[0] = groupTemplate;
			tmpltName[1] = NULL;
			groupTemplate = NULL;
			szText = NULL;
			szFileDesc = NULL;
			/*
			if (eventData->dwFlags & IEEDF_UNICODE_NICK) {
				szName = encodeUTF8(eventData->pszNickW, szRealProto, ENF_NAMESMILEYS);
   			} else {
                szName = encodeUTF8(eventData->pszNick, szRealProto, ENF_NAMESMILEYS);
			}*/
			if (eventData->dwFlags & IEEDF_UNICODE_TEXT) {
				szText = encodeUTF8(event->hContact, szRealProto, eventData->pszTextW, ENF_ALL);
   			} else {
                szText = encodeUTF8(event->hContact, szRealProto, eventData->pszText, event->codepage, ENF_ALL);
			}
			if (eventData->dwFlags & IEEDF_UNICODE_TEXT2) {
				szFileDesc = encodeUTF8(event->hContact, szRealProto, eventData->pszText2W, ENF_ALL);
   			} else {
                szFileDesc = encodeUTF8(event->hContact, szRealProto, eventData->pszText2, event->codepage, ENF_ALL);
			}
			if (eventData->iType == IEED_EVENT_MESSAGE) {
                if (isGrouping && (getFlags(protoSettings) & Options::LOG_GROUP_MESSAGES)) {
	                if (isGroupBreak) {
              		    tmpltName[1] = isHistory ? isSent ? "hMessageOutGroupStart" : "hMessageInGroupStart" : isSent ? "MessageOutGroupStart" : "MessageInGroupStart";
                   	} else {
                   		tmpltName[0] = isHistory ? isSent ? "hMessageOutGroupInner" : "hMessageInGroupInner" : isSent ? "MessageOutGroupInner" : "MessageInGroupInner";
                   	}
               		groupTemplate = isHistory ? isSent ? "hMessageOutGroupEnd" : "hMessageInGroupEnd" : isSent ? "MessageOutGroupEnd" : "MessageInGroupEnd";
               	} else {
               		tmpltName[1] = isHistory ? isSent ? "hMessageOut" : "hMessageIn" : isSent ? "MessageOut" : "MessageIn";
               	}
			} else if (eventData->iType == IEED_EVENT_FILE) {
                tmpltName[1] = isHistory ? isSent ? "hFileOut" : "hFileIn" : isSent ? "FileOut" : "FileIn";
                Template *tmplt = (event->dwFlags & IEEF_RTL) ? TemplateMap::getTemplate(getTemplateFilenameRtl(protoSettings), tmpltName[1]) : TemplateMap::getTemplate(getTemplateFilename(protoSettings), tmpltName[1]);
                if (tmplt == NULL) {
                	tmpltName[1] = isHistory ? "hFile" : "File";
				}
			} else if (eventData->iType == IEED_EVENT_URL) {
                tmpltName[1] = isHistory ? isSent ? "hURLOut" : "hURLIn" : isSent ? "URLOut" : "URLIn";
                Template *tmplt = (event->dwFlags & IEEF_RTL) ? TemplateMap::getTemplate(getTemplateFilenameRtl(protoSettings), tmpltName[1]) : TemplateMap::getTemplate(getTemplateFilename(protoSettings), tmpltName[1]);
                if (tmplt == NULL) {
	                tmpltName[1] = isHistory ? "hURL" : "URL";
				}
			} else if (eventData->iType == IEED_EVENT_STATUSCHANGE) {
                tmpltName[1] = isHistory ? "hStatus" : "Status";
			}
			/* template-specific formatting */
			for (int i=0;i<2;i++) {
				Template *tmplt;
				if (tmpltName[i] == NULL) continue;
				tmplt = TemplateMap::getTemplate((event->dwFlags & IEEF_RTL) ? getTemplateFilenameRtl(protoSettings) : getTemplateFilename(protoSettings), tmpltName[i]);
				if (tmplt == NULL) continue;
				for (Token *token = tmplt->getTokens();token!=NULL;token=token->getNext()) {
					const char *tokenVal;
					tokenVal = NULL;
					switch (token->getType()) {
						case Token::PLAIN:
                            tokenVal = token->getText();
							break;
						case Token::NAME:
                            tokenVal = szName;
							break;
						case Token::TIME:
							if (getFlags(protoSettings) & Options::LOG_SHOW_TIME) {
	                            tokenVal = timestampToString(getFlags(protoSettings), eventData->time, 1);
							} else {
                                tokenVal = "&nbsp;";
							}
							break;
						case Token::DATE:
							if (getFlags(protoSettings) & Options::LOG_SHOW_DATE) {
	                            tokenVal = timestampToString(getFlags(protoSettings), eventData->time, 0);
							} else {
                                tokenVal = "&nbsp;";
							}
							break;
	  					case Token::TEXT:
							tokenVal = szText;
							break;
	  					case Token::AVATAR:
  					    	tokenVal = szAvatar;
							break;
	  					case Token::CID:
							tokenVal = szCID;
							break;
						case Token::BASE:
						    tokenVal = szBase;
						    break;
						case Token::NAMEIN:
		                    tokenVal = szNameIn;
							break;
						case Token::NAMEOUT:
		                    tokenVal = szNameOut;
							break;
						case Token::AVATARIN:
					    	tokenVal = szAvatarIn;
							break;
						case Token::AVATAROUT:
					    	tokenVal = szAvatarOut;
							break;
						case Token::PROTO:
						    tokenVal = szRealProto;
						    break;
						case Token::UIN:
						    tokenVal = szUIN;
						    break;
						case Token::UININ:
						    tokenVal = szUINIn;
						    break;
						case Token::UINOUT:
						    tokenVal = szUINOut;
						    break;
						case Token::STATUSMSG:
						    tokenVal = szStatusMsg;
						    break;
						case Token::NICKIN:
						    tokenVal = szNickIn;
						    break;
		 				case Token::NICKOUT:
						    tokenVal = szNickOut;
		 				    break;
						case Token::FILEDESC:
							tokenVal = szFileDesc;
							break;
					}
					if (tokenVal != NULL) {
						if (token->getEscape()) {
							char *escapedToken  = Utils::escapeString(tokenVal);
							Utils::appendText(&output, &outputSize, "%s", escapedToken);
							delete escapedToken;
						} else {
							Utils::appendText(&output, &outputSize, "%s", tokenVal);
						}
					}
				}
			}
			setLastEventType(MAKELONG(eventData->dwFlags, eventData->iType));
			setLastEventTime(eventData->time);
			if (szText!=NULL) delete szText;
		}
		if (output != NULL) {
            view->write(output);
			free(output);
		}
    }
	if (szBase!=NULL) delete szBase;
    if (szRealProto!=NULL) delete szRealProto;
    if (szProto!=NULL) delete szProto;
	if (szUINIn!=NULL) delete szUINIn;
	if (szUINOut!=NULL) delete szUINOut;
	if (szNoAvatar!=NULL) delete szNoAvatar;
	if (szAvatarIn!=NULL) delete szAvatarIn;
	if (szAvatarOut!=NULL) delete szAvatarOut;
	if (szNameIn!=NULL) delete szNameIn;
	if (szNameOut!=NULL) delete szNameOut;
	if (szNickIn!=NULL) delete szNickIn;
	if (szNickOut!=NULL) delete szNickOut;
	if (szStatusMsg!=NULL) delete szStatusMsg;
	if (szFileDesc!=NULL) delete szFileDesc;
    view->documentClose();
}

time_t TemplateHTMLBuilder::getStartedTime() {
	return startedTime;
}

