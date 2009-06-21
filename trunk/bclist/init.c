/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2005 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "commonheaders.h"
#include <m_icolib.h>

HINSTANCE g_hInst = 0;
PLUGINLINK *pluginLink;
CLIST_INTERFACE* pcli = NULL;
HIMAGELIST himlCListClc = NULL;

struct MM_INTERFACE mmi;
BOOL(WINAPI * MySetLayeredWindowAttributes) (HWND, COLORREF, BYTE, DWORD) = NULL;

LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RebuildEntireList(HWND hwnd, struct ClcData *dat);
void RebuildEntireListInternal(HWND hwnd, struct ClcData *dat, BOOL call_orig);
void SetGroupExpand(HWND hwnd, struct ClcData *dat, struct ClcGroup *group, int newState);
void ScrollTo( HWND hwnd, struct ClcData *dat, int desty, int noSmooth );
void RecalcScrollBar( HWND hwnd, struct ClcData *dat );
void LoadClcOptions( HWND hwnd, struct ClcData *dat );
int GetRowHeight(struct ClcData *dat, int item);
void SortCLC(HWND hwnd, struct ClcData *dat, int useInsertionSort);

LRESULT ( CALLBACK *pfnContactListWndProc )( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT ( CALLBACK *pfnContactListControlWndProc )( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
void ( *pfnRebuildEntireList )( HWND hwnd, struct ClcData *dat );
void ( *pfnSetGroupExpand )(HWND hwnd, struct ClcData *dat, struct ClcGroup *group, int newState);
void ( *pfnScrollTo )( HWND hwnd, struct ClcData *dat, int desty, int noSmooth );
void ( *pfnRecalcScrollBar )( HWND hwnd, struct ClcData *dat );
void ( *pfnLoadClcOptions )( HWND hwnd, struct ClcData *dat );
int ( *pfnGetRowHeight )(struct ClcData *dat, int item);
void ( *pfnSortCLC )( HWND hwnd, struct ClcData *dat, int useInsertionSort );


/////////////////////////////////////////////////////////////////////////////////////////
// external functions

void InitCustomMenus( void );
void PaintClc(HWND hwnd, struct ClcData *dat, HDC hdc, RECT * rcPaint);

int ClcOptInit(WPARAM wParam, LPARAM lParam);
int CluiOptInit(WPARAM wParam, LPARAM lParam);
int CListOptInit(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// dll stub

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns the plugin information

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFO),
	#if defined( _UNICODE )
		"BClist (Unicode)",
	#else
		"BClist",
	#endif
	PLUGIN_MAKE_VERSION(0, 8, 0, 1),
	"A contact list for blind folks",
	"Ricardo Pescuma Domenecci, based on previous work from Miranda IM project",
	"",
	"Copyright 2000-2009 Miranda IM project, Ricardo Pescuma Domenecci",
	"http://pescuma.org/miranda/bclist",
	UNICODE_AWARE,
	DEFMOD_CLISTALL,
	#if defined( _UNICODE )
	{ 0x53e095a3, 0x2695, 0x490a, { 0x9d, 0xad, 0xd2, 0x4, 0x79, 0x9, 0x38, 0x31 } } // {53E095A3-2695-490a-9DAD-D20479093831}
	#else
	{ 0x924dfbcc, 0x71df, 0x4f46, { 0x81, 0x6, 0x5a, 0xc4, 0x3, 0xca, 0xb2, 0x4b } } // {924DFBCC-71DF-4f46-8106-5AC403CAB24B}
	#endif
};

__declspec(dllexport) PLUGININFO *MirandaPluginInfo(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 8, 0, 9))
		return NULL;
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO *) &pluginInfo;
}

__declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 8, 0, 9))
		return NULL;
	pluginInfo.cbSize = sizeof(PLUGININFOEX);
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's interfaces information

