/*
Chat module plugin for Miranda IM

Copyright (C) 2003 J�rgen Persson

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



#include "chat.h"
#include <math.h>

static int RTFColorToIndex(int *pIndex, int iCol, SESSION_INFO * si)
{
	int i;
	MODULEINFO * pMod = MM_FindModule(si->pszModule);

	for (i = 0; i < pMod->nColorCount ; i++)
	{
		if ( pIndex[i] == iCol )
			return i;
	}

	return -1;
}

static void CreateColorMap(char * Text, int *pIndex, SESSION_INFO * si)
{
	char *p1, *p2, *pEnd;
	int iIndex = 1;

	static const char* lpszFmt = "\\red%[^ \x5b\\]\\green%[^ \x5b\\]\\blue%[^ \x5b;];";
	char szRed[10], szGreen[10], szBlue[10];

	p1 = strstr(Text, "\\colortbl" );
	if(!p1)
		return;

	pEnd = strchr(p1, '}');
	p2 = strstr(p1, "\\red");

	while (p2 && p2 < pEnd) {
		if ( sscanf( p2, lpszFmt, &szRed, &szGreen, &szBlue) > 0 ) {
			int i;
			MODULEINFO * pMod = MM_FindModule(si->pszModule);
			for (i = 0; i < pMod->nColorCount ; i ++)
				if (pMod->crColors[i] == RGB(atoi(szRed), atoi(szGreen), atoi(szBlue)))
					pIndex[i] = iIndex;
			}
		iIndex++;
		p1 = p2;
		p1 ++;
		p2 = strstr(p1, "\\red");
}	}

TCHAR* DoRtfToTags( char* pszText, SESSION_INFO* si)
{
	char *p, *p1;
	int * pIndex;
	int i, iRemoveChars, cp = CP_ACP;
	char InsertThis[50];
	BOOL bJustRemovedRTF = TRUE;
	BOOL bTextHasStarted = FALSE;
	TCHAR *ptszResult, *d;

	if(!pszText)
		return FALSE;

	// create an index of colors in the module and map them to
	// corresponding colors in the RTF color table
	pIndex = mir_alloc(sizeof(int) * MM_FindModule(si->pszModule)->nColorCount);
	for(i = 0; i < MM_FindModule(si->pszModule)->nColorCount ; i++)
		pIndex[i] = -1;

	CreateColorMap(pszText, pIndex, si);

	// scan the file for rtf commands and remove or parse them
	p1 = strstr(pszText, "\\pard");
	if ( p1 == NULL ) {
		mir_free(pIndex);
		return FALSE;
	}

		p1 += 5;

		MoveMemory(pszText, p1, lstrlenA(p1) +1);
		p1 = pszText;

	#if defined( _UNICODE )
		ptszResult = d = mir_alloc( strlen( p1 ) * sizeof( TCHAR ));
	#endif

		// iterate through all characters, if rtf control character found then take action
	while ( *p1 != '\0' ) {
		InsertThis[0] = 0;
			iRemoveChars = 0;

		switch (*p1) {
			case '\\':
			if ( !memcmp(p1, "\\cf", 3 )) // foreground color
				{
				TCHAR szTemp[20];
					int iCol = atoi(p1 + 3);
					int iInd = RTFColorToIndex(pIndex, iCol, si);
					bJustRemovedRTF = TRUE;

				_itot(iCol, szTemp, 10);
				iRemoveChars = 3 + lstrlen(szTemp);
					if(bTextHasStarted || iInd >= 0)
					mir_snprintf( InsertThis, SIZEOF(InsertThis), ( iInd >= 0 ) ? "%%c%02u" : "%%C", iInd);
				}
			else if ( !memcmp(p1, "\\highlight", 10 )) //background color
				{
				TCHAR szTemp[20];
					int iCol = atoi(p1 + 10);
					int iInd = RTFColorToIndex(pIndex, iCol, si);
					bJustRemovedRTF = TRUE;

				_itot(iCol, szTemp, 10);
				iRemoveChars = 10 + lstrlen(szTemp);
					if(bTextHasStarted || iInd >= 0)
					mir_snprintf( InsertThis, SIZEOF(InsertThis), ( iInd >= 0 ) ? "%%f%02u" : "%%F", iInd);
				}
			else if ( !memcmp(p1, "\\par", 4 )) // newline
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = TRUE;
					iRemoveChars = 4;
				strcpy(InsertThis, "\n" );
				}
			else if ( !memcmp(p1, "\\u", 2 ) && p1[2] != 'c' ) // unicode char
			{
				char temp[10];
				int i=0;
				p = p1 + 2;
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 2;
				while ( i < 10 && *p != ' ' && *p != '\\' ) {
					temp[i++] = *p++;
					iRemoveChars++;
				}
				temp[i] = 0;

				#if defined( _UNICODE )
					*d++ = atoi( temp );
					if ( *p == '\\' && p[1] == '\'' )
						iRemoveChars += 4;
				#endif
			}
			else if ( !memcmp(p1, "\\b", 2 )) //bold
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = TRUE;
					iRemoveChars = (p1[2] != '0')?2:3;
				mir_snprintf(InsertThis, SIZEOF(InsertThis), (p1[2] != '0') ? "%%b": "%%B" );
				}
			else if ( !memcmp(p1, "\\i", 2 )) // italics
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = TRUE;
					iRemoveChars = (p1[2] != '0')?2:3;
				mir_snprintf(InsertThis, SIZEOF(InsertThis), (p1[2] != '0') ? "%%i" : "%%I" );
				}
			else if ( !memcmp(p1, "\\ul", 3 )) // underlined
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = TRUE;
					if(p1[3] == 'n')
						iRemoveChars = 7;
					else if(p1[3] == '0')
						iRemoveChars = 4;
					else
						iRemoveChars = 3;
				mir_snprintf(InsertThis, SIZEOF(InsertThis), (p1[3] != '0' && p1[3] != 'n') ? "%%u" : "%%U" );
				}
			else if ( !memcmp(p1, "\\tab", 4 )) // tab
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = TRUE;
					iRemoveChars = 4;
				strcpy(InsertThis, " " );
				}
				else if(p1[1] == '\\' || p1[1] == '{' || p1[1] == '}' ) // escaped characters
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = FALSE;
					iRemoveChars = 2;
				mir_snprintf(InsertThis, SIZEOF(InsertThis), "%c", p1[1]);
				}
				else if(p1[1] == '\'' ) // special character
				{
				char tmp[4], *p3 = tmp;
					bTextHasStarted = TRUE;
					bJustRemovedRTF = FALSE;
				if (p1[2] != ' ' && p1[2] != '\\') {
					*p3++ = p1[2];
					iRemoveChars = 3;
					if ( p1[3] != ' ' && p1[3] != '\\') {
						*p3++ = p1[3];
						iRemoveChars++;
						}
					*p3 = 0;
					sscanf( tmp, "%x", InsertThis );

					#if defined( _UNICODE )
					{	TCHAR pwszLine[2];
						MultiByteToWideChar( cp, 0, InsertThis, 1, pwszLine, 2 );
						*d++ = pwszLine[0];
						InsertThis[0] = 0;
						}
					#else
						InsertThis[1] = 0;
					#endif
						}
				else iRemoveChars = 2;
				}
				else // remove unknown RTF command
				{
					int j = 1;
					bJustRemovedRTF = TRUE;
					while(p1[j] != ' ' && p1[j] != '\\' && p1[j] != '\0')
						j++;
					iRemoveChars = j;
				}
				break;

			case '{': // other RTF control characters
			case '}':
				iRemoveChars = 1;
				break;

			case '%': // escape chat -> protocol control character
				bTextHasStarted = TRUE;
				bJustRemovedRTF = FALSE;
				iRemoveChars = 1;
			mir_snprintf(InsertThis, SIZEOF(InsertThis), "%%%%" );
				break;
			case ' ': // remove spaces following a RTF command
				if(bJustRemovedRTF)
					iRemoveChars = 1;
				bJustRemovedRTF = FALSE;
				bTextHasStarted = TRUE;
				break;

			default: // other text that should not be touched
				bTextHasStarted = TRUE;
				bJustRemovedRTF = FALSE;
				break;
			}

			// move the memory and paste in new commands instead of the old RTF
		if ( InsertThis[0] || iRemoveChars ) {
			#if defined( _UNICODE )
				for ( p = InsertThis; *p; p++, d++ )
					*d = ( BYTE )*p;
			#endif
				MoveMemory(p1 + lstrlenA(InsertThis) , p1 + iRemoveChars, lstrlenA(p1) - iRemoveChars +1);
				CopyMemory(p1, InsertThis, lstrlenA(InsertThis));
				p1 += lstrlenA(InsertThis);
			}
		else {
			#if defined( _UNICODE )
				*d++ = ( BYTE )*p1++;
			#else
				p++;
			#endif
	}	}

	mir_free(pIndex);

	#if !defined( _UNICODE )
		return pszText;
	#else
		*d = 0;
		return ptszResult;
	#endif
	}

static DWORD CALLBACK Message_StreamCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	static DWORD dwRead;
    char ** ppText = (char **) dwCookie;

	if (*ppText == NULL)
	{
		*ppText = mir_alloc(cb + 1);
		memcpy(*ppText, pbBuff, cb);
		(*ppText)[cb] = 0;
		*pcb = cb;
		dwRead = cb;
	}
	else
	{
		char  *p = mir_alloc(dwRead + cb + 1);
		memcpy(p, *ppText, dwRead);
		memcpy(p+dwRead, pbBuff, cb);
		p[dwRead + cb] = 0;
		mir_free(*ppText);
		*ppText = p;
		*pcb = cb;
		dwRead += cb;
	}

    return 0;
}

char * Message_GetFromStream(HWND hwndDlg, SESSION_INFO * si)
{
	EDITSTREAM stream;
	char * pszText = NULL;
	DWORD dwFlags;

	if(hwndDlg == 0 || si == 0)
		return NULL;

	ZeroMemory(&stream, sizeof(stream));
	stream.pfnCallback = Message_StreamCallback;
	stream.dwCookie = (DWORD) &pszText; // pass pointer to pointer

	dwFlags = SF_RTFNOOBJS | SF_NCRFORNONASCII | SFF_PLAINRTF;
	#if defined( _UNICODE )
		dwFlags |= SF_UNICODE;
	#endif

	SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_STREAMOUT, dwFlags, (LPARAM) & stream);
	return pszText; // pszText contains the text
}
