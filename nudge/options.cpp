#include "headers.h"
#include "shake.h"
#include "main.h"

extern HINSTANCE hInst;
extern bool Shaking;
extern bool ShakingChat;
extern bool bShakeClist;
extern bool bShakeChat;
extern int nScaleClist; 
extern int nScaleChat;
extern int nMoveClist; 
extern int nMoveChat;
extern bool bShowPopup;
extern COLORREF colorBack;
extern COLORREF colorText;
extern int popupTime;
extern bool bUseWindowColor;
extern CNudgeElement* NudgeList;

static BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcNudgeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK DlgProcShakeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

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
         MoveWindow((HWND)tci.lParam,5,26,rcClient.right-8,rcClient.bottom-29,1);

         tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_SHAKE),hwnd,DlgProcShakeOpt);
         tci.pszText = TranslateT("Window Shaking");
         TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 1, &tci);
         MoveWindow((HWND)tci.lParam,5,26,rcClient.right-8,rcClient.bottom-29,1);
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
						CNudgeElement *n;
						for(n = NudgeList;n != NULL; n = n->next)
						{
							n->popupTimeSec = popupTime;
							n->showPopup = bShowPopup;
							n->popupBackColor = colorBack;
							n->popupTextColor = colorText;
							n->popupWindowColor = bUseWindowColor;
						}
						
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
			_snprintf(szBuf, 10, "%d", nMoveClist);
			SetWindowTextA(GetDlgItem(hwnd, IDC_LNUMBER_CLIST), szBuf);
			_snprintf(szBuf, 10, "%d", nMoveChat);
			SetWindowTextA(GetDlgItem(hwnd, IDC_LNUMBER_CHAT), szBuf);

			_snprintf(szBuf, 10, "%d", nScaleClist);
			SetWindowTextA(GetDlgItem(hwnd, IDC_LSCALE_CLIST), szBuf);
			_snprintf(szBuf, 10, "%d", nScaleChat);
			SetWindowTextA(GetDlgItem(hwnd, IDC_LSCALE_CHAT), szBuf);

			SendDlgItemMessage(hwnd, IDC_SNUMBER_CLIST, TBM_SETRANGE, 0, (LPARAM)MAKELONG(1, 60));
			SendDlgItemMessage(hwnd, IDC_SNUMBER_CHAT, TBM_SETRANGE, 0, (LPARAM)MAKELONG(1, 60));

			SendDlgItemMessage(hwnd, IDC_SNUMBER_CLIST, TBM_SETPOS, TRUE, nMoveClist);
			SendDlgItemMessage(hwnd, IDC_SNUMBER_CHAT, TBM_SETPOS, TRUE, nMoveChat);

			SendDlgItemMessage(hwnd, IDC_SSCALE_CLIST, TBM_SETRANGE, 0, (LPARAM)MAKELONG(1, 40));
			SendDlgItemMessage(hwnd, IDC_SSCALE_CHAT, TBM_SETRANGE, 0, (LPARAM)MAKELONG(1, 40));

			SendDlgItemMessage(hwnd, IDC_SSCALE_CLIST, TBM_SETPOS, TRUE, nMoveClist);
			SendDlgItemMessage(hwnd, IDC_SSCALE_CHAT, TBM_SETPOS, TRUE, nMoveChat);

			CheckDlgButton(hwnd,IDC_CHECKCLIST,bShakeClist);
			CheckDlgButton(hwnd,IDC_CHECKCHAT,bShakeChat);
			EnableWindow(GetDlgItem(hwnd,IDC_SSCALE_CLIST),IsDlgButtonChecked(hwnd,IDC_CHECKCLIST)==BST_CHECKED);
			EnableWindow(GetDlgItem(hwnd,IDC_SSCALE_CHAT),IsDlgButtonChecked(hwnd,IDC_CHECKCHAT)==BST_CHECKED);
			EnableWindow(GetDlgItem(hwnd,IDC_SNUMBER_CLIST),(IsDlgButtonChecked(hwnd,IDC_CHECKCLIST)==BST_CHECKED));
			EnableWindow(GetDlgItem(hwnd,IDC_SNUMBER_CHAT),(IsDlgButtonChecked(hwnd,IDC_CHECKCHAT)==BST_CHECKED));
			EnableWindow(GetDlgItem(hwnd,IDC_LSCALE_CLIST),(IsDlgButtonChecked(hwnd,IDC_CHECKCLIST)==BST_CHECKED));
			EnableWindow(GetDlgItem(hwnd,IDC_LSCALE_CHAT),(IsDlgButtonChecked(hwnd,IDC_CHECKCHAT)==BST_CHECKED));
			EnableWindow(GetDlgItem(hwnd,IDC_LNUMBER_CLIST),(IsDlgButtonChecked(hwnd,IDC_CHECKCLIST)==BST_CHECKED));
			EnableWindow(GetDlgItem(hwnd,IDC_LNUMBER_CHAT),(IsDlgButtonChecked(hwnd,IDC_CHECKCHAT)==BST_CHECKED));
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
				case IDC_CHECKCLIST:
				case IDC_CHECKCHAT:
					EnableWindow(GetDlgItem(hwnd,IDC_SSCALE_CLIST),IsDlgButtonChecked(hwnd,IDC_CHECKCLIST)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_SSCALE_CHAT),IsDlgButtonChecked(hwnd,IDC_CHECKCHAT)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_SNUMBER_CLIST),(IsDlgButtonChecked(hwnd,IDC_CHECKCLIST)==BST_CHECKED));
					EnableWindow(GetDlgItem(hwnd,IDC_SNUMBER_CHAT),(IsDlgButtonChecked(hwnd,IDC_CHECKCHAT)==BST_CHECKED));
					EnableWindow(GetDlgItem(hwnd,IDC_LSCALE_CLIST),(IsDlgButtonChecked(hwnd,IDC_CHECKCLIST)==BST_CHECKED));
					EnableWindow(GetDlgItem(hwnd,IDC_LSCALE_CHAT),(IsDlgButtonChecked(hwnd,IDC_CHECKCHAT)==BST_CHECKED));
					EnableWindow(GetDlgItem(hwnd,IDC_LNUMBER_CLIST),(IsDlgButtonChecked(hwnd,IDC_CHECKCLIST)==BST_CHECKED));
					EnableWindow(GetDlgItem(hwnd,IDC_LNUMBER_CHAT),(IsDlgButtonChecked(hwnd,IDC_CHECKCHAT)==BST_CHECKED));
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
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
							nMoveClist = (int) SendMessage((HWND) GetDlgItem(hwnd, IDC_SNUMBER_CLIST), TBM_GETPOS, 0, 0);
							nMoveChat = (int) SendMessage((HWND) GetDlgItem(hwnd, IDC_SNUMBER_CHAT), TBM_GETPOS, 0, 0);
							nScaleClist = (int) SendMessage((HWND) GetDlgItem(hwnd, IDC_SSCALE_CLIST), TBM_GETPOS, 0, 0);
							nScaleChat = (int) SendMessage((HWND) GetDlgItem(hwnd, IDC_SSCALE_CHAT), TBM_GETPOS, 0, 0);
							bShakeClist = (IsDlgButtonChecked(hwnd,IDC_CHECKCLIST)==BST_CHECKED);
							bShakeChat = (IsDlgButtonChecked(hwnd,IDC_CHECKCHAT)==BST_CHECKED);
						}
					}
			}
			break;
	}

	return FALSE;
}

