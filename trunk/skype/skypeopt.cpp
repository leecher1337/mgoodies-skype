#include "skype.h"
#include "skypeopt.h"
#include "pthread.h"
#include "gchat.h"
#include "skypeprofile.h"
#include "uxtheme.h"

extern HINSTANCE hInst;
extern PLUGININFO pluginInfo;
extern char pszSkypeProtoName[MAX_PATH+30],protocol;
extern BOOL SkypeInitialized;

CSkypeProfile myProfile;

static HBITMAP hAvatar = NULL;

extern BOOL (WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD);

int RegisterOptions(WPARAM wParam, LPARAM lParam) {
   OPTIONSDIALOGPAGE odp;
   
   ZeroMemory(&odp, sizeof(odp));
   odp.cbSize = sizeof(odp);
   odp.hInstance = hInst;
   odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
   odp.pszGroup = Translate("Network");
   odp.pszTitle = "Skype";
   odp.pfnDlgProc = OptionsDlgProc;
   CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
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
         tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_DEFAULT), hwnd, OptionsDefaultDlgProc);
         tci.pszText = TranslateT("Skype default");
		 TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 0, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-3,rcClient.bottom-29,1);
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

         tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_ADVANCED),hwnd,OptionsAdvancedDlgProc);
         tci.pszText = TranslateT("Skype advanced");
         TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 1, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-3,rcClient.bottom-29,1);
         ShowWindow((HWND)tci.lParam, SW_HIDE);
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

		 tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_PROXY),hwnd,OptionsProxyDlgProc);
         tci.pszText = TranslateT("Skype proxy");
         TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 2, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-3,rcClient.bottom-29,1);
         ShowWindow((HWND)tci.lParam, SW_HIDE);
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

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

int CALLBACK OptionsProxyDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int Skype2SocketControls[]={IDC_HOST, IDC_PORT, IDC_REQPASS, IDC_PASSWORD};
	static BOOL initDlg=FALSE;
	DBVARIANT dbv;
	int i;
	
	switch (uMsg){
		case WM_INITDIALOG:	
			initDlg=TRUE;
			TranslateDialogDefault(hwndDlg);
			if (!DBGetContactSetting(NULL, pszSkypeProtoName, "Host", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_HOST, dbv.pszVal);
				DBFreeVariant(&dbv);
			} else SetDlgItemText(hwndDlg, IDC_HOST, "localhost");
			SetDlgItemInt(hwndDlg, IDC_PORT, DBGetContactSettingWord(NULL, pszSkypeProtoName, "Port", 1401), FALSE);
			CheckDlgButton(hwndDlg, IDC_REQPASS, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "RequiresPassword", 0));
			CheckDlgButton(hwndDlg, IDC_USES2S, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseSkype2Socket", 0));
			if (!DBGetContactSetting(NULL, pszSkypeProtoName, "Password", &dbv)) {
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal)+1, (LPARAM)dbv.pszVal);
				SetDlgItemText(hwndDlg, IDC_PASSWORD, dbv.pszVal);
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
					char buf[1024];
					GetDlgItemText(hwndDlg, IDC_HOST, buf, sizeof(buf));
					DBWriteContactSettingString(NULL, pszSkypeProtoName, "Host", buf);
					DBWriteContactSettingWord(NULL, pszSkypeProtoName, "Port", (unsigned short)GetDlgItemInt(hwndDlg, IDC_PORT, NULL, FALSE));
					DBWriteContactSettingByte(NULL, pszSkypeProtoName, "RequiresPassword", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_REQPASS), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UseSkype2Socket", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_USES2S), BM_GETCHECK,0,0)));
					ZeroMemory(buf, sizeof(buf));
					GetDlgItemText(hwndDlg, IDC_PASSWORD, buf, sizeof(buf));
					CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(buf), (LPARAM)buf);
					DBWriteContactSettingString(NULL, pszSkypeProtoName, "Password", buf);
					return TRUE;
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

