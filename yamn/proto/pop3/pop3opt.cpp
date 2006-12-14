/*
 * This code implements POP3 options window handling
 *
 * (c) majvan 2002-2003
*/

/*
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <winuser.h>
#include <commctrl.h>
#include "../../../../../SDK/headers_c/newpluginapi.h"
//#include "../../../../random/utils/m_utils.h"		//for window broadcasting
#include "../../../../../SDK/headers_c/m_langpack.h"
#include "../../../../../SDK/headers_c/m_options.h"
#include "../../../../../SDK/headers_c/m_utils.h"
#include "../../SDK/import/m_popup.h"
#include "../../m_protoplugin.h"
#include "../../m_synchro.h"
#include "../../m_messages.h"
#include "../../resources/resource.h"
#include "../../m_yamn.h"
#include "../../debug.h"
*/
#include "../../yamn.h"
#include "../../main.h"
#include "pop3comm.h"
#include "pop3opt.h"
#include "uxtheme.h"

//- imported ---------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

extern PLUGINLINK *pluginLink;
extern PYAMN_VARIABLES pYAMNVar;
extern HYAMNPROTOPLUGIN POP3Plugin;
extern struct YAMNExportedFcns *pYAMNFcn;
extern YAMN_VARIABLES YAMNVar;

extern HICON hYamnIcons[];

extern DWORD WINAPI WritePOP3Accounts();
extern DWORD HotKeyThreadID;
extern LPCRITICAL_SECTION PluginRegCS;
//From filterplugin.cpp
extern PYAMN_FILTERPLUGINQUEUE FirstFilterPlugin;
//From protoplugin.cpp
extern PYAMN_PROTOPLUGINQUEUE FirstProtoPlugin;
//for XP themes
extern BOOL (WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

struct _tcptable CodePageNames[]=
{
	{_T("OEM"),CP_OEMCP},
	{_T("ARAB-TRANSPARENT"),710},
	{_T("ASMO-TRANSPARENT"),720},
	{_T("ASMO-449"),709},
	{_T("ASMO-708"),708},
	{_T("BIG5"),950},
	{_T("EUC-CH(SP)"),51936},
	{_T("EUC-CH(TR)"),51950},
	{_T("EUC-JP"),51932},
	{_T("EUC-KR"),51949},
	{_T("GB-2312"),20936},
	{_T("HZGB-2312"),52936},
	{_T("IBM-037"),37},
	{_T("IBM-290"),290},
	{_T("IBM-437"),437},
	{_T("IBM-500"),500},
	{_T("IBM-775"),775},
	{_T("IBM-850"),850},
	{_T("IBM-852"),852},
	{_T("IBM-855"),855},
	{_T("IBM-857"),857},
	{_T("IBM-860"),860},
	{_T("IBM-861"),861},
	{_T("IBM-862"),862},
	{_T("IBM-863"),863},
	{_T("IBM-864"),864},
	{_T("IBM-865"),865},
	{_T("IBM-866"),866},
	{_T("IBM-869"),869},
	{_T("IBM-870"),870},
	{_T("IBM-875"),875},
	{_T("IBM-1026"),1026},
	{_T("IBM-273"),20273},
	{_T("IBM-277"),20277},
	{_T("IBM-277"),20278},
	{_T("IBM-278"),20277},
	{_T("IBM-280"),20280},
	{_T("IBM-284"),20284},
	{_T("IBM-285"),20285},
	{_T("IBM-290"),20290},
	{_T("IBM-297"),20297},
	{_T("IBM-420"),20420},
	{_T("IBM-423"),20423},
	{_T("IBM-871"),20871},
	{_T("IBM-880"),20880},
	{_T("IBM-905"),20905},
	{_T("IBM-THAI"),20838},
	{_T("ISCII-DEVANAGARI"),57002},
	{_T("ISCII-BENGALI"),57003},
	{_T("ISCII-TAMIL"),57004},
	{_T("ISCII-TELUGU"),57005},
	{_T("ISCII-ASSAMESE"),57006},
	{_T("ISCII-ORIYA"),57007},
	{_T("ISCII-KANNADA"),57008},
	{_T("ISCII-MALAYALAM"),57009},
	{_T("ISCII-GUJARATI"),57010},
	{_T("ISCII-PUNJABI"),57011},
	{_T("ISO-2022/2-JP"),50220},
	{_T("ISO-2022-JP"),50221},
	{_T("ISO-2022/JIS-JP"),50222},
	{_T("ISO-2022-KR"),50225},
	{_T("ISO-2022-CH(SP)"),50227},
	{_T("ISO-2022-CH(TR)"),50229},
	{_T("ISO-8859-1"),28591},
	{_T("ISO-8859-2"),28592},
	{_T("ISO-8859-3"),28593},
	{_T("ISO-8859-4"),28594},
	{_T("ISO-8859-5"),28595},
	{_T("ISO-8859-6"),28596},
	{_T("ISO-8859-7"),28597},
	{_T("ISO-8859-8"),28598},
	{_T("ISO-8859-9"),28599},
	{_T("ISO-10646-USC2"),1200},
	{_T("KOI8-R"),20866},
	{_T("KOI8-U"),21866},
	{_T("KOR-JOHAB"),1361},
	{_T("KSC-5601"),1361},
	{_T("MAC-ROMAN"),10000},
	{_T("MAC-JP"),10001},
	{_T("MAC-CH(SP)(BIG5)"),10002},
	{_T("MAC-KR"),10003},
	{_T("MAC-AR"),10004}, 
	{_T("MAC-HW"),10005},
	{_T("MAC-GR"),10006},
	{_T("MAC-CY"),10007},
	{_T("MAC-CH(SP)(GB2312)"),10008},
	{_T("MAC-ROMANIA"),10010},
	{_T("MAC-UA"),10017},
	{_T("MAC-TH"),10021},
	{_T("MAC-LAT2"),10029},
	{_T("MAC-ICE"),10079},
	{_T("MAC-TR"),10081},
	{_T("MAC-CR"),10082},
	{_T("UTF-7"),65000},
	{_T("UTF-8"),65001},
	{_T("WINDOWS-1250"),1250},
	{_T("WINDOWS-1251"),1251},
	{_T("WINDOWS-1252"),1252},
	{_T("WINDOWS-1253"),1253},
	{_T("WINDOWS-1254"),1254},
	{_T("WINDOWS-1255"),1255},
	{_T("WINDOWS-1256"),1256},
	{_T("WINDOWS-1257"),1257},
	{_T("WINDOWS-1258"),1258},
};

#define CPLEN	(sizeof(CodePageNames)/sizeof(CodePageNames[0]))
#define CPDEFINDEX	63	//ISO-8859-1

BOOL Check0,Check1,Check2,Check3,Check4,Check5,Check6,Check7,Check8,Check9;
TCHAR DlgInput[MAX_PATH];

//Fuction took from Miranda
void WordToModAndVk(WORD w,UINT *mod,UINT *vk);

//Initializes YAMN general options for Miranda
int YAMNOptInitSvc(WPARAM wParam,LPARAM lParam);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void WordToModAndVk(WORD w,UINT *mod,UINT *vk)
{
	*mod=0;
	if(HIBYTE(w)&HOTKEYF_CONTROL) *mod|=MOD_CONTROL;
	if(HIBYTE(w)&HOTKEYF_SHIFT) *mod|=MOD_SHIFT;
	if(HIBYTE(w)&HOTKEYF_ALT) *mod|=MOD_ALT;
	if(HIBYTE(w)&HOTKEYF_EXT) *mod|=MOD_WIN;
	*vk=LOBYTE(w);
}


BOOL CALLBACK DlgProcYAMNOpt(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hDlg);
			CheckDlgButton(hDlg,IDC_CHECKTTB,DBGetContactSettingByte(NULL,YAMN_DBMODULE,YAMN_TTBFCHECK,1) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hDlg,IDC_HKFORCE,HKM_SETHOTKEY,DBGetContactSettingByte(NULL,YAMN_DBMODULE,YAMN_HKCHECKMAIL,YAMN_DEFAULTHK),0);
			CheckDlgButton(hDlg,IDC_LONGDATE,(optDateTime&SHOWDATELONG) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg,IDC_SMARTDATE,(optDateTime&SHOWDATENOTODAY) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg,IDC_NOSECONDS,(optDateTime&SHOWDATENOSECONDS) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg,IDC_MAINMENU,DBGetContactSettingByte(NULL, YAMN_DBMODULE, YAMN_SHOWMAINMENU, 0));
			CheckDlgButton(hDlg,IDC_YAMNASPROTO,DBGetContactSettingByte(NULL, YAMN_DBMODULE, YAMN_SHOWASPROTO, 0));
			CheckDlgButton(hDlg,IDC_CLOSEONDELETE,DBGetContactSettingByte(NULL, YAMN_DBMODULE, YAMN_CLOSEDELETE, 0));
			
			break;
		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);
			switch(LOWORD(wParam))
			{
				case IDC_YAMNASPROTO:
				case IDC_MAINMENU:
				case IDC_CHECKTTB:
				case IDC_HKFORCE:
				case IDC_CLOSEONDELETE:
				case IDC_LONGDATE:
				case IDC_SMARTDATE:
				case IDC_NOSECONDS:
					SendMessage(GetParent(hDlg),PSM_CHANGED,0,0);
					break;

			}
			break;
		}
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch(((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							WORD ForceHotKey =(WORD)SendDlgItemMessage(hDlg,IDC_HKFORCE,HKM_GETHOTKEY,0,0);
							BYTE TTBFCheck =(BYTE)IsDlgButtonChecked(hDlg,IDC_CHECKTTB);
							BYTE MainMenu = (BYTE)IsDlgButtonChecked(hDlg,IDC_MAINMENU);
							BYTE CloseDelete = (BYTE)IsDlgButtonChecked(hDlg,IDC_CLOSEONDELETE);
							BYTE ShowAsProto =  (BYTE)IsDlgButtonChecked(hDlg,IDC_YAMNASPROTO);
							UINT mod,vk;

							DBWriteContactSettingByte(NULL,YAMN_DBMODULE,YAMN_SHOWASPROTO,ShowAsProto);
							DBWriteContactSettingWord(NULL,YAMN_DBMODULE,YAMN_HKCHECKMAIL,ForceHotKey);
							DBWriteContactSettingByte(NULL,YAMN_DBMODULE,YAMN_TTBFCHECK,TTBFCheck);
							DBWriteContactSettingByte(NULL,YAMN_DBMODULE,YAMN_SHOWMAINMENU,MainMenu);
							DBWriteContactSettingByte(NULL,YAMN_DBMODULE,YAMN_CLOSEDELETE,CloseDelete);
							WordToModAndVk(ForceHotKey,&mod,&vk);
							PostThreadMessage(HotKeyThreadID,WM_YAMN_CHANGEHOTKEY,(WPARAM)mod,(LPARAM)vk);

							optDateTime = 0;
							if (IsDlgButtonChecked(hDlg,IDC_LONGDATE))optDateTime |= SHOWDATELONG;
							if (IsDlgButtonChecked(hDlg,IDC_SMARTDATE))optDateTime |= SHOWDATENOTODAY;
							if (IsDlgButtonChecked(hDlg,IDC_NOSECONDS))optDateTime |= SHOWDATENOSECONDS;
							DBWriteContactSettingByte(NULL,YAMN_DBMODULE,YAMN_DBTIMEOPTIONS,optDateTime);
						}
					}
			}
			break;
	}

	return FALSE;
}

