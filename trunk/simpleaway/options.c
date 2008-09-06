/*

SimpleAway plugin for Miranda-IM

Copyright © 2005 Harven, © 2006-2008 Dezeath

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

extern UINT	SATimer, SARandMsgTimer;
extern BOOL	is_timer, is_randmsgtimer;
extern void CALLBACK SATimerProc(HWND, UINT, UINT_PTR, DWORD);
extern void CALLBACK SARandMsgTimerProc(HWND, UINT, UINT_PTR, DWORD);
extern VOID APIENTRY HandlePopupMenu(HWND hwnd, POINT pt, HWND edit_control);

#ifndef GET_X_LPARAM
#define 	GET_X_LPARAM(lp)   ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define 	GET_Y_LPARAM(lp)   ((int)(short)HIWORD(lp))
#endif

WNDPROC OldDlgProc;

LRESULT CALLBACK OptEditBoxSubProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_CONTEXTMENU: {
			RECT rc;             
			POINT pt;

			GetClientRect(hwndDlg, (LPRECT) &rc);
 
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);

			if (pt.x == -1 && pt.y == -1) {
				GetCursorPos(&pt);
				if (!PtInRect((LPRECT) &rc, pt)) {
	                pt.x = rc.left + (rc.right - rc.left) / 2;
		            pt.y = rc.top + (rc.bottom - rc.top) / 2;
				}
            }
			else
				ScreenToClient(hwndDlg, &pt);

			if (PtInRect((LPRECT) &rc, pt)) 
				HandlePopupMenu(hwndDlg, pt, GetDlgItem(GetParent(hwndDlg), IDC_OPTEDIT1));
                
			return 0;
		}
        break;
		case WM_CHAR:
			if (wParam=='\n' && GetKeyState(VK_CONTROL)&0x8000) {
				PostMessage(GetParent(hwndDlg), WM_COMMAND, IDC_OK, 0);
				return 0;
			}
			if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000) {//ctrl-a
				SendMessage(hwndDlg, EM_SETSEL, 0, -1);
				return 0;
			}
			if (wParam == 23 && GetKeyState(VK_CONTROL) & 0x8000) {//ctrl-w
				SendMessage(GetParent(hwndDlg), WM_COMMAND, IDC_CANCEL, 0);
				return 0;
			}
			break;
	}

	return CallWindowProc(OldDlgProc, hwndDlg, uMsg, wParam, lParam);
}

struct SingleProtoMsg {
	int		flags;
	char	*msg;
	int		max_length;
};

struct SingleStatusMsg {
	int		flags[9];
	char	msg[9][1024];
};

struct OptDlgData {
	BOOL					proto_ok;
	int						proto_count;
	struct SingleProtoMsg	*proto_msg;
	struct SingleStatusMsg	*status_msg;
};

static BOOL	DlgInInit=FALSE;

INT_PTR CALLBACK DlgOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	struct OptDlgData *data;

	data = (struct OptDlgData*)GetWindowLong(hwndDlg,GWL_USERDATA);

	switch (uMsg) {
		case WM_INITDIALOG: {
			int					val;
			int					i;
			PROTOCOLDESCRIPTOR	**proto;
			int					proto_count;
			int					index;
			DBVARIANT			dbv;

			TranslateDialogDefault(hwndDlg);

			data = (struct OptDlgData*)mir_alloc(sizeof(struct OptDlgData));
			SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)data);

			DlgInInit = TRUE;
			SendDlgItemMessage(hwndDlg, IDC_OPTEDIT1, EM_LIMITTEXT, 1024, 0);

			SendMessage(GetDlgItem(hwndDlg, IDC_SMAXLENGTH), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_EMAXLENGTH), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SMAXLENGTH), UDM_SETRANGE32, (WPARAM)1, (LPARAM)1024);
			SendMessage(GetDlgItem(hwndDlg, IDC_EMAXLENGTH), EM_SETLIMITTEXT, (WPARAM)4, 0);

			CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);

			data->status_msg = (struct SingleStatusMsg *)mir_alloc(sizeof(struct SingleStatusMsg)*(proto_count+1));

			for (i=ID_STATUS_ONLINE; i<=ID_STATUS_OUTTOLUNCH; i++) {
				if (ProtoStatusMsgFlags & Proto_Status2Flag(i)) {
					index = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_INSERTSTRING, (WPARAM)-1, (LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, i, 0));

					if (index != CB_ERR && index != CB_ERRSPACE) {
						int		j;
						char	setting[80];

						SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_SETITEMDATA, (WPARAM)index, (LPARAM)i-ID_STATUS_ONLINE);

						val = DBGetContactSettingByte(NULL, "SimpleAway", (char *)StatusModeToDbSetting(i, "Flags"), STATUS_SHOW_DLG|STATUS_LAST_MSG);
						data->status_msg[0].flags[i-ID_STATUS_ONLINE] = val;
						if(DBGetContactSetting(NULL, "SRAway", StatusModeToDbSetting(i, "Default"), &dbv))
							dbv.pszVal = mir_strdup(GetDefaultMessage(i));
						lstrcpy(data->status_msg[0].msg[i-ID_STATUS_ONLINE], dbv.pszVal);
						DBFreeVariant(&dbv);

						for(j=0; j<proto_count; j++) {
							if (proto[j]->type != PROTOTYPE_PROTOCOL || !CallProtoService(proto[j]->szName, PS_GETCAPS, PFLAGNUM_3, 0) || !(CallProtoService(proto[j]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
								continue;

							_snprintf(setting, sizeof(setting), "%sFlags", proto[j]->szName);
							val = DBGetContactSettingByte(NULL, "SimpleAway", (char *)StatusModeToDbSetting(i, setting), STATUS_SHOW_DLG|STATUS_LAST_MSG);
							data->status_msg[j+1].flags[i-ID_STATUS_ONLINE] = val;
							_snprintf(setting, sizeof(setting), "%sDefault", proto[j]->szName);
							if(DBGetContactSetting(NULL, "SRAway", StatusModeToDbSetting(i, setting), &dbv))
								dbv.pszVal = mir_strdup(GetDefaultMessage(i));
							lstrcpy(data->status_msg[j+1].msg[i-ID_STATUS_ONLINE], dbv.pszVal);
							DBFreeVariant(&dbv);
						}
					}
				}
			}
			SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_SETCURSEL, (WPARAM)0, 0);

			data->proto_msg = (struct SingleProtoMsg *)mir_alloc(sizeof(struct SingleProtoMsg)*(proto_count+1));
			if (!data->proto_msg) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO1), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO2), FALSE);
				data->proto_ok = FALSE;
			}
			else {
				char setting[64];

				data->proto_ok = TRUE;
				data->proto_count = proto_count;

				index = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_ADDSTRING, 0, (LPARAM)Translate("Global"));
//				SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_SETITEMDATA, index, (LPARAM)0);
				if (index != CB_ERR && index != CB_ERRSPACE) {
					data->proto_msg[0].msg = NULL;
					data->proto_msg[0].flags = PROTO_POPUPDLG;
					data->proto_msg[0].max_length = 0;
					SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_SETITEMDATA, (WPARAM)index, (LPARAM)0);
				}

				for(i=0; i<proto_count; i++) {
					char protoLabel[MAXMODULELABELLENGTH+1];

					if (proto[i]->type != PROTOTYPE_PROTOCOL || !CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0) || !(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND)) {
						data->proto_msg[i+1].msg = NULL;
						continue;
					}

					CallProtoService(proto[i]->szName, PS_GETNAME, MAXMODULELABELLENGTH, (LPARAM)protoLabel);
					index = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_ADDSTRING, 0, (LPARAM)protoLabel);
//					SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_SETITEMDATA, index, (LPARAM)i+1);
					if (index != CB_ERR && index != CB_ERRSPACE) {
						_snprintf(setting, sizeof(setting), "Proto%sDefault", proto[i]->szName);
						if(!DBGetContactSetting(NULL, "SimpleAway", setting, &dbv)) {
							data->proto_msg[i+1].msg = mir_strdup(dbv.pszVal);
							DBFreeVariant(&dbv);
						}
						else
							data->proto_msg[i+1].msg = NULL;

						_snprintf(setting, sizeof(setting), "Proto%sFlags", proto[i]->szName);
						val = DBGetContactSettingByte(NULL, "SimpleAway", setting, PROTO_POPUPDLG);
						data->proto_msg[i+1].flags = val;
						_snprintf(setting, sizeof(setting), "Proto%sMaxLen", proto[i]->szName);
						val = DBGetContactSettingWord(NULL, "SimpleAway", setting, 1024);
						data->proto_msg[i+1].max_length = val;
						SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_SETITEMDATA, (WPARAM)index, (LPARAM)i+1);
					}
				}

				if (ProtoStatusMsgCount == 1) {
//					ShowWindow(GetDlgItem(hwndDlg, IDC_BOPTPROTO), SW_HIDE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTPROTO), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), FALSE);
					SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_SETCURSEL, (WPARAM)1, 0);
				}
				else
					SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_SETCURSEL, (WPARAM)0, 0);

				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CBOPTPROTO, CBN_SELCHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_CBOPTPROTO));
			}

			if (DBGetContactSettingByte(NULL, "SimpleAway", "PutDefInList", 0))
				CheckDlgButton(hwndDlg, IDC_COPTMSG2, BST_CHECKED);

			if (ServiceExists(MS_VARS_FORMATSTRING)) {
				HICON hIcon=NULL;
				char *szTipInfo=NULL;

				if (ServiceExists(MS_VARS_GETSKINITEM)) {
					hIcon = (HICON)CallService(MS_VARS_GETSKINITEM, 0, (LPARAM)VSI_HELPICON);
					szTipInfo = (char *)CallService(MS_VARS_GETSKINITEM, 0, (LPARAM)VSI_HELPTIPTEXT);
				}

				if (hIcon != NULL)
					SendMessage(GetDlgItem(hwndDlg, IDC_VARSHELP), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
				else
					SetDlgItemText(hwndDlg, IDC_VARSHELP, _T("V"));

				if (szTipInfo == NULL)
					SendMessage(GetDlgItem(hwndDlg, IDC_VARSHELP), BUTTONADDTOOLTIP, (WPARAM)Translate("Open String Formatting Help"), 0);
				else
					SendMessage(GetDlgItem(hwndDlg, IDC_VARSHELP), BUTTONADDTOOLTIP, (WPARAM)szTipInfo, 0);

				SendDlgItemMessage(hwndDlg, IDC_VARSHELP, BUTTONSETASFLATBTN, 0, 0);
//				variables_skin_helpbutton(hwndDlg, IDC_VARSHELP);
			}
			ShowWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), ServiceExists(MS_VARS_FORMATSTRING));

			OldDlgProc = (WNDPROC)SetWindowLong(GetDlgItem(hwndDlg, IDC_OPTEDIT1), GWL_WNDPROC, (LONG)OptEditBoxSubProc);

			DlgInInit = FALSE;
			return TRUE;
		}
		break;
		case WM_COMMAND:
			if ( ( (HIWORD(wParam) == BN_CLICKED) || /*(HIWORD(wParam) == EN_KILLFOCUS) ||*/ (HIWORD(wParam) == EN_CHANGE)
				|| ( (HIWORD(wParam) == CBN_SELCHANGE) && (LOWORD(wParam) != IDC_CBOPTPROTO) && (LOWORD(wParam) != IDC_CBOPTSTATUS) )
				) && (!DlgInInit) )
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			switch(LOWORD(wParam)) {
				case IDC_EMAXLENGTH:
					switch (HIWORD(wParam)) {
						case EN_KILLFOCUS: {
							BOOL	translated;
							int		val;
							int		i;

							val = GetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, &translated, FALSE);
							if (translated && val > 1024)
								SetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, 1024, FALSE);
							if (translated && val < 1)
								SetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, 1, FALSE);
							val = GetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, &translated, FALSE);

							i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETCURSEL, 0, 0), 0);
							
							data->proto_msg[i].max_length = val;
						}
						break;
					}
				break;
				case IDC_CBOPTPROTO:
					switch(HIWORD(wParam)) {
						case CBN_SELCHANGE:
						case CBN_SELENDOK: {
							PROTOCOLDESCRIPTOR	**proto;
							int					i, j, l, proto_count, k;
							int					status_modes=0, newindex=0;

							i = SendMessage((HWND)lParam, CB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);

							if (i==0) {
								EnableWindow(GetDlgItem(hwndDlg, IDC_EMAXLENGTH), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_SMAXLENGTH), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO2), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO3), FALSE);
								DlgInInit=TRUE;
								SetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, 1024, FALSE);
								DlgInInit=FALSE;
								CheckRadioButton(hwndDlg, IDC_ROPTPROTO1, IDC_ROPTPROTO3, IDC_ROPTPROTO3);
							}
							else {
								EnableWindow(GetDlgItem(hwndDlg, IDC_EMAXLENGTH), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_SMAXLENGTH), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO1), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO2), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO3), TRUE);
							
								DlgInInit=TRUE;
								SetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, data->proto_msg[i].max_length, FALSE);
								DlgInInit=FALSE;

								if (data->proto_msg[i].flags & PROTO_POPUPDLG)
									CheckRadioButton(hwndDlg, IDC_ROPTPROTO1, IDC_ROPTPROTO3, IDC_ROPTPROTO3);
								else if (data->proto_msg[i].flags & PROTO_NO_MSG)
									CheckRadioButton(hwndDlg, IDC_ROPTPROTO1, IDC_ROPTPROTO3, IDC_ROPTPROTO1);
								else if (data->proto_msg[i].flags & PROTO_THIS_MSG)
									CheckRadioButton(hwndDlg, IDC_ROPTPROTO1, IDC_ROPTPROTO3, IDC_ROPTPROTO2);
							}

							if (data->proto_msg[i].flags & PROTO_NO_MSG || data->proto_msg[i].flags & PROTO_THIS_MSG) {
								EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTSTATUS), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG1), FALSE);

								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG2), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG3), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG5), FALSE);

								if (data->proto_msg[i].flags & PROTO_NO_MSG) {
									EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
								}
								else {
									EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), TRUE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
								}

								EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG2), FALSE);
							}
							else {
								EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTSTATUS), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG1), TRUE);

								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG1), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG2), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG3), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG5), TRUE);

								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);

								EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG2), TRUE);
							}

							CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
							if (i) {
								k=i-1;
								status_modes = CallProtoService(proto[k]->szName, PS_GETCAPS, PFLAGNUM_3, 0);
							}
							else
								status_modes = ProtoStatusMsgFlags;

							j = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);
							SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_RESETCONTENT, (WPARAM)0, 0);

							for (l=ID_STATUS_ONLINE; l<=ID_STATUS_OUTTOLUNCH; l++) {
								int		index;

								if (status_modes & Proto_Status2Flag(l)) {
									index = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_INSERTSTRING, (WPARAM)-1, (LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, l, 0));

									if (index != CB_ERR && index != CB_ERRSPACE) {
										SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_SETITEMDATA, (WPARAM)index, (LPARAM)l-ID_STATUS_ONLINE);
										if (j == l-ID_STATUS_ONLINE)
											newindex=index;
									}
								}
							}

							if (!newindex) {
								SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_SETCURSEL, (WPARAM)0, 0);
								j = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);
							}
							else
								SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_SETCURSEL, (WPARAM)newindex, 0);

							if (data->status_msg[i].flags[j] & STATUS_SHOW_DLG)
								CheckDlgButton(hwndDlg, IDC_COPTMSG1, BST_CHECKED);
							else
								CheckDlgButton(hwndDlg, IDC_COPTMSG1, BST_UNCHECKED);

							if (data->proto_msg[i].flags & PROTO_THIS_MSG) {
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);
								if (data->proto_msg[i].msg)
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->proto_msg[i].msg);
								else
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
							}
							else {
								if (data->status_msg[i].flags[j] & STATUS_EMPTY_MSG) {
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
									CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG1);
								}
								else if (data->status_msg[i].flags[j] & STATUS_DEFAULT_MSG) {
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, GetDefaultMessage(j+ID_STATUS_ONLINE));
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
									CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG2);
								}
								else if (data->status_msg[i].flags[j] & STATUS_LAST_MSG) {
									char	setting[80];
									DBVARIANT	dbv,dbv2;

									if (i)
										_snprintf(setting, sizeof(setting), "Last%sMsg", proto[k]->szName);
									else
										_snprintf(setting, sizeof(setting), "LastMsg");

									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
									if (!DBGetContactSetting(NULL, "SimpleAway", setting, &dbv)) {
										if (dbv.pszVal) {
											if (!DBGetContactSetting(NULL, "SimpleAway", dbv.pszVal, &dbv2) && strlen(dbv.pszVal)) {
												if ((dbv2.pszVal) && (strlen(dbv2.pszVal)))
													SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv2.pszVal);

												DBFreeVariant(&dbv2);
											}
										}
										DBFreeVariant(&dbv);
									}
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
									CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG3);
								}
								else if (data->status_msg[i].flags[j] & STATUS_THIS_MSG) {
									if (data->proto_msg[i].flags & PROTO_NO_MSG) {
										EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
										EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
									}
									else {
										EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
										EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
									}
									CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->status_msg[i].msg[j]);
								}
								else if (data->status_msg[i].flags[j] & STATUS_LAST_STATUS_MSG) {
									char	setting[80];
									DBVARIANT	dbv;

									if (i)
										_snprintf(setting, sizeof(setting), "%sMsg", proto[k]->szName);
									else
										_snprintf(setting, sizeof(setting), "Msg");

									if (!DBGetContactSetting(NULL, "SRAway", StatusModeToDbSetting(j+ID_STATUS_ONLINE, setting), &dbv)) {
										SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv.pszVal);
										DBFreeVariant(&dbv);
									}
									else
										SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");

									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
									CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG5);
								}
							}
						}
						break;
					}
				break;
				case IDC_ROPTPROTO1:
				case IDC_ROPTPROTO2:
				case IDC_ROPTPROTO3:
					switch(HIWORD(wParam)) {
						case BN_CLICKED: {
							int	i, j;

							i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETCURSEL, 0, 0), 0);
							j = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);

							data->proto_msg[i].flags = 0;

							if (LOWORD(wParam) == IDC_ROPTPROTO1) {
								data->proto_msg[i].flags |= PROTO_NO_MSG;
								EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTSTATUS), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG1), FALSE);

								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG2), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG3), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG5), FALSE);

								EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG2), FALSE);
							}
							else if (LOWORD(wParam) == IDC_ROPTPROTO2) {
								data->proto_msg[i].flags |= PROTO_THIS_MSG;
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
								if (data->proto_msg[i].msg)
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->proto_msg[i].msg);
								else
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
								EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTSTATUS), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG1), FALSE);

								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG2), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG3), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG5), FALSE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);

								EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG2), FALSE);
							}
							else if (LOWORD(wParam) == IDC_ROPTPROTO3) {
								data->proto_msg[i].flags |= PROTO_POPUPDLG;
								EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTSTATUS), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG1), TRUE);

								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG1), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG2), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG3), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG5), TRUE);

								EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG2), TRUE);
							}

							if (LOWORD(wParam) != IDC_ROPTPROTO2) {
								if (data->status_msg[i].flags[j] & STATUS_EMPTY_MSG) {
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
									CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG1);
								}
								else if (data->status_msg[i].flags[j] & STATUS_DEFAULT_MSG) {
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, GetDefaultMessage(j+ID_STATUS_ONLINE));
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
									CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG2);
								}
								else if (data->status_msg[i].flags[j] & STATUS_LAST_MSG) {
									char	setting[80];
									DBVARIANT	dbv,dbv2;

									if (i) {
										PROTOCOLDESCRIPTOR	**proto;
										int					proto_count;

										CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
										_snprintf(setting, sizeof(setting), "Last%sMsg", proto[i-1]->szName);
									}
									else
										_snprintf(setting, sizeof(setting), "LastMsg");

									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
									if (!DBGetContactSetting(NULL, "SimpleAway", setting, &dbv)) {
										if (dbv.pszVal) {
											if (!DBGetContactSetting(NULL, "SimpleAway", dbv.pszVal, &dbv2) && strlen(dbv.pszVal)) {
												if ((dbv2.pszVal) && (strlen(dbv2.pszVal)))
													SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv2.pszVal);
												DBFreeVariant(&dbv2);
											}
										}
										DBFreeVariant(&dbv);
									}
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
									CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG3);
								}
								else if (data->status_msg[i].flags[j] & STATUS_THIS_MSG) {
									if (LOWORD(wParam) == IDC_ROPTPROTO1) {
										EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
										EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
									}
									else {
										EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
										EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
									}
									CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->status_msg[i].msg[j]);
								}
								else if (data->status_msg[i].flags[j] & STATUS_LAST_STATUS_MSG) {
									char	setting[80];
									DBVARIANT	dbv;

									if (i) {
										PROTOCOLDESCRIPTOR	**proto;
										int					proto_count;

										CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
										_snprintf(setting, sizeof(setting), "%sMsg", proto[i-1]->szName);
									}
									else
										_snprintf(setting, sizeof(setting), "Msg");

									if (!DBGetContactSetting(NULL, "SRAway", StatusModeToDbSetting(j+ID_STATUS_ONLINE, setting), &dbv)) {
										SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv.pszVal);
										DBFreeVariant(&dbv);
									}
									else
										SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");

									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
									CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG5);
								}
							}
						}
						break;
					}
				break;
				case IDC_CBOPTSTATUS:
					switch(HIWORD(wParam)) {
						case CBN_SELCHANGE:
						case CBN_SELENDOK: {
							int	i, j;

							i = SendMessage((HWND)lParam, CB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);
							j = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETCURSEL, 0, 0), 0);

							if (data->status_msg[j].flags[i] & STATUS_SHOW_DLG)
								CheckDlgButton(hwndDlg, IDC_COPTMSG1, BST_CHECKED);
							else
								CheckDlgButton(hwndDlg, IDC_COPTMSG1, BST_UNCHECKED);

							if (data->status_msg[j].flags[i] & STATUS_EMPTY_MSG) {
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG1);
							}
							else if (data->status_msg[j].flags[i] & STATUS_DEFAULT_MSG) {
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, GetDefaultMessage(i+ID_STATUS_ONLINE));
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG2);
							}
							else if (data->status_msg[j].flags[i] & STATUS_LAST_MSG) {
								char	setting[80];
								DBVARIANT	dbv,dbv2;

								if (j) {
									PROTOCOLDESCRIPTOR	**proto;
									int					proto_count;

									CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
									_snprintf(setting, sizeof(setting), "Last%sMsg", proto[j-1]->szName);
								}
								else
									_snprintf(setting, sizeof(setting), "LastMsg");

								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
								if (!DBGetContactSetting(NULL, "SimpleAway", setting, &dbv)) {
									if (dbv.pszVal) {
										if (!DBGetContactSetting(NULL, "SimpleAway", dbv.pszVal, &dbv2) && strlen(dbv.pszVal)) {
											if ((dbv2.pszVal) && (strlen(dbv2.pszVal)))
												SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv2.pszVal);
											DBFreeVariant(&dbv2);
										}
									}
									DBFreeVariant(&dbv);
								}
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG3);
							}
							else if (data->status_msg[j].flags[i] & STATUS_THIS_MSG) {
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->status_msg[j].msg[i]);
							}
							else if (data->status_msg[j].flags[i] & STATUS_LAST_STATUS_MSG) {
								char	setting[80];
								DBVARIANT	dbv;

								if (j) {
									PROTOCOLDESCRIPTOR	**proto;
									int					proto_count;

									CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
									_snprintf(setting, sizeof(setting), "%sMsg", proto[j-1]->szName);
								}
								else
									_snprintf(setting, sizeof(setting), "Msg");

								if (!DBGetContactSetting(NULL, "SRAway", StatusModeToDbSetting(i+ID_STATUS_ONLINE, setting), &dbv)) {
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv.pszVal);
									DBFreeVariant(&dbv);
								}
								else
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");

								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG5);
							}
						}
						break;
					}
				break;
				case IDC_COPTMSG1:
					switch(HIWORD(wParam)) {
						case BN_CLICKED: {
							int	i, j;

							i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);
							j = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETCURSEL, 0, 0), 0);
							if (IsDlgButtonChecked(hwndDlg, IDC_COPTMSG1) == BST_CHECKED)
								data->status_msg[j].flags[i] |= STATUS_SHOW_DLG;
							else
								data->status_msg[j].flags[i] &= ~STATUS_SHOW_DLG;
						}
						break;
					}
				break;
				case IDC_ROPTMSG1:
				case IDC_ROPTMSG2:
				case IDC_ROPTMSG3:
				case IDC_ROPTMSG4:
				case IDC_ROPTMSG5:
					switch(HIWORD(wParam)) {
						case BN_CLICKED: {
							int	i, j;

							i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);
							j = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETCURSEL, 0, 0), 0);

							if (LOWORD(wParam) == IDC_ROPTMSG4 && data->proto_msg[j].flags & PROTO_THIS_MSG)
								break;

							data->status_msg[j].flags[i] = 0;

							if (IsDlgButtonChecked(hwndDlg, IDC_COPTMSG1) == BST_CHECKED)
								data->status_msg[j].flags[i] |= STATUS_SHOW_DLG;

							if (LOWORD(wParam) == IDC_ROPTMSG1) {
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
								data->status_msg[j].flags[i] |= STATUS_EMPTY_MSG;
							}
							else if (LOWORD(wParam) == IDC_ROPTMSG2) {
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, GetDefaultMessage(i+ID_STATUS_ONLINE));
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
								data->status_msg[j].flags[i] |= STATUS_DEFAULT_MSG;
							}
							else if (LOWORD(wParam) == IDC_ROPTMSG3) {
								char	setting[80];
								DBVARIANT	dbv,dbv2;

								if (j) {
									PROTOCOLDESCRIPTOR	**proto;
									int					proto_count;

									CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
									_snprintf(setting, sizeof(setting), "Last%sMsg", proto[j-1]->szName);
								}
								else
									_snprintf(setting, sizeof(setting), "LastMsg");

								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
								if (!DBGetContactSetting(NULL, "SimpleAway", setting, &dbv)) {
									if (dbv.pszVal) {
										if (!DBGetContactSetting(NULL, "SimpleAway", dbv.pszVal, &dbv2) && strlen(dbv.pszVal)) {
											if ((dbv2.pszVal) && (strlen(dbv2.pszVal)))
												SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv2.pszVal);
											DBFreeVariant(&dbv2);
										}
									}
									DBFreeVariant(&dbv);
								}
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
								data->status_msg[j].flags[i] |= STATUS_LAST_MSG;
							}
							else if (LOWORD(wParam) == IDC_ROPTMSG4) {
								data->status_msg[j].flags[i] |= STATUS_THIS_MSG;
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->status_msg[j].msg[i]);
							}
							else if (LOWORD(wParam) == IDC_ROPTMSG5) {
								char	setting[80];
								DBVARIANT	dbv;

								if (j) {
									PROTOCOLDESCRIPTOR	**proto;
									int					proto_count;

									CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
									_snprintf(setting, sizeof(setting), "%sMsg", proto[j-1]->szName);
								}
								else
									_snprintf(setting, sizeof(setting), "Msg");

								if (!DBGetContactSetting(NULL, "SRAway", StatusModeToDbSetting(i+ID_STATUS_ONLINE, setting), &dbv)) {
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv.pszVal);
									DBFreeVariant(&dbv);
								}
								else
									SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");

								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
								data->status_msg[j].flags[i] |= STATUS_LAST_STATUS_MSG;
							}
						}
						break;
					}
				break;
				case IDC_OPTEDIT1: {
					int	i, j;

					i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);
					j = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETCURSEL, 0, 0), 0);

					if(HIWORD(wParam) == EN_KILLFOCUS) {
						char msg[1024];

						if (data->proto_msg[j].flags & PROTO_THIS_MSG) {
							int		len;

							len = GetDlgItemText(hwndDlg, IDC_OPTEDIT1, msg, sizeof(msg));
							if (len > 0) {	
								if (data->proto_msg[j].msg == NULL)
									data->proto_msg[j].msg = mir_strdup(msg);
								else {
									mir_free(data->proto_msg[j].msg);
									data->proto_msg[j].msg = mir_strdup(msg);
								}
							}
							else {
								if (data->proto_msg[j].msg != NULL) {
									mir_free(data->proto_msg[j].msg);
									data->proto_msg[j].msg = NULL;
								}
							}
						}
						else {
							GetDlgItemText(hwndDlg, IDC_OPTEDIT1, msg, sizeof(msg));
							lstrcpy(data->status_msg[j].msg[i], msg);
						}
					}
				}
				break;
				case IDC_VARSHELP:
					variables_showhelp(hwndDlg, IDC_OPTEDIT1, VHF_FULLDLG|VHF_SETLASTSUBJECT, NULL, NULL);
					break;
				case IDC_BOPTPROTO: {
					PROTOCOLDESCRIPTOR	**proto;
					int					proto_count, i, j, k;

					j = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETCURSEL, 0, 0), 0);

					if (j) {
						for (i=ID_STATUS_ONLINE; i<=ID_STATUS_OUTTOLUNCH; i++) {
							if (ProtoStatusMsgFlags & Proto_Status2Flag(i)) {
								data->status_msg[0].flags[i-ID_STATUS_ONLINE] = data->status_msg[j].flags[i-ID_STATUS_ONLINE];
								if (data->status_msg[j].flags[i-ID_STATUS_ONLINE] & STATUS_THIS_MSG)
									lstrcpy(data->status_msg[0].msg[i-ID_STATUS_ONLINE], data->status_msg[j].msg[i-ID_STATUS_ONLINE]);
							}
						}
					}

					CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
					for(k=0; k<proto_count; k++) {
						if (proto[k]->type != PROTOTYPE_PROTOCOL || !CallProtoService(proto[k]->szName, PS_GETCAPS, PFLAGNUM_3, 0) || !(CallProtoService(proto[k]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
							continue;

						if (k != j-1) {
							data->proto_msg[k+1].flags = data->proto_msg[j].flags;
							if (j)
								data->proto_msg[k+1].max_length = data->proto_msg[j].max_length;

							if (data->proto_msg[j].flags & PROTO_THIS_MSG) {
								int	 len;
								len = lstrlen(data->proto_msg[j].msg);
								if (len > 0) {	
									if (data->proto_msg[k+1].msg == NULL)
										data->proto_msg[k+1].msg = mir_strdup(data->proto_msg[j].msg);
									else {
										mir_free(data->proto_msg[k+1].msg);
										data->proto_msg[k+1].msg = mir_strdup(data->proto_msg[j].msg);
									}
								}
								else {
									if (data->proto_msg[k+1].msg != NULL) {
										mir_free(data->proto_msg[k+1].msg);
										data->proto_msg[k+1].msg = NULL;
									}
								}
							}
							else if (data->proto_msg[j].flags & PROTO_POPUPDLG) {
								for (i=ID_STATUS_ONLINE; i<=ID_STATUS_OUTTOLUNCH; i++) {
									if (CallProtoService(proto[k]->szName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(i)) {
										data->status_msg[k+1].flags[i-ID_STATUS_ONLINE] = data->status_msg[j].flags[i-ID_STATUS_ONLINE];
										if (data->status_msg[j].flags[i-ID_STATUS_ONLINE] & STATUS_THIS_MSG)
											lstrcpy(data->status_msg[k+1].msg[i-ID_STATUS_ONLINE], data->status_msg[j].msg[i-ID_STATUS_ONLINE]);
									}
								}
							}
						}
					}
				} //case IDC_BOPTPROTO:
				break;
				case IDC_BOPTSTATUS: {
					PROTOCOLDESCRIPTOR	**proto;
					int					proto_count, status_modes, i, j, k;

					i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);
					j = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETCURSEL, 0, 0), 0);

					CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
					if (j)
						status_modes = CallProtoService(proto[j-1]->szName, PS_GETCAPS, PFLAGNUM_3, 0);
					else
						status_modes = ProtoStatusMsgFlags;

					for (k=ID_STATUS_ONLINE; k<=ID_STATUS_OUTTOLUNCH; k++) {
						if (k-ID_STATUS_ONLINE != i && status_modes & Proto_Status2Flag(k)) {
							data->status_msg[j].flags[k-ID_STATUS_ONLINE] = data->status_msg[j].flags[i];
							if (data->status_msg[j].flags[i] & STATUS_THIS_MSG)
								lstrcpy(data->status_msg[j].msg[k-ID_STATUS_ONLINE], data->status_msg[j].msg[i]);
						}
					}
				} //case IDC_BOPTSTATUS:
				break;
			}
		break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch(((LPNMHDR)lParam)->code) {
						case PSN_APPLY: {
							int		i;

							for (i=ID_STATUS_ONLINE; i<=ID_STATUS_OUTTOLUNCH; i++) {
								if (ProtoStatusMsgFlags & Proto_Status2Flag(i)) {
									PROTOCOLDESCRIPTOR	**proto;
									int					proto_count, j, status_modes;
									char				setting[80];

									DBWriteContactSettingString(NULL, "SRAway", StatusModeToDbSetting(i, "Default"), data->status_msg[0].msg[i-ID_STATUS_ONLINE]);
									DBWriteContactSettingByte(NULL, "SimpleAway", StatusModeToDbSetting(i, "Flags"), (BYTE)data->status_msg[0].flags[i-ID_STATUS_ONLINE]);

									CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
									for(j=0; j<proto_count; j++) {
										if (proto[j]->type != PROTOTYPE_PROTOCOL)
											continue;

										if (!(CallProtoService(proto[j]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
											continue;

										status_modes = CallProtoService(proto[j]->szName, PS_GETCAPS, PFLAGNUM_3, 0);
										if (!status_modes)
											continue;

										if (status_modes & Proto_Status2Flag(i)) {
											_snprintf(setting, sizeof(setting), "%sDefault", proto[j]->szName);
											DBWriteContactSettingString(NULL, "SRAway", StatusModeToDbSetting(i, setting), data->status_msg[j+1].msg[i-ID_STATUS_ONLINE]);
											_snprintf(setting, sizeof(setting), "%sFlags", proto[j]->szName);
											DBWriteContactSettingByte(NULL, "SimpleAway", StatusModeToDbSetting(i, setting), (BYTE)data->status_msg[j+1].flags[i-ID_STATUS_ONLINE]);
										}
									}
								}
							}

							if (IsDlgButtonChecked(hwndDlg, IDC_COPTMSG2) == BST_CHECKED)
								DBWriteContactSettingByte(NULL, "SimpleAway", "PutDefInList", (BYTE)1);
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "PutDefInList", (BYTE)0);

							if (data->proto_ok) {
								PROTOCOLDESCRIPTOR	**proto;
								int					proto_count;
								char				setting[64];

								CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
								for(i=0; i<proto_count; i++) {
									if (proto[i]->type != PROTOTYPE_PROTOCOL)
										continue;

									if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
										continue;

									if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
										continue;

									_snprintf(setting, sizeof(setting), "Proto%sDefault", proto[i]->szName);
									if (data->proto_msg[i+1].msg && (data->proto_msg[i+1].flags & PROTO_THIS_MSG))
										DBWriteContactSettingString(NULL, "SimpleAway", setting, data->proto_msg[i+1].msg);
									else
										DBDeleteContactSetting(NULL, "SimpleAway", setting);

									_snprintf(setting, sizeof(setting), "Proto%sMaxLen", proto[i]->szName);
									DBWriteContactSettingWord(NULL, "SimpleAway", setting, (WORD)data->proto_msg[i+1].max_length);

									_snprintf(setting, sizeof(setting), "Proto%sFlags", proto[i]->szName);
									DBWriteContactSettingByte(NULL, "SimpleAway", setting, (BYTE)data->proto_msg[i+1].flags);
								}
							}

							return TRUE;
						}
						break;
					}
				break;
			}
		break;
		case WM_DESTROY:
			SetWindowLong(GetDlgItem(hwndDlg, IDC_OPTEDIT1), GWL_WNDPROC, (LONG)OldDlgProc);
			if (data->proto_ok) {
				int i;

				for (i=0; i<(data->proto_count+1); i++) {
					if (data->proto_msg[i].msg) //they want to be free, do they?
						mir_free(data->proto_msg[i].msg);
				}
				mir_free(data->proto_msg);
			}
			mir_free(data->status_msg);
			mir_free(data);
		break;

	}
	return FALSE;
}

