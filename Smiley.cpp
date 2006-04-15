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
//#include "Smiley.h"
#include "Utils.h"

SmileyPattern::SmileyPattern(const char *pattern) {
	next = NULL;
	length = 0;
	this->pattern = Utils::dupString(pattern);
	this->wpattern = Utils::convertToWCS(this->pattern);
	if (this->pattern!=NULL) {
		length = strlen(pattern);
	}
}

SmileyPattern::~SmileyPattern() {
	if (pattern != NULL) delete pattern;
	if (wpattern != NULL) delete wpattern;
}

void SmileyPattern::setNext(SmileyPattern *ptr) {
	next = ptr;
}

SmileyPattern * SmileyPattern::getNext() {
	return next;
}

int SmileyPattern::getLength() {
	return length;
}

bool SmileyPattern::equals(const char *text) {
	if (!strncmp(text, this->pattern, length)) {
		return true;
	}
	return false;
}

bool SmileyPattern::equals(const wchar_t *wtext) {
	if (!wcsncmp(wtext, this->wpattern, length)) {
		return true;
	}
	return false;
}

const char *SmileyPattern::getPattern() {
	return pattern;
}

SmileyMap* SmileyMap::mapList = NULL;

Smiley::Smiley(const char *file, const char *description, bool isHidden) {
	next = NULL;
	patterns = NULL;
	this->file = Utils::dupString(file);
	this->description = Utils::dupString(description);
	Utils::convertPath(this->file);
	hidden = isHidden;
}

Smiley::~Smiley() {
	SmileyPattern *ptr, *ptr2;
	if (file != NULL) delete file;
	if (description != NULL) delete description;
	ptr = patterns;
	for (;ptr!=NULL;ptr=ptr2) {
		ptr2 = ptr->getNext();
		delete ptr;
	}
}

void Smiley::setNext(Smiley *ptr) {
	next = ptr;
}

Smiley* Smiley::getNext() {
	return next;
}


SmileyPattern *	Smiley::addPattern(const char *pattern) {
	SmileyPattern *p, *ptr;
	p = new SmileyPattern(pattern);
	for (ptr = patterns; ptr!=NULL && ptr->getNext()!=NULL; ptr = ptr->getNext());
	if (ptr == NULL) {
		patterns = p;
	} else {
		ptr->setNext(p);
	}
	return p;
}

const char *Smiley::getPatternString() {
	if (patterns != NULL) {
		return patterns->getPattern();
	}
	return "";
}

const char *Smiley::getDescription() {
	return description;
}

const char *Smiley::getFile() {
	return file;
}

int Smiley::match(const char *text, int minLen) {
	int maxl = minLen;
	for (SmileyPattern *ptr = patterns; ptr!=NULL; ptr=ptr->getNext()) {
		if (ptr->getLength() > maxl) {
			if (ptr->equals(text)) {
				maxl = ptr->getLength();
			}
		}
	}
	return maxl > minLen ? maxl : 0;
}

int Smiley::match(const wchar_t *wtext, int minLen) {
	int maxl = minLen;
	for (SmileyPattern *ptr = patterns; ptr!=NULL; ptr=ptr->getNext()) {
		if (ptr->getLength() > maxl) {
			if (ptr->equals(wtext)) {
				maxl = ptr->getLength();
			}
		}
	}
	return maxl > minLen ? maxl : 0;
}

bool Smiley::isHidden() {
	return hidden;
}

SmileyMap::SmileyMap(const char *name, const char *filename) {
	entries = NULL;
	next = NULL;
	this->name = Utils::dupString(name);
	this->filename = Utils::dupString(filename);
	this->version = Utils::dupString("");
	this->author = Utils::dupString("");
	this->description = Utils::dupString("");
//	window = new SmileyWindow(this);
	smileyNum = 0;
	visibleSmileyNum = 0;
}

SmileyMap::SmileyMap() {
	entries = NULL;
	next = NULL;
//	window = NULL;
	smileyNum = 0;
	visibleSmileyNum = 0;
	this->name = Utils::dupString("");
	this->filename = Utils::dupString("");
	this->version = Utils::dupString("");
	this->author = Utils::dupString("");
	this->description = Utils::dupString("");
}

SmileyMap::~SmileyMap() {
	if (name != NULL) {
		delete name;
	}
	if (filename != NULL) {
		delete filename;
	}
	if (version != NULL) {
		delete version;
	}
	if (author != NULL) {
		delete author;
	}
	if (description != NULL) {
		delete description;
	}
//	if (window != NULL) {
//		delete window;
//	}
	clear();
}

void SmileyMap::clear() {
	Smiley *ptr, *ptr2;
	ptr = entries;
	entries = NULL;
	for (;ptr!=NULL;ptr=ptr2) {
		ptr2 = ptr->getNext();
		delete ptr;
	}
	smileyNum = 0;
	visibleSmileyNum = 0;
}


