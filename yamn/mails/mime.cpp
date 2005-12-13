/*
 * This code implements retrieving info from MIME header
 *
 * (c) majvan 2002-2004
 */

#pragma warning( disable : 4290 )
/*
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include "../m_messages.h"
#include "../m_account.h"
#include "../browser/m_browser.h"
#include "../debug.h"
#include "m_mails.h"
#include "m_decode.h"*/
#include "../yamn.h"

//- imported ---------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

extern SWMRG *AccountBrowserSO;
extern struct WndHandles *MessageWnd;

extern int GetCharsetFromString(char *input,int size);
extern void SendMsgToRecepients(struct WndHandles *FirstWin,UINT msg,WPARAM wParam,LPARAM lParam);
extern void ConvertCodedStringToUnicode(char *stream,WCHAR **storeto,DWORD cp,int mode);
extern DWORD WINAPI MailBrowser(LPVOID Param);
extern DWORD WINAPI NoNewMailProc(LPVOID Param);
extern DWORD WINAPI BadConnection(LPVOID Param);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//Copies one string to another
// srcstart- source string
// srcend- address to the end of source string
// dest- pointer that stores new allocated string that contains copy of source string
// mode- MIME_PLAIN or MIME_MAIL (MIME_MAIL deletes '"' characters (or '<' and '>') if they are at start and end of source string
void CopyToHeader(char *srcstart,char *srcend,char **dest,int mode);

//Extracts email address (finds nick name and mail and then stores them to strings)
// finder- source string
// storeto- pointer that receives address of mail string
// storetonick- pointer that receives address of nickname
void ExtractAddressFromLine(char *finder,char **storeto,char **storetonick);

//Extracts simple text from string
// finder- source string
// storeto- pointer that receives address of string
void ExtractStringFromLine(char *finder,char **storeto);

//Extracts some item from content-type string
//Example: ContentType string: "TEXT/PLAIN; charset=US-ASCII", item:"charset=", returns: "US-ASCII"
// ContetType- content-type string
// value- string item
// returns extracted string (or NULL when not found)
char *ExtractFromContentType(char *ContentType,char *value);

//Extracts info from header text into header members
//Note that this function as well as struct CShortHeadwer can be always changed, because there are many items to extract
//(e.g. the X-Priority and Importance and so on)
// items- translated header (see TranslateHeaderFcn)
// head- header to be filled with values extracted from items
void ExtractShortHeader(struct CMimeItem *items,struct CShortHeader *head);

//Extracts header to mail using ExtractShortHeader fcn.
// items- translated header (see TranslateHeaderFcn)
// CP- codepage used when no default found
// head- header to be filled with values extracted from items, in unicode (wide char)
void ExtractHeader(struct CMimeItem *items,int CP,struct CHeader *head);

//Deletes items in CShortHeader structure
// head- structure whose items are deleted
void DeleteShortHeaderContent(struct CShortHeader *head);

//Deletes list of YAMN_MIMENAMES structures
// Names- pointer to first item of list
void DeleteNames(PYAMN_MIMENAMES Names);

//Deletes list of YAMN_MIMESHORTNAMES structures
// Names- pointer to first item of list
void DeleteShortNames(PYAMN_MIMESHORTNAMES Names);

//Makes a string lowercase
// string- string to be lowercased
void inline ToLower(char *string);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void CopyToHeader(char *srcstart,char *srcend,char **dest,int mode)
{
	char *dst;

	if(dest==NULL)
		return;
	if(srcstart>=srcend)
		return;

	if((mode==MIME_MAIL) && (((*srcstart=='"') && (*(srcend-1)=='"')) || ((*srcstart=='<') && (*(srcend-1)=='>'))))
	{
		srcstart++;
		srcend--;
	}
       
	if(srcstart>=srcend)
		return;

	if(NULL!=*dest)
		delete[] *dest;
	if(NULL==(*dest=new char[srcend-srcstart+1]))
		return;

	dst=*dest;

	for(;srcstart<srcend;dst++,srcstart++)
	{
		if(ENDLINE(srcstart))
		{
			while(ENDLINE(srcstart) || WS(srcstart)) srcstart++;
			*dst=' ';
			srcstart--;		//because at the end of "for loop" we increment srcstart
		}
		else
			*dst=*srcstart;
	}
	*dst=0;
}

