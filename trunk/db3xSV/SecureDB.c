/*
 *  SecureDB
 *
 *  Copyright (C) 2005  Piotr Pawluczuk (piotrek@piopawlu.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "commonheaders.h"
#include "database.h"
#include "SecureDB.h"
#include "sha256.h"
#include "initmenu.h"
#include "virtdb.h"
extern void __cdecl dbpanic(void *arg);

//we want to link to the old libs
#if defined _MSC_VER && _MSC_VER >= 1300
#define VC6LIBSPATH "d:/piotr/projekty/_libs_/lib/vc6/"
#pragma comment(lib,VC6LIBSPATH "msvcrt.lib")
#endif 

//global vars
char g_password[128] = {0};
long g_passlen = 1;
unsigned char g_sha256sum[32] = {0};

char g_newpassword[128] = {0};
long g_newpasslen = 0;
unsigned char g_new256sum[32] = {0};

long g_secured = 0;

//global handles
 HANDLE hSetPwdMenu = NULL;
 HANDLE hDelPwdMenu = NULL;

//encryption offset; all data except the header;
#define ENC_OFFSET sizeof(struct DBHeader)

BOOL CALLBACK DlgStdInProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			SendDlgItemMessage(hDlg,IDC_EDIT1,EM_LIMITTEXT,sizeof(g_password)-1,0);

			if(lParam)
			{
				SetDlgItemText(hDlg,IDC_EDIT2,(LPCSTR)lParam);
			}else{
				SendDlgItemMessage(hDlg,IDC_EDIT2,EM_LIMITTEXT,sizeof(g_password)-1,0);
			}
			SetWindowLong(hDlg,GWL_USERDATA,(lParam==0));

			return (TRUE);
		}
	case WM_COMMAND:
		{
			UINT uid = LOWORD(wParam);
			if(uid == IDOK){
				if(!GetWindowLong(hDlg,GWL_USERDATA))
				{
					g_passlen = GetDlgItemText(hDlg,IDC_EDIT1,g_password,sizeof(g_password)-1);
					if(g_passlen<4){
						MessageBox(hDlg,"Password is too short!","db3xS",MB_ICONEXCLAMATION);
						return TRUE;
					}else{
						EndDialog(hDlg,IDOK);
					}
				}else{
					char temp1[sizeof(g_password)+1];
					char temp2[sizeof(g_password)+1];
					long tlen1 = 0;
					long tlen2 = 0;

					tlen1 = GetDlgItemText(hDlg,IDC_EDIT2,temp1,sizeof(temp1)-1);
					tlen2 = GetDlgItemText(hDlg,IDC_EDIT1,temp2,sizeof(temp2)-1);

					if(tlen1 < 4 || tlen2 < 4){
						MessageBox(hDlg,"Password is too short!","db3xS",MB_ICONEXCLAMATION);
						return TRUE;
					}else if(strcmp(temp1,temp2)){
						MessageBox(hDlg,"Passwords do not match!","db3xS",MB_ICONEXCLAMATION);
					}else{
						strcpy(g_newpassword,temp1);
						g_newpasslen = tlen1;
						EndDialog(hDlg,IDOK);
					}
				}
			}else if(uid == IDCANCEL){
				EndDialog(hDlg,IDCANCEL);
			}
		}
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}

void updateCachedHdr(struct DBHeader* dbh)
{
	extern struct DBCacheSectionInfo cacheSectionInfo[CACHESECTIONCOUNT];
	extern PBYTE pDbCache;
	extern struct DBHeader dbHeader;
	int i=0;

	struct DBHeader* pdbh;

	dbHeader.version = dbh->version;
	memcpy(dbHeader.signature,dbh->signature,sizeof(pdbh->signature));

	//this is not that necessary, but it's better to make sure :)
	for(;i<CACHESECTIONCOUNT;i++){
		if(cacheSectionInfo[i].ofsBase == 0)
		{
			pdbh = (struct DBHeader*)(pDbCache + (i*CACHESECTIONSIZE));
			pdbh->version = dbh->version;
			memcpy(pdbh->signature,dbh->signature,sizeof(pdbh->signature));
			break;
		}
	}
}

int EncGetPassword(void* pdbh,const char* dbase)
{
	extern HINSTANCE g_hInst;
	int res;
	unsigned long pwdcrc=0;
	char* tmp;
	sha256_context ctx;

	struct DBHeader* dbh = (struct DBHeader*)pdbh;

	memset(g_password,0,sizeof(g_password));

	if(dbase){
		tmp = strrchr(dbase,'\\');
		if(tmp)dbase = tmp + 1;
	}

Again:
	res = DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_DIALOG1),NULL,(DLGPROC)DlgStdInProc,(LPARAM)dbase);
	if(res != IDOK)return -1;

	//generate the sha256 sum for the password; used in encryption + password checking
	sha256_starts(&ctx);
	sha256_update(&ctx,(uint8*)g_password,g_passlen);
	sha256_finish(&ctx,g_sha256sum);

	pwdcrc = (g_sha256sum[0] | (g_sha256sum[5] << 8)) + ((g_sha256sum[10]) | (g_sha256sum[31] << 8));

	if(dbh->version != pwdcrc){
		MessageBox(NULL,"Password is not correct!","db3xS",MB_ICONERROR);
		goto Again;
	}

	g_secured = 1;
	return 0;
}

static __inline void rotl(unsigned char* a,unsigned char n)
{
	_asm mov eax,a;
	_asm mov cl,n;
	_asm rol BYTE PTR[eax],cl;
}

static __inline void rotr(unsigned char* a,unsigned char n)
{
	_asm mov eax,a;
	_asm mov cl,n;
	_asm ror BYTE PTR[eax],cl;
}

static int RemovePassword()
{
	extern CRITICAL_SECTION csDbAccess;
	extern HANDLE hDbFile;
	extern struct DBSignature dbSignature;

	int result = 1;
	unsigned long size;
	unsigned long rw;
	unsigned char* buffer = NULL;
	struct DBHeader *dbh = NULL;

	EnterCriticalSection(&csDbAccess);

	VirtualFlushFileBuffers(hDbFile);
	size = VirtualGetFileSize(hDbFile,NULL);
	if(size < ENC_OFFSET)goto End;
	buffer = (unsigned char*)malloc(size + 1);
	if(!buffer)goto End;
	dbh = (struct DBHeader*)buffer;

	VirtualSetFilePointer(hDbFile,0,NULL,FILE_BEGIN);
	if(!EncReadFile(hDbFile,buffer,size,&rw,NULL) || rw!=size)goto End;
	g_secured = 0;

	dbh->version = DB_THIS_VERSION;
	memcpy(dbh->signature,&dbSignature,sizeof(dbh->signature));
	updateCachedHdr(dbh);

	VirtualSetFilePointer(hDbFile,0,NULL,FILE_BEGIN);
	if(!EncWriteFile(hDbFile,buffer,size,&rw,NULL) || rw!=size)goto End;
	VirtualFlushFileBuffers(hDbFile);
	result = 0;
End:
	if(buffer)free(buffer);
	LeaveCriticalSection(&csDbAccess);

	if(!g_secured){
		xModifyMenu(hSetPwdMenu,0,"Set Password");
		xModifyMenu(hDelPwdMenu,CMIF_GRAYED,NULL);
	}
	return result;
}

static int SetPassword()
{
	extern CRITICAL_SECTION csDbAccess;
	extern HANDLE hDbFile;
	extern struct DBSignature dbSecSignature;

	int result = 1;
	unsigned long size;
	unsigned long rw;
	unsigned char* buffer = NULL;
	struct DBHeader *dbh = NULL;

	EnterCriticalSection(&csDbAccess);

	VirtualFlushFileBuffers(hDbFile);
	size = VirtualGetFileSize(hDbFile,NULL);
	if(size < ENC_OFFSET)goto End;
	buffer = (unsigned char*)malloc(size + 1);
	if(!buffer)goto End;
	dbh = (struct DBHeader*)buffer;

	VirtualSetFilePointer(hDbFile,0,NULL,FILE_BEGIN);
	if(!EncReadFile(hDbFile,buffer,size,&rw,NULL) || rw!=size)goto End;
	g_secured = 1;

	dbh->version = (g_sha256sum[0] | (g_sha256sum[5] << 8)) + ((g_sha256sum[10]) | (g_sha256sum[31] << 8));
	memcpy(dbh->signature,&dbSecSignature,sizeof(dbh->signature));

	updateCachedHdr(dbh);

	VirtualSetFilePointer(hDbFile,0,NULL,FILE_BEGIN);
	if(!EncWriteFile(hDbFile,buffer,size,&rw,NULL) || rw!=size)goto End;
	VirtualFlushFileBuffers(hDbFile);
	result = 0;
End:
	if(buffer)free(buffer);
	LeaveCriticalSection(&csDbAccess);

	if(g_secured){
		xModifyMenu(hSetPwdMenu,0,"Change Password");
		xModifyMenu(hDelPwdMenu,0,NULL);
	}
	return result;
}


 int DB3XSSetPassword(WPARAM wParam, LPARAM lParam)
{
	extern HINSTANCE g_hInst;
	sha256_context ctx;
	int res;

	res = DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_DIALOG2),NULL,(DLGPROC)DlgStdInProc,FALSE);
	if(res != IDOK)return 0;

	sha256_starts(&ctx);
	sha256_update(&ctx,(uint8*)g_newpassword,g_newpasslen);
	sha256_finish(&ctx,g_new256sum);

	if(g_secured){RemovePassword();}
	g_passlen = g_newpasslen;
	memcpy(g_password,g_newpassword,g_newpasslen+1);
	memcpy(g_sha256sum,g_new256sum,32);

	if(SetPassword()){
		dbpanic(NULL);
	}else{
		MessageBox(NULL,Translate("Password has been changed!"),"SecureDB",MB_ICONINFORMATION);
	}

	return 0;
}

 int DB3XSRemovePassword(WPARAM wParam, LPARAM lParam)
{
	if(g_secured && MessageBox(NULL,Translate("Are you sure you want to remove the password?"),"SecureDB",MB_ICONQUESTION | MB_YESNO)==IDYES)
	{
		if(RemovePassword()){
			dbpanic(NULL);
		}else{
			MessageBox(NULL,Translate("Password has been removed!"),"SecureDB",MB_ICONINFORMATION);
		}
	}
	return 0;
}

 int DB3XSMakeBackup(WPARAM wParam, LPARAM lParam)
{
	extern CRITICAL_SECTION csDbAccess;
	extern HANDLE hDbFile;
	extern struct DBHeader dbHeader;

	HANDLE hNewFile = NULL;
	unsigned long size = 0;
	unsigned long rw = 0;
	unsigned long trw = 0;
	unsigned char buffer[2048];
	int result = 1;

	if(!lParam)return 1;

	hNewFile = CreateFile((LPCTSTR)lParam,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
	if(hNewFile == INVALID_HANDLE_VALUE)return 1;

	EnterCriticalSection(&csDbAccess);

	size = VirtualGetFileSize(hDbFile,NULL);
	if(size < sizeof(dbHeader))goto End;
	size -= sizeof(dbHeader);

	VirtualSetFilePointer(hDbFile,sizeof(dbHeader),NULL,FILE_BEGIN);
	
	if(!WriteFile(hNewFile,&dbHeader,sizeof(dbHeader),&rw,NULL)){
		goto End;
	}
	//we only make a copy so there is no need to decrypt the data;
	while(size){
		trw = min(2048,size);
		if(!VirtualReadFile(hDbFile,buffer,trw,&rw,NULL) || rw != trw){
			goto End;
		}
		if(!WriteFile(hNewFile,buffer,trw,&rw,NULL) || rw != trw){
			goto End;
		}
		size -= trw;
	}
	result = 0;
End:
	LeaveCriticalSection(&csDbAccess);

	CloseHandle(hNewFile);
	if(result){
		DeleteFile((LPCTSTR)lParam);
	}
	return result;
}

int EncReadFile(HANDLE hFile,void* data,unsigned long toread,unsigned long* read,void* dummy)
{
	unsigned char* bd = (unsigned char*)data;
	unsigned long i;
	unsigned long rtw;
	unsigned char pw;

	if(!g_secured){
		return VirtualReadFile(hFile,data,toread,read,NULL);
	}

	i = VirtualSetFilePointer(hFile,0,NULL,FILE_CURRENT);

	if(VirtualReadFile(hFile,data,toread,read,NULL))
	{
		for(rtw=i+*read;i<rtw;i++){
			if(i >= ENC_OFFSET){
				pw = (unsigned char)g_password[i%g_passlen];
				if(g_sha256sum[i%32] & 0x01){
					*bd = ~*bd;
				}
				if(pw & 0x01){
					rotr(bd,2);
				}
				*bd ^= pw;
				rotr(bd,pw);
				*bd ^= g_sha256sum[i%32];
			}
			bd++;
		}
		return TRUE;
	}else{
		return FALSE;
	}
}

int EncWriteFile(HANDLE hFile,void* data,unsigned long towrite,unsigned long* written,void* dummy)
{
	static unsigned char statbuff[16*1024];

	unsigned char* bd;
	unsigned char* dynbuff = NULL;
	unsigned long rtw;
	unsigned char* buffer;
	unsigned long i;
	unsigned char pw;

	if(!g_secured){
		return VirtualWriteFile(hFile,data,towrite,written,NULL);
	}

	if(towrite > 16*1024){
		buffer = dynbuff = (unsigned char*)malloc(towrite+1);
		if(!buffer)return FALSE;
	}else{
		buffer = statbuff;
	}

	memcpy(buffer,data,towrite);
	bd = buffer;

	i = VirtualSetFilePointer(hFile,0,NULL,FILE_CURRENT);
	rtw = i + towrite;

	for(;i<rtw;i++){
		if(i >= ENC_OFFSET){
			pw = (unsigned char)g_password[i%g_passlen];
			*bd ^= g_sha256sum[i%32];
			rotl(bd,pw);
			*bd ^= pw;
			if(pw & 0x01){
				rotl(bd,2);
			}
			if(g_sha256sum[i%32] & 0x01){
				*bd = ~*bd;
			}
		}else{
			pw = *bd;
		}
		bd++;
	}

	i = VirtualWriteFile(hFile,buffer,towrite,written,NULL);

	if(dynbuff){
		free(dynbuff);
	}
	return i;
}
