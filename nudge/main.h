#include "nudge.h"

/*
*
****************************/
void RegisterToUpdate(void);

/*
*
****************************/
void RegisterToTrigger(void);

/*
*
****************************/
void LoadProtocols(void);

/*
*
****************************/
void LoadIcons(void);

/*
*
****************************/
static int LoadChangedIcons(WPARAM, LPARAM);

/*
*
****************************/
DWORD WINAPI ShakeClistWindow(LPVOID);

/*
*
****************************/
DWORD WINAPI ShakeChatWindow(LPVOID);

/*
*
****************************/
int ModulesLoaded(WPARAM,LPARAM);

/*
*
****************************/
void Nudge_ShowPopup(CNudgeElement, HANDLE, TCHAR *);

/*
*
****************************/
void Nudge_ShowEvent(CNudgeElement, HANDLE, DWORD timestamp);

/*
*
****************************/
void Nudge_SentEvent(CNudgeElement, HANDLE);

/*
*
****************************/
void Nudge_ShowStatus(CNudgeElement, HANDLE, DWORD timestamp);

/*
*
****************************/
void Nudge_SentStatus(CNudgeElement, HANDLE);

/*
*
****************************/
int Nudge_AddElement(char*, HANDLE);

/*
*
****************************/
int FreeVSApi();

/*
*
****************************/
int InitVSApi();

/*
*
****************************/
int TriggerActionRecv( DWORD actionID, REPORTINFO *ri);

/*
*
****************************/
int TriggerActionSend( DWORD actionID, REPORTINFO *ri);

/*
*
****************************/
void AutoResendNudge(void *wParam) ;