#include "jabber.h"
#include "jabber_iq.h"
#include <m_popup.h>
#include <m_utils.h>
#include <m_database.h>
#include "resource.h"

void StringFromUnixTime(char* str, int length, unsigned long t)
{
	SYSTEMTIME st;
    LONGLONG ll;
	FILETIME ft;
    ll = UInt32x32To64(CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,t,0), 10000000) + 116444736000000000;
    ft.dwLowDateTime = (DWORD)ll;
    ft.dwHighDateTime = (DWORD)(ll >> 32);
    FileTimeToSystemTime(&ft, &st);
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, str, length);
	int l = strlen(str);
	str[l] = ' ';
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, str+l+1, length-l+1);
}

static int sprint64u( char* buffer, unsigned __int64 x) {
  unsigned __int64 quot = x / 1000;
  int chars_written;
  if ( quot != 0) {
    chars_written = sprint64u( buffer, quot);
    chars_written += sprintf( buffer + chars_written, "%03u", ( unsigned int)( x % 1000));
  }
  else {
    chars_written = sprintf( buffer, "%u", ( unsigned int)( x % 1000));
  }
  return chars_written;
}

int makeHead(char *target, int tSize, __int64 tid, __int64 time){
//	char sttemp[50];
	int l = 0;
	target[0] = '\0';
	if (tid != -1){
		l = mir_snprintf(target,tSize,"MinTrhd: ");
		StringFromUnixTime(target+l,tSize-l,(long)((tid>>20)/1000));
		l = strlen(target);
		l += mir_snprintf(target+l,tSize-l,".%03d",
			((tid>>20)%1000)
		);
		l += mir_snprintf(target+l,tSize-l," (%05X)",
			(int)(tid&0xFFFFF)
		);
		if (time != -1) l += mir_snprintf(target+l,tSize-l,"\n");
	}
	if (time != -1){
		l += mir_snprintf(target+l,tSize-l,"MinTime: ");
		StringFromUnixTime(target+l,tSize-l,(long)((time)/1000));
		l = strlen(target);
		l += mir_snprintf(target+l,tSize-l,".%03d",
			((time)%1000)
		);
	}
	return l;
}

void JabberEnableNotifications(ThreadData *info){
	if(CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0)){//will enable notifications only if we have popups
		JabberSend( info->s, "<iq type='set' to='%s@%s' id='EnMailNotify'><usersetting xmlns='google:setting'><mailnotifications value='true'/></usersetting></iq>", info->username, info->server );
	}
}


static __int64 maxtid = 0;
static __int64 maxtime = 0;

void JabberRequestMailBox(HANDLE hConn){
	if(CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0)){//will request mailbox only if popup is working
		int iqId = JabberSerialNext();
		if (!maxtid) maxtid = ((__int64)DBGetContactSettingDword( NULL, jabberProtoName,"MaxTidHi",0)<<32)+
			             ((__int64)DBGetContactSettingDword( NULL, jabberProtoName,"MaxTidLo",0));
		if (!maxtime) maxtime=((__int64)DBGetContactSettingDword( NULL, jabberProtoName,"MaxTimeHi",0)<<32)+
			             ((__int64)DBGetContactSettingDword( NULL, jabberProtoName,"MaxTimeLo",0));

		JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultMailNotify );
		char stid[21]; sprint64u(stid,maxtid);
		char stime[21];sprint64u(stime,maxtime);
		JabberSend( hConn, "<iq type='get' id='"JABBER_IQID"%d'><query xmlns='google:mail:notify' newer-than-time='%s' newer-than-tid='%s'/></iq>", 
			iqId,
			stime,
			stid
		);
		if (JGetByte(NULL,"ShowRequest",0)) {
			POPUPDATAEX ppd;
			ZeroMemory((void *)&ppd, sizeof(ppd));
			ppd.lchContact = 0;
//			ppd.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
			ppd.lchIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_MAIL_INFO ));
			mir_snprintf(ppd.lpzContactName, MAX_SECONDLINE - 5, "%s: Maibox request",jabberProtoName);
			ppd.colorText = NULL;
			ppd.colorBack = RGB(255,255,128);
			ppd.iSeconds = JGetDword(NULL,"PopUpTimeout",-1);
			makeHead(ppd.lpzText, MAX_SECONDLINE - 5,maxtid,maxtime);
			CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultMailNotify - Receive the e-mails from gmail:mail:notify

