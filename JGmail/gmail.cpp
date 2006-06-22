#include "jabber.h"
#include "jabber_iq.h"
#include <m_utils.h>
#include <m_database.h>
#include "sdk/m_popupt.h"
#include "resource.h"

#define DEF_LABELS _T("label:^u ((!label:^s) (!label:^k) (!label:^vm))")

#ifdef __GNUC__
#define NUM100NANOSEC  116444736000000000ULL
#else
#define NUM100NANOSEC  116444736000000000
#endif

#ifndef _ttoi64
#ifdef UNICODE
#define _ttoi64   _wtoi64
#else
#define _ttoi64   _atoi64
#endif
#endif

#ifndef UInt32x32To64
#define UInt32x32To64(a, b)   ((DWORDLONG)((DWORD)(a)) * (DWORDLONG)((DWORD)(b)))
#endif



LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
typedef struct {
	TCHAR *username;
//	char *url;
	char *password;
	__int64 tid;
} POPUP_ACCINFO;


void StringFromUnixTime(TCHAR* str, int length, unsigned long t)
{
	SYSTEMTIME st;
    ULONGLONG ll;
	FILETIME ft;
    ll = UInt32x32To64(CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,t,0), 10000000) + NUM100NANOSEC;
    ft.dwLowDateTime = (DWORD)ll;
    ft.dwHighDateTime = (DWORD)(ll >> 32);
    FileTimeToSystemTime(&ft, &st);
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, str, length);
	int l = _tcslen(str);
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

int makeHead(TCHAR *target, int tSize, __int64 tid, __int64 time){
//	char sttemp[50];
	int l = 0;
	target[0] = 0;
	if (tid != -1){
		l = mir_sntprintf(target,tSize,TranslateT("MinThrd: "));
		StringFromUnixTime(target+l,tSize-l,(long)((tid>>20)/1000));
		l = _tcslen(target);
		l += mir_sntprintf(target+l,tSize-l,_T(".%03d"),
			((tid>>20)%1000)
		);
		l += mir_sntprintf(target+l,tSize-l,_T(" (%05X)"),
			(int)(tid&0xFFFFF)
		);
		if (time != -1) l += mir_sntprintf(target+l,tSize-l,_T("\n"));
	}
	if (time != -1){
		l += mir_sntprintf(target+l,tSize-l,TranslateT("MinTime: "));
		StringFromUnixTime(target+l,tSize-l,(long)((time)/1000));
		l = _tcslen(target);
		l += mir_sntprintf(target+l,tSize-l,_T(".%03d"),
			((time)%1000)
		);
	}
	return l;
}

void JabberDummyResult( XmlNode *iqNode, void *userdata ){
	JabberLog( 
		"Received DummyResult. id: \""TCHAR_STR_PARAM"\", type: \""TCHAR_STR_PARAM"\"",
		JabberXmlGetAttrValue( iqNode, "id"), JabberXmlGetAttrValue( iqNode, "type"));
}
void JabberEnableNotifications(ThreadData *info){
	int iqId = JabberSerialNext();
	TCHAR *localJid = mir_tstrdup( info->fullJID );
	TCHAR *pos;
	if (pos = _tcschr( localJid, '/' )) *pos = '\0'; //cut the resource
	JabberIqAdd( iqId, IQ_PROC_NONE, JabberDummyResult );
	XmlNode iq( "iq" ); iq.addAttr( "type", "set" ); iq.addAttrID( iqId ); iq.addAttr( "to", localJid );
	XmlNode* usersetting = iq.addChild( "usersetting" ); usersetting->addAttr( "xmlns", "google:setting" );
	XmlNode*  mailnotifications = usersetting->addChild( "mailnotifications" ); mailnotifications->addAttr( "value", "true" );
	JabberSend( jabberThreadInfo->s, iq );
	mir_free(localJid);
//	MessagePopup(NULL,_T("Text"),_T("Testing 0"),0);
//	MessagePopup(NULL,_T("Text"),_T("Testing MB_ICONHAND"),MB_ICONHAND);
//	MessagePopup(NULL,_T("Text"),_T("Testing MB_ICONQUESTION"),MB_ICONQUESTION);
//	MessagePopup(NULL,_T("Text"),_T("Testing MB_ICONWARNING"),MB_ICONWARNING);
//	MessagePopup(NULL,_T("Text"),_T("Testing MB_ICONASTERISK"),MB_ICONASTERISK);
//	MessagePopup(NULL,_T("Text"),_T("Testing MB_USERICON"),MB_USERICON);
}

HANDLE fakeContact = NULL;
static HANDLE fakeContactFindCreate(){
	if (!fakeContact){
		HANDLE fcTemp = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		while ( fcTemp != NULL ) {
			char *szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) fcTemp, 0 );
			if ( szProto!=NULL && !strcmp( jabberProtoName, szProto )) {
				DBVARIANT dbv;
				if ( !DBGetContactSetting( fcTemp, jabberProtoName, "FakeContact", &dbv )) {
					if (  !strncmp( dbv.pszVal, "GMAIL", 5 )) {
						JFreeVariant( &dbv );
						break;
					}
					JFreeVariant( &dbv );
			}	}
			fcTemp = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM ) fcTemp, 0 );
		}
		if (!fcTemp){ //Still no FakeContact? Ok, Create one!
			fcTemp = ( HANDLE ) JCallService( MS_DB_CONTACT_ADD, 0, 0 );
			JCallService( MS_PROTO_ADDTOCONTACT, ( WPARAM ) fcTemp, ( LPARAM )jabberProtoName );
			JSetString( fcTemp, "FakeContact", "GMAIL" );
			JSetStringT( fcTemp, "Nick", _T("NewMail") );
			DBWriteContactSettingTString( fcTemp, "UserInfo", "MyNotes", _T("This is a dummy contact to collect new e-mail notifications history"));
			JabberLog( "Create Jabber contact jid=%s, nick=%s", jabberProtoName, "NewMail" );
		}
		return fcTemp;
	} else return fakeContact;
}

