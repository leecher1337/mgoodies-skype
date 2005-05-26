/*

IEView Plugin for Miranda IM
Copyright (C) 2005  Piotr Piastucki

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

#include "IEView.h"
#include "m_ieview.h"
#include "ieview_services.h"
#include "Options.h"
#include "Utils.h"
#include "SmileyWindow.h"

char *ieviewModuleName;
HINSTANCE hInstance;
PLUGINLINK *pluginLink;
IEView *debugView;
SmileyWindow *smileyWindow;
char *workingDir;
HANDLE hHookEvent;
static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int PreShutdown(WPARAM wParam, LPARAM lParam);

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"IEView Plugin",
	PLUGIN_MAKE_VERSION(1,0,3,3),
	"IE Control Chat Log (1.0.3.3 "__DATE__")",
	"Piotr Piastucki",
	"the_leech@users.berlios.de",
	"(c) 2004-2005 Piotr Piastucki",
	"http://developer.berlios.de/projects/mgoodies",
	0,
	0
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFO *MirandaPluginInfo(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0,3,1,0)) {
		MessageBox(NULL, "The IEView plugin cannot be loaded. It requires Miranda IM 0.3.1 or later.", "IEView Plugin", MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return NULL;
	}
	return &pluginInfo;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	char text[_MAX_PATH];
	char *p, *q;
	
	int wdsize = GetCurrentDirectory(0, NULL);
	workingDir = new char[wdsize];
	GetCurrentDirectory(wdsize, workingDir);
	Utils::convertPath(workingDir);

	GetModuleFileName(hInstance, text, sizeof(text));
	p = strrchr(text, '\\');
	p++;
	q = strrchr(p, '.');
	*q = '\0';
	ieviewModuleName = _strdup(p);
	_strupr(ieviewModuleName);

	pluginLink = link;

	HookEvent(ME_OPT_INITIALISE, IEViewOptInit);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	CreateServiceFunction(MS_IEVIEW_WINDOW, HandleIEWindow);
	CreateServiceFunction(MS_IEVIEW_EVENT, HandleIEEvent);
	CreateServiceFunction(MS_IEVIEW_SHOWSMILEYSELECTION, HandleSmileyShowSelection);
	return 0;
}

void test();

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	IEView::init();
	Options::init();
	return 0;
}

static int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	IEView::release();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	delete workingDir;
	return 0;
}


/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
char szClassName[ ] = "IEVIewTestWindowClass";

void test() {
    HWND hwnd;               /* This is the handle for our window */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl)) return;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "IEView Test Windows",       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
	ShowWindow (hwnd, SW_SHOW);

}

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
		case WM_CREATE:
			{
				debugView = new IEView(hwnd, (HTMLBuilder *)NULL, 0, 0, 300, 200);
			}
			break;
        case WM_DESTROY:
			break;
		case WM_SIZE:
//			debugView->setWindowPos(0, 0, LOWORD(lParam), HIWORD(lParam));
			debugView->setWindowPos(0, 0, LOWORD(lParam), HIWORD(lParam));
			break;
        default:
			break;
    }

    return DefWindowProc (hwnd, message, wParam, lParam);
}
