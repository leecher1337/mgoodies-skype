#include "skype.h"
#include "skypeapi.h"
#include "utf8.h"
#include "msglist.h"
#include "pthread.h"

extern char g_szProtoName[];
extern DWORD mirandaVersion;

/* We are compiling for Miranda 0.4, but if user uses newer version, we are using 
   newer struct at runtime, therefore we have to redeclare it here (see m_protocols.h)
   so that we can use it, if user has Miranda version >= 0.9
 */
typedef struct tagPROTOFILETRANSFERSTATUS_V2
{
	size_t cbSize;
	HANDLE hContact;
	DWORD  flags;      // one of PFTS_* constants

    union {
  	  char **pszFiles;
      TCHAR **ptszFiles;
      WCHAR **pwszFiles;
    };

    int totalFiles;
	int currentFileNumber;
	unsigned __int64 totalBytes;
	unsigned __int64 totalProgress;

    union {
	   char *szWorkingDir;
      TCHAR *tszWorkingDir;
      WCHAR *wszWorkingDir;
    };

    union {
  	  char *szCurrentFile;
      TCHAR *tszCurrentFile;
      WCHAR *wszCurrentFile;
    };

	unsigned __int64 currentFileSize;
	unsigned __int64 currentFileProgress;
	unsigned __int64 currentFileTime;  //as seconds since 1970
} 
PROTOFILETRANSFERSTATUS_V2;

#ifndef IS_MIRANDAIM
/* Miranda NG has no PROTOFILETRANSFERSTATUS_V1 */
typedef struct tagPROTOFILETRANSFERSTATUS_V1 
{
	size_t cbSize;
	MCONTACT hContact;
	int    sending;
    char **files;
	int totalFiles;
	int currentFileNumber;
	unsigned long totalBytes;
	unsigned long totalProgress;
    char *workingDir;
    char *currentFile;
	unsigned long currentFileSize;
	unsigned long currentFileProgress;
	unsigned long currentFileTime;  //as seconds since 1970
} 
PROTOFILETRANSFERSTATUS_V1;
#endif

/* Services */
INT_PTR SkypeRecvFile(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO dbei = { 0 };
	CCSDATA *ccs = (CCSDATA *)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *)ccs->lParam;
	TYP_MSGLENTRY *pEntry;
	DWORD cbFilename, nFiles;
	INT_PTR ret = 0;

	UNREFERENCED_PARAMETER(wParam);

	db_unset(ccs->hContact, "CList", "Hidden");
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = SKYPE_PROTONAME;
	dbei.timestamp = pre->timestamp;
	if (pre->flags & PREF_CREATEREAD) dbei.flags |= DBEF_READ;
	if (pre->flags & PREF_UTF) dbei.flags |= DBEF_UTF;
	dbei.eventType = EVENTTYPE_FILE;
	dbei.cbBlob=sizeof(DWORD);
	if (pre->flags & PREF_UNICODE) {
		for(nFiles=0;cbFilename=wcslen((wchar_t*)&pre->szMessage[dbei.cbBlob])*sizeof(wchar_t);nFiles++)
			dbei.cbBlob+=cbFilename+sizeof(wchar_t);
		dbei.cbBlob+=sizeof(wchar_t);
	} else {
		for(nFiles=0;cbFilename=strlen(&pre->szMessage[dbei.cbBlob]);nFiles++)
			dbei.cbBlob+=cbFilename+1;
		dbei.cbBlob++;
	}
	dbei.pBlob = (PBYTE)pre->szMessage;
	if (pEntry = MsgList_Add(pre->lParam, db_event_add(ccs->hContact, &dbei))) {
		DWORD cbSize = mirandaVersion<0x090000?sizeof(PROTOFILETRANSFERSTATUS_V1):sizeof(PROTOFILETRANSFERSTATUS_V2);
		/* Allocate basic entry and fill some stuff we already know */
		if (pEntry->pfts = calloc(1, cbSize)) {
			PROTOFILETRANSFERSTATUS_V1 *pfts = (PROTOFILETRANSFERSTATUS_V1*)pEntry->pfts;
			int i, iOffs=sizeof(DWORD);

			pfts->cbSize = cbSize;
			pfts->hContact = ccs->hContact;
			pfts->totalFiles = nFiles;
			if (pfts->files=(char**)calloc(nFiles+1, sizeof(char*))) {
				if (pre->flags & PREF_UNICODE) {
					wchar_t *pFN;
					for (i=0; cbFilename=wcslen(pFN=(wchar_t*)&pre->szMessage[iOffs])*sizeof(wchar_t); i++) {
						pfts->files[i]=(char*)wcsdup(pFN);
						iOffs+=cbFilename+sizeof(wchar_t);
					}
				} else {
					char *pFN;
					for(i=0;cbFilename=strlen(pFN=&pre->szMessage[iOffs]);i++) {
						pfts->files[i]=strdup(pFN);
						iOffs+=cbFilename+1;
					}
					if (pre->flags & PREF_UTF) pfts->sending |= PFTS_UTF;
				}
				ret = pre->lParam;
			}
		}
	}
	return ret;
}

