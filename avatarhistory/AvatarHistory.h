#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>


extern "C"
{
#include <newpluginapi.h>
#include <m_folders.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_avatars.h>
#include <m_database.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_contacts.h>
#include <m_popup.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_metacontacts.h>
#include <m_history.h>
#include <m_updater.h>

// Globals
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern HANDLE hMenu;

}

#include "resource.h"
#include "m_avatarhist.h"

#include "../utils/mir_memory.h"

#define MODULE_NAME "AvatarHistory"

#define AVH_DEF_POPUPFG 0
#define AVH_DEF_POPUPBG 0x2DB6FF
#define AVH_DEF_AVPOPUPS 0
#define AVH_DEF_LOGTODISK 1
#define AVH_DEF_LOGTOHISTORY 0
#define AVH_DEF_DEFPOPUPS 0
#define AVH_DEF_SHOWMENU 1

#define DEFAULT_TEMPLATE_REMOVED "removed his/her avatar"
#define DEFAULT_TEMPLATE_CHANGED "changed his/her avatar"

TCHAR * MyDBGetStringT(HANDLE hContact, char* module, char* setting, TCHAR* out, size_t len);
char * MyDBGetString(HANDLE hContact, char* module, char* setting, char * out, size_t len);
void LoadOptions();

 // from icolib.cpp
#define NUMICONS 2
extern HICON iconList[NUMICONS];
extern HICON overlayedIcon; //will be inited in SetupIcoLib()
extern HICON overlayedBigIcon; // - || -
void SetupIcoLib();
HICON getOverlayedIcon(HICON icon, HICON overlay, BOOL big); // from icolib.cpp

#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )

#define POPUP_ACTION_DONOTHING 0
#define POPUP_ACTION_CLOSEPOPUP 1
#define POPUP_ACTION_OPENAVATARHISTORY 2
#define POPUP_ACTION_OPENHISTORY 3

#define POPUP_DELAY_DEFAULT 0
#define POPUP_DELAY_CUSTOM 1
#define POPUP_DELAY_PERMANENT 2


struct Options {
	// Templates
	TCHAR template_changed[1024];
	TCHAR template_removed[1024];

	// Track
	BYTE track_removes;

	// Popup
	WORD popup_delay_type;
	WORD popup_timeout;
	BYTE popup_use_win_colors;
	BYTE popup_use_default_colors;
	COLORREF popup_bkg_color;
	COLORREF popup_text_color;
	WORD popup_left_click_action;
	WORD popup_right_click_action;
};

extern Options opts;


#include "popup.h"


#ifdef UNICODE

#define TCHAR_TO_CHAR(dest, orig)	mir_snprintf(dest, MAX_REGS(dest), "%S", orig)
#define CHAR_TO_TCHAR(dest, orig)	mir_sntprintf(dest, MAX_REGS(dest), "%S", orig)

#else

#define TCHAR_TO_CHAR(dest, orig)	lstrcpynA(dest, orig, MAX_REGS(dest))
#define CHAR_TO_TCHAR(dest, orig)	lstrcpynA(dest, orig, MAX_REGS(dest))

#endif


int PathToAbsolute(char *pSrc, char *pOut);