void ExtractAddressFromLine(char *finder,char **storeto,char **storetonick)
{
	if(finder==NULL)
	{
		*storeto=*storetonick=NULL;
		return;
	}
	while(WS(finder)) finder++;
	if((*finder)!='<')
	{
		char *finderend=finder+1;
		do
		{
			if(ENDLINEWS(finderend))						//after endline information continues
				finderend+=2;
			while(!ENDLINE(finderend) && !EOS(finderend)) finderend++;		//seek to the end of line or to the end of string
		}while(ENDLINEWS(finderend));
		finderend--;
		while(WS(finderend) || ENDLINE(finderend)) finderend--;				//find the end of text, no whitespace
		if(*finderend!='>')						//not '>' at the end of line
			CopyToHeader(finder,finderend+1,storeto,MIME_MAIL);
		else								//at the end of line, there's '>'
		{
			char *finder2=finderend;
			while((*finder2!='<') && (finder2>finder)) finder2--;		//go to matching '<' or to the start
			CopyToHeader(finder2,finderend+1,storeto,MIME_MAIL);
			if(*finder2=='<')						//if we found '<', the rest copy as from nick
			{
				finder2--;
				while(WS(finder2) || ENDLINE(finder2)) finder2--;		//parse whitespace
				CopyToHeader(finder,finder2+1,storetonick,MIME_MAIL);		//and store nickname
			}
		}
	}
	else
	{
		char *finderend=finder+1;
		do
		{
			if(ENDLINEWS(finderend))							//after endline information continues
				finderend+=2;
			while(!ENDLINE(finderend) && (*finderend!='>') && !EOS(finderend)) finderend++;		//seek to the matching < or to the end of line or to the end of string
		}while(ENDLINEWS(finderend));
		CopyToHeader(finder,finderend+1,storeto,MIME_MAIL);				//go to first '>' or to the end and copy
		finder=finderend+1;
		while(WS(finder)) finder++;								//parse whitespace
		if(!ENDLINE(finder) && !EOS(finder))					//if there are chars yet, it's nick
		{
			finderend=finder+1;
			while(!ENDLINE(finderend) && !EOS(finderend)) finderend++;	//seek to the end of line or to the end of string
			finderend--;
			while(WS(finderend)) finderend--;				//find the end of line, no whitespace
			CopyToHeader(finder,finderend+1,storetonick,MIME_MAIL);
		}
	}
}

void ExtractStringFromLine(char *finder,char **storeto)
{
	if(finder==NULL)
	{
		*storeto=NULL;
		return;
	}
	while(WS(finder)) finder++;
	char *finderend=finder;

	do
	{
		if(ENDLINEWS(finderend)) finderend++;						//after endline information continues
		while(!ENDLINE(finderend) && !EOS(finderend)) finderend++;
	}while(ENDLINEWS(finderend));
	finderend--;
	while(WS(finderend)) finderend--;				//find the end of line, no whitespace
	CopyToHeader(finder,finderend+1,storeto,MIME_PLAIN);
}

char *ExtractFromContentType(char *ContentType,char *value)
{
	char *finder=strstr(ContentType,value);
	char *temp,*copier;
	char *CopiedString;

	if(finder==NULL)
		return NULL;
	temp=finder-1;
	while((temp>ContentType) && WS(temp)) temp--;			//now we have to find, if the word "Charset=" is located after ';' like "; Charset="
	if(*temp!=';' && temp!=ContentType)
		return NULL;
	finder=finder+strlen(value);						//jump over value string

	while(WS(finder)) finder++;					//jump over whitespaces
	temp=finder;
	while(*temp!=0 && *temp!=';') temp++;				//jump to the end of setting (to the next ;)
	temp--;
	while(WS(temp))	temp--;						//remove whitespaces from the end
	if(NULL==(CopiedString=new char[++temp-finder+1]))
		return NULL;
	for(copier=CopiedString;finder!=temp;*copier++=*finder++);			//copy string
	*copier=0;						//and end it with zero character

	return CopiedString;
}

