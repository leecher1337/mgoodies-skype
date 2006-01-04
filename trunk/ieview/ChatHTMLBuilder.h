class ChatHTMLBuilder;

#ifndef CHATTMLBUILDER_INCLUDED
#define CHATHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"

class ChatHTMLBuilder:public HTMLBuilder
{
protected:
    virtual void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	virtual char *timestampToString(DWORD dwFlags, time_t check);
public:
    ChatHTMLBuilder();
	void buildHead(IEView *, IEVIEWEVENT *event);
	void appendEvent(IEView *, IEVIEWEVENT *event);
	void appendEventMem(IEView *, IEVIEWEVENT *event);
};

#endif
