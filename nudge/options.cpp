#include "headers.h"
#include "main.h"
#include "shake.h"
#include "options.h"


int NudgeOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize						= sizeof(odp);
	odp.position					= -790000000;
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle					= "Nudge";
	odp.pszGroup					= "Events";
	odp.flags						= ODPF_BOLDGROUPS;
//	odp.nIDBottomSimpleControl = IDC_STMSNGROUP;
	odp.pfnDlgProc					= OptionsDlgProc;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );
	return 0;
}

int InitOptions()
{
	HookEvent(ME_OPT_INITIALISE, NudgeOptInit);
	return 0;
}

static BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int iInit = TRUE;
   
   switch(msg)
   {
      case WM_INITDIALOG:
      {
         TCITEM tci;
         RECT rcClient;
         GetClientRect(hwnd, &rcClient);

		 iInit = TRUE;
         tci.mask = TCIF_PARAM|TCIF_TEXT;
         tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_NUDGE), hwnd, DlgProcNudgeOpt);
         tci.pszText = TranslateT("Nudge");
		 TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 0, &tci);
         MoveWindow((HWND)tci.lParam,14,29,rcClient.right-30,rcClient.bottom-45,1);

         tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_SHAKE),hwnd,DlgProcShakeOpt);
         tci.pszText = TranslateT("Window Shaking");
         TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 1, &tci);
         MoveWindow((HWND)tci.lParam,14,29,rcClient.right-30,rcClient.bottom-45,1);
         ShowWindow((HWND)tci.lParam, SW_HIDE);
         iInit = FALSE;
         return FALSE;
      }

      case PSM_CHANGED: // used so tabs dont have to call SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
         if(!iInit)
             SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
         break;
      case WM_NOTIFY:
         switch(((LPNMHDR)lParam)->idFrom) {
            case 0:
               switch (((LPNMHDR)lParam)->code)
               {
                  case PSN_APPLY:
                     {
                        TCITEM tci;
                        int i,count;
                        tci.mask = TCIF_PARAM;
                        count = TabCtrl_GetItemCount(GetDlgItem(hwnd,IDC_OPTIONSTAB));
                        for (i=0;i<count;i++)
                        {
                           TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),i,&tci);
                           SendMessage((HWND)tci.lParam,WM_NOTIFY,0,lParam);
                        }

						NudgeElementList *n;
						for(n = NudgeList;n != NULL; n = n->next)
						{
							n->item.Save();
						}
						DefaultNudge.Save();						
                     }
                  break;
               }
            break;
            case IDC_OPTIONSTAB:
               switch (((LPNMHDR)lParam)->code)
               {
                  case TCN_SELCHANGING:
                     {
                        TCITEM tci;
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwnd,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_HIDE);                     
                     }
                  break;
                  case TCN_SELCHANGE:
                     {
                        TCITEM tci;
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwnd,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_SHOW);                     
                     }
                  break;
               }
            break;

         }
      break;
   }
   return FALSE;
}

