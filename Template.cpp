#include "Template.h"
#include "Utils.h"

Token::Token(int type, const char *text) {
	next = NULL;
	this->type = type;
	if (text!=NULL) {
		this->text = Utils::dupString(text);
	} else {
        this->text = NULL;
	}
}

Token::~Token() {
	if (text!=NULL) {
		delete text;
	}
}

Token * Token::getNext() {
	return next;
}

void Token::setNext(Token *ptr) {
	next = ptr;
}

int Token::getType() {
	return type;
}

const char *Token::getText() {
	return text;
}

Template::Template(const char *name, const char *text) {
	next = NULL;
	tokens = NULL;
	this->text = Utils::dupString(text);
	this->name = Utils::dupString(name);
	tokenize();
}

Template::~Template() {
	if (text != NULL) delete text;
	if (name != NULL) delete name;
	Token *ptr = tokens, *ptr2;
	tokens = NULL;
	for (;ptr!=NULL;ptr = ptr2) {
		ptr2 = ptr->getNext();
		delete ptr;
	}
}

const char *Template::getText() {
	return text;
}

const char *Template::getName() {
	return name;
}

Template* Template::getNext() {
	return next;
}

bool Template::equals(const char *name) {
	if (!strcmp(name, this->name)) {
		return true;
	}
	return false;
}

void Template::tokenize() {
	if (text!=NULL) {
//		debugView->writef("Tokenizing: %s<br>---<br>", text);
		char *str = Utils::dupString(text);
		Token *lastToken = NULL;
		int lastTokenType = Token::PLAIN;
		int l = strlen(str);
		for (int i=0, lastTokenStart=0; i<=l;) {
			Token *newToken;
			int newTokenType, newTokenSize;
			if (str[i]=='\0') {
				newTokenType = Token::END;
				newTokenSize = 1;
			} else if (!strncmp(str+i, "%name%", 6)) {
				newTokenType = Token::NAME;
				newTokenSize = 6;
			} else if (!strncmp(str+i, "%time%", 6)) {
				newTokenType = Token::TIME;
				newTokenSize = 6;
			} else if (!strncmp(str+i, "%text%", 6)) {
				newTokenType = Token::TEXT;
				newTokenSize = 6;
			} else if (!strncmp(str+i, "%date%", 6)) {
				newTokenType = Token::DATE;
				newTokenSize = 6;
			} else if (!strncmp(str+i, "%base%", 6)) {
				newTokenType = Token::BASE;
				newTokenSize = 6;
			} else if (!strncmp(str+i, "%avatar%", 8)) {
				newTokenType = Token::AVATAR;
				newTokenSize = 8;
			} else if (!strncmp(str+i, "%cid%", 5)) {
				newTokenType = Token::CID;
				newTokenSize = 5;
			} else if (!strncmp(str+i, "%proto%", 7)) {
				newTokenType = Token::PROTO;
				newTokenSize = 7;
			} else if (!strncmp(str+i, "%avatarIn%", 10)) {
				newTokenType = Token::AVATARIN;
				newTokenSize = 10;
			} else if (!strncmp(str+i, "%avatarOut%", 11)) {
				newTokenType = Token::AVATAROUT;
				newTokenSize = 11;
			} else if (!strncmp(str+i, "%nameIn%", 8)) {
				newTokenType = Token::NAMEIN;
				newTokenSize = 8;
			} else if (!strncmp(str+i, "%nameOut%", 9)) {
				newTokenType = Token::NAMEOUT;
				newTokenSize = 9;
			} else if (!strncmp(str+i, "%uin%", 5)) {
				newTokenType = Token::UIN;
				newTokenSize = 5;
			} else if (!strncmp(str+i, "%uinIn%", 7)) {
				newTokenType = Token::UININ;
				newTokenSize = 7;
			} else if (!strncmp(str+i, "%uinOut%", 8)) {
				newTokenType = Token::UINOUT;
				newTokenSize = 8;
			} else {
				newTokenType = Token::PLAIN;
				newTokenSize = 1;
			}
			if (newTokenType != Token::PLAIN) {
				if (str[i + newTokenSize] == '%') {
                    //newTokenSize++;
				}
				str[i] = '\0';
			}
			if (lastTokenType!=newTokenType && i!=lastTokenStart) {
				if (lastTokenType == Token::PLAIN) {
                    newToken = new Token(lastTokenType, str+lastTokenStart);
				} else {
					newToken = new Token(lastTokenType, NULL);
				}
				if (lastToken != NULL) {
					lastToken->setNext(newToken);
				} else {
					tokens = newToken;
				}
				lastToken = newToken;
				lastTokenStart = i;
			}
			lastTokenType = newTokenType;
			i += newTokenSize;
		}
		delete str;
	}
}

Token * Template::getTokens() {
	return tokens;
}

TemplateMap* TemplateMap::mapList = NULL;

TemplateMap::TemplateMap(const char *name) {
	entries = NULL;
	next = NULL;
	filename = NULL;
	this->name = Utils::dupString(name);
}

TemplateMap* TemplateMap::add(const char *proto, const char *filename) {
	TemplateMap *map;
	for (map=mapList; map!=NULL; map=map->next) {
		if (!strcmp(map->name, proto)) {
			map->clear();
			map->setFilename(filename);
			return map;
		}
	}
	map = new TemplateMap(proto);
	map->setFilename(filename);
	map->next = mapList;
	mapList = map;
	return map;
}

void TemplateMap::addTemplate(const char *name, const char *text) {
	Template *tmplate = new Template(name, text);
	tmplate->next = entries;
	entries = tmplate;
}

