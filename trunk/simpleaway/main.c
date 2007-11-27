/*

SimpleAway plugin for Miranda-IM

Copyright © 2005 Harven, © 2006-2007 Dezeath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "simpleaway.h"

HINSTANCE	hInst;
PLUGINLINK	*pluginLink;
struct MM_INTERFACE	mmi;
BOOL		terminated=TRUE;
DWORD		ProtoStatusMsgFlags;
UINT		SATimer, SARandMsgTimer, *SASetStatusTimer;
char		*winampsong;
BOOL		is_timer, is_randmsgtimer;
BOOL		removeCR;
BOOL		ShowCopy;
HANDLE		TopButton, h_oktoexit, h_shutdown, h_terminated, h_modulesloaded, h_ttbloaded, h_optinitialise, h_statusmodechange, h_protoack;
PROTOCOLDESCRIPTOR **protocols;
int			ProtoCount, ProtoStatusCount, ProtoStatusMsgCount, StatusMenuItemCount;
HWND		hwndSAMsgDialog;
HANDLE		h_changedicons, h_csstatuschange;
static HANDLE hChangeStatusMsgMenuItem;
static HANDLE hGlobalStatusMenuItem;
static HANDLE *hProtoStatusMenuItem;
HANDLE		h_prebuildstatusmenu;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"SimpleAway",
	PLUGIN_MAKE_VERSION(1,7,5,0),
	"This plugin replaces built-in away system.\r\n[Release Candidate 2]",
	"Harven, Dezeath",
	"harven@users.berlios.de, dezred@gmail.com",
	"© 2005 Harven, © 2006-2007 Dezeath",
	"http://dezhq.rogacz.com/miranda/",
	0,		//not transient
	DEFMOD_SRAWAY,
	// {7D548A69-05E7-4d00-89BC-ACCE781022C1}
	{ 0x7d548a69, 0x5e7, 0x4d00, { 0x89, 0xbc, 0xac, 0xce, 0x78, 0x10, 0x22, 0xc1 } }
};

static const MUUID interfaces[] = {MIID_SRAWAY, MIID_LAST};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst=hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) {
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0,6,0,0)) {
		MessageBoxA(NULL, "The SimpleAway plugin cannot be loaded. It requires Miranda IM 0.6 or later.", "SimpleAway Plugin", MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return NULL;
	}
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO *)&pluginInfo;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	return &pluginInfo;
}

__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void) {
	return interfaces;
}

#ifdef _DEBUG
void log2file(const char *fmt, ...) {
	HANDLE hFile;
	unsigned long dwBytesWritten = 0;
	va_list	va;
	char szText[1024];

	hFile = CreateFile("simpleaway.log", GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(hFile, 0, 0, FILE_END);

	strncpy(szText, "[\0", sizeof(szText));
	WriteFile(hFile, szText, strlen(szText), &dwBytesWritten, NULL);

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, NULL, szText, sizeof(szText));
	WriteFile(hFile, szText, strlen(szText), &dwBytesWritten, NULL);

	strncpy(szText, "] \0", sizeof(szText));
//	strncat(szText, "::\0", sizeof(szText) - strlen(szText));

	va_start(va, fmt);
	_vsnprintf(szText + strlen(szText), sizeof(szText) - strlen(szText), fmt, va);
	va_end(va);

	WriteFile(hFile, szText, strlen(szText), &dwBytesWritten, NULL);

	strncpy(szText, "\n\0", sizeof(szText));
	WriteFile(hFile, szText, strlen(szText), &dwBytesWritten, NULL);

	CloseHandle(hFile);
}
#endif

//From clist interface
int GetProtocolVisibility(char *ProtoName) {
	int i;
	int res=0;
	DBVARIANT dbv;
	char buf2[10];
	int count;

	if (!ProtoName)
		return 0;

	count = (int)DBGetContactSettingDword(0, "Protocols", "ProtoCount", -1);
	if (count == -1)
		return 1;

	for (i=0; i<count; i++) {
		_itoa(i, buf2, 10);
		if (!DBGetContactSetting(NULL, "Protocols", buf2, &dbv)) {
			if (strcmp(ProtoName, dbv.pszVal) == 0) {
				DBFreeVariant(&dbv);
				_itoa(i+400, buf2, 10);
				res = DBGetContactSettingDword(NULL, "Protocols", buf2, 0);
				return res;
			}
			DBFreeVariant(&dbv);
		}
	}
	return 0;
}

//From SRAway module
char *StatusModeToDbSetting(int status,const char *suffix) {
    char *prefix;
	static char str[64];

	switch(status) {
		case ID_STATUS_AWAY: prefix="Away";	break;
		case ID_STATUS_NA: prefix="Na";	break;
		case ID_STATUS_DND: prefix="Dnd"; break;
		case ID_STATUS_OCCUPIED: prefix="Occupied"; break;
		case ID_STATUS_FREECHAT: prefix="FreeChat"; break;
		case ID_STATUS_ONLINE: prefix="On"; break;
		case ID_STATUS_OFFLINE: prefix="Off"; break;
		case ID_STATUS_INVISIBLE: prefix="Inv"; break;
		case ID_STATUS_ONTHEPHONE: prefix="Otp"; break;
		case ID_STATUS_OUTTOLUNCH: prefix="Otl"; break;
		case ID_STATUS_IDLE: prefix="Idl"; break;
		default: return NULL;
	}
	lstrcpy(str,prefix); lstrcat(str,suffix);
	return str;
}

char *GetDefaultMessage(int status) {
	switch(status) {
		case ID_STATUS_AWAY: return Translate("I've been away since %time%.");
		case ID_STATUS_NA: return Translate("Give it up, I'm not in!");
		case ID_STATUS_OCCUPIED: return Translate("Not right now.");
		case ID_STATUS_DND: return Translate("Give a guy some peace, would ya?");
		case ID_STATUS_FREECHAT: return Translate("I'm a chatbot!");
		case ID_STATUS_ONLINE: return Translate("Yep, I'm here.");
		case ID_STATUS_OFFLINE: return Translate("Nope, not here.");
		case ID_STATUS_INVISIBLE: return Translate("I'm hiding from the mafia.");
		case ID_STATUS_ONTHEPHONE: return Translate("That'll be the phone.");
		case ID_STATUS_OUTTOLUNCH: return Translate("Mmm...food.");
		case ID_STATUS_IDLE: return Translate("idleeeeeeee");
	}
	return NULL;
}

#define WM_WA_IPC WM_USER
#define IPC_ISPLAYING 104

void WinampTitle(char *title) {
	char *pstr;

	if (pstr=strstr(title, " - Winamp *** ")) {
		if  (lstrlen(title)-((pstr-title)+14) == 0) {
			title[(pstr-title)+9]='\0';

			if (pstr=strstr(title,"** ")) {
				if (pstr-title+3<=5) {
					strncpy(title, title+(pstr-title)+3, lstrlen(title)-(pstr-title+3));
					title[lstrlen(title)-(pstr-title+3)]='\0';
				}
			}
		}
		else {
			strncpy(title, title+(pstr-title)+14, lstrlen(title)-(pstr-title+14));
			title[lstrlen(title)-(pstr-title+14)]='\0';
		}
	}
	else if (pstr=strstr(title, "p *** ")) {
		if (pstr-title+6<=13) {
			strncpy(title, title+(pstr-title)+6, lstrlen(title)-(pstr-title+6));
			title[lstrlen(title)-(pstr-title+6)]='\0';
		}
	}
}

char *InsertVarsIntoMsg2(char *in, char *proto_name, int status) {
	int i, count=0, len;
	char substituteStr[1024];
	char winamp_title[2048];
	char *p;
	char *msg;
	char buff[128];

	msg = mir_strdup(in);

	for(i=0;msg[i];i++) {
		if(msg[i] == 0x0D && removeCR) {
			p = msg+i;
			if (i+1 <= 1024 && msg[i+1]) {
				if (msg[i+1] == 0x0A) {
					if (i+2 <= 1024 && msg[i+2]) {
						count++;
						MoveMemory(p, p+1, lstrlen(p)-1);
					}
					else {
						msg[i+1] = 0;
						msg[i] = 0x0A;
					}
				}
			}
		}

		if(msg[i]!='%')
			continue;

		if(!_strnicmp(msg+i,"%winampsong%",12)) {
			HWND hwndWinamp = FindWindow("Winamp v1.x",NULL);

			if (hwndWinamp) {
				if (SendMessage(hwndWinamp, WM_WA_IPC, 0, IPC_ISPLAYING) != 1)
					continue;

				GetWindowText(hwndWinamp, winamp_title, sizeof(winamp_title));
				WinampTitle(winamp_title);
			}
			else if (strcmp(winampsong, "SimpleAway")
				&& DBGetContactSettingByte(NULL, "SimpleAway", "AmpLeaveTitle", 1)) {
				strcpy(winamp_title, winampsong);
			}
			else
				continue;

			p = winamp_title;

			while (*p != ' ' && p<winamp_title+sizeof(winamp_title))
				p++;
			p++;

			if (p<winamp_title+sizeof(winamp_title))
				strncpy(winamp_title, p, sizeof(winamp_title)-(p-winamp_title));

//			p = winamp_title+strlen(winamp_title)-8;
			p = winamp_title+sizeof(winamp_title)-1;
			while (p>=winamp_title && strnicmp(p,"- Winamp",8))
				p--;

			if (p>=winamp_title)
				p--;

			if (*p != ' ')
				*p = '\0';
			else {
				while (p>=winamp_title && *p == ' ')
					p--;
				*++p=0;
			}
					
			if(lstrlen(winamp_title)>12)
				msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(winamp_title)-12);

			MoveMemory(msg+i+lstrlen(winamp_title), msg+i+12, lstrlen(msg)-i-11);
			CopyMemory(msg+i, winamp_title, lstrlen(winamp_title));
		}
		else if(!_strnicmp(msg+i,"%fortunemsg%",12)) {
			char	*FortuneMsg;
			
			if (!ServiceExists(MS_FORTUNEMSG_GETMESSAGE))
				continue;

			FortuneMsg = (char*)CallService(MS_FORTUNEMSG_GETMESSAGE, 0, 0);

			if(lstrlen(FortuneMsg)>12)
				msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(FortuneMsg)-12);

			MoveMemory(msg+i+lstrlen(FortuneMsg),msg+i+12,lstrlen(msg)-i-11);
			CopyMemory(msg+i,FortuneMsg,lstrlen(FortuneMsg));
			
			CallService(MS_FORTUNEMSG_FREEMEMORY, 0, (LPARAM)FortuneMsg);
		}
		else if(!_strnicmp(msg+i,"%protofortunemsg%",17)) {
			char	*FortuneMsg;
			
			if (!ServiceExists(MS_FORTUNEMSG_GETPROTOMSG))
				continue;

			FortuneMsg = (char*)CallService(MS_FORTUNEMSG_GETPROTOMSG, (WPARAM)proto_name, 0);

			if(lstrlen(FortuneMsg)>17)
				msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(FortuneMsg)-17);

			MoveMemory(msg+i+lstrlen(FortuneMsg),msg+i+17,lstrlen(msg)-i-16);
			CopyMemory(msg+i,FortuneMsg,lstrlen(FortuneMsg));
			
			CallService(MS_FORTUNEMSG_FREEMEMORY, 0, (LPARAM)FortuneMsg);
		}
		else if(!_strnicmp(msg+i,"%statusfortunemsg%",18)) {
			char	*FortuneMsg;
			
			if (!ServiceExists(MS_FORTUNEMSG_GETSTATUSMSG))
				continue;

			FortuneMsg = (char*)CallService(MS_FORTUNEMSG_GETSTATUSMSG, (WPARAM)status, 0);

			if(lstrlen(FortuneMsg)>18)
				msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(FortuneMsg)-18);

			MoveMemory(msg+i+lstrlen(FortuneMsg),msg+i+18,lstrlen(msg)-i-17);
			CopyMemory(msg+i,FortuneMsg,lstrlen(FortuneMsg));
			
			CallService(MS_FORTUNEMSG_FREEMEMORY, 0, (LPARAM)FortuneMsg);
		}
		else if(!_strnicmp(msg+i,"%time%",6)) {
			GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOSECONDS,NULL,NULL,substituteStr,sizeof(substituteStr));

			if(lstrlen(substituteStr)>6)
				msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(substituteStr)-6);

			MoveMemory(msg+i+lstrlen(substituteStr),msg+i+6,lstrlen(msg)-i-5);
			CopyMemory(msg+i,substituteStr,lstrlen(substituteStr));
		}
		else if(!_strnicmp(msg+i,"%rand(",6)) {
			char	*temp;
			int		ran_from;
			int		ran_to;
			char	*token;
			int		k;

			temp = mir_strdup(msg+i+6);

			token = strtok(temp, ",)");
			ran_from = atoi(token);
			token = strtok(NULL, ",)%%");
			ran_to = atoi(token);

			if (ran_to > ran_from) {
				_snprintf(substituteStr, sizeof(substituteStr), "%d", ranfr(ran_from, ran_to));

				for (k=i+1; msg[k]; k++) {
					if (msg[k] == '%') {
						k++;
						break;
					}
				}

				if(lstrlen(substituteStr) > k-i)
					msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(substituteStr)-(k-i));

				MoveMemory(msg+i+lstrlen(substituteStr),msg+i+(k-i),lstrlen(msg)-i-(k-i-1));
				CopyMemory(msg+i,substituteStr,lstrlen(substituteStr));
			}
			mir_free(temp);
		}
		else if(!_strnicmp(msg+i,"%date%",6)) {
			GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,NULL,NULL,substituteStr,sizeof(substituteStr));

			if(lstrlen(substituteStr)>6)
				msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(substituteStr)-6);

			MoveMemory(msg+i+lstrlen(substituteStr),msg+i+6,lstrlen(msg)-i-5);
			CopyMemory(msg+i,substituteStr,lstrlen(substituteStr));
		}
		else if(!_strnicmp(msg+i,"%randmsg%",9)) {
			char		buff1[16];
			int			k, maxk, k2=0;
			DBVARIANT	dbv;
			BOOL		rmark[25];

			for (k=0; k<26; k++)
				rmark[k] = FALSE;

			maxk = DBGetContactSettingByte(NULL, "SimpleAway", "MaxHist", 10);

			if (maxk==0)
				rmark[0] = TRUE;

			while (!rmark[0]) {
				k = ranfr(1, maxk);
				if (rmark[k])
					continue;

				rmark[k] = TRUE;
				k2++;
				if ((k2==maxk) || (k2>maxk))
					rmark[0] = TRUE;

				_snprintf(buff1, sizeof(buff1), "SMsg%d", k);
				if (!DBGetContactSetting(NULL, "SimpleAway", buff1, &dbv)) {//0 - no error
					if (dbv.pszVal)
						strcpy(substituteStr, dbv.pszVal);
					else {
						DBFreeVariant(&dbv);
						continue;
					}
					DBFreeVariant(&dbv);
				}
				else
					continue;

				if (!lstrlen(substituteStr))
					continue;

				if ((strstr(substituteStr, "%randmsg%") != NULL) || (strstr(substituteStr, "%randdefmsg%") != NULL)) {
					if (k==maxk)
						maxk--;
				}
				else
					rmark[0] = TRUE;
			}

			if ((k2==maxk) || (k2>maxk))
				strcpy(substituteStr, "");

			if(lstrlen(substituteStr)>9)
				msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(substituteStr)-9);

			MoveMemory(msg+i+lstrlen(substituteStr),msg+i+9,lstrlen(msg)-i-8);
			CopyMemory(msg+i,substituteStr,lstrlen(substituteStr));
		}
		else if(!_strnicmp(msg+i,"%randdefmsg%",12)) {
			char		buff1[16];
			int			k, maxk, k2=0;
			DBVARIANT	dbv;
			BOOL		rmark[25];

			for (k=0; k<26; k++)
				rmark[k] = FALSE;

			maxk = DBGetContactSettingWord(NULL, "SimpleAway", "DefMsgCount", 0);

			if (maxk==0)
				rmark[0] = TRUE;

			while (!rmark[0]) {
				k = ranfr(1, maxk);
				if (rmark[k])
					continue;

				rmark[k] = TRUE;
				k2++;
				if ((k2==maxk) || (k2>maxk))
					rmark[0] = TRUE;

				_snprintf(buff1, sizeof(buff1), "DefMsg%d", k);
				if (!DBGetContactSetting(NULL, "SimpleAway", buff1, &dbv)) {//0 - no error
					if (dbv.pszVal)
						strcpy(substituteStr, dbv.pszVal);
					else {
						DBFreeVariant(&dbv);
						continue;
					}
					DBFreeVariant(&dbv);
				}
				else
					continue;

				if (!lstrlen(substituteStr))
					continue;

				if ((strstr(substituteStr, "%randmsg%") != NULL) || (strstr(substituteStr, "%randdefmsg%") != NULL)) {
					if (k==maxk)
						maxk--;
				}
				else
					rmark[0] = TRUE;
			}

			if ((k2==maxk) || (k2>maxk))
				strcpy(substituteStr, "");

			if(lstrlen(substituteStr)>12)
				msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(substituteStr)-12);

			MoveMemory(msg+i+lstrlen(substituteStr),msg+i+12,lstrlen(msg)-i-11);
			CopyMemory(msg+i,substituteStr,lstrlen(substituteStr));
		}
	}
	if (count)
		msg[lstrlen(msg)-count] = 0;

	if (proto_name) {
		_snprintf(buff, sizeof(buff), "Proto%sMaxLen", proto_name);
		len = DBGetContactSettingWord(NULL, "SimpleAway", buff, 1024);
		if (len < lstrlen(msg)) {
			msg = (char*)mir_realloc(msg, len);
			msg[len] = 0;
		}
	}

	return msg;
}

char *InsertVarsIntoMsg(char *msg, char *proto_name, int status) {
	char	*format;

	format = InsertVarsIntoMsg2(msg, proto_name, status);

	if (ServiceExists(MS_VARS_FORMATSTRING)) {
		FORMATINFO		fInfo;
		char			*after_format;

		ZeroMemory(&fInfo, sizeof(fInfo));
		fInfo.cbSize = sizeof(FORMATINFO);
		fInfo.szFormat = format;
		after_format = (char *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fInfo, 0);
		if (after_format == NULL)
			return format;
		mir_free(format);
		format = mir_strdup(after_format);
		CallService(MS_VARS_FREEMEMORY, (WPARAM)after_format, 0);
		return format;
	}
	else
		return format;
}

static char *GetAwayMessageFormat(WPARAM wParam, LPARAM lParam) {
	DBVARIANT		dbv, dbv2;
	int				statusMode = (int)wParam;
	int				flags;
	char			*format, setting[80];

	if (lParam)
		_snprintf(setting, sizeof(setting), "%sFlags", (char *)lParam);
	else
		_snprintf(setting, sizeof(setting), "Flags");
	flags = DBGetContactSettingByte(NULL, "SimpleAway", (char *)StatusModeToDbSetting(statusMode, setting), STATUS_SHOW_DLG|STATUS_LAST_MSG);

	if (flags & STATUS_EMPTY_MSG)
		return mir_strdup("");

	if (flags & STATUS_LAST_STATUS_MSG) {
		if (lParam)
			_snprintf(setting, sizeof(setting), "%sMsg", (char *)lParam);
		else
			_snprintf(setting, sizeof(setting), "Msg");

		if(DBGetContactSetting(NULL,"SRAway",StatusModeToDbSetting(statusMode, setting),&dbv))
			return mir_strdup("");
		else {
			format = mir_strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	else if (flags & STATUS_LAST_MSG) {
		if (lParam)
			_snprintf(setting, sizeof(setting), "Last%sMsg", (char *)lParam);
		else
			_snprintf(setting, sizeof(setting), "LastMsg");

		if (DBGetContactSetting(NULL, "SimpleAway", setting, &dbv2))
			return mir_strdup("");
		else {
			if (DBGetContactSetting(NULL, "SimpleAway", dbv2.pszVal, &dbv))
				return mir_strdup("");
			else {
				format = mir_strdup(dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			DBFreeVariant(&dbv2);
		}
	}
	else if (flags & STATUS_THIS_MSG) {
		if (lParam)
			_snprintf(setting, sizeof(setting), "%sDefault", (char *)lParam);
		else
			_snprintf(setting, sizeof(setting), "Default");

		if(DBGetContactSetting(NULL,"SRAway",StatusModeToDbSetting(statusMode,setting),&dbv))
			return mir_strdup("");
		else {
			format = mir_strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	else
		format = mir_strdup(GetDefaultMessage(statusMode));

	return format;
}

void DBWriteMessage(char *buff, char *message) {
	if (message) {
		if (!lstrlen(message))
			DBDeleteContactSetting(NULL, "SimpleAway", buff);
		else
			DBWriteContactSettingString(NULL, "SimpleAway", buff, message);
	}
	else
		DBDeleteContactSetting(NULL, "SimpleAway", buff);
}

void SaveMessageToDB(char *proto, char *message, BOOL is_format) {
	char	buff[80];
		
	if (!proto) {
		int		i;
		
		for (i=0; i<ProtoCount; i++) {
			if (protocols[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (!CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
				continue;
			
			if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
				continue;

			if (is_format)
				_snprintf(buff, sizeof(buff), "FCur%sMsg", protocols[i]->szName);
			else
				_snprintf(buff, sizeof(buff), "Cur%sMsg", protocols[i]->szName);
			DBWriteMessage(buff, message);

		#ifdef _DEBUG
			log2file("SaveMessageToDB(): Set \"%s\" status message for %s.", message, protocols[i]->szName);
		#endif
		}
	}
	else {
		if (!(CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
			return;

		if (is_format)
			_snprintf(buff, sizeof(buff), "FCur%sMsg", proto);
		else
			_snprintf(buff, sizeof(buff), "Cur%sMsg", proto);
		DBWriteMessage(buff, message);

	#ifdef _DEBUG
		log2file("SaveMessageToDB(): Set \"%s\" status message for %s.", message, proto);
	#endif
	}
}

void SaveStatusAsCurrent(char *proto_name, int status) {
	char	setting[80];

	_snprintf(setting, sizeof(setting), "Cur%sStatus", proto_name);
	DBWriteContactSettingWord(NULL, "SimpleAway", setting, (WORD)status);
}

//remember to mir_free() the return value
static int GetAwayMessage(WPARAM wParam, LPARAM lParam) {
	char			*format, *ret, setting[80];
	int				flags;

	_snprintf(setting, sizeof(setting), "Proto%sFlags", (char *)lParam);
	flags = DBGetContactSettingByte(NULL, "SimpleAway", setting, PROTO_POPUPDLG);

	if (flags & PROTO_NO_MSG) {
		format = mir_strdup("");
	}
	else if (flags & PROTO_THIS_MSG) {
		DBVARIANT			dbv;

		_snprintf(setting, sizeof(setting), "Proto%sDefault", (char *)lParam);
		if(!DBGetContactSetting(NULL, "SimpleAway", setting, &dbv)) {
			format = mir_strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		else
			format = mir_strdup("");
	}
	else {
		format = GetAwayMessageFormat(wParam, lParam);
	}

#ifdef _DEBUG
	log2file("GetAwayMessage(): %s has %s status and \"%s\" status message.", (char *)lParam, StatusModeToDbSetting((int)wParam, ""), format);
#endif
	if (!format)
		return (int)NULL;

	ret = InsertVarsIntoMsg(format, (char *)lParam, (int)wParam);
	mir_free(format);

	if (ret) {
		char *tmp = mir_strdup(ret);
		mir_free(ret);
		return (int)tmp;
	}
	else {
		return (int)mir_strdup("");
	}
}

int	CheckProtoSettings(char *proto_name, int initial_status_mode) {
	int	check;

	check = DBGetContactSettingWord(NULL, proto_name, "LeaveStatus", -1); //GG settings
	if (check != -1) {
		if (check == 0)
			return initial_status_mode;
		else
			return check;
	}
	check = DBGetContactSettingWord(NULL, proto_name, "OfflineMessageOption", -1); //TLEN settings
	if (check != -1) {
		if (check == 0)
			return initial_status_mode;
		else {
			switch(check) {
				case 1: return ID_STATUS_ONLINE; break;
				case 2: return ID_STATUS_AWAY; break;
				case 3: return ID_STATUS_NA; break;
				case 4: return ID_STATUS_DND; break;
				case 5: return ID_STATUS_FREECHAT; break;
				case 6: return ID_STATUS_INVISIBLE; break;
				default: return initial_status_mode;
			}
		}
	}
	return initial_status_mode;
}

void DisableKeepStatus(char *proto) {
	if (ServiceExists(MS_KS_ENABLEPROTOCOL))
		CallService(MS_KS_ENABLEPROTOCOL, 0, (LPARAM)proto);
}

void EnableKeepStatus(char *proto) {
	if (ServiceExists(MS_KS_ISPROTOCOLENABLED)) {
		if (!CallService(MS_KS_ISPROTOCOLENABLED, 0, (LPARAM)proto)) {
			if (ServiceExists(MS_KS_ENABLEPROTOCOL))
				CallService(MS_KS_ENABLEPROTOCOL, 1, (LPARAM)proto);
		}
	}
}

int HasProtoStaticStatusMsg(char *proto, int initial_status, int status) {
	char	setting[80];
	int		flags;

	_snprintf(setting, sizeof(setting), "Proto%sFlags", proto);
	flags = DBGetContactSettingByte(NULL, "SimpleAway", setting, PROTO_POPUPDLG);

	if (flags & PROTO_NO_MSG) {
		if ((initial_status != status) && (status == ID_STATUS_OFFLINE))
			DisableKeepStatus(proto);
		else if (initial_status != status)
			EnableKeepStatus(proto);
		CallProtoService(proto, PS_SETSTATUS, (WPARAM)status, 0);
		SaveStatusAsCurrent(proto, status);
		if (!(CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
			CallProtoService(proto, PS_SETAWAYMSG, (WPARAM)status, (LPARAM)"");
		SaveMessageToDB(proto, NULL, TRUE);
		SaveMessageToDB(proto, NULL, FALSE);
		return 1;
	}
	else if (flags & PROTO_THIS_MSG) {
		DBVARIANT			dbv;
		char				*msg;

		_snprintf(setting, sizeof(setting), "Proto%sDefault", proto);
		if(!DBGetContactSetting(NULL, "SimpleAway", setting, &dbv)) {
			SaveMessageToDB(proto, dbv.pszVal, TRUE);
			msg = InsertVarsIntoMsg(dbv.pszVal, proto, status);
			DBFreeVariant(&dbv);

			if ((initial_status != status) && (status == ID_STATUS_OFFLINE)) {
				DisableKeepStatus(proto);

				if (!(CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG)) {
					int		status_from_proto_settings;

					status_from_proto_settings = CheckProtoSettings(proto, initial_status);

					CallProtoService(proto, PS_SETSTATUS, (WPARAM)status_from_proto_settings, 0);
					CallProtoService(proto, PS_SETAWAYMSG, (WPARAM)status_from_proto_settings, (LPARAM)msg);
				}
				CallProtoService(proto, PS_SETSTATUS, (WPARAM)status, 0);
			}
			else {
				EnableKeepStatus(proto);
				CallProtoService(proto, PS_SETSTATUS, (WPARAM)status, 0);
				if (!(CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
					CallProtoService(proto, PS_SETAWAYMSG, (WPARAM)status, (LPARAM)msg);
			}
			SaveMessageToDB(proto, msg, FALSE);
			mir_free(msg);
		}
		else {
			if ((initial_status != status) && (status == ID_STATUS_OFFLINE)) {
				DisableKeepStatus(proto);
				CallProtoService(proto, PS_SETSTATUS, (WPARAM)status, 0);
			}
			else {
				EnableKeepStatus(proto);
				CallProtoService(proto, PS_SETSTATUS, (WPARAM)status, 0);
				if (!(CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
					CallProtoService(proto, PS_SETAWAYMSG, (WPARAM)status, (LPARAM)"");
			}
			SaveMessageToDB(proto, NULL, TRUE);
			SaveMessageToDB(proto, NULL, FALSE);
		}
		SaveStatusAsCurrent(proto, status);
		return 1;
	}
	return 0;
}

int SetStatusModeFromExtern(WPARAM wParam, LPARAM lParam) {
	int	i, status_modes_msg;
	char	buff[80];
	BOOL	currentstatus=FALSE;
	
	if (wParam < ID_STATUS_OFFLINE || (wParam > ID_STATUS_OUTTOLUNCH && wParam != ID_STATUS_CURRENT))
		return 0;
		
	for (i=0; i<ProtoCount; i++) {
		if (protocols[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0)))
			continue;

		if (DBGetContactSettingByte(NULL, protocols[i]->szName, "LockMainStatus", 0) == 1)
			continue;
	
		if (currentstatus)
			wParam = ID_STATUS_CURRENT;

		if (wParam == ID_STATUS_CURRENT) {
			_snprintf(buff, sizeof(buff), "Cur%sStatus", protocols[i]->szName);
			wParam = DBGetContactSettingWord(NULL, "SimpleAway", buff, ID_STATUS_OFFLINE);
			if (!currentstatus)
				currentstatus=TRUE;
		}

		if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND)) {
			CallProtoService(protocols[i]->szName, PS_SETSTATUS, wParam, 0);
			SaveStatusAsCurrent(protocols[i]->szName, (int)wParam);
			continue;
		}
			
		status_modes_msg = CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0);

		if ((Proto_Status2Flag(wParam) & status_modes_msg) || (wParam == ID_STATUS_OFFLINE && (Proto_Status2Flag(ID_STATUS_INVISIBLE) & status_modes_msg))) {
			if (HasProtoStaticStatusMsg(protocols[i]->szName, (int)wParam, (int)wParam))
				continue;
				
			if (lParam && wParam == ID_STATUS_OFFLINE) {//ugly hack to set offline status message
				DisableKeepStatus(protocols[i]->szName);

				if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG)) {
					int		status_from_proto_settings;

					status_from_proto_settings = CheckProtoSettings(protocols[i]->szName, wParam);

					CallProtoService(protocols[i]->szName, PS_SETSTATUS, (WPARAM)status_from_proto_settings, 0);
					CallProtoService(protocols[i]->szName, PS_SETAWAYMSG, (WPARAM)status_from_proto_settings, lParam);
				}
				CallProtoService(protocols[i]->szName, PS_SETSTATUS, wParam, 0);
				SaveStatusAsCurrent(protocols[i]->szName, (int)wParam);
				continue;
			}
			
			CallProtoService(protocols[i]->szName, PS_SETSTATUS, wParam, 0);
			SaveStatusAsCurrent(protocols[i]->szName, (int)wParam);
			EnableKeepStatus(protocols[i]->szName);

			if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG)) {
				if (lParam)
					CallProtoService(protocols[i]->szName, PS_SETAWAYMSG, wParam, lParam);
				else
					CallProtoService(protocols[i]->szName, PS_SETAWAYMSG, wParam, (LPARAM)"");
			}
		}
		else {
			CallProtoService(protocols[i]->szName, PS_SETSTATUS, wParam, 0);
			SaveStatusAsCurrent(protocols[i]->szName, (int)wParam);
			continue;
		}
	}
	return 0;
}

int ChangeStatusMessage(WPARAM wParam,LPARAM lParam);

void SetStatusMessage(char *proto_name, int initial_status_mode, int status_mode, char *message) {
	char	*msg=NULL;

	if (proto_name) {
		char	setting[80];

		if (message)
			msg = InsertVarsIntoMsg(message, proto_name, status_mode);
			
		SaveMessageToDB(proto_name, message, TRUE);
		SaveMessageToDB(proto_name, msg, FALSE);

		if (initial_status_mode == ID_STATUS_CURRENT) {
			_snprintf(setting, sizeof(setting), "Cur%sStatus", proto_name);
			initial_status_mode = DBGetContactSettingWord(NULL, "SimpleAway", setting, ID_STATUS_OFFLINE);
		}

		if (initial_status_mode != status_mode) {
			if (msg && status_mode == ID_STATUS_OFFLINE) {//ugly hack to set offline status message
				DisableKeepStatus(proto_name);

				if (!(CallProtoService(proto_name, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG)) {
					int		status_from_proto_settings;

					status_from_proto_settings = CheckProtoSettings(proto_name, initial_status_mode);

					CallProtoService(proto_name, PS_SETSTATUS, (WPARAM)status_from_proto_settings, 0);
					CallProtoService(proto_name, PS_SETAWAYMSG, (WPARAM)status_from_proto_settings, (LPARAM)msg);
				}
				CallProtoService(proto_name, PS_SETSTATUS, (WPARAM)status_mode, 0);
				SaveStatusAsCurrent(proto_name, status_mode);
				mir_free(msg);
				return;
			}
		}

		CallProtoService(proto_name,PS_SETSTATUS, (WPARAM)status_mode, 0);
		SaveStatusAsCurrent(proto_name, status_mode);
		EnableKeepStatus(proto_name);

		if (msg) {
			if (!(CallProtoService(proto_name, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
				CallProtoService(proto_name,PS_SETAWAYMSG, (WPARAM)status_mode, (LPARAM)msg);
			mir_free(msg);
		}
		else {
			if (!(CallProtoService(proto_name, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
				CallProtoService(proto_name,PS_SETAWAYMSG, (WPARAM)status_mode, (LPARAM)"");
		}
	}
	else {
		int					proto_count, i, pflags=0, profilestatus=0;
		PROTOCOLDESCRIPTOR	**proto;
		BOOL				currentstatus=FALSE, initial_currentstatus=FALSE;
		char				setting[128];

		CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
		for(i=0; i<proto_count; i++) {
			if (proto[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
				continue;

			if (DBGetContactSettingByte(NULL, proto[i]->szName, "LockMainStatus", 0) == 1)
				continue;

			pflags = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0);

			if (profilestatus)
				status_mode = profilestatus;

			if ((status_mode > ID_STATUS_CURRENT) && (pflags & PF1_MODEMSGSEND)) {
				int profilenumber = status_mode-40083;

				profilestatus = status_mode;
				_snprintf(setting, sizeof(setting), "%d_%s", profilenumber, proto[i]->szName);
				status_mode = DBGetContactSettingWord(NULL, "StartupStatus", setting, ID_STATUS_OFFLINE);
				if (status_mode == ID_STATUS_IDLE) {//the same as ID_STATUS_LAST in StartupStatus
					_snprintf(setting, sizeof(setting), "last_%s", proto[i]->szName);
					status_mode = DBGetContactSettingWord(NULL, "StartupStatus", setting, ID_STATUS_OFFLINE);
				}
				else if (status_mode == ID_STATUS_CURRENT) {
					_snprintf(setting, sizeof(setting), "Cur%sStatus", proto[i]->szName);
					status_mode = DBGetContactSettingWord(NULL, "SimpleAway", setting, ID_STATUS_OFFLINE);
				}
			}

			if (currentstatus)
				status_mode = ID_STATUS_CURRENT;

			if (!(pflags & PF1_MODEMSGSEND)) {
				if (status_mode != ID_STATUS_CURRENT)
					SaveStatusAsCurrent(proto[i]->szName, status_mode);
				continue;
			}

			if (status_mode == ID_STATUS_CURRENT) {
				_snprintf(setting, sizeof(setting), "Cur%sStatus", proto[i]->szName);
				status_mode = DBGetContactSettingWord(NULL, "SimpleAway", setting, ID_STATUS_OFFLINE);
				if (!currentstatus)
					currentstatus=TRUE;
			}

			if (initial_currentstatus)
				initial_status_mode = ID_STATUS_CURRENT;

			if (initial_status_mode == ID_STATUS_CURRENT) {
				_snprintf(setting, sizeof(setting), "Cur%sStatus", proto[i]->szName);
				initial_status_mode = DBGetContactSettingWord(NULL, "SimpleAway", setting, ID_STATUS_OFFLINE);
				if (!initial_currentstatus)
					initial_currentstatus=TRUE;
			}

			if (HasProtoStaticStatusMsg(proto[i]->szName, initial_status_mode, status_mode))
				continue;

			if (message)
				msg = InsertVarsIntoMsg(message, proto[i]->szName, status_mode);

			SaveMessageToDB(proto[i]->szName, message, TRUE);
			SaveMessageToDB(proto[i]->szName, msg, FALSE);

			if (initial_status_mode != status_mode) {
				if (msg && status_mode == ID_STATUS_OFFLINE) {//ugly hack to set offline status message
					DisableKeepStatus(proto[i]->szName);

					if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG)) {
						int		status_from_proto_settings;

						status_from_proto_settings = CheckProtoSettings(proto[i]->szName, initial_status_mode);

						CallProtoService(proto[i]->szName, PS_SETSTATUS, (WPARAM)status_from_proto_settings, 0);
						CallProtoService(proto[i]->szName, PS_SETAWAYMSG, (WPARAM)status_from_proto_settings, (LPARAM)msg);
					}
					CallProtoService(proto[i]->szName, PS_SETSTATUS, (WPARAM)status_mode, 0);
					SaveStatusAsCurrent(proto[i]->szName, status_mode);
					mir_free(msg);
					continue;
				}
			}

			CallProtoService(proto[i]->szName, PS_SETSTATUS, (WPARAM)status_mode, 0);
			SaveStatusAsCurrent(proto[i]->szName, status_mode);
			EnableKeepStatus(proto[i]->szName);

			
			if (msg) {
				if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
					CallProtoService(proto[i]->szName, PS_SETAWAYMSG, (WPARAM)status_mode, (LPARAM)msg);
				mir_free(msg);
			}
			else {
				if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
					CallProtoService(proto[i]->szName, PS_SETAWAYMSG, (WPARAM)status_mode, (LPARAM)"");
			}
		}

//		if (CallService(MS_CLIST_GETSTATUSMODE, 0, 0) != status_mode)
		if ((CallService(MS_CLIST_GETSTATUSMODE, 0, 0) != status_mode) && (!currentstatus) && (!profilestatus)) {
			UnhookEvent(h_statusmodechange); //my beautiful haxor part 1
			CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)status_mode, 0);
			h_statusmodechange = HookEvent(ME_CLIST_STATUSMODECHANGE, ChangeStatusMessage); //my beautiful haxor part 2
		}
	}
}

int TTChangeStatusMessage(WPARAM wParam,LPARAM lParam) {
	struct MsgBoxInitData	*box_data;
	int						i, count;
	PROTOCOLDESCRIPTOR		**proto;
	BOOL					idvstatusmsg=FALSE;
	
	if (terminated)
		return 0;
		
	if (ServiceExists(MS_TTB_SETBUTTONSTATE) && TopButton) {
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)TopButton, (LPARAM)TTBST_RELEASED);
		CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM((WORD)TTBO_TIPNAME, (WORD)TopButton), (LPARAM)Translate("Change Status Message"));
	}

	box_data = (struct MsgBoxInitData *) mir_alloc(sizeof(struct MsgBoxInitData));

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&proto);
	if (ProtoStatusMsgCount == 1) {
		for (i=0; i<count; i++) {
			if (proto[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
				continue;

			if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
				continue;

			box_data->proto_name = proto[i]->szName;
			box_data->all_modes = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0);
			box_data->all_modes_msg = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0);
			break;
		}
	}
	else {
		for (i=0; i<count; i++) {
			if (proto[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
				continue;

			if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
				continue;

			if (!GetProtocolVisibility(proto[i]->szName))
				continue;
	
			if (hProtoStatusMenuItem[i]==(HANDLE)lParam) {
				box_data->proto_name = proto[i]->szName;
				box_data->all_modes = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0);
				box_data->all_modes_msg = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0);

				idvstatusmsg = TRUE;
				break;
			}
		}
		if (!idvstatusmsg) {
			box_data->proto_name = NULL;
			box_data->all_modes = ProtoStatusMsgFlags;
			box_data->all_modes_msg = box_data->all_modes;
		}
	}
	box_data->status_mode = ID_STATUS_CURRENT;
	box_data->ttchange = TRUE;

	if (hwndSAMsgDialog)
		DestroyWindow(hwndSAMsgDialog);
	hwndSAMsgDialog = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_AWAYMSGBOX), NULL, AwayMsgBoxDlgProc, (LPARAM)box_data);
	return 0;
}

int ShowStatusMessageChangeDialog(WPARAM wParam,LPARAM lParam) {
	struct MsgBoxInitData	*box_data;
	int						i, count;
	PROTOCOLDESCRIPTOR		**proto;
	BOOL					idvstatusmsg=FALSE;
	
	if (terminated)
		return 0;
		
	box_data = (struct MsgBoxInitData *) mir_alloc(sizeof(struct MsgBoxInitData));

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&proto);
	for (i=0; i<count; i++) {
		if (proto[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
			continue;

		if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
			continue;

		if (!GetProtocolVisibility(proto[i]->szName))
			continue;
	
		if (!strcmp(proto[i]->szName, (char *)lParam)) {
			box_data->proto_name = proto[i]->szName;
			box_data->all_modes = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0);
			box_data->all_modes_msg = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0);

			idvstatusmsg = TRUE;
			break;
		}
	}
	if (!idvstatusmsg) {
		box_data->proto_name = NULL;
		box_data->all_modes = ProtoStatusMsgFlags;
		box_data->all_modes_msg = box_data->all_modes;
	}
	box_data->status_mode = ID_STATUS_CURRENT;
	box_data->ttchange = TRUE;

	if (hwndSAMsgDialog)
		DestroyWindow(hwndSAMsgDialog);
	hwndSAMsgDialog = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_AWAYMSGBOX), NULL, AwayMsgBoxDlgProc, (LPARAM)box_data);
	return 0;
}

int ChangeStatusMessage(WPARAM wParam,LPARAM lParam) {
	int		status_modes=0;
	int		status_modes_msg=0;
	int		dlg_flags;
	BOOL	show_dlg=FALSE, on_startup=FALSE;
	char	buff[80];

	if (terminated)
		return 0;

	if (lParam) {
		if (!strcmp((char *)lParam, "SimpleAwayGlobalStartupStatus")) {
			lParam = (LPARAM)NULL;
			on_startup = TRUE;
		}
	}

	if (ProtoStatusMsgCount == 1 && !lParam) {
		int				proto_count, i;
		PROTOCOLDESCRIPTOR	**proto;

		CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
		for (i=0; i<proto_count; i++) {
			if (proto[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
				continue;

			if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
				continue;

			lParam = (LPARAM)proto[i]->szName;
			break;
		}
	}

	if (lParam)
		_snprintf(buff, sizeof(buff), "%sFlags", (char *)lParam);
	else
		_snprintf(buff, sizeof(buff), "Flags");
	dlg_flags = DBGetContactSettingByte(NULL, "SimpleAway", (char *)StatusModeToDbSetting((int)wParam, buff), STATUS_SHOW_DLG|STATUS_LAST_MSG);
	if (dlg_flags & STATUS_SHOW_DLG)
		show_dlg = TRUE;

	if (lParam) {
		status_modes = CallProtoService((char *)lParam, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService((char *)lParam, PS_GETCAPS, PFLAGNUM_5, 0);

		if ((!status_modes || !(Proto_Status2Flag(wParam) & status_modes)) && (wParam != ID_STATUS_OFFLINE))
			return 0;

		if (CallProtoService((char *)lParam, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) {
			status_modes_msg = CallProtoService((char *)lParam, PS_GETCAPS, PFLAGNUM_3, 0);

			if (!status_modes_msg || !(Proto_Status2Flag(wParam) & status_modes_msg)) {
				SaveStatusAsCurrent((char *)lParam, (int)wParam);
				return 0;
			}
			else {
				struct MsgBoxInitData	*box_data;
				char					setting[80];
				int						flags;

				_snprintf(setting, sizeof(setting), "Proto%sFlags", (char *)lParam);
				flags = DBGetContactSettingByte(NULL, "SimpleAway", setting, PROTO_POPUPDLG);

				if (!(flags & PROTO_POPUPDLG)) {
					if (HasProtoStaticStatusMsg((char*)lParam, (int)wParam, (int)wParam))
						return 1;
				}

				if (!show_dlg) {
					char *msg = GetAwayMessageFormat(wParam, lParam);
				#ifdef _DEBUG
					log2file("ChangeStatusMessage(): Set %s status and \"%s\" status message for %s.", StatusModeToDbSetting((int)wParam, ""), msg, (char *)lParam);
				#endif
					SetStatusMessage((char *)lParam, (int)wParam, (int)wParam, msg);
					if (msg)
						mir_free(msg);
					return 1;
				}

				box_data = (struct MsgBoxInitData *) mir_alloc(sizeof(struct MsgBoxInitData));
				box_data->proto_name = (char *)lParam;

				if (!on_startup)
					SaveStatusAsCurrent((char *)lParam, (int)wParam);
				_snprintf(buff, sizeof(buff), "Cur%sStatus", (char *)lParam);
				if (DBGetContactSettingWord(NULL, "SimpleAway", buff, ID_STATUS_OFFLINE) == (int)wParam)
					box_data->status_mode = ID_STATUS_CURRENT;
				else
					box_data->status_mode = (int)wParam; 

				box_data->all_modes = status_modes;
				box_data->all_modes_msg = status_modes_msg;
				box_data->ttchange = FALSE;

				if (hwndSAMsgDialog)
					DestroyWindow(hwndSAMsgDialog);
				hwndSAMsgDialog = CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_AWAYMSGBOX),NULL,AwayMsgBoxDlgProc,(LPARAM)box_data);
			}
		}
		else
			SaveStatusAsCurrent((char *)lParam, (int)wParam);
	}
	else {
		struct MsgBoxInitData	*box_data;

		if (wParam == ID_STATUS_OFFLINE) {
			int				proto_count, i;
			PROTOCOLDESCRIPTOR	**proto;

			CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
			for(i=0; i<proto_count; i++) {
				if (proto[i]->type != PROTOTYPE_PROTOCOL)
					continue;

				if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0)))
					continue;

				SaveStatusAsCurrent(proto[i]->szName, (int)wParam);
			}
			return 0;
		}

		if (!(ProtoStatusMsgFlags & Proto_Status2Flag(wParam)))
			return 0;

		if (!show_dlg) {
			int					proto_count, i;
			PROTOCOLDESCRIPTOR	**proto;

			CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
			for(i=0; i<proto_count; i++) {
				if (proto[i]->type != PROTOTYPE_PROTOCOL)
					continue;

				if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
					continue;

				if (DBGetContactSettingByte(NULL, proto[i]->szName, "LockMainStatus", 0) == 1)
					continue;

				if (CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) {
					char *msg = GetAwayMessageFormat(wParam, (LPARAM)NULL);
				#ifdef _DEBUG
					log2file("ChangeStatusMessage(): Set %s status and \"%s\" status message for %s.", StatusModeToDbSetting((int)wParam, ""), msg, proto[i]->szName);
				#endif
					SetStatusMessage(proto[i]->szName, (int)wParam, (int)wParam, msg);
					if (msg)
						mir_free(msg);
				}
				else
					SaveStatusAsCurrent(proto[i]->szName, (int)wParam);
			}
			return 1;
		}

		box_data = (struct MsgBoxInitData *) mir_alloc(sizeof(struct MsgBoxInitData));
		box_data->proto_name = NULL;
		box_data->status_mode = (int)wParam;
		box_data->all_modes = ProtoStatusMsgFlags;
		box_data->all_modes_msg = box_data->all_modes;
		box_data->ttchange = FALSE;

		if (hwndSAMsgDialog)
			DestroyWindow(hwndSAMsgDialog);
		hwndSAMsgDialog = CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_AWAYMSGBOX),NULL,AwayMsgBoxDlgProc,(LPARAM)box_data);
	}
	return 0;
}

int SetOfflineStatus(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMessage((WPARAM)ID_STATUS_OFFLINE, (LPARAM)NULL);
	return 0;
}

int SetOnlineStatus(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMessage((WPARAM)ID_STATUS_ONLINE, (LPARAM)NULL);
	return 0;
}

int SetAwayStatus(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMessage((WPARAM)ID_STATUS_AWAY, (LPARAM)NULL);
	return 0;
}

int SetDNDStatus(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMessage((WPARAM)ID_STATUS_DND, (LPARAM)NULL);
	return 0;
}

int SetNAStatus(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMessage((WPARAM)ID_STATUS_NA, (LPARAM)NULL);
	return 0;
}

int SetOccupiedStatus(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMessage((WPARAM)ID_STATUS_OCCUPIED, (LPARAM)NULL);
	return 0;
}

int SetFreeChatStatus(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMessage((WPARAM)ID_STATUS_FREECHAT, (LPARAM)NULL);
	return 0;
}

int SetInvisibleStatus(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMessage((WPARAM)ID_STATUS_INVISIBLE, (LPARAM)NULL);
	return 0;
}

int SetOnThePhoneStatus(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMessage((WPARAM)ID_STATUS_ONTHEPHONE, (LPARAM)NULL);
	return 0;
}

int SetOutToLunchStatus(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMessage((WPARAM)ID_STATUS_OUTTOLUNCH, (LPARAM)NULL);
	return 0;
}

static int ProcessProtoAck(WPARAM wParam,LPARAM lParam) {
	ACKDATA *ack=(ACKDATA *)lParam;

	if ((ack->type == ACKTYPE_AWAYMSG) && (ack->result == ACKRESULT_SENTREQUEST) && !ack->lParam) {
		int		status_mode, flags;
		char	setting[80], *msg=NULL;

		status_mode = CallProtoService((char *)ack->szModule, PS_GETSTATUS, 0, 0);

		_snprintf(setting, sizeof(setting), "Proto%sFlags", (char *)ack->szModule);
		flags = DBGetContactSettingByte(NULL, "SimpleAway", setting, PROTO_POPUPDLG);

		if (flags & PROTO_THIS_MSG) {
			DBVARIANT			dbv;

			_snprintf(setting, sizeof(setting), "Proto%sDefault", (char *)ack->szModule);
			if(!DBGetContactSetting(NULL, "SimpleAway", setting, &dbv)) {
				msg = InsertVarsIntoMsg(dbv.pszVal, (char *)ack->szModule, status_mode);
				DBFreeVariant(&dbv);
			}
		}
		else if (flags & PROTO_POPUPDLG) {
			char	*fmsg = GetAwayMessageFormat((WPARAM)status_mode, (LPARAM)(char *)ack->szModule);
			if (fmsg) {
				msg = InsertVarsIntoMsg(fmsg, (char *)ack->szModule, status_mode);
				mir_free(fmsg);
			}
		}

		CallContactService(ack->hContact, PSS_AWAYMSG, (WPARAM)(HANDLE)ack->hProcess, (LPARAM)(const char*)msg);

		if (msg)
			mir_free(msg);

		return 0;
	}

	if ((ack->type != ACKTYPE_STATUS) && (ack->result != ACKRESULT_SUCCESS) && (ack->hContact))
		return 0;

	if (ack->lParam>=ID_STATUS_CONNECTING && ack->lParam < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)
		return 0;

	if (ack->type == ACKTYPE_STATUS) {
		SaveStatusAsCurrent((char *)ack->szModule, (int)ack->lParam);
	#ifdef _DEBUG
		log2file("ProcessProtoAck(): Set %s status for %s.", StatusModeToDbSetting((int)ack->lParam, ""), (char *)ack->szModule);
	#endif
	}

	return 0;
}

int SetStartupStatus(int i) {
	int		status_mode;
	char	setting[80], *fmsg, *msg=NULL;

	_snprintf(setting, sizeof(setting), "Startup%sStatus", protocols[i]->szName);
	status_mode = DBGetContactSettingWord(NULL, "SimpleAway", setting, ID_STATUS_OFFLINE);

	if (status_mode == ID_STATUS_CURRENT) {//this means load status used last time for this proto
		_snprintf(setting, sizeof(setting), "Last%sStatus", protocols[i]->szName);
		status_mode = DBGetContactSettingWord(NULL, "SimpleAway", setting, ID_STATUS_OFFLINE);
	}

	if (status_mode == ID_STATUS_OFFLINE)
		return -1;

	CallProtoService(protocols[i]->szName, PS_SETSTATUS, (WPARAM)status_mode, 0);
	SaveStatusAsCurrent(protocols[i]->szName, status_mode);

	if (!CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
		return -1;

	if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
		return -1;

	_snprintf(setting, sizeof(setting), "Proto%sFlags", protocols[i]->szName);
	if (!(DBGetContactSettingByte(NULL, "SimpleAway", setting, PROTO_POPUPDLG) & PROTO_POPUPDLG)) {
		if (HasProtoStaticStatusMsg(protocols[i]->szName, ID_STATUS_OFFLINE, status_mode))
			return -1;
	}

	fmsg = GetAwayMessageFormat((WPARAM)status_mode, (LPARAM)protocols[i]->szName);

#ifdef _DEBUG
	log2file("SetStartupStatus(): Set %s status and \"%s\" status message for %s.", StatusModeToDbSetting(status_mode, ""), fmsg, protocols[i]->szName);
#endif

	if (fmsg)
		msg = InsertVarsIntoMsg(fmsg, protocols[i]->szName, status_mode);
			
	SaveMessageToDB(protocols[i]->szName, fmsg, TRUE);
	SaveMessageToDB(protocols[i]->szName, msg, FALSE);

	if (fmsg)
		mir_free(fmsg);

	if (msg) {
		if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
			CallProtoService(protocols[i]->szName, PS_SETAWAYMSG, (WPARAM)status_mode, (LPARAM)msg);
		mir_free(msg);
	}
	else {
		if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
			CallProtoService(protocols[i]->szName, PS_SETAWAYMSG, (WPARAM)status_mode, (LPARAM)"");
	}

	return 0;
}

void CALLBACK SetStartupStatusGlobal(HWND timerhwnd, UINT uMsg, UINT_PTR idEvent, DWORD  dwTime) {
	int		i;

	int		prev_status_mode=-1, status_mode;
	char	setting[80];
	BOOL	globalstatus=TRUE;

	KillTimer(timerhwnd, idEvent);

//is global status mode going to be set?
	for(i=0; i<ProtoCount; i++) {
		if (!CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)protocols[i]->szName))
			continue;

		if (protocols[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0)))
			continue;

		_snprintf(setting, sizeof(setting), "Startup%sStatus", protocols[i]->szName);
		status_mode = DBGetContactSettingWord(NULL, "SimpleAway", setting, ID_STATUS_OFFLINE);

		if (status_mode == ID_STATUS_CURRENT) {//this means load status used last time for this proto
			_snprintf(setting, sizeof(setting), "Last%sStatus", protocols[i]->szName);
			status_mode = DBGetContactSettingWord(NULL, "SimpleAway", setting, ID_STATUS_OFFLINE);
		}

		if (status_mode != prev_status_mode && prev_status_mode != -1) {
			globalstatus=FALSE;
			break;
		}

		prev_status_mode = status_mode;
	}

//popup status msg change dialog as startup?
	if (globalstatus) {
		ChangeStatusMessage((WPARAM)status_mode, (LPARAM)"SimpleAwayGlobalStartupStatus");
		return;
	}

	for(i=0; i<ProtoCount; i++) {
		if (!CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)protocols[i]->szName))
			continue;

		if (protocols[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0)))
			continue;

//		if (DBGetContactSettingByte(NULL, protocols[i]->szName, "LockMainStatus", 0) == 1)
//			continue;

		SetStartupStatus(i);
	}
}

void CALLBACK SetStartupStatusProc(HWND timerhwnd, UINT uMsg, UINT_PTR idEvent, DWORD  dwTime) {
	int		i;
	BOOL	found = FALSE;

	for(i=0; i<ProtoCount; i++) {
		if (!CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)protocols[i]->szName))
			continue;

		if (protocols[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0)))
			continue;

		if (SASetStatusTimer[i] == idEvent) {
			KillTimer(NULL, SASetStatusTimer[i]);
			found = TRUE;
			break;
		}
	}

	if (!found) {
		KillTimer(timerhwnd, idEvent);
		return;
	}

	SetStartupStatus(i);
}

void CALLBACK SATimerProc(HWND timerhwnd, UINT uMsg, UINT_PTR idEvent, DWORD  dwTime) {
	HWND				hwndWinamp = FindWindow("Winamp v1.x",NULL);
	char				winamp_title[2048];
	PROTOCOLDESCRIPTOR	**proto;
	int					proto_count = 0, i;
	char				buff[64];
	DBVARIANT			dbv;
	char				*msg;
	int					status;
	
	if (hwndSAMsgDialog)
		return;

//	#ifdef _DEBUG
//		log2file("SATimerProc(): winampsong = %s.", winampsong);
//	#endif

	if (hwndWinamp) {
		if (SendMessage(hwndWinamp, WM_WA_IPC, 0, IPC_ISPLAYING) != 1)
			return;

		GetWindowText(hwndWinamp, winamp_title, sizeof(winamp_title));
		WinampTitle(winamp_title);

		if (!winampsong)
			winampsong = mir_strdup("SimpleAway");

		if (!strstr(winamp_title, " - Winamp")) {
			mir_free(winampsong);
			winampsong = mir_strdup("SimpleAway");
			return;
		}

		if (!strcmp(winamp_title, winampsong)) //newsong
			return;
	}
	else if (!strcmp(winampsong, "SimpleAway")
		|| DBGetContactSettingByte(NULL, "SimpleAway", "AmpLeaveTitle", 1))
		return;

	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);

	for(i=0; i<proto_count; i++) {
		if (proto[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
			continue;

		if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
			continue;

		status = CallProtoService(proto[i]->szName, PS_GETSTATUS, 0, 0);
		if (status == ID_STATUS_OFFLINE)
			continue;

		_snprintf(buff, sizeof(buff), "FCur%sMsg", proto[i]->szName);
		if(DBGetContactSetting(NULL,"SimpleAway", buff, &dbv))
			continue;

		if (!strstr(dbv.pszVal, "%winampsong%")) {
			DBFreeVariant(&dbv);
			continue;
		}

		msg = InsertVarsIntoMsg(dbv.pszVal, proto[i]->szName, status);
		DBFreeVariant(&dbv);

	#ifdef _DEBUG
		log2file("SATimerProc(): Set %s status and \"%s\" status message for %s.", StatusModeToDbSetting(status, ""), msg, proto[i]->szName);
	#endif
		CallProtoService(proto[i]->szName, PS_SETSTATUS, (WPARAM)status, 0);
		if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
			CallProtoService(proto[i]->szName, PS_SETAWAYMSG, (WPARAM)status, (LPARAM)msg);

		SaveMessageToDB(proto[i]->szName, msg, FALSE);
		mir_free(msg);
	}

	mir_free(winampsong);
	if (hwndWinamp)
		winampsong = mir_strdup(winamp_title);
	else
		winampsong = mir_strdup("SimpleAway");
}

void CALLBACK SARandMsgTimerProc(HWND timerhwnd, UINT uMsg, UINT_PTR idEvent, DWORD  dwTime) {
	if (!hwndSAMsgDialog) {
		PROTOCOLDESCRIPTOR	**proto;
		int					proto_count = 0, i;
		char				buff[64];
		DBVARIANT			dbv;
		char				*msg;
		int					status;

		CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);

		for(i=0; i<proto_count; i++) {
			if (proto[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
				continue;

			if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
				continue;

			status = CallProtoService(proto[i]->szName, PS_GETSTATUS, 0, 0);
			if (status == ID_STATUS_OFFLINE)
				continue;

			_snprintf(buff, sizeof(buff), "FCur%sMsg", proto[i]->szName);
			if(DBGetContactSetting(NULL,"SimpleAway", buff, &dbv))
				continue;

			if ((!strstr(dbv.pszVal, "%randmsg%")) && (!strstr(dbv.pszVal, "%randdefmsg%"))) {
				DBFreeVariant(&dbv);
				continue;
			}

			msg = InsertVarsIntoMsg(dbv.pszVal, proto[i]->szName, status);
			DBFreeVariant(&dbv);

		#ifdef _DEBUG
			log2file("SARandMsgTimerProc(): Set %s status and \"%s\" status message for %s.", StatusModeToDbSetting(status, ""), msg, proto[i]->szName);
		#endif
			CallProtoService(proto[i]->szName, PS_SETSTATUS, (WPARAM)status, 0);

			if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG))
				CallProtoService(proto[i]->szName, PS_SETAWAYMSG, (WPARAM)status, (LPARAM)msg);

			SaveMessageToDB(proto[i]->szName, msg, FALSE);
			mir_free(msg);
		}
	}
}

int AddTopToolbarButton(WPARAM wParam, LPARAM lParam) {
	if (ServiceExists(MS_TTB_ADDBUTTON)) {
		TTBButton sabutton;

		ZeroMemory(&sabutton, sizeof(sabutton));
      	sabutton.cbSize = sizeof(sabutton);
		sabutton.hbBitmapUp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MSGTTB));
		sabutton.hbBitmapDown = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MSGTTB));
		sabutton.pszServiceUp = sabutton.pszServiceDown = MS_SA_TTCHANGESTATUSMSG;
		sabutton.wParamDown = sabutton.wParamUp = (WPARAM)0;
		sabutton.lParamDown = sabutton.lParamUp = (LPARAM)0;
		sabutton.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
		sabutton.name = Translate("Change Status Message");

		TopButton = (HANDLE)CallService(MS_TTB_ADDBUTTON, (WPARAM)&sabutton, 0);
		if (TopButton == (HANDLE)-1)
			return 1;

		CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM((WORD)TTBO_TIPNAME, (WORD)TopButton), (LPARAM)Translate("Change Status Message"));
	}
	
	return 0;
}

char *sa_ico_name[NUM_ICONS] = {
	ICON_DELETE,
	ICON_RECENT,
	ICON_PREDEF,
	ICON_ADD,
	ICON_CLEAR,
	ICON_COPY,
	ICON_CSMSG
};

char *sa_ico_descr[NUM_ICONS] = {
	"Delete Selected",
	"Recent Message",
	"Predefined Message",
	"Add to Predefined",
	"Clear History",
	"Copy Away Message",
	"Change Status Message"
};

int sa_ico_id[NUM_ICONS] = {
	IDI_CROSS,
	IDI_HISTORY,
	IDI_MESSAGE,
	IDI_PLUS,
	IDI_CHIST,
	IDI_COPY,
	IDI_CSMSG
};

static int ChangedIcons(WPARAM wParam,LPARAM lParam) {
	CLISTMENUITEM		mi;
	int					proto_count, i;
	PROTOCOLDESCRIPTOR	**proto;

	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_ICON;
	mi.hIcon = (HICON)CallService(MS_SKIN2_GETICON, (WPARAM)0, (LPARAM)ICON_CSMSG);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hChangeStatusMsgMenuItem, (LPARAM)&mi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hGlobalStatusMenuItem, (LPARAM)&mi);

	if (StatusMenuItemCount == 1)
		return 0;

	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
	for(i=0; i<proto_count; i++) {
		if (proto[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
			continue;

		if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
			continue;

		if (!GetProtocolVisibility(proto[i]->szName))
			continue;

		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hProtoStatusMenuItem[i], (LPARAM)&mi);
	}
	return 0;
}

void LoadIcons(void) {
	SKINICONDESC 	ico;
	int				i;

	ZeroMemory(&ico, sizeof(ico));
	ico.cbSize = sizeof(ico);
	ico.pszDefaultFile = "plugins\\simpleaway.dll";
	ico.pszSection = "SimpleAway";
	for (i=0; i<NUM_ICONS; i++) {
		ico.iDefaultIndex = -sa_ico_id[i];
		ico.pszDescription = Translate(sa_ico_descr[i]);
		ico.pszName = sa_ico_name[i];
		CallService(MS_SKIN2_ADDICON, (WPARAM)0, (LPARAM)&ico);
	}
	h_changedicons = HookEvent(ME_SKIN2_ICONSCHANGED, ChangedIcons);
}

int ChangeStatusMsgMenuItemInit(void) {
	CLISTMENUITEM mi;

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = 0;
	mi.position = 500050001;
	if (ServiceExists(MS_SKIN2_GETICON))
		mi.hIcon = (HICON)CallService(MS_SKIN2_GETICON, (WPARAM)0, (LPARAM)ICON_CSMSG);
	else
		mi.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_CSMSG), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	mi.pszName = Translate("Change Status Message");
	mi.pszService = MS_SA_TTCHANGESTATUSMSG;
	mi.pszPopupName = NULL;
	hChangeStatusMsgMenuItem = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);

	return 0;
}

int ChangeStatusMsgStatusMenuItemInit(void) {
	CLISTMENUITEM		mi;
	int					proto_count, i;
	PROTOCOLDESCRIPTOR	**proto;
	char				ProtoName[128];

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = 0;
	if (ServiceExists(MS_SKIN2_GETICON))
		mi.hIcon = (HICON)CallService(MS_SKIN2_GETICON, (WPARAM)0, (LPARAM)ICON_CSMSG);
	else
		mi.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_CSMSG), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	mi.pszService = MS_SA_TTCHANGESTATUSMSG;
	mi.pszName = Translate("Status Message...");
	mi.position = 2000200000;
	mi.pszPopupName = NULL;

	hGlobalStatusMenuItem = (HANDLE)CallService(MS_CLIST_ADDSTATUSMENUITEM, 0, (LPARAM)&mi);

	StatusMenuItemCount = 0;
	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
	for(i=0; i<proto_count; i++) {
		if (proto[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!GetProtocolVisibility(proto[i]->szName))
			continue;

		StatusMenuItemCount++;
	}

	if (StatusMenuItemCount == 1)
		return 0;

	mi.popupPosition= 500084000;
	mi.position = 2000040000;

	for(i=0; i<proto_count; i++) {
		if (proto[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
			continue;

		if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
			continue;

		if (!GetProtocolVisibility(proto[i]->szName))
			continue;

		CallProtoService(proto[i]->szName, PS_GETNAME, SIZEOF(ProtoName), (LPARAM)ProtoName);
		mi.pszPopupName = ProtoName;
		hProtoStatusMenuItem[i] = (HANDLE)CallService(MS_CLIST_ADDSTATUSMENUITEM, 0, (LPARAM)&mi);
	}

	return 0;
}

static int ChangeStatusMsgPrebuild(WPARAM wParam, LPARAM lParam) {
	ChangeStatusMsgStatusMenuItemInit();
	return 0;
}

int CSStatusChange(WPARAM wParam, LPARAM lParam) {
	PROTOCOLSETTINGEX** ps = *(PROTOCOLSETTINGEX***)wParam;
	int					i, status_mode, CSProtoCount;
	char				setting[80], *msg=NULL;

	if (!ps)
		return -1;

	CSProtoCount = CallService(MS_CS_GETPROTOCOUNT, 0, 0);
	for (i=0; i<CSProtoCount; i++) {
		if (ps[i]->status == ID_STATUS_IDLE)
			status_mode = ps[i]->lastStatus;
		else if (ps[i]->status == ID_STATUS_CURRENT)
			status_mode = CallProtoService(ps[i]->szName, PS_GETSTATUS, 0, 0);
		else
			status_mode = ps[i]->status;

		SaveStatusAsCurrent(ps[i]->szName, status_mode);
	#ifdef _DEBUG
		log2file("CSStatusChange(): Set %s status for %s.", StatusModeToDbSetting(status_mode, ""), ps[i]->szName);
	#endif

		if (ps[i]->szMsg) {
			int				max_hist_msgs, j;
			DBVARIANT		dbv;
			char			buff[80];
			BOOL			found=FALSE;

		#ifdef _DEBUG
			log2file("CSStatusChange(): Set \"%s\" status message for %s.", ps[i]->szMsg, ps[i]->szName);
		#endif
			max_hist_msgs = DBGetContactSettingByte(NULL, "SimpleAway", "MaxHist", 10);
			for (j=1; j<=max_hist_msgs; j++) {
				_snprintf(buff, sizeof(buff), "SMsg%d", j);
				if (!DBGetContactSetting(NULL, "SimpleAway", buff, &dbv)) {
					if (!strcmp(dbv.pszVal, ps[i]->szMsg)) {
						found = TRUE;
						_snprintf(setting, sizeof(setting), "Last%sMsg", ps[i]->szName);
						DBWriteContactSettingString(NULL, "SimpleAway", setting, buff);
						DBFreeVariant(&dbv);
						break;
					}
				}
			}

			if (!found) {
				_snprintf(buff, sizeof(buff), "FCur%sMsg", ps[i]->szName);
				_snprintf(setting, sizeof(setting), "Last%sMsg", ps[i]->szName);
				DBWriteContactSettingString(NULL, "SimpleAway", setting, buff);
			}

			_snprintf(setting, sizeof(setting), "%sMsg", ps[i]->szName);
			DBWriteContactSettingString(NULL, "SRAway", StatusModeToDbSetting(status_mode, setting), ps[i]->szMsg);

			msg = InsertVarsIntoMsg(ps[i]->szMsg, ps[i]->szName, status_mode);
			SaveMessageToDB(ps[i]->szName, ps[i]->szMsg, TRUE);
			SaveMessageToDB(ps[i]->szName, msg, FALSE);
			mir_free(msg);
		}
	}

	return 0;
}

int InitAwayModule(WPARAM wParam,LPARAM lParam) {
	int		i;

	// known modules list
	if (ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule", (WPARAM)"SimpleAway", 0);
	
	ProtoCount = 0;
	ProtoStatusCount = 0;
	ProtoStatusMsgFlags = 0;
	ProtoStatusMsgCount = 0;

	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&ProtoCount,(LPARAM)&protocols);
	for(i=0; i<ProtoCount; i++) {
		if (protocols[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0))
			ProtoStatusCount++;

		if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
			continue;

		ProtoStatusMsgFlags |= CallProtoService(protocols[i]->szName,PS_GETCAPS, PFLAGNUM_3,0);

		if (!CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
			continue;

		ProtoStatusMsgCount++;
	}

	if (ProtoStatusMsgFlags) {
		terminated = FALSE;
		LoadAwayMsgModule();
		if (ServiceExists(MS_SKIN2_ADDICON))
			LoadIcons();
		h_ttbloaded = HookEvent(ME_TTB_MODULELOADED, AddTopToolbarButton);
		h_optinitialise = HookEvent(ME_OPT_INITIALISE, InitOptions);
		h_statusmodechange = HookEvent(ME_CLIST_STATUSMODECHANGE, ChangeStatusMessage);
		h_protoack = HookEvent(ME_PROTO_ACK, ProcessProtoAck);
		ChangeStatusMsgMenuItemInit();
		hProtoStatusMenuItem = (static HANDLE *)mir_alloc(sizeof(HANDLE)*ProtoCount);
		if ((ServiceExists(MS_CLIST_ADDSTATUSMENUITEM)) && (DBGetContactSettingByte(NULL, "SimpleAway", "ShowStatusMenuItem", 1) == 1)) {
			h_prebuildstatusmenu = HookEvent(ME_CLIST_PREBUILDSTATUSMENU, ChangeStatusMsgPrebuild);
			ChangeStatusMsgStatusMenuItemInit();
		}

		winampsong = mir_strdup("SimpleAway");
		if (DBGetContactSettingByte(NULL, "SimpleAway", "AmpCheckOn", 1)) {
			SATimer = SetTimer(NULL,0,DBGetContactSettingByte(NULL, "SimpleAway", "AmpCheck", 15)*1000,(TIMERPROC)SATimerProc);
			is_timer = TRUE;
		}
		else
			is_timer = FALSE;

		if (DBGetContactSettingByte(NULL, "SimpleAway", "RandMsgChangeOn", 1)) {
			SARandMsgTimer = SetTimer(NULL,0,DBGetContactSettingByte(NULL, "SimpleAway", "RandMsgChange", 3)*60*1000,(TIMERPROC)SARandMsgTimerProc);
			is_randmsgtimer = TRUE;
		}
		else
			is_randmsgtimer = FALSE;

		if (DBGetContactSettingByte(NULL, "SimpleAway", "RemoveCR", 1))
			removeCR=TRUE;
		else
			removeCR=FALSE;
			
		if (DBGetContactSettingByte(NULL, "SimpleAway", "ShowCopy", 1))
			ShowCopy=TRUE;
		else
			ShowCopy=FALSE;
	}
	else if (ProtoStatusCount) {
		terminated = FALSE;
		if (!ServiceExists(MS_SS_GETPROFILECOUNT))
			h_optinitialise = HookEvent(ME_OPT_INITIALISE, InitStatusOptions);
	}
	else
		return 0;

	if (ServiceExists(MS_CS_SETSTATUSEX))
		h_csstatuschange = HookEvent(ME_CS_STATUSCHANGEEX, CSStatusChange);

	if (!ServiceExists(MS_SS_GETPROFILECOUNT)) {
		if (DBGetContactSettingByte(NULL, "SimpleAway", "GlobalStatusDelay", 1)) {
			SetTimer(NULL, 0, DBGetContactSettingWord(NULL, "SimpleAway", "SetStatusDelay", 300), (TIMERPROC)SetStartupStatusGlobal);
		}
		else {
			char	setting[80];
			SASetStatusTimer = (UINT *)malloc(sizeof(UINT)*ProtoCount);

			for(i=0; i<ProtoCount; i++) {
				if (!CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)protocols[i]->szName))
					continue;

				if (protocols[i]->type != PROTOTYPE_PROTOCOL)
					continue;

				if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0)))
					continue;

				_snprintf(setting, sizeof(setting), "Set%sStatusDelay", protocols[i]->szName);
				SASetStatusTimer[i] = SetTimer(NULL, 0, DBGetContactSettingWord(NULL, "SimpleAway", setting, 300), (TIMERPROC)SetStartupStatusProc);
			}
		}
	}

	return 0;
}

int OkToExitSA(WPARAM wParam,LPARAM lParam) {
	if (ProtoStatusCount) {
		int		i;
		char	setting[80];
		
		for (i=0; i<ProtoCount; i++) {
			if (protocols[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (!(CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0)))
				continue;
			
			_snprintf(setting, sizeof(setting), "Last%sStatus", protocols[i]->szName);
			DBWriteContactSettingWord(NULL, "SimpleAway", setting, (WORD)CallProtoService(protocols[i]->szName, PS_GETSTATUS, 0, 0));
		}
	}
	return 0;
}

int ShutdownSA(WPARAM wParam,LPARAM lParam) {
	if (ProtoStatusMsgFlags) {
		AwayMsgPreShutdown();
		if (hwndSAMsgDialog)
			DestroyWindow(hwndSAMsgDialog);
		if (hProtoStatusMenuItem)
			mir_free(hProtoStatusMenuItem);
		if (SASetStatusTimer)
			mir_free(SASetStatusTimer);
		if (winampsong)
			mir_free(winampsong);
		if (is_timer)
			KillTimer(NULL, SATimer);
		if (is_randmsgtimer)
			KillTimer(NULL, SARandMsgTimer);
	}
	return 0;
}

int	SimpleAwayTerminated(WPARAM wParam,LPARAM lParam) {
	return (terminated = TRUE);
}

static int IsSARunning(WPARAM wParam, LPARAM lParam) {
	return 1;
}

int __declspec(dllexport) Load(PLUGINLINK *link) {
	pluginLink=link;

	mir_getMMI(&mmi); // set the memory
	hwndSAMsgDialog	= NULL;
	init_mm();
	DBWriteContactSettingWord(NULL, "CList", "Status", (WORD)ID_STATUS_OFFLINE);
	h_modulesloaded = HookEvent(ME_SYSTEM_MODULESLOADED, InitAwayModule);
	
	CreateServiceFunction(MS_AWAYMSG_GETSTATUSMSG, GetAwayMessage);
	CreateServiceFunction(MS_SA_ISSARUNNING, IsSARunning);
	CreateServiceFunction(MS_SA_CHANGESTATUSMSG, ChangeStatusMessage);
	CreateServiceFunction(MS_SA_TTCHANGESTATUSMSG, TTChangeStatusMessage);
	CreateServiceFunction(MS_SA_SHOWSTATUSMSGDIALOG, ShowStatusMessageChangeDialog);
	CreateServiceFunction(MS_SA_SETSTATUSMODE, SetStatusModeFromExtern);

	CreateServiceFunction(MS_SA_SETOFFLINESTATUS, SetOfflineStatus);
	CreateServiceFunction(MS_SA_SETONLINESTATUS, SetOnlineStatus);
	CreateServiceFunction(MS_SA_SETAWAYSTATUS, SetAwayStatus);
	CreateServiceFunction(MS_SA_SETDNDSTATUS, SetDNDStatus);
	CreateServiceFunction(MS_SA_SETNASTATUS, SetNAStatus);
	CreateServiceFunction(MS_SA_SETOCCUPIEDSTATUS, SetOccupiedStatus);
	CreateServiceFunction(MS_SA_SETFREECHATSTATUS, SetFreeChatStatus);
	CreateServiceFunction(MS_SA_SETINVISIBLESTATUS, SetInvisibleStatus);
	CreateServiceFunction(MS_SA_SETONTHEPHONESTATUS, SetOnThePhoneStatus);
	CreateServiceFunction(MS_SA_SETOUTTOLUNCHSTATUS, SetOutToLunchStatus);

	h_oktoexit = HookEvent(ME_SYSTEM_OKTOEXIT, OkToExitSA);
	h_terminated = HookEvent(MS_SYSTEM_TERMINATED, SimpleAwayTerminated);
	h_shutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownSA);

	return 0;
}

int __declspec(dllexport) Unload(void) {
	if (ProtoStatusMsgFlags) {
		if (!ServiceExists(MS_SKIN2_ADDICON))
			UnhookEvent(h_changedicons);
		if (ServiceExists(MS_CLIST_ADDSTATUSMENUITEM))
			UnhookEvent(h_prebuildstatusmenu);
	}
	if (ServiceExists(MS_CS_SETSTATUSEX))
		UnhookEvent(h_csstatuschange);
	UnhookEvent(h_modulesloaded);
	UnhookEvent(h_oktoexit);
	UnhookEvent(h_terminated);
	UnhookEvent(h_shutdown);
	if (h_ttbloaded)
		UnhookEvent(h_ttbloaded);
	if (h_optinitialise)
		UnhookEvent(h_optinitialise);
	if (ProtoStatusMsgFlags) {
		UnhookEvent(h_statusmodechange);
		UnhookEvent(h_protoack);
	}
	return 0;
}
