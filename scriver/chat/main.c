/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "chat.h"

extern struct GlobalMessageData *g_dat;
//globals
HINSTANCE   g_hInst;
HANDLE      g_hWindowList;
HMENU       g_hMenu = NULL;


FONTINFO    aFonts[OPTIONS_FONTCOUNT];
HICON       hIcons[30];
BOOL        SmileyAddInstalled = FALSE;
BOOL        IEviewInstalled = FALSE;
BOOL        PopUpInstalled = FALSE;
HBRUSH      hEditBkgBrush = NULL;
HBRUSH      hListBkgBrush = NULL;

HIMAGELIST  hImageList = NULL;

HIMAGELIST  hIconsList = NULL;
int         eventMessageIcon = 0;
int			overlayIcon = 0;

TCHAR*      pszActiveWndID = 0;
char*       pszActiveWndModule = 0;

struct GlobalLogSettings_t g_Settings;

static void InitREOleCallback(void);


static PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	#ifdef _UNICODE
		"Chat (Unicode)",
	#else
		"Chat",
	#endif
	PLUGIN_MAKE_VERSION(0,6,0,1),
	"Provides chat rooms for protocols supporting it",
	"MatriX",
	"project-info@miranda-im.org",
	"© 2003 - 2005 Jörgen Persson",
	"http://miranda-im.org/",
	0,
	0
};

int Chat_Load(PLUGINLINK *link)
{
	BOOL bFlag = FALSE;
	HINSTANCE hDll;

	// set the memory manager
	memoryManagerInterface.cbSize = sizeof(struct MM_INTERFACE);
	CallService(MS_SYSTEM_GET_MMI,0,(LPARAM)&memoryManagerInterface);

	hDll = LoadLibraryA("riched20.dll");
	if ( hDll ) {
		char modulePath[MAX_PATH];
		if (GetModuleFileNameA(hDll, modulePath, MAX_PATH)) {
			DWORD dummy;
			VS_FIXEDFILEINFO* vsInfo;
			UINT vsInfoSize;
			DWORD size = GetFileVersionInfoSizeA(modulePath, &dummy);
			BYTE* buffer = (BYTE*) mir_alloc(size);

			GetFileVersionInfoA(modulePath, 0, size, buffer);
			VerQueryValueA(buffer, "\\", (LPVOID*) &vsInfo, &vsInfoSize);
			if(LOWORD(vsInfo->dwFileVersionMS) != 0)
				bFlag= TRUE;

			mir_free(buffer);
		}
	}

	if ( !bFlag ) {
		if(IDYES == MessageBoxA(0,Translate("Miranda could not load the Chat plugin because Microsoft Rich Edit v 3 is missing.\nIf you are using Windows 95/98/NT or WINE please upgrade your Rich Edit control.\n\nDo you want to download an update now?."),Translate("Information"),MB_YESNO|MB_ICONINFORMATION))
			CallService(MS_UTILS_OPENURL, 1, (LPARAM) "http://members.chello.se/matrix/re3/richupd.exe");
		FreeLibrary(GetModuleHandleA("riched20.dll"));
		return 1;
	}

	UpgradeCheck();

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
    OleInitialize(NULL);
    InitREOleCallback();
	HookEvents();
	CreateServiceFunctions();
	CreateHookableEvents();
	OptionsInit();
	return 0;
}


int Chat_Unload(void)
{
	DBWriteContactSettingWord(NULL, "Chat", "SplitterX", (WORD)g_Settings.iSplitterX);
	DBWriteContactSettingWord(NULL, "Chat", "SplitterY", (WORD)g_Settings.iSplitterY);

	CList_SetAllOffline(TRUE);

	mir_free( pszActiveWndID );
	mir_free( pszActiveWndModule );

	DestroyMenu(g_hMenu);
	DestroyServiceFunctions();
	FreeIcons();
	OptionsUnInit();
	FreeLibrary(GetModuleHandleA("riched20.dll"));
	OleUninitialize();
	UnhookEvents();
	return 0;
}

