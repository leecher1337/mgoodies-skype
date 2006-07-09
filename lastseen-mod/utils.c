#include "seen.h"
#include <m_ignore.h>



void FileWrite(HANDLE);
void HistoryWrite(HANDLE hcontact);
//void SetOffline(void);
void ShowHistory(HANDLE hContact, BYTE isAlert);

char * courProtoName = 0;

//copied from ..\..\miranda32\protocols\protocols\protocols.c
PROTOCOLDESCRIPTOR* Proto_IsProtocolLoaded(const char* szProto)
{
	return (PROTOCOLDESCRIPTOR*) CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)szProto);
}


/*
Returns true if the protocols is to be monitored
*/
int IsWatchedProtocol(const char* szProto)
{
	DBVARIANT dbv;
	char *szProtoPointer, *szWatched;
	int iProtoLen, iWatchedLen;
	int retval = 0;
	PROTOCOLDESCRIPTOR *pd;

	if (szProto == NULL)
		return 0;
	
	pd=Proto_IsProtocolLoaded(szProto);
	if (pd==NULL || pd->type!=PROTOTYPE_PROTOCOL || CallProtoService(pd->szName,PS_GETCAPS,PFLAGNUM_2,0)==0)
		return 0;

	iProtoLen = strlen(szProto);
	if(DBGetContactSetting(NULL, S_MOD, "WatchedProtocols", &dbv))
		szWatched = DEFAULT_WATCHEDPROTOCOLS;
	else
		szWatched = dbv.pszVal;
	iWatchedLen = strlen(szWatched);

	if (*szWatched == '\0') 
	{
		retval=1; //empty string: all protocols are watched
	} 
	else 
	{
		char sTemp [MAXMODULELABELLENGTH+1]="";
		strcat(sTemp,szProto);
		strcat(sTemp," ");
		szProtoPointer = strstr(szWatched, sTemp);
		if (szProtoPointer == NULL)
			retval=0;
		else 
			retval=1;
	}

	DBFreeVariant(&dbv);
	return retval;
}

