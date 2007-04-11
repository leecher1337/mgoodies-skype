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

static char resultPath[MAX_PATH];
DB_VIRTUAL_RESULT dbResult = {
	sizeof(DB_VIRTUAL_RESULT),	// sizeof()
	resultPath,					// filename written or read
	MAX_PATH,					// amount of memory reserved for the FileName
	0,	 // Size of DB in bytes if virtual or 0;
	0,   // 0 if no error; TODO; ErrorCodes;
	0,  // number of blocks written if writing
    0    // total number of blocks if writing
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

static void showResult(HWND hwndDlg, int r){
	char temp[256];
	SendDlgItemMessage(hwndDlg, IDC_LABEL_FN, WM_SETTEXT, 0,(LPARAM)dbResult.szFileName); 
	mir_snprintf(temp,256,"%d bytes",dbResult.imageSize);
	SendDlgItemMessage(hwndDlg, IDC_LABEL_IMAGESIZE, WM_SETTEXT, 0,(LPARAM)temp); 
	mir_snprintf(temp,256,"%d out of %d",dbResult.blWritten, dbResult.blTotal);
	SendDlgItemMessage(hwndDlg, IDC_LABEL_TOTALBLOCKS, WM_SETTEXT, 0,(LPARAM)temp); 
	switch (dbResult.errState){
		case DB_VIRTUAL_ERR_NOTVIRTUAL:mir_snprintf(temp,256,"DB is not virtual"); break;
		case DB_VIRTUAL_ERR_SUCCESS:mir_snprintf(temp,256,"No error"); break;
		case DB_VIRTUAL_ERR_NOFILE:mir_snprintf(temp,256,"No file selected"); break;
		case DB_VIRTUAL_ERR_ACCESS:mir_snprintf(temp,256,"No write access to the file"); break;
		case DB_VIRTUAL_ERR_PARTIAL:mir_snprintf(temp,256,"File partially written. Disk full?"); break;
		default: mir_snprintf(temp,256,"Unknown error code %d",dbResult.errState);
	}
	SendDlgItemMessage(hwndDlg, IDC_LABEL_ERRSTATE, WM_SETTEXT, 0,(LPARAM)temp); 
	mir_snprintf(temp,256,"%d",r);
	SendDlgItemMessage(hwndDlg, IDC_LABEL_RESULT, WM_SETTEXT, 0,(LPARAM)temp); 
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
			showResult(hwndDlg,CallService(MS_DBV_SAVEFILE,(WPARAM)str,(LPARAM)&dbResult)); //save DB to specified filename
		}
		break;
	case WM_USER+2:
		showResult(hwndDlg,CallService(MS_DBV_SAVEFILE,-1,(LPARAM)&dbResult)); //Sincronize DB but keep it virtual
		break;
	case WM_USER+3:
		showResult(hwndDlg,CallService(MS_DBV_SAVEFILE,0,(LPARAM)&dbResult));//Open SaveAs dialog and save DB to choosen file.
		break;
	case WM_USER+4:
		showResult(hwndDlg,CallService(MS_DBV_VIRTUALIZE,1,(LPARAM)&dbResult)); //Virtualize!
		break;
	case WM_USER+5:
		showResult(hwndDlg,CallService(MS_DBV_VIRTUALIZE,2,(LPARAM)&dbResult)); //Realize!
		break;
	case WM_USER+6:
		showResult(hwndDlg,CallService(MS_DBV_VIRTUALIZE,0,(LPARAM)&dbResult)); //Virtualize/Realize
		break;
	case WM_USER+7:
		showResult(hwndDlg,CallService(MS_DBV_VIRTUALIZE,-1,(LPARAM)&dbResult)); // GetStatus
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
 