void UpgradeCheck(void)
{
	DWORD dwVersion = DBGetContactSettingDword(NULL, "Chat", "OldVersion", PLUGIN_MAKE_VERSION(0,2,9,9));
	if(	pluginInfo.version > dwVersion)
	{
		if(dwVersion < PLUGIN_MAKE_VERSION(0,3,0,0))
		{
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font18");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font18Col");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font18Set");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font18Size");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font18Sty");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font19");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font19Col");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font19Set");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font19Size");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font19Sty");
			DBDeleteContactSetting(NULL, "Chat",		"ColorNicklistLines");
			DBDeleteContactSetting(NULL, "Chat",		"NicklistIndent");
			DBDeleteContactSetting(NULL, "Chat",		"NicklistRowDist");
			DBDeleteContactSetting(NULL, "Chat",		"ShowFormatButtons");
			DBDeleteContactSetting(NULL, "Chat",		"ShowLines");
			DBDeleteContactSetting(NULL, "Chat",		"ShowName");
			DBDeleteContactSetting(NULL, "Chat",		"ShowTopButtons");
			DBDeleteContactSetting(NULL, "Chat",		"SplitterX");
			DBDeleteContactSetting(NULL, "Chat",		"SplitterY");
			DBDeleteContactSetting(NULL, "Chat",		"IconFlags");
			DBDeleteContactSetting(NULL, "Chat",		"LogIndentEnabled");
		}
	}
	DBWriteContactSettingDword(NULL, "Chat", "OldVersion", pluginInfo.version);
	return;
}

