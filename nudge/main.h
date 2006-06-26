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
void Nudge_ShowPopup(CNudgeElement, HANDLE, char *);

/*
*
****************************/
void Nudge_ShowEvent(CNudgeElement, HANDLE);

/*
*
****************************/
void Nudge_SentEvent(CNudgeElement, HANDLE);

/*
*
****************************/
void Nudge_ShowStatus(CNudgeElement, HANDLE);

/*
*
****************************/
void Nudge_SentStatus(CNudgeElement, HANDLE);

/*
*
****************************/
int Nudge_AddElement(char*);

/*
*
****************************/
int FreeVSApi();

/*
*
****************************/
int InitVSApi();
