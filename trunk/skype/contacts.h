// Prototypes
HANDLE add_contextmenu(HANDLE hContact);
HANDLE find_contact(char *name);
HANDLE add_contact(char *name, DWORD flags);
HANDLE add_mainmenu(void);
CLISTMENUITEM HupItem(void);
CLISTMENUITEM CallItem(void);
void logoff_contacts(void);
int PrebuildContactMenu(WPARAM, LPARAM);
//int ClistDblClick(WPARAM, LPARAM);