int CALLBACK OptionsAdvancedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int StartControls[]={IDC_NOSPLASH, IDC_MINIMIZED, IDC_NOTRAY};
	static BOOL initDlg=FALSE;
	static int statusModes[]={ID_STATUS_OFFLINE,ID_STATUS_ONLINE,ID_STATUS_AWAY,ID_STATUS_NA,ID_STATUS_OCCUPIED,ID_STATUS_DND,ID_STATUS_FREECHAT,ID_STATUS_INVISIBLE,ID_STATUS_OUTTOLUNCH,ID_STATUS_ONTHEPHONE};
	int i, j;
	
	switch (uMsg){
		case WM_INITDIALOG:	
			initDlg=TRUE;
			DBVARIANT dbv;

			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_ENABLEMENU, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "EnableMenu", 1));
			CheckDlgButton(hwndDlg, IDC_NOERRORS, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "SuppressErrors", 0));
			CheckDlgButton(hwndDlg, IDC_KEEPSTATE, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "KeepState", 0));

			if (ServiceExists(MS_GC_NEWCHAT) && atoi(SKYPE_PROTO+strlen(SKYPE_PROTO)-1)>=5)
				CheckDlgButton(hwndDlg, IDC_GROUPCHAT, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseGroupchat", 0));
			else
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT), FALSE);

#ifdef USEPOPUP
			if (ServiceExists(MS_POPUP_ADDPOPUP))
				CheckDlgButton(hwndDlg, IDC_USEPOPUP, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UsePopup", 0));
			else
#endif
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEPOPUP), FALSE);

			j=DBGetContactSettingDword(NULL, pszSkypeProtoName, "SkypeOutStatusMode", ID_STATUS_ONTHEPHONE);
			for(i=0;i<sizeof(statusModes)/sizeof(statusModes[0]);i++) {
				int k;

				k=SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_ADDSTRING,0,(LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,statusModes[i],0));
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
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "EnableMenu", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_ENABLEMENU), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UsePopup", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_USEPOPUP), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UseGroupchat", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_GROUPCHAT), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "SuppressErrors", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOERRORS), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "KeepState", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_KEEPSTATE), BM_GETCHECK,0,0)));
					DBWriteContactSettingDword(NULL, pszSkypeProtoName, "SkypeOutStatusMode", SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_GETITEMDATA,SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_GETCURSEL,0,0),0));
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