INT_PTR SkypeSendFile(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	DBVARIANT dbv;
	char *mymsgcmd, *utfmsg = NULL, *pszFile=NULL;
	TCHAR **files = (TCHAR**)ccs->lParam;
	int nFiles, iLen = 0, ret=0;
	BYTE bIsChatroom = 0 != db_get_b(ccs->hContact, SKYPE_PROTONAME, "ChatRoom", 0);

	UNREFERENCED_PARAMETER(wParam);
	if (bIsChatroom) {
		if (db_get_s(ccs->hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv))
			return 0;
		mymsgcmd = "CHATFILE";
	} else {
		if (db_get_s(ccs->hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv))
			return 0;
		mymsgcmd = "FILE";
	}
	for (nFiles=0; files[nFiles]; nFiles++) {
#ifdef _UNICODE
		utfmsg=(char*)make_utf8_string(files[nFiles]);
#else
		utf8_encode(files[nFiles], &utfmsg);
#endif
		iLen+=strlen(utfmsg)+3;
		if (pszFile=pszFile?(char*)realloc(pszFile, iLen):(char*)calloc(1,iLen)) {
			if (nFiles>0) strcat(pszFile, ",");
			strcat(pszFile, "\"");
			strcat(pszFile, utfmsg);
			strcat(pszFile, "\"");
		}
		free(utfmsg);
	}
	if (pszFile) {
		if (SkypeSend("%s %s %s", mymsgcmd, dbv.pszVal, pszFile) == 0) {
			char *str;
			/* No chatmessage IDs available for filetransfers, there is no possibility 
			 * in SkypeKit to check if incoming filetransfer SENT message belongs to 
			 * the last file sent :(  */
			if (str = SkypeRcvTime("CHATFILE", SkypeTime(NULL), INFINITE)) {
				if (strncmp(str, "ERROR", 5)) {
					char *pTok=strtok(str+9, " ");
					TYP_MSGLENTRY *pEntry;

					if (pTok) {
						ret=strtoul(pTok, NULL, 10);
						if (pEntry = MsgList_Add(ret, INVALID_HANDLE_VALUE)) {
							DWORD cbSize = mirandaVersion<0x090000?sizeof(PROTOFILETRANSFERSTATUS_V1):sizeof(PROTOFILETRANSFERSTATUS_V2);
							/* Allocate basic entry and fill some stuff we already know */
							if (pEntry->pfts = calloc(1, cbSize)) {
								PROTOFILETRANSFERSTATUS_V1 *pfts = (PROTOFILETRANSFERSTATUS_V1*)pEntry->pfts;
								int i;

								pfts->cbSize = cbSize;
								pfts->hContact = ccs->hContact;
								pfts->totalFiles = nFiles;
								pfts->sending = PFTS_SENDING;
								if (pfts->files=(char**)calloc(nFiles+1, sizeof(char*))) {
										for (i=0; i<nFiles; i++) ((TCHAR**)pfts->files)[i]=_tcsdup(files[i]);
								}
							}

						}
					}
				}
				free(str);
			}
		}
		free(pszFile);
	}
	db_free(&dbv);
	return ret;
}