static const MUUID interfaces[] = {MIID_CLIST, MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called when number of accounts has been changed

static int OnAccountsChanged( WPARAM wParam, LPARAM lParam )
{
	himlCListClc = (HIMAGELIST) CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called when all modules got loaded

static int OnModulesLoaded( WPARAM wParam, LPARAM lParam )
{
	himlCListClc = (HIMAGELIST) CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);

    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://pescuma.org/miranda/bclist_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.org/miranda/?p=bclist#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"BClist ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/bclistW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/bclist.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO *) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// options iniatialization

static int OnOptsInit(WPARAM wParam, LPARAM lParam)
{
	ClcOptInit(wParam, lParam);
	CluiOptInit(wParam, lParam);
	CListOptInit(wParam, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// menu status services

static INT_PTR GetStatusMode(WPARAM wParam, LPARAM lParam)
{
	return pcli->currentDesiredStatusMode;
}

/////////////////////////////////////////////////////////////////////////////////////////
// main clist initialization routine

int __declspec(dllexport) CListInitialise(PLUGINLINK * link)
{
	pluginLink = link;
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	// get the internal malloc/free()
	mir_getMMI( &mmi );

	pcli = ( CLIST_INTERFACE* )CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)g_hInst);
	if ( (INT_PTR)pcli == CALLSERVICE_NOTFOUND ) {
LBL_Error:
		MessageBoxA( NULL, "This version of plugin requires Miranda IM 0.8.0.9 or later", "Fatal error", MB_OK );
		return 1;
	}
	if ( pcli->version < 6 )
		goto LBL_Error;


#define CLIST_SWAP(a)  pfn##a = pcli->pfn##a; pcli->pfn##a = a

	CLIST_SWAP(ContactListWndProc);
	CLIST_SWAP(ContactListControlWndProc);
	CLIST_SWAP(RebuildEntireList);
	CLIST_SWAP(SetGroupExpand);
	CLIST_SWAP(RecalcScrollBar);
	CLIST_SWAP(ScrollTo);
	CLIST_SWAP(LoadClcOptions);
	CLIST_SWAP(GetRowHeight);
	CLIST_SWAP(SortCLC);

	pcli->pfnPaintClc = PaintClc;

	MySetLayeredWindowAttributes = (BOOL(WINAPI *) (HWND, COLORREF, BYTE, DWORD)) GetProcAddress(
		LoadLibraryA("user32.dll"), "SetLayeredWindowAttributes");

	CreateServiceFunction(MS_CLIST_GETSTATUSMODE, GetStatusMode);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccountsChanged);
	HookEvent(ME_OPT_INITIALISE, OnOptsInit);

	InitCustomMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a plugin loader aware of CList exports will never call this.

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a plugin unloader

int __declspec(dllexport) Unload(void)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

TCHAR* MyDBGetContactSettingTString(HANDLE hContact, char* module, char* setting, TCHAR* out, size_t len, TCHAR *def)
{
	DBVARIANT dbv;

	out[0] = _T('\0');

	if (!DBGetContactSettingTString(hContact, module, setting, &dbv))
	{
#ifdef UNICODE
		if (dbv.type == DBVT_ASCIIZ)
		{
			MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, out, len);
		}
		else if (dbv.type == DBVT_UTF8)
		{
			MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, out, len);
		}
		else if (dbv.type == DBVT_WCHAR)
		{
			lstrcpyn(out, dbv.pwszVal, len);
		}
#else
		if (dbv.type == DBVT_ASCIIZ)
		{
			lstrcpyn(out, dbv.pszVal, len);
		}
#endif
		else
		{
			if (def != NULL)
				lstrcpyn(out, def, len);
		}
		
		DBFreeVariant(&dbv);
	}
	else
	{
		if (def != NULL)
			lstrcpyn(out, def, len);
	}

	return out;
}

#define DATA_BLOCK 128

typedef struct
{
	TCHAR *text;
	size_t allocated;
	size_t used;

} StringHelper;

int CopyData(StringHelper *str, const TCHAR *text, size_t len)
{
	size_t totalSize;

	if (len == 0)
		return 0;

	if (text == NULL)
		return 0;

	totalSize = str->used + len + 1;

	if (totalSize > str->allocated)
	{
		totalSize += DATA_BLOCK - (totalSize % DATA_BLOCK);

		if (str->text != NULL)
		{
			TCHAR *tmp = (TCHAR *) mir_realloc(str->text, sizeof(TCHAR) * totalSize);

			if (tmp == NULL)
			{
				mir_free(str->text);
				return -1;
			}

			str->text = tmp;
		}
		else
		{
			str->text = (TCHAR *) mir_alloc(sizeof(TCHAR) * totalSize);

			if (str->text == NULL)
			{
				return -2;
			}
		}

		str->allocated = totalSize;
	}

	memmove(&str->text[str->used], text, sizeof(TCHAR) * len);
	str->used += len;
	str->text[str->used] = '\0';
	
	return 0;
}


