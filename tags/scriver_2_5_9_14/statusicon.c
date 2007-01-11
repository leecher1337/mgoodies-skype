#include "commonheaders.h"
#include "statusicon.h"

struct StatusIconListNode {
	StatusIconData sid;
	struct StatusIconListNode *next;
};

static HANDLE hHookIconPressedEvt;
static struct StatusIconListNode *status_icon_list = 0;
static int status_icon_list_size = 0;

static int AddStatusIcon(WPARAM wParam, LPARAM lParam) {
	StatusIconData *sid = (StatusIconData *)lParam;
	struct StatusIconListNode *siln = (struct StatusIconListNode *)mir_alloc(sizeof(struct StatusIconListNode));

	siln->sid.cbSize = sid->cbSize;
	siln->sid.szModule = mir_strdup(sid->szModule);
	siln->sid.dwId = sid->dwId;
	siln->sid.hIcon = DuplicateIcon(NULL, sid->hIcon);
	siln->sid.hIconDisabled = DuplicateIcon(NULL, sid->hIconDisabled);
	siln->sid.flags = sid->flags;
	if(sid->szTooltip) siln->sid.szTooltip = mir_strdup(sid->szTooltip);
	else siln->sid.szTooltip = 0;

	siln->next = status_icon_list;
	status_icon_list = siln;
	status_icon_list_size++;

	WindowList_Broadcast(g_dat->hParentWindowList, DM_STATUSICONCHANGE, 0, 0);
	return 0;
}

int AddStickyStatusIcon(WPARAM wParam, LPARAM lParam) {
	StatusIconData *sid = (StatusIconData *)lParam;
	struct StatusIconListNode *siln = (struct StatusIconListNode *)mir_alloc(sizeof(struct StatusIconListNode));
	struct StatusIconListNode *siln2 = status_icon_list;

	siln->sid.cbSize = sid->cbSize;
	siln->sid.szModule = mir_strdup(sid->szModule);
	siln->sid.dwId = sid->dwId;
	siln->sid.hIcon = DuplicateIcon(NULL, sid->hIcon);
	siln->sid.hIconDisabled = DuplicateIcon(NULL, sid->hIconDisabled);
	siln->sid.flags = sid->flags;
	if(sid->szTooltip) siln->sid.szTooltip = mir_strdup(sid->szTooltip);
	else siln->sid.szTooltip = 0;
	siln->next = NULL;

	while(siln2 && siln2->next) {
		siln2 = siln2->next;
	}
	if (siln2) {
		siln2->next = siln;
	} else {
		status_icon_list = siln;
	}

	status_icon_list_size++;

	WindowList_Broadcast(g_dat->hParentWindowList, DM_STATUSICONCHANGE, 0, 0);
	return 0;
}

static int RemoveStatusIcon(WPARAM wParam, LPARAM lParam) {
	StatusIconData *sid = (StatusIconData *)lParam;
	struct StatusIconListNode *current = status_icon_list, *prev = 0;

	while(current) {
		if(strcmp(current->sid.szModule, sid->szModule) == 0 && current->sid.dwId == sid->dwId) {
			if(prev) prev->next = current->next;
			else status_icon_list = current->next;

			status_icon_list_size--;

			mir_free(current->sid.szModule);
			DestroyIcon(current->sid.hIcon);
			if(current->sid.hIconDisabled) DestroyIcon(current->sid.hIconDisabled);
			if(current->sid.szTooltip) mir_free(current->sid.szTooltip);
			mir_free(current);
			WindowList_Broadcast(g_dat->hParentWindowList, DM_STATUSICONCHANGE, 0, 0);
			return 0;
		}

		prev = current;
		current = current->next;
	}

	return 1;
}

static void RemoveAllStatusIcons(void) {
	struct StatusIconListNode *current;

	while(status_icon_list) {
		current = status_icon_list;
		status_icon_list = status_icon_list->next;
		status_icon_list_size--;

		mir_free(current->sid.szModule);
		DestroyIcon(current->sid.hIcon);
		if(current->sid.hIconDisabled) DestroyIcon(current->sid.hIconDisabled);
		if(current->sid.szTooltip) mir_free(current->sid.szTooltip);
		mir_free(current);
	}

	WindowList_Broadcast(g_dat->hParentWindowList, DM_STATUSICONCHANGE, 0, 0);
}

