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


typedef struct {
	int bSortAscending;
	int iProtoSort;
	int iLastColumnSortIndex;
} LVDLGDAT;

typedef struct {
	HANDLE hContact;
	int iiProto;
	int iStatus;
} LVITEMDAT;


static int GetStatusOnlineness(int status) {
	switch (status) {
		case ID_STATUS_FREECHAT:
			return 110;
		case ID_STATUS_ONLINE:
			return 100;
		case ID_STATUS_OCCUPIED:
			return 60;
		case ID_STATUS_ONTHEPHONE:
			return 50;
		case ID_STATUS_DND:
			return 40;
		case ID_STATUS_AWAY:
			return 30;
		case ID_STATUS_OUTTOLUNCH:
			return 20;
		case ID_STATUS_NA:
			return 10;
		case ID_STATUS_INVISIBLE:
			return 5;
	}
	return 0;
}

static BOOL CheckStatusMessage(HANDLE hContact, TCHAR* str) {
	if (DBGetContactSettingByte(hContact, "CList", "Hidden", 0)) return 0;
	
	MyDBGetContactSettingTString_dup(hContact, "CList", "StatusMsg", str);

	if (str != NULL && str[0] != _T('\0')) return 1;

	return 0;
}

static BOOL ListOpenContact(HWND hList, int item) {
	if (item != -1)
	{
		LVITEM lvi;
		LVITEMDAT *lvidat;
		ZeroMemory(&lvi, sizeof(lvi));
		lvi.mask = LVIF_PARAM;
		lvi.lParam = (LONG)NULL;
		lvi.iItem = item;
		lvi.iSubItem = 0;
		ListView_GetItem(hList, &lvi);
		lvidat = (LVITEMDAT*)lvi.lParam;
		if(lvidat->hContact != NULL)
		{
			CallService(MS_MSG_SENDMESSAGE, (WPARAM)lvidat->hContact, (LONG)NULL);
			CallService("SRMsg/LaunchMessageWindow", (WPARAM)lvidat->hContact, (LONG)NULL);
			return 0;
		}
	}
	return 1;
}

static BOOL ListOpenContactMenu(HWND hDlg, HWND hList, int item) {
	if (item != -1)
	{
		LVITEM lvi;
		LVITEMDAT *lvidat;
		ZeroMemory(&lvi, sizeof(lvi));
		lvi.mask = LVIF_PARAM;
		lvi.lParam = (LONG)NULL;
		lvi.iItem = item;
		lvi.iSubItem = 0;
		ListView_GetItem(hList, &lvi);
		lvidat = (LVITEMDAT*)lvi.lParam;
		if (lvidat->hContact != NULL)
		{
			HMENU hCMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)lvidat->hContact, 0);
			if (hCMenu != NULL)
			{
				POINT p;
				int ret;
				GetCursorPos(&p);
				ret = TrackPopupMenu(hCMenu, TPM_NONOTIFY | TPM_RETURNCMD, p.x, p.y, 0, hDlg, NULL);
				CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(ret, MPCF_CONTACTMENU), (LPARAM)lvidat->hContact);
				DestroyMenu(hCMenu);
				return 0;
			}
		}
	}
	return 1;
}

static int CALLBACK ListSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	LVDLGDAT *dat = (LVDLGDAT*)lParamSort;
	HWND hList = GetDlgItem(hListDlg, IDC_LIST);
	int result = 0;

	const int maxSize = 128;
	TCHAR text1[128];
	TCHAR text2[128];
	ListView_GetItemText(hList, (int)lParam1, dat->iLastColumnSortIndex, text1, maxSize);
	ListView_GetItemText(hList, (int)lParam2, dat->iLastColumnSortIndex, text2, maxSize);

	switch (dat->iLastColumnSortIndex)
	{
		case 0:
		{
			LVITEM lvi;
			LVITEMDAT *lvid1, *lvid2;
			ZeroMemory(&lvi, sizeof(lvi));
			lvi.mask = LVIF_PARAM;
			lvi.lParam = (LONG)NULL;
			lvi.iSubItem = 0;

			lvi.iItem = (int)lParam1;
			ListView_GetItem(hList, &lvi);
			lvid1 = (LVITEMDAT*)lvi.lParam;

			lvi.iItem = (int)lParam2;
			ListView_GetItem(hList, &lvi);
			lvid2 = (LVITEMDAT*)lvi.lParam;

			if ((lvid1 == NULL) || (lvid2 == NULL)) return 0;

			if (dat->iProtoSort == 1)
			{
				if (lvid1->iStatus == lvid2->iStatus) break;
				else
				{
					result = (lvid1->iStatus < lvid2->iStatus) ? 1 : -1;
				}
			}
			else if (dat->iProtoSort == 0)
			{
				result = lstrcmp(text1, text2);
				if (result == 0)
				{
					if (lvid1->iStatus == lvid2->iStatus) break;
					else
					{
						result = (lvid1->iStatus < lvid2->iStatus) ? 1 : -1;
					}
				}
			}
			break;
		}
		case 1:
		case 2:
			result = lstrcmpi(text1, text2);
			break;
	}

	return dat->bSortAscending ? result : -result;
}