BOOL CALLBACK DlgProcPluginOpt(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hDlg);
			break;
		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);
			switch(LOWORD(wParam))
			{
				case IDC_COMBOPLUGINS:
					if(wNotifyCode==CBN_SELCHANGE)
					{
						HWND hCombo=GetDlgItem(hDlg,IDC_COMBOPLUGINS);
						PYAMN_PROTOPLUGINQUEUE PParser;
						PYAMN_FILTERPLUGINQUEUE FParser;
						int index,id;
	
						if(CB_ERR==(index=SendMessage(hCombo,CB_GETCURSEL,0,0)))
							break;
						id=SendMessage(hCombo,CB_GETITEMDATA,(WPARAM)index,(LPARAM)0);
						EnterCriticalSection(PluginRegCS);
						for(PParser=FirstProtoPlugin;PParser!=NULL;PParser=PParser->Next)
							if(id==(int)PParser->Plugin)
							{
								SetDlgItemText(hDlg,IDC_STVER,PParser->Plugin->PluginInfo->Ver);
								SetDlgItemText(hDlg,IDC_STDESC,PParser->Plugin->PluginInfo->Description == NULL ? "" : PParser->Plugin->PluginInfo->Description);
								SetDlgItemText(hDlg,IDC_STCOPY,PParser->Plugin->PluginInfo->Copyright == NULL ? "" : PParser->Plugin->PluginInfo->Copyright);
								SetDlgItemText(hDlg,IDC_STMAIL,PParser->Plugin->PluginInfo->Email == NULL ? "" : PParser->Plugin->PluginInfo->Email);
								SetDlgItemText(hDlg,IDC_STWWW,PParser->Plugin->PluginInfo->WWW == NULL ? "" : PParser->Plugin->PluginInfo->WWW);
								break;
							}
						for(FParser=FirstFilterPlugin;FParser!=NULL;FParser=FParser->Next)
							if(id==(int)FParser->Plugin)
							{
								SetDlgItemText(hDlg,IDC_STVER,FParser->Plugin->PluginInfo->Ver);
								SetDlgItemText(hDlg,IDC_STDESC,FParser->Plugin->PluginInfo->Description == NULL ? "" : FParser->Plugin->PluginInfo->Description);
								SetDlgItemText(hDlg,IDC_STCOPY,FParser->Plugin->PluginInfo->Copyright == NULL ? "" : FParser->Plugin->PluginInfo->Copyright);
								SetDlgItemText(hDlg,IDC_STMAIL,FParser->Plugin->PluginInfo->Email == NULL ? "" : FParser->Plugin->PluginInfo->Email);
								SetDlgItemText(hDlg,IDC_STWWW,FParser->Plugin->PluginInfo->WWW == NULL ? "" : FParser->Plugin->PluginInfo->WWW);
								break;
							}
						LeaveCriticalSection(PluginRegCS);
					}
					break;
				case IDC_STWWW:
				{
					char str[1024];

					GetDlgItemText(hDlg,IDC_STWWW,str,sizeof(str));
					CallService(MS_UTILS_OPENURL,1,(LPARAM)str);
					break;
				}

			}
			break;
		}
		case WM_SHOWWINDOW:
			if(TRUE==(BOOL)wParam)
			{
				PYAMN_PROTOPLUGINQUEUE PParser;
				PYAMN_FILTERPLUGINQUEUE FParser;
				int index;
			
				EnterCriticalSection(PluginRegCS);
				for(PParser=FirstProtoPlugin;PParser!=NULL;PParser=PParser->Next)
				{
					index=SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_ADDSTRING,0,(LPARAM)PParser->Plugin->PluginInfo->Name);
					index=SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_SETITEMDATA,(WPARAM)index,(LPARAM)PParser->Plugin);
				}
				for(FParser=FirstFilterPlugin;FParser!=NULL;FParser=FParser->Next)
				{
					index=SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_ADDSTRING,0,(LPARAM)FParser->Plugin->PluginInfo->Name);
					index=SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_SETITEMDATA,(WPARAM)index,(LPARAM)FParser->Plugin);
				}

				LeaveCriticalSection(PluginRegCS);
				SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_SETCURSEL,(WPARAM)0,(LPARAM)0);
				SendMessage(hDlg,WM_COMMAND,MAKELONG(IDC_COMBOPLUGINS,CBN_SELCHANGE),(LPARAM)NULL);
				break;
			}
			else		//delete all items in combobox
			{
				int cbn=SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_GETCOUNT,(WPARAM)0,(LPARAM)0);

				for(int i=0;i<cbn;i++)
					SendDlgItemMessage(hDlg,IDC_COMBOPLUGINS,CB_DELETESTRING,(WPARAM)0,(LPARAM)0);
				break;
			}
		case WM_NOTIFY:
			break;
	}

	return FALSE;
}


int YAMNOptInitSvc(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp={0};
	
	odp.cbSize=sizeof(odp);
	odp.position=0x00000000;
	odp.hInstance=YAMNVar.hInst;
	odp.pszGroup=Translate("Plugins");
	odp.pszTitle=Translate("YAMN");
	odp.flags=ODPF_BOLDGROUPS;
//insert YAMN options dialog
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc=(DLGPROC)DlgOptionsProc;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	return 0;
}


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
BOOL DlgEnableAccountStatus(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST0),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST1),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST2),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST3),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST4),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST5),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST6),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST7),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST8),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST9),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST9),(BOOL)wParam);
	return TRUE;
}