void ExtractShortHeader(struct CMimeItem *items,struct CShortHeader *head)
{
	for(;items!=NULL;items=items->Next)
	{
		//at the start of line
		if(0==_strnicmp(items->name,"From",4))
		{
			if(items->value==NULL)
				continue;
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting from>");
#endif
			ExtractAddressFromLine(items->value,&head->From,&head->FromNick);
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
#endif
		}
		else if(0==_strnicmp(items->name,"Return-Path",11))
		{
			if(items->value==NULL)
				continue;
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting return-path>");
#endif
			ExtractAddressFromLine(items->value,&head->ReturnPath,&head->ReturnPathNick);
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
#endif
		}
		else if(0==_strnicmp(items->name,"Subject",7))
		{
			if(items->value==NULL)
				continue;
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting subject>");
#endif
			ExtractStringFromLine(items->value,&head->Subject);
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
#endif
		}
		else if(0==_strnicmp(items->name,"Date",4))
		{
			if(items->value==NULL)
				continue;
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting date>");
#endif
			ExtractStringFromLine(items->value,&head->Date);
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
#endif
		}
		else if(0==_strnicmp(items->name,"Content-Type",12))
		{
			if(items->value==NULL)
				continue;

			char *ContentType=NULL,*CharSetStr;
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting Content-Type>");
#endif
			ExtractStringFromLine(items->value,&ContentType);
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
#endif
			ToLower(ContentType);
			if(NULL!=(CharSetStr=ExtractFromContentType(ContentType,"charset=")))
			{
				head->CP=GetCharsetFromString(CharSetStr,strlen(CharSetStr));
				delete[] CharSetStr;
			}
			delete[] ContentType;
		}
		else if(0==_strnicmp(items->name,"Importance",10))
		{
			if(items->value==NULL)
				continue;
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting importance>");
#endif
			if(head->Priority!=-1)
			{
				if(0==strncmp(items->value,"low",3))
					head->Priority=5;
				else if(0==strncmp(items->value,"normal",6))
					head->Priority=3;
				else if(0==strncmp(items->value,"high",4))
					head->Priority=1;
			}
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
#endif
		}
		else if(0==_strnicmp(items->name,"X-Priority",10))
		{
			if(items->value==NULL)
				continue;
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<X-Priority>");
#endif
			if((*items->value>='1') && (*items->value<='5'))
				head->Priority=*items->value-'0';
#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
#endif
		}

	}
}

void ExtractHeader(struct CMimeItem *items,int CP,struct CHeader *head)
{
	struct CShortHeader ShortHeader;

	ZeroMemory(&ShortHeader,sizeof(struct CShortHeader));
	ShortHeader.Priority=ShortHeader.CP=-1;
#ifdef DEBUG_DECODE
	DebugLog(DecodeFile,"<Extracting header>\n");
#endif
	ExtractShortHeader(items,&ShortHeader);

	head->Priority=ShortHeader.Priority==-1 ? 3 : ShortHeader.Priority;
	CP=ShortHeader.CP==-1 ? CP : ShortHeader.CP;
#ifdef DEBUG_DECODE
	if(NULL!=ShortHeader.From)
		DebugLog(DecodeFile,"<Decoded from>%s</Decoded)\n",ShortHeader.From);
	if(NULL!=ShortHeader.FromNick)
		DebugLog(DecodeFile,"<Decoded from-nick>%s</Decoded)\n",ShortHeader.FromNick);
	if(NULL!=ShortHeader.ReturnPath)
		DebugLog(DecodeFile,"<Decoded return-path>%s</Decoded)\n",ShortHeader.ReturnPath);
	if(NULL!=ShortHeader.ReturnPathNick)
		DebugLog(DecodeFile,"<Decoded return-path nick>%s</Decoded)\n",ShortHeader.ReturnPathNick);
	if(NULL!=ShortHeader.Subject)
		DebugLog(DecodeFile,"<Decoded subject>%s</Decoded)\n",ShortHeader.Subject);
	if(NULL!=ShortHeader.Date)
		DebugLog(DecodeFile,"<Decoded date>%s</Decoded)\n",ShortHeader.Date);
	DebugLog(DecodeFile,"</Extracting header>\n");
	DebugLog(DecodeFile,"<Convert>\n");
#endif

	ConvertCodedStringToUnicode(ShortHeader.From,&head->From,CP,MIME_PLAIN);

#ifdef DEBUG_DECODE
	if(NULL!=head->From)
		DebugLogW(DecodeFile,L"<Converted from>%s</Converted>\n",head->From);
#endif
	ConvertCodedStringToUnicode(ShortHeader.FromNick,&head->FromNick,CP,MIME_MAIL);
#ifdef DEBUG_DECODE
	if(NULL!=head->FromNick)
		DebugLogW(DecodeFile,L"<Converted from-nick>%s</Converted>\n",head->FromNick);
#endif
	ConvertCodedStringToUnicode(ShortHeader.ReturnPath,&head->ReturnPath,CP,MIME_PLAIN);
#ifdef DEBUG_DECODE
	if(NULL!=head->ReturnPath)
		DebugLogW(DecodeFile,L"<Converted return-path>%s</Converted>\n",head->ReturnPath);
#endif
	ConvertCodedStringToUnicode(ShortHeader.ReturnPathNick,&head->ReturnPathNick,CP,MIME_MAIL);
#ifdef DEBUG_DECODE
	if(NULL!=head->ReturnPathNick)
		DebugLogW(DecodeFile,L"<Converted return-path nick>%s</Converted>\n",head->ReturnPathNick);
#endif
	ConvertCodedStringToUnicode(ShortHeader.Subject,&head->Subject,CP,MIME_PLAIN);
#ifdef DEBUG_DECODE
	if(NULL!=head->Subject)
		DebugLogW(DecodeFile,L"<Converted subject>%s</Converted>\n",head->Subject);
#endif
	ConvertCodedStringToUnicode(ShortHeader.Date,&head->Date,CP,MIME_PLAIN);
#ifdef DEBUG_DECODE
	if(NULL!=head->Date)
		DebugLogW(DecodeFile,L"<Converted date>%s</Converted>\n",head->Date);
#endif

#ifdef DEBUG_DECODE
	DebugLog(DecodeFile,"</Convert>\n");
#endif
	DeleteShortHeaderContent(&ShortHeader);

//	head->From=L"Frommmm";
//	head->Subject=L"Subject";
	return;
}

