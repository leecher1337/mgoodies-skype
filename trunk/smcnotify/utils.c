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


#define TMPMAX	1024
TCHAR* GetStr(STATUSMSGINFO *n, const TCHAR *tmplt) {
	TCHAR tmp[TMPMAX];
	TCHAR *str;
	int i;
	int len;
	time_t timestamp;
	struct tm smsgtime;

	if (tmplt == NULL || tmplt[0] == _T('\0'))
		return NULL;

	str = (TCHAR*)mir_alloc0(2048 * sizeof(TCHAR));
	str[0] = _T('\0');
	len = lstrlen(tmplt);

	if (n->dTimeStamp != 0)
		timestamp = n->dTimeStamp;
	else
		timestamp = time(NULL);
	ZeroMemory(&smsgtime, sizeof(smsgtime));
	localtime_s(&smsgtime, &timestamp);

	for (i = 0; i < len; i++)
	{
		tmp[0] = _T('\0');

		if (tmplt[i] == _T('%'))
		{
			i++;
			switch (tmplt[i])
			{
				case 'n':
					if (n->compare == 2) lstrcpyn(tmp, TranslateT("<no status message>"), TMPMAX);
					else lstrcpyn(tmp, n->newstatusmsg, TMPMAX);
					break;
				case 'o':
					if (n->oldstatusmsg == NULL || n->oldstatusmsg[0] == _T('\0')) lstrcpyn(tmp, TranslateT("<no status message>"), TMPMAX);
					else lstrcpyn(tmp, n->oldstatusmsg, TMPMAX);
					break;
				case 'c':
					if (n->cust == NULL || n->cust[0] == _T('\0')) lstrcpyn(tmp, TranslateT("Contact"), TMPMAX);
					else lstrcpyn(tmp, n->cust, TMPMAX);
					break;
				case 'D':
					mir_sntprintf(tmp, 1024, _T("%02i"), smsgtime.tm_mday);
					break;
				case 'M':
					mir_sntprintf(tmp, TMPMAX, _T("%02i"), smsgtime.tm_mon + 1);
					break;
				case 'Y':
					mir_sntprintf(tmp, TMPMAX, _T("%i"), smsgtime.tm_year + 1900);
					break;
				case 'H':
					mir_sntprintf(tmp, TMPMAX, _T("%i"), smsgtime.tm_hour);
					break;
				case 'h':
					mir_sntprintf(tmp, TMPMAX, _T("%i"), smsgtime.tm_hour%12 == 0 ? 12 : smsgtime.tm_hour%12);
					break;
				case 'm':
					mir_sntprintf(tmp, TMPMAX, _T("%02i"), smsgtime.tm_min);
					break;
				case 's':
					mir_sntprintf(tmp, TMPMAX, _T("%02i"), smsgtime.tm_sec);
					break;
				case 'a':
					if (smsgtime.tm_hour > 11) lstrcpyn(tmp, _T("PM"), TMPMAX);
					if (smsgtime.tm_hour < 12) lstrcpyn(tmp, _T("AM"), TMPMAX);
					break;
				default:
					//lstrcpyn(tmp, _T("%"), TMPMAX);
					i--;
					tmp[0] = tmplt[i]; tmp[1] = _T('\0');
					break;
			}
		}
		else if (tmplt[i] == _T('\\'))
		{
			i++;
			switch (tmplt[i])
			{
				case 'n':
					//_tcscat_s(tmp, TMPMAX, _T("\r\n"));
					tmp[0] = _T('\r'); tmp[1] = _T('\n'); tmp[2] = _T('\0');
					break;
				case 't':
					//_tcscat_s(tmp, TMPMAX, _T("\t"));
					tmp[0] = _T('\t'); tmp[1] = _T('\0');
					break;
				default:
					//lstrcpyn(tmp, _T("\\"), TMPMAX);
					i--;
					tmp[0] = tmplt[i]; tmp[1] = _T('\0');
					break;
			}
		}
		else
		{
			tmp[0] = tmplt[i]; tmp[1] = _T('\0');
		}

		if (tmp[0] != _T('\0'))
		{
			if (lstrlen(tmp) + lstrlen(str) < 2044)
			{
				lstrcat(str, tmp);
			}
			else
			{
				lstrcat(str, _T("..."));
				break;
			}
		}
	}
	return str;
}

char* BuildSetting(WORD index, char *suffix) {
	static char setting[16];
	mir_snprintf(setting, sizeof(setting), "History_%i%s", index, (suffix == NULL)?"":suffix);
	return setting;
}

extern BOOL FreeSmiStr(STATUSMSGINFO *smi) {
	mir_free(smi->newstatusmsg);
	mir_free(smi->oldstatusmsg);
	return 0;
}

extern WCHAR *mir_dupToUnicodeEx(char *ptr, UINT CodePage)
{
	size_t size;
	WCHAR *tmp;

	if (ptr == NULL)
		return NULL;

	size = strlen(ptr) + 1;
	tmp = (WCHAR *) mir_alloc0(size * sizeof(WCHAR));

	MultiByteToWideChar(CodePage, 0, ptr, -1, tmp, size * sizeof(WCHAR));

	return tmp;
}

extern TCHAR* MyDBGetContactSettingTString_dup(HANDLE hContact, const char *szModule, const char *szSetting, TCHAR *out) {
	DBVARIANT dbv;

	if (!DBGetContactSettingTString(hContact, szModule, szSetting, &dbv))
	{
		switch (dbv.type)
		{
			case DBVT_ASCIIZ:
#ifdef UNICODE
				out = mir_dupToUnicodeEx(dbv.pszVal, CP_ACP);
				break;
			case DBVT_UTF8:
				out = mir_dupToUnicodeEx(dbv.pszVal, CP_UTF8);
				break;
			case DBVT_WCHAR:
				out = mir_wstrdup(dbv.pwszVal);
#else
				out = mir_strdup(dbv.pszVal);
#endif
				break;
			default:
				out = NULL;
				break;
		}
		DBFreeVariant(&dbv);
	}
	else
	{
		out = NULL;
	}

	return out;
}

extern ProtoAck(WPARAM wParam, LPARAM lParam) {
	ACKDATA *ack = (ACKDATA*)lParam;

	if (ack->type == ACKTYPE_STATUS)
	{
		WORD newStatus = (WORD)ack->lParam;
		WORD oldStatus = (WORD)ack->hProcess;
		char *proto = (char*)ack->szModule;

		if (oldStatus == newStatus) return 0;
		if (newStatus == ID_STATUS_OFFLINE)
		{
			DBWriteContactSettingDword(NULL, MODULE_NAME, proto, 0);
		}
		else if ((oldStatus < ID_STATUS_ONLINE) && (newStatus >= ID_STATUS_ONLINE))
		{
			DBWriteContactSettingDword(NULL, MODULE_NAME, proto, GetTickCount());
		}

		return 0;
	}

#ifdef CUSTOMBUILD_CATCHICQSTATUSMSG
	if (ack->type == ACKTYPE_AWAYMSG && !lstrcmpA(ack->szModule, "ICQ"))
	{
		if (ack->result == ACKRESULT_SUCCESS && !ServiceExists("SMR/MsgRetrievalEnabledForProtocol"))
		{
			DBWriteContactSettingString(ack->hContact, "CList", "StatusMsg", (const char*)ack->lParam);
		}
		return 0;
	}
#endif

	return 0;
}
