#include "Smileys.h"

static char *dupString(const char *a) {
	if (a!=NULL) {
		char *b = new char[strlen(a)+1];
		strcpy(b, a);
		return b;
	} 
	return NULL;
}

SmileyMap* SmileyMap::mapList = NULL;

Smiley::Smiley(const char *file, const char *text) {
	next = NULL;
	wtext = NULL;
	this->text = dupString(text);
	this->file = dupString(file);
	fLength = strlen(file);
	tLength = strlen(text);
	wtext = new wchar_t[tLength+1];
	MultiByteToWideChar(CP_ACP, 0, text, -1, wtext, tLength+1);
	Utils::convertPath(this->file);
}

Smiley::~Smiley() {
	if (wtext != NULL) delete wtext;
	if (text != NULL) delete text;
	if (file != NULL) delete file;
}

const char *Smiley::getText() {
	return text;
}

const char *Smiley::getFile() {
	return file;
}

int Smiley::getTextLength() {
	return tLength;
}

int Smiley::getFileLength() {
	return fLength;
}

Smiley* Smiley::getNext() {
	return next;
}

bool Smiley::equals(const char *text) {
	if (!strncmp(text, this->text, tLength)) {
		return true;
	}
	return false;
}
/*
bool Smiley::equals(wchar_t *text) {
	if (!wscncmp(text, this->text, tLength)) {
		return true;
	}
	return false;
}
*/
bool SmileyMap::loadSmileyFile(const char *proto, const char *filename, bool onlyInfo) {
	FILE* fh;
	char tmp[1024], tmp2[1024];
	char pathstring[500];

	SmileyMap *smap = SmileyMap::add(proto, filename);
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
	char store[1024];
	while (fgets(store, sizeof(store), fh) != NULL) {
    	//is comment?
    	if (store[0] == ';') continue;
    	//empty line?
    	if (sscanf(store, "%s", tmp) == EOF) continue;
    	//name tag?
    	if (strncmp(store, "Name", 4) == 0) {
      		sscanf(store, "Name = \"%[^\"]", tmp);
      		continue;
    	}
    	//author tag?
    	if (strncmp(store, "Author", 6) == 0) {
      		sscanf(store, "Author = \"%[^\"]", tmp);
      		continue;
    	}
	    //date tag?
	    if (strncmp(store, "Date", 4) == 0) {
			sscanf(store, "Date = \"%[^\"]", tmp);
			continue;
	    }
	    //version tag?
	    if (strncmp(store, "Version", 7) == 0) {
			sscanf(store, "Version = \"%[^\"]", tmp);
			continue;
	    }
	    if (strncmp(store, "ButtonSmiley", 12) == 0) {
			sscanf(store, "ButtonSmiley = \"%[^\"]", tmp);
			continue;
	    }
	    //smiley icon tag?
	    if ( strncmp(store, "Smiley", 6) == 0  && !onlyInfo) {
			int iconIndex;
	    	char resourceFile[255];
	      	tmp[0] = 0; tmp2[0] = 0;
	    	if (store[6] == '*') { //hidden
		        sscanf(store, "Smiley* = \" %[^\"] \" , %d , \"%[^\"] \" , \"%[^\"]\" ",
		               resourceFile, &iconIndex, tmp, tmp2);
	    	} else {
        		sscanf(store, "Smiley = \" %[^\"] \" , %d , \"%[^\"] \" , \"%[^\"]\" ",
               			resourceFile, &iconIndex, tmp, tmp2);
			}
			strcpy(pathrun, resourceFile);
			strcpy(resourceFile, pathstring);
			int tokenMode = 0;
			int i, j, l;
			l = strlen(tmp);
			for (i=j=0; i<=l;i++) {
				switch (tokenMode) {
					case 0:
						if (tmp[i]!=' ' && tmp[i]!='\t' && tmp[i]!='\r' && tmp[i]!='\n') {
							j = i;
							tokenMode = 1;
						}
						break;
					case 1:
						if (tmp[i]==' ' || tmp[i]=='\t' || tmp[i]=='\r' || tmp[i]=='\n' || tmp[i]=='\0') {
                            tmp[i] = '\0';
							int m, n;
							for (m=n=j;m<i;) {
								if (!strncmp(tmp+m, "%%_%%", 5)) {
									tmp[n++] = ' ';
									m += 5;
								} else {
									tmp[n++] = tmp[m++];
								}
							}
							tmp[n] = '\0';
							//debugView->writef("Smile str: %s<br>", tmp+j);
							smap->addSmiley(resourceFile, tmp+j);
							tokenMode = 0;
						}
				}
			}
    	}
  	}
  	fclose(fh);
	return true;
}

Smiley* SmileyMap::getSmiley(const char *text) {
	Smiley *ptr, *foundPtr = NULL;
	int maxLen = 0;
	for (ptr=entries; ptr!=NULL; ptr=ptr->getNext()) {
		if (ptr->equals(text)) {
			if (maxLen < ptr->getTextLength()) {
				maxLen = ptr->getTextLength();
				foundPtr = ptr;
			}
		}
	}
	return foundPtr;
}

SmileyMap::SmileyMap(const char *name, const char *filename) {
	entries = NULL;
	next = NULL;
	this->name = dupString(name);
	this->filename = dupString(filename);
}

void SmileyMap::clear() {
	Smiley *ptr, *ptr2;
	ptr = entries;
	entries = NULL;
	for (;ptr!=NULL;ptr=ptr2) {
		ptr2 = ptr->getNext();
		delete ptr;
	}
}

SmileyMap::~SmileyMap() {
	if (name != NULL) {
		delete name;
	}
	if (filename != NULL) {
		delete filename;
	}
	clear();
}

Smiley* SmileyMap::getSmiley(const char *proto, const char *text) {
	SmileyMap *ptr;
	for (ptr=mapList; ptr!=NULL; ptr=ptr->next) {
		if (!strcmp(ptr->name, proto)) {
			return ptr->getSmiley(text);
		}
	}
	return NULL;
}

SmileyMap* SmileyMap::add(const char *proto, const char *filename) {
	SmileyMap *map;
	for (map=mapList; map!=NULL; map=map->next) {
		if (!strcmp(map->name, proto)) {
			map->clear();
			map->setFilename(filename);
			return map;
		}
	}
	map = new SmileyMap(proto, filename);
	map->next = mapList;
	mapList = map;
	return map;
}

void SmileyMap::setFilename(const char *filename) {
	if (this->filename != NULL) {
		delete this->filename;
	}
	this->filename = dupString(filename);
}

const char *SmileyMap::getFilename() {
	return filename;
}

void SmileyMap::addSmiley(const char *file, const char *text) {
	Smiley *smiley = new Smiley(file, text);
	smiley->next = entries;
	entries = smiley;
}


SmileyMap *	SmileyMap::getSmileyMap(const char *proto) {
	SmileyMap *map;
	for (map=mapList; map!=NULL; map=map->next) {
		if (!strcmp(map->name, proto)) {
			break;
		}
	}
	return map;
}

void SmileyMap::loadLibrary(const char *proto, const char *filename) {
	loadSmileyFile(proto, filename, false);
}
