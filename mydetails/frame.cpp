/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#include "commons.h"
#include "frame.h"
#include "wingdi.h"
#include "winuser.h"


// Prototypes /////////////////////////////////////////////////////////////////////////////////////


#define WINDOW_NAME_PREFIX "mydetails_window"
#define WINDOW_CLASS_NAME "MyDetailsFrame"
#define CONTAINER_CLASS_NAME "MyDetailsFrameContainer"

#define ID_FRAME_TIMER			1011
#define ID_RECALC_TIMER			1012
#define ID_STATUSMESSAGE_TIMER	1013

#define RECALC_TIME				1000

#define IDC_HAND				MAKEINTRESOURCE(32649)


// Messages
#define MWM_REFRESH				(WM_USER+10)
#define MWM_NICK_CHANGED		(WM_USER+11)
#define MWM_STATUS_CHANGED		(WM_USER+12)
#define MWM_STATUS_MSG_CHANGED	(WM_USER+13)
#define MWM_AVATAR_CHANGED		(WM_USER+14)
#define MWM_LISTENINGTO_CHANGED	(WM_USER+15)
#define MWM_LOCK_CHANGED		(WM_USER+16)
#define MWM_EMAIL_COUNT_CHANGED (WM_USER+17)


HWND hwnd_frame = NULL;
HWND hwnd_container = NULL;

int frame_id = -1;

HANDLE hMenuShowHideFrame = 0;

int CreateFrame();
void FixMainMenu();
void RefreshFrame();
void RedrawFrame();


// used when no multiwindow functionality available
BOOL MyDetailsFrameVisible();
void SetMyDetailsFrameVisible(BOOL visible);
int ShowHideMenuFunc(WPARAM wParam, LPARAM lParam);
int ShowFrameFunc(WPARAM wParam, LPARAM lParam);
int HideFrameFunc(WPARAM wParam, LPARAM lParam);
int ShowHideFrameFunc(WPARAM wParam, LPARAM lParam);



LRESULT CALLBACK FrameContainerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void SetCycleTime();
void SetCycleTime(HWND hwnd);
void SetStatusMessageRefreshTime();
void SetStatusMessageRefreshTime(HWND hwnd);
int SettingsChangedHook(WPARAM wParam, LPARAM lParam);
int AvatarChangedHook(WPARAM wParam, LPARAM lParam);
int ProtoAckHook(WPARAM wParam, LPARAM lParam);
int SmileyAddOptionsChangedHook(WPARAM wParam,LPARAM lParam);
int ListeningtoEnableStateChangedHook(WPARAM wParam,LPARAM lParam);


void ExternalRect(RECT &ret, const RECT r1, const RECT r2);
BOOL InsideRect(const POINT &p, const RECT &r);


int operator==(const RECT& left, const RECT& right)
{
	return left.left == right.left && left.right == right.right
			&& left.top == right.top && left.bottom == right.bottom;
}

class ToolTipArea
{
public:
	ToolTipArea() : hwndTT(0), hwndParent(0) { memset(&rc, 0, sizeof(rc)); }
	~ToolTipArea() { removeTooltip(); }

	void createTooltip(HWND hwnd, const RECT &rc, const TCHAR *text)
	{
		if (text == NULL || text[0] == 0)
		{
			removeTooltip();
			return;
		}

		this->text = text;

		if (this->rc == rc && hwndParent == hwnd && hwndTT != NULL)
			return;

		removeTooltip();

		this->rc = rc;
		this->hwndParent = hwnd;
		this->hwndTT = CreateTooltip(this->hwndParent, this->rc);
	}

	void removeTooltip()
	{
		if (hwndTT == NULL)
			return;

		DestroyWindow(hwndTT);
		hwndTT = NULL;
		hwndParent = NULL;
	}

	const TCHAR * getTextFor(HWND hwndFrom)
	{
		if (hwndTT == NULL || hwndTT != hwndFrom)
			return NULL;
		return text.c_str();
	}


private:
	
	HWND hwndTT;
	RECT rc;
	HWND hwndParent;
	std::tstring text;