BOOL DlgEnableAccount(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	EnableWindow(GetDlgItem(hDlg,IDC_CHECK),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITSERVER),wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITPORT),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITLOGIN),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITPASS),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITINTERVAL),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKSND),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKMSG),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKICO),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKPOP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKCOL),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPB),lParam && (IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPT),lParam && (IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_RADIOPOPN),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_RADIOPOP1),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKAPP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKKBN),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_BTNAPP),(IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITAPP),(IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITAPPPARAM),(IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKNPOP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITNPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKNCOL),(IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPNB),lParam && (IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPNT),lParam && (IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKNMSGP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFSND),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFMSG),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFICO),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFPOP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_EDITFPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFCOL),(IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPFB),lParam && (IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CPFT),lParam && (IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED) && wParam);
	/*EnableWindow(GetDlgItem(hDlg,IDC_CHECKST0),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST1),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST2),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST3),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST4),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST5),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST6),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST7),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST8),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST9),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKST9),(BOOL)wParam);*/
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKSTART),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKFORCE),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_COMBOCP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_STTIMELEFT),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_BTNRESET),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_BTNDEFAULT),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_BTNSTATUS),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKSSL),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKNOTLS),(IsDlgButtonChecked(hDlg,IDC_CHECKSSL)==BST_UNCHECKED) && wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKAPOP),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKCONTACT),(BOOL)wParam);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECKCONTACTNICK),(IsDlgButtonChecked(hDlg,IDC_CHECKCONTACT)==BST_CHECKED) && wParam);
	return TRUE;
}
BOOL DlgShowAccountStatus(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	HPOP3ACCOUNT ActualAccount=(HPOP3ACCOUNT)lParam;
	
	if((DWORD)wParam==M_SHOWACTUAL)
	{
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read wait\n");
		#endif
		WaitToRead(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read enter\n");
		#endif
		CheckDlgButton(hDlg,IDC_CHECKST0,ActualAccount->StatusFlags & YAMN_ACC_ST0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST1,ActualAccount->StatusFlags & YAMN_ACC_ST1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST2,ActualAccount->StatusFlags & YAMN_ACC_ST2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST3,ActualAccount->StatusFlags & YAMN_ACC_ST3 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST4,ActualAccount->StatusFlags & YAMN_ACC_ST4 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST5,ActualAccount->StatusFlags & YAMN_ACC_ST5 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST6,ActualAccount->StatusFlags & YAMN_ACC_ST6 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST7,ActualAccount->StatusFlags & YAMN_ACC_ST7 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST8,ActualAccount->StatusFlags & YAMN_ACC_ST8 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST9,ActualAccount->StatusFlags & YAMN_ACC_ST9 ? BST_CHECKED : BST_UNCHECKED);
		ReadDone(ActualAccount);
	}
	else
	{
		CheckDlgButton(hDlg,IDC_CHECKST0,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST1,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST2,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST3,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST4,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST5,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST6,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST7,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST8,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST9,BST_CHECKED);
	}
	return TRUE;
}
BOOL DlgShowAccount(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	HPOP3ACCOUNT ActualAccount=(HPOP3ACCOUNT)lParam;
	int i;
	
	if((DWORD)wParam==M_SHOWACTUAL)
	{
		TCHAR accstatus[256];
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read wait\n");
		#endif
		WaitToRead(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read enter\n");
		#endif
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITSERVER,(LPARAM)ActualAccount->Server->Name);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITLOGIN,(LPARAM)ActualAccount->Server->Login);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITPASS,(LPARAM)ActualAccount->Server->Passwd);
		DlgSetItemTextW(hDlg,(WPARAM)IDC_EDITAPP,(LPARAM)ActualAccount->NewMailN.App);
		DlgSetItemTextW(hDlg,(WPARAM)IDC_EDITAPPPARAM,(LPARAM)ActualAccount->NewMailN.AppParam);
		SetDlgItemInt(hDlg,IDC_EDITPORT,ActualAccount->Server->Port,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITINTERVAL,ActualAccount->Interval/60,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITPOPS,ActualAccount->NewMailN.PopUpTime,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITNPOPS,ActualAccount->NoNewMailN.PopUpTime,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITFPOPS,ActualAccount->BadConnectN.PopUpTime,FALSE);
		for(i=0;i<=CPLEN;i++)
			if((i<CPLEN) && (CodePageNames[i].CP==ActualAccount->CP))
			{
				SendMessage(GetDlgItem(hDlg,IDC_COMBOCP),CB_SETCURSEL,(WPARAM)i,(LPARAM)0);
				break;
			}
		if(i==CPLEN)
			SendMessage(GetDlgItem(hDlg,IDC_COMBOCP),CB_SETCURSEL,(WPARAM)CPDEFINDEX,(LPARAM)0);

		CheckDlgButton(hDlg,IDC_CHECK,ActualAccount->Flags & YAMN_ACC_ENA ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKSND,ActualAccount->NewMailN.Flags & YAMN_ACC_SND ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKMSG,ActualAccount->NewMailN.Flags & YAMN_ACC_MSG ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKICO,ActualAccount->NewMailN.Flags & YAMN_ACC_ICO ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKPOP,ActualAccount->NewMailN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKCOL,ActualAccount->NewMailN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKAPP,ActualAccount->NewMailN.Flags & YAMN_ACC_APP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKKBN,ActualAccount->NewMailN.Flags & YAMN_ACC_KBN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKNPOP,ActualAccount->NoNewMailN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKNCOL,ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKNMSGP,ActualAccount->NoNewMailN.Flags & YAMN_ACC_MSGP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKFSND,ActualAccount->BadConnectN.Flags & YAMN_ACC_SND ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFMSG,ActualAccount->BadConnectN.Flags & YAMN_ACC_MSG ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFICO,ActualAccount->BadConnectN.Flags & YAMN_ACC_ICO ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFPOP,ActualAccount->BadConnectN.Flags & YAMN_ACC_POP ? BST_CHECKED : BST_UNCHECKED); 
		CheckDlgButton(hDlg,IDC_CHECKFCOL,ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOPN,ActualAccount->Flags & YAMN_ACC_POPN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOP1,ActualAccount->Flags & YAMN_ACC_POPN ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKSSL,ActualAccount->Flags & YAMN_ACC_SSL23 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKNOTLS,ActualAccount->Flags & YAMN_ACC_NOTLS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKAPOP,ActualAccount->Flags & YAMN_ACC_APOP ? BST_CHECKED : BST_UNCHECKED);
		/*CheckDlgButton(hDlg,IDC_CHECKST0,ActualAccount->StatusFlags & YAMN_ACC_ST0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST1,ActualAccount->StatusFlags & YAMN_ACC_ST1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST2,ActualAccount->StatusFlags & YAMN_ACC_ST2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST3,ActualAccount->StatusFlags & YAMN_ACC_ST3 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST4,ActualAccount->StatusFlags & YAMN_ACC_ST4 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST5,ActualAccount->StatusFlags & YAMN_ACC_ST5 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST6,ActualAccount->StatusFlags & YAMN_ACC_ST6 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST7,ActualAccount->StatusFlags & YAMN_ACC_ST7 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST8,ActualAccount->StatusFlags & YAMN_ACC_ST8 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST9,ActualAccount->StatusFlags & YAMN_ACC_ST9 ? BST_CHECKED : BST_UNCHECKED);*/
		Check0=ActualAccount->StatusFlags & YAMN_ACC_ST0;
		Check1=ActualAccount->StatusFlags & YAMN_ACC_ST1;
		Check2=ActualAccount->StatusFlags & YAMN_ACC_ST2;
		Check3=ActualAccount->StatusFlags & YAMN_ACC_ST3;
		Check4=ActualAccount->StatusFlags & YAMN_ACC_ST4;
		Check5=ActualAccount->StatusFlags & YAMN_ACC_ST5;
		Check6=ActualAccount->StatusFlags & YAMN_ACC_ST6;
		Check7=ActualAccount->StatusFlags & YAMN_ACC_ST7;
		Check8=ActualAccount->StatusFlags & YAMN_ACC_ST8;
		Check9=ActualAccount->StatusFlags & YAMN_ACC_ST9;
		CheckDlgButton(hDlg,IDC_CHECKSTART,ActualAccount->StatusFlags & YAMN_ACC_STARTS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFORCE,ActualAccount->StatusFlags & YAMN_ACC_FORCE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCONTACT,ActualAccount->NewMailN.Flags & YAMN_ACC_CONT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCONTACTNICK,ActualAccount->NewMailN.Flags & YAMN_ACC_CONTNICK ? BST_CHECKED : BST_UNCHECKED);
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"Options:SHOWACCOUNT:ActualAccountSO-read done\n");
#endif
		GetAccountStatus(ActualAccount,accstatus);
		SetDlgItemTextA(hDlg,IDC_STSTATUS,accstatus);
		ReadDone(ActualAccount);
	}				       
	else			            //default
	{
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITSERVER,(LPARAM)NULL);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITLOGIN,(LPARAM)NULL);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITPASS,(LPARAM)NULL);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITAPP,(LPARAM)NULL);
		DlgSetItemText(hDlg,(WPARAM)IDC_EDITAPPPARAM,(LPARAM)NULL);
		DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,(LPARAM)NULL);
		SetDlgItemInt(hDlg,IDC_EDITPORT,110,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITINTERVAL,10,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITPOPS,0,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITNPOPS,0,FALSE);
		SetDlgItemInt(hDlg,IDC_EDITFPOPS,0,FALSE);
		SendMessage(GetDlgItem(hDlg,IDC_COMBOCP),CB_SETCURSEL,(WPARAM)CPDEFINDEX,(LPARAM)0);
		CheckDlgButton(hDlg,IDC_CHECK,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKSND,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKMSG,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKICO,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKPOP,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCOL,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKAPP,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKPOP,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKCOL,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFSND,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFMSG,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFICO,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFPOP,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFCOL,BST_CHECKED);
		/*CheckDlgButton(hDlg,IDC_CHECKST0,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST1,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST2,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST3,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST4,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST5,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST6,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST7,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST8,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKST9,BST_CHECKED);*/
		CheckDlgButton(hDlg,IDC_CHECKSTART,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKFORCE,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOPN,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIOPOP1,BST_CHECKED);
		CheckDlgButton(hDlg,IDC_CHECKSSL,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKNOTLS,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECKAPOP,BST_UNCHECKED);

		SetDlgItemText(hDlg,IDC_STSTATUS,Translate("No account selected"));
	}
	return TRUE;
}

