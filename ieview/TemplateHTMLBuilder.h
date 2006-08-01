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
	void buildHeadTemplate(IEView *, IEVIEWEVENT *event, ProtocolSettings* protoSettings);
	void appendEventTemplate(IEView *, IEVIEWEVENT *event, ProtocolSettings* protoSettings);
	virtual const char *getTemplateFilename(ProtocolSettings *);
	virtual int getFlags(ProtocolSettings *);
public:
    TemplateHTMLBuilder();
//	void buildHead(IEView *, IEVIEWEVENT *event);
};

#endif