BOOL isYahoo(char * protoname){
	if (protoname) {
		char *pszUniqueSetting = (char*)CallProtoService(protoname, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (pszUniqueSetting){
			return (!strcmp(pszUniqueSetting,"yahoo_id"));
	}	}
	return FALSE;
}
BOOL isJabber(char * protoname){
	if (protoname) {
		char *pszUniqueSetting = (char*)CallProtoService(protoname, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (pszUniqueSetting){
			return (!strcmp(pszUniqueSetting,"jid"));
	}	}
	return FALSE;
}
BOOL isICQ(char * protoname){
	if (protoname) {
		char *pszUniqueSetting = (char*)CallProtoService(protoname, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (pszUniqueSetting){
			return (!strcmp(pszUniqueSetting,"UIN"));
	}	}
	return FALSE;
}
BOOL isMSN(char * protoname){
	if (protoname) {
		char *pszUniqueSetting = (char*)CallProtoService(protoname, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (pszUniqueSetting){
			return (!strcmp(pszUniqueSetting,"e-mail"));
	}	}
	return FALSE;
}

char *ParseString(char *szstring,HANDLE hcontact,BYTE isfile)
{
#define MAXSIZE 1024
	static char sztemp[MAXSIZE+1];
	int sztemplen = 0;
	char szdbsetting[128]="";
	char *charPtr;
	UINT loop=0;
	int isetting=0;
	DWORD dwsetting=0;
	struct in_addr ia;
	char *weekdays[]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
	char *wdays_short[]={"Sun.","Mon.","Tue.","Wed.","Thu.","Fri.","Sat."};
	char *monthnames[]={"January","February","March","April","May","June","July","August","September","October","November","December"};
	char *mnames_short[]={"Jan.","Feb.","Mar.","Apr.","May","Jun.","Jul.","Aug.","Sep.","Oct.","Nov.","Dec."};
	CONTACTINFO ci;
	BOOL wantempty;

	ci.cbSize=sizeof(CONTACTINFO);
	ci.hContact=hcontact;
	ci.szProto=hcontact?(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hcontact,0):courProtoName;
	sztemp[0] = '\0';
	for(;loop<strlen(szstring);loop++)
	{
		if (sztemplen == MAXSIZE) break;
		if((szstring[loop]!='%')&(szstring[loop]!='#'))
		{
			strncat(sztemp,szstring+loop,1);
			sztemplen++;
			continue;
		}

		else
		{
			wantempty = (szstring[loop]=='#');
			switch(szstring[++loop]){
				case 'Y':
					if (!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Year",0))) goto LBL_noData;
					sztemplen += mir_snprintf(sztemp+sztemplen,MAXSIZE-sztemplen,"%04i",isetting);
					break;

				case 'y':
					if (!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Year",0))) goto LBL_noData;
					wsprintf(szdbsetting,"%04i",isetting);
					sztemplen += mir_snprintf(sztemp+sztemplen,MAXSIZE-sztemplen,"%s",szdbsetting+2);
					break;

				case 'm':
					if (!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Month",0))) goto LBL_noData;
LBL_2DigNum:
					sztemplen += mir_snprintf(sztemp+sztemplen,MAXSIZE-sztemplen,"%02i",isetting);
					break;

				case 'd':
					if (isetting=DBGetContactSettingWord(hcontact,S_MOD,"Day",0)) goto LBL_2DigNum;
					else goto LBL_noData;

				case 'W':
					isetting=DBGetContactSettingWord(hcontact,S_MOD,"WeekDay",-1);
					if(isetting==-1){
LBL_noData:
						charPtr = wantempty?"":Translate("<unknown>");
						goto LBL_charPtr;
					}
					charPtr = Translate(weekdays[isetting]);
LBL_charPtr:
					sztemplen += mir_snprintf(sztemp+sztemplen,MAXSIZE-sztemplen,"%s",charPtr);
					break;

				case 'w':
					isetting=DBGetContactSettingWord(hcontact,S_MOD,"WeekDay",-1);
					if(isetting==-1)goto LBL_noData;
					charPtr = Translate(wdays_short[isetting]);
					goto LBL_charPtr;

				case 'E':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Month",0)))goto LBL_noData;
					charPtr = Translate(monthnames[isetting-1]);
					goto LBL_charPtr;

				case 'e':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Month",0)))goto LBL_noData;
					charPtr = Translate(mnames_short[isetting-1]);
					goto LBL_charPtr;

				case 'H':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Hours",-1))==-1)goto LBL_noData;
					goto LBL_2DigNum;

				case 'h':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Hours",-1))==-1)goto LBL_noData;
					if(!isetting) isetting=12;
					isetting = isetting-((isetting>12)?12:0);
					goto LBL_2DigNum;

				case 'p':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Hours",-1))==-1)goto LBL_noData;
					charPtr = (isetting>12)?"PM":"AM";
					goto LBL_charPtr;

				case 'M':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Minutes",-1))==-1)goto LBL_noData;
					goto LBL_2DigNum;

				case 'S':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Seconds",-1))==-1)goto LBL_noData;
					goto LBL_2DigNum;

				case 'n':
					charPtr = hcontact?(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hcontact,0):(wantempty?"":"---");
					goto LBL_charPtr;
				case 'N':
					ci.dwFlag=CNF_NICK;
					if(!CallService(MS_CONTACT_GETCONTACTINFO,(WPARAM)0,(LPARAM)&ci)){
						charPtr = ci.pszVal;
					} else goto LBL_noData;
					goto LBL_charPtr;
				case 'G':
					{
						DBVARIANT dbv;
						if (!DBGetContactSetting(hcontact,"CList","Group",&dbv)){
							strcpy(szdbsetting,dbv.pszVal);
							DBFreeVariant(&dbv);
							charPtr = szdbsetting;
							goto LBL_charPtr;
						} else; //do nothing
					}
					break;

				case 'u':
					ci.dwFlag=CNF_UNIQUEID;
					if(!CallService(MS_CONTACT_GETCONTACTINFO,(WPARAM)0,(LPARAM)&ci))
					{
						switch(ci.type)
						{
							case CNFT_BYTE:
								ltoa(ci.bVal,szdbsetting,10);
								break;
							case CNFT_WORD:
								ltoa(ci.wVal,szdbsetting,10);
								break;
							case CNFT_DWORD:
								ltoa(ci.dVal,szdbsetting,10);
								break;
							case CNFT_ASCIIZ:
								strcpy(szdbsetting,ci.pszVal);
								break;
						}

					}
					else if (ci.szProto != NULL) 
					{
						if (isYahoo(ci.szProto)) // YAHOO support
						{
							DBVARIANT dbv;
							DBGetContactSetting(hcontact,ci.szProto,"id",&dbv);
							strcpy(szdbsetting,dbv.pszVal);
							DBFreeVariant(&dbv);
						}
						else if (isJabber(ci.szProto)) // JABBER support
						{
							DBVARIANT dbv;
							if (DBGetContactSetting(hcontact,ci.szProto,"LoginName",&dbv)) goto LBL_noData;
							strcpy(szdbsetting,dbv.pszVal);
							DBFreeVariant(&dbv);
							DBGetContactSetting(hcontact,ci.szProto,"LoginServer",&dbv);
							strcat(szdbsetting,"@");
							strcat(szdbsetting,dbv.pszVal);
							DBFreeVariant(&dbv);
						} else goto LBL_noData;
					}
					else goto LBL_noData;
					charPtr = szdbsetting;
					goto LBL_charPtr;

				case 's':
					if (isetting=DBGetContactSettingWord(hcontact,S_MOD,hcontact?"Status":courProtoName,0)){
						strcpy(szdbsetting,Translate((const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)(isetting|0x8000),0)));
						if (!(isetting&0x8000)){
							strcat(szdbsetting,"/");
							strcat(szdbsetting,Translate("Idle"));
						}
						charPtr = szdbsetting;
					} else goto LBL_noData;
					goto LBL_charPtr;
				case 'T':
					{
						DBVARIANT dbv;
						if (!DBGetContactSetting(hcontact,"CList","StatusMsg",&dbv)){
							sztemplen += mir_snprintf(sztemp+sztemplen,MAXSIZE-sztemplen,"%s",dbv.pszVal);
							DBFreeVariant(&dbv);
						} else goto LBL_noData;
					}
					break;
				case 'o':
					if (isetting=DBGetContactSettingWord(hcontact,S_MOD,hcontact?"OldStatus":courProtoName,0)){
						strcpy(szdbsetting,Translate((const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)isetting|0x8000,0)));
						if (!(isetting&0x8000)){
							strcat(szdbsetting,"/");
							strcat(szdbsetting,Translate("Idle"));
						}
						charPtr = szdbsetting;
					} else goto LBL_noData;
					goto LBL_charPtr;

				case 'i':
				case 'r': if (isJabber(ci.szProto)){
							DBVARIANT dbv;
							if (!DBGetContactSetting(hcontact,ci.szProto,szstring[loop]=='i'?"Resource":"System",&dbv)){
								strcpy(szdbsetting,dbv.pszVal);
								DBFreeVariant(&dbv);
								charPtr = szdbsetting;
							} else goto LBL_noData;
						  } else {
							dwsetting=DBGetContactSettingDword(hcontact,ci.szProto,szstring[loop]=='i'?"IP":"RealIP",0);
							if(dwsetting){
								ia.S_un.S_addr=htonl(dwsetting);
								charPtr = inet_ntoa(ia);
							} else goto LBL_noData;
						  }
					goto LBL_charPtr;
				case 'P':if (ci.szProto) charPtr = ci.szProto; else charPtr = wantempty?"":"ProtoUnknown";
					goto LBL_charPtr;
				case 'b':
					charPtr = /*"\n"*/"\x0D\x0A";
					goto LBL_charPtr;
				case 'C': // Get Client Info
					if (isMSN(ci.szProto)) {
						if (hcontact) {
							dwsetting = (int)DBGetContactSettingDword(hcontact,ci.szProto,"FlagBits",0);
							wsprintf(szdbsetting,"MSNC%i",(dwsetting&0x70000000)>>28);
							if (dwsetting & 0x00000001) strcat(szdbsetting," MobD"); //Mobile Device
							if (dwsetting & 0x00000004) strcat(szdbsetting," InkG"); //GIF Ink Send/Receive
							if (dwsetting & 0x00000008) strcat(szdbsetting," InkI"); //ISF Ink Send/Receive
							if (dwsetting & 0x00000010) strcat(szdbsetting," WCam"); //Webcam
							if (dwsetting & 0x00000020) strcat(szdbsetting," MPkt"); //Multi packet messages
							if (dwsetting & 0x00000040) strcat(szdbsetting," SMSr"); //Paging
							if (dwsetting & 0x00000080) strcat(szdbsetting," DSMS"); //Using MSN Direct
							if (dwsetting & 0x00000200) strcat(szdbsetting," WebM"); //WebMessenger
							if (dwsetting & 0x00001000) strcat(szdbsetting," MS7+"); //Unknown (Msgr 7 always[?] sets it)
							if (dwsetting & 0x00004000) strcat(szdbsetting," DirM"); //DirectIM
							if (dwsetting & 0x00008000) strcat(szdbsetting," Wink"); //Send/Receive Winks
							if (dwsetting & 0x00010000) strcat(szdbsetting," MSrc"); //MSN Search ??
							if (dwsetting & 0x00040000) strcat(szdbsetting," VoiC"); //Voice Clips
						} else strcpy(szdbsetting,"Miranda");
					} else {
						DBVARIANT dbv;
						if (!DBGetContactSetting(hcontact,ci.szProto,"MirVer",&dbv)){
							strcpy(szdbsetting,dbv.pszVal);
							DBFreeVariant(&dbv);
						} else goto LBL_noData;
					}
					charPtr = szdbsetting;
					goto LBL_charPtr;
				case 't':
					charPtr = "\t";
					goto LBL_charPtr;

				default:
					strncpy(szdbsetting,szstring+loop-1,2);
					goto LBL_charPtr;
			}
		}
	}

	return sztemp;
}



void DBWriteTime(SYSTEMTIME *st,HANDLE hcontact)
{
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Day",st->wDay);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Month",st->wMonth);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Year",st->wYear);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Hours",st->wHour);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Minutes",st->wMinute);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Seconds",st->wSecond);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"WeekDay",st->wDayOfWeek);

}

void GetColorsFromDWord(LPCOLORREF First, LPCOLORREF Second, DWORD colDword){
	WORD temp;
	COLORREF res=0;
	temp = (WORD)(colDword>>16);
	res |= ((temp & 0x1F) <<3);
	res |= ((temp & 0x3E0) <<6);
	res |= ((temp & 0x7C00) <<9);
	if (First) *First = res;
	res = 0;
	temp = (WORD)colDword;
	res |= ((temp & 0x1F) <<3);
	res |= ((temp & 0x3E0) <<6);
	res |= ((temp & 0x7C00) <<9);
	if (Second) *Second = res;
}

DWORD StatusColors15bits[] = {
	0x63180000, // 0x00C0C0C0, 0x00000000, Offline - LightGray
	0x7B350000, // 0x00F0C8A8, 0x00000000, Online  - LightBlue
	0x33fe0000, // 0x0070E0E0, 0x00000000, Away -LightOrange
	0x295C0000, // 0x005050E0, 0x00000000, DND  -DarkRed
	0x5EFD0000, // 0x00B8B8E8, 0x00000000, NA   -LightRed
	0x295C0000, // 0x005050E0, 0x00000000, Occupied
	0x43900000, // 0x0080E080, 0x00000000, Free for chat - LightGreen
	0x76AF0000, // 0x00E8A878, 0x00000000, Invisible
	0x431C0000, // 0x0080C0E0, 0x00000000, On the phone
	0x5EFD0000, // 0x00B8B8E8, 0x00000000, Out to lunch
};

DWORD GetDWordFromColors(COLORREF First, COLORREF Second){
	DWORD res = 0;
	res |= (First&0xF8)>>3;
	res |= (First&0xF800)>>6;
	res |= (First&0xF80000)>>9;
	res <<= 16;
	res |= (Second&0xF8)>>3;
	res |= (Second&0xF800)>>6;
	res |= (Second&0xF80000)>>9;
	return res;
}

LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch(message) {
		case WM_COMMAND: 
			if (HIWORD(wParam) == STN_CLICKED){
				HANDLE hContact = PUGetContact(hwnd);
				if (hContact > 0) CallService(MS_MSG_SENDMESSAGE,(WPARAM)hContact,0);
			}
		case WM_CONTEXTMENU:
			PUDeletePopUp(hwnd);
			break;
		case UM_INITPOPUP: return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
};

void ShowPopup(HANDLE hcontact, const char * lpzProto, int newStatus){
	if (ServiceExists(MS_POPUP_QUERY)){
		if (DBGetContactSettingByte(NULL,S_MOD,"UsePopups",0)){
			if (!DBGetContactSettingByte(hcontact,"CList","Hidden",0)){
				POPUPDATAEX ppd = {0};
				DBVARIANT dbv = {0};
				char szstamp[10];
				DWORD sett;
				sprintf(szstamp, "Col_%d",newStatus-ID_STATUS_OFFLINE);
				sett = DBGetContactSettingDword(NULL,S_MOD,szstamp,StatusColors15bits[newStatus-ID_STATUS_OFFLINE]);
				GetColorsFromDWord(&ppd.colorBack,&ppd.colorText,sett);
				ppd.lchContact = hcontact;
				ppd.lchIcon = LoadSkinnedProtoIcon(lpzProto, newStatus);
				strncpy(ppd.lpzContactName,ParseString(!DBGetContactSetting(NULL,S_MOD,"PopupStamp",&dbv)?dbv.pszVal:DEFAULT_POPUPSTAMP,hcontact,0),MAX_CONTACTNAME);
				DBFreeVariant(&dbv);
				strncpy(ppd.lpzText,ParseString(!DBGetContactSetting(NULL,S_MOD,"PopupStampText",&dbv)?dbv.pszVal:DEFAULT_POPUPSTAMPTEXT,hcontact,0),MAX_SECONDLINE);
				DBFreeVariant(&dbv);
				ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
				CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
			}
		}
	}
}

typedef struct logthread_info {
  char sProtoName[MAXMODULELABELLENGTH];
  HANDLE hContact;
  WORD courStatus;
  int queueIndex;
} logthread_info;

//will give hContact position or zero
int isContactQueueActive(HANDLE hContact){
	int i = 0;
	if (!hContact) {
		MessageBox(0,"Is myself to queue: never","LastSeen-Mod",0);
		return 0;
	}
	for (i=1;i<contactQueueSize;i++){
		if (contactQueue[i]==hContact) return (i);
	}
	return 0;
}

//will add hContact to queue and will return position;
int addContactToQueue(HANDLE hContact){
	int i = 0;
	if (!hContact) {
		MessageBox(0,"Adding myself to queue","LastSeen-Mod",0);
		return 0;
	}
	for (i=1;i<contactQueueSize;i++){
		if (!contactQueue[i]) {
			contactQueue[i] = hContact;
			return (i);
		}
	}
	//no free space. Create some
	MessageBox(0,"Creating more space","LastSeen-Mod",0);
	contactQueue = (HANDLE *)realloc(contactQueue,(contactQueueSize+16)*sizeof(contactQueue[0]));
	ZeroMemory(&contactQueue[contactQueueSize], 16*sizeof(contactQueue[0]));
	contactQueue[contactQueueSize] = hContact;
	contactQueueSize += 16;
	return 0;
}

static DWORD __stdcall waitThread(logthread_info* infoParam)
{
//	char str[MAXMODULELABELLENGTH];
//	sprintf(str,"In Thread: %s; %s; %s\n",
//		infoParam->sProtoName,
//		(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)infoParam->hContact,0),
//		(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)infoParam->courStatus,0)
//	);
//	OutputDebugStringA(str);
	Sleep(1500); // I hope in 1.5 second all the needed info will be set
	if (includeIdle){
		if (DBGetContactSettingDword(infoParam->hContact,infoParam->sProtoName,"IdleTS",0)) {
			infoParam->courStatus &=0x7FFF;
		}
	}
	DBWriteContactSettingWord(infoParam->hContact,S_MOD,"Status",infoParam->courStatus);
//	sprintf(str,"OutThread: %s; %s; %s\n",
//		infoParam->sProtoName,
//		(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)infoParam->hContact,0),
//		(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)infoParam->courStatus,0)
//	);
	contactQueue[infoParam->queueIndex] = 0;
	free(infoParam);
//	OutputDebugStringA(str);
	return 0;
}

#ifndef PERMITNSN
static int uniqueEventId=0;
#endif

int UpdateValues(HANDLE hContact,LPARAM lparam)
{
	DBCONTACTWRITESETTING *cws;
	SYSTEMTIME time;
	BOOL isIdleEvent;
	// to make this code faster
	if (!hContact) return 0;
	cws=(DBCONTACTWRITESETTING *)lparam;
	if(CallService(MS_IGNORE_ISIGNORED,(WPARAM)hContact,IGNOREEVENT_USERONLINE)) return 0;
	isIdleEvent = includeIdle?(strcmp(cws->szSetting,"IdleTS")==0):0;
	if (strcmp(cws->szSetting,"Status") && (isIdleEvent==0)) return 0;
	if (!strcmp(cws->szModule,S_MOD)){
		//here we will come when Settings/SeenModule/Status is changed
		WORD prevStatus=DBGetContactSettingWord(hContact,S_MOD,"OldStatus",ID_STATUS_OFFLINE);
		if((cws->value.wVal|0x8000)<=ID_STATUS_OFFLINE)
		{
			char * proto;
			// avoid repeating the offline status
			if ((cws->value.wVal|0x8000)<=ID_STATUS_OFFLINE) 
				return 0;
			proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
			DBWriteContactSettingByte(hContact,S_MOD,"Offline",1);
			{
				DWORD t;
				char *str = malloc(MAXMODULELABELLENGTH+9);
				mir_snprintf(str,MAXMODULELABELLENGTH+8,"OffTime-%s",proto);
				if (t = DBGetContactSettingDword(NULL,S_MOD,str,0)){
					FILETIME ft;
					LONGLONG ll = UInt32x32To64(CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,t,0), 10000000) + NUM100NANOSEC;
					ft.dwLowDateTime = (DWORD)ll;
					ft.dwHighDateTime = (DWORD)(ll >> 32);
					FileTimeToSystemTime(&ft, &time);
				} else GetLocalTime(&time);
				free(str);
			}
			DBWriteTime(&time,hContact);

			if(!DBGetContactSettingByte(NULL,S_MOD,"IgnoreOffline",1))
			{
				char * sProto;
				if(DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0))
					FileWrite(hContact);

				if (CallProtoService(sProto = 
					(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0),
					PS_GETSTATUS,0,0
					)>ID_STATUS_OFFLINE)	{
					if(DBGetContactSettingByte(NULL,S_MOD,"UsePopups",0)){
						ShowPopup(hContact,sProto,ID_STATUS_OFFLINE);
				}	}

				if(DBGetContactSettingByte(NULL,S_MOD,"KeepHistory",0))
					HistoryWrite(hContact);

				if(DBGetContactSettingByte(hContact,S_MOD,"OnlineAlert",0)) 
					ShowHistory(hContact, 1);
			}

		} else {

			if(cws->value.wVal==prevStatus && !DBGetContactSettingByte(hContact,S_MOD,"Offline",0)) 
				return 0;

			GetLocalTime(&time);
			DBWriteTime(&time,hContact);

			DBWriteContactSettingWord(hContact,S_MOD,"Status",(WORD)cws->value.wVal);

			if(DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0)) FileWrite(hContact);
			if(DBGetContactSettingByte(NULL,S_MOD,"UsePopups",0))
				if (prevStatus != cws->value.wVal) ShowPopup(hContact,(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0),cws->value.wVal|0x8000);

			if(DBGetContactSettingByte(NULL,S_MOD,"KeepHistory",0)) HistoryWrite(hContact);
			if(DBGetContactSettingByte(hContact,S_MOD,"OnlineAlert",0)) ShowHistory(hContact, 1);
			DBWriteContactSettingByte(hContact,S_MOD,"Offline",0);
		}
	} else if (IsWatchedProtocol(cws->szModule)){
		//here we will come when <User>/<module>/Status is changed or it is idle event and if <module> is watched
		if (CallProtoService(cws->szModule,PS_GETSTATUS,0,0)>ID_STATUS_OFFLINE){
			if (!isContactQueueActive(hContact)){
				logthread_info *info;
				WORD prevStatus;
				info = (logthread_info *)malloc(sizeof(logthread_info));
				strncpy(info->sProtoName,cws->szModule,MAXMODULELABELLENGTH);
				info->hContact = hContact;
				info->queueIndex = addContactToQueue(hContact);
				info->courStatus = isIdleEvent?DBGetContactSettingWord(hContact,cws->szModule,"Status",ID_STATUS_OFFLINE):cws->value.wVal;
				forkthreadex(NULL, 0, waitThread, info, 0, 0);
				prevStatus=DBGetContactSettingWord(hContact,S_MOD,"Status",ID_STATUS_OFFLINE);
				DBWriteContactSettingWord(hContact,S_MOD,"OldStatus",prevStatus);
	}	}	}
