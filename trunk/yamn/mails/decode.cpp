/*
 * This code implements decoding encoded MIME header in style
 * =?iso-8859-2?Q? "User using email in central Europe characters such as =E9" ?=
 *
 * (c) majvan 2002-2004
 */
/*
#include <windows.h>
#include <string.h>
#include "../debug.h"
#include "m_mails.h"
#include "m_decode.h"

#include <stdio.h>
#include <wchar.h>
*/
#include "../yamn.h"
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

struct cptable codepages[]=
{
	{"ibm037",37},
	{"ibm290",290},
	{"ibm437",437},
	{"ibm500",500},
	{"iso88596",708},
	{"asmo449",709},
	{"ibm775",775},
	{"ibm850",850},
	{"ibm852",852},
	{"ibm855",855},
	{"ibm857",857},
	{"ibm860",860},
	{"ibm861",861},
	{"ibm862",862},
	{"ibm863",863},
	{"ibm864",864},
	{"ibm865",865},
	{"ibm866",866},
	{"ibm869",869},
	{"ibm870",870},
	{"ibm423",875},
	{"gb231280",936},
	{"ksc5601",949},
	{"big5",950},
	{"ibm1026",1026},
	{"iso10646usc2",1200},
	{"windows1250",1250},
	{"windows1251",1251},
	{"windows1252",1252},
	{"windows1253",1253},
	{"windows1254",1254},
	{"windows1255",1255},
	{"windows1256",1256},
	{"windows1257",1257},
	{"windows1258",1258},
	{"ksc5601",1361},
	{"din66003",20105},
	{"din66003",20106},
	{"sen850200b",20107},
	{"t618bit",20261},
	{"ibm273",20273},
	{"ibm277",20277},
	{"ibm277",20278},
	{"ibm278",20277},
	{"ibm280",20280},
	{"ibm284",20284},
	{"ibm285",20285},
	{"ibm290",20290},
	{"ibm297",20297},
	{"ibm420",20420},
	{"ibm423",20423},
	{"ibm297",20297},
	{"ibmthai",20838},
	{"koi8r",20866},
	{"ibm871",20871},
	{"ibm880",20880},
	{"ibm905",20905},
	{"koi8u",21866},
	{"iso88591",28591},
	{"iso88592",28592},
	{"iso88593",28593},
	{"iso88594",28594},
	{"iso88595",28595},
	{"iso88596",28596},
	{"iso88597",28597},
	{"iso88598",28598},
	{"iso88599",28599},
	{"iso2022jp",50220},
	{"iso2022kr",50225},
	{"eucjp",51932},
	{"euckr",51949},
	{"hzgb2312",52936},
	{"utf7",CP_UTF7},
	{"utf8",CP_UTF8},
};

//Gets codepage ID from string representing charset such as "iso-8859-1"
// input- the string
// size- max length of input string
int GetCharsetFromString(char *input,int size);

//HexValue to DecValue ('a' to 10)
// HexValue- hexa value ('a')
// DecValue- poiner where to store dec value
// returns 0 if not success
int FromHexa(char HexValue,char *DecValue);

//Decodes a char from Base64
// Base64Value- input char in Base64
// DecValue- pointer where to store the result
// returns 0 if not success
int FromBase64(char Base64Value,char *DecValue);

//Decodes string in quoted printable
// Src- input string
// Dst- where to store output string
// DstLen- how max long should be output string
// always returns 1
int DecodeQuotedPrintable(char *Src,char *Dst,int DstLen);

//Decodes string in base64
// Src- input string
// Dst- where to store output string
// DstLen- how max long should be output string
// returns 0 if string was not properly decoded
int DecodeBase64(char *Src,char *Dst,int DstLen);

//Converts string to unicode from string with specified codepage
// stream- input string
// cp- codepage of input string
// out- pointer to new allocated memory that contains unicode string
int ConvertStringToUnicode(char *stream,unsigned int cp,WCHAR **out);

//Converts string from MIME header to unicode
// stream- input string
// cp- codepage of input string
// storeto- pointer to memory that contains unicode string
// mode- MIME_PLAIN or MIME_MAIL (MIME_MAIL deletes '"' from start and end of string)
void ConvertCodedStringToUnicode(char *stream,WCHAR **storeto,DWORD cp,int mode);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

int GetCharsetFromString(char *input,int size)
//Converts "ISO-8859-1" to "iso88591" and then return ID from table
{
	char *pin=input;
	char *pout,*parser;

	if((size<1) || (parser=pout=new char[size+1])==NULL)
		return -1;
	while((*pin!=0) && (pin-input<size))
	{
		if ((*pin>='A') && (*pin<='Z'))
			*parser++=*(pin++)+('a'-'A');
		else if((*pin>='0') && (*pin<='9') || (*pin>='a') && (*pin<='z'))
			*parser++=*pin++;
		else pin++;
	}

	*parser=(char)0;

#ifdef DEBUG_DECODECODEPAGE
	DebugLog(DecodeFile,"<CodePage>%s</CodePage>",pout);
#endif
	for(int i=0;i<sizeof(codepages)/sizeof(codepages[0]);i++)
		if(0==strncmp(pout,codepages[i].name,strlen(codepages[i].name)))
		{
			delete[] pout;
			return codepages[i].ID;
		}
	delete[] pout;
	return -1;		//not found
}