	HWND CreateTooltip(HWND hwnd, RECT &rect)
	{
			  // struct specifying control classes to register
		INITCOMMONCONTROLSEX iccex; 
		HWND hwndTT;                 // handle to the ToolTip control
			  // struct specifying info about tool in ToolTip control
		TOOLINFO ti;
		unsigned int uid = 0;       // for ti initialization

		// Load the ToolTip class from the DLL.
		iccex.dwSize = sizeof(iccex);
		iccex.dwICC  = ICC_BAR_CLASSES;

		if(!InitCommonControlsEx(&iccex))
		   return NULL;

		/* CREATE A TOOLTIP WINDOW */
		hwndTT = CreateWindowEx(WS_EX_TOPMOST,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hwnd,
			NULL,
			hInst,
			NULL
			);

		/* Gives problem with mToolTip
		SetWindowPos(hwndTT,
			HWND_TOPMOST,
			0,
			0,
			0,
			0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		*/

		/* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hwnd;
		ti.hinst = hInst;
		ti.uId = uid;
		ti.lpszText = LPSTR_TEXTCALLBACK;
			// ToolTip control will cover the whole window
		ti.rect.left = rect.left;    
		ti.rect.top = rect.top;
		ti.rect.right = rect.right;
		ti.rect.bottom = rect.bottom;

		/* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
		SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
		SendMessage(hwndTT, TTM_SETDELAYTIME, (WPARAM) (DWORD) TTDT_AUTOPOP, (LPARAM) MAKELONG(24 * 60 * 60 * 1000, 0));	

		return hwndTT;
	} 
};


struct SimpleItem
{
	RECT rc;
	BOOL draw;
	BOOL mouseOver;
	ToolTipArea tt;
	BOOL alignRight;

	SimpleItem() : draw(FALSE), mouseOver(FALSE), alignRight(FALSE)
	{ 
		memset(&rc, 0, sizeof(rc)); 
	}
	virtual ~SimpleItem() {}

	virtual void hide()
	{
		draw = FALSE;
		mouseOver = FALSE;
		tt.removeTooltip();
	}

	virtual void update(HWND hwnd, SkinFieldState *item)
	{
		draw = item->isVisible();
		alignRight = ( item->getHorizontalAlign() == SKN_HALIGN_RIGHT );

		if (draw)
		{
			rc = item->getRect();
			tt.createTooltip(hwnd, rc, item->getToolTip());
		}
		else
		{
			tt.removeTooltip();
		}
	}

	virtual BOOL hitTest(const POINT &p)
	{
		return draw && InsideRect(p, rc);
	}

	virtual const TCHAR * getToolTipFor(HWND hwndFrom)
	{
		return tt.getTextFor(hwndFrom);
	}
};

struct IconAndItem : public SimpleItem
{
	RECT rcIcon;
	RECT rcItem;
	BOOL drawIcon;
	BOOL drawItem;
	ToolTipArea ttIcon;

	IconAndItem() : drawIcon(FALSE), drawItem(FALSE) 
	{ 
		memset(&rcIcon, 0, sizeof(rcIcon)); 
		memset(&rcItem, 0, sizeof(rcItem)); 
	}
	virtual ~IconAndItem() {}
	
	virtual void hide()
	{
		SimpleItem::hide();
		drawIcon = FALSE;
		drawItem = FALSE;
	}

	virtual void update(HWND hwnd, SkinIconFieldState *icon, SkinTextFieldState *item)
	{
		drawIcon = icon->isVisible();
		drawItem = icon->isVisible();
		alignRight = ( item->getHorizontalAlign() == SKN_HALIGN_RIGHT );

		draw = drawIcon || drawItem;
		if (draw)
		{
			if (drawIcon)
				rcIcon = icon->getRect();
			if (drawItem)
				rcItem = item->getRect();
			
			if (drawIcon && drawItem)
				ExternalRect(rc, rcIcon, rcItem);
			else if (drawIcon)
				rc = rcIcon;
			else // if (drawItem)
				rc = rcItem;
		}

		if (drawItem)
			tt.createTooltip(hwnd, rcItem, item->getToolTip());
		else
			tt.removeTooltip();

		if (drawIcon)
			ttIcon.createTooltip(hwnd, rcIcon, icon->getToolTip());
		else
			ttIcon.removeTooltip();
	}

	virtual const TCHAR * getToolTipFor(HWND hwndFrom)
	{
		const TCHAR * ret = tt.getTextFor(hwndFrom);

		if (ret == NULL)
			ret = ttIcon.getTextFor(hwndFrom);

		return ret;
	}
};


struct MyDetailsFrameData
{
	std::vector<SimpleItem*> items;
	SimpleItem proto;
	SimpleItem proto_cycle_next;
	SimpleItem proto_cycle_prev;
	SimpleItem avatar;
	SimpleItem nick;
	IconAndItem status;
	SimpleItem away_msg;
	IconAndItem listening_to;
	IconAndItem email;

	int protocol_number;

	BOOL showing_menu;

	BOOL get_status_messages;

	MyDetailsFrameData() : protocol_number(0), showing_menu(FALSE), get_status_messages(FALSE) 
	{
		items.push_back(&proto);
		items.push_back(&proto_cycle_next);
		items.push_back(&proto_cycle_prev);
		items.push_back(&avatar);
		items.push_back(&nick);
		items.push_back(&status);
		items.push_back(&away_msg);
		items.push_back(&listening_to);
		items.push_back(&email);
	}
};



// Functions //////////////////////////////////////////////////////////////////////////////////////

void InitFrames()
{
	InitContactListSmileys();

	CreateFrame();

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingsChangedHook);
	HookEvent(ME_AV_MYAVATARCHANGED, AvatarChangedHook);
	HookEvent(ME_PROTO_ACK, ProtoAckHook);
	HookEvent(ME_SMILEYADD_OPTIONSCHANGED,SmileyAddOptionsChangedHook);
	HookEvent(ME_LISTENINGTO_ENABLE_STATE_CHANGED,ListeningtoEnableStateChangedHook);
}


void DeInitFrames()
{
	if(ServiceExists(MS_CLIST_FRAMES_REMOVEFRAME) && frame_id != -1) 
	{
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)frame_id, 0);
	}

	if (hwnd_frame != NULL) DestroyWindow(hwnd_frame);
	if (hwnd_container != NULL) DestroyWindow(hwnd_container);
}

int SmileyAddOptionsChangedHook(WPARAM wParam,LPARAM lParam)
{
	RefreshFrame();
	return 0;
}

int CreateFrame() 
{
	WNDCLASS wndclass;
	wndclass.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; //CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = FrameWindowProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInst;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = WINDOW_CLASS_NAME;
	RegisterClass(&wndclass);

	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
	{
		hwnd_frame = CreateWindow(WINDOW_CLASS_NAME, Translate("My Details"), 
				WS_CHILD | WS_VISIBLE, 
				0,0,10,10, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL, hInst, NULL);

		CLISTFrame Frame = {0};
		
		Frame.cbSize = sizeof(Frame);
		Frame.name = Translate("My Details");
		Frame.cbSize = sizeof(CLISTFrame);
		Frame.hWnd = hwnd_frame;
		Frame.align = alTop;
		Frame.Flags = F_VISIBLE | F_SHOWTB | F_SHOWTBTIP | F_NOBORDER;
		Frame.height = 100;

		frame_id = CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);

		
		if (DBGetContactSettingByte(NULL, "MyDetails", "ForceHideFrame", 0))
		{
			int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
			if(flags & F_VISIBLE) 
				CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);

			DBDeleteContactSetting(NULL, "MyDetails", "ForceHideFrame");
		}

		if (DBGetContactSettingByte(NULL, "MyDetails", "ForceShowFrame", 0))
		{	
			int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
			if(!(flags & F_VISIBLE)) 
				CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);

			DBDeleteContactSetting(NULL, "MyDetails", "ForceShowFrame");
		}
	}
	else 
	{
		wndclass.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;//CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc   = FrameContainerWindowProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = hInst;
		wndclass.hIcon         = NULL;
		wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
		wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = CONTAINER_CLASS_NAME;
		RegisterClass(&wndclass);

		hwnd_container = CreateWindowEx(WS_EX_TOOLWINDOW, CONTAINER_CLASS_NAME, Translate("My Details"), 
			(WS_THICKFRAME | WS_CAPTION | WS_SYSMENU) & ~WS_VISIBLE,
			0,0,200,130, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL, hInst, NULL);
	
		hwnd_frame = CreateWindow(WINDOW_CLASS_NAME, Translate("My Details"), 
			WS_CHILD | WS_VISIBLE,
			0,0,10,10, hwnd_container, NULL, hInst, NULL);

		SetWindowLong(hwnd_container, GWL_USERDATA, (LONG)hwnd_frame);
		SendMessage(hwnd_container, WM_SIZE, 0, 0);

		// Create menu item

		CLISTMENUITEM menu = {0};

		menu.cbSize=sizeof(menu);
		menu.flags = CMIM_ALL;
		menu.popupPosition = -0x7FFFFFFF;
		menu.pszPopupName = Translate("My Details");
		menu.position = 1; // 500010000
		menu.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		menu.pszName = Translate("Show My Details");
		menu.pszService= MODULE_NAME "/ShowHideMyDetails";
		hMenuShowHideFrame = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menu);

		if(DBGetContactSettingByte(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 1) == 1) 
		{
			ShowWindow(hwnd_container, SW_SHOW);
			FixMainMenu();
		}
	}

	CreateServiceFunction(MS_MYDETAILS_SHOWFRAME, ShowFrameFunc);
	CreateServiceFunction(MS_MYDETAILS_HIDEFRAME, HideFrameFunc);
	CreateServiceFunction(MS_MYDETAILS_SHOWHIDEFRAME, ShowHideFrameFunc);

	return 0;
}


BOOL FrameIsFloating() 
{
	if (frame_id == -1) 
	{
		return true; // no frames, always floating
	}
	
	return (CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, frame_id), 0) != 0);
}


LRESULT CALLBACK FrameContainerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) 
	{
		case WM_SHOWWINDOW:
		{
			if ((BOOL)wParam)
				Utils_RestoreWindowPosition(hwnd, 0, MODULE_NAME, WINDOW_NAME_PREFIX);
			else
				Utils_SaveWindowPosition(hwnd, 0, MODULE_NAME, WINDOW_NAME_PREFIX);
			break;
		}

		case WM_ERASEBKGND:
		{
			HWND child = (HWND)GetWindowLong(hwnd, GWL_USERDATA);

			SendMessage(child, WM_ERASEBKGND, wParam, lParam);
			break;
		}

		case WM_SIZE:
		{
			HWND child = (HWND)GetWindowLong(hwnd, GWL_USERDATA);
			RECT r;
			GetClientRect(hwnd, &r);

			SetWindowPos(child, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
			InvalidateRect(child, NULL, TRUE);

			return TRUE;
		}

		case WM_CLOSE:
		{
			DBWriteContactSettingByte(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 0);
			ShowWindow(hwnd, SW_HIDE);
			FixMainMenu();
			return TRUE;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}



BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	BOOL ret;

	POINT pt;

	pt.x = lpRect->left;
	pt.y = lpRect->top;

	ret = ScreenToClient(hWnd, &pt);

	if (!ret) return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;


	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}


BOOL MoveWindow(HWND hWnd, const RECT &rect, BOOL bRepaint)
{
	return MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, bRepaint);
}


RECT GetInnerRect(const RECT &rc, const RECT &clipping)
{
	RECT rc_ret = rc;

	rc_ret.left = max(rc.left, clipping.left);
	rc_ret.top = max(rc.top, clipping.top);
	rc_ret.right = min(rc.right, clipping.right);
	rc_ret.bottom = min(rc.bottom, clipping.bottom);

	return rc_ret;
}





void ExternalRect(RECT &ret, const RECT r1, const RECT r2)
{
	ret.left = min(r1.left, r2.left);
	ret.right = max(r1.right, r2.right);
	ret.top = min(r1.top, r2.top);
	ret.bottom = max(r1.bottom, r2.bottom);
}


HBITMAP CreateBitmap32(int cx, int cy)
{
   BITMAPINFO RGB32BitsBITMAPINFO; 
    UINT * ptPixels;
    HBITMAP DirectBitmap;

    ZeroMemory(&RGB32BitsBITMAPINFO,sizeof(BITMAPINFO));
    RGB32BitsBITMAPINFO.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    RGB32BitsBITMAPINFO.bmiHeader.biWidth=cx;//bm.bmWidth;
    RGB32BitsBITMAPINFO.bmiHeader.biHeight=cy;//bm.bmHeight;
    RGB32BitsBITMAPINFO.bmiHeader.biPlanes=1;
    RGB32BitsBITMAPINFO.bmiHeader.biBitCount=32;

    DirectBitmap = CreateDIBSection(NULL, 
                                       (BITMAPINFO *)&RGB32BitsBITMAPINFO, 
                                       DIB_RGB_COLORS,
                                       (void **)&ptPixels, 
                                       NULL, 0);
    return DirectBitmap;
}

 void EraseBackground(HWND hwnd, HDC hdc)
{
	RECT r;
	GetClientRect(hwnd, &r);

	HBRUSH hB = CreateSolidBrush((COLORREF) DBGetContactSettingDword(NULL,"MyDetails","BackgroundColor",GetSysColor(COLOR_BTNFACE)));
	FillRect(hdc, &r, hB);
	DeleteObject(hB);
}

void DrawTextWithRect(HDC hdc, const char *text, RECT rc, RECT rc_internal, UINT uFormat, 
					  BOOL mouse_over, Protocol *proto, BOOL replace_smileys = true)
{
	const char *tmp = text;

	// Only first line
	char *tmp2 = strdup(tmp);
	char *pos = strchr(tmp2, '\r');
	if (pos != NULL) pos[0] = '\0';
	pos = strchr(tmp2, '\n');
	if (pos != NULL) pos[0] = '\0';

	HRGN rgn = CreateRectRgnIndirect(&rc_internal);
	SelectClipRgn(hdc, rgn);

	if (replace_smileys)
		DRAW_TEXT(hdc, tmp2, strlen(tmp2), &rc_internal, uFormat, proto->name, NULL);
	else
		DrawText(hdc, tmp2, strlen(tmp2), &rc_internal, uFormat);

	SelectClipRgn(hdc, NULL);
	DeleteObject(rgn);

	if (mouse_over)
		FrameRect(hdc, &rc, (HBRUSH) GetStockObject(GRAY_BRUSH));

	free(tmp2);
}

static int Width(const RECT &rc)
{
	return rc.right - rc.left;
}

static int Height(const RECT &rc)
{
	return rc.bottom - rc.top;
}

static HICON CreateOverlayedIcon(HICON icon, HICON overlay)
{
	HIMAGELIST il = ImageList_Create(
				GetSystemMetrics(SM_CXICON),
				GetSystemMetrics(SM_CYICON),
				ILC_COLOR32|ILC_MASK, 2, 2);
	ImageList_AddIcon(il, icon);
	ImageList_AddIcon(il, overlay);
	HIMAGELIST newImage = ImageList_Merge(il,0,il,1,0,0);
	ImageList_Destroy(il);
	HICON hIcon = ImageList_GetIcon(newImage, 0, 0);
	ImageList_Destroy(newImage);
	return hIcon; // the result should be destroyed by DestroyIcon()
}


void Draw(HWND hwnd, HDC hdc_orig)
{
	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
	Protocol *proto = protocols->Get(data->protocol_number);

	if (proto == NULL)
	{
		EraseBackground(hwnd, hdc_orig);
		return;
	}

	proto->data_changed = false;

	if (ServiceExists(MS_CLIST_FRAMES_SETFRAMEOPTIONS) && frame_id != -1)
	{
		int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
		if(flags & F_UNCOLLAPSED) 
		{
			RECT rf;
			GetClientRect(hwnd, &rf);

			if (rf.bottom - rf.top != 0)
			{
				if (FrameIsFloating()) 
				{
					HWND parent = GetParent(hwnd);

					if (parent != NULL)
					{
						RECT rp_client, rp_window, r_window;
						GetClientRect(parent, &rp_client);
						GetWindowRect(parent, &rp_window);
						GetWindowRect(hwnd, &r_window);
						int diff = (rp_window.bottom - rp_window.top) - (rp_client.bottom - rp_client.top);
						if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
							diff += (r_window.top - rp_window.top);

						SetWindowPos(parent, 0, 0, 0, rp_window.right - rp_window.left, diff, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
					}
				}
			}

			for (size_t i = 0; i < data->items.size(); ++i)
				data->items[i]->hide();

			return;
		}
	}


	RECT r_full;
	GetClientRect(hwnd, &r_full);

	HDC hdc = CreateCompatibleDC(hdc_orig);
	HBITMAP hBmp = CreateBitmap32(Width(r_full), Height(r_full));
	SelectObject(hdc, hBmp);

	int old_bk_mode = SetBkMode(hdc, TRANSPARENT);
	HFONT old_font = (HFONT) GetCurrentObject(hdc, OBJ_FONT);
	COLORREF old_color = GetTextColor(hdc);
	SetStretchBltMode(hdc, HALFTONE);


	HICON hStatusIcon;
	bool freeStatusIcon = false;
	if (proto->custom_status != 0 && ProtoServiceExists(proto->name, PS_ICQ_GETCUSTOMSTATUSICON))
		hStatusIcon = (HICON) CallProtoService(proto->name, PS_ICQ_GETCUSTOMSTATUSICON, proto->custom_status, LR_SHARED);
	else
		hStatusIcon = LoadSkinnedProtoIcon(proto->name, proto->status);

	if (proto->locked)
	{
		HICON hLockOverlay = LoadSkinnedIcon(SKINICON_OTHER_STATUS_LOCKED);
		if (hLockOverlay != NULL)
		{
			freeStatusIcon = true;
			hStatusIcon = CreateOverlayedIcon(hStatusIcon, hLockOverlay);
		}
	}


	HICON hListeningIcon = LoadIconEx("LISTENING_TO_ICON");
	HICON hEmailIcon = LoadIconEx("MYDETAILS_EMAIL");
	HICON hNextIcon = LoadIconEx("MYDETAILS_NEXT_PROTOCOL");
	HICON hPrevIcon = LoadIconEx("MYDETAILS_PREV_PROTOCOL");

	{
		dialog->setInfoBool("resize_frame", opts.resize_frame);
		dialog->setInfoBool("protocol.locked", proto->locked);


		if (opts.resize_frame)
			dialog->setSize(Width(r_full), 0x1FFFFFFF);
		else
			dialog->setSize(Width(r_full), Height(r_full));



		SkinImageField avatar = dialog->getImageField("avatar");
		if (proto->CanGetAvatar() && proto->avatar_bmp != NULL)
		{
			avatar.setEnabled(TRUE);
			avatar.setImage(proto->avatar_bmp);
		}
		else
		{
			avatar.setEnabled(FALSE);
			avatar.setImage(NULL);
		}

		SkinTextField nickname = dialog->getTextField("nickname");
		nickname.setText(proto->nickname);

		SkinTextField protocol = dialog->getTextField("protocol");
		protocol.setText(proto->description);

		SkinIconField status_icon = dialog->getIconField("status_icon");
		status_icon.setIcon(hStatusIcon);

		SkinTextField status_name = dialog->getTextField("status_name");
		status_name.setText(proto->status_name);

		SkinTextField status_msg = dialog->getTextField("status_msg");
		if (proto->CanGetStatusMsg()) 
		{
			status_msg.setEnabled(TRUE);
			status_msg.setText(proto->status_message);
		}
		else
		{
			status_msg.setEnabled(FALSE);
			status_msg.setText(_T(""));
		}

		SkinIconField listening_icon = dialog->getIconField("listening_icon");
		SkinTextField listening = dialog->getTextField("listening");
		if (proto->ListeningToEnabled() && proto->GetStatus() > ID_STATUS_OFFLINE && proto->listening_to[0] != 0) 
		{
			listening_icon.setEnabled(TRUE);
			listening.setEnabled(TRUE);
			listening_icon.setIcon(hListeningIcon);
			listening.setText(proto->listening_to);
		}
		else
		{
			listening_icon.setEnabled(FALSE);
			listening.setEnabled(FALSE);
			listening_icon.setIcon(NULL);
			listening.setText(_T(""));
		}

		SkinIconField email_icon = dialog->getIconField("email_icon");
		SkinTextField email = dialog->getTextField("email");
		if (proto->CanGetEmailCount()) 
		{
			email_icon.setEnabled(TRUE);
			email.setEnabled(TRUE);
			email_icon.setIcon(hEmailIcon);

			TCHAR tmp[64];
			_sntprintf(tmp, MAX_REGS(tmp), _T("%d"), proto->emails);
			email.setText(tmp);
		}
		else
		{
			email_icon.setEnabled(FALSE);
			email.setEnabled(FALSE);
			email_icon.setIcon(NULL);
			email.setText(_T(""));
		}

		SkinIconField next_proto = dialog->getIconField("next_proto");
		SkinIconField prev_proto = dialog->getIconField("prev_proto");
		prev_proto.setIcon(hPrevIcon);
		next_proto.setIcon(hNextIcon);
	}

	SkinDialogState state = dialog->run();
	SkinImageFieldState avatar = state.getImageField("avatar");
	SkinTextFieldState nickname = state.getTextField("nickname");
	SkinTextFieldState protocol = state.getTextField("protocol");
	SkinIconFieldState status_icon = state.getIconField("status_icon");
	SkinTextFieldState status_name = state.getTextField("status_name");
	SkinTextFieldState status_msg = state.getTextField("status_msg");
	SkinIconFieldState listening_icon = state.getIconField("listening_icon");
	SkinTextFieldState listening = state.getTextField("listening");
	SkinIconFieldState email_icon = state.getIconField("email_icon");
	SkinTextFieldState email = state.getTextField("email");
	SkinIconFieldState next_proto = state.getIconField("next_proto");
	SkinIconFieldState prev_proto = state.getIconField("prev_proto");
		

	{
		data->proto.update(hwnd, &protocol);
		data->proto_cycle_next.update(hwnd, &next_proto);
		data->proto_cycle_prev.update(hwnd, &prev_proto);
		data->avatar.update(hwnd, &avatar);
		data->nick.update(hwnd, &nickname);
		data->status.update(hwnd, &status_icon, &status_name);
		data->away_msg.update(hwnd, &status_msg);
		data->listening_to.update(hwnd, &listening_icon, &listening);
		data->email.update(hwnd, &email_icon, &email);
	}

	// Erase
	EraseBackground(hwnd, hdc);

	// Draw items

	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS 
					| (opts.draw_text_rtl ? DT_RTLREADING : 0);

#define HALIGN( _F_ )  ( _F_.getHorizontalAlign() == SKN_HALIGN_RIGHT ? DT_RIGHT : ( _F_.getHorizontalAlign() == SKN_HALIGN_CENTER ? DT_CENTER : DT_LEFT ) )

	// Image
	if (avatar.isVisible() && proto->CanGetAvatar() && proto->avatar_bmp != NULL)
	{
		RECT rc = avatar.getInsideRect();
		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		int width = Width(rc);
		int height = Height(rc);

		int round_radius;
		if (opts.draw_avatar_round_corner)
		{
			if (opts.draw_avatar_use_custom_corner_size)
				round_radius = opts.draw_avatar_custom_corner_size;
			else
				round_radius = min(width, height) / 6;
		}
		else
		{
			round_radius = 0;
		}


		AVATARDRAWREQUEST adr = {0};

		adr.cbSize = sizeof(AVATARDRAWREQUEST);
		adr.hTargetDC = hdc;
		adr.rcDraw = rc;

		adr.dwFlags = AVDRQ_OWNPIC | AVDRQ_HIDEBORDERONTRANSPARENCY | 
			(opts.draw_avatar_border ? AVDRQ_DRAWBORDER : 0 ) |
			(opts.draw_avatar_round_corner ? AVDRQ_ROUNDEDCORNER : 0 );
		adr.clrBorder =  opts.draw_avatar_border_color;
		adr.radius = round_radius;
		adr.alpha = 255;
		adr.szProto = proto->name;

		CallService(MS_AV_DRAWAVATAR, 0, (LPARAM) &adr);

		// Clipping rgn
		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);
	}

	// Nick
	if (nickname.isVisible())
	{
		SelectObject(hdc, nickname.getFont());
		SetTextColor(hdc, nickname.getFontColor());

		DrawTextWithRect(hdc, nickname.getText(), nickname.getRect(), nickname.getInsideRect(), 
						 uFormat | HALIGN(nickname), data->nick.mouseOver && proto->CanSetNick(), proto);
	}

	// Protocol cycle icon
	if (next_proto.isVisible())
	{
		RECT rc = next_proto.getInsideRect();
		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		DrawIconEx(hdc, rc.left, rc.top, next_proto.getIcon(), Width(rc), Height(rc), 0, NULL, DI_NORMAL);

		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);
	}

	if (prev_proto.isVisible())
	{
		RECT rc = prev_proto.getInsideRect();
		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		DrawIconEx(hdc, rc.left, rc.top, prev_proto.getIcon(), Width(rc), Height(rc), 0, NULL, DI_NORMAL);

		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);
	}

	// Protocol
	if (protocol.isVisible())
	{
		RECT rc = protocol.getInsideRect();
		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		SelectObject(hdc, protocol.getFont());
		SetTextColor(hdc, protocol.getFontColor());

		DrawText(hdc, protocol.getText(), -1, &rc, uFormat | HALIGN(protocol));

		// Clipping rgn
		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);

		if (data->proto.mouseOver)
			FrameRect(hdc, &protocol.getRect(), (HBRUSH) GetStockObject(GRAY_BRUSH));
	}

	// Status
	if (status_icon.isVisible() || status_name.isVisible())
	{
		if (status_icon.isVisible())
		{
			RECT rc = status_icon.getInsideRect();
			HRGN rgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hdc, rgn);

			DrawIconEx(hdc, rc.left, rc.top, status_icon.getIcon(), Width(rc), Height(rc), 0, NULL, DI_NORMAL);
			
			SelectClipRgn(hdc, NULL);
			DeleteObject(rgn);
		}

		if (status_name.isVisible())
		{
			RECT rc = status_name.getInsideRect();

			HRGN rgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hdc, rgn);

			SelectObject(hdc, status_name.getFont());
			SetTextColor(hdc, status_name.getFontColor());

			DrawText(hdc, status_name.getText(), -1, &rc, uFormat | HALIGN(status_name));

			SelectClipRgn(hdc, NULL);
			DeleteObject(rgn);			
		}

		if (data->status.mouseOver)
			FrameRect(hdc, &data->status.rc, (HBRUSH) GetStockObject(GRAY_BRUSH));
	}

	// Away message
	if (status_msg.isVisible())
	{
		SelectObject(hdc, status_msg.getFont());
		SetTextColor(hdc, status_msg.getFontColor());

		DrawTextWithRect(hdc, status_msg.getText(), status_msg.getRect(), status_msg.getInsideRect(), 
						 uFormat | HALIGN(protocol), data->away_msg.mouseOver && proto->CanSetStatusMsg(), proto);
	}

	// Listening to
	if (listening_icon.isVisible() || listening.isVisible())
	{
		if (listening_icon.isVisible())
		{
			RECT rc = listening_icon.getInsideRect();
			HRGN rgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hdc, rgn);
			
			DrawIconEx(hdc, rc.left, rc.top, listening_icon.getIcon(), Width(rc), Height(rc), 0, NULL, DI_NORMAL);
			
			SelectClipRgn(hdc, NULL);
			DeleteObject(rgn);
		}
		
		if (listening.isVisible())
		{
			RECT rc = listening.getInsideRect();
			HRGN rgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hdc, rgn);
			
			SelectObject(hdc, listening.getFont());
			SetTextColor(hdc, listening.getFontColor());
			
			DrawText(hdc, listening.getText(), -1, &rc, uFormat | HALIGN(listening));
			
			SelectClipRgn(hdc, NULL);
			DeleteObject(rgn);			
		}
		
		if (data->listening_to.mouseOver && protocols->CanSetListeningTo())
			FrameRect(hdc, &data->listening_to.rc, (HBRUSH) GetStockObject(GRAY_BRUSH));
	}

	// Unread email count
	if (email_icon.isVisible() || email.isVisible())
	{
		if (email_icon.isVisible())
		{
			RECT rc = email_icon.getInsideRect();
			HRGN rgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hdc, rgn);
			
			DrawIconEx(hdc, rc.left, rc.top, email_icon.getIcon(), Width(rc), Height(rc), 0, NULL, DI_NORMAL);
			
			SelectClipRgn(hdc, NULL);
			DeleteObject(rgn);
		}
		
		if (email.isVisible())
		{
			RECT rc = email.getInsideRect();
			HRGN rgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hdc, rgn);
			
			SelectObject(hdc, email.getFont());
			SetTextColor(hdc, email.getFontColor());
			
			DrawText(hdc, email.getText(), -1, &rc, uFormat | HALIGN(email));
			
			SelectClipRgn(hdc, NULL);
			DeleteObject(rgn);			
		}
	}
	SelectObject(hdc, old_font);
	SetTextColor(hdc, old_color);
	SetBkMode(hdc, old_bk_mode);

	BitBlt(hdc_orig, r_full.left, r_full.top, r_full.right - r_full.left, 
			r_full.bottom - r_full.top, hdc, r_full.left, r_full.top, SRCCOPY);
	DeleteDC(hdc);
	DeleteObject(hBmp);


	if (freeStatusIcon)
		DestroyIcon(hStatusIcon);
	ReleaseIconEx(hListeningIcon);
	ReleaseIconEx(hEmailIcon);
	ReleaseIconEx(hPrevIcon);
	ReleaseIconEx(hNextIcon);

	if (opts.resize_frame && ServiceExists(MS_CLIST_FRAMES_SETFRAMEOPTIONS) && frame_id != -1)
	{
		RECT rf;
		GetClientRect(hwnd, &rf);

		int currentSize = Height(r_full);

		int expectedSize = 0;
		for(size_t i = 0; i < data->items.size(); ++i)
		{
			SimpleItem *item = data->items[i];
			if (!item->draw)
				continue;

			expectedSize = max(expectedSize, item->rc.bottom);
		}
		expectedSize += state.getBorders().bottom;

		if (expectedSize != currentSize)
		{
			if (FrameIsFloating()) 
			{
				HWND parent = GetParent(hwnd);

				if (parent != NULL)
				{
					RECT rp_client, rp_window, r_window;
					GetClientRect(parent, &rp_client);
					GetWindowRect(parent, &rp_window);
					GetWindowRect(hwnd, &r_window);
					int diff = (rp_window.bottom - rp_window.top) - (rp_client.bottom - rp_client.top);
					if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
						diff += (r_window.top - rp_window.top);

					SetWindowPos(parent, 0, 0, 0, rp_window.right - rp_window.left, expectedSize + diff, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
				}
			}
			else if (IsWindowVisible(hwnd) && ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
			{
				int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
				if(flags & F_VISIBLE) 
				{
					CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, frame_id), (LPARAM) expectedSize);
					CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)(FU_TBREDRAW | FU_FMREDRAW | FU_FMPOS));
				}
			}
		}
	}
}

BOOL InsideRect(const POINT &p, const RECT &r)
{
	return p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom;
}

void MakeHover(HWND hwnd, POINT *p, SimpleItem *item)
{
	if (p != NULL && item->hitTest(*p))
	{
		item->mouseOver = TRUE;

		InvalidateRect(hwnd, NULL, FALSE);

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = hwnd;
		tme.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&tme);
	}
	else 
	{
		item->mouseOver = FALSE;

		InvalidateRect(hwnd, NULL, FALSE);
	}
}

int ShowPopupMenu(HWND hwnd, HMENU submenu, SimpleItem &item)
{
	POINT p;
	if (item.alignRight)
		p.x = item.rc.right;
	else
		p.x = item.rc.left;
	p.y =  item.rc.bottom+1;
	ClientToScreen(hwnd, &p);
	
	return TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD
			| (item.alignRight ? TPM_RIGHTALIGN : TPM_LEFTALIGN), p.x, p.y, 0, hwnd, NULL);
}


void ShowGlobalStatusMenu(HWND hwnd, MyDetailsFrameData *data, Protocol *proto, POINT &p)
{
	HMENU submenu = (HMENU) CallService(MS_CLIST_MENUGETSTATUS,0,0);
	
	int ret = ShowPopupMenu(hwnd, submenu, data->status);
	if(ret)
		CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(ret),MPCF_MAINMENU),(LPARAM)NULL);
}

void ShowProtocolStatusMenu(HWND hwnd, MyDetailsFrameData *data, Protocol *proto, POINT &p)
{
	HMENU menu = (HMENU) CallService(MS_CLIST_MENUGETSTATUS,0,0);
	HMENU submenu = NULL;

	if (menu != NULL)
	{
		// Find the correct menu item
		int count = GetMenuItemCount(menu);
		for (int i = 0 ; i < count && submenu == NULL; i++)
		{
			MENUITEMINFO mii = {0};

			mii.cbSize = sizeof(mii);

			if(!IsWinVer98Plus()) 
			{
				mii.fMask = MIIM_TYPE;
			}
			else 
			{
				mii.fMask = MIIM_STRING;
			}

			GetMenuItemInfo(menu, i, TRUE, &mii);

			if (mii.cch != 0)
			{
				mii.cch++;
				mii.dwTypeData = (char *)malloc(sizeof(char) * mii.cch);
				GetMenuItemInfo(menu, i, TRUE, &mii);

				if (strcmp(mii.dwTypeData, proto->description) == 0)
				{
					submenu = GetSubMenu(menu, i);
				}

				free(mii.dwTypeData);
			}
		}

		if (submenu == NULL && protocols->GetSize() == 1)
		{
			submenu = menu;
		}
	}

	if (submenu != NULL)
	{
		int ret = ShowPopupMenu(hwnd, submenu, data->status);
		if(ret)
			CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(ret),MPCF_MAINMENU),(LPARAM)NULL);
	}
	else
	{
		// Well, lets do it by hand
		static int statusModePf2List[]={0xFFFFFFFF,PF2_ONLINE,PF2_SHORTAWAY,PF2_LONGAWAY,PF2_LIGHTDND,PF2_HEAVYDND,PF2_FREECHAT,PF2_INVISIBLE,PF2_ONTHEPHONE,PF2_OUTTOLUNCH};

		menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
		submenu = GetSubMenu(menu, 0);
		CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

		DWORD flags = CallProtoService(proto->name, PS_GETCAPS, PFLAGNUM_2,0);
		for ( int i = GetMenuItemCount(submenu) -1  ; i >= 0 ; i-- )
		{
			if (!(flags & statusModePf2List[i]))
			{
				// Hide menu
				RemoveMenu(submenu, i, MF_BYPOSITION);
			}
		}

		int ret = ShowPopupMenu(hwnd, submenu, data->status);
		DestroyMenu(menu);

		if(ret) 
			proto->SetStatus(ret);
	}
}

void ShowListeningToMenu(HWND hwnd, MyDetailsFrameData *data, Protocol *proto, POINT &p)
{
	HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
	HMENU submenu = GetSubMenu(menu, 5);
	CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

	// Add this proto to menu
	char tmp[128];
	mir_snprintf(tmp, sizeof(tmp), Translate("Enable Listening To for %s"), proto->description);

	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
	mii.fType = MFT_STRING;
	mii.fState = proto->ListeningToEnabled() ? MFS_CHECKED : 0;
	mii.dwTypeData = tmp;
	mii.cch = strlen(tmp);
	mii.wID = 1;

	if (!proto->CanSetListeningTo())
	{
		mii.fState |= MFS_DISABLED;
	}

	InsertMenuItem(submenu, 0, TRUE, &mii);

	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	mii.fState = protocols->ListeningToEnabled() ? MFS_CHECKED : 0;

	if (!protocols->CanSetListeningTo())
	{
		mii.fState |= MFS_DISABLED;
	}

	SetMenuItemInfo(submenu, ID_LISTENINGTOPOPUP_SENDLISTENINGTO, FALSE, &mii);
	
	int ret = ShowPopupMenu(hwnd, submenu, data->listening_to);

	DestroyMenu(menu);

	switch(ret)
	{
		case 1:
		{
			CallService(MS_LISTENINGTO_ENABLE, (LPARAM) proto->name, !proto->ListeningToEnabled());
			break;
		}
		case ID_LISTENINGTOPOPUP_SENDLISTENINGTO:
		{
			CallService(MS_LISTENINGTO_ENABLE, 0, !protocols->ListeningToEnabled());
			break;
		}
	}

}


LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch(msg) 
	{
		case WM_CREATE: 
		{
			MyDetailsFrameData *data = new MyDetailsFrameData();
			SetWindowLong(hwnd, GWL_USERDATA, (LONG) data);

			data->get_status_messages = false;
			data->showing_menu = false;

			data->protocol_number = DBGetContactSettingWord(NULL,"MyDetails","ProtocolNumber",0);
			if (data->protocol_number >= protocols->GetSize())
			{
				data->protocol_number = 0;
			}

			SetCycleTime(hwnd);
			SetStatusMessageRefreshTime(hwnd);

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER | TME_LEAVE;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);

			return TRUE;
		}

		/*
		case WM_ERASEBKGND:
		{
			//EraseBackground(hwnd, (HDC)wParam); 
			Draw(hwnd, (HDC)wParam); 
			return TRUE;
		}
		*/

		case WM_PRINTCLIENT:
		{
			Draw(hwnd, (HDC)wParam);
			return TRUE;
		}

		case WM_PAINT:
		{
			RECT r;

			if(GetUpdateRect(hwnd, &r, FALSE)) 
			{
				PAINTSTRUCT ps;

				HDC hdc = BeginPaint(hwnd, &ps);
				Draw(hwnd, hdc);
				EndPaint(hwnd, &ps);
			}
			
			return TRUE;
		}

		case WM_SIZE:
		{
			//InvalidateRect(hwnd, NULL, FALSE);
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			RedrawFrame();
			break;
		}

		case WM_TIMER:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);

