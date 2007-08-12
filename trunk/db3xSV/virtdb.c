#include "commonheaders.h"
#include "virtdb.h"
#include "initmenu.h"

extern char szDbPath[MAX_PATH];


 PBYTE virtualdb;
static DWORD virtualDBsize=0;
static DWORD virtualPosition=0;
static DWORD numVirtualBlocks = 0;
 BOOL isDBvirtual = FALSE;

short int virtOnBoot = 1;// - yes 
short int realOnExit = 2;// - ask
short int disableMenu= 0;// - no

static DB_VIRTUAL_RESULT dbResult;

 HANDLE hOnExitHook = NULL;
 HANDLE hOnPreShutdownHook = NULL;


BOOL virtualizeDB()
{
	if (!isDBvirtual) {
		DWORD i = 0;
		DWORD bytesRead;
		extern CRITICAL_SECTION csDbAccess;
		EnterCriticalSection(&csDbAccess);
		virtualPosition =  SetFilePointer( hDbFile,0,NULL,FILE_CURRENT);
		virtualDBsize=GetFileSize(hDbFile,&i);
	    if (virtualDBsize==INVALID_FILE_SIZE) return isDBvirtual=FALSE;
		numVirtualBlocks = (virtualDBsize / VirtualDBgranularity)+1;
		virtualdb = (PBYTE)malloc(numVirtualBlocks*VirtualDBgranularity);
	
		SetFilePointer(hDbFile,0,NULL,FILE_BEGIN);
		for (i=0;i<numVirtualBlocks;i++){
			ReadFile(hDbFile,virtualdb+i*VirtualDBgranularity,VirtualDBgranularity,&bytesRead,NULL);
		}
		CloseHandle(hDbFile);
		LeaveCriticalSection(&csDbAccess);
		return isDBvirtual=TRUE;
	} else return FALSE;
}

BOOL writeMemToFile(char * filename, boolean leaveOpen){
	DWORD i;
	DWORD bytesRead;
	DWORD fileSize;
	DWORD courFilePos;
	char buff[VirtualDBgranularity];
	extern CRITICAL_SECTION csDbAccess;
	boolean result = TRUE;
	if (dbResult.cbSize) {
		strncpy(dbResult.szFileName,filename,dbResult.szFileNameSize);
		dbResult.blTotal = numVirtualBlocks;
		dbResult.blWritten = 0;
		dbResult.imageSize = virtualDBsize;
		dbResult.errState = DB_VIRTUAL_ERR_SUCCESS;
	}
	hDbFile=CreateFile(filename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,NULL);
	if (hDbFile == INVALID_HANDLE_VALUE) {
		if (dbResult.cbSize) {
			dbResult.blWritten = 0;
			dbResult.errState = DB_VIRTUAL_ERR_ACCESS; // failed to open file
		} else {
			char messg[MAX_PATH+200];
			sprintf(messg,Translate("Unable to realize DB to %s"),filename);
			MessageBox(NULL,messg,Translate("Miranda IM Profile Virtual Database"),MB_OK|MB_ICONINFORMATION); 
		}
		return FALSE;
	}
	EnterCriticalSection(&csDbAccess);
	fileSize = GetFileSize(hDbFile,&i);
	for (i=0;i<numVirtualBlocks;i++){
		boolean willWrite;
		courFilePos = i*VirtualDBgranularity;
		bytesRead = ((i+1)*VirtualDBgranularity>virtualDBsize)?(virtualDBsize-i*VirtualDBgranularity):VirtualDBgranularity;
		if ((courFilePos+bytesRead)<=fileSize){
			DWORD bytesActuallyRead;
			ReadFile(hDbFile, buff,bytesRead,&bytesActuallyRead,NULL);
			if (memcmp(buff,virtualdb+courFilePos,bytesRead)){
				// the block is not equal. will be overwritten
				SetFilePointer( hDbFile,courFilePos,NULL,FILE_BEGIN); // turnback the file pointer
				willWrite= TRUE;
			} else willWrite=FALSE; // the block is equal;
		} else willWrite = TRUE;//the file is smaller - will append the data;
		if (willWrite) {
			if (!WriteFile(hDbFile,virtualdb+courFilePos,bytesRead, &bytesRead,NULL)) {
				if (dbResult.cbSize) {
					dbResult.errState = DB_VIRTUAL_ERR_PARTIAL;
				} else {
					char messg[MAX_PATH+200];
					result = FALSE;
					sprintf(messg,Translate("Error writing DB to %s"),filename);
					MessageBox(NULL,messg,Translate("Miranda IM Profile Virtual Database"),MB_OK|MB_ICONINFORMATION); 
				}
				i = numVirtualBlocks; // urgent exit from the cicle ;)
			} else {
				//the block was successfuly written
				if (dbResult.cbSize) {
					dbResult.blWritten++;
				}
			}
		}
	}
	if (fileSize>virtualDBsize)SetEndOfFile(hDbFile);// trunkating the file if it was bigger by some reason
	LeaveCriticalSection(&csDbAccess);
	if (!leaveOpen) CloseHandle(hDbFile);
	return result;
}