BOOL CALLBACK DlgProcShakeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			char szBuf[20];
			TranslateDialogDefault(hwnd);
			_snprintf(szBuf, 10, "%d", shake.nMoveClist);
			SetWindowTextA(GetDlgItem(hwnd, IDC_LNUMBER_CLIST), szBuf);
			_snprintf(szBuf, 10, "%d", shake.nMoveChat);
			SetWindowTextA(GetDlgItem(hwnd, IDC_LNUMBER_CHAT), szBuf);

			_snprintf(szBuf, 10, "%d", shake.nScaleClist);
			SetWindowTextA(GetDlgItem(hwnd, IDC_LSCALE_CLIST), szBuf);
			_snprintf(szBuf, 10, "%d", shake.nScaleChat);
			SetWindowTextA(GetDlgItem(hwnd, IDC_LSCALE_CHAT), szBuf);

			SendDlgItemMessage(hwnd, IDC_SNUMBER_CLIST, TBM_SETPOS, TRUE, shake.nMoveClist);
			SendDlgItemMessage(hwnd, IDC_SNUMBER_CHAT, TBM_SETPOS, TRUE, shake.nMoveChat);

			SendDlgItemMessage(hwnd, IDC_SSCALE_CLIST, TBM_SETPOS, TRUE, shake.nScaleClist);
			SendDlgItemMessage(hwnd, IDC_SSCALE_CHAT, TBM_SETPOS, TRUE, shake.nScaleChat);

			SendDlgItemMessage(hwnd, IDC_SNUMBER_CLIST, TBM_SETRANGE, 0, (LPARAM)MAKELONG(1, 60));
			SendDlgItemMessage(hwnd, IDC_SNUMBER_CHAT, TBM_SETRANGE, 0, (LPARAM)MAKELONG(1, 60));

			SendDlgItemMessage(hwnd, IDC_SSCALE_CLIST, TBM_SETRANGE, 0, (LPARAM)MAKELONG(1, 40));
			SendDlgItemMessage(hwnd, IDC_SSCALE_CHAT, TBM_SETRANGE, 0, (LPARAM)MAKELONG(1, 40));



			break;
		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);
			switch(LOWORD(wParam))
			{
				case IDC_PREVIEW:
					ShakeClist(0,0);
					//SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
			}
			break;
		}
		case WM_HSCROLL:
            if((HWND)lParam == GetDlgItem(hwnd, IDC_SNUMBER_CLIST) || (HWND)lParam == GetDlgItem(hwnd, IDC_SNUMBER_CHAT) 
				|| (HWND)lParam == GetDlgItem(hwnd, IDC_SSCALE_CLIST) || (HWND)lParam == GetDlgItem(hwnd, IDC_SSCALE_CHAT)) 
			{
                char szBuf[20];
                DWORD dwPos = SendMessage((HWND) lParam, TBM_GETPOS, 0, 0);
                _snprintf(szBuf, 10, "%d", dwPos);
                if ((HWND)lParam == GetDlgItem(hwnd, IDC_SNUMBER_CLIST))
                    SetWindowTextA(GetDlgItem(hwnd, IDC_LNUMBER_CLIST), szBuf);
                if ((HWND)lParam == GetDlgItem(hwnd, IDC_SNUMBER_CHAT))
                    SetWindowTextA(GetDlgItem(hwnd, IDC_LNUMBER_CHAT), szBuf);
				if ((HWND)lParam == GetDlgItem(hwnd, IDC_SSCALE_CLIST))
                    SetWindowTextA(GetDlgItem(hwnd, IDC_LSCALE_CLIST), szBuf);
                if ((HWND)lParam == GetDlgItem(hwnd, IDC_SSCALE_CHAT))
                    SetWindowTextA(GetDlgItem(hwnd, IDC_LSCALE_CHAT), szBuf);
                SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
            }
			break;

		case WM_SHOWWINDOW:
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch(((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							shake.nMoveClist = (int) SendMessage((HWND) GetDlgItem(hwnd, IDC_SNUMBER_CLIST), TBM_GETPOS, 0, 0);
							shake.nMoveChat = (int) SendMessage((HWND) GetDlgItem(hwnd, IDC_SNUMBER_CHAT), TBM_GETPOS, 0, 0);
							shake.nScaleClist = (int) SendMessage((HWND) GetDlgItem(hwnd, IDC_SSCALE_CLIST), TBM_GETPOS, 0, 0);
							shake.nScaleChat = (int) SendMessage((HWND) GetDlgItem(hwnd, IDC_SSCALE_CHAT), TBM_GETPOS, 0, 0);
							shake.Save();
						}
					}
			}
			break;
	}

	return FALSE;
}

void CreateImageList(HWND hWnd)
{
	// Create and populate image list
	HIMAGELIST hImList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),	ILC_MASK | ILC_COLOR32, nProtocol, 0);

	NudgeElementList *n;
	for(n = NudgeList;n != NULL; n = n->next)
	{
		HICON hIcon = NULL;
		hIcon=(HICON)CallProtoService(n->item.ProtocolName, PS_LOADICON,PLI_PROTOCOL | PLIF_SMALL, 0);
		if (hIcon == NULL || (int)hIcon == CALLSERVICE_NOTFOUND) 
		{
			hIcon=(HICON)CallProtoService(n->item.ProtocolName, PS_LOADICON, PLI_PROTOCOL, 0);
		}
 
		if (hIcon == NULL || (int)hIcon == CALLSERVICE_NOTFOUND) 
			hIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_NUDGE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

		ImageList_AddIcon(hImList, hIcon);
		DestroyIcon(hIcon);
	}
	//ADD default Icon for nudge
	HICON hIcon = NULL;
	hIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_NUDGE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	ImageList_AddIcon(hImList, hIcon);
	DestroyIcon(hIcon);

	HWND hLstView = GetDlgItem(hWnd, IDC_PROTOLIST);
	TreeView_SetImageList(hLstView, hImList, TVSIL_NORMAL);
}

