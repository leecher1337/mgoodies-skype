/*
Status Message Change Notify plugin for Miranda IM.

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

#include "commonheaders.h"


static void LogToFile(STATUSMSGINFO *smi) {
	HANDLE hFile;
	TCHAR filename[MAX_PATH] = _T("");
	TCHAR *p = NULL;

	p = _tcsstr(opts.logfile, _T("%c"));
	if (p != NULL)
	{
		p[1] = _T('s');
		mir_sntprintf(filename, MAX_PATH, opts.logfile, smi->cust);
		p[1] = _T('c');
	}
	else
		lstrcpyn(filename, opts.logfile, MAX_PATH);

	hFile = CreateFile(filename/*opts.logfile*/, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		TCHAR *buffer = GetStr(smi, opts.log);

		if (buffer != NULL && buffer[0] != _T('\0'))
		{
			DWORD dwWritten;

			SetFilePointer(hFile, 0, 0, FILE_END);
#ifdef UNICODE
			if (opts.bLogAscii)
			{
				char *bufferA = mir_dupToAscii(buffer);
				WriteFile(hFile, bufferA, lstrlenA(bufferA), &dwWritten, NULL);
				WriteFile(hFile, "\r\n", 2, &dwWritten, NULL);
				mir_free(bufferA);
			}
			else
#endif
			{
				WriteFile(hFile, buffer, lstrlen(buffer) * sizeof(TCHAR), &dwWritten, NULL);
				WriteFile(hFile, _T("\r\n"), 2 * sizeof(TCHAR), &dwWritten, NULL);
			}

			mir_free(buffer);
		}
		CloseHandle(hFile);
	}

	return;
}

static void AddToHistory(STATUSMSGINFO *smi) {
	WORD historyFirst, historyLast, historyMax;
//	TCHAR *p;

	historyMax = DBGetContactSettingWord(smi->hContact, MODULE_NAME, "HistoryMax", opts.dHistoryMax);
	if (historyMax <= 0)
		return;
	else if (historyMax > 99)
		historyMax = 99;

	historyFirst = DBGetContactSettingWord(smi->hContact, MODULE_NAME, "HistoryFirst", 0);
	if (historyFirst >=  historyMax)
		historyFirst = 0;
	historyLast = DBGetContactSettingWord(smi->hContact, MODULE_NAME, "HistoryLast", 0);
	if (historyLast >= historyMax)
		historyLast = historyMax - 1;

	//fix CR/LF from SimpleAway
//	p = &smi->newstatusmsg[0];
//	while (p = _tcsstr(p, _T("\n")))
//	{
//		if (p == &smi->newstatusmsg[0] || p[-1] != _T('\r'))
//			p[0] = _T('\r');
//	}

	//write old status message and its timestamp seperately
	DBWriteContactSettingTString(smi->hContact, MODULE_NAME, BuildSetting(historyLast, NULL), smi->newstatusmsg);
	DBWriteContactSettingDword(smi->hContact, MODULE_NAME, BuildSetting(historyLast, "_ts"), smi->dTimeStamp);

	historyLast = (historyLast + 1) % historyMax;
	DBWriteContactSettingWord(smi->hContact, MODULE_NAME, "HistoryLast", historyLast);
	if (historyLast == historyFirst)
		DBWriteContactSettingWord(smi->hContact, MODULE_NAME, "HistoryFirst", (WORD)((historyFirst + 1) % historyMax));

	return;
}

//return values:
//	0 - No window found
//	1 - Window found
static BOOL MsgWindowCheck(HANDLE hContact) {
	MessageWindowData mwd;
	MessageWindowInputData mwid;
	mwid.cbSize = sizeof(MessageWindowInputData);
	mwid.hContact = hContact;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
	mwd.cbSize = sizeof(MessageWindowData);
	mwd.hContact = hContact;
	if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd))
	{
		if (mwd.hwndWindow != NULL && (mwd.uState & MSG_WINDOW_STATE_EXISTS)) return 1;
	}
	return 0;
}

// Returns true if the unicode buffer only contains 7-bit characters.
static BOOL IsUnicodeAscii(const WCHAR * pBuffer, int nSize) {
	BOOL bResult = TRUE;
	int nIndex;

	for (nIndex = 0; nIndex < nSize; nIndex++) {
		if (pBuffer[nIndex] > 0x7F) {
			bResult = FALSE;
			break;
		}
	}
	return bResult;
}