static BOOL willRealize = FALSE;
#define MS_CLUI_GETHWND     "CLUI/GetHwnd"

BOOL AskToRealizeProc(WPARAM wParam, LPARAM lParam){
	if (isDBvirtual){
		if (realOnExit) {
			if (realOnExit==1){
				willRealize = TRUE;
			} else {
				char messg[MAX_PATH+200];
				int answ = IDYES;
				HANDLE clhwnd = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
				sprintf(messg,"%s\n%s",Translate("Realize DB?"),szDbPath);
				willRealize = FALSE;
				if (!IsWindow(clhwnd)) clhwnd = NULL; //Fsck! Someone destroyed Clist window too early!
				answ = MessageBox(clhwnd,messg,Translate("Miranda IM Profile Virtual Database"),MB_YESNO|MB_ICONQUESTION|MB_SETFOREGROUND|MB_TOPMOST);
				willRealize= (answ == IDYES);
			}
		}
	}
	UnhookEvent(hOnPreShutdownHook);
	hOnPreShutdownHook = NULL;
	return 0;
}

BOOL realizeDBonExit(WPARAM wParam, LPARAM lParam)
{
	if (willRealize) {
		realizeDB();
	} 
	UnhookEvent(hOnExitHook);
	hOnExitHook = NULL;
	return 0;
}
BOOL realizeDB()
{
	if (isDBvirtual){
		if (writeMemToFile (szDbPath, TRUE)){
			free(virtualdb);
			virtualDBsize = 0;
			return isDBvirtual = FALSE;
		};
	}
	return isDBvirtual;
}

BOOL VirtualFlushFileBuffers(HANDLE hFile)
{
	if (isDBvirtual) {
		return TRUE;
		//do nothing
	} else return FlushFileBuffers(hFile);
}

DWORD VirtualGetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
{
	if (isDBvirtual) {
		return virtualDBsize;
	}else{
		return GetFileSize(hFile, lpFileSizeHigh);
	}
}

DWORD VirtualSetFilePointer(HANDLE hFile,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod)
{
	if (isDBvirtual) {
		signed long result = 0;
		switch (dwMoveMethod) {
			case FILE_BEGIN : result = lDistanceToMove; break;
			case FILE_CURRENT : result = virtualPosition + lDistanceToMove; break;
			case FILE_END : result = virtualDBsize + lDistanceToMove; break;
			default : SetLastError(ERROR_SEEK_ON_DEVICE); return -1; break;
		}
		if (result<0) {
			SetLastError(ERROR_NEGATIVE_SEEK); 
			return -1;
		} else {
			SetLastError(ERROR_SUCCESS); 
			virtualPosition = result;
			return result;
		}
	} else {
		return SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
	}
}

