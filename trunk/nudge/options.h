extern HINSTANCE hInst;
extern NudgeElementList* NudgeList;
extern int nProtocol;
extern bool useByProtocol;
extern CNudgeElement DefaultNudge;
extern CShake shake;
extern CNudge GlobalNudge;

static BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcNudgeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK DlgProcShakeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

void CreateImageList(HWND hWnd);
void PopulateProtocolList(HWND hWnd);
void UpdateControls(HWND hwnd);
int GetSelProto(HWND hwnd, HTREEITEM hItem);
void CheckChange(HWND hwnd, HTREEITEM hItem);