#ifndef PERMITNSN
	//Some useronline.c functionality
	{
		int newStatus,oldStatus;
		newStatus=(cws->value.wVal|0x8000);
		oldStatus=DBGetContactSettingWord(hContact,"UserOnline","OldStatus",ID_STATUS_OFFLINE);
		DBWriteContactSettingWord(hContact,"UserOnline","OldStatus",(WORD)newStatus);
		if(DBGetContactSettingByte(hContact,"CList","Hidden",0)) return 0;
		if((newStatus==ID_STATUS_ONLINE || newStatus==ID_STATUS_FREECHAT) &&
		   oldStatus!=ID_STATUS_ONLINE && oldStatus!=ID_STATUS_FREECHAT) {
			BYTE supp = db_byte_get(NULL, S_MOD, "SuppCListOnline", 3); //By default no online allert :P
			BOOL willAlert = FALSE;
			switch (supp) {
			case 3: willAlert = FALSE; break;
			case 2: willAlert = !IsWatchedProtocol(cws->szModule); break;
			case 1: willAlert = IsWatchedProtocol(cws->szModule); break;
			case 0: willAlert = TRUE; break;
			}
			if (willAlert) {
				DWORD ticked = db_dword_get(NULL, "UserOnline", cws->szModule, GetTickCount());
				// only play the sound (or show event) if this event happens at least 10 secs after the proto went from offline
				if ( GetTickCount() - ticked > (1000*10) ) { 
					CLISTEVENT cle;
					char tooltip[256];

					ZeroMemory(&cle,sizeof(cle));
					cle.cbSize=sizeof(cle);
					cle.flags=CLEF_ONLYAFEW;
					cle.hContact=hContact;
					cle.hDbEvent=(HANDLE)(uniqueEventId++);
					cle.hIcon=LoadSkinnedIcon(SKINICON_OTHER_USERONLINE);
					cle.pszService="UserOnline/Description";
					mir_snprintf(tooltip,256,Translate("%s is Online"),(char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0));
					cle.pszTooltip=tooltip;
					CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);

					SkinPlaySound("UserOnline");
				}
			}
		}
	}
