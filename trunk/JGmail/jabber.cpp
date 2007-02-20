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
#include <m_icolib.h>
#include "../updater/Docs/m_updater.h"
//updater defines
#ifndef STATICSSL
  #ifdef _UNICODE
    #define BETAURL "http://saaplugin.no-ip.info/jabber/u/JGmail.dll"
    #define FLNAME "Jabber - GMail Unicode"
  #else
    #define BETAURL "http://saaplugin.no-ip.info/jabber/JGmail.dll"
    #define FLNAME "Jabber - GMail ANSI"
  #endif
#else 
  #ifdef _UNICODE
    #define BETAURL "http://saaplugin.no-ip.info/jabber/staticssl/u/JGmail.dll"
  #else
    #define BETAURL "http://saaplugin.no-ip.info/jabber/staticssl/JGmail.dll"
  #endif
#endif  

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
 	"( c ) 2002-07 Santithorn Bunchua, George Hazan, YB",
 	"http://forums.miranda-im.org/showthread.php?p=43865",
 	UNICODE_AWARE,
	0
};

MM_INTERFACE    mmi;
LIST_INTERFACE  li;
UTF8_INTERFACE  utfi;
MD5_INTERFACE   md5i;
SHA1_INTERFACE  sha1i;

HANDLE hMainThread = NULL;
DWORD jabberMainThreadId;
char* jabberProtoName;	// "JABBER"
char* jabberModuleName;	// "Jabber"
HANDLE hNetlibUser;
// Main jabber server connection thread global variables
ThreadData* jabberThreadInfo = NULL;
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
int    jabberSearchID;
JABBER_MODEMSGS modeMsgs;
CRITICAL_SECTION modeMsgMutex;
#ifdef _UNICODE
LISTENINGTOINFO listeningToInfo;
CRITICAL_SECTION listeningToInfoMutex;
#endif
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

static int compareTransports( const TCHAR* p1, const TCHAR* p2 )
{	return _tcsicmp( p1, p2 );
}

LIST<TCHAR> jabberTransports( 50, compareTransports );

int JabberOptInit( WPARAM wParam, LPARAM lParam );
int JabberUserInfoInit( WPARAM wParam, LPARAM lParam );
int JabberMsgUserTyping( WPARAM wParam, LPARAM lParam );
void JabberMenuInit( void );
int JabberSvcInit( void );
int JabberSvcUninit( void );

int bSecureIM;
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
		 hReloadIcons = NULL,
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
	HookEvent( ME_USERINFO_INITIALISE, JabberUserInfoInit );
	if (JGetByte(NULL,"EnableGMail",1) & 1) {
		//Setup the sound
		extern char soundname[64];
		char sounddesc[64];
		mir_snprintf(soundname,64, "%s/NewMail",jabberProtoName);
		mir_snprintf(sounddesc,64, "%s: %s",jabberProtoName,"New Mail Notify");
		SkinAddNewSound( soundname, sounddesc, "newmail.wav" );
	}

	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);
#ifndef STATICSSL
		update.pbVersionPrefix = (unsigned char *)FLNAME;
		update.cpbVersionPrefix = strlen((char *)update.pbVersionPrefix);
	#if defined( _UNICODE )
		update.szVersionURL = "http://addons.miranda-im.org/details.php?action=viewfile&id=3228";
		update.szUpdateURL = "http://addons.miranda-im.org/feed.php?dlfile=3228";
	#else 
		update.szVersionURL = "http://addons.miranda-im.org/details.php?action=viewfile&id=3227";
		update.szUpdateURL = "http://addons.miranda-im.org/feed.php?dlfile=3227";
	#endif		
#else
    // there are no "stable" versions for StaticSSL builds.
