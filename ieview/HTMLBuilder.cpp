#include "HTMLBuilder.h"
#include "Utils.h"
#include "Smiley.h"
#include "Options.h"

TextToken::TextToken(int type, const char *text, int len) {
	next = NULL;
	this->type = type;
	if (text!=NULL) {
		this->text = Utils::dupString(text, len);
	} else {
        this->text = NULL;
	}
	wtext = new wchar_t[len+1];
	MultiByteToWideChar(CP_ACP, 0, this->text, -1, wtext, len+1);
	this->link = NULL;
}

TextToken::~TextToken() {
	if (text!=NULL) {
		delete text;
	}
	if (wtext!=NULL) {
		delete wtext;
	}
	if (link!=NULL) {
		delete link;
	}
}

TextToken * TextToken::getNext() {
	return next;
}

void TextToken::setNext(TextToken *ptr) {
	next = ptr;
}

int TextToken::getType() {
	return type;
}

const char *TextToken::getText() {
	return text;
}

const char *TextToken::getLink() {
	return link;
}

void TextToken::setLink(const char *link) {
    if (this->link != NULL) {
        delete this->link;
    }    
    this->link = Utils::dupString(link);
}    

static int countNoWhitespace(const char *str) {
	int c;
	for (c=0; *str!='\n' && *str!='\r' && *str!='\t' && *str!=' ' && *str!='\0'; str++, c++);
	return c;
}

TextToken* TextToken::tokenizeLinks(const char *text) {
    TextToken *firstToken = NULL, *lastToken = NULL;
    int lastTokenType = TEXT;
    int l = strlen(text);
    for (int i=0, lastTokenStart=0; i<=l;) {
        TextToken *newToken;
		int newTokenType, newTokenSize;
        if (text[i]=='\0') {
			newTokenType = END;
			newTokenSize = 1;
		} else if (!strncmp(text+i, "ftp:/", 5)) {
			newTokenType = LINK;
        	newTokenSize = countNoWhitespace(text+i);
       	} else if (!strncmp(text+i, "http:/", 6)) {
			newTokenType = LINK;
       		newTokenSize = countNoWhitespace(text+i);
  		} else if (!strncmp(text+i, "www.", 4)) {
			newTokenType = WWWLINK;
  			newTokenSize = countNoWhitespace(text+i);
     	} else if (!strncmp(text+i, "mailto:", 7)) {
			newTokenType = LINK;
            newTokenSize = countNoWhitespace(text+i);
        } else {
			newTokenType = TEXT;
			newTokenSize = 1;
		}
		if (lastTokenType!=TEXT || (lastTokenType!=newTokenType && i!=lastTokenStart)) {
            newToken = new TextToken(lastTokenType, text+lastTokenStart, i-lastTokenStart);
            if (lastTokenType == WWWLINK || lastTokenType == LINK) {
                newToken->setLink(newToken->getText());
            }    
			if (lastToken == NULL) {
				firstToken = newToken;
			} else {
			    lastToken->setNext(newToken);
			}    
			lastToken = newToken;
			lastTokenStart = i;
		}
		lastTokenType = newTokenType;
		i += newTokenSize;
    }
    return firstToken;
}    

TextToken* TextToken::tokenizeSmileys(const char *proto, const char *text) {
 	SmileyMap *smileyMap;
    TextToken *firstToken = NULL, *lastToken = NULL;
	Smiley *lastSmiley = NULL;
	bool wasSpace;
    int lastTokenType = TEXT;
    int l = strlen(text);
	if (!(Options::getSmileyFlags() & Options::SMILEY_ENABLED)) {
	    return new TextToken(TEXT, text, l);
 	}   
	if (Options::getSmileyFlags() & Options::SMILEY_PROTOCOLS) {
		smileyMap = SmileyMap::getSmileyMap(proto);
	} else {
		smileyMap = SmileyMap::getSmileyMap("");
    }
	if (smileyMap == NULL) {
 		return new TextToken(TEXT, text, l);
 	}   
 	wasSpace = true;
    for (int i=0, lastTokenStart=0; i<=l;) {
        TextToken *newToken;
		int newTokenType, newTokenSize;
		Smiley *newSmiley = NULL;
        if (text[i]=='\0') {
			newTokenType = END;
			newTokenSize = 1;
		} else {
		    Smiley* smiley = NULL;
      		if (wasSpace) {
    			smiley = smileyMap->getSmiley(text+i, &newTokenSize);
                if (smiley != NULL) {
                   	newTokenType = SMILEY;
                   	newSmiley = smiley;
                   	if (Options::getSmileyFlags() & Options::SMILEY_ISOLATED) {
						int dummy;
                       	if (smileyMap->getSmiley(text+i+newTokenSize, &dummy)==NULL && text[i+newTokenSize]!='\n'
                       		&& text[i+newTokenSize]!='\r' && text[i+newTokenSize]!='\t' 
                         	&& text[i+newTokenSize]!=' ' && text[i+newTokenSize]!='\0') {
                         	    smiley = NULL;
                  		}    
                     }  		
                } 
            }    
            if (smiley == NULL) {
                wasSpace = true;
               	if (Options::getSmileyFlags() & Options::SMILEY_ISOLATED) {
                    if (text[i]!='\n' && text[i]!='\r' && text[i]!='\t' && text[i]!=' ') {
                        wasSpace = false;
                    }    
                }    
           	    smiley = NULL;
    			newTokenType = TEXT;
    			newTokenSize = 1;
    			newSmiley = NULL;
            }
        }    
		if (lastTokenType!=TEXT || (lastTokenType!=newTokenType && i!=lastTokenStart)) {
            if (lastTokenType == SMILEY) {
                newToken = new TextToken(lastTokenType, text+lastTokenStart, i-lastTokenStart);
                newToken->setLink(lastSmiley->getFile());
            } else {
                newToken = new TextToken(lastTokenType, text+lastTokenStart, i-lastTokenStart);
            }    
			if (lastToken == NULL) {
				firstToken = newToken;
			} else {
			    lastToken->setNext(newToken);
			}
			lastToken = newToken;
			lastTokenStart = i;
		}
		lastSmiley = newSmiley;
		lastTokenType = newTokenType;
		i += newTokenSize;
    }
    return firstToken;
}    


