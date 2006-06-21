#include <m_popup.h>
#include "m_popupw.h"

// Unicode Popup Info
typedef struct {
	HANDLE lchContact;
	HICON lchIcon;
#ifdef _UNICODE
	WCHAR lptzContactName[MAX_CONTACTNAME];
	WCHAR lptzText[MAX_SECONDLINE];
#else
	char lptzContactName[MAX_CONTACTNAME];
	char lptzText[MAX_SECONDLINE];
#endif
	COLORREF colorBack;                   
	COLORREF colorText;
	WNDPROC PluginWindowProc;
	void * PluginData;
	int iSeconds;                         //Custom delay time in seconds. -1 means "forever", 0 means "default time".
	char cZero[16];                       //some unused bytes which may come useful in the future.
} POPUPDATAT, *LPPOPUPDATAT;

#ifdef _UNICODE
#define MS_POPUP_ADDPOPUPT MS_POPUP_ADDPOPUPW
#else 
#define MS_POPUP_ADDPOPUPT MS_POPUP_ADDPOPUPEX
#endif