			if (wParam == ID_FRAME_TIMER)
			{
				if (!data->showing_menu)
					CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
			}
			else if (wParam == ID_RECALC_TIMER)
			{
				KillTimer(hwnd, ID_RECALC_TIMER);

				if (data->get_status_messages)
				{
					SetStatusMessageRefreshTime(hwnd);
					data->get_status_messages = false;

					protocols->GetStatuses();
					protocols->GetStatusMsgs();
				}

				RedrawFrame();
			}
			else if (wParam == ID_STATUSMESSAGE_TIMER)
			{
				SetStatusMessageRefreshTime(hwnd);

				PostMessage(hwnd, MWM_STATUS_MSG_CHANGED, 0, 0);
			}

			return TRUE;
		}

		case WM_LBUTTONUP:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			Protocol *proto = protocols->Get(data->protocol_number);
			if (proto == NULL)
				break;

			POINT p;
			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 

			// In image?
			if (data->avatar.hitTest(p) && proto->CanSetAvatar())
			{
				if (opts.global_on_avatar)
					CallService(MS_MYDETAILS_SETMYAVATARUI, 0, 0);
				else
					CallService(MS_MYDETAILS_SETMYAVATARUI, 0, (LPARAM) proto->name);
			}
			// In nick?
			else if (data->nick.hitTest(p) && proto->CanSetNick())
			{
				if (opts.global_on_nickname)
					CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, 0);
				else
					CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, (LPARAM) proto->name);
			}
			// In proto cycle button?
			else if (data->proto_cycle_next.hitTest(p))
			{
				CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
			}
			else if (data->proto_cycle_prev.hitTest(p))
			{
				CallService(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, 0, 0);
			}
			// In status message?
			else if (data->away_msg.hitTest(p) && proto->CanSetStatusMsg())
			{
				if (opts.global_on_status_message)
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, 0);
				else
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, (LPARAM) proto->name);
			}
			// In status?
			else if (data->status.hitTest(p))
			{
				data->showing_menu = true;

				if (opts.global_on_status)
					ShowGlobalStatusMenu(hwnd, data, proto, p);
				else
					ShowProtocolStatusMenu(hwnd, data, proto, p);

				data->showing_menu = false;
			}
			// In listening to?
			else if (data->listening_to.hitTest(p) && protocols->CanSetListeningTo())
			{
				ShowListeningToMenu(hwnd, data, proto, p);
			}
			// In protocol?
			else if (data->proto.hitTest(p))
			{
				data->showing_menu = true;

				HMENU menu = CreatePopupMenu();

				for (int i = protocols->GetSize() - 1 ; i >= 0 ; i--)
				{
					MENUITEMINFO mii = {0};
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID | MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = protocols->Get(i)->description;
					mii.cch = strlen(protocols->Get(i)->description);
					mii.wID = i + 1;

					if (i == data->protocol_number)
					{
						mii.fMask |= MIIM_STATE;
						mii.fState = MFS_DISABLED;
					}

					InsertMenuItem(menu, 0, TRUE, &mii);
				}
				
				int ret = ShowPopupMenu(hwnd, menu, data->proto);

				DestroyMenu(menu);

				if (ret != 0)
					PluginCommand_ShowProtocol(NULL, (WPARAM) protocols->Get(ret-1)->name);

				data->showing_menu = false;
			}

			break;
		}

		case WM_MEASUREITEM:
		{
			return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);
		}
		case WM_DRAWITEM:
		{
			return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
		}

		case WM_CONTEXTMENU:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			Protocol *proto = protocols->Get(data->protocol_number);
			if (proto == NULL)
				break;

			POINT p;
			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 

			ScreenToClient(hwnd, &p);

			data->showing_menu = true;

			// In image?
			if (data->avatar.hitTest(p))
			{
				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 4);
				CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

				// Add this proto to menu
				char tmp[128];
				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Avatar for %s..."), proto->description);

				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 1;

				if (!proto->CanSetAvatar())
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);
				
				ClientToScreen(hwnd, &p);
	
				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch(ret)
				{
					case 1:
					{
						CallService(MS_MYDETAILS_SETMYAVATARUI, 0, (LPARAM) proto->name);
						break;
					}
					case ID_AVATARPOPUP_SETMYAVATAR:
					{
						CallService(MS_MYDETAILS_SETMYAVATARUI, 0, 0);
						break;
					}
				}
			}
			// In nick?
			else if (data->nick.hitTest(p))
			{
				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 2);
				CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

				// Add this proto to menu
				char tmp[128];
				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Nickname for %s..."), proto->description);

				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 1;

				if (!proto->CanSetNick())
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);
				
				ClientToScreen(hwnd, &p);
	
				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch(ret)
				{
					case 1:
					{
						CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, (LPARAM) proto->name);
						break;
					}
					case ID_NICKPOPUP_SETMYNICKNAME:
					{
						CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, 0);
						break;
					}
				}
			}
			// In proto cycle button?
			else if (data->proto_cycle_next.hitTest(p))
			{
				CallService(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, 0, 0);
			}
			else if (data->proto_cycle_prev.hitTest(p))
			{
				CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
			}
			// In status message?
			else if (data->away_msg.hitTest(p))
			{
				char tmp[128];

				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 3);
				CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

				if (protocols->CanSetStatusMsgPerProtocol())
				{
					// Add this proto to menu
					mir_snprintf(tmp, sizeof(tmp), Translate("Set My Status Message for %s..."), 
								 proto->description);

					MENUITEMINFO mii = {0};
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID | MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = tmp;
					mii.cch = strlen(tmp);
					mii.wID = 1;

					if (!proto->CanSetStatusMsg())
					{
						mii.fMask |= MIIM_STATE;
						mii.fState = MFS_DISABLED;
					}

					InsertMenuItem(submenu, 0, TRUE, &mii);
				}
				
				{
					// Add this to menu
					mir_snprintf(tmp, sizeof(tmp), Translate("Set My Status Message for %s..."), 
								 CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, proto->status, 0));

					MENUITEMINFO mii = {0};
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID | MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = tmp;
					mii.cch = strlen(tmp);
					mii.wID = 2;

					if (proto->status == ID_STATUS_OFFLINE)
					{
						mii.fMask |= MIIM_STATE;
						mii.fState = MFS_DISABLED;
					}

					InsertMenuItem(submenu, 0, TRUE, &mii);
				}
				
				ClientToScreen(hwnd, &p);
	
				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch(ret)
				{
					case 1:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, (LPARAM) proto->name);
						break;
					}
					case 2:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, (WPARAM) proto->status, 0);
						break;
					}
					case ID_STATUSMESSAGEPOPUP_SETMYSTATUSMESSAGE:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, 0);
						break;
					}
				}
			}
			// In status?
			else if (data->status.hitTest(p))
			{
				if (opts.global_on_status)
					ShowProtocolStatusMenu(hwnd, data, proto, p);
				else
					ShowGlobalStatusMenu(hwnd, data, proto, p);
			}
			// In listening to?
			else if (data->listening_to.hitTest(p) && protocols->CanSetListeningTo())
			{
				ShowListeningToMenu(hwnd, data, proto, p);
			}
			// In protocol?
			else if (data->proto.hitTest(p))
			{
			}
			// Default context menu
			else 
			{
				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 1);
				CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

				if (opts.cycle_through_protocols)
					RemoveMenu(submenu, ID_CYCLE_THROUGH_PROTOS, MF_BYCOMMAND);
				else
					RemoveMenu(submenu, ID_DONT_CYCLE_THROUGH_PROTOS, MF_BYCOMMAND);

				// Add this proto to menu
				char tmp[128];
				MENUITEMINFO mii = {0};

				mir_snprintf(tmp, sizeof(tmp), Translate("Enable Listening To for %s"), proto->description);

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
				mii.fType = MFT_STRING;
				mii.fState = proto->ListeningToEnabled() ? MFS_CHECKED : 0;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 5;

				if (!proto->CanSetListeningTo())
				{
					mii.fState |= MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				// Add this to menu
				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Status Message for %s..."), 
							 CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, proto->status, 0));

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 4;

				if (proto->status == ID_STATUS_OFFLINE)
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				if (protocols->CanSetStatusMsgPerProtocol())
				{
					// Add this proto to menu
					mir_snprintf(tmp, sizeof(tmp), Translate("Set My Status Message for %s..."), proto->description);

					ZeroMemory(&mii, sizeof(mii));
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID | MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = tmp;
					mii.cch = strlen(tmp);
					mii.wID = 3;

					if (!proto->CanSetStatusMsg())
					{
						mii.fMask |= MIIM_STATE;
						mii.fState = MFS_DISABLED;
					}

					InsertMenuItem(submenu, 0, TRUE, &mii);
				}

				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Nickname for %s..."), proto->description);

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 2;

				if (!proto->CanSetNick())
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Avatar for %s..."), proto->description);

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 1;

				if (!proto->CanSetAvatar())
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_STATE;
				mii.fState = protocols->ListeningToEnabled() ? MFS_CHECKED : 0;

				if (!protocols->CanSetListeningTo())
				{
					mii.fState |= MFS_DISABLED;
				}

				SetMenuItemInfo(submenu, ID_CONTEXTPOPUP_ENABLELISTENINGTO, FALSE, &mii);

				ClientToScreen(hwnd, &p);
	
				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch(ret)
				{
					case 1:
					{
						CallService(MS_MYDETAILS_SETMYAVATARUI, 0, (LPARAM) proto->name);
						break;
					}
					case ID_AVATARPOPUP_SETMYAVATAR:
					{
						CallService(MS_MYDETAILS_SETMYAVATARUI, 0, 0);
						break;
					}
					case 2:
					{
						CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, (LPARAM) proto->name);
						break;
					}
					case ID_NICKPOPUP_SETMYNICKNAME:
					{
						CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, 0);
						break;
					}
					case 3:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, (LPARAM) proto->name);
						break;
					}
					case 4:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, (WPARAM) proto->status, 0);
						break;
					}
					case ID_STATUSMESSAGEPOPUP_SETMYSTATUSMESSAGE:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, 0);
						break;
					}
					case 5:
					{
						CallService(MS_LISTENINGTO_ENABLE, (LPARAM) proto->name, !proto->ListeningToEnabled());
						break;
					}
					case ID_CONTEXTPOPUP_ENABLELISTENINGTO:
					{
						CallService(MS_LISTENINGTO_ENABLE, 0, !protocols->ListeningToEnabled());
						break;
					}
					case ID_SHOW_NEXT_PROTO:
					{
						CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
						break;
					}
					case ID_SHOW_PREV_PROTO:
					{
						CallService(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, 0, 0);
						break;
					}
					case ID_CYCLE_THROUGH_PROTOS:
					{
						CallService(MS_MYDETAILS_CYCLE_THROUGH_PROTOCOLS, TRUE, 0);
						break;
					}
					case ID_DONT_CYCLE_THROUGH_PROTOS:
					{
						CallService(MS_MYDETAILS_CYCLE_THROUGH_PROTOCOLS, FALSE, 0);
						break;
					}
				}
			}

			data->showing_menu = false;


			break;
		}

		case WM_MOUSELEAVE:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
		}
		case WM_NCMOUSEMOVE:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);

			MakeHover(hwnd, NULL, &data->avatar);
			MakeHover(hwnd, NULL, &data->nick);
			MakeHover(hwnd, NULL, &data->proto);
			MakeHover(hwnd, NULL, &data->status);
			MakeHover(hwnd, NULL, &data->away_msg);
			MakeHover(hwnd, NULL, &data->listening_to);

			break;
		}

		case WM_MOUSEHOVER:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
		}
		case WM_MOUSEMOVE:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			Protocol *proto = protocols->Get(data->protocol_number);
			if (proto == NULL)
				break;

			POINT p;
			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 

			MakeHover(hwnd, &p, &data->avatar);
			MakeHover(hwnd, &p, &data->nick);
			MakeHover(hwnd, &p, &data->proto);
			MakeHover(hwnd, &p, &data->status);
			MakeHover(hwnd, &p, &data->away_msg);
			MakeHover(hwnd, &p, &data->listening_to);

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR) lParam;

			int i = (int) lpnmhdr->code;

			switch (lpnmhdr->code) {
				case TTN_GETDISPINFO:
				{
					MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);

					LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO) lpnmhdr;
					SendMessage(lpnmhdr->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
					
					for(int i = 0; i < data->items.size(); i++)
					{
						lpttd->lpszText = (char *) data->items[i]->getToolTipFor(lpnmhdr->hwndFrom);
						if (lpttd->lpszText != NULL)
							break;
					}

					return 0;
				}
			}

			break;
		}

		case WM_DESTROY:
		{
			KillTimer(hwnd, ID_FRAME_TIMER);

			MyDetailsFrameData *tmp = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			if (tmp != NULL) delete tmp;

			break;
		}

		// Custom Messages //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		case MWM_REFRESH:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);

			KillTimer(hwnd, ID_RECALC_TIMER);
			SetTimer(hwnd, ID_RECALC_TIMER, RECALC_TIME, NULL);
			break;
		}

		case MWM_AVATAR_CHANGED:
		{
			Protocol *proto = protocols->Get((const char *) wParam);

			if (proto != NULL)
			{
				proto->GetAvatar();
				RefreshFrame();
			}

			break;
		}

		case MWM_NICK_CHANGED:
		{
			Protocol *proto = protocols->Get((const char *) wParam);

			if (proto != NULL)
			{
				proto->GetNick();
				RefreshFrame();
			}

			break;
		}

		case MWM_STATUS_CHANGED:
		{
			Protocol *proto = protocols->Get((const char *) wParam);

			if (proto != NULL)
			{
				proto->GetStatus();
				proto->GetStatusMsg();
				proto->GetNick();

				RefreshFrame();
			}

			break;
		}

		case MWM_STATUS_MSG_CHANGED:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			data->get_status_messages = true;

			RefreshFrame();
			break;
		}

		case MWM_LISTENINGTO_CHANGED:
		{
			if (wParam != NULL)
			{
				Protocol *proto = protocols->Get((const char *) wParam);
				if (proto != NULL)
					proto->GetListeningTo();
			}

			RefreshFrameAndCalcRects();
			break;
		}

		case MWM_LOCK_CHANGED:
		{
			Protocol *proto = protocols->Get((const char *) wParam);
			if (proto != NULL)
			{
				proto->GetLocked();
				RefreshFrameAndCalcRects();
			}
			break;
		}

		case MWM_EMAIL_COUNT_CHANGED:
		{
			Protocol *proto = protocols->Get((const char *) wParam);
			if (proto != NULL)
			{
				proto->GetEmailCount();
				RefreshFrameAndCalcRects();
			}
			break;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


int ShowHideFrameFunc(WPARAM wParam, LPARAM lParam) 
{
	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
	{
		CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);
	}
	else
	{
		if (MyDetailsFrameVisible())
		{
			SendMessage(hwnd_container, WM_CLOSE, 0, 0);
		}
		else 
		{
			ShowWindow(hwnd_container, SW_SHOW);
			DBWriteContactSettingByte(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 1);
		}

		FixMainMenu();
	}
	return 0;
}


