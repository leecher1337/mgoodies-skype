class SRMMHTMLBuilder;

#ifndef TEMPLATEHTMLBUILDER_INCLUDED
#define TEMPLATEHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"

class TemplateHTMLBuilder:public HTMLBuilder
{
protected:
	virtual bool isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei);
	virtual char *timestampToString(time_t check, int mode);
	bool        isCleared;
	time_t 		startedTime;
	time_t 		getStartedTime();
	const char *groupTemplate;
public:
    TemplateHTMLBuilder();
	void buildHead(IEView *, IEVIEWEVENT *event);
	void appendEvent(IEView *, IEVIEWEVENT *event);
};

#endif
