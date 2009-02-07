#ifndef M_MUSIC
#define M_MUSIC

#define MIID_WATRACK {0xfc6c81f4, 0x837e, 0x4430, {0x96, 0x01, 0xa0, 0xaa, 0x43, 0x17, 0x7a, 0xe3}}

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
    CHAR *lyric;
    CHAR *cover;
    int volume;
    CHAR *url;
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
    WCHAR *lyric;
    WCHAR *cover;
    int volume;
    WCHAR *url;
} SONGINFO, *LPSONGINFO;

#if defined(_UNICODE)
  #define WAT_INF_TCHAR WAT_INF_UNICODE
  #define SongInfoT tSongInfo
#else
  #define WAT_INF_TCHAR WAT_INF_ANSI
  #define SongInfoT tSongInfoA
#endif 

  // result codes
#define WAT_RES_UNKNOWN     -2
#define WAT_RES_NOTFOUND    -1
#define WAT_RES_ERROR       WAT_RES_NOTFOUND
#define WAT_RES_OK          0
#define WAT_RES_ENABLED     WAT_RES_OK
#define WAT_RES_DISABLED    1
  // internal
#define WAT_RES_NEWFILE     3

#define WAT_PLS_NORMAL   WAT_RES_OK
#define WAT_PLS_NOMUSIC  WAT_RES_DISABLED
#define WAT_PLS_NOTFOUND WAT_RES_NOTFOUND

#define WAT_INF_UNICODE 0
#define WAT_INF_ANSI    1
#define WAT_INF_UTF8    2
#define WAT_INF_CHANGES 0x100

/*
  wParam : WAT_INF_* constant
  lParam : pointer to LPSONGINGO (Unicode) or LPSONGINFOA (ANSI/UTF8)
  Affects: Fill structure by currently played music info
  returns: WAT_PLS_* constant
  note: pointer will be point to global SONGINFO structure of plugin
  warning: Non-Unicode data filled only by request
  if lParam=0 only internal SongInfo structure will be filled
  Example:
    LPSONGINFO p;
    PluginLink->CallService(MS_WAT_GETMUSICINFO,0,(DWORD)&p);
*/

#define MS_WAT_GETMUSICINFO  "WATrack/GetMusicInfo"

/*
  wParam:0
  lParam : pointer to pSongInfo (Unicode)
  Affects: Fill structure by info from file named in SongInfo.mfile
  returns: 0, if success
  note: fields, which values can't be obtained, leaves old values.
    you must free given strings by miranda mmi.free
*/
#define MS_WAT_GETFILEINFO "WATrack/GetFileInfo"

/*
  Get user's Music Info
*/
#define MS_WAT_GETCONTACTINFO = "WATrack/GetContactInfo"

#define WAT_CTRL_PREV   1
#define WAT_CTRL_PLAY   2
#define WAT_CTRL_PAUSE  3
#define WAT_CTRL_STOP   4
#define WAT_CTRL_NEXT   5
#define WAT_CTRL_VOLDN  6
#define WAT_CTRL_VOLUP  7
#define WAT_CTRL_SEEK   8 // lParam is new position (sec)
/*
  wParam: button code (WAT_CTRL_* const)
  lParam: 0, or value (see WAT_CTRL_* const comments)
  Affects: emulate player button pressing
  returns: 0 if unsuccesful
*/
#define MS_WAT_PRESSBUTTON  "WATrack/PressButton"

// ------------ Plugin/player status ------------

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

#define ME_WAT_MODULELOADED "WATrack/ModuleLoaded"

#define WAT_EVENT_PLAYERSTATUS    1 // 0-normal; 1-no music (possibly stopped); 2-not found
#define WAT_EVENT_NEWTRACK        2
#define WAT_EVENT_PLUGINSTATUS    3 // 0-enabled; 1-dis.temporary; 2-dis.permanent
#define WAT_EVENT_NEWPLAYER       4 //
#define WAT_EVENT_NEWTEMPLATE     5 // TM_* constant

/*
  Plugin or player status changed:
  wParam: type of event (see above)
  lParam: value
*/
#define ME_WAT_NEWSTATUS    "WATrack/NewStatus"

// ---------- Popup module ------------

/*
  wParam: not used
  lParam: not used
  Affects: Show popup or Info window with current music information
  note: Only Info window will be showed if Popup plugin disabled
*/

