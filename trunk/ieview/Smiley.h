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
class SmileyMap;
class Smiley;
#ifndef SMILEYS_INCLUDED
#define SMILEYS_INCLUDED

#include "ieview_common.h"

class SmileyPattern {
private:
    wchar_t *		wpattern;
	char *			pattern;
	int   			length;
	SmileyPattern *	next;
protected:
	friend class Smiley;
	SmileyPattern(const char *pattern);
	void            	setNext(SmileyPattern *ptr);
public:
	~SmileyPattern();
	SmileyPattern *		getNext();
	const char *        getPattern();
	bool        		equals(const char *text);
	bool        		equals(const wchar_t *wtext);
	int         		getLength();
};

class Smiley {
private:
	char *		file;
	char *		description;
	bool        hidden;
	Smiley *	next;
	SmileyPattern *patterns;
protected:
	friend class SmileyMap;
	Smiley(const char *file, const char *description, bool isHidden);
	void            setNext(Smiley *ptr);
	SmileyPattern *	addPattern(const char *pattern);
	int        		match(const char *text, int minLen);
	int        		match(const wchar_t *wtext, int minLen);
public:
	~Smiley();
	const char *	getDescription();
	const char *	getFile();
	const char *	getPatternString();
	bool            isHidden();
	Smiley *		getNext();
};

class SmileyMap {
private:
	static SmileyMap * 	mapList;
	int                 smileyNum, visibleSmileyNum;
	char *				name;
	char *				filename;
	char *				author;
	char *				version;
	char *				description;
	SmileyMap *         next;
	Smiley *			entries;
	SmileyWindow *		window;
	SmileyMap(const char *name, const char *filename);
	void                clear();
	void                setFilename(const char *filename);
	void                setAuthor(const char *author);
	void                setDescription(const char *description);
	void                setVersion(const char *version);
	Smiley *			addSmiley(const char *file, const char *text, bool isHidden);
	static bool			loadSmileyFile(const char *proto, const char *filename, bool onlyInfo);
	static SmileyMap* 	add(const char *proto, const char *filename);
	static void         remove(const char *proto);
public:
	~SmileyMap();
	Smiley *            getSmiley(const char *text, int *len);
	Smiley *            getSmiley(const wchar_t *text, int *len);
	Smiley *            getSmiley();
	SmileyWindow *      getWindow();
	static SmileyMap *	getSmileyMap(const char *proto);
	static Smiley *		getSmiley(const char *proto, const char *text, int *len);
	static bool         loadLibrary(const char *proto, const char *filename);
	const char *        getFilename();
	const char *        getAuthor();
	const char *        getVersion();
	const char *        getDescription();
	int                 getSmileyNum();
	int                 getVisibleSmileyNum();
};

#endif
