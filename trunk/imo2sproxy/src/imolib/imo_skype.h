#include "cJSON.h"

struct _tagIMOSKYPE;
typedef struct _tagIMOSKYPE IMOSKYPE;
// 0 	- 	Message received, you may delete the cJSON object now
// 1	-	Don't delete cJSON object, callback will take care of freeing
typedef int(*IMOSTATCB)(cJSON *pMsg, void *pUser);

IMOSKYPE *ImoSkype_Init(IMOSTATCB StatusCb, void *pUser);
void ImoSkype_Exit(IMOSKYPE *hSkype);
char *ImoSkype_GetLastError(IMOSKYPE *hSkype);
char *ImoSkype_GetUserHandle(IMOSKYPE *hSkype);

int ImoSkype_Login(IMOSKYPE *hSkype, char *pszUser, char *pszPass);
int ImoSkype_Logout(IMOSKYPE *hSkype);
int ImoSkype_Poll(IMOSKYPE *hSkype);
int ImoSkype_Typing(IMOSKYPE *hSkype, char *pszBuddy, char *pszStatus);
int ImoSkype_SetStatus(IMOSKYPE *hSkype, char *pszStatus, char *pszStatusMsg);
int ImoSkype_SendMessage(IMOSKYPE *hSkype, char *pszBuddy, char *pszMessage);
int ImoSkype_AddBuddy(IMOSKYPE *hSkype, char *pszBuddy);
int ImoSkype_DelBuddy(IMOSKYPE *hSkype, char *pszBuddy, char *pszGroup);
int ImoSkype_BlockBuddy(IMOSKYPE *hSkype, char *pszBuddy);
int ImoSkype_UnblockBuddy(IMOSKYPE *hSkype, char *pszBuddy);
int ImoSkype_ChangeAlias(IMOSKYPE *hSkype, char *pszBuddy, char *pszNewAlias);
int ImoSkype_StartVoiceCall(IMOSKYPE *hSkype, char *pszBuddy);
int ImoSkype_KeepAlive(IMOSKYPE *hSkype);
int ImoSkype_Ping(IMOSKYPE *hSkype);