#define MS_WAT_SHOWMUSICINFO "WATrack/ShowMusicInfo"

// --------- Statistic (report) module -------------

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
  wParam: not used
  lParam: pointer to SongInfo
*/
#define MS_WAT_ADDTOLOG = "WATrack/AddToLog"

// ----------- Formats and players -----------

// media file status

#define WAT_MES_STOPPED 0
#define WAT_MES_PLAYING 1
#define WAT_MES_PAUSED  2
#define WAT_MES_UNKNOWN -1

#define WAT_ACT_REGISTER    1
#define WAT_ACT_UNREGISTER  2
#define WAT_ACT_DISABLE     3
#define WAT_ACT_ENABLE      4
#define WAT_ACT_GETSTATUS   5         // not found/enabled/disabled
#define WAT_ACT_SETACTIVE   6
#define WAT_ACT_REPLACE     0x10000   // can be combined with WAT_REGISTERFORMAT

  // flags
#define WAT_OPT_DISABLED    0x0001 // format registered but disabled
#define WAT_OPT_ONLYONE     0x0002 // format can't be overwriten
#define WAT_OPT_PLAYERINFO  0x0004 // song info from player
#define WAT_OPT_WINAMPAPI   0x0008 // Winamp API support
#define WAT_OPT_CHECKTIME   0x0010 // check file time for changes
#define WAT_OPT_VIDEO       0x0020 // only for format registering used
#define WAT_OPT_LAST        0x0040 // (internal)
#define WAT_OPT_FIRS        0x0080 // (internal)
#define WAT_OPT_TEMPLATE    0x0100 // (internal)
#define WAT_OPT_IMPLANTANT  0x0200 // use process implantation
#define WAT_OPT_HASURL      0x0400 // (player registration) URL field present
#define WAT_OPT_CHANGES     0x0800 // obtain only chaged values
                                   // (volume, status, window text, elapsed time)
#define WAT_OPT_MULTITHREAD 0x8000 // Use multithread scan
#define WAT_OPT_KEEPOLD     0x4000 // Keep Old opened file


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
    HICON Icon;            // can be 0. for registration only
    LPCHECKPROC Check;     // check player 
    LPNAMEPROC GetName;    // can be NULL. get media filename
    LPINFOPROC GetInfo;    // can be NULL. get info from player
    LPCOMMANDPROC Command; // can be NULL. send command to player
    CHAR *URL;             // only if WAT_OPT_HASURL flag present
} PLAYERCELL, *LPPLAYERCELL;

/*
  wParam: action
  lParam: pointer to PLAYERCELL if wParam = WAT_ACT_REGISTER,
          else - pointer to player description string (ANSI)
  returns: player window handle or value>0 if found
*/

#define MS_WAT_PLAYER   "WATrack/Player"

// --------- Templates ----------

//templates
#define TM_MESSAGE    0 // privat message
#define TM_CHANNEL    1 // chat
#define TM_STAT_TITLE 2 // xstatus title
#define TM_STAT_TEXT  3 // [x]status text
#define TM_POPTITLE   4 // popup title
#define TM_POPTEXT    5 // popup text
#define TM_EXPORT     6 // other app
#define TM_FRAMEINFO  7 // frame

#define TM_SETTEXT    0x100 // only for service
#define TM_GETTEXT    0     // only for service

/*
  wParam: not used
  lParam: Unicode template
  returns: New Unicode (replaced) string
*/
#define MS_WAT_REPLACETEXT "WATrack/ReplaceText"

/*
  event types for History
  Blob structure for EVENTTYPE_WAT_ANSWER:
   Uniciode artist#0title#0album#0answer
*/
#define EVENTTYPE_WAT_REQUEST 9601
#define EVENTTYPE_WAT_ANSWER  9602
#define EVENTTYPE_WAT_ERROR   9603
#define EVENTTYPE_WAT_MESSAGE 9604

/*
  wParam:  Template type (TM_* constants).
  lParam:  Template string for template setup, or not used
  returns: pointer to statically allocated string (DON'T free!)
  note:    Template set if wParam with TM_SETTEXT combined. If used for
           Protocol-dependent templates, used only for default templates.
*/
#define MS_WAT_TEMPLATE = "WATrack/Templates"

#endif
