class TemplateMap;
class Template;
#ifndef TEMPLATE_INCLUDED
#define TEMPLATE_INCLUDED

#include "ieview_common.h"

struct TokenDef {
	const char *tokenString;
	int 		token;
	int         tokenLen;
	int         escape;
};

class Token {
private:
	int  escape;
	int  type;
	char *text;
	Token *next;
public:
	enum TOKENS {
		END      = 0,
		BASE,
		PLAIN,
		TEXT,
		NAME,
		TIME,
		DATE,
		AVATAR,
		CID,
		PROTO,
		AVATARIN,
		AVATAROUT,
		NAMEIN,
		NAMEOUT,
		UIN,
		UININ,
		UINOUT,
		STATUSMSG,
		NICKIN,
	};
	Token(int, const char *, int );
	~Token();
	int getType();
	int getEscape();
	const char *getText();
	Token *getNext();
	void   setNext(Token *);
};

class Template {
private:
	char *name;
	char *text;
	Template *next;
	Token *tokens;
protected:
	friend class TemplateMap;
	bool        equals(const char *name);
	void        tokenize();
	Template *	getNext();
	Template(const char *name, const char *text);
public:
	~Template();
	const char *getText();
	const char *getName();
	Token *getTokens();
};

class TemplateMap {
private:
	static TemplateMap *mapList;
	char *				name;
	char *				filename;
	bool    			grouping;
	Template *			entries;
	TemplateMap *       next;
	TemplateMap(const char *name);
	void				addTemplate(const char *name, const char *text);
	void				setFilename(const char *filename);
	void                clear();
	static TemplateMap*	add(const char *proto, const char *filename);
	static void 		appendText(char **str, int *sizeAlloced, const char *fmt, ...);
	static bool 		loadTemplateFile(const char *proto, const char *filename, bool onlyInfo);
public:
	static Template *	getTemplate(const char *proto, const char *name);
	static TemplateMap *getTemplateMap(const char *proto);
	static void         loadTemplates(const char *proto, const char *filename);
	Template *          getTemplate(const char *text);
	const char *		getFilename();
	bool        		isGrouping();
};


#endif