void PopulateProtocolList(HWND hWnd)
{
	bool useOne = IsDlgButtonChecked(hWnd, IDC_USEBYPROTOCOL) == BST_UNCHECKED;

	HWND hLstView = GetDlgItem(hWnd, IDC_PROTOLIST);

	TreeView_DeleteAllItems(hLstView);

	TVINSERTSTRUCT tvi = {0};
	tvi.hParent = TVI_ROOT;
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_STATE | TVIF_SELECTEDIMAGE;
	tvi.item.stateMask = TVIS_STATEIMAGEMASK;

	NudgeElementList *n;
	int i = 0;
	if (GlobalNudge.useByProtocol)
	{
		for(n = NudgeList;n != NULL; n = n->next)
		{
			tvi.item.pszText = (TCHAR*)n->item.ProtocolName;
			tvi.item.iImage  = i;
			n->item.iProtoNumber = i;
			tvi.item.iSelectedImage = i;
			tvi.item.state = INDEXTOSTATEIMAGEMASK(n->item.enabled?2:1);	
			TreeView_InsertItem(hLstView, &tvi);
			i++;
		}
	}
	else
	{
		tvi.item.pszText = Translate("Nudge");
		tvi.item.iImage  = nProtocol;
		DefaultNudge.iProtoNumber = nProtocol;
		tvi.item.iSelectedImage = nProtocol;
		tvi.item.state = INDEXTOSTATEIMAGEMASK(DefaultNudge.enabled?2:1);	
		TreeView_InsertItem(hLstView, &tvi);

	}
	TreeView_SelectItem(hLstView, TreeView_GetRoot(hLstView));
	//TreeView_SetCheckState(hLstView, TreeView_GetRoot(hLstView), TRUE)
}


BOOL CALLBACK DlgProcNudgeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hwnd);
			CreateImageList(hwnd);
			PopulateProtocolList(hwnd);
			UpdateControls(hwnd);
			break;
		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);
			switch(LOWORD(wParam))
			{
				case IDC_PREVIEW:
					Preview();
					break;
				case IDC_POPUPTIME:
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_POPUPTEXTCOLOR:
				case IDC_POPUPBACKCOLOR:
					ActualNudge->popupBackColor = SendDlgItemMessage(hwnd,IDC_POPUPBACKCOLOR,CPM_GETCOLOUR,0,0);
					ActualNudge->popupTextColor = SendDlgItemMessage(hwnd,IDC_POPUPTEXTCOLOR,CPM_GETCOLOUR,0,0);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_USEWINCOLORS:
					ActualNudge->popupWindowColor = (IsDlgButtonChecked(hwnd,IDC_USEWINCOLORS)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR), ActualNudge->showPopup && ! ActualNudge->popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR), ActualNudge->showPopup && ! ActualNudge->popupWindowColor);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_CHECKPOP:
					ActualNudge->showPopup = (IsDlgButtonChecked(hwnd,IDC_CHECKPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORS),ActualNudge->showPopup);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR),ActualNudge->showPopup && ! ActualNudge->popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR),ActualNudge->showPopup && ! ActualNudge->popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIME),ActualNudge->showPopup);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_USEBYPROTOCOL:
					GlobalNudge.useByProtocol = (IsDlgButtonChecked(hwnd,IDC_USEBYPROTOCOL)==BST_CHECKED);
					PopulateProtocolList(hwnd);
					UpdateControls(hwnd);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_CHECKEVENT:
				case IDC_CHECKCLIST:
				case IDC_CHECKCHAT:
					ActualNudge->shakeClist = (IsDlgButtonChecked(hwnd,IDC_CHECKCLIST)==BST_CHECKED);
					ActualNudge->shakeChat = (IsDlgButtonChecked(hwnd,IDC_CHECKCHAT)==BST_CHECKED);
					ActualNudge->showEvent = (IsDlgButtonChecked(hwnd,IDC_CHECKEVENT)==BST_CHECKED);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
			}
			break;
		}
		case WM_SHOWWINDOW:
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch(((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							BOOL Translated;
							ActualNudge->popupTimeSec = GetDlgItemInt(hwnd,IDC_POPUPTIME,&Translated,FALSE);
							ActualNudge->popupWindowColor = (IsDlgButtonChecked(hwnd,IDC_USEWINCOLORS)==BST_CHECKED);
							ActualNudge->showPopup = (IsDlgButtonChecked(hwnd,IDC_CHECKPOP)==BST_CHECKED);
							ActualNudge->Save();
							GlobalNudge.Save();
						}
					}
				case IDC_PROTOLIST:
					switch (((LPNMHDR)lParam)->code)
					{
						case NM_CLICK:
							{
								TVHITTESTINFO ht = {0};

								DWORD dwpos = GetMessagePos();
								POINTSTOPOINT(ht.pt, MAKEPOINTS(dwpos));
								MapWindowPoints(HWND_DESKTOP, ((LPNMHDR)lParam)->hwndFrom, &ht.pt, 1);

								TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &ht);
								/*if (TVHT_ONITEM & ht.flags)
									CheckChange(hwnd,ht.hItem);*/
								if (TVHT_ONITEMSTATEICON & ht.flags)
									CheckChange(hwnd,ht.hItem);
								SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
							}

						case TVN_KEYDOWN:
							 if (((LPNMTVKEYDOWN) lParam)->wVKey == VK_SPACE)
									CheckChange(hwnd, TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
							break;

						case TVN_SELCHANGEDA:
						case TVN_SELCHANGEDW:
							{
								LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) lParam;
								if (pnmtv->itemNew.state & TVIS_SELECTED)
									UpdateControls(hwnd);
							}
							break;
					}
					break;
			}
			break;
	}

	return FALSE;
}

