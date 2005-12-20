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
	time_t 		getStartedTime();
public:
    TabSRMMHTMLBuilder();
	void buildHead(IEView *, IEVIEWEVENT *event);
	void appendEvent(IEView *, IEVIEWEVENT *event);
};

#endif