TCHAR * ParseText(const TCHAR *text, 
				   const TCHAR **variables, size_t variablesSize, 
				   const TCHAR **data, size_t dataSize) 
{
	size_t length = lstrlen(text);
	size_t nextPos = 0;
	StringHelper ret = {0};
	size_t i;

	// length - 1 because a % in last char will be a % and point
	for (i = 0 ; i < length - 1 ; i++)
	{
		if (text[i] == _T('%'))
		{
			BOOL found = FALSE;

			if (CopyData(&ret, &text[nextPos], i - nextPos))
				return NULL;

			if (text[i + 1] == _T('%'))
			{
				if (CopyData(&ret, _T("%"), 1))
					return NULL;

				i++;

				found = TRUE;
			}
			else
			{
				size_t size = min(variablesSize, dataSize);
				size_t j;

				// See if can find it
				for(j = 0 ; j < size ; j++)
				{
					size_t vlen = lstrlen(variables[j]);

					if (_tcsnicmp(&text[i], variables[j], vlen) == 0)
					{
						if (CopyData(&ret, data[j], lstrlen(data[j])))
							return NULL;

						i += vlen - 1;

						found = TRUE;

						break;
					}
				}
			}

			if (found)
				nextPos = i + 1;
			else
				nextPos = i;
		}
	}

	if (nextPos < length)
		if (CopyData(&ret, &text[nextPos], length - nextPos))
			return NULL;

	return ret.text;
}

LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_NCCREATE:
		{
			break;
		}
		case WM_CREATE:
		{
			break;
		}
	}
	return pfnContactListWndProc(hwnd, msg, wParam, lParam);
}

struct MyClcData
{
	union {
		struct ClcData;
	};
	HWND hwnd_list;
	BOOL need_rebuild;
};

LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct MyClcData *dat = (struct MyClcData *) GetWindowLong(hwnd, 0);

	switch (msg) {
		case WM_CREATE:
		{
			RECT r;
			LRESULT ret = pfnContactListControlWndProc(hwnd, msg, wParam, lParam);

			// Fix dat
			struct ClcData *tmp = (struct ClcData *) GetWindowLong(hwnd, 0);
			dat = (struct MyClcData *) mir_alloc(sizeof(struct MyClcData));
			memmove(dat, tmp, sizeof(struct ClcData));
			mir_free(tmp);
			SetWindowLong(hwnd, 0, (LONG) dat);

			dat->hwnd_list = CreateWindow(_T("LISTBOX"), _T(""),
					(WS_VISIBLE | WS_CHILD | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT | WS_VSCROLL), 
					0, 0, 0, 0, hwnd, NULL, g_hInst,0);
			dat->need_rebuild = FALSE;

			GetClientRect(hwnd, &r);
			SetWindowPos(dat->hwnd_list, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);

			return ret;
		}

		case WM_SIZE:
		{
			RECT r;
			GetClientRect(hwnd, &r);
			SetWindowPos(dat->hwnd_list, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
			break;
		}

		case WM_PRINTCLIENT:
		case WM_PAINT:
			if (dat->need_rebuild)
				RebuildEntireListInternal(hwnd, (struct ClcData *) dat, FALSE);
			// no break
		case WM_VSCROLL:
		case WM_MOUSEWHEEL:
		case WM_KEYDOWN:
			return DefWindowProc(hwnd, msg, wParam, lParam);

		case INTM_SCROLLBARCHANGED:
			return TRUE;

		case WM_VKEYTOITEM:
		{
			int key = LOWORD(wParam);
			if (key == VK_LEFT || key == VK_RIGHT || key == VK_RETURN || key == VK_DELETE || key == VK_F2) 
			{
				pfnContactListControlWndProc(hwnd, WM_KEYDOWN, key, 0);
				return dat->selection;
			}
			else 
			{
				NMKEY nmkey;
				nmkey.hdr.hwndFrom = hwnd;
				nmkey.hdr.idFrom = GetDlgCtrlID(hwnd);
				nmkey.hdr.code = NM_KEYDOWN;
				nmkey.nVKey = key;
				nmkey.uFlags = 0;
				if (SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nmkey))
					return -2;
			}
			return -1;
		}

		case WM_COMMAND:
		{
			if ((HANDLE) lParam != dat->hwnd_list || HIWORD(wParam) != LBN_SELCHANGE)
				break;

			dat->selection = SendMessage(dat->hwnd_list, LB_GETCURSEL, 0, 0);

			KillTimer(hwnd, TIMERID_INFOTIP);
			KillTimer(hwnd, TIMERID_RENAME);
			dat->szQuickSearch[0] = 0;
			pcli->pfnInvalidateRect(hwnd, NULL, FALSE);
			pcli->pfnEnsureVisible(hwnd, (struct ClcData *) dat, dat->selection, 0);
			UpdateWindow(hwnd);
			break;
		}

		case WM_SETFOCUS:
		case WM_ENABLE:
			SetFocus(dat->hwnd_list);
			break;

	}

	return pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}