BOOL DlgShowAccountColors(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	HPOP3ACCOUNT ActualAccount=(HPOP3ACCOUNT)lParam;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"Options:SHOWACCOUNTCOLORS:ActualAccountSO-read wait\n");
#endif
	WaitToRead(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"Options:SHOWACCOUNTCOLORS:ActualAccountSO-read enter\n");
#endif
	if(ActualAccount->NewMailN.Flags & YAMN_ACC_POPC)
	{
		SendDlgItemMessage(hDlg,IDC_CPB,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->NewMailN.PopUpB);
		SendDlgItemMessage(hDlg,IDC_CPT,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->NewMailN.PopUpT);
	}
	else
	{
		SendDlgItemMessage(hDlg,IDC_CPB,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_BTNFACE));
		SendDlgItemMessage(hDlg,IDC_CPT,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_WINDOWTEXT));
	}
	if(ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC)
	{
		SendDlgItemMessage(hDlg,IDC_CPFB,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->BadConnectN.PopUpB);
		SendDlgItemMessage(hDlg,IDC_CPFT,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->BadConnectN.PopUpT);
	}
	else
	{
		SendDlgItemMessage(hDlg,IDC_CPFB,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_BTNFACE));
		SendDlgItemMessage(hDlg,IDC_CPFT,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_WINDOWTEXT));
	}
	if(ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC)
	{
		SendDlgItemMessage(hDlg,IDC_CPNB,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->NoNewMailN.PopUpB);
		SendDlgItemMessage(hDlg,IDC_CPNT,CPM_SETCOLOUR,0,(LPARAM)ActualAccount->NoNewMailN.PopUpT);
	}
	else
	{
		SendDlgItemMessage(hDlg,IDC_CPNB,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_BTNFACE));
		SendDlgItemMessage(hDlg,IDC_CPNT,CPM_SETCOLOUR,0,(LPARAM)GetSysColor(COLOR_WINDOWTEXT));
	}
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"Options:SHOWACCOUNTCOLORS:ActualAccountSO-read done\n");
#endif
	ReadDone(ActualAccount);		//we do not need to check if account is deleted. It is not deleted, because only thread that can delete account is this thread
	return TRUE;
}

BOOL DlgSetItemText(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	if((TCHAR*)lParam==NULL)
		SetDlgItemText(hDlg,(UINT)wParam,_T(""));
	else
		SetDlgItemText(hDlg,(UINT)wParam,Translate((TCHAR *)lParam));
	return TRUE;
}

BOOL DlgSetItemTextW(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
	if((WCHAR*)lParam==NULL)
		SetDlgItemTextW(hDlg,(UINT)wParam,(LPWSTR)L"");
	else
		SetDlgItemTextW(hDlg,(UINT)wParam,(LPWSTR)lParam);
	return TRUE;
}

BOOL CALLBACK DlgProcPOP3AccStatusOpt(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static HPOP3ACCOUNT ActualAccount;
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)DlgInput);
			if(ActualAccount != NULL)
			{
				DlgShowAccountStatus(hDlg,(WPARAM)M_SHOWACTUAL,(LPARAM)ActualAccount);
				DlgEnableAccountStatus(hDlg,(WPARAM)TRUE,(LPARAM)TRUE);
			}
			TranslateDialogDefault(hDlg);
			SendMessage(GetParent(hDlg),PSM_UNCHANGED,(WPARAM)hDlg,0);
			return TRUE;
			break;
		}
		case WM_COMMAND:
		{
		
			WORD wNotifyCode = HIWORD(wParam);
			switch(LOWORD(wParam))
			{
				case IDOK:
					Check0=(IsDlgButtonChecked(hDlg,IDC_CHECKST0)==BST_CHECKED);
					Check1=(IsDlgButtonChecked(hDlg,IDC_CHECKST1)==BST_CHECKED);
					Check2=(IsDlgButtonChecked(hDlg,IDC_CHECKST2)==BST_CHECKED);
					Check3=(IsDlgButtonChecked(hDlg,IDC_CHECKST3)==BST_CHECKED);
					Check4=(IsDlgButtonChecked(hDlg,IDC_CHECKST4)==BST_CHECKED);
					Check5=(IsDlgButtonChecked(hDlg,IDC_CHECKST5)==BST_CHECKED);
					Check6=(IsDlgButtonChecked(hDlg,IDC_CHECKST6)==BST_CHECKED);
					Check7=(IsDlgButtonChecked(hDlg,IDC_CHECKST7)==BST_CHECKED);
					Check8=(IsDlgButtonChecked(hDlg,IDC_CHECKST8)==BST_CHECKED);
					Check9=(IsDlgButtonChecked(hDlg,IDC_CHECKST9)==BST_CHECKED);
					WindowList_BroadcastAsync(YAMNVar.MessageWnds,WM_YAMN_CHANGESTATUSOPTION,(WPARAM)0,(LPARAM)0);
					EndDialog(hDlg,0);
					DestroyWindow(hDlg);
					break;
				
				case IDCANCEL:
					EndDialog(hDlg,0);
					DestroyWindow(hDlg);
					break;
				
				default:
                        break;
			}
		}
		default:
			break;
	}
	return FALSE;
}

BOOL CALLBACK DlgOptionsProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
         tci.lParam = (LPARAM)CreateDialog(YAMNVar.hInst,MAKEINTRESOURCE(IDD_POP3ACCOUNTOPT), hwnd, DlgProcPOP3AccOpt);
         tci.pszText = TranslateT("Accounts");
		 TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 0, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-3,rcClient.bottom-33,1);
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

         tci.lParam = (LPARAM)CreateDialog(YAMNVar.hInst,MAKEINTRESOURCE(IDD_YAMNOPT), hwnd, DlgProcYAMNOpt);
         tci.pszText = TranslateT("General");
		 TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 1, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-3,rcClient.bottom-33,1);
		 ShowWindow((HWND)tci.lParam, SW_HIDE);
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

         tci.lParam = (LPARAM)CreateDialog(YAMNVar.hInst,MAKEINTRESOURCE(IDD_PLUGINOPT),hwnd,DlgProcPluginOpt);
         tci.pszText = TranslateT("Plugins");
         TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 2, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-3,rcClient.bottom-33,1);
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

BOOL CALLBACK DlgProcPOP3AccOpt(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	BOOL Changed=FALSE;
	static BOOL InList=FALSE;
	static HPOP3ACCOUNT ActualAccount;
	static UCHAR ActualStatus;
//	static struct CPOP3Options POP3Options;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			int i;

			EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),FALSE);
			DlgEnableAccount(hDlg,(WPARAM)FALSE,(LPARAM)FALSE);
			DlgShowAccount(hDlg,(WPARAM)M_SHOWDEFAULT,0);
//			DlgShowAccountColors(hDlg,0,(LPARAM)ActualAccount);
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"Options:INITDIALOG:AccountBrowserSO-read wait\n");
			#endif
			WaitToReadSO(POP3Plugin->AccountBrowserSO);
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"Options:INITDIALOG:AccountBrowserSO-read enter\n");
			#endif
			//SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_ADDSTRING,0,(LPARAM)""); //this was in YAMN.rc initialisation but seems to be useless
			if(POP3Plugin->FirstAccount!=NULL)
				for(ActualAccount=(HPOP3ACCOUNT)POP3Plugin->FirstAccount;ActualAccount!=NULL;ActualAccount=(HPOP3ACCOUNT)ActualAccount->Next)
					if(ActualAccount->Name!=NULL)
						SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_ADDSTRING,0,(LPARAM)ActualAccount->Name);
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"Options:INITDIALOG:AccountBrowserSO-read done\n");
			#endif
			ReadDoneSO(POP3Plugin->AccountBrowserSO);
			for(i=0;i<CPLEN;i++)
				SendDlgItemMessage(hDlg,IDC_COMBOCP,CB_ADDSTRING,0,(LPARAM)CodePageNames[i].Name);

			SendMessage(GetDlgItem(hDlg,IDC_COMBOCP),CB_SETCURSEL,(WPARAM)CPDEFINDEX,(LPARAM)0);
			ActualAccount=NULL;
