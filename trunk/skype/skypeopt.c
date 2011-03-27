#include "skype.h"
#include "skypeopt.h"
#include "pthread.h"
#include "gchat.h"
#include "skypeprofile.h"
#if(WINVER >= 0x0500)
#include "uxtheme.h"
#define HAVE_UXTHEMES
#endif

#ifdef SKYPE_AUTO_DETECTION
#include "ezxml/ezxml.c"
#endif

extern HINSTANCE hInst;
extern PLUGININFO pluginInfo;
extern char protocol;
extern BOOL SkypeInitialized, bProtocolSet;
extern DWORD mirandaVersion;

BOOL showPopup, showPopupErr, popupWindowColor, popupWindowColorErr;
unsigned int popupBackColor, popupBackColorErr;
unsigned int popupTextColor, popupTextColorErr;
int popupTimeSec, popupTimeSecErr;
POPUPDATAT InCallPopup;
POPUPDATAT ErrorPopup;

static SkypeProfile myProfile;
static HBITMAP hAvatar = NULL;

extern BOOL (WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD);

int RegisterOptions(WPARAM wParam, LPARAM lParam) {
   OPTIONSDIALOGPAGE odp;
   
   ZeroMemory(&odp, sizeof(odp));
   odp.cbSize = sizeof(odp);
   odp.hInstance = hInst;
   odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
   odp.ptszGroup = LPGENT("Network");
   odp.ptszTitle = LPGENT("Skype");
   odp.pfnDlgProc = OptionsDlgProc;
   odp.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;
   CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

   odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP);
   odp.ptszGroup = LPGENT("Popups");
   odp.ptszTitle = LPGENT("Skype");
   odp.pfnDlgProc = OptPopupDlgProc;
   CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
   return 0;
}

