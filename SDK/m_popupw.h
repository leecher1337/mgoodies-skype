/*
===============================================================================
                                PopUp plugin
Plugin Name: PopUp
Plugin authors: Luca Santarelli aka hrk (hrk@users.sourceforge.net)
                Victor Pavlychko aka zazoo (nullbie@gmail.com)
===============================================================================
The purpose of this plugin is to give developers a common "platform/interface"
to show PopUps. It is born from the source code of NewStatusNotify, another
plugin I've made.

Remember that users *must* have this plugin enabled, or they won't get any
popup. Write this in the requirements, do whatever you wish ;-)... but tell
them!
===============================================================================
*/
#ifndef M_POPUPW_H
#define M_POPUPW_H

#ifndef MAX_CONTACTNAME
	#define MAX_CONTACTNAME 2048
#endif

#ifndef MAX_SECONDLINE
	#define MAX_SECONDLINE 2048
#endif

// Unicode Popup Info
typedef struct {
	HANDLE lchContact;
	HICON lchIcon;
	WCHAR lpwzContactName[MAX_CONTACTNAME];
	WCHAR lpwzText[MAX_SECONDLINE];
	COLORREF colorBack;                   
	COLORREF colorText;
	WNDPROC PluginWindowProc;
	void * PluginData;
	int iSeconds;                         //Custom delay time in seconds. -1 means "forever", 0 means "default time".
	char cZero[16];                       //some unused bytes which may come useful in the future.
} POPUPDATAW, *LPPOPUPDATAW;

// Create Popup
#define MS_POPUP_ADDPOPUPW "PopUp/AddPopUpW"

static int __inline PUAddPopUpW(POPUPDATAW* ppdp) {
	return CallService(MS_POPUP_ADDPOPUPW, (WPARAM)ppdp,0);
}

// Change Text
#define MS_POPUP_CHANGETEXTW "PopUp/ChangetextW"

static int __inline PUChangeTextW(HWND hWndPopUp, LPCWSTR lpwzNewText) {
	return (int)CallService(MS_POPUP_CHANGETEXTW, (WPARAM)hWndPopUp, (LPARAM)lpwzNewText);
}

#endif