//			AddWndToWndQueue(hDlg);
/*			DlgSetItemText(hDlg,(WPARAM)IDC_BTNDEL,(LPARAM)"Delete");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECK,(LPARAM)"Check this account");
			DlgSetItemText(hDlg,(WPARAM)IDC_STSERVER,(LPARAM)"Server:");
			DlgSetItemText(hDlg,(WPARAM)IDC_STPORT,(LPARAM)"Port:");
			DlgSetItemText(hDlg,(WPARAM)IDC_STLOGIN,(LPARAM)"User:");
			DlgSetItemText(hDlg,(WPARAM)IDC_STPASS,(LPARAM)"Password:");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKAPOP,(LPARAM)"APOP auth");

			DlgSetItemText(hDlg,(WPARAM)IDC_STINTERVAL,(LPARAM)"Check interval [min]:");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKSND,(LPARAM)"Sound notification");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKMSG,(LPARAM)"Message notification");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKICO,(LPARAM)"Tray icon notification");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKPOP,(LPARAM)"Popup notification");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKAPP,(LPARAM)"Application execution:");

			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKNPOP,(LPARAM)"Popup if no mail");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKNPOP,(LPARAM)"Persistant message");

			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKFSND,(LPARAM)"Sound notification if failed");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKFMSG,(LPARAM)"Message notification if failed");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKFICO,(LPARAM)"Tray icon notification if failed");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKFPOP,(LPARAM)"Popup notification if failed");

//			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKHEADERS,(LPARAM)"Store headers");
			DlgSetItemText(hDlg,(WPARAM)IDC_STCP,(LPARAM)"Default codepage:");
			DlgSetItemText(hDlg,(WPARAM)IDC_BTNDEL,(LPARAM)"Delete");

			DlgSetItemText(hDlg,(WPARAM)IDC_STWCHECK,(LPARAM)"Check while:");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKST0,(LPARAM)"Offline");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKST1,(LPARAM)"Online");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKST2,(LPARAM)"Away");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKST3,(LPARAM)"N/A");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKST4,(LPARAM)"Occupied");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKST5,(LPARAM)"DND");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKST6,(LPARAM)"Free for chat");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKST7,(LPARAM)"Invisible");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKST8,(LPARAM)"On the phone");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKST9,(LPARAM)"Out to lunch");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKSTART,(LPARAM)"Startup check");
			DlgSetItemText(hDlg,(WPARAM)IDC_CHECKFORCE,(LPARAM)"Check from menu");
			DlgSetItemText(hDlg,(WPARAM)IDC_BTNRESET,(LPARAM)"Reset counter");
			DlgSetItemText(hDlg,(WPARAM)IDC_BTNDEFAULT,(LPARAM)"Default");

			DlgSetItemText(hDlg,(WPARAM)IDC_RADIOPOP1,(LPARAM)"Single popup");
			DlgSetItemText(hDlg,(WPARAM)IDC_RADIOPOPN,(LPARAM)"Multi popup");
*/
			TranslateDialogDefault(hDlg);

			SendMessage(GetParent(hDlg),PSM_UNCHANGED,(WPARAM)hDlg,0);
			return TRUE;
		}
