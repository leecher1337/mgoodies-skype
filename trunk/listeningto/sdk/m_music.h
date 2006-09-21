#ifndef M_MUSIC
#define M_MUSIC

typedef struct tSongInfoA {
    CHAR *artist;
    CHAR *title;
    CHAR *album;
    CHAR *genre;
    CHAR *comment;
    CHAR *year;
    CHAR *mfile;    // media file
    int kbps;
    int khz;
    int channels;
    int track;
    int total;      // music length
    int time;       // elapsed time
    CHAR *wndtext;  // window title
    CHAR *player;   // player name
    int plyver;     // player version
    HANDLE icon;    // player icon
    int fsize;      // media file size
    int vbr;
    int status;     // player status: 0 - stopped; 1 - playing; 2 - paused
    HWND plwnd;     // player window
    // video part
    int codec;
    int width;
    int height;
    int fps;
    __int64 date;
    CHAR *txtver;
    // not implemented yet
    HANDLE cover;   //TBITMAP?
    WCHAR *lyric;
} SONGINFOA, *LPSONGINFOA;

typedef struct tSongInfo {
    WCHAR *artist;
    WCHAR *title;
    WCHAR *album;
    WCHAR *genre;
    WCHAR *comment;
    WCHAR *year;
    WCHAR *mfile;   // media file
    int kbps;
    int khz;
    int channels;
    int track;
    int total;      // music length
    int time;       // elapsed time
    WCHAR *wndtext; // window title
    WCHAR *player;  // player name
    int plyver;     // player version
    HANDLE icon;    // player icon
    int fsize;      // media file size
    int vbr;
    int status;     // player status: 0 - stopped; 1 - playing; 2 - paused
    HWND plwnd;     // player window
    // video part
    int codec;
    int width;
    int height;
    int fps;
    __int64 date;
    WCHAR txtver;
    // not implemented yet
    HANDLE cover;   //TBITMAP?
    WCHAR *lyric;
} SONGINFO, *LPSONGINFO;

/*
  wParam : 0 - Unicode strings, 1 - ANSI, other - UTF8
  lParam : pointer to LPSONGINGO (Unicode) or LPSONGINFOA (ANSI/UTF8)
  Affects: Fill structure by currently played music info
  returns: 0 if unsuccesful, 1 if song played, 2 if only player is run
  note: pointer will be point to global SONGINFO structure of plugin
  warning: Non-Unicode data filled only by request
  Example:
    LPSONGINFO p;
    PluginLink->CallService(MS_WAT_GETMUSICINFO,0,(DWORD)&p);
*/

#define MS_WAT_GETMUSICINFO  "WATrack/GetMusicInfo"

/*
  wParam: not used
  lParam: not used
  Affects: Show popup or Info window with current music information
  note: Only Info window will be showed if Popup plugin disabled
*/

#define MS_NORMAL   0
#define MS_NOMUSIC  1
#define MS_NOTFOUND 2

#define MS_WAT_SHOWMUSICINFO "WATrack/ShowMusicInfo"

/*
  wParam: pointer to log file name or NULL
  lParam: pointer to report file name or NULL
  Affects: Create report from log and run it (if option is set)
  returns: 0 if unsuccesful
  note: if wParam or lParam is a NULL then file names from options are used
*/
#define MS_WAT_MAKEREPORT   "WATrack/MakeReport"

/*
  wParam, lParam - not used
  Affects: pack statistic file
*/
#define MS_WAT_PACKLOG = "WATrack/PackLog"

/*
  wParam, lParam: not used
  returns: pointer to INI file path (ANSI, do not free!)
*/
#define MS_WAT_GETINIPATH   "WATrack/GetINIPath"

/*
  wParam: 1  - switch off plugin
          0  - switch on plugin
          -1 - switch plugin status
          other - get plugin status
  lParam: 0
  Affects: Switch plugin status to enabled or disabled
  returns: old plugin status, 0, if was enabled
*/

#define MS_WAT_PLUGINSTATUS "WATrack/PluginStatus"


#define WAT_CTRL_PREV   1
#define WAT_CTRL_PLAY   2
#define WAT_CTRL_PAUSE  3
#define WAT_CTRL_STOP   4
#define WAT_CTRL_NEXT   5
#define WAT_CTRL_VOLDN  6
#define WAT_CTRL_VOLUP  7
#define WAT_CTRL_SEEK   8 // lParam is new position (sec)
// #define WAT_CTRL_OPEN   9

