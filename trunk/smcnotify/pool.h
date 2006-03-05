#ifndef __POOL_H__
# define __POOL_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" 
{
#endif


void InitPool();
void FreePool();

BOOL PoolCheckProtocol(const char *protocol);
BOOL PoolCheckContact(HANDLE hContact);
void PoolStatusChangeAddContact(HANDLE hContact, const char* protocol);











#ifdef __cplusplus
}
#endif

#endif // __POOL_H__
