#include "m_chat.h"

#define MAX_BUF 256    // Buffer for topic-string

typedef struct {
	char*		   szChatName;		 // name of chat session
	HANDLE*		   mJoinedContacts;  //	contacts
	int            mJoinedCount;     // contacts count
} gchat_contacts;

int ChatInit(WPARAM, LPARAM);
int ChatStart(char*);
gchat_contacts *GetChat(char*);
void RemChatContact(gchat_contacts*, HANDLE);
int AddMembers(char*);
int GCEventHook (WPARAM, LPARAM);
int GCMenuHook (WPARAM, LPARAM);
void KillChatSession(GCDEST*);