#define MS_DBV_VIRTUALIZE "DB3XV/Virtualize"
// wParam: 0: If DB is real - virtualise it, if it is virtual - realize it;
// wParam: 1: If DB is real - virtualise it;
// wParam: 2: if it is virtual - realize it;
// wParam: else: do nothing.
// lParam is not used;
// returns the virtual state;

#define MS_DBV_SAVEFILE "DB3XV/SaveFile"
// wParam: null: ask for file
// wParam: char *: the filename
// lParam is not used
// returns TRUE on success; FALSE on error



#define VirtualDBgranularity 4096

extern BOOL isDBvirtual;
extern DWORD virtualDBsize;
extern HANDLE hDbFile;

BOOL virtualizeDB();
BOOL realizeDB();

extern short int virtOnBoot;
extern short int realOnExit;
extern short int disableMenu;

extern HANDLE hOnExitHook;


extern PBYTE virtualdb;
extern BOOL isDBvirtual;


BOOL VirtualFlushFileBuffers(HANDLE hFile);
DWORD VirtualGetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
DWORD VirtualSetFilePointer(HANDLE hFile,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod);
BOOL VirtualReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
BOOL VirtualWriteFile(HANDLE hFile,LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite,LPDWORD lpNumberOfBytesWritten,LPOVERLAPPED lpOverlapped);
BOOL realizeDBonExit(WPARAM wParam, LPARAM lParam);
BOOL virtualizeDB();
void parseIniSettings();
int virtOnLoad();

 int virtualizeService(WPARAM wParam, LPARAM lParam);
 int saveFileService(WPARAM wParam, LPARAM lParam);





