/* Triggers */

typedef struct {
	int cbSize;
	char *pszName;				// used as identifier, must be unique
	HINSTANCE hInstance;		// only needed when options screen is available
	DLGPROC pfnDlgProc;			// only needed when options screen is available; the callback proc
	char *pszTemplate;			// only needed when options screen is available; must be WS_CHILD, Control parent and !visible are recommended
} TRIGGERREGISTER;

/* /Trigger/RegisterTrigger service
Register your trigger at the TriggerPlugin
  wParam = 0
  lParam = (LPARAM)(TRIGGERREGISTER *)&tr
If you try to register a trigger with a name that already exists, the service will fail.
Returns 0 on success, any other value on failure.
*/
#define MS_TRIGGER_REGISTERTRIGGER		"/Trigger/RegisterTrigger"

/* Dialog messages
These message will be send to the options dialog, if available. 
*/
/* TM_ADDTRIGGER
'OK' is pressed and a new trigger will be added. Save your settings using the given triggerID.
  wParam = (WPARAM)(DWORD)triggerID
  lParam = 0
*/
#define TM_ADDTRIGGER		WM_APP+10
/* TM_DELTRIGGER
The trigger addociated with the given triggerID will be removed, cleanup your settings for this trigger.
  wParam = (WPARAM)(DWORD)triggerID
  lParam = 0
*/
#define TM_DELTRIGGER		WM_APP+11

#define DF_CONTACT		0x01	// the hContact member is valid
#define DF_PROTO		0x02	// the szProto member is valid
#define DF_STATUS		0x04	// the status member is valid
#define DF_TEXT			0x08	// the szText member is valid
#define DF_LPARAM		0x10	// reserverd, lParam is valid

typedef struct {
	int cbSize;
	int dFlags;			// DF_*
	HANDLE hContact;
	char *szProto;
	int status;
	char *szText;
	LPARAM lParam;
} TRIGGERDATA;

typedef struct {
	int cbSize;
	DWORD triggerID;
	char *pszSummary;	// small summary to be shown in the options screen
	TRIGGERDATA *td;	// may be NULL, or only the dfFlags member set if triggerID is 0
} SPECIFICTRIGGERINFO;

/* TM_GETTRIGGERINFO
(optional) Information for a specific trigger is requested.
  wParam = (WPARAM)(DWORD)triggerID
  lParam = (LPARAM)(SPECIFICTRIGGERINFO **)&sti
Set *(SPECIFICTRIGGERINFO **)lParam to a (SPECIFICTRIGGERINFO *)&sti, and keep it in memory
If triggerID is 0, you may only set the dFlags member of the TRIGGERDATA struct.
*/
#define TM_GETTRIGGERINFO	WM_APP+12
/* TM_ACTIONCHANGED
(optional) Notification when the user changed the action in the options dialog.
  wParam = 0
  lParam = (SPECIFICACTIONINFO *)&sai
*/
#define TM_ACTIONCHANGED	WM_APP+13

#define TRG_PERFORM		0x01	// perform the actions associated to this trigger
#define TRG_CLEANUP		0x02	// remove all related information after the call
#define TRG_GETINFO		0x04	// retrieve the (SPECIFICATIONINFO *)&sai for this trigger (only if triggerID != 0)

typedef struct {
	int cbSize;
	DWORD triggerID;			// triggerID of the event to trigger or 0 for all
	char *pszName;				// name of trigger (may be NULL if triggerID isn't 0)
	int flags;					// flags (TRG_*)
	SPECIFICTRIGGERINFO *sti;	// may be NULL
} REPORT_INFO;

/* /Trigger/ReportAction
Service to call when you trigger your event.
  wParam = 0
  lParam = (REPORT_INFO *)&ri
If you're trigger does not have any options, set triggerID to 0 and TriggerPlugin will call all
associated actions, but set the triggerName member in that case.
*/
#define MS_TRIGGER_REPORTACTION			"/Trigger/ReportAction"




/* Actions */

typedef struct {
	int cbSize;
	char *pszName;			// must be unique!
	char *pszService;		// called with wParam = flags, lParam = (SPECIFICATIONINFO *)&sai
	HINSTANCE hInstance;	// only needed when options screen is available
	DLGPROC pfnDlgProc;		// only needed when options screen is available; the callback proc
	char *pszTemplate;		// only needed when options screen is available; must be WS_CHILD, Control parent and !visible are recommended
} ACTIONREGISTER;

/* /Trigger/RegisterAction service
Register your action at the TriggerPlugin
  wParam = 0
  lParam = (LPARAM)(ACTIONREGISTER *)&ar
If you try to register an action with a name that already exists, the service will fail.
Returns 0 on success, any other value on failure.
*/
#define MS_TRIGGER_REGISTERACTION		"/Trigger/RegisterAction"