//		case WM_NCDESTROY:
//		case WM_DESTROY:
		case WM_SHOWWINDOW:
			if((BOOL)wParam==FALSE)
			{
				WindowList_Remove(pYAMNVar->MessageWnds,hDlg);
				SendMessage(GetParent(hDlg),PSM_UNCHANGED,(WPARAM)hDlg,(LPARAM)0);
			}
			else
				WindowList_Add(pYAMNVar->MessageWnds,hDlg,NULL);
			return TRUE;
		case WM_YAMN_CHANGESTATUS:
		{
			char accstatus[256];

			if((HPOP3ACCOUNT)wParam!=ActualAccount)
				break;
			GetAccountStatus(ActualAccount,accstatus);
			SetDlgItemTextA(hDlg,IDC_STSTATUS,accstatus);
		}
			return TRUE;
		case WM_YAMN_CHANGESTATUSOPTION:
		{
			Changed=TRUE;
			SendMessage(GetParent(hDlg),PSM_CHANGED,0,0);
		}
			return TRUE;
		case WM_YAMN_CHANGETIME:
			if((HPOP3ACCOUNT)wParam==ActualAccount)
			{
				TCHAR Text[256];
				_stprintf(Text,Translate("Time left to next check [s]: %d"),(DWORD)lParam);
				SetDlgItemText(hDlg,IDC_STTIMELEFT,Text);
			}
			return TRUE;
		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);
			switch(LOWORD(wParam))
			{
				LONG Result;
				case IDC_COMBOACCOUNT:
					switch(wNotifyCode)
					{
						case CBN_EDITCHANGE :
							ActualAccount=NULL;
							DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,(LPARAM)NULL);
							if(GetDlgItemText(hDlg,IDC_COMBOACCOUNT,DlgInput,sizeof(DlgInput)/sizeof(TCHAR)))
								DlgEnableAccount(hDlg,(WPARAM)TRUE,(LPARAM)FALSE);
							else
								DlgEnableAccount(hDlg,(WPARAM)FALSE,(LPARAM)FALSE);
							break;
						case CBN_KILLFOCUS:
							GetDlgItemText(hDlg,IDC_COMBOACCOUNT,DlgInput,sizeof(DlgInput)/sizeof(TCHAR));
							if(NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)DlgInput)))
							{
								DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,(LPARAM)NULL);
								EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),FALSE);
								if(lstrlen(DlgInput))
									DlgEnableAccount(hDlg,(WPARAM)TRUE,(LPARAM)TRUE);
								else
									DlgEnableAccount(hDlg,(WPARAM)FALSE,(LPARAM)FALSE);
							}
							else
							{
								DlgShowAccount(hDlg,(WPARAM)M_SHOWACTUAL,(LPARAM)ActualAccount);
								DlgShowAccountColors(hDlg,0,(LPARAM)ActualAccount);
								DlgEnableAccount(hDlg,(WPARAM)TRUE,(LPARAM)TRUE);
								EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),TRUE);
							}
							break;
						case CBN_SELCHANGE:
							if(CB_ERR!=(Result=SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_GETCURSEL,0,0)))
								SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_GETLBTEXT,(WPARAM)Result,(LPARAM)DlgInput);
							if((Result==CB_ERR) || (NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)DlgInput))))
							{
								DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,(LPARAM)NULL);
								EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),FALSE);
							}
							else
							{
								DlgShowAccount(hDlg,(WPARAM)M_SHOWACTUAL,(LPARAM)ActualAccount);
								DlgShowAccountColors(hDlg,0,(LPARAM)ActualAccount);
								DlgEnableAccount(hDlg,(WPARAM)TRUE,(LPARAM)FALSE);
								EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),TRUE);
							}
							break;
					}
					break;
				case IDC_CHECK:
				case IDC_CHECKSND:
				case IDC_CHECKMSG:
				case IDC_CHECKICO:
				case IDC_CHECKFSND:
				case IDC_CHECKFMSG:
				case IDC_CHECKFICO:
				case IDC_CHECKST0:
				case IDC_CHECKST1:
				case IDC_CHECKST2:
				case IDC_CHECKST3:
				case IDC_CHECKST4:
				case IDC_CHECKST5:
				case IDC_CHECKST6:
				case IDC_CHECKST7:
				case IDC_CHECKST8:
				case IDC_CHECKST9:
				case IDC_CHECKSTART:
				case IDC_CHECKFORCE:
				case IDC_EDITAPPPARAM:
				case IDC_COMBOCP:
				case IDC_RADIOPOPN:
				case IDC_RADIOPOP1:
				case IDC_CHECKAPOP:
				case IDC_CHECKCONTACTNICK:
				case IDC_CHECKNOTLS:
					Changed=TRUE;
					break;
				case IDC_CHECKCONTACT:
					Changed=TRUE;
					EnableWindow(GetDlgItem(hDlg,IDC_CHECKCONTACTNICK),IsDlgButtonChecked(hDlg,IDC_CHECKCONTACT)==BST_CHECKED);
					break;
				case IDC_CHECKSSL:
				{
					BOOL SSLC=(IsDlgButtonChecked(hDlg,IDC_CHECKSSL)==BST_CHECKED);
					SetDlgItemInt(hDlg,IDC_EDITPORT,SSLC ? 995 : 110,FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_CHECKNOTLS),SSLC?0:1);
				}
					Changed=TRUE;
					break;
				case IDC_CPB:
				case IDC_CPT:
				case IDC_CPFB:
				case IDC_CPFT:
				case IDC_CPNB:
				case IDC_CPNT:
					if(HIWORD(wParam)!=CPN_COLOURCHANGED)
						break;
				case IDC_CHECKCOL:
				case IDC_CHECKFCOL:
				case IDC_CHECKNCOL:
				{
					POPUPDATA Tester;
					POPUPDATA TesterF;
					POPUPDATA TesterN;
					BOOL TesterC=(IsDlgButtonChecked(hDlg,IDC_CHECKCOL)==BST_CHECKED);
					BOOL TesterFC=(IsDlgButtonChecked(hDlg,IDC_CHECKFCOL)==BST_CHECKED);
					BOOL TesterNC=(IsDlgButtonChecked(hDlg,IDC_CHECKNCOL)==BST_CHECKED);
					
					ZeroMemory(&Tester,sizeof(Tester));
					ZeroMemory(&TesterF,sizeof(TesterF));
					ZeroMemory(&TesterF,sizeof(TesterN));
					Tester.lchContact=NULL;
					TesterF.lchContact=NULL;
					TesterN.lchContact=NULL;
					Tester.lchIcon=hYamnIcons[2];
					TesterF.lchIcon=hYamnIcons[3];
					TesterN.lchIcon=hYamnIcons[1];

					lstrcpy(Tester.lpzContactName,Translate("Account Test"));
					lstrcpy(TesterF.lpzContactName,Translate("Account Test (failed)"));
					lstrcpy(TesterN.lpzContactName,Translate("Account Test"));
					lstrcpy(Tester.lpzText,Translate("You have N new mails"));
					lstrcpy(TesterF.lpzText,Translate("Connection failed message"));
					lstrcpy(TesterN.lpzText,Translate("No new mail"));
					if(TesterC)
					{
						Tester.colorBack=SendDlgItemMessage(hDlg,IDC_CPB,CPM_GETCOLOUR,0,0);
						Tester.colorText=SendDlgItemMessage(hDlg,IDC_CPT,CPM_GETCOLOUR,0,0);
					}
					else
					{
						Tester.colorBack=GetSysColor(COLOR_BTNFACE);
						Tester.colorText=GetSysColor(COLOR_WINDOWTEXT);
					}
					if(TesterFC)
					{
						TesterF.colorBack=SendDlgItemMessage(hDlg,IDC_CPFB,CPM_GETCOLOUR,0,0);
						TesterF.colorText=SendDlgItemMessage(hDlg,IDC_CPFT,CPM_GETCOLOUR,0,0);
					}
					else
					{
						TesterF.colorBack=GetSysColor(COLOR_BTNFACE);
						TesterF.colorText=GetSysColor(COLOR_WINDOWTEXT);
					}
					if(TesterNC)
					{
						TesterN.colorBack=SendDlgItemMessage(hDlg,IDC_CPNB,CPM_GETCOLOUR,0,0);
						TesterN.colorText=SendDlgItemMessage(hDlg,IDC_CPNT,CPM_GETCOLOUR,0,0);
					}
					else
					{
						TesterN.colorBack=GetSysColor(COLOR_BTNFACE);
						TesterN.colorText=GetSysColor(COLOR_WINDOWTEXT);
					}
					Tester.PluginWindowProc=(WNDPROC)NULL;
					TesterF.PluginWindowProc=(WNDPROC)NULL;
					TesterN.PluginWindowProc=(WNDPROC)NULL;
					Tester.PluginData=NULL;	
					TesterF.PluginData=NULL;
					TesterN.PluginData=NULL;

					if(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED)
						CallService(MS_POPUP_ADDPOPUP,(WPARAM)&Tester,0);
					if(IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED)
						CallService(MS_POPUP_ADDPOPUP,(WPARAM)&TesterF,0);
					if(IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED)
						CallService(MS_POPUP_ADDPOPUP,(WPARAM)&TesterN,0);
					Changed=TRUE;
				}
					break;
				case IDC_CHECKKBN:
					Changed=TRUE;
					break;
				case IDC_CHECKAPP:
					Changed=TRUE;
					EnableWindow(GetDlgItem(hDlg,IDC_BTNAPP),IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_EDITAPP),IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_EDITAPPPARAM),IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED);
					break;
				case IDC_CHECKPOP:
					Changed=TRUE;
					EnableWindow(GetDlgItem(hDlg,IDC_CHECKCOL),IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPB),IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPT),IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIOPOPN),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED));
					EnableWindow(GetDlgItem(hDlg,IDC_RADIOPOP1),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED));
					EnableWindow(GetDlgItem(hDlg,IDC_EDITPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED));
					break;
				case IDC_CHECKFPOP:
					Changed=TRUE;
					EnableWindow(GetDlgItem(hDlg,IDC_CHECKFCOL),IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPFB),IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPFT),IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_EDITFPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED));
					break;
				case IDC_CHECKNPOP:
					Changed=TRUE;
					EnableWindow(GetDlgItem(hDlg,IDC_CHECKNCOL),IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPNB),IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_CPNT),IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg,IDC_EDITNPOPS),(IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED));
					break;
				case IDC_BTNSTATUS:
				{
					DialogBoxParamW(pYAMNVar->hInst,MAKEINTRESOURCEW(IDD_CHOOSESTATUSMODES),hDlg,(DLGPROC)DlgProcPOP3AccStatusOpt,(LPARAM)NULL);										
					break;
				}
				
				case IDC_BTNAPP:
				{
					OPENFILENAME OFNStruct;

					memset(&OFNStruct,0,sizeof(OPENFILENAME));
					OFNStruct.lStructSize=sizeof(OPENFILENAME);
					OFNStruct.hwndOwner=hDlg;
					OFNStruct.lpstrFilter=_T("Executables (*.exe;*.bat;*.cmd;*.com)\0*.exe;*.bat;*.cmd;*.com\0All Files (*.*)\0*.*\0");
					OFNStruct.nFilterIndex=1;
					OFNStruct.nMaxFile=MAX_PATH;
					OFNStruct.lpstrFile=new TCHAR[MAX_PATH];
					OFNStruct.lpstrFile[0]=(TCHAR)0;
					OFNStruct.lpstrTitle=Translate("Select executable used for notification");
					OFNStruct.Flags=OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
					if(!GetOpenFileName(&OFNStruct))
					{
						if(CommDlgExtendedError())
							MessageBox(hDlg,_T("Dialog box error"),_T("Failed"),MB_OK);
					}
					else
						DlgSetItemText(hDlg,(WPARAM)IDC_EDITAPP,(LPARAM)OFNStruct.lpstrFile);
					delete[] OFNStruct.lpstrFile;
					break;
				}
				case IDC_BTNDEFAULT:
					DlgShowAccount(hDlg,(WPARAM)M_SHOWDEFAULT,0);
//					DlgShowAccountColors(hDlg,0,(LPARAM)ActualAccount);
					break;
				case IDC_BTNDEL:
					GetDlgItemText(hDlg,IDC_COMBOACCOUNT,DlgInput,sizeof(DlgInput)/sizeof(TCHAR));
					EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),FALSE);
					if((CB_ERR==(Result=SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_GETCURSEL,0,0)))
						|| (NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)DlgInput))))
						return TRUE;

					if(IDOK!=MessageBox(hDlg,Translate("Do you really want to delete this account?"),Translate("Delete account confirmation"),MB_OKCANCEL | MB_ICONWARNING))
						return TRUE;

					DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,(LPARAM)Translate("Please wait while no account is in use."));

					if(ActualAccount->hContact != NULL)
						CallService(MS_DB_CONTACT_DELETE,(WPARAM)(HANDLE) ActualAccount->hContact, 0);

					CallService(MS_YAMN_DELETEACCOUNT,(WPARAM)POP3Plugin,(LPARAM)ActualAccount);
					
					//We can consider our account as deleted.

					SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_DELETESTRING,(WPARAM)Result,0);
					DlgSetItemText(hDlg,(WPARAM)IDC_COMBOACCOUNT,(LPARAM)NULL);
					DlgEnableAccount(hDlg,(WPARAM)FALSE,0);
					DlgShowAccount(hDlg,(WPARAM)M_SHOWDEFAULT,0);
