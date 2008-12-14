/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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
#include "simpleaway.h"

HANDLE h_prebuildmenu;

static HANDLE hAwayMsgMenuItem;
static HANDLE hCopyMsgMenuItem;
static HANDLE hWindowList;
static HANDLE hWindowList2;

struct AwayMsgDlgData {
	HANDLE hContact;
	HANDLE hSeq;
	HANDLE hAwayMsgEvent;
};
#define HM_AWAYMSG  (WM_USER+11)
static BOOL CALLBACK ReadAwayMsgDlgProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam) {
	struct AwayMsgDlgData *dat;
	dat=(struct AwayMsgDlgData*)GetWindowLong(hwndDlg,GWL_USERDATA);
	switch(message) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			dat=(struct AwayMsgDlgData*)mir_alloc(sizeof(struct AwayMsgDlgData));
			SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)dat);
			dat->hContact=(HANDLE)lParam;
			dat->hAwayMsgEvent=HookEventMessage(ME_PROTO_ACK,hwndDlg,HM_AWAYMSG);
			dat->hSeq=(HANDLE)CallContactService(dat->hContact,PSS_GETAWAYMSG,0,0);
			WindowList_Add(hWindowList,hwndDlg,dat->hContact);
			{	char str[256],format[128];
				char *status,*contactName;
				char *szProto;
				WORD dwStatus;
				contactName=(char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)dat->hContact,0);
				szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)dat->hContact,0);
				dwStatus = DBGetContactSettingWord(dat->hContact,szProto,"Status",ID_STATUS_OFFLINE);
				status=(char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,dwStatus,0);
				GetWindowText(hwndDlg,format,SIZEOF(format));
				_snprintf(str,SIZEOF(str),format,status,contactName);
				SetWindowText(hwndDlg,str);
				GetDlgItemText(hwndDlg,IDC_RETRIEVING,format,SIZEOF(format));
				_snprintf(str,SIZEOF(str),format,status);
				SetDlgItemText(hwndDlg,IDC_RETRIEVING,str);
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedProtoIcon(szProto, dwStatus));
				EnableWindow(GetDlgItem(hwndDlg, IDC_COPY), FALSE);
			}
			return TRUE;
		case HM_AWAYMSG: {
			ACKDATA *ack=(ACKDATA*)lParam;
			if(ack->hContact!=dat->hContact) break;
			if(ack->type!=ACKTYPE_AWAYMSG) break;
			if(ack->hProcess!=dat->hSeq) break;
			if(ack->result!=ACKRESULT_SUCCESS) break;
			if(dat->hAwayMsgEvent!=NULL) {UnhookEvent(dat->hAwayMsgEvent); dat->hAwayMsgEvent=NULL;}
			SetDlgItemText(hwndDlg,IDC_MSG,(const char*)ack->lParam);
			if (ack->lParam) {
				if (strlen((char*)ack->lParam))
					EnableWindow(GetDlgItem(hwndDlg, IDC_COPY), TRUE);
			}
			ShowWindow(GetDlgItem(hwndDlg,IDC_RETRIEVING),SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg,IDC_MSG),SW_SHOW);
			SetDlgItemText(hwndDlg,IDOK,Translate("&Close"));
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDCANCEL:
				case IDOK:
					DestroyWindow(hwndDlg);
					break;
				case IDC_COPY: {
					if (OpenClipboard(hwndDlg)) {
						if (EmptyClipboard()) {
							HGLOBAL hglbCopy;
							LPTSTR  lptstrCopy;
							int		len;
							char	msg[1024];

							len = GetDlgItemText(hwndDlg, IDC_MSG, msg, SIZEOF(msg));
							if (len) {
								hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(TCHAR)); 
								if (hglbCopy == NULL) { 
									CloseClipboard(); 
									break; 
								}
								lptstrCopy = GlobalLock(hglbCopy); 
								memcpy(lptstrCopy, msg, len*sizeof(TCHAR)); 
								lptstrCopy[len] = (TCHAR)0;    // null character 
								GlobalUnlock(hglbCopy);
								SetClipboardData(CF_TEXT, hglbCopy);
							}
						}
						CloseClipboard();
					}
				}
				break;
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;
		case WM_DESTROY:
			if(dat->hAwayMsgEvent!=NULL) UnhookEvent(dat->hAwayMsgEvent);
			WindowList_Remove(hWindowList,hwndDlg);
			mir_free(dat);
			break;
	}
	return FALSE;
}

static int GetMessageCommand(WPARAM wParam,LPARAM lParam) {
	HWND hwnd;
	if(hwnd=WindowList_Find(hWindowList,(HANDLE)wParam)) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_READAWAYMSG),NULL,ReadAwayMsgDlgProc,wParam);
	return 0;
}