/* Dialog messages
These message will be send to the options dialog, if available. 
*/
/* TM_ADDACTION
'OK' is pressed and a new trigger will be added. Save your settings using the given actionID.
  wParam = (WPARAM)(DWORD)actionID
  lParam = 0
*/
#define	TM_ADDACTION		WM_APP+1
/* TM_TRIGGERCHANGED
(optional) Notification when the user changed the trigger in the options dialog.
  wParam = 0
  lParam = (SPECIFICTRIGGERINFO *)&sai
*/
#define TM_TRIGGERCHANGED	WM_APP+2

#define ACT_PERFORM		0x01	// perform your action
#define ACT_CLEANUP		0x02	// cleanup associated settings
#define ACT_GETINFO		0x04	// return (SPECIFICACTIONINFO *)&sai

/* pszService
The service called, specified in the ACTIONREGISTER
  wParam = (WPARAM)ACT_* flags
  lParam = (LPARAM)SPECIFICACTIONINFO *)&sai
You can retrieve the actionID from the SPECIFICACTIONINFO. If ACT_PERFORM is set, you can retrive
additional data given by the trigger plugin from the (TRIGGERDATA *)&td member (this can be NULL).
Return 0 on success, any other on failure.
*/
typedef struct {
	int cbSize;
	DWORD actionID;		// actionID of the info requested
	char *pszSummary;	// null terminating string containing a summary of the action, may be NULL
	TRIGGERDATA *td;	// may be NULL
} SPECIFICACTIONINFO;



/* Helpers */
#define PREFIX_ACTIONID		"aid"
#define PREFIX_TRIGGERID	"tid"

typedef struct {
	int cbSize;
	char *prefix;		// prefix, defaults are PREFIX_ACTIONID for actions, PREFIX_TRIGGERID for triggers
	DWORD id;			// action or trigger ID of the setting to remove
	char *szModule;		// module where the settings are stored
	HANDLE hContact;	// associated contact
} REMOVETRIGGERSETTINGS;

/* /Trigger/RemoveSettings Service
  wParam = 0
  lParam = (LPARAM)(REMOVETRIGGERSETTINGS *)&rts
Removes settings from the database you wrote with the DBWriteAction* or DBWriteTrigger* helpers.
Returns the number of settings removed or -1 on failure.
*/
#define MS_TRIGGER_REMOVESETTINGS	"/Trigger/RemoveSettings"

typedef struct {
	int cbSize;
	char *prefix;		// prefix, defaults are PREFIX_ACTIONID for actions, PREFIX_TRIGGERID for triggers
	DWORD id;			// action or trigger ID of the previous setting
	char *szModule;		// module where the settings are stored
	HANDLE hContact;	// assoicated contact
} FINDTRIGGERSETTING;

/* /Trigger/FindNextSetting Service
  wParam = 0
  lParam = (LPARAM)(FINDTRIGGERSETTING *)&fts
Search for a setting in the database. Set id to 0 to start from the beginning, else search is done from id+1.
Returns (DWORD)triggerID or actionID depending on prefix of the next setting in the database. Returns 0 if
no more settings are found 
*/
#define MS_TRIGGER_FINDNEXTSETTING	"/Trigger/FindNextSetting"



/* Database helpers */
static int __inline DBWriteActionSettingByte(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,BYTE val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingByte(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteActionSettingWord(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,WORD val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingWord(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteActionSettingDword(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,DWORD val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingDword(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteActionSettingString(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingString(hContact, szModule, dbSetting, val);
}

static int __inline DBGetActionSettingByte(DWORD actionID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSettingByte(hContact, szModule, dbSetting, errorValue);
}

static WORD __inline DBGetActionSettingWord(DWORD actionID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSettingWord(hContact, szModule, dbSetting, errorValue);
}

static DWORD __inline DBGetActionSettingDword(DWORD actionID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSettingDword(hContact, szModule, dbSetting, errorValue);
}

static int __inline DBGetActionSetting(DWORD actionID, HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSetting(hContact, szModule, dbSetting, dbv);
}

static int __inline DBDeleteActionSetting(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBDeleteContactSetting(hContact, szModule, dbSetting);
}

/* Triggers */

static int __inline DBWriteTriggerSettingByte(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,BYTE val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingByte(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteTriggerSettingWord(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,WORD val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingWord(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteTriggerSettingDword(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,DWORD val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingDword(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteTriggerSettingString(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingString(hContact, szModule, dbSetting, val);
}

static int __inline DBGetTriggerSettingByte(DWORD triggerID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSettingByte(hContact, szModule, dbSetting, errorValue);
}

static WORD __inline DBGetTriggerSettingWord(DWORD triggerID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSettingWord(hContact, szModule, dbSetting, errorValue);
}

static DWORD __inline DBGetTriggerSettingDword(DWORD triggerID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSettingDword(hContact, szModule, dbSetting, errorValue);
}

static int __inline DBGetTriggerSetting(DWORD triggerID, HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSetting(hContact, szModule, dbSetting, dbv);
}

static int __inline DBDeleteTriggerSetting(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBDeleteContactSetting(hContact, szModule, dbSetting);
}