void CheckChange(HWND hwnd, HTREEITEM hItem)
{
	HWND hLstView = GetDlgItem(hwnd, IDC_PROTOLIST);
	bool isChecked = !TreeView_GetCheckState(hLstView, hItem);

	TreeView_SelectItem(hLstView, hItem);
		
	int proto = nProtocol;
	NudgeElementList *n;
	if (GlobalNudge.useByProtocol)
	{
		proto = GetSelProto(hwnd, hItem);
		for(n = NudgeList;n != NULL; n = n->next)
		{
			if(n->item.iProtoNumber == proto)
				ActualNudge = &n->item;
		}
	}
	else
		ActualNudge = &DefaultNudge;

	ActualNudge->enabled = isChecked;
	UpdateControls(hwnd);
}

void UpdateControls(HWND hwnd)
{
	int proto = nProtocol;
	NudgeElementList *n;
	if (GlobalNudge.useByProtocol)
	{
		proto = GetSelProto(hwnd,NULL);
		for(n = NudgeList;n != NULL; n = n->next)
		{
			if(n->item.iProtoNumber == proto)
				ActualNudge = &n->item;
		}
	}
	else
		ActualNudge = &DefaultNudge;

	CheckDlgButton(hwnd, IDC_CHECKPOP, (WPARAM) ActualNudge->showPopup);
	CheckDlgButton(hwnd, IDC_USEWINCOLORS, (WPARAM) ActualNudge->popupWindowColor);
	CheckDlgButton(hwnd, IDC_CHECKCLIST, (WPARAM) ActualNudge->shakeClist);
	CheckDlgButton(hwnd, IDC_CHECKCHAT, (WPARAM) ActualNudge->shakeChat);
	CheckDlgButton(hwnd, IDC_CHECKEVENT, (WPARAM) ActualNudge->showEvent);
	SetDlgItemInt(hwnd, IDC_POPUPTIME, ActualNudge->popupTimeSec,FALSE);
	CheckDlgButton(hwnd, IDC_USEBYPROTOCOL, (WPARAM) GlobalNudge.useByProtocol);
	SendDlgItemMessage(hwnd, IDC_POPUPBACKCOLOR, CPM_SETCOLOUR,0, ActualNudge->popupBackColor);
	SendDlgItemMessage(hwnd, IDC_POPUPBACKCOLOR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_BTNFACE));
	SendDlgItemMessage(hwnd, IDC_POPUPTEXTCOLOR, CPM_SETCOLOUR,0, ActualNudge->popupTextColor);
	SendDlgItemMessage(hwnd, IDC_POPUPTEXTCOLOR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_WINDOWTEXT));
	EnableWindow(GetDlgItem(hwnd, IDC_USEWINCOLORS), ActualNudge->showPopup);
	EnableWindow(GetDlgItem(hwnd, IDC_POPUPBACKCOLOR), ActualNudge->showPopup && ! ActualNudge->popupWindowColor);
	EnableWindow(GetDlgItem(hwnd, IDC_POPUPTEXTCOLOR), ActualNudge->showPopup && ! ActualNudge->popupWindowColor);
	EnableWindow(GetDlgItem(hwnd, IDC_POPUPTIME), ActualNudge->showPopup);

}

int GetSelProto(HWND hwnd, HTREEITEM hItem)
{
	HWND hLstView = GetDlgItem(hwnd, IDC_PROTOLIST);
	TVITEM tvi = {0};

	tvi.mask = TVIF_IMAGE;
	tvi.hItem = hItem == NULL ? TreeView_GetSelection(hLstView) : hItem;

	TreeView_GetItem(hLstView, &tvi);

	return tvi.iImage;
}