#ifndef NUDGE_H
#define NUDGE_H
typedef struct CNudgeElement
{
	char ProtocolName[64];
	char NudgeSoundname[100];
	bool showPopup;
	bool showEvent;
	bool popupWindowColor;
	COLORREF popupBackColor;
	COLORREF popupTextColor;
	int popupTimeSec;
	HICON hIcon;
	CNudgeElement *next;
} CNUDGEELEMENT;


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
void Nudge_ShowPopup(CNudgeElement*, HANDLE);

/*
*
****************************/
int Nudge_AddElement(char*);

#endif