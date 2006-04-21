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

	if(!CreateDirectory(szmpath,NULL))
	{
		if(!(GetFileAttributes(szmpath) & FILE_ATTRIBUTE_DIRECTORY))
		{		
			MessageBox(NULL,"Directory could not be created\nPlease choose another!","Last seen plugin",MB_OK|MB_ICONERROR);
			DBWriteContactSettingByte(NULL,S_MOD,"FileOutput",0);
			return 0;
		}
	}

	DBWriteContactSettingString(NULL,S_MOD,"PathToFile",szfpath);

	return 0;
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
	DBFreeVariant(&dbv);
	SetFilePointer(fhout,0,0,FILE_END);

	strcpy(szout,ParseString(!DBGetContactSetting(NULL,S_MOD,"FileStamp",&dbv)?dbv.pszVal:DEFAULT_FILESTAMP,hcontact,1));
	DBFreeVariant(&dbv);
	
	WriteFile(fhout,szout,strlen(szout),&byteswritten,NULL);
	WriteFile(fhout,sznl,strlen(sznl),&byteswritten,NULL);

	CloseHandle(fhout);

	
}