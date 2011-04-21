// m_chat users these BaseTSD types, so if we are compiling with an old PSDK, these typedefs
// are not there, so better define them, just in case...
#ifndef LongToPtr
#define DWORD_PTR DWORD
#endif

#pragma warning (push)
#pragma warning (disable: 4201) // nonstandard extension used : nameless struct/union
#include "../../include/m_chat.h"
#pragma warning (pop)

#define MAX_BUF 256    // Buffer for topic-string

typedef struct {
	HANDLE hContact;
	TCHAR szRole[12];
} gchat_contact;

typedef struct {
	TCHAR*		   szChatName;		 // name of chat session
	gchat_contact* mJoinedContacts;  //	contacts
	int            mJoinedCount;     // contacts count
} gchat_contacts;

int ChatInit(WPARAM, LPARAM);
int  __cdecl ChatStart(char *szChatId, BOOL bJustCreate);
gchat_contacts *GetChat(TCHAR *szChatId);
HANDLE find_chat(TCHAR *chatname);
#ifdef _UNICODE
HANDLE find_chatA(char *chatname);
#else
#define find_chatA find_chat
#endif
void RemChatContact(gchat_contacts*, HANDLE);
gchat_contact *GetChatContact(gchat_contacts *gc, HANDLE hContact);
int AddMembers(char*);
void AddMembersThread(char *szSkypeMsg);
void RemChat(TCHAR *szChatId);
int GCEventHook (WPARAM, LPARAM);
int GCMenuHook (WPARAM, LPARAM);
void KillChatSession(GCDEST*);
INT_PTR GCOnLeaveChat(WPARAM wParam,LPARAM lParam);
INT_PTR GCOnJoinChat(WPARAM wParam,LPARAM lParam);
void GCInit(void);
void GCExit(void);
void SetChatTopic (TCHAR *szChatId, TCHAR *szTopic, BOOL bSet);