static int GetRealStatus(struct ClcContact *contact, int status)
{
	int i;
	char *szProto = contact->proto;
	if (!szProto)
		return status;
	for (i = 0; i < pcli->hClcProtoCount; i++) {
		if (!lstrcmpA(pcli->clcProto[i].szProto, szProto)) {
			return pcli->clcProto[i].dwStatus;
		}
	}
	return status;
}

TCHAR status_name[128];
TCHAR *GetStatusName(struct ClcContact *item) 
{
	int status;

    status_name[0] = _T('\0');
	if (item->hContact == NULL || item->proto == NULL)
		return status_name;

    // Get XStatusName
	MyDBGetContactSettingTString(item->hContact, item->proto, "XStatusName", status_name, MAX_REGS(status_name), NULL);
    if (status_name[0] != _T('\0'))
        return status_name;

    // Get status name
	status = DBGetContactSettingWord(item->hContact, item->proto, "Status", ID_STATUS_OFFLINE);
	lstrcpyn(status_name, pcli->pfnGetStatusModeDescription(status, GCMDF_TCHAR), MAX_REGS(status_name));

	return status_name;
}


TCHAR status_message[256];
TCHAR *GetStatusMessage(struct ClcContact *item) 
{
    status_message[0] = _T('\0');
	if (item->hContact == NULL || item->proto == NULL)
		return status_message;

    // Get XStatusMsg
	MyDBGetContactSettingTString(item->hContact, item->proto, "XStatusMsg", status_message, MAX_REGS(status_message), NULL);
    if (status_message[0] != _T('\0'))
        return status_message;

    // Get status message
	MyDBGetContactSettingTString(item->hContact, "CList", "StatusMsg", status_message, MAX_REGS(status_message), NULL);

	return status_message;
}


TCHAR proto_name[128];
TCHAR *GetProtoName(struct ClcContact *item) 
{
	PROTOACCOUNT *acc;
#ifdef UNICODE
	char description[128];
#endif

    proto_name[0] = '\0';
	if (item->hContact == NULL || item->proto == NULL)
	{
		lstrcpyn(proto_name, TranslateT("Unknown Protocol"), MAX_REGS(proto_name));
		return proto_name;
	}

	acc = ProtoGetAccount(item->proto);

	if (acc == NULL)
	{
#ifdef UNICODE
		CallProtoService(item->proto, PS_GETNAME, sizeof(description),(LPARAM) description);
		mir_sntprintf(proto_name, MAX_REGS(proto_name), L"%S", description);
#else
		CallProtoService(item->proto, PS_GETNAME, sizeof(proto_name),(LPARAM) proto_name);
#endif
		return proto_name;
	}

	lstrcpyn(proto_name, acc->tszAccountName, MAX_REGS(proto_name));

	return proto_name;
}