INT_PTR CALLBACK OptPopupDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static RECT r;

	switch ( msg ) 
	{
		case WM_INITDIALOG:
			TranslateDialogDefault( hwnd );
			// Message Popup
			popupTimeSec = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupTimeSec", 4);
			popupTextColor = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupTextColor", GetSysColor(COLOR_WINDOWTEXT));
			popupBackColor = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupBackColor", GetSysColor(COLOR_BTNFACE));
			popupWindowColor = DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "popupWindowColor", FALSE);
			showPopup = DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "showPopup", TRUE);
			// ERROR Message Popup
			popupTimeSecErr = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupTimeSecErr", 4);
			popupTextColorErr = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupTextColorErr", GetSysColor(COLOR_WINDOWTEXT));
			popupBackColorErr = DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "popupBackColorErr", GetSysColor(COLOR_BTNFACE));
			popupWindowColorErr = DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "popupWindowColorErr", FALSE);
			showPopupErr = DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "showPopupErr", TRUE);

			EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORS),showPopup);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR),showPopup && ! popupWindowColor);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR),showPopup && ! popupWindowColor);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIME),showPopup);
			EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORSERR),showPopupErr);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLORERR),showPopupErr && ! popupWindowColorErr);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLORERR),showPopupErr && ! popupWindowColorErr);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIMEERR),showPopupErr);
			CheckDlgButton(hwnd, IDC_POPUPINCOMING, (WPARAM) showPopup);
			CheckDlgButton(hwnd, IDC_USEWINCOLORS, (WPARAM) popupWindowColor);
			CheckDlgButton(hwnd, IDC_POPUPERROR, (WPARAM) showPopupErr);
			CheckDlgButton(hwnd, IDC_USEWINCOLORSERR, (WPARAM) popupWindowColorErr);
			SetDlgItemInt(hwnd, IDC_POPUPTIME, popupTimeSec,FALSE);
			SetDlgItemInt(hwnd, IDC_POPUPTIMEERR, popupTimeSecErr,FALSE);
			SendDlgItemMessage(hwnd, IDC_POPUPBACKCOLOR, CPM_SETCOLOUR,0, popupBackColor);
			SendDlgItemMessage(hwnd, IDC_POPUPBACKCOLOR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_BTNFACE));
			SendDlgItemMessage(hwnd, IDC_POPUPTEXTCOLOR, CPM_SETCOLOUR,0, popupTextColor);
			SendDlgItemMessage(hwnd, IDC_POPUPTEXTCOLOR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_WINDOWTEXT));
			SendDlgItemMessage(hwnd, IDC_POPUPBACKCOLORERR, CPM_SETCOLOUR,0, popupBackColorErr);
			SendDlgItemMessage(hwnd, IDC_POPUPBACKCOLORERR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_BTNFACE));
			SendDlgItemMessage(hwnd, IDC_POPUPTEXTCOLORERR, CPM_SETCOLOUR,0, popupTextColorErr);
			SendDlgItemMessage(hwnd, IDC_POPUPTEXTCOLORERR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_WINDOWTEXT));


			return TRUE;
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
							DBWriteContactSettingDword(NULL, SKYPE_PROTONAME, "popupBackColor", popupBackColor);
							DBWriteContactSettingDword(NULL, SKYPE_PROTONAME, "popupTextColor", popupTextColor);
							DBWriteContactSettingDword(NULL, SKYPE_PROTONAME, "popupTimeSec", popupTimeSec);
							DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "popupWindowColor", (BYTE)popupWindowColor);
							DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "showPopup", (BYTE)showPopup);
							DBWriteContactSettingDword(NULL, SKYPE_PROTONAME, "popupBackColorErr", popupBackColorErr);
							DBWriteContactSettingDword(NULL, SKYPE_PROTONAME, "popupTextColorErr", popupTextColorErr);
							DBWriteContactSettingDword(NULL, SKYPE_PROTONAME, "popupTimeSecErr", popupTimeSecErr);
							DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "popupWindowColorErr", (BYTE)popupWindowColorErr);
							DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "showPopupErr", (BYTE)showPopupErr);
							break;
					}
			}
			break;

		

		case WM_COMMAND:
			switch( LOWORD( wParam )) 
			{
				case IDC_PREVIEW:
				{
					HANDLE hContact;
					TCHAR * lpzContactName;

					hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST,0,0);
					lpzContactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,GCDNF_TCHAR);
					InCallPopup.lchContact = hContact;
					InCallPopup.lchIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALL));
					InCallPopup.colorBack = ! popupWindowColor ? popupBackColor : GetSysColor(COLOR_BTNFACE);
					InCallPopup.colorText = ! popupWindowColor ? popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
					InCallPopup.iSeconds = popupTimeSec;
					InCallPopup.PluginData = (void *)1;
					
					lstrcpy(InCallPopup.lptzText, TranslateT("Incoming Skype Call"));

					lstrcpy(InCallPopup.lptzContactName, lpzContactName);

					CallService(MS_POPUP_ADDPOPUPT,(WPARAM)&InCallPopup,0);


					break;
				}
				case IDC_PREVIEWERR:					
					ErrorPopup.lchContact = NULL;
					ErrorPopup.lchIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALL));
					ErrorPopup.colorBack = ! popupWindowColorErr ? popupBackColorErr : GetSysColor(COLOR_BTNFACE);
					ErrorPopup.colorText = ! popupWindowColorErr ? popupTextColorErr : GetSysColor(COLOR_WINDOWTEXT);
					ErrorPopup.iSeconds = popupTimeSecErr;
					ErrorPopup.PluginData = (void *)1;
					
					lstrcpy(ErrorPopup.lptzText, TranslateT("Preview Error Message"));

					lstrcpy(ErrorPopup.lptzContactName, _T("Error Message"));


					CallService(MS_POPUP_ADDPOPUPT,(WPARAM)&ErrorPopup,0);

					break;

				case IDC_POPUPTIME:
				case IDC_POPUPTIMEERR:
				{
					BOOL Translated;
					popupTimeSec = GetDlgItemInt(hwnd,IDC_POPUPTIME,&Translated,FALSE);
					popupTimeSecErr = GetDlgItemInt(hwnd,IDC_POPUPTIMEERR,&Translated,FALSE);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				}
				case IDC_POPUPTEXTCOLOR:
				case IDC_POPUPBACKCOLOR:
				case IDC_POPUPTEXTCOLORERR:
				case IDC_POPUPBACKCOLORERR:
					popupBackColor = SendDlgItemMessage(hwnd,IDC_POPUPBACKCOLOR,CPM_GETCOLOUR,0,0);
					popupTextColor = SendDlgItemMessage(hwnd,IDC_POPUPTEXTCOLOR,CPM_GETCOLOUR,0,0);
					popupBackColorErr = SendDlgItemMessage(hwnd,IDC_POPUPBACKCOLORERR,CPM_GETCOLOUR,0,0);
					popupTextColorErr = SendDlgItemMessage(hwnd,IDC_POPUPTEXTCOLORERR,CPM_GETCOLOUR,0,0);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_USEWINCOLORS:
					popupWindowColor = (IsDlgButtonChecked(hwnd,IDC_USEWINCOLORS)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR), showPopup && ! popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR), showPopup && ! popupWindowColor);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_POPUPINCOMING:
					showPopup = (IsDlgButtonChecked(hwnd,IDC_POPUPINCOMING)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORS),showPopup);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR),showPopup && ! popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR),showPopup && ! popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIME),showPopup);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_USEWINCOLORSERR:
					popupWindowColorErr = (IsDlgButtonChecked(hwnd,IDC_USEWINCOLORSERR)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLORERR), showPopupErr && ! popupWindowColorErr);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLORERR), showPopupErr && ! popupWindowColorErr);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_POPUPERROR:
					showPopupErr = (IsDlgButtonChecked(hwnd,IDC_POPUPERROR)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORSERR),showPopupErr);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLORERR),showPopupErr && ! popupWindowColorErr);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLORERR),showPopupErr && ! popupWindowColorErr);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIMEERR),showPopupErr);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
			}	
			
			break;

		case WM_DESTROY:
			break;
	}

	return 0;
}

