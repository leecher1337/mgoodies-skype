#ifndef _STATUS_ICON_INC
#define _STATUS_ICON_INC

#include <windows.h>

int InitStatusIcons();
int DeinitStatusIcons();

int  GetStatusIconsCount(HANDLE hContact);
void DrawStatusIcons(HANDLE hContact, HDC hdc, RECT r, int gap);
void CheckStatusIconClick(HANDLE hContact, HWND hwndFrom, POINT pt, RECT rc, int gap, int flags);
int AddStickyStatusIcon(WPARAM wParam, LPARAM lParam);

#endif
