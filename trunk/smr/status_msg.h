#ifndef __STATUS_MSG_H__
# define __STATUS_MSG_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" 
{
#endif

#include "commons.h"


void InitStatusMsgs();
void FreeStatusMsgs();


void ClearStatusMessage(HANDLE hContact);
void SetStatusMessage(HANDLE hContact, TCHAR *msg);











#ifdef __cplusplus
}
#endif

#endif // __STATUS_MSG_H__
