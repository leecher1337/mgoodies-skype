#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>

#include <m_system.h>
#include <m_skin.h>
#include <m_utils.h>
#include <m_options.h>
#include <m_userinfo.h>
#include <m_clist.h>
#include <m_userinfo.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_protosvc.h>
#include <m_protocols.h>



//#pragma optimize("gsy",on)

#define S_MOD "SeenModule"

//#define UM_CHECKHOOKS (WM_USER+1)

#define debug(a) MessageBox(NULL,a,"Debug",MB_OK)

#define IDI_USERDETAILS                 160

#define ICON_OFFLINE		13
#define ICON_ONLINE			14
#define ICON_AWAY			15
#define ICON_NA				16
#define ICON_OCC			17
#define ICON_DND			18
#define ICON_FREE			19
#define	ICON_INVIS			20

#define DEFAULT_MENUSTAMP          "%d.%m.%Y - %H:%M [%s]"
#define DEFAULT_USERSTAMP          "Date: %d.%m.%Y%b%bTime: %H:%M:%S%b%bStatus: %s"
#define DEFAULT_FILESTAMP          "%d.%m.%Y %H:%M:%S%t%n%t%s%t%u%t%r | %i%t%N"
#define DEFAULT_FILENAME           "logs\\%P.txt"
#define DEFAULT_HISTORYSTAMP       "%d.%m.%Y - %H:%M [%s]"
#define DEFAULT_WATCHEDPROTOCOLS   ""

#define VARIABLE_LIST "%s       \n%%Y: \t %s                    \n%%y: \t %s                    \n%%m: \t %s                    \n%%E: \t %s                    \n%%e: \t %s                    \n%%d: \t %s                    \n%%W: \t %s                    \n%%w: \t %s                    \n\n%s                          \n%%H: \t %s                    \n%%h: \t %s                    \n%%p: \t %s                    \n%%M: \t %s                    \n%%S: \t %s                    \n\n%s                          \n%%n: \t %s                    \n%%N: \t %s                    \n%%u: \t %s                    \n%%s: \t %s                    \n%%i: \t %s                    \n%%r: \t %s                    \n%%P: \t %s                    \n\n%s                          \n%%t: \t %s                    \n%%b: \t %s",                Translate("-- Date --"),      Translate("year (4 digits)"),     Translate("year (2 digits)"),       Translate("month"),              Translate("name of month"),         Translate("short name of month"),   Translate("day"),                Translate("weekday (full)"),        Translate("weekday (abbreviated)"), Translate("-- Time --"),         Translate("hours (24)"),            Translate("hours (12)"),          Translate("AM/PM"),                Translate("minutes"),             Translate("seconds"),             Translate("-- User --"),            Translate("username"),            Translate("nick"),             Translate("UIN/handle"),            Translate("status"),               Translate("external IP"),           Translate("internal IP"),Translate("Protocol"),           Translate("-- Format --"),          Translate("tabulator"),           Translate("line break")

typedef struct{
	int count;
	WPARAM wpcontact[1024];
	BYTE times[1024];
} MISSEDCONTACTS;

/* utils.c */
int IsWatchedProtocol(const char* szProto);
char *ParseString(char *,HANDLE,BYTE);


