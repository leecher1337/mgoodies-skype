#include "seen.h"





/*
Prepares the log file:
- calculates the absolute path (and store it in the db)
- creates the directory

*/
int InitFileOutput(void)
{
	char szfpath[256]="",szmpath[256]="",*str;
	DBVARIANT dbv;

	GetModuleFileName(NULL,szmpath,MAX_PATH);
	strcpy(szfpath,!DBGetContactSetting(NULL,S_MOD,"FileName",&dbv)?dbv.pszVal:DEFAULT_FILENAME);

	DBFreeVariant(&dbv);

	if(szfpath[0]=='\\')
		strcpy(szfpath,szfpath+1);

	str=strrchr(szmpath,'\\');
	if(str!=NULL)
		*++str=0;

	strcat(szmpath,szfpath);
	
	strcpy(szfpath,szmpath);

	str=strrchr(szmpath,'\\');
	if(str!=NULL)
		*++str=0;
/*
//we dont need this anylonger. the directory is created in filewrite 
	if(!CreateDirectory(szmpath,NULL))
	{
		if(!(GetFileAttributes(szmpath) & FILE_ATTRIBUTE_DIRECTORY))
		{		
			MessageBox(NULL,"Directory could not be created\nPlease choose another!","Last seen plugin",MB_OK|MB_ICONERROR);
			DBWriteContactSettingByte(NULL,S_MOD,"FileOutput",0);
			return 0;
		}
	}
*/
	DBWriteContactSettingString(NULL,S_MOD,"PathToFile",szfpath);

	return 0;
}

//borrowed from netliblog.c
static void CreateDirectoryTree(char *szDir)
{
	DWORD dwAttributes;
	char *pszLastBackslash,szTestDir[MAX_PATH];

	lstrcpynA(szTestDir,szDir,sizeof(szTestDir));
	if((dwAttributes=GetFileAttributesA(szTestDir))!=0xffffffff && dwAttributes&FILE_ATTRIBUTE_DIRECTORY) return;
	pszLastBackslash=strrchr(szTestDir,'\\');
	if(pszLastBackslash==NULL) return;
	*pszLastBackslash='\0';
	CreateDirectoryTree(szTestDir);
	CreateDirectoryA(szTestDir,NULL);
}

/*
Writes a line into the log.
*/
void FileWrite(HANDLE hcontact)
{
	HANDLE fhout;
	DWORD byteswritten;
	char szout[1024],sznl[3]="\r\n";
	DBVARIANT dbv;

	DBGetContactSetting(NULL,S_MOD,"PathToFile",&dbv);
	strcpy(szout,ParseString(dbv.pszVal,hcontact,1));
	fhout=CreateFile(szout,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,0,NULL);
	if (fhout==INVALID_HANDLE_VALUE){
		CreateDirectoryTree(szout);
		fhout=CreateFile(szout,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,0,NULL);
		if (fhout==INVALID_HANDLE_VALUE) return;
	}
	DBFreeVariant(&dbv);
	SetFilePointer(fhout,0,0,FILE_END);

	strcpy(szout,ParseString(!DBGetContactSetting(NULL,S_MOD,"FileStamp",&dbv)?dbv.pszVal:DEFAULT_FILESTAMP,hcontact,1));
	DBFreeVariant(&dbv);
	
	WriteFile(fhout,szout,strlen(szout),&byteswritten,NULL);
	WriteFile(fhout,sznl,strlen(sznl),&byteswritten,NULL);

	CloseHandle(fhout);

	
}