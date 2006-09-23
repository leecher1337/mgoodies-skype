#include <windows.h> 
#include <stdio.h> 
#include <process.h>

#include "wa_ipc.h" 
#include "GEN.h" 

// Plugin data //////////////////////////////////////////////////////////////////////////

int init(); 
void quit();

winampGeneralPurposePlugin plugin = {
	GPPHDR_VER,
	"Miranda ListeningTo Winamp Plugin", // Plug-in description 
	init,
	NULL, 
	quit,
}; 


// Globals //////////////////////////////////////////////////////////////////////////////


#define MIRANDA_WINDOWCLASS "Miranda.ListeningTo"
#define MIRANDA_DW_PROTECTION 0x8754

#define MESSAGE_WINDOWCLASS MIRANDA_WINDOWCLASS ".Winamp"

#define DATA_SIZE 1024

WNDPROC oldWndProc = NULL;
HMENU hMenuCreated = NULL;
HWND hMsgWnd = NULL;
HINSTANCE hInst = NULL;

// Message window proc
LRESULT CALLBACK MsgWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// Playlist window message processor
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


// Functions ////////////////////////////////////////////////////////////////////////////


void WindowThread(void *param)
{
	// Create window
	WNDCLASS wc = {0};
	wc.lpfnWndProc		= MsgWndProc;
	wc.hInstance		= hInst;
	wc.lpszClassName	= MESSAGE_WINDOWCLASS;

	RegisterClass(&wc);

	hMsgWnd = CreateWindow(MESSAGE_WINDOWCLASS, "Miranda ListeningTo Winamp Plugin", 
							0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, NULL);

	if (hMsgWnd != NULL)
		if (FindWindow(MIRANDA_WINDOWCLASS, NULL) != NULL)
			SetTimer(hMsgWnd, 0, 5000, NULL);

	MSG msg;
	BOOL bRet; 
    while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
    { 
        if (bRet == -1)
        {
            // handle the error and possibly exit
			break;
        }
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved) 
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstDll;

		plugin.hwndParent = NULL;

		_beginthread(WindowThread, 0, NULL);
	}

	return TRUE;
}


// Winamp interface function
extern "C" winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() 
{
	KillTimer(hMsgWnd, 0);

	return &plugin; 
}


inline void SendData(HWND hwnd, WCHAR *text) {
	COPYDATASTRUCT cds;
	cds.dwData = MIRANDA_DW_PROTECTION;
	cds.lpData = text;
	cds.cbData = (wcslen(text) + 1) * sizeof(WCHAR);

	SendMessage(hwnd, WM_COPYDATA, (WPARAM) plugin.hwndParent, (LPARAM) &cds);
}


void Concat(WCHAR *data, size_t &size, char *str)
{
	if (size < 3 * sizeof(WCHAR))
		return;

	if (str != NULL)
	{
		size_t len = strlen(str);
		if (size >= len + 3)
		{
			MultiByteToWideChar(CP_ACP, 0, str, (len+1)  * sizeof(char), &data[DATA_SIZE - size], size * sizeof(WCHAR));
			size -= len;
		}
	}

	wcscat(data, L"\\0");
	size -= 2;
}


void GetMetadata(extendedFileInfoStruct *efi, char *field, WCHAR *data, size_t &size)
{
	efi->ret[0] = '\0';
	efi->metadata = field;
	if (SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM) efi, IPC_GET_EXTENDED_FILE_INFO_HOOKABLE))
	{
		Concat(data, size, efi->ret);
	}
	else
	{
		Concat(data, size, NULL);
	}
}


void SendDataToMiranda(HWND hwnd, long currentSong)
{
	extendedFileInfoStruct efi;
	char tmp[256];

	efi.ret = tmp;
	efi.retlen = sizeof(tmp);

	// Get filename
	efi.filename = (char *) SendMessage(plugin.hwndParent, WM_WA_IPC, currentSong, IPC_GETPLAYLISTFILE);

	if (efi.filename == NULL 
		|| efi.filename[0] == '\0' 
		// Ignore streams
		|| strstr(efi.filename, "//") != NULL)
	{
		// Send empty data
		SendData(hwnd, L"0\\0Winamp\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0");
		return;
	}

	WCHAR data[DATA_SIZE];
	size_t size = DATA_SIZE;
	data[0] = L'\0';

	// L"<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0\\0"
	Concat(data, size, "1");
	Concat(data, size, "Winamp");

	if (SendMessage(hwnd, WM_WA_IPC, 3, IPC_GETINFO))
		Concat(data, size, "Video");
	else
		Concat(data, size, "Music");

	GetMetadata(&efi, "TITLE", data, size);
	GetMetadata(&efi, "ARTIST", data, size);
	GetMetadata(&efi, "ALBUM", data, size);
	GetMetadata(&efi, "TRACK", data, size);
	GetMetadata(&efi, "YEAR", data, size);
	GetMetadata(&efi, "GENRE", data, size);

	efi.ret[0] = '\0';
	efi.metadata = "LENGTH";
	if (SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM) &efi, IPC_GET_EXTENDED_FILE_INFO_HOOKABLE)
		&& efi.ret[0] != '\0' && efi.ret[1] != '\0')
	{
		char tmp[10];
		itoa(atoi(efi.ret) / 1000, tmp, 10);
		Concat(data, size, tmp);
	}
	else
	{
		Concat(data, size, NULL);
	}

	SendData(hwnd, data);
}


// Message window proc
LRESULT CALLBACK MsgWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_TIMER:
		{
			KillTimer(hwnd, 0);

			if (plugin.hwndParent == NULL)
			{
				plugin.hwndParent = FindWindow("Winamp v1.x", NULL);
				if (plugin.hwndParent != NULL)
					init();
			}

			break;
		}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


// Playlist window message processor
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL last_was_stop = TRUE;

	switch(message)
	{
		case WM_USER:
		{
			if (wParam == 666)
			{
				if (lParam & 0x40000000)
				{
					last_was_stop = FALSE;

					// Miranda is running?
					HWND mir_hwnd = FindWindow(MIRANDA_WINDOWCLASS, NULL);
					if (mir_hwnd != NULL)
						SendDataToMiranda(mir_hwnd, lParam & 0x0FFFFFFF);
				}
				else if (lParam == 0)
				{
					if (!last_was_stop)
					{
						last_was_stop = TRUE;

						HWND mir_hwnd = FindWindow(MIRANDA_WINDOWCLASS, NULL);
						if (mir_hwnd != NULL)
							SendData(mir_hwnd, L"0\\0Winamp\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0");
					}
				}
			}
			break;
		}
		case WM_CLOSE:
		{
			PostMessage(hMsgWnd, WM_CLOSE, 0, 0);
			break;
		}

	}

	return CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
}


int init() 
{
	KillTimer(hMsgWnd, 0);

	HWND hwnd = (HWND) SendMessage(plugin.hwndParent, WM_WA_IPC, IPC_GETWND_PE, IPC_GETWND);
	if (hwnd != NULL)
		oldWndProc = (WNDPROC) SetWindowLong(hwnd, GWL_WNDPROC, (LONG)WndProc);

	return 0; 
} 


void quit() 
{
} 