void TextToken::toString(char **str, int *sizeAlloced) {
    char *eText = NULL, *eLink = NULL;
    switch (type) {
        case TEXT:
            eText = urlEncode(text);
            Utils::appendText(str, sizeAlloced, "%s", eText);
            break;
        case WWWLINK:
            eText = urlEncode(text);
            eLink = urlEncode(link);
            Utils::appendText(str, sizeAlloced, "<a class=\"link\" target=\"_self\" href=\"http://%s\">%s</a>", eLink, eText);
            break;
        case LINK:
            eText = urlEncode(text);
            eLink = urlEncode(link);
            Utils::appendText(str, sizeAlloced, "<a class=\"link\" target=\"_self\" href=\"%s\">%s</a>", eLink, eText);
            break;
        case SMILEY:
            eText = urlEncode(text);
            if (Options::getSmileyFlags() & Options::SMILEY_SURROUND) {
            	Utils::appendText(str, sizeAlloced, "<img class=\"img\" src=\"%s\" alt=\"%s\" /> ", link, eText);
            } else {
            	Utils::appendText(str, sizeAlloced, "<img class=\"img\" src=\"%s\" alt=\"%s\" />", link, eText);
            }     
            break;
    }    
    if (eText!=NULL) delete eText;
    if (eLink!=NULL) delete eLink;
}    

char *TextToken::urlEncode(const char *str) {
    char *out;
    const char *ptr;
    bool wasSpace;
    int c;
    c = 0;
    wasSpace = false;
    for (ptr=str; *ptr!='\0'; ptr++) {
    	if (*ptr==' ' && wasSpace) {
   			wasSpace = true;
   		 	c += 6;
   		} else {
   		    wasSpace = false;
        	switch (*ptr) {
        		case '\n': c += 4; break;
    			case '\r': break;
        		case '&': c += 5; break;
        		case '>': c += 4; break;
        		case '<': c += 4; break;
        		case '"': c += 6; break;
    			case ' ': wasSpace = true;
        		default: c += 1; break;
        	}
      	}   	
    }    
    char *output = new char[c+1];
    wasSpace = false;
    for (out=output, ptr=str; *ptr!='\0'; ptr++) {
    	if (*ptr==' ' && wasSpace) {
	      	strcpy(out, "&nbsp;"); 
    		out += 6; 
   		} else {    
   		    wasSpace = false;
        	switch (*ptr) {
    			case '\n': strcpy(out, "<br>"); out += 4; break;
    			case '\r': break;
    			case '&': strcpy(out, "&amp;"); out += 5; break;
    			case '>': strcpy(out, "&gt;"); out += 4; break;
    			case '<': strcpy(out, "&lt;"); out += 4; break;
    			case '"': strcpy(out, "&quot;"); out += 6; break;
    			case ' ': wasSpace = true;
        		default: *out = *ptr; out += 1; break;
        	}
         }   	
    }    
    *out  = '\0';
    return output;
}

char *TextToken::urlEncode2(const char *str) {
    char *out;
    const char *ptr;
    int c;
    c = 0;
    for (ptr=str; *ptr!='\0'; ptr++) {
    	switch (*ptr) {
    		case '>': c += 4; break;
    		case '<': c += 4; break;
    		case '"': c += 6; break;
    		default: c += 1; break;
    	}
    }    
    char *output = new char[c+1];
    for (out=output, ptr=str; *ptr!='\0'; ptr++) {
    	switch (*ptr) {
			case '>': strcpy(out, "&gt;"); out += 4; break;
			case '<': strcpy(out, "&lt;"); out += 4; break;
			case '"': strcpy(out, "&quot;"); out += 6; break;
    		default: *out = *ptr; out += 1; break;
    	}
    }    
    *out  = '\0';
    return output;
}

char * HTMLBuilder::encode(const char *text, const char *proto, bool useSmiley) {
	char *output;
 	int outputSize;
	output = NULL;
	if (text == NULL) return NULL;
	TextToken *token, *token1, *token2, *token3;
	for (token = token1 = TextToken::tokenizeLinks(text);token!=NULL;token=token1) {
	    token1 = token->getNext();
	    if (useSmiley && token->getType() == TextToken::TEXT) {
    		for (token2 = token3 = TextToken::tokenizeSmileys(proto, token->getText());token2!=NULL;token2=token3) {
    		    token3 = token2->getNext();
    	    	token2->toString(&output, &outputSize);
    	    	delete token2;
            }
        } else {
        	token->toString(&output, &outputSize);
        }    
        delete token;
	}
	return output;
}
