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
	HWND hwndParent = (HWND) GetWindowLong(hwndDlg, GWL_USERDATA);
	switch (msg) {
		case WM_INITDIALOG:
		{
			RECT rc, rcParent;
			struct ErrorWindowData *ewd = (struct ErrorWindowData *) lParam;

			TranslateDialogDefault(hwndDlg);

			if (ewd != NULL) {
				hwndParent = ewd->hwndParent;
				SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) hwndParent);
				if (!ewd->szDescription||!strlen(ewd->szDescription))
					ewd->szDescription = strdup(Translate("An unknown error has occured."));
				SetDlgItemTextA(hwndDlg, IDC_ERRORTEXT, ewd->szDescription);
				free(ewd->szDescription);
			}

			GetWindowRect(hwndDlg, &rc);
			GetWindowRect(hwndParent, &rcParent);
			SetWindowPos(hwndDlg, HWND_TOP, (rcParent.right - rcParent.left - rc.right + rc.left) / 2, (rcParent.bottom - rcParent.top - rc.bottom + rc.top), 0, 0, SWP_NOSIZE);
		}
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					SendMessage(hwndParent, DM_ERRORDECIDED, MSGERROR_RETRY, 0);
					DestroyWindow(hwndDlg);
					break;
				case IDCANCEL:
					SendMessage(hwndParent, DM_ERRORDECIDED, MSGERROR_CANCEL, 0);
					DestroyWindow(hwndDlg);
					break;
			}
			break;
	}
	return FALSE;

}