void MyNotification(POPUPDATAT *ppd){
	unsigned short int gMailUse = (byte)JGetByte( NULL, "GMailUse",1);
	BOOL usePopUps = ( gMailUse & 1) & ( ServiceExists(MS_POPUP_QUERY) != 0);
#ifdef _UNICODE
	BOOL unicodePopUps = usePopUps & ( ServiceExists(MS_POPUP_ADDPOPUPW) != 0);
	char *aContact = t2a(ppd->lptzContactName);
	char *aText = t2a(ppd->lptzText);
#endif
	BOOL useFakeContact = (( gMailUse & 2)==2);
	if (useFakeContact){
		if (!fakeContact) fakeContact = fakeContactFindCreate();
		// add the message to the fake contact's history
		DBEVENTINFO dbei = { 0 };
		int l = _tcslen(ppd->lptzContactName)+_tcslen(ppd->lptzText)+2;
		TCHAR * msgT =( TCHAR* )mir_alloc(l*sizeof(TCHAR));
#ifdef _UNICODE
		mir_sntprintf(msgT,l,_T("%s\n%s"),ppd->lptzContactName,ppd->lptzText);
		char * msgA = t2a(msgT);
		BOOL isUnicodePartNeeded = false;
		for (int i=0;i<l;i++) if (msgT[i]>127){isUnicodePartNeeded = true; break;}
		if (isUnicodePartNeeded){
//		if (false){
			char *temp = (char *)mir_alloc(l*3);
			strcpy(temp,msgA);
			mir_free(msgA);
			msgA = temp; // because mir_realoc doesnt work here
			_tcscpy((TCHAR *)(msgA + l),msgT); //add the unicode part
			dbei.cbBlob = l*3;
		} else dbei.cbBlob = l;
		dbei.pBlob = ( PBYTE )msgA;
#else
		mir_snprintf(msgT,l,"%s\n%s",ppd->lptzContactName,ppd->lptzText);
		dbei.pBlob = ( PBYTE )msgT;
		dbei.cbBlob = l;
#endif
		dbei.cbSize = sizeof( dbei );
		dbei.szModule = jabberProtoName;
		dbei.timestamp = time(NULL);
		dbei.flags = usePopUps?DBEF_READ:0; // Do not notify twice
		dbei.eventType = EVENTTYPE_MESSAGE;
		JCallService( MS_DB_EVENT_ADD, ( WPARAM ) fakeContact, ( LPARAM )&dbei );
		mir_free(msgT);
#ifdef _UNICODE
		mir_free(msgA);
#endif
	}
#ifdef _UNICODE
	if (usePopUps) {
		if (unicodePopUps) CallService(MS_POPUP_ADDPOPUPW, (WPARAM)ppd, 0);
		else { //create new one with ansi strings
			POPUPDATAEX ppdA;
			int s = (unsigned int)&ppdA.lpzContactName-(unsigned int)&ppdA.lchContact;
			memmove(
				(void *)&(ppdA.lchContact),
				(void *)&(ppd->lchContact),s); //noone knows how big will be these in next compile
			strncpy(ppdA.lpzContactName, aContact, MAX_CONTACTNAME);
			strncpy(ppdA.lpzText, aText, MAX_SECONDLINE);
			s = sizeof(ppdA)-((unsigned int)&ppdA.colorBack-(unsigned int)&ppdA.lchContact);
			memmove(
				(void *)&(ppdA.colorBack),
				(void *)&(ppd->colorBack),s);
			CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppdA, 0);
	}	}
#else
	if (usePopUps) CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)ppd, 0);
#endif
	if (!gMailUse) {
#ifdef _UNICODE
		JabberLog( "Show PopUp: %s", aContact);
		JabberLog( "Text PopUp: %s", aText);
#else
		JabberLog( "Show PopUp: %s", ppd->lptzContactName);
		JabberLog( "Text PopUp: %s", ppd->lptzText);
#endif
	}
#ifdef _UNICODE
	mir_free(aContact);
	mir_free(aText);
#endif
}

static __int64 maxtid = 0;
static __int64 maxtime = 0;
char soundname[64];
void __cdecl JabberRerequestMailBoxThread( JABBER_SOCKET s ){
	JabberLog("Re-requestMailBoxThread: %s","Entering");
	NETLIBSELECT nls = {0};

	nls.cbSize = sizeof( NETLIBSELECT );
	nls.dwTimeout = 60000;	// in 1 min in milliseconds
	nls.hExceptConns[0] = s;
	if ( JCallService( MS_NETLIB_SELECT, 0, ( LPARAM )&nls ) == 0 )
		JabberRequestMailBox(s);
	JabberLog("Re-requestMailBoxThread: %s","Exitting");
}

static HWND hWndPopupError = 0;
void JabberRequestMailBox(HANDLE hConn){

//	if(CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0)){//will request mailbox only if popup is working
		if (hWndPopupError) {
			PUDeletePopUp(hWndPopupError);
		}

		int iqId = JabberSerialNext();
		if (!maxtid) maxtid = ((__int64)DBGetContactSettingDword( NULL, jabberProtoName,"MaxTidHi",0)<<32)+
							  ((__int64)DBGetContactSettingDword( NULL, jabberProtoName,"MaxTidLo",0));
		if (!maxtime) maxtime=((__int64)DBGetContactSettingDword( NULL, jabberProtoName,"MaxTimeHi",0)<<32)+
							  ((__int64)DBGetContactSettingDword( NULL, jabberProtoName,"MaxTimeLo",0));

		JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultMailNotify );
		char stid[21]; sprint64u(stid,maxtid);
		char stime[21];sprint64u(stime,maxtime);
		XmlNodeIq iq("get",iqId);
		XmlNode* query = iq.addQuery("google:mail:notify");
		query->addAttr("newer-than-time",stime);
		query->addAttr("newer-than-tid",stid);
		if (JGetByte(NULL,"Labels",0) & 0x80 ){
			DBVARIANT dbv;
			int res = JGetStringT(NULL,"LabelsExpr", &dbv);
			query->addAttr("q",res?DEF_LABELS:dbv.ptszVal);
			if (!res) JFreeVariant(&dbv);
		}

		JabberSend( hConn,iq );
  		if (JGetByte(NULL,"ShowRequest",0)) {
			POPUPDATAT ppd;
			ZeroMemory((void *)&ppd, sizeof(ppd));
			ppd.lchContact = 0;
			ppd.lchIcon = iconList[12];
			mir_sntprintf(ppd.lptzContactName, MAX_CONTACTNAME - 5, _T(TCHAR_STR_PARAM)_T(": %s"),jabberProtoName,TranslateT("Mailbox request"));
			ppd.colorText = JGetDword(NULL,"ColDebugText",0);
			ppd.colorBack = JGetDword(NULL,"ColDebugBack",RGB(255,255,128));
			ppd.iSeconds = (WORD)(JGetDword(NULL,"PopUpTimeoutDebug",0xFFFF0000)&0xFFFF);
			makeHead(ppd.lptzText, MAX_SECONDLINE - 5,maxtid,maxtime);
			MyNotification(&ppd);
		}
//	}
}

LRESULT CALLBACK PopupErrorDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case UM_INITPOPUP:
			hWndPopupError = hWnd;
			break;
		case WM_COMMAND:
			if (HIWORD(wParam) != STN_CLICKED) break;
		case WM_CONTEXTMENU:
			PUDeletePopUp(hWnd);
			break;
		case UM_FREEPLUGINDATA:
			hWndPopupError = 0;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIqResultMailNotify - Receive the e-mails from gmail:mail:notify

