/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-06  George Hazan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL$
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

*/

#include "jabber.h"
#include "jabber_ssl.h"
#include "jabber_iq.h"
#include "resource.h"
#include "version.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

PLUGININFO pluginInfo = {
	sizeof( PLUGININFO ),
	#if defined( _UNICODE )
		"Jabber Protocol (GMail) (Unicode)",
	#else
		"Jabber Protocol (GMail)",
	#endif
  	__VERSION_DWORD,
 	"Jabber protocol plugin (GMail mod) for Miranda IM ( "__DATE__" )",
 	"George Hazan, YB",
 	"yb@saaplugin.no-ip.info",
 	"( c ) 2002-05 Santithorn Bunchua, George Hazan, YB",
 	"http://forums.miranda-im.org/showthread.php?p=43865",
	0,
	0
};

MM_INTERFACE memoryManagerInterface;
LIST_INTERFACE_V2 li;

HANDLE hMainThread = NULL;
DWORD jabberMainThreadId;
char* jabberProtoName;	// "JABBER"
char* jabberModuleName;	// "Jabber"
CRITICAL_SECTION mutex;
HANDLE hNetlibUser;
// Main jabber server connection thread global variables
struct ThreadData *jabberThreadInfo = NULL;
BOOL   jabberConnected = FALSE;
time_t jabberLoggedInTime = 0;
BOOL   jabberOnline = FALSE;
BOOL   jabberChatDllPresent = FALSE;
int    jabberStatus = ID_STATUS_OFFLINE;
int    jabberDesiredStatus;
BOOL   modeMsgStatusChangePending = FALSE;
BOOL   jabberChangeStatusMessageOnly = FALSE;
TCHAR* jabberJID = NULL;
char*  streamId = NULL;
DWORD  jabberLocalIP;
UINT   jabberCodePage;
JABBER_MODEMSGS modeMsgs;
//char* jabberModeMsg;
CRITICAL_SECTION modeMsgMutex;
char* jabberVcardPhotoFileName = NULL;
char* jabberVcardPhotoType = NULL;
BOOL  jabberSendKeepAlive;

// SSL-related global variable
#ifndef STATICSSL
HMODULE hLibSSL = NULL;
#endif
PVOID jabberSslCtx;

const char xmlnsAdmin[] = "http://jabber.org/protocol/muc#admin";
const char xmlnsOwner[] = "http://jabber.org/protocol/muc#owner";

HWND hwndJabberAgents = NULL;
HWND hwndJabberGroupchat = NULL;
HWND hwndJabberJoinGroupchat = NULL;
HWND hwndAgentReg = NULL;
HWND hwndAgentRegInput = NULL;
HWND hwndAgentManualReg = NULL;
HWND hwndRegProgress = NULL;
HWND hwndJabberVcard = NULL;
HWND hwndMucVoiceList = NULL;
HWND hwndMucMemberList = NULL;
HWND hwndMucModeratorList = NULL;
HWND hwndMucBanList = NULL;
HWND hwndMucAdminList = NULL;
HWND hwndMucOwnerList = NULL;
HWND hwndJabberChangePassword = NULL;

// Service and event handles
HANDLE heventRawXMLIn;
HANDLE heventRawXMLOut;

int JabberOptInit( WPARAM wParam, LPARAM lParam );
int JabberUserInfoInit( WPARAM wParam, LPARAM lParam );
int JabberMsgUserTyping( WPARAM wParam, LPARAM lParam );
void JabberMenuInit( void );
int JabberSvcInit( void );
int JabberSvcUninit( void );

extern "C" BOOL WINAPI DllMain( HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved )
{
	#ifdef _DEBUG
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif
	hInst = hModule;
	return TRUE;
}

