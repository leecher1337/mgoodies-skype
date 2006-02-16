extern HINSTANCE hInst;
extern int nScaleClist; 
extern int nScaleChat;
extern int nMoveClist; 
extern int nMoveChat;
extern NudgeElementList* NudgeList;
extern int nProtocol;
extern bool useByProtocol;
extern CNudgeElement DefaultNudge;
CNudgeElement* ActualNudge;

static BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcNudgeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK DlgProcShakeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
void CreateImageList(HWND hWnd);
void PopulateProtocolList(HWND hWnd);