INT_PTR SkypeFileAllow(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	char *pszXferIDs, *pszMsgNum, szMsgNum[16], *ptr, *pszDir=NULL;
	TYP_MSGLENTRY *pEntry;
	INT_PTR ret = 0;

	if (!lParam || !ccs->lParam || !ccs->wParam ||
		!(pEntry = MsgList_FindMessage(ccs->wParam))) return 0;
	sprintf(szMsgNum, "%d", ccs->wParam);
	if (!(pszXferIDs = SkypeGetErr("CHATMESSAGE", szMsgNum, "FILETRANSFERS"))) 
		return 0;
#ifdef _UNICODE
	pszDir = (char*)make_utf8_string((wchar_t*)ccs->lParam);
#else
	utf8_encode ((char*)ccs->lParam, &pszDir);
#endif
	if (pszDir) {
		for (pszMsgNum = strtok(pszXferIDs, ", "); pszMsgNum; pszMsgNum = strtok(NULL, ", ")) {
			if (SkypeSend ("ALTER FILETRANSFER %s ACCEPT %s", pszMsgNum, pszDir)!=-1) {
				if (ptr=SkypeRcv("ALTER FILETRANSFER ACCEPT", 2000)) {
					if (strncmp(ptr, "ERROR", 5)) ret=ccs->wParam;
					free(ptr);
				}
			}
		}

		/* Now we know the save directory in pfts */
		if (mirandaVersion < 0x090000)
		{
			PROTOFILETRANSFERSTATUS_V1 *pfts = (PROTOFILETRANSFERSTATUS_V1*)pEntry->pfts;
			utf8_decode(pszDir, &pfts->workingDir);
			free(pszDir);
		}
		else
		{
			PROTOFILETRANSFERSTATUS_V2 *pfts = (PROTOFILETRANSFERSTATUS_V2*)pEntry->pfts;
			pfts->szWorkingDir = pszDir;
		}
	}
	free(pszXferIDs);
	return ret;
}

INT_PTR SkypeFileCancel(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	char *pszXferIDs, *pszMsgNum, szMsgNum[16], *ptr;
	INT_PTR ret = 1;

	if (!lParam || !ccs->wParam) return 0;
	sprintf(szMsgNum, "%d", ccs->wParam);
	if (!(pszXferIDs = SkypeGetErr("CHATMESSAGE", szMsgNum, "FILETRANSFERS"))) 
		return 0;
	for (pszMsgNum = strtok(pszXferIDs, ", "); pszMsgNum; pszMsgNum = strtok(NULL, ", ")) {
		if (SkypeSend ("ALTER FILETRANSFER %s CANCEL", pszMsgNum)!=-1) {
			if (ptr=SkypeRcv("ALTER FILETRANSFER CANCEL", 2000)) {
				if (strncmp(ptr, "ERROR", 5)) ret=0;
				free(ptr);
			}
		}
	}
	free(pszXferIDs);
	return ret;
}

void FXFreePFTS(void *Ppfts)
{
	PROTOFILETRANSFERSTATUS_V1 *pfts = (PROTOFILETRANSFERSTATUS_V1*)Ppfts;

	if (pfts->files)
	{
		int i;

		for (i=0; i<pfts->totalFiles; i++)
			free(pfts->files[i]);
		free(pfts->files);
		free(mirandaVersion<0x090000?pfts->workingDir:((PROTOFILETRANSFERSTATUS_V2*)pfts)->szWorkingDir);
	}
	free(pfts);
}