static BOOL CALLBACK CopyAwayMsgDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	struct AwayMsgDlgData *dat;

	dat=(struct AwayMsgDlgData*)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch(message) {
		case WM_INITDIALOG: {
			char str[256],format[128];
			char *contactName;

			TranslateDialogDefault(hwndDlg);
			dat = (struct AwayMsgDlgData*)mir_alloc(sizeof(struct AwayMsgDlgData));
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)dat);
			dat->hContact = (HANDLE)lParam;
			dat->hAwayMsgEvent = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_AWAYMSG);
			dat->hSeq = (HANDLE)CallContactService(dat->hContact, PSS_GETAWAYMSG, 0, 0);
			WindowList_Add(hWindowList2, hwndDlg, dat->hContact);
			contactName = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)dat->hContact, 0);
			GetWindowText(hwndDlg, format, SIZEOF(format));
			_snprintf(str, SIZEOF(str), format, contactName);
			SetWindowText(hwndDlg, str);
			return TRUE;
		}
		case HM_AWAYMSG: {	
			ACKDATA *ack=(ACKDATA*)lParam;
			if(ack->hContact!=dat->hContact)
				break;
			if(ack->type!=ACKTYPE_AWAYMSG)
				break;
			if(ack->hProcess!=dat->hSeq)
				break;
			if(ack->result!=ACKRESULT_SUCCESS)
				break;
			if(dat->hAwayMsgEvent != NULL) {
				UnhookEvent(dat->hAwayMsgEvent);
				dat->hAwayMsgEvent = NULL;
			}
			if (ack->lParam) {
				if (lstrlen((char*)ack->lParam)) {
					if (OpenClipboard(hwndDlg)) {
						if (EmptyClipboard()) {
							HGLOBAL hglbCopy;
							LPTSTR  lptstrCopy;
							int		len = lstrlen((char*)ack->lParam);

							hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(TCHAR)); 
							if (hglbCopy == NULL) { 
								CloseClipboard();
								DestroyWindow(hwndDlg);
								break; 
							}
							lptstrCopy = GlobalLock(hglbCopy);
							memcpy(lptstrCopy, (char*)ack->lParam, len*sizeof(TCHAR));
							lptstrCopy[len] = (TCHAR)0;    // null character 
							GlobalUnlock(hglbCopy);
							SetClipboardData(CF_TEXT, hglbCopy);
						}
						CloseClipboard();
					}
				}
			}
			DestroyWindow(hwndDlg);
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDCANCEL:
				case IDOK:
					DestroyWindow(hwndDlg);
				break;
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;
		case WM_DESTROY:
			if(dat->hAwayMsgEvent!=NULL)
				UnhookEvent(dat->hAwayMsgEvent);
			WindowList_Remove(hWindowList2,hwndDlg);
			mir_free(dat);
			break;
	}
	return FALSE;
}

static int CopyAwayMsgInit(WPARAM wParam, LPARAM lParam) {
	HWND hwnd;
	if(hwnd=WindowList_Find(hWindowList2, (HANDLE)wParam)) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_COPY), NULL, CopyAwayMsgDlgProc, wParam);
	return 0;
}

/*static*/ int AwayMsgPreBuildMenu(WPARAM wParam,LPARAM lParam) {
	CLISTMENUITEM clmi;
	char str[128];
	int status;
	char *szProto;

	szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);
	ZeroMemory(&clmi,sizeof(clmi));
	clmi.cbSize=sizeof(clmi);
	clmi.flags=CMIM_FLAGS|CMIF_HIDDEN;
	
	if (szProto != NULL) {
		int chatRoom = szProto ? DBGetContactSettingByte((HANDLE)wParam, szProto, "ChatRoom", 0) : 0;
		if ( !chatRoom ) {
			status = DBGetContactSettingWord((HANDLE)wParam,szProto,"Status",ID_STATUS_OFFLINE);
			wsprintf(str,Translate("Re&ad %s Message"),(char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,status,0));
			clmi.pszName = str;
			if(CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_1,0) & PF1_MODEMSGRECV) {
				if(CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_3,0) & Proto_Status2Flag(status == ID_STATUS_OFFLINE ? ID_STATUS_INVISIBLE : status)) {
					clmi.flags = CMIM_FLAGS | CMIM_NAME | CMIM_ICON;
					clmi.hIcon = LoadSkinnedProtoIcon(szProto, status);
				}
			}
		}
	}
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hAwayMsgMenuItem,(LPARAM)&clmi);

	if (!ShowCopy)
		clmi.flags |= CMIF_HIDDEN;

	wsprintf(str,Translate("Copy %s Message"),(char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,status,0));
	clmi.pszName = str;

	if (ServiceExists(MS_SKIN2_GETICON))
		clmi.hIcon = (HICON)CallService(MS_SKIN2_GETICON, (WPARAM)0, (LPARAM)ICON_COPY);
	else
		clmi.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_COPY), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hCopyMsgMenuItem,(LPARAM)&clmi);
	return 0;
}

int AwayMsgPreShutdown(void) {
	UnhookEvent(h_prebuildmenu);
	if (hWindowList)
		WindowList_BroadcastAsync(hWindowList,WM_CLOSE,0,0);
	if (hWindowList2)
		WindowList_BroadcastAsync(hWindowList2,WM_CLOSE,0,0);
	return 0;
}

int LoadAwayMsgModule(void) {
	CLISTMENUITEM mi;

	hWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
	CreateServiceFunction(MS_AWAYMSG_SHOWAWAYMSG,GetMessageCommand);
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=0;
	mi.hIcon=NULL;
	mi.pszContactOwner=NULL;
	mi.pszName=LPGEN("Re&ad Away Message");
	mi.pszService=MS_AWAYMSG_SHOWAWAYMSG;
	hAwayMsgMenuItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	hWindowList2 = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
	CreateServiceFunction(MS_SA_COPYAWAYMSG, CopyAwayMsgInit);
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-2000006000;
	mi.flags=0;
	mi.hIcon = NULL;
	mi.pszContactOwner=NULL;
	mi.pszName=LPGEN("Copy Away Message");
	mi.pszService = MS_SA_COPYAWAYMSG;
	hCopyMsgMenuItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	h_prebuildmenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU,AwayMsgPreBuildMenu);

	return 0;
}
