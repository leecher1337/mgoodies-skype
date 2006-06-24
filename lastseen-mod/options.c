#include "seen.h"



extern HINSTANCE hInstance;
extern HANDLE ehuserinfo,hmenuitem,ehmissed_proto;
void BuildInfo(char *,char *,char *);
int BuildContactMenu(WPARAM,LPARAM);
int UserinfoInit(WPARAM,LPARAM);
int InitFileOutput(void);
void ShutdownFileOutput(void);
void InitMenuitem(void);
int ModeChange_mo(WPARAM,LPARAM);
int CheckIfOnline(void);
int ResetMissed(void);

BOOL CALLBACK OptsPopUpsDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	DBVARIANT dbv;
	int i;
	char szstamp[256];
	BOOL hasPopups;
	switch(msg)
	{
		case WM_INITDIALOG:{
			if (hasPopups = (ServiceExists(MS_POPUP_QUERY) != 0))
				hasPopups = CallService(MS_POPUP_QUERY,PUQS_GETSTATUS,0);
			TranslateDialogDefault(hdlg);
			ShowWindow(GetDlgItem(hdlg,IDC_MISSPOPUP),hasPopups?SW_HIDE:SW_SHOW);
			ShowWindow(GetDlgItem(hdlg,IDC_POPUPS),hasPopups?SW_SHOW:SW_HIDE);
			ShowWindow(GetDlgItem(hdlg,IDC_POPUPSTAMP),hasPopups?SW_SHOW:SW_HIDE);
			ShowWindow(GetDlgItem(hdlg,IDC_LABTEXT),hasPopups?SW_SHOW:SW_HIDE);
			ShowWindow(GetDlgItem(hdlg,IDC_LABTTITLE),hasPopups?SW_SHOW:SW_HIDE);
			ShowWindow(GetDlgItem(hdlg,IDC_POPUPSTAMPTEXT),hasPopups?SW_SHOW:SW_HIDE);
			CheckDlgButton(hdlg,IDC_POPUPS,DBGetContactSettingByte(NULL,S_MOD,"UsePopups",0)&hasPopups);
			EnableWindow(GetDlgItem(hdlg,IDC_POPUPS),hasPopups);
			hasPopups = IsDlgButtonChecked(hdlg,IDC_POPUPS);
			EnableWindow(GetDlgItem(hdlg,IDC_POPUPSTAMP),hasPopups);
			EnableWindow(GetDlgItem(hdlg,IDC_POPUPSTAMPTEXT),hasPopups);
			for (i=ID_STATUS_OFFLINE;i<=ID_STATUS_OUTTOLUNCH;i++){
				DWORD sett;
				COLORREF back, text;
				sprintf(szstamp, "Col_%d",i-ID_STATUS_OFFLINE);
				sett = DBGetContactSettingDword(NULL,S_MOD,szstamp,StatusColors15bits[i-ID_STATUS_OFFLINE]);
				GetColorsFromDWord(&back, &text, sett);
				SendDlgItemMessage(hdlg,i,CPM_SETCOLOUR,0,back);
				SendDlgItemMessage(hdlg,i+20,CPM_SETCOLOUR,0,text);
				EnableWindow(GetDlgItem(hdlg,i),hasPopups);
				EnableWindow(GetDlgItem(hdlg,i+20),hasPopups);
			}

			SetDlgItemText(hdlg,IDC_POPUPSTAMP,!DBGetContactSetting(NULL,S_MOD,"PopupStamp",&dbv)?dbv.pszVal:DEFAULT_POPUPSTAMP);
			DBFreeVariant(&dbv);
			SetDlgItemText(hdlg,IDC_POPUPSTAMPTEXT,!DBGetContactSetting(NULL,S_MOD,"PopupStampText",&dbv)?dbv.pszVal:DEFAULT_POPUPSTAMPTEXT);
			DBFreeVariant(&dbv);
#ifndef PERMITNSN
			i = DBGetContactSettingByte(NULL,S_MOD,"SuppCListOnline",3);
			CheckDlgButton(hdlg,IDC_DISWATCHED,i&1);
			CheckDlgButton(hdlg,IDC_DISNONWATCHED,i&2);
#endif
		}
		break; //case WM_INITDIALOG
		case WM_COMMAND:
			if((HIWORD(wparam)==BN_CLICKED || HIWORD(wparam)==EN_CHANGE) && GetFocus()==(HWND)lparam)
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			else if (HIWORD(wparam)==CPN_COLOURCHANGED){
				WORD idText, idBack;
				POPUPDATAEX ppd = {0};
				DBVARIANT dbv = {0};
				DWORD temp;
				if (LOWORD(wparam)>ID_STATUS_OUTTOLUNCH){ // we have clicked a text color
					idText = wparam; idBack = wparam-20;
				} else {idText = wparam+20; idBack = wparam;}
				ppd.colorBack = SendDlgItemMessage(hdlg,idBack,CPM_GETCOLOUR,0,0);
				ppd.colorText = SendDlgItemMessage(hdlg,idText,CPM_GETCOLOUR,0,0);
				temp = GetDWordFromColors(ppd.colorBack,ppd.colorText);
				GetColorsFromDWord(&ppd.colorBack,&ppd.colorText,temp);
				SendDlgItemMessage(hdlg,idBack,CPM_SETCOLOUR,0,ppd.colorBack);
				SendDlgItemMessage(hdlg,idText,CPM_SETCOLOUR,0,ppd.colorText);
				ppd.lchIcon = LoadSkinnedProtoIcon(NULL, idBack);
				GetDlgItemText(hdlg,IDC_POPUPSTAMP,szstamp,255);
				strncpy(ppd.lpzContactName,ParseString(szstamp,NULL,0),MAX_CONTACTNAME);
				GetDlgItemText(hdlg,IDC_POPUPSTAMPTEXT,szstamp,255);
				strncpy(ppd.lpzText,ParseString(szstamp,NULL,0),MAX_SECONDLINE);
				CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);

				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			} 
			if(HIWORD(wparam)==BN_CLICKED)
			{
				switch(LOWORD(wparam)){
					case IDC_POPUPS:
						hasPopups = IsDlgButtonChecked(hdlg,IDC_POPUPS);
						EnableWindow(GetDlgItem(hdlg,IDC_POPUPSTAMP),hasPopups);
						EnableWindow(GetDlgItem(hdlg,IDC_POPUPSTAMPTEXT),hasPopups);
						for (i=ID_STATUS_OFFLINE;i<=ID_STATUS_OUTTOLUNCH;i++){
							EnableWindow(GetDlgItem(hdlg,i),hasPopups);
							EnableWindow(GetDlgItem(hdlg,i+20),hasPopups);
						}
						break;
					case IDC_DEFAULTCOL:
						for (i=ID_STATUS_OFFLINE;i<=ID_STATUS_OUTTOLUNCH;i++){
							DWORD sett;
							COLORREF back, text;
							sprintf(szstamp, "Col_%d",i-ID_STATUS_OFFLINE);
							sett = StatusColors15bits[i-ID_STATUS_OFFLINE];
							GetColorsFromDWord(&back, &text, sett);
							SendDlgItemMessage(hdlg,i,CPM_SETCOLOUR,0,back);
							SendDlgItemMessage(hdlg,i+20,CPM_SETCOLOUR,0,text);
						}
						break;
				}
			}
			break; //case WM_COMMAND

		case WM_NOTIFY:
			switch(((LPNMHDR)lparam)->idFrom) 
			{
				case 0: 
					switch (((LPNMHDR)lparam)->code)
					{
						BYTE checkValue;

						case PSN_APPLY:
							GetDlgItemText(hdlg,IDC_POPUPSTAMP,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"PopupStamp",szstamp);
							GetDlgItemText(hdlg,IDC_POPUPSTAMPTEXT,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"PopupStampText",szstamp);

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_POPUPS);
							if (DBGetContactSettingByte(NULL,S_MOD,"UsePopups",0) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"UsePopups",checkValue);
							}
							for (i=ID_STATUS_OFFLINE;i<=ID_STATUS_OUTTOLUNCH;i++){
								DWORD sett;
								COLORREF back=0, text=0;
								sprintf(szstamp, "Col_%d",i-ID_STATUS_OFFLINE);
								back = SendDlgItemMessage(hdlg,i,CPM_GETCOLOUR,0,0);
								text = SendDlgItemMessage(hdlg,i+20,CPM_GETCOLOUR,0,0);
								sett=GetDWordFromColors(back,text);
								if (sett!=StatusColors15bits[i-ID_STATUS_OFFLINE])
									DBWriteContactSettingDword(NULL,S_MOD,szstamp,sett);
								else DBDeleteContactSetting(NULL,S_MOD,szstamp);
							}
