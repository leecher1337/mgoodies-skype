/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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
	char * pszText;
	char * p1;
	char * p2;
	char * pEnd;
	int iIndex = 1;

	static const char* lpszFmt = "\\red%[^ \x5b\\]\\green%[^ \x5b\\]\\blue%[^ \x5b;];";
	char szRed[10], szGreen[10], szBlue[10];

	pszText = strdup(Text);

	p1 = strstr(pszText, "\\colortbl");
	if(!p1)
		return;

	pEnd = strchr(p1, '}');

	p2 = strstr(p1, "\\red");

	while(p2 && p2 < pEnd)
	{
		if( sscanf(p2, lpszFmt, &szRed, &szGreen, &szBlue) > 0 )
		{
			int i;
			MODULEINFO * pMod = MM_FindModule(si->pszModule);
			for (i = 0; i < pMod->nColorCount ; i ++)
			{
				if (pMod->crColors[i] == RGB(atoi(szRed), atoi(szGreen), atoi(szBlue)))
					pIndex[i] = iIndex;
			}
		}
		iIndex++;
		p1 = p2;
		p1 ++;

		p2 = strstr(p1, "\\red");
	}

	free(pszText);

	return ;

}

BOOL DoRtfToTags(char * pszText, SESSION_INFO * si)
{
	char * p1;
	int * pIndex;
	int i;
	BOOL bJustRemovedRTF = TRUE;
	BOOL bTextHasStarted = FALSE;

	if(!pszText)
		return FALSE;

	// create an index of colors in the module and map them to
	// corresponding colors in the RTF color table
	pIndex = malloc(sizeof(int) * MM_FindModule(si->pszModule)->nColorCount);
	for(i = 0; i < MM_FindModule(si->pszModule)->nColorCount ; i++)
		pIndex[i] = -1;
	CreateColorMap(pszText, pIndex, si);

	// scan the file for rtf commands and remove or parse them
	p1 = strstr(pszText, "\\pard");
	if(p1)
	{
		int iRemoveChars;
		char InsertThis[50];
		p1 += 5;

		MoveMemory(pszText, p1, lstrlenA(p1) +1);
		p1 = pszText;

		// iterate through all characters, if rtf control character found then take action
		while(*p1 != '\0')
		{
			mir_snprintf(InsertThis, sizeof(InsertThis), "");
			iRemoveChars = 0;

			switch (*p1)
			{
			case '\\':
				if(p1 == strstr(p1, "\\cf")) // foreground color
				{
					char szTemp[20];
					int iCol = atoi(p1 + 3);
					int iInd = RTFColorToIndex(pIndex, iCol, si);
					bJustRemovedRTF = TRUE;

					itoa(iCol, szTemp, 10);
					iRemoveChars = 3 + lstrlenA(szTemp);
					if(bTextHasStarted || iInd >= 0)
						mir_snprintf(InsertThis, sizeof(InsertThis), ( iInd >= 0 )?"%%c%02u":"%%C", iInd);
				}
				else if(p1 == strstr(p1, "\\highlight")) //background color
				{
					char szTemp[20];
					int iCol = atoi(p1 + 10);
					int iInd = RTFColorToIndex(pIndex, iCol, si);
					bJustRemovedRTF = TRUE;

					itoa(iCol, szTemp, 10);
					iRemoveChars = 10 + lstrlenA(szTemp);
					if(bTextHasStarted || iInd >= 0)
						mir_snprintf(InsertThis, sizeof(InsertThis), ( iInd >= 0 )?"%%f%02u":"%%F", iInd);
				}
				else if(p1 == strstr(p1, "\\par")) // newline
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = TRUE;
					iRemoveChars = 4;
					mir_snprintf(InsertThis, sizeof(InsertThis), "\n");
				}
				else if(p1 == strstr(p1, "\\b")) //bold
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = TRUE;
					iRemoveChars = (p1[2] != '0')?2:3;
					mir_snprintf(InsertThis, sizeof(InsertThis), (p1[2] != '0')?"%%b":"%%B");

				}
				else if(p1 == strstr(p1, "\\i")) // italics
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = TRUE;
					iRemoveChars = (p1[2] != '0')?2:3;
					mir_snprintf(InsertThis, sizeof(InsertThis), (p1[2] != '0')?"%%i":"%%I");

				}
				else if(p1 == strstr(p1, "\\ul")) // underlined
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = TRUE;
					if(p1[3] == 'n')
						iRemoveChars = 7;
					else if(p1[3] == '0')
						iRemoveChars = 4;
					else
						iRemoveChars = 3;
					mir_snprintf(InsertThis, sizeof(InsertThis), (p1[3] != '0' && p1[3] != 'n')?"%%u":"%%U");

				}
				else if(p1 == strstr(p1, "\\tab")) // tab
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = TRUE;
					iRemoveChars = 4;
					mir_snprintf(InsertThis, sizeof(InsertThis), " ");

				}
				else if(p1[1] == '\\' || p1[1] == '{' || p1[1] == '}' ) // escaped characters
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = FALSE;
					iRemoveChars = 2;
					mir_snprintf(InsertThis, sizeof(InsertThis), "%c", p1[1]);

				}
				else if(p1[1] == '\'' ) // special character
				{
					bTextHasStarted = TRUE;
					bJustRemovedRTF = FALSE;
					if(p1[2] != ' ' && p1[2] != '\\')
					{
						int iLame = 0;
						char * p3;
						if(p1[3] != ' ' && p1[3] != '\\')
						{


							lstrcpynA(InsertThis, p1 + 2, 3);
							iRemoveChars = 4;
						}
						else
						{
							lstrcpynA(InsertThis, p1 + 2, 2);
							iRemoveChars = 3;

						}
						// convert string containing char in hex format to int.
						// i'm sure there is a better way than this lame stuff that came out of me
						p3 = InsertThis;
						while (*p3)
						{
							if(*p3 == 'a')
								iLame += 10 * (int)pow(16, lstrlenA(p3) -1);
							else if(*p3 == 'b')
								iLame += 11 * (int)pow(16, lstrlenA(p3) -1);
							else if(*p3 == 'c')
								iLame += 12 * (int)pow(16, lstrlenA(p3) -1);
							else if(*p3 == 'd')
								iLame += 13 * (int)pow(16, lstrlenA(p3) -1);
							else if(*p3 == 'e')
								iLame += 14 * (int)pow(16, lstrlenA(p3) -1);
							else if(*p3 == 'f')
								iLame += 15 * (int)pow(16, lstrlenA(p3) -1);
							else
								iLame += (*p3 - 48) * (int)pow(16, lstrlenA(p3) -1);
							p3++;
						}
						mir_snprintf(InsertThis, sizeof(InsertThis), "%c", iLame);
					}
					else
						iRemoveChars = 2;
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
				mir_snprintf(InsertThis, sizeof(InsertThis), "%%%%");
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
			if(lstrlenA(InsertThis) || iRemoveChars)
			{
				MoveMemory(p1 + lstrlenA(InsertThis) , p1 + iRemoveChars, lstrlenA(p1) - iRemoveChars +1);
				CopyMemory(p1, InsertThis, lstrlenA(InsertThis));
				p1 += lstrlenA(InsertThis);
			}
			else
				p1++;
		}


	}
	else // error
	{
		free(pIndex);
		return FALSE;
	}

	free(pIndex);

	return TRUE;
}
static DWORD CALLBACK Message_StreamCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	static DWORD dwRead;
    char ** ppText = (char **) dwCookie;

	if (*ppText == NULL)
	{
		*ppText = malloc(cb + 1);
		memcpy(*ppText, pbBuff, cb);
		(*ppText)[cb] = 0;
		*pcb = cb;
		dwRead = cb;
	}
	else
	{
		char  *p = malloc(dwRead + cb + 1);
		memcpy(p, *ppText, dwRead);
		memcpy(p+dwRead, pbBuff, cb);
		p[dwRead + cb] = 0;
		free(*ppText);
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


	if(hwndDlg == 0 || si == 0)
		return NULL;

	ZeroMemory(&stream, sizeof(stream));
	stream.pfnCallback = Message_StreamCallback;
	stream.dwCookie = (DWORD) &pszText; // pass pointer to pointer

	SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_STREAMOUT, SF_RTF|SF_RTFNOOBJS|SFF_PLAINRTF, (LPARAM) & stream);

	return pszText; // pszText contains the text

}
