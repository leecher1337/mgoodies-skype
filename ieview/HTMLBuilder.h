class HTMLBuilder;

#ifndef HTMLBUILDER_INCLUDED
#define HTMLBUILDER_INCLUDED

#include "IEView.h"

class TextToken {
private:
	int  type;
	wchar_t *wtext;
	char *text;
	char *link;
	TextToken *next;
public:
	enum TOKENS {
		END      = 0,
		TEXT,
		LINK,
		WWWLINK,
		SMILEY,
	};
	TextToken(int, const char *, int);
	~TextToken();
	int getType();
	const char *		getText();
	const wchar_t*      getTextW();
	const char *		getLink();
	void 				setLink(const char *link);
	TextToken *			getNext();
	void   				setNext(TextToken *);
	void				toString(char **str, int *sizeAlloced);
	void				toStringW(wchar_t **str, int *sizeAlloced);
	static char *		urlEncode(const char *str);
	static char *		urlEncode2(const char *str);
	static TextToken* 	tokenizeLinks(const char *text);
	static TextToken*	tokenizeSmileys(const char *proto, const char *text);
};

class HTMLBuilder {
protected:
	virtual char *encode(const char *text, const char *proto, bool replaceSmiley);
//	virtual wchar_t *encode(const wchar_t *text, const char *proto, bool replaceSmiley);
public:
	virtual void buildHead(IEView *)=0;
	virtual void appendEvent(IEView *, IEVIEWEVENT *event)=0;
};

#endif
