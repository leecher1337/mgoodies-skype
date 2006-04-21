#include "seen.h"



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
	return ((strstr(protoname,"YAHOO") > 0) || (strstr(protoname,"yahoo") > 0));
}
BOOL isJabber(char * protoname){
	return ((strstr(protoname,"JABBER") > 0));
}
BOOL isICQ(char * protoname){
	return ((strstr(protoname,"ICQ") > 0));
}
BOOL isMSN(char * protoname){
	return ((strstr(protoname,"MSN") > 0));
}

char *ParseString(char *szstring,HANDLE hcontact,BYTE isfile)
{
	static char sztemp[1024];
	char szdbsetting[128]="";
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
	*sztemp = '\0';
	
	for(;loop<strlen(szstring);loop++)
	{
		if((szstring[loop]!='%')&(szstring[loop]!='#'))
		{
			strncat(sztemp,szstring+loop,1);
			continue;
		}

		else
		{
			wantempty = (szstring[loop]=='#');
			switch(szstring[++loop]){
				case 'Y':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Year",0)))
						return wantempty?"":Translate("<unknown>");
					wsprintf(szdbsetting,"%04i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'y':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Year",0)))
						return wantempty?"":Translate("<unknown>");
					wsprintf(szdbsetting,"%04i",isetting);
					strcat(sztemp,szdbsetting+2);
					break;

				case 'm':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Month",0)))
						return wantempty?"":Translate("<unknown>");
					wsprintf(szdbsetting,"%02i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'd':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Day",0)))
						return wantempty?"":Translate("<unknown>");
					wsprintf(szdbsetting,"%02i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'W':
					isetting=DBGetContactSettingWord(hcontact,S_MOD,"WeekDay",-1);
					if(isetting==-1)
						break;
					strcat(sztemp,Translate(weekdays[isetting]));
					break;

				case 'w':
					isetting=DBGetContactSettingWord(hcontact,S_MOD,"WeekDay",-1);
					if(isetting==-1)
						break;
					strcat(sztemp,Translate(wdays_short[isetting]));
					break;

				case 'E':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Month",0)))
						return wantempty?"":Translate("<unknown>");
					strcat(sztemp,Translate(monthnames[isetting-1]));
					break;

				case 'e':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Month",0)))
						return wantempty?"":Translate("<unknown>");
					strcat(sztemp,Translate(mnames_short[isetting-1]));
					break;

				case 'H':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Hours",-1))==-1)
						return wantempty?"":Translate("<unknown>");
					wsprintf(szdbsetting,"%02i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'h':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Hours",-1))==-1)
						return wantempty?"":Translate("<unknown>");

					if(!isetting) isetting=12;
					
					wsprintf(szdbsetting,"%i",(isetting-((isetting>12)?12:0)));
					strcat(sztemp,szdbsetting);
					break;

				case 'p':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Hours",-1))==-1)
						return wantempty?"":Translate("<unknown>");
					if(isetting>12)
						strcat(sztemp,"PM");
					else strcat(sztemp,"AM");
					break;

				case 'M':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Minutes",-1))==-1)
						return wantempty?"":Translate("<unknown>");
					wsprintf(szdbsetting,"%02i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'S':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Seconds",-1))==-1)
						return wantempty?"":Translate("<unknown>");
					wsprintf(szdbsetting,"%02i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'n':
					strcat(sztemp,hcontact?(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hcontact,0):(wantempty?"":"---"));
					break;
				case 'N':
					ci.dwFlag=CNF_NICK;
					if(!CallService(MS_CONTACT_GETCONTACTINFO,(WPARAM)0,(LPARAM)&ci)){
						strcat(sztemp,ci.pszVal);
					} else {
						strcat(sztemp,wantempty?"":Translate("<unknown>"));
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
						if (isYahoo(ci.szProto)) // hard-wired YAHOO support
						{
							DBVARIANT dbv;
							DBGetContactSetting(hcontact,ci.szProto,"id",&dbv);
							strcpy(szdbsetting,dbv.pszVal);
							DBFreeVariant(&dbv);
						}
						else if (isJabber(ci.szProto)) // hard-wired JABBER support
						{
							DBVARIANT dbv;
							DBGetContactSetting(hcontact,ci.szProto,"LoginName",&dbv);
							strcpy(szdbsetting,dbv.pszVal);
							DBFreeVariant(&dbv);
							DBGetContactSetting(hcontact,ci.szProto,"LoginServer",&dbv);
							strcat(szdbsetting,"@");
							strcat(szdbsetting,dbv.pszVal);
							DBFreeVariant(&dbv);
						} else strcpy(szdbsetting,wantempty?"":Translate("<unknown>"));
					}
					else
					{
						strcpy(szdbsetting,wantempty?"":Translate("<unknown>"));
					}
					strcat(sztemp,szdbsetting);
					break;

				case 's':
					isetting=DBGetContactSettingWord(hcontact,S_MOD,hcontact?"Status":courProtoName,ID_STATUS_OFFLINE);
					strcpy(szdbsetting,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)isetting,0));
					strcat(sztemp,Translate(szdbsetting));
					break;

				case 'i':
				case 'r':
					dwsetting=DBGetContactSettingDword(hcontact,ci.szProto,szstring[loop]=='i'?"IP":"RealIP",0);
					if(!dwsetting)
						strcat(sztemp,wantempty?"":Translate("<unknown>"));
					else
					{
						ia.S_un.S_addr=htonl(dwsetting);
						strcat(sztemp,inet_ntoa(ia));
					}
					break;
				case 'P':if (ci.szProto) strcat(sztemp,ci.szProto); else strcat(sztemp,wantempty?"":"ProtoUnknown");
					break;
				case 'b':
					strcat(sztemp,/*"\n"*/"\x0D\x0A");
					break;
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
						} else strcpy(szdbsetting,wantempty?"":Translate("<unknown>"));
					}
					strcat(sztemp,szdbsetting);
					break;
				case 't':
					strcat(sztemp,"\t");
					break;

				default:
					strncat(sztemp,szstring+loop-1,2);
					break;
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