void DeleteShortHeaderContent(struct CShortHeader *head)
{
	if(head->From!=NULL) delete[] head->From;
	if(head->FromNick!=NULL) delete[] head->FromNick;
	if(head->ReturnPath!=NULL) delete[] head->ReturnPath;
	if(head->ReturnPathNick!=NULL) delete[] head->ReturnPathNick;
	if(head->Subject!=NULL) delete[] head->Subject;
	if(head->Date!=NULL) delete[] head->Date;
	if(head->To!=NULL) DeleteShortNames(head->To);
	if(head->Cc!=NULL) DeleteShortNames(head->Cc);
	if(head->Bcc!=NULL) DeleteShortNames(head->Bcc);
}

void DeleteHeaderContent(struct CHeader *head)
{
	if(head->From!=NULL) delete[] head->From;
	if(head->FromNick!=NULL) delete[] head->FromNick;
	if(head->ReturnPath!=NULL) delete[] head->ReturnPath;
	if(head->ReturnPathNick!=NULL) delete[] head->ReturnPathNick;
	if(head->Subject!=NULL) delete[] head->Subject;
	if(head->Date!=NULL) delete[] head->Date;
	if(head->To!=NULL) DeleteNames(head->To);
	if(head->Cc!=NULL) DeleteNames(head->Cc);
	if(head->Bcc!=NULL) DeleteNames(head->Bcc);
}

void DeleteNames(PYAMN_MIMENAMES Names)
{
	PYAMN_MIMENAMES Parser=Names,Old;
	for(;Parser!=NULL;Parser=Parser->Next)
	{
		if(Parser->Value!=NULL)
			delete[] Parser->Value;
		if(Parser->ValueNick!=NULL)
			delete[] Parser->ValueNick;
		Old=Parser;
		Parser=Parser->Next;
		delete Old;
	}
}

void DeleteShortNames(PYAMN_MIMESHORTNAMES Names)
{
	PYAMN_MIMESHORTNAMES Parser=Names,Old;
	for(;Parser!=NULL;Parser=Parser->Next)
	{
		if(Parser->Value!=NULL)
			delete[] Parser->Value;
		if(Parser->ValueNick!=NULL)
			delete[] Parser->ValueNick;
		Old=Parser;
		Parser=Parser->Next;
		delete Old;
	}
}


void inline ToLower(char *string)
{
	for(;*string!=0;string++)
		if(*string>='A' && *string<='Z') *string=*string-'A'+'a';
}