int ShowFrameFunc(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
	{
		int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
		if(!(flags & F_VISIBLE)) 
			CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);
	}
	else
	{
		if (!MyDetailsFrameVisible())
		{
			ShowWindow(hwnd_container, SW_SHOW);
			DBWriteContactSettingByte(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 1);

			FixMainMenu();
		}

	}
	return 0;
}


int HideFrameFunc(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
	{
		int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
		if (flags & F_VISIBLE) 
			CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);
	}
	else
	{
		if (MyDetailsFrameVisible())
		{
			SendMessage(hwnd_container, WM_CLOSE, 0, 0);

			FixMainMenu();
		}
	}
	return 0;
}


void FixMainMenu() 
{
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIM_NAME;

	if(MyDetailsFrameVisible())
		mi.pszName = Translate("Hide My Details");
	else
		mi.pszName = Translate("Show My Details");

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuShowHideFrame, (LPARAM)&mi);
}

#include <math.h>

void RedrawFrame() 
{
//	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd_frame, GWL_USERDATA);
//	if (data != NULL) 
//	{

		if(frame_id == -1) 
		{
			InvalidateRect(hwnd_container, NULL, TRUE);
		}
		else
		{
			CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)FU_TBREDRAW | FU_FMREDRAW);
		}
//	}
}

