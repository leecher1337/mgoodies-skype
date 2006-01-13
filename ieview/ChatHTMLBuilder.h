class ChatHTMLBuilder;

#ifndef CHATTMLBUILDER_INCLUDED
#define CHATHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"

class ChatHTMLBuilder:public HTMLBuilder
{
protected:
    void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	char *timestampToString(DWORD dwFlags, time_t check);
	void appendEventNonTemplate(IEView *, IEVIEWEVENT *event);
public:
    ChatHTMLBuilder();
	void buildHead(IEView *, IEVIEWEVENT *event);
	void appendEvent(IEView *, IEVIEWEVENT *event);
};

#endif
