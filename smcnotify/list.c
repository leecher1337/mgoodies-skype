
#include <mbstring.h>
#include "main.h"

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
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL ListOpenContactMenu(HWND hDlg, HWND hList, int item, LVDLGDAT *lvdat) {
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
			HMENU hCMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)lvidat->hContact, (LONG)NULL);
			if (hCMenu != NULL)
			{
				POINT p;
				BOOL ret;
				GetCursorPos(&p);
				lvdat->hContact = lvidat->hContact;
				ret = TrackPopupMenu(hCMenu, 0, p.x, p.y, 0, hDlg, NULL);
				DestroyMenu(hCMenu);
				if (ret) return TRUE;
				lvdat->hContact = NULL;
			}
		}
	}
	return FALSE;
}

static int CALLBACK ListSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	LVDLGDAT *dat = (LVDLGDAT*)lParamSort;
	LVITEMDAT *lid1 = (LVITEMDAT*)lParam1;
	LVITEMDAT *lid2 = (LVITEMDAT*)lParam2;
	int result;
	if ((lid1 == NULL) || (lid2 == NULL)) return 0;
	switch (dat->iLastColumnSortIndex) {
		case 0: {
			switch (dat->iProtoSort) {
				case 0:
					result = lstrcmp(lid1->proto, lid2->proto);
					if (result == 0) {
						if (lid1->iStatus == lid2->iStatus) break;
						else
						{
							if (lid1->iStatus < lid2->iStatus) return dat->bSortAscending?1:-1;
							else return dat->bSortAscending?-1:1;
						}
					}
					else
					{
						return result;
					}
				case 1:
					if (lid1->iStatus == lid2->iStatus) break;
					else
					{
						if (lid1->iStatus < lid2->iStatus) return dat->bSortAscending?1:-1;
						else return dat->bSortAscending?-1:1;
					}
			}
		}
		case 1:
			//return strcmp(lid1->nick, lid2->nick)*dat->bSortAscending;
			result = _mbsicoll(lid1->nick, lid2->nick);
			return dat->bSortAscending?result:-result;
	}
	return 0;
}

BOOL CheckStatusMessage(HANDLE hContact, char str[2048]) {
	DBVARIANT dbv;
//	char *lpzProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (DBGetContactSettingByte(hContact, "CList", "Hidden", 0)) return 0;
	if (DBGetContactSetting(hContact, "CList", "StatusMsg", &dbv) == 0)
	{
		lstrcpyn(str, dbv.pszVal, lstrlen(dbv.pszVal)+1);
		DBFreeVariant(&dbv);
		if (strcmp(str, "") != 0) return 1;
	}
/*	if (DBGetContactSetting(hContact, lpzProto, "StatusDescr", &dbv) == 0) {
		lstrcpyn(str, dbv.pszVal, lstrlen(dbv.pszVal)+1);
		DBFreeVariant(&dbv);
		if (!strcmp(str, "")) return 0;
		return 1;
	}
	if (DBGetContactSetting(hContact, lpzProto, "YMsg", &dbv) == 0) {
		lstrcpyn(str, dbv.pszVal, lstrlen(dbv.pszVal)+1);
		DBFreeVariant(&dbv);
		if (!strcmp(str, "")) return 0;
		return 1;
	}*/
	return 0;
}

static void BuildContactsStatusMsgList(HWND hList) {
	HIMAGELIST hImgList;
	LVCOLUMN lvc;
	//set listview styles
	ListView_SetExtendedListViewStyleEx(hList,
		LVS_EX_FULLROWSELECT
		| 0x00004000 // LVS_EX_LABELTIP
		//| 0x00008000 // LVS_EX_BORDERSELECT
		//| LVS_EX_GRIDLINES
		, -1);
	//create image list
	hImgList = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	if (hImgList != NULL)
		ListView_SetImageList(hList, hImgList, LVSIL_SMALL);
	//add header columns to listview
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH;
	lvc.fmt = LVCFMT_IMAGE;
	lvc.iSubItem = 0;
	lvc.cx = DBGetContactSettingWord(NULL, MODULE, "List_ColWidth0", 20);
	ListView_InsertColumn(hList, 0, &lvc);
	lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;
	lvc.iSubItem = 1;
	lvc.pszText = Translate("Nick");
	lvc.cx = DBGetContactSettingWord(NULL, MODULE, "List_ColWidth1", 70);
	ListView_InsertColumn(hList, 1, &lvc);
	lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;
	lvc.iSubItem = 2;
	lvc.pszText = Translate("Status Message");
	lvc.cx = DBGetContactSettingWord(NULL, MODULE, "List_ColWidth2", 250);
	ListView_InsertColumn(hList, 2, &lvc);
	//customize
	ListView_SetTextColor(hList, options.colListText);
	if (options.bUseBgImage && strcmp(options.listbgimage, "")) {
		LVBKIMAGE lvbi;
		ZeroMemory(&lvbi, sizeof(lvbi));
		lvbi.ulFlags = LVBKIF_SOURCE_URL | LVBKIF_STYLE_TILE;
		lvbi.pszImage = options.listbgimage;
		ListView_SetBkImage(hList, &lvbi);
		ListView_SetBkColor(hList, GetSysColor(COLOR_HIGHLIGHT));
		ListView_SetTextBkColor(hList, CLR_NONE);
	}
	else {
		ListView_SetBkColor(hList, options.colListBack);
		ListView_SetTextBkColor(hList, options.colListBack);
	}
}