//					Beep(100,50);
					break;
				case IDC_BTNRESET:
					if(ActualAccount!=NULL)
						ActualAccount->TimeLeft=ActualAccount->Interval;
					return 1;
 			}
			if(HIWORD(wParam)==EN_CHANGE)
				Changed=TRUE;
			break;
		}
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch(((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							TCHAR Text[MAX_PATH];
							WCHAR TextW[MAX_PATH];
							BOOL Translated,NewAcc=FALSE,Check,CheckMsg,CheckSnd,CheckIco,CheckPopup,CheckPopupW,CheckApp;
							BOOL CheckNPopup,CheckNPopupW,CheckNMsgP,CheckFMsg,CheckFSnd,CheckFIco,CheckFPopup,CheckFPopupW;
							BOOL CheckPopN,CheckKBN, CheckContact,CheckContactNick;
							BOOL CheckSSL,CheckAPOP, CheckNoTLS;
							//BOOL Check0,Check1,Check2,Check3,Check4,Check5,Check6,Check7,Check8,Check9,
							BOOL CheckStart,CheckForce;
							int Length,index;
							UINT Port,Interval,Time,TimeN,TimeF;

							if(GetDlgItemText(hDlg,IDC_COMBOACCOUNT,Text,sizeof(Text)/sizeof(TCHAR)))
							{
								Check=(IsDlgButtonChecked(hDlg,IDC_CHECK)==BST_CHECKED);
								CheckSSL=(IsDlgButtonChecked(hDlg,IDC_CHECKSSL)==BST_CHECKED);
								CheckNoTLS=(IsDlgButtonChecked(hDlg,IDC_CHECKNOTLS)==BST_CHECKED);
								CheckAPOP=(IsDlgButtonChecked(hDlg,IDC_CHECKAPOP)==BST_CHECKED);
								CheckMsg=(IsDlgButtonChecked(hDlg,IDC_CHECKMSG)==BST_CHECKED);
								CheckSnd=(IsDlgButtonChecked(hDlg,IDC_CHECKSND)==BST_CHECKED);
								CheckIco=(IsDlgButtonChecked(hDlg,IDC_CHECKICO)==BST_CHECKED);
								CheckPopup=(IsDlgButtonChecked(hDlg,IDC_CHECKPOP)==BST_CHECKED);
								CheckPopupW=(IsDlgButtonChecked(hDlg,IDC_CHECKCOL)==BST_CHECKED);
								CheckApp=(IsDlgButtonChecked(hDlg,IDC_CHECKAPP)==BST_CHECKED);
								CheckKBN=(IsDlgButtonChecked(hDlg,IDC_CHECKKBN)==BST_CHECKED);
								CheckContact=(IsDlgButtonChecked(hDlg,IDC_CHECKCONTACT)==BST_CHECKED);
								CheckContactNick=(IsDlgButtonChecked(hDlg,IDC_CHECKCONTACTNICK)==BST_CHECKED);

								CheckFSnd=(IsDlgButtonChecked(hDlg,IDC_CHECKFSND)==BST_CHECKED);
								CheckFMsg=(IsDlgButtonChecked(hDlg,IDC_CHECKFMSG)==BST_CHECKED);
								CheckFIco=(IsDlgButtonChecked(hDlg,IDC_CHECKFICO)==BST_CHECKED);
								CheckFPopup=(IsDlgButtonChecked(hDlg,IDC_CHECKFPOP)==BST_CHECKED);
								CheckFPopupW=(IsDlgButtonChecked(hDlg,IDC_CHECKFCOL)==BST_CHECKED);

								CheckNPopup=(IsDlgButtonChecked(hDlg,IDC_CHECKNPOP)==BST_CHECKED);
								CheckNPopupW=(IsDlgButtonChecked(hDlg,IDC_CHECKNCOL)==BST_CHECKED);
								CheckNMsgP=(IsDlgButtonChecked(hDlg,IDC_CHECKNMSGP)==BST_CHECKED);

								Port=GetDlgItemInt(hDlg,IDC_EDITPORT,&Translated,FALSE);
								if(!Translated)
								{
									MessageBox(hDlg,Translate("This is not a valid number value"),Translate("Input error"),MB_OK);
									SetFocus(GetDlgItem(hDlg,IDC_EDITPORT));
								        break;
								}
								Interval=GetDlgItemInt(hDlg,IDC_EDITINTERVAL,&Translated,FALSE);
								if(!Translated)
								{
									MessageBox(hDlg,Translate("This is not a valid number value"),Translate("Input error"),MB_OK);
									SetFocus(GetDlgItem(hDlg,IDC_EDITINTERVAL));
								        break;
								}
								Time=GetDlgItemInt(hDlg,IDC_EDITPOPS,&Translated,FALSE);
								if(!Translated)
								{
									MessageBox(hDlg,Translate("This is not a valid number value"),Translate("Input error"),MB_OK);
									SetFocus(GetDlgItem(hDlg,IDC_EDITPOPS));
								        break;
								}
								TimeN=GetDlgItemInt(hDlg,IDC_EDITNPOPS,&Translated,FALSE);
								if(!Translated)
								{
									MessageBox(hDlg,Translate("This is not a valid number value"),Translate("Input error"),MB_OK);
									SetFocus(GetDlgItem(hDlg,IDC_EDITNPOPS));
								        break;
								}
								TimeF=GetDlgItemInt(hDlg,IDC_EDITFPOPS,&Translated,FALSE);
								if(!Translated)
								{
									MessageBox(hDlg,Translate("This is not a valid number value"),Translate("Input error"),MB_OK);
									SetFocus(GetDlgItem(hDlg,IDC_EDITFPOPS));
								        break;
								}
				        
								if(Check && !CheckMsg && !CheckSnd && !CheckIco && !CheckApp && !CheckPopup)
								{
									MessageBox(hDlg,Translate(_T("At least one mail notification event must be checked")),Translate(_T("Input error")),MB_OK);
									break;
								}
				        
								GetDlgItemText(hDlg,IDC_EDITAPP,Text,sizeof(Text)/sizeof(TCHAR));
								if(CheckApp && !(Length=_tcslen(Text)))
								{
									MessageBox(hDlg,Translate(_T("Please select application to run")),Translate(_T("Input error")),MB_OK);
									break;
								}
				        
								GetDlgItemText(hDlg,IDC_COMBOACCOUNT,Text,sizeof(Text)/sizeof(TCHAR));
								if(!(Length=_tcslen(Text)))
									break;
				        
								DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,(LPARAM)Translate("Please wait while no account is in use."));
				        
								if(NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)Text)))
								{
									NewAcc=TRUE;
									#ifdef DEBUG_SYNCHRO                    
									DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write wait\n");
									#endif                                  
									WaitToWriteSO(POP3Plugin->AccountBrowserSO);
									#ifdef DEBUG_SYNCHRO                    
									DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write enter\n");
									#endif                                  
									if(NULL==(ActualAccount=(HPOP3ACCOUNT)CallService(MS_YAMN_GETNEXTFREEACCOUNT,(WPARAM)POP3Plugin,(LPARAM)YAMN_ACCOUNTVERSION)))
									{
										#ifdef DEBUG_SYNCHRO                    
										DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write done\n");
										#endif                                  
										WriteDoneSO(POP3Plugin->AccountBrowserSO);
										MessageBox(hDlg,Translate("Cannot allocate memory space for new account"),Translate("Memory error"),MB_OK);
										break;
									}
								}
								else
								{
									#ifdef DEBUG_SYNCHRO                    
									DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write wait\n");
									#endif                                  
									//We have to get full access to AccountBrowser, so other iterating thrads cannot get new account until new account is right set
									WaitToWriteSO(POP3Plugin->AccountBrowserSO);
									#ifdef DEBUG_SYNCHRO                    
									DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write enter\n");
									#endif                                  
								}
								#ifdef DEBUG_SYNCHRO
								DebugLog(SynchroFile,"Options:APPLY:ActualAccountSO-write wait\n");
								#endif
								if(WAIT_OBJECT_0!=WaitToWrite(ActualAccount))
								{
									#ifdef DEBUG_SYNCHRO
									DebugLog(SynchroFile,"Options:APPLY:ActualAccountSO-write wait failed\n");
									#endif
									#ifdef DEBUG_SYNCHRO
									DebugLog(SynchroFile,"Options:APPLY:ActualBrowserSO-write done\n");
									#endif
									WriteDoneSO(POP3Plugin->AccountBrowserSO);

								}
								#ifdef DEBUG_SYNCHRO
								DebugLog(SynchroFile,"Options:APPLY:ActualAccountSO-write enter\n");
								#endif
				        
//								Beep(1000,100);Sleep(200);
								if(NULL==ActualAccount->Name)
									ActualAccount->Name=new TCHAR[Length+1];
								_tcscpy(ActualAccount->Name,Text);

								
				        
//								Beep(1000,100);Sleep(200);
								GetDlgItemText(hDlg,IDC_EDITSERVER,Text,sizeof(Text)/sizeof(TCHAR));
								if(NULL!=ActualAccount->Server->Name)
									delete[] ActualAccount->Server->Name;
								ActualAccount->Server->Name=new TCHAR[_tcslen(Text)+1];
								_tcscpy(ActualAccount->Server->Name,Text);
				        
//								Beep(1000,100);Sleep(200);
								GetDlgItemText(hDlg,IDC_EDITLOGIN,Text,sizeof(Text)/sizeof(TCHAR));
								if(NULL!=ActualAccount->Server->Login)
									delete[] ActualAccount->Server->Login;
								ActualAccount->Server->Login=new TCHAR[_tcslen(Text)+1];
								_tcscpy(ActualAccount->Server->Login,Text);
				        
//								Beep(1000,100);Sleep(200);
								GetDlgItemText(hDlg,IDC_EDITPASS,Text,sizeof(Text)/sizeof(TCHAR));
								if(NULL!=ActualAccount->Server->Passwd)
									delete[] ActualAccount->Server->Passwd;
								ActualAccount->Server->Passwd=new TCHAR[_tcslen(Text)+1];
								_tcscpy(ActualAccount->Server->Passwd,Text);
				        