#endif
	return 0;
}

static DWORD __stdcall cleanThread(logthread_info* infoParam)
{
	HANDLE hcontact=NULL;
//	char str[MAXMODULELABELLENGTH];
//	sprintf(str,"In Clean: %s; %s; %s\n",
//		infoParam->sProtoName,
//		(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)infoParam->hContact,0),
//		(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)infoParam->courStatus,0)
//	);
//	OutputDebugStringA(str);
	Sleep(10000); // I hope in 10 secons all logged-in contacts will be listed
	//Searching for contact marked as online but now are offline

	hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hcontact!=NULL)
	{
		char * contactProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hcontact,0);
		if (contactProto) {
			if (!strcmp(infoParam->sProtoName,contactProto)){
				WORD oldStatus;
				if ((oldStatus = DBGetContactSettingWord(hcontact,S_MOD,"Status",ID_STATUS_OFFLINE))>ID_STATUS_OFFLINE){
					if (DBGetContactSettingWord(hcontact,contactProto,"Status",ID_STATUS_OFFLINE)==ID_STATUS_OFFLINE){
						DBWriteContactSettingWord(hcontact,S_MOD,"OldStatus",oldStatus);
						DBWriteContactSettingWord(hcontact,S_MOD,"Status",ID_STATUS_OFFLINE);
					}
				}
			}
		}
		hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hcontact,0);
	}

