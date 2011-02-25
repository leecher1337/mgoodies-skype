struct _tagIOLAYER;
typedef struct _tagIOLAYER IOLAYER;

IOLAYER*IoLayer_Init(void);
void IoLayer_Exit (IOLAYER *hIO);

char *IoLayer_Post(IOLAYER *hIO, char *pszURL, char *pszPostFields, unsigned int cbPostFields);
char *IoLayer_Get(IOLAYER *hIO, char *pszURL);
char *IoLayer_GetLastError(IOLAYER *hIO);
char *IoLayer_EscapeString(IOLAYER *hIO, char *pszData);
void IoLayer_FreeEscapeString(char *pszData);
