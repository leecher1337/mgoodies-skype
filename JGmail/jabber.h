/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-2004  Santithorn Bunchua

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

*/

#ifndef _JABBER_H_
#define _JABBER_H_

#if defined(UNICODE) && !defined(_UNICODE)
	#define _UNICODE
#endif

#include <malloc.h>

#define NEWSTR_ALLOCA(A) (A==NULL)?NULL:strcpy((char*)alloca(strlen(A)+1),A)

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

/*******************************************************************
 * Global header files
 *******************************************************************/
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_netlib.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_options.h>
#include <m_userinfo.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_message.h>
#include <m_skin.h>
#include <m_chat.h>
#include <win2k.h>

#include "jabber_xml.h"
#include "jabber_byte.h"

/*******************************************************************
 * Global constants
 *******************************************************************/
#define JABBER_DEFAULT_PORT 5222
#define JABBER_IQID "mir_"
#define JABBER_MAX_JID_LEN  256

// User-defined message
#define WM_JABBER_REGDLG_UPDATE        WM_USER + 100
#define WM_JABBER_AGENT_REFRESH        WM_USER + 101
#define WM_JABBER_TRANSPORT_REFRESH    WM_USER + 102
#define WM_JABBER_REGINPUT_ACTIVATE    WM_USER + 103
#define WM_JABBER_REFRESH              WM_USER + 104
#define WM_JABBER_CHECK_ONLINE         WM_USER + 105
#define WM_JABBER_CHANGED              WM_USER + 106
#define WM_JABBER_ACTIVATE             WM_USER + 107
#define WM_JABBER_SET_FONT             WM_USER + 108
#define WM_JABBER_FLASHWND             WM_USER + 109
#define WM_JABBER_GC_MEMBER_ADD        WM_USER + 110
#define WM_JABBER_GC_FORCE_QUIT        WM_USER + 111
#define WM_JABBER_SHUTDOWN             WM_USER + 112
#define WM_JABBER_SMILEY               WM_USER + 113
#define WM_JABBER_JOIN                 WM_USER + 114
#define WM_JABBER_ADD_TO_ROSTER        WM_USER + 115
// Error code
#define JABBER_ERROR_REDIRECT				302
#define JABBER_ERROR_BAD_REQUEST			400
#define JABBER_ERROR_UNAUTHORIZED			401
#define JABBER_ERROR_PAYMENT_REQUIRED		402
#define JABBER_ERROR_FORBIDDEN				403
#define JABBER_ERROR_NOT_FOUND				404
#define JABBER_ERROR_NOT_ALLOWED			405
#define JABBER_ERROR_NOT_ACCEPTABLE			406
#define JABBER_ERROR_REGISTRATION_REQUIRED	407
#define JABBER_ERROR_REQUEST_TIMEOUT		408
#define JABBER_ERROR_CONFLICT				409
#define JABBER_ERROR_INTERNAL_SERVER_ERROR	500
#define JABBER_ERROR_NOT_IMPLEMENTED		501
#define JABBER_ERROR_REMOTE_SERVER_ERROR	502
#define JABBER_ERROR_SERVICE_UNAVAILABLE	503
#define JABBER_ERROR_REMOTE_SERVER_TIMEOUT	504
// Vcard flag
#define JABBER_VCEMAIL_HOME			1
#define JABBER_VCEMAIL_WORK			2
#define JABBER_VCEMAIL_INTERNET		4
#define JABBER_VCEMAIL_X400			8
#define JABBER_VCTEL_HOME			1
#define JABBER_VCTEL_WORK			2
#define JABBER_VCTEL_VOICE			4
#define JABBER_VCTEL_FAX			8
#define JABBER_VCTEL_PAGER			16
#define JABBER_VCTEL_MSG			32
#define JABBER_VCTEL_CELL			64
#define JABBER_VCTEL_VIDEO			128
#define JABBER_VCTEL_BBS			256
#define JABBER_VCTEL_MODEM			512
#define JABBER_VCTEL_ISDN			1024
#define JABBER_VCTEL_PCS			2048
// File transfer setting
#define JABBER_OPTION_FT_DIRECT		0	// Direct connection
#define JABBER_OPTION_FT_PASS		1	// Use PASS server
#define JABBER_OPTION_FT_PROXY		2	// Use proxy with local port forwarding
// Font style saved in DB
#define JABBER_FONT_BOLD			1
#define JABBER_FONT_ITALIC			2
// Font for groupchat log dialog
#define JABBER_GCLOG_NUM_FONT		6	// 6 fonts ( 0:send, 1:msg, 2:time, 3:nick, 4:sys, 5:/me )
// Old SDK don't have this
#ifndef SPI_GETSCREENSAVERRUNNING
#define SPI_GETSCREENSAVERRUNNING 114
#endif
#define IDC_STATIC ( -1 )
// Icon list
enum {
	JABBER_IDI_GCOWNER = 0,
	JABBER_IDI_GCADMIN,
	JABBER_IDI_GCMODERATOR,
	JABBER_IDI_GCVOICE,
	JABBER_ICON_TOTAL
};

