/*

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

struct SingleProtoMsg
{
	int		flags;
	char	*msg;
	int		max_length;
};

struct OptDlgData
{
	int						status[9];
	int						flags[9];
	char					msg[9][1024];
	BOOL					statmsg_changed[9];
	BOOL					proto_ok;
	int						proto_count;
	struct SingleProtoMsg	*proto_msg;
};

INT_PTR CALLBACK DlgOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct OptDlgData *data;

	data = (struct OptDlgData*)GetWindowLong(hwndDlg,GWL_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			int					val;
			int					i, i_btnhide, i_btnlist, i_btndown, i_btndownflat;
			PROTOCOLDESCRIPTOR	**proto;
			int					proto_count;
			int					index;
			DBVARIANT			dbv;

			TranslateDialogDefault(hwndDlg);

			data = (struct OptDlgData*)mir_alloc(sizeof(struct OptDlgData));
			SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)data);

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

			SendDlgItemMessage(hwndDlg, IDC_OPTEDIT1, EM_LIMITTEXT, 1024, 0);
			SendDlgItemMessage(hwndDlg, IDC_OPTEDIT2, EM_LIMITTEXT, 1024, 0);

			SendMessage(GetDlgItem(hwndDlg, IDC_SMAXLENGTH), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_EMAXLENGTH), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SMAXLENGTH), UDM_SETRANGE32, (WPARAM)1, (LPARAM)1024);
			SendMessage(GetDlgItem(hwndDlg, IDC_EMAXLENGTH), EM_SETLIMITTEXT, (WPARAM)4, 0);

			SendMessage(GetDlgItem(hwndDlg, IDC_SMAXHIST), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_EMAXHIST), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SMAXHIST), UDM_SETRANGE32, (WPARAM)0, (LPARAM)25);
			SendMessage(GetDlgItem(hwndDlg, IDC_EMAXHIST), EM_SETLIMITTEXT, (WPARAM)2, 0);
			SetDlgItemInt(hwndDlg, IDC_EMAXHIST, DBGetContactSettingByte(NULL, "SimpleAway", "MaxHist", 10), FALSE);

			SendMessage(GetDlgItem(hwndDlg, IDC_STIMEOUT), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_ETIMEOUT), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_STIMEOUT), UDM_SETRANGE32, (WPARAM)0, (LPARAM)60);
			SendMessage(GetDlgItem(hwndDlg, IDC_ETIMEOUT), EM_SETLIMITTEXT, (WPARAM)2, 0);
			SetDlgItemInt(hwndDlg, IDC_ETIMEOUT, DBGetContactSettingByte(NULL, "SimpleAway", "DlgTime", 5), FALSE);

			SendMessage(GetDlgItem(hwndDlg, IDC_SSECWINAMP), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_ESECWINAMP), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SSECWINAMP), UDM_SETRANGE32, (WPARAM)1, (LPARAM)240);
			SendMessage(GetDlgItem(hwndDlg, IDC_ESECWINAMP), EM_SETLIMITTEXT, (WPARAM)3, 0);
			val = DBGetContactSettingByte(NULL, "SimpleAway", "AmpCheck", 15);
			SetDlgItemInt(hwndDlg, IDC_ESECWINAMP, val, FALSE);
			if (!val)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_ESECWINAMP), FALSE);
				CheckDlgButton(hwndDlg, IDC_CCHECKWINAMP, BST_UNCHECKED);
			}
			else
				CheckDlgButton(hwndDlg, IDC_CCHECKWINAMP, BST_CHECKED);

			if (DBGetContactSettingByte(NULL, "SimpleAway", "RemoveCR", 1))
				CheckDlgButton(hwndDlg, IDC_CREMOVECR, BST_CHECKED);

			if (DBGetContactSettingByte(NULL, "SimpleAway", "ShowCopy", 1))
				CheckDlgButton(hwndDlg, IDC_CSHOWCOPY, BST_CHECKED);

			val = DBGetContactSettingByte(NULL, "SimpleAway", "AutoClose", 1);
			if (!val)
				EnableWindow(GetDlgItem(hwndDlg, IDC_ETIMEOUT), FALSE);
			else
				SendMessage(GetDlgItem(hwndDlg, IDC_CCLOSEWND), BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

			val = DBGetContactSettingByte(NULL, "SimpleAway", "DlgFlags", DLG_SHOW_STATUS|DLG_SHOW_STATUS_ICONS|DLG_SHOW_LIST_ICONS|DLG_SHOW_BUTTONS);
			if (val & DLG_SHOW_STATUS)
				CheckDlgButton(hwndDlg, IDC_CSTATUSLIST, BST_CHECKED);
			else
				EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS1), FALSE);

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

			for (i=ID_STATUS_ONLINE; i<=ID_STATUS_OUTTOLUNCH; i++)
			{
				if (ProtoStatusFlags & Proto_Status2Flag(i))
				{
					index = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_INSERTSTRING, (WPARAM)-1, (LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, i, 0));

					if (index != CB_ERR && index != CB_ERRSPACE)
					{
						SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_SETITEMDATA, (WPARAM)index, (LPARAM)i-ID_STATUS_ONLINE);
						val = DBGetContactSettingByte(NULL, "SimpleAway", (char *)StatusModeToDbSetting(i, "Flags"), STATUS_SHOW_DLG|STATUS_LAST_MSG);
						data->flags[i-ID_STATUS_ONLINE] = val;
						data->status[i-ID_STATUS_ONLINE] = i;
						if(DBGetContactSetting(NULL, "SRAway", StatusModeToDbSetting(i, "Default"), &dbv))
							dbv.pszVal = mir_strdup(GetDefaultMessage(i));
						lstrcpy(data->msg[i-ID_STATUS_ONLINE], dbv.pszVal);
						DBFreeVariant(&dbv);
					}
				}
			}
			SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_SETCURSEL, (WPARAM)0, 0);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CBOPTSTATUS, CBN_SELCHANGE),(LPARAM)GetDlgItem(hwndDlg, IDC_CBOPTSTATUS));

			CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
			data->proto_msg = (struct SingleProtoMsg *)mir_alloc(sizeof(struct SingleProtoMsg)*proto_count);
			if (!data->proto_msg)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT2), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COPTPROTO1), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COPTPROTO2), FALSE);
				data->proto_ok = FALSE;
			}
			else
			{
				char setting[64];

				data->proto_ok = TRUE;
				data->proto_count = proto_count;
				for(i=0; i<proto_count; i++)
				{
					if (proto[i]->type != PROTOTYPE_PROTOCOL)
					{
						data->proto_msg[i].msg = NULL;
						continue;
					}

					index = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_ADDSTRING, 0, (LPARAM)proto[i]->szName);
					if (index != CB_ERR && index != CB_ERRSPACE)
					{
						_snprintf(setting, sizeof(setting), "Proto%sDefault", proto[i]->szName);
						if(!DBGetContactSetting(NULL, "SimpleAway", setting, &dbv))
						{
							data->proto_msg[i].msg = mir_strdup(dbv.pszVal);
							DBFreeVariant(&dbv);
						}
						else
							data->proto_msg[i].msg = NULL;

						_snprintf(setting, sizeof(setting), "Proto%sFlags", proto[i]->szName);
						val = DBGetContactSettingByte(NULL, "SimpleAway", setting, PROTO_POPUPDLG);
						data->proto_msg[i].flags = val;
						_snprintf(setting, sizeof(setting), "Proto%sMaxLen", proto[i]->szName);
						val = DBGetContactSettingWord(NULL, "SimpleAway", setting, 1024);
						data->proto_msg[i].max_length = val;
						SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
					}
				}
				SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_SETCURSEL, (WPARAM)0, 0);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CBOPTPROTO, CBN_SELCHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_CBOPTPROTO));
			}

			if (!DBGetContactSettingWord(NULL, "SimpleAway", "DefMsgCount", 0))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BOPDEF), FALSE);
			return TRUE;
		}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_CCLOSEWND:
					switch (HIWORD(wParam))
					{
						case BN_CLICKED:
						{
							LRESULT checked;

							checked = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);
							if (BST_UNCHECKED == checked)
							{
								if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ETIMEOUT)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_ETIMEOUT), FALSE);
							}
							else
							{
								if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ETIMEOUT)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_ETIMEOUT), TRUE);
							}
						}
						break;
					}
				break;
				case IDC_EMAXHIST:
					switch (HIWORD(wParam))
					{
						case EN_KILLFOCUS:
						{
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
					switch (HIWORD(wParam))
					{
						case EN_KILLFOCUS:
						{
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
					switch (HIWORD(wParam))
					{
						case EN_KILLFOCUS:
						{
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
				case IDC_EMAXLENGTH:
					switch (HIWORD(wParam))
					{
						case EN_KILLFOCUS:
						{
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
				case IDC_CSTATUSLIST:
					switch (HIWORD(wParam))
					{
						case BN_CLICKED:
						{
							LRESULT checked;

							checked = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);
							if (BST_UNCHECKED == checked)
							{
								if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CICONS1)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS1), FALSE);
							}
							else
							{
								if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CICONS1)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS1), TRUE);
							}
						}
						break;
					}
				break;
				case IDC_CCHECKWINAMP:
					switch (HIWORD(wParam))
					{
						case BN_CLICKED:
						{
							if (IsDlgButtonChecked(hwndDlg, IDC_CCHECKWINAMP) == BST_CHECKED)
							{
								if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ESECWINAMP)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_ESECWINAMP), TRUE);
								SetDlgItemInt(hwndDlg, IDC_ESECWINAMP, 1, FALSE);
							}
							else if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ESECWINAMP)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_ESECWINAMP), FALSE);
						}
						break;
					}
				break;
				case IDC_CBOPTPROTO:
					switch(HIWORD(wParam))
					{
						case CBN_SELCHANGE:
						case CBN_SELENDOK:
						{
							int	i;

							i = SendMessage((HWND)lParam, CB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);

							SetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, data->proto_msg[i].max_length, FALSE);

							if (data->proto_msg[i].flags & PROTO_POPUPDLG)
								CheckDlgButton(hwndDlg, IDC_COPTPROTO3, BST_CHECKED);
							else
								CheckDlgButton(hwndDlg, IDC_COPTPROTO3, BST_UNCHECKED);

							if (data->proto_msg[i].flags & PROTO_NO_MSG)
							{
								CheckDlgButton(hwndDlg, IDC_COPTPROTO1, BST_CHECKED);
								CheckDlgButton(hwndDlg, IDC_COPTPROTO2, BST_UNCHECKED);
							}
							else if (data->proto_msg[i].flags & PROTO_THIS_MSG)
							{
								CheckDlgButton(hwndDlg, IDC_COPTPROTO2, BST_CHECKED);
								CheckDlgButton(hwndDlg, IDC_COPTPROTO1, BST_UNCHECKED);
							}
							else
							{
								CheckDlgButton(hwndDlg, IDC_COPTPROTO1, BST_UNCHECKED);
								CheckDlgButton(hwndDlg, IDC_COPTPROTO2, BST_UNCHECKED);
							}

							if (data->proto_msg[i].flags & PROTO_THIS_MSG)
							{
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT2), TRUE);
								if (data->proto_msg[i].msg)
									SetDlgItemText(hwndDlg, IDC_OPTEDIT2, data->proto_msg[i].msg);
								else
									SetDlgItemText(hwndDlg, IDC_OPTEDIT2, "");
							}
							else
							{
								SetDlgItemText(hwndDlg, IDC_OPTEDIT2, "");
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT2), FALSE);
							}
						}
						break;
					}
	//			return 0;
				break;
				case IDC_COPTPROTO1:
				case IDC_COPTPROTO2:
				case IDC_COPTPROTO3:
					switch(HIWORD(wParam))
					{
						case BN_CLICKED:
						{
							int	i;

							i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETCURSEL, 0, 0), 0);

							data->proto_msg[i].flags = 0;

							if (LOWORD(wParam) == IDC_COPTPROTO1)
							{
								if (IsDlgButtonChecked(hwndDlg, IDC_COPTPROTO1) == BST_CHECKED)
								{
									data->proto_msg[i].flags = PROTO_NO_MSG;
									CheckDlgButton(hwndDlg, IDC_COPTPROTO2, BST_UNCHECKED);
									SetDlgItemText(hwndDlg, IDC_OPTEDIT2, "");
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT2), FALSE);
								}
							}
							else if (LOWORD(wParam) == IDC_COPTPROTO2)
							{
								if (IsDlgButtonChecked(hwndDlg, IDC_COPTPROTO2) == BST_CHECKED)
								{
									CheckDlgButton(hwndDlg, IDC_COPTPROTO1, BST_UNCHECKED);
									data->proto_msg[i].flags = PROTO_THIS_MSG;
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT2), TRUE);
									if (data->proto_msg[i].msg)
										SetDlgItemText(hwndDlg, IDC_OPTEDIT2, data->proto_msg[i].msg);
									else
										SetDlgItemText(hwndDlg, IDC_OPTEDIT2, "");
								}
								else
								{
									SetDlgItemText(hwndDlg, IDC_OPTEDIT2, "");
									EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT2), FALSE);
								}
							}
							else if (LOWORD(wParam) == IDC_COPTPROTO3)
							{
								if (IsDlgButtonChecked(hwndDlg, IDC_COPTPROTO3) == BST_CHECKED)
									data->proto_msg[i].flags |= PROTO_POPUPDLG;
							}
						}
					}
				break;
				case IDC_OPTEDIT2:
				{
					int	i;

					i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), CB_GETCURSEL, 0, 0), 0);

					if(HIWORD(wParam) == EN_KILLFOCUS)
					{
						char	msg[1024];
						int		len;

						len = GetDlgItemText(hwndDlg, IDC_OPTEDIT2, msg, sizeof(msg));
						if (len > 0)
						{
							if (data->proto_msg[i].msg == NULL)
								data->proto_msg[i].msg = mir_strdup(msg);
							else
							{
								mir_free(data->proto_msg[i].msg);
								data->proto_msg[i].msg = mir_strdup(msg);
							}
						}
						else
						{
							if (data->proto_msg[i].msg != NULL)
							{
								mir_free(data->proto_msg[i].msg);
								data->proto_msg[i].msg = NULL;
							}
						}
					}
				}
				break;
				case IDC_CBOPTSTATUS:
					switch(HIWORD(wParam))
					{
						case CBN_SELCHANGE:
						case CBN_SELENDOK:
						{
							int	i;

							i = SendMessage((HWND)lParam, CB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);

							if (data->flags[i] & STATUS_SHOW_DLG)
								CheckDlgButton(hwndDlg, IDC_COPTMSG1, BST_CHECKED);
							else
								CheckDlgButton(hwndDlg, IDC_COPTMSG1, BST_UNCHECKED);

							if (data->flags[i] & STATUS_EMPTY_MSG)
							{
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG1);
							}
							else if (data->flags[i] & STATUS_DEFAULT_MSG)
							{
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG2);
							}
							else if (data->flags[i] & STATUS_LAST_MSG)
							{
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG3);
							}
							else if (data->flags[i] & STATUS_THIS_MSG)
							{
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->msg[i]);
							}
							else if (data->flags[i] & STATUS_LAST_STATUS_MSG)
							{
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG5);
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
							}

							if (data->flags[i] & STATUS_PUT_DEF_IN_LIST)
								CheckDlgButton(hwndDlg, IDC_COPTMSG2, BST_CHECKED);
							else
								CheckDlgButton(hwndDlg, IDC_COPTMSG2, BST_UNCHECKED);
						}
						break;
					}
	//			return 0;
				break;
				case IDC_COPTMSG1:
					switch(HIWORD(wParam))
					{
						case BN_CLICKED:
						{
							int	i;

							i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);
							if (IsDlgButtonChecked(hwndDlg, IDC_COPTMSG1) == BST_CHECKED)
								data->flags[i] |= STATUS_SHOW_DLG;
							else
								data->flags[i] &= ~STATUS_SHOW_DLG;
						}
					}
				break;
				case IDC_COPTMSG2:
					switch(HIWORD(wParam))
					{
						case BN_CLICKED:
						{
							int	i;

							i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);
							if (IsDlgButtonChecked(hwndDlg, IDC_COPTMSG2) == BST_CHECKED)
								data->flags[i] |= STATUS_PUT_DEF_IN_LIST;
							else
								data->flags[i] &= ~STATUS_PUT_DEF_IN_LIST;
						}
					}
				break;
				case IDC_ROPTMSG1:
				case IDC_ROPTMSG2:
				case IDC_ROPTMSG3:
				case IDC_ROPTMSG4:
				case IDC_ROPTMSG5:
					switch(HIWORD(wParam))
					{
						case BN_CLICKED:
						{
							int	i;

							i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);
							data->flags[i] = 0;

							if (IsDlgButtonChecked(hwndDlg, IDC_COPTMSG1) == BST_CHECKED)
								data->flags[i] |= STATUS_SHOW_DLG;

							if (LOWORD(wParam) == IDC_ROPTMSG1)
								data->flags[i] |= STATUS_EMPTY_MSG;
							else if (LOWORD(wParam) == IDC_ROPTMSG2)
								data->flags[i] |= STATUS_DEFAULT_MSG;
							else if (LOWORD(wParam) == IDC_ROPTMSG3)
								data->flags[i] |= STATUS_LAST_MSG;
							else if (LOWORD(wParam) == IDC_ROPTMSG5)
								data->flags[i] |= STATUS_LAST_STATUS_MSG;
							else if (LOWORD(wParam) == IDC_ROPTMSG4)
							{
								data->flags[i] |= STATUS_THIS_MSG;
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->msg[i]);
							}

							if (LOWORD(wParam) != IDC_ROPTMSG4)
							{
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, "");
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
							}
						}
					}
				break;
				case IDC_OPTEDIT1:
				{
					int	i;

					i = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), CB_GETCURSEL, 0, 0), 0);

					if(HIWORD(wParam) == EN_CHANGE)
						data->statmsg_changed[i] = TRUE;

					if(HIWORD(wParam) == EN_KILLFOCUS)
					{
						char msg[1024];

						GetDlgItemText(hwndDlg, IDC_OPTEDIT1, msg, sizeof(msg));
						lstrcpy(data->msg[i], msg);
					}
				}
				break;
				case IDC_BOPTHIST:
				{
					int		i, max_hist_msgs;
					char	text[8];

					max_hist_msgs = DBGetContactSettingByte(NULL, "SimpleAway", "MaxHist", 10);

					for (i=1; i<=max_hist_msgs; i++)
					{
						_snprintf(text, sizeof(text), "SMsg%d", i);
						DBWriteContactSettingString(NULL, "SimpleAway", text, "");
					}
					DBWriteContactSettingString(NULL, "SimpleAway", "LastMsg", "");
					DBWriteContactSettingWord(NULL, "SimpleAway", "LMMsg", (WORD)max_hist_msgs);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTHIST), FALSE);
					return 0;
				}
				break;
				case IDC_BOPDEF:
				{
					int		i, num_predef;
					char	text[16];

					num_predef = DBGetContactSettingWord(NULL, "SimpleAway", "DefMsgCount", 0);

					for (i=1; i<=num_predef; i++)
					{
						_snprintf(text, sizeof(text), "DefMsg%d", i);
						DBDeleteContactSetting(NULL, "SimpleAway", text);
					}
					DBWriteContactSettingWord(NULL, "SimpleAway", "DefMsgCount", 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BOPDEF), FALSE);
					return 0;
				}
				break;
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch(((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							BOOL	translated;
							int		val;
							LRESULT checked;
							int		flags=0, i;

							checked = SendMessage(GetDlgItem(hwndDlg, IDC_CCLOSEWND), BM_GETCHECK, 0, 0);
							if (checked == BST_CHECKED)
								DBWriteContactSettingByte(NULL, "SimpleAway", "AutoClose", (BYTE)1);
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "AutoClose", (BYTE)0);

							if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CSTATUSLIST))
								flags |= DLG_SHOW_STATUS;
							if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CICONS1))
								flags |= DLG_SHOW_STATUS_ICONS;
							if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CICONS2))
								flags |= DLG_SHOW_LIST_ICONS;
							val = SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_GETCURSEL, 0, 0);
							if (val != CB_ERR)
								flags |= SendMessage(GetDlgItem(hwndDlg, IDC_CBOPTBUTTONS), CB_GETITEMDATA, (WPARAM)val, 0);

							if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CREMOVECR))
							{
								DBWriteContactSettingByte(NULL, "SimpleAway", "RemoveCR", (BYTE)1);
								removeCR = TRUE;
							}
							else
							{
								DBWriteContactSettingByte(NULL, "SimpleAway", "RemoveCR", (BYTE)0);
								removeCR = FALSE;
							}

							if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CSHOWCOPY))
							{
								DBWriteContactSettingByte(NULL, "SimpleAway", "ShowCopy", (BYTE)1);
								ShowCopy = TRUE;
							}
							else
							{
								DBWriteContactSettingByte(NULL, "SimpleAway", "ShowCopy", (BYTE)0);
								ShowCopy = FALSE;
							}

							DBWriteContactSettingByte(NULL, "SimpleAway", "DlgFlags", (BYTE)flags);

							val = GetDlgItemInt(hwndDlg, IDC_EMAXHIST, &translated, FALSE);
							if (translated)
								DBWriteContactSettingByte(NULL, "SimpleAway", "MaxHist", (BYTE)val);

							val = GetDlgItemInt(hwndDlg, IDC_ETIMEOUT, &translated, FALSE);
							if (translated)
								DBWriteContactSettingByte(NULL, "SimpleAway", "DlgTime", (BYTE)val);

							if (IsDlgButtonChecked(hwndDlg, IDC_CCHECKWINAMP) == BST_CHECKED)
							{
								val = GetDlgItemInt(hwndDlg, IDC_ESECWINAMP, &translated, FALSE);
								if (translated)
									DBWriteContactSettingByte(NULL, "SimpleAway", "AmpCheck", (BYTE)val);
							}
							else
								DBWriteContactSettingByte(NULL, "SimpleAway", "AmpCheck", (BYTE)0);

							for (i=ID_STATUS_ONLINE; i<=ID_STATUS_OUTTOLUNCH; i++)
							{
								if (ProtoStatusFlags & Proto_Status2Flag(i))
								{
									DBWriteContactSettingString(NULL, "SRAway", StatusModeToDbSetting(i, "Default"), data->msg[i-ID_STATUS_ONLINE]);
									DBWriteContactSettingByte(NULL, "SimpleAway", StatusModeToDbSetting(i, "Flags"), (BYTE)data->flags[i-ID_STATUS_ONLINE]);
								}
							}

							if (data->proto_ok)
							{
								PROTOCOLDESCRIPTOR	**proto;
								int					proto_count;
								char				setting[64];

								CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&proto);
								for(i=0; i<proto_count; i++)
								{
									if (proto[i]->type != PROTOTYPE_PROTOCOL)
										continue;

									_snprintf(setting, sizeof(setting), "Proto%sDefault", proto[i]->szName);
									if (data->proto_msg[i].msg && (data->proto_msg[i].flags & PROTO_THIS_MSG))
										DBWriteContactSettingString(NULL, "SimpleAway", setting, data->proto_msg[i].msg);
									else
										DBDeleteContactSetting(NULL, "SimpleAway", setting);

									_snprintf(setting, sizeof(setting), "Proto%sMaxLen", proto[i]->szName);
										DBWriteContactSettingWord(NULL, "SimpleAway", setting, (WORD)data->proto_msg[i].max_length);

									_snprintf(setting, sizeof(setting), "Proto%sFlags", proto[i]->szName);
									DBWriteContactSettingByte(NULL, "SimpleAway", setting, (BYTE)data->proto_msg[i].flags);
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
			if (data->proto_ok)
			{
				int i;

				for (i=0; i<data->proto_count; i++)
				{
					if (data->proto_msg[i].msg) //they want to be free, do they?
						mir_free(data->proto_msg[i].msg);
				}
				mir_free(data->proto_msg);
			}
			mir_free(data);
		break;

	}
	return FALSE;
}

int InitOptions(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };

    odp.cbSize = sizeof(odp);
    odp.position = 870000000;
    odp.hInstance = hInst;
    odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONDLG);
    odp.pszTitle = Translate("Status Messages");
    odp.pszGroup = Translate("Status");
    odp.pfnDlgProc = DlgOptionsProc;
    odp.flags = ODPF_BOLDGROUPS;
    CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);

	return 0;
}