#ifndef PERMITNSN
							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_DISNONWATCHED)<<1;
							checkValue |= (BYTE)IsDlgButtonChecked(hdlg,IDC_DISWATCHED);
							if (3 == checkValue) DBDeleteContactSetting(NULL,S_MOD,"SuppCListOnline");
							else DBWriteContactSettingByte(NULL,S_MOD,"SuppCListOnline",checkValue);
#endif
							break; //case PSN_APPLY
					}
					break; //case 0
			}
			break;//case WM_NOTIFY
       
	}

	return 0;
}

BOOL CALLBACK OptsSettingsDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	DBVARIANT dbv;
	char szstamp[256];
	BYTE bchecked=0;
	WPARAM wpsend=0;

	switch(msg)
	{
		case WM_INITDIALOG:{
			TranslateDialogDefault(hdlg);

			CheckDlgButton(hdlg,IDC_MENUITEM,DBGetContactSettingByte(NULL,S_MOD,"MenuItem",1));
			CheckDlgButton(hdlg,IDC_USERINFO,DBGetContactSettingByte(NULL,S_MOD,"UserinfoTab",1));
			CheckDlgButton(hdlg,IDC_FILE,DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0));
			CheckDlgButton(hdlg,IDC_HISTORY,DBGetContactSettingByte(NULL,S_MOD,"KeepHistory",0));
			CheckDlgButton(hdlg,IDC_IGNOREOFFLINE,DBGetContactSettingByte(NULL,S_MOD,"IgnoreOffline",1));
			CheckDlgButton(hdlg,IDC_MISSEDONES,DBGetContactSettingByte(NULL,S_MOD,"MissedOnes",0));
			CheckDlgButton(hdlg,IDC_SHOWICON,DBGetContactSettingByte(NULL,S_MOD,"ShowIcon",1));
			CheckDlgButton(hdlg,IDC_COUNT,DBGetContactSettingByte(NULL,S_MOD,"MissedOnes_Count",0));

			EnableWindow(GetDlgItem(hdlg,IDC_MENUSTAMP),IsDlgButtonChecked(hdlg,IDC_MENUITEM));
			EnableWindow(GetDlgItem(hdlg,IDC_SHOWICON),IsDlgButtonChecked(hdlg,IDC_MENUITEM));
			EnableWindow(GetDlgItem(hdlg,IDC_USERSTAMP),IsDlgButtonChecked(hdlg,IDC_USERINFO));
			EnableWindow(GetDlgItem(hdlg,IDC_FILESTAMP),IsDlgButtonChecked(hdlg,IDC_FILE));
			EnableWindow(GetDlgItem(hdlg,IDC_FILENAME),IsDlgButtonChecked(hdlg,IDC_FILE));
			EnableWindow(GetDlgItem(hdlg,IDC_HISTORYSIZE),IsDlgButtonChecked(hdlg,IDC_HISTORY));
			EnableWindow(GetDlgItem(hdlg,IDC_HISTORYSTAMP),IsDlgButtonChecked(hdlg,IDC_HISTORY));
			EnableWindow(GetDlgItem(hdlg,IDC_COUNT),IsDlgButtonChecked(hdlg,IDC_MISSEDONES));

			SetDlgItemText(hdlg,IDC_MENUSTAMP,!DBGetContactSetting(NULL,S_MOD,"MenuStamp",&dbv)?dbv.pszVal:DEFAULT_MENUSTAMP);
			DBFreeVariant(&dbv);
			SetDlgItemText(hdlg,IDC_USERSTAMP,!DBGetContactSetting(NULL,S_MOD,"UserStamp",&dbv)?dbv.pszVal:DEFAULT_USERSTAMP);
			DBFreeVariant(&dbv);
			SetDlgItemText(hdlg,IDC_FILESTAMP,!DBGetContactSetting(NULL,S_MOD,"FileStamp",&dbv)?dbv.pszVal:DEFAULT_FILESTAMP);
			DBFreeVariant(&dbv);
			SetDlgItemText(hdlg,IDC_FILENAME,!DBGetContactSetting(NULL,S_MOD,"FileName",&dbv)?dbv.pszVal:DEFAULT_FILENAME);
			DBFreeVariant(&dbv);
			SetDlgItemInt(hdlg,IDC_HISTORYSIZE,DBGetContactSettingWord(NULL,S_MOD,"HistoryMax",10-1)-1,FALSE);
			SetDlgItemText(hdlg,IDC_HISTORYSTAMP,!DBGetContactSetting(NULL,S_MOD,"HistoryStamp",&dbv)?dbv.pszVal:DEFAULT_HISTORYSTAMP);
			DBFreeVariant(&dbv);

			// load protocol list
			SetWindowLong(GetDlgItem(hdlg,IDC_PROTOCOLLIST),GWL_STYLE,GetWindowLong(GetDlgItem(hdlg,IDC_PROTOCOLLIST),GWL_STYLE)|TVS_CHECKBOXES);
			{	
				TVINSERTSTRUCT tvis;
				int numberOfProtocols,i;
				PROTOCOLDESCRIPTOR** protos;
				char *protoName;
				char *protoLabel;

				tvis.hParent=NULL;
				tvis.hInsertAfter=TVI_LAST;
				tvis.item.mask=TVIF_TEXT | TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
				tvis.item.stateMask = TVIS_STATEIMAGEMASK;

				CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&numberOfProtocols,(LPARAM)&protos);
				for (i=0; i<numberOfProtocols; i++) {
					if(protos[i]->type!=PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName,PS_GETCAPS,PFLAGNUM_2,0)==0) continue;
					protoName = (char *)malloc(strlen(protos[i]->szName)+1);
					strcpy(protoName,protos[i]->szName);
//debug(protoName);
					protoLabel = (char *)malloc(MAXMODULELABELLENGTH+1);
					CallProtoService(protoName,PS_GETNAME,MAXMODULELABELLENGTH,(LPARAM)protoLabel);
//debug(protoLabel);
					tvis.item.pszText = protoLabel;
					tvis.item.lParam = (LPARAM)protoName;
					tvis.item.state = INDEXTOSTATEIMAGEMASK(IsWatchedProtocol(protoName)+1);
					TreeView_InsertItem(GetDlgItem(hdlg,IDC_PROTOCOLLIST),&tvis);
					free(protoLabel);

				}
			}
		   }
			break; //case WM_INITDIALOG

		case WM_COMMAND:
			if((HIWORD(wparam)==BN_CLICKED || HIWORD(wparam)==EN_CHANGE) && GetFocus()==(HWND)lparam)
				if (LOWORD(wparam)!=IDC_VARIABLES)SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);

			if(HIWORD(wparam)==BN_CLICKED)
			{
				switch(LOWORD(wparam)){
					case IDC_MENUITEM:
						EnableWindow(GetDlgItem(hdlg,IDC_MENUSTAMP),IsDlgButtonChecked(hdlg,IDC_MENUITEM));
						EnableWindow(GetDlgItem(hdlg,IDC_SHOWICON),IsDlgButtonChecked(hdlg,IDC_MENUITEM));
						break;
					case IDC_USERINFO:
						EnableWindow(GetDlgItem(hdlg,IDC_USERSTAMP),IsDlgButtonChecked(hdlg,IDC_USERINFO));
						break;
					case IDC_FILE:
						EnableWindow(GetDlgItem(hdlg,IDC_FILESTAMP),IsDlgButtonChecked(hdlg,IDC_FILE));
						EnableWindow(GetDlgItem(hdlg,IDC_FILENAME),IsDlgButtonChecked(hdlg,IDC_FILE));
						break;
					case IDC_HISTORY:
						EnableWindow(GetDlgItem(hdlg,IDC_HISTORYSTAMP),IsDlgButtonChecked(hdlg,IDC_HISTORY));
						EnableWindow(GetDlgItem(hdlg,IDC_HISTORYSIZE),IsDlgButtonChecked(hdlg,IDC_HISTORY));
						break;
					case IDC_MISSEDONES:
						EnableWindow(GetDlgItem(hdlg,IDC_COUNT),IsDlgButtonChecked(hdlg,IDC_MISSEDONES));
						break;
				}
			}
			
			if (LOWORD(wparam)==IDC_VARIABLES)
			{
				char szout[2048]="";
				wsprintf(szout,VARIABLE_LIST);
				MessageBox(NULL,szout,"Last Seen Variables",MB_OK|MB_TOPMOST);
			}

			break; //case WM_COMMAND

		case WM_NOTIFY:
			switch(((LPNMHDR)lparam)->idFrom) 
			{
				case 0: 
					switch (((LPNMHDR)lparam)->code)
					{
						BYTE checkValue;

						case PSN_APPLY:

							GetDlgItemText(hdlg,IDC_MENUSTAMP,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"MenuStamp",szstamp);

							GetDlgItemText(hdlg,IDC_USERSTAMP,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"UserStamp",szstamp);

							GetDlgItemText(hdlg,IDC_FILESTAMP,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"FileStamp",szstamp);

							GetDlgItemText(hdlg,IDC_FILENAME,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"FileName",szstamp);

							GetDlgItemText(hdlg,IDC_HISTORYSTAMP,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"HistoryStamp",szstamp);
							
							DBWriteContactSettingWord(NULL,S_MOD,"HistoryMax",(WORD)(GetDlgItemInt(hdlg,IDC_HISTORYSIZE,NULL,FALSE)+1));

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_MENUITEM);
							if (DBGetContactSettingByte(NULL,S_MOD,"MenuItem",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"MenuItem",checkValue);
								if(hmenuitem==NULL && checkValue) {
									InitMenuitem();
								}
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_USERINFO);
							if (DBGetContactSettingByte(NULL,S_MOD,"UserinfoTab",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"UserinfoTab",checkValue);
								if(checkValue) {
									ehuserinfo=HookEvent(ME_USERINFO_INITIALISE,UserinfoInit);
								} else {
									UnhookEvent(ehuserinfo);
								}
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_FILE);
							if (DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"FileOutput",checkValue);
								if(checkValue) {
									InitFileOutput();
								}
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_HISTORY);
							if (DBGetContactSettingByte(NULL,S_MOD,"KeepHistory",0) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"KeepHistory",checkValue);
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_IGNOREOFFLINE);
							if (DBGetContactSettingByte(NULL,S_MOD,"IgnoreOffline",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"IgnoreOffline",checkValue);
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_MISSEDONES);
							if (DBGetContactSettingByte(NULL,S_MOD,"MissedOnes",0) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"MissedOnes",checkValue);
								if(checkValue) {
									ehmissed_proto=HookEvent(ME_PROTO_ACK,ModeChange_mo);
								} else {
									UnhookEvent(ehmissed_proto);
								}
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_SHOWICON);
							if (DBGetContactSettingByte(NULL,S_MOD,"ShowIcon",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"ShowIcon",checkValue);
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_COUNT);
							if (DBGetContactSettingByte(NULL,S_MOD,"MissedOnes_Count",0) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"MissedOnes_Count",checkValue);
							}

							// save protocol list
							{
								HWND hwndTreeView = GetDlgItem(hdlg,IDC_PROTOCOLLIST);
								HTREEITEM hItem;
								TVITEM tvItem;
								char *watchedProtocols;
								char *protocol;
								int size=1;

								watchedProtocols = (char *)malloc(sizeof(char));
								*watchedProtocols = '\0';
								hItem = TreeView_GetRoot(hwndTreeView);
								tvItem.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
								tvItem.stateMask = TVIS_STATEIMAGEMASK;

								while (hItem != NULL) {
									tvItem.hItem = hItem;
									TreeView_GetItem(hwndTreeView, &tvItem);
									protocol = (char*)tvItem.lParam;
									if ((BOOL)(tvItem.state >> 12) -1) {
										size = (size + strlen(protocol)+2) * sizeof(char);
										watchedProtocols = (char *)realloc(watchedProtocols, size);
										strcat(watchedProtocols, protocol); 
										strcat(watchedProtocols, " "); 
									}
									hItem = TreeView_GetNextSibling(hwndTreeView, hItem);
								}
								DBWriteContactSettingString(NULL,S_MOD,"WatchedProtocols",watchedProtocols);
								free(watchedProtocols);
							}

							break; //case PSN_APPLY
					}
					break; //case 0

				case IDC_PROTOCOLLIST:
					switch (((LPNMHDR)lparam)->code) 
					{
						case NM_CLICK:
							{
								HWND hTree=((LPNMHDR)lparam)->hwndFrom;
								TVHITTESTINFO hti;
								HTREEITEM hItem;

								hti.pt.x=(short)LOWORD(GetMessagePos());
								hti.pt.y=(short)HIWORD(GetMessagePos());
								ScreenToClient(hTree,&hti.pt);
								if(hItem=TreeView_HitTest(hTree,&hti))
								{
									if (hti.flags & TVHT_ONITEM) 
										TreeView_SelectItem(hTree,hItem);
									if (hti.flags & TVHT_ONITEMSTATEICON) 
										SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
									
								}
							}
							break; 
					}
					break; //case IDC_PROTOCOLLIST
			}
			break;//case WM_NOTIFY
        
		case WM_DESTROY:
			// free protocol list 
			{
				HWND hwndTreeView = GetDlgItem(hdlg,IDC_PROTOCOLLIST);
				HTREEITEM hItem;
				TVITEM tvItem;

				hItem = TreeView_GetRoot(hwndTreeView);
				tvItem.mask = TVIF_HANDLE | TVIF_PARAM;

				while (hItem != NULL) {
					tvItem.hItem = hItem;
					TreeView_GetItem(hwndTreeView, &tvItem);
					free((void *)tvItem.lParam);
					hItem = TreeView_GetNextSibling(hwndTreeView, hItem);
				}
			}
			break;

	}

	return 0;
}

