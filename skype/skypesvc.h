#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <time.h>
#include "resource.h"

void HookEvents(void);
void CreateServices(void);
int SkypeLoadIcon(WPARAM wParam, LPARAM lParam);
int SkypeGetName(WPARAM wParam, LPARAM lParam);
int SkypeGetCaps(WPARAM wParam, LPARAM lParam);
/* SkypeGetAvatar
 * 
 * Purpose: Return the avatar file name
 * Params : wParam=0
 *			lParam=0
 * Returns: 0 - Success
 *		   -1 - Failure
 */
int SkypeGetAvatar(WPARAM wParam,LPARAM lParam);