//								Beep(1000,100);Sleep(200);
								GetDlgItemTextW(hDlg,IDC_EDITAPP,TextW,sizeof(TextW)/sizeof(WCHAR));
								if(NULL!=ActualAccount->NewMailN.App)
									delete[] ActualAccount->NewMailN.App;
								ActualAccount->NewMailN.App=new WCHAR[wcslen(TextW)+1];
								wcscpy(ActualAccount->NewMailN.App,TextW);
				        
//								Beep(1000,100);Sleep(200);
								GetDlgItemTextW(hDlg,IDC_EDITAPPPARAM,TextW,sizeof(TextW)/sizeof(WCHAR));
								if(NULL!=ActualAccount->NewMailN.AppParam)
									delete[] ActualAccount->NewMailN.AppParam;
								ActualAccount->NewMailN.AppParam=new WCHAR[wcslen(TextW)+1];
								wcscpy(ActualAccount->NewMailN.AppParam,TextW);
				        
								ActualAccount->Server->Port=Port;
								ActualAccount->Interval=Interval*60;
				        
								ActualAccount->NewMailN.PopUpB=SendDlgItemMessage(hDlg,IDC_CPB,CPM_GETCOLOUR,0,0);
								ActualAccount->NewMailN.PopUpT=SendDlgItemMessage(hDlg,IDC_CPT,CPM_GETCOLOUR,0,0);
								ActualAccount->NewMailN.PopUpTime=Time;
				        
								ActualAccount->NoNewMailN.PopUpB=SendDlgItemMessage(hDlg,IDC_CPNB,CPM_GETCOLOUR,0,0);
								ActualAccount->NoNewMailN.PopUpT=SendDlgItemMessage(hDlg,IDC_CPNT,CPM_GETCOLOUR,0,0);
								ActualAccount->NoNewMailN.PopUpTime=TimeN;
								
								ActualAccount->BadConnectN.PopUpB=SendDlgItemMessage(hDlg,IDC_CPFB,CPM_GETCOLOUR,0,0);
								ActualAccount->BadConnectN.PopUpT=SendDlgItemMessage(hDlg,IDC_CPFT,CPM_GETCOLOUR,0,0);
								ActualAccount->BadConnectN.PopUpTime=TimeF;
								
//								Beep(1000,100);Sleep(200);
								if(CB_ERR==(index=SendDlgItemMessage(hDlg,IDC_COMBOCP,CB_GETCURSEL,0,0)))
									index=CPDEFINDEX;
								ActualAccount->CP=CodePageNames[index].CP;
				        
//								Beep(1000,100);Sleep(200);
								if(NewAcc)
									ActualAccount->TimeLeft=Interval*60;
				        
								CheckPopN=(IsDlgButtonChecked(hDlg,IDC_RADIOPOPN)==BST_CHECKED);
				        
								/*Check0=(IsDlgButtonChecked(hDlg,IDC_CHECKST0)==BST_CHECKED);
								Check1=(IsDlgButtonChecked(hDlg,IDC_CHECKST1)==BST_CHECKED);
								Check2=(IsDlgButtonChecked(hDlg,IDC_CHECKST2)==BST_CHECKED);
								Check3=(IsDlgButtonChecked(hDlg,IDC_CHECKST3)==BST_CHECKED);
								Check4=(IsDlgButtonChecked(hDlg,IDC_CHECKST4)==BST_CHECKED);
								Check5=(IsDlgButtonChecked(hDlg,IDC_CHECKST5)==BST_CHECKED);
								Check6=(IsDlgButtonChecked(hDlg,IDC_CHECKST6)==BST_CHECKED);
								Check7=(IsDlgButtonChecked(hDlg,IDC_CHECKST7)==BST_CHECKED);
								Check8=(IsDlgButtonChecked(hDlg,IDC_CHECKST8)==BST_CHECKED);
								Check9=(IsDlgButtonChecked(hDlg,IDC_CHECKST9)==BST_CHECKED);*/
							
								CheckStart=(IsDlgButtonChecked(hDlg,IDC_CHECKSTART)==BST_CHECKED);
								CheckForce=(IsDlgButtonChecked(hDlg,IDC_CHECKFORCE)==BST_CHECKED);
				        
								ActualAccount->Flags=
									(Check ? YAMN_ACC_ENA : 0) |
					        			(CheckSSL ? YAMN_ACC_SSL23 : 0) |
					        			(CheckNoTLS ? YAMN_ACC_NOTLS : 0) |
					        			(CheckAPOP ? YAMN_ACC_APOP : 0) |
									(CheckPopN ? YAMN_ACC_POPN : 0);
				        
								ActualAccount->StatusFlags=
									(Check0 ? YAMN_ACC_ST0 : 0) |
									(Check1 ? YAMN_ACC_ST1 : 0) |
									(Check2 ? YAMN_ACC_ST2 : 0) |
									(Check3 ? YAMN_ACC_ST3 : 0) |
									(Check4 ? YAMN_ACC_ST4 : 0) |
									(Check5 ? YAMN_ACC_ST5 : 0) |
									(Check6 ? YAMN_ACC_ST6 : 0) |
									(Check7 ? YAMN_ACC_ST7 : 0) |
									(Check8 ? YAMN_ACC_ST8 : 0) |
									(Check9 ? YAMN_ACC_ST9 : 0) |
									(CheckStart ? YAMN_ACC_STARTS : 0) |
									(CheckForce ? YAMN_ACC_FORCE : 0);

								ActualAccount->NewMailN.Flags=
									(CheckSnd ? YAMN_ACC_SND : 0) |
									(CheckMsg ? YAMN_ACC_MSG : 0) |
									(CheckIco ? YAMN_ACC_ICO : 0) |
									(CheckPopup ? YAMN_ACC_POP : 0) |
									(CheckPopupW ? YAMN_ACC_POPC : 0) |
									(CheckApp ? YAMN_ACC_APP : 0) |
									(CheckKBN ? YAMN_ACC_KBN : 0) |
									(CheckContact ? YAMN_ACC_CONT : 0) |
									(CheckContactNick ? YAMN_ACC_CONTNICK : 0) |
									YAMN_ACC_MSGP;			//this is default: when new mail arrives and window was displayed, leave it displayed.

								ActualAccount->NoNewMailN.Flags=
									(CheckNPopup ? YAMN_ACC_POP : 0) |
									(CheckNPopupW ? YAMN_ACC_POPC : 0) |
									(CheckNMsgP ? YAMN_ACC_MSGP : 0);

								ActualAccount->BadConnectN.Flags=
									(CheckFSnd ? YAMN_ACC_SND : 0) |
									(CheckFMsg ? YAMN_ACC_MSG : 0) |
									(CheckFIco ? YAMN_ACC_ICO : 0) |
									(CheckFPopup ? YAMN_ACC_POP : 0) |
									(CheckFPopupW ? YAMN_ACC_POPC : 0);

								#ifdef DEBUG_SYNCHRO
								DebugLog(SynchroFile,"Options:APPLY:ActualAccountSO-write done\n");
								#endif
								WriteDone(ActualAccount);
								#ifdef DEBUG_SYNCHRO                    
								DebugLog(SynchroFile,"Options:APPLY:AccountBrowserSO-write done\n");
								#endif                                  
								WriteDoneSO(POP3Plugin->AccountBrowserSO);
								if(NewAcc)
								{
									index=SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_ADDSTRING,0,(LPARAM)ActualAccount->Name);
									if((index==CB_ERR) || (index==CB_ERRSPACE))
										break;
									SendDlgItemMessage(hDlg,IDC_COMBOACCOUNT,CB_SETCURSEL,(WPARAM)index,(LPARAM)ActualAccount->Name);
								}
								EnableWindow(GetDlgItem(hDlg,IDC_BTNDEL),TRUE);
							
								DlgSetItemText(hDlg,(WPARAM)IDC_STTIMELEFT,(LPARAM)NULL);

//								if(0==WritePOP3Accounts())
//									Beep(500,100);
								WritePOP3Accounts();
								RefreshContact();
								return TRUE;
							}
						}
						break;
					}
					break;
			}
			break;
	}
	if(Changed)
		SendMessage(GetParent(hDlg),PSM_CHANGED,0,0);
	return FALSE;
}

int POP3OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp={0};
	
	odp.cbSize=sizeof(odp);
	odp.position=0x00000000;
	odp.hInstance=pYAMNVar->hInst;
	odp.pszGroup=Translate("Plugins");
	odp.flags=ODPF_BOLDGROUPS;
//insert POP3 account options dialog
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_POP3ACCOUNTOPT);
	odp.pszTitle="YAMN-POP3";
	odp.pfnDlgProc=(DLGPROC)DlgProcPOP3AccOpt;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	return 0;
}