static int ModifyStatusIcon(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;

	StatusIconData *sid = (StatusIconData *)lParam;
	struct StatusIconListNode *current = status_icon_list;

	while(current) {
		if(strcmp(current->sid.szModule, sid->szModule) == 0 && current->sid.dwId == sid->dwId) {
			if(!hContact) {
				current->sid.flags = sid->flags;
				if(sid->hIcon) {
					DestroyIcon(current->sid.hIcon);
					current->sid.hIcon = DuplicateIcon(NULL, sid->hIcon);
				}
				if(sid->hIconDisabled) {
					DestroyIcon(current->sid.hIconDisabled);
					current->sid.hIconDisabled = DuplicateIcon(NULL, sid->hIconDisabled);
				}
				if(sid->szTooltip) {
					if(current->sid.szTooltip) mir_free(current->sid.szTooltip);
					current->sid.szTooltip = mir_strdup(sid->szTooltip);
				}

				WindowList_Broadcast(g_dat->hParentWindowList, DM_STATUSICONCHANGE, 0, 0);
			} else {
				char buff[256];
				HWND hwnd;
				sprintf(buff, "SRMMStatusIconFlags%d", (int)sid->dwId);
				DBWriteContactSettingByte(hContact, sid->szModule, buff, (BYTE)sid->flags);
				hwnd = WindowList_Find(g_dat->hMessageWindowList, hContact);
				if (hwnd == NULL) {
					hwnd = SM_FindWindowByContact(hContact);

				}
				if (hwnd != NULL) {
					PostMessage(GetParent(hwnd), DM_STATUSICONCHANGE, 0, 0);
				}
			}
			return 0;
		}

		current = current->next;
	}

	return 1;
}

void DrawStatusIcons(HANDLE hContact, HDC hDC, RECT r, int gap) {
	struct StatusIconListNode *current = status_icon_list;
	HICON hIcon;
	char buff[256];
	int flags;
	int x = r.left;
	while(current) {
		sprintf(buff, "SRMMStatusIconFlags%d", (int)current->sid.dwId);
		flags = DBGetContactSettingByte(hContact, current->sid.szModule, buff, current->sid.flags);
		if(!(flags & MBF_HIDDEN)) {
			if((flags & MBF_DISABLED) && current->sid.hIconDisabled) hIcon = current->sid.hIconDisabled;
			else hIcon = current->sid.hIcon;

			SetBkMode(hDC, TRANSPARENT);
			DrawIconEx(hDC, x, (r.top + r.bottom - GetSystemMetrics(SM_CYSMICON)) >> 1, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);

			x += GetSystemMetrics(SM_CYSMICON) + gap;
		}
		current = current->next;
	}
}

void CheckStatusIconClick(HANDLE hContact, HWND hwndFrom, POINT pt, RECT r, int gap, int click_flags) {
	StatusIconClickData sicd;
	struct StatusIconListNode *current = status_icon_list;
	unsigned int iconNum = (pt.x - r.left) / (GetSystemMetrics(SM_CXSMICON) + gap);
	int flags;
	char buff[256];

	while(current && iconNum > 0) {
		sprintf(buff, "SRMMStatusIconFlags%d", (int)current->sid.dwId);
		flags = DBGetContactSettingByte(hContact, current->sid.szModule, buff, current->sid.flags);
		if(!(flags & MBF_HIDDEN)) iconNum--;
		current = current->next;
	}

	if(current) {
		ClientToScreen(hwndFrom, &pt);
		sicd.cbSize = sizeof(StatusIconClickData);
		sicd.clickLocation = pt;
		sicd.dwId = current->sid.dwId;
		sicd.szModule = current->sid.szModule;
		sicd.flags = click_flags;

		NotifyEventHooks(hHookIconPressedEvt, (WPARAM)hContact, (LPARAM)&sicd);
	}
}


HANDLE hServiceIcon[3];
int InitStatusIcons() {
	hServiceIcon[0] = CreateServiceFunction(MS_MSG_ADDICON, AddStatusIcon);
	hServiceIcon[1] = CreateServiceFunction(MS_MSG_ADDICON, RemoveStatusIcon);
	hServiceIcon[2] = CreateServiceFunction(MS_MSG_MODIFYICON, ModifyStatusIcon);
	hHookIconPressedEvt = CreateHookableEvent(ME_MSG_ICONPRESSED);

	return 0;
}

int DeinitStatusIcons() {
	int i;
	DestroyHookableEvent(hHookIconPressedEvt);
	for(i = 0; i < 3; i++) DestroyServiceFunction(hServiceIcon[i]);
	RemoveAllStatusIcons();
	return 0;
}

int GetStatusIconsCount(HANDLE hContact) {
	char buff[256];
	int count = 0;
	int flags;
	struct StatusIconListNode *current = status_icon_list;
	while(current) {
		sprintf(buff, "SRMMStatusIconFlags%d", (int)current->sid.dwId);
		flags = DBGetContactSettingByte(hContact, current->sid.szModule, buff, current->sid.flags);
		if(!(flags & MBF_HIDDEN)) {
			count ++;
		}
		current = current->next;
	}
	return count;
//	return status_icon_list_size;
}
