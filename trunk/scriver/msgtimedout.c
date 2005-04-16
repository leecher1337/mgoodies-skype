/*
SRMM

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
#pragma hdrstop
#include "msgs.h"


BOOL CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct ErrorWindowData *ewd = (struct ErrorWindowData *) GetWindowLong(hwndDlg, GWL_USERDATA);
	//if (ewd==NULL && msg!=WM_INITDIALOG) return FALSE;
	switch (msg) {
		case WM_INITDIALOG:
		{
			RECT rc, rcParent;
			ewd = (struct ErrorWindowData *) lParam;
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) ewd);
			TranslateDialogDefault(hwndDlg);
			if (ewd != NULL) {
				if (!ewd->szDescription) 
					ewd->szDescription = strdup(Translate("An unknown error has occured."));
				if (!ewd->szText) 
					ewd->szText = strdup("");
				if (!ewd->szName) 
					ewd->szName = strdup("");
				SetDlgItemTextA(hwndDlg, IDC_ERRORTEXT, ewd->szDescription);
		#if defined( _UNICODE )
				SetDlgItemTextW(hwndDlg, IDC_MSGTEXT, (TCHAR *)(ewd->szText + strlen(ewd->szText) + 1));
		#else
				SetDlgItemTextA(hwndDlg, IDC_MSGTEXT, ewd->szText);
		#endif
			}

			GetWindowRect(hwndDlg, &rc);
			GetWindowRect(ewd->hwndParent, &rcParent);
			SetWindowPos(hwndDlg, HWND_TOP, rcParent.left + (rcParent.right - rcParent.left - rc.right + rc.left) / 2, rcParent.top + (rcParent.bottom - rcParent.top - rc.bottom + rc.top), 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					SendMessage(ewd->hwndParent, DM_ERRORDECIDED, MSGERROR_RETRY, (LPARAM) ewd);
					DestroyWindow(hwndDlg);
					break;
				case IDCANCEL:
					SendMessage(ewd->hwndParent, DM_ERRORDECIDED, MSGERROR_CANCEL, (LPARAM) ewd);
					DestroyWindow(hwndDlg);
					break;
			}
			break;
		case WM_DESTROY:
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) NULL);
			free(ewd->szName);
			free(ewd->szDescription);
			free(ewd->szText);
			free(ewd);
			break;

	}
	return FALSE;

}