static INT_PTR CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
         tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_DEFAULT), hwnd, OptionsDefaultDlgProc);
         tci.pszText = TranslateT("Skype default");
		 TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 0, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-5,rcClient.bottom-31,1);
#ifdef HAVE_UXTHEMES
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);
#endif

         tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_ADVANCED),hwnd,OptionsAdvancedDlgProc);
         tci.pszText = TranslateT("Skype advanced");
         TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 1, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-5,rcClient.bottom-31,1);
         ShowWindow((HWND)tci.lParam, SW_HIDE);
#ifdef HAVE_UXTHEMES
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);
#endif

		 tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_PROXY),hwnd,OptionsProxyDlgProc);
         tci.pszText = TranslateT("Skype proxy");
         TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 2, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-5,rcClient.bottom-31,1);
         ShowWindow((HWND)tci.lParam, SW_HIDE);
#ifdef HAVE_UXTHEMES
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);
#endif

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

INT_PTR CALLBACK OptionsProxyDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int Skype2SocketControls[]={IDC_HOST, IDC_PORT, IDC_REQPASS, IDC_PASSWORD};
	static BOOL initDlg=FALSE;
	DBVARIANT dbv;
	int i;
	
	switch (uMsg){
		case WM_INITDIALOG:	
			initDlg=TRUE;
			TranslateDialogDefault(hwndDlg);
			if (!DBGetContactSettingString(NULL, SKYPE_PROTONAME, "Host", &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_HOST, dbv.pszVal);
				DBFreeVariant(&dbv);
			} else SetDlgItemText(hwndDlg, IDC_HOST, _T("localhost"));
			SetDlgItemInt(hwndDlg, IDC_PORT, DBGetContactSettingWord(NULL, SKYPE_PROTONAME, "Port", 1401), FALSE);
			CheckDlgButton(hwndDlg, IDC_REQPASS, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "RequiresPassword", 0));
			CheckDlgButton(hwndDlg, IDC_USES2S, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseSkype2Socket", 0));
			if (!DBGetContactSettingString(NULL, SKYPE_PROTONAME, "Password", &dbv)) {
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal)+1, (LPARAM)dbv.pszVal);
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			SendMessage(hwndDlg, WM_COMMAND, IDC_USES2S, 0);
			SendMessage(hwndDlg, WM_COMMAND, IDC_REQPASS, 0);
			initDlg=FALSE;
			return TRUE;
		case WM_NOTIFY: {
			NMHDR* nmhdr = (NMHDR*)lParam;

			switch (nmhdr->code){
				case PSN_APPLY:
				case PSN_KILLACTIVE:
				{
					char buf[1024];
					GetDlgItemTextA(hwndDlg, IDC_HOST, buf, sizeof(buf));
					DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "Host", buf);
					DBWriteContactSettingWord(NULL, SKYPE_PROTONAME, "Port", (unsigned short)GetDlgItemInt(hwndDlg, IDC_PORT, NULL, FALSE));
					DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "RequiresPassword", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_REQPASS), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "UseSkype2Socket", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_USES2S), BM_GETCHECK,0,0)));
					ZeroMemory(buf, sizeof(buf));
					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, buf, sizeof(buf));
					CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(buf), (LPARAM)buf);
					DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "Password", buf);
					return TRUE;
				}
			}			
			break; 
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_USES2S:
					for (i=0; i<sizeof(Skype2SocketControls)/sizeof(Skype2SocketControls[0]); i++) EnableWindow(GetDlgItem(hwndDlg, Skype2SocketControls[i]), SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0));
					if (SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0)) SendMessage(hwndDlg, WM_COMMAND, IDC_REQPASS, 0);
					break;
				case IDC_REQPASS:
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0));
					break;

			}
			if (!initDlg) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
	}
	return 0;
}