void RefreshFrameAndCalcRects() 
{
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, MWM_REFRESH, 0, 0);
}

void RefreshFrame() 
{
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, MWM_REFRESH, 0, 0);
}

// only used when no multiwindow functionality is available
BOOL MyDetailsFrameVisible() 
{
	return IsWindowVisible(hwnd_container) ? true : false;
}

void SetMyDetailsFrameVisible(BOOL visible) 
{
	if (frame_id == -1 && hwnd_container != 0) 
	{
		ShowWindow(hwnd_container, visible ? SW_SHOW : SW_HIDE);
	}
}

void SetCycleTime()
{
	if (hwnd_frame != NULL)
		SetCycleTime(hwnd_frame);
}

void SetCycleTime(HWND hwnd)
{
	KillTimer(hwnd, ID_FRAME_TIMER);

	if (opts.cycle_through_protocols)
		SetTimer(hwnd, ID_FRAME_TIMER, opts.seconds_to_show_protocol * 1000, 0);
}

void SetStatusMessageRefreshTime()
{
	if (hwnd_frame != NULL)
		SetStatusMessageRefreshTime(hwnd_frame);
}

void SetStatusMessageRefreshTime(HWND hwnd)
{
	KillTimer(hwnd, ID_STATUSMESSAGE_TIMER);

	opts.refresh_status_message_timer = DBGetContactSettingWord(NULL,"MyDetails","RefreshStatusMessageTimer",12);
	if (opts.refresh_status_message_timer > 0)
	{
		SetTimer(hwnd, ID_STATUSMESSAGE_TIMER, opts.refresh_status_message_timer * 1000, NULL);
	}
}

