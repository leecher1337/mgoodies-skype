/*

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
BOOL		terminated=TRUE;
DWORD		ProtoStatusFlags;
UINT		SATimer;
char		*winampsong;
BOOL		is_timer;
BOOL		removeCR;
BOOL		ShowCopy;
HANDLE		TopButton, h_shutdown, h_terminated, h_modulesloaded, h_ttbloaded, h_optinitialise, h_statusmodechange;
PROTOCOLDESCRIPTOR **protocols;
int			ProtoCount;
HWND		hwndSAMsgDialog;
struct MM_INTERFACE memoryManagerInterface;

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"SimpleAway",
	PLUGIN_MAKE_VERSION(1,6,1,2),
	"This plugin replaces build-in away system.",
	"Harven",
	"harven@users.berlios.de",
	"© 2005 Harven",
	"http://developer.berlios.de/projects/mgoodies/",
	0,		//not transient
	DEFMOD_SRAWAY
};

PLUGININFOEX pluginInfoEx={
	sizeof(PLUGININFO),
	"SimpleAway",
	PLUGIN_MAKE_VERSION(1,6,1,2),
	"This plugin replaces build-in away system.",
	"Harven",
	"harven@users.berlios.de",
	"© 2005 Harven",
	"http://developer.berlios.de/projects/mgoodies/",
	0,		//not transient
	DEFMOD_SRAWAY,
	{0x8a63ff78, 0x6557, 0x4a42, { 0x8a, 0x82, 0x23, 0xa1, 0x6d, 0x46, 0x84, 0x4c }} //{84636F78-2057-4302-8A65-23A16D46844C}
};

static const MUUID interfaces[] = {MIID_SRAWAY, MIID_LAST};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

__declspec(dllexport)
	 PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

__declspec(dllexport)
     const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


//From SRAway module
char *StatusModeToDbSetting(int status,const char *suffix)
{
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

char *GetDefaultMessage(int status)
{
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

char* InsertVarsIntoMsg2(char *in, char *proto_name, int status)
{
	int i,j, count=0, len;
	char substituteStr[1024];
	char winamp_title[2048];
	char *p;
	char *msg;
	char buff[128];

	msg = mir_strdup(in);

	for(i=0;msg[i];i++)
	{
		if(msg[i] == 0x0D && removeCR)
		{
			p = msg+i;
			if (i+1 <= 1024 && msg[i+1])
			{
				if (msg[i+1] == 0x0A)
				{
					if (i+2 <= 1024 && msg[i+2])
					{
						count++;
						MoveMemory(p, p+1, lstrlen(p)-1);
					}
					else
					{
						msg[i+1] = 0;
						msg[i] = 0x0A;
					}
				}
			}
		}

		if(msg[i]!='%')
			continue;

		if(!_strnicmp(msg+i,"%winampsong%",12))
		{
			HWND hwndWinamp = FindWindow("Winamp v1.x",NULL);

			if (hwndWinamp)
			{
				if (SendMessage(hwndWinamp, WM_WA_IPC, 0, IPC_ISPLAYING) == 1)
				{
					GetWindowText(hwndWinamp, winamp_title, sizeof(winamp_title));

					p = winamp_title;
					j=0;

					while (*p != ' ')
					{
						j++;
						MoveMemory(p, p+1, lstrlen(p)-1);
					}
					MoveMemory(p, p+1, lstrlen(p)-1);

					p = winamp_title+strlen(winamp_title)-8;
					while (p >= winamp_title)
					{
						if (!strnicmp(p,"- Winamp",8))
							break;
						p--;
					}
					if (p >= winamp_title)
						p--;
					while (p >= winamp_title && *p == ' ')
						p--;
					*++p=0;

					if(lstrlen(winamp_title)>12)
						msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(winamp_title)-12);

					MoveMemory(msg+i+lstrlen(winamp_title), msg+i+12, lstrlen(msg)-i-11);
					CopyMemory(msg+i, winamp_title, lstrlen(winamp_title));
					continue;
				}
			}
		}
		else if(!_strnicmp(msg+i,"%fortunemsg%",12))
		{
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
		else if(!_strnicmp(msg+i,"%protofortunemsg%",17))
		{
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
		else if(!_strnicmp(msg+i,"%statusfortunemsg%",18))
		{
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
		else if(!_strnicmp(msg+i,"%time%",6))
		{
			GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOSECONDS,NULL,NULL,substituteStr,sizeof(substituteStr));

			if(lstrlen(substituteStr)>6)
				msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(substituteStr)-6);

			MoveMemory(msg+i+lstrlen(substituteStr),msg+i+6,lstrlen(msg)-i-5);
			CopyMemory(msg+i,substituteStr,lstrlen(substituteStr));
		}
		else if(!_strnicmp(msg+i,"%rand(",6))
		{
			char	*temp;
			int		ran_from;
			int		ran_to;
			char	*token;
			int		k;

			temp = mir_strdup(msg+i+6);

			token = strtok (temp, ",)");
			ran_from = atoi(token);
			token = strtok(NULL, ",)%%");
			ran_to = atoi(token);

			if (ran_to > ran_from)
			{
				_snprintf(substituteStr, sizeof(substituteStr), "%d", ranfr(ran_from, ran_to));

				for (k=i+1; msg[k]; k++)
				{
					if (msg[k] == '%')
					{
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
		else if(!_strnicmp(msg+i,"%date%",6))
		{
			GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,NULL,NULL,substituteStr,sizeof(substituteStr));

			if(lstrlen(substituteStr)>6)
				msg=(char*)mir_realloc(msg,lstrlen(msg)+1+lstrlen(substituteStr)-6);

			MoveMemory(msg+i+lstrlen(substituteStr),msg+i+6,lstrlen(msg)-i-5);
			CopyMemory(msg+i,substituteStr,lstrlen(substituteStr));
		}
		else
			continue;
	}
	if (count)
		msg[lstrlen(msg)-count] = 0;

	if (proto_name)
	{
		_snprintf(buff, sizeof(buff), "Proto%sMaxLen", proto_name);
		len = DBGetContactSettingWord(NULL, "SimpleAway", buff, 1024);
		if (len < lstrlen(msg))
		{
			msg = (char*)mir_realloc(msg, len);
			msg[len] = 0;
		}
	}

	return msg;
}

char* InsertVarsIntoMsg(char *msg, char *proto_name, int status)
{
	char	*format;

	format = (char *)InsertVarsIntoMsg2(msg, proto_name, status);

	if (ServiceExists(MS_VARS_FORMATSTRING))
	{
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

static char* GetAwayMessageFormat(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT		dbv, dbv2;
	int				statusMode = (int)wParam;
	int				flags;
	char			*format;

	flags = DBGetContactSettingByte(NULL, "SimpleAway", (char *)StatusModeToDbSetting(statusMode, "Flags"), STATUS_SHOW_DLG|STATUS_LAST_MSG);

	if (flags & STATUS_EMPTY_MSG)
		return mir_strdup("");

	if (flags & STATUS_LAST_STATUS_MSG)
	{
		if(DBGetContactSetting(NULL,"SRAway",StatusModeToDbSetting(statusMode,"Msg"),&dbv))
			return mir_strdup("");
		else
		{
			format = mir_strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	else if (flags & STATUS_LAST_MSG)
	{
		if (DBGetContactSetting(NULL, "SimpleAway", "LastMsg", &dbv2))
			return mir_strdup("");
		else
		{
			if (DBGetContactSetting(NULL, "SimpleAway", dbv2.pszVal, &dbv))
				return mir_strdup("");
			else
			{
				format = mir_strdup(dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			DBFreeVariant(&dbv2);
		}
	}
	else if (flags & STATUS_THIS_MSG)
	{
		if(DBGetContactSetting(NULL,"SRAway",StatusModeToDbSetting(statusMode,"Default"),&dbv))
			return mir_strdup("");
		else
		{
			format = mir_strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	else
		format = mir_strdup(GetDefaultMessage(statusMode));

	return format;
}

void DBWriteMessage(char *buff, char *message)
{
	if (message)
	{
		if (!lstrlen(message))
			DBDeleteContactSetting(NULL, "SimpleAway", buff);
		else
			DBWriteContactSettingString(NULL, "SimpleAway", buff, message);
	}
	else
		DBDeleteContactSetting(NULL, "SimpleAway", buff);
}

void SaveMessageToDB(char *proto, char *message, BOOL is_format)
{
	char	buff[128];

	if (!proto)
	{
		int		i;

		for (i=0; i<ProtoCount; i++)
		{
			if (protocols[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (!CallProtoService(protocols[i]->szName, PS_GETCAPS,PFLAGNUM_3, 0))
				continue;

			if (is_format)
				_snprintf(buff, sizeof(buff), "FCur%sMsg", protocols[i]->szName);
			else
				_snprintf(buff, sizeof(buff), "Cur%sMsg", protocols[i]->szName);
			DBWriteMessage(buff, message);
		}
	}
	else
	{
		if (is_format)
			_snprintf(buff, sizeof(buff), "FCur%sMsg", proto);
		else
			_snprintf(buff, sizeof(buff), "Cur%sMsg", proto);
		DBWriteMessage(buff, message);
	}
}

static int GetAwayMessage(WPARAM wParam, LPARAM lParam)
{
	char			*format;
	char			*ret;

	format = GetAwayMessageFormat(wParam, lParam);
	if (!format)
		return (int)NULL;

	ret = InsertVarsIntoMsg(format, NULL, (int)wParam);
	SaveMessageToDB(NULL, format, TRUE);
	SaveMessageToDB(NULL, ret, FALSE);
	mir_free(format);

	if (ret)
	{
		return (int)(ret);
	}
	else
	{
		return (int)mir_strdup("");
	}
}

int	CheckProtoSettings(char *proto_name, int initial_status_mode)
{
	int	check;

	check = DBGetContactSettingWord(NULL, proto_name, "LeaveStatus", -1); //GG settings
	if (check != -1)
	{
		if (check == 0)
			return initial_status_mode;
		else
			return check;
	}
	check = DBGetContactSettingWord(NULL, proto_name, "OfflineMessageOption", -1); //TELN settings
	if (check != -1)
	{
		if (check == 0)
			return initial_status_mode;
		else
		{
			switch(check)
			{
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

void DisableKeepStatus(char *proto)
{
	if (ServiceExists(MS_KS_ENABLEPROTOCOL))
		CallService(MS_KS_ENABLEPROTOCOL, 0, (LPARAM)proto);
}

void EnableKeepStatus(char *proto)
{
	if (ServiceExists(MS_KS_ISPROTOCOLENABLED))
	{
		BOOL	isEnabled;
		isEnabled = CallService(MS_KS_ISPROTOCOLENABLED, 0, (LPARAM)proto);

		if (!isEnabled)
		{
			if (ServiceExists(MS_KS_ENABLEPROTOCOL))
				CallService(MS_KS_ENABLEPROTOCOL, 1, (LPARAM)proto);
		}
	}
}

int	HasProtoStaticStatusMsg(char *proto, int initial_status, int status)
{
	char	setting[64];
	int		flags;

	_snprintf(setting, sizeof(setting), "Proto%sFlags", proto);

	flags = DBGetContactSettingByte(NULL, "SimpleAway", setting, 0);

	if (flags & PROTO_NO_MSG)
	{
		if ((initial_status != status) && (status == ID_STATUS_OFFLINE))
			DisableKeepStatus(proto);
		else if (initial_status != status)
			EnableKeepStatus(proto);
		CallProtoService(proto, PS_SETSTATUS, (WPARAM)status, 0);
		CallProtoService(proto, PS_SETAWAYMSG, (WPARAM)status, 0);
		SaveMessageToDB(proto, NULL, TRUE);
		SaveMessageToDB(proto, NULL, FALSE);
		return 1;
	}
	else if (flags & PROTO_THIS_MSG)
	{
		DBVARIANT			dbv;
		char				*msg;

		_snprintf(setting, sizeof(setting), "Proto%sDefault", proto);
		if(!DBGetContactSetting(NULL, "SimpleAway", setting, &dbv))
		{
			SaveMessageToDB(proto, dbv.pszVal, TRUE);
			msg = (char *)InsertVarsIntoMsg(dbv.pszVal, proto, status);
			DBFreeVariant(&dbv);

			if ((initial_status != status) && (status == ID_STATUS_OFFLINE))
			{
				int		status_from_proto_settings;

				DisableKeepStatus(proto);

				status_from_proto_settings = CheckProtoSettings(proto, initial_status);

				CallProtoService(proto, PS_SETSTATUS, (WPARAM)status_from_proto_settings, 0);
				CallProtoService(proto, PS_SETAWAYMSG, (WPARAM)status_from_proto_settings, (LPARAM)msg);
				CallProtoService(proto, PS_SETSTATUS, (WPARAM)status, 0);
			}
			else
			{
				EnableKeepStatus(proto);
				CallProtoService(proto, PS_SETSTATUS, (WPARAM)status, 0);
				CallProtoService(proto, PS_SETAWAYMSG, (WPARAM)status, (LPARAM)msg);
			}
			SaveMessageToDB(proto, msg, FALSE);
			mir_free(msg);
		}
		else
		{
			if ((initial_status != status) && (status == ID_STATUS_OFFLINE))
			{
				DisableKeepStatus(proto);
				CallProtoService(proto, PS_SETSTATUS, (WPARAM)status, 0);
			}
			else
			{
				EnableKeepStatus(proto);
				CallProtoService(proto, PS_SETSTATUS, (WPARAM)status, 0);
				CallProtoService(proto, PS_SETAWAYMSG, (WPARAM)status, 0);
			}
			SaveMessageToDB(proto, NULL, TRUE);
			SaveMessageToDB(proto, NULL, FALSE);
		}
		return 1;
	}
	return 0;
}

int ChangeStatusMessage(WPARAM wParam,LPARAM lParam);

int SetStatusModeFromExtern(WPARAM wParam, LPARAM lParam)
{
	int	i, status_modes_msg, pflags;

	if (wParam < ID_STATUS_OFFLINE || wParam > ID_STATUS_OUTTOLUNCH)
		return 0;

	for (i=0; i<ProtoCount; i++)
	{
		if (!strcmp(protocols[i]->szName, "mTV"))
			continue;

		if (protocols[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!CallProtoService(protocols[i]->szName, PS_GETCAPS,PFLAGNUM_3, 0))
			continue;

		pflags = CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0);

		if (!(pflags & PF1_MODEMSGSEND) && (pflags & PF1_INDIVMODEMSG))
		{
			CallProtoService(protocols[i]->szName, PS_SETSTATUS, wParam, 0);
			continue;
		}

		status_modes_msg = CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0);

		if ((Proto_Status2Flag(wParam) & status_modes_msg) || (wParam == ID_STATUS_OFFLINE && (Proto_Status2Flag(ID_STATUS_INVISIBLE) & status_modes_msg)))
		{
			if (HasProtoStaticStatusMsg(protocols[i]->szName, (int)wParam, (int)wParam))
				continue;

			if (lParam && wParam == ID_STATUS_OFFLINE) //ugly hack to set offline status message
			{
				int		status_from_proto_settings;

				DisableKeepStatus(protocols[i]->szName);

				status_from_proto_settings = CheckProtoSettings(protocols[i]->szName, wParam);

				CallProtoService(protocols[i]->szName, PS_SETSTATUS, (WPARAM)status_from_proto_settings, 0);
				CallProtoService(protocols[i]->szName, PS_SETAWAYMSG, (WPARAM)status_from_proto_settings, lParam);
				CallProtoService(protocols[i]->szName, PS_SETSTATUS, wParam, 0);
				continue;
			}

			CallProtoService(protocols[i]->szName, PS_SETSTATUS, wParam, 0);
			EnableKeepStatus(protocols[i]->szName);

			if (lParam)
				CallProtoService(protocols[i]->szName, PS_SETAWAYMSG, wParam, lParam);
			else
				CallProtoService(protocols[i]->szName, PS_SETAWAYMSG, wParam, 0);
		}
		else
		{
			CallProtoService(protocols[i]->szName, PS_SETSTATUS, wParam, 0);
			continue;
		}
	}
	return 0;
}

void SetStatusMessage(char *proto_name, int initial_status_mode, int status_mode, char *message)
{
	char	*msg=NULL;

	if (proto_name)
	{
		if (!strcmp(proto_name, "mTV"))
			return;
		
		if (message)
			msg = (char *)InsertVarsIntoMsg(message, proto_name, status_mode);

		SaveMessageToDB(proto_name, message, TRUE);
		SaveMessageToDB(proto_name, msg, FALSE);

		if (initial_status_mode != status_mode)
		{
			if (msg && status_mode == ID_STATUS_OFFLINE) //ugly hack to set offline status message
			{
				int		status_from_proto_settings;

				DisableKeepStatus(proto_name);

				status_from_proto_settings = CheckProtoSettings(proto_name, initial_status_mode);

				CallProtoService(proto_name,PS_SETSTATUS, (WPARAM)status_from_proto_settings, 0);
				CallProtoService(proto_name,PS_SETAWAYMSG, (WPARAM)status_from_proto_settings, (LPARAM)msg);
				CallProtoService(proto_name,PS_SETSTATUS, (WPARAM)status_mode, 0);
				mir_free(msg);
				return;
			}
		}

		CallProtoService(proto_name,PS_SETSTATUS, (WPARAM)status_mode, 0);
		EnableKeepStatus(proto_name);

		if (msg)
		{
			CallProtoService(proto_name,PS_SETAWAYMSG, (WPARAM)status_mode, (LPARAM)msg);
			mir_free(msg);
		}
		else
			CallProtoService(proto_name,PS_SETAWAYMSG, (WPARAM)status_mode, 0);
	}
	else
	{
		int					proto_count, i, pflags=0;
		PROTOCOLDESCRIPTOR	**proto;

		CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
		for(i=0; i<proto_count; i++)
		{
			if (proto[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (!strcmp(proto[i]->szName, "mTV"))
				continue;

			if (!CallProtoService(proto[i]->szName, PS_GETCAPS,PFLAGNUM_3, 0))
				continue;

			pflags = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0);

			if (!(pflags & PF1_MODEMSGSEND) && (pflags & PF1_INDIVMODEMSG))
				continue;

			if (HasProtoStaticStatusMsg(proto[i]->szName, initial_status_mode, status_mode))
				continue;

			if (message)
				msg = (char *)InsertVarsIntoMsg(message, proto[i]->szName, status_mode);

			SaveMessageToDB(proto[i]->szName, message, TRUE);
			SaveMessageToDB(proto[i]->szName, msg, FALSE);

			if (initial_status_mode != status_mode)
			{
				if (msg && status_mode == ID_STATUS_OFFLINE)//ugly hack to set offline status message
				{
					int		status_from_proto_settings;

					DisableKeepStatus(proto[i]->szName);

					status_from_proto_settings = CheckProtoSettings(proto[i]->szName, initial_status_mode);

					CallProtoService(proto[i]->szName, PS_SETSTATUS, (WPARAM)status_from_proto_settings, 0);
					CallProtoService(proto[i]->szName, PS_SETAWAYMSG, (WPARAM)status_from_proto_settings, (LPARAM)msg);
					CallProtoService(proto[i]->szName, PS_SETSTATUS, (WPARAM)status_mode, 0);
					mir_free(msg);
					continue;
				}
			}

			CallProtoService(proto[i]->szName, PS_SETSTATUS, (WPARAM)status_mode, 0);
			EnableKeepStatus(proto[i]->szName);

			if (msg)
			{
				CallProtoService(proto[i]->szName, PS_SETAWAYMSG, (WPARAM)status_mode, (LPARAM)msg);
				mir_free(msg);
			}
			else
				CallProtoService(proto[i]->szName, PS_SETAWAYMSG, (WPARAM)status_mode, 0);
		}
		if (CallService(MS_CLIST_GETSTATUSMODE, 0, 0) != status_mode)
		{
			UnhookEvent(h_statusmodechange); //my beautiful haxor part 1
			CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)status_mode, 0);
			h_statusmodechange = HookEvent(ME_CLIST_STATUSMODECHANGE, ChangeStatusMessage); //my beautiful haxor part 2
		}
	}
}

int TTChangeStatusMessage(WPARAM wParam,LPARAM lParam)
{
	struct MsgBoxInitData	*box_data;

	if (terminated)
		return 0;

	if (ServiceExists(MS_TTB_SETBUTTONSTATE) && TopButton)
	{
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)TopButton, (LPARAM)TTBST_RELEASED);
		CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM((WORD)TTBO_TIPNAME, (WORD)TopButton), (LPARAM)Translate("Change Status Message"));
	}

	box_data = (struct MsgBoxInitData *) mir_alloc (sizeof(struct MsgBoxInitData));
	box_data->proto_name = NULL;
	box_data->status_mode = (int)CallService(MS_CLIST_GETSTATUSMODE, (WPARAM)0, (LPARAM)0);
	box_data->all_modes = PF2_ONLINE|PF2_INVISIBLE|PF2_SHORTAWAY|PF2_LONGAWAY|PF2_LIGHTDND|PF2_HEAVYDND|PF2_FREECHAT|PF2_OUTTOLUNCH|PF2_ONTHEPHONE;
	box_data->all_modes_msg = box_data->all_modes;

	if (hwndSAMsgDialog)
		DestroyWindow(hwndSAMsgDialog);
	hwndSAMsgDialog = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_AWAYMSGBOX), NULL, AwayMsgBoxDlgProc, (LPARAM)box_data);
	return 0;
}

int ChangeStatusMessage(WPARAM wParam,LPARAM lParam)
{
	int		status_modes=0;
	int		status_modes_msg=0;
	int		pflags;
	int		dlg_flags;
	BOOL	show_dlg=FALSE;

	if (terminated)
		return 0;

	dlg_flags = DBGetContactSettingByte(NULL, "SimpleAway", (char *)StatusModeToDbSetting((int)wParam, "Flags"), STATUS_SHOW_DLG|STATUS_LAST_MSG);
	if (dlg_flags & STATUS_SHOW_DLG)
		show_dlg = TRUE;

	if (lParam)
	{
		if (!strcmp((char *)lParam, "mTV"))
			return 0;

		status_modes = CallProtoService((char *)lParam, PS_GETCAPS, PFLAGNUM_2, 0);

		if ((!status_modes || !(Proto_Status2Flag(wParam) & status_modes)) && (wParam != ID_STATUS_OFFLINE))
			return 0;

		pflags = CallProtoService((char *)lParam, PS_GETCAPS, PFLAGNUM_1, 0);
		if ((pflags & PF1_MODEMSGSEND) && !(pflags & PF1_INDIVMODEMSG))
		{
			status_modes_msg = CallProtoService((char *)lParam, PS_GETCAPS, PFLAGNUM_3, 0);

			if (!status_modes_msg || !(Proto_Status2Flag(wParam) & status_modes_msg))
				return 0;
			else
			{
				struct MsgBoxInitData	*box_data;
				char					setting[64];
				int						flags;

				_snprintf(setting, sizeof(setting), "Proto%sFlags", (char *)lParam);
				flags = DBGetContactSettingByte(NULL, "SimpleAway", setting, 0);

				if (!(flags & PROTO_POPUPDLG))
				{
					if (HasProtoStaticStatusMsg((char*)lParam, (int)wParam, (int)wParam))
						return 1;
				}

				if (!show_dlg)
				{
					char *msg = (char*)GetAwayMessageFormat(wParam, 0);
					SetStatusMessage((char *)lParam, (int)wParam, (int)wParam, msg);
					if (msg)
						mir_free(msg);
					return 1;
				}

				box_data = (struct MsgBoxInitData *) mir_alloc (sizeof(struct MsgBoxInitData));
				box_data->proto_name = (char *)lParam;
				box_data->status_mode = (int)wParam;
				box_data->all_modes = status_modes;
				box_data->all_modes_msg = status_modes_msg;

				if (hwndSAMsgDialog)
					DestroyWindow(hwndSAMsgDialog);
				hwndSAMsgDialog = CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_AWAYMSGBOX),NULL,AwayMsgBoxDlgProc,(LPARAM)box_data);
			}
		}
	}
	else
	{
		struct MsgBoxInitData	*box_data;

		if (wParam == ID_STATUS_OFFLINE)
			return 0;

		if (!(ProtoStatusFlags & Proto_Status2Flag(wParam)))
			return 0;

		if (!show_dlg)
		{
			int					proto_count, i, pflags=0;
			PROTOCOLDESCRIPTOR	**proto;

			CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
			for(i=0; i<proto_count; i++)
			{
				if (proto[i]->type != PROTOTYPE_PROTOCOL)
					continue;

				if (!strcmp(proto[i]->szName, "mTV"))
					continue;

				if (!CallProtoService(proto[i]->szName, PS_GETCAPS,PFLAGNUM_3, 0))
					continue;

				pflags = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0);

				if (!(pflags & PF1_MODEMSGSEND) && (pflags & PF1_INDIVMODEMSG))
					continue;
				else
				{
					char *msg = (char*)GetAwayMessageFormat(wParam, 0);
					SetStatusMessage(proto[i]->szName, (int)wParam, (int)wParam, msg);
					if (msg)
						mir_free(msg);
				}
			}
			return 1;
		}

		box_data = (struct MsgBoxInitData *) mir_alloc (sizeof(struct MsgBoxInitData));
		box_data->proto_name = NULL;
		box_data->status_mode = (int)wParam;
		box_data->all_modes = PF2_ONLINE|PF2_INVISIBLE|PF2_SHORTAWAY|PF2_LONGAWAY|PF2_LIGHTDND|PF2_HEAVYDND|PF2_FREECHAT|PF2_OUTTOLUNCH|PF2_ONTHEPHONE;
		box_data->all_modes_msg = box_data->all_modes;

		if (hwndSAMsgDialog)
			DestroyWindow(hwndSAMsgDialog);
		hwndSAMsgDialog = CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_AWAYMSGBOX),NULL,AwayMsgBoxDlgProc,(LPARAM)box_data);
	}
	return 0;
}

void CALLBACK SATimerProc(HWND timerhwnd, UINT uMsg, UINT_PTR idEvent, DWORD  dwTime)
{
	BOOL				winamp_playing = FALSE;
	HWND				hwndWinamp = FindWindow("Winamp v1.x",NULL);
	char				winamp_title[2048];

	if (hwndWinamp && !hwndSAMsgDialog)
	{
		if (SendMessage(hwndWinamp, WM_WA_IPC, 0, IPC_ISPLAYING) == 1)
				winamp_playing = TRUE;

		if (!winamp_playing)
			return;

		if (hwndWinamp)
			GetWindowText(hwndWinamp, winamp_title, sizeof(winamp_title));

		if (!winampsong)
			winampsong = mir_strdup("SimpleAway");

		if (winampsong)
		{
			if (strcmp(winamp_title, winampsong)) //newsong
			{
				PROTOCOLDESCRIPTOR	**proto;
				int					proto_count = 0, i;
				char				buff[64];
				DBVARIANT			dbv;
				char				*msg;
				int					status;

				CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);

				for(i=0; i<proto_count; i++)
				{
					if (proto[i]->type != PROTOTYPE_PROTOCOL)
						continue;

					if (!strcmp(proto[i]->szName, "mTV"))
						continue;

					if (!CallProtoService(proto[i]->szName, PS_GETCAPS,PFLAGNUM_3, 0))
						continue;


					status = CallProtoService(proto[i]->szName, PS_GETSTATUS, 0, 0);
					if (status == ID_STATUS_OFFLINE)
						continue;

					DBFreeVariant(&dbv);
					_snprintf(buff, sizeof(buff), "FCur%sMsg", proto[i]->szName);
					if(DBGetContactSetting(NULL,"SimpleAway", buff, &dbv))
						continue;

					if (!strstr(dbv.pszVal, "%winampsong%"))
					{
						DBFreeVariant(&dbv);
						continue;
					}

					msg = (char*)InsertVarsIntoMsg(dbv.pszVal, proto[i]->szName, status);
					DBFreeVariant(&dbv);

					CallProtoService(proto[i]->szName, PS_SETSTATUS, (WPARAM)status, 0);
					CallProtoService(proto[i]->szName, PS_SETAWAYMSG, (WPARAM)status, (LPARAM)msg);

					SaveMessageToDB(proto[i]->szName, msg, FALSE);
					mir_free(msg);
				}

				mir_free(winampsong);
				winampsong = mir_strdup(winamp_title);
			}
		}
	}
	return;
}

int AddTopToolbarButton(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_TTB_ADDBUTTON))
	{
		TTBButton sabutton;

		ZeroMemory(&sabutton, sizeof(sabutton));
      	sabutton.cbSize = sizeof(sabutton);
		sabutton.hbBitmapUp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MSGUP));
		sabutton.hbBitmapDown = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MSGDN));
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

char *sa_ico_name[NUM_ICONS] =
{
	ICON_DELETE,
	ICON_RECENT,
	ICON_PREDEF,
	ICON_ADD,
	ICON_CLEAR,
	ICON_COPY
};

char *sa_ico_descr[NUM_ICONS] =
{
	"Delete Selected",
	"Recent Message",
	"Predefined Message",
	"Add to Predefined",
	"Clear History",
	"Copy Away Message"
};

int sa_ico_id[NUM_ICONS] =
{
	IDI_CROSS,
	IDI_HISTORY,
	IDI_MESSAGE,
	IDI_PLUS,
	IDI_CHIST,
	IDI_COPY
};

void LoadIcons(void)
{
	SKINICONDESC 	ico;
	int				i;

	ZeroMemory(&ico, sizeof(ico));
	ico.cbSize = sizeof(ico);
	ico.pszDefaultFile = "plugins\\simpleaway.dll";
	ico.pszSection = "SimpleAway";
	for (i=0; i<NUM_ICONS; i++)
	{
		ico.iDefaultIndex = -sa_ico_id[i];
		ico.pszDescription = Translate(sa_ico_descr[i]);
		ico.pszName = sa_ico_name[i];
		CallService(MS_SKIN2_ADDICON, (WPARAM)0, (LPARAM)&ico);
	}
}

int InitAwayModule(WPARAM wParam,LPARAM lParam)
{
	int i, val;

	ProtoCount = 0;

	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&ProtoCount,(LPARAM)&protocols);
	for(i=0; i<ProtoCount; i++)
	{
		if (protocols[i]->type != PROTOTYPE_PROTOCOL)
			continue;
		ProtoStatusFlags |= CallProtoService(protocols[i]->szName,PS_GETCAPS,PFLAGNUM_3,0);
	}

	if (ProtoStatusFlags)
	{
		terminated = FALSE;
		LoadAwayMsgModule();
		if (ServiceExists(MS_SKIN2_ADDICON))
			LoadIcons();
		h_ttbloaded = HookEvent(ME_TTB_MODULELOADED, AddTopToolbarButton);
		h_optinitialise = HookEvent(ME_OPT_INITIALISE, InitOptions);
		h_statusmodechange = HookEvent(ME_CLIST_STATUSMODECHANGE, ChangeStatusMessage);

		val = DBGetContactSettingByte(NULL, "SimpleAway", "AmpCheck", 15);
		if (val)
		{
			SATimer = SetTimer(NULL,0,val*1000,(TIMERPROC)SATimerProc);
			is_timer = TRUE;
		}
		else
			is_timer = FALSE;

		if (DBGetContactSettingByte(NULL, "SimpleAway", "RemoveCR", 1))
			removeCR=TRUE;
		else
			removeCR=FALSE;

		if (DBGetContactSettingByte(NULL, "SimpleAway", "ShowCopy", 1))
			ShowCopy=TRUE;
		else
			ShowCopy=FALSE;
	}

	// known modules list
	if (ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule", (WPARAM)"SimpleAway", 0);

	return 0;
}

int ShutdownSA (WPARAM wParam,LPARAM lParam)
{
	if (winampsong)
		mir_free(winampsong);
	if (is_timer)
		KillTimer(NULL, SATimer);
	return 0;
}

int	SimpleAwayTerminated(WPARAM wParam,LPARAM lParam)
{
	return (terminated = TRUE);
}

static int IsSARunning(WPARAM wParam, LPARAM lParam)
{
	return 1;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink=link;
	mir_getMMI( &mmi );

	hwndSAMsgDialog	= NULL;
	init_mm();
	h_modulesloaded = HookEvent(ME_SYSTEM_MODULESLOADED, InitAwayModule);
	CreateServiceFunction(MS_AWAYMSG_GETSTATUSMSG, GetAwayMessage);
	CreateServiceFunction(MS_SA_ISSARUNNING, IsSARunning);
	CreateServiceFunction(MS_SA_CHANGESTATUSMSG, ChangeStatusMessage);
	CreateServiceFunction(MS_SA_TTCHANGESTATUSMSG, TTChangeStatusMessage);
	CreateServiceFunction(MS_SA_SETSTATUSMODE, SetStatusModeFromExtern);
	h_terminated = HookEvent(MS_SYSTEM_TERMINATED, SimpleAwayTerminated);
	h_shutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownSA);

	return 0;
}

int __declspec(dllexport) Unload(void)
{
	UnhookEvent(h_modulesloaded);
	UnhookEvent(h_terminated);
	UnhookEvent(h_shutdown);
	UnhookEvent(h_ttbloaded);
	UnhookEvent(h_optinitialise);
	UnhookEvent(h_statusmodechange);
	return 0;
}