INT_PTR CALLBACK OptionsAdvancedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int StartControls[]={IDC_NOSPLASH, IDC_MINIMIZED, IDC_NOTRAY};
	static BOOL initDlg=FALSE;
	static int statusModes[]={ID_STATUS_OFFLINE,ID_STATUS_ONLINE,ID_STATUS_AWAY,ID_STATUS_NA,ID_STATUS_OCCUPIED,ID_STATUS_DND,ID_STATUS_FREECHAT,ID_STATUS_INVISIBLE,ID_STATUS_OUTTOLUNCH,ID_STATUS_ONTHEPHONE};
	int i, j;
	
	switch (uMsg){
		case WM_INITDIALOG:	
			initDlg=TRUE;

			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_ENABLEMENU, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "EnableMenu", 1));
			CheckDlgButton(hwndDlg, IDC_NOERRORS, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "SuppressErrors", 0));
			CheckDlgButton(hwndDlg, IDC_KEEPSTATE, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "KeepState", 0));
			CheckDlgButton(hwndDlg, IDC_TIMEZONE, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseTimeZonePatch", 0));
			CheckDlgButton(hwndDlg, IDC_IGNTZ, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "IgnoreTimeZones", 0));
			CheckDlgButton(hwndDlg, IDC_SHOWDEFAULTAVATAR, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "ShowDefaultSkypeAvatar", 0));
			CheckDlgButton(hwndDlg, IDC_SUPPRESSCALLSUMMARYMESSAGE, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "SuppressCallSummaryMessage", 1));

			if (ServiceExists(MS_GC_NEWSESSION) && (!bProtocolSet || protocol>=5)) {
				CheckDlgButton(hwndDlg, IDC_GROUPCHAT, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseGroupchat", 0));
				CheckDlgButton(hwndDlg, IDC_GROUPCHATREAD, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "MarkGroupchatRead", 0));
			} else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHATREAD), FALSE);
			}