int PluginCommand_ShowNextProtocol(WPARAM wParam,LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return -1;

	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd_frame, GWL_USERDATA);

	data->protocol_number ++;
	if (data->protocol_number >= protocols->GetSize())
	{
		data->protocol_number = 0;
	}

	DBWriteContactSettingWord(NULL,"MyDetails","ProtocolNumber",data->protocol_number);

	SetCycleTime();

	RedrawFrame();

	return 0;
}

int PluginCommand_ShowPreviousProtocol(WPARAM wParam,LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return -1;

	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd_frame, GWL_USERDATA);

	data->protocol_number --;
	if (data->protocol_number < 0)
	{
		data->protocol_number = protocols->GetSize() - 1;
	}

	DBWriteContactSettingWord(NULL,"MyDetails","ProtocolNumber",data->protocol_number);

	SetCycleTime();

	RedrawFrame();

	return 0;
}

int PluginCommand_ShowProtocol(WPARAM wParam,LPARAM lParam)
{
	char * proto = (char *)lParam;
	int proto_num = -1;

	if (proto == NULL)
		return -1;

	for(int i = 0 ; i < protocols->GetSize() ; i++)
	{
		if (stricmp(protocols->Get(i)->name, proto) == 0)
		{
			proto_num = i;
			break;
		}
	}

	if (proto_num == -1)
		return -2;

	if (hwnd_frame == NULL)
		return -3;

	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd_frame, GWL_USERDATA);

	data->protocol_number = proto_num;
	DBWriteContactSettingWord(NULL,"MyDetails","ProtocolNumber",data->protocol_number);

	SetCycleTime();

	RedrawFrame();

	return 0;
}