static void AddToDB(STATUSMSGINFO *smi) {
	TCHAR *buffer = NULL;

	if (smi->compare == 2)
		buffer = GetStr(smi, opts.msgremoved);
	else if (smi->compare == 1)
		buffer = GetStr(smi, opts.msgchanged);

	if (buffer != NULL && buffer[0] != _T('\0'))
	{
		DBEVENTINFO dbei = {0};
#ifdef UNICODE
		size_t needed, len, size;
		BYTE *tmp = NULL;

		needed = WideCharToMultiByte(CP_ACP, 0, buffer, -1, NULL, 0, NULL, NULL);
		len = lstrlen(buffer);

		if (/*opts.history_only_ansi_if_possible && */IsUnicodeAscii(buffer, len))
			size = needed;
		else
			size = needed + (len + 1) * sizeof(WCHAR);

		tmp = (BYTE*)mir_alloc0(size);

		WideCharToMultiByte(CP_ACP, 0, buffer, -1, (char*)tmp, needed, NULL, NULL);

		if (size > needed)
			lstrcpyn((WCHAR*)&tmp[needed], buffer, len + 1);

		//ZeroMemory(&dbei, sizeof(dbei));
		dbei.cbSize = sizeof(dbei);
		dbei.pBlob = tmp;
		dbei.cbBlob = size;
#else
		dbei.pBlob = (PBYTE)buffer;
		dbei.cbBlob = lstrlen(buffer) + 1;
#endif

		dbei.eventType = EVENTTYPE_STATUSCHANGE;
		dbei.flags = 0;
		dbei.timestamp = smi->dTimeStamp;
		dbei.szModule = smi->proto;
		CallService(MS_DB_EVENT_ADD, (WPARAM)smi->hContact, (LPARAM)&dbei);
#ifdef UNICODE
		mir_free(tmp);
#endif
	}
	mir_free(buffer);

	return;
}

static int __inline CheckStr(char *str, int not_empty, int empty) {
	if (str == NULL || str[0] == '\0')
		return empty;
	else
		return not_empty;
}

#ifdef UNICODE

static int __inline CheckStrW(WCHAR *str, int not_empty, int empty) {
	if (str == NULL || str[0] == L'\0')
		return empty;
	else
		return not_empty;
}

#endif

static int CompareStatusMsg(STATUSMSGINFO *smi, DBCONTACTWRITESETTING *cws_new) {
	DBVARIANT dbv_old;
	int ret;

	switch (cws_new->value.type)
	{
		case DBVT_DELETED:
			smi->newstatusmsg = NULL;
			break;
		case DBVT_ASCIIZ:
#ifdef UNICODE
			smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(cws_new->value.pszVal, CP_ACP));
			break;
		case DBVT_UTF8:
			smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(cws_new->value.pszVal, CP_UTF8));
			break;
		case DBVT_WCHAR:
			smi->newstatusmsg = (CheckStrW(cws_new->value.pwszVal, 0, 1) ? NULL : mir_wstrdup(cws_new->value.pwszVal));
#else
			smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? NULL : mir_strdup(cws_new->value.pszVal));
#endif
			break;
		default:
			smi->newstatusmsg = NULL;
			break;
	}

	if (!
#ifdef UNICODE
	DBGetContactSettingW(smi->hContact, "UserOnline", "OldStatusMsg", &dbv_old)
#else
	DBGetContactSetting(smi->hContact, "UserOnline", "OldStatusMsg", &dbv_old)
#endif
	)
	{
		switch (dbv_old.type)
		{
			case DBVT_ASCIIZ:
#ifdef UNICODE
				smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(dbv_old.pszVal, CP_ACP));
				break;
			case DBVT_UTF8:
				smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(dbv_old.pszVal, CP_UTF8));
				break;
			case DBVT_WCHAR:
				smi->oldstatusmsg = (CheckStrW(dbv_old.pwszVal, 0, 1) ? NULL : mir_wstrdup(dbv_old.pwszVal));
#else
				smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? NULL : mir_strdup(dbv_old.pszVal));
#endif
				break;
			default:
				smi->oldstatusmsg = NULL;
				break;
		}

		if (cws_new->value.type == DBVT_DELETED)
			if (
#ifdef UNICODE
				dbv_old.type == DBVT_WCHAR)
				ret = CheckStrW(dbv_old.pwszVal, 2, 0);
			else if (dbv_old.type == DBVT_UTF8 ||
#endif
				dbv_old.type == DBVT_ASCIIZ)
				ret = CheckStr(dbv_old.pszVal, 2, 0);
			else
				ret = 2;
		else if (dbv_old.type != cws_new->value.type)
#ifdef UNICODE
			ret = (lstrcmpW(smi->newstatusmsg, smi->oldstatusmsg) ? CheckStrW(smi->newstatusmsg, 1, 2) : 0);
#else
			ret = 1;