/*******************************************************************
 * Global data structures and data type definitions
 *******************************************************************/
typedef HANDLE JABBER_SOCKET;

enum JABBER_SESSION_TYPE
{
	JABBER_SESSION_NORMAL,
	JABBER_SESSION_REGISTER
};

struct ThreadData {
	HANDLE hThread;
	JABBER_SESSION_TYPE type;

	char username[128];
	char password[128];
	char server[128];
	char manualHost[128];
	char resource[128];
	char fullJID[256];
	WORD port;
	JABBER_SOCKET s;
	BOOL useSSL;

	char newPassword[128];

	HWND reg_hwndDlg;
	BOOL reg_done;
};

struct JABBER_MODEMSGS
{
	char* szOnline;
	char* szAway;
	char* szNa;
	char* szDnd;
	char* szFreechat;
};

struct JABBER_REG_ACCOUNT
{
	char username[128];
	char password[128];
	char server[128];
	char manualHost[128];
	WORD port;
	BOOL useSSL;
};

typedef enum { FT_SI, FT_OOB, FT_BYTESTREAM } JABBER_FT_TYPE;
typedef enum { FT_CONNECTING, FT_INITIALIZING, FT_RECEIVING, FT_DONE, FT_ERROR, FT_DENIED } JABBER_FILE_STATE;

struct filetransfer
{
	filetransfer();
	~filetransfer();

	void close();
	void complete();
	int  create();	

	PROTOFILETRANSFERSTATUS std;

//	HANDLE hContact;
	JABBER_FT_TYPE type;
	JABBER_SOCKET s;
	JABBER_FILE_STATE state;
	char* jid;
	int   fileId;
	char* iqId;
	char* sid;
	int   bCompleted;
	HANDLE hWaitEvent;
	WCHAR* wszFileName;

	// For type == FT_BYTESTREAM
	JABBER_BYTE_TRANSFER *jbt;

	// Used by file receiving only
	char* httpHostName;
	WORD httpPort;
	char* httpPath;

	// Used by file sending only
	HANDLE hFileEvent;
	long *fileSize;
	char* szDescription;
};

struct JABBER_SEARCH_RESULT
{
	PROTOSEARCHRESULT hdr;
	char jid[256];
};

struct JABBER_GCLOG_FONT
{
	char face[LF_FACESIZE];		// LF_FACESIZE is from LOGFONT struct
	BYTE style;
	char size;	// signed
	BYTE charset;
	COLORREF color;
};

struct JABBER_FIELD_MAP 
{
	int id;
	char* name;
};

enum JABBER_MUC_JIDLIST_TYPE 
{
	MUC_VOICELIST,
	MUC_MEMBERLIST,
	MUC_MODERATORLIST,
	MUC_BANLIST,
	MUC_ADMINLIST,
	MUC_OWNERLIST
};

