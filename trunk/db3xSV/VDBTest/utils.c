#include "headers.h"

int ShowPopup(const char *fmt, ...)
{
    POPUPDATA ppd;
    va_list va;
    //char    text[1024];
    //int     ibsize = 1023;
    
    
    if(CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) == 1) {
        ZeroMemory((void *)&ppd, sizeof(ppd));
        ppd.lchContact = 0;
        ppd.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
        strncpy(ppd.lpzContactName, "Virtual Database:", MAX_CONTACTNAME);
        va_start(va, fmt);
        mir_vsnprintf(ppd.lpzText, MAX_SECONDLINE - 5, fmt, va);
        ppd.colorText = RGB(0,0,0);
        ppd.colorBack = RGB(255,255,0);
        CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
    }
    return 0;
}

