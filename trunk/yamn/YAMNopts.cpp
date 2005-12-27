/*
 * This code implements YAMN general options window handling
 *
 * (c) majvan 2002-2004
 */

#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_utils.h>
#include "m_yamn.h"
#include "m_protoplugin.h"
#include "m_filterplugin.h"
#include "m_account.h"
#include "m_messages.h"
#include "main.h"
#include "resources/resource.h"

//- imported ---------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//From main.cpp
extern YAMN_VARIABLES YAMNVar;
extern DWORD HotKeyThreadID;
extern LPCRITICAL_SECTION PluginRegCS;
//From filterplugin.cpp
extern PYAMN_FILTERPLUGINQUEUE FirstFilterPlugin;
//From protoplugin.cpp
extern PYAMN_PROTOPLUGINQUEUE FirstProtoPlugin;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

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
			break;
		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);
			switch(LOWORD(wParam))
			{
				case IDC_CHECKTTB:
				case IDC_HKFORCE:
					SendMessage(GetParent(hDlg),PSM_CHANGED,0,0);
					break;
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
			
				SendDlgItemMessage(hDlg,IDC_HKFORCE,HKM_SETHOTKEY,DBGetContactSettingWord(NULL,YAMN_DBMODULE,YAMN_HKCHECKMAIL,YAMN_DEFAULTHK),0);

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
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch(((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							WORD ForceHotKey=(WORD)SendDlgItemMessage(hDlg,IDC_HKFORCE,HKM_GETHOTKEY,0,0);
							BYTE TTBFCheck=(BYTE)IsDlgButtonChecked(hDlg,IDC_CHECKTTB);
							UINT mod,vk;

							DBWriteContactSettingWord(NULL,YAMN_DBMODULE,YAMN_HKCHECKMAIL,ForceHotKey);
							DBWriteContactSettingByte(NULL,YAMN_DBMODULE,YAMN_TTBFCHECK,TTBFCheck);
							WordToModAndVk(ForceHotKey,&mod,&vk);
							PostThreadMessage(HotKeyThreadID,WM_YAMN_CHANGEHOTKEY,(WPARAM)mod,(LPARAM)vk);
						}
					}
			}
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
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_YAMNOPT);
	odp.pfnDlgProc=(DLGPROC)DlgProcYAMNOpt;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	return 0;
}