extern "C" __declspec( dllexport ) PLUGININFO *MirandaPluginInfo( DWORD mirandaVersion )
{
	if ( mirandaVersion < PLUGIN_MAKE_VERSION( 0,4,0,0 )) {
		MessageBoxA( NULL, "The Jabber protocol plugin cannot be loaded. It requires Miranda IM 0.4 or later.", "Jabber Protocol Plugin", MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
		return NULL;
	}

	return &pluginInfo;
}

///////////////////////////////////////////////////////////////////////////////
// OnPreShutdown - prepares Miranda to be shut down

static int OnPreShutdown( WPARAM wParam, LPARAM lParam )
{
	if ( hwndJabberAgents ) SendMessage( hwndJabberAgents, WM_CLOSE, 0, 0 );
	if ( hwndJabberGroupchat ) SendMessage( hwndJabberGroupchat, WM_CLOSE, 0, 0 );
	if ( hwndJabberJoinGroupchat ) SendMessage( hwndJabberJoinGroupchat, WM_CLOSE, 0, 0 );
	if ( hwndAgentReg ) SendMessage( hwndAgentReg, WM_CLOSE, 0, 0 );
	if ( hwndAgentRegInput ) SendMessage( hwndAgentRegInput, WM_CLOSE, 0, 0 );
	if ( hwndRegProgress ) SendMessage( hwndRegProgress, WM_CLOSE, 0, 0 );
	if ( hwndJabberVcard ) SendMessage( hwndJabberVcard, WM_CLOSE, 0, 0 );
	if ( hwndMucVoiceList ) SendMessage( hwndMucVoiceList, WM_CLOSE, 0, 0 );
	if ( hwndMucMemberList ) SendMessage( hwndMucMemberList, WM_CLOSE, 0, 0 );
	if ( hwndMucModeratorList ) SendMessage( hwndMucModeratorList, WM_CLOSE, 0, 0 );
	if ( hwndMucBanList ) SendMessage( hwndMucBanList, WM_CLOSE, 0, 0 );
	if ( hwndMucAdminList ) SendMessage( hwndMucAdminList, WM_CLOSE, 0, 0 );
	if ( hwndMucOwnerList ) SendMessage( hwndMucOwnerList, WM_CLOSE, 0, 0 );
	if ( hwndJabberChangePassword ) SendMessage( hwndJabberChangePassword, WM_CLOSE, 0, 0 );

	hwndJabberAgents = NULL;
	hwndJabberGroupchat = NULL;
	hwndJabberJoinGroupchat = NULL;
	hwndAgentReg = NULL;
	hwndAgentRegInput = NULL;
	hwndAgentManualReg = NULL;
	hwndRegProgress = NULL;
	hwndJabberVcard = NULL;
	hwndMucVoiceList = NULL;
	hwndMucMemberList = NULL;
	hwndMucModeratorList = NULL;
	hwndMucBanList = NULL;
	hwndMucAdminList = NULL;
	hwndMucOwnerList = NULL;
	hwndJabberChangePassword = NULL;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OnModulesLoaded - execute some code when all plugins are initialized

int JabberGcEventHook( WPARAM, LPARAM );
int JabberGcMenuHook( WPARAM, LPARAM );
int JabberGcInit( WPARAM, LPARAM );

static COLORREF crCols[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
HANDLE hChatEvent = NULL,
       hChatMenu = NULL,
		 hChatMess = NULL,
		 hInitChat = NULL,
		 hEvInitChat = NULL,
		 hEvModulesLoaded = NULL,
		 hEvOptInit = NULL,
		 hEvPreShutdown = NULL,
		 hEvUserInfoInit = NULL;

void JGmailSetupIcons();
void JGmailSetupIcoLib();
static int OnModulesLoaded( WPARAM wParam, LPARAM lParam )
{
	JabberWsInit();
	JabberSslInit();
	HookEvent( ME_USERINFO_INITIALISE, JabberUserInfoInit );
	if (JGetByte(NULL,"EnableGMail",1) & 1) {
		//Setup the sound
		extern char soundname[64];
		char sounddesc[64];
		mir_snprintf(soundname,64, "%s/NewMail",jabberProtoName);
		mir_snprintf(sounddesc,64, "%s: %s",jabberProtoName,"New Mail Notify");
		SkinAddNewSound( soundname, sounddesc, "newmail.wav" );
	}


	if ( ServiceExists( MS_GC_REGISTER )) {
		jabberChatDllPresent = true;

		GCREGISTER gcr = {0};
		gcr.cbSize = sizeof( GCREGISTER );
		gcr.dwFlags = GC_TYPNOTIF|GC_CHANMGR;
		gcr.iMaxText = 0;
		gcr.nColors = 16;
		gcr.pColors = &crCols[0];
		gcr.pszModuleDispName = jabberProtoName;
		gcr.pszModule = jabberProtoName;
		JCallService( MS_GC_REGISTER, NULL, ( LPARAM )&gcr );

		hChatEvent = HookEvent( ME_GC_EVENT, JabberGcEventHook );
		hChatMenu = HookEvent( ME_GC_BUILDMENU, JabberGcMenuHook );

		char szEvent[ 200 ];
		mir_snprintf( szEvent, sizeof szEvent, "%s\\ChatInit", jabberProtoName );
		hInitChat = CreateHookableEvent( szEvent );
		hEvInitChat = HookEvent( szEvent, JabberGcInit );
	}
	JGmailSetupIcoLib();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OnLoad - initialize the plugin instance

extern "C" int __declspec( dllexport ) Load( PLUGINLINK *link )
{
	pluginLink = link;

	// set the memory manager
	memoryManagerInterface.cbSize = sizeof(MM_INTERFACE);
	JCallService(MS_SYSTEM_GET_MMI,0,(LPARAM)&memoryManagerInterface);

	// set the lists manager;
	li.cbSize = sizeof( li ); 
	li.List_InsertPtr = NULL; li.List_RemovePtr = NULL; // Just in case
	if ( CallService(MS_SYSTEM_GET_LI,0,(LPARAM)&li) == CALLSERVICE_NOTFOUND ) {
		MessageBoxA( NULL, "This version of plugin requires Miranda 0.4.3 bld#42 or later", "Fatal error", MB_OK );
		return 1;
	}
	if (!li.List_InsertPtr) li.List_InsertPtr = JList_InsertPtr;
	if (!li.List_RemovePtr) li.List_RemovePtr = JList_RemovePtr;

	if ( !ServiceExists( MS_DB_CONTACT_GETSETTING_STR )) {
		MessageBoxA( NULL, "This plugin requires db3x plugin version 0.5.1.0 or later", "Jabber", MB_OK );
		return 1;
	}

	char text[_MAX_PATH];
	char* p, *q;

	GetModuleFileNameA( hInst, text, sizeof( text ));
	p = strrchr( text, '\\' );
	p++;
	q = strrchr( p, '.' );
	*q = '\0';
	jabberProtoName = mir_strdup( p );
	_strupr( jabberProtoName );

	mir_snprintf( text, sizeof( text ), "%s/Status", jabberProtoName );
	JCallService( MS_DB_SETSETTINGRESIDENT, TRUE, ( LPARAM )text );

	jabberModuleName = mir_strdup( jabberProtoName );
	_strlwr( jabberModuleName );
	jabberModuleName[0] = toupper( jabberModuleName[0] );

	JabberLog( "Setting protocol/module name to '%s/%s'", jabberProtoName, jabberModuleName );

	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, FALSE, 0 );
	jabberMainThreadId = GetCurrentThreadId();

	hEvOptInit       = HookEvent( ME_OPT_INITIALISE, JabberOptInit );
	hEvModulesLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, OnModulesLoaded );
	hEvPreShutdown   = HookEvent( ME_SYSTEM_PRESHUTDOWN, OnPreShutdown );

	//Load the icons. Later will change the menus if IcoLib is available
	JGmailSetupIcons();
	// Register protocol module
	PROTOCOLDESCRIPTOR pd;
	ZeroMemory( &pd, sizeof( PROTOCOLDESCRIPTOR ));
	pd.cbSize = sizeof( PROTOCOLDESCRIPTOR );
	pd.szName = jabberProtoName;
	pd.type = PROTOTYPE_PROTOCOL;
	JCallService( MS_PROTO_REGISTERMODULE, 0, ( LPARAM )&pd );

	// Set all contacts to offline
	HANDLE hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL ) {
		char* szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
		if ( szProto != NULL && !strcmp( szProto, jabberProtoName ))
			if ( JGetWord( hContact, "Status", ID_STATUS_OFFLINE ) != ID_STATUS_OFFLINE )
				JSetWord( hContact, "Status", ID_STATUS_OFFLINE );

		hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM ) hContact, 0 );
	}

	memset(( char* )&modeMsgs, 0, sizeof( JABBER_MODEMSGS ));
	//jabberModeMsg = NULL;
	jabberCodePage = JGetWord( NULL, "CodePage", CP_ACP );

	InitializeCriticalSection( &mutex );
	InitializeCriticalSection( &modeMsgMutex );

	srand(( unsigned ) time( NULL ));
	JabberSerialInit();
	JabberIqInit();
	JabberListInit();
	JabberSvcInit();
	JabberMenuInit();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Unload - destroy the plugin instance

