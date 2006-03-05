
#include "main.h"
#include <time.h>


static BOOL CheckPopupTimer(WPARAM contact) {
	char *lpzProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, contact, 0);
	if (DBGetContactSettingDword(NULL, MODULE, lpzProto, 0) == 1) return TRUE;
	if ((GetTickCount() - DBGetContactSettingDword(NULL, MODULE, lpzProto, 0)) > TMR_CONNECTIONTIMEOUT)
	{
		DBWriteContactSettingDword(NULL, MODULE, lpzProto, 1);
		return TRUE;
	}
	return FALSE;
}

static BOOL CheckPopup(STATUSMSGINFO *lpn) {
	if (options.bIgnoreEmptyPopup && !lstrcmp(lpn->newstatusmsg, "")) return FALSE;
	if (options.bShowOnConnect) return TRUE;
	else if (CheckPopupTimer((WPARAM)lpn->hContact)) return TRUE;
	return FALSE;
}

// return values:
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
	if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd)) {
		if (mwd.hwndWindow != NULL && (mwd.uState & MSG_WINDOW_STATE_EXISTS)) return 1;
	}
	return 0;
}


void StatusMsgChanged(WPARAM wParam, DBCONTACTWRITESETTING* cws) {
	DBVARIANT dbv;
	STATUSMSGINFO n;
	int i;
	TCHAR buffer[2048];
	//Ok, if we're here we have a contact who is changing his status message.
	//The first thing we need to do is save the new Status in place of the old
	//one, then we'll proceed.
	//If we don't do this, we may exit from the function without updating the
	//UserOnline settings.

	n.hContact = (HANDLE)wParam;

	// check for unicode
	if (cws->value.type == DBVT_UTF8) {
		DBGetContactSetting(n.hContact, cws->szModule, cws->szSetting, &dbv);
		n.newstatusmsg = malloc(strlen(dbv.pszVal) + 2);
		lstrcpy(n.newstatusmsg, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	else {
		if (cws->value.type == DBVT_DELETED) n.newstatusmsg = "";
		else n.newstatusmsg = strdup(cws->value.pszVal);
	}

	// ignore empty status messages
	if (options.bIgnoreEmptyAll && !lstrcmp(n.newstatusmsg, "")) return;

	if (!DBGetContactSetting(n.hContact, "UserOnline", "OldStatusMsg", &dbv)) {
		n.oldstatusmsg = malloc(strlen(dbv.pszVal) + 2);
		lstrcpy(n.oldstatusmsg, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	else n.oldstatusmsg = "";

	//If they are the same, you don't need to write to the DB or do anything else.
	if (!lstrcmp(n.oldstatusmsg, n.newstatusmsg)) return;

	n.cust = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, wParam, 0);
	DBWriteContactSettingString(n.hContact, "UserOnline", "OldStatusMsg", n.newstatusmsg);

	// fix CR/LF from SimpleAway
	lstrcpy(buffer, "");
	for (i = 0; i < lstrlen(n.newstatusmsg); i++) {
		if ((n.newstatusmsg[i] != 0x0D) && (n.newstatusmsg[i+1] == 0x0A)) {
			wsprintf(buffer, "%s%c\r", buffer, n.newstatusmsg[i]);
		}
		else wsprintf(buffer, "%s%c", buffer, n.newstatusmsg[i]);
	}
	lstrcpy(n.newstatusmsg, buffer);

	// ignore not on list and hidden contacts
	if (!DBGetContactSettingByte(n.hContact, "CList", "NotOnList", 0) && !DBGetContactSettingByte(n.hContact, "CList", "Hidden", 0)) {
		// popup
		if (!options.bDisablePopUps && DBGetContactSettingByte(n.hContact, MODULE, "Popup", TRUE)) {
			if (CheckPopup(&n)) {
				// if this is the first time the plugin runs, don't show popup
				//if (!DBGetContactSetting(n.hContact,"CList","StatusMsg",&dbv))
				ShowPopup(n);
				// play the sound event
				SkinPlaySound("statusmsgchanged");
			}
		}

		// log to file
		if (options.bLogToFile && lstrcmp(options.logfile, "") && DBGetContactSettingByte(n.hContact, MODULE, "External", TRUE)) {
			HANDLE hFile;
			unsigned long dwBytesWritten = 0;
			hFile = CreateFile(options.logfile, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			SetFilePointer(hFile, 0, 0, FILE_END);
			lstrcpy(buffer, GetStr(n, options.log));
			lstrcat(buffer, "\r\n");
			WriteFile(hFile, buffer, strlen(buffer), &dwBytesWritten, NULL);
			CloseHandle(hFile);
		}
	}

	// writing to message window
	if (options.bShowMsgChanges && CheckMsgWnd(wParam)) {
		DBEVENTINFO dbei;
		int iLen;
		if (!lstrcmp(n.newstatusmsg, "")) {
			mir_snprintf(buffer, sizeof(buffer), GetStr(n, options.msgcleared));
		}
		else {
			mir_snprintf(buffer, sizeof(buffer), GetStr(n, options.msgchanged));
		}
		iLen = strlen(buffer) + 1;
		MultiByteToWideChar(CP_ACP, 0, buffer, iLen, (LPWSTR)&buffer[iLen], iLen);
		dbei.cbSize = sizeof(dbei);
		dbei.pBlob = (PBYTE)buffer;
		dbei.cbBlob = (strlen(buffer) + 1) * (sizeof(TCHAR) + 1);
		dbei.eventType = EVENTTYPE_STATUSCHANGE;
		dbei.flags = 0;
		dbei.timestamp = time(NULL);
		dbei.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
		CallService(MS_DB_EVENT_ADD, wParam, (LPARAM)&dbei);
	}

	// writing to history
	if (DBGetContactSettingByte(n.hContact, MODULE, "Internal", TRUE) && (BOOL)options.dHistMax) {
		short historyFirst, historyLast, historyMax;
		historyMax = (int)options.dHistMax;
		if (historyMax < 0) historyMax = 0; else if (historyMax > 99) historyMax = 99;
		//if (historyMax == 0) return;
		historyFirst = DBGetContactSettingWord(n.hContact, MODULE, "HistoryFirst", 0);
		if (historyFirst >=  historyMax) historyFirst = 0;
		historyLast = DBGetContactSettingWord(n.hContact, MODULE, "HistoryLast", 0);
		if (historyLast >= historyMax) historyLast = historyMax - 1;
		DBWriteContactSettingString(n.hContact, MODULE, BuildSetting(historyLast), GetStr(n, options.his));
		historyLast = (historyLast + 1) % historyMax;
		DBWriteContactSettingWord(n.hContact, MODULE, "HistoryLast", historyLast);
		if (historyLast == historyFirst)
			DBWriteContactSettingWord(n.hContact, MODULE, "HistoryFirst", (short)((historyFirst + 1) % historyMax));
	}

	// free memory
	if (n.oldstatusmsg) free(n.oldstatusmsg);
	if (n.newstatusmsg) free(n.newstatusmsg);
	return;
}

