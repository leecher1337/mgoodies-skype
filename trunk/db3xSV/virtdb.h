#define MS_DBV_VIRTUALIZE "DB3XV/Virtualize"
// wParam: 0: If DB is real - virtualise it, if it is virtual - realize it;
// wParam: 1: If DB is real - virtualise it;
// wParam: 2: if it is virtual - realize it;
// wParam: else: do nothing.
// lParam: &DB_VIRTUAL_RESULT or 0;;
// returns the virtual state;

#define MS_DBV_SAVEFILE "DB3XV/SaveFile"
// wParam: null: ask for file
// wparam: -1: write to current db, but stay virtual
// wParam: char *: the filename
// lParam: &DB_VIRTUAL_RESULT or 0;
// returns TRUE on success; FALSE on error

typedef struct {
	int cbSize;			     // sizeof()
	char *szFileName;   // filename written or read
	unsigned int szFileNameSize;// amount of memory reserved for the FileName
	unsigned int imageSize;	 // Size of DB in bytes if virtual or 0;
	unsigned int errState;   // 0 if no error; TODO; ErrorCodes;
	unsigned int blWritten;  // number of blocks written if writing
    unsigned int blTotal;    // total number of blocks if writing
} DB_VIRTUAL_RESULT;

#define DB_VIRTUAL_ERR_NOTVIRTUAL 1
#define DB_VIRTUAL_ERR_SUCCESS 0
#define DB_VIRTUAL_ERR_NOFILE -1
#define DB_VIRTUAL_ERR_ACCESS -2
#define DB_VIRTUAL_ERR_PARTIAL -3



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