Smiley* SmileyMap::getSmiley(const char *text, int *maxLen) {
	int l;
	Smiley *ptr, *foundPtr = NULL;
	*maxLen = 0;
	for (ptr=entries; ptr!=NULL; ptr=ptr->getNext()) {
		if (l=ptr->match(text, *maxLen)) {
			*maxLen = l;
			foundPtr = ptr;
		}
	}
	return foundPtr;
}

Smiley* SmileyMap::getSmiley(const wchar_t *text, int *maxLen) {
	int l;
	Smiley *ptr, *foundPtr = NULL;
	*maxLen = 0;
	for (ptr=entries; ptr!=NULL; ptr=ptr->getNext()) {
		if (l=ptr->match(text, *maxLen)) {
			*maxLen = l;
			foundPtr = ptr;
		}
	}
	return foundPtr;
}

Smiley* SmileyMap::getSmiley(const char *proto, const char *text, int *len) {
	SmileyMap *ptr;
	for (ptr=mapList; ptr!=NULL; ptr=ptr->next) {
		if (!strcmp(ptr->name, proto)) {
			return ptr->getSmiley(text, len);
		}
	}
	return NULL;
}

Smiley* SmileyMap::getSmiley() {
	return entries;
}

//SmileyWindow* SmileyMap::getWindow() {
//	return window;
//}

void SmileyMap::remove(const char *proto) {
	SmileyMap *map, *lastmap;
	for (map=mapList, lastmap=NULL; map!=NULL; map=map->next) {
		if (!strcmp(map->name, proto)) {
			if (lastmap == NULL) {
				mapList = map->next;
			} else {
				lastmap->next = map->next;
			}
			delete map;
			break;
		}
		lastmap = map;
	}
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
	this->filename = Utils::dupString(filename);
}

void SmileyMap::setAuthor(const char *author) {
	if (this->author != NULL) {
		delete this->author;
	}
	this->author = Utils::dupString(author);
}

void SmileyMap::setVersion(const char *version) {
	if (this->version != NULL) {
		delete this->version;
	}
	this->version = Utils::dupString(version);
}

void SmileyMap::setDescription(const char *description) {
	if (this->description != NULL) {
		delete this->description;
	}
	this->description = Utils::dupString(description);
}

const char *SmileyMap::getFilename() {
	return filename;
}

const char *SmileyMap::getAuthor() {
	return author;
}

const char *SmileyMap::getVersion() {
	return version;
}

const char *SmileyMap::getDescription() {
	return description;
}

Smiley* SmileyMap::addSmiley(const char *file, const char *description, bool isHidden) {
	Smiley *p, *ptr;
	p = new Smiley(file, description, isHidden);
	for (ptr = entries; ptr!=NULL && ptr->getNext()!=NULL; ptr = ptr->getNext());
	if (ptr == NULL) {
		entries = p;
	} else {
		ptr->setNext(p);
	}
	if (!isHidden) visibleSmileyNum++;
	smileyNum++;
	return p;
}

int SmileyMap::getSmileyNum() {
	return 	smileyNum;
}

int SmileyMap::getVisibleSmileyNum() {
	return 	visibleSmileyNum;
}

SmileyMap *	SmileyMap::getSmileyMap(const char *proto) {
	SmileyMap *map;
	if (proto == NULL) return NULL;
	for (map=mapList; map!=NULL; map=map->next) {
		if (!strcmp(map->name, proto)) {
			break;
		}
	}
	return map;
}


SmileyMap *SmileyMap::getLibraryInfo(const char *filename) {
	FILE* fh;
	char description[1024];
	char store[1024];

	if (filename == NULL || strlen(filename) == 0) {
		return NULL;
	}
	fh = fopen(filename, "rt");
	if (fh == NULL) {
		return NULL;
	}

	SmileyMap *smap = new SmileyMap();
	smap->setFilename(filename);
	while (fgets(store, sizeof(store), fh) != NULL) {
    	//is comment?
    	if (store[0] == ';') continue;
    	//empty line?
    	if (sscanf(store, "%s", description) == EOF) continue;
    	//name tag?
    	if (strncmp(store, "Name", 4) == 0) {
      		sscanf(store, "Name = \"%[^\"]", description);
			smap->setDescription(description);
      		continue;
    	}
    	//author tag?
    	if (strncmp(store, "Author", 6) == 0) {
      		sscanf(store, "Author = \"%[^\"]", description);
			smap->setAuthor(description);
      		continue;
    	}
	    //date tag?
	    if (strncmp(store, "Date", 4) == 0) {
			sscanf(store, "Date = \"%[^\"]", description);
			//smap->setDescription(description);
			continue;
	    }
	    //version tag?
	    if (strncmp(store, "Version", 7) == 0) {
			sscanf(store, "Version = \"%[^\"]", description);
			smap->setVersion(description);
			continue;
	    }
	}
  	fclose(fh);
	return smap;
}

