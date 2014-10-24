#define __MAJOR_VERSION            0
#define __MINOR_VERSION            0
#define __RELEASE_NUM              0
#define __BUILD_NUM               56

#include <m_core.h>
#include <stdver.h>

#ifdef IS_MIRANDAIM
#define __PLUGIN_NAME              "Skype Protocol"
#define __INTERNAL_NAME            "Skype"
#define __FILENAME                 "Skype.dll"
#define __DESCRIPTION              "Skype protocol support for Miranda IM."
#define __AUTHORWEB                "https://code.google.com/p/mgoodies-skype/"
#else
#define __PLUGIN_NAME              "Skype protocol (Classic)"
#define __INTERNAL_NAME            "SkypeClassic"
#define __FILENAME                 "SkypeClassic.dll"
#define __DESCRIPTION              "Skype protocol support for Miranda NG. Classic implementation which requires running original Skype client."
#define __AUTHORWEB                "http://miranda-ng.org/p/SkypeClassic/"
#endif
#define __AUTHOR                   "leecher, tweety, jls17"
#define __AUTHOREMAIL              "leecher@dose.0wnz.at; tweety@user.berlios.de"
#define __COPYRIGHT                "© 2004-2014 leecher, tweety"