//	sprintf(str,"OutClean: %s; %s; %s\n",
//		infoParam->sProtoName,
//		(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)infoParam->hContact,0),
//		(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)infoParam->courStatus,0)
//	);
	{
		char *str = malloc(MAXMODULELABELLENGTH+9);
		mir_snprintf(str,MAXMODULELABELLENGTH+8,"OffTime-%s",infoParam->sProtoName);
		DBDeleteContactSetting(NULL,S_MOD,str);
		free(str);
	}
	free(infoParam);
//	OutputDebugStringA(str);
	return 0;
}


int ModeChange(WPARAM wparam,LPARAM lparam)
{
	ACKDATA *ack;
	WORD isetting=0;
	SYSTEMTIME tm;

	ack=(ACKDATA *)lparam;

	if(ack->type!=ACKTYPE_STATUS || ack->result!=ACKRESULT_SUCCESS || ack->hContact!=NULL) return 0;
	courProtoName = (char *)ack->szModule;
	if (!IsWatchedProtocol(courProtoName)) return 0;
	GetLocalTime(&tm);

	DBWriteTime(&tm,NULL);

//	isetting=CallProtoService(ack->szModule,PS_GETSTATUS,0,0);
	isetting=(WORD)ack->lParam;
	if (isetting<ID_STATUS_OFFLINE) isetting = ID_STATUS_OFFLINE;
	if ((isetting>ID_STATUS_OFFLINE)&&((WORD)ack->hProcess<=ID_STATUS_OFFLINE)){
		//we have just loged-in
		db_dword_set(NULL, "UserOnline", ack->szModule, GetTickCount());
		if (IsWatchedProtocol(ack->szModule)){
			logthread_info *info;
			info = (logthread_info *)malloc(sizeof(logthread_info));
			strncpy(info->sProtoName,courProtoName,MAXMODULELABELLENGTH);
			info->hContact = 0;
			info->courStatus = 0;
			forkthreadex(NULL, 0, cleanThread, info, 0, 0);
		}
	} else if ((isetting==ID_STATUS_OFFLINE)&&((WORD)ack->hProcess>ID_STATUS_OFFLINE)){
		//we have just loged-off
		if (IsWatchedProtocol(ack->szModule)){
			char *str = malloc(MAXMODULELABELLENGTH+9);
			DWORD t;
			time(&t);
			mir_snprintf(str,MAXMODULELABELLENGTH+8,"OffTime-%s",ack->szModule);
			DBWriteContactSettingDword(NULL,S_MOD,str,t);
			free(str);
	}	}
	if (isetting==DBGetContactSettingWord(NULL,S_MOD,courProtoName,ID_STATUS_OFFLINE)) return 0;
	DBWriteContactSettingWord(NULL,S_MOD,courProtoName,isetting);

	// log "myself"
	if(DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0))
		FileWrite(NULL);