void LoadLogIcons(void)
{
	hIcons[ICON_ACTION] = LoadIconEx(IDI_ACTION, "log_action", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_ACTION),IMAGE_ICON,0,0,0);
	hIcons[ICON_ADDSTATUS] = LoadIconEx(IDI_ADDSTATUS, "log_addstatus", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_ADDSTATUS),IMAGE_ICON,0,0,0);
	hIcons[ICON_HIGHLIGHT] = LoadIconEx(IDI_HIGHLIGHT, "log_highlight", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_HIGHLIGHT),IMAGE_ICON,0,0,0);
	hIcons[ICON_INFO] = LoadIconEx(IDI_INFO, "log_info", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_INFO),IMAGE_ICON,0,0,0);
	hIcons[ICON_JOIN] = LoadIconEx(IDI_JOIN, "log_join", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_JOIN),IMAGE_ICON,0,0,0);
	hIcons[ICON_KICK] = LoadIconEx(IDI_KICK, "log_kick", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_KICK),IMAGE_ICON,0,0,0);
	hIcons[ICON_MESSAGE] = LoadIconEx(IDI_MESSAGE, "log_message_in", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_MESSAGE),IMAGE_ICON,0,0,0);
	hIcons[ICON_MESSAGEOUT] = LoadIconEx(IDI_MESSAGEOUT, "log_message_out", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_MESSAGEOUT),IMAGE_ICON,0,0,0);
	hIcons[ICON_NICK] = LoadIconEx(IDI_NICK, "log_nick", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_NICK),IMAGE_ICON,0,0,0);
	hIcons[ICON_NOTICE] = LoadIconEx(IDI_CHAT_NOTICE, "log_notice", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_NOTICE),IMAGE_ICON,0,0,0);
	hIcons[ICON_PART] = LoadIconEx(IDI_PART, "log_part", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_PART),IMAGE_ICON,0,0,0);
	hIcons[ICON_QUIT] = LoadIconEx(IDI_QUIT, "log_quit", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_QUIT),IMAGE_ICON,0,0,0);
	hIcons[ICON_REMSTATUS] = LoadIconEx(IDI_REMSTATUS, "log_removestatus", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_REMSTATUS),IMAGE_ICON,0,0,0);
	hIcons[ICON_TOPIC] = LoadIconEx(IDI_TOPIC, "log_topic", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_TOPIC),IMAGE_ICON,0,0,0);
	hIcons[ICON_STATUS1] = LoadIconEx(IDI_STATUS1, "status1", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_STATUS1),IMAGE_ICON,0,0,0);
	hIcons[ICON_STATUS2] = LoadIconEx(IDI_STATUS2, "status2", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_STATUS2),IMAGE_ICON,0,0,0);
	hIcons[ICON_STATUS3] = LoadIconEx(IDI_STATUS3, "status3", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_STATUS3),IMAGE_ICON,0,0,0);
	hIcons[ICON_STATUS4] = LoadIconEx(IDI_STATUS4, "status4", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_STATUS4),IMAGE_ICON,0,0,0);
	hIcons[ICON_STATUS0] = LoadIconEx(IDI_STATUS0, "status0", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_STATUS0),IMAGE_ICON,0,0,0);
	hIcons[ICON_STATUS5] = LoadIconEx(IDI_STATUS5, "status5", 10, 10); //LoadImage(g_hInst,MAKEINTRESOURCE(IDI_STATUS5),IMAGE_ICON,0,0,0);

	return;
}
void LoadIcons(void)
{
	int i;

	for(i = 0; i < 20; i++)
		hIcons[i] = NULL;

	LoadLogIcons();
	LoadMsgLogBitmaps();

	hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK,0,3);
	hIconsList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK,0,100);
	eventMessageIcon = ImageList_AddIcon(g_dat->hTabIconList,LoadSkinnedIcon( SKINICON_EVENT_MESSAGE));
	overlayIcon = ImageList_AddIcon(g_dat->hTabIconList,LoadIconEx(IDI_OVERLAY, "overlay", 0, 0));
	ImageList_SetOverlayImage(g_dat->hTabIconList, overlayIcon, 1);
	ImageList_AddIcon_Ex2(hImageList,LoadImage(g_hInst,MAKEINTRESOURCE(IDI_BLANK),IMAGE_ICON,0,0,0));
	ImageList_AddIcon_Ex2(hImageList,LoadImage(g_hInst,MAKEINTRESOURCE(IDI_BLANK),IMAGE_ICON,0,0,0));
	return ;
}

void FreeIcons(void)
{
	FreeMsgLogBitmaps();
	ImageList_Destroy(hImageList);
//	ImageList_Destroy(hIconsList);
	return;
}

static IRichEditOleCallbackVtbl reOleCallbackVtbl;
struct CREOleCallback reOleCallback;