int SettingsChangedHook(WPARAM wParam, LPARAM lParam) 
{
	if (hwnd_frame == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	if ((HANDLE)wParam == NULL)
	{
		Protocol *proto = protocols->Get((const char *) cws->szModule);

		if (!strcmp(cws->szSetting,"Status") 
				|| ( proto != NULL && proto->custom_status != 0 
					 && proto->custom_status_name != NULL 
					 && !strcmp(cws->szSetting, proto->custom_status_name) )
				|| ( proto != NULL && proto->custom_status != 0 
					 && proto->custom_status_message != NULL 
					 && !strcmp(cws->szSetting, proto->custom_status_message) ))
		{
			// Status changed
			if (proto != NULL)
				PostMessage(hwnd_frame, MWM_STATUS_CHANGED, (WPARAM) proto->name, 0);
		}
		else if(!strcmp(cws->szSetting,"MyHandle")
				|| !strcmp(cws->szSetting,"UIN") 
				|| !strcmp(cws->szSetting,"Nick") 
				|| !strcmp(cws->szSetting,"FirstName") 
				|| !strcmp(cws->szSetting,"e-mail") 
				|| !strcmp(cws->szSetting,"LastName") 
				|| !strcmp(cws->szSetting,"JID"))
		{
			// Name changed
			if (proto != NULL)
				PostMessage(hwnd_frame, MWM_NICK_CHANGED, (WPARAM) proto->name, 0);
		}
		else if (strstr(cws->szModule,"Away"))
		{
			// Status message changed
			PostMessage(hwnd_frame, MWM_STATUS_MSG_CHANGED, 0, 0);
		}
		else if (proto != NULL && strcmp(cws->szSetting,"ListeningTo") == 0)
		{
			PostMessage(hwnd_frame, MWM_LISTENINGTO_CHANGED, (WPARAM) proto->name, 0);
		}
		else if (proto != NULL && strcmp(cws->szSetting,"LockMainStatus") == 0)
		{
			PostMessage(hwnd_frame, MWM_LOCK_CHANGED, (WPARAM) proto->name, 0);
		}
	}

	return 0;
}

int AvatarChangedHook(WPARAM wParam, LPARAM lParam) 
{
	if (hwnd_frame == NULL)
		return 0;

	Protocol *proto = protocols->Get((const char *) wParam);

	if (proto != NULL)
		PostMessage(hwnd_frame, MWM_AVATAR_CHANGED, (WPARAM) proto->name, 0);

	return 0;
}

int ProtoAckHook(WPARAM wParam, LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return 0;

	ACKDATA *ack = (ACKDATA*)lParam;

	if (ack->type == ACKTYPE_STATUS) 
	{
		Protocol *proto = protocols->Get((const char *) ack->szModule);

		if (proto != NULL)
			PostMessage(hwnd_frame, MWM_STATUS_CHANGED, (WPARAM) proto->name, 0);
	}
	else if (ack->type == ACKTYPE_AWAYMSG)
	{
		Protocol *proto = protocols->Get((const char *) ack->szModule);

		if (proto != NULL)
			PostMessage(hwnd_frame, MWM_STATUS_MSG_CHANGED, (WPARAM) proto->name, 0);
	}
	else if (ack->type == ACKTYPE_EMAIL)
	{
		Protocol *proto = protocols->Get((const char *) ack->szModule);

		if (proto != NULL)
			PostMessage(hwnd_frame, MWM_EMAIL_COUNT_CHANGED, (WPARAM) proto->name, 0);
	}

	return 0;
}

int ListeningtoEnableStateChangedHook(WPARAM wParam,LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return 0;

	if (wParam == NULL || protocols->Get((const char *) wParam) != NULL)
		PostMessage(hwnd_frame, MWM_LISTENINGTO_CHANGED, wParam, 0);

	return 0;
}
