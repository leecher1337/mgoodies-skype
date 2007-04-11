#include "headers.h"
#include "m_virtdb.h"

static HANDLE hEventOptionsInitialize;
HINSTANCE hInst;
OPTIONSDIALOGPAGE odp;
PLUGINLINK *pluginLink;

//========================
//  MirandaPluginInfo
//========================
PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"VirtDBTest",
	PLUGIN_MAKE_VERSION(0,0,0,0),
	"Plugin to test and demostrate VirualDB services.\r\nNot for general use!",
	"VirualDB Author",
	"/dev/null",
	"© same",
	"http://saaplugin.no-ip.info/db3xV",		// www
	0,		//not transient
	0		//doesn't replace anything built-in
};
__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

int MainInit(WPARAM wParam,LPARAM lParam)
{
	return 0;
}

static BOOL CALLBACK DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_USER+1:
		{
			char str[1000];
			GetDlgItemText(hwndDlg,IDC_EDIT1,(LPSTR)str,1000);
			if (CallService(MS_DBV_SAVEFILE,(WPARAM)str,0)){ //save DB to specified filename
				MessageBox(	0,"Success","VirtDB Test",0);
			} else {
				MessageBox(	0,"Failed","VirtDB Test",0);
			};
		}
		break;
	case WM_USER+2:
		if (CallService(MS_DBV_SAVEFILE,-1,0)){ //Sincronize DB but keepit virtual
			MessageBox(	0,"Success","VirtDB Test",0);
		} else {
			MessageBox(	0,"Failed","VirtDB Test",0);
		};
		break;
	case WM_USER+3:
		if (CallService(MS_DBV_SAVEFILE,0,0)){//Open SaveAs dialog and save DB to choosen file.
			MessageBox(	0,"Success","VirtDB Test",0);
		} else {
			MessageBox(	0,"Failed","VirtDB Test",0);
		};
		break;
	case WM_USER+4:
		if (CallService(MS_DBV_VIRTUALIZE,1,0)){ //Virtualize!
			MessageBox(	0,"Now is virtual","VirtDB Test",0);
		} else {
			MessageBox(	0,"Now is real","VirtDB Test",0);
		};
		break;
	case WM_USER+5:
		if (CallService(MS_DBV_VIRTUALIZE,2,0)){ //Realize!
			MessageBox(	0,"Now is virtual","VirtDB Test",0);
		} else {
			MessageBox(	0,"Now is real","VirtDB Test",0);
		};
		break;
	case WM_USER+6:
		if (CallService(MS_DBV_VIRTUALIZE,0,0)){ //Virtualize/Realize
			MessageBox(	0,"Now is virtual","VirtDB Test",0);
		} else {
			MessageBox(	0,"Now is real","VirtDB Test",0);
		};
		break;
	case WM_USER+7:
		if (CallService(MS_DBV_VIRTUALIZE,-1,0)){ // GetStatus
			MessageBox(	0,"Now is virtual","VirtDB Test",0);
		} else {
			MessageBox(	0,"Now is real","VirtDB Test",0);
		};
		break;
	case WM_COMMAND:
        switch (LOWORD(wParam)) {
			case IDC_BUTTON1: // Backup to:
				SendMessage(hwndDlg, WM_USER+1, 0, 0);
				break;
			case IDC_BUTTON2: // Override DB
				SendMessage(hwndDlg, WM_USER+2, 0, 0);
				break;
			case IDC_BUTTON3: // SaveAs
				SendMessage(hwndDlg, WM_USER+3, 0, 0);
				break;
			case IDC_BUTTON4: // Virtualize!
				SendMessage(hwndDlg, WM_USER+4, 0, 0);
				break;
			case IDC_BUTTON5: // Realize!
				SendMessage(hwndDlg, WM_USER+5, 0, 0);
				break;
			case IDC_BUTTON6: // Virt/Real
				SendMessage(hwndDlg, WM_USER+6, 0, 0);
				break;
			case IDC_BUTTON7: // GetStatus
				SendMessage(hwndDlg, WM_USER+7, 0, 0);
				break;
		}
	}
	return FALSE;
}


static int OptionsInitialize(WPARAM wParam,LPARAM lParam)
{
	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG1);
	odp.pszTitle = "Test";
	odp.pszGroup = "VirtualDB";
	odp.groupPosition = 100000000;
	odp.pfnDlgProc = DlgProc;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}


int __declspec(dllexport) Load(PLUGINLINK *link)
{ 	
	
	pluginLink = link; 
	hEventOptionsInitialize = HookEvent(ME_OPT_INITIALISE,OptionsInitialize);

	return 0; 
}

int __declspec(dllexport) Unload(void) 
{ 
	UnhookEvent(hEventOptionsInitialize);
	return 0;
} 
 