static void AddColumns(HWND hList) {
	HIMAGELIST hImgList;
	LVCOLUMN lvc;

	ListView_SetExtendedListViewStyle(hList,
		LVS_EX_FULLROWSELECT
		| LVS_EX_INFOTIP
		| LVS_EX_LABELTIP
		| LVS_EX_BORDERSELECT
		| LVS_EX_GRIDLINES
		);

	hImgList = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	if (hImgList != NULL)
		ListView_SetImageList(hList, hImgList, LVSIL_SMALL);

	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_IMAGE;
	lvc.pszText = TranslateT("Protocol");
	lvc.cx = DBGetContactSettingWord(NULL, MODULE_NAME, "ListColWidth0", 30);
	ListView_InsertColumn(hList, 0, &lvc);
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
//	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = TranslateT("Nick");
	lvc.cx = DBGetContactSettingWord(NULL, MODULE_NAME, "ListColWidth1", 70);
	ListView_InsertColumn(hList, 1, &lvc);
	lvc.pszText = TranslateT("Status Message");
	lvc.cx = DBGetContactSettingWord(NULL, MODULE_NAME, "ListColWidth2", 250);
	ListView_InsertColumn(hList, 2, &lvc);

	ListView_SetTextColor(hList, opts.colListText);
	if (opts.bListUseBkImage && lstrcmp(opts.listbkimage, _T("")))
	{
		LVBKIMAGE lvbi;
		ZeroMemory(&lvbi, sizeof(lvbi));
		lvbi.ulFlags = LVBKIF_SOURCE_URL | LVBKIF_STYLE_TILE;
		lvbi.pszImage = opts.listbkimage;
		ListView_SetBkImage(hList, &lvbi);
		ListView_SetBkColor(hList, GetSysColor(COLOR_HIGHLIGHT));
		ListView_SetTextBkColor(hList, CLR_NONE);
	}
	else
	{
		ListView_SetBkColor(hList, opts.colListBack);
		ListView_SetTextBkColor(hList, opts.colListBack);
	}
}

static void LoadContacts(HWND hList) {
	HANDLE hContact;
	LVITEM lvi;
	LVITEMDAT *lvidat;
	int i = 0;
	TCHAR *proto = NULL;
	TCHAR *smsg = NULL;

	ZeroMemory(&lvi, sizeof(lvi));
	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
//		if (DBGetContactSettingByte(hContact, "CList", "Hidden", 0));
		smsg = MyDBGetContactSettingTString_dup(hContact, "CList", "StatusMsg", smsg);
		if (smsg != NULL && smsg[0] != _T('\0'))
		{
			lvidat = (LVITEMDAT*)mir_alloc(sizeof(LVITEMDAT));
			lvidat->iiProto = CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0);
			lvidat->iStatus = GetStatusOnlineness(DBGetContactSettingWord(hContact, (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0), "Status", 0));
			lvidat->hContact = hContact;
			lvi.mask = LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT;
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.lParam = (LPARAM)lvidat;
			lvi.iImage = CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0);
			proto = MyDBGetContactSettingTString_dup(hContact, "Protocol", "p", proto);
			lvi.pszText = proto;
			ListView_InsertItem(hList, &lvi);
			ListView_SetItemText(hList, i, 1, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR));
			ListView_SetItemText(hList, i, 2, smsg);
			i++;
			mir_free(smsg);
			mir_free(proto);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}

static BOOL CALLBACK ListDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	HWND hList = GetDlgItem(hwndDlg, IDC_LIST);
	LVDLGDAT *lvdat;
	lvdat = (LVDLGDAT*)GetWindowLong(hwndDlg, GWL_USERDATA);
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			DWORD sort;
			TranslateDialogDefault(hwndDlg);
			SendMessage(hwndDlg, WM_SETICON, (WPARAM)ICON_BIG, CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)ICO_LIST));
			SendMessage(hwndDlg, WM_SETICON, (WPARAM)ICON_SMALL, CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)ICO_LIST));
			Utils_RestoreWindowPosition(hwndDlg, NULL, MODULE_NAME, "List");

			lvdat = (LVDLGDAT*)mir_alloc(sizeof(LVDLGDAT));
			sort = DBGetContactSettingDword(NULL, MODULE_NAME, "ListColumnSort", 9);
			lvdat->iLastColumnSortIndex = sort >> 2;
			lvdat->bSortAscending = (sort & 1) ? 1 : 0;
			lvdat->iProtoSort = (sort & 2) ? 1 : 0;
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lvdat);

			AddColumns(hList);

			LoadContacts(hList);

			ListView_SortItemsEx(hList, ListSortFunc, (LPARAM)lvdat);
