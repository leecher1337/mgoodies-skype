#ifndef __popup_wnd_h__
#define __popup_wnd_h__

/* This sturcy containf information about single popup window
 * windows can access it through GWL_USERDATA parameter of
 * corresponding window
 */

typedef struct {
	//Handles needed for drawing purposes.
	HWND lchMain;          //Main window
	HWND lchBackground;    //Background control
	//CallingPlugin data
	POPUPDATAEX * PopUpDataPt;
	HBRUSH hBrushBackground;

	// New fields
	char time[2+1+2+1];
	HFONT hfnTitle;
	HFONT hfnText;
	TEXTCHUNK *title;
	TEXTCHUNK *text;
	SIZE sz;
	RECT rc;
	int title_height;

	MyBitmap *bmp;
	SkinVars *skinVars;

	bool deleting;
} PopUpWindow;

/*
typedef struct {
	//For new render
	HWND lchMain;          //Main window
	char time[2+1+2+1];
	HFONT hfnTitle;
	HFONT hfnText;
	TEXTCHUNK *title;
	TEXTCHUNK *text;
	HFONT fntTitle;
	HFONT fntText;
	SIZE sz;

	//CallingPlugin data
	POPUPDATAEX * PopUpDataPt;
	HBRUSH hBrushBackground;
} PopUpWindow;
*/
extern char *szWndClass;
BOOL RegisterWindow();

PopUpWindow *CreatePopUp(POPUPDATAEX*);
void ResizePopUpWindow(HWND);
void ShowPopUpWindow(HWND, RECT*); //Shows a previously non visible popup (w or w/o effects).
void MovePopUpWindow(HWND, RECT*); //Changes the position of a alrady visible popup.
void HidePopUpWindow(HWND); //Hides a previously visible popup (w or w/o effects).
void CalculatePopUpWindowPosition(HWND, RECT*); //Computes the coordinates of a popup, given the coordinates of the previous one.


#endif // __popup_wnd_h__
