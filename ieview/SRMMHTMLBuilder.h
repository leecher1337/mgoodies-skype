class SRMMHTMLBuilder;

#ifndef SRMMHTMLBUILDER_INCLUDED
#define SRMMHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"

class SRMMHTMLBuilder:public HTMLBuilder
{
protected:
    virtual void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	virtual bool isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei);
	virtual char *timestampToString(DWORD dwFlags, time_t check);
public:
	void buildHead(IEView *);
	void appendEvent(IEView *, IEVIEWEVENT *event);
	void clear();
};

#endif
