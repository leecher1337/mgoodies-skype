#define MS_DBV_VIRTUALIZE "DB3XV/Virtualize"
// wParam: 0: If DB is real - virtualise it, if it is virtual - realize it;
// wParam: 1: If DB is real - virtualise it;
// wParam: 2: if it is virtual - realize it;
// wParam: else: do nothing - just return the state
// lParam: null or &DB_VIRTUAL_RESULT
// returns the virtual state;

#define MS_DBV_SAVEFILE "DB3XV/SaveFile"
// wParam: null: ask for file
// wParam: char *: the filename
// wparam: -1: write to current db, but stay virtual
// lParam: null or &DB_VIRTUAL_RESULT
// returns TRUE on success; FALSE on error

typedef struct {
	int cbSize;			     // sizeof()
	char *szFileName;   // filename written or read
	unsigned int szFileNameSize;// amount of memory reserved for the FileName
	unsigned int imageSize;	 // Size of DB in bytes if virtual or 0;
	unsigned int errState;   // 0 if no error;
	unsigned int blWritten;  // number of blocks written if writing
	unsigned int blTotal;    // total number of blocks if writing
} DB_VIRTUAL_RESULT;

#define DB_VIRTUAL_ERR_NOTVIRTUAL 1
#define DB_VIRTUAL_ERR_SUCCESS 0
#define DB_VIRTUAL_ERR_NOFILE -1
#define DB_VIRTUAL_ERR_ACCESS -2
#define DB_VIRTUAL_ERR_PARTIAL -3
