class SRMM2HTMLBuilder;

#ifndef SRMM2HTMLBUILDER_INCLUDED
#define SRMM2HTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"

class SRMM2HTMLBuilder:public HTMLBuilder
{
protected:
    virtual void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	virtual bool isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei);
	virtual char *timestampToString(DWORD dwFlags, time_t check, int groupStart);
	time_t 		lastEventTime;
	int     	iLastEventType;
	int         getLastEventType();
	time_t 		getLastEventTime();
	void        setLastEventTime(time_t);
	void        setLastEventType(int);
public:
	void buildHead(IEView *);
	void appendEvent(IEView *, IEVIEWEVENT *event);
	void clear();
};

#endif
