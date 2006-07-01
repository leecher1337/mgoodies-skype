#define AVH_DEF_POPUPFG 0
#define AVH_DEF_POPUPBG 0x2DB6FF
#define AVH_DEF_AVPOPUPS 0
#define AVH_DEF_LOGTODISK 1
#define AVH_DEF_DEFPOPUPS 0
#define AVH_DEF_SHOWMENU 1

char* GetContactFolder(HANDLE hContact, char* fn);
char* MyDBGetString(HANDLE hContact, char* module, char* setting, char* out, size_t len);