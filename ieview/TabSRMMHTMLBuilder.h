class TabSRMMHTMLBuilder;

#ifndef TABSRMMHTMLBUILDER_INCLUDED
#define TABSRMMHTMLBUILDER_INCLUDED

#include "SRMMHTMLBuilder.h"

class TabSRMMHTMLBuilder:public SRMMHTMLBuilder
{
protected:
    virtual void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	virtual bool isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei);
	virtual char *timestampToString(DWORD dwFlags, time_t check, int isGroupBreak);
	time_t 		startedTime;
	time_t 		lastEventTime;
	int     	iLastEventType;
	time_t 		getStartedTime();
	int         getLastEventType();
	time_t 		getLastEventTime();
	void        setLastEventTime(time_t);
	void        setLastEventType(int);
public:
    TabSRMMHTMLBuilder();
	void buildHead(IEView *);
	void appendEvent(IEView *, IEVIEWEVENT *event);
};

#endif
