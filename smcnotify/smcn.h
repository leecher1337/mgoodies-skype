
#ifndef SMCN_H
#define SMCN_H

#define EVENTTYPE_STATUSCHANGE		25368

#define MODULE						"StatusMsgChangeNotify"

// Default option values
#define DEFAULT_COLBACK				RGB(201,125,234)
#define DEFAULT_COLTEXT				RGB(0,0,0)
#define DEFAULT_COLLISTBACK			0x00FFFFFF
#define DEFAULT_COLLISTTEXT			0x00000000

// Entrys in the database, don't translate
#define OPT_DISPOPUPS				"Disabled"
#define OPT_COLDEFAULT				"DefaultColor"
#define OPT_COLBACK					"ColorBack"
#define OPT_COLTEXT					"ColorText"
#define OPT_DSEC					"Delay"
#define OPT_DINFINITE				"DelayInfinite"
#define OPT_SHOWCH					"ShowMsgChanges"
#define OPT_HISTMAX					"HistoryMax"
#define OPT_LEFT					"LeftClickOption"
#define OPT_RIGHT					"RightClickOption"
#define OPT_SHOWONC					"ShowOnConnection"
#define OPT_IGNOREPOP				"PopupsIgnoreEmpty"
#define OPT_IGNOREALL				"AllIgnoreEmpty"
#define OPT_LCLKACT					"LeftClickAction"
#define OPT_RCLKACT					"RightClickAction"
#define OPT_LOG						"Log"
#define OPT_HISTORY					"History"
#define OPT_LOGTOFILE				"LogToFile"
#define OPT_LOGFILE					"LogFile"
#define OPT_POPTXT					"PopupText"
#define OPT_MSGCLRNTF				"MsgCleared"
#define OPT_MSGCHNNTF				"MsgChanged"
#define OPT_COLLISTTEXT				"List_TextColor"
#define OPT_COLLISTBACK				"List_BgColor"
#define OPT_LISTBG					"List_BgImage"
#define OPT_USEBGIMG				"ListUseBgImage"
#define OPT_AUTOREFRESH				"ListAutoRefresh"
#define OPT_USEOSD					"UseOSD"
#define OPT_HIDEMENU				"HideSettingsMenu"

// Service functions
#define MS_SMCN_POPUPS				"smcn/popups"
#define MS_SMCN_HIST				"smcn/showhistory"
#define MS_SMCN_LIST				"smcn/showlist"
#define MS_SMCN_GOTOURL				"smcn/gotourl"
#define MS_SMCN_CPOPUP				"smcn/contactpopups"

#define MAXSTRLEN					512
#define MAXPOPUPLEN					2048

#define TMR_CONNECTIONTIMEOUT		6000
//#define TMR_ICQFIRSTCHECK			TMR_CONNECTIONTIMEOUT // Need to be the same, because of timer
//#define TMR_AUTOREFRESHDELAY		500

#define TIMERID_STATUSCHECK			1
#define TIMERID_LISTAUTOREFRESH		2

#define ICONCOUNT					6

#define ICO_LIST			hLibIcons[0]
#define ICO_URL				hLibIcons[1]
#define ICO_HIST			hLibIcons[2]
#define ICO_LOG				hLibIcons[3]
#define ICO_POPUP_E			hLibIcons[4]
#define ICO_POPUP_D			hLibIcons[5]

#define IGNORE_POP		1
#define IGNORE_INT		2
#define IGNORE_EXT		4

#define IGNORE_MODULE	"Ignore"
#define IGNORE_MASK		"smcn"

#endif // SMCN_H