BOOL CALLBACK DlgProcNudgeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hwnd);
			CheckDlgButton(hwnd,IDC_CHECKPOP, (WPARAM) bShowPopup);
			CheckDlgButton(hwnd,IDC_USEWINCOLORS, (WPARAM) bUseWindowColor);
			SetDlgItemInt(hwnd,IDC_POPUPTIME, popupTime,FALSE);
			SendDlgItemMessage(hwnd,IDC_POPUPBACKCOLOR,CPM_SETCOLOUR,0,(LPARAM) colorBack);
			SendDlgItemMessage(hwnd,IDC_POPUPTEXTCOLOR,CPM_SETCOLOUR,0,(LPARAM) colorText);
			EnableWindow(GetDlgItem(hwnd,IDC_CHECKPOP),bShowPopup);
			EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORS),bUseWindowColor);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR),bShowPopup && ! bUseWindowColor);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR),bShowPopup && ! bUseWindowColor);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIME),bShowPopup);
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
				case IDC_POPUPTEXTCOLOR:
				case IDC_POPUPBACKCOLOR:
					colorBack = SendDlgItemMessage(hwnd,IDC_POPUPBACKCOLOR,CPM_GETCOLOUR,0,0);
					colorText = SendDlgItemMessage(hwnd,IDC_POPUPTEXTCOLOR,CPM_GETCOLOUR,0,0);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_USEWINCOLORS:
					bUseWindowColor = (IsDlgButtonChecked(hwnd,IDC_USEWINCOLORS)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR), bShowPopup && ! bUseWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR), bShowPopup && ! bUseWindowColor);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_CHECKPOP:
					bShowPopup = (IsDlgButtonChecked(hwnd,IDC_CHECKPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORS),bShowPopup);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR),bShowPopup && ! bUseWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR),bShowPopup && ! bUseWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIME),bShowPopup);
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
							popupTime = GetDlgItemInt(hwnd,IDC_POPUPTIME,&Translated,FALSE);
							bUseWindowColor = (IsDlgButtonChecked(hwnd,IDC_USEWINCOLORS)==BST_CHECKED);
							bShowPopup = (IsDlgButtonChecked(hwnd,IDC_CHECKPOP)==BST_CHECKED);
						}
					}
			}
			break;
	}

	return FALSE;
}