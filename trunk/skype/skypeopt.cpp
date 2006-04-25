#include "skype.h"
#include "skypeopt.h"
#include "pthread.h"
#include "gchat.h"

extern HINSTANCE hInst;
extern PLUGININFO pluginInfo;
extern char pszSkypeProtoName[MAX_PATH+30],protocol;

static HBITMAP hAvatar = NULL;

int RegisterOptions(WPARAM wParam, LPARAM lParam) {
   OPTIONSDIALOGPAGE odp;
   
   ZeroMemory(&odp, sizeof(odp));
   odp.cbSize = sizeof(odp);
   odp.hInstance = hInst;
   odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
   odp.pszGroup = Translate("Network");
   odp.pszTitle = pluginInfo.shortName;
   odp.pfnDlgProc = OptionsDlgProc;
   CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
   return 0;
}

int CALLBACK OptionsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int StartControls[]={IDC_NOSPLASH, IDC_MINIMIZED, IDC_NOTRAY};
	const int Skype2SocketControls[]={IDC_HOST, IDC_PORT, IDC_REQPASS, IDC_PASSWORD};
	static BOOL initDlg=FALSE;
	static int statusModes[]={ID_STATUS_OFFLINE,ID_STATUS_ONLINE,ID_STATUS_AWAY,ID_STATUS_NA,ID_STATUS_OCCUPIED,ID_STATUS_DND,ID_STATUS_FREECHAT,ID_STATUS_INVISIBLE,ID_STATUS_OUTTOLUNCH,ID_STATUS_ONTHEPHONE};
	DBVARIANT dbv;
	int i, j;
	
	switch (uMsg){
		case WM_INITDIALOG:	
			initDlg=TRUE;
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_STARTSKYPE, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "StartSkype", 1));
			CheckDlgButton(hwndDlg, IDC_NOSPLASH, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "nosplash", 1));
			CheckDlgButton(hwndDlg, IDC_MINIMIZED, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "minimized", 1));
			CheckDlgButton(hwndDlg, IDC_NOTRAY, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "notray", 0));
			CheckDlgButton(hwndDlg, IDC_SHUTDOWN, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "Shutdown", 0));
			CheckDlgButton(hwndDlg, IDC_ENABLEMENU, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "EnableMenu", 1));
			CheckDlgButton(hwndDlg, IDC_UNLOADOFFLINE, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UnloadOnOffline", 0));
			CheckDlgButton(hwndDlg, IDC_USES2S, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseSkype2Socket", 0));
			CheckDlgButton(hwndDlg, IDC_NOERRORS, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "SuppressErrors", 0));
			CheckDlgButton(hwndDlg, IDC_KEEPSTATE, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "KeepState", 0));
			SetDlgItemInt (hwndDlg, IDC_CONNATTEMPTS, DBGetContactSettingWord(NULL, pszSkypeProtoName, "ConnectionAttempts", 10), FALSE);
			if (ServiceExists(MS_GC_NEWCHAT) && atoi(SKYPE_PROTO+strlen(SKYPE_PROTO)-1)>=3)
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

			if (!DBGetContactSetting(NULL, pszSkypeProtoName, "Host", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_HOST, dbv.pszVal);
				DBFreeVariant(&dbv);
			} else SetDlgItemText(hwndDlg, IDC_HOST, "localhost");
			SetDlgItemInt(hwndDlg, IDC_PORT, DBGetContactSettingWord(NULL, pszSkypeProtoName, "Port", 1401), FALSE);
			CheckDlgButton(hwndDlg, IDC_REQPASS, (BYTE)DBGetContactSettingByte(NULL, pszSkypeProtoName, "RequiresPassword", 0));
			if (!DBGetContactSetting(NULL, pszSkypeProtoName, "Password", &dbv)) {
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal)+1, (LPARAM)dbv.pszVal);
				SetDlgItemText(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			SendMessage(hwndDlg, WM_COMMAND, IDC_STARTSKYPE, 0);
			SendMessage(hwndDlg, WM_COMMAND, IDC_USES2S, 0);
			SendMessage(hwndDlg, WM_COMMAND, IDC_REQPASS, 0);
			initDlg=FALSE;
			return TRUE;
		case WM_NOTIFY: {
			NMHDR* nmhdr = (NMHDR*)lParam;
			char buf[1024];

			switch (nmhdr->code){
				case PSN_APPLY:
				case PSN_KILLACTIVE:
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "StartSkype", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_STARTSKYPE), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "nosplash", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOSPLASH), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "minimized", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_MINIMIZED), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "notray", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOTRAY), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "Shutdown", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_SHUTDOWN), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "EnableMenu", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_ENABLEMENU), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UnloadOnOffline", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_UNLOADOFFLINE), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UsePopup", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_USEPOPUP), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UseSkype2Socket", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_USES2S), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "UseGroupchat", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_GROUPCHAT), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "SuppressErrors", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_NOERRORS), BM_GETCHECK,0,0)));
					DBWriteContactSettingByte (NULL, pszSkypeProtoName, "KeepState", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_KEEPSTATE), BM_GETCHECK,0,0)));
					DBWriteContactSettingWord (NULL, pszSkypeProtoName, "ConnectionAttempts", (unsigned short)GetDlgItemInt(hwndDlg, IDC_CONNATTEMPTS, NULL, FALSE));
					DBWriteContactSettingDword(NULL, pszSkypeProtoName, "SkypeOutStatusMode", SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_GETITEMDATA,SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_GETCURSEL,0,0),0));
					GetDlgItemText(hwndDlg, IDC_HOST, buf, sizeof(buf));
					DBWriteContactSettingString(NULL, pszSkypeProtoName, "Host", buf);
					DBWriteContactSettingWord(NULL, pszSkypeProtoName, "Port", (unsigned short)GetDlgItemInt(hwndDlg, IDC_PORT, NULL, FALSE));
					DBWriteContactSettingByte(NULL, pszSkypeProtoName, "RequiresPassword", (BYTE)(SendMessage(GetDlgItem(hwndDlg, IDC_REQPASS), BM_GETCHECK,0,0)));
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
				case IDC_STARTSKYPE:
					for (i=0; i<sizeof(StartControls)/sizeof(StartControls[0]); i++) EnableWindow(GetDlgItem(hwndDlg, StartControls[i]), SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0));
					break;
				case IDC_USES2S:
					for (i=0; i<sizeof(Skype2SocketControls)/sizeof(Skype2SocketControls[0]); i++) EnableWindow(GetDlgItem(hwndDlg, Skype2SocketControls[i]), SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0));
					if (SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0)) SendMessage(hwndDlg, WM_COMMAND, IDC_REQPASS, 0);
					break;
				case IDC_REQPASS:
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), BM_GETCHECK,0,0));
					break;
				case IDC_CLEANUP:
					pthread_create(( pThreadFunc )CleanupNicknames, NULL);
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