void TemplateMap::clear() {
	Template *ptr, *ptr2;
	ptr = entries;
	entries = NULL;
	for (;ptr!=NULL;ptr=ptr2) {
		ptr2 = ptr->getNext();
		delete ptr;
	}
}
bool TemplateMap::loadTemplateFile(const char *proto, const char *filename, bool onlyInfo) {
	FILE* fh;
	char lastTemplate[1024], tmp2[1024];
	char pathstring[500];

	TemplateMap *tmap = TemplateMap::add(proto, filename);
	if (filename == NULL || strlen(filename) == 0) {
		return false;
	}
	strcpy(pathstring, filename);
	char* pathrun = pathstring + strlen(pathstring);
	while ((*pathrun != '\\' && *pathrun != '/') && (pathrun > pathstring)) pathrun--;
	pathrun++;
	*pathrun = '\0';

	fh = fopen(filename, "rt");
	if (fh == NULL) {
		return false;
	}
	char store[4096];
	bool wasTemplate = false;
	char *templateText = NULL;
	int templateTextSize = 0;
	while (fgets(store, sizeof(store), fh) != NULL) {
    	if (sscanf(store, "%s", tmp2) == EOF) continue;
	    //template start
	    if (!onlyInfo) {
            if ( !strncmp(store, "<!--HTMLStart-->", 16) ||
				 !strncmp(store, "<!--MessageIn-->", 16) ||
				 !strncmp(store, "<!--MessageOut-->", 17) ||
				 !strncmp(store, "<!--hMessageIn-->", 17) ||
				 !strncmp(store, "<!--hMessageOut-->", 18) ||
				 !strncmp(store, "<!--File-->", 11) ||
				 !strncmp(store, "<!--hFile-->", 12) ||
				 !strncmp(store, "<!--URL-->", 10) ||
				 !strncmp(store, "<!--hURL-->", 11) ||
				 !strncmp(store, "<!--Status-->", 13) ||
				 !strncmp(store, "<!--hStatus-->", 14) ||
				 !strncmp(store, "<!--MessageInGroupStart-->", 26) ||
				 !strncmp(store, "<!--MessageInGroupInner-->", 26) ||
				 !strncmp(store, "<!--MessageInGroupEnd-->", 24) ||
				 !strncmp(store, "<!--hMessageInGroupStart-->", 27) ||
				 !strncmp(store, "<!--hMessageInGroupInner-->", 27) ||
				 !strncmp(store, "<!--hMessageInGroupEnd-->", 25) ||
				 !strncmp(store, "<!--MessageOutGroupStart-->", 27) ||
				 !strncmp(store, "<!--MessageOutGroupInner-->", 27) ||
				 !strncmp(store, "<!--MessageOutGroupEnd-->", 25) ||
				 !strncmp(store, "<!--hMessageOutGroupStart-->", 28) ||
				 !strncmp(store, "<!--hMessageOutGroupInner-->", 28) ||
				 !strncmp(store, "<!--hMessageOutGroupEnd-->", 26)
				 ) {
				if (wasTemplate) {
					tmap->addTemplate(lastTemplate, templateText);
	                //debugView->writef("1. %s<br>%s", lastTemplate, templateText);
				}
				if (templateText!=NULL) {
					free (templateText);
				}
				templateText = NULL;
				templateTextSize = 0;
				wasTemplate = true;
                sscanf(store, "<!--%[^-]", lastTemplate);
			} else if (wasTemplate) {
				Utils::appendText(&templateText, &templateTextSize, "%s", store);
			}
		}
  	}
  	if (wasTemplate) {
		tmap->addTemplate(lastTemplate, templateText);
        //debugView->writef("2. %s<br>%s", lastTemplate, templateText);
	}
  	fclose(fh);
	static const char *groupTemplates[] = {"MessageInGroupStart", "MessageInGroupInner",
	                                       "hMessageInGroupStart", "hMessageInGroupInner",
										   "MessageOutGroupStart", "MessageOutGroupInner",
	                                       "hMessageOutGroupStart", "hMessageOutGroupInner"};
	tmap->grouping = true;
	for (int i=0; i<8; i++) {
		if (tmap->getTemplate(groupTemplates[i])== NULL) {
			tmap->grouping = false;
			break;
		}
	}
	return true;
}

bool TemplateMap::isGrouping() {
	return grouping;
}

Template* TemplateMap::getTemplate(const char *text) {
	Template *ptr;
	for (ptr=entries; ptr!=NULL; ptr=ptr->getNext()) {
		if (ptr->equals(text)) {
			break;
		}
	}
	return ptr;
}

Template* TemplateMap::getTemplate(const char *proto, const char *text) {
	TemplateMap *ptr;
	for (ptr=mapList; ptr!=NULL; ptr=ptr->next) {
		if (!strcmp(ptr->name, proto)) {
			return ptr->getTemplate(text);
		}
	}
	return NULL;
}

TemplateMap* TemplateMap::getTemplateMap(const char *proto) {
	TemplateMap *ptr;
	for (ptr=mapList; ptr!=NULL; ptr=ptr->next) {
		if (!strcmp(ptr->name, proto)) {
			return ptr;
		}
	}
	return NULL;
}

const char *TemplateMap::getFilename() {
    return filename;
}    

void TemplateMap::setFilename(const char *filename) {
	if (this->filename != NULL) {
	    delete this->filename;
	}
	this->filename = Utils::dupString(filename);
	Utils::convertPath(this->filename);
}    

void TemplateMap::loadTemplates(const char *proto, const char *filename) {
	loadTemplateFile(proto, filename, false);
}


