class SmileyWindow;

#ifndef SMILEYWINDOW_INCLUDED
#define SMILEYWINDOW_INCLUDED

#include "IEView.h"
#include "Smiley.h"
#include "ieview_common.h"

class SmileyWindow {

private:
	HWND    hwnd;
	IEView* view;
	SmileyMap *map;
	int 	cellWidth, maxWidth, maxHeight, cellHeight;
	int     viewWidth, viewHeight;
    HWND 	hwndTarget;
	UINT 	targetMessage;
  	LPARAM 	targetWParam;
public:
	SmileyWindow(SmileyMap *);
	~SmileyWindow();
	void    show(HWND hwndTarget, UINT targetMessage, WPARAM targetWParam, int , int);
	void    init(int cw, int ch);
	void    choose(const char *);
	
};

#endif
