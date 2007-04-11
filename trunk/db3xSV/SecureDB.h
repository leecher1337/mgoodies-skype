#ifndef _SECURE_DB__
#define _SECURE_DB__

#include <m_clist.h>

int EncReadFile(HANDLE hFile,void* data,unsigned long toread,unsigned long* read,void* ov);
int EncWriteFile(HANDLE hFile,void* data,unsigned long towrite,unsigned long* written,void* ov);
int EncGetPassword(void* dbh,const char* dbase);
int EncInitMenus(WPARAM wParam, LPARAM lParam);
int EncOnLoad();

 int DB3XSSetPassword(WPARAM wParam, LPARAM lParam);
 int DB3XSRemovePassword(WPARAM wParam, LPARAM lParam);
 int DB3XSMakeBackup(WPARAM wParam, LPARAM lParam);

extern long g_secured;
extern HANDLE hSetPwdMenu;
extern HANDLE hDelPwdMenu;
extern HANDLE hOnLoadHook;


#endif //_SECURE_DB__