//	if(isetting==ID_STATUS_OFFLINE) //this is removed 'cause I want other contacts to be logged only if the status changed while I was offline
//		SetOffline();

	courProtoName = NULL;

	return 0;
}



/*int GetInfoAck(WPARAM wparam,LPARAM lparam)
{
	ACKDATA *ack;
	DWORD dwsetting=0;

	ack=(ACKDATA *)lparam;

	if(ack->type!=ACKTYPE_GETINFO || ack->hContact==NULL) return 0;
	if(((int)ack->hProcess-1)!=(int)ack->lParam) return 0;
	
	dwsetting=DBGetContactSettingDword(ack->hContact,ack->szModule,"IP",0);
	if(dwsetting)
		DBWriteContactSettingDword(ack->hContact,S_MOD,"IP",dwsetting);

	dwsetting=DBGetContactSettingDword(ack->hContact,ack->szModule,"RealIP",0);
	if(dwsetting)
		DBWriteContactSettingDword(ack->hContact,S_MOD,"RealIP",dwsetting);

	return 0;
}*/



/*void SetOffline(void)
{
	HANDLE hcontact=NULL;
	char * szProto;

	hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hcontact!=NULL)
	{
		szProto=(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hcontact,0);
		if (szProto != NULL && IsWatchedProtocol(szProto)) {	
			DBWriteContactSettingByte(hcontact,S_MOD,"Offline",1);
		}
		hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hcontact,0);
	}
}*/



