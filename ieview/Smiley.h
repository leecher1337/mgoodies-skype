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
	SmileyMap *         next;
	Smiley *			entries;
	SmileyWindow *		window;
	SmileyMap(const char *name, const char *filename);
	void                clear();
	void                setFilename(const char *filename);
	Smiley *			addSmiley(const char *file, const char *text, bool isHidden);
	static bool			loadSmileyFile(const char *proto, const char *filename, bool onlyInfo);
	static SmileyMap* 	add(const char *proto, const char *filename);
public:
	~SmileyMap();
	Smiley *            getSmiley(const char *text, int *len);
	Smiley *            getSmiley();
	SmileyWindow *      getWindow();
	static SmileyMap *	getSmileyMap(const char *proto);
	static Smiley *		getSmiley(const char *proto, const char *text, int *len);
	static bool         loadLibrary(const char *proto, const char *filename);
	const char *        getFilename();
	int                 getSmileyNum();
	int                 getVisibleSmileyNum();
};

#endif
