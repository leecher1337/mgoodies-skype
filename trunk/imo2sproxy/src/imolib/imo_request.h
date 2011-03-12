#ifndef _IMO_REQUEST_H_
#define _IMO_REQUEST_H_

#include "cJSON.h"

struct _tagIMORQ;
typedef struct _tagIMORQ IMORQ;

IMORQ *ImoRq_Init(void);
IMORQ *ImoRq_Clone (IMORQ *hRq);
void ImoRq_Exit (IMORQ *hRq);

char *ImoRq_SessId(IMORQ *hRq);
char *ImoRq_GetLastError(IMORQ *hRq);
char *ImoRq_PostAmy(IMORQ *hRq, char *pszMethod, cJSON *data);
void ImoRq_CreateID(char *pszID, int cbID);
char *ImoRq_PostSystem(IMORQ *hRq, char *pszMethod, char *pszSysTo, char *pszSysFrom, cJSON *data, int bFreeData);
char *ImoRq_PostToSys(IMORQ *hRq, char *pszMethod, char *pszSysTo, cJSON *data, int bFreeData);
void ImoRq_UpdateAck(IMORQ *hRq, unsigned long lAck);
unsigned long ImoRq_GetSeq(IMORQ *hRq);
char *ImoRq_UserActivity(IMORQ *hRq);
char *ImoRq_ResetRPC(IMORQ *hRq);
char *ImoRq_Reui_Session(IMORQ *hRq);
char *ImoRq_Echo(IMORQ *hRq);
#endif
