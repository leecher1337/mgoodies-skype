class TemplateHTMLBuilder;

#ifndef TEMPLATEHTMLBUILDER_INCLUDED
#define TEMPLATEHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"

class TemplateHTMLBuilder:public HTMLBuilder
{
protected:
	char *timestampToString(DWORD dwFlags, time_t check, int mode);
	bool        isCleared;
	time_t 		startedTime;
	time_t 		getStartedTime();
	const char *groupTemplate;
	void buildHeadTemplate(IEView *, IEVIEWEVENT *event);
	void appendEventTemplate(IEView *, IEVIEWEVENT *event);
	void buildHeadHistoryTemplate(IEView *, IEVIEWEVENT *event);
	void appendEventHistoryTemplate(IEView *, IEVIEWEVENT *event);
	virtual const char *getTemplateFilename(ProtocolSettings *);
	virtual const char *getTemplateFilenameRtl(ProtocolSettings *);
public:
    TemplateHTMLBuilder();
	void buildHead(IEView *, IEVIEWEVENT *event);
};

#endif
