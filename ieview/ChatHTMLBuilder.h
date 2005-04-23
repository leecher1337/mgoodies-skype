class ChatHTMLBuilder;

#ifndef CHATTMLBUILDER_INCLUDED
#define CHATHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"

class ChatHTMLBuilder:public HTMLBuilder
{
protected:
    virtual void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	virtual char *timestampToString(DWORD dwFlags, time_t check);
	time_t 		lastEventTime;
	int     	iLastEventType;
	int         getLastEventType();
	time_t 		getLastEventTime();
	void        setLastEventTime(time_t);
	void        setLastEventType(int);
public:
    ChatHTMLBuilder();
	void buildHead(IEView *, IEVIEWEVENT *event);
	void appendEvent(IEView *, IEVIEWEVENT *event);
	void clear();
};

#endif