static BOOL	DlgInInit2=FALSE;

INT_PTR CALLBACK DlgAdvancedOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG: {
			int					val;
			int					i_btnhide, i_btnlist, i_btndown, i_btndownflat;
			int					i, j;
			char				buff[16];
			DBVARIANT			dbv;

			TranslateDialogDefault(hwndDlg);

			DlgInInit2 = TRUE;
			i_btnhide = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_ADDSTRING, 0, (LPARAM)Translate("Hide"));
			if (i_btnhide != CB_ERR && i_btnhide != CB_ERRSPACE)
				SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_SETITEMDATA, (WPARAM)i_btnhide, (LPARAM)0);
			i_btndown = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_ADDSTRING, 0, (LPARAM)Translate("Show next to cancel button"));
			if (i_btndown != CB_ERR && i_btndown != CB_ERRSPACE)
				SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_SETITEMDATA, (WPARAM)i_btndown, (LPARAM)DLG_SHOW_BUTTONS);
			i_btndownflat = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_ADDSTRING, 0, (LPARAM)Translate("Flat, next to cancel button"));
			if (i_btndownflat != CB_ERR && i_btndownflat != CB_ERRSPACE)
				SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_SETITEMDATA, (WPARAM)i_btndownflat, (LPARAM)DLG_SHOW_BUTTONS_FLAT);
			i_btnlist = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_ADDSTRING, 0, (LPARAM)Translate("Show in message list"));
			if (i_btnlist != CB_ERR && i_btnlist != CB_ERRSPACE)
				SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_SETITEMDATA, (WPARAM)i_btnlist, (LPARAM)DLG_SHOW_BUTTONS_INLIST);

			SendMessage(GetDlgItem(hwndDlg, IDC_SMAXHIST), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_EMAXHIST), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SMAXHIST), UDM_SETRANGE32, (WPARAM)0, (LPARAM)25);
			SendMessage(GetDlgItem(hwndDlg, IDC_EMAXHIST), EM_SETLIMITTEXT, (WPARAM)2, 0);
			val = DBGetContactSettingByte(NULL, "SimpleAway", "MaxHist", 10);
			SetDlgItemInt(hwndDlg, IDC_EMAXHIST, val, FALSE);

			if (!val)
				EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS2), FALSE);

			//looking for history MSGS
			EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTHIST), FALSE);
			j = DBGetContactSettingWord(NULL, "SimpleAway", "LMMsg", 1);

			for(i=1; i<=val; i++) {
				if (j<1)
					j = val;
				_snprintf(buff, sizeof(buff), "SMsg%d", j);
				j--;
				if (!DBGetContactSetting(NULL, "SimpleAway", buff, &dbv)) {//0 - no error
					if (dbv.pszVal) {
						if (!strlen(dbv.pszVal)) {
							DBFreeVariant(&dbv);
							continue;
						}
						EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTHIST), TRUE);
						break;
					}
					DBFreeVariant(&dbv);
				}
			}

			SendMessage(GetDlgItem(hwndDlg, IDC_STIMEOUT), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_ETIMEOUT), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_STIMEOUT), UDM_SETRANGE32, (WPARAM)0, (LPARAM)60);
			SendMessage(GetDlgItem(hwndDlg, IDC_ETIMEOUT), EM_SETLIMITTEXT, (WPARAM)2, 0);
			SetDlgItemInt(hwndDlg, IDC_ETIMEOUT, DBGetContactSettingByte(NULL, "SimpleAway", "DlgTime", 5), FALSE);

			SendMessage(GetDlgItem(hwndDlg, IDC_SSECWINAMP), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_ESECWINAMP), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SSECWINAMP), UDM_SETRANGE32, (WPARAM)1, (LPARAM)240);
			SendMessage(GetDlgItem(hwndDlg, IDC_ESECWINAMP), EM_SETLIMITTEXT, (WPARAM)3, 0);
			SetDlgItemInt(hwndDlg, IDC_ESECWINAMP, DBGetContactSettingByte(NULL, "SimpleAway", "AmpCheck", 15), FALSE);
			if (!DBGetContactSettingByte(NULL, "SimpleAway", "AmpCheckOn", 1)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_ESECWINAMP), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SSECWINAMP), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CLEAVEWINAMP), FALSE);
				CheckDlgButton(hwndDlg, IDC_CCHECKWINAMP, BST_UNCHECKED);
			}
			else
				CheckDlgButton(hwndDlg, IDC_CCHECKWINAMP, BST_CHECKED);

			if (DBGetContactSettingByte(NULL, "SimpleAway", "AmpLeaveTitle", 1))
				CheckDlgButton(hwndDlg, IDC_CLEAVEWINAMP, BST_CHECKED);

			SendMessage(GetDlgItem(hwndDlg, IDC_SMINRANDMSG), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_EMINRANDMSG), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SMINRANDMSG), UDM_SETRANGE32, (WPARAM)1, (LPARAM)60);
			SendMessage(GetDlgItem(hwndDlg, IDC_EMINRANDMSG), EM_SETLIMITTEXT, (WPARAM)2, 0);
			SetDlgItemInt(hwndDlg, IDC_EMINRANDMSG, DBGetContactSettingByte(NULL, "SimpleAway", "RandMsgChange", 3), FALSE);
			if (!DBGetContactSettingByte(NULL, "SimpleAway", "RandMsgChangeOn", 1)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_EMINRANDMSG), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SMINRANDMSG), FALSE);
				CheckDlgButton(hwndDlg, IDC_CSETRANDMSG, BST_UNCHECKED);
			}
			else
				CheckDlgButton(hwndDlg, IDC_CSETRANDMSG, BST_CHECKED);

			if (DBGetContactSettingByte(NULL, "SimpleAway", "RemoveCR", 1))
				CheckDlgButton(hwndDlg, IDC_CREMOVECR, BST_CHECKED);

			if (DBGetContactSettingByte(NULL, "SimpleAway", "ShowCopy", 1))
				CheckDlgButton(hwndDlg, IDC_CSHOWCOPY, BST_CHECKED);

			if (!DBGetContactSettingByte(NULL, "SimpleAway", "WinCentered", 1))
				CheckDlgButton(hwndDlg, IDC_CRPOSWND, BST_CHECKED);

			if (DBGetContactSettingByte(NULL, "SimpleAway", "ShowStatusMenuItem", 1))
				CheckDlgButton(hwndDlg, IDC_CSHOWSMSG, BST_CHECKED);

			if (!ServiceExists(MS_CLIST_ADDSTATUSMENUITEM)) {
				char text[150];
//				_snprintf(text, sizeof(text), Translate("Show 'Status Message...' item in status menu"));
				lstrcpy(text, Translate("Show 'Status Message...' item in status menu"));
				if (!ServiceExists(MS_SS_GETPROFILECOUNT)) {
					lstrcat(text, " **");
					SetDlgItemText(hwndDlg, IDC_CSHOWSMSG, text);
					lstrcpy(text,"*");
					lstrcat(text, Translate("* Your contact list plugin doesn't support this feature. Try another one."));
					SetDlgItemText(hwndDlg, IDC_NOTE2, text);
				}
				else {
					RECT rc;

					lstrcat(text, " *");
					SetDlgItemText(hwndDlg, IDC_CSHOWSMSG, text);
					GetWindowRect(GetDlgItem(hwndDlg, IDC_NOTE2), &rc);
					ScreenToClient(hwndDlg, (POINT *)&rc);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_NOTE2), NULL, rc.left, rc.top-16, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}

				EnableWindow(GetDlgItem(hwndDlg, IDC_CSHOWSMSG), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_NOTE2), SW_SHOW);
			}

			val = DBGetContactSettingByte(NULL, "SimpleAway", "AutoClose", 1);
			if (!val) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_ETIMEOUT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STIMEOUT), FALSE);
			}
			else
				SendMessage(GetDlgItem(hwndDlg, IDC_CCLOSEWND), BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

			val = DBGetContactSettingByte(NULL, "SimpleAway", "DlgFlags", DLG_SHOW_STATUS|DLG_SHOW_STATUS_ICONS|DLG_SHOW_LIST_ICONS|DLG_SHOW_BUTTONS);
			if (val & DLG_SHOW_STATUS)
				CheckDlgButton(hwndDlg, IDC_CSTATUSLIST, BST_CHECKED);
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CPROFILES), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS1), FALSE);
			}
				
			if (val & DLG_SHOW_STATUS_ICONS)
				CheckDlgButton(hwndDlg, IDC_CICONS1, BST_CHECKED);
			if (val & DLG_SHOW_LIST_ICONS)
				CheckDlgButton(hwndDlg, IDC_CICONS2, BST_CHECKED);
			if (val & DLG_SHOW_BUTTONS)
				SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_SETCURSEL, (WPARAM)i_btndown, (LPARAM)0);
			else if (val & DLG_SHOW_BUTTONS_FLAT)
				SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_SETCURSEL, (WPARAM)i_btndownflat, (LPARAM)0);
			else if (val & DLG_SHOW_BUTTONS_INLIST)
				SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_SETCURSEL, (WPARAM)i_btnlist, (LPARAM)0);
			else
				SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_SETCURSEL, (WPARAM)i_btnhide, (LPARAM)0);
			if (val & DLG_SHOW_STATUS_PROFILES)
				CheckDlgButton(hwndDlg, IDC_CPROFILES, BST_CHECKED);

			if (!ServiceExists(MS_SS_GETPROFILECOUNT)) {
				char text[100];
//				_snprintf(text, sizeof(text), Translate("Show status profiles in status list"));
				lstrcpy(text, Translate("Show status profiles in status list"));
				lstrcat(text, " *");
				SetDlgItemText(hwndDlg, IDC_CPROFILES, text);
				if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CPROFILES)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_CPROFILES), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_NOTE1), SW_SHOW);
			}

			if (!DBGetContactSettingWord(NULL, "SimpleAway", "DefMsgCount", 0))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTDEF), FALSE);

			DlgInInit2 = FALSE;
			return TRUE;
		}
		break;
		case WM_COMMAND:
