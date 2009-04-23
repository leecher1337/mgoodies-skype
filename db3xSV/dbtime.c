/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

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
#include "database.h"

static int daysInMonth[12]={31,28,31,30,31,30,31,31,30,31,30,31};
static int IsLeapYear(int year)
{
	if(year&3) return 0;
	if(year%100) return 1;
	if(year%400) return 0;
	return 1;
}

static int CompareSystemTimes(SYSTEMTIME *st,SYSTEMTIME *switchDate)
{
	FILETIME ft1,ft2;
	
	if(switchDate->wYear==0) {	   //strange day-in-month thing
		SYSTEMTIME tempst;

		//short-circuit if the months aren't the same
		if(st->wMonth<switchDate->wMonth) return -1;
		if(st->wMonth>switchDate->wMonth) return 1;
		
		tempst=*switchDate;
		tempst.wYear=st->wYear;
		tempst.wDay=1;
		SystemTimeToFileTime(&tempst,&ft1);
		FileTimeToSystemTime(&ft1,&tempst);	  //gets the day of week of the first of the month
		tempst.wDay=1+(7+switchDate->wDayOfWeek-tempst.wDayOfWeek)%7;
		if(switchDate->wDay==5) {	  //last wDayOfWeek in month
			if(tempst.wMonth==2) {
				if(IsLeapYear(tempst.wYear)) daysInMonth[1]=29;
				else daysInMonth[1]=28;
			}
			tempst.wDay+=7*3;		//can't be less than 4 of that day in the month
			if(tempst.wDay+7<=daysInMonth[switchDate->wMonth-1]) tempst.wDay+=7;
		}
		else tempst.wDay+=7*(switchDate->wDay-1);	//nth of month
		SystemTimeToFileTime(&tempst,&ft2);
	}
	else {
		switchDate->wYear=st->wYear;
		SystemTimeToFileTime(switchDate,&ft2);
	}
	SystemTimeToFileTime(st,&ft1);
	return CompareFileTime(&ft1,&ft2);
}

static INT_PTR TimestampToLocal(WPARAM wParam,LPARAM lParam)
{
	TIME_ZONE_INFORMATION tzInfo;
	LARGE_INTEGER liFiletime;
	FILETIME filetime;
	SYSTEMTIME st;

	GetTimeZoneInformation(&tzInfo);
	if(tzInfo.StandardDate.wMonth==0) {	 //no daylight savings time
		return (int)(wParam-tzInfo.Bias*60);
	}
	//this huge number is the difference between 1970 and 1601 in seconds
	liFiletime.QuadPart=(mir_i64(11644473600)+(__int64)wParam)*10000000;
	filetime.dwHighDateTime=liFiletime.HighPart;
	filetime.dwLowDateTime=liFiletime.LowPart;
	FileTimeToSystemTime(&filetime,&st);
	if(tzInfo.DaylightDate.wMonth<tzInfo.StandardDate.wMonth) {
		//northern hemisphere
		if(CompareSystemTimes(&st,&tzInfo.DaylightDate)<0 ||
		   CompareSystemTimes(&st,&tzInfo.StandardDate)>0) {
		    return (int)(wParam-(tzInfo.Bias+tzInfo.StandardBias)*60);
		}
	    return (int)(wParam-(tzInfo.Bias+tzInfo.DaylightBias)*60);
	}
	else {
		//southern hemisphere
		if(CompareSystemTimes(&st,&tzInfo.StandardDate)<0 ||
		   CompareSystemTimes(&st,&tzInfo.DaylightDate)>0) {
		    return (int)(wParam-(tzInfo.Bias+tzInfo.DaylightBias)*60);
		}
	    return (int)(wParam-(tzInfo.Bias+tzInfo.StandardBias)*60);
	}
	return 0;
}

static INT_PTR TimestampToString(WPARAM wParam,LPARAM lParam)
{
	DBTIMETOSTRING *tts=(DBTIMETOSTRING*)lParam;
	LARGE_INTEGER liFiletime;
	FILETIME filetime;
	SYSTEMTIME st;
	char dateTimeStr[64];
	char *pDest,*pFormat;
	int destCharsLeft,dateTimeStrLen;

	//this huge number is the difference between 1970 and 1601 in seconds
	liFiletime.QuadPart=(mir_i64(11644473600)+(__int64)(DWORD)TimestampToLocal(wParam,0))*10000000;
	filetime.dwHighDateTime=liFiletime.HighPart;
	filetime.dwLowDateTime=liFiletime.LowPart;
	FileTimeToSystemTime(&filetime,&st);
	destCharsLeft=tts->cbDest;
	for(pFormat=tts->szFormat,pDest=tts->szDest;*pFormat;pFormat++) {
		switch(*pFormat) {
			case 't':
				GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOSECONDS,&st,NULL,dateTimeStr,sizeof(dateTimeStr));
				break;
			case 's':
				GetTimeFormat(LOCALE_USER_DEFAULT,0,&st,NULL,dateTimeStr,sizeof(dateTimeStr));
				break;
			case 'm':
				GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOMINUTESORSECONDS,&st,NULL,dateTimeStr,sizeof(dateTimeStr));
				break;
			case 'd':
				GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,NULL,dateTimeStr,sizeof(dateTimeStr));
				break;
			case 'D':
				GetDateFormat(LOCALE_USER_DEFAULT,DATE_LONGDATE,&st,NULL,dateTimeStr,sizeof(dateTimeStr));
				break;
			default:
				if(destCharsLeft) {
					*pDest++=*pFormat;
					destCharsLeft--;
				}
				continue;
		}
		dateTimeStrLen=(int)strlen(dateTimeStr);
		if(destCharsLeft<dateTimeStrLen) dateTimeStrLen=destCharsLeft;
		CopyMemory(pDest,dateTimeStr,dateTimeStrLen);
		destCharsLeft-=dateTimeStrLen;
		pDest+=dateTimeStrLen;
	}
	if(destCharsLeft) *pDest=0;
	else tts->szDest[tts->cbDest-1]=0;
	return 0;
}

int InitTime(void)
{
	CreateServiceFunction(MS_DB_TIME_TIMESTAMPTOLOCAL,TimestampToLocal);
	CreateServiceFunction(MS_DB_TIME_TIMESTAMPTOSTRING,TimestampToString);
	return 0;
}
