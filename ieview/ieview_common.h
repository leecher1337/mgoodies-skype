
#ifndef IEVIEW_COMMON_H
#define IEVIEW_COMMON_H
#include <windows.h>
#include <stdio.h>

#include <process.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_clist.h>
//#include <m_clui.h>
#include <m_options.h>
//#include <m_userinfo.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_message.h>
#include <m_contacts.h>
#include <richedit.h>
#include "m_smileyadd.h"

#include "m_ieview.h"

#include "IEView.h"

extern HINSTANCE hInstance;
extern IEView *debugView;
extern char *workingDir;
extern char *muccModuleName;

#endif