BOOL VirtualReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	if (isDBvirtual) {
		if (nNumberOfBytesToRead+virtualPosition>virtualDBsize){
			if (virtualPosition<=virtualDBsize){
				memcpy(lpBuffer,virtualdb+virtualPosition,virtualDBsize-virtualPosition);
				*lpNumberOfBytesRead=virtualDBsize-virtualPosition;
				virtualPosition = virtualDBsize; // to the end of DB data
			} else {
				*lpNumberOfBytesRead=0;
				//virtualPosition remains the same
			}
		} else {
			memcpy(lpBuffer,virtualdb+virtualPosition,nNumberOfBytesToRead);
			*lpNumberOfBytesRead=nNumberOfBytesToRead;
			virtualPosition += nNumberOfBytesToRead; 
		}
		return TRUE;
	} else {
		return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}
}

BOOL VirtualWriteFile(HANDLE hFile,LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite,LPDWORD lpNumberOfBytesWritten,LPOVERLAPPED lpOverlapped)
{
	if (isDBvirtual) {
		if ((nNumberOfBytesToWrite+virtualPosition)>(numVirtualBlocks*VirtualDBgranularity)){
		//enlarge the DB
			DWORD newspaceSize = (nNumberOfBytesToWrite+virtualPosition);
			DWORD newNumBlocks = (newspaceSize / VirtualDBgranularity)+1;
            numVirtualBlocks = newNumBlocks;
			virtualdb = realloc(virtualdb,newNumBlocks*VirtualDBgranularity);
		}
		memcpy(virtualdb+virtualPosition,lpBuffer,nNumberOfBytesToWrite);
		*lpNumberOfBytesWritten=nNumberOfBytesToWrite;
		virtualPosition += nNumberOfBytesToWrite;
		if (virtualPosition>virtualDBsize) virtualDBsize = virtualPosition;
		return TRUE;
	} else {
		return WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	}
}


int virtualizeService(WPARAM wParam, LPARAM lParam){
	// wParam: 0: If DB is real - virtualise it, if it is virtual - realize it;
	// wParam: 1: If DB is real - virtualise it;
	// wParam: 2: if it is virtual - realize it;
	// wParam: else: do nothing.
	// lParam is not used;
	// returns the virtual state;
	if (lParam){
		memcpy(&dbResult,(DB_VIRTUAL_RESULT *)lParam,sizeof(dbResult));
		dbResult.blTotal = numVirtualBlocks;
		dbResult.blWritten = 0;
		dbResult.errState = DB_VIRTUAL_ERR_SUCCESS;
		strncpy(dbResult.szFileName,szDbPath,dbResult.szFileNameSize);
	} else dbResult.cbSize = 0;
	switch (wParam) {
		case 0 :
			if (isDBvirtual){
				realizeDB();
			} else {
				virtualizeDB();
			}
			break;
		case 1 : virtualizeDB(); break;
		case 2 : realizeDB(); break;
	}
	updateMenus();
	if (dbResult.cbSize) {
		dbResult.imageSize = virtualDBsize ;
		memcpy((DB_VIRTUAL_RESULT *)lParam,&dbResult,dbResult.cbSize);
	}
	dbResult.cbSize=0;
	return isDBvirtual;
}