//			if ( ( (HIWORD(wParam) == BN_CLICKED) || /*(HIWORD(wParam) == EN_KILLFOCUS) ||*/ (HIWORD(wParam) == EN_CHANGE)
			if ( ( ( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) != IDC_BOPTHIST) && (LOWORD(wParam) != IDC_BOPTDEF) )
				|| /*(HIWORD(wParam) == EN_KILLFOCUS) ||*/ (HIWORD(wParam) == EN_CHANGE)
				|| ( (HIWORD(wParam) == CBN_SELCHANGE) && (LOWORD(wParam) != IDC_CBOPTPROTO) && (LOWORD(wParam) != IDC_CBOPTSTATUS) )
				) && (!DlgInInit2) )
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			switch(LOWORD(wParam)) {
				case IDC_CCLOSEWND:
					switch (HIWORD(wParam)) {
						case BN_CLICKED:
							if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_UNCHECKED) {
								if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ETIMEOUT))) {
									EnableWindow(GetDlgItem(hwndDlg, IDC_ETIMEOUT), FALSE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_STIMEOUT), FALSE);
								}
							}
							else {
								if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ETIMEOUT))) {
									EnableWindow(GetDlgItem(hwndDlg, IDC_ETIMEOUT), TRUE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_STIMEOUT), TRUE);
								}
							}
							break;
					}
				break;
				case IDC_EMAXHIST:
					switch (HIWORD(wParam)) {
						case EN_CHANGE: {
							BOOL	translated;
							int		val;

							val = GetDlgItemInt(hwndDlg, IDC_EMAXHIST, &translated, FALSE);
							if (translated && !val) {
								if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CICONS2)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS2), FALSE);
							}
							else if (translated && val > 0) {
								if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CICONS2)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS2), TRUE);
							}
						}
						break;
						case EN_KILLFOCUS: {
							BOOL	translated;
							int		val;

							val = GetDlgItemInt(hwndDlg, IDC_EMAXHIST, &translated, FALSE);
							if (translated && val > 25)
								SetDlgItemInt(hwndDlg, IDC_EMAXHIST, 25, FALSE);
						}
						break;
					}
				break;
				case IDC_ETIMEOUT:
					switch (HIWORD(wParam)) {
						case EN_KILLFOCUS: {
							BOOL	translated;
							int		val;

							val = GetDlgItemInt(hwndDlg, IDC_ETIMEOUT, &translated, FALSE);
							if (translated && val > 60)
								SetDlgItemInt(hwndDlg, IDC_ETIMEOUT, 60, FALSE);
						}
						break;
					}
				break;
				case IDC_ESECWINAMP:
					switch (HIWORD(wParam)) {
						case EN_KILLFOCUS: {
							BOOL	translated;
							int		val;

							val = GetDlgItemInt(hwndDlg, IDC_ESECWINAMP, &translated, FALSE);
							if (translated && val > 240)
								SetDlgItemInt(hwndDlg, IDC_ESECWINAMP, 240, FALSE);
							if (translated && val < 1)
								SetDlgItemInt(hwndDlg, IDC_ESECWINAMP, 1, FALSE);
						}
						break;
					}
				break;
				case IDC_EMINRANDMSG:
					switch (HIWORD(wParam)) {
						case EN_KILLFOCUS: {
							BOOL	translated;
							int		val;

							val = GetDlgItemInt(hwndDlg, IDC_EMINRANDMSG, &translated, FALSE);
							if (translated && val > 60)
								SetDlgItemInt(hwndDlg, IDC_EMINRANDMSG, 60, FALSE);
							if (translated && val < 1)
								SetDlgItemInt(hwndDlg, IDC_EMINRANDMSG, 1, FALSE);
						}
						break;
					}
				break;
				case IDC_CSTATUSLIST:
					switch (HIWORD(wParam)) {
						case BN_CLICKED:
							if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_UNCHECKED) {
								if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CICONS1)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS1), FALSE);
								if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CPROFILES)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_CPROFILES), FALSE);
							}
							else {
								if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CICONS1)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS1), TRUE);
								if (ServiceExists(MS_SS_GETPROFILECOUNT) && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CPROFILES)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_CPROFILES), TRUE);
							}
							break;
					}
				break;
				case IDC_CCHECKWINAMP:
					switch (HIWORD(wParam)) {
						case BN_CLICKED:
							if (IsDlgButtonChecked(hwndDlg, IDC_CCHECKWINAMP) == BST_CHECKED) {
								if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ESECWINAMP))) {
									EnableWindow(GetDlgItem(hwndDlg, IDC_ESECWINAMP), TRUE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_SSECWINAMP), TRUE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_CLEAVEWINAMP), TRUE);
								}
							}
							else if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ESECWINAMP))) {
								EnableWindow(GetDlgItem(hwndDlg, IDC_ESECWINAMP), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_SSECWINAMP), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_CLEAVEWINAMP), FALSE);
							}
							break;
					}
				break;
				case IDC_CSETRANDMSG:
					switch (HIWORD(wParam)) {
						case BN_CLICKED:
							if (IsDlgButtonChecked(hwndDlg, IDC_CSETRANDMSG) == BST_CHECKED) {
								if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EMINRANDMSG))) {
									EnableWindow(GetDlgItem(hwndDlg, IDC_EMINRANDMSG), TRUE);
									EnableWindow(GetDlgItem(hwndDlg, IDC_SMINRANDMSG), TRUE);
								}
							}
							else if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EMINRANDMSG))) {
								EnableWindow(GetDlgItem(hwndDlg, IDC_EMINRANDMSG), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_SMINRANDMSG), FALSE);
							}
						break;
					}
				break;
				case IDC_BOPTHIST:
					if (MessageBox(NULL, Translate("Are you sure you want to clear status message history?"), Translate("Confirm clearing history"), MB_ICONQUESTION | MB_YESNO) == IDYES) {
						int		i, max_hist_msgs, proto_count;
						char	text[8], setting[80];
						PROTOCOLDESCRIPTOR	**proto;

						max_hist_msgs = DBGetContactSettingByte(NULL, "SimpleAway", "MaxHist", 10);

						for (i=1; i<=max_hist_msgs; i++) {
							_snprintf(text, sizeof(text), "SMsg%d", i);
							DBWriteContactSettingString(NULL, "SimpleAway", text, "");
						}
						DBWriteContactSettingString(NULL, "SimpleAway", "LastMsg", "");
						CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
						for(i=0; i<proto_count; i++) {
							if (proto[i]->type != PROTOTYPE_PROTOCOL)
								continue;

							if (!CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_3, 0))
								continue;

							if (!(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
								continue;

							_snprintf(setting, sizeof(setting), "Last%sMsg", proto[i]->szName);
							DBWriteContactSettingString(NULL, "SimpleAway", setting, "");
						}
						DBWriteContactSettingWord(NULL, "SimpleAway", "LMMsg", (WORD)max_hist_msgs);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTHIST), FALSE);
					}
					break;
				case IDC_BOPTDEF:
					if (MessageBox(NULL, Translate("Are you sure you want to clear predefined status messages?"), Translate("Confirm clearing predefined"), MB_ICONQUESTION | MB_YESNO) == IDYES) {
						int		i, num_predef;
						char	text[16];

						num_predef = DBGetContactSettingWord(NULL, "SimpleAway", "DefMsgCount", 0);

						for (i=1; i<=num_predef; i++) {
							_snprintf(text, sizeof(text), "DefMsg%d", i);
							DBDeleteContactSetting(NULL, "SimpleAway", text);
						}
						DBWriteContactSettingWord(NULL, "SimpleAway", "DefMsgCount", 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTDEF), FALSE);
					}
					break;
			}
		break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch(((LPNMHDR)lParam)->code) {
						case PSN_APPLY: {
							BOOL	translated;
							int		val;
							int		flags=0;

							if (IsDlgButtonChecked(hwndDlg, IDC_CCLOSEWND) == BST_CHECKED)
								DBWriteContactSettingByte(NULL, "SimpleAway", "AutoClose", (BYTE)1);
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "AutoClose", (BYTE)0);

							if (IsDlgButtonChecked(hwndDlg, IDC_CSTATUSLIST) == BST_CHECKED)
								flags |= DLG_SHOW_STATUS;
							if (IsDlgButtonChecked(hwndDlg, IDC_CICONS1) == BST_CHECKED)
								flags |= DLG_SHOW_STATUS_ICONS;
							if (IsDlgButtonChecked(hwndDlg, IDC_CICONS2) == BST_CHECKED)
								flags |= DLG_SHOW_LIST_ICONS;
							if (IsDlgButtonChecked(hwndDlg, IDC_CPROFILES) == BST_CHECKED)
								flags |= DLG_SHOW_STATUS_PROFILES;
							val = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_GETCURSEL, 0, 0);
							if (val != CB_ERR)
								flags |= SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_GETITEMDATA, (WPARAM)val, 0);

							DBWriteContactSettingByte(NULL, "SimpleAway", "DlgFlags", (BYTE)flags);

							if (IsDlgButtonChecked(hwndDlg, IDC_CREMOVECR) == BST_CHECKED) {
								DBWriteContactSettingByte(NULL, "SimpleAway", "RemoveCR", (BYTE)1);
								removeCR = TRUE;
							}
							else {
								DBWriteContactSettingByte(NULL, "SimpleAway", "RemoveCR", (BYTE)0);
								removeCR = FALSE;
							}

							if (IsDlgButtonChecked(hwndDlg, IDC_CSHOWCOPY) == BST_CHECKED) {
								DBWriteContactSettingByte(NULL, "SimpleAway", "ShowCopy", (BYTE)1);
								ShowCopy = TRUE;
							}
							else {
								DBWriteContactSettingByte(NULL, "SimpleAway", "ShowCopy", (BYTE)0);
								ShowCopy = FALSE;
							}

							if (IsDlgButtonChecked(hwndDlg, IDC_CRPOSWND) == BST_CHECKED)
								DBWriteContactSettingByte(NULL, "SimpleAway", "WinCentered", (BYTE)0);
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "WinCentered", (BYTE)1);

							if (IsDlgButtonChecked(hwndDlg, IDC_CSHOWSMSG) == BST_CHECKED)
								DBWriteContactSettingByte(NULL, "SimpleAway", "ShowStatusMenuItem", (BYTE)1);
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "ShowStatusMenuItem", (BYTE)0);

							if (IsDlgButtonChecked(hwndDlg, IDC_CLEAVEWINAMP) == BST_CHECKED)
								DBWriteContactSettingByte(NULL, "SimpleAway", "AmpLeaveTitle", (BYTE)1);
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "AmpLeaveTitle", (BYTE)0);


							val = GetDlgItemInt(hwndDlg, IDC_EMAXHIST, &translated, FALSE);
							if (translated)
								DBWriteContactSettingByte(NULL, "SimpleAway", "MaxHist", (BYTE)val);

							val = GetDlgItemInt(hwndDlg, IDC_ETIMEOUT, &translated, FALSE);
							if (translated)
								DBWriteContactSettingByte(NULL, "SimpleAway", "DlgTime", (BYTE)val);

							if (is_timer)
								KillTimer(NULL, SATimer);
							is_timer = FALSE;

							val = GetDlgItemInt(hwndDlg, IDC_ESECWINAMP, &translated, FALSE);
							if (translated)
								DBWriteContactSettingByte(NULL, "SimpleAway", "AmpCheck", (BYTE)val);

							if (IsDlgButtonChecked(hwndDlg, IDC_CCHECKWINAMP) == BST_CHECKED && translated && val) {
								DBWriteContactSettingByte(NULL, "SimpleAway", "AmpCheckOn", (BYTE)1);
								SATimer = SetTimer(NULL, 0, val*1000, (TIMERPROC)SATimerProc);
								is_timer = TRUE;
							}
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "AmpCheckOn", (BYTE)0);

							if (is_randmsgtimer)
								KillTimer(NULL, SARandMsgTimer);
							is_randmsgtimer = FALSE;

							val = GetDlgItemInt(hwndDlg, IDC_EMINRANDMSG, &translated, FALSE);
							if (translated)
								DBWriteContactSettingByte(NULL, "SimpleAway", "RandMsgChange", (BYTE)val);

							if (IsDlgButtonChecked(hwndDlg, IDC_CSETRANDMSG) == BST_CHECKED && translated && val) {
								DBWriteContactSettingByte(NULL, "SimpleAway", "RandMsgChangeOn", (BYTE)1);
								SARandMsgTimer = SetTimer(NULL,0,val*60*1000,(TIMERPROC)SARandMsgTimerProc);
								is_randmsgtimer = TRUE;
							}
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "RandMsgChangeOn", (BYTE)0);

							return TRUE;
						}
						break;
					}
				break;
			}
		break;
	}
	return FALSE;
}

