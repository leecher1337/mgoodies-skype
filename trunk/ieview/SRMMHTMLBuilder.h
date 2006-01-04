class SRMMHTMLBuilder;

#ifndef SRMMHTMLBUILDER_INCLUDED
#define SRMMHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"

class SRMMHTMLBuilder:public HTMLBuilder
{
protected:
    virtual void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	virtual char *timestampToString(DWORD dwFlags, time_t check);
	bool isDbEventShown(DBEVENTINFO * dbei);
public:
	void buildHead(IEView *, IEVIEWEVENT *event);
	void appendEvent(IEView *, IEVIEWEVENT *event);
	void appendEventMem(IEView *, IEVIEWEVENT *event);
};

#endif