int UpdateValues(WPARAM wparam,LPARAM lparam)
{
	DBCONTACTWRITESETTING *cws;
	SYSTEMTIME time;
	HANDLE hContact;
	int prevStatus;
	
	hContact = (HANDLE)wparam;
	cws=(DBCONTACTWRITESETTING *)lparam;
//	if(CallProtoService(cws->szModule,PS_GETSTATUS,0,0)==ID_STATUS_OFFLINE) return 0;
	if(hContact==NULL || strcmp(cws->szSetting,"Status") || !IsWatchedProtocol(cws->szModule)) 
		return 0;

	prevStatus=DBGetContactSettingWord(hContact,S_MOD,"Status",ID_STATUS_OFFLINE);
	
	if(cws->value.wVal<=ID_STATUS_OFFLINE)
	{
		// avoid repeating the offline status
		if (prevStatus<=ID_STATUS_OFFLINE) 
			return 0;

		DBWriteContactSettingByte(hContact,S_MOD,"Offline",1);
		GetLocalTime(&time);
		DBWriteTime(&time,hContact);

		if(!DBGetContactSettingByte(NULL,S_MOD,"IgnoreOffline",1))
		{
			DBWriteContactSettingWord(hContact,S_MOD,"Status",ID_STATUS_OFFLINE);

			if(DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0))
				FileWrite(hContact);

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

		if(DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0))
			FileWrite(hContact);

		if(DBGetContactSettingByte(NULL,S_MOD,"KeepHistory",0))
			HistoryWrite(hContact);

		if(DBGetContactSettingByte(hContact,S_MOD,"OnlineAlert",0)) 
			ShowHistory(hContact, 1);

	//	CallContactService(hContact,PSS_GETINFO,0,0);

		DBWriteContactSettingByte(hContact,S_MOD,"Offline",0);
	}
	return 0;
}



int ModeChange(WPARAM wparam,LPARAM lparam)
{
	ACKDATA *ack;
	int isetting=0;
	SYSTEMTIME time;

	ack=(ACKDATA *)lparam;

	if(ack->type!=ACKTYPE_STATUS || ack->result!=ACKRESULT_SUCCESS || ack->hContact!=NULL) return 0;
	
	courProtoName = (char *)ack->szModule;
	GetLocalTime(&time);

	DBWriteTime(&time,NULL);

	isetting=CallProtoService(ack->szModule,PS_GETSTATUS,0,0);
	if (isetting<ID_STATUS_OFFLINE) isetting = ID_STATUS_OFFLINE;
	if (isetting==DBGetContactSettingWord(NULL,S_MOD,courProtoName,ID_STATUS_OFFLINE)) return 0;
	DBWriteContactSettingWord(NULL,S_MOD,courProtoName,(WORD)isetting);

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