struct StatusOptDlgData {
	int		*status;
	int		*setdelay;
	int		setglobaldelay;
};

static BOOL	DlgInInit3=FALSE;

INT_PTR CALLBACK DlgStatusOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	struct StatusOptDlgData *data;

	data = (struct StatusOptDlgData*)GetWindowLong(hwndDlg,GWL_USERDATA);

	switch (uMsg) {
		case WM_INITDIALOG:	{
			PROTOCOLDESCRIPTOR	**proto;
			int					proto_count;
			int					index, i;

			TranslateDialogDefault(hwndDlg);

			data = (struct StatusOptDlgData*)mir_alloc(sizeof(struct StatusOptDlgData));
			SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)data);

			DlgInInit3 = TRUE;

			CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
			data->status = (int *)mir_alloc(sizeof(int)*proto_count);
			data->setdelay = (int *)mir_alloc(sizeof(int)*proto_count);
			for(i=0; i<proto_count; i++) {
				char protoLabel[MAXMODULELABELLENGTH+1];

				if (proto[i]->type != PROTOTYPE_PROTOCOL || !(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0)))
					continue;

				CallProtoService(proto[i]->szName, PS_GETNAME, MAXMODULELABELLENGTH, (LPARAM)protoLabel);
				index = SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_ADDSTRING, 0, (LPARAM)protoLabel);
				if (index != LB_ERR && index != LB_ERRSPACE) {
					char	setting[80];

					_snprintf(setting, sizeof(setting), "Startup%sStatus", proto[i]->szName);
					data->status[i] = DBGetContactSettingWord(NULL, "SimpleAway", setting, ID_STATUS_OFFLINE);
					_snprintf(setting, sizeof(setting), "Set%sStatusDelay", proto[i]->szName);
					data->setdelay[i] = DBGetContactSettingWord(NULL, "SimpleAway", setting, 300);
					SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
				}
			}
			SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_SETCURSEL, (WPARAM)0, 0);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_LISTPROTO, LBN_SELCHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_LISTPROTO));

			data->setglobaldelay = DBGetContactSettingWord(NULL, "SimpleAway", "SetStatusDelay", 300);

			SendMessage(GetDlgItem(hwndDlg, IDC_SSETSTATUS), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_ESETSTATUS), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SSETSTATUS), UDM_SETRANGE32, (WPARAM)0, (LPARAM)9000);
			SendMessage(GetDlgItem(hwndDlg, IDC_ESETSTATUS), EM_SETLIMITTEXT, (WPARAM)4, 0);

			if (!DBGetContactSettingByte(NULL, "SimpleAway", "GlobalStatusDelay", 1)) {
				CheckDlgButton(hwndDlg, IDC_SPECSET, BST_CHECKED);
				i = SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_GETCURSEL, 0, 0), 0);
				SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setdelay[i], FALSE);
			}
			else {
				CheckDlgButton(hwndDlg, IDC_SPECSET, BST_UNCHECKED);
				SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setglobaldelay, FALSE);
			}

			if (DBGetContactSettingByte(NULL, "SimpleAway", "StartupPopupDlg", 1)) {
				CheckDlgButton(hwndDlg, IDC_POPUPDLG, BST_CHECKED);

				if (IsDlgButtonChecked(hwndDlg, IDC_SPECSET) == BST_CHECKED) {
					CheckDlgButton(hwndDlg, IDC_SPECSET, BST_UNCHECKED);
					SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setglobaldelay, FALSE);
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_SPECSET), FALSE);
			}
			else {
				CheckDlgButton(hwndDlg, IDC_POPUPDLG, BST_UNCHECKED);
			}

			if (ProtoStatusCount == 1 && ProtoStatusMsgCount == 1) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_SPECSET), FALSE);
				CheckDlgButton(hwndDlg, IDC_SPECSET, BST_UNCHECKED); //should work like when checked, but should not be checked
				i = SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_GETCURSEL, 0, 0), 0);
				SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setdelay[i], FALSE);
			}

			DlgInInit3 = FALSE;
			return TRUE;
		}
		break;
		case WM_COMMAND:
			if ( ( (HIWORD(wParam) == BN_CLICKED) || /*(HIWORD(wParam) == EN_KILLFOCUS) ||*/ (HIWORD(wParam) == EN_CHANGE)
				|| ( (HIWORD(wParam) == LBN_SELCHANGE)  && (LOWORD(wParam) != IDC_LISTPROTO) )
				) && (!DlgInInit3) )
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			switch(LOWORD(wParam)) {
				case IDC_ESETSTATUS:
					switch (HIWORD(wParam)) {
						case EN_KILLFOCUS: {
							BOOL	translated;
							int		val;

							val = GetDlgItemInt(hwndDlg, IDC_ESETSTATUS, &translated, FALSE);
							if (translated && val > 9000)
								SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, 9000, FALSE);
							if (translated && val < 0)
								SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, 0, FALSE);
							val = GetDlgItemInt(hwndDlg, IDC_ESETSTATUS, &translated, FALSE);

							if (IsDlgButtonChecked(hwndDlg, IDC_SPECSET) == BST_CHECKED || (ProtoStatusCount == 1 && ProtoStatusMsgCount == 1)) {
								int	i;

								i = SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_GETCURSEL, 0, 0), 0);
								data->setdelay[i] = val;
							}
							else
								data->setglobaldelay = val;
						}
						break;
					}
				break;
				case IDC_SPECSET:
					switch(HIWORD(wParam)) {
						case BN_CLICKED:
							if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED || (ProtoStatusCount == 1 && ProtoStatusMsgCount == 1)) {
								int	i;

								i = SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_GETCURSEL, 0, 0), 0);
								SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setdelay[i], FALSE);
							}
							else
								SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setglobaldelay, FALSE);
							break;
					}
				break;
				case IDC_POPUPDLG:
					switch(HIWORD(wParam)) {
						case BN_CLICKED:
							if (ProtoStatusCount == 1 && ProtoStatusMsgCount == 1)
								break;

							if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED) {
								if (IsDlgButtonChecked(hwndDlg, IDC_SPECSET) == BST_CHECKED) {
									CheckDlgButton(hwndDlg, IDC_SPECSET, BST_UNCHECKED);
									SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setglobaldelay, FALSE);
								}
								EnableWindow(GetDlgItem(hwndDlg, IDC_SPECSET), FALSE);
							}
							else {
								EnableWindow(GetDlgItem(hwndDlg, IDC_SPECSET), TRUE);
							}
							break;
					}
				break;
				case IDC_LISTPROTO:
					switch(HIWORD(wParam)) {
						case LBN_SELCHANGE: {
							PROTOCOLDESCRIPTOR	**proto;
							int					proto_count, status_modes, i, l, index, newindex=0;

							i = SendMessage((HWND)lParam, LB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0), 0);

							CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
							status_modes = CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0);

							SendMessage(GetDlgItem(hwndDlg, IDC_LISTSTATUS), LB_RESETCONTENT, (WPARAM)0, 0);
							for (l=ID_STATUS_OFFLINE; l<=ID_STATUS_OUTTOLUNCH; l++) {
								if (status_modes & Proto_Status2Flag(l) || l==ID_STATUS_OFFLINE) {
									index = SendMessage(GetDlgItem(hwndDlg, IDC_LISTSTATUS), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, l, 0));

									if (index != LB_ERR && index != LB_ERRSPACE) {
										SendMessage(GetDlgItem(hwndDlg, IDC_LISTSTATUS), LB_SETITEMDATA, (WPARAM)index, (LPARAM)l-ID_STATUS_OFFLINE);
										if (data->status[i] == l)
											newindex = index;
									}
								}
							}

							index = SendMessage(GetDlgItem(hwndDlg, IDC_LISTSTATUS), LB_INSERTSTRING, (WPARAM)-1, (LPARAM)Translate("<Last status>"));
							if (index != LB_ERR && index != LB_ERRSPACE) {
								SendMessage(GetDlgItem(hwndDlg, IDC_LISTSTATUS), LB_SETITEMDATA, (WPARAM)index, (LPARAM)ID_STATUS_CURRENT-ID_STATUS_OFFLINE);
								if (data->status[i] == ID_STATUS_CURRENT)
									newindex = index;
							}

							SendMessage(GetDlgItem(hwndDlg, IDC_LISTSTATUS), LB_SETCURSEL, (WPARAM)newindex, 0);

							if (IsDlgButtonChecked(hwndDlg, IDC_SPECSET) == BST_CHECKED || (ProtoStatusCount == 1 && ProtoStatusMsgCount == 1))
								SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setdelay[i], FALSE);
						}
						break;
					}
				break;
				case IDC_LISTSTATUS:
					switch(HIWORD(wParam)) {
						case LBN_SELCHANGE: {
							int		i, j;

							i = SendMessage((HWND)lParam, LB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0), 0);
							j = SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_LISTPROTO), LB_GETCURSEL, 0, 0), 0);

							data->status[j] = i+ID_STATUS_OFFLINE;
						}
						break;
					}
				break;
			}
		break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch(((LPNMHDR)lParam)->code) {
						case PSN_APPLY: {
							PROTOCOLDESCRIPTOR	**proto;
							int					proto_count;
							int					i;
							char				setting[80];

							CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
							for(i=0; i<proto_count; i++) {
								if (proto[i]->type != PROTOTYPE_PROTOCOL || !(CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(proto[i]->szName, PS_GETCAPS, PFLAGNUM_5, 0)))
									continue;

								_snprintf(setting, sizeof(setting), "Startup%sStatus", proto[i]->szName);
								DBWriteContactSettingWord(NULL, "SimpleAway", setting, (WORD)data->status[i]);

								_snprintf(setting, sizeof(setting), "Set%sStatusDelay", proto[i]->szName);
								DBWriteContactSettingWord(NULL, "SimpleAway", setting, (WORD)data->setdelay[i]);
							}

							if (IsDlgButtonChecked(hwndDlg, IDC_SPECSET) == BST_CHECKED)
								DBWriteContactSettingByte(NULL, "SimpleAway", "GlobalStatusDelay", (BYTE)0);
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "GlobalStatusDelay", (BYTE)1);

							if (IsDlgButtonChecked(hwndDlg, IDC_POPUPDLG) == BST_CHECKED)
								DBWriteContactSettingByte(NULL, "SimpleAway", "StartupPopupDlg", (BYTE)1);
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "StartupPopupDlg", (BYTE)0);

							DBWriteContactSettingWord(NULL, "SimpleAway", "SetStatusDelay", (WORD)data->setglobaldelay);

							return TRUE;
						}
						break;
					}
				break;
			}
		break;
		case WM_DESTROY:
			mir_free(data->status);
			mir_free(data->setdelay);
			mir_free(data);
			break;
	}
	return FALSE;
}

int InitStatusOptions(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp = { 0 };

	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.position = 0;
	odp.pszTitle = Translate("Status");
	odp.pszGroup = NULL;
	odp.pszTab = NULL;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTSTATUS);
	odp.pfnDlgProc = DlgStatusOptionsProc;
	odp.flags = ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);

	return 0;
}

int InitOptions(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp = { 0 };

	odp.cbSize = sizeof(odp);
	odp.position = 870000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONDLG);
	odp.pszTitle = Translate("Status Messages");
	odp.pszGroup = Translate("Status");
	odp.pszTab = Translate("General");
	odp.pfnDlgProc = DlgOptionsProc;
	odp.flags = ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);

	odp.pszTab = Translate("Advanced");
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONDLG2);
	odp.pfnDlgProc = DlgAdvancedOptionsProc;
	odp.flags = ODPF_BOLDGROUPS | ODPF_EXPERTONLY;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);

	if (!ServiceExists(MS_SS_GETPROFILECOUNT))
		InitStatusOptions(wParam, lParam);

	return 0;
}