void JabberIqResultMailNotify( XmlNode *iqNode, void *userdata )
{
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	XmlNode *queryNode;
	TCHAR* type;
	TCHAR* str;

	// RECVED: mailbox info
	// ACTION: show popups with the received e-mails
	//JabberLog( "<iq/> mailbox" );
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) return;

	if (( queryNode=JabberXmlGetChild( iqNode, "error" )) ){ // error situation
		TCHAR *errcode = JabberXmlGetAttrValue( queryNode, "code" );
		TCHAR *errtype = JabberXmlGetAttrValue( queryNode, "type" );
		if (((byte)JGetByte( NULL, "GMailUse",1) & 2)==2){ // we use Fake Contact
			if (!fakeContact) fakeContact = fakeContactFindCreate();
			JSetWord( fakeContact, "Status", ID_STATUS_NA );
		}
		int showresult = JGetByte(NULL,"ShowResult",0);
		if (!(showresult & 2)){ // we do not suppress errors
			POPUPDATAT ppd;
			ZeroMemory((void *)&ppd, sizeof(ppd));
			ppd.lchContact = 0;
			//ppd.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
			ppd.lchIcon = iconList[11];
			mir_sntprintf(ppd.lptzContactName, MAX_CONTACTNAME - 5, 
				_T(TCHAR_STR_PARAM)_T(": %s \"%s\"; %s \"%s\"."),
				jabberProtoName,
				TranslateT("Error Code"),
				errcode?errcode:TranslateT("Unknown"),
				TranslateT("Type"),
				errtype?TranslateTS(errtype):TranslateT("Unknown"));
			XmlNode *textNode = JabberXmlGetChild( queryNode, "text" );
			int l = mir_sntprintf(ppd.lptzText, MAX_SECONDLINE - 5, 
				_T("%s: ")_T(TCHAR_STR_PARAM)_T(": %s\n"),
				TranslateT("Message"),
				queryNode->numChild?queryNode->child[0]->name:Translate("none"),
				textNode?textNode->text:TranslateT("none")
			);
			textNode = JabberXmlGetChild( iqNode, "query" );
			if (textNode) {
				__int64 tid = _ttoi64(JabberXmlGetAttrValue( textNode, "newer-than-tid" ));
				__int64 time = _ttoi64(JabberXmlGetAttrValue( textNode, "newer-than-time" ));
				l = makeHead(ppd.lptzText+l,MAX_SECONDLINE-5-l,tid,time);
			}

			ppd.colorText = JGetDword(NULL,"ColErrorText",0);
			ppd.colorBack = JGetDword(NULL,"ColErrorBack",RGB(255,128,128));
			ppd.iSeconds = (WORD)(JGetDword(NULL,"PopUpTimeoutDebug",0xFFFF0000)>>16);
			ppd.PluginWindowProc = (WNDPROC)PopupErrorDlgProc;
			MyNotification(&ppd);
		}
		if (!(showresult & 4)){ // we do not suppress automatic re-request
			if (_tcscmp(errtype,_T("cancel"))) { // errortype <> cancel: We re-request the mailbox
				JabberForkThread( JabberRerequestMailBoxThread, 0, info->s );
			}
		}
		return;
	}

	if (( queryNode=JabberXmlGetChild( iqNode, "mailbox" )) == NULL ) {
		//hm... returned result with no data... it happens some time
		if (((byte)JGetByte( NULL, "GMailUse",1) & 2)==2){ // we use Fake Contact
			if (!fakeContact) fakeContact = fakeContactFindCreate();
			JSetWord( fakeContact, "Status", ID_STATUS_AWAY );
		}
		if (!(JGetByte(NULL,"ShowResult",0) & 4)){ // we do not suppress automatic re-request
			JabberForkThread( JabberRerequestMailBoxThread, 0, info->s );
		}
		return;
	}

	if ( !_tcscmp( type, _T("result" ))) {
		str = JabberXmlGetAttrValue( queryNode, "xmlns" );
		if ( str!=NULL && !_tcscmp( str, _T("google:mail:notify") )) {
			if (((byte)JGetByte( NULL, "GMailUse",1) & 2)==2){ // we use Fake Contact
				if (!fakeContact) fakeContact = fakeContactFindCreate();
				JSetWord( fakeContact, "Status", ID_STATUS_ONLINE );
			}
			__int64 rt = _ttoi64(JabberXmlGetAttrValue( queryNode, "result-time" ));
			BOOL syncTimeResult = false;
			int drift = ((unsigned int)(rt/1000)) - time(NULL);
			if (drift) if ( 0x1 & JGetByte(NULL,"SyncTime",0)){
				SYSTEMTIME st;
			    LONGLONG ll;
				FILETIME ft;
			    ll = (rt*10000) + NUM100NANOSEC;
				ft.dwLowDateTime = (DWORD)ll;
				ft.dwHighDateTime = (DWORD)(ll >> 32);
				FileTimeToSystemTime(&ft, &st);
				syncTimeResult = SetSystemTime(&st);
			}
			if (JGetByte(NULL,"ShowResult",0)) {
				POPUPDATAT ppd;
				ZeroMemory((void *)&ppd, sizeof(ppd));
				ppd.lchContact = 0;
				ppd.lchIcon = iconList[12];
				mir_sntprintf(ppd.lptzContactName, MAX_SECONDLINE - 5, _T(TCHAR_STR_PARAM)_T(": %s: %s %s"),
					jabberProtoName,
					TranslateT("Maibox result"),TranslateT("Matched"),
					JabberXmlGetAttrValue( queryNode, "total-matched" )
				);
				ppd.colorText = JGetDword(NULL,"ColDebugText",0);
				ppd.colorBack = JGetDword(NULL,"ColDebugBack",RGB(255,255,128));
				ppd.iSeconds = (WORD)(JGetDword(NULL,"PopUpTimeoutDebug",0xFFFF0000)&0xFFFF);
				int pos = makeHead(ppd.lptzText, MAX_SECONDLINE - 5,
					-1,
					rt
				);
				if (drift){
					pos += mir_sntprintf(ppd.lptzText+pos, MAX_SECONDLINE - 5,_T("\n%s: %d %s"),
						TranslateT("LocalDrift"),
						drift,
						TranslateT("seconds")
					);
					if (syncTimeResult) mir_sntprintf(ppd.lptzText+pos, MAX_SECONDLINE - 5,_T("; %s."),TranslateT("Synchronized"));
				}
				MyNotification(&ppd);
			} else {
				if (drift) if ((0x2 & JGetByte(NULL,"SyncTime",0))==0){
					POPUPDATAT ppd;
					ZeroMemory((void *)&ppd, sizeof(ppd));
					ppd.lchContact = 0;
					ppd.lchIcon = iconList[13];
					mir_sntprintf(ppd.lptzContactName, MAX_SECONDLINE - 5, _T(TCHAR_STR_PARAM)_T(": %s %s%s."),
						jabberProtoName,TranslateT("System clock"),
						syncTimeResult?_T(""):TranslateT("NOT "),
						TranslateT("synchronized")
					);
					ppd.colorText = JGetDword(NULL,"ColClockText",0);
					ppd.colorBack = JGetDword(NULL,"ColClockBack",0);
					ppd.iSeconds = (WORD)(JGetDword(NULL,"PopUpTimeout",0x0000FFFF)>>16);
					mir_sntprintf(ppd.lptzText, MAX_SECONDLINE - 5,_T("%s: %d %s."),
						TranslateT("LocalDrift"),
						drift,
						TranslateT("seconds")
					);
					MyNotification(&ppd);
				}
			}
			XmlNode *threadNode;
			int i;
			for ( i=queryNode->numChild-1;i>=0; i-- ) {
				threadNode = queryNode->child[i];
				__int64 gtstamp = _ttoi64(JabberXmlGetAttrValue( threadNode, "tid" ));
				if (gtstamp>maxtid)maxtid=gtstamp;
				__int64 gmstamp = _ttoi64(JabberXmlGetAttrValue( threadNode, "date" ));
//				char *url = t2a(JabberXmlGetAttrValue( threadNode, "url" ));
				if (gmstamp>maxtime)maxtime=gmstamp;
				int numMesg = _ttoi(JabberXmlGetAttrValue( threadNode, "messages" ));
				TCHAR mesgs[10];
				if (numMesg>1) mir_sntprintf(mesgs,10,_T(" (%d)"),numMesg); else mesgs[0] = '\0';
				TCHAR sttime[50];
				StringFromUnixTime(sttime,50,(long)(gmstamp/1000));
//				char stthread[50];
//				StringFromUnixTime(stthread,50,(long)((gtstamp>>20)/1000));
				XmlNode *sendersNode = JabberXmlGetChild( threadNode, "senders" );
				int k; TCHAR sendersList[150];
				sendersList[0] = '\0';
				mir_sntprintf(sendersList,150,
					_T(TCHAR_STR_PARAM)_T(": %s: "),
					jabberProtoName,
					TranslateT("New mail from"));
				if (sendersNode) for ( k=0; k<sendersNode->numChild; k++ ) {
					if (k) _tcsncat(sendersList,_T(", "),150);
					TCHAR * senderName = JabberXmlGetAttrValue(sendersNode->child[sendersNode->numChild-1-k],"name");
					if (!senderName) senderName = JabberXmlGetAttrValue(sendersNode->child[sendersNode->numChild-1-k],"address");
					_tcsncat(sendersList,senderName,150);
				}
//				JabberLog( "Senders: %s",sendersList );
				{ //create and show popup
					POPUPDATAT ppd;
					ZeroMemory((void *)&ppd, sizeof(ppd));
					ppd.lchContact = 0;
					ppd.lchIcon = iconList[10];
					_tcsncpy(ppd.lptzContactName, sendersList, MAX_CONTACTNAME);
					sendersNode = JabberXmlGetChild( threadNode, "subject" );
					XmlNode *snippetNode = JabberXmlGetChild( threadNode, "snippet" );
					int pos = mir_sntprintf(ppd.lptzText, MAX_SECONDLINE - 5, _T("%s%s: %s\n%s: %s\n%s"),
						TranslateT("Subject"),
						mesgs,
						sendersNode?sendersNode->text:TranslateT("none"),
						TranslateT("Date"),sttime,
						snippetNode?snippetNode->text:TranslateT("none")
						);
					if (JGetByte(NULL,"Labels",0)&1){
						if (snippetNode = JabberXmlGetChild( threadNode, "labels" )){
							if (snippetNode->text){
								mir_sntprintf(ppd.lptzText+pos,MAX_SECONDLINE -5 -pos,_T("\n%s: %s"),TranslateT("Labels"),snippetNode->text);
					}	}	}

//					if (url) {
//						strncat(ppd.lpzText,"\n",MAX_SECONDLINE - 5-strlen(ppd.lpzText));
//						strncat(ppd.lpzText,url,MAX_SECONDLINE - 5-strlen(ppd.lpzText));
//					}
					ppd.colorText = JGetDword(NULL,"ColMsgText",0);
					ppd.colorBack = JGetDword(NULL,"ColMsgBack",0);
					ppd.iSeconds = (WORD)(JGetDword(NULL,"PopUpTimeout",0x0000FFFF)&0xFFFF);
					POPUP_ACCINFO * acci = NULL;
					if (JGetByte("OnClick",1)){
						if ((JGetByte( NULL, "GMailUse",1) & 1) & ( ServiceExists(MS_POPUP_QUERY) != 0)){
							acci = (POPUP_ACCINFO*)malloc(sizeof(POPUP_ACCINFO));
							ZeroMemory(acci, sizeof(acci)); //This is always a good thing to do.
							ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
							acci->username = info->username;
							acci->password = info->password;
//							acci->url;
							acci->tid = gtstamp;
							ppd.PluginData = (void *)acci;
					}	}
					MyNotification(&ppd);
			    }
				SkinPlaySound(soundname);
			}
			JSetDword(NULL,"MaxTidLo",(DWORD)maxtid);
			JSetDword(NULL,"MaxTidHi",(DWORD)(maxtid>>32));
			JSetDword(NULL,"MaxTimeLo",(DWORD)maxtime);
			JSetDword(NULL,"MaxTimeHi",(DWORD)(maxtime>>32));
		}
	}
}