#endif
		int size = strlen( pluginInfo.shortName );
		char *betaPrefix = (char *)mir_alloc(size + 2);
		strcpy(betaPrefix, pluginInfo.shortName); betaPrefix[size] = ' '; betaPrefix[size+1] = 0;
		update.pbBetaVersionPrefix = (unsigned char *)betaPrefix;
		update.cpbBetaVersionPrefix = size+1;
		update.szBetaUpdateURL = BETAURL;
		size = strlen( update.szBetaUpdateURL );
		char *betaVersionUrl = (char *)mir_alloc(size+5);
		strcpy(betaVersionUrl,update.szBetaUpdateURL);
		strcpy(&betaVersionUrl[size],".md5");
		update.szBetaVersionURL = betaVersionUrl;

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
		mir_free(betaVersionUrl);
		mir_free(betaPrefix);
	}
 	bSecureIM = (ServiceExists("SecureIM/IsContactSecured"));
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

	JCreateServiceFunction( JS_GETADVANCEDSTATUSICON, JGetAdvancedStatusIcon );
	JabberCheckAllContactsAreTransported();
	JGmailSetupIcoLib();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OnLoad - initialize the plugin instance
char* deprecatedUtf8Decode( char* str, WCHAR** ucs2 );
char* deprecatedUtf8Encode( const char* str );
char* deprecatedUtf8EncodeW( const WCHAR* wstr );

extern "C" int __declspec( dllexport ) Load( PLUGINLINK *link )
{
	pluginLink = link;

	hasForkThreadService = ServiceExists(MS_SYSTEM_FORK_THREAD);
	// set the memory, lists & utf8 managers
	mir_getMMI( &mmi );
	if (mir_getLI( &li ) == CALLSERVICE_NOTFOUND ) {
		MessageBoxA( NULL, "This version of plugin requires Miranda 0.4.3 bld#42 or later", "Fatal error", MB_OK );
		return 1;
	}
	if (mir_getUTFI( &utfi ) == CALLSERVICE_NOTFOUND ) {
		// older core version. use local functions.
		utfi.utf8_decode   = deprecatedUtf8Decode;
		utfi.utf8_decodecp = NULL;
		utfi.utf8_encode   = deprecatedUtf8Encode;
		utfi.utf8_encodecp = NULL;
		utfi.utf8_encodeW  = deprecatedUtf8EncodeW;
	}
	mir_getMD5I( &md5i );
	mir_getSHA1I( &sha1i );

	// creating the plugins name
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
		if ( szProto != NULL && !strcmp( szProto, jabberProtoName )) {
			if ( JGetWord( hContact, "Status", ID_STATUS_OFFLINE ) != ID_STATUS_OFFLINE )
				JSetWord( hContact, "Status", ID_STATUS_OFFLINE );

			if ( JGetByte( hContact, "IsTransport", 0 )) {
				DBVARIANT dbv;
				if ( !JGetStringT( hContact, "jid", &dbv )) {
					TCHAR* domain = NEWTSTR_ALLOCA(dbv.ptszVal);
					TCHAR* resourcepos = _tcschr( domain, '/' );
					if ( resourcepos != NULL )
						*resourcepos = '\0';
					jabberTransports.insert( _tcsdup( domain ));
					JFreeVariant( &dbv );
		}	}	}

		hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM ) hContact, 0 );
	}

	memset(( char* )&modeMsgs, 0, sizeof( JABBER_MODEMSGS ));
#ifdef _UNICODE
	memset(&listeningToInfo, 0, sizeof( listeningToInfo ));
#endif
	//jabberModeMsg = NULL;
	jabberCodePage = JGetWord( NULL, "CodePage", CP_ACP );

	InitializeCriticalSection( &modeMsgMutex );
#ifdef _UNICODE
	InitializeCriticalSection( &listeningToInfoMutex );
#endif

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
	if ( hReloadIcons )     UnhookEvent( hReloadIcons );
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
#ifdef _UNICODE
	DeleteCriticalSection( &listeningToInfoMutex );
#endif
	DeleteCriticalSection( &modeMsgMutex );
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

	for ( int i=0; i < jabberTransports.getCount(); i++ )
		free( jabberTransports[i] );
	jabberTransports.destroy();

	if ( hMainThread ) CloseHandle( hMainThread );
	return 0;
}
