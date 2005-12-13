#include <wchar.h>
#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <direct.h>		//For _chdir()
#include <commctrl.h>		//For hotkeys
#include "../../../SDK/headers_c/newpluginapi.h"	//CallService,UnHookEvent
#include "../../../SDK/headers_c/m_utils.h"			//window broadcasting
#include "../../../SDK/headers_c/m_system.h"
#include "../../../SDK/headers_c/m_skin.h"
#include "../../../SDK/headers_c/m_langpack.h"
#include "../../../SDK/headers_c/m_clist.h"
#include "../../../SDK/headers_c/m_options.h"
#include "../../../SDK/headers_c/m_database.h"		//database
#include "../../../SDK/headers_c/m_contacts.h"		//contact
#include "../../../SDK/headers_c/m_protocols.h"		//protocols
#include "../../../SDK/headers_c/m_protomod.h"		//protocols module
#include "../../../SDK/headers_c/m_protosvc.h"
#include "SDK/Import/m_uninstaller.h"				//PluginUninstaller structures
#include "SDK/Import/m_toptoolbar.h"
#include "SDK/Import/icolib.h"
#include "SDK/Import/m_kbdnotify.h"
#include "SDK/import/m_popup.h"
#include "m_account.h"	//Account structure and all needed structures to cooperate with YAMN
#include "m_messages.h"	//Messages sent to YAMN windows
#include "mails/m_mails.h"	//use YAMN's mails
#include "mails/m_decode.h"	//use decoding macros (needed for header extracting)
#include "browser/m_browser.h"	//we want to run YAMN mailbrowser, no new mail notification and bad connect window
#include "resources/resource.h"
#include "m_protoplugin.h"
#include "m_filterplugin.h"
#include "m_yamn.h"	//Main YAMN's variables
#include "m_protoplugin.h"	//Protocol registration and so on
#include "m_synchro.h"	//Synchronization
#include "debug.h"	

//From services.cpp
void CreateServiceFunctions(void);
void HookEvents(void);
void RefreshContact(void);
void ContactDoubleclicked(WPARAM wParam,LPARAM lParam);
int ClistContactDoubleclicked(WPARAM wParam, LPARAM lParam);

//From debug.cpp
#ifdef YAMN_DEBUG
void InitDebug();
void UnInitDebug();
#endif

//From synchro.cpp
struct CExportedFunctions SynchroExported[];

//From yamn.cpp
int GetFcnPtrSvc(WPARAM wParam,LPARAM lParam);
int GetVariablesSvc(WPARAM,LPARAM);
int AddWndToYAMNWindowsSvc(WPARAM,LPARAM);
int RemoveWndFromYAMNWindowsSvc(WPARAM,LPARAM);
DWORD WINAPI YAMNHotKeyThread(LPVOID);
void CALLBACK TimerProc(HWND,UINT,UINT,DWORD);
int ForceCheckSvc(WPARAM,LPARAM);
// int ExitProc(WPARAM,LPARAM);

//From account.cpp
struct CExportedFunctions AccountExported[];
int CreatePluginAccountSvc(WPARAM wParam,LPARAM lParam);
int DeletePluginAccountSvc(WPARAM wParam,LPARAM lParam);
int WriteAccountsToFileASvc(WPARAM wParam,LPARAM lParam);
int WriteAccountsToFileWSvc(WPARAM wParam,LPARAM lParam);
int AddAccountsFromFileASvc(WPARAM,LPARAM);
int AddAccountsFromFileWSvc(WPARAM,LPARAM);
int DeleteAccountSvc(WPARAM,LPARAM);
int FindAccountByNameSvc(WPARAM wParam,LPARAM lParam);
int GetNextFreeAccountSvc(WPARAM wParam,LPARAM lParam);

//From protoplugin.cpp
struct CExportedFunctions ProtoPluginExported[];
int UnregisterProtoPlugins();
int RegisterProtocolPluginSvc(WPARAM,LPARAM);
int UnregisterProtocolPluginSvc(WPARAM,LPARAM);
int GetFileNameWSvc(WPARAM,LPARAM);
int GetFileNameASvc(WPARAM,LPARAM);
int DeleteFileNameSvc(WPARAM,LPARAM);

//From filterplugin.cpp
struct CExportedFunctions FilterPluginExported[];
int UnregisterFilterPlugins();
int RegisterFilterPluginSvc(WPARAM,LPARAM);
int UnregisterFilterPluginSvc(WPARAM,LPARAM);
int FilterMailSvc(WPARAM,LPARAM);

//From mails.cpp (MIME)
struct CExportedFunctions MailExported[];
int CreateAccountMailSvc(WPARAM wParam,LPARAM lParam);
int DeleteAccountMailSvc(WPARAM wParam,LPARAM lParam);
int LoadMailDataSvc(WPARAM wParam,LPARAM lParam);
int UnloadMailDataSvc(WPARAM wParam,LPARAM);
int SaveMailDataSvc(WPARAM wParam,LPARAM lParam);

//From mime.cpp
//void WINAPI ExtractHeaderFcn(char *,int,WORD,HYAMNMAIL);	//already in MailExported

//From pop3comm.cpp
int RegisterPOP3Plugin(WPARAM,LPARAM);
int UninstallPOP3(PLUGINUNINSTALLPARAMS* ppup);			//to uninstall POP3 plugin with YAMN

//From mailbrowser.cpp
int RunMailBrowserSvc(WPARAM,LPARAM);

//From badconnect.cpp
int RunBadConnectionSvc(WPARAM,LPARAM);

//From YAMNopts.cpp
void WordToModAndVk(WORD,UINT *,UINT *);
int YAMNOptInitSvc(WPARAM,LPARAM);

//From main.cpp
int PostLoad(WPARAM,LPARAM);				//Executed after all plugins loaded YAMN reads mails from file and notify every protocol it should set its functions
int Shutdown(WPARAM,LPARAM);				//Executed before Miranda is going to shutdown
int AddTopToolbarIcon(WPARAM,LPARAM);		//Executed when TopToolBar plugin loaded Adds bitmap to toolbar
void LoadPlugins();							//Loads plugins located in MirandaDir/Plugins/YAMN/*.dll
int UninstallQuestionSvc(WPARAM,LPARAM);	//Ask information when user wants to uninstall plugin

//From synchro.cpp
extern DWORD WINAPI WaitToWriteFcn(PSWMRG SObject,PSCOUNTER SCounter=NULL);
extern void WINAPI WriteDoneFcn(PSWMRG SObject,PSCOUNTER SCounter=NULL);
extern DWORD WINAPI WaitToReadFcn(PSWMRG SObject);
extern void WINAPI ReadDoneFcn(PSWMRG SObject);
extern DWORD WINAPI SCIncFcn(PSCOUNTER SCounter);
extern DWORD WINAPI SCDecFcn(PSCOUNTER SCounter);
//From mails.cpp
extern void WINAPI DeleteMessageFromQueueFcn(HYAMNMAIL *From,HYAMNMAIL Which,int mode);
extern void WINAPI SetRemoveFlagsInQueueFcn(HYAMNMAIL From,DWORD FlagsSet,DWORD FlagsNotSet,DWORD FlagsToSet,int mode);
//From mime.cpp
void ExtractHeader(struct CMimeItem *items,int CP,struct CHeader *head);
void DeleteHeaderContent(struct CHeader *head);
//From account.cpp
void WINAPI GetStatusFcn(HACCOUNT Which,char *Value);