void JabberIqResultMailNotify( XmlNode *iqNode, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	XmlNode *queryNode;
	char* type;
	char* str;
	// RECVED: mailbox info
	// ACTION: show popups with the received e-mails
	JabberLog( "<iq/> mailbox" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;

	if (( queryNode=JabberXmlGetChild( iqNode, "error" )) ){ // error situation
		char *errcode = JabberXmlGetAttrValue( queryNode, "code" );
		char *errtype = JabberXmlGetAttrValue( queryNode, "type" );

		POPUPDATAEX ppd;
		ZeroMemory((void *)&ppd, sizeof(ppd));
        ppd.lchContact = 0;
//        ppd.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		ppd.lchIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_MAIL_STOP ));
        mir_snprintf(ppd.lpzContactName, MAX_SECONDLINE - 5, "%s: Error Code %s; Type %s.",jabberProtoName, 
			errcode?errcode:"Unknown",
			errtype?errtype:"Unknown");
		XmlNode *textNode = JabberXmlGetChild( queryNode, "text" );
        int l = mir_snprintf(ppd.lpzText, MAX_SECONDLINE - 5, "Message: %s\n", 
			textNode?JabberUtf8Decode( textNode->text, 0 ):"none"
		);
		textNode = JabberXmlGetChild( iqNode, "query" );
		if (textNode) {
			__int64 tid = _atoi64(JabberXmlGetAttrValue( textNode, "newer-than-tid" ));
			__int64 time = _atoi64(JabberXmlGetAttrValue( textNode, "newer-than-time" ));
			l = makeHead(ppd.lpzText+l,MAX_SECONDLINE-5-l,tid,time);
		}
		
        ppd.colorText = NULL;
        ppd.colorBack = RGB(255,128,128);
		ppd.iSeconds = JGetDword(NULL,"PopUpTimeout",-1);
		JabberLog( "Notify error: %s\n%s", ppd.lpzContactName, ppd.lpzText);
        CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);

		return;
	}
	
	if (( queryNode=JabberXmlGetChild( iqNode, "mailbox" )) == NULL ) return;

	if ( !strcmp( type, "result" )) {
		str = JabberXmlGetAttrValue( queryNode, "xmlns" );
		if ( str!=NULL && !strcmp( str, "google:mail:notify" )) {
			if (JGetByte(NULL,"ShowResult",0)) {
				POPUPDATAEX ppd;
				ZeroMemory((void *)&ppd, sizeof(ppd));
				ppd.lchContact = 0;
//				ppd.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
				ppd.lchIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_MAIL_INFO ));
				mir_snprintf(ppd.lpzContactName, MAX_SECONDLINE - 5, "%s: Maibox result: Matched %s",
					jabberProtoName,
					JabberXmlGetAttrValue( queryNode, "total-matched" )
				);
				ppd.colorText = NULL;
				ppd.colorBack = RGB(255,255,128);
				ppd.iSeconds = JGetDword(NULL,"PopUpTimeout",-1);
				__int64 rt = _atoi64(JabberXmlGetAttrValue( queryNode, "result-time" ));
				int pos = makeHead(ppd.lpzText, MAX_SECONDLINE - 5,
					-1,
					rt
				);
				mir_snprintf(ppd.lpzText+pos, MAX_SECONDLINE - 5,"\nLocalDrift: %d seconds",
					((unsigned int)(rt/1000)) - time(NULL)
				);
				CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
			}
			XmlNode *threadNode;
			int i;
			for ( i=0; i<queryNode->numChild; i++ ) {
				threadNode = queryNode->child[i];
				__int64 gtstamp = _atoi64(JabberXmlGetAttrValue( threadNode, "tid" ));
				if (gtstamp>maxtid)maxtid=gtstamp;
				__int64 gmstamp = _atoi64(JabberXmlGetAttrValue( threadNode, "date" ));
				if (gmstamp>maxtime)maxtime=gmstamp;
				int numMesg = atoi(JabberXmlGetAttrValue( threadNode, "messages" ));
				char mesgs[10];
				if (numMesg>1) mir_snprintf(mesgs,10," (%d)",numMesg); else mesgs[0] = '\0';
				char sttime[50];
				StringFromUnixTime(sttime,50,(long)(gmstamp/1000));
//				char stthread[50];
//				StringFromUnixTime(stthread,50,(long)((gtstamp>>20)/1000));
				XmlNode *sendersNode = JabberXmlGetChild( threadNode, "senders" );
				int k; char sendersList[150];
				sendersList[0] = '\0';
				mir_snprintf(sendersList,150,"%s: New mail from ",jabberProtoName);
				if (sendersNode) for ( k=0; k<sendersNode->numChild; k++ ) {
					strncat(sendersList,k?", ":"",150);
					char * senderName = JabberXmlGetAttrValue(sendersNode->child[k],"name");
					if (!senderName) senderName = JabberXmlGetAttrValue(sendersNode->child[k],"address");
					strncat(sendersList,JabberUtf8Decode(senderName,0),150);
				}
				JabberLog( "Senders: %s",sendersList );
				{ //create and show popup
			        POPUPDATAEX ppd;
					ZeroMemory((void *)&ppd, sizeof(ppd));
			        ppd.lchContact = 0;
//			        ppd.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
					ppd.lchIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_MAIL_NEW ));
			        strncpy(ppd.lpzContactName, sendersList, MAX_CONTACTNAME);
					sendersNode = JabberXmlGetChild( threadNode, "subject" );
					XmlNode *snippetNode = JabberXmlGetChild( threadNode, "snippet" );
			        mir_snprintf(ppd.lpzText, MAX_SECONDLINE - 5, "Subject%s: %s\n%Time: %s\n%s", 
						mesgs,
						sendersNode?JabberUtf8Decode( sendersNode->text, 0 ):"none",
						sttime,
//						stthread,
						snippetNode?JabberUtf8Decode( snippetNode->text, 0 ):"none"						
						);
			        ppd.colorText = NULL;
			        ppd.colorBack = NULL;
					ppd.iSeconds = JGetDword(NULL,"PopUpTimeout",-1);
			        CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
			    }
			}
			JSetDword(NULL,"MaxTidLo",(DWORD)maxtid);
			JSetDword(NULL,"MaxTidHi",(DWORD)(maxtid>>32));
			JSetDword(NULL,"MaxTimeLo",(DWORD)maxtime);
			JSetDword(NULL,"MaxTimeHi",(DWORD)(maxtime>>32));
		}		
	}
}	