#ifdef USEPOPUP
			if (ServiceExists(MS_POPUP_ADDPOPUP))
				CheckDlgButton(hwndDlg, IDC_USEPOPUP, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UsePopup", 0));
			else
#endif
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEPOPUP), FALSE);

			j=DBGetContactSettingDword(NULL, SKYPE_PROTONAME, "SkypeOutStatusMode", ID_STATUS_ONTHEPHONE);
			for(i=0;i<sizeof(statusModes)/sizeof(statusModes[0]);i++) {
				int k;

				k=SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_ADDSTRING,0,(LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,statusModes[i],GCMDF_TCHAR));
				SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_SETITEMDATA,k,statusModes[i]);
				if (statusModes[i]==j) SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_SETCURSEL,i,0);
			}
			initDlg=FALSE;
			return TRUE;

		case WM_NOTIFY: {
			NMHDR* nmhdr = (NMHDR*)lParam;

			switch (nmhdr->code){
				case PSN_APPLY:
				case PSN_KILLACTIVE:
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "EnableMenu", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_ENABLEMENU), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "UsePopup", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_USEPOPUP), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "UseGroupchat", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_GROUPCHAT), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "MarkGroupchatRead", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_GROUPCHATREAD), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "SuppressErrors", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOERRORS), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "KeepState", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_KEEPSTATE), BM_GETCHECK,0,0)));
					DBWriteContactSettingDword(NULL, SKYPE_PROTONAME, "SkypeOutStatusMode", SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_GETITEMDATA,SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_GETCURSEL,0,0),0));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "UseTimeZonePatch", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_TIMEZONE), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "IgnoreTimeZones", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_IGNTZ), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "ShowDefaultSkypeAvatar", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_SHOWDEFAULTAVATAR), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "SuppressCallSummaryMessage", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_SUPPRESSCALLSUMMARYMESSAGE), BM_GETCHECK,0,0)));
					return TRUE;
			}			
			break; 
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_CLEANUP:
					pthread_create(( pThreadFunc )CleanupNicknames, NULL);
					break;
			}
			if (!initDlg) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
	}
	return 0;
}

