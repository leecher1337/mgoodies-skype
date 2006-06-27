/*
StatusMessageChangeNotify plugin for Miranda IM.

Copyright © 2004-2005 NoName
Copyright © 2005-2006 Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci

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

#include "main.h"
#include <time.h>

/*static BOOL CheckPopupTimer(WPARAM contact) {
	char *lpzProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, contact, 0);
	if (DBGetContactSettingDword(NULL, MODULE, lpzProto, 0) == 1) return TRUE;
	if ((GetTickCount() - DBGetContactSettingDword(NULL, MODULE, lpzProto, 0)) > TMR_CONNECTIONTIMEOUT)
	{
		DBWriteContactSettingDword(NULL, MODULE, lpzProto, 1);
		return TRUE;
	}
	return FALSE;
}*/
static BOOL CheckPopupTimer(HANDLE hContact, const char *module, const char *setting) {
	if (DBGetContactSettingDword(hContact, module, setting, 0) == 1) return TRUE;
	if ((GetTickCount() - DBGetContactSettingDword(hContact, module, setting, 0)) > TMR_CONNECTIONTIMEOUT)
	{
		DBWriteContactSettingDword(hContact, module, setting, 1);
		return TRUE;
	}
	return FALSE;
}

static BOOL CheckPopup(STATUSMSGINFO *smi) {
	if (options.bIgnoreEmptyPopup && smi->bIsEmpty)
		return FALSE;
//	if (options.bShowOnConnect)
//		return TRUE;
//	else if (CheckPopupTimer(NULL, MODULE, smi->proto)) return TRUE;
/*	if ((BOOL)DBGetContactSettingByte(NULL, MODULE, "IgnoreAfterStatusChange", 0))
//		return (GetTickCount() - DBGetContactSettingDword(smi->hContact, "UserOnline", "LastStatusChange", 0)) > TMR_CONNECTIONTIMEOUT;
		return CheckPopupTimer(smi->hContact, "UserOnline", "LastStatusChange");
*/
	if (!CheckPopupTimer(NULL, MODULE, smi->proto))
	{
		return (options.bShowOnConnect && (lstrcmp(smi->newstatusmsg, smi->oldstatusmsg) || !options.bOnlyIfChanged));
	}
	if ((BOOL)DBGetContactSettingByte(NULL, MODULE, "IgnoreAfterStatusChange", 0))
		return CheckPopupTimer(smi->hContact, "UserOnline", "LastStatusChange");

	return TRUE;
}

//return values:
//	0 - No window found
//	1 - Window found
static int CheckMsgWnd(WPARAM contact) {
	MessageWindowData mwd;
	MessageWindowInputData mwid;
	mwid.cbSize = sizeof(MessageWindowInputData);
	mwid.hContact = (HANDLE)contact;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
	mwd.cbSize = sizeof(MessageWindowData);
	mwd.hContact = (HANDLE)contact;
	if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd))
	{
		if (mwd.hwndWindow != NULL && (mwd.uState & MSG_WINDOW_STATE_EXISTS)) return 1;
	}
	return 0;
}