extern "C" int __declspec( dllexport ) Unload( void )
{
	if ( hChatEvent  )      UnhookEvent( hChatEvent );
	if ( hChatMenu   )      UnhookEvent( hChatMenu );
	if ( hChatMess   )      UnhookEvent( hChatMess );
	if ( hEvInitChat )      UnhookEvent( hEvInitChat );
	if ( hEvModulesLoaded ) UnhookEvent( hEvModulesLoaded );
	if ( hEvOptInit  )      UnhookEvent( hEvOptInit );
	if ( hEvPreShutdown )   UnhookEvent( hEvPreShutdown );
	if ( hEvUserInfoInit )  UnhookEvent( hEvUserInfoInit );

	if ( hInitChat )
		DestroyHookableEvent( hInitChat );

	JabberSvcUninit();
	JabberSslUninit();
	JabberListUninit();
	JabberIqUninit();
	JabberSerialUninit();
	JabberWsUninit();
	DeleteCriticalSection( &modeMsgMutex );
	DeleteCriticalSection( &mutex );
	mir_free( modeMsgs.szOnline );
	mir_free( modeMsgs.szAway );
	mir_free( modeMsgs.szNa );
	mir_free( modeMsgs.szDnd );
	mir_free( modeMsgs.szFreechat );
	mir_free( jabberModuleName );
	mir_free( jabberProtoName );
	if ( jabberVcardPhotoFileName ) {
		DeleteFileA( jabberVcardPhotoFileName );
		mir_free( jabberVcardPhotoFileName );
	}
	if ( jabberVcardPhotoType ) mir_free( jabberVcardPhotoType );
	if ( streamId ) mir_free( streamId );

	if ( hMainThread ) CloseHandle( hMainThread );
	return 0;
}