int saveFileService(WPARAM wParam, LPARAM lParam){
// wParam: null: ask for file
// wparam: -1: write to current db, but stay virtual
// wParam: char *: the filename
// lParam is not used
// returns TRUE on success; FALSE on error
	int ret = 0;
	if (lParam){
		memcpy(&dbResult,(DB_VIRTUAL_RESULT *)lParam,sizeof(dbResult));
	} else dbResult.cbSize = 0;
	if (isDBvirtual) {
		if (!wParam){
		//extern HINSTANCE g_hInst;
			char str[MAX_PATH]="*.dat";
			OPENFILENAME ofn={0};
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = NULL;
			ofn.hInstance = NULL;
			ofn.lpstrFilter = "*.dat";
			ofn.lpstrFile = str;
			ofn.Flags = OFN_HIDEREADONLY;
			ofn.nMaxFile = sizeof(str);
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrDefExt = "dat";
			if(GetSaveFileName(&ofn)) {
				ret = writeMemToFile(str, FALSE);
			} else {
				ret = FALSE;
				if (dbResult.cbSize) {
					dbResult.errState = DB_VIRTUAL_ERR_NOFILE;
					dbResult.blTotal = numVirtualBlocks;
					dbResult.blWritten = 0;
					dbResult.imageSize = virtualDBsize ;
				}
			}
		} else if(wParam==-1) {
			ret = writeMemToFile(szDbPath, FALSE);
		} else {
			ret = writeMemToFile((char *)wParam, FALSE);
		}
	} else { 
		if (dbResult.cbSize){
		  dbResult.errState = DB_VIRTUAL_ERR_NOTVIRTUAL; // DB is not virtual
		  dbResult.blTotal = numVirtualBlocks;
		  dbResult.blWritten = 0;
		  dbResult.imageSize = virtualDBsize ;
		}
		ret = FALSE;
	}

	if (dbResult.cbSize) memcpy((DB_VIRTUAL_RESULT *)lParam,&dbResult,dbResult.cbSize);
	dbResult.cbSize=0;
	return ret;
}




void parseIniSettings(){
	char iniFile[MAX_PATH];
	char temp[4];
	{
		char mirDir[MAX_PATH]; 
		char *str2;
		GetModuleFileName(GetModuleHandle(NULL),mirDir,sizeof(mirDir));
		str2=strrchr(mirDir,'\\');
		if(str2!=NULL) *str2=0;
		lstrcpy(iniFile,mirDir);
		lstrcat(iniFile,"\\mirandaboot.ini");
	}
//MessageBox(NULL,iniFile,Translate("Miranda IM Profile Virtual Database"),MB_OK|MB_ICONEXCLAMATION);
	GetPrivateProfileString("Database","VDB_VirtOnBoot","yes",temp,sizeof(temp),iniFile);
	if (!lstrcmpi(temp,"yes")) virtOnBoot = 1;
	else if (!lstrcmpi(temp,"no")) virtOnBoot = 0;
	else MessageBox(NULL,"Invalid value of 'VDB_VirtOnBoot' in 'Database' section of 'mirandaboot.ini'.\nAllowed 'yes' or 'no'. Default is 'yes'",Translate("Miranda IM Profile Virtual Database"),MB_OK|MB_ICONEXCLAMATION); 

	GetPrivateProfileString("Database","VDB_RealOnExit","ask",temp,sizeof(temp),iniFile);
	if (!lstrcmpi(temp,"ask")) realOnExit = 2;
	else if (!lstrcmpi(temp,"yes")) realOnExit = 1;
	else if (!lstrcmpi(temp,"no")) realOnExit = 0;
	else MessageBox(NULL,"Invalid value of 'VDB_RealOnExit' in 'Database' section of 'mirandaboot.ini'.\nAllowed 'ask', 'yes' or 'no'. Default is 'ask'",Translate("Miranda IM Profile Virtual Database"),MB_OK|MB_ICONEXCLAMATION); 

	GetPrivateProfileString("Database","VDB_DisableMenu","no",temp,sizeof(temp),iniFile);
	if (!lstrcmpi(temp,"yes")) disableMenu = 1;
	else if (!lstrcmpi(temp,"no")) disableMenu = 0;
	else MessageBox(NULL,"Invalid value of 'VDB_DisableMenu' in 'Database' section of 'mirandaboot.ini'.\nAllowed 'yes' or 'no'. Default is 'no'",Translate("Miranda IM Profile Virtual Database"),MB_OK|MB_ICONEXCLAMATION); 

}

