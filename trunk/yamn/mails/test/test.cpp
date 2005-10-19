/*
 * This file is for testing purposes. Save in header.txt your problem header and you can
 * browse through functions to get result
 *
 * (c) majvan 2002-2004
 */

#include <stdio.h>
#include "../m_mails.h"

extern void WINAPI TranslateHeaderFcn(char *stream,int len,struct CMimeItem **head);
extern void ExtractHeader(struct CMimeItem *items,int CP,struct CHeader *head);

void main()
{
	char Buffer[8192];			//we do not suppose longer header
	FILE *fp;
	YAMNMAIL Mail;
	struct CHeader ExtractedHeader;

	if(NULL==(fp=fopen("header.txt","r")))
		return;
	fread(Buffer,sizeof(Buffer),1,fp);
	if(ferror(fp))
	{
		fclose(fp);
		return;
	}
	fclose(fp);
	TranslateHeaderFcn(Buffer,strlen(Buffer),&Mail.TranslatedHeader);
	ExtractHeader(Mail.TranslatedHeader,CP_ACP,&ExtractedHeader);
	return;
}