/*
  wParam: button code (WAT_CTRL_* const)
  lParam: 0, or value (see WAT_CTRL_* const comments)
  Affects: emulate player button pressing
  returns: 0 if unsuccesful
*/

#define MS_WAT_PRESSBUTTON  "WATrack/PressButton"

#define ME_WAT_MODULELOADED "WATrack/ModuleLoaded"


#define WAT_EVENT_PLAYERSTATUS    1 // 0-normal; 1-no music (possibly stopped); 2-not found
#define WAT_EVENT_NEWTRACK        2
#define WAT_EVENT_PLUGINSTATUS    3 // 0-enabled; 1-dis.temporary; 2-dis.permanent

/*
  Plugin or player status changed:
  wParam: type of event (see above)
  lParam: value
*/

#define ME_WAT_NEWSTATUS    "WATrack/NewStatus"

#define WAT_ACT_REGISTER    1
#define WAT_ACT_UNREGISTER  2
#define WAT_ACT_DISABLE     3
#define WAT_ACT_ENABLE      4
#define WAT_ACT_GETSTATUS   5         // not found/enabled/disabled
#define WAT_ACT_REPLACE     0x10000   // can be combined with WAT_REGISTERFORMAT

  // result codes
#define WAT_RES_NOTFOUND    -1
#define WAT_RES_ERROR       WAT_RES_NOTFOUND
#define WAT_RES_OK          0
#define WAT_RES_DISABLED    1
#define WAT_RES_ENABLED     2
  // internal
#define WAT_RES_NEWFILE     3

  // flags
#define WAT_OPT_DISABLED    0x01	// format registered but disabled
#define WAT_OPT_ONLYONE     0x02	// format can't be overwriten
#define WAT_OPT_PLAYERINFO  0x04	// song info from player
#define WAT_OPT_WINAMPAPI   0x08	// Winamp API support
#define WAT_OPT_CHECKTIME   0x10	// check file time for changes
#define WAT_OPT_USEOLE      0x20	// use COM/OLE interface
#define WAT_OPT_LAST        0x40	// (for Winamp Clone) put to the end of queue
#define WAT_OPT_INTERNAL    0x80  // internal. Do not use this flag!!!


typedef BOOL (__cdecl *LPREADFORMATPROC)(LPSONGINFO Info);

typedef struct tMusicFormat {
    LPREADFORMATPROC proc;
    CHAR ext[8];
    int flags;
} MUSICFORMAT, *LPMUSICFORMAT;

/*
  wParam: action
  lParam: pointer to MUSICFORMAT if wParam = WAT_ACT_REGISTER,
          else - pointer to extension string (ANSI)
  returns: see result codes
*/

#define MS_WAT_FORMAT  "WATrack/Format"

/*
  wParam - pointer to SONGINFO structure (plwind field must be initialized)
  lParam - flags
*/

#define MS_WAT_WINAMPINFO   "WATrack/WinampInfo"

/*
  wParam: window
  lParam: LoWord - command; HiWord - value
*/

#define MS_WAT_WINAMPCOMMAND  "WATrack/WinampCommand"

typedef WCHAR (__cdecl *LPNAMEPROC)();
typedef HWND (__cdecl *LPCHECKPROC)(int flags);
typedef int (__cdecl *LPINFOPROC)(LPSONGINFO Info, int flags);
typedef int (__cdecl *LPCOMMANDPROC)(int command, int value);

typedef struct tPlayerCell {
    CHAR *Desc;
    int flags;
    HICON Icon;                 // can be 0. for registration only
    LPCHECKPROC Check;          // check player 
    LPNAMEPROC GetName;         // can be NULL. get media filename
    LPINFOPROC GetInfo;         // can be NULL. get info from player
    LPCOMMANDPROC Command;      // can be NULL. send command to player
} PLAYERCELL, *LPPLAYERCELL;

/*
  wParam: action
  lParam: pointer to PLAYERCELL if wParam = WAT_ACT_REGISTER,
          else - pointer to player description string (ANSI)
  returns: player window handle or value>0 if found
*/

#define MS_WAT_PLAYER   "WATrack/Player"

#endif
