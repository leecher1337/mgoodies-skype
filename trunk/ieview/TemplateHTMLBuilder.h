class TemplateHTMLBuilder;

#ifndef TEMPLATEHTMLBUILDER_INCLUDED
#define TEMPLATEHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"
#include "Template.h"

class TemplateHTMLBuilder:public HTMLBuilder
{
protected:
		char *timestampToString(DWORD dwFlags, time_t check, int mode);
		time_t 		startedTime;
		time_t 		getStartedTime();
		const char *groupTemplate;
		time_t 		flashAvatarsTime[2];
		char *		flashAvatars[2];
		const char *getFlashAvatar(const char *file, int index);
		char *getAvatar(HANDLE hContact, const char *szProto);
		void buildHeadTemplate(IEView *, IEVIEWEVENT *event, ProtocolSettings* protoSettings);
		void appendEventTemplate(IEView *, IEVIEWEVENT *event, ProtocolSettings* protoSettings);
		virtual TemplateMap *getTemplateMap(ProtocolSettings *);
		virtual int getFlags(ProtocolSettings *);
public:
		TemplateHTMLBuilder();
		virtual ~TemplateHTMLBuilder();
//	void buildHead(IEView *, IEVIEWEVENT *event);
};

#endif