int FromHexa(char HexValue,char *DecValue)
{
	if(HexValue>='0' && HexValue<='9')
	{
		*DecValue=HexValue-'0';
		return 1;
	}
	if(HexValue>='A' && HexValue<='F')
	{
		*DecValue=HexValue-'A'+10;
		return 1;
	}
	if(HexValue>='a' && HexValue<='f')
	{
		*DecValue=HexValue-'a'+10;
		return 1;
	}
	return 0;
}

int FromBase64(char Base64Value,char *DecValue)
{
	if(Base64Value>='A' && Base64Value<='Z')
	{
		*DecValue=Base64Value-'A';
		return 1;
	}
	if(Base64Value>='a' && Base64Value<='z')
	{
		*DecValue=Base64Value-'a'+26;
		return 1;
	}
	if(Base64Value>='0' && Base64Value<='9')
	{
		*DecValue=Base64Value-'0'+52;
		return 1;
	}
	if(Base64Value=='+')
	{
		*DecValue=Base64Value-'+'+62;
		return 1;
	}
	if(Base64Value=='/')
	{
		*DecValue=Base64Value-'/'+63;
		return 1;
	}
	if(Base64Value=='=')
	{
		*DecValue=0;
		return 1;
	}
	return 0;
}

int DecodeQuotedPrintable(char *Src,char *Dst,int DstLen)
{
#ifdef DEBUG_DECODEQUOTED
	char *DstTemp=Dst;
	DebugLog(DecodeFile,"<Decode Quoted><Input>%s</Input>",Src);
#endif
	for(int Counter=0;((char)*Src!=0) && DstLen && (Counter++<DstLen);Src++,Dst++)
		if(*Src=='=')
		{
			char First,Second;
			if(!FromHexa(*(++Src),&First))
			{
				*Dst++='=';Src--;
				continue;
			}
			if(!FromHexa(*(++Src),&Second))
			{
				*Dst++='=';Src--;Src--;
				continue;
			}
			*Dst=(char)(First)<<4;
			*Dst+=Second;
		}
		else if(*Src=='_')
			*Dst=' ';
		else
			*Dst=*Src;
	*Dst=(char)0;
#ifdef DEBUG_DECODEQUOTED
	DebugLog(DecodeFile,"<Output>%s</Output></Decode Quoted>",DstTemp);
#endif
	return 1;
}

int DecodeBase64(char *Src,char *Dst,int DstLen)
{
	int Result=0;
	char Locator=0,MiniResult[4];
	char *End=Dst+DstLen;

	MiniResult[0]=MiniResult[1]=MiniResult[2]=MiniResult[3]=0;

#ifdef DEBUG_DECODEBASE64
	char *DstTemp=Dst;
	DebugLog(DecodeFile,"<Decode Base64><Input>\n%s\n</Input>\n",Src);
#endif
	while(*Src!=0 && DstLen && Dst!=End)
	{
		if((!(Result=FromBase64(*Src,MiniResult+Locator)) && (*Src==0)) || Locator++==3)	//end_of_str || end_of_4_bytes
		{
			Locator=0;									//next write to the first byte
			*Dst++=(char)((MiniResult[0]<<2) | (MiniResult[1]>>4));
			if(Dst==End) goto end;								//DstLen exceeded?
			*Dst++=(char)((MiniResult[1]<<4) | (MiniResult[2]>>2));
			if(Dst==End) goto end;								//someones don't like goto, but not me
			*Dst++=(char)((MiniResult[2]<<6) | MiniResult[3]);
			if(!Result && (*Src==0)) goto end;						//end of string?
			MiniResult[0]=MiniResult[1]=MiniResult[2]=MiniResult[3]=0;			//zero 4byte buffer for next loop
		}
		if(!Result) return 0;									//unrecognised character occured
		Src++;
	}
end:
	*Dst=0;
#ifdef DEBUG_DECODEBASE64
	DebugLog(DecodeFile,"<Output>\n%s\n</Output></Decode Base64>",DstTemp);
#endif
	return 1;
}

