#ifndef __POOL_H__
# define __POOL_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" 
{
#endif

#include "commons.h"


void InitPool();
void FreePool();

void PoolSetTimer(void);

BOOL PoolCheckProtocol(const char *protocol);
BOOL PoolCheckContact(HANDLE hContact);

void PoolRemoveContact(HANDLE hContact);

void PoolStatusChangeAddContact(HANDLE hContact);

void PoolAddAllContacts(int timer, const char *protocol);










#ifdef __cplusplus
}
#endif

#endif // __POOL_H__