struct JABBER_MUC_JIDLIST_INFO
{
	JABBER_MUC_JIDLIST_TYPE type;
	char* roomJid;	// filled-in by the WM_JABBER_REFRESH code
	XmlNode *iqNode;

	char* type2str( void ) const;
};

typedef void ( *JABBER_FORM_SUBMIT_FUNC )( char* submitStr, void *userdata );
typedef void ( __cdecl *JABBER_THREAD_FUNC )( void * );

#include "jabber_list.h"

/*******************************************************************
 * Global variables
 *******************************************************************/
extern HINSTANCE hInst;
extern HANDLE hMainThread;
extern DWORD jabberMainThreadId;
extern char* jabberProtoName;
extern char* jabberModuleName;
extern HANDLE hNetlibUser;
#ifndef STATICSSL
extern HMODULE hLibSSL;
#endif
extern PVOID jabberSslCtx;

extern struct ThreadData *jabberThreadInfo;
extern char* jabberJID;
extern char* streamId;
extern DWORD jabberLocalIP;
extern BOOL jabberConnected;
extern BOOL jabberOnline;
extern int jabberStatus;
extern int jabberDesiredStatus;

extern CRITICAL_SECTION modeMsgMutex;
extern JABBER_MODEMSGS modeMsgs;
extern BOOL modeMsgStatusChangePending;

extern BOOL   jabberChangeStatusMessageOnly;
extern BOOL   jabberSendKeepAlive;
extern HICON  jabberIcon[JABBER_ICON_TOTAL];
extern BOOL   jabberChatDllPresent;

extern HWND hwndJabberAgents;
extern HWND hwndAgentReg;
extern HWND hwndAgentRegInput;
extern HWND hwndAgentManualReg;
extern HWND hwndRegProgress;
extern HWND hwndJabberVcard;
extern HWND hwndJabberChangePassword;
extern HWND hwndJabberGroupchat;
extern HWND hwndJabberJoinGroupchat;
extern HWND hwndMucVoiceList;
extern HWND hwndMucMemberList;
extern HWND hwndMucModeratorList;
extern HWND hwndMucBanList;
extern HWND hwndMucAdminList;
extern HWND hwndMucOwnerList;

extern const char xmlnsOwner[], xmlnsAdmin[];

/*******************************************************************
 * Function declarations
 *******************************************************************/

//---- jabber_bitmap.cpp ----------------------------------------------

HBITMAP __stdcall JabberBitmapToAvatar( HBITMAP hBitmap );
int     __stdcall JabberEnterBitmapName( char* szDest );

//---- jabber_chat.cpp ----------------------------------------------

void JabberGcLogCreate( JABBER_LIST_ITEM* item );
void JabberGcLogUpdateMemberStatus( JABBER_LIST_ITEM* item, char* nick, int action, XmlNode* reason );
void JabberGcQuit( JABBER_LIST_ITEM* jid, int code, XmlNode* reason );

//---- jabber_file.c ------------------------------------------------

void __cdecl JabberFileReceiveThread( filetransfer* ft );
void __cdecl JabberFileServerThread( filetransfer* ft );

//---- jabber_form.c ------------------------------------------------

void JabberFormCreateUI( HWND hwndStatic, XmlNode *xNode, int *formHeight );
char* JabberFormGetData( HWND hwndStatic, XmlNode *xNode );
void JabberFormCreateDialog( XmlNode *xNode, char* defTitle, JABBER_FORM_SUBMIT_FUNC pfnSubmit, void *userdata );

//---- jabber_ft.c --------------------------------------------------

void JabberFtCancel( filetransfer* ft );
void JabberFtInitiate( char* jid, filetransfer* ft );
void JabberFtHandleSiRequest( XmlNode *iqNode );
void JabberFtAcceptSiRequest( filetransfer* ft );
BOOL JabberFtHandleBytestreamRequest( XmlNode *iqNode );

