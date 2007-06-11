class TabSRMMHTMLBuilder;

#ifndef TABSRMMHTMLBUILDER_INCLUDED
#define TABSRMMHTMLBUILDER_INCLUDED

#include "TemplateHTMLBuilder.h"

class TabSRMMHTMLBuilder:public TemplateHTMLBuilder
{
protected:
    virtual void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	char *timestampToString(DWORD dwFlags, time_t check, int isGroupBreak);
	time_t 		startedTime;
	time_t 		getStartedTime();
	virtual bool isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei);
	bool isDbEventShown(DBEVENTINFO * dbei);
	bool isRTL(IEVIEWEVENT *event);
	void appendEventNonTemplate(IEView *, IEVIEWEVENT *event);
public:
    TabSRMMHTMLBuilder();
	void buildHead(IEView *, IEVIEWEVENT *event);
	void appendEvent(IEView *, IEVIEWEVENT *event);

};

#endif
