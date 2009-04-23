#ifndef _SECURE_DB__
#define _SECURE_DB__

#include <m_clist.h>

INT_PTR EncReadFile(HANDLE hFile,void* data,unsigned long toread,unsigned long* read,void* ov);
INT_PTR EncWriteFile(HANDLE hFile,void* data,unsigned long towrite,unsigned long* written,void* ov);
INT_PTR EncGetPassword(void* dbh,const char* dbase);
INT_PTR EncInitMenus(WPARAM wParam, LPARAM lParam);
INT_PTR EncOnLoad();

 INT_PTR DB3XSSetPassword(WPARAM wParam, LPARAM lParam);
 INT_PTR DB3XSRemovePassword(WPARAM wParam, LPARAM lParam);
 INT_PTR DB3XSMakeBackup(WPARAM wParam, LPARAM lParam);

extern long g_secured;
extern HANDLE hSetPwdMenu;
extern HANDLE hDelPwdMenu;
extern HANDLE hOnLoadHook;


#endif //_SECURE_DB__