#endif;
		else if (dbv_old.type == DBVT_ASCIIZ)
			ret = (lstrcmpA(cws_new->value.pszVal, dbv_old.pszVal) ? CheckStr(cws_new->value.pszVal, 1, 2) : 0);
#ifdef UNICODE
		else if (dbv_old.type == DBVT_UTF8)
			ret = (lstrcmpA(cws_new->value.pszVal, dbv_old.pszVal) ? CheckStr(cws_new->value.pszVal, 1, 2) : 0);
		else if (dbv_old.type == DBVT_WCHAR)
			ret = (lstrcmpW(cws_new->value.pwszVal, dbv_old.pwszVal) ? CheckStrW(cws_new->value.pwszVal, 1, 2) : 0);
#endif
		DBFreeVariant(&dbv_old);
	}
	else
	{
		if (cws_new->value.type == DBVT_DELETED)
			ret = 0;
		else if (
#ifdef UNICODE
			cws_new->value.type == DBVT_WCHAR)
			ret = CheckStrW(cws_new->value.pwszVal, 1, 0);
		else if (cws_new->value.type == DBVT_UTF8 ||
#endif
			cws_new->value.type == DBVT_ASCIIZ)
			ret = CheckStr(cws_new->value.pszVal, 1, 0);
		else
			ret = 1;

		smi->oldstatusmsg = NULL;
	}

	return ret;
}

static BOOL ProtocolEnabled(const char *proto) {
	char setting[256];

	if (proto == NULL)
		return FALSE;

	if (!AllowProtocol(proto))
		return FALSE;

	mir_snprintf(setting, sizeof(setting), "%sEnabled", proto);
	return (BOOL)DBGetContactSettingByte(NULL, MODULE_NAME, setting, TRUE);
}

extern int ContactSettingChanged(WPARAM wParam, LPARAM lParam) {
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	STATUSMSGINFO smi;

	if ((HANDLE)wParam == NULL) return 0;

	ZeroMemory(&smi, sizeof(smi));
	smi.proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (!ProtocolEnabled(smi.proto)) return 0;

	if (!lstrcmpA(cws->szModule, "CList") && !lstrcmpA(cws->szSetting, "StatusMsg"))
	{
		if (smi.proto != NULL && CallProtoService(smi.proto, PS_GETSTATUS, 0, 0) != ID_STATUS_OFFLINE)
		{
			smi.hContact = (HANDLE)wParam;
			smi.ignore = DBGetContactSettingDword(smi.hContact, "Ignore", MODULE_NAME, 0);
			if (smi.ignore == SMII_ALL) return 0;

			smi.compare = CompareStatusMsg(&smi, cws);
			if ((smi.compare == 0) || (opts.bIgnoreRemove && puopts.bIgnoreRemove && (smi.compare == 2)))
				return FreeSmiStr(&smi);

			if (DBGetContactSettingByte(NULL, MODULE_NAME, "IgnoreTlenAway", 0))
			{
				int len = lstrlen(smi.newstatusmsg);
				if (smi.newstatusmsg[len-13] == _T('[') && smi.newstatusmsg[len-10] == _T(':') && smi.newstatusmsg[len-7] == _T(' ') &&
					smi.newstatusmsg[len-4] == _T('.') && smi.newstatusmsg[len-1] == _T(']'))
					return FreeSmiStr(&smi);
			}

			if (cws->value.type == DBVT_DELETED)
			{
				DBDeleteContactSetting(smi.hContact, "UserOnline", "OldStatusMsg");
			}
			else
			{
				DBCONTACTWRITESETTING cws_old;
				cws_old.szModule = "UserOnline";
				cws_old.szSetting = "OldStatusMsg";
				cws_old.value = cws->value;
				CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)smi.hContact, (LPARAM)&cws_old);
			}
			smi.cust = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, wParam, GCDNF_TCHAR);
			smi.dTimeStamp = (DWORD)time(NULL);

			if (puopts.bEnable && !(smi.ignore & SMII_POPUP))
				PopupCheck(&smi);

			if (opts.bIgnoreRemove && (smi.ignore == 2))
				smi.ignore = SMII_ALL;

			if (opts.bDBEnable && MsgWindowCheck(smi.hContact))
				AddToDB(&smi);

			if (opts.bHistoryEnable && !(smi.ignore & SMII_HISTORY))
				AddToHistory(&smi);

			if (opts.bLogEnable && !(smi.ignore & SMII_LOG))
				LogToFile(&smi);

			FreeSmiStr(&smi);
		}
	}
	else
	{
		if (!lstrcmpA(cws->szSetting, "Status") && !lstrcmpA(cws->szModule, smi.proto))
		{
			DBWriteContactSettingDword((HANDLE)wParam, "UserOnline", "LastStatusChange", GetTickCount());
		}
	}

	return 0;
}