//---- jabber_groupchat.c -------------------------------------------

int JabberMenuHandleGroupchat( WPARAM wParam, LPARAM lParam );
void JabberGroupchatProcessPresence( XmlNode *node, void *userdata );
void JabberGroupchatProcessMessage( XmlNode *node, void *userdata );
void JabberGroupchatProcessInvite( char* roomJid, char* from, char* reason, char* password );

//---- jabber_libstr.c ----------------------------------------------

void  __stdcall replaceStr( char*& dest, const char* src );
char* __stdcall rtrim( char *string );

//---- jabber_misc.c ------------------------------------------------

void   JabberAddContactToRoster( const char* jid, const char* nick, const char* grpName );
void   JabberChatDllError( void );
int    JabberCompareJids( const char* jid1, const char* jid2 );
void   JabberContactListCreateGroup( char* groupName );
void   JabberDBAddAuthRequest( char* jid, char* nick );
HANDLE JabberDBCreateContact( char* jid, char* nick, BOOL temporary, BOOL stripResource );
ULONG  JabberForkThread( void ( __cdecl *threadcode )( void* ), unsigned long stacksize, void *arg );
void   JabberGetAvatarFileName( HANDLE hContact, char* pszDest, int cbLen );
void   JabberSetServerStatus( int iNewStatus );
char*  EscapeChatTags(char* pszText);
char* UnEscapeChatTags(char* str_in);

//---- jabber_svc.c -------------------------------------------------

void JabberEnableMenuItems( BOOL bEnable );

//---- jabber_std.cpp ----------------------------------------------

#if defined( _DEBUG )
	#define JCallService CallService
#else
	int __stdcall  JCallService( const char* szSvcName, WPARAM wParam, LPARAM lParam );
#endif

HANDLE __stdcall  JCreateServiceFunction( const char* szService, MIRANDASERVICE serviceProc );
void   __stdcall  JDeleteSetting( HANDLE hContact, const char* valueName );
DWORD  __stdcall  JGetByte( const char* valueName, int parDefltValue );
DWORD  __stdcall  JGetByte( HANDLE hContact, const char* valueName, int parDefltValue );
char*  __stdcall  JGetContactName( HANDLE hContact );
DWORD  __stdcall  JGetDword( HANDLE hContact, const char* valueName, DWORD parDefltValue );
int    __stdcall  JGetStaticString( const char* valueName, HANDLE hContact, char* dest, int dest_len );
int    __stdcall  JGetStringUtf( HANDLE hContact, char* valueName, DBVARIANT* dbv );
WORD   __stdcall  JGetWord( HANDLE hContact, const char* valueName, int parDefltValue );
void   __fastcall JFreeVariant( DBVARIANT* dbv );
int    __stdcall  JSendBroadcast( HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam );
DWORD  __stdcall  JSetByte( const char* valueName, int parValue );
DWORD  __stdcall  JSetByte( HANDLE hContact, const char* valueName, int parValue );
DWORD  __stdcall  JSetDword( HANDLE hContact, const char* valueName, DWORD parValue );
DWORD  __stdcall  JSetString( HANDLE hContact, const char* valueName, const char* parValue );
DWORD  __stdcall  JSetStringUtf( HANDLE hContact, const char* valueName, const char* parValue );
DWORD  __stdcall  JSetWord( HANDLE hContact, const char* valueName, int parValue );
char*  __stdcall  JTranslate( const char* str );

//---- jabber_thread.cpp -------------------------------------------

void __cdecl JabberServerThread( struct ThreadData *info );

//---- jabber_util.c ----------------------------------------------