int ConvertStringToUnicode(char *stream,unsigned int cp,WCHAR **out)
{
	CPINFO CPInfo;
	WCHAR *temp,*src=*out,*dest;
	int streamlen,outlen,Index;

	//codepages, which require to have set 0 in dwFlags parameter when calling MultiByteToWideChar
	DWORD CodePagesZeroFlags[]={50220,50221,50222,50225,50227,50229,52936,54936,57002,57003,57004,57005,57006,57007,57008,57009,57010,57011,65000,65001};

	if((cp!=CP_ACP) && (cp!=CP_OEMCP) && (cp!=CP_MACCP) && (cp!=CP_THREAD_ACP) && (cp!=CP_SYMBOL) && (cp!=CP_UTF7) && (cp!=CP_UTF8) && !GetCPInfo(cp,&CPInfo))
		cp=CP_OEMCP;
#ifdef DEBUG_DECODECODEPAGE
	DebugLog(DecodeFile,"<CodePage #>%d</CodePage #>",cp);
#endif
		
	for(Index=0;Index<sizeof(CodePagesZeroFlags)/sizeof(CodePagesZeroFlags[0]);Index++)
		if(CodePagesZeroFlags[Index]==cp)
		{
			Index=-1;
			break;
		}
	if(Index==-1)
		streamlen=MultiByteToWideChar(cp,0,stream,-1,NULL,0);
	else
		streamlen=MultiByteToWideChar(cp,MB_USEGLYPHCHARS,stream,-1,NULL,0);

	if(*out!=NULL)
		outlen=wcslen(*out);
	else
		outlen=0;
	temp=new WCHAR[streamlen+outlen+1];

	if(*out!=NULL)
	{
		for(dest=temp;*src!=(WCHAR)0;src++,dest++)				//copy old string from *out to temp
			*dest=*src;
//		*dest++=L' ';								//add space?
		delete[] *out;
	}
	else
		dest=temp;
	*out=temp;
	
	if(Index==-1)
	{
		if(!MultiByteToWideChar(cp,0,stream,-1,dest,streamlen))
			return 0;
	}
	else
	{
		if(!MultiByteToWideChar(cp,MB_USEGLYPHCHARS,stream,-1,dest,streamlen))
			return 0;
	}
	return 1;
}

void ConvertCodedStringToUnicode(char *stream,WCHAR **storeto,DWORD cp,int mode)
{
	char *start=stream,*finder,*finderend;
	char Encoding=0;
	char *DecodedResult=NULL;

	if(stream==NULL)
		return;

	while(WS(start)) start++;
	while(*start!=0)
	{
		if(CODES(start))
		{
			finder=start+2;finderend=finder;
			while(!CODED(finderend) && !EOS(finderend)) finderend++;
			if(CODED(finderend))
			{
				Encoding=*(finderend+1);
				switch(Encoding)
				{
					case 'b':
					case 'B':
					case 'q':
					case 'Q':
						break;
					default:
						goto NotEncoded;
				}
				if(-1==(cp=GetCharsetFromString(finder,finderend-finder)))
					cp=CP_ACP;
				if(Encoding!=0)
				{
					int size,codeend;
					char *pcodeend;

					finder=finderend+2;
					if(CODED(finder))
						finder++;
					while(WS(finder)) finder++;
					finderend=finder;
					while(!CODEE(finderend) && !EOS(finderend)) finderend++;
					if(codeend=CODEE(finderend))
						pcodeend=finderend;
					while(WS(finderend-1)) finderend--;
				        if((mode==MIME_MAIL) && (((*finder=='"') && (*(finderend-1)=='"'))))
					{
				                finder++;
				                finderend--;
					}
					*finderend=(char)0;
					switch(Encoding)
					{
						case 'b':
						case 'B':
							size=(finderend-finder)*3/4+3+1+1;
							break;
						case 'q':
						case 'Q':
							size=finderend-finder+1+1;
							break;
					}
					if(DecodedResult!=NULL)
						delete[] DecodedResult;
					DecodedResult=new char[size+1];
					switch(Encoding)
					{
						case 'q':
						case 'Q':
							DecodeQuotedPrintable(finder,DecodedResult,size);
							break;
						case 'b':
						case 'B':
							DecodeBase64(finder,DecodedResult,size);
							break;
					}
					if(codeend)
						finderend=pcodeend+2;
					if(WS(finderend))	//if string continues and there's some whitespace, add space to string that is to be converted
					{
						int len=strlen(DecodedResult);
						DecodedResult[len]=' ';
						DecodedResult[len+1]=0;
						finderend++;
					}
					if(!ConvertStringToUnicode(DecodedResult,cp,storeto))
						continue;
				}
				else if(!ConvertStringToUnicode(start,cp,storeto))
					continue;
			}
			else if(!ConvertStringToUnicode(start,cp,storeto))
				continue;
		}
		else
		{
NotEncoded:
			finderend=start;
			while(!EOS(finderend) && !CODES(finderend)) finderend++;
			if(DecodedResult!=NULL)
				delete[] DecodedResult;
			DecodedResult=new char[finderend-start+2];
			for(finder=DecodedResult;start!=finderend;start++,finder++)
				*finder=*start;
			*finder=0;
			if(!ConvertStringToUnicode(DecodedResult,cp,storeto))
				continue;
		}
		while(WS(finderend)) finderend++;
		start=finderend;
	}
	if(DecodedResult!=NULL)
		delete[] DecodedResult;
}
