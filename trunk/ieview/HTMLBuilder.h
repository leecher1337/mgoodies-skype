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
class HTMLBuilder;

#ifndef HTMLBUILDER_INCLUDED
#define HTMLBUILDER_INCLUDED

#include "IEView.h"

class TextToken {
private:
	int  type;
	wchar_t *wtext;
	char *text;
	wchar_t *wlink;
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
	TextToken(int type, const char *text, int len);
	TextToken(int type, const wchar_t *wtext, int len);
	~TextToken();
	int getType();
	const char *		getText();
	const wchar_t*      getTextW();
	const char *		getLink();
	const wchar_t *		getLinkW();
	void 				setLink(const char *link);
	void 				setLink(const wchar_t *wlink);
	TextToken *			getNext();
	void   				setNext(TextToken *);
	void				toString(char **str, int *sizeAlloced);
	void				toString(wchar_t **str, int *sizeAlloced);
	static char *		urlEncode(const char *str);
	static char *		urlEncode2(const char *str);
	static TextToken* 	tokenizeLinks(const char *text);
	static TextToken*	tokenizeSmileys(const char *proto, const char *text);
	// UNICODE
	wchar_t *			urlEncode(const wchar_t *str);
	static TextToken* 	tokenizeLinks(const wchar_t *wtext);
	static TextToken* 	tokenizeSmileys(const char *proto, const wchar_t *wtext);
};

class HTMLBuilder {
protected:
	virtual char *encode(const char *text, const char *proto, bool replaceSmiley);
	virtual wchar_t *encode(const wchar_t *text, const char *proto, bool replaceSmiley);
	virtual char *encodeUTF8(const wchar_t *text, const char *proto, bool replaceSmiley);
	virtual char *encodeUTF8(const char *text, const char *proto, bool replaceSmiley);
public:
	virtual void buildHead(IEView *, IEVIEWEVENT *event)=0;
	virtual void appendEvent(IEView *, IEVIEWEVENT *event)=0;
};

#endif