void          __stdcall JabberSerialInit( void );
void          __stdcall JabberSerialUninit( void );
unsigned int  __stdcall JabberSerialNext( void );
int           __stdcall JabberSend( JABBER_SOCKET s, const char* fmt, ... );
HANDLE        __stdcall JabberHContactFromJID( const char* jid );
void          __stdcall JabberLog( const char* fmt, ... );
char*         __stdcall JabberNickFromJID( const char* jid );
char*         __stdcall JabberUrlDecode( char* str );
void          __stdcall JabberUrlDecodeW( WCHAR* str );
char*         __stdcall JabberUrlEncode( const char* str );
int           __stdcall JabberUtfCompareI( const char* s1, const char* s2 );
char*         __stdcall JabberUtf8Decode( char*,WCHAR** );
char*         __stdcall JabberUtf8Encode( const char* str );
char*         __stdcall JabberSha1( char* str );
char*         __stdcall JabberUnixToDos( const char* str );
WCHAR*        __stdcall JabberUnixToDosW( const WCHAR* str );
void          __stdcall JabberHttpUrlDecode( char* str );
char*         __stdcall JabberHttpUrlEncode( const char* str );
int           __stdcall JabberCombineStatus( int status1, int status2 );
char*         __stdcall JabberErrorStr( int errorCode );
char*         __stdcall JabberErrorMsg( XmlNode *errorNode );
void          __stdcall JabberSendVisibleInvisiblePresence( BOOL invisible );
char*         __stdcall JabberTextEncode( const char* str );
char*         __stdcall JabberTextEncodeW( const wchar_t *str );
char*         __stdcall JabberTextDecode( const char* str );
char*         __stdcall JabberBase64Encode( const char* buffer, int bufferLen );
char*         __stdcall JabberBase64Decode( const char* buffer, int *resultLen );
char*         __stdcall JabberGetVersionText();
time_t        __stdcall JabberIsoToUnixTime( char* stamp );
int           __stdcall JabberCountryNameToId( char* ctry );
void          __stdcall JabberSendPresenceTo( int status, char* to, char* extra );
void          __stdcall JabberSendPresence( int );
void          __stdcall JabberStringAppend( char* *str, int *sizeAlloced, const char* fmt, ... );
char*         __stdcall JabberGetClientJID( const char* jid, char*, size_t );
char*         __stdcall JabberStripJid( const char* jid, char* dest, size_t destLen );
int           __stdcall JabberGetPictureType( const char* buf );

//---- jabber_vcard.c -----------------------------------------------

int JabberSendGetVcard( const char* );

//---- jabber_ws.c -------------------------------------------------

BOOL          JabberWsInit( void );
void          JabberWsUninit( void );
JABBER_SOCKET JabberWsConnect( char* host, WORD port );
int           JabberWsSend( JABBER_SOCKET s, char* data, int datalen );
int           JabberWsRecv( JABBER_SOCKET s, char* data, long datalen );

///////////////////////////////////////////////////////////////////////////////
// TXT encode helper

class TextEncoder {
	char* m_body;

public:
	__forceinline TextEncoder( const char* pSrc ) :
		m_body( JabberTextEncode( pSrc ))
		{}

	__forceinline ~TextEncoder()
		{  free( m_body );
		}

	__forceinline const char* str() const { return m_body; }
};

#define TXT(A) TextEncoder(A).str()

///////////////////////////////////////////////////////////////////////////////
// UTF encode helper

class Utf8Encoder {
	char* m_body;

public:
	__forceinline Utf8Encoder( const char* pSrc ) :
		m_body( JabberUtf8Encode( pSrc ))
		{}

	__forceinline ~Utf8Encoder()
		{  free( m_body );
		}

	__forceinline const char* str() const { return m_body; }
};

#define UTF8(A) Utf8Encoder(A).str()

///////////////////////////////////////////////////////////////////////////////
// memory interface

extern MM_INTERFACE memoryManagerInterface;

#define mir_alloc(n) memoryManagerInterface.mmi_malloc(n)
#define mir_free(ptr) memoryManagerInterface.mmi_free(ptr)
#define mir_realloc(ptr,size) memoryManagerInterface.mmi_realloc(ptr,size)

#endif
