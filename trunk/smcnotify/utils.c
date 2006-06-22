
#include "main.h"
#include <time.h>

// make display and history strings
TCHAR* GetStr(STATUSMSGINFO *n, const TCHAR *dis)
{
	TCHAR chr, *str, tmp[1024/*128*/];
	int i;
	int len;
//	SYSTEMTIME systime;
	time_t timestamp;
	struct tm smsgtime;
	str = (TCHAR*)malloc(2048 * sizeof(TCHAR));
	str[0] = _T('\0');
	len = lstrlen(dis);

	//use timestamp from statusmsginfo if present
//	ZeroMemory(&systime, sizeof(systime));
	ZeroMemory(&smsgtime, sizeof(smsgtime));
	if (n->dTimeStamp != 0)
	{
//		FILETIME ftime;
//		LONGLONG llong;
//		llong = Int32x32To64(n.dTimeStamp, 10000000) + 116444736000000000;
//		ftime.dwLowDateTime = (DWORD)llong;
//		ftime.dwHighDateTime = (DWORD)(llong >> 32);
//		FileTimeToSystemTime(&ftime, &systime);

		timestamp = n->dTimeStamp;
	}
	else
	{
//		GetLocalTime(&systime);
		timestamp = time(NULL);
	}
	localtime_s(&smsgtime, &timestamp);

	for (i = 0; i < len; i++)
	{

		tmp[0] = _T('\0');

		if (dis[i] == _T('%'))
		{
			i++;
			chr = dis[i];
			switch (chr)
			{
				case 'D':
//					mir_sntprintf(tmp, sizeof(tmp), "%02i", systime.wDay);
					mir_sntprintf(tmp, sizeof(tmp), "%02i", smsgtime.tm_mday);
					break;
				case 'H':
//					mir_sntprintf(tmp, sizeof(tmp), "%i", systime.wHour);
					mir_sntprintf(tmp, sizeof(tmp), "%i", smsgtime.tm_hour);
					break;
				case 'M':
//					mir_sntprintf(tmp, sizeof(tmp), "%02i", systime.wMonth);
					mir_sntprintf(tmp, sizeof(tmp), "%02i", smsgtime.tm_mon + 1);
					break;
				case 'Y':
//					mir_sntprintf(tmp, sizeof(tmp), "%i", systime.wYear);
					mir_sntprintf(tmp, sizeof(tmp), "%i", smsgtime.tm_year + 1900);
					break;
				case 'a':
//					if (systime.wHour > 11) strcat_s(tmp, sizeof(tmp), "PM");
					if (smsgtime.tm_hour > 11) strcat_s(tmp, sizeof(tmp), "PM");
//					if (systime.wHour < 12) strcat_s(tmp, sizeof(tmp), "AM");
					if (smsgtime.tm_hour < 12) strcat_s(tmp, sizeof(tmp), "AM");
					break;
				case 'c':
					lstrcpyn(tmp, n->cust, sizeof(tmp));
					break;
				case 'h':
					mir_sntprintf(tmp, sizeof(tmp), "%i", smsgtime.tm_hour%12 == 0 ? 12 : smsgtime.tm_hour%12);
					break;
				case 'm':
//					mir_sntprintf(tmp, sizeof(tmp), "%02i", systime.wMinute);
					mir_sntprintf(tmp, sizeof(tmp), "%02i", smsgtime.tm_min);
					break;
				case 'n':
					if (!strcmp(n->newstatusmsg, "")) strcat_s(tmp, sizeof(tmp), TranslateT("<empty status message>"));
					else lstrcpyn(tmp, n->newstatusmsg, sizeof(tmp));
					break;
				case 'o':
					if (!strcmp(n->oldstatusmsg, "")) strcat_s(tmp, sizeof(tmp), TranslateT("<empty status message>"));
					else lstrcpyn(tmp, n->oldstatusmsg, sizeof(tmp));
					break;
				case 's':
//					mir_sntprintf(tmp, sizeof(tmp), "%02i", systime.wSecond);
					mir_sntprintf(tmp, sizeof(tmp), "%02i", smsgtime.tm_sec);
					break;
				default:
					strcat_s(tmp, sizeof(tmp), "%");
					i--;
					break;
			}
		}
		else if (dis[i] == _T('\\'))
			{
				i++;
				chr = dis[i];
				switch (chr)
				{
					case 'n':
						strcat_s(tmp, sizeof(tmp), "\r\n");
						break;
					case 't':
						strcat_s(tmp, sizeof(tmp), "\t");
						break;
					default:
						strcat_s(tmp, sizeof(tmp), "\\");
						i--;
						break;
				}
			}
			else mir_sntprintf(tmp, sizeof(tmp), "%c", dis[i]);

		if (tmp[0] != _T('\0'))
		{
			if (lstrlen(tmp) + lstrlen(str) < 2044/*508*/)
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

//build history setting name
char* BuildSetting(short historyLast, BOOL bTS)
{
	static char setting[16];
	mir_sntprintf(setting, sizeof(setting), "%s%i%s", _T("History_"), historyLast, bTS?_T("_ts"):"");
	return setting;
}

int ProtoAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (ack->type == ACKTYPE_STATUS)
	{
		//We get here on a status change, or a status notification (meaning:
		//old status and new status are just like the same)
		WORD newStatus = (WORD)ack->lParam;
		WORD oldStatus = (WORD)ack->hProcess;
		char *szProtocol = (char*)ack->szModule;
		//Now we have the statuses and (a pointer to the string representing) the protocol.
		if (oldStatus == newStatus) return 0; //Useless message.
		if (newStatus == ID_STATUS_OFFLINE) 
		{
			//The protocol switched to offline. Disable the popups for this protocol
			DBWriteContactSettingDword(NULL, MODULE, szProtocol, 0);
		}
		else if ((oldStatus < ID_STATUS_ONLINE) && (newStatus >= ID_STATUS_ONLINE)) {
			//The protocol changed from a disconnected status to a connected status.
			//Enable the popups for this protocol.
			DBWriteContactSettingDword(NULL, MODULE, szProtocol, /*(options.bShowOnConnect && !options.bOnlyIfChanged)?1:*/GetTickCount());
		}

		return 0;
	}

//	if (ack->type == ACKTYPE_AWAYMSG && !lstrcmp(ack->szModule, "ICQ")) {
//		// Store in db?
//		if (ack->result == ACKRESULT_SUCCESS && !ServiceExists("SMR/MsgRetrievalEnabledForProtocol"))
//		{
//			// Store in db
//			DBWriteContactSettingString(ack->hContact, "CList", "StatusMsg", (const char*)ack->lParam);
//		}
//		return 0;
//	}

	return 0; //The protocol changed in a way we don't care.
}