INT_PTR CALLBACK OptionsDefaultDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int StartControls[]={IDC_NOSPLASH, IDC_MINIMIZED, IDC_NOTRAY};
	static BOOL initDlg=FALSE;
	static int statusModes[]={ID_STATUS_OFFLINE,ID_STATUS_ONLINE,ID_STATUS_AWAY,ID_STATUS_NA,ID_STATUS_OCCUPIED,ID_STATUS_DND,ID_STATUS_FREECHAT,ID_STATUS_INVISIBLE,ID_STATUS_OUTTOLUNCH,ID_STATUS_ONTHEPHONE};
	
	switch (uMsg){
		case WM_INITDIALOG:	
		{
			DBVARIANT dbv;

			initDlg=TRUE;
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_STARTSKYPE, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "StartSkype", 1));
			CheckDlgButton(hwndDlg, IDC_NOSPLASH, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "nosplash", 1));
			CheckDlgButton(hwndDlg, IDC_MINIMIZED, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "minimized", 1));
			CheckDlgButton(hwndDlg, IDC_NOTRAY, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "notray", 0));
			CheckDlgButton(hwndDlg, IDC_REMOVEABLE, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "removable", 0));
			CheckDlgButton(hwndDlg, IDC_DATAPATHO, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "datapath:", 0));
			CheckDlgButton(hwndDlg, IDC_SHUTDOWN, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "Shutdown", 0));
			CheckDlgButton(hwndDlg, IDC_UNLOADOFFLINE, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UnloadOnOffline", 0));
			
			CheckDlgButton(hwndDlg, IDC_CUSTOMCOMMAND, (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseCustomCommand", 0));
			if(!DBGetContactSettingString(NULL,SKYPE_PROTONAME,"CommandLine",&dbv)) 
			{
				SetWindowTextA(GetDlgItem(hwndDlg, IDC_COMMANDLINE), dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			if(!DBGetContactSettingString(NULL,SKYPE_PROTONAME,"datapath",&dbv)) 
			{
				SetWindowTextA(GetDlgItem(hwndDlg, IDC_DATAPATH), dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_COMMANDLINE), SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0));
			EnableWindow(GetDlgItem(hwndDlg, IDC_DATAPATH), SendMessage(GetDlgItem(hwndDlg, IDC_DATAPATHO), BM_GETCHECK,0,0));

            // LoginUserName
            if(!DBGetContactSettingWString(NULL,SKYPE_PROTONAME,"LoginUserName",&dbv)) 
			{
				SetWindowTextW(GetDlgItem(hwndDlg, IDC_USERNAME), dbv.pwszVal);
				DBFreeVariant(&dbv);
			}

            // LoginPassword
            if(!DBGetContactSettingWString(NULL,SKYPE_PROTONAME,"LoginPassword",&dbv)) 
			{
				SetWindowTextW(GetDlgItem(hwndDlg, IDC_PASSWORD), dbv.pwszVal);
				DBFreeVariant(&dbv);
			}

			SetDlgItemInt (hwndDlg, IDC_CONNATTEMPTS, DBGetContactSettingWord(NULL, SKYPE_PROTONAME, "ConnectionAttempts", 10), FALSE);
			SendMessage(hwndDlg, WM_COMMAND, IDC_STARTSKYPE, 0);
			initDlg=FALSE;
			return TRUE;
		}
		case WM_NOTIFY: {
			NMHDR* nmhdr = (NMHDR*)lParam;

			switch (nmhdr->code){
				case PSN_APPLY:
				case PSN_KILLACTIVE:
				{
					char text[500];
                    WCHAR wtext[500];

					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "StartSkype", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_STARTSKYPE), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "nosplash", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOSPLASH), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "minimized", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_MINIMIZED), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "notray", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOTRAY), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "Shutdown", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_SHUTDOWN), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "UnloadOnOffline", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_UNLOADOFFLINE), BM_GETCHECK,0,0)));
					DBWriteContactSettingWord (NULL, SKYPE_PROTONAME, "ConnectionAttempts", (unsigned short)GetDlgItemInt(hwndDlg, IDC_CONNATTEMPTS, NULL, FALSE));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "UseCustomCommand", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "datapath:", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_DATAPATHO), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "removable", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_REMOVEABLE), BM_GETCHECK,0,0)));
					GetDlgItemTextA(hwndDlg,IDC_COMMANDLINE,text,sizeof(text));
					DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "CommandLine", text);
					GetDlgItemTextA(hwndDlg,IDC_DATAPATH,text,sizeof(text));
					DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "datapath", text);

                   
                    // LoginUserName
                    GetDlgItemTextW(hwndDlg,IDC_USERNAME,wtext,sizeof(wtext)/sizeof(WCHAR));
					DBWriteContactSettingWString(NULL, SKYPE_PROTONAME, "LoginUserName", wtext);

                    // LoginPassword
                    GetDlgItemTextW(hwndDlg,IDC_PASSWORD,wtext,sizeof(wtext)/sizeof(WCHAR));
					DBWriteContactSettingWString(NULL, SKYPE_PROTONAME, "LoginPassword", wtext);

					return TRUE;
				}
			}			
			break; 
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_STARTSKYPE:
					break;
				case IDC_CLEANUP:
					pthread_create(( pThreadFunc )CleanupNicknames, NULL);
					break;
				case IDC_DATAPATHO:
					EnableWindow(GetDlgItem(hwndDlg, IDC_DATAPATH), SendMessage(GetDlgItem(hwndDlg, IDC_DATAPATHO), BM_GETCHECK,0,0));
					break;
				case IDC_CUSTOMCOMMAND:
					EnableWindow(GetDlgItem(hwndDlg, IDC_COMMANDLINE), SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0));
					break;
#ifdef SKYPE_AUTO_DETECTION
				case IDC_AUTODETECTION:
					DoAutoDetect(hwndDlg);
					break;
#endif
			}
			if (!initDlg) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnDetailsInit - initializes user info dialog pages.

