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
void Nudge_ShowPopup(CNudgeElement, HANDLE);

/*
*
****************************/
int Nudge_AddElement(char*);

