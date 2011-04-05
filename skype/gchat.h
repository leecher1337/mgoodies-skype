// m_chat users these BaseTSD types, so if we are compiling with an old PSDK, these typedefs
// are not there, so better define them, just in case...
#ifndef LongToPtr
#define DWORD_PTR DWORD
#endif

#include "../../include/m_chat.h"

#define MAX_BUF 256    // Buffer for topic-string

typedef struct {
	TCHAR*		   szChatName;		 // name of chat session
	HANDLE*		   mJoinedContacts;  //	contacts
	int            mJoinedCount;     // contacts count
} gchat_contacts;

int ChatInit(WPARAM, LPARAM);
int ChatStart(char*);
gchat_contacts *GetChat(TCHAR *szChatId);
HANDLE find_chat(TCHAR *chatname);
#ifdef _UNICODE
HANDLE find_chatA(char *chatname);
#else
#define find_chatA find_chat
#endif
void RemChatContact(gchat_contacts*, HANDLE);
int AddMembers(char*);
int GCEventHook (WPARAM, LPARAM);
int GCMenuHook (WPARAM, LPARAM);
void KillChatSession(GCDEST*);
INT_PTR GCOnLeaveChat(WPARAM wParam,LPARAM lParam);
INT_PTR GCOnJoinChat(WPARAM wParam,LPARAM lParam);
void GCInit(void);
void GCExit(void);