static STDMETHODIMP_(ULONG) CREOleCallback_QueryInterface(struct CREOleCallback *lpThis, REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, &IID_IRichEditOleCallback)) {
        *ppvObj = lpThis;
        lpThis->lpVtbl->AddRef((IRichEditOleCallback *) lpThis);
        return S_OK;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

static STDMETHODIMP_(ULONG) CREOleCallback_AddRef(struct CREOleCallback *lpThis)
{
    if (lpThis->refCount == 0) {
        if (S_OK != StgCreateDocfile(NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &lpThis->pictStg))
            lpThis->pictStg = NULL;
        lpThis->nextStgId = 0;
    }
    return ++lpThis->refCount;
}

static STDMETHODIMP_(ULONG) CREOleCallback_Release(struct CREOleCallback *lpThis)
{
    if (--lpThis->refCount == 0) {
        if (lpThis->pictStg)
            lpThis->pictStg->lpVtbl->Release(lpThis->pictStg);
    }
    return lpThis->refCount;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_ContextSensitiveHelp(struct CREOleCallback *lpThis, BOOL fEnterMode)
{
    return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_DeleteObject(struct CREOleCallback *lpThis, LPOLEOBJECT lpoleobj)
{
    return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetClipboardData(struct CREOleCallback *lpThis, CHARRANGE * lpchrg, DWORD reco, LPDATAOBJECT * lplpdataobj)
{
    return E_NOTIMPL;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetContextMenu(struct CREOleCallback *lpThis, WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE * lpchrg, HMENU * lphmenu)
{
    return E_INVALIDARG;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetDragDropEffect(struct CREOleCallback *lpThis, BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
    return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetInPlaceContext(struct CREOleCallback *lpThis, LPOLEINPLACEFRAME * lplpFrame, LPOLEINPLACEUIWINDOW * lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    return E_INVALIDARG;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetNewStorage(struct CREOleCallback *lpThis, LPSTORAGE * lplpstg)
{
    WCHAR szwName[64];
    char szName[64];
    wsprintfA(szName, "s%u", lpThis->nextStgId);
	MultiByteToWideChar(CP_ACP, 0, szName, -1, szwName, SIZEOF(szwName));
    if (lpThis->pictStg == NULL)
        return STG_E_MEDIUMFULL;
    return lpThis->pictStg->lpVtbl->CreateStorage(lpThis->pictStg, szwName, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, lplpstg);
}

static STDMETHODIMP_(HRESULT) CREOleCallback_QueryAcceptData(struct CREOleCallback *lpThis, LPDATAOBJECT lpdataobj, CLIPFORMAT * lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
{
    return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_QueryInsertObject(struct CREOleCallback *lpThis, LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
{
    return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_ShowContainerUI(struct CREOleCallback *lpThis, BOOL fShow)
{
    return S_OK;
}

static void InitREOleCallback(void)
{
    reOleCallback.lpVtbl = &reOleCallbackVtbl;
    reOleCallback.lpVtbl->AddRef = (ULONG(__stdcall *) (IRichEditOleCallback *)) CREOleCallback_AddRef;
    reOleCallback.lpVtbl->Release = (ULONG(__stdcall *) (IRichEditOleCallback *)) CREOleCallback_Release;
    reOleCallback.lpVtbl->QueryInterface = (ULONG(__stdcall *) (IRichEditOleCallback *, REFIID, PVOID *)) CREOleCallback_QueryInterface;
    reOleCallback.lpVtbl->ContextSensitiveHelp = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL)) CREOleCallback_ContextSensitiveHelp;
    reOleCallback.lpVtbl->DeleteObject = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPOLEOBJECT)) CREOleCallback_DeleteObject;
    reOleCallback.lpVtbl->GetClipboardData = (HRESULT(__stdcall *) (IRichEditOleCallback *, CHARRANGE *, DWORD, LPDATAOBJECT *)) CREOleCallback_GetClipboardData;
    reOleCallback.lpVtbl->GetContextMenu = (HRESULT(__stdcall *) (IRichEditOleCallback *, WORD, LPOLEOBJECT, CHARRANGE *, HMENU *)) CREOleCallback_GetContextMenu;
    reOleCallback.lpVtbl->GetDragDropEffect = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL, DWORD, LPDWORD)) CREOleCallback_GetDragDropEffect;
	reOleCallback.lpVtbl->GetInPlaceContext = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPOLEINPLACEFRAME *, LPOLEINPLACEUIWINDOW *, LPOLEINPLACEFRAMEINFO))CREOleCallback_GetInPlaceContext;
    reOleCallback.lpVtbl->GetNewStorage = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPSTORAGE *)) CREOleCallback_GetNewStorage;
    reOleCallback.lpVtbl->QueryAcceptData = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPDATAOBJECT, CLIPFORMAT *, DWORD, BOOL, HGLOBAL)) CREOleCallback_QueryAcceptData;
    reOleCallback.lpVtbl->QueryInsertObject = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPCLSID, LPSTORAGE, LONG)) CREOleCallback_QueryInsertObject;
    reOleCallback.lpVtbl->ShowContainerUI = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL)) CREOleCallback_ShowContainerUI;
    reOleCallback.refCount = 0;
}