void StatusMsgChanged(WPARAM wParam, STATUSMSGINFO* smi) {
	DBVARIANT dbv;
	int i;
	DWORD ignore_mask;
	TCHAR buffer[2048];
	BOOL bExitAfterPopup = FALSE;

	//ignore empty status messages
	if (options.bIgnoreEmptyPopup && options.bIgnoreEmptyAll && smi->bIsEmpty) return;
	//read out old status message
	if (!DBGetContactSetting(smi->hContact, "UserOnline", "OldStatusMsg", &dbv))
	{
		smi->oldstatusmsg = _strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	else smi->oldstatusmsg = "";
	//If they are the same, you don't need to write to the DB or do anything else.
	if (!lstrcmp(smi->oldstatusmsg, smi->newstatusmsg))
	{
		if (options.bShowOnConnect && !options.bOnlyIfChanged)
			bExitAfterPopup = TRUE;
		else
			return;
	}

	DBWriteContactSettingString(smi->hContact, "UserOnline", "OldStatusMsg", smi->newstatusmsg);

	//fix CR/LF from SimpleAway
	lstrcpy(buffer, "");
	for (i = 0; i < lstrlen(smi->newstatusmsg); i++)
	{
		if ((smi->newstatusmsg[i] != 0x0D) && (smi->newstatusmsg[i+1] == 0x0A))
		{
			wsprintf(buffer, "%s%c\r", buffer, smi->newstatusmsg[i]);
		}
		else wsprintf(buffer, "%s%c", buffer, smi->newstatusmsg[i]);
	}
	lstrcpy(smi->newstatusmsg, buffer);

	smi->cust = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, wParam, 0);
	smi->dTimeStamp = (DWORD)time(NULL);
	ignore_mask = DBGetContactSettingDword(smi->hContact, IGNORE_MODULE, IGNORE_MASK, 0);

	//ignore not on list and hidden contacts
	if (!DBGetContactSettingByte(smi->hContact, "CList", "NotOnList", 0) && !DBGetContactSettingByte(smi->hContact, "CList", "Hidden", 0))
	{
		//popup
		if (!options.bDisablePopUps && !(ignore_mask & IGNORE_POP))
		{
			if (CheckPopup(smi))
			{
				//if this is the first time the plugin runs, don't show popup
				//if (!DBGetContactSetting(n.hContact,"CList","StatusMsg",&dbv))
				ShowPopup(smi);
				//play the sound event
				SkinPlaySound("statusmsgchanged");
			}
		}
		if (bExitAfterPopup) return;
		if (options.bIgnoreEmptyAll && smi->bIsEmpty) return;

		//log to file
		if (options.bLogToFile && lstrcmp(options.logfile, "") && !(ignore_mask & IGNORE_EXT))
		{
			HANDLE hFile;
			unsigned long dwBytesWritten = 0;
			hFile = CreateFile(options.logfile, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			SetFilePointer(hFile, 0, 0, FILE_END);
			lstrcpy(buffer, GetStr(smi, options.log));
			lstrcat(buffer, "\r\n");
			WriteFile(hFile, buffer, strlen(buffer), &dwBytesWritten, NULL);
			CloseHandle(hFile);
		}
	}

	//writing to message window
	if (options.bShowMsgChanges && CheckMsgWnd(wParam))
	{
		DBEVENTINFO dbei;
		int iLen;
		if (smi->bIsEmpty)
		{
			mir_snprintf(buffer, sizeof(buffer), GetStr(smi, options.msgcleared));
		}
		else
		{
			mir_snprintf(buffer, sizeof(buffer), GetStr(smi, options.msgchanged));
		}
		iLen = strlen(buffer) + 1;
		MultiByteToWideChar(CP_ACP, 0, buffer, iLen, (LPWSTR)&buffer[iLen], iLen);
		dbei.cbSize = sizeof(dbei);
		dbei.pBlob = (PBYTE)buffer;
		dbei.cbBlob = (strlen(buffer) + 1) * (sizeof(TCHAR) + 1);
		dbei.eventType = EVENTTYPE_STATUSCHANGE;
		dbei.flags = 0;
		dbei.timestamp = smi->dTimeStamp;
		dbei.szModule = smi->proto;//(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
		CallService(MS_DB_EVENT_ADD, wParam, (LPARAM)&dbei);
	}

	//writing to history
	if (!(ignore_mask & IGNORE_INT)/* && (BOOL)options.dHistMax*/)
	{
		short historyFirst, historyLast, historyMax;
		historyMax = (short)DBGetContactSettingDword(smi->hContact, MODULE, OPT_HISTMAX, options.dHistMax);
		if (historyMax < 0) historyMax = 0;
		else if (historyMax > 99) historyMax = 99;
		if (historyMax > 0)
		{
			historyFirst = DBGetContactSettingWord(smi->hContact, MODULE, "HistoryFirst", 0);
			if (historyFirst >=  historyMax) historyFirst = 0;
			historyLast = DBGetContactSettingWord(smi->hContact, MODULE, "HistoryLast", 0);
			if (historyLast >= historyMax) historyLast = historyMax - 1;
			//write old status message and its timestamp seperately
			DBWriteContactSettingString(smi->hContact, MODULE, BuildSetting(historyLast, FALSE), smi->newstatusmsg);
			DBWriteContactSettingDword(smi->hContact, MODULE, BuildSetting(historyLast, TRUE), smi->dTimeStamp);
			
			historyLast = (historyLast + 1) % historyMax;
			DBWriteContactSettingWord(smi->hContact, MODULE, "HistoryLast", historyLast);
			if (historyLast == historyFirst)
				DBWriteContactSettingWord(smi->hContact, MODULE, "HistoryFirst", (short)((historyFirst + 1) % historyMax));
		}
	}

	return;
}

