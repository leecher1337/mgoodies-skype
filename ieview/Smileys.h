class SmileyMap;
class Smiley;
#ifndef SMILEYS_INCLUDED
#define SMILEYS_INCLUDED

#include "ieview_common.h"

class Smiley {
private:
    wchar_t *wtext;
	char *text;
	char *file;
	int   fLength, tLength;
	Smiley *next;
protected:
	friend class SmileyMap;
	bool        equals(const char *text);
	bool        equals(const wchar_t *wtext);
	Smiley *	getNext();
	Smiley(const char *file, const char *text);
public:
	~Smiley();
	const wchar_t *getTextW();
	const char *getText();
	const char *getFile();
	int         getTextLength();
	int         getFileLength();
};

class SmileyMap {
private:
	static SmileyMap * 	mapList;
	char *				name;
	char *				filename;
	SmileyMap *         next;
	Smiley *			entries;
	SmileyMap(const char *name, const char *filename);
	void                clear();
	void                setFilename(const char *filename);
	void				addSmiley(const char *file, const char *text);
	static bool			loadSmileyFile(const char *proto, const char *filename, bool onlyInfo);
	static SmileyMap* 	add(const char *proto, const char *filename);
	static void 		remove(const char *proto);
	static void 		removeAll();
public:
	~SmileyMap();
	Smiley *            getSmiley(const char *text);
	static SmileyMap *	getSmileyMap(const char *proto);
	static Smiley *		getSmiley(const char *proto, const char *text);
	static void         loadLibrary(const char *proto, const char *filename);
	const char *        getFilename();
};

#endif