int CALLBACK OptionsDefaultDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int StartControls[]={IDC_NOSPLASH, IDC_MINIMIZED, IDC_NOTRAY};
	static BOOL initDlg=FALSE;
	static int statusModes[]={ID_STATUS_OFFLINE,ID_STATUS_ONLINE,ID_STATUS_AWAY,ID_STATUS_NA,ID_STATUS_OCCUPIED,ID_STATUS_DND,ID_STATUS_FREECHAT,ID_STATUS_INVISIBLE,ID_STATUS_OUTTOLUNCH,ID_STATUS_ONTHEPHONE};
	int i, j;
	
	switch (uMsg){
		case WM_INITDIALOG:	
			initDlg=TRUE;
			DBVARIANT dbv;

			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_STARTSKYPE, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "StartSkype", 1));
			CheckDlgButton(hwndDlg, IDC_NOSPLASH, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "nosplash", 1));
			CheckDlgButton(hwndDlg, IDC_MINIMIZED, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "minimized", 1));
			CheckDlgButton(hwndDlg, IDC_NOTRAY, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "notray", 0));
			CheckDlgButton(hwndDlg, IDC_REMOVEABLE, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "removeable", 0));
			CheckDlgButton(hwndDlg, IDC_DATAPATHO, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "datapatho", 0));
			CheckDlgButton(hwndDlg, IDC_SHUTDOWN, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "Shutdown", 0));
			CheckDlgButton(hwndDlg, IDC_UNLOADOFFLINE, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UnloadOnOffline", 0));
			
			CheckDlgButton(hwndDlg, IDC_CUSTOMCOMMAND, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseCustomCommand", 0));
			if(!DBGetContactSetting(NULL,pszSkypeProtoName,"CommandLine",&dbv)) 
			{
				SetWindowTextA(GetDlgItem(hwndDlg, IDC_COMMANDLINE), dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			if(!DBGetContactSetting(NULL,pszSkypeProtoName,"datapath",&dbv)) 
			{
				SetWindowTextA(GetDlgItem(hwndDlg, IDC_DATAPATH), dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_COMMANDLINE), SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0));
			EnableWindow(GetDlgItem(hwndDlg, IDC_DATAPATH), SendMessage(GetDlgItem(hwndDlg, IDC_DATAPATHO), BM_GETCHECK,0,0));
			//EnableWindow(GetDlgItem(hwndDlg, IDC_MINIMIZED), !SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0));
			//EnableWindow(GetDlgItem(hwndDlg, IDC_NOSPLASH), !SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0));
			//EnableWindow(GetDlgItem(hwndDlg, IDC_NOTRAY), !SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0));

			SetDlgItemInt (hwndDlg, IDC_CONNATTEMPTS, DBGetContactSettingWord(NULL, pszSkypeProtoName, "ConnectionAttempts", 10), FALSE);
			SendMessage(hwndDlg, WM_COMMAND, IDC_STARTSKYPE, 0);
			initDlg=FALSE;
			return TRUE;

		case WM_NOTIFY: {
			NMHDR* nmhdr = (NMHDR*)lParam;

			switch (nmhdr->code){
				case PSN_APPLY:
				case PSN_KILLACTIVE:
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "StartSkype", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_STARTSKYPE), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "nosplash", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOSPLASH), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "minimized", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_MINIMIZED), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "notray", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOTRAY), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "Shutdown", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_SHUTDOWN), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UnloadOnOffline", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_UNLOADOFFLINE), BM_GETCHECK,0,0)));
					DBWriteContactSettingWord (NULL, pszSkypeProtoName, "ConnectionAttempts", (unsigned short)GetDlgItemInt(hwndDlg, IDC_CONNATTEMPTS, NULL, FALSE));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UseCustomCommand", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "datapatho", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_DATAPATHO), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "removeable", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_REMOVEABLE), BM_GETCHECK,0,0)));
					char text[500];
					GetDlgItemText(hwndDlg,IDC_COMMANDLINE,text,sizeof(text));
					DBWriteContactSettingString(NULL, pszSkypeProtoName, "CommandLine", text);
					GetDlgItemText(hwndDlg,IDC_DATAPATH,text,sizeof(text));
					DBWriteContactSettingString(NULL, pszSkypeProtoName, "datapath", text);
					return TRUE;
			}			
			break; 
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_STARTSKYPE:
					for (i=0; i<sizeof(StartControls)/sizeof(StartControls[0]); i++) EnableWindow(GetDlgItem(hwndDlg, StartControls[i]), SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0));
					break;
				case IDC_CLEANUP:
					pthread_create(( pThreadFunc )CleanupNicknames, NULL);
					break;
				case IDC_DATAPATHO:
					EnableWindow(GetDlgItem(hwndDlg, IDC_DATAPATH), SendMessage(GetDlgItem(hwndDlg, IDC_DATAPATHO), BM_GETCHECK,0,0));
					break;
				case IDC_CUSTOMCOMMAND:
					EnableWindow(GetDlgItem(hwndDlg, IDC_COMMANDLINE), SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0));
					//EnableWindow(GetDlgItem(hwndDlg, IDC_MINIMIZED), !SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0));
					//EnableWindow(GetDlgItem(hwndDlg, IDC_NOSPLASH), !SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0));
					//EnableWindow(GetDlgItem(hwndDlg, IDC_NOTRAY), !SendMessage(GetDlgItem(hwndDlg, IDC_CUSTOMCOMMAND), BM_GETCHECK,0,0));
					break;

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
	odp.cbSize = sizeof(odp);
	odp.hIcon = NULL;
	odp.hInstance = hInst;

	HANDLE hContact = ( HANDLE )lParam;
	if ( hContact == NULL ) {
		
		char szTitle[256];
		mir_snprintf( szTitle, sizeof( szTitle ), "Skype %s", Translate( "Avatar" ));

		odp.pfnDlgProc = AvatarDlgProc;
		odp.position = 1900000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_SETAVATAR);
		odp.pszTitle = szTitle;
		CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);

		mir_snprintf( szTitle, sizeof( szTitle ), "Skype %s", Translate( "Details" ));
	
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
BOOL CALLBACK AvatarDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static RECT r;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		hAvatar = NULL;
		if(ServiceExists(MS_AV_GETMYAVATAR)){
			struct avatarCacheEntry *ace = (struct avatarCacheEntry *)CallService(MS_AV_GETMYAVATAR, 0,(LPARAM) pszSkypeProtoName);
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
				char szFileName[ MAX_PATH ];
				if ( EnterBitmapFileName( szFileName ) != ERROR_SUCCESS )
					return false;

				hAvatar = ( HBITMAP )CallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )szFileName);
				if ( hAvatar != NULL ){
					SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)hAvatar );
					CallService(SKYPE_SETAVATAR, 0, ( LPARAM )szFileName);
				}
				break;

			case IDC_DELETEAVATAR:
				if ( hAvatar != NULL ) {
					DeleteObject( hAvatar );
					hAvatar = NULL;
					CallService(SKYPE_SETAVATAR, 0, NULL);
				}
				DBDeleteContactSetting( NULL, pszSkypeProtoName, "AvatarFile" );
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
BOOL CALLBACK DetailsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static RECT r;
	static int sexM = 0,sexF = 0, sex;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		strcpy(myProfile.SkypeProtoName,pszSkypeProtoName);

		myProfile.Load();
		if(SkypeInitialized)
			myProfile.LoadFromSkype();

		SendDlgItemMessage(hwndDlg,IDC_SEX,CB_ADDSTRING,0,(LPARAM)"");
		sexM = SendDlgItemMessage(hwndDlg,IDC_SEX,CB_ADDSTRING,0,(LPARAM)Translate("MALE"));
		sexF = SendDlgItemMessage(hwndDlg,IDC_SEX,CB_ADDSTRING,0,(LPARAM)Translate("FEMALE"));
		
		if(myProfile.Sex == 0x4D){
			SendDlgItemMessage(hwndDlg,IDC_SEX,CB_SETCURSEL, sexM, (LPARAM) Translate("MALE"));
		}
		if(myProfile.Sex == 0x46){
			SendDlgItemMessage(hwndDlg,IDC_SEX,CB_SETCURSEL, sexF, (LPARAM) Translate("FEMALE"));
		}

		SetDlgItemText(hwndDlg, IDC_FULLNAME, myProfile.FullName);
		SetDlgItemText(hwndDlg, IDC_HOMEPAGE, myProfile.HomePage);
		SetDlgItemText(hwndDlg, IDC_HOMEPHONE, myProfile.HomePhone);
		SetDlgItemText(hwndDlg, IDC_OFFICEPHONE, myProfile.OfficePhone);
		SetDlgItemText(hwndDlg, IDC_CITY, myProfile.City);
		SetDlgItemText(hwndDlg, IDC_PROVINCE, myProfile.Province);
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_SAVEDETAILS:

				char text[256];
				GetDlgItemText(hwndDlg,IDC_FULLNAME,text,sizeof(text));
				strcpy(myProfile.FullName,text);

				GetDlgItemText(hwndDlg,IDC_HOMEPAGE,text,sizeof(text));
				strcpy(myProfile.HomePage,text);

				GetDlgItemText(hwndDlg,IDC_HOMEPHONE,text,sizeof(text));
				strcpy(myProfile.HomePhone,text);

				GetDlgItemText(hwndDlg,IDC_OFFICEPHONE,text,sizeof(text));
				strcpy(myProfile.OfficePhone,text);

				GetDlgItemText(hwndDlg,IDC_CITY,text,sizeof(text));
				strcpy(myProfile.City,text);

				GetDlgItemText(hwndDlg,IDC_PROVINCE,text,sizeof(text));
				strcpy(myProfile.Province,text);

				sex = SendMessage(GetDlgItem(hwndDlg,IDC_SEX),CB_GETCURSEL,0,0);
				
				myProfile.Sex = 0;
				if(sex == sexF) myProfile.Sex = 0x46;
				if(sex == sexM) myProfile.Sex = 0x4D;

				myProfile.Save();
				if(SkypeInitialized)
					myProfile.SaveToSkype();
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