#ifndef SRMM_RICHUTIL_H
#define SRMM_RICHUTIL_H

#ifndef EP_EDITTEXT
#define EP_EDITTEXT 1
#endif
#ifndef ETS_NORMAL
#define ETS_NORMAL 1
#endif
#ifndef ETS_DISABLED
#define ETS_DISABLED 4
#endif
#ifndef ETS_READONLY
#define ETS_READONLY 6
#endif

typedef struct {
	HWND hwnd;
	RECT rect;
	int hasUglyBorder;
} TRichUtil;

void RichUtil_Load();
void RichUtil_Unload();
int RichUtil_SubClass(HWND hwndEdit);

#endif