void RebuildEntireListInternal(HWND hwnd, struct ClcData *tmp_dat, BOOL call_orig)
{
	struct MyClcData *dat = (struct MyClcData *) tmp_dat;
	struct ClcGroup *group;
	struct ClcContact *item;
	TCHAR tmp[1024];
	TCHAR count[128];
	TCHAR template_contact[1024];
	TCHAR template_group[1024];
	TCHAR template_divider[1024];
	TCHAR template_info[1024];
	TCHAR *text;
	size_t size;
	int selection = dat->selection;
	BOOL has_focus = (GetFocus() == dat->hwnd_list || GetFocus() == hwnd);

	if (call_orig)
		pfnRebuildEntireList(hwnd, (struct ClcData *) dat);

	MyDBGetContactSettingTString(NULL, "CLC", "TemplateContact", template_contact, 1024, TranslateT("%name% [%status% %protocol%] %status_message%"));
	MyDBGetContactSettingTString(NULL, "CLC", "TemplateGroup", template_group, 1024, TranslateT("Group: %name% %count% [%mode%]"));
	MyDBGetContactSettingTString(NULL, "CLC", "TemplateDivider", template_divider, 1024, TranslateT("Divider: %s"));
	MyDBGetContactSettingTString(NULL, "CLC", "TemplateInfo", template_info, 1024, TranslateT("Info: %s"));

	SendMessage(dat->hwnd_list, WM_SETREDRAW, FALSE, 0);

	// Reset content
	SendMessage(dat->hwnd_list, LB_RESETCONTENT, 0, 0);

	// Set font
	SendMessage(dat->hwnd_list, WM_SETFONT, (WPARAM) dat->fontInfo[FONTID_CONTACTS].hFont, 0);

	// Add all items to the list
	group = &dat->list;
	group->scanIndex = 0;
	text = tmp;
	size = MAX_REGS(tmp);
	while(1)
	{
		if (group->scanIndex == group->cl.count) 
		{
			group = group->parent;
			if (group == NULL)
				break;
			text -= 2;
			size += 2;
			group->scanIndex++;
			continue;
		}

		item = group->cl.items[group->scanIndex];
		text[0] = _T('\0');
		switch(item->type)
		{
			case CLCIT_GROUP:
			{
				char *szCounts = pcli->pfnGetGroupCountsText((struct ClcData *) dat, item);
				TCHAR *t[] = {
					_T("%name%"),
					_T("%count%"),
					_T("%mode%")
				};
				TCHAR *v[] = {
					item->szText,
					count,
					item->group->expanded ? TranslateT("Expanded") : TranslateT("Colapsed")
				};
				TCHAR *txt;

				if (szCounts[0] != '\0') 
				{
#ifdef UNICODE
					mir_sntprintf(count, MAX_REGS(count), L"%S ", szCounts);
#else
					mir_sntprintf(count, MAX_REGS(count), "%s ", szCounts);
#endif
				}
				else
				{
					count[0] = _T('\0');
				}

				txt = ParseText(template_group, t, MAX_REGS(t), v, MAX_REGS(v));
				if (txt != NULL)
					lstrcpyn(text, txt, size);
				mir_free(txt);
				break;
			}
			case CLCIT_CONTACT:
			{
				char *szCounts = pcli->pfnGetGroupCountsText((struct ClcData *) dat, item);
				TCHAR *t[] = {
					_T("%name%"),
					_T("%status%"),
					_T("%protocol%"), 
					_T("%status_message%")
				};
				TCHAR *v[] = {
					item->szText,
					GetStatusName(item),
					GetProtoName(item),
					GetStatusMessage(item)
				};
				TCHAR *txt;

				txt = ParseText(template_contact, t, MAX_REGS(t), v, MAX_REGS(v));
				if (txt != NULL)
					lstrcpyn(text, txt, size);
				mir_free(txt);
				break;
			}
			case CLCIT_DIVIDER:
			{
				mir_sntprintf(text, size, template_divider, item->szText);
				break;
			}
			case CLCIT_INFO:
			{
				mir_sntprintf(text, size, template_info, item->szText);
				break;
			}
		}

		SendMessage(dat->hwnd_list, LB_ADDSTRING, 0, (LPARAM) tmp);

		if (item->type == CLCIT_GROUP && item->group->expanded) 
		{
			group = item->group;
			text[0] = _T(' ');
			text[1] = _T(' ');
			text += 2;
			size -= 2;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}

	SendMessage(dat->hwnd_list, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(dat->hwnd_list, NULL, TRUE);

	dat->selection = selection;
	SendMessage(dat->hwnd_list, LB_SETCURSEL, dat->selection, 0);
	if (has_focus)
		SetFocus(dat->hwnd_list);

	dat->need_rebuild = FALSE;
}

void RebuildEntireList(HWND hwnd, struct ClcData *dat)
{
	RebuildEntireListInternal(hwnd, dat, TRUE);
}

void SetGroupExpand(HWND hwnd, struct ClcData *tmp_dat, struct ClcGroup *group, int newState)
{
	struct MyClcData *dat = (struct MyClcData *) tmp_dat;

	pfnSetGroupExpand(hwnd, tmp_dat, group, newState);
	dat->need_rebuild = TRUE;
}

void ScrollTo( HWND hwnd, struct ClcData *dat, int desty, int noSmooth )
{
}

void RecalcScrollBar( HWND hwnd, struct ClcData *dat )
{
}

void LoadClcOptions( HWND hwnd, struct ClcData *tmp_dat )
{
	struct MyClcData *dat = (struct MyClcData *) tmp_dat;

	pfnLoadClcOptions(hwnd, tmp_dat);
	dat->rowHeight = SendMessage(dat->hwnd_list, LB_GETITEMHEIGHT, 0, 0);
}

int GetRowHeight(struct ClcData *tmp_dat, int item)
{
	struct MyClcData *dat = (struct MyClcData *) tmp_dat;

	dat->rowHeight = SendMessage(dat->hwnd_list, LB_GETITEMHEIGHT, 0, 0);
	return dat->rowHeight;
}

void SortCLC(HWND hwnd, struct ClcData *tmp_dat, int useInsertionSort)
{
	if ( tmp_dat->needsResort ) 
	{
		struct MyClcData *dat = (struct MyClcData *) tmp_dat;

		pfnSortCLC(hwnd, tmp_dat, useInsertionSort);
		dat->need_rebuild = TRUE;
	}
}