int OnDetailsInit( WPARAM wParam, LPARAM lParam )
{
	OPTIONSDIALOGPAGE odp = {0};
	HANDLE hContact = ( HANDLE )lParam;

	odp.cbSize = sizeof(odp);
	odp.hIcon = NULL;
	odp.hInstance = hInst;

	if ( hContact == NULL ) {
		
		char szTitle[256];
		
		if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 7, 0, 27))
		{
			mir_snprintf( szTitle, sizeof( szTitle ), "%s %s", SKYPE_PROTONAME, Translate( "Avatar" ));

			odp.pfnDlgProc = AvatarDlgProc;
			odp.position = 1900000000;
			odp.pszTemplate = MAKEINTRESOURCEA(IDD_SETAVATAR);
			odp.pszTitle = szTitle;
			CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);
		}

		mir_snprintf( szTitle, sizeof( szTitle ), "%s %s", SKYPE_PROTONAME, Translate( "Details" ));
	
		odp.pfnDlgProc = DetailsDlgProc;
		odp.position = 1900000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_SETDETAILS);
		odp.pszTitle = szTitle;
		CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);
	}

	return 0;
}

/*AvatarDlgProc
*
* For setting the skype avatar
*
*/
INT_PTR CALLBACK AvatarDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static RECT r;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		hAvatar = NULL;
		if(ServiceExists(MS_AV_GETMYAVATAR)){
			struct avatarCacheEntry *ace = (struct avatarCacheEntry *)CallService(MS_AV_GETMYAVATAR, 0,(LPARAM) SKYPE_PROTONAME);
			if (ace!=NULL) {
				hAvatar = ( HBITMAP )CallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )ace->szFilename);
				if ( hAvatar != NULL )
					SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)hAvatar );
			}
		}


		
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_SETAVATAR:
			{
				char szFileName[ MAX_PATH ];
				if ( EnterBitmapFileName( szFileName ) != ERROR_SUCCESS )
					return FALSE;

				hAvatar = ( HBITMAP )CallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )szFileName);
				if ( hAvatar != NULL ){
					SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)hAvatar );
					CallService(SKYPE_SETAVATAR, 0, ( LPARAM )szFileName);
				}
				break;
			}
			case IDC_DELETEAVATAR:
				if ( hAvatar != NULL ) {
					DeleteObject( hAvatar );
					hAvatar = NULL;
					CallService(SKYPE_SETAVATAR, 0, 0);
				}
				DBDeleteContactSetting( NULL, SKYPE_PROTONAME, "AvatarFile" );
				InvalidateRect( hwndDlg, NULL, TRUE );
				break;
		}	}
		break;

	case WM_DESTROY:
		if ( hAvatar != NULL )
			DeleteObject( hAvatar );
		break;
	}

	return 0;
}

