#include "seen.h"



WNDPROC MainProc;



extern HINSTANCE hInstance;
extern DWORD dwmirver;



BOOL CALLBACK EditProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg){
		case WM_SETCURSOR:
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			return 1;

		default:
			break;
	}
	return CallWindowProc(MainProc,hdlg,msg,wparam,lparam);
}



BOOL CALLBACK UserinfoDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	char *szout;
	DBVARIANT dbv;
	
	switch(msg){

		case WM_INITDIALOG:
			MainProc=(WNDPROC)SetWindowLong(GetDlgItem(hdlg,IDC_INFOTEXT),GWL_WNDPROC,(LONG)EditProc);
			szout=strdup(ParseString((!DBGetContactSetting(NULL,S_MOD,"UserStamp",&dbv)?dbv.pszVal:DEFAULT_USERSTAMP),(HANDLE)lparam,0));
			SetDlgItemText(hdlg,IDC_INFOTEXT,szout);
			if(!strcmp(szout,Translate("<unknown>")))
			EnableWindow(GetDlgItem(hdlg,IDC_INFOTEXT),FALSE);
			free(szout);
			DBFreeVariant(&dbv);
			break;

		case WM_COMMAND:
			if(HIWORD(wparam)==EN_SETFOCUS)
				SetFocus(GetParent(hdlg));
			break;
	}

	return 0;
}



int UserinfoInit(WPARAM wparam,LPARAM lparam)
{
	OPTIONSDIALOGPAGE uip;


	ZeroMemory(&uip,sizeof(uip));
	uip.cbSize=sizeof(uip);
	uip.hInstance=hInstance;
	uip.pszTemplate=MAKEINTRESOURCE(IDD_USERINFO);
	uip.pszTitle=Translate("Last seen");
	uip.pfnDlgProc=UserinfoDlgProc;

	
	CallService(MS_USERINFO_ADDPAGE,wparam,(LPARAM)&uip);
	return 0;
}