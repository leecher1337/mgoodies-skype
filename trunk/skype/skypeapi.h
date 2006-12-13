// Skype API defines
#define SKYPECONTROLAPI_ATTACH_SUCCESS 0
#define SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION 1
#define SKYPECONTROLAPI_ATTACH_REFUSED 2
#define SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE 3
#define SKYPECONTROLAPI_ATTACH_API_AVAILABLE 0x8001

#define MISC_ERROR 1
#define USER_NOT_FOUND 2
#define USER_NOT_ONLINE 3
#define USER_BLOCKED 4
#define TYPE_UNSUPPORTED 5
#define SENDER_NOT_FRIEND 6
#define SENDER_NOT_AUTHORIZED 7

#define MAX_ENTRIES 128		// Max. 128 number-Entries in Dial-dlg.

#pragma comment(lib, "ws2_32")

typedef struct {
	int id;
	char *szStat;
} status_map;

struct MsgQueue {
	char *message;
	time_t tAdded;
	struct MsgQueue *next;
};

// Prototypes
int SkypeMsgInit(void);
int SkypeMsgAdd(char *msg);
void SkypeMsgCleanup(void);
char *SkypeMsgGet(void);
int SkypeSend(char*, ...);
char *SkypeRcv(char *what, DWORD maxwait);
int SkypeCall(WPARAM wParam, LPARAM lParam);
int SkypeOutCall(WPARAM wParam, LPARAM lParam);
int SkypeHup(WPARAM wParam, LPARAM lParam);
int SkypeHoldCall(WPARAM wParam, LPARAM lParam);
void SkypeFlush(void);
int SkypeStatusToMiranda(char *s);
char *MirandaStatusToSkype(int id);
char *GetSkypeErrorMsg(char *str);
BOOL testfor(char *what, DWORD maxwait);
int ConnectToSkypeAPI(char *path, bool bStart);
int SkypeAdduserDlg(WPARAM wParam, LPARAM lParam);
int SkypeAnswerCall(WPARAM wParam, LPARAM lParam);
int SkypeMsgCollectGarbage(time_t age);
int SkypeSendFile(WPARAM wParam, LPARAM lParam);
int SkypeSetAvatar(WPARAM wParam, LPARAM lParam);
int SkypeSetAwayMessage(WPARAM wParam, LPARAM lParam);
int SkypeSetNick(WPARAM wParam, LPARAM lParam);
int SkypeChatCreate(WPARAM wParam, LPARAM lParam);
int SkypeSetProfile(char *szProperty, char *szValue);
char *SkypeGet(char *szWhat, char *szWho, char *szProperty);
char *SkypeGetProfile(char *szProperty);