long OptsSettingsDlg, OptsPopUpsDlg;

 BOOL CALLBACK OptTabDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG: 
		{	
			TCITEMA tci;
			RECT theTabSpace;
			RECT rcClient;
			{
				RECT rcTab, rcDlg;
				HWND hwndTab = GetDlgItem(hwndDlg, IDC_OPTIONSTAB);
				TabCtrl_GetItemRect(hwndTab,0,&rcTab);
				TabCtrl_DeleteAllItems(hwndTab);
				theTabSpace.top = rcTab.bottom; // the size of the tab
				GetWindowRect(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), &rcTab);
				GetWindowRect(hwndDlg, &rcDlg);
				theTabSpace.bottom = rcTab.bottom -rcTab.top  -theTabSpace.top;
				theTabSpace.top =  rcTab.top -rcDlg.top +theTabSpace.top;
				theTabSpace.left = rcTab.left - rcDlg.left;
				theTabSpace.right = rcTab.right-rcTab.left;
			}
			tci.mask = TCIF_PARAM|TCIF_TEXT;
			if (!OptsPopUpsDlg) OptsPopUpsDlg = (long)CreateDialog(hInstance,MAKEINTRESOURCE(IDD_POPUPS), hwndDlg, OptsPopUpsDlgProc);
			tci.lParam = OptsPopUpsDlg;
			GetClientRect((HWND)tci.lParam,&rcClient);
			tci.pszText = Translate("Popups");
			SendMessage(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), TCM_INSERTITEMA, (WPARAM)0, (LPARAM)&tci);
			MoveWindow((HWND)tci.lParam,theTabSpace.left+(theTabSpace.right-rcClient.right)/2,
				theTabSpace.top+(theTabSpace.bottom-rcClient.bottom)/2,
				rcClient.right,rcClient.bottom,1);
			ShowWindow((HWND)tci.lParam, SW_HIDE);

			if (!OptsSettingsDlg) OptsSettingsDlg = (long)CreateDialog(hInstance,MAKEINTRESOURCE(IDD_SETTINGS), hwndDlg, OptsSettingsDlgProc);
			tci.lParam = OptsSettingsDlg;
			tci.pszText = Translate("Settings");
			GetClientRect((HWND)tci.lParam,&rcClient);
			SendMessage(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), TCM_INSERTITEMA, (WPARAM)0, (LPARAM)&tci);
			MoveWindow((HWND)tci.lParam,theTabSpace.left+(theTabSpace.right-rcClient.right)/2,
				theTabSpace.top+(theTabSpace.bottom-rcClient.bottom)/2,
				rcClient.right,rcClient.bottom,1);
			ShowWindow((HWND)tci.lParam, SW_SHOW);
			TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_OPTIONSTAB),0);
			return TRUE;
		}
		case PSM_CHANGED:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (unsigned int)hwndDlg, 0);
			break;
		case WM_DESTROY:
			OptsSettingsDlg = OptsPopUpsDlg = 0;
			break;
		case WM_NOTIFY:
		{
		  switch(((LPNMHDR)lParam)->idFrom) {
            case 0:	{
			  BOOL CommandApply = FALSE;
			  if ( (CommandApply = lParam && ((LPNMHDR)lParam)->code == PSN_APPLY) || (lParam && ((LPNMHDR)lParam)->code == PSN_RESET) ) {
#ifdef _DEBUG
				MessageBoxA(hwndDlg,CommandApply?"Apply":"Cancel","EventHapened",0);
#endif
				if (CommandApply) {
					SendMessage((HWND)OptsSettingsDlg, WM_NOTIFY, wParam, lParam);
					SendMessage((HWND)OptsPopUpsDlg, WM_NOTIFY, wParam, lParam);
					return TRUE;
				} else {
				}
			  } //if PSN_APPLY
			}
            break;
            case IDC_OPTIONSTAB:
               switch (((LPNMHDR)lParam)->code)
               {
                  case TCN_SELCHANGING:
                     {
                        TCITEM tci;
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwndDlg,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwndDlg,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_HIDE);                     
                     }
                  break;
                  case TCN_SELCHANGE:
                     {
                        TCITEM tci;
						short int t;
                        tci.mask = TCIF_PARAM;
						t = TabCtrl_GetCurSel(GetDlgItem(hwndDlg,IDC_OPTIONSTAB));
                        TabCtrl_GetItem(GetDlgItem(hwndDlg,IDC_OPTIONSTAB),t,&tci);
                        ShowWindow((HWND)tci.lParam,SW_SHOW);                     
                     }
                  break;
               }
			   break;
			}
		  }//end case(LPNMHDR)lParam)->idFrom
		  break;			
	}
	return FALSE;
}



int OptionsInit(WPARAM wparam,LPARAM lparam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.pszGroup=Translate("Plugins");
	odp.hInstance=hInstance;
	odp.pszTemplate=MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszTitle=Translate("Last seen");
	odp.pfnDlgProc=OptTabDlgProc;
	odp.flags=ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE,wparam,(LPARAM)&odp);
	return 0;
}
