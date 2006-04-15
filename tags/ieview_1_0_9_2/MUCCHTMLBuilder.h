class MUCCHTMLBuilder;

#ifndef MUCCHTMLBUILDER_INCLUDED
#define MUCCHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"

class MUCCHTMLBuilder:public HTMLBuilder
{
protected:
    void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	char *timestampToString(DWORD dwFlags, time_t check);
	void appendEventNonTemplate(IEView *, IEVIEWEVENT *event);
	bool isDbEventShown(DBEVENTINFO * dbei);
public:
    MUCCHTMLBuilder();
	void buildHead(IEView *, IEVIEWEVENT *event);
	void appendEvent(IEView *, IEVIEWEVENT *event);
};

#endif
