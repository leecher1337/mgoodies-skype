#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <time.h>
#include "resource.h"

void HookEvents(void);
void HookEventsLoaded(void);
void UnhookEvents(void);
void CreateServices(void);
INT_PTR SkypeLoadIcon(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeGetName(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeGetCaps(WPARAM wParam, LPARAM lParam);
/* SkypeGetAvatar
 * 
 * Purpose: Return the avatar file name
 * Params : wParam=0
 *			lParam=0
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeGetAvatar(WPARAM wParam,LPARAM lParam);