//			SendMessage(hwndDlg, WM_SIZE, 0, 0);
//			ShowWindow(hwndDlg, SW_SHOW);
			break;
		}
		case WM_NOTIFY:
		{
			if (((LPNMHDR)lParam)->idFrom == IDC_LIST)
			{
				switch (((LPNMHDR)lParam)->code)
				{
					case NM_DBLCLK:
					{
						LVHITTESTINFO lvh;
						POINT p;
						GetCursorPos(&p);
						ZeroMemory(&lvh, sizeof(lvh));
						lvh.pt = p;
						ScreenToClient(hList, &lvh.pt);
						ListView_HitTest(hList, &lvh);
						if ((lvh.flags & (LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON)) && lvh.iItem != -1)
							ListOpenContact(hList, lvh.iItem);
						break;
					}
					case NM_RCLICK:
					{
						LVHITTESTINFO lvh;
						POINT p;
						GetCursorPos(&p);
						ZeroMemory(&lvh, sizeof(lvh));
						lvh.pt = p;
						ScreenToClient(hList, &lvh.pt);
						ListView_HitTest(hList, &lvh);
						if ((lvh.flags & (LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON)) && lvh.iItem != -1)
							ListOpenContactMenu(hwndDlg, hList, lvh.iItem);
						break;
					}
					case LVN_COLUMNCLICK:
					{
						LPNMLISTVIEW nmlv = (LPNMLISTVIEW)lParam;
//						if (nmlv->iSubItem == 2) {
//							SendMessage(hwndDlg, WM_USER + 10, 0, 0);
//							break;
//						}
						if (nmlv->iSubItem != lvdat->iLastColumnSortIndex)
						{
							lvdat->bSortAscending = 1;
							lvdat->iLastColumnSortIndex = nmlv->iSubItem;
							lvdat->iProtoSort = 0;
						}
						else
						{
							lvdat->bSortAscending = !lvdat->bSortAscending;
							if (lvdat->bSortAscending == 1) lvdat->iProtoSort = !lvdat->iProtoSort;
						}
						ListView_SortItemsEx(hList, ListSortFunc, (LPARAM)lvdat);
						break;
					}
				}
			}
			break;
		}
		case WM_MEASUREITEM:
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		case WM_DRAWITEM:
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		case WM_COMMAND:
		{
			switch (wParam)
			{
				case IDOK:
					ListOpenContact(hList, ListView_GetNextItem(hList, -1, LVIS_SELECTED));
					break;
				case IDCANCEL:
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
			}
			break;
		}
//		case WM_USER + 10: // refresh
//			ListView_DeleteAllItems(hList);
//			LoadContactsStatusMsgList(hList);
//			ListView_SortItems(hList, ListSortFunc, (LPARAM)lvdat);
//			return 0;
		case WM_SIZE:
		{
			RECT rc;
			POINT p;
			GetWindowRect(hwndDlg, &rc);
			p.x = rc.right - GetSystemMetrics(SM_CYDLGFRAME);
			p.y = rc.bottom - GetSystemMetrics(SM_CXDLGFRAME);
			ScreenToClient(hwndDlg, &p);
			MoveWindow(hList, 0, 0, p.x, p.y, TRUE);
			break;
		}
		case WM_CLOSE:
		{
			DestroyWindow(hwndDlg);
//			KillTimer(hwndDlg, TIMERID_LISTAUTOREFRESH);
			break;
		}
		case WM_DESTROY:
		{
			DWORD sort;
			int i;
			Utils_SaveWindowPosition(hwndDlg, NULL, MODULE_NAME, "List");
			DBWriteContactSettingWord(NULL, MODULE_NAME, "ListColWidth0", (WORD)ListView_GetColumnWidth(hList, 0));
			DBWriteContactSettingWord(NULL, MODULE_NAME, "ListColWidth1", (WORD)ListView_GetColumnWidth(hList, 1));
			DBWriteContactSettingWord(NULL, MODULE_NAME, "ListColWidth2", (WORD)ListView_GetColumnWidth(hList, 2));

//			sort = lvdat->iLastColumnSortIndex<<2;
			sort = lvdat->iLastColumnSortIndex * 4;
			sort = lvdat->bSortAscending ? sort | 1 : sort;
			sort = lvdat->iProtoSort ? sort | 2 : sort;
			DBWriteContactSettingDword(NULL, MODULE_NAME, "ListColumnSort", sort);

			for (i = 0; i < ListView_GetItemCount(hList); i++)
			{
				LVITEM lvi;
				ZeroMemory(&lvi, sizeof(lvi));
				lvi.mask = LVIF_PARAM;
				lvi.lParam = (LONG)NULL;
				lvi.iItem = i;
				lvi.iSubItem = 0;
				ListView_GetItem(hList, &lvi);
				mir_free((LVITEMDAT*)lvi.lParam);
			}
			mir_free(lvdat);
			hListDlg = NULL;
			break;
		}
	}
	return FALSE;
}

extern void ShowList() {
	if (hListDlg == NULL)
	{
		hListDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_LIST), NULL, ListDlgProc);
		//ShowWindow(hListDlg, SW_SHOW);
	}
	else
	{
		SetForegroundWindow(hListDlg);
		SetFocus(hListDlg);
	}
}