BOOL FXHandleRecv(PROTORECVEVENT *pre, MCONTACT hContact)
{
	// Our custom Skypekit FILETRANSFER extension
	char *pszXferIDs, *pszMsgNum, szMsgNum[16];
	DWORD cbMsg = sizeof(DWORD), cbNewSize;

	sprintf (szMsgNum, "%d", pre->lParam);
	if (!(pszXferIDs = SkypeGetErr("CHATMESSAGE", szMsgNum, "FILETRANSFERS"))) 
		return FALSE;
	for (pszMsgNum = strtok(pszXferIDs, ", "); pszMsgNum; pszMsgNum = strtok(NULL, ", "))
	{
		char *pszStatus;
		if (pszStatus = SkypeGetErrID("FILETRANSFER", pszMsgNum, "STATUS"))
		{
			char *pszType;

			if (!strcmp(pszStatus, "NEW") || !strcmp(pszStatus, "PLACEHOLDER"))
			{
				if (pszType = SkypeGetErr("FILETRANSFER", pszMsgNum, "TYPE"))
				{
					if (!strcmp(pszType, "INCOMING"))
					{
						char *pszFN;

						if (pszFN = SkypeGetErr("FILETRANSFER", pszMsgNum, "FILENAME"))
						{
							TCHAR *msgptr;

							if (mirandaVersion >= 0x070000) {
								cbNewSize = cbMsg+strlen(pszFN)+2;
								if ((pre->szMessage = (char*)realloc(pre->szMessage, cbNewSize)))
								{
									memcpy(pre->szMessage+cbMsg, pszFN, cbNewSize-cbMsg-1);
									cbMsg=cbNewSize-1;
								} else pszMsgNum=NULL;
								pre->flags |= PREF_UTF;
							} else {
								msgptr = make_tchar_string((const unsigned char*)pszFN);
								cbNewSize = cbMsg+((_tcslen((TCHAR*)pszFN)+2)*sizeof(TCHAR));
								if ((pre->szMessage = (char*)realloc(pre->szMessage, cbNewSize)))
								{
									memcpy(pre->szMessage+cbMsg, msgptr, cbNewSize-cbMsg-sizeof(TCHAR));
									cbMsg=cbNewSize-sizeof(TCHAR);
								} else pszMsgNum=NULL;
								pre->flags |= PREF_TCHAR;
								free(msgptr);
							}
							free(pszFN);
						}
					}
					free (pszType);
				}
			}
			free(pszStatus);
		}
	}
	free(pszXferIDs);
	if (pre->szMessage)
	{
		CCSDATA ccs = {0};

		*((TCHAR*)&pre->szMessage[cbMsg])=0;
		*((DWORD*)pre->szMessage)=pre->lParam;
		ccs.szProtoService = PSR_FILE;
		ccs.hContact = hContact;
		ccs.wParam = 0;
		ccs.lParam = (LPARAM)pre;
		CallServiceSync(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
		free(pre->szMessage);
		return TRUE;
	}
	return FALSE;
}

typedef struct {
	BOOL bStatus;
	char szNum[16];
	char szArg[32];
} ft_args;

void FXHandleMessageThread(ft_args *pargs)
{
	char *pszChat;
	DWORD dwChat;
	TYP_MSGLENTRY *pEntry;
	MCONTACT hContact;

	if (!(pszChat=SkypeGetErr("FILETRANSFER", pargs->szNum, "CHATMESSAGE")) ||
		!(pEntry = MsgList_FindMessage(dwChat = strtoul(pszChat, NULL, 10))) || 
		!(hContact = ((PROTOFILETRANSFERSTATUS_V1*)pEntry->pfts)->hContact)) 
	{
		free(pszChat);
		free(pargs);
		return;
	}
	if (pargs->bStatus) {
		if (!strcmp(pargs->szArg, "CONNECTING"))
			ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)dwChat, 0);
		else if (!strncmp(pargs->szArg, "TRANSFERRING", 12))
			ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)dwChat, 0);
		else if (!strcmp(pargs->szArg, "FAILED"))
			ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)dwChat, 0);
		else if (!strcmp(pargs->szArg, "CANCELLED"))
			ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)dwChat, 0);
		else if (!strcmp(pargs->szArg, "COMPLETED")) {
			// Check if all transfers from this message are completed.
			char *pszXferIDs, *pszMsgNum, *pszStatus;
			BOOL bAllComplete=TRUE;

			if ((pszXferIDs = SkypeGetErr("CHATMESSAGE", pszChat, "FILETRANSFERS")))  {
				for (pszMsgNum = strtok(pszXferIDs, ", "); pszMsgNum; pszMsgNum = strtok(NULL, ", ")) {
					if (pszStatus=SkypeGetErrID("FILETRANSFER", pszMsgNum, "STATUS")) {
						if (strcmp(pszStatus, "COMPLETED")) bAllComplete=FALSE;
						free(pszStatus);
						if (!bAllComplete) break;
					}
				}
				free(pszXferIDs);
				if (bAllComplete) {
					ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)dwChat, 0);
					// We could free pEntry at this point, but Garbage Collector will take care of it anyway
				}
			}
		}
	} else {
		// BYTESTRANSFERRED
		PROTOFILETRANSFERSTATUS_V1 pfts={0};
		char *pszXferIDs, *pszMsgNum;
		int i;

		// This always needs some fetching to fill PFTS :/
		if ((pszXferIDs = SkypeGetErr("CHATMESSAGE", pszChat, "FILETRANSFERS")))  {
			for (pszMsgNum = strtok(pszXferIDs, ", "),i=0; pszMsgNum; pszMsgNum = strtok(NULL, ", "),i++) {
				char *pszcbFile;
				DWORD dwTransferred;
				BOOL bIsCurFil = strcmp(pargs->szNum,pszMsgNum)==0;

				if (bIsCurFil) pfts.currentFileNumber = i;
				if (pszcbFile = SkypeGetErr("FILETRANSFER", pszMsgNum, "FILESIZE")) {
					dwTransferred = strtoul(pszcbFile, NULL, 10);
					pfts.totalBytes+=dwTransferred;
					if (bIsCurFil) pfts.currentFileSize=dwTransferred;
					free(pszcbFile);
				}
				if (pszcbFile = SkypeGetErrID("FILETRANSFER", pszMsgNum, "BYTESTRANSFERRED")) {
					dwTransferred = strtoul(pszcbFile, NULL, 10);
					pfts.totalProgress+=dwTransferred;
					if (bIsCurFil) pfts.currentFileProgress=dwTransferred;
					free(pszcbFile);
				}
			}
			free(pszXferIDs);
			if (mirandaVersion < 0x090000) {
				PROTOFILETRANSFERSTATUS_V1 *pftsv1 = (PROTOFILETRANSFERSTATUS_V1*)pEntry->pfts;

				pftsv1->currentFileNumber = pfts.currentFileNumber;
				pftsv1->totalBytes = pfts.totalBytes;
				pftsv1->totalProgress = pfts.totalProgress;
				pftsv1->currentFileSize = pfts.currentFileSize;
				pftsv1->currentFileProgress = pfts.currentFileProgress;
				pftsv1->currentFile = pftsv1->files[pftsv1->currentFileNumber];
			} else {
				PROTOFILETRANSFERSTATUS_V2 *pftsv2 = (PROTOFILETRANSFERSTATUS_V2*)pEntry->pfts;

				pftsv2->currentFileNumber = pfts.currentFileNumber;
				pftsv2->totalBytes = pfts.totalBytes;
				pftsv2->totalProgress = pfts.totalProgress;
				pftsv2->currentFileSize = pfts.currentFileSize;
				pftsv2->currentFileProgress = pfts.currentFileProgress;
				pftsv2->szCurrentFile = pftsv2->pszFiles[pftsv2->currentFileNumber];
			}
			ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)dwChat, (LPARAM)pEntry->pfts);
		}
	}
	free(pszChat);
	free(pargs);
}

BOOL FXHandleMessage(const char *pszMsg)
{
	const char *pTok;
	ft_args args={0}, *pargs;

	if (!(pTok=strchr(pszMsg, ' '))) return FALSE;
	strncpy(args.szNum, pszMsg, pTok-pszMsg);
	pszMsg=pTok+1;
	if (!(pTok=strchr(pszMsg, ' '))) return FALSE;
	pTok++;
	if (!(args.bStatus=!strncmp(pszMsg, "STATUS", 6)) && strncmp(pszMsg, "BYTESTRANSFERRED", 16))
		return FALSE;
	if (!(pargs=(ft_args*)malloc(sizeof(args)))) return TRUE;
	strncpy(args.szArg, pTok, sizeof(args.szArg));
	memcpy(pargs, &args, sizeof(args));
	pthread_create((pThreadFunc)FXHandleMessageThread, pargs);
	return TRUE;
}

