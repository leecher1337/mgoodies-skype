#define _CRT_SECURE_NO_DEPRECATE 1

// System includes
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <time.h>
#include "resource.h"

// Miranda Includes
#include "../../include/newpluginapi.h"
#include "../../include/m_utils.h"
#include "../../include/m_protosvc.h"
#include "../../include/m_protomod.h"
#include "../../include/m_skin.h"
#include "../../include/m_message.h"
#include "../../include/m_database.h"
#include "../../include/m_clist.h"
#include "../../include/m_system.h"
#include "../../include/m_updater.h"
#include "../../include/m_folders.h"
#include "../../include/m_options.h"
#include "../../include/m_langpack.h"
#include "../../include/m_userinfo.h"
#include "../../include/m_avatars.h"
#include "../../include/m_contacts.h"

// MyDetails defines

// wParam=NULL
// lParam=(char *) new nickname - do not free
// return=0 for sucess
#define PS_SETMYNICKNAME "/SetNickname"

// Optional, default value is 1024
// wParam=NULL
// lParam=NULL
// return= <=0 for error, >0 the max length of the nick
#define PS_GETMYNICKNAMEMAXLENGTH "/GetMyNicknameMaxLength"

// wParam=(char *)Buffer to file name
// lParam=(int)Buffer size
// return=0 for sucess
#define PS_GETMYAVATAR "/GetMyAvatar"

// wParam=0
// lParam=(const char *)Avatar file name
// return=0 for sucess
#define PS_SETMYAVATAR "/SetMyAvatar"


// Program defines
#define SKYPE_NAME		"Username"
#define SKYPE_PROTO		"PROTOCOL 5"
#define MAX_MSGS		128		// Maximum messages in queue
#define MAX_USERLEN     32      // Maximum length of a username in Skype
#define PING_INTERVAL	10000	// Ping every 10000 msec to see if Skype is still available
#define USEPOPUP		1		// Use the popup-plugin?
#define TIMEOUT_MSGSEND 9000	// Stolen from msgdialog.c
#define MAX_MSG_AGE		30		// Maximum age in seconds before a Message from queue gets trashed
#define SKYPEBUG_OFFLN	1		// Activate fix for the SkypeAPI Offline-Bug

// Program hooks

#define SKYPE_CALL "Skype_protocol/CallUser"
#define SKYPEOUT_CALL "Skype_protocol/SkypeOutCallUser"
#define SKYPE_ADDUSER "Skype_Protocol/AddUser"
#define SKYPE_IMPORTHISTORY "Skype_Protocol/ImportHistory"
#define SKYPE_ANSWERCALL "Skype_protocol/AnswerCall"
#define SKYPE_HOLDCALL "Skype_protocol/HoldCall"
#define SKYPE_SENDFILE "Skype_protocol/SendFile"
#define SKYPE_SETAVATAR "Skype_protocol/SetAvatar"
#define SKYPE_CHATNEW "Skype_protocol/ChatNew"
#define EVENTTYPE_CALL 2000

// Common used code-pieces
#define OUTPUT(a) ShowMessage(IDI_ERRORS, a, 1);
#define ERRCHK 	if (!strncmp(ptr, "ERROR", 5)) { OUTPUT(ptr); free(ptr); SetEvent(SkypeMsgFetched); LOG("FetchMessageThread", "terminated."); return; }

typedef void ( __cdecl* pThreadFunc )( void* );

// Prototypes

void __cdecl SkypeSystemInit(char *);
void __cdecl MsgPump (char *dummy);
int HookContactAdded(WPARAM wParam, LPARAM lParam);
void PingPong(void);
void CheckIfApiIsResponding(char *);
void TellError(DWORD err);
int ShowMessage(int, char*, int);
void EndCallThread(char *);
void GetInfoThread(HANDLE);
int OnDetailsInit( WPARAM, LPARAM );
int SkypeGetAvatarInfo(WPARAM wParam,LPARAM lParam);
int SkypeGetAwayMessage(WPARAM wParam,LPARAM lParam);
int HookContactAdded(WPARAM wParam, LPARAM lParam);
int HookContactDeleted(WPARAM wParam, LPARAM lParam);
int ImportHistory(WPARAM wParam, LPARAM lParam);
int CreateTopToolbarButton(WPARAM wParam, LPARAM lParam);
int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
int SkypeSetStatus(WPARAM wParam, LPARAM lParam);
int SkypeGetStatus(WPARAM wParam, LPARAM lParam);
int SkypeGetInfo(WPARAM wParam,LPARAM lParam);
int SkypeAddToList(WPARAM wParam, LPARAM lParam);
int SkypeBasicSearch(WPARAM wParam, LPARAM lParam);
int SkypeSendMessage(WPARAM wParam, LPARAM lParam);
int SkypeRecvMessage(WPARAM wParam, LPARAM lParam);
int SkypeSendAuthRequest(WPARAM wParam, LPARAM lParam);
int SkypeRecvAuth(WPARAM wParam, LPARAM lParam);
int SkypeAuthAllow(WPARAM wParam, LPARAM lParam);
int SkypeAuthDeny(WPARAM wParam, LPARAM lParam);
int SkypeAddToListByEvent(WPARAM wParam, LPARAM lParam);
int OkToExit(WPARAM wParam, LPARAM lParam);
int MirandaExit(WPARAM wParam, LPARAM lParam);
int __stdcall EnterBitmapFileName( char* szDest );
void CleanupNicknames(char *dummy);
int InitVSApi();
int FreeVSApi();

// Structs

typedef struct {
	char *SkypeSetting;
	char *MirandaSetting;
} settings_map;

typedef struct {
	char *msgnum;
	BOOL getstatus;
} fetchmsg_arg;

// Optional includes
#ifdef USEPOPUP
  #include "m_popup.h"
#endif
