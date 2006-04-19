// System includes
#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <time.h>
#include "resource.h"

// Miranda Includes
#include "../headers_c/newpluginapi.h"
#include "../headers_c/m_protosvc.h"
#include "../headers_c/m_protomod.h"
#include "../headers_c/m_skin.h"
#include "../headers_c/m_message.h"
#include "../headers_c/m_database.h"
#include "../headers_c/m_clist.h"
#include "../headers_c/m_system.h"

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
#define EVENTTYPE_CALL 2000

// Common used code-pieces
#define OUTPUT(a) ShowMessage(IDI_ICON1, a, 1);
#define ERRCHK 	if (!strncmp(ptr, "ERROR", 5)) { OUTPUT(ptr); free(ptr); SetEvent(SkypeMsgFetched); return; }

// Prototypes

void SkypeSystemInit(char *);
int HookContactAdded(WPARAM wParam, LPARAM lParam);
void PingPong(void);
void SkypeSystemInit(char *);
void CheckIfApiIsResponding(char *);
void TellError(DWORD err);
int ShowMessage(int, char*, int);
void EndCallThread(char *);
DWORD WINAPI ThreadFunc(VOID);
void GetInfoThread(HANDLE);

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