static void LoadContactsStatusMsgList(HWND hList) {
	HANDLE hContact;
	LVITEM lvi;
	LVITEMDAT *lvidat;
	int i = 0;
	char smsg[2048];
	//add conacts to listview
	ZeroMemory(&lvi, sizeof(lvi));
	//start looking for status messages
	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) {
		if (CheckStatusMessage(hContact, smsg)) {
			lvidat = (LVITEMDAT*)malloc(sizeof(LVITEMDAT));
			lvidat->nick = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
			lvidat->proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			lvidat->iiProto = CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0);
			lvidat->iStatus = GetStatusOnlineness(DBGetContactSettingWord(hContact, lvidat->proto, "Status", 0));
			lvidat->hContact = hContact;
			lvi.mask = LVIF_PARAM | LVIF_IMAGE;
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.lParam = (LPARAM)lvidat;
			lvi.iImage = CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0);
			ListView_InsertItem(hList, &lvi);
			ListView_SetItemText(hList, i, 1, (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0));
			ListView_SetItemText(hList, i, 2, smsg);
			i++;
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}

static BOOL CALLBACK ShowListMainDlgProc(HWND hwndDlg, UINT Message, WPARAM wParam, LPARAM lParam) {
	HWND hList = GetDlgItem(hwndDlg, IDC_LIST);
	LVDLGDAT *lvdat;
	lvdat = (LVDLGDAT*)GetWindowLong(hwndDlg, GWL_USERDATA);
	switch (Message) {
		case WM_INITDIALOG: {
			TranslateDialogDefault(hwndDlg);
			SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM)ICO_LIST);
			SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_SMALL, (LPARAM)ICO_LIST);
			lvdat = (LVDLGDAT*)malloc(sizeof(LVDLGDAT));
			lvdat->iLastColumnSortIndex = DBGetContactSettingByte(NULL, MODULE, "List_LastColSort", 2);
			lvdat->bSortAscending = DBGetContactSettingByte(NULL, MODULE, "List_LastSortOrder", 1);
			lvdat->iProtoSort = DBGetContactSettingByte(NULL, MODULE, "List_LastProtoSort", 0);
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lvdat);
			BuildContactsStatusMsgList(hList);
			LoadContactsStatusMsgList(hList);
			Utils_RestoreWindowPosition(hwndDlg, NULL, MODULE, "List_");
			ListView_SortItems(hList, ListSortFunc, (LPARAM)lvdat);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			WindowList_Add(hWindowList, hwndDlg, NULL/*hListDlg*/);
			ShowWindow(hwndDlg, SW_SHOW);
			break;
		}
		case WM_NOTIFY: {
			switch (((LPNMHDR)lParam)->idFrom) {
				case IDC_LIST:
					switch (((LPNMHDR)lParam)->code) {
						case NM_DBLCLK: {
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
						case NM_RCLICK: {
							LVHITTESTINFO lvh;
							POINT p;
							GetCursorPos(&p);
							ZeroMemory(&lvh, sizeof(lvh));
							lvh.pt = p;
							ScreenToClient(hList, &lvh.pt);
							ListView_HitTest(hList, &lvh);
							if ((lvh.flags & (LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON)) && lvh.iItem != -1)
								ListOpenContactMenu(hwndDlg, hList, lvh.iItem, lvdat);
							break;
						}
						case LVN_COLUMNCLICK: {
							LPNMLISTVIEW nmlv = (LPNMLISTVIEW)lParam;
							if (nmlv->iSubItem == 2) {
								SendMessage(hwndDlg, WM_USER + 10, 0, 0);
								break;
							}
							if (nmlv->iSubItem != lvdat->iLastColumnSortIndex) {
								lvdat->bSortAscending = 1;
								lvdat->iLastColumnSortIndex = nmlv->iSubItem;
								lvdat->iProtoSort = 0;
							}
							else {
								lvdat->bSortAscending = !lvdat->bSortAscending;
								if (lvdat->bSortAscending == 1) lvdat->iProtoSort = !lvdat->iProtoSort;
							}
							ListView_SortItems(hList, ListSortFunc, (LPARAM)lvdat);
							break;
						}
					}
					break;
			}
			break;
		}
		case WM_MEASUREITEM: {
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		}
		case WM_DRAWITEM: {
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		}
		case WM_COMMAND: {
			if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)lvdat->hContact))
				break;
			switch (wParam) {
				case IDOK:
					ListOpenContact(hList, ListView_GetNextItem(hList, -1, LVIS_SELECTED));
					break;
				case IDCANCEL:
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
			}
			break;
		}
		case WM_USER + 10:
			ListView_DeleteAllItems(hList);
			LoadContactsStatusMsgList(hList);
			ListView_SortItems(hList, ListSortFunc, (LPARAM)lvdat);
			return 0;
		case WM_GETMINMAXINFO: {
			MINMAXINFO mmi;
			CopyMemory (&mmi, (LPMINMAXINFO) lParam, sizeof (MINMAXINFO));
			//The minimum width in points
			mmi.ptMinTrackSize.x = ListView_GetColumnWidth(hList, 0) + ListView_GetColumnWidth(hList, 1) + 50;//250;
			//The minimum height in points
			mmi.ptMinTrackSize.y = 150;
			CopyMemory ((LPMINMAXINFO) lParam, &mmi, sizeof (MINMAXINFO));
			break;
		}
		case WM_SIZE: {
			RECT rc;
			POINT p;
			GetWindowRect(hwndDlg, &rc);
			p.x = rc.right - GetSystemMetrics(SM_CYDLGFRAME);
			p.y = rc.bottom - GetSystemMetrics(SM_CXDLGFRAME);
			ScreenToClient(hwndDlg, &p);
			MoveWindow(hList, 0, 0, p.x, p.y, TRUE);
			p.x = p.x - ListView_GetColumnWidth(hList, 0) - ListView_GetColumnWidth(hList, 1) - GetSystemMetrics(SM_CYHSCROLL);
			//ListView_SetColumnWidth(hList, 2, LVSCW_AUTOSIZE);
			ListView_SetColumnWidth(hList, 2, p.x);
			break;
		}
		case WM_CLOSE: {
			DestroyWindow(hwndDlg);
			KillTimer(hwndDlg, TIMERID_LISTAUTOREFRESH);
			WindowList_Remove(hWindowList, hwndDlg);
			break;
		}
		case WM_DESTROY: {
			int i;
			char szSetting[32];
			Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "List_");
			for (i = 0; i <= 2; i++) {
				wsprintf(szSetting, "List_ColWidth%d", i);
				DBWriteContactSettingWord(NULL, MODULE, szSetting, (WORD)ListView_GetColumnWidth(hList, i));
			}
			DBWriteContactSettingByte(NULL, MODULE, "List_LastColSort", (BYTE)lvdat->iLastColumnSortIndex);
			DBWriteContactSettingByte(NULL, MODULE, "List_LastSortOrder", (BYTE)lvdat->bSortAscending);
			DBWriteContactSettingByte(NULL, MODULE, "List_LastProtoSort", (BYTE)lvdat->iProtoSort);
			//Remove entry from Window list
			WindowList_Remove(hWindowList, hwndDlg);
			break;
		}
	}
	return FALSE;
}

void ShowList() {
	HWND hListDlg;

	hListDlg = WindowList_Find(hWindowList, (HANDLE)NULL);
	if (hListDlg == NULL) {
	//if (hListDlg == NULL || WindowList_Find(hWindowList, (HANDLE)hListDlg) == NULL) {
		hListDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_LIST), NULL, ShowListMainDlgProc, (LPARAM)NULL);
	}
	else {
		SetForegroundWindow(hListDlg);
		SetFocus(hListDlg);
	}
}