/*DetailsDlgProc
*
* For setting the skype infos
*
*/
INT_PTR CALLBACK DetailsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int sexM = 0,sexF = 0, sex;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		ZeroMemory (&myProfile, sizeof(myProfile));
		SkypeProfile_Load(&myProfile);
		if(SkypeInitialized)
			SkypeProfile_LoadFromSkype(&myProfile);

		SendDlgItemMessage(hwndDlg,IDC_SEX,CB_ADDSTRING,0,(LPARAM)_T(""));
		sexM = SendDlgItemMessage(hwndDlg,IDC_SEX,CB_ADDSTRING,0,(LPARAM)TranslateT("MALE"));
		sexF = SendDlgItemMessage(hwndDlg,IDC_SEX,CB_ADDSTRING,0,(LPARAM)TranslateT("FEMALE"));
		
		switch(myProfile.Sex) {
		case 0x4D: SendDlgItemMessage(hwndDlg,IDC_SEX,CB_SETCURSEL, sexM, 0); break;
		case 0x46: SendDlgItemMessage(hwndDlg,IDC_SEX,CB_SETCURSEL, sexF, 0); break;
		}

		SetDlgItemText(hwndDlg, IDC_FULLNAME, myProfile.FullName);
		SetDlgItemTextA(hwndDlg, IDC_HOMEPAGE, myProfile.HomePage);
		SetDlgItemTextA(hwndDlg, IDC_HOMEPHONE, myProfile.HomePhone);
		SetDlgItemTextA(hwndDlg, IDC_OFFICEPHONE, myProfile.OfficePhone);
		SetDlgItemText(hwndDlg, IDC_CITY, myProfile.City);
		SetDlgItemText(hwndDlg, IDC_PROVINCE, myProfile.Province);
		DateTime_SetSystemtime (GetDlgItem (hwndDlg, IDC_BIRTHDAY), GDT_VALID, &myProfile.Birthday);
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_SAVEDETAILS:
				GetDlgItemText(hwndDlg,IDC_FULLNAME,myProfile.FullName,sizeof(myProfile.FullName)/sizeof(TCHAR));
				GetDlgItemTextA(hwndDlg,IDC_HOMEPAGE,myProfile.HomePage,sizeof(myProfile.HomePage)/sizeof(TCHAR));
				GetDlgItemTextA(hwndDlg,IDC_HOMEPHONE,myProfile.HomePhone,sizeof(myProfile.HomePhone)/sizeof(TCHAR));
				GetDlgItemTextA(hwndDlg,IDC_OFFICEPHONE,myProfile.OfficePhone,sizeof(myProfile.OfficePhone)/sizeof(TCHAR));
				GetDlgItemText(hwndDlg,IDC_CITY,myProfile.City,sizeof(myProfile.City)/sizeof(TCHAR));
				GetDlgItemText(hwndDlg,IDC_PROVINCE,myProfile.Province,sizeof(myProfile.Province)/sizeof(TCHAR));
				sex = SendMessage(GetDlgItem(hwndDlg,IDC_SEX),CB_GETCURSEL,0,0);
				
				myProfile.Sex = 0;
				if(sex == sexF) myProfile.Sex = 0x46; else
				if(sex == sexM) myProfile.Sex = 0x4D;
				DateTime_GetSystemtime (GetDlgItem (hwndDlg, IDC_BIRTHDAY), &myProfile.Birthday);

				SkypeProfile_Save(&myProfile);
				if(SkypeInitialized)
					SkypeProfile_SaveToSkype(&myProfile);
				break;
			}	
		}
		break;

	case WM_DESTROY:
		if ( hAvatar != NULL )
			DeleteObject( hAvatar );
		break;
	}

	return 0;
}

#ifdef SKYPE_AUTO_DETECTION
/**
 * DoAutoDetect
 * @param dlg The default option dialog handle
 */
void DoAutoDetect(HWND dlg)
{
	char basePath[MAX_PATH];
	char fileName[MAX_PATH];
	char tmpUser[255];
	ezxml_t f1, acc;
	
	if (FAILED(SHGetFolderPath(dlg,CSIDL_APPDATA,NULL,0,basePath)))
	{
		OUTPUT("Error in retrieving appdata path!");
		return;
	}

	strcat(basePath,"\\Skype\\");
	sprintf (fileName, "%s\\shared.xml", basePath);

	if (f1 = ezxml_parse_file(fileName))
	{
 		if (acc = ezxml_get(f1, "Lib", 0, "Account", 0, "Default", -1))
		{
			if (GetWindowTextA(GetDlgItem(dlg,IDC_USERNAME),tmpUser,sizeof(tmpUser)))
				SetWindowTextA(GetDlgItem(dlg,IDC_USERNAME),acc->txt);
			/* Can't find this stuff in current Skype verions??
			sprintf (fileName, "%s\\%s\\config.xml", basePath, acc->txt);
			if ((acc = ezxml_get(f1, "UI", 0, "Messages", 0, "OpenWindowInCompactMode", -1)) && *acc->txt!='0')
			{
				ezxml_set_txt (acc, "0");
				// ezXML doesn't supprot saving yet
			}
			*/
		}
		ezxml_free(f1);
	}
	else
	{
		OUTPUT("Failed to open skypes configuration files!");
		return;
	}
}
#endif