int JabberMenuVisitGMail( WPARAM wParam, LPARAM lParam )
{
	char url[MAX_PATH];
	DBVARIANT dbv;
	url[0] = '\0';
	strcat(url,"https://www.google.com/accounts/ServiceLoginAuth?service=mail&Email=");
	if ( !DBGetContactSetting( NULL, jabberProtoName, "LoginName", &dbv )) {
		strcat(url,dbv.pszVal);
		JFreeVariant( &dbv );
	} else return 0;
	strcat(url,"&Passwd=");
	if ( !DBGetContactSetting( NULL, jabberProtoName, "Password", &dbv )) {
		JCallService( MS_DB_CRYPT_DECODESTRING, strlen( dbv.pszVal )+1, ( LPARAM )dbv.pszVal );
		char *passwdUrlEncoded = JabberUrlEncode(dbv.pszVal);
		JFreeVariant( &dbv );
		strcat(url,passwdUrlEncoded);
		mir_free(passwdUrlEncoded);
	} else return 0;
	strcat(url,"&continue=http%3A%2F%2Fmail.google.com%2Fmail%2F");

	CallService(MS_UTILS_OPENURL,1,(LPARAM)url);
	return 0;
}

LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	POPUP_ACCINFO * mpd = NULL;
	switch(message) {
		case WM_COMMAND:
			if (HIWORD(wParam) == STN_CLICKED) { //It was a click on the Popup.
				char url[MAX_PATH];
				mpd = (POPUP_ACCINFO * )PUGetPluginData(hWnd);
				if ((mpd) && (int)mpd!=-1){
#ifdef _UNICODE
					char *tempusername = u2a(mpd->username);
#endif
					char *passwdUrlEncoded = JabberUrlEncode(mpd->password);
					mir_snprintf(url,MAX_PATH,
//						"https://www.google.com/accounts/ServiceLoginAuth?service=mail&Email=%s&Passwd=%s&continue=https://mail.google.com/mail/",
//http://mail.google.com/mail/?view=cv&search=inbox&th=10859fac99369d5f&lvp=-1&cvp=9&qt=&fs=1&tf=1
//http%3A%2F%2Fmail.google.com%2Fmail%3Fview%3Dcv%26search%3Dinbox%26th%3D1085ab8ed25d733c%26lvp%3D-1%26cvp%3D9%26qt%3D%26fs%3D1%26tf%3D1&source=googletalk
						"https://www.google.com/accounts/ServiceLoginAuth?service=mail&Email=%s&Passwd=%s&continue=http%%3A%%2F%%2Fmail.google.com%%2Fmail%%3Fview%%3Dcv%%26search%%3Dinbox%%26th%%3D%x%x%%26lvp%%3D-1%%26cvp%%3D9%%26qt%%3D%%26fs%%3D1%%26tf%%3D1&source=googletalk",
#ifdef _UNICODE
						tempusername,
#else
						mpd->username,
#endif
						passwdUrlEncoded,
						(DWORD)(mpd->tid>>32),
						(DWORD)(mpd->tid)
					);
					mir_free(passwdUrlEncoded);
#ifdef _UNICODE
					mir_free(tempusername);
#endif
//					JabberLog("Redir: %s",url);
					CallService(MS_UTILS_OPENURL,1,(LPARAM)url);
				}
				PUDeletePopUp(hWnd);
				return TRUE;
			}
			break;
		case WM_CONTEXTMENU: {
			PUDeletePopUp(hWnd);
			break;
		}
		case UM_FREEPLUGINDATA: {
				mpd = (POPUP_ACCINFO * )PUGetPluginData(hWnd);
				if ((mpd) && (int)mpd!=-1) free(mpd);
				return TRUE; //TRUE or FALSE is the same, it gets ignored.
			}
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int saveChatsToServer = -1; //-1: unknown; 0: disabled; 1: enabled
extern HWND GMailOptsDlg;
void JabberUserConfigResult( XmlNode *iqNode, void *userdata ){
	struct ThreadData *info = ( struct ThreadData * ) userdata;
	XmlNode *queryNode;
	TCHAR* type;
	int tempSaveChatsToServer = saveChatsToServer;
	saveChatsToServer = -1;
	JabberLog( "Received JabberUserConfigResult");
	if (( type=JabberXmlGetAttrValue( iqNode, "type" )) == NULL ) goto LBLEnd;
	if (( queryNode=JabberXmlGetChild( iqNode, "error" )) )goto LBLEnd; // error situation
	/*if ( !strcmp( type, "result" ))*/ {
		TCHAR *str;
		if (( queryNode=JabberXmlGetChild( iqNode, "usersetting" )) == NULL ) goto LBLOK;
		str = JabberXmlGetAttrValue( queryNode, "xmlns" );
		if ( str!=NULL && !_tcscmp( str, _T("google:setting") )) {
			XmlNode *settingNode = JabberXmlGetChild( queryNode, "archivingenabled" );
			str = JabberXmlGetAttrValue(settingNode,"value");
			if (str){
				if ( !_tcscmp( str, _T("true") )) tempSaveChatsToServer = 1;
				if ( !_tcscmp( str, _T("false") )) tempSaveChatsToServer = 0;
			}
		}  else goto LBLEnd;
LBLOK:
		saveChatsToServer = tempSaveChatsToServer;
	}
LBLEnd:
	if (GMailOptsDlg) SetDlgItemText((HWND)GMailOptsDlg, IDC_SAVECHATS, (saveChatsToServer==-1)?TranslateT("Unknown"):(saveChatsToServer?TranslateT("Enabled"):TranslateT("Disabled")));
}

void JabberUserConfigRequest(ThreadData *info){
	int iqId = JabberSerialNext();
	TCHAR *localJid = mir_tstrdup( info->fullJID );
	TCHAR *pos;
	if (pos = _tcschr( localJid, '/' )) *pos = '\0'; //cut the resource
	if (saveChatsToServer!=-1) {
		//send the option
		JabberIqAdd( iqId, IQ_PROC_NONE, JabberDummyResult );
		XmlNode iq( "iq" ); iq.addAttr( "type", "set" ); iq.addAttrID( iqId ); iq.addAttr( "to", localJid );
		XmlNode* usersetting = iq.addChild( "usersetting" ); usersetting->addAttr( "xmlns", "google:setting" );
		XmlNode*  archivingenabled = usersetting->addChild( "archivingenabled" ); archivingenabled->addAttr( "value", saveChatsToServer?_T("false"):_T("true") );
		JabberSend( jabberThreadInfo->s, iq );
		iqId = JabberSerialNext(); //we already used the initial one
	}
	JabberIqAdd( iqId, IQ_PROC_NONE, JabberUserConfigResult );
	{
		XmlNode iq( "iq" ); iq.addAttr( "type", "get" ); iq.addAttrID( iqId ); iq.addAttr( "to", localJid );
		XmlNode* usersetting = iq.addChild( "usersetting" ); usersetting->addAttr( "xmlns", "google:setting" );
		JabberSend( jabberThreadInfo->s, iq );
	}
	mir_free(localJid);
}

static TCHAR *timesSt[] = {_T("check"),_T("hour"),_T("day"),_T("week"),_T("month")};

BOOL CALLBACK JabberGmailOptDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ){
	BOOL bChecked;
	BOOL popupavail;

	switch ( msg ) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault( hwndDlg );
		unsigned int gmailuse = JGetByte( NULL, "GMailUse",1);
		popupavail = ServiceExists(MS_POPUP_QUERY);
		CheckDlgButton( hwndDlg, IDC_USEPOPUP, (0x1 & gmailuse));
		CheckDlgButton( hwndDlg, IDC_USEFAKE, (0x2 & gmailuse)==2);
		unsigned int i;
		unsigned int engmail = (i = JGetByte(NULL,"EnableGMail",1))&1;
		CheckDlgButton( hwndDlg, IDC_ENGMAIL, (0x1 & i));
		EnableWindow(GetDlgItem(hwndDlg, IDC_USEPOPUP),popupavail );
		popupavail &= gmailuse;
		popupavail &= (0x1 & i);
		CheckDlgButton( hwndDlg, IDC_ENGMAILSTARTUP, (0x2 & i));
		CheckDlgButton( hwndDlg, IDC_SHOWREQUEST, JGetByte(NULL,"ShowRequest",0));
		i = JGetByte(NULL,"ShowResult",0);
		CheckDlgButton( hwndDlg, IDC_SHOWRESULT, 0x1 & i);
		CheckDlgButton( hwndDlg, IDC_SUPERR, 0x2 & i);
		CheckDlgButton( hwndDlg, IDC_SUPREREQUEST, 0x4 & i);
		i = JGetByte(NULL,"SyncTime",0);
		CheckDlgButton( hwndDlg, IDC_SYNCHRONIZE, (0x1 & i));
		CheckDlgButton( hwndDlg, IDC_SYNCHRONIZESILENT, (0x2 & i)!=0);
		EnableWindow( GetDlgItem( hwndDlg, IDC_SYNCHRONIZESILENT ), engmail &&  (0x1 & i));
		CheckDlgButton( hwndDlg, IDC_VISITGMAIL, JGetByte("OnClick",1));
		EnableWindow( GetDlgItem( hwndDlg, IDC_VISITGMAIL ), engmail &&  popupavail );
		CheckDlgButton( hwndDlg, IDC_INVASUNAVAIL, JGetByte(NULL,"InvAsUnavail",TRUE));
		EnableWindow( GetDlgItem( hwndDlg, IDC_ENGMAILSTARTUP ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_USEFAKE ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_FORCECHECK ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_CHECKTIMES ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_SYNCHRONIZE ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_SHOWREQUEST ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_SHOWRESULT ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_PREVIEW ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_SUPERR ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_SUPREREQUEST ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_SHOWLABELS ), engmail );
		EnableWindow( GetDlgItem( hwndDlg, IDC_EXPERTLABELS ), engmail );
		i = JGetByte(NULL,"Labels",0);
		CheckDlgButton( hwndDlg, IDC_SHOWLABELS, (0x1 & i));
		CheckDlgButton( hwndDlg, IDC_EXPERTLABELS, (0x80 & i)!=0);
		EnableWindow(GetDlgItem( hwndDlg, IDC_EDIT_LABELS ), ((0x80 & i)!=0) && engmail);
		{	DBVARIANT dbv;
			int res = JGetStringT(NULL,"LabelsExpr", &dbv);
			SetDlgItemText(hwndDlg, IDC_EDIT_LABELS,res?DEF_LABELS:dbv.ptszVal);
			if (!res) JFreeVariant(&dbv);
		}

		SendDlgItemMessage(hwndDlg,IDC_COLOURTEXT,CPM_SETCOLOUR,0,JGetDword(NULL,"ColMsgText",0));
		SendDlgItemMessage(hwndDlg,IDC_COLOURBACK,CPM_SETCOLOUR,0,JGetDword(NULL,"ColMsgBack",0));
		SendDlgItemMessage(hwndDlg,IDC_DEBUGCOLOURTEXT,CPM_SETCOLOUR,0,JGetDword(NULL,"ColDebugText",0));
		SendDlgItemMessage(hwndDlg,IDC_DEBUGCOLOURBACK,CPM_SETCOLOUR,0,JGetDword(NULL,"ColDebugBack",RGB(255,255,128)));
		SendDlgItemMessage(hwndDlg,IDC_ERRORCOLOURTEXT,CPM_SETCOLOUR,0,JGetDword(NULL,"ColErrorText",0));
		SendDlgItemMessage(hwndDlg,IDC_ERRORCOLOURBACK,CPM_SETCOLOUR,0,JGetDword(NULL,"ColErrorBack",RGB(255,128,128)));
		SendDlgItemMessage(hwndDlg,IDC_CLOCKCOLOURTEXT,CPM_SETCOLOUR,0,JGetDword(NULL,"ColClockText",0));
		SendDlgItemMessage(hwndDlg,IDC_CLOCKCOLOURBACK,CPM_SETCOLOUR,0,JGetDword(NULL,"ColClockBack",0));
		i = JGetDword(NULL,"PopUpTimeout",0x0000FFFF);
		SetDlgItemInt(hwndDlg, IDC_EDIT_TIMEOUT,((i&0xFFFF)==0xFFFF)?-1:(i&0xFFFF),TRUE);
		SetDlgItemInt(hwndDlg, IDC_EDIT_CLOCKTIMEOUT,((i>>16)==0xFFFF)?-1:(i>>16),TRUE);
		i = JGetDword(NULL,"PopUpTimeoutDebug",0xFFFF0000);
		SetDlgItemInt(hwndDlg, IDC_EDIT_DEBUGTIMEOUT,((i&0xFFFF)==0xFFFF)?-1:(i&0xFFFF),TRUE);
		SetDlgItemInt(hwndDlg, IDC_EDIT_ERRORTIMEOUT,((i>>16)==0xFFFF)?-1:(i>>16),TRUE);
		SetDlgItemText(hwndDlg, IDC_SAVECHATS, (saveChatsToServer==-1)?TranslateT("Unknown"):(saveChatsToServer?TranslateT("Enabled"):TranslateT("Disabled")));
		for (i=0;i<SIZEOF(timesSt);i++){
			TCHAR temp[32];
			mir_sntprintf( temp, 32,_T("%s %s"),TranslateT("Last"), TranslateTS(timesSt[i]));
			SendDlgItemMessage(hwndDlg, IDC_CHECKTIMES, CB_ADDSTRING, 0, (LPARAM)temp );
		}
#ifdef _DEBUG
		SendDlgItemMessageA(hwndDlg, IDC_CHECKTIMES, CB_ADDSTRING, 0, (LPARAM)"All (Don't Use!)" );
#endif
		SendDlgItemMessage(hwndDlg, IDC_CHECKTIMES, CB_SETCURSEL, 0, 0);
		return TRUE;
	}
	case WM_COMMAND:
		bChecked = IsDlgButtonChecked( hwndDlg, IDC_ENGMAIL );
		switch ( LOWORD( wParam )) {
		case IDC_ENGMAIL:
			popupavail = ServiceExists(MS_POPUP_QUERY);
			EnableWindow( GetDlgItem( hwndDlg, IDC_ENGMAILSTARTUP ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_USEPOPUP ), bChecked && popupavail);
			EnableWindow( GetDlgItem( hwndDlg, IDC_USEFAKE ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_FORCECHECK ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_CHECKTIMES ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_SYNCHRONIZE ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_SHOWREQUEST ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_SHOWRESULT ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_PREVIEW ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_VISITGMAIL ), bChecked && popupavail);
			EnableWindow( GetDlgItem( hwndDlg, IDC_SUPERR ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_SUPREREQUEST ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_SHOWLABELS ), bChecked );
			EnableWindow( GetDlgItem( hwndDlg, IDC_EXPERTLABELS ), bChecked );
		case IDC_SYNCHRONIZE:
			EnableWindow( GetDlgItem( hwndDlg, IDC_SYNCHRONIZESILENT),IsDlgButtonChecked( hwndDlg, IDC_SYNCHRONIZE ) && bChecked );
		case IDC_EXPERTLABELS:
			EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT_LABELS ), IsDlgButtonChecked( hwndDlg, IDC_EXPERTLABELS) && bChecked );
		case IDC_USEPOPUP:
			EnableWindow( GetDlgItem( hwndDlg, IDC_VISITGMAIL ), IsDlgButtonChecked( hwndDlg, IDC_USEPOPUP ) && bChecked);
			goto LBL_Apply;
		case IDC_PREVIEW: {
				if (bChecked){
				//Declarations and initializations
					POPUPDATAT ppd = { 0 };
					TCHAR sendersList[150];

					{ // show the error popup
						ppd.lchIcon = iconList[11];
						mir_sntprintf(ppd.lptzContactName, MAX_CONTACTNAME - 5, 
							_T(TCHAR_STR_PARAM)_T(": %s \"%s\"; %s \"%s\"."),
							jabberProtoName,
							TranslateT("Error Code"),
							_T("500"),
							TranslateT("Type"),
							TranslateT("wait"));
						mir_sntprintf(sendersList,150,_T("Unable to access mailbox for %s"),jabberJID);
						int l = mir_sntprintf(ppd.lptzText, MAX_SECONDLINE - 5, 
							_T("%s: ")_T(TCHAR_STR_PARAM)_T(": %s\n"),
							TranslateT("Message"),
							"internal-server-error",
							sendersList
						);
						l = makeHead(ppd.lptzText+l,MAX_SECONDLINE-5-l,maxtid,maxtime);
						ppd.colorText = SendDlgItemMessage(hwndDlg,IDC_ERRORCOLOURTEXT,CPM_GETCOLOUR,0,0);
						ppd.colorBack = SendDlgItemMessage(hwndDlg,IDC_ERRORCOLOURBACK,CPM_GETCOLOUR,0,0);//RGB(255,128,128);
						ppd.iSeconds = GetDlgItemInt(hwndDlg, IDC_EDIT_ERRORTIMEOUT, NULL, TRUE);
						MyNotification(&ppd);
					}
					if (IsDlgButtonChecked( hwndDlg, IDC_SHOWREQUEST ) ){
						ppd.lchIcon = iconList[12];
						mir_sntprintf(ppd.lptzContactName, MAX_CONTACTNAME - 5, _T(TCHAR_STR_PARAM)_T(": %s"),jabberProtoName,TranslateT("Mailbox request"));
						ppd.colorText = SendDlgItemMessage(hwndDlg,IDC_DEBUGCOLOURTEXT,CPM_GETCOLOUR,0,0);
						ppd.colorBack = SendDlgItemMessage(hwndDlg,IDC_DEBUGCOLOURBACK,CPM_GETCOLOUR,0,0);//RGB(255,255,128);
						makeHead(ppd.lptzText, MAX_SECONDLINE - 5,maxtid,maxtime);
						ppd.iSeconds = GetDlgItemInt(hwndDlg, IDC_EDIT_DEBUGTIMEOUT, NULL, TRUE);
						MyNotification(&ppd);
					}
					if (IsDlgButtonChecked( hwndDlg, IDC_SHOWRESULT ) ){
						ppd.lchIcon = iconList[12];
						mir_sntprintf(ppd.lptzContactName, MAX_SECONDLINE - 5, _T(TCHAR_STR_PARAM)_T(": %s: %s %s"),
							jabberProtoName,
							TranslateT("Maibox result"),TranslateT("Matched"),
							_T("1")
						);
						ppd.colorText = SendDlgItemMessage(hwndDlg,IDC_DEBUGCOLOURTEXT,CPM_GETCOLOUR,0,0);
						ppd.colorBack = SendDlgItemMessage(hwndDlg,IDC_DEBUGCOLOURBACK,CPM_GETCOLOUR,0,0);//RGB(255,255,128);
						ppd.iSeconds = GetDlgItemInt(hwndDlg, IDC_EDIT_DEBUGTIMEOUT, NULL, TRUE);
						int pos = makeHead(ppd.lptzText, MAX_SECONDLINE - 5,
							-1,
							((ULONGLONG)time(NULL)*1000+784)
						);
						pos += mir_sntprintf(ppd.lptzText+pos, MAX_SECONDLINE - 5,_T("\n%s: %d %s"),
							TranslateT("LocalDrift"),
							5,
							TranslateT("seconds")
						);
						if (IsDlgButtonChecked( hwndDlg, IDC_SYNCHRONIZE )) mir_sntprintf(ppd.lptzText+pos, MAX_SECONDLINE - 5,_T("; %s."),TranslateT("Synchronized"));;
						MyNotification(&ppd);
					} else { // the clock syncronisation will be shown only if the result popup is disabled
						if (IsDlgButtonChecked( hwndDlg, IDC_SYNCHRONIZE ) && (IsDlgButtonChecked( hwndDlg, IDC_SYNCHRONIZESILENT )==0)){
							ppd.lchIcon = iconList[13];
							mir_sntprintf(ppd.lptzContactName, MAX_SECONDLINE - 5, _T(TCHAR_STR_PARAM)_T(": %s %s%s."),
								jabberProtoName,TranslateT("System clock"),
								IsDlgButtonChecked( hwndDlg, IDC_SYNCHRONIZE )?_T(""):TranslateT("NOT "),
								TranslateT("synchronized")
							);
							ppd.colorText = SendDlgItemMessage(hwndDlg,IDC_CLOCKCOLOURTEXT,CPM_GETCOLOUR,0,0);
							ppd.colorBack = SendDlgItemMessage(hwndDlg,IDC_CLOCKCOLOURBACK,CPM_GETCOLOUR,0,0);//RGB(255,255,128);
							ppd.iSeconds = GetDlgItemInt(hwndDlg, IDC_EDIT_CLOCKTIMEOUT, NULL, TRUE);
							mir_sntprintf(ppd.lptzText, MAX_SECONDLINE - 5,_T("%s: %d %s."),
								TranslateT("LocalDrift"),
								5,
								TranslateT("seconds")
							);
							MyNotification(&ppd);
						}
					}
					ppd.lchIcon = iconList[10];
					mir_sntprintf(ppd.lptzContactName,150,
						_T(TCHAR_STR_PARAM)_T(": %s: %s, %s"),
						jabberProtoName,
						TranslateT("New mail from"),
						TranslateT("Some One"),
						TranslateT("Me Myself")
					);
					StringFromUnixTime(sendersList,50,time(NULL));
					int pos = mir_sntprintf(ppd.lptzText, MAX_SECONDLINE - 5, _T("%s%s: %s\n%s: %s\n%s"),
						TranslateT("Subject"),_T(" (3)"),
						TranslateT("A GMail notify test"),
						TranslateT("Date"),sendersList,
						TranslateT("Hi, I am trying the e-mail notification in JGmail.dll")
						);
					if (IsDlgButtonChecked( hwndDlg, IDC_SHOWLABELS )){
						mir_sntprintf(ppd.lptzText+pos,MAX_SECONDLINE -5 -pos,_T("\n%s: ^i|%s"),TranslateT("Labels"),TranslateT("Misc"));
					}
					ppd.colorText = SendDlgItemMessage(hwndDlg,IDC_COLOURTEXT,CPM_GETCOLOUR,0,0);
					ppd.colorBack = SendDlgItemMessage(hwndDlg,IDC_COLOURBACK,CPM_GETCOLOUR,0,0);
					ppd.iSeconds = GetDlgItemInt(hwndDlg, IDC_EDIT_TIMEOUT, NULL, TRUE);
					MyNotification(&ppd);
					SkinPlaySound(soundname);
				}
			}
			break;
		case IDC_CHECKTIMES: break;
		case IDC_FORCECHECK:{
				if (jabberThreadInfo){
					__int64 tmptid = maxtid;
					__int64 tmptime = maxtime;
					__int64 now  = (ULONGLONG)time(NULL)*1000;
					switch (SendDlgItemMessage(hwndDlg, IDC_CHECKTIMES, CB_GETCURSEL, 0, 0)){
						case 0 : break; // last check
						case 1 : maxtime = now - (ULONGLONG)3600000; maxtid = (now - (ULONGLONG)3600000)<<20; break; 
						case 2 : maxtime = now - (ULONGLONG)86400000; maxtid = (now - (ULONGLONG)86400000)<<20; break; 
						case 3 : maxtime = now - (ULONGLONG)604800000; maxtid = (now - (ULONGLONG)604800000)<<20; break; 
						case 4 : maxtime = now - (ULONGLONG)2678400000; maxtid = (now - (ULONGLONG)2678400000)<<20; break; 
#ifdef _DEBUG
						default: maxtime = 1; maxtid = 1<<20; break;
#endif
					}
					JabberRequestMailBox(jabberThreadInfo->s);
					maxtid = tmptid;
					maxtime = tmptime;
				} else {
					POPUPDATAT ppd = { 0 };
					ppd.lchIcon = iconList[11];
					mir_sntprintf(ppd.lptzContactName, MAX_SECONDLINE - 5, _T(TCHAR_STR_PARAM)_T(": %s."),jabberProtoName,TranslateT("No connection"));
					mir_sntprintf(ppd.lptzText, MAX_SECONDLINE - 5, TranslateT("Impossible to check"));
					ppd.colorText = SendDlgItemMessage(hwndDlg,IDC_ERRORCOLOURTEXT,CPM_GETCOLOUR,0,0);
					ppd.colorBack = SendDlgItemMessage(hwndDlg,IDC_ERRORCOLOURBACK,CPM_GETCOLOUR,0,0);//RGB(255,128,128);
					ppd.iSeconds = GetDlgItemInt(hwndDlg, IDC_EDIT_ERRORTIMEOUT, NULL, TRUE);
					MyNotification(&ppd);
				}
			}break;
		case IDC_SAVECHATS:{
				if (jabberThreadInfo)JabberUserConfigRequest(jabberThreadInfo);
			}break;
		case IDC_EDIT_TIMEOUT:
		case IDC_EDIT_DEBUGTIMEOUT:
		case IDC_EDIT_ERRORTIMEOUT:
		case IDC_EDIT_CLOCKTIMEOUT:
			if (HIWORD(wParam)==EN_KILLFOCUS){
				int i = GetDlgItemInt(hwndDlg, LOWORD( wParam ), NULL, TRUE);
				SetDlgItemInt(hwndDlg, LOWORD( wParam ),((i<-1)?-1:((i>0xFFFE)?0xFFFE:i)),TRUE);
			}
		case IDC_EDIT_LABELS:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()) return true;
			goto LBL_Apply;
		case IDC_RESET: {
				SendDlgItemMessage(hwndDlg,IDC_COLOURTEXT,CPM_SETCOLOUR,0,0);
				SendDlgItemMessage(hwndDlg,IDC_COLOURBACK,CPM_SETCOLOUR,0,0);
				SendDlgItemMessage(hwndDlg,IDC_DEBUGCOLOURTEXT,CPM_SETCOLOUR,0,0);
				SendDlgItemMessage(hwndDlg,IDC_DEBUGCOLOURBACK,CPM_SETCOLOUR,0,RGB(255,255,128));
				SendDlgItemMessage(hwndDlg,IDC_ERRORCOLOURTEXT,CPM_SETCOLOUR,0,0);
				SendDlgItemMessage(hwndDlg,IDC_ERRORCOLOURBACK,CPM_SETCOLOUR,0,RGB(255,128,128));
				SendDlgItemMessage(hwndDlg,IDC_CLOCKCOLOURTEXT,CPM_SETCOLOUR,0,0);
				SendDlgItemMessage(hwndDlg,IDC_CLOCKCOLOURBACK,CPM_SETCOLOUR,0,0);
				SetDlgItemInt(hwndDlg, IDC_EDIT_TIMEOUT,-1,TRUE);
				SetDlgItemInt(hwndDlg, IDC_EDIT_CLOCKTIMEOUT,0,TRUE);
				SetDlgItemInt(hwndDlg, IDC_EDIT_DEBUGTIMEOUT,0,TRUE);
				SetDlgItemInt(hwndDlg, IDC_EDIT_ERRORTIMEOUT,-1,TRUE);
			}
		default:
			LBL_Apply:
			SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
			break;
		}
		break;
	case WM_NOTIFY:
		if (( ( LPNMHDR ) lParam )->code == PSN_APPLY ) {
			int i = 0;
			bChecked = 0;
			if (IsDlgButtonChecked( hwndDlg, IDC_ENGMAIL )) bChecked += 1;
			if (IsDlgButtonChecked( hwndDlg, IDC_ENGMAILSTARTUP )) bChecked += 2;
			JSetByte( "EnableGMail",bChecked);
			if (bChecked & 1){
				JSetByte( "ShowRequest", ( BYTE ) IsDlgButtonChecked( hwndDlg, IDC_SHOWREQUEST ));
				i = IsDlgButtonChecked( hwndDlg, IDC_SHOWRESULT );
				if (IsDlgButtonChecked( hwndDlg, IDC_SUPERR )) i |= 2;
				if (IsDlgButtonChecked( hwndDlg, IDC_SUPREREQUEST )) i |= 4;
				JSetByte( "ShowResult", ( BYTE ) i);
				JSetByte( "OnClick", ( BYTE ) IsDlgButtonChecked( hwndDlg, IDC_VISITGMAIL ));
			}
			bChecked = IsDlgButtonChecked( hwndDlg, IDC_USEPOPUP );
			bChecked |= (IsDlgButtonChecked( hwndDlg, IDC_USEFAKE )<<1);
			JSetByte( "GMailUse",bChecked);
			i = IsDlgButtonChecked( hwndDlg, IDC_SYNCHRONIZE );
			if (IsDlgButtonChecked( hwndDlg, IDC_SYNCHRONIZESILENT )) i |= 2;
			JSetByte( "SyncTime",i);
			JSetByte( "InvAsUnavail", ( BYTE ) IsDlgButtonChecked( hwndDlg, IDC_INVASUNAVAIL ));
			JSetDword(NULL, "ColMsgText", SendDlgItemMessage(hwndDlg,IDC_COLOURTEXT,CPM_GETCOLOUR,0,0));
			JSetDword(NULL, "ColMsgBack", SendDlgItemMessage(hwndDlg,IDC_COLOURBACK,CPM_GETCOLOUR,0,0));
			JSetDword(NULL, "ColDebugText", SendDlgItemMessage(hwndDlg,IDC_DEBUGCOLOURTEXT,CPM_GETCOLOUR,0,0));
			JSetDword(NULL, "ColDebugBack", SendDlgItemMessage(hwndDlg,IDC_DEBUGCOLOURBACK,CPM_GETCOLOUR,0,0));
			JSetDword(NULL, "ColErrorText", SendDlgItemMessage(hwndDlg,IDC_ERRORCOLOURTEXT,CPM_GETCOLOUR,0,0));
			JSetDword(NULL, "ColErrorBack", SendDlgItemMessage(hwndDlg,IDC_ERRORCOLOURBACK,CPM_GETCOLOUR,0,0));
			JSetDword(NULL, "ColClockText", SendDlgItemMessage(hwndDlg,IDC_CLOCKCOLOURTEXT,CPM_GETCOLOUR,0,0));
			JSetDword(NULL, "ColClockBack", SendDlgItemMessage(hwndDlg,IDC_CLOCKCOLOURBACK,CPM_GETCOLOUR,0,0));
			i = ((WORD)GetDlgItemInt(hwndDlg, IDC_EDIT_CLOCKTIMEOUT, NULL, TRUE)<<16) | ((WORD)GetDlgItemInt(hwndDlg, IDC_EDIT_TIMEOUT, NULL, TRUE));
			JSetDword(NULL,"PopUpTimeout",i);
			i = ((WORD)GetDlgItemInt(hwndDlg, IDC_EDIT_ERRORTIMEOUT, NULL, TRUE)<<16) | ((WORD)GetDlgItemInt(hwndDlg, IDC_EDIT_DEBUGTIMEOUT, NULL, TRUE));
			JSetDword(NULL,"PopUpTimeoutDebug",i);
			i = IsDlgButtonChecked( hwndDlg, IDC_SHOWLABELS );
			if (IsDlgButtonChecked( hwndDlg, IDC_EXPERTLABELS )) i |= 0x80;
			if (i)JSetByte(NULL,"Labels",i);else JDeleteSetting(NULL,"Lables");
			{ TCHAR temp[1024];
				GetDlgItemText(hwndDlg, IDC_EDIT_LABELS, temp, 1024);
				if (!_tcscmp(temp,DEF_LABELS)) JDeleteSetting(NULL,"LabelsExpr");
				else JSetStringT(NULL,"LabelsExpr",temp);
			}
		}
		break;
	}//switch (msg)
return false;
}