bool SmileyMap::loadLibrary(const char *proto, const char *filename) {
	FILE* fh;
	int previewW = 40, previewH =30;
	char patterns[1024], description[1024];
	char pathstring[500];

	if (filename == NULL || strlen(filename) == 0) {
        SmileyMap::remove(proto);
		return false;
	}
	strcpy(pathstring, filename);
	char* pathrun = pathstring + strlen(pathstring);
	while ((*pathrun != '\\' && *pathrun != '/') && (pathrun > pathstring)) pathrun--;
	pathrun++;
	*pathrun = '\0';

	fh = fopen(filename, "rt");
	if (fh == NULL) {
        SmileyMap::remove(proto);
		return false;
	}
	SmileyMap *smap = SmileyMap::add(proto, filename);
//	if (smileyFlags & Options::SMILEY_ENABLED) { // && smileyFlags & Options::SMILEY_PROTOCOLS
	{
		char store[1024];
		while (fgets(store, sizeof(store), fh) != NULL) {
	    	//is comment?
	    	if (store[0] == ';') continue;
	    	//empty line?
	    	if (sscanf(store, "%s", description) == EOF) continue;
	    	//name tag?
	    	if (strncmp(store, "Name", 4) == 0) {
	      		sscanf(store, "Name = \"%[^\"]", description);
				smap->setDescription(description);
	      		continue;
	    	}
	    	//author tag?
	    	if (strncmp(store, "Author", 6) == 0) {
	      		sscanf(store, "Author = \"%[^\"]", description);
				smap->setAuthor(description);
	      		continue;
	    	}
		    //date tag?
		    if (strncmp(store, "Date", 4) == 0) {
				sscanf(store, "Date = \"%[^\"]", description);
				//smap->setDescription(description);
				continue;
		    }
		    //version tag?
		    if (strncmp(store, "Version", 7) == 0) {
				sscanf(store, "Version = \"%[^\"]", description);
				smap->setVersion(description);
				continue;
		    }
		    if (strncmp(store, "SelectionSize", 12) == 0) {
				sscanf(store, "SelectionSize = %d, %d", &previewW, &previewH);
				if (previewW < 10) previewW = 10;
				else if (previewW > 100) previewW = 100;
				if (previewH < 10) previewH = 10;
				else if (previewH > 100) previewH = 100;
				continue;
		    }
		    //smiley icon tag?
		    if (strncmp(store, "Smiley", 6) == 0) {
				int iconIndex;
				bool isHidden;
		    	char resourceFile[255];
		      	patterns[0] = 0; description[0] = 0;
		      	isHidden = 0;
		    	if (store[6] == '*') { //hidden
			        sscanf(store, "Smiley* = \" %[^\"] \" , %d , \"%[^\"] \" , \"%[^\"]\" ",
			               resourceFile, &iconIndex, patterns, description);
			      	isHidden = 1;
		    	} else {
	        		sscanf(store, "Smiley = \" %[^\"] \" , %d , \"%[^\"] \" , \"%[^\"]\" ",
	               			resourceFile, &iconIndex, patterns, description);
				}
				strcpy(pathrun, resourceFile);
				strcpy(resourceFile, pathstring);
				Smiley *smiley = smap->addSmiley(resourceFile, description, isHidden);
				int tokenMode = 0;
				int i, j, l;
				l = strlen(patterns);
				for (i=j=0; i<=l;i++) {
					switch (tokenMode) {
						case 0:
							if (patterns[i]!=' ' && patterns[i]!='\t' && patterns[i]!='\r' && patterns[i]!='\n') {
								j = i;
								tokenMode = 1;
							}
							break;
						case 1:
							if (patterns[i]==' ' || patterns[i]=='\t' || patterns[i]=='\r' || patterns[i]=='\n' || patterns[i]=='\0') {
	                            patterns[i] = '\0';
								int m, n;
								for (m=n=j;m<i;) {
									if (!strncmp(patterns+m, "%%_%%", 5)) {
										patterns[n++] = ' ';
										m += 5;
									} else if (!strncmp(patterns+m, "%%''%%", 6)) {
										patterns[n++] = '\"';
										m += 6;
									} else {
										patterns[n++] = patterns[m++];
									}
								}
								patterns[n] = '\0';
								smiley->addPattern(patterns+j);
								tokenMode = 0;
							}
					}
				}
	    	}
	  	}
//	  	smap->getWindow()->init(previewW, previewH);
	}
  	fclose(fh);
	return true;
}

void SmileyMap::setSelectorBackground(DWORD color) {
	SmileyMap *map;
	for (map=mapList; map!=NULL; map=map->next) {
//		if (map->getWindow() != NULL) {
	//		map->getWindow()->setBackground(